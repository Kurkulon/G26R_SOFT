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
//	static bool c = false;

	TM32 tm;
	Dbt db(100);

	InitHardware();

	static byte i = 0;

	u32 adr = 0;

	while (1)
	{
		UpdateHardware();

//		switch(i)
//		{
//			case 0:
//
				//HW::GPIO->MASK0 = ~(0xF<<17);
				//HW::GPIO->MPIN0 = adr<<17;
//
//				i++;
//
//				break;
//
//			case 1:
//
////				HW::SCT->CTRL_L = (1<<3);
//
//				i++;
//
//				break;
//
//			case 2:
//
//				if (tm.Check(3))
//				{
//					adr += 1;

//					if (adr > 12) adr = 0;
//
//					i = 0;
//				};
//
//				break;
//		};

		HW::GPIO->NOT0 = 1<<12;

	}; // while (1)
}
