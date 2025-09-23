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
** CCSDS V1 Command Message IDs (MID) must be 0x18xx
*/
#define CFE_MISSION_ARANYA_EP_CMD_TOPICID          0x18A0


/*
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define CFE_MISSION_ARANYA_EP_SEND_HK_TOPICID      0x08A0
#define CFE_MISSION_ARANYA_EP_HK_TLM_TOPICID       0x08A0

#endif /* _ARANYA_EP_MSGIDS_H_ */
