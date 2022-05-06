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
#include "hw_nand.h"

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

__align(16) T_HW::DMADESC DmaTable[32];
__align(16) T_HW::DMADESC DmaWRB[32];

#elif defined(CPU_XMC48)

//#pragma O3
//#pragma Otime

#endif 

#define GEAR_RATIO 12.25

const u16 pulsesPerHeadRoundFix4 = GEAR_RATIO * 6 * 16;

const u16 testNandChipMask = 0xFFFF;

static volatile u32 shaftCounter = 0;
static volatile u32 shaftPrevTime = 0;
static volatile u32 shaftCount = 0;
static volatile u32 shaftTime = 0;
u16 shaftRPS = 0;
volatile u16 curShaftCounter = 0;



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//static void InitVectorTable();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

__forceinline 	void EnableVCORE()	{ PIO_ENVCORE->CLR(ENVCORE); 	}
__forceinline 	void DisableVCORE()	{ PIO_ENVCORE->SET(ENVCORE); 	}
				void EnableDSP()	{ PIO_RESET->CLR(RESET); 		}
				void DisableDSP()	{ PIO_RESET->SET(RESET); 		}

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*----------------------------------------------------------------------------
  Initialize the system
 *----------------------------------------------------------------------------*/
#ifndef WIN32

extern "C" void SystemInit()
{
	//u32 i;
	using namespace CM4;
	using namespace HW;

//	__breakpoint(0);

	SEGGER_RTT_Init();

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "SystemInit ... ");

	#ifdef CPU_SAME53	

		HW::PIOA->DIRSET = (1<<27)|(1<<25)|(1<<24)|(1<<16)|0xFF;
		HW::PIOA->CLR((1<<27)|(1<<25)|(1<<24)|(1<<16));

		PIO_USART0->SetWRCONFIG(UTXD0|URXD0, PORT_PMUX(3)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_PULLEN);
		PIO_USART1->SetWRCONFIG(UTXD1|URXD1, PORT_PMUX(2)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_PULLEN);
		PIO_USART2->SetWRCONFIG(UTXD2|URXD2, PORT_PMUX(2)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_PULLEN);

		HW::PIOB->DIRSET = (1<<15)|(1<<18)|(1<<24)|(1<<21);
		//HW::PIOB->WRCONFIG = ((1<<17)>>16) |PORT_HWSEL_HI|PORT_PMUX(11)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX;

		HW::PIOC->DIRSET = (1<<5)|(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14)|(1<<15)|(1<<17)|(1<<18)|(1<<19)|(1<<21);
		HW::PIOC->SET((1<<15));

		HW::PIOA->BCLR(25);
		HW::PIOA->BSET(25);
		HW::PIOA->BCLR(25);

		OSCCTRL->XOSC[1] = XOSC_ENABLE|XOSC_ONDEMAND; // RUNSTDBY|ENABLE

		HW::PIOA->BSET(25);

		OSCCTRL->DPLL[0].CTRLA = 0; while ((OSCCTRL->DPLL[0].SYNCBUSY & DPLLSYNCBUSY_ENABLE) != 0);

		OSCCTRL->DPLL[0].CTRLB = DPLL_REFCLK_XOSC1|DPLL_DIV(24);	// 0x70010; // XOSC clock source division factor 50 = 2*(DIV+1), XOSC clock reference
		OSCCTRL->DPLL[0].RATIO = DPLL_LDR((MCK*2+500000)/1000000-1)|DPLL_LDRFRAC(0);	// 47; // Loop Divider Ratio = 200, Loop Divider Ratio Fractional Part = 0

		OSCCTRL->DPLL[0].CTRLA = DPLL_ONDEMAND|DPLL_ENABLE; 

		HW::PIOA->BCLR(25);

		HW::GCLK->GENCTRL[GEN_MCK] = GCLK_DIV(0)|GCLK_SRC_DPLL0|GCLK_GENEN;

		HW::PIOA->BSET(25);

		HW::MCLK->AHBMASK |= AHB_DMAC;
		HW::DMAC->CTRL = 0;
		HW::DMAC->CTRL = DMAC_SWRST;
		HW::DMAC->DBGCTRL = DMAC_DBGRUN;
		HW::DMAC->BASEADDR	= DmaTable;
		HW::DMAC->WRBADDR	= DmaWRB;
		HW::DMAC->CTRL = DMAC_DMAENABLE|DMAC_LVLEN0|DMAC_LVLEN1|DMAC_LVLEN2|DMAC_LVLEN3;

		HW::PIOA->BCLR(25);

		if ((CMCC->SR & CMCC_CSTS) == 0)
		{
			CMCC->CTRL = CMCC_CEN;
		};

		HW::PIOA->BSET(25);
		HW::PIOA->BCLR(25);
		HW::PIOA->BSET(25);
		HW::PIOA->BCLR(25);

	#elif defined(CPU_XMC48)

		__disable_irq();

