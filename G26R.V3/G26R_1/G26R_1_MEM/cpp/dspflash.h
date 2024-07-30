#ifndef DSPFLASH_H__29_07_2024__17_19
#define DSPFLASH_H__29_07_2024__17_19

#include "core.h"

extern void at25df021_Init();
extern void at25df021_Destroy();
extern bool at25df021_Write(const byte *data, u32 stAdr, u32 count, bool verify);
extern bool BlackFin_CheckFlash(u16 *pcrc, u16 *plen);


#endif // DSPFLASH_H__29_07_2024__17_19 
