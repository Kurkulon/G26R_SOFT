#include "common.h"
#include "main.h"
#include "adc.h"
#include "rtc.h"
#include "types.h"

unsigned short adc_min[8];
unsigned short adc_max[8];
unsigned int adc_value[8];
unsigned short adc_count[8];
//unsigned short adc_time_ms;

void ADC_Idle()
{
	static RTM32 tm;

	static byte state = 0;
	static byte n = 0;
	static u32 status;

	//unsigned int ms = RTC_Get_Millisecond();

	switch (state)
	{
		case 0:

			if(tm.Check(MS2RT(ADC_PERIOD_MS)) && (AT91C_BASE_ADC->ADC_SR & ADC_CHANNELS_MASK) == ADC_CHANNELS_MASK)
			{
				n = 0;

				state++;
			};

			break;

		case 1:

       	    if((AT91C_BASE_ADC->ADC_SR & (1 << n)) && adc_count[n] < 0xFFFF)
			{
				u16 adc = ((u32*)&AT91C_BASE_ADC->ADC_CDR0)[n] & AT91C_ADC_DATA;

				adc_value[n] += adc; 
				adc_count[n] ++;
				if(adc_min[n] > adc) adc_min[n] = adc; 
				if(adc_max[n] < adc) adc_max[n] = adc; 
			};

			n++;

			if (n >= 8)
			{
				state = 0;

				AT91C_BASE_ADC->ADC_CR = AT91C_ADC_START;
			};

			break;

		default:

			state = 0;
	};

	//if(tm.Check(MS2RT(ADC_PERIOD_MS)))
	//{
	//	//adc_time_ms = ms;
 //      	unsigned int status = AT91C_BASE_ADC->ADC_SR;
	//	if((status & ADC_CHANNELS_MASK) == ADC_CHANNELS_MASK)
	//	{
	//		for(unsigned char i = 0; i < 8; i++)
	//		{
	//       	    if((status & (1 << i)) && adc_count[i] < 0xFFFF)
	//			{
	//				unsigned short adc = (*(unsigned int *)((unsigned int)(&AT91C_BASE_ADC->ADC_CDR0) + i * sizeof(unsigned int))) & AT91C_ADC_DATA;
	//				adc_value[i] += adc; 
	//				adc_count[i] ++;
	//				if(adc_min[i] > adc) adc_min[i] = adc; 
	//				if(adc_max[i] < adc) adc_max[i] = adc; 
	//			};
	//		};
	//	};

	//	AT91C_BASE_ADC->ADC_CR = AT91C_ADC_START;
	//};
}

bool ADC_Get(unsigned char channel, adc_measure_type type, unsigned short *value)
{
	if(adc_count[channel] == 0) return false;
	unsigned short average = (unsigned short)(adc_value[channel] / adc_count[channel]);
	switch (type)
	{
		case ADC_MEASURE_AVERAGING:
			*value = average;
			break;
		case ADC_MEASURE_PEAK_DETECTION:
			if(abs((short)average - (short)adc_min[channel]) >= abs((short)average - (short)adc_max[channel])) *value = adc_min[channel]; else *value = adc_max[channel];
			break;
		case ADC_MEASURE_MAX:
			*value = adc_max[channel];
			break;
		case ADC_MEASURE_MIN:
			*value = adc_min[channel];
			break;
	}
	adc_value[channel] = 0;
	adc_count[channel] = 0;
	adc_min[channel] = AT91C_ADC_DATA;
	adc_max[channel] = 0;
	return true;
}

void ADC_Init()
{
	for(unsigned char i = 0; i < 8; i++)
	{
		adc_value[i] = 0;
		adc_count[i] = 0;
	};

	AT91C_BASE_ADC->ADC_CR = AT91C_ADC_SWRST;
	AT91C_BASE_ADC->ADC_MR = AT91C_ADC_TRGEN_DIS | AT91C_ADC_LOWRES_10_BIT | AT91C_ADC_SLEEP_NORMAL_MODE | (AT91C_ADC_PRESCAL) | (AT91C_ADC_STARTUP); // частота по-минимуму, стартап тоже
	AT91C_BASE_ADC->ADC_CHER = ADC_CHANNELS_MASK;
	AT91C_BASE_ADC->ADC_CR = AT91C_ADC_START;
}