//		__DSB();
		__enable_irq();

		HW::FLASH0->FCON = FLASH_FCON_IDLE_Msk | PMU_FLASH_WS;

		/* enable PLL */
		SCU_PLL->PLLCON0 &= ~(SCU_PLL_PLLCON0_VCOPWD_Msk | SCU_PLL_PLLCON0_PLLPWD_Msk);

		SCU_OSC->OSCHPCTRL = OSC_MODE(2) | OSC_OSCVAL(OSCHP_FREQUENCY / FOSCREF - 1UL);

			/* select OSC_HP clock as PLL input */
			SCU_PLL->PLLCON2 = 0;

			/* restart OSC Watchdog */
			SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_OSCRES_Msk;

			while ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_OSC_USABLE) != SCU_PLL_PLLSTAT_OSC_USABLE);
		//};

		/* Go to bypass the Main PLL */
		SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_VCOBYP_Msk;

		/* disconnect Oscillator from PLL */
		SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_FINDIS_Msk;

		/* Setup divider settings for main PLL */
		SCU_PLL->PLLCON1 =  PLL_CON1(PLL_NDIV, PLL_K2DIV_24MHZ, PLL_PDIV);

		/* Set OSCDISCDIS */
		SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_OSCDISCDIS_Msk;

		/* connect Oscillator to PLL */
		SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_FINDIS_Msk;

		/* restart PLL Lock detection */
		SCU_PLL->PLLCON0 |= SCU_PLL_PLLCON0_RESLD_Msk;	while ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOLOCK_Msk) == 0U);

		/* Disable bypass- put PLL clock back */
		SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_VCOBYP_Msk;	while ((SCU_PLL->PLLSTAT & SCU_PLL_PLLSTAT_VCOBYST_Msk) != 0U);

		/* Before scaling to final frequency we need to setup the clock dividers */
		SCU_CLK->SYSCLKCR = __SYSCLKCR;
		SCU_CLK->PBCLKCR = __PBCLKCR;
		SCU_CLK->CPUCLKCR = __CPUCLKCR;
		SCU_CLK->CCUCLKCR = __CCUCLKCR;
		SCU_CLK->WDTCLKCR = __WDTCLKCR;
		SCU_CLK->EBUCLKCR = __EBUCLKCR;
		SCU_CLK->USBCLKCR = __USBCLKCR;
		SCU_CLK->ECATCLKCR = __ECATCLKCR;
		SCU_CLK->EXTCLKCR = __EXTCLKCR;

		/* PLL frequency stepping...*/
		/* Reset OSCDISCDIS */
		SCU_PLL->PLLCON0 &= ~SCU_PLL_PLLCON0_OSCDISCDIS_Msk;

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV_48MHZ, PLL_PDIV);	delay(DELAY_CNT_50US_48MHZ);

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV_72MHZ, PLL_PDIV);	delay(DELAY_CNT_50US_72MHZ);

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV_96MHZ, PLL_PDIV);	delay(DELAY_CNT_50US_96MHZ);

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV_120MHZ, PLL_PDIV);	delay(DELAY_CNT_50US_120MHZ);

		SCU_PLL->PLLCON1 = PLL_CON1(PLL_NDIV, PLL_K2DIV, PLL_PDIV);			delay(DELAY_CNT_50US_144MHZ);

		/* Enable selected clocks */
		SCU_CLK->CLKSET = __CLKSET;

		SCU_POWER->PWRSET = SCU_POWER_PWRSET_HIB_Msk;	while((SCU_POWER->PWRSTAT & SCU_POWER_PWRSTAT_HIBEN_Msk) == 0);
		SCU_RESET->RSTCLR = SCU_RESET_RSTCLR_HIBRS_Msk;	while((SCU_RESET->RSTSTAT & SCU_RESET_RSTSTAT_HIBRS_Msk) != 0);

		if (SCU_HIBERNATE->OSCULCTRL != OSCULCTRL_MODE_BYPASS)
		{
			while (SCU_GENERAL->MIRRSTS & SCU_GENERAL_MIRRSTS_OSCULCTRL_Msk);	SCU_HIBERNATE->OSCULCTRL = OSCULCTRL_MODE_BYPASS;
			while (SCU_GENERAL->MIRRSTS & SCU_GENERAL_MIRRSTS_HDCR_Msk);		SCU_HIBERNATE->HDCR |= SCU_HIBERNATE_HDCR_ULPWDGEN_Msk;
			while (SCU_GENERAL->MIRRSTS & SCU_GENERAL_MIRRSTS_HDCLR_Msk);		SCU_HIBERNATE->HDCLR = SCU_HIBERNATE_HDCLR_ULPWDG_Msk;
		};

		while (SCU_GENERAL->MIRRSTS & SCU_GENERAL_MIRRSTS_HDCR_Msk);	SCU_HIBERNATE->HDCR |= SCU_HIBERNATE_HDCR_RCS_Msk | SCU_HIBERNATE_HDCR_STDBYSEL_Msk;


		//P2->ModePin10(	G_PP	);
		//P2->BSET(10);

		P0->ModePin0(	G_PP	);
		P0->ModePin1(	G_PP	);
		P0->ModePin2(	HWIO1	);
		P0->ModePin3(	HWIO1	);
		P0->ModePin4(	HWIO1	);
		P0->ModePin5(	HWIO1	);
		P0->ModePin6(	I1DPD	);
		P0->ModePin7(	HWIO1	);
		P0->ModePin8(	HWIO1	);
		P0->ModePin9(	G_PP	);
		P0->ModePin10(	G_PP	);
		P0->ModePin11(	G_PP	);
		P0->ModePin12(	G_PP	);
		P0->ModePin13(	G_PP	);
		P0->ModePin14(	G_PP	);
		P0->ModePin15(	G_PP	);

		P0->PPS = 0;

		P1->ModePin0(	G_PP	);
		P1->ModePin1(	G_PP	);
		P1->ModePin2(	G_PP	);
		P1->ModePin3(	G_PP	);
		P1->ModePin4(	I2DPU	);
		P1->ModePin5(	A2PP	);
		P1->ModePin6(	G_PP	);
		P1->ModePin7(	G_PP	);
		P1->ModePin8(	G_PP	);
		P1->ModePin9(	G_PP	);
		P1->ModePin10(	I2DPU	);
		P1->ModePin11(	I2DPU	);
		P1->ModePin12(	G_PP	);
		P1->ModePin13(	G_PP	);
		P1->ModePin14(	HWIO1	);
		P1->ModePin15(	HWIO1	);

		P1->PPS = 0;

		P2->ModePin0(	HWIO0	);
		P2->ModePin1(	I1DPD	);
		P2->ModePin2(	I2DPU	);
		P2->ModePin3(	I1DPD	);
		P2->ModePin4(	I1DPD	);
		P2->ModePin5(	A1PP	);
		P2->ModePin6(	G_PP	);
		P2->ModePin7(	A1PP	);
		P2->ModePin8(	A1PP	);
		P2->ModePin9(	A1PP	);
		P2->ModePin10(	G_PP	);
		P2->ModePin11(	G_PP	);
		P2->ModePin12(	G_PP	);
		P2->ModePin13(	G_PP	);
		//P2->ModePin14(	A2PP	);
		//P2->ModePin15(	I2DPU	);

		P2->PPS = 0;

		P3->ModePin0(	HWIO1	);
		P3->ModePin1(	HWIO1	);
		P3->ModePin2(	G_PP	);
		P3->ModePin3(	G_PP	);
		P3->ModePin4(	G_PP	);
		P3->ModePin5(	HWIO1	);
		P3->ModePin6(	HWIO1	);
		P3->ModePin7(	G_PP	);
		P3->ModePin8(	G_PP	);
		P3->ModePin9(	G_PP	);
		P3->ModePin10(	G_PP	);
		P3->ModePin11(	G_PP	);
		P3->ModePin12(	G_PP	);
		P3->ModePin13(	G_PP	);
		P3->ModePin14(	I2DPU	);
		P3->ModePin15(	A2PP	);

		P3->PPS = 0;

		P4->ModePin0(	G_PP	);
		P4->ModePin1(	G_PP	);
		P4->ModePin2(	G_PP	);
		P4->ModePin3(	G_PP	);
		P4->ModePin4(	G_PP	);
		P4->ModePin5(	G_PP	);
		P4->ModePin6(	I2DPU	);
		P4->ModePin7(	A1PP	);

		P4->PPS = 0;

		P5->ModePin0(	HWIO0	);
		P5->ModePin1(	G_PP	);
		P5->ModePin2(	A1OD	);
		P5->ModePin3(	G_PP	);
		P5->ModePin4(	G_PP	);
		P5->ModePin5(	G_PP	);
		P5->ModePin6(	G_PP	);
		P5->ModePin7(	G_PP	);
		//P5->ModePin8(	A2PP	);
		//P5->ModePin9(	G_PP	);
		P5->ModePin10(	G_PP	);
		//P5->ModePin11(	G_PP	);

		P5->PPS = 0;


		P6->ModePin0(	G_PP	);
		P6->ModePin1(	G_PP	);
		P6->ModePin2(	G_PP	);
		P6->ModePin3(	I2DPU	);
		P6->ModePin4(	A2PP	);
		P6->ModePin5(	G_PP	);
		P6->ModePin6(	G_PP	);

		P6->PPS = 0;

		P14->ModePin0(	I2DPU	);
		P14->ModePin1(	I2DPU	);
		P14->ModePin2(	I2DPU	);
		P14->ModePin3(	I2DPU	);
		P14->ModePin4(	I2DPU	);
		P14->ModePin5(	I2DPU	);
		P14->ModePin6(	I2DPU	);
		P14->ModePin7(	I2DPU	);
		P14->ModePin8(	I2DPU	);
		P14->ModePin9(	I2DPU	);
		P14->ModePin12(	I0DNP	);
		P14->ModePin13(	I2DPU	);
		P14->ModePin14(	I2DPU	);
		P14->ModePin15(	I2DPU	);

		P14->PPS = 0;
		P14->PDISC = (1<<0);

		P15->ModePin2(	I2DPU	);
		P15->ModePin3(	I2DPU	);
		P15->ModePin4(	I2DPU	);
		P15->ModePin5(	I2DPU	);
		P15->ModePin6(	I2DPU	);
		P15->ModePin7(	I2DPU	);
		P15->ModePin8(	I2DPU	);
		P15->ModePin9(	I1DPD	);
		P15->ModePin12(	I2DPU	);
		P15->ModePin13(	I2DPU	);
		P15->ModePin14(	I2DPU	);
		P15->ModePin15(	I2DPU	);

		P15->PPS = 0;
		P15->PDISC = 0;

		HW::Peripheral_Enable(PID_DMA0);
		HW::Peripheral_Enable(PID_DMA1);

		//HW::DLR->SRSEL0 = SRSEL0(10,11,0,0,0,0,0,0);
		//HW::DLR->SRSEL1 = SRSEL1(0,0,0,0);

		HW::DLR->DRL0 = DRL0_USIC0_SR0;
		HW::DLR->DRL1 = DRL1_USIC1_SR0;
		HW::DLR->DRL2 = DRL2_USIC0_SR1;
		HW::DLR->DRL9 = DRL9_USIC2_SR1;

		HW::DLR->LNEN |= (1<<0)|(1<<2)|(1<<9);

	#endif

	#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
		CM4::SCB->CPACR |= ((3UL << 10*2) |                 /* set CP10 Full Access */
							(3UL << 11*2)  );               /* set CP11 Full Access */
	#else
		CM4::SCB->CPACR = 0;
	#endif

  /* Enable unaligned memory access - SCB_CCR.UNALIGN_TRP = 0 */
	CM4::SCB->CCR &= ~(SCB_CCR_UNALIGN_TRP_Msk);

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n");
}

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

inline void ManDisable()	{ PIO_MANCH->CLR(MAN_TX1|MAN_TX2);								} 
inline void ManZero()		{ PIO_MANCH->CLR(MAN_TX2);			PIO_MANCH->SET(MAN_TX1);	} 
inline void ManOne()		{ PIO_MANCH->CLR(MAN_TX1);			PIO_MANCH->SET(MAN_TX2);	} 
inline void ManDischarge()	{ PIO_MANCH->CLR(MAN_TX1|MAN_TX2);								} 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static const u16 manbaud[5] = { BAUD2CLK(20833), BAUD2CLK(41666), BAUD2CLK(62500), BAUD2CLK(83333), BAUD2CLK(104166) };//0:20833Hz, 1:41666Hz,2:62500Hz,3:83333Hz

static u16 trmHalfPeriod = (manbaud[0]+1)/2;
static u16 trmHalfPeriod2 = manbaud[0];
static u16 trmHalfPeriod3 = (manbaud[0]*3+1)/2;
static u16 trmHalfPeriod4 = manbaud[0] * 2;
static u16 trmHalfPeriod6 = manbaud[0] * 3;
static u16 trmHalfPeriod7 = (manbaud[0] * 7 + 1) / 2;

#endif 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static byte stateManTrans = 0;
static MTB *manTB = 0;
static bool trmBusy = false;
static bool trmTurbo = false;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static u16 GetTrmBaudRate(byte i)
{
	if (i >= ArraySize(manbaud)) { i = ArraySize(manbaud) - 1; };

	return (manbaud[i]+1)/2;
}
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static u16 rcvCount = 0;
static bool rcvBusy = false;
byte stateManRcvr = 0;

const u16 rcvPeriod = BAUD2CLK(20833);

