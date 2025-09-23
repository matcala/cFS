#ifndef _ARANYA_EP_MSGDEFS_H_
#define _ARANYA_EP_MSGDEFS_H_

#include "common_types.h"
#include "aranya_ep_fcncodes.h"

typedef struct ARANYA_EP_HkTlm
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    uint8 spare[2];
} ARANYA_EP_HkTlm_t;

#endif
