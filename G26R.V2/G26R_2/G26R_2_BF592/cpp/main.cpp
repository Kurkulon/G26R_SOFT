#include "hardware.h"
#include "ComPort.h"
#include "list.h"
#include "CRC16_CCIT.h"
//#include "CRC16.h"
//#include "at25df021.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

const byte build_date[] = "\n" "G26R1BF592" "\n" __DATE__ "\n" __TIME__ "\n";

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static ComPort com;
//static ComPort::WriteBuffer wb;
//
//static byte data[256*48];

//static u16 spd[1024*13];

//static byte twiBuffer[14] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE};

//struct Rsp30 { Rsp30 *next; struct Hdr { u16 rw; u16 fnum; u16 gain; u16 st; u16 sl; u16 sd; u16 flt; } h; u16 data[1024+32]; };
struct Rsp30 { Rsp30 *next; struct Hdr { u16 rw; u16 gain; u16 st; u16 sl; u16 sd; u16 flt; } h; u16 data[1024+32]; };

#pragma instantiate List<Rsp30>
static List<Rsp30> freeRsp30;
static List<Rsp30> rawRsp30;
static List<Rsp30> readyRsp30;

static Rsp30 rsp30[14];

static Rsp30 *curRsp30 = 0;

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

static Cmd cmdreq = {0, 0, 52, 255, 25*2000, 0, 0, 0};
static Cmd cmdrsp;

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

static const u16 manReqWord = 0xA900;
static const u16 manReqMask = 0xFF00;

static u16 numDevice = 1;
static u16 verDevice = 0x101;

static u16 manCounter = 0;

static bool startFire = false;

static u16 sampleDelay = 600;//800;
static u16 sampleTime = 8;
static u16 sampleLen = 512;
static u16 gain = 0;
static u16 filtr = 0;
static u16 firePeriod = 0;
static u32 period = MS2CCLK(100);



