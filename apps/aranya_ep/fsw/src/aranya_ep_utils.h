/*******************************************************************************
** File: aranya_ep_utils.h
**
** Purpose:
**   This file contains utility function declarations for the ARANYA_EP application.
**
*******************************************************************************/
#ifndef ARANYA_EP_UTILS_H
#define ARANYA_EP_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include "cfe.h"

/* Initialization helpers */
int32 ARANYA_EP_Init(void);
bool  ARANYA_EP_InitAranya(void);

/* Support / utility routines */
void  ARANYA_EP_AranyaLibTest(void);
void  ARANYA_EP_SendHousekeeping(void);
void  ARANYA_EP_SendOnboardAnnounce(void);
bool  ARANYA_EP_ValidateUdsPath(const char *Path);
int32 ARANYA_EP_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t Expected);

/* FS utility functions */
int32 ARANYA_EP_MountAndList(const char *dev_name, const char *mount_point, const char *dir_path);
int32 ARANYA_EP_ListDir(const char *dir_path);

#endif /* ARANYA_EP_UTILS_H */
