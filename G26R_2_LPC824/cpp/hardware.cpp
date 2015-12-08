#include "types.h"
#include "core.h"
#include "time.h"

#include "hardware.h"

#define MSTEN			(0x1)
#define SLVEN			(0x2)
#define MSTPENDING		(0x1)
#define MSTSTATE		(0xe)
#define MSTST_IDLE		(0x0)
#define MSTST_RX 		(0x2)
#define MSTST_TX 		(0x4)
#define MSTST_NACK_ADDR (0x6)
#define MSTST_NACK_TX	(0x8)
#define SLVPENDING		(0x100)
#define SLVSTATE		(0x600)
#define SLVST_ADDR		(0x000)
#define SLVST_RX 		(0x200)
#define SLVST_TX 		(0x400)
#define MSTCONTINUE		(0x1)
#define MSTSTART		(0x2)
#define MSTSTOP			(0x4)
#define SLVCONTINUE		(0x1)
#define SLVNACK			(0x2)
#define SLVDMA			8
#define SLVPENDINGEN	(0x100)
#define SLVDESELEN		(1<<15)


//u16 curHV = 0;
//u16 reqHV = 800;
u16 curADC = 0;
u32 fcurADC = 0;
u16 vAP = 0;
u32 fvAP = 0;
//u32 tachoCount = 0;
//i32 shaftPos = 0;
//
//u32 startTime = 0;
//u32 stopTime = 0;

