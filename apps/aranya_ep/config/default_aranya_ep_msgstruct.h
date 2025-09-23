#ifndef _ARANYA_EP_MSGSTRUCT_H_
#define _ARANYA_EP_MSGSTRUCT_H_

/************************************************************************
 * Includes
 ************************************************************************/

#include "cfe_msg_hdr.h"
#include "aranya_ep_mission_cfg.h"

/*************************************************************************/

/*************************************************************************/
/* Type definition (Aranya_EP commands) */
/*************************************************************************/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ARANYA_EP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ARANYA_EP_ResetCountersCmd_t;


/*************************************************************************/
/* Type definition (Aranya_EP housekeeping) */
/*************************************************************************/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ARANYA_EP_SendHkCmd_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry header */
    ARANYA_EP_HkTlm_t         Payload;         /**< \brief Telemetry payload */
} ARANYA_EP_HkTlmPkt_t;

#endif /* _ARANYA_EP_MSGSTRUCT_H_ */