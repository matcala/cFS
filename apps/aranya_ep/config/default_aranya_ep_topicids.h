/************************************************************************
** File:
**   $Id: sample_msgids.h  $
**
** Purpose:
**  Define ARANYA_EP Message IDs
**
*************************************************************************/
#ifndef _ARANYA_EP_TOPICIDS_H_
#define _ARANYA_EP_TOPICIDS_H_

/*
We get
EVS Port1 1980-012-14:03:20.56161 66/1/CFE_SB 7: Duplicate Subscription,MsgId 0x18a0 on ARANYA_EP_CMD_PIPE pipe,app ARANYA_EP_APP
from cFS so maybe the 0x18A0 is already taken?
or number 1 before 8A0 is not read.
TODO: fix double subscription
*/

/*
** CCSDS V1 Command Message IDs (MID) must be 0x18xx
*/
#define CFE_MISSION_ARANYA_EP_CMD_TOPICID          0x18A0


/*
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define CFE_MISSION_ARANYA_EP_SEND_HK_TOPICID      0x08A0
#define CFE_MISSION_ARANYA_EP_HK_TLM_TOPICID       0x08A0

#endif /* _ARANYA_EP_MSGIDS_H_ */
