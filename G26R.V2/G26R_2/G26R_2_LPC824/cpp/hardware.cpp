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


#define PIN_LIN			13 
#define PIN_SDA			11 
#define PIN_SCL			10 
#define PIN_DIS			16 
#define PIN_DATA		27
#define PIN_URXD		26
#define PIN_SCLK		25
#define PIN_UTXD		24
#define PIN_DACSYNC		15
#define PIN_SYNC		0
#define PIN_HIN			22
#define PIN_A3			20
#define PIN_A2			19
#define PIN_A1			18
#define PIN_A0			17


#define LIN				(1<<PIN_LIN	) 
#define SDA				(1<<PIN_SDA	) 
#define SCL				(1<<PIN_SCL	) 
#define DIS				(1<<PIN_DIS	) 
#define DATA			(1<<PIN_DATA) 
#define URXD			(1<<PIN_URXD) 
#define SCLK			(1<<PIN_SCLK) 
#define UTXD			(1<<PIN_UTXD) 
#define DACSYNC			(1<<PIN_DACSYNC) 
#define SYNC			(1<<PIN_SYNC) 
#define HIN				(1<<PIN_HIN	) 
#define A3				(1<<PIN_A3	) 
#define A2				(1<<PIN_A2	) 
#define A1				(1<<PIN_A1	) 
#define A0				(1<<PIN_A0	) 


//u16 curHV = 0;
//u16 reqHV = 800;
u16 curHV = 0;
u16 reqHV = 0;
u32 fHV = 0;
u16 freq = 500;
u16 freqCLK = US2CLK(1);
u16 sampleTime = US2CLK(500);
byte chnl = 0;

//u32 tachoCount = 0;
//i32 shaftPos = 0;
//
//u32 startTime = 0;
//u32 stopTime = 0;

//static byte twiWriteData[2];

__packed struct  Cmd
{
	byte cmd; 
	byte chnl; 
	byte clk; 
	byte disTime; 
	u16 enTime; 
	byte chkSum; 
	byte busy; 
	byte ready; 
};

