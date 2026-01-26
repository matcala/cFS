#include "cfe.h"
#include "cfe_evs.h"
#include "cfe_es.h"
#include "cfe_sb.h"
#include "cfe_msg.h"
#include "osapi.h"
#include "aranya_ep_app.h"
#include "aranya_ep_utils.h"
#include "aranya_ep_eventids.h"
#include "aranya-client.h"
#include <string.h>
#include <stdbool.h>

/* SAMPLE_APP interface for command forwarding */
#include "sample_app_msgids.h"
#include "sample_app_fcncodes.h"
#include "sample_app_msg.h"

/* Global app data - defined here, declared extern in header */
ARANYA_EP_AppData_t ARANYA_EP_App;

/* Entry point (refactored to call ARANYA_EP_Init) */
void ARANYA_EP_AppMain(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(ARANYA_EP_PERF_ID);

    status = ARANYA_EP_Init();
    if (status != CFE_SUCCESS)
    {
        ARANYA_EP_App.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&ARANYA_EP_App.RunStatus))
    {
        CFE_ES_PerfLogExit(ARANYA_EP_PERF_ID);
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, ARANYA_EP_App.CmdPipeId, CFE_SB_PEND_FOREVER);
        CFE_ES_PerfLogEntry(ARANYA_EP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            ARANYA_EP_ProcessCommand(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ARANYA_EP: SB Read Error RC=0x%08lX", (unsigned long)status);
        }
    }

    CFE_ES_PerfLogExit(ARANYA_EP_PERF_ID);
    CFE_ES_ExitApp(ARANYA_EP_App.RunStatus);
}

/* Command dispatcher */
void ARANYA_EP_ProcessCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t    msgid = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t fcode = 0;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &msgid);

    if (CFE_SB_MsgId_Equal(msgid, CFE_SB_ValueToMsgId(ARANYA_EP_CMD_MID)))
    {
        CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &fcode);
        ARANYA_EP_ProcessGroundCommand(SBBufPtr, fcode);
    }
    else if (CFE_SB_MsgId_Equal(msgid, CFE_SB_ValueToMsgId(ARANYA_EP_SEND_HK_MID)))
    {
        ARANYA_EP_SendHousekeeping();
    }
    else
    {
        ARANYA_EP_App.ErrCounter++;
        CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid MsgId=0x%08lX",
                          (unsigned long)CFE_SB_MsgIdToValue(msgid));
    }
}

/* Ground command processor */
void ARANYA_EP_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr, CFE_MSG_FcnCode_t FcnCode)
{
    switch (FcnCode)
    {
    case ARANYA_EP_NOOP_CC:
        if (ARANYA_EP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ARANYA_EP_NoopCmd_t)) == CFE_SUCCESS)
        {
            ARANYA_EP_App.CmdCounter++;
            CFE_EVS_SendEvent(ARANYA_EP_NOOP_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "NOOP v%u.%u.%u.%u",
                              ARANYA_EP_MAJOR_VERSION, ARANYA_EP_MINOR_VERSION,
                              ARANYA_EP_REVISION, ARANYA_EP_MISSION_REV);
        }
        break;

    case ARANYA_EP_RESET_CC:
        if (ARANYA_EP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ARANYA_EP_ResetCmd_t)) == CFE_SUCCESS)
        {
            ARANYA_EP_App.CmdCounter      = 0;
            ARANYA_EP_App.ErrCounter      = 0;
            ARANYA_EP_App.AuthorizedCount = 0;
            ARANYA_EP_App.DeniedCount     = 0;
            ARANYA_EP_App.LastAuthResult  = ARANYA_EP_AUTH_UNKNOWN;
            CFE_EVS_SendEvent(ARANYA_EP_RESET_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "Counters reset");
        }
        break;

    case ARANYA_EP_EXP1_CC:
        if (ARANYA_EP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ARANYA_EP_Exp1Cmd_t)) == CFE_SUCCESS)
        {
            ARANYA_EP_App.CmdCounter++;
            CFE_EVS_SendEvent(ARANYA_EP_EXP1_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "EXP1 command received");

            OS_printf("[ARANYA_EP] Enforcing policy on CMD EXP1...\n");
            OS_printf("[ARANYA_EP] Command Accepted, instructing SAMPLE_APP (NOOP)...\n");

            /* Forward NOOP command to SAMPLE_APP */
            SAMPLE_APP_NoopCmd_t sample_cmd;
            CFE_Status_t         sample_status;

            CFE_MSG_Init(CFE_MSG_PTR(sample_cmd.CommandHeader),
                         CFE_SB_ValueToMsgId(SAMPLE_APP_CMD_MID),
                         sizeof(SAMPLE_APP_NoopCmd_t));
            CFE_MSG_SetFcnCode(CFE_MSG_PTR(sample_cmd.CommandHeader), SAMPLE_APP_NOOP_CC);
            sample_status = CFE_SB_TransmitMsg(CFE_MSG_PTR(sample_cmd.CommandHeader), true);

            if (sample_status != CFE_SUCCESS)
            {
                ARANYA_EP_App.ErrCounter++;
                CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Failed to send SAMPLE_APP NOOP command: 0x%08lX",
                                  (unsigned long)sample_status);
            }
            else
            {
                OS_printf("[ARANYA_EP] SAMPLE_APP NOOP command transmitted.\n");
            }
        }
        break;

    case ARANYA_EP_EXP2_CC:
        if (ARANYA_EP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ARANYA_EP_Exp2Cmd_t)) == CFE_SUCCESS)
        {
            ARANYA_EP_App.CmdCounter++;
            CFE_EVS_SendEvent(ARANYA_EP_EXP2_INF_EID, CFE_EVS_EventType_INFORMATION,
                              "EXP2 command received");

            OS_printf("[ARANYA_EP] Enforcing policy on CMD EXP2...\n");
            OS_printf("[ARANYA_EP] Command Accepted, instructing SAMPLE_APP (RESET_COUNTERS)...\n");

            /* Forward RESET_COUNTERS command to SAMPLE_APP */
            SAMPLE_APP_ResetCountersCmd_t sample_cmd;
            CFE_Status_t                  sample_status;

            CFE_MSG_Init(CFE_MSG_PTR(sample_cmd.CommandHeader),
                         CFE_SB_ValueToMsgId(SAMPLE_APP_CMD_MID),
                         sizeof(SAMPLE_APP_ResetCountersCmd_t));
            CFE_MSG_SetFcnCode(CFE_MSG_PTR(sample_cmd.CommandHeader), SAMPLE_APP_RESET_COUNTERS_CC);
            sample_status = CFE_SB_TransmitMsg(CFE_MSG_PTR(sample_cmd.CommandHeader), true);

            if (sample_status != CFE_SUCCESS)
            {
                ARANYA_EP_App.ErrCounter++;
                CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "Failed to send SAMPLE_APP RESET_COUNTERS command: 0x%08lX",
                                  (unsigned long)sample_status);
            }
            else
            {
                OS_printf("[ARANYA_EP] SAMPLE_APP RESET_COUNTERS command transmitted.\n");
            }
        }
        break;

    default:
        ARANYA_EP_App.ErrCounter++;
        CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid CC=%u", (unsigned)FcnCode);
        break;
    }
}
