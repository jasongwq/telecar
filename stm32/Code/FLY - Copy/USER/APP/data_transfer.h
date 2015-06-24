#ifndef _DATA_TRANSFER_H_
#define _DATA_TRANSFER_H_
#ifdef __cplusplus
       extern "C" {
#endif

#include "sys.h"

typedef union
{
	u8 u8[4];
	u32 u32;
}_Uu32u8;//Ëø¶¨Ê±¼ä
extern _Uu32u8 TimeUnlock;
extern u8 TimeUnlockFlag;
extern u8 RelayStata[8];
#ifdef __cplusplus
        }
#endif
#endif

