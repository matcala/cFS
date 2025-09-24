// #include "cfe.h"
// #include "cfe_evs.h"
// #include "cfe_es.h"
// #include "cfe_sb.h"
// #include "cfe_msg.h"
// #include "osapi.h"
#include "aranya_ep_app.h"          // un-commented to get prototypes/defines
#include "aranya_ep_eventids.h"   // event IDs
#include "aranya-client.h"          // ensure Aranya C API symbols visible
#include <string.h>
#include <stdbool.h>

/* Define default file permissions if not already defined */
#ifndef OS_DEFAULT_FILE_PERMISSIONS
#define OS_DEFAULT_FILE_PERMISSIONS 0777
#endif

/* Global app data - defined here, declared extern in header */
ARANYA_EP_AppData_t ARANYA_EP_App;

/* Aranya presence smoke-test */
static void ARANYA_EP_AranyaLibTest(void)
{
    struct AranyaExtError ext;
    memset(&ext, 0, sizeof(ext));
    size_t need = 0;
    (void)aranya_ext_error_msg(&ext, NULL, &need);
    CFE_EVS_SendEvent(ARANYA_EP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "Aranya API presence OK (ext msg need=%lu)", (unsigned long)need);
}

/* Initialization patterned after SAMPLE_APP_Init */
CFE_Status_t ARANYA_EP_Init(void)
{
    CFE_Status_t status;

    memset(&ARANYA_EP_App, 0, sizeof(ARANYA_EP_App));
    ARANYA_EP_App.RunStatus = CFE_ES_RunStatus_APP_RUN;
    ARANYA_EP_App.DestMsgId = CFE_SB_INVALID_MSG_ID;
    // strncpy(ARANYA_EP_App.UdsPath, ARANYA_EP_DEFAULT_UDS_PATH, sizeof(ARANYA_EP_App.UdsPath) - 1);
    // ARANYA_EP_App.UdsPath[sizeof(ARANYA_EP_App.UdsPath) - 1] = '\0';
    // (void)OS_mkdir("/ram/aranya", OS_DEFAULT_FILE_PERMISSIONS); /* Ensure dir exists */

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ARANYA_EP: EVS Register failed RC=0x%08lX\n", (unsigned long)status);
        return status;
    }

    /* Create command pipe */
    status = CFE_SB_CreatePipe(&ARANYA_EP_App.CmdPipeId, ARANYA_EP_PIPE_DEPTH, "ARANYA_EP_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Pipe create failed RC=0x%08lX", (unsigned long)status);
        return status;
    }

    /* Subscribe CMD MID */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ARANYA_EP_CMD_MID), ARANYA_EP_App.CmdPipeId);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Subscribe CMD failed RC=0x%08lX", (unsigned long)status);
        return status;
    }

    /* Subscribe HK request MID */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ARANYA_EP_SEND_HK_MID), ARANYA_EP_App.CmdPipeId);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Subscribe HK failed RC=0x%08lX", (unsigned long)status);
        return status;
    }

    // /* Aranya library smoke test */
    ARANYA_EP_AranyaLibTest();

    // /* Attempt Aranya client init (non-fatal) */
    // if (!ARANYA_EP_InitAranya())
    // {
    //     CFE_EVS_SendEvent(ARANYA_EP_ARANYA_ERR_EID, CFE_EVS_EventType_ERROR,
    //                       "Aranya client init failed; authorization disabled");
    // }

    CFE_EVS_SendEvent(ARANYA_EP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "ARANYA_EP Initialized v%u.%u.%u.%u",
                      ARANYA_EP_MAJOR_VERSION, ARANYA_EP_MINOR_VERSION,
                      ARANYA_EP_REVISION, ARANYA_EP_MISSION_REV);

    return CFE_SUCCESS;
}

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

// /* Aranya client bring-up (re-enabled) */
// static bool ARANYA_EP_InitAranya(void)
// {
//     struct AranyaClientConfig cfg;
//     memset(&cfg, 0, sizeof(cfg));
//     struct AranyaExtError ext;
//     memset(&ext, 0, sizeof(ext));

