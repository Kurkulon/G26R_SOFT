#include "hardware.h"
#include "ComPort.h"
#include "CRC16.h"


static ComPort com;
//static ComPort::WriteBuffer wb;
//
//static byte data[256*48];

static u16 spd[4000];

//static u16 spd2[512*2];
//
//static i16 ch1[512];
//static i16 ch2[512];
//static i16 ch3[512];
//static i16 ch4[512];

static bool ready1 = false, ready2 = false;

//static u32 CRCOK = 0;
//static u32 CRCER = 0;

static byte sampleTime[3] = { 19, 19, 9};
static byte gain[3] = { 7, 7, 7 };
static byte netAdr = 1;


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void UpdateBlackFin()
//{
//	static byte i = 0;
//	static ComPort::WriteBuffer wb;
//	static ComPort::ReadBuffer rb;
//	static byte buf[1024];
//
//	switch(i)
//	{
//		case 0:
//
//			rb.data = buf;
//			rb.maxLen = sizeof(buf);
//			com.Read(&rb, (u32)-1, 720);
//			i++;
//
//			break;
//
//		case 1:
//
//			if (!com.Update())
//			{
//				if (rb.recieved && rb.len > 0)
//				{
//					if (RequestFunc(&rb, &wb))
//					{
//						com.Write(&wb);
//						i++;
//					}
//					else
//					{
//						i = 0;
//					};
//				}
//				else
//				{
//					i = 0;
//				};
//			};
//
//			break;
//
//		case 2:
//
//			if (!com.Update())
//			{
//				i = 0;
//			};
//
//			break;
//	};
//}
//
////+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//static void InitNetAdress()
//{
//	U32u f;
//	
//	f.w[1] = ReadADC();
//
//	u32 t = GetRTT();
//
//	while ((GetRTT()-t) < 1000000)
//	{
//		f.d += (i32)ReadADC() - (i32)f.w[1];
//	};
//
//	netAdr = (f.w[1] / 398) + 1;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main( void )
{
	static byte s = 0;
	static byte i = 0;

	static u32 pt = 0;

	static RTM32 tm;

	InitHardware();

//	com.Connect(6250000, 0);

//	InitNetAdress();

	WriteTWI(spd, 2);

	ReadPPI(spd, sizeof(spd), 5, &ready1);

	while (1)
	{
		*pPORTFIO_TOGGLE = 1<<5;

		if ((*pTWI_MASTER_CTL & MEN) == 0)
		{
			WriteTWI(spd, 8);
		};

		switch (i)
		{
			case 0:

				if (ready1)
				{
					ReadPPI(spd, sizeof(spd), 5, &ready1);
					tm.Reset();
					i++;
				};

				break;

			case 1:

				if (tm.Check(MS2CLK(20)))
				{
					i = 0;
				};

				break;
		};

//		UpdateBlackFin();

		*pPORTFIO_TOGGLE = 1<<5;

	};

//	return 0;
}
