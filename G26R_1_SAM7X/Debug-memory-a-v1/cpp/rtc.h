#ifndef MEMORY_AT91SAM7X256_RTC_H
#define MEMORY_AT91SAM7X256_RTC_H

#include "types.h"

#define RTC_INTERRUPT_LEVEL	4

#define RTC_TIME_PERIOD_MS		1	// Дискрет времени 1мс
#define RTC_TIME_PERIOD_TICS 		(unsigned int)(((float)(CLOCK_MCK/16) * (float)RTC_TIME_PERIOD_MS/1000.0f) - 1)

typedef __packed struct  //__attribute__ ((packed))   
{
	__packed union
	{
		__packed struct
		{
		    unsigned int msec:10;     // mili second value - [0,999] 
			unsigned int sec:6;     // Second value - [0,59] 
    		unsigned int min:6;     // Minute value - [0,59] 
    		unsigned int hour:5;    // Hour value - [0,23] 
//    			unsigned int reserved:5;   
		};

		unsigned int time;
	};

	__packed union
	{
		__packed struct
		{
			unsigned int day:5;    // Day of the month value - [1,31] 
			unsigned int mon:4;     // Month value - [1,12] 
			unsigned int year:12;    // Year value - [0,4095] 
//    			unsigned int reserved:11;   
		};
		unsigned int date;
	};
	
} RTC_type;

/************ RDC functions *************/
extern void RTC_Init();
extern void RTC_Idle();
extern RTC_type RTC_Get();
extern bool RTC_Check(RTC_type d_t);
extern bool RTC_Check(RTC_type start, RTC_type stop);
extern unsigned short RTC_Get_Millisecond();
extern void RTC_Set(RTC_type x);
extern short RTC_Get_Temperature();

/*****************************************************************************/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define US2RT(x) ((x*32768+500000)/1000000)
#define MS2RT(x) ((x*32768+500)/1000)

inline u32 GetRTT() { return AT91C_BASE_RTTC->RTTC_RTVR; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RTM32
{
	u32 pt;

	//RTM32() : pt(0) {}
	bool Check(u32 v) { u32 t = GetRTT(); if ((t - pt) >= v) { pt = t; return true; } else { return false; }; }
	bool Timeout(u32 v) { return (GetRTT() - pt) >= v; }
	void Reset() { pt = GetRTT(); }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