static byte twiWriteData[2];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void IntDummyHandler()
{
	__breakpoint(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void HardFaultHandler()
{
	__breakpoint(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void ExtDummyHandler()
{
	__breakpoint(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitVectorTable()
{
	for (u32 i = 0; i < ArraySize(VectorTableInt); i++)
	{
		VectorTableInt[i] = IntDummyHandler;
	};

	for (u32 i = 0; i < ArraySize(VectorTableExt); i++)
	{
		VectorTableExt[i] = ExtDummyHandler;
	};

	VectorTableInt[3] = HardFaultHandler;

	CM0::SCB->VTOR = (u32)VectorTableInt;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*----------------------------------------------------------------------------
  Initialize the system
 *----------------------------------------------------------------------------*/
extern "C" void SystemInit()
{
	u32 i;
	using namespace CM0;
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::SWM_M | CLK::IOCON_M | CLK::GPIO_M | HW::CLK::MRT_M | HW::CLK::CRC_M;

	GPIO->DIRSET0 = (1<<17)|(1<<18)|(1<<19)|(1<<20)|(1<<22)|(1<<12)|(1<<13)|(1<<16);
	GPIO->CLR0 = (1<<13)|(1<<22);
	GPIO->SET0 = (1<<17)|(1<<18)|(1<<19)|(1<<20);

	IOCON->PIO0_1.B.MODE = 0;

	SWM->PINENABLE0.B.CLKIN = 0;

	for (i = 0; i < 200; i++) __nop();

	SYSCON->SYSPLLCLKSEL  = 3;					/* Select PLL Input         */
	SYSCON->SYSPLLCLKUEN  = 0;					/* Update Clock Source      */
	SYSCON->SYSPLLCLKUEN  = 1;					/* Update Clock Source      */
	while (!(SYSCON->SYSPLLCLKUEN & 1));		/* Wait Until Updated       */

	SYSCON->MAINCLKSEL    = 1;					/* Select PLL Clock Output  */
	SYSCON->MAINCLKUEN    = 0;					/* Update MCLK Clock Source */
	SYSCON->MAINCLKUEN    = 1;					/* Update MCLK Clock Source */
	while (!(SYSCON->MAINCLKUEN & 1));			/* Wait Until Updated       */

//	SYSCON->SYSAHBCLKDIV  = SYSAHBCLKDIV_Val;

	//SYSCON->UARTCLKDIV = 1;
	//SWM->U0_RXD = 26;
	//SWM->U0_TXD = 16;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateADC()
{
	using namespace HW;

	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( fcurADC += (((ADC->DAT0&0xFFF0) * 1800 ) >> 16) - curADC;	curADC = fcurADC >> 3;	);
		CALL( fvAP += (((ADC->DAT1&0xFFF0) * 3300) >> 16) - vAP; vAP = fvAP >> 3;	);
	};

//	i = (i > (__LINE__-S-3)) ? 0 : i;
	i &= 1;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitPWM()
{
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::SCT_M;

	SCT->STATE_L = 0;
	SCT->REGMODE_L = 0;

	SCT->MATCH_L[0] = US2CLK(1);		// LIN=0; HIN=1; DIS=0;
	SCT->MATCH_L[1] = US2CLK(2.0416);	// LIN=1; HIN=0; DIS=0;
	SCT->MATCH_L[2] = US2CLK(3.0833);	// LIN=0; HIN=0; DIS=0;
	SCT->MATCH_L[3] = US2CLK(5);		// LIN=0; HIN=0; DIS=1;
	SCT->MATCH_L[4] = US2CLK(2000);		// LIN=1; HIN=0; DIS=0;

	SCT->OUT[0].SET = (1<<1)|(1<<4);
	SCT->OUT[0].CLR = (1<<0)|(1<<2)|(1<<3);

	SCT->OUT[1].SET = (1<<0);
	SCT->OUT[1].CLR = (1<<1)|(1<<2)|(1<<3)|(1<<4);

	SCT->OUT[2].SET = (1<<3);//|(1<<0)|(1<<1)|(1<<2)|(1<<4);
	SCT->OUT[2].CLR = (1<<0)|(1<<1)|(1<<2)|(1<<4);

	SCT->EVENT[0].STATE = 1;
	SCT->EVENT[0].CTRL = (1<<5)|(0<<6)|(1<<12)|0;

	SCT->EVENT[1].STATE = 1;
	SCT->EVENT[1].CTRL = (1<<5)|(0<<6)|(1<<12)|1;

	SCT->EVENT[2].STATE = 1;
	SCT->EVENT[2].CTRL = (1<<5)|(0<<6)|(1<<12)|2;

	SCT->EVENT[3].STATE = 1;
	SCT->EVENT[3].CTRL = (1<<5)|(0<<6)|(1<<12)|3;

	SCT->EVENT[4].STATE = 1;
	SCT->EVENT[4].CTRL = (1<<5)|(0<<6)|(1<<12)|4;

	SCT->EVENT[5].STATE = 1;
	SCT->EVENT[5].CTRL = (3<<10)|(2<<12);

	SCT->START_L = 1<<5;
	SCT->STOP_L = (1<<4);
	SCT->HALT_L = 0;
	SCT->LIMIT_L = 1<<4;

	SCT->CONFIG = 1<<7; 

	SWM->CTOUT_0 = 13;
	SWM->CTOUT_1 = 22;
	SWM->CTOUT_2 = 16;

	INPUTMUX->SCT0_INMUX0 = 0;
	SWM->CTIN_0 = 0;

	SCT->CTRL_L = (1<<3);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitADC()
{
	using namespace HW;

	//SWM->PINASSIGN[3] = (SWM->PINASSIGN[3] & 0x00FFFFFF) | 0x09000000;
	//SWM->PINASSIGN[4] = (SWM->PINASSIGN[4] & 0xFF000000) | 0x00100FFF;

	SWM->PINENABLE0.B.ADC_0 = 0;
	SWM->PINENABLE0.B.ADC_1 = 0;


	SYSCON->PDRUNCFG &= ~(1<<4);
	SYSCON->SYSAHBCLKCTRL |= CLK::ADC_M;

	ADC->CTRL = (1<<30)|49;

	while(ADC->CTRL & (1<<30));

	ADC->CTRL = 24;
	ADC->SEQA_CTRL = 3|(1UL<<31)|(1<<27);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void Handler_TWI()
{
	using namespace HW;

	if(I2C0->INTSTAT & SLVPENDING)
	{
		u32 state = I2C0->STAT & SLVSTATE;

		if(state == SLVST_ADDR) // Address plus R/W received
		{
			I2C0->SLVCTL = SLVCONTINUE;
		}
		else if(state == SLVST_RX) // Received data is available
		{
			byte data = I2C0->SLVDAT; // receive data

			I2C0->SLVCTL = SLVCONTINUE;
		}
		else if(state == SLVST_TX) // Data can be transmitted 
		{
			I2C0->SLVDAT = 0; // write data

			I2C0->SLVCTL = SLVCONTINUE;
		};
	};

	//if(I2C0->INTSTAT & SLVDESELEN)
	//{
	//	I2C0->STAT = SLVDESELEN;
	//};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitTWI()
{
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::I2C0_M;

	SWM->PINENABLE0.B.I2C0_SCL = 0;
	SWM->PINENABLE0.B.I2C0_SDA = 0;

	VectorTableExt[I2C0_IRQ] = Handler_TWI;
	CM0::NVIC->ICPR[0] = 1 << I2C0_IRQ;
	CM0::NVIC->ISER[0] = 1 << I2C0_IRQ;

	I2C0->SLVADR0 = 1 << 1;
	I2C0->INTENSET = SLVPENDINGEN/*|SLVDESELEN*/;
	I2C0->CFG = SLVEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	InitVectorTable();
	Init_time();
	InitADC();
	InitPWM();
//	InitTWI();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	static byte i = 0;

	static TM32 tm;


	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateADC() );
//		CALL( PID_Update() );
	};

//	i = (i > (__LINE__-S-3)) ? 0 : i;
	i &= 1;
	
//	UpdateMotor();

//	if (tm.Check(1000)) { TahoHandler(); }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 GetCRC(const void *data, u32 len)
{
	union { u32 *pd; u16 *pw; u8 *pb; };

	pb = (byte*)data;

//	byte * const p = (byte*)HW::CRC->B;

	HW::CRC->MODE = 0x15;
	HW::CRC->SEED = 0xFFFF;

	u32 dl = len>>2;
	u32 wl = (len&3)>>1;
	u32 bl = (len&1);

	for ( ; dl > 0; dl--) 
	{
		HW::CRC->D = *(pd++);
	};

	for ( ; wl > 0; wl--) 
	{
		HW::CRC->W = *(pw++);
	};

	for ( ; bl > 0; bl--) 
	{
		HW::CRC->B = *(pb++);
	};

	//for ( ; len > 0; len--) 
	//{
	//	HW::CRC->B = *(pb++);
	//};

	return HW::CRC->SUM;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
