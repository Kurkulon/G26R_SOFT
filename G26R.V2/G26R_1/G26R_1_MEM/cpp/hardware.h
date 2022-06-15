#ifndef HARDWARE_H__23_12_2013__11_37
#define HARDWARE_H__23_12_2013__11_37

#include "types.h"
//#include "core.h"
#include "time.h"
#include <i2c.h>
#include <spi.h>
#include "hw_nand.h"
#include "hw_rtm.h"
#include "manch.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


extern void Hardware_Init();

extern void UpdateHardware();

extern u16 CRC_CCITT_PIO(const void *data, u32 len, u16 init);
extern u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init);
extern bool CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init);
extern bool CRC_CCITT_DMA_CheckComplete(u16* crc);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void InitHardware();
extern void UpdateHardware();
extern void SetClock(const RTC &t);

extern void EnableDSP();	
extern void DisableDSP();	

extern void EnableDSP();	
extern void DisableDSP();	
//extern void DSP_CopyDataDMA(volatile void *src, volatile void *dst, u16 len);
//extern bool DSP_CheckDataComplete();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__forceinline u32 Push_IRQ()
{
	register u32 t;

#ifndef WIN32

	register u32 primask __asm("primask");

	t = primask;

	__disable_irq();

#endif

	return t;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__forceinline void Pop_IRQ(u32 t)
{
#ifndef WIN32

	register u32 primask __asm("primask");

	primask = t;

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WIN32

extern void I2C_Destroy();
extern void SPI_Destroy();
extern void UpdateDisplay();
extern int PutString(u32 x, u32 y, byte c, const char *str);
extern int Printf(u32 x, u32 y, byte c, const char *format, ... );

extern void NAND_FlushBlockBuffers();
extern void NAND_ReqFlushBlockBuffers();

#endif

#endif // HARDWARE_H__23_12_2013__11_37
