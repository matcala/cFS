/*******************************************************************************
** File:
**   ARANYA_EP_msg.h
**
** Purpose:
**  Define ARANYA_EP application commands and telemetry messages
**
** Notes:
**  This file combines and replaces _msgdefs.h and _msgstruct.h files
**
*******************************************************************************/
#ifndef _ARANYA_EP_MSG_H_
#define _ARANYA_EP_MSG_H_

#include "cfe_msg.h"
#include "aranya_ep_fcncodes.h"

/*
** Generic "no arguments" command type definition
** Executed command depends on the command code in the command header
*/
typedef struct
{
    /* Every command requires a header used to identify it */
    CFE_MSG_CommandHeader_t CmdHeader;

} ARANYA_EP_NoopCmd_t, ARANYA_EP_ResetCmd_t;

/* 
** Experiment commands with 128-byte payload 
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader;
    uint8                   Data[128];
} ARANYA_EP_Exp1Cmd_t, ARANYA_EP_Exp2Cmd_t;


/*
** Housekeeping packet definition
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    uint8                     CmdCounter;
    uint8                     ErrCounter;
    uint32                    AuthorizedCount;
    uint32                    DeniedCount;
    uint32                    LastAuthResult; /* 0=unknown, 1=authorized, 2=denied, 3=aranya_err */
    uint32                    DestMsgIdVal;   /* current destination MID value */

} __attribute__((packed)) ARANYA_EP_HkTlm_t;
#define ARANYA_EP_HK_LNGTH sizeof(ARANYA_EP_HkTlm_t)

/*
** Onboard Announce telemetry packet definition
**
** Used to communicate successful connection to Aranya daemon and retrieval of existing state.
** Includes device ID, key bundle length, and a hash string of the key bundle.
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TlmHeader;
    /* Human-readable device ID (null-terminated) */
    char DeviceId[128];
    /* Serialized key bundle length in bytes */
    uint32 KeyBundleLen;
    /* Hex-encoded hash string of the key bundle (null-terminated).
       Using 64-bit FNV-1a -> 16 hex chars + NUL (up to 65 reserved for future expansion) */
    char KeyBundleHash[65];

} __attribute__((packed)) ARANYA_EP_OnboardAnnounceTlm_t;
#define ARANYA_EP_ONBOARD_ANNOUNCE_LNGTH sizeof(ARANYA_EP_OnboardAnnounceTlm_t)

#endif /* _ARANYA_EP_MSG_H_ */