//static void SaveParams();


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SetFirePeriod(u16 p)
{
	if (firePeriod != p)
	{
		firePeriod = p;
		period = MS2CCLK(p);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitRspList()
{
	for (u16 i = 0; i < ArraySize(rsp30); i++)
	{
		freeRsp30.Add(&rsp30[i]);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateFire()
{
	static byte i = 0;
	static volatile bool ready = false;
	static CTM32 tm;
	static CTM32 tm2;

	static byte count = 0;
	static byte fnum = 0;


	static Rsp30 *rsp = 0;

	switch (i)
	{
		case 0:

			cmdreq.chnl = fnum;

			WriteTWI(&cmdreq, sizeof(cmdreq));

			count = 10;

			i++;

			break;

		case 1:

			if ((*pTWI_MASTER_CTL & MEN) == 0)
			{
				ReadTWI(&cmdrsp, sizeof(cmdrsp));

				i++;
			};

			break;

		case 2:

			if ((*pTWI_MASTER_CTL & MEN) == 0)
			{
				if ((count--) == 0)
				{
					i = 1;
				}
				else
				{
					i = (cmdrsp.busy || !cmdrsp.ready) ? (i-1) : (i+1);
				};

				tm.Reset();
			};

			break;

		case 3:

			if (tm.Check(US2CCLK(50)))
			{
				i++;
			};

			break;

		case 4:

			rsp = freeRsp30.Get();

			if (rsp != 0)
			{
				rsp->h.gain = gain;

				SetGain(gain);

				tm.Reset();

				i++;
			};

			break;

		case 5:
		{
			i32 d = (i32)sampleDelay - (i32)sampleTime*2;

			if (d < 0) d = 0;

			rsp->h.rw = manReqWord|0x30|fnum;
			rsp->h.st = sampleTime;
			rsp->h.sl = sampleLen;
			rsp->h.sd = sampleDelay;
			rsp->h.flt = 0;

			ReadPPI(rsp->data, sampleLen + 8, sampleTime, d, &ready);

			i++;

			break;

		};

		case 6:

			if (ready)
			{
				rawRsp30.Add(rsp);
				//freeRsp30.Add(rsp);

				rsp = 0;

				tm.Reset();

				i++;
			};

			break;

		case 7:

			if (tm.Check(US2CCLK(50)))
			{
				if (fnum < 12)
				{
					fnum += 1;
					i = 4;
				}
				else
				{
					fnum = 0;
					i++;
				};
			};

			break;

		case 8:

			if (tm2.Check(period))
			{
				i = 0;
			};

			break;
	};

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ProcessData()
{
	static Rsp30 *rsp = 0;

	static byte i = 0;

	switch (i)
	{
		case 0:

			rsp = rawRsp30.Get();

			if (rsp != 0)
			{
				u16 *p = rsp->data;

				for (u16 j = rsp->h.sl; j > 0; j--)
				{
					*p = p[8] - 2048;
					p++;
				};

				i++;
			};

			break;

		case 1:

			rsp->data[rsp->h.sl] = GetCRC16_CCIT_refl(&rsp->h, sizeof(rsp->h) + rsp->h.sl*2); 

			readyRsp30.Add(rsp);

			i = 0;

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_01(u16 *data, u16 len, ComPort::WriteBuffer *wb)
{
	static RspDsp01 rsp;

	ReqDsp01 *req = (ReqDsp01*)data;

	gain		= req->gain;
	sampleDelay = req->sampleDelay;
	sampleLen	= req->sampleLen;
	sampleTime	= req->sampleTime;
	filtr		= req->flt;
	
	SetFirePeriod(req->firePeriod);

	if (wb == 0) return false;

	if (curRsp30 != 0)
	{
		freeRsp30.Add(curRsp30);

		curRsp30 = 0;
	};

	curRsp30 = readyRsp30.Get();

	if (curRsp30 == 0)
	{
		rsp.rw = data[0];
		rsp.counter = manCounter;
		rsp.crc = GetCRC16_CCIT_refl(&rsp, sizeof(rsp)-2);

		wb->data = &rsp;			 
		wb->len = sizeof(rsp);	 
	}
	else
	{
		wb->data = &curRsp30->h;			 
		wb->len = sizeof(curRsp30->h) + (curRsp30->h.sl+1)*2;	 
	};

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

	//manCounter += 1;

	u16 len = (rb->len)>>1;

	t &= 0xFF;

	switch (t)
	{
		case 1: 	r = RequestMan_01(p, len, wb); break;
		
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

	ResetWDT();

	switch(i)
	{
		case 0:

			rb.data = buf;
			rb.maxLen = sizeof(buf);
			com.Read(&rb, ~0, US2CCLK(100));
			i++;

			break;

		case 1:

			if (!com.Update())
			{
				if (rb.recieved && rb.len > 0 && GetCRC16_CCIT_refl(rb.data, rb.len) == 0)
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void SaveParams()
//{
//	u16 buf[12];
//
//	ERROR_CODE result;
//
//	buf[0] = buf[6] = sampleDelay;
//	buf[1] = buf[7] = sampleTime;
//	buf[2] = buf[8] = sampleLen;
//	buf[3] = buf[9] = gain;
//	buf[4] = buf[10] = numDevice;
//	buf[5] = buf[11] = GetCRC16(buf, 10);
//
//	result = EraseBlock(0x20000);
//	result = at25df021_Write((byte*)buf, 0x20000, sizeof(buf), false);
//
//	result = EraseBlock(0x21000);
//	result = at25df021_Write((byte*)buf, 0x21000, sizeof(buf), false);
//
//	result = EraseBlock(0x22000);
//	result = at25df021_Write((byte*)buf, 0x22000, sizeof(buf), false);
//
//	result = EraseBlock(0x23000);
//	result = at25df021_Write((byte*)buf, 0x23000, sizeof(buf), false);
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void LoadParams()
//{
//	u16 buf[12];
//
//	ERROR_CODE result;
//
//	u32 stAdr = 0x20000;
//
//
//	bool crc = false;
//
//	for (byte n = 0; n < 4; n++, stAdr += 0x1000)
//	{
//		result = at25df021_Read((byte*)buf, stAdr, sizeof(buf));
//
//		if (result == NO_ERR)
//		{
//			u16 *p = buf;
//
//			for (byte i = 0; i < 2; i++)
//			{
//				if (GetCRC16(p, 12) == 0)
//				{
//					sampleDelay = p[0];
//					sampleTime = p[1];
//					sampleLen = p[2];
//					gain = p[3];
//					numDevice = p[4];
//
//					crc = true;
//
//					break;
//				}
//				else
//				{
//					p += 6;
//				};
//			};
//		};
//
//		if (crc) break;
//	};
//
//	if(!crc)
//	{
//		sampleDelay = 600;
//		sampleTime = 8;
//		sampleLen = 512;
//		gain = 0;
//		numDevice = 1;
//
//		SaveParams();
//	};
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main( void )
{
	static byte s = 0;
	static byte i = 0;

	static u32 pt = 0;

	static RTM32 tm;

	InitHardware();

	InitRspList();

	com.Connect(6250000, 2);

//	InitNetAdress();

	while (1)
	{
		*pPORTFIO_TOGGLE = 1<<8;

		#define CALL(p) case (__LINE__-S): p; break;

		enum C { S = (__LINE__+3) };
		switch(i++)
		{
			CALL( UpdateFire()		);
			CALL( ProcessData()		);
			CALL( UpdateBlackFin()	);
		};

		i = (i > (__LINE__-S-3)) ? 0 : i;

		#undef CALL

		//if (tm.Check(MS2CLK(100)))
		//{
		//	startFire = true;
		//};

		*pPORTFIO_TOGGLE = 1<<8;

	};

//	return 0;
}
