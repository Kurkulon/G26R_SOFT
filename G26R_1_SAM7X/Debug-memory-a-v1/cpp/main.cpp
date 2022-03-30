
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

/**************************** Main ********************************************/

int main(void)
{
	TestPins_Init();

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



