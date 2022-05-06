#include "types.h"
#include "core.h"
#include "time.h"
#include "COM_DEF.h"
#include "CRC16_8005.h"
#include "list.h"
#include "PointerCRC.h"

#include "hardware.h"
#include "SEGGER_RTT.h"
#include "hw_conf.h"
#include "hw_rtm.h"


#ifdef WIN32

#include <windows.h>
#include <Share.h>
#include <conio.h>
#include <stdarg.h>
#include <stdio.h>
#include <intrin.h>
#include "CRC16_CCIT.h"
#include "list.h"

static HANDLE handleNandFile;
static const char nameNandFile[] = "NAND_FLASH_STORE.BIN";

static HANDLE handleWriteThread;
static HANDLE handleReadThread;

static byte nandChipSelected = 0;

static u64 curNandFilePos = 0;
//static u64 curNandFileBlockPos = 0;
static u32 curBlock = 0;
static u32 curRawBlock = 0;
static u16 curPage = 0;
static u16 curCol = 0;

static OVERLAPPED	_overlapped;
static u32			_ovlReadedBytes = 0;
static u32			_ovlWritenBytes = 0;

static void* nandEraseFillArray;
static u32 nandEraseFillArraySize = 0;
static byte nandReadStatus = 0x41;
static u32 lastError = 0;


static byte fram_I2c_Mem[0x10000];
static byte fram_SPI_Mem[0x40000];

static bool fram_spi_WREN = false;

static u16 crc_ccit_result = 0;


struct BlockBuffer { BlockBuffer *next; u32 block; u32 prevBlock; u32 writed; u32 data[((NAND_PAGE_SIZE+NAND_SPARE_SIZE) << NAND_PAGE_BITS) >> 2]; };

static BlockBuffer _blockBuf[16];

static List<BlockBuffer> freeBlockBuffer;
static List<BlockBuffer> rdBlockBuffer;
static List<BlockBuffer> wrBlockBuffer;

static BlockBuffer *curNandBlockBuffer[4] = { 0 };

static volatile bool busyWriteThread = false;

#elif defined(CPU_SAME53)

#elif defined(CPU_XMC48)

#endif 


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

