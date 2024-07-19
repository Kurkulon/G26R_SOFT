#include "hardware.h"
//#include "options.h"
//#include "hw_emac.h"
#include "xtrap.h"
#include "flash.h"
#include "CRC16.h"
#include "ComPort.h"
#include "CRC16_CCIT.h"
#include "req.h"
#include "list.h"
#include "PointerCRC.h"
#include "SEGGER_RTT.h"
#include "hw_conf.h"
#include "hw_com.h"
#include "at25df021.h"

#ifdef WIN32

#include <conio.h>
//#include <stdio.h>

//static const bool __WIN32__ = true;

#else

//static const bool __WIN32__ = false;

//#pragma diag_suppress 546,550,177

#endif

enum { VERSION = 0x205 };

//#pragma O3
//#pragma Otime

#ifndef _DEBUG
	static const bool __debug = false;
#else
	static const bool __debug = true;
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct MainVars // NonVolatileVars  
{
	u32 timeStamp;

	u16 numDevice;
	u16 numMemDevice;

	u16 gain;
	u16 sampleTime;
	u16 sampleLen;
	u16 sampleDelay;
	u16 firePeriod;
	u16 freq;

	u16 fireVoltage;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static MainVars mv;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 req40_count1 = 0;
u32 req40_count2 = 0;
u32 req40_count3 = 0;

u32 fps;
i16 tempClock = 0;
i16 cpu_temp = 0;
u32 i2cResetCount = 0;

//inline u16 ReverseWord(u16 v) { __asm	{ rev16 v, v };	return v; }

//static void* eepromData = 0;
//static u16 eepromWriteLen = 0;
//static u16 eepromReadLen = 0;
//static u16 eepromStartAdr = 0;

//static MTB mtb;

//static u16 manBuf[16];

u16 manRcvData[10];
u16 manTrmData[50];
static u16 manTrmBaud = 0;
static u16 memTrmBaud = 0;

u16 txbuf[128 + 512 + 16];


static RequestQuery qmoto(&commoto);
static RequestQuery qdsp(&comdsp);
//static RequestQuery qmem(&commem);

//static R01 r02[8];

static Ptr<MB> manVec30[13];

static Ptr<MB> curManVec30;

//static RspMan60 rspMan60;

//static byte curRcv[3] = {0};
//static byte curVec[3] = {0};

//static List<R01> freeR01;
//static List<R01> readyR01;
static ListPtr<REQ> readyR01;

//static RMEM rmem[4];
//static List<RMEM> lstRmem;
//static List<RMEM> freeRmem;

//static byte fireType = 0;

//static u16 gain = 0;
//static u16 sampleTime = 5;
//static u16 sampleLen = 1024;
//static u16 sampleDelay = 200;
//static u16 deadTime = 400;
//static u16 descriminant = 400;
//static u16 freq = 500;

//static u16 gainRef = 0;
//static u16 sampleTimeRef = 5;
//static u16 sampleLenRef = 1024;
//static u16 sampleDelayRef = 200;
//static u16 deadTimeRef = 400;
//static u16 descriminantRef = 400;
//static u16 refFreq = 500;
//static u16 filtrType = 0;
//static u16 packType = 0;
//static u16 vavesPerRoundCM = 100;	
//static u16 vavesPerRoundIM = 100;

//static u16 mode = 0;

//static TM32 imModeTimeout;

//static u16 motoEnable = 0;		// двигатель включить или выключить
//static u16 motoTargetRPS = 0;		// заданные обороты двигателя
//static u16 motoRPS = 0;				// обороты двигателя, об/сек
//static u16 motoCur = 0;				// ток двигателя, мА
//static u16 motoStat = 0;			// статус двигателя: 0 - выкл, 1 - вкл
//static u16 motoCounter = 0;			// счётчик оборотов двигателя 1/6 оборота
//static u16 cmSPR = 32;			// Количество волновых картин на оборот головки в режиме цементомера
//static u16 imSPR = 100;			// Количество точек на оборот головки в режиме имиджера
//static u16 *curSPR = &cmSPR;		// Количество импульсов излучателя на оборот в текущем режиме
//static u16 motoVoltage = 90;
static u16 motoRcvCount = 0;

static u16 curFireVoltage = 500;

//static u32 dspMMSEC = 0;
//static u32 shaftMMSEC = 0;

const u16 dspReqWord = 0xA900;
//const u16 dspReqMask = 0xFF00;

static u16 manReqWord = 0xA900;
static u16 manReqMask = 0xFF00;

static u16 memReqWord = 0x3E00;
static u16 memReqMask = 0xFF00;

//static u16 numDevice = 0;
static u16 verDevice = VERSION;

//static u16 numMemDevice = 0;
static u16 verMemDevice = 0x100;

//static u32 manCounter = 0;
//static u32 fireCounter = 0;

static byte mainModeState = 0;
static byte dspStatus = 0;

static bool cmdWriteStart_00 = false;
static bool cmdWriteStart_10 = false;
//static bool cmdWriteStart_20 = false;

//static u32 dspRcv40 = 0;
//static u32 dspRcv50 = 0;
static u16 dspRcvCount = 0;
static u16 dspRcv30 = 0;
static u16 dspRcv01 = 0;


//static u32 rcvCRCER = 0;

//static u32 chnlCount[4] = {0};

//static u32 crcErr02 = 0;
//static u32 crcErr03 = 0;
static u32 crcErr06 = 0;
static u32 wrtErr06 = 0;

static u32 notRcv02 = 0;
//static u32 lenErr02 = 0;

static i16 ax = 0, ay = 0, az = 0, at = 0;
i16 temperature = 0;
i16 cpuTemp = 0;
i16 temp = 0;

static byte svCount = 0;


//struct AmpTimeMinMax { bool valid; u16 ampMax, ampMin, timeMax, timeMin; };

//static AmpTimeMinMax sensMinMax[2] = { {0, 0, ~0, 0, ~0}, {0, 0, ~0, 0, ~0} };
//static AmpTimeMinMax sensMinMaxTemp[2] = { {0, 0, ~0, 0, ~0}, {0, 0, ~0, 0, ~0} };

static u32 testDspReqCount = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SaveMainParams()
{
	svCount = 1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void SetNumDevice(u16 num)
{
	mv.numDevice = num;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern u16 GetNumDevice()
{
	return mv.numDevice;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern u16 GetVersionDevice()
{
	return verDevice;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Update_RPS_SPR()
{
//	Set_Sync_Rot(motoTargetRPS, (mode == 0) ? mv.cmSPR : mv.imSPR);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void Response_0(u16 rw, MTB &mtb)
//{
//	__packed struct Rsp {u16 rw; u16 device; u16 session; u32 rcvVec; u32 rejVec; u32 wrVec; u32 errVec; u16 wrAdr[3]; u16 numDevice; u16 version; u16 temp; byte status; byte flags; RTC rtc; };
//
//	Rsp &rsp = *((Rsp*)&txbuf);
//
//	rsp.rw = rw;
//	rsp.device = GetDeviceID();  
//	rsp.session = FLASH_Session_Get();	  
//	rsp.rcvVec =  FLASH_Vectors_Recieved_Get();
//	rsp.rejVec = FLASH_Vectors_Rejected_Get();
//	rsp.wrVec = FLASH_Vectors_Saved_Get();
//	rsp.errVec = FLASH_Vectors_Errors_Get();
//	*((__packed u64*)rsp.wrAdr) = FLASH_Current_Adress_Get();
//	rsp.temp = temp*5/2;
//	rsp.status = FLASH_Status();
//
//	GetTime(&rsp.rtc);
//
//	mtb.data1 = txbuf;
//	mtb.len1 = sizeof(rsp)/2;
//	mtb.data2 = 0;
//	mtb.len2 = 0;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CallBackDspReq01(Ptr<REQ> &q)
{
	RspDsp01 &rsp = *((RspDsp01*)q->rb.data);
	 
	if (q->rb.recieved && q->crcOK)
	{
		if ((rsp.rw & ~15) == (dspReqWord|0x30))
		{
			q->rsp->len = q->rb.len;

			dspRcv30++;
			dspRcvCount++;
		}
		else if (rsp.rw == (dspReqWord|1))
		{
			q->rsp->len = 0;

			dspRcv01++;
			dspRcvCount++;
		};
	}
	else
	{
		q->crcOK = false;
		notRcv02++;
	};

	if (!q->crcOK)
	{
		if (q->tryCount > 0)
		{
			q->tryCount--;
			qdsp.Add(q);

			return false;
		}
		else
		{
			dspStatus &= ~1; 

			//q.Free();
		};
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Ptr<REQ> CreateDspReq01(bool repeat, u16 tryCount)
{
	Ptr<REQ> rq;
	
	rq.Alloc();//= REQ::Alloc();

	if (!rq.Valid()) return rq;

	rq->rsp = AllocFlashWriteBuffer(sizeof(RspDsp30::Hdr)+mv.sampleLen*2+2);

	if (!rq->rsp.Valid()) { rq.Free(); return rq; };

	ReqDsp01 &req = *((ReqDsp01*)rq->reqData);
	RspDsp30 &rsp = *((RspDsp30*)(rq->rsp->GetDataPtr()));

	rq->rsp->len = 0;
	
	REQ &q = *rq;

	q.CallBack = CallBackDspReq01;
	//q.rb = &rb;
	//q.wb = &wb;
	q.preTimeOut = MS2COM(1);
	q.postTimeOut = US2COM(100);
	q.ready = false;
	q.tryCount = tryCount;
	//q.ptr = &r;
	q.checkCRC = true;
	q.updateCRC = true;
	q.crcType = q.CRC16_CCIT;
	
	q.wb.data = &req;
	q.wb.len = sizeof(req);

	q.rb.data = &rsp;
	q.rb.maxLen = sizeof(rsp);
	q.rb.recieved = false;
	
	req.rw				= dspReqWord|1;
	req.gain			= mv.gain;
	req.sampleTime		= mv.sampleTime; 
	req.sampleLen		= mv.sampleLen; 
	req.sampleDelay 	= mv.sampleDelay; 
	req.firePeriod		= mv.firePeriod; //ms
	req.verDevice		= verDevice;
	req.repeatResponse	= (repeat) ? ~0 : 0;

	return rq;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Ptr<MB> CreateTestDspReq01()
{
	Ptr<MB> rq(AllocFlashWriteBuffer(sizeof(RspDsp30::h) + 1024*2 + 2));
	
	if (!rq.Valid()) { return rq; };

	RspDsp30 &rsp = *((RspDsp30*)(rq->GetDataPtr()));

	rsp.h.rw		= manReqWord|0x30;
	rsp.h.verDevice = verDevice;

	rsp.h.gain		= 0;
	rsp.h.st		= 1;
	rsp.h.sl		= 1024;
	rsp.h.sd		= 0;

	for (u32 i = 0; i < rsp.h.sl; i++) rsp.data[i] = i;

	rq->len = sizeof(rsp.h) + rsp.h.sl*2;
	
	return rq;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool CallBackDspReq05(Ptr<REQ> &q)
{
	if (!q->crcOK) 
	{
		if (q->tryCount > 0)
		{
			q->tryCount--;
			qdsp.Add(q);

			return false;
		};
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Ptr<REQ> CreateDspReq05(u16 tryCount)
{
	Ptr<REQ> rq;
	
	rq.Alloc();//= REQ::Alloc();

	if (!rq.Valid()) return rq;

	rq->rsp = AllocMemBuffer(sizeof(RspDsp05));

	if (!rq->rsp.Valid()) { rq.Free(); return rq; };

	ReqDsp05 &req = *((ReqDsp05*)rq->reqData);
	RspDsp05 &rsp = *((RspDsp05*)(rq->rsp->GetDataPtr()));
	
	REQ &q = *rq;

	q.CallBack = CallBackDspReq05;
	q.preTimeOut = MS2COM(10);
	q.postTimeOut = US2COM(100);
	//q.rb = &rb;
	//q.wb = &wb;
	q.ready = false;
	q.tryCount = tryCount;
	q.checkCRC = true;
	q.updateCRC = false;
	q.crcType = q.CRC16;
	
	q.wb.data = &req;
	q.wb.len = sizeof(req);
	
	q.rb.data = &rsp;
	q.rb.maxLen = sizeof(rsp);

	req.rw = dspReqWord|5;
	req.crc	= GetCRC16(&req, sizeof(req)-2);

	return rq;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CallBackDspReq06(Ptr<REQ> &q)
{
	bool retry = false;

	if (!q->crcOK) 
	{
		crcErr06++;

		retry = true;
	}
	else
	{
		RspDsp06 &rsp = *((RspDsp06*)q->rb.data);

		if (rsp.res != 0) wrtErr06++, retry = true;
	};

	if (retry && q->tryCount > 0)
	{
		q->tryCount--;
		qdsp.Add(q);

		return false;
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Ptr<REQ> CreateDspReq06(u16 stAdr, u16 count, void* data, u16 count2, void* data2, u16 tryCount)
{
	Ptr<REQ> rq;
	
	rq.Alloc();//= REQ::Alloc();

	if (!rq.Valid()) return rq;

	rq->rsp = AllocMemBuffer(sizeof(RspDsp06));

	if (!rq->rsp.Valid()) { rq.Free(); return rq; };

	ReqDsp06 &req = *((ReqDsp06*)rq->reqData);
	RspDsp06 &rsp = *((RspDsp06*)(rq->rsp->GetDataPtr()));
	
	REQ &q = *rq;

	q.CallBack = CallBackDspReq06;
	q.preTimeOut = MS2COM(500);
	q.postTimeOut = US2COM(100);
	//q.rb = &rb;
	//q.wb = &wb;
	q.ready = false;
	q.tryCount = tryCount;
	q.checkCRC = true;
	q.updateCRC = false;
	q.crcType = q.CRC16;
	
	q.rb.data = &rsp;
	q.rb.maxLen = sizeof(rsp);

	req.rw = dspReqWord|6;

	u16 max = sizeof(req.data)-2;

	if (count > max)
	{
		count = max;
		count2 = 0;
	}
	else if ((count + count2) > max)
	{
		count2 = max - count;
	};

	req.stAdr = stAdr;
	req.count = count+count2;

	byte *d = req.data;
	byte *s = (byte*)data;

	while(count > 0)
	{
		*d++ = *s++;
		count--;
	};

	if (data2 != 0)
	{
		s = (byte*)data2;

		while(count2 > 0)
		{
			*d++ = *s++;
			count2--;
		};
	};

	u16 len = sizeof(req) - 2 - sizeof(req.data) + req.count;

	u16 crc = GetCRC16(&req, len);

	d[0] = crc;
	d[1] = crc>>8;

	q.wb.data = &req;
	q.wb.len = len+2;

	return rq;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Ptr<REQ> CreateDspReq07()
{
	Ptr<REQ> rq;
	
	rq.Alloc();//= REQ::Alloc();

	if (!rq.Valid()) return rq;

	//rq->rsp.Alloc();

	//if (!rq->rsp.Valid()) { rq.Free(); return rq; };

	ReqDsp07 &req = *((ReqDsp07*)rq->reqData);
	//RspDsp06 &rsp = *((RspDsp06*)(rq->rsp->GetDataPtr()));
	
	REQ &q = *rq;

	q.CallBack = 0;
	//q.preTimeOut = US2RT(500);
	//q.postTimeOut = US2RT(100);
	//q.rb = 0;
	//q.wb = &wb;
	q.ready = false;
	q.tryCount = 0;
	q.checkCRC = false;
	q.updateCRC = false;
	q.crcType = q.CRC16;
	
	q.wb.data = &req;
	q.wb.len = sizeof(req);

	q.rb.data = 0;
	q.rb.maxLen = 0;
	
	//rb.data = &rsp;
	//rb.maxLen = sizeof(rsp);

	req.rw	= dspReqWord|7;

	req.crc = GetCRC16(&req, sizeof(ReqDsp07)-2);

	return rq;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CallBackMotoReq(Ptr<REQ> &q)
{
	if (!q->crcOK) 
	{
		if (q->tryCount > 0)
		{
			q->tryCount--;
			qmoto.Add(q);

			return false;
		};
	}
	else
	{
		RspMoto &rsp = *((RspMoto*)q->rb.data);

		if (rsp.rw == 0x101)
		{
			curFireVoltage = rsp.curHV;

			motoRcvCount++;
		};
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Ptr<REQ> CreateMotoReq()
{
	Ptr<REQ> rq;
	
	rq.Alloc();

	if (!rq.Valid()) return rq;

	rq->rsp = AllocMemBuffer(sizeof(RspMoto));

	if (!rq->rsp.Valid()) { rq.Free(); return rq; };

	ReqMoto &req = *((ReqMoto*)rq->reqData);
	RspMoto &rsp = *((RspMoto*)(rq->rsp->GetDataPtr()));
	
	REQ &q = *rq;

	q.CallBack = CallBackMotoReq;
	//q.rb = &rb;
	//q.wb = &wb;
	q.preTimeOut = MS2COM(1);
	q.postTimeOut = US2COM(100);
	q.ready = false;
	q.checkCRC = true;
	q.updateCRC = true;
	q.crcType = q.CRC16;
	q.tryCount = 1;
	
	q.wb.data = &req;
	q.wb.len = sizeof(req) - sizeof(req.crc);

	q.rb.data = &rsp;
	q.rb.maxLen = sizeof(rsp);
	
	req.rw = 0x101;
	req.reqHV = mv.fireVoltage;
	req.freq = mv.freq;
	req.sampleTime = (mv.sampleLen * mv.sampleTime + mv.sampleDelay) / 20;

	return &q;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CallBackBootMotoReq01(Ptr<REQ> &q)
{
	if (!q->crcOK) 
	{
		if (q->tryCount > 0)
		{
			q->tryCount--;
			qmoto.Add(q);

			return false;
		};
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Ptr<REQ> CreateBootMotoReq01(u16 flashLen, u16 tryCount)
{
	Ptr<REQ> rq;
	
	rq.Alloc();

	if (!rq.Valid()) return rq;

	rq->rsp = AllocMemBuffer(sizeof(RspBootMoto));

	if (!rq->rsp.Valid()) { rq.Free(); return rq; };

	ReqBootMoto &req = *((ReqBootMoto*)rq->reqData);
	RspBootMoto &rsp = *((RspBootMoto*)(rq->rsp->GetDataPtr()));
	
	REQ &q = *rq;

	q.CallBack = CallBackBootMotoReq01;
	q.preTimeOut = MS2COM(10);
	q.postTimeOut = US2COM(100);
	//q.rb = &rb;
	//q.wb = &wb;
	q.ready = false;
	q.tryCount = tryCount;
	q.checkCRC = true;
	q.updateCRC = false;
	q.crcType = q.CRC16;
	
	q.wb.data = &req;
	q.wb.len = sizeof(req.F01) + sizeof(req.func);
	
	q.rb.data = &rsp;
	q.rb.maxLen = sizeof(rsp);

	req.func = 1;
	req.F01.flashLen = flashLen;
	req.F01.align = ~flashLen;

	req.F01.crc	= GetCRC16(&req, q.wb.len - sizeof(req.F01.crc));

	return rq;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CallBackBootMotoReq02(Ptr<REQ> &q)
{
	if (!q->crcOK) 
	{
		if (q->tryCount > 0)
		{
			q->tryCount--;
			qmoto.Add(q);

			return false;
		};
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Ptr<REQ> CreateBootMotoReq02(u16 stAdr, u16 count, const u32* data, u16 tryCount)
{
	Ptr<REQ> rq;
	
	rq.Alloc();

	if (!rq.Valid()) return rq;

	rq->rsp = AllocMemBuffer(sizeof(RspBootMoto));

	if (!rq->rsp.Valid()) { rq.Free(); return rq; };

	ReqBootMoto &req = *((ReqBootMoto*)rq->reqData);
	RspBootMoto &rsp = *((RspBootMoto*)(rq->rsp->GetDataPtr()));
	
	REQ &q = *rq;

	q.CallBack = CallBackBootMotoReq02;
	q.preTimeOut = MS2COM(300);
	q.postTimeOut = US2COM(100);
	//q.rb = &rb;
	//q.wb = &wb;
	q.ready = false;
	q.tryCount = tryCount;
	q.checkCRC = true;
	q.updateCRC = false;
	q.crcType = q.CRC16;
	
	q.rb.data = &rsp;
	q.rb.maxLen = sizeof(rsp);

	req.func = 2;

	u16 max = ArraySize(req.F02.page);

	if (count > max)
	{
		count = max;
	};

	u32 count2 = max - count;

	req.F02.padr = stAdr;

	u32 *d = req.F02.page;

	while(count > 0)
	{
		*d++ = *data++;
		count--;
	};

	if (count2 > 0)
	{
		*d++ = ~0;
		count2--;
	};

	u16 len = sizeof(req.F02) + sizeof(req.func) - sizeof(req.F02.crc);

	req.F02.align = 0xAAAA;
	req.F02.crc = GetCRC16(&req, len);

	q.wb.data = &req;
	q.wb.len = len+sizeof(req.F02.crc);

	return rq;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Ptr<REQ> CreateBootMotoReq03()
{
	Ptr<REQ> rq;
	
	rq.Alloc();

	if (!rq.Valid()) return rq;

	rq->rsp = AllocMemBuffer(sizeof(RspBootMoto));

	if (!rq->rsp.Valid()) { rq.Free(); return rq; };

	ReqBootMoto &req = *((ReqBootMoto*)rq->reqData);
	RspBootMoto &rsp = *((RspBootMoto*)(rq->rsp->GetDataPtr()));
	
	REQ &q = *rq;

	q.CallBack = 0;
	q.preTimeOut = MS2COM(10);
	q.postTimeOut = US2COM(100);
	//q.rb = &rb;
	//q.wb = &wb;
	q.ready = false;
	q.tryCount = 1;
	q.checkCRC = true;
	q.updateCRC = false;
	q.crcType = q.CRC16;
	
	q.rb.data = &rsp;
	q.rb.maxLen = sizeof(rsp);
	
	req.func = 3;
	req.F03.align += 1; 

	u16 len = sizeof(req.F03) + sizeof(req.func) - sizeof(req.F03.crc);

	req.F03.crc = GetCRC16(&req, len);

	q.wb.data = &req;
	q.wb.len = len + sizeof(req.F03.crc);

	return rq;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void InitRmemList()
//{
//	for (u16 i = 0; i < ArraySize(r02); i++)
//	{
//		freeR01.Add(&r02[i]);
//	};
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u32 InitRspMan_00(__packed u16 *data)
{
	__packed u16 *start = data;

	*(data++)	= (manReqWord & manReqMask) | 0;
	*(data++)	= mv.numDevice;
	*(data++)	= verDevice;
	
	return data - start;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFlashWrite_00()
{
	Ptr<MB> mb; 

	mb = AllocFlashWriteBuffer(6);

	if (mb.Valid())
	{
		__packed u16* data = (__packed u16*)mb->GetDataPtr();

		mb->len = InitRspMan_00(data) * 2;

		return RequestFlashWrite(mb, data[0], true);
	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_00(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || len > 2 || mtb == 0) return false;

	InitRspMan_00(manTrmData);

	mtb->data1 = manTrmData;
	mtb->len1 = 3;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u32 InitRspMan_10(__packed u16 *data)
{
	__packed u16 *start = data;

	*(data++)	= (manReqWord & manReqMask) | 0x10;		//	1. ответное слово
	*(data++)	= mv.gain;								//	2. КУ
	*(data++)	= mv.sampleTime;						//	3. Шаг оцифровки
	*(data++)	= mv.sampleLen;							//	4. Длина оцифровки
	*(data++)	= mv.sampleDelay; 						//	5. Задержка оцифровки
	*(data++)	= mv.freq;								//	6. Частота излучателя (кГц) 100...1000
	*(data++)	= mv.fireVoltage;						//	7. Напряжение излучателя
	*(data++)	= mv.firePeriod;						//	8. Период опроса(мс)(ushort)

	return data - start;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFlashWrite_10()
{
	Ptr<MB> mb; 

	mb = AllocFlashWriteBuffer(16);

	if (mb.Valid())
	{
		__packed u16* data = (__packed u16*)mb->GetDataPtr();

		mb->len = InitRspMan_10(data) * 2;

		return RequestFlashWrite(mb, data[0], true);
	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_10(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || len > 2 || mtb == 0) return false;

	len = InitRspMan_10(manTrmData);

	mtb->data1 = manTrmData;
	mtb->len1 = len;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u32 InitRspMan_20(__packed u16 *data)
{
	__packed u16 *start = data;

	*(data++)	= manReqWord|0x20;				//	1. ответное слово
	*(data++)	= verDevice;					//	2. Версия ПО
	*(data++)	= ax;							//	3. AX (уе)
	*(data++)	= ay;							//	4. AY (уе)
	*(data++)  	= az;							//	5. AZ (уе)
	*(data++)  	= at;							//	6. AT (short 0.01 гр)
	*(data++)  	= temp;							//	7. Температура в приборе (short)(0.1гр)
	*(data++)  	= curFireVoltage;							//	8. Напряжение излучателя (В)
	*(data++)	= dspRcvCount; 					//	9. Счётчик запросов DSP
	*(data++)	= motoRcvCount;					//	10. Счётчик запросов излучателя
	*(data++)  	= GetRcvManQuality();			//	11. Качество сигнала запроса телеметрии (%)	

	return data - start;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestFlashWrite_20()
{
	Ptr<MB> mb; 

	mb = AllocFlashWriteBuffer(16);

	if (mb.Valid())
	{
		__packed u16* data = (__packed u16*)mb->GetDataPtr();

		mb->len = InitRspMan_20(data) * 2;

		return RequestFlashWrite(mb, data[0], true);
	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_20(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || len > 2 || mtb == 0) return false;

	len = InitRspMan_20(manTrmData);

	mtb->data1 = manTrmData;
	mtb->len1 = len;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_30(u16 *data, u16 reqlen, MTB* mtb)
{
	__packed struct Req { u16 rw; u16 off; u16 len; };

	Req &req = *((Req*)data);

	if (data == 0 || reqlen == 0 || reqlen > 4 || mtb == 0) return false;

	//byte nf = ((req.rw>>4)-3)&3;
	//byte nr = req.rw & 7;

//	curRcv[nf] = nr;

	struct Rsp { u16 rw; };
	
	static Rsp rsp; 
	
	static u16 prevOff = 0;
	static u16 prevLen = 0;
	static u16 maxLen = 200;

	byte sensInd = req.rw & 15;

	rsp.rw = req.rw;

	mtb->data1 = (u16*)&rsp;
	mtb->len1 = sizeof(rsp)/2;
	mtb->data2 = 0;
	mtb->len2 = 0;

	//Ptr<MB> &r01 = curManVec40;
	
	//u16 sz = 18 + r01->rsp.CM.sl;

	if (reqlen == 1 || (reqlen >= 2 && data[1] == 0))
	{
		curManVec30 = manVec30[sensInd];

		if (curManVec30.Valid())
		{
			RspDsp30 &rsp = *((RspDsp30*)(curManVec30->GetDataPtr()));

			u16 sz = sizeof(rsp.h)/2 + rsp.h.sl - 1;

			mtb->data2 = ((u16*)&rsp)+1;

			prevOff = 0;

			if (reqlen == 1)
			{
				mtb->len2 = sz;
				prevLen = sz;
			}
			else 
			{
				req40_count1++;

				if (reqlen == 3) maxLen = data[2];

				u16 len = maxLen;

				if (len > sz) len = sz;

				mtb->len2 = len;

				prevLen = len;
			};
		};

		//sensInd = (sensInd + 1) & 1;
	}
	else if (curManVec30.Valid())
	{
		RspDsp30 &rsp = *((RspDsp30*)(curManVec30->GetDataPtr()));

		req40_count2++;

		u16 off = prevOff + prevLen;
		u16 len = prevLen;

		if (reqlen == 3)
		{
			off = data[1];
			len = data[2];
		};

		u16 sz = sizeof(rsp.h)/2 + rsp.h.sl - 1;

		if (sz >= off)
		{
			req40_count3++;

			u16 ml = sz - off;

			if (len > ml) len = ml;

			mtb->data2 = (u16*)&rsp + data[1]+1;
			mtb->len2 = len;
		};
	};

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_80(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len < 3 || len > 4 || mtb == 0) return false;

	switch (data[1])
	{
		case 1:

			mv.numDevice = data[2];

			break;

		case 2:

			manTrmBaud = data[2] - 1;	//SetTrmBoudRate(data[2]-1);

			break;
	};

	manTrmData[0] = (manReqWord & manReqMask) | 0x80;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_90(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len < 3 || len > 4 || mtb == 0) return false;

	switch(data[1])
	{
		case 0x1:	mv.gain				= MIN(data[2], 7);			break;
		case 0x2:	mv.sampleTime		= LIM(data[2], 1, 1*20);	break;
		case 0x3:	mv.sampleLen		= LIM(data[2], 16, 1024);	break;
		case 0x4:	mv.sampleDelay 		= MIN(data[2], 500*20);		break;
		case 0x5:	mv.freq				= LIM(data[2], 100, 1000);	break;
		case 0x6:	mv.fireVoltage		= MIN(data[2], 500);		break;
		case 0x7:	mv.firePeriod		= LIM(data[2], 50, 1000);	break;

		default:

			return false;
	};

	manTrmData[0] = (manReqWord & manReqMask) | 0x90;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan_F0(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || len > 2 || mtb == 0) return false;

	SaveMainParams();

	manTrmData[0] = (manReqWord & manReqMask) | 0xF0;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	bool r = false;

	byte i = (buf[0]>>4)&0xF;

	switch (i)
	{
		case 0: 	r = RequestMan_00(buf, len, mtb); break;
		case 1: 	r = RequestMan_10(buf, len, mtb); break;
		case 2: 	r = RequestMan_20(buf, len, mtb); break;
		case 3:		r = RequestMan_30(buf, len, mtb); break;
		case 8: 	r = RequestMan_80(buf, len, mtb); break;
		case 9:		r = RequestMan_90(buf, len, mtb); break;
		case 0xF:	r = RequestMan_F0(buf, len, mtb); break;
	};

	if (r) { mtb->baud = manTrmBaud; };

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_00(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (memReqWord & memReqMask) | 0;
	manTrmData[1] = mv.numMemDevice;
	manTrmData[2] = verMemDevice;

	mtb->data1 = manTrmData;
	mtb->len1 = 3;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_10(u16 *data, u16 len, MTB* mtb)
{
	//__packed struct T { u16 g[8]; u16 st; u16 len; u16 delay; u16 voltage; };
	//__packed struct Rsp { u16 hdr; u16 rw; T t1, t2, t3; };
	
	if (data == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (memReqWord & memReqMask) | 0x10;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_20(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || len > 2 || mtb == 0) return false;

	__packed struct Rsp {u16 rw; u16 device; u16 session; u32 rcvVec; u32 rejVec; u32 wrVec; u32 errVec; u16 wrAdr[3]; u16 temp; byte status; byte flags; RTC rtc; };

	if (len != 1) return false;

	Rsp &rsp = *((Rsp*)&manTrmData);

	rsp.rw = (memReqWord & memReqMask)|0x20;
	rsp.device = GetDeviceID();  
	rsp.session = FLASH_Session_Get();	  
	rsp.rcvVec =  FLASH_Vectors_Recieved_Get();
	rsp.rejVec = FLASH_Vectors_Rejected_Get();
	rsp.wrVec = FLASH_Vectors_Saved_Get();
	rsp.errVec = FLASH_Vectors_Errors_Get();
	*((__packed u64*)rsp.wrAdr) = FLASH_Current_Adress_Get();
	rsp.temp = (temp+5)/10;
	rsp.status = FLASH_Status();

	GetTime(&rsp.rtc);

	mtb->data1 = manTrmData;
	mtb->len1 = 20;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_30(u16 *data, u16 len, MTB* mtb)
{
	if (len != 5) return false;

	SetClock(*(RTC*)(&data[1]));

	manTrmData[0] = (memReqWord & memReqMask)|0x30;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_31(u16 *data, u16 len, MTB* mtb)
{
	if (len != 1) return false;

	cmdWriteStart_00 = cmdWriteStart_10 = FLASH_WriteEnable();

	manTrmData[0] = (memReqWord & memReqMask)|0x31;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_32(u16 *data, u16 len, MTB* mtb)
{
	if (len != 1) return false;

	FLASH_WriteDisable();

	manTrmData[0] = (memReqWord & memReqMask)|0x32;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_33(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || len > 4 || mtb == 0) return false;

	// Erase all

	manTrmData[0] = (memReqWord & memReqMask)|0x33;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_80(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len < 3 || len > 4 || mtb == 0) return false;

	switch (data[1])
	{
		case 1:

			mv.numMemDevice = data[2];

			break;

		case 2:

			memTrmBaud = data[2] - 1;	//SetTrmBoudRate(data[2]-1);

			break;
	};

	manTrmData[0] = (memReqWord & memReqMask) | 0x80;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_90(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len < 1 || mtb == 0) return false;

	manTrmData[0] = (memReqWord & memReqMask) | 0x90;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMem_F0(u16 *data, u16 len, MTB* mtb)
{
	if (data == 0 || len == 0 || len > 2 || mtb == 0) return false;

	SaveMainParams();

	manTrmData[0] = (memReqWord & memReqMask) | 0xF0;

	mtb->data1 = manTrmData;
	mtb->len1 = 1;
	mtb->data2 = 0;
	mtb->len2 = 0;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


static bool RequestMem(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	bool r = false;

	byte i = buf[0]&0xFF;

	switch (i)
	{
		case 0x00: 	r = RequestMem_00(buf, len, mtb); break;
		case 0x10: 	r = RequestMem_10(buf, len, mtb); break;
		case 0x20: 	r = RequestMem_20(buf, len, mtb); break;
		case 0x30: 	r = RequestMem_30(buf, len, mtb); break;
		case 0x31: 	r = RequestMem_31(buf, len, mtb); break;
		case 0x32: 	r = RequestMem_32(buf, len, mtb); break;
		case 0x33: 	r = RequestMem_33(buf, len, mtb); break;
		case 0x80: 	r = RequestMem_80(buf, len, mtb); break;
		case 0x90: 	r = RequestMem_90(buf, len, mtb); break;
		case 0xF0: 	r = RequestMem_F0(buf, len, mtb); break;
	};

	if (r) { mtb->baud = memTrmBaud; };

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMan()
{
	static MTB mtb;
	static MRB mrb;

	static byte i = 0;

	static RTM tm;


//	u16 c;

	switch (i)
	{
		case 0:

//			HW::P5->BSET(7);

			mrb.data = manRcvData;
			mrb.maxLen = ArraySize(manRcvData);
			RcvManData(&mrb);

			i++;

			break;

		case 1:

			ManRcvUpdate();

			if (mrb.ready)
			{
				tm.Reset();

				if (mrb.OK && mrb.len > 0 &&	(((manRcvData[0] & manReqMask) == manReqWord && RequestMan(manRcvData, mrb.len, &mtb)) 
										||		((manRcvData[0] & memReqMask) == memReqWord && RequestMem(manRcvData, mrb.len, &mtb))))
				{
					//HW::P5->BCLR(7);

					i++;
				}
				else
				{
					i = 0;
				};
			}
			else if (mrb.len > 0)
			{

			};

			break;

		case 2:

			if (tm.Check(US2RT(100)))
			{
				//manTrmData[0] = 1;
				//manTrmData[1] = 0;
				//mtb.len1 = 2;
				//mtb.data1 = manTrmData;
				SendManData(&mtb);

				i++;
			};

			break;

		case 3:

			if (mtb.ready)
			{
				i = 0;
			};

			break;

	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void MainMode()
{
	static Ptr<REQ> rq;
	//static Ptr<MB> flwb;
//	static TM32 tm;
	static RspDsp30 *rsp = 0;

	switch (mainModeState)
	{
		case 0:

			rq = readyR01.Get();

			if (rq.Valid())
			{
				rsp = (RspDsp30*)(rq->rsp->GetDataPtr());

				mainModeState++;
			};

			break;

		case 1:
		{
			byte n = rsp->h.rw & 15;

			if (n == 0)
			{
				if (RequestFlashWrite_20()) mainModeState++;
			}
			else
			{
				mainModeState++;
			};

			break;
		};

		case 2:
		{
			RequestFlashWrite(rq->rsp, rsp->h.rw, false);

			byte n = rsp->h.rw & 15;

			manVec30[n] = rq->rsp;

			rq.Free();

			mainModeState++;

			break;
		};

		case 3:

			if (cmdWriteStart_00)
			{
				cmdWriteStart_00 = !RequestFlashWrite_00();
			}
			else if (cmdWriteStart_10)
			{
				cmdWriteStart_10 = !RequestFlashWrite_10();
			};

			mainModeState = 0;

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static DSCSPI dscAccel;

//static i16 ax = 0, ay = 0, az = 0, at = 0;


static u8 txAccel[25] = { 0 };
static u8 rxAccel[25];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool AccelReadReg(byte reg, u16 count)
{
	dscAccel.adr = (reg<<1)|1;
	dscAccel.alen = 1;
	//dscAccel.baud = 8000000;
	dscAccel.csnum = 0;
	dscAccel.wdata = 0;
	dscAccel.wlen = 0;
	dscAccel.rdata = rxAccel;
	dscAccel.rlen = count;

	return SPI_AddRequest(&dscAccel);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool AccelWriteReg(byte reg, u16 count)
{
	dscAccel.adr = (reg<<1)|0;
	dscAccel.alen = 1;
	dscAccel.csnum = 0;
	dscAccel.wdata = txAccel;
	dscAccel.wlen = count;
	dscAccel.rdata = 0;
	dscAccel.rlen = 0;

	return SPI_AddRequest(&dscAccel);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateAccel()
{
	static byte i = 0; 
	static i32 x = 0, y = 0, z = 0, t = 0;
	static i32 fx = 0, fy = 0, fz = 0;

	static TM32 tm;

	switch (i)
	{
		case 0:

			txAccel[0] = 0x52;
			AccelWriteReg(0x2F, 1); // Reset

			i++;

			break;

		case 1:

			if (dscAccel.ready)
			{
				tm.Reset();

				i++;
			};

			break;

		case 2:

			if (tm.Check(35))
			{
				AccelReadReg(0, 4);

				i++;
			};

			break;

		case 3:

			if (dscAccel.ready)
			{
				txAccel[0] = 0;
				AccelWriteReg(0x28, 1); // FILTER SETTINGS REGISTER

				i++;
			};

			break;

		case 4:

			if (dscAccel.ready)
			{
				txAccel[0] = 0;
				AccelWriteReg(0x2D, 1); // CTRL Set PORST to zero

				i++;
			};

			break;

		case 5:

			if (dscAccel.ready)
			{
				AccelReadReg(0x2D, 1);

				i++;
			};

			break;

		case 6:

			if (dscAccel.ready)
			{
				if (rxAccel[0] != 0)
				{
					txAccel[0] = 0;
					AccelWriteReg(0x2D, 1); // CTRL Set PORST to zero
					i--; 
				}
				else
				{
					txAccel[0] = 0;
					AccelWriteReg(0x2E, 1); // Self Test

					tm.Reset();
					i++;
				};
			};

			break;

		case 7:

			if (dscAccel.ready)
			{
				i++;
			};

			break;

		case 8:

			if (tm.Check(10))
			{
				AccelReadReg(6, 11); // X_MSB 

				i++;
			};

			break;

		case 9:

			if (dscAccel.ready)
			{
				t = (rxAccel[0] << 8) | rxAccel[1];
				x = (rxAccel[2] << 24) | (rxAccel[3] << 16) | (rxAccel[4]<<8);
				y = (rxAccel[5] << 24) | (rxAccel[6] << 16) | (rxAccel[7]<<8);
				z = (rxAccel[8] << 24) | (rxAccel[9] << 16) | (rxAccel[10]<<8);

				//x /= 4096;
				//y /= 4096;
				//z /= 4096;

				fx += (x - fx) / 8;
				fy += (y - fy) / 8;
				fz += (z - fz) / 8;

				ay = -(fz / 65536); 
				ax = (fy / 65536); 
				az = -(fx / 65536);

				at = 250 + ((1852 - t) * 1000 + 452) / 905;
				//at = 250 + (1852 - t) * 1.1049723756906077348066298342541f;

				i--;
			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTemp()
{
	static byte i = 0;

	static DSCI2C dsc, dsc2;

//	static byte reg = 0;
	static u16 rbuf = 0;
	static byte buf[10];

	static TM32 tm;

	switch (i)
	{
		case 0:

			if (tm.Check(100))
			{
#ifndef WIN32
				if (!__debug) { HW::ResetWDT(); };
#endif

				buf[0] = 0x11;

				dsc.adr = 0x68;
				dsc.wdata = buf;
				dsc.wlen = 1;
				dsc.rdata = &rbuf;
				dsc.rlen = 2;
				dsc.wdata2 = 0;
				dsc.wlen2 = 0;

				if (I2C_AddRequest(&dsc))
				{
					i++;
				};
			};

			break;

		case 1:

			if (dsc.ready)
			{
				if (dsc.ack && dsc.readedLen == dsc.rlen)
				{
					i32 t = (i16)ReverseWord(rbuf);
					
					t = (t * 10 + 128) / 256;

					if (t < (-600))
					{
						t += 2560;
					};

					tempClock = t;
				};
				//else
				//{
				//	tempClock = -2730;
				//};

				i++;
			};

			break;

		case 2:

			buf[0] = 0x0E;
			buf[1] = 0x20;
			buf[2] = 0xC8;

			dsc2.adr = 0x68;
			dsc2.wdata = buf;
			dsc2.wlen = 3;
			dsc2.rdata = 0;
			dsc2.rlen = 0;
			dsc2.wdata2 = 0;
			dsc2.wlen2 = 0;

			if (I2C_AddRequest(&dsc2))
			{
				i++;
			};

			break;

		case 3:

			if (dsc2.ready)
			{
				buf[0] = 0;

				dsc.adr = 0x49;
				dsc.wdata = buf;
				dsc.wlen = 1;
				dsc.rdata = &rbuf;
				dsc.rlen = 2;
				dsc.wdata2 = 0;
				dsc.wlen2 = 0;

				if (I2C_AddRequest(&dsc))
				{
					i++;
				};
			};

			break;

		case 4:

			if (dsc.ready)
			{
				if (dsc.ack && dsc.readedLen == dsc.rlen)
				{
					i32 t = (i16)ReverseWord(rbuf);

					temp = (t * 10 + 64) / 128;
				};
				//else
				//{
				//	temp = -2730;
				//};

#ifdef CPU_SAME53	

				i = 0;
			};

			break;

#elif defined(CPU_XMC48)

				HW::SCU_GENERAL->DTSCON = SCU_GENERAL_DTSCON_START_Msk;
				
				i++;
			};

			break;

		case 5:

			if (HW::SCU_GENERAL->DTSSTAT & SCU_GENERAL_DTSSTAT_RDY_Msk)
			{
				cpu_temp = ((i32)(HW::SCU_GENERAL->DTSSTAT & SCU_GENERAL_DTSSTAT_RESULT_Msk) - 605) * 1000 / 205;

				i = 0;
			};

			break;

#elif defined(WIN32)

				i = 0;
			};

			break;
#endif
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateI2C()
{
	if (!comdsp.Update())
	{
		if (I2C_Update())
		{
			comdsp.InitHW();

			i2cResetCount++;
		};
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMoto()
{
	static Ptr<REQ> rq;
	static TM32 tm;

	static byte i = 0;
	static byte count = 0;

	switch (i)
	{
		case 0:

			rq = CreateMotoReq();

			if (rq.Valid())
			{
				qmoto.Add(rq);

				i++;
			};

			break;

		case 1:

			if (rq->ready)
			{
				if (count >= 10)
				{
					DisableLPC();

					motoRcvCount = 0;

					tm.Reset();

					count = 0;

					i++;
				}
				else if (rq->crcOK)
				{
					count = 0;
				}
				else
				{
					count += 1;
				};

				rq.Free();

				i++;
			};

			break;

		case 2:

			if (tm.Check(101)) i = 0;

			break;

		case 3:

			if (tm.Check(2))
			{
				EnableLPC();

				i++;
			};

			break;

		case 4:

			if (tm.Check(500))
			{
				i = 0;
			};

			break;
	};

	qmoto.Update();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTestFlashWrite()
{
	static Ptr<MB> ptr;
	static u32 count = 0;

	static RTM rtm;

	if (rtm.Check(MS2RT(1)))
	{
		testDspReqCount++;

		count = 1000;
	};

//	if (count != 0)
	{
		ptr = CreateTestDspReq01();

		if (ptr.Valid())
		{
			count--;

			RspDsp01 *rsp = (RspDsp01*)(ptr->GetDataPtr());
			RequestFlashWrite(ptr, rsp->rw, true);

		};
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateDSP()
{
	static Ptr<REQ> rq;

	static byte i = 0;
	static TM32 tm;

	static bool repeat = false;

	switch (i)
	{
		case 0:

			if (mv.fireVoltage == 1)
			{
				if (FLASH_Status() != 0) UpdateTestFlashWrite();
			}
			else
			{
				rq = CreateDspReq01(repeat, 0);

				if (rq.Valid())	qdsp.Add(rq), i++;
			};

			break;

		case 1:

			if (rq->ready)
			{
				repeat = !rq->crcOK;

				if (rq->crcOK && rq->rsp->len != 0)
				{
					readyR01.Add(rq);

					tm.Reset();
				};

				if (tm.Check(1000))
				{
					DisableDSP();

					dspRcvCount = 0;

					i++;
				}
				else
				{
					i = 0;
				};

				rq.Free();
			};

			break;

		case 2:

			if (tm.Check(2))
			{
				EnableDSP();

				i++;
			};

			break;

		case 3:

			if (tm.Check(500))
			{
				i = 0;
			};

			break;

	};

	qdsp.Update();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

static const u32 dspFlashPages[] = {
#include "G26R_2_BF592.LDR.H"
};

//u16 dspFlashLen = 0;
//u16 dspFlashCRC = 0;

static const u32 dspBootFlashPages[] = {
#include "G26R_2_BF592.LDR.H"
};

//u16 dspBootFlashLen = 0;
//u16 dspBootFlashCRC = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void FlashDSP()
{
	TM32 tm;

	Ptr<REQ> req;

	EnableDSP();

	while (!req.Valid()) req = CreateDspReq05(50);

	qdsp.Add(req); while(!req->ready) { qdsp.Update(); HW::ResetWDT(); };

	//tm.Reset();

	//while (!tm.Check(10)) HW::WDT->Update();

	if (req->crcOK)
	{
		RspDsp05 *rsp = (RspDsp05*)req->rb.data;

		u16 flen;
		const u32 *fpages;
		u16 flashCRC;
		u16 flashLen;
		u32 startAdr;

		if (req->rb.len < sizeof(RspDsp05))
		{
			startAdr = 0; flashLen = rsp->v1.flashLen; flashCRC = rsp->v1.flashCRC;
		}
		else
		{
			startAdr = rsp->v2.startAdr; flashLen = rsp->v2.flashLen; flashCRC = rsp->v2.flashCRC;
		};

		if (startAdr > 0)
		{
			flen = sizeof(dspFlashPages);
			fpages = dspFlashPages;
		}
		else
		{
			flen = sizeof(dspBootFlashPages);
			fpages = dspBootFlashPages;
		};

		u16 fcrc = GetCRC16(fpages, flen);

		if (flashCRC != fcrc || flashLen != flen)
		{
			u16 count = flen+2;
			u16 adr = 0;
			byte *p = (byte*)fpages;

			while (count > 0)
			{
				u16 len;
				
				if (count > 256)
				{
					len = 256;

					req = CreateDspReq06(adr, len, p, 0, 0, 10);
				}
				else
				{
					len = count;

					if (len > 2)
					{
						req = CreateDspReq06(adr, len-2, p, sizeof(fcrc), &fcrc, 10);
					}
					else
					{
						req = CreateDspReq06(adr, sizeof(fcrc), &fcrc, 0, 0, 10);
					};
				};

				qdsp.Add(req); while(!req->ready) { qdsp.Update(); HW::ResetWDT(); };

				count -= len;
				p += len;
				adr += len;
			};
		};

		tm.Reset();

		while (!tm.Check(50)) HW::ResetWDT();

		req = CreateDspReq07();

		qdsp.Add(req); while(!req->ready) { qdsp.Update(); HW::ResetWDT();	};

		tm.Reset();

		while (!tm.Check(50)) HW::ResetWDT();
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void FlashDSP_Direct()
{
	u16 flen;
	const u32 *fpages;
	u16 flashCRC;
	u16 flashLen;
//	u32 startAdr;

	at25df021_Init(5000000);
	
	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "Start BF592 flash check ... ");

	flen = sizeof(dspFlashPages);
	fpages = dspFlashPages;

	BlackFin_CheckFlash(&flashCRC, &flashLen);

	//flashCRC = at25df021_GetCRC16(flen);

	u16 fcrc = GetCRC16(fpages, flen);

	if (flashCRC != fcrc || flashLen != flen)
	{
		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED "ERRROR\n" RTT_CTRL_TEXT_BRIGHT_WHITE "Start BF592 flash write ... ");

		if (NO_ERR == at25df021_Write((byte*)fpages, 0, flen, true))
		{
			SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n");
		}
		else
		{
			SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED "ERRROR\n");
		};
	}
	else
	{
		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n");
	};

	at25df021_Destroy();

	EnableDSP();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static const u32 motoFlashPages[] = {
#include "G26R_2_LPC824.BIN.H"
};

u16 motoFlashLen = 0;
u16 motoFlashCRC = 0;

#define SGUID	0x53EE4AA1A6944D07
#define MGUID	0x60F1952B63424888

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void FlashMoto()
{
	static ReqBootMotoHS		reqHS;
	static RspBootMotoHS		rspHS;
	static ComPort::WriteBuffer wb;
	static ComPort::ReadBuffer	rb;

	const unsigned __int64 masterGUID = MGUID;
	const unsigned __int64 slaveGUID = SGUID;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_CYAN "Flash LPC824 start ...\n");

	TM32 tm;

	Ptr<REQ> req;

	motoFlashLen = sizeof(motoFlashPages);
	motoFlashCRC = GetCRC16(motoFlashPages, motoFlashLen);

	tm.Reset();

	while(!tm.Check(1));

	EnableLPC();

	bool hs = false;

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_WHITE "HandShake LPC824 ... ");

	while (!tm.Check(200))
	{
		reqHS.guid = masterGUID;
		reqHS.crc = GetCRC16(&reqHS, sizeof(reqHS) - sizeof(reqHS.crc));
		wb.data = &reqHS;
		wb.len = sizeof(reqHS);

		commoto.Write(&wb);

		while (commoto.Update()) HW::ResetWDT(); 

		rb.data = &rspHS;
		rb.maxLen = sizeof(rspHS);
		commoto.Read(&rb, MS2COM(5), US2COM(100));

		while (commoto.Update()) HW::ResetWDT();

		if (rb.recieved)
		{
			if (rb.len == sizeof(rspHS) && GetCRC16(&rspHS, sizeof(rspHS)) == 0 && rspHS.guid == slaveGUID)
			{
				hs = true;
				break;
			};
		};
	};

	if (hs)
	{
		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n");

		while (!tm.Check(10)) HW::ResetWDT();

		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "Request LPC824 Flash CRC ... ");

		req = CreateBootMotoReq01(motoFlashLen, 2);

		qmoto.Add(req); while(!req->ready) { qmoto.Update(); HW::ResetWDT(); };

		if (req->crcOK)
		{
			RspBootMoto *rsp = (RspBootMoto*)req->rb.data;

			SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n");

			if (rsp->F01.flashCRC != motoFlashCRC || rsp->F01.flashLen != motoFlashLen)
			{
				SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "Start write flash LPC824 ...");

				u16 count = motoFlashLen/4;
				u32 adr = 0;
				const u32 *p = motoFlashPages;

				while (count > 0)
				{
					u16 len = (count > 16) ? 16 : count;

					for(u32 i = 3; i > 0; i--)
					{
						req = CreateBootMotoReq02(adr, len, p, 3);

						qmoto.Add(req); while(!req->ready) { qmoto.Update(); HW::ResetWDT(); };

						RspBootMoto *rsp = (RspBootMoto*)req->rb.data;

						if (req->crcOK && rsp->F02.status) { break;	}
					};

					tm.Reset();

					while (!tm.Check(1)) HW::ResetWDT();

					count -= len;
					p += len;
					adr += len*4;
				};

				SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n");
			};
		}
		else
		{
			SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED "!!! ERROR !!!\n");
		};

		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_CYAN "Request LPC824 Main App start\n");

		req = CreateBootMotoReq03();

		qmoto.Add(req); while(!req->ready) { qmoto.Update(); HW::ResetWDT();	};

		tm.Reset();

		while (!tm.Check(1)) HW::ResetWDT();
	}
	else
	{
		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED "!!! ERROR !!!\n");
	};
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void InitMainVars()
{
	mv.numDevice		= 0;
	mv.numMemDevice		= 0;
	mv.gain				= 0; 
	mv.sampleTime		= 8; 
	mv.sampleLen		= 500; 
	mv.sampleDelay 		= 400; 
	mv.freq				= 500; 
	mv.firePeriod		= 1000;
	mv.fireVoltage		= 500;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void LoadVars()
{
	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_CYAN "Load Vars ... ");

	static DSCI2C dsc;
	static DSCSPI spi;
	static u16 romAdr = 0;
	
	byte buf[sizeof(mv)*2+4];

	MainVars mv1, mv2;

	bool c1 = false, c2 = false;

	//spi.adr = ((u32)ReverseWord(FRAM_SPI_MAINVARS_ADR)<<8)|0x9F;
	//spi.alen = 1;
	//spi.csnum = 1;
	//spi.wdata = 0;
	//spi.wlen = 0;
	//spi.rdata = buf;
	//spi.rlen = 9;

	//if (SPI_AddRequest(&spi))
	//{
	//	while (!spi.ready);
	//};

	bool loadVarsOk = false;

	spi.adr = (ReverseDword(FRAM_SPI_MAINVARS_ADR) & ~0xFF) | 3;
	spi.alen = 4;
	spi.csnum = 1;
	spi.wdata = 0;
	spi.wlen = 0;
	spi.rdata = buf;
	spi.rlen = sizeof(buf);

	if (SPI_AddRequest(&spi))
	{
		while (!spi.ready) { SPI_Update(); };
	};

	PointerCRC p(buf);

	for (byte i = 0; i < 2; i++)
	{
		p.CRC.w = 0xFFFF;
		p.ReadArrayB(&mv1, sizeof(mv1));
		p.ReadW();

		if (p.CRC.w == 0) { c1 = true; break; };
	};

	romAdr = ReverseWord(FRAM_I2C_MAINVARS_ADR);

	dsc.wdata = &romAdr;
	dsc.wlen = sizeof(romAdr);
	dsc.wdata2 = 0;
	dsc.wlen2 = 0;
	dsc.rdata = buf;
	dsc.rlen = sizeof(buf);
	dsc.adr = 0x50;


	if (I2C_AddRequest(&dsc))
	{
		while (!dsc.ready) { I2C_Update(); };
	};

//	bool c = false;

	p.b = buf;

	for (byte i = 0; i < 2; i++)
	{
		p.CRC.w = 0xFFFF;
		p.ReadArrayB(&mv2, sizeof(mv2));
		p.ReadW();

		if (p.CRC.w == 0) { c2 = true; break; };
	};

	SEGGER_RTT_WriteString(0, "FRAM SPI - "); SEGGER_RTT_WriteString(0, (c1) ? (RTT_CTRL_TEXT_BRIGHT_GREEN "OK") : (RTT_CTRL_TEXT_BRIGHT_RED "ERROR"));

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_WHITE " - FRAM I2C - "); SEGGER_RTT_WriteString(0, (c2) ? (RTT_CTRL_TEXT_BRIGHT_CYAN "OK\n") : (RTT_CTRL_TEXT_BRIGHT_RED "ERROR\n"));

	if (c1 && c2)
	{
		if (mv1.timeStamp > mv2.timeStamp)
		{
			c2 = false;
		}
		else if (mv1.timeStamp < mv2.timeStamp)
		{
			c1 = false;
		};
	};

	if (c1)	{ mv = mv1; } else if (c2) { mv = mv2; };

	loadVarsOk = c1 || c2;

	if (!c1 || !c2)
	{
		if (!loadVarsOk) InitMainVars();

		svCount = 2;
	};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void SaveVars()
{
	static DSCI2C dsc;
	static DSCSPI spi,spi2;
	static u16 romAdr = 0;
	static byte buf[sizeof(mv) * 2 + 8];

	static byte i = 0;
	static TM32 tm;

	PointerCRC p(buf);

	switch (i)
	{
		case 0:

			if (svCount > 0)
			{
				svCount--;
				i++;
			};

			break;

		case 1:

			mv.timeStamp = GetMilliseconds();

			for (byte j = 0; j < 2; j++)
			{
				p.CRC.w = 0xFFFF;
				p.WriteArrayB(&mv, sizeof(mv));
				p.WriteW(p.CRC.w);
			};

			spi.adr = (ReverseDword(FRAM_SPI_MAINVARS_ADR) & ~0xFF) | 2;
			spi.alen = 4;
			spi.csnum = 1;
			spi.wdata = buf;
			spi.wlen = p.b-buf;
			spi.rdata = 0;
			spi.rlen = 0;

			romAdr = ReverseWord(FRAM_I2C_MAINVARS_ADR);

			dsc.wdata = &romAdr;
			dsc.wlen = sizeof(romAdr);
			dsc.wdata2 = buf;
			dsc.wlen2 = p.b-buf;
			dsc.rdata = 0;
			dsc.rlen = 0;
			dsc.adr = 0x50;

			spi2.adr = 6;
			spi2.alen = 1;
			spi2.csnum = 1;
			spi2.wdata = 0;
			spi2.wlen = 0;
			spi2.rdata = 0;
			spi2.rlen = 0;

			tm.Reset();

			SPI_AddRequest(&spi2);

			i++;

			break;

		case 2:

			if (spi2.ready || tm.Check(200))
			{
				SPI_AddRequest(&spi);

				i++;
			};

			break;

		case 3:

			if (spi.ready || tm.Check(200))
			{
				I2C_AddRequest(&dsc);
				
				i++;
			};

			break;

		case 4:

			if (dsc.ready || tm.Check(100))
			{
				i = 0;
			};

			break;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateParams()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( MainMode()				);
		CALL( UpdateTemp()				);
		CALL( UpdateMan(); 				);
		CALL( FLASH_Update();			);
		CALL( UpdateAccel();			);
		CALL( UpdateI2C();				);
		CALL( SaveVars();				);
		CALL( UpdateMoto();				);
		CALL( UpdateDSP();				);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateEMAC();		);
		CALL( UpdateParams();	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Update()
{
	NAND_Idle();	

	if (EmacIsConnected())
	{
		UpdateEMAC();
		UpdateTraps();

#ifndef WIN32
		if (!__debug) { HW::ResetWDT(); };
#endif
	};
	
	UpdateHardware();

	if (!(IsComputerFind() && EmacIsConnected()))
	{
		UpdateMisc();
	}
	else
	{
		FLASH_Update();
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_SAME53

	#define FPS_PIN_SET()	HW::PIOA->BSET(25)
	#define FPS_PIN_CLR()	HW::PIOA->BCLR(25)

#elif defined(CPU_XMC48)

	#define FPS_PIN_SET()	HW::P2->BSET(13)
	#define FPS_PIN_CLR()	HW::P2->BCLR(13)

#elif defined(WIN32)

	#define FPS_PIN_SET()	
	#define FPS_PIN_CLR()	

#endif

//static ComPort com1;

int main()
{
	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_WHITE "main() start ...\n");

//	static bool c = true;

	//static u16 buf[100] = {0};

	//volatile byte * const FLD = (byte*)0x60000000;	
	
	//static ComPort commem;

	DSCSPI dsc, dsc2;

	TM32 tm;

	//__breakpoint(0);

#ifndef WIN32

	DisableLPC();
	DisableDSP();

#endif

	InitHardware();

	LoadVars();

	InitEMAC();

	//InitTraps();

	FLASH_Init();

//	InitRmemList();

	Update_RPS_SPR();

#ifndef WIN32

	commoto.Connect(ComPort::ASYNC, 1562500, 0, 1);
	comdsp.Connect(ComPort::ASYNC, 6250000, 2, 1);

	//__breakpoint(0);

	//ComPort::WriteBuffer wb;
	//CTM32 ctm;

	//wb.data = buf;
	//wb.len = 1;

	//for (u32 i = 0; i < 10; i++)
	//{
	//	comdsp.Write(&wb);
	//	while(comdsp.Update());
	//	ctm.Reset(); 
	//	while(!ctm.Check(US2COM(20)));
	//	wb.len += 1;
	//};

	//__breakpoint(0);

	FlashMoto();

	FlashDSP_Direct();

//	comdsp.Disconnect();

//	comdsp.Connect(ComPort::ASYNC, 1000000, 2, 1);

#endif

	u32 fc = 0;

	//ComPort::WriteBuffer wb;

	//for (u32 i = 0; i < ArraySize(buf); i++) buf[i] = i;

	//fps = CRC_CCITT_DMA(0, 100);
	//fc = CRC_CCITT_DMA(0, 50);
	//fc = CRC_CCITT_DMA((void*)50, 50, fc);

	//fps = CRC_CCITT_DMA(buf, 6000, 0xFFFF);
	//fps = CRC_CCITT_DMA(buf+6000, 2000, fps);

	//fc = CRC_CCITT_PIO(buf, 6000, 0xFFFF);
	//fc = CRC_CCITT_PIO(buf+6000, 2000, fc);

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_WHITE "Main Loop start ...\n");

	while (1)
	{
		FPS_PIN_SET();

		Update();

		FPS_PIN_CLR();

		fc++;

		if (tm.Check(1000))
		{ 
			fps = fc; fc = 0; 

#ifdef WIN32

			extern u32 txThreadCount;

			Printf(0, 0, 0xFC, "FPS=%9i", fps);
			Printf(0, 1, 0xF0, "%lu", testDspReqCount);
			Printf(0, 2, 0xF0, "%lu", txThreadCount);
#endif
		};

#ifdef WIN32

		UpdateDisplay();

		static TM32 tm2;

		byte key = 0;

		if (tm2.Check(50))
		{
			if (_kbhit())
			{
				key = _getch();

				if (key == 27) break;
			};

			if (key == 'w')
			{
				FLASH_WriteEnable();
			}
			else if (key == 'e')
			{
				FLASH_WriteDisable();
			}
			else if (key == 'p')
			{
				NAND_FullErase();
			};
		};

#endif

	}; // while (1)

#ifdef WIN32

	NAND_FlushBlockBuffers();

	I2C_Destroy();
	SPI_Destroy();

	//_fcloseall();

#endif

}
