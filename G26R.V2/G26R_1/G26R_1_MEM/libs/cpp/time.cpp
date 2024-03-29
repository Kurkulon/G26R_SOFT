#include "time.h"
#include "core.h"

#include "SEGGER_RTT.h"

#ifdef WIN32

#include <windows.h>
#include <time.h>

LARGE_INTEGER queryPerformanceFrequency = { 0, 0 };

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

volatile u32 msec = 0;

RTC timeBDC;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static const byte daysInMonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define SECONDS_FROM_1900_TO_1970       2208988800UL
#define SECONDS_FROM_1972_TO_2000       883612800UL
#define SECONDS_FROM_1970_TO_1972       63072000UL
#define SECONDS_FROM_1970_TO_2000		(SECONDS_FROM_1970_TO_1972+SECONDS_FROM_1972_TO_2000)      

#define SECONDS_PER_DAY                 (24 * 60 * 60)
#define DAYS_FROM_1900_TO_1970          ( ( long ) ( SECONDS_FROM_1900_TO_1970 / SECONDS_PER_DAY ) ) 

#define MONTH_YR        ( 12 )
#define DAY_YR          ( 365 )
#define HOUR_YR         ( DAY_YR * 24 )
#define MINUTE_YR       ( HOUR_YR * 60 )
#define SECOND_YR       ( MINUTE_YR * 60 )
#define __MONTHS        ( INT_MIN / MONTH_YR )
#define __DAYS          ( INT_MIN / DAY_YR )

#define SMALLEST_YEAR_VALUE ( __MONTHS + __DAYS + __MINUTES + __SECONDS )

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void GetTime(RTC *t)
{
	if (t == 0) return;

#ifndef WIN32

	__disable_irq();

	t->date = timeBDC.date;
	t->time = timeBDC.time;

	__enable_irq();

#else

	SYSTEMTIME lt;

	GetLocalTime(&lt);

	t->msec =  lt.wMilliseconds;
	t->sec	=  lt.wSecond;
	t->min 	=  lt.wMinute;
	t->hour =  lt.wHour;
	t->day 	=  lt.wDay;
	t->mon 	=  lt.wMonth;
	t->year =  lt.wYear;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CheckTime(const RTC &t)
{
	if (t.sec > 59 || t.min > 59 || t.hour > 23 || t.day < 1 || (t.mon-1) > 11 || (t.year-2000) > 99) { return false; };

	byte d = daysInMonth[t.mon] + ((t.mon == 2 && (t.year&3) == 0) ? 1 : 0);

	if (t.day > d) { return false; };

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool SetTime(const RTC &t)
{
#ifndef WIN32

	if (!CheckTime(t)) { return false; };

	__disable_irq();

	timeBDC = t;

	__enable_irq();

	return true;

#else

	SYSTEMTIME lt;

	lt.wMilliseconds	=	t.msec;
	lt.wSecond			=	t.sec;
	lt.wMinute			=	t.min;
	lt.wHour			=	t.hour;
	lt.wDay				=	t.day;
	lt.wMonth			=	t.mon;
	lt.wYear			=	t.year;

	return SetLocalTime(&lt);

	return true;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static __irq void Timer_Handler (void)
{
	msec++;

	if (timeBDC.msec < 999)
	{
		timeBDC.msec += 1;
	}
	else
	{
		timeBDC.msec = 0;

		if (timeBDC.sec < 59)
		{
			timeBDC.sec += 1;
		}
		else
		{
			timeBDC.sec = 0;

			if (timeBDC.min < 59)
			{
				timeBDC.min += 1;
			}
			else
			{
				timeBDC.min = 0;

				if (timeBDC.hour < 23)
				{
					timeBDC.hour += 1;
				}
				else
				{
					timeBDC.hour = 0;

					byte day = daysInMonth[timeBDC.mon] + ((timeBDC.mon == 2 && (timeBDC.year&3) == 0) ? 1 : 0);

//					if ((timeBDC.dayofweek += 1) > 6) timeBDC.dayofweek = 0;

					if (timeBDC.day < day)
					{
						timeBDC.day += 1;
					}
					else
					{
						timeBDC.day = 1;

						if (timeBDC.mon < 12)
						{
							timeBDC.mon += 1;
						}
						else
						{
							timeBDC.mon = 1;

							timeBDC.year += 1;
						};
					};
				};
			};
		};
	};
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitTimer()
{
#ifndef WIN32

	enum { freq = 1000 };

	timeBDC.day = 1;
	timeBDC.mon = 1;
	timeBDC.year = 2000;
	timeBDC.time = 0;

	CM4::SysTick->LOAD = (MCK+freq/2)/freq;
	VectorTableInt[15] = Timer_Handler;
	CM4::SysTick->VAL = 0;
	CM4::SysTick->CTRL = 7;
	__enable_irq();

#endif

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_CYAN "System Timer Init ... \n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RTT_Init()
{
#ifndef WIN32

	using namespace HW;

	#ifdef CPU_SAME53	

		HW::OSC32KCTRL->RTCCTRL = OSC32_RTCSEL_ULP32K;

		HW::MCLK->APBAMASK |= APBA_RTC;

		HW::RTC->CTRLA = 0;

		HW::RTC->CTRLA = RTC_SWRST;

		while(HW::RTC->CTRLA & RTC_SWRST);

		HW::RTC->CTRLA = RTC_ENABLE|RTC_MODE_COUNT32|RTC_COUNTSYNC;

		while(HW::RTC->SYNCBUSY != 0);

	#elif defined(CPU_XMC48)

		T_HW::CCU4_GLOBAL_Type * const module = HW::CCU43;
		T_HW::CCU4_CC4_Type * const slice = HW::CCU43_CC43;

		CCU_Enable(PID_CCU43);

		module->GCTRL = 0;

		module->GIDLC = CCU4_GIDLC_CS3I_Msk|CCU4_GIDLC_SPRB_Msk;

		slice->PRS = 0xFFFF;
		slice->PSC = 11; //20.48us

		module->GCSS = CCU4_GCSS_S3SE_Msk;  

		slice->TCSET = 1;

	#endif

#else

	if (!QueryPerformanceFrequency(&queryPerformanceFrequency))
	{
		queryPerformanceFrequency.LowPart = 1;
	};

#endif

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "RTT Init ... \n");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitCycleCountTimer()
{
#ifndef WIN32

	CM4::CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_WHITE "Cycle Count Timer Init ... ");

	__dsb(15);

	if ((CM4::DWT->CTRL & DWT_CTRL_NOCYCCNT_Msk) == 0)
	{
		CM4::DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n");
	}
	else
	{
		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED "!!! ERROR !!! Cycle counter not supported\n");
	};

#endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Init_time()
{
	//using namespace HW;

	InitTimer();
	RTT_Init();
	InitCycleCountTimer();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

