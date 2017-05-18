#include "hardware.h"
#include "time.h"
#include "ComPort.h"
#include "crc16.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//u32 fps = 0;

//static byte sec = 0;




//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	InitHardware();

	while (1)
	{
		UpdateHardware();

		HW::GPIO->NOT0 = 1<<12;

	}; // while (1)
}
