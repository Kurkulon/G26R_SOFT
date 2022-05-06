#ifndef HARDWARE_H__23_12_2013__11_37
#define HARDWARE_H__23_12_2013__11_37

#include "types.h"
//#include "core.h"
#include "time.h"
#include <i2c.h>
#include <spi.h>

//#define NAND_SAMSUNG
#define NAND_MICRON

#define NAND_CHIP_BITS		2
#define NAND_MAX_CHIP		(1<<NAND_CHIP_BITS)
#define NAND_CHIP_MASK		(NAND_MAX_CHIP-1)

#define K9K8_CHIP_BITS		3
#define K9K8_MAX_CHIP		(1<<3)
#define K9K8_CHIP_MASK		(K9K8_MAX_CHIP-1)
#define K9K8_COL_BITS		11
#define K9K8_BLOCK_BITS		13
#define K9K8_PAGE_BITS		6
#define K9K8_SPARE_SIZE		64
#define K9K8_PAGE_SIZE		(1 << K9K8_COL_BITS)
#define K9K8_RAWPAGE_MASK	((1 << (K9K8_PAGE_BITS + K9K8_CHIP_BITS + K9K8_BLOCK_BITS)) - 1)
#define K9K8_RAWBLOCK_MASK	((1 << (K9K8_CHIP_BITS + K9K8_BLOCK_BITS)) - 1)
#define K9K8_RAWADR_MASK	(((u64)1 << (K9K8_COL_BITS + K9K8_PAGE_BITS + K9K8_CHIP_BITS + K9K8_BLOCK_BITS)) - 1)

#define MT29_CHIP_BITS		3
#define MT29_MAX_CHIP		(1<<3)
#define MT29_CHIP_MASK		(MT29_MAX_CHIP-1)
#define MT29_COL_BITS		13
#define MT29_BLOCK_BITS		12
#define MT29_PAGE_BITS		7
#define MT29_SPARE_SIZE		448
#define MT29_PAGE_SIZE		(1 << MT29_COL_BITS)
#define MT29_RAWPAGE_MASK	((1 << (MT29_PAGE_BITS + MT29_CHIP_BITS + MT29_BLOCK_BITS)) - 1)
#define MT29_RAWBLOCK_MASK	((1 << (MT29_CHIP_BITS + MT29_BLOCK_BITS)) - 1)
#define MT29_RAWADR_MASK	(((u64)1 << (MT29_COL_BITS + MT29_PAGE_BITS + MT29_CHIP_BITS + MT29_BLOCK_BITS)) - 1)

#ifdef NAND_SAMSUNG

//#define NAND_MAX_CHIP			K9K8_MAX_CHIP	
//#define NAND_CHIP_MASK		K9K8_CHIP_MASK	
//#define NAND_CHIP_BITS		K9K8_CHIP_BITS	
#define NAND_COL_BITS		K9K8_COL_BITS			
#define NAND_BLOCK_BITS		K9K8_BLOCK_BITS		
#define NAND_PAGE_BITS		K9K8_PAGE_BITS		
#define NAND_SPARE_SIZE		K9K8_SPARE_SIZE		
#define NAND_PAGE_SIZE		K9K8_PAGE_SIZE		
#define NAND_RAWPAGE_MASK	K9K8_RAWPAGE_MASK	
#define NAND_RAWBLOCK_MASK	K9K8_RAWBLOCK_MASK	
#define NAND_RAWADR_MASK	K9K8_RAWADR_MASK	
#define nandType			K9K8 

#elif defined(NAND_MICRON)

//#define NAND_MAX_CHIP			MT29_MAX_CHIP
//#define NAND_CHIP_MASK		MT29_CHIP_MASK
//#define NAND_CHIP_BITS		MT29_CHIP_BITS
#define NAND_COL_BITS		MT29_COL_BITS		
#define NAND_BLOCK_BITS		MT29_BLOCK_BITS		
#define NAND_PAGE_BITS		MT29_PAGE_BITS		
#define NAND_SPARE_SIZE		MT29_SPARE_SIZE		
#define NAND_PAGE_SIZE		MT29_PAGE_SIZE		
#define NAND_RAWPAGE_MASK	MT29_RAWPAGE_MASK	
#define NAND_RAWBLOCK_MASK	MT29_RAWBLOCK_MASK	
#define NAND_RAWADR_MASK	MT29_RAWADR_MASK	
#define nandType			MT29 

#endif


#define NAND_SR_FAIL				0x01	// 0 = Pass, 1 = Fail
#define NAND_SR_FAILC				0x02	// 0 = Pass, 1 = Fail
#define NAND_SR_ARDY				0x20	// 0 = Busy, 1 = Ready
#define NAND_SR_RDY					0x40	// 0 = Busy, 1 = Ready
#define NAND_SR_WP					0x80	// 0 = Protected, 1 = Not Protected

#define FRAM_SPI_MAINVARS_ADR 0
#define FRAM_SPI_SESSIONS_ADR 0x200

#define FRAM_I2C_MAINVARS_ADR 0
#define FRAM_I2C_SESSIONS_ADR 0x200

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct NandID
{
 	byte maker;
 	byte device;
	byte data0;

	__packed union
	{
		__packed struct 
		{
			u16 pageSize			: 2;
			u16 z_reserved1			: 2;
			u16 blockSize			: 2;
			u16 z_reserved2			: 4;
			u16 planeNumber			: 2;
			u16 planeSize			: 3;
		};

	 	byte data[2];
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


extern void Hardware_Init();

extern void UpdateHardware();

extern u16 CRC_CCITT_PIO(const void *data, u32 len, u16 init);
extern u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init);
extern void CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init);
extern bool CRC_CCITT_DMA_CheckComplete(u16* crc);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//struct DSCI2C
//{
//	DSCI2C*			next;
//	void*			wdata;
//	void*			rdata;
//	void*			wdata2;
//	u16				wlen;
//	u16				wlen2;
//	u16				rlen;
//	u16				readedLen;
//	byte			adr;
//	volatile bool	ready;
//	volatile bool	ack;
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//extern bool Write_I2C(DSCI2C *d);
//inline bool Read_I2C(DSCI2C *d) { return Write_I2C(d); }
//extern bool Check_I2C_ready();
//extern bool I2C_AddRequest(DSCI2C *d);
//extern bool I2C_Update();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//struct DSCSPI
//{
//	DSCSPI*			next;
//	//u32				baud;
//	//u32				FDR;
//	void*			wdata;
//	void*			rdata;
//	u32				adr;
//	u16				alen;
//	u16				wlen;
//	u16				rlen;
//	volatile bool	ready;
//	byte			csnum;
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//extern bool SPI_AddRequest(DSCSPI *d);
//extern bool SPI_Update();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MRB
{
	bool	ready;
	bool	OK;
	u16		len;
	u16		maxLen;
	u16		*data;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct MTB
{
	bool		ready;
	u16			baud;
	u16			len1;
	const u16	*data1;
	u16			len2;
	const u16	*data2;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern bool RcvManData(MRB *mrb);
extern bool SendManData(MTB *mtb);
extern bool SendManData2(MTB* mtb);
//extern void SetTrmBoudRate(byte i);
extern void ManRcvUpdate();
//extern void ManRcvStop();

extern bool SendMLT3(MTB *mtb);

inline u16 GetRcvManQuality() { extern u16 rcvManQuality; return rcvManQuality; }

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
