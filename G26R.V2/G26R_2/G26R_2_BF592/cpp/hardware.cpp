#include "hardware.h"

#include <bfrom.h>
#include <sys\exception.h>
//#include <cdefBF592-A.h>
//#include <ccblkfn.h>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void LowLevelInit();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

byte bitGain[16] = {0, 2, 3, 6, 7, 10, 11, 14, 15, 15, 15, 15, 15, 15, 15, 15 };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitRTT()
{
	*pTIMER0_CONFIG = PERIOD_CNT|PWM_OUT|OUT_DIS;
	*pTIMER0_PERIOD = 0xFFFFFFFF;
	*pTIMER_ENABLE = TIMEN0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_PLL()
{
	u32 SIC_IWR1_reg;                /* backup SIC_IWR1 register */

	/* use Blackfin ROM SysControl() to change the PLL */
    ADI_SYSCTRL_VALUES sysctrl = {	VRCTL_VALUE,
									PLLCTL_VALUE,		/* (25MHz CLKIN x (MSEL=16))::CCLK = 400MHz */
									PLLDIV_VALUE,		/* (400MHz/(SSEL=4))::SCLK = 100MHz */
									PLLLOCKCNT_VALUE,
									PLLSTAT_VALUE };

	/* use the ROM function */
	bfrom_SysControl( SYSCTRL_WRITE | SYSCTRL_PLLCTL | SYSCTRL_PLLDIV, &sysctrl, 0);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

volatile bool defPPI_Ready = false;
static volatile bool *ppi_Ready = &defPPI_Ready;

EX_INTERRUPT_HANDLER(PPI_ISR)
{
	if (*pDMA0_IRQ_STATUS & 1)
	{
		*pDMA0_IRQ_STATUS = 1;
		*pPPI_CONTROL = 0;
		*pDMA0_CONFIG = 0;
		*ppi_Ready = true;

		*pTIMER_DISABLE = TIMDIS1;
		*pPORTFIO_CLEAR = 1<<9; // SYNC 
	}
	else if (*pDMA0_IRQ_STATUS & 2)
	{
		*pDMA0_IRQ_STATUS = 2;
		*pPPI_CONTROL = 0;
		*pDMA0_CONFIG = 0;
		*ppi_Ready = true;

		*pTIMER_DISABLE = TIMDIS1;
		*pPORTFIO_CLEAR = 1<<9; // SYNC 
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EX_INTERRUPT_HANDLER(TIMER_PPI_ISR)
{
	*pDMA0_CONFIG = FLOW_STOP|DI_EN|WDSIZE_16|SYNC|WNR|DMAEN;
	*pPPI_CONTROL = FLD_SEL|PORT_CFG|POLC|DLEN_12|XFR_TYPE|PORT_EN;
	*pTIMER_ENABLE = TIMEN1;
	*pTCNTL = 0;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitPPI()
{
	*pTIMER_DISABLE = TIMDIS1;
	*pTIMER1_CONFIG = PERIOD_CNT|PWM_OUT;
	*pTIMER1_PERIOD = 5;
	*pTIMER1_WIDTH = 2;

	*pPPI_CONTROL = 0;
	*pDMA0_CONFIG = 0;

	*pEVT8 = (void*)PPI_ISR;
	*pIMASK |= EVT_IVG8; 
	*pSIC_IMASK |= 1<<8;

	*pEVT6 = (void*)TIMER_PPI_ISR;
	*pIMASK |= EVT_IVTMR; 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ReadPPI(void *dst, u16 len, u16 clkdiv, u32 delay, volatile bool *ready)
{
	ppi_Ready = ready;

	*ppi_Ready = false;
	*pPPI_CONTROL = 0;
	*pDMA0_CONFIG = 0;

	clkdiv = clkdiv * 5;

	if (clkdiv < 5) { clkdiv = 5; };

	*pTIMER_DISABLE = TIMDIS1;
	*pTIMER1_CONFIG = PERIOD_CNT|PWM_OUT;
	*pTIMER1_PERIOD = clkdiv;
	*pTIMER1_WIDTH = clkdiv>>1;

	*pDMA0_START_ADDR = dst;
	*pDMA0_X_COUNT = len;
	*pDMA0_X_MODIFY = 2;

//	delay = delay * 5 / 4;

	if (delay == 0)
	{ 
		*pTCNTL = 0;
		*pDMA0_CONFIG = FLOW_STOP|DI_EN|WDSIZE_16|SYNC|WNR|DMAEN;
		*pPPI_CONTROL = FLD_SEL|PORT_CFG|POLC|DLEN_12|XFR_TYPE|PORT_EN;
		*pTIMER_ENABLE = TIMEN1;
	}
	else
	{
		*pTSCALE = 4;
		*pTCOUNT = delay;
		*pTCNTL = TINT|TMPWR|TMREN;
	};

	*pPORTFIO_SET = 1<<9; // SYNC 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 twiWriteCount = 0;
static u16 twiReadCount = 0;
static u16 *twiWriteData = 0;
static u16 *twiReadData = 0;

EX_INTERRUPT_HANDLER(TWI_ISR)
{
	if (*pTWI_INT_STAT & RCVSERV)
	{
		*twiReadData++ = *pTWI_RCV_DATA16;
		twiReadCount++;


		*pTWI_INT_STAT = RCVSERV;
	};
	
	if (*pTWI_INT_STAT & XMTSERV)
	{
		*pTWI_XMT_DATA16 = *twiWriteData++;
		twiWriteCount--;


		*pTWI_INT_STAT = XMTSERV;
	};
	
	if (*pTWI_INT_STAT & MERR)
	{
		

		*pTWI_INT_STAT = MERR;
	};

	if (*pTWI_INT_STAT & MCOMP)
	{
		*pTWI_MASTER_CTL = 0;
		*pTWI_MASTER_STAT = 0x3E;
		*pTWI_FIFO_CTL = XMTFLUSH|RCVFLUSH;

		*pTWI_INT_MASK = 0;
		*pTWI_INT_STAT = MCOMP;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitTWI()
{
	*pTWI_CONTROL = TWI_ENA | 10;
	*pTWI_CLKDIV = (8<<8)|12;
	*pTWI_INT_MASK = 0;
	*pTWI_MASTER_ADDR = 0;

//	*pSIC_IAR3 = (*pSIC_IAR3 & ~0xF)|8;
	*pEVT12 = (void*)TWI_ISR;
	*pIMASK |= EVT_IVG12; 
	*pSIC_IMASK |= 1<<24;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void WriteTWI(void *src, u16 len)
{
	*pTWI_MASTER_CTL = 0;
	*pTWI_MASTER_STAT = 0x3E;
	*pTWI_FIFO_CTL = XMTINTLEN;

	twiWriteData = (u16*)src;
	twiWriteCount = len>>1;
	*pTWI_MASTER_ADDR = 11;
	*pTWI_XMT_DATA16 = *twiWriteData++;	twiWriteCount--;
	*pTWI_INT_MASK = XMTSERV|MERR|MCOMP;
	*pTWI_MASTER_CTL = (len<<6)|FAST|MEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ReadTWI(void *dst, u16 len)
{
	*pTWI_MASTER_CTL = 0;
	*pTWI_MASTER_STAT = 0x3E;

	twiReadData = (u16*)dst;
	twiReadCount = 0;
	*pTWI_MASTER_ADDR = 11;
	*pTWI_FIFO_CTL = RCVINTLEN;
	*pTWI_INT_MASK = RCVSERV|MERR|MCOMP;
	*pTWI_MASTER_CTL = (len<<6)|MDIR|FAST|MEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LowLevelInit()
{
	Init_PLL();

								//  5  2 1  8 7  4 3  0								
	*pPORTF_MUX = 0x000F;		//  0000 0000 0000 1111
	*pPORTG_MUX = 0xFF00;		//  1111 1111 0000 0000

	*pPORTF_FER = 0x1C0F;		//  0001 1100 0000 1111
	*pPORTG_FER = 0xFF00;		//  1111 1111 0000 0000

	*pPORTFIO_DIR = 0x0300;		//  0000 0011 0000 0000
	*pPORTGIO_DIR = 0x000F;		//  0000 0000 0000 1111

	*pPORTFIO_INEN = 0x0000;	//  0000 0000 0000 0000
	*pPORTGIO_INEN = 0x0000;	//  0000 0000 0000 0000

	*pPORTGIO = 0;
	*pPORTFIO = 0;

	*pWDOG_CNT = MS2CLK(10);
	*pWDOG_CTL = WDEV_RESET|WDEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetGain(byte v) 
{
	*pPORTGIO = (*pPORTGIO & ~0xF) | bitGain[v&0xF];
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	LowLevelInit();

	InitRTT();

	InitPPI();

	InitTWI();

//	InitSPORT();

	//u32 t = GetRTT();

	//while((GetRTT() - t) < 100000)
	//{
	//	UpdateHardware();
	//};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{

//	spi.Update();

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
