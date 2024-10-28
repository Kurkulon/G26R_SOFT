#ifndef HARDWARE_H__23_12_2013__11_37
#define HARDWARE_H__23_12_2013__11_37

#include "types.h"
//#include "core.h"
#include "time.h"
#include <i2c.h>
#include <spi.h>
#include "hw_nand.h"
#include "hw_rtm.h"
#include "MANCH\manch.h"
#include "hw_conf.h"
//#include "FLASH\flash.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//#define UNIBUF_LEN (3072)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//struct UNIBUF : public PtrItem<UNIBUF>
//{
//	UNIBUF() : dataOffset(sizeof(VecData::Hdr)), dataLen(0)  { /*freeBufList.Add(this);*/ }
//
//	u16		dataOffset;
//	u16 	dataLen;
//
//	byte	data[UNIBUF_LEN]; // Последние 2 байта CRC16
//
//	void*	GetDataPtr() { return data+dataOffset; }
//};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


extern void Hardware_Init();

extern void UpdateHardware();


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void InitHardware();
extern void UpdateHardware();
extern void SetClock(const RTC &t);

#ifndef WIN32

__forceinline void EnableLPC()	{ PIO_RESET->CLR(RESET); 		}
__forceinline void DisableLPC()	{ PIO_RESET->SET(RESET); 		}

__forceinline void EnableDSP()	{ PIO_DSP_RESET->SET(DSP_RESET); 		}
__forceinline void DisableDSP()	{ PIO_DSP_RESET->CLR(DSP_RESET); 		}

#else

__forceinline void EnableLPC()		{		}
__forceinline void DisableLPC()	{		}

__forceinline void EnableDSP()		{ 		}
__forceinline void DisableDSP()	{ 		}

#endif

//__forceinline void Set_DSP_BootMode_UART()	{ PIO_DSP_BMODE1->CLR(DSP_BMODE1); 		}
//__forceinline void Set_DSP_BootMode_SPI()	{ PIO_DSP_BMODE1->SET(DSP_BMODE1); 	}

//extern void DSP_CopyDataDMA(volatile void *src, volatile void *dst, u16 len);
//extern bool DSP_CheckDataComplete();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__forceinline u32 Push_IRQ()
//{
//#ifndef WIN32
//	register u32 t;
//
//	register u32 primask __asm("primask");
//
//	t = primask;
//
//	__disable_irq();
//
//	return t;
//
//#else
//
//	return 0;
//
//#endif
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__forceinline void Pop_IRQ(u32 t)
//{
//#ifndef WIN32
//
//	register u32 primask __asm("primask");
//
//	primask = t;
//
//#endif
//}

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