static u16* rcvManPtr = 0;
static u16 rcvManCount = 0;

static u16 rcvManLen = 0;

static MRB *manRB = 0;

//static u16 rcvManLen72 = 0;
//static u16 rcvManLen96 = 0;
//static u16 rcvManLen24 = 0;
//static u16 rcvManLen48 = 0;
//
//static u32 rcvManSum72 = 0;
//static u32 rcvManSum96 = 0;
//static u32 rcvManSum24 = 0;
//static u32 rcvManSum48 = 0;
//
//static u16 rcvManCount72 = 0;
//static u16 rcvManCount96 = 0;
//static u16 rcvManCount24 = 0;
//static u16 rcvManCount48 = 0;

static u16 rcvManLen12 = 0;
static u32 rcvManSum12 = 0;
static u16 rcvManCount12 = 0;
u16 rcvManQuality = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

inline u16 CheckParity(u16 x)
{
	u16 y = x ^ (x >> 1);

	y = y ^ (y >> 2);
	y = y ^ (y >> 4);
	
	return (y ^ (y >> 8))^1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManTrmIRQ()
{
	static u32 tw = 0;
	static u16 count = 0;
	static byte i = 0;
	static const u16 *data = 0;
	static u16 len = 0;
	static bool cmd = false;

	Pin_ManTrmIRQ_Set();

	switch (stateManTrans)
	{
		case 0:	// Idle; 

			ManDisable();
		
			data = manTB->data1;
			len = manTB->len1;
			stateManTrans = 1;

			break;

		case 1: // Start data

			i = 3;
			tw = ((u32)(*data) << 1) | (CheckParity(*data) & 1);

			data++;
			len--;

			ManOne();

			stateManTrans++;

			break;

		case 2:	// Wait data 1-st sync imp

			i--;

			if (i == 0)
			{
				stateManTrans++;
				ManZero();
				i = 3;
			};

			break;

		case 3: // Wait 2-nd sync imp

			i--; 

			if (i == 0)
			{
				stateManTrans++;
				count = 17;

				if (tw & 0x10000) { ManZero(); } else { ManOne(); };
			};

			break;

		case 4: // 1-st half bit wait

//			HW::PIOE->SODR = 1;

			if (tw & 0x10000) { ManOne(); } else { ManZero(); };

			count--;

			if (count == 0)
			{
				if (len > 0)
				{
					stateManTrans = 1;
				}
				else if (manTB->len2 != 0 && manTB->data2 != 0)
				{
					len = manTB->len2;
					data = manTB->data2;

					manTB->len2 = 0;
					manTB->data2 = 0;

					stateManTrans = 1;
				}
				else
				{
					stateManTrans = 6;
				};
			}
			else
			{
				stateManTrans++;
			};

			break;

		case 5: // 2-nd half bit wait

			tw <<= 1;
			stateManTrans = 4;
			if (tw & 0x10000) { ManZero(); } else { ManOne(); };

			break;

		case 6:

			if (tw & 0x10000) { ManZero(); } else { ManOne(); };

			stateManTrans++;

			break;

		case 7:

			ManDisable();
			stateManTrans = 0;

			#ifdef CPU_SAME53	
				ManTT->CTRLA = 0;
				ManTT->INTENCLR = ~0;
			#elif defined(CPU_XMC48)
				ManTT->TCCLR = CC4_TRBC;
			#endif

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)


	#ifdef CPU_SAME53	
		ManTT->INTFLAG = ~0;//TCC_OVF;
	#endif

	Pin_ManTrmIRQ_Clr();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendManData(MTB *mtb)
{
#ifndef WIN32

	if (trmBusy || rcvBusy || mtb == 0 || mtb->data1 == 0 || mtb->len1 == 0)
	{
		return false;
	};

	mtb->ready = false;

	manTB = mtb;

	stateManTrans = 0;

	#ifdef CPU_SAME53	


		ManTT->CTRLA = TC_MODE_COUNT8;
		ManTT->WAVE = TC_WAVEGEN_NPWM;
		ManTT->PER8 = GetTrmBaudRate(mtb->baud) - 1; //trmHalfPeriod-1;

		ManTT->INTENCLR = ~TC_OVF;
		ManTT->INTENSET = TC_OVF;

		ManTT->INTFLAG = ~0;

		ManTT->CTRLA = TC_MODE_COUNT8|TC_ENABLE;
		ManTT->CTRLBSET = TC_CMD_RETRIGGER;

		//ManTT->CTRLA = 0;

		//ManTT->PER = trmHalfPeriod-1;

		//ManTT->INTENCLR = ~TCC_OVF;
		//ManTT->INTENSET = TCC_OVF;

		//ManTT->INTFLAG = ~0;

		//ManTT->CTRLA = TCC_ENABLE;

	#elif defined(CPU_XMC48)

		ManTT->PRS = GetTrmBaudRate(mtb->baud)-1; //trmHalfPeriod - 1;
		ManTT->PSC = 3; //0.08us

		ManCCU->GCSS = CCU4_S0SE;  

		ManTT->SWR = ~0;
		ManTT->INTE = CC4_PME;

		ManTT->TCSET = CC4_TRBS;

	#endif

	return trmBusy = true;

#else

	mtb->ready = true;

	return true;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static void InitManTransmit()
{
	using namespace HW;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_CYAN "Manchester transmit Init ... ");

	VectorTableExt[MANT_IRQ] = ManTrmIRQ;
	CM4::NVIC->CLR_PR(MANT_IRQ);
	CM4::NVIC->SET_ER(MANT_IRQ);

#ifdef CPU_SAME53	

	HW::GCLK->PCHCTRL[GCLK_TC0_TC1] = GCLK_GEN(GEN_1M)|GCLK_CHEN;

	HW::MCLK->APBAMASK |= APBA_TC0;

	PIO_MANCH->DIRSET = MAN_TX1|MAN_TX2;

	ManTT->CTRLA = TCC_SWRST;

	while(ManTT->SYNCBUSY);

	//SetTrmBoudRate(0);

	ManTT->CTRLA = TC_MODE_COUNT8;
	ManTT->WAVE = TC_WAVEGEN_NPWM;
	ManTT->PER8 = GetTrmBaudRate(0) - 1;

	ManTT->INTENCLR = ~TC_OVF;
	ManTT->INTENSET = TC_OVF;

	ManTT->INTFLAG = ~0;

#elif defined(CPU_XMC48)

	HW::CCU_Enable(ManCCU_PID);

	ManCCU->GCTRL = 0;

	ManCCU->GIDLC = CCU4_S0I|CCU4_PRB;

	ManTT->PRS = GetTrmBaudRate(0)-1;
	ManTT->PSC = 3; //0.08us

	ManCCU->GCSS = CCU4_S0SE;  

	ManTT->SRS = 0;

	ManTT->SWR = ~0;
	ManTT->INTE = CC4_PME;

#endif

	ManDisable();

	SEGGER_RTT_WriteString(0, "OK\n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ManTrmIRQ2()
{
	static u32 tw = 0;
	static u16 count = 0;
	static byte i = 0;
	static const u16* data = 0;
	static u16 len = 0;
	static bool cmd = false;

	Pin_ManTrmIRQ_Set();

	switch (stateManTrans)
	{
		case 0:	// 1-st sync imp 

			data = manTB->data1;
			len = manTB->len1;
			cmd = false;
			stateManTrans++;

		case 1:

			if (len == 0)
			{
				data = manTB->data2;
				len = manTB->len2;
				manTB->len2 = 0;
			};

			if (len != 0)
			{
				tw = ((u32)(*data) << 1) | (CheckParity(*data) & 1);

				data++;
				len--;

				count = 17;

				u32 tadd = (cmd) ? trmHalfPeriod : 0;

				ManT_SET_CR(trmHalfPeriod3+tadd);

				if (tw & 0x10000)
				{
					ManT_SET_PR(trmHalfPeriod7+tadd); //US2MT(96);
					stateManTrans += 2;
				}
				else
				{
					ManT_SET_PR(trmHalfPeriod6+tadd); //US2MT(72);
					stateManTrans++;
				};

				//ManT_CCU8->GCSS = ManT_CCU8_GCSS;

				tw <<= 1;
				count--;
			}
			else
			{
				stateManTrans = 4;
			};

			break;

		case 2:	

			ManT_SET_CR(trmHalfPeriod);

			if (count == 0)
			{
				ManT_SET_PR(trmHalfPeriod2);
				cmd = false;
				stateManTrans = 1;
			}
			else
			{
				if (tw & 0x10000)
				{
					ManT_SET_PR(trmHalfPeriod3);

					if (count == 1)
					{
						cmd = true;
						stateManTrans = 1;
					}
					else
					{
						stateManTrans++;
					};
				}
				else
				{
					ManT_SET_PR(trmHalfPeriod2);
				};

				tw <<= 1;
				count--;
			};

//			ManT_CCU8->GCSS = ManT_CCU8_GCSS;

			break;

		case 3: 

			if (tw & 0x10000)
			{
				ManT_SET_CR(trmHalfPeriod);
				ManT_SET_PR(trmHalfPeriod2);

				tw <<= 1;
				count--;

				if (count == 0)
				{
					cmd = true;
					stateManTrans = 1;
				};
			}
			else
			{
				tw <<= 1;
				count--;

				ManT_SET_CR(trmHalfPeriod2);

				if (tw & 0x10000)
				{
					ManT_SET_PR(trmHalfPeriod4);
					
					if (count == 1)
					{
						cmd = true;
						stateManTrans = 1;
					};
				}
				else
				{
					ManT_SET_PR(trmHalfPeriod3);

					if (count == 0)
					{
						cmd = false;
						stateManTrans = 1;
					}
					else
					{
						stateManTrans--;
					}
				};

//				if (count != 0)
				{
					tw <<= 1;
					count--;
				};
			};

//			ManT_CCU8->GCSS = ManT_CCU8_GCSS;

			break;

		case 4:

			stateManTrans++;
			break;

		case 5:

			stateManTrans = 0;

//			HW::SCU_GENERAL->CCUCON &= ~ManT_CCUCON;


			//ManT1->TCCLR = CC8_TRBC;
			//ManT1->INTE = 0;
			//ManT2->TCCLR = CC8_TRBC;
			//ManT3->TCCLR = CC8_TRBC;

			//ManT_CCU8->GCSS = ManT_OUT_GCSS;
			//ManT_CCU8->GCSC = ManT_OUT_GCSC;

			manTB->ready = true;
			trmBusy = false;

			break;


	}; // 	switch (stateManTrans)


	Pin_ManTrmIRQ_Clr();
}

#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SendManData2(MTB* mtb)
{
#ifndef WIN32
	if (trmBusy || rcvBusy || mtb == 0 || mtb->data1 == 0 || mtb->len1 == 0)
	{
		return false;
	};

	mtb->ready = false;

	manTB = mtb;

	trmHalfPeriod = GetTrmBaudRate(mtb->baud);
	trmHalfPeriod2 = trmHalfPeriod * 2 - 1;
	trmHalfPeriod3 = trmHalfPeriod * 3 - 1;
	trmHalfPeriod4 = trmHalfPeriod * 4 - 1;
	trmHalfPeriod6 = trmHalfPeriod * 6 - 1;
	trmHalfPeriod7 = trmHalfPeriod * 7 - 1;

	stateManTrans = 0;

#ifdef CPU_SAME53	


	ManTT->CTRLA = TC_MODE_COUNT8;
	ManTT->WAVE = TC_WAVEGEN_NPWM;
	ManTT->PER8 = GetTrmBaudRate(mtb->baud) - 1; //trmHalfPeriod-1;

	ManTT->INTENCLR = ~TC_OVF;
	ManTT->INTENSET = TC_OVF;

	ManTT->INTFLAG = ~0;

	ManTT->CTRLA = TC_MODE_COUNT8 | TC_ENABLE;
	ManTT->CTRLBSET = TC_CMD_RETRIGGER;

	//ManTT->CTRLA = 0;

	//ManTT->PER = trmHalfPeriod-1;

	//ManTT->INTENCLR = ~TCC_OVF;
	//ManTT->INTENSET = TCC_OVF;

	//ManTT->INTFLAG = ~0;

	//ManTT->CTRLA = TCC_ENABLE;

#elif defined(CPU_XMC48)

	ManT_SET_PR(US2MT(50)-1); //trmHalfPeriod - 1;
	ManT1->CR2S = (~0); ManT2->CR1S = (0); ManT2->CR2S = (0); ManT3->CR1S = (~0);

	ManT1->PSC = ManT_PSC; //0.08us
	ManT2->PSC = ManT_PSC; //0.08us
	ManT3->PSC = ManT_PSC; //0.08us

	//ManT1->PSL = ManT1_PSL;
	//ManT2->PSL = ManT2_PSL;
	//ManT3->PSL = ManT3_PSL;

	ManT1->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;
	ManT2->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;
	ManT3->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;

	ManT1->CMC = CC4_STRTS_EVENT0;
	ManT2->CMC = CC4_STRTS_EVENT0;
	ManT3->CMC = CC4_STRTS_EVENT0;
	
	ManT1->TC = CC8_STRM;
	ManT2->TC = CC8_STRM;
	ManT3->TC = CC8_STRM;

	ManT1->SWR = ~0;
	ManT2->SWR = ~0;
	ManT3->SWR = ~0;

	ManT1->CHC = ManT1_CHC;
	ManT2->CHC = ManT2_CHC;
	ManT3->CHC = ManT3_CHC;

	ManT1->INTE = CC8_PME;
	ManT2->INTE = 0;
	ManT3->INTE = 0;

	ManT_CCU8->GCSS = ManT_CCU8_GCSS;
	ManT_CCU8->GIDLC = ManT_CCU8_GIDLC;

	HW::SCU_GENERAL->CCUCON |= ManT_CCUCON;

#endif

	return trmBusy = true;
#else

	mtb->ready = true;

	return true;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static void InitManTransmit2()
{
	using namespace HW;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "Manchester transmit2 Init ... ");

	VectorTableExt[MANT_IRQ] = ManTrmIRQ2;
	CM4::NVIC->CLR_PR(MANT_IRQ);
	CM4::NVIC->SET_ER(MANT_IRQ);

#ifdef CPU_SAME53	

	HW::GCLK->PCHCTRL[GCLK_TC0_TC1] = GCLK_GEN(GEN_1M)|GCLK_CHEN;

	HW::MCLK->APBAMASK |= APBA_TC0;

	PIO_MANCH->DIRSET = MAN_TX1|MAN_TX2;

	ManTT->CTRLA = TCC_SWRST;

	while(ManTT->SYNCBUSY);

	//SetTrmBoudRate(0);

	ManTT->CTRLA = TC_MODE_COUNT8;
	ManTT->WAVE = TC_WAVEGEN_NPWM;
	ManTT->PER8 = GetTrmBaudRate(0) - 1;

	ManTT->INTENCLR = ~TC_OVF;
	ManTT->INTENSET = TC_OVF;

	ManTT->INTFLAG = ~0;

#elif defined(CPU_XMC48)

	HW::CCU_Enable(ManT_CCU8_PID);

	ManT_CCU8->GCTRL = 0;
	ManT_CCU8->GIDLC = ManT_CCU8_GIDLC;
	ManT_CCU8->GCSS = ManT_OUT_GCSS;
	ManT_CCU8->GCSC = ManT_OUT_GCSC;

	PIO_MANCH->ModePin(PIN_MAN_TX1, A3PP);
	PIO_MANCH->ModePin(PIN_MAN_TX2, A3PP);

	//ManT_CCU8->GIDLS = ManT_CCU8_GIDLS;

	//ManT1->PRS = US2MT(100) - 1;
	//ManT1->CR1S = US2MT(50) - 1;
	//ManT1->CR2S = US2MT(50) - 1;

	//ManT1->PSC = ManT_PSC; 
	//ManT2->PSC = ManT_PSC;
	//ManT3->PSC = ManT_PSC; 

	////ManT->CHC = CC8_;

	////ManT->TCSET = CC8_TRBS;

	////ManT1->PSL = ManT1_PSL;
	////ManT2->PSL = ManT2_PSL;
	////ManT3->PSL = ManT3_PSL;

	//ManT1->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;
	//ManT2->INS = CC8_EV0IS(7) | CC4_EV0EM_RISING_EDGE;

	//ManT1->CMC = CC4_STRTS_EVENT0;
	//ManT2->CMC = CC4_STRTS_EVENT0;
	//ManT3->CMC = CC4_STRTS_EVENT0;

	ManT1->CHC = ManT1_CHC;
	ManT2->CHC = ManT2_CHC;
	ManT3->CHC = ManT3_CHC;

	//ManT_CCU8->GCSS = ManT_CCU8_GCSS;

	//ManT1->SRS = 0;

	//ManT1->SWR = ~0;
	//ManT_CCU8->GIDLC = ManT_CCU8_GIDLC;
	//ManT->INTE = CC8_PME;

#endif

	SEGGER_RTT_WriteString(0, "OK\n");
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ManRcvEnd(bool ok)
{
#ifdef CPU_SAME53	
	ManRT->INTENCLR = ~0;
#elif defined(CPU_XMC48)
	ManTmr->INTE = 0;
#endif

	manRB->OK = ok;
	manRB->ready = true;
	manRB->len = rcvManLen;

	rcvManLen12 = (rcvManCount12 != 0) ? (rcvManSum12 / rcvManCount12) : 0;

	rcvManQuality = (rcvManLen12 > MT(12)) ? 0 : (((MT(12) - rcvManLen12) * 100 + MT(6))/MT(12));

	rcvBusy = false;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static RTM manRcvTime;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static __irq void ManRcvIRQ2()
{
	using namespace HW;

	static u32 _number = 0;
	static u32 _length = 0;
	static u32 _data = 0;
	static bool _command = false;
	static bool _parity_temp = false;
	const bool _parity = true;
	static bool _sync = false;
	static bool _state = false;

	Pin_ManRcvIRQ_Set();

	#ifdef CPU_SAME53	

		u32 len = ManRT->CC[0];

		ManRT->CTRLBSET = TCC_CMD_RETRIGGER;

	#elif defined(CPU_XMC48)

		u16 len = ManTmr->CV[1];

		//ManTmr->TCCLR = CC4_TCC;
		//ManTmr->TCSET = CC4_TRB;

	#endif

	_state = !_state;

	if (len <= MT(60))
	{
		i16 dl;

		if (len <= MT(36))
		{
			_length += 1; dl = len - MT(24); 
		}
		else
		{
			_length += 2; dl = len - MT(48);
		};

		if (dl < 0) dl = -dl; rcvManSum12 += dl; rcvManCount12++;

		if(_length >= 3)
		{
			_sync = false;
		};
	}
	else
	{
		if(len > MT(108))
		{
			_sync = false;
		}
		else
		{
			manRcvTime.Reset();

			_sync = true;
			_data = 0;
			_parity_temp = _parity;
			_number = 0;
			_command = !_state; 

			i16 dl;

			if (len <= MT(84))
			{
				_length = 1; dl = len - MT(72); 
			}
			else
			{
				_length = 2; dl = len - MT(96); 
			};

			if (dl < 0) dl = -dl; rcvManSum12 += dl; rcvManCount12++;
		};
	};

	if(_sync && _length == 2)
	{
		manRcvTime.Reset();

		if(_number < 16)
		{
			_data <<= 1;
			_data |= _state;
			_parity_temp ^= _state;
			_number++;
			_length = 0;
		}
	 	else
		{
			Pin_ManRcvSync_Set();

			_sync = false;

			if(_state != _parity_temp)
			{
				_state = !_state;
				_data = (~_data);
				_command = !_command;
			};

			if (rcvManLen == 0)
			{
				if(_command)
				{
					*rcvManPtr++ = _data;
					rcvManLen = 1;
				};
			}
			else 
			{
				if(rcvManLen < rcvManCount)
				{
					*rcvManPtr++ = _data;
				};

				rcvManLen += 1;	
			};

			Pin_ManRcvSync_Clr();
		};
	};

	#ifdef CPU_SAME53	
		ManRT->INTFLAG = ~0;
	#elif defined(CPU_XMC48)
	#endif

	Pin_ManRcvIRQ_Clr();
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ManRcvUpdate()
{
	if (rcvBusy)
	{
		u32 irq = Push_IRQ();

		if (rcvManLen > 0 && manRcvTime.Timeout(US2RT(200)))
		{
			ManRcvEnd(true);
		}
		else
		{
			manRB->len = rcvManLen;
		};

		Pop_IRQ(irq);
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static void InitManRecieve()
{
	using namespace HW;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "Manchester transmit Init ... ");

	VectorTableExt[MANR_IRQ] = ManRcvIRQ2;
	CM4::NVIC->CLR_PR(MANR_IRQ);
	CM4::NVIC->SET_ER(MANR_IRQ);	

#ifdef CPU_SAME53	

	HW::GCLK->PCHCTRL[GCLK_TCC2_TCC3]	= GCLK_GEN(GEN_1M)|GCLK_CHEN;
	HW::GCLK->PCHCTRL[GCLK_TC0_TC1]		= GCLK_GEN(GEN_1M)|GCLK_CHEN;

	HW::MCLK->APBCMASK |= APBC_TCC2;
	HW::MCLK->APBAMASK |= APBA_TC1;

	HW::GCLK->PCHCTRL[EVENT_MANR_1+GCLK_EVSYS0] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;
	HW::GCLK->PCHCTRL[EVENT_MANR_2+GCLK_EVSYS0] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;

	EIC->CTRLA = 0;
	while(EIC->SYNCBUSY);

	EIC->EVCTRL |= EIC_EXTINT0<<MANR_EXTINT;
	EIC->SetConfig(MANR_EXTINT, 1, EIC_SENSE_BOTH);
	EIC->INTENCLR = EIC_EXTINT0<<MANR_EXTINT;
	EIC->CTRLA = EIC_ENABLE;

	EVSYS->CH[EVENT_MANR_1].CHANNEL = (EVGEN_EIC_EXTINT_0+MANR_EXTINT)|EVSYS_PATH_ASYNCHRONOUS;
	EVSYS->USER[EVSYS_USER_TC1_EVU] = EVENT_MANR_1+1;

	EVSYS->CH[EVENT_MANR_2].CHANNEL = EVGEN_TC1_OVF|EVSYS_PATH_ASYNCHRONOUS|EVSYS_EDGSEL_RISING_EDGE;;
	EVSYS->USER[EVSYS_USER_TCC2_MC_0] = EVENT_MANR_2+1;

	PIO_MAN_RX->DIRCLR = MAN_RX;
	PIO_MAN_RX->CTRL |= MAN_RX;

	PIO_RXD->DIRCLR = RXD;
	PIO_RXD->CTRL |= RXD;

	PIO_MAN_RX->SetWRCONFIG(	MAN_RX,		PORT_PMUX(0)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_INEN);
	PIO_RXD->SetWRCONFIG(		RXD,		PORT_PMUX(0)|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_INEN);

	//PIO_MANCHRX->PINCFG[PIN_MANCHRX] = PINGFG_INEN|PINGFG_PMUXEN;
	//PIO_RXD->PINCFG[PIN_RXD] = PINGFG_INEN|PINGFG_PMUXEN;

	ManRT->CTRLA = TCC_SWRST;
	while(ManRT->SYNCBUSY);

	ManRT->CTRLA = TCC_CPTEN0;
	ManRT->EVCTRL = TCC_MCEI0;

	ManRT->PER = ~0;
	ManRT->CC[1] = 250;

	ManRT->INTENCLR = ~0;
	//ManRT->INTENSET = TCC_MC0;

	ManRT->CTRLA = TCC_CPTEN0|TCC_ENABLE;


	ManIT->CTRLA = TCC_SWRST;

	while(ManIT->SYNCBUSY);

	ManIT->CTRLA = TC_MODE_COUNT8;
	ManIT->WAVE = TC_WAVEGEN_NPWM;

	ManIT->EVCTRL = TC_TCEI|TC_EVACT_RETRIGGER|TC_OVFEO;

	ManIT->PER8 = 11;
	ManIT->CC8[0] = ~0;
	ManIT->CC8[1] = ~0;

	ManIT->INTENCLR = ~0;

	ManIT->INTFLAG = ~0;

	ManIT->CTRLA = TC_MODE_COUNT8|TC_ENABLE;

	ManIT->CTRLBSET = TC_ONESHOT;

#elif defined(CPU_XMC48)

	HW::CCU_Enable(ManCCU_PID);

	P1->ModePin10(I2DPU);
	P1->ModePin11(I2DPU);

	ManCCU->GCTRL = 0;

	ManCCU->GIDLC = ManCCU_GIDLC;//CCU4_CS1I|CCU4_CS2I|CCU4_SPRB;

	ManRT->PRS = MT(12) - 1;
	ManRT->CRS = MT(12) - 1;
	ManRT->PSC = ManRT_PSC; //1.28us

	ManTmr->PRS = MT(250);
	ManTmr->PSC = ManRT_PSC; //1.28us

	ManCCU->GCSS = ManCCU_GCSS;//CCU4_S1SE|CCU4_S2SE;  

	ManRT->INS = ManRT_INS;//CC4_EV0IS(2)|CC4_EV0EM_BOTH_EDGES|CC4_LPF0M_7CLK;
	ManRT->CMC = CC4_STRTS_EVENT0;
	ManRT->TC = CC4_STRM|CC4_TSSM;

	ManRT->INTE = 0;//CC4_PME;
	ManRT->SRS = 0;//ManRT_SRS;//CC4_POSR(2);

	ManTmr->INS = CC4_EV0IS(15) | CC4_EV0EM_RISING_EDGE;
	ManTmr->CMC = CC4_CAP0S_EVENT0|CC4_STRTS_EVENT0;
	ManTmr->SRS = CC4_E0SR(2);
	ManTmr->TC = CC4_TSSM|CC4_CAPC_ALWAYS;
	ManTmr->TCSET = CC4_TRB;

	ManTmr->INTE = 0;//CC4_PME;

#endif

	SEGGER_RTT_WriteString(0, "OK\n");
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool RcvManData(MRB *mrb)
{
#ifndef WIN32

	if (rcvBusy /*|| trmBusy*/ || mrb == 0 || mrb->data == 0 || mrb->maxLen == 0)
	{
		return false;
	};

	//ManDisable();

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	manRB = mrb;
	
	rcvManLen = 0;

	rcvManPtr = manRB->data;
	rcvManCount = manRB->maxLen;

	rcvManSum12 = 0; rcvManCount12 = 0;

	#ifdef CPU_SAME53	

		ManRT->INTFLAG = ~0;
		ManRT->INTENSET = TCC_MC0;

	#elif defined(CPU_XMC48)

		ManTmr->SWR = CC4_RE0A;
		ManTmr->INTE = CC4_E0AE;

	#endif

	return rcvBusy = true;

#else

	mrb->ready = mrb->OK = false;
	mrb->len = 0;

	return true;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetClock(const RTC &t)
{
	static DSCI2C dsc;

	static byte reg = 0;
	static u16 rbuf = 0;
	static byte buf[10];

	buf[0] = 0;
	buf[1] = ((t.sec/10) << 4)|(t.sec%10);
	buf[2] = ((t.min/10) << 4)|(t.min%10);
	buf[3] = ((t.hour/10) << 4)|(t.hour%10);
	buf[4] = 1;
	buf[5] = ((t.day/10) << 4)|(t.day%10);
	buf[6] = ((t.mon/10) << 4)|(t.mon%10);

	byte y = t.year % 100;

	buf[7] = ((y/10) << 4)|(y%10);

	dsc.adr = 0x68;
	dsc.wdata = buf;
	dsc.wlen = 8;
	dsc.rdata = 0;
	dsc.rlen = 0;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	if (SetTime(t))
	{
		I2C_AddRequest(&dsc);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef WIN32

static __irq void Clock_IRQ()
{

#ifdef CPU_SAME53	

	timeBDC.msec = (timeBDC.msec < 500) ? 0 : 999;

	HW::EIC->INTFLAG = 1 << CLOCK_EXTINT;

#elif defined(CPU_XMC48)

	if (HW::SCU_HIBERNATE->HDSTAT & SCU_HIBERNATE_HDSTAT_ULPWDG_Msk)
	{
		if ((HW::SCU_GENERAL->MIRRSTS & SCU_GENERAL_MIRRSTS_HDCLR_Msk) == 0)	HW::SCU_HIBERNATE->HDCLR = SCU_HIBERNATE_HDCLR_ULPWDG_Msk;
	}
	else
	{
		timeBDC.msec = (timeBDC.msec < 500) ? 0 : 999;
	};

	HW::SCU_GCU->SRCLR = SCU_INTERRUPT_SRCLR_PI_Msk;

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitClock()
{
	DSCI2C dsc;

	byte reg = 0;
	byte buf[10];
	
	RTC t;

	TM32 tm;

	dsc.adr = 0x68;
	dsc.wdata = &reg;
	dsc.wlen = 1;
	dsc.rdata = buf;
	dsc.rlen = 7;
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "Sync with DS3232 ... ");

	tm.Reset();

	if (I2C_AddRequest(&dsc))
	{
		while (!(dsc.ready || tm.Check(100))) { I2C_Update(); };
	};

	if (dsc.ready && dsc.ack && dsc.readedLen >= dsc.rlen)
	{
		t.sec	= (buf[0]&0xF) + ((buf[0]>>4)*10);
		t.min	= (buf[1]&0xF) + ((buf[1]>>4)*10);
		t.hour	= (buf[2]&0xF) + ((buf[2]>>4)*10);
		t.day	= (buf[4]&0xF) + ((buf[4]>>4)*10);
		t.mon	= (buf[5]&0xF) + ((buf[5]>>4)*10);
		t.year	= (buf[6]&0xF) + ((buf[6]>>4)*10) + 2000;

		SetTime(t);
	
		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n");
	}
	else
	{
		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED "!!! ERROR !!!\n");
	};

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_WHITE "Clock Init ... ");

	VectorTableExt[CLOCK_IRQ] = Clock_IRQ;
	CM4::NVIC->CLR_PR(CLOCK_IRQ);
	CM4::NVIC->SET_ER(CLOCK_IRQ);	

#ifdef CPU_SAME53	

	using namespace HW;

	PIO_RTC->SetWRCONFIG(RTCINT, PORT_PMUX_A|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_INEN);

	if ((EIC->CTRLA & EIC_ENABLE) == 0)
	{
		EIC->CTRLA = EIC_SWRST;
		while(EIC->SYNCBUSY);
	}
	else
	{
		EIC->CTRLA = 0;
		while(EIC->SYNCBUSY);
	};

	EIC->EVCTRL &= ~(1 << CLOCK_EXTINT);
	EIC->SetConfig(CLOCK_EXTINT, 1, EIC_SENSE_FALL);
	EIC->ASYNCH |= 1<<CLOCK_EXTINT;
	EIC->INTENSET = 1 << CLOCK_EXTINT;
	EIC->CTRLA = EIC_ENABLE;

#elif defined(CPU_XMC48)

	HW::RTC->CTR = (0x7FFFUL << RTC_CTR_DIV_Pos) | RTC_CTR_ENB_Msk;

	while (HW::SCU_GCU->MIRRSTS & SCU_GENERAL_MIRRSTS_RTC_MSKSR_Msk);

	HW::RTC->MSKSR = RTC_MSKSR_MPSE_Msk;
	HW::SCU_GCU->SRMSK = SCU_INTERRUPT_SRMSK_PI_Msk;

#endif

	SEGGER_RTT_WriteString(0, "OK\n");
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_XMC48

u16 CRC_CCITT_PIO(const void *data, u32 len, u16 init)
{
	CRC_FCE->CRC = init;	//	DataCRC CRC = { init };

	__packed const byte *s = (__packed const byte*)data;

	for ( ; len > 0; len--)
	{
		CRC_FCE->IR = *(s++);
	};

	//if (len > 0)
	//{
	//	CRC_FCE->IR = *(s++)&0xFF;
	//}

	__dsb(15);

	return CRC_FCE->RES;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init)
{
	HW::P6->BSET(5);

	byte *s = (byte*)data;

	CRC_FCE->CRC = init;	//	DataCRC CRC = { init };

	CRC_DMACH->SAR = (u32)s;

	while (len > 0)
	{
		u32 l = (len > BLOCK_TS(~0)) ? BLOCK_TS(~0) : len;

		CRC_DMACH->CTLH = l;

		CRC_DMA->CHENREG = CRC_DMA_CHEN;

		while(CRC_DMA->CHENREG & (1<<2));

		//s += l;
		len -= l;
	};

	HW::P6->BCLR(5);

	__dsb(15);

	return (byte)CRC_FCE->RES;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init)
{
	HW::P6->BSET(5);

	byte* s = (byte*)data;

	CRC_FCE->CRC = init;	//	DataCRC CRC = { init };

//	if ((u32)s & 1) { CRC_FCE->IR = *s++; len--; };

	if (len > 0)
	{
		CRC_DMACH->CTLH = BLOCK_TS(len);

		CRC_DMACH->SAR = (u32)s;

		CRC_DMA->CHENREG = CRC_DMA_CHEN;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_CheckComplete(u16* crc)
{
	if ((CRC_DMA->CHENREG & (1 << 2)) == 0)
	{
		*crc = (byte)CRC_FCE->RES;

		HW::P6->BCLR(5);

		return true;
	}
	else
	{
		return false;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_CRC_CCITT_DMA()
{
	HW::Peripheral_Enable(PID_FCE);

	HW::FCE->CLC = 0;
	CRC_FCE->CFG = 0;

	CRC_DMA->DMACFGREG = 1;

	CRC_DMACH->CTLL = DST_NOCHANGE|SRC_INC|DST_TR_WIDTH_8|SRC_TR_WIDTH_8|TT_FC_M2M_GPDMA|DEST_MSIZE_1|SRC_MSIZE_1;
	CRC_DMACH->DAR = (u32)&CRC_FCE->IR;
	CRC_DMACH->CFGL = 0;
	CRC_DMACH->CFGH = PROTCTL(1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#elif defined(CPU_SAME53)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init)
{
	//HW::PIOC->BSET(26);

	T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	T_HW::S_DMAC::S_DMAC_CH	&dmach = HW::DMAC->CH[CRC_DMACH];

	dmadsc.DESCADDR = 0;
	dmadsc.DSTADDR = (void*)init;
	dmadsc.SRCADDR = (byte*)data+len;
	dmadsc.BTCNT = len;
	dmadsc.BTCTRL = DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_SRCINC;

	HW::DMAC->CRCCTRL = DMAC_CRCBEATSIZE_BYTE|DMAC_CRCPOLY_CRC16|DMAC_CRCMODE_CRCGEN|DMAC_CRCSRC(0x20+CRC_DMACH);

	dmach.INTENCLR = ~0;
	dmach.INTFLAG = ~0;
	dmach.CTRLA = DMCH_ENABLE/*|DMCH_TRIGACT_TRANSACTION*/;

	HW::DMAC->SWTRIGCTRL = 1UL << CRC_DMACH;

	while (((dmach.CTRLA & DMCH_ENABLE) != 0) && (dmach.INTFLAG & DMCH_TCMPL) == 0);

	//HW::PIOC->BCLR(26);

	return ReverseWord(HW::DMAC->CRCCHKSUM);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init)
{
//	HW::PIOC->BSET(26);

	T_HW::DMADESC& dmadsc = DmaTable[CRC_DMACH];
	T_HW::S_DMAC::S_DMAC_CH& dmach = HW::DMAC->CH[CRC_DMACH];

	dmadsc.DESCADDR = 0;
	dmadsc.DSTADDR = (void*)init;
	dmadsc.SRCADDR = (byte*)data + len;
	dmadsc.BTCNT = len;
	dmadsc.BTCTRL = DMDSC_VALID | DMDSC_BEATSIZE_BYTE | DMDSC_SRCINC;

	HW::DMAC->CRCCTRL = DMAC_CRCBEATSIZE_BYTE | DMAC_CRCPOLY_CRC16 | DMAC_CRCMODE_CRCGEN | DMAC_CRCSRC(0x20 + CRC_DMACH);

	dmach.INTENCLR = ~0;
	dmach.INTFLAG = ~0;
	dmach.CTRLA = DMCH_ENABLE/*|DMCH_TRIGACT_TRANSACTION*/;

	HW::DMAC->SWTRIGCTRL = 1UL << CRC_DMACH;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_CheckComplete(u16* crc)
{
	//T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	T_HW::S_DMAC::S_DMAC_CH& dmach = HW::DMAC->CH[CRC_DMACH];

	if ((dmach.CTRLA & DMCH_ENABLE) == 0 || (dmach.INTFLAG & DMCH_TCMPL))
	{
		*crc = ReverseWord(HW::DMAC->CRCCHKSUM);

		//HW::PIOC->BCLR(26);

		return true;
	}
	else
	{
		return false;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_CRC_CCITT_DMA()
{
	//T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	//T_HW::S_DMAC::S_DMAC_CH	&dmach = HW::DMAC->CH[CRC_DMACH];

	//HW::DMAC->CRCCTRL = DMAC_CRCBEATSIZE_BYTE|DMAC_CRCPOLY_CRC16|DMAC_CRCMODE_CRCGEN|DMAC_CRCSRC(0x3F);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#elif defined(WIN32)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 CRC_CCITT_DMA(const void *data, u32 len, u16 init)
{
	return 0;//GetCRC16_CCIT(data, len, init);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void CRC_CCITT_DMA_Async(const void* data, u32 len, u16 init)
{
	crc_ccit_result = 0;//GetCRC16_CCIT(data, len, init);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CRC_CCITT_DMA_CheckComplete(u16* crc)
{
	if (crc != 0)
	{
		*crc = crc_ccit_result;

		return true;
	}
	else
	{
		return false;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_CRC_CCITT_DMA()
{
	//T_HW::DMADESC &dmadsc = DmaTable[CRC_DMACH];
	//T_HW::S_DMAC::S_DMAC_CH	&dmach = HW::DMAC->CH[CRC_DMACH];

	//HW::DMAC->CRCCTRL = DMAC_CRCBEATSIZE_BYTE|DMAC_CRCPOLY_CRC16|DMAC_CRCMODE_CRCGEN|DMAC_CRCSRC(0x3F);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WDT_Init()
{
	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_WHITE "Manchester transmit Init ... ");

	#ifdef CPU_SAME53	

		//HW::MCLK->APBAMASK |= APBA_WDT;

		//HW::WDT->CONFIG = WDT_WINDOW_CYC512|WDT_PER_CYC1024;
	
		#ifndef _DEBUG
//		HW::WDT->CTRLA = WDT_ENABLE|WDT_WEN|WDT_ALWAYSON;
		#else
//		HW::WDT->CTRLA = WDT_ENABLE|WDT_WEN|WDT_ALWAYSON;
		#endif

		//while(HW::WDT->SYNCBUSY);

	#elif defined(CPU_XMC48)

		#ifndef _DEBUG
	
		//HW::WDT_Enable();

		//HW::WDT->WLB = OFI_FREQUENCY/2;
		//HW::WDT->WUB = (3 * OFI_FREQUENCY)/2;
		//HW::SCU_CLK->WDTCLKCR = 0|SCU_CLK_WDTCLKCR_WDTSEL_OFI;

		//HW::WDT->CTR = WDT_CTR_ENB_Msk|WDT_CTR_DSP_Msk;

		#else

		HW::WDT_Disable();

		#endif

	#endif

	SEGGER_RTT_WriteString(0, "OK\n");
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void DSP_CopyDataDMA(volatile void *src, volatile void *dst, u16 len)
{
#ifndef WIN32

	using namespace HW;

	#ifdef CPU_SAME53	

		DmaTable[DSP_DMACH].SRCADDR = (byte*)src+len;
		DmaTable[DSP_DMACH].DSTADDR = (byte*)dst+len;
		DmaTable[DSP_DMACH].DESCADDR = 0;
		DmaTable[DSP_DMACH].BTCNT = len;
		DmaTable[DSP_DMACH].BTCTRL = DMDSC_VALID|DMDSC_BEATSIZE_BYTE|DMDSC_DSTINC|DMDSC_SRCINC;

		DMAC->CH[DSP_DMACH].INTENCLR = ~0;
		DMAC->CH[DSP_DMACH].INTFLAG = ~0;
		DMAC->CH[DSP_DMACH].CTRLA = DMCH_ENABLE|DMCH_TRIGACT_TRANSACTION;
		DMAC->SWTRIGCTRL = 1UL<<DSP_DMACH;

	#elif defined(CPU_XMC48)

//		register u32 t __asm("r0");

		if (len > BLOCK_TS(~0)) { len = BLOCK_TS(~0); };

		DSP_DMA->DMACFGREG = 1;

		DSP_DMACH->CTLL = DST_INC|SRC_INC|TT_FC(0)|DEST_MSIZE(0)|SRC_MSIZE(0);
		DSP_DMACH->CTLH = BLOCK_TS(len);

		DSP_DMACH->SAR = (u32)src;
		DSP_DMACH->DAR = (u32)dst;
		DSP_DMACH->CFGL = 0;
		DSP_DMACH->CFGH = PROTCTL(1);

		DSP_DMA->CHENREG = DSP_DMA_CHEN;

	#endif
#else

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool DSP_CheckDataComplete()
{
	#ifdef CPU_SAME53

	return (HW::DMAC->CH[DSP_DMACH].CTRLA & DMCH_ENABLE) == 0 || (HW::DMAC->CH[DSP_DMACH].INTFLAG & DMCH_TCMPL);
	
	#elif defined(CPU_XMC48)

		return (DSP_DMA->CHENREG & DSP_DMA_CHST) == 0;

	#elif defined(WIN32)

		return true;
		
	#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WIN32

//extern dword DI;
//extern dword DO;
char pressedKey;
//extern  dword maskLED[16];

//byte _array1024[0x60000]; 

HWND  hMainWnd;

HCURSOR arrowCursor;
HCURSOR handCursor;

HBRUSH	redBrush;
HBRUSH	yelBrush;
HBRUSH	grnBrush;
HBRUSH	gryBrush;

RECT rectLed[10] = { {20, 41, 33, 54}, {20, 66, 33, 79}, {20, 91, 33, 104}, {21, 117, 22, 118}, {20, 141, 33, 154}, {218, 145, 219, 146}, {217, 116, 230, 129}, {217, 91, 230, 104}, {217, 66, 230, 79}, {217, 41, 230, 54}  }; 
HBRUSH* brushLed[10] = { &yelBrush, &yelBrush, &yelBrush, &gryBrush, &grnBrush, &gryBrush, &redBrush, &redBrush, &redBrush, &redBrush };

//int x,y,Depth;

HFONT font1;
HFONT font2;

HDC memdc;
HBITMAP membm;

//HANDLE facebitmap;

static const u32 secBufferWidth = 80;
static const u32 secBufferHeight = 12;
static const u32 fontWidth = 12;
static const u32 fontHeight = 16;


static char secBuffer[secBufferWidth*secBufferHeight*2];

static u32 pallete[16] = {	0x000000,	0x800000,	0x008000,	0x808000,	0x000080,	0x800080,	0x008080,	0xC0C0C0,
							0x808080,	0xFF0000,	0x00FF00,	0xFFFF00,	0x0000FF,	0xFF00FF,	0x00FFFF,	0xFFFFFF };

const char lpAPPNAME[] = "¿—76÷_”œ–";

int screenWidth = 0, screenHeight = 0;

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static __int64		tickCounter = 0;

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

static void InitDisplay()
{
	WNDCLASS		    wcl;

	wcl.hInstance		= NULL;
	wcl.lpszClassName	= lpAPPNAME;
	wcl.lpfnWndProc		= WindowProc;
	wcl.style	    	= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	wcl.hIcon	    	= NULL;
	wcl.hCursor	    	= NULL;
	wcl.lpszMenuName	= NULL;

	wcl.cbClsExtra		= 0;
	wcl.cbWndExtra		= 0;
	wcl.hbrBackground	= NULL;

	RegisterClass (&wcl);

	int sx = screenWidth = GetSystemMetrics (SM_CXSCREEN);
	int sy = screenHeight = GetSystemMetrics (SM_CYSCREEN);

	hMainWnd = CreateWindowEx (0, lpAPPNAME, lpAPPNAME,	WS_DLGFRAME|WS_POPUP, 0, 0,	640, 480, NULL,	NULL, NULL, NULL);

	if(!hMainWnd) 
	{
		cputs("Error creating window\r\n");
		exit(0);
	};

	RECT rect;

	if (GetClientRect(hMainWnd, &rect))
	{
		MoveWindow(hMainWnd, 0, 0, 641 + 768 - rect.right - 2, 481 - rect.bottom - 1 + 287, true);
	};

	ShowWindow (hMainWnd, SW_SHOWNORMAL);

	font1 = CreateFont(30, 14, 0, 0, 100, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_DONTCARE, "Lucida Console");
	font2 = CreateFont(fontHeight, fontWidth-2, 0, 0, 100, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_DONTCARE, "Lucida Console");

	if (font1 == 0 || font2 == 0)
	{
		cputs("Error creating font\r\n");
		exit(0);
	};

	GetClientRect(hMainWnd, &rect);
	HDC hdc = GetDC(hMainWnd);
    memdc = CreateCompatibleDC(hdc);
	membm = CreateCompatibleBitmap(hdc, rect.right - rect.left + 1, rect.bottom - rect.top + 1 + secBufferHeight*fontHeight);
    SelectObject(memdc, membm);
	ReleaseDC(hMainWnd, hdc);


	arrowCursor = LoadCursor(0, IDC_ARROW);
	handCursor = LoadCursor(0, IDC_HAND);

	if (arrowCursor == 0 || handCursor == 0)
	{
		cputs("Error loading cursors\r\n");
		exit(0);
	};

	LOGBRUSH lb;

	lb.lbStyle = BS_SOLID;
	lb.lbColor = RGB(0xFF, 0, 0);

	redBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0xFF, 0xFF, 0);

	yelBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0x7F, 0x7F, 0x7F);

	gryBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0, 0xFF, 0);

	grnBrush = CreateBrushIndirect(&lb);

	for (u32 i = 0; i < sizeof(secBuffer); i+=2)
	{
		secBuffer[i] = 0x20;
		secBuffer[i+1] = 0xF0;
	};
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

void UpdateDisplay()
{
	static byte curChar = 0;
	//static byte c = 0, i = 0;
	//static byte flashMask = 0;
	static const byte a[4] = { 0x80, 0x80+0x40, 0x80+20, 0x80+0x40+20 };

	MSG msg;

	static dword pt = 0;
	static TM32 tm;

	u32 t = GetTickCount();

	if ((t-pt) >= 2)
	{
		pt = t;

		while(PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			GetMessage (&msg, NULL, 0, 0);

			TranslateMessage (&msg);

			DispatchMessage (&msg);
		};
	};

	static char buf[80];
	static char buf1[sizeof(secBuffer)];

	if (tm.Check(20))
	{
		bool rd = true;

		if (rd)
		{
			//SelectObject(memdc, font1);
			//SetBkColor(memdc, 0x074C00);
			//SetTextColor(memdc, 0x00FF00);
			//TextOut(memdc, 443, 53, buf, 20);
			//TextOut(memdc, 443, 30*1+53, buf+20, 20);
			//TextOut(memdc, 443, 30*2+53, buf+40, 20);
			//TextOut(memdc, 443, 30*3+53, buf+60, 20);

			SelectObject(memdc, font2);

			for (u32 j = 0; j < secBufferHeight; j++)
			{
				for (u32 i = 0; i < secBufferWidth; i++)
				{
					u32 n = (j*secBufferWidth+i)*2;

					u8 t = secBuffer[n+1];

					SetBkColor(memdc, pallete[(t>>4)&0xF]);
					SetTextColor(memdc, pallete[t&0xF]);
					TextOut(memdc, i*fontWidth, j*fontHeight+287, secBuffer+n, 1);
				};
			};

			RedrawWindow(hMainWnd, 0, 0, RDW_INVALIDATE);

		}; // if (rd)

	}; // if ((t-pt) > 10)

}

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	HDC hdc;
	PAINTSTRUCT ps;
	bool c;
	static int x, y;
	int x0, y0, r0;
	static char key = 0;
	static char pkey = 0;

	RECT rect;

	static bool move = false;
	static int movex, movey = 0;

//	char *buf = (char*)screenBuffer;

    switch (message)
	{
        case WM_CREATE:

            break;

	    case WM_DESTROY:

		    break;

        case WM_PAINT:

			if (GetUpdateRect(hWnd, &rect, false)) 
			{
				hdc = BeginPaint(hWnd, &ps);

				//printf("Update RECT: %li, %li, %li, %li\r\n", ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);

				c = BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left + 1, ps.rcPaint.bottom - ps.rcPaint.top + 1, memdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
	 
				EndPaint(hWnd, &ps);
			};

            break;

        case WM_CHAR:

			pressedKey = wParam;

			if (pressedKey == '`')
			{
				GetWindowRect(hWnd, &rect);

				if ((rect.bottom-rect.top) > 340)
				{
					MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top-fontHeight*secBufferHeight, true); 
				}
				else
				{
					MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top+fontHeight*secBufferHeight, true); 
				};
			};

            break;

		case WM_MOUSEMOVE:

			x = LOWORD(lParam); y = HIWORD(lParam);

			if (move)
			{
				GetWindowRect(hWnd, &rect);
				SetWindowPos(hWnd, HWND_TOP, rect.left+x-movex, rect.top+y-movey, 0, 0, SWP_NOSIZE); 
			};

			return 0;

			break;

		case WM_MOVING:

			return TRUE;

			break;

		case WM_SYSCOMMAND:

			return 0;

			break;

		case WM_LBUTTONDOWN:

			move = true;
			movex = x; movey = y;

			SetCapture(hWnd);

			return 0;

			break;

		case WM_LBUTTONUP:

			move = false;

			ReleaseCapture();

			return 0;

			break;

		case WM_MBUTTONDOWN:

			ShowWindow(hWnd, SW_MINIMIZE);

			return 0;

			break;

		case WM_ACTIVATE:

			if (HIWORD(wParam) != 0 && LOWORD(wParam) != 0)
			{
				ShowWindow(hWnd, SW_NORMAL);
			};

			break;

		case WM_CLOSE:

			//run = false;

			break;
	};
    
	return DefWindowProc(hWnd, message, wParam, lParam);
}

#endif		
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

int PutString(u32 x, u32 y, byte c, const char *str)
{
	char *dst = secBuffer+(y*secBufferWidth+x)*2;
	dword i = secBufferWidth-x;

	while (*str != 0 && i > 0)
	{
		*(dst++) = *(str++);
		*(dst++) = c;
		i -= 1;
	};

	return secBufferWidth-x-i;
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef	WIN32

int Printf(u32 xx, u32 yy, byte c, const char *format, ... )
{
	char buf[1024];

	va_list arglist;

    va_start(arglist, format);
    vsprintf(buf, format, arglist);
    va_end(arglist);

	return PutString(xx, yy, c, buf);
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i32	Get_FRAM_SPI_SessionsAdr() { return FRAM_SPI_SESSIONS_ADR; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i32	Get_FRAM_I2C_SessionsAdr() { return FRAM_I2C_SESSIONS_ADR; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "Hardware Init ... \n");

#ifdef CPU_SAME53	
	
	using namespace HW;

	#ifdef _DEBUG

		SEGGER_RTT_printf(0, "HW::WDT->CTRLA = 0x%02X\n", (u32)(HW::WDT->CTRLA));

		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "Disable WDT ... ");

		HW::MCLK->APBAMASK |= APBA_WDT;

		while(HW::WDT->SYNCBUSY);

		if (HW::WDT->CTRLA & WDT_ENABLE)
		{
			HW::WDT->CTRLA = 0;
			
			while(HW::WDT->SYNCBUSY);
		};

		SEGGER_RTT_WriteString(0, "OK\n");

	#endif

//	HW::PIOA->BSET(13);

	HW::GCLK->GENCTRL[GEN_32K]	= GCLK_DIV(1)	|GCLK_SRC_OSCULP32K	|GCLK_GENEN;

	HW::GCLK->GENCTRL[GEN_1M]	= GCLK_DIV(25)	|GCLK_SRC_XOSC1		|GCLK_GENEN		|GCLK_OE;

	HW::GCLK->GENCTRL[GEN_25M]	= GCLK_DIV(1)	|GCLK_SRC_XOSC1		|GCLK_GENEN;

//	HW::GCLK->GENCTRL[GEN_500K] = GCLK_DIV(50)	|GCLK_SRC_XOSC1		|GCLK_GENEN;


	HW::MCLK->APBAMASK |= APBA_EIC;
	HW::GCLK->PCHCTRL[GCLK_EIC] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;

	HW::MCLK->APBBMASK |= APBB_EVSYS;

	HW::GCLK->PCHCTRL[GCLK_SERCOM_SLOW]		= GCLK_GEN(GEN_32K)|GCLK_CHEN;	// 32 kHz
	HW::GCLK->PCHCTRL[GCLK_SERCOM5_CORE]	= GCLK_GEN(GEN_MCK)|GCLK_CHEN;	
	HW::GCLK->PCHCTRL[GCLK_SERCOM6_CORE]	= GCLK_GEN(GEN_MCK)|GCLK_CHEN;	
	HW::GCLK->PCHCTRL[GCLK_SERCOM7_CORE]	= GCLK_GEN(GEN_MCK)|GCLK_CHEN;	

	HW::MCLK->APBDMASK |= APBD_SERCOM5|APBD_SERCOM6|APBD_SERCOM7;


#endif

	Init_time(MCK);
	HW_NAND_Init();
	I2C_Init();
	SPI_Init();

#ifndef WIN32

	InitClock();

	//InitManTransmit();
	InitManRecieve();
	//Init_CRC_CCITT_DMA();

	InitManTransmit2();

	EnableVCORE();

	WDT_Init();

#else

	InitDisplay();

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
#ifndef WIN32

	static byte i = 0;

	static Deb db(false, 20);

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( SPI_Update();		);
		CALL( I2C_Update();		);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
