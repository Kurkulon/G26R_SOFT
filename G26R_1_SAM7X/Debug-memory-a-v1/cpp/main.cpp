
#include "common.h"
#include "main.h"

#include "rtc.h"
#include "testpin.h"
#include "emac.h"
#include "fram.h"
#include "flash.h"
#include "usart1.h"
#include "manchester.h"
#include "k9k8g08u.h"
#include "vector.h"
#include "telemetry.h"
#include "adc.h"
#include "sensors.h"
#include "ds32c35.h"
#include "wdt.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

main_mode_type main_mode = MAIN_MODE_NONE;
unsigned short main_mode_check_ms = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Update_ALL_MODE()
{
	static unsigned char i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL(	WDT_Restart();  								);
		CALL(	ADC_Idle();										);
		CALL( 	Sensors_Idle();									);
		CALL( 	USART1_Idle();									);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static unsigned char i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( 	RTC_Idle();										);
		CALL( 	K9K8G08U_Idle();								);
		CALL( 	FRAM_Idle();									);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Update_AUTONOM_MODE()
{
	static unsigned char i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( 	USART1_Idle();						);
		CALL( 	Manchester_Idle();					);
		CALL( 	ADC_Idle();							);
		CALL( 	Sensors_Idle();						);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ErrorHalt(u32 code)
{
	RTM32 rtm;

	byte highDigit = code / 10 + 1;
	byte lowDigit = code % 10 + 1;

	while(1)
	{
		if (rtm.Check(MS2RT(100)))
		{
			for (byte i = 0; i < highDigit; i++)
			{
				TestPin1_Set();
				__nop();
				TestPin1_Clr();
				__nop();
			};

			delay(10);

			for (byte i = 0; i < lowDigit; i++)
			{
				TestPin1_Set();
				__nop();
				TestPin1_Clr();
				__nop();
			};
		};
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Error_PIO_pin(byte code, byte pio, byte pin)
{
	byte highDigit = pin / 10 + 1;
	byte lowDigit = pin % 10 + 1;

	code += 1;
	pio += 1;

	while(1)
	{
		for (byte i = 0; i < code; i++)
		{
			TestPin1_Set();
			__nop();
			TestPin1_Clr();
			__nop();
		};

		delay(10);

		for (byte i = 0; i < pio; i++)
		{
			TestPin1_Set();
			__nop();
			TestPin1_Clr();
			__nop();
		};

		delay(10);

		for (byte i = 0; i < highDigit; i++)
		{
			TestPin1_Set();
			__nop();
			TestPin1_Clr();
			__nop();
		};

		delay(10);

		for (byte i = 0; i < lowDigit; i++)
		{
			TestPin1_Set();
			__nop();
			TestPin1_Clr();
			__nop();
		};

		delay(10000);
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Test_PIO_Pins()
{
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOA ) ;
	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOB ) ;

	const u32 testMaskA = ((1<<19)-1) | (1<<22) | (7<<27);
	const u32 testMaskB = ((1<<28)-1);

	for (u32 i = 0; i < 64; i++)
	{
		PIOA->PIO_PER	= testMaskA;		PIOB->PIO_PER	= testMaskB;
		PIOA->PIO_ODR	= testMaskA;		PIOB->PIO_ODR	= testMaskB;
		PIOA->PIO_PPUER = testMaskA;		PIOB->PIO_PPUER = testMaskB;

		u32 pinMaskA = 1ULL<<i;
		u32 pinMaskB = (1ULL<<i)>>32;

		if ((pinMaskA & testMaskA) || (pinMaskB & testMaskB))
		{
			delay(100); 

			u32 a1 = PIOA->PIO_PDSR;
			u32 b1 = PIOB->PIO_PDSR;

			if ((pinMaskA && ((a1 & pinMaskA) == 0)) || (pinMaskB && ((b1 & pinMaskB) == 0)))
			{
				Error_PIO_pin(ERROR_PIO_PIN_LOW, i>>5, i&31);
			};

			PIOA->PIO_OER = pinMaskA; PIOA->PIO_CODR = pinMaskA;
			PIOB->PIO_OER = pinMaskB; PIOA->PIO_CODR = pinMaskB;

			delay(100); 

			u32 a2 = PIOA->PIO_PDSR;
			u32 b2 = PIOB->PIO_PDSR;

			if ((a2 & pinMaskA) || (b2 & pinMaskB))
			{
				Error_PIO_pin(ERROR_PIO_PIN_HI, i>>5, i&31);
			};

			PIOA->PIO_SODR = pinMaskA;
			PIOB->PIO_SODR = pinMaskB;
			
			delay(100); 

			u32 a3 = PIOA->PIO_PDSR;
			u32 b3 = PIOB->PIO_PDSR;

			bool ca1 = (a1 ^ a2) & testMaskA & ~pinMaskA;
			bool ca2 = (a2 ^ a3) & testMaskA & ~pinMaskA;

			bool cb1 = (b1 ^ b2) & testMaskB & ~pinMaskB;
			bool cb2 = (b2 ^ b3) & testMaskB & ~pinMaskB;

			if (ca1 || ca2 || cb1 || cb2) 
			{
				Error_PIO_pin(ERROR_PIO_PIN_SHORT, i>>5, i&31);
			};
		};
	};

	PIOA->PIO_PER	= testMaskA;		PIOB->PIO_PER	= testMaskB;
	PIOA->PIO_ODR	= testMaskA;		PIOB->PIO_ODR	= testMaskB;
	PIOA->PIO_PPUER = testMaskA;		PIOB->PIO_PPUER = testMaskB;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/**************************** Main ********************************************/

int main(void)
{
	TestPins_Init();

	Test_PIO_Pins();

	RTC_Init();

	EMAC_Init();
	DS32C35_Init();
	FRAM_Init();
	K9K8G08U_Init();
	USART1_Init();

	for(;;)
	{
		TestPin1_Set();

		UpdateMisc();

		TestPin1_Clr();

		EMAC_Idle();
	}; // 	for(;;)
}