//     AranyaError rc = aranya_client_init_ext(&ARANYA_EP_App.ArClient, &cfg, &ext);
//     if (rc != ARANYA_ERROR_SUCCESS)
//     {
//         char    buf[128];
//         size_t  need = 0;
//         aranya_ext_error_msg(&ext, NULL, &need);
//         if (need < sizeof(buf))
//         {
//             aranya_ext_error_msg(&ext, buf, &need);
//             CFE_EVS_SendEvent(ARANYA_EP_ARANYA_ERR_EID, CFE_EVS_EventType_ERROR,
//                               "aranya_client_init_ext: %s", buf);
//         }
//         return false;
//     }

//     struct AranyaDeviceId devid;
//     rc = aranya_get_device_id_ext(&ARANYA_EP_App.ArClient, &devid, &ext);
//     if (rc == ARANYA_ERROR_SUCCESS)
//     {
//         CFE_EVS_SendEvent(ARANYA_EP_SOCKET_INF_EID, CFE_EVS_EventType_INFORMATION,
//                           "Aranya device id obtained");
//     }
//     ARANYA_EP_App.ArClientInit = true;
//     return true;
// }

// /* Simple authorization placeholder */
// static bool ARANYA_EP_AuthorizeForward(void)
// {
//     if (!ARANYA_EP_App.ArClientInit)
//     {
//         ARANYA_EP_App.LastAuthResult = ARANYA_EP_AUTH_ERROR;
//         return false;
//     }
//     ARANYA_EP_App.LastAuthResult = ARANYA_EP_AUTH_ALLOWED;
//     return true;
// }

/* Command dispatcher (takes buffer pointer now) */
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
    // else if (CFE_SB_MsgId_Equal(msgid, CFE_SB_ValueToMsgId(ARANYA_EP_SEND_HK_MID)))
    // {
    //     ARANYA_EP_SendHousekeeping();
    // }
    else
    {
        ARANYA_EP_App.ErrCounter++;
        CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid MsgId=0x%08lX",
                          (unsigned long)CFE_SB_MsgIdToValue(msgid));
    }
}

/* Ground command processor updated to use passed buffer */
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
    /* Other command cases (RESET, SET_DEST, SET_UDS, FORWARD) remain commented for now */
    default:
        ARANYA_EP_App.ErrCounter++;
        CFE_EVS_SendEvent(ARANYA_EP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid CC=%u", (unsigned)FcnCode);
        break;
    }
}

// /* Housekeeping (re-enabled) */
// static void ARANYA_EP_SendHousekeeping(void)
// {
//     ARANYA_EP_HkTlm_t hk;
//     memset(&hk, 0, sizeof(hk));
//     CFE_MSG_Init(&hk.TlmHeader, CFE_SB_ValueToMsgId(ARANYA_EP_HK_TLM_MID), sizeof(hk));
//     hk.CmdCounter      = ARANYA_EP_App.CmdCounter;
//     hk.ErrCounter      = ARANYA_EP_App.ErrCounter;
//     hk.AuthorizedCount = ARANYA_EP_App.AuthorizedCount;
//     hk.DeniedCount     = ARANYA_EP_App.DeniedCount;
//     hk.LastAuthResult  = ARANYA_EP_App.LastAuthResult;
//     hk.DestMsgIdVal    = CFE_SB_MsgIdToValue(ARANYA_EP_App.DestMsgId);
//     CFE_SB_TransmitMsg((CFE_MSG_Message_t *)&hk, true);
// }

/* Central length verifier */
int32 ARANYA_EP_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t Expected)
{
    size_t actual = 0;
    CFE_MSG_GetSize(MsgPtr, &actual);
    if (actual != Expected)
    {
        CFE_SB_MsgId_t mid;
        CFE_MSG_FcnCode_t fc;
        CFE_MSG_GetMsgId(MsgPtr, &mid);
        CFE_MSG_GetFcnCode(MsgPtr, &fc);
        CFE_EVS_SendEvent(ARANYA_EP_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Length error MID=0x%08lX FC=%u Got=%lu Exp=%lu",
                          (unsigned long)CFE_SB_MsgIdToValue(mid),
                          (unsigned)fc, (unsigned long)actual, (unsigned long)Expected);
        ARANYA_EP_App.ErrCounter++;
        return CFE_SB_BAD_ARGUMENT;
    }
    return CFE_SUCCESS;
}