static Cmd req;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline void SetPWM()
{
	HW::SCT->MATCHREL_L[0] = US2CLK(1);				// LIN=0; HIN=1; DIS=0;
	HW::SCT->MATCHREL_L[1] = freqCLK + US2CLK(1);	// LIN=1; HIN=0; DIS=0;
	HW::SCT->MATCHREL_L[2] = freqCLK*2 + US2CLK(1);	// LIN=0; HIN=0; DIS=0;
	HW::SCT->MATCHREL_L[3] = US2CLK(7);				// LIN=0; HIN=0; DIS=1;
	HW::SCT->MATCHREL_L[4] = sampleTime;			// LIN=0; HIN=0; DIS=0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetFreq(u16 v)
{
	if (freq != v)
	{
		freq = v;
		freqCLK = (MCK / 2000 + freq/2) / freq;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetSampleTime(u16 v)
{
	sampleTime = (MCK/1000000) * v + US2CLK(10);

	SetPWM();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*----------------------------------------------------------------------------
  Initialize the system
 *----------------------------------------------------------------------------*/
extern "C" void SystemInit()
{
	u32 i;
	using namespace CM0;
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::SWM_M | CLK::IOCON_M | CLK::GPIO_M | HW::CLK::MRT_M | HW::CLK::CRC_M | HW::CLK::WWDT_M | HW::CLK::UART0_M | HW::CLK::DMA_M;

	SYSCON->PDRUNCFG &= ~(1<<6); // WDTOSC_PD = 0

	GPIO->DIRSET0 = A0|A1|A2|A3|HIN|(1<<12)|LIN|DIS;
	GPIO->CLR0 = LIN|HIN;
	GPIO->SET0 = A0|A1|A2|A3;

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

	//SYSCON->SYSAHBCLKDIV  = SYSAHBCLKDIV_Val;

	SYSCON->UARTCLKDIV = 1;
	SWM->U0_RXD = PIN_URXD;
	SWM->U0_TXD = PIN_UTXD;

	DMA->SRAMBASE = DmaTable;
	DMA->CTRL = 1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static __irq void Handler_SCT()
{
	HW::SCT->EVFLAG = ~0;

	chnl += 1;

	if (chnl > 12) chnl = 0;

	HW::GPIO->MASK0 = ~(A0|A1|A2|A3);
	HW::GPIO->MPIN0 = chnl<<PIN_A0;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitSCT()
{
	using namespace HW;

	SYSCON->SYSAHBCLKCTRL |= CLK::SCT_M;

	SCT->STATE_L = 0;
	SCT->REGMODE_L = 0;

	SCT->MATCHREL_L[0] = US2CLK(1);				// LIN=0; HIN=1; DIS=0;
	SCT->MATCHREL_L[1] = freqCLK + US2CLK(1);	// LIN=1; HIN=0; DIS=0;
	SCT->MATCHREL_L[2] = freqCLK*2 + US2CLK(1);	// LIN=0; HIN=0; DIS=0;
	SCT->MATCHREL_L[3] = US2CLK(7);				// LIN=0; HIN=0; DIS=1;
	SCT->MATCHREL_L[4] = US2CLK(500);			// LIN=0; HIN=0; DIS=0;

	SCT->OUT[0].SET = (1<<1)|(1<<4);//|(1<<5);					// LIN
	SCT->OUT[0].CLR = (1<<0)|(1<<2)|(1<<3)/*|(1<<6)*/;

	SCT->OUT[1].SET = (1<<0);//|(1<<2);					// HIN
	SCT->OUT[1].CLR = (1<<1)|(1<<3)|(1<<4)/*|(1<<5)|(1<<6)*/;

	SCT->OUT[2].SET = (1<<3);							// DIS
	SCT->OUT[2].CLR = (1<<0)|(1<<1)|(1<<2)|(1<<4)/*|(1<<5)|(1<<6)*/;

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

	//SCT->EVENT[5].STATE = 0;
	//SCT->EVENT[5].CTRL = (1<<5)|(0<<6)|(1<<12)|5;

	//SCT->EVENT[6].STATE = 0;
	//SCT->EVENT[6].CTRL = (1<<5)|(0<<6)|(1<<12)|6;

	SCT->EVENT[7].STATE = 1;
	SCT->EVENT[7].CTRL = (3<<10)|(2<<12);

	SCT->START_L = 1<<7;
	SCT->STOP_L = (1<<4);
	SCT->HALT_L = 0;
	SCT->LIMIT_L = (1<<4);

	SCT->CONFIG = 0; 

	SWM->CTOUT_0 = PIN_LIN;
	SWM->CTOUT_1 = PIN_HIN;
	SWM->CTOUT_2 = PIN_DIS;

	INPUTMUX->SCT0_INMUX[0] = 0;
	SWM->CTIN_0 = 0;

//	SCT->OUTPUT = 1;
	SCT->CTRL_L = (1<<3);//|(1<<2);


	VectorTableExt[SCT_IRQ] = Handler_SCT;
	CM0::NVIC->ICPR[0] = 1 << SCT_IRQ;
	CM0::NVIC->ISER[0] = 1 << SCT_IRQ;

	SCT->EVEN = 1<<4;
	SCT->EVFLAG = 1<<4;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void InitADC()
//{
//	using namespace HW;
//
//	//SWM->PINASSIGN[3] = (SWM->PINASSIGN[3] & 0x00FFFFFF) | 0x09000000;
//	//SWM->PINASSIGN[4] = (SWM->PINASSIGN[4] & 0xFF000000) | 0x00100FFF;
//
//	SWM->PINENABLE0.B.ADC_0 = 0;
//	SWM->PINENABLE0.B.ADC_1 = 0;
//
//
//	SYSCON->PDRUNCFG &= ~(1<<4);
//	SYSCON->SYSAHBCLKCTRL |= CLK::ADC_M;
//
//	ADC->CTRL = (1<<30)|49;
//
//	while(ADC->CTRL & (1<<30));
//
//	ADC->CTRL = 24;
//	ADC->SEQA_CTRL = 3|(1UL<<31)|(1<<27);
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//byte *twiData = (byte*)&twiReq;
u16 twiCount = 0;
u16 twiMaxCount = sizeof(req);
u32 twiReqCount = 0;
bool twiWrite = false;
bool twiRead = false;

static __irq void Handler_TWI()
{
	using namespace HW;

	static byte *p = (byte*)&req;
	static u16 count = 0;
	static bool read = false;
	static bool write = false;

	if(I2C0->INTSTAT & SLVPENDING)
	{
		u32 state = I2C0->STAT & SLVSTATE;

		if(state == SLVST_ADDR) // Address plus R/W received
		{
			p = (byte*)&req;	count = 0;

			read = false;
			write = false;

			//*p = I2C0->SLVDAT; // receive data
			//if (count <= twiMaxCount)
			//{
			//	p++;
			//};

			//count++;

			I2C0->SLVCTL = SLVCONTINUE;
		}
		else if(state == SLVST_RX) // Received data is available
		{
			*p = I2C0->SLVDAT; // receive data

			I2C0->SLVCTL = SLVCONTINUE;

			if (count <= twiMaxCount)
			{
				p++;
			};

			count++;

			read = true;
		}
		else if(state == SLVST_TX) // Data can be transmitted 
		{
			I2C0->SLVDAT = *p++; // write data

			I2C0->SLVCTL = SLVCONTINUE;

			//if (count <= twiMaxCount)
			//{
			//	p++;
			//};

			count++;

			write = true;
		};
		
		I2C0->STAT = SLVPENDING;
	};

	if(I2C0->INTSTAT & SLVDESELEN)
	{
		I2C0->STAT = SLVDESELEN;
		twiCount = count;
		twiReqCount++;
		twiWrite = write;
		twiRead = read;

		if (read)
		{
			req.busy = true;
			req.ready = false;
		};
	};
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

	I2C0->CLKDIV = 14;
	I2C0->SLVADR0 = 11 << 1;
	I2C0->INTENSET = SLVPENDINGEN|SLVDESELEN;
	I2C0->CFG = SLVEN;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTWI()
{
	static byte i = 0;

	switch (i)
	{
		case 0:

			if (twiRead)
			{
				HW::SCT->CTRL_L = 1<<2;
				//HW::SCT->OUTPUT = 0;

				//HW::SCT->OUTPUT = 1;

				i++;
			};

			break;

		case 1:

			chnl = req.chnl;

			HW::GPIO->MASK0 = ~(A0|A1|A2|A3);
			HW::GPIO->MPIN0 = chnl<<PIN_A0;

			i++;

			break;

		case 2:

			//HW::SCT->OUTPUT = 1;
			HW::SCT->CTRL_L = 1<<1;

			twiRead = false;
			req.busy = false;
			req.ready = true;

			i = 0;

			break;
	};

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
		CALL( fHV += (((ADC->DAT2&0xFFF0) * 567) >> 16) - 6 - curHV; curHV = fHV >> 4; );
		//CALL( fvAP += (((ADC->DAT1&0xFFF0) * 3300) >> 16) - vAP; vAP = fvAP >> 3;	);
	};

//	i = (i > (__LINE__-S-3)) ? 0 : i;
	i &= 1;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitADC()
{
	using namespace HW;

	SWM->PINENABLE0.B.ADC_2 = 0;

	SYSCON->PDRUNCFG &= ~(1<<4);
	SYSCON->SYSAHBCLKCTRL |= CLK::ADC_M;

	ADC->CTRL = ADC_CTRL_CALMODE(1)|ADC_CTRL_CLKDIV(49);

	while(ADC->CTRL & ADC_CTRL_CALMODE(1));

	ADC->CTRL = ADC_CTRL_CLKDIV(24);
	ADC->SEQA_CTRL = ADC_SEQ_CTRL_CHANNELS(4)|ADC_SEQ_CTRL_SEQ_ENA(1)|ADC_SEQ_CTRL_BURST(1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateDAC()
{
	using namespace HW;

	static TM32 tm;
	static u32 dstFV = 0;

	if ((SPI0->STAT & SPI_STAT_TXRDY(1)) == 0) return;

	if (tm.Check(1))
	{
		u32 t = ((reqHV+14) * 521711UL);

		dstFV += (i32)(t - dstFV) / 128;

		t = dstFV >> 16;

		if (t > 0xFFF) t = 0xFFF;

		t = (~t) & 0xFFF;

		SPI0->TXDATCTL = t | SPI_LEN(15)|SPI_EOT(1)|SPI_TXSSEL0_N(0)|SPI_TXSSEL1_N(1)|SPI_TXSSEL2_N(1)|SPI_TXSSEL3_N(1)|SPI_RXIGNORE(1);
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitDAC()
{
	using namespace HW;

	//SWM->PINASSIGN[3] = (SWM->PINASSIGN[3] & 0x00FFFFFF) | 0x09000000;
	//SWM->PINASSIGN[4] = (SWM->PINASSIGN[4] & 0xFF000000) | 0x00100FFF;

	SWM->SPI0_SCK	= PIN_SCLK;	//9;
	SWM->SPI0_MOSI	= PIN_DATA;	//15;
	SWM->SPI0_SSEL0	= PIN_DACSYNC;	//16;

	SYSCON->SYSAHBCLKCTRL |= CLK::SPI0_M;

	SPI0->CFG = SPI_MASTER(1) | SPI_ENABLE(1);
	SPI0->DLY = SPI_PRE_DELAY(1)|SPI_POST_DELAY(1);
	SPI0->DIV = 4;

	//SPI0->TXDATCTL = SPI_TXDATCTL_LEN(16) | SPI_TXDATCTL_EOT(1) | SPI_TXDATCTL_TXSSEL0_N(1) | SPI_TXDATCTL_RXIGNORE(1);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	using namespace HW;

	Init_time();
	InitADC();
	InitDAC();
	InitSCT();
	InitTWI();

	SYSCON->SYSAHBCLKCTRL |= HW::CLK::WWDT_M;
	SYSCON->PDRUNCFG &= ~(1<<6); // WDTOSC_PD = 0
	SYSCON->WDTOSCCTRL = (1<<5)|1; 

#ifndef _DEBUG
	WDT->TC = 0x1FF;
	WDT->MOD = 0x3;
	ResetWDT();
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
	HW::ResetWDT();

	static byte i = 0;

	static TM32 tm;


	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateTWI() );
		CALL( UpdateADC() );
		CALL( UpdateDAC() );
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;
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
