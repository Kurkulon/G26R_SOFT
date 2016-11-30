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

//static bool ready1 = false, ready2 = false;

//static u32 CRCOK = 0;
//static u32 CRCER = 0;

//static byte sampleTime[3] = { 19, 19, 9};
//static byte gain[3] = { 7, 7, 7 };
//static byte netAdr = 1;

static u16 manReqWord = 0xA900;
static u16 manReqMask = 0xFF00;

static u16 numDevice = 0;
static u16 verDevice = 1;

static u32 manCounter = 0;



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

static bool RequestMan_00(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[3];

	if (wb == 0) return false;

	rsp[0] = manReqWord;
	rsp[1] = numDevice;
	rsp[2] = verDevice;

	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_10(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	//1.1 Запрос параметров
	//	0xA910
	//	------

	static u16 rsp[6];

	if (wb == 0) return false;

	rsp[0] = manReqWord|0x10;	// 	1. ответное слово
	rsp[1] = 1;				 	//	2. КУ
	rsp[2] = 2;		 			//	3. Шаг оцифровки
	rsp[3] = 512;			 	//	4. Длина оцифровки
	rsp[4] = 0;					//	5. Задержка оцифровки
	rsp[5] = 0;					//	6. Фильтр

	wb->data = rsp;			 
	wb->len = sizeof(rsp);	 

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_20(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[8];

	if (wb == 0) return false;

	rsp[0] = manReqWord|0x20;			//	1. ответное слово	
	rsp[1] = GD(&manCounter, u16, 0);	//	2. счётчик. младшие 2 байта
	rsp[2] = GD(&manCounter, u16, 1);	//	3. счётчик. старшие 2 байта
	rsp[3] = 4;							//	4. статус
	rsp[4] = 5;							//	5. AX
	rsp[5] = 6;							//	6. AY
	rsp[6] = 7;							//	7. AZ
	rsp[7] = 8;							//	8. AT
 
	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_30(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[6+512];

	if (wb == 0) return false;

	rsp[0] = data[0];			// 	1. ответное слово
	rsp[1] = 1;				 	//	2. КУ
	rsp[2] = 2;		 			//	3. Шаг оцифровки
	rsp[3] = 512;			 	//	4. Длина оцифровки
	rsp[4] = 0;					//	5. Задержка оцифровки
	rsp[5] = 0;					//	6. Фильтр
	rsp[6] = 0;					//	7-?. данные (до 2048шт)

	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_80(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[1];

	if (wb == 0 || len < 3) return false;

	switch (data[1])
	{
		case 1:

			numDevice = data[2];

			break;
	};

	rsp[0] = manReqWord|0x80;
 
	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_90(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[1];

	if (wb == 0 || len < 3) return false;

	rsp[0] = manReqWord|0x90;
 
	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_F0(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static u16 rsp[1];

	if (wb == 0) return false;

//	SaveParams();

	rsp[0] = manReqWord|0xF0;
 
	wb->data = rsp;
	wb->len = sizeof(rsp);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan(ComPort::WriteBuffer *wb, ComPort::ReadBuffer *rb)
{
	u16 *p = (u16*)rb->data;
	bool r = false;

	u16 t = p[0];

	if ((t & manReqMask) != manReqWord || rb->len < 2)
	{
//		bfERC++; 
		return false;
	};

	u16 len = (rb->len)>>1;

	t = (t>>4) & 0xF;

	switch (t)
	{
		case 0: 	r = RequestMan_00(p, len, wb); break;
		case 1: 	r = RequestMan_10(p, len, wb); break;
		case 2: 	r = RequestMan_20(p, len, wb); break;
		case 3: 	r = RequestMan_30(p, len, wb); break;
		case 8: 	r = RequestMan_80(p, len, wb); break;
		case 9:		r = RequestMan_90(p, len, wb); break;
		case 0xF:	r = RequestMan_F0(p, len, wb); break;
		
//		default:	bfURC++; 
	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateBlackFin()
{
	static byte i = 0;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer rb;
	static byte buf[1024];

	switch(i)
	{
		case 0:

			rb.data = buf;
			rb.maxLen = sizeof(buf);
			com.Read(&rb, (u32)-1, 10000);
			i++;

			break;

		case 1:

			if (!com.Update())
			{
				if (rb.recieved && rb.len > 0)
				{
					if (RequestMan(&wb, &rb))
					{
						com.Write(&wb);
						i++;
					}
					else
					{
						i = 0;
					};
				}
				else
				{
					i = 0;
				};
			};

			break;

		case 2:

			if (!com.Update())
			{
				i = 0;
			};

			break;
	};
}

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

	com.Connect(1000000, 2);

//	InitNetAdress();

	while (1)
	{
		*pPORTFIO_TOGGLE = 1<<8;

		UpdateFire();

		UpdateBlackFin();

		*pPORTFIO_TOGGLE = 1<<8;

	};

//	return 0;
}
