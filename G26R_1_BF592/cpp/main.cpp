#include "hardware.h"
#include "ComPort.h"
#include "CRC16.h"


static ComPort com;
//static ComPort::WriteBuffer wb;
//
//static byte data[256*48];

static u16 spd[4000];

//static byte twiBuffer[14] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE};

struct Cmd
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

static Cmd req = {0, 0, 52, 255, 25*2000, 0, 0, 0};
static Cmd rsp;

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

static void UpdateFire()
{
	static byte i = 0;
	static bool ready = false;
	static RTM32 tm;


	switch (i)
	{
		case 0:

			WriteTWI(&req, sizeof(req));

			i++;

			break;

		case 1:

			if ((*pTWI_MASTER_CTL & MEN) == 0)
			{
				ReadTWI(&rsp, sizeof(rsp));

				i++;
			};

			break;

		case 2:

			if ((*pTWI_MASTER_CTL & MEN) == 0)
			{
				i = (rsp.busy || !rsp.ready) ? 1 : (i+1);
			};

			break;

		case 3:

			ReadPPI(spd, sizeof(spd), 5, &ready);

			i++;

			break;

		case 4:

			if (ready)
			{
				tm.Reset();

				i++;
			};

			break;

		case 5:

			if (tm.Check(MS2CLK(100)))
			{
				i = 0;
			};


	};

}

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

	while (1)
	{
		*pPORTFIO_TOGGLE = 1<<5;

		UpdateFire();

//		UpdateBlackFin();

		*pPORTFIO_TOGGLE = 1<<5;

	};

//	return 0;
}