List<DSCI2C>	i2c_ReqList;
DSCI2C*			i2c_dsc = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool I2C_Update()
{
#ifdef CPU_SAME53	

	enum STATE { WAIT = 0, WRITE, READ, STOP };

	static STATE state = WAIT;
	static byte *wrPtr = 0;
	static byte *rdPtr = 0;
	static u16 	wrCount = 0;
	static u16 	rdCount = 0;
	static byte *wrPtr2 = 0;
	static u16	wrCount2 = 0;
	static byte adr = 0;
	static __align(16) T_HW::DMADESC wr_dmadsc;

	switch (state)
	{
		case WAIT:

			i2c_dsc = i2c_ReqList.Get();

			if (i2c_dsc != 0)
			{
				DSCI2C &dsc = *i2c_dsc;

				dsc.ready = false;
				dsc.ack = false;

				wrPtr = (byte*)dsc.wdata;	
				rdPtr = (byte*)dsc.rdata;	
				wrPtr2 = (byte*)dsc.wdata2;	
				wrCount = dsc.wlen;
				wrCount2 = dsc.wlen2;
				rdCount = dsc.rlen;
				adr = dsc.adr;

				if (wrPtr2 == 0) wrCount2 = 0;

				I2C->CTRLB = I2C_SMEN;
				I2C->STATUS.BUSSTATE = BUSSTATE_IDLE;

				I2C->INTFLAG = ~0;

				T_HW::DMADESC &dmadsc = DmaTable[I2C_DMACH];

				if (wrCount == 0)
				{
					dmadsc.SRCADDR	= &I2C->DATA;
					dmadsc.DSTADDR	= rdPtr + rdCount;
					dmadsc.DESCADDR = 0;
					dmadsc.BTCNT	= rdCount;
					dmadsc.BTCTRL	= DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_DSTINC;

					__disable_irq();

					HW::DMAC->CH[I2C_DMACH].INTENCLR = ~0;
					HW::DMAC->CH[I2C_DMACH].INTFLAG = ~0;
					HW::DMAC->CH[I2C_DMACH].CTRLA = DMCH_ENABLE|DMCH_TRIGACT_BURST|I2C_TRIGSRC_RX;

					__enable_irq();

					I2C->ADDR = ((rdCount <= 255) ? (I2C_LEN(rdCount)|I2C_LENEN) : 0) | (adr << 1) | 1;
					state = READ; 
				}
				else
				{
					dmadsc.SRCADDR	= wrPtr + wrCount;
					dmadsc.DSTADDR	= &I2C->DATA;
					dmadsc.BTCNT	= wrCount;
					dmadsc.BTCTRL	= DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_SRCINC;

					if (wrCount2 == 0)
					{
						dmadsc.DESCADDR = 0;
					}
					else
					{
						wr_dmadsc.SRCADDR	= wrPtr2 + wrCount2;
						wr_dmadsc.DSTADDR	= &I2C->DATA;
						wr_dmadsc.BTCNT		= wrCount2;
						wr_dmadsc.BTCTRL	= DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_SRCINC;
						dmadsc.DESCADDR		= &wr_dmadsc;
					};

					__disable_irq();

					HW::DMAC->CH[I2C_DMACH].INTENCLR = ~0;
					HW::DMAC->CH[I2C_DMACH].INTFLAG = ~0;
					HW::DMAC->CH[I2C_DMACH].CTRLA = DMCH_ENABLE|DMCH_TRIGACT_BURST|I2C_TRIGSRC_TX;

					__enable_irq();

					I2C->ADDR = (adr << 1);
					state = WRITE; 
				};
			};

			break;

		case WRITE:

			if((I2C->INTFLAG & I2C_ERROR) || I2C->STATUS.RXNACK)
			{
				I2C->CTRLB = I2C_SMEN|I2C_CMD_STOP;
				
				state = STOP; 
			}
			else
			{
				DSCI2C &dsc = *i2c_dsc;

				__disable_irq();

				bool c = ((HW::DMAC->CH[I2C_DMACH].CTRLA & DMCH_ENABLE) == 0 || (HW::DMAC->CH[I2C_DMACH].INTFLAG & DMCH_TCMPL)) && (I2C->INTFLAG & I2C_MB);
				
				__enable_irq();

				if (c)
				{
					dsc.ack = true;

					if (rdCount > 0)
					{
						T_HW::DMADESC &dmadsc = DmaTable[I2C_DMACH];

						dmadsc.SRCADDR	= &I2C->DATA;
						dmadsc.DSTADDR	= rdPtr + rdCount;
						dmadsc.DESCADDR = 0;
						dmadsc.BTCNT	= rdCount;
						dmadsc.BTCTRL	= DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_DSTINC;

						__disable_irq();

						HW::DMAC->CH[I2C_DMACH].INTENCLR = ~0;
						HW::DMAC->CH[I2C_DMACH].INTFLAG = ~0;
						HW::DMAC->CH[I2C_DMACH].CTRLA = DMCH_ENABLE|DMCH_TRIGACT_BURST|I2C_TRIGSRC_RX;

						__enable_irq();

						I2C->ADDR = ((rdCount <= 255) ? (I2C_LEN(rdCount)|I2C_LENEN) : 0) | (adr << 1) | 1;
		
						state = READ; 
					}
					else
					{
						I2C->CTRLB = I2C_SMEN|I2C_ACKACT|I2C_CMD_STOP;
						
						state = STOP; 
					};
				};
			};

			break;

		case READ:

			if((I2C->INTFLAG & I2C_ERROR) || I2C->STATUS.RXNACK)
			{
				I2C->CTRLB = I2C_SMEN|I2C_ACKACT|I2C_CMD_STOP;
				
				state = STOP; 
			}
			else
			{
				DSCI2C &dsc = *i2c_dsc;

				__disable_irq();

				bool c = (HW::DMAC->CH[I2C_DMACH].CTRLA & DMCH_ENABLE) == 0 || (HW::DMAC->CH[I2C_DMACH].INTFLAG & DMCH_TCMPL);
				
				__enable_irq();

				if (c)
				{
					dsc.ack = true;

					dsc.readedLen = rdCount;

					I2C->CTRLB = I2C_SMEN|I2C_ACKACT|I2C_CMD_STOP;
						
					state = STOP; 
				};
			};

			break;

		case STOP:

			if (I2C->STATUS.BUSSTATE == BUSSTATE_IDLE)
			{
				i2c_dsc->ready = true;
				
				i2c_dsc = 0;
				
				I2C->CTRLB = I2C_SMEN;

				state = WAIT; 
			}
			else if (I2C->SYNCBUSY == 0)
			{
				I2C->CTRLB = I2C_SMEN|I2C_ACKACT|I2C_CMD_STOP;
			};

			break;
	};

#elif defined(CPU_XMC48)

#endif

	return false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool I2C_AddRequest(DSCI2C *d)
{
	if (d == 0) { return false; };
	if ((d->wdata == 0 || d->wlen == 0) && (d->rdata == 0 || d->rlen == 0)) { return false; }

#ifndef WIN32

	d->next = 0;
	d->ready = false;

	d->ready = false;

	i2c_ReqList.Add(d);

#else

	u16 adr;

	switch (d->adr)
	{
		case 0x49: //Temp

			if (d->rlen >= 2)
			{
				byte *p = (byte*)d->rdata;

				p[0] = 0;
				p[1] = 0;
			};
				
			d->readedLen = d->rlen;
			d->ack = true;
			d->ready = true;

			break;

		case 0x50: // FRAM

			d->readedLen = 0;

			if (d->wdata != 0 && d->wlen == 2)
			{
				adr = ReverseWord(*((u16*)d->wdata));

				adr %= sizeof(fram_I2c_Mem);

				if (d->wdata2 != 0 && d->wlen2 != 0)
				{
					u16 count = d->wlen2;
					byte *s = (byte*)d->wdata2;
					byte *d = fram_I2c_Mem + adr;

					while (count-- != 0) { *(d++) = *(s++); adr++; if (adr >= sizeof(fram_I2c_Mem)) { adr = 0; d = fram_I2c_Mem; }; };
				}
				else if (d->rdata != 0 && d->rlen != 0)
				{
					d->readedLen = d->rlen;
					u16 count = d->rlen;

					byte *p = (byte*)(d->rdata);
					byte *s = fram_I2c_Mem + adr;

					while (count-- != 0) { *(p++) = *(s++); adr++; if (adr >= sizeof(fram_I2c_Mem)) { adr = 0; s = fram_I2c_Mem; }; };
				};
			};

			d->ack = true;
			d->ready = true;

			break;
	};

#endif

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void I2C_Init()
{
#ifndef WIN32

	using namespace HW;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_CYAN "I2C Init ... ");

	#ifdef CPU_SAME53	

		HW::GCLK->PCHCTRL[GCLK_SERCOM3_CORE]	= GCLK_GEN(GEN_25M)|GCLK_CHEN;	// 25 MHz

		MCLK->APBBMASK |= APBB_SERCOM3;

		PIO_I2C->SetWRCONFIG(SCL|SDA, PORT_PMUX_C | PORT_PMUXEN | PORT_WRPMUX | PORT_PULLEN | PORT_WRPINCFG);
		PIO_I2C->SET(SCL|SDA);

		I2C->CTRLA = I2C_SWRST;

		while(I2C->SYNCBUSY);

		I2C->CTRLA = SERCOM_MODE_I2C_MASTER;

		I2C->CTRLA = SERCOM_MODE_I2C_MASTER|I2C_INACTOUT_205US|I2C_SPEED_SM;
		I2C->CTRLB = I2C_SMEN;
		I2C->BAUD = 0x0018;

		I2C->CTRLA |= I2C_ENABLE;

		while(I2C->SYNCBUSY);

		I2C->STATUS = 0;
		I2C->STATUS.BUSSTATE = BUSSTATE_IDLE;

	#elif defined(CPU_XMC48)

		HW::Peripheral_Enable(I2C_PID);

 		P5->ModePin0(A1OD);
		P5->ModePin2(A1PP);

		I2C->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(0);

		I2C->SCTR = I2C__SCTR;

		I2C->FDR = I2C__FDR;
		I2C->BRG = I2C__BRG;
	    
		I2C->TCSR = I2C__TCSR;

		I2C->PSCR = ~0;

		I2C->CCR = 0;

		I2C->DX0CR = I2C__DX0CR;
		I2C->DX1CR = I2C__DX1CR;

		I2C->CCR = I2C__CCR;


		I2C->PCR_IICMode = I2C__PCR;

		VectorTableExt[I2C_IRQ] = I2C_Handler;
		CM4::NVIC->CLR_PR(I2C_IRQ);
		CM4::NVIC->SET_ER(I2C_IRQ);

	#endif

#else

	HANDLE h;

	h = CreateFile("FRAM_I2C_STORE.BIN", GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

	if (h == INVALID_HANDLE_VALUE)
	{
		return;
	};

	dword bytes;

	ReadFile(h, fram_I2c_Mem, sizeof(fram_I2c_Mem), &bytes, 0);
	CloseHandle(h);

#endif

	SEGGER_RTT_WriteString(0, "OK\n");
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WIN32

void I2C_Destroy()
{
	HANDLE h;

	h = CreateFile("FRAM_I2C_STORE.BIN", GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);

	if (h == INVALID_HANDLE_VALUE)
	{
		return;
	};

	dword bytes;

	if (!WriteFile(h, fram_I2c_Mem, sizeof(fram_I2c_Mem), &bytes, 0))
	{
		dword le = GetLastError();
	};

	CloseHandle(h);
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
