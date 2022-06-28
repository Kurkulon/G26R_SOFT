#ifndef REQ_H__09_10_2014__10_31
#define REQ_H__09_10_2014__10_31

#include "ComPort.h"

#include "list.h"
#include "flash.h"

//struct Request
//{
//	byte adr;
//	byte func;
//	
//	union
//	{
//		struct  { byte n; word crc; } f1;  // ����� ���������
//		struct  { byte n; byte chnl; word crc; } f2;  // ������ �������
//		struct  { byte dt[3]; byte ka[3]; word crc; } f3;  // ��������� ������� ������������� ������� � ������������ ��������
//	};
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqMoto
{
	u16 	rw;
	u16 	reqHV; 
	u16 	crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct RspMoto
{
	u16 	rw;
	u16 	curHV; 
	u16 	crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqBootMotoHS { unsigned __int64 guid; u16 crc; };
__packed struct RspBootMotoHS { unsigned __int64 guid; u16 crc; };

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct ReqBootMoto
{
	u32 func;

	union
	{
		struct { u32 flashLen;  u16 align; u16 crc; } F01; // Get Flash CRC
		struct { u32 padr; u32 page[16]; u16 align; u16 crc; } F02; // Write page
		struct { u16 align; u16 crc; } F03; // Exit boot loader
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RspBootMoto
{
	u32 func;

	union
	{
		struct { u32 flashLen; u16 flashCRC; u16 crc; } F01;
		struct { u32 padr; u32 status; u16 align; u16 crc; } F02;
		struct { u16 align; u16 crc; } F03;							// Exit boot loader
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct ReqDsp01	// ������ �������
{
	u16 	rw;

	u16		gain; 
	u16		sampleTime; 
	u16		sampleLen; 
	u16		sampleDelay; 
	u16		flt;
	u16		firePeriod; //ms

	u16 	crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RspDsp01	// ������ �������
{
	u16		rw; 

	u16		counter;		
	u16 	crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RspDsp30 
{ 
	struct Hdr 
	{ 
		u16 rw; 
		u16 fnum; 
		u16 gain; 
		u16 st; 
		u16 sl; 
		u16 sd; 
		u16 flt; 
	} h; 
	u16 data[1024+32]; 
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  ReqDsp05	// ������ ����������� ����� � ����� ��������� �� ����-������
{ 
	u16		rw; 
	u16 	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  RspDsp05	// ������ ����������� ����� � ����� ��������� �� ����-������
{ 
	u16		rw; 

	__packed union
	{
		__packed struct { u16 flashLen; u16 flashCRC; u16 crc; } v1;
		__packed struct { u16 flashLen; u32 startAdr; u16 flashCRC; u16 crc; } v2;
	};
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  ReqDsp06	// ������ �������� �� ����
{ 
	u16		rw; 
	u16		stAdr; 
	u16		count; 
	byte	data[258]; 
	u16		crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  RspDsp06	// ������ �������� �� ����
{ 
	u16		rw; 
	u16		res; 
	word	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  ReqDsp07	// ������������� �������
{ 
	u16		rw; 
	word 	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

union ReqUnion
{
	ReqDsp01 	dsp01;	
	ReqDsp05 	dsp05;	
	ReqDsp06 	dsp06;	
	ReqDsp07 	dsp07;	
	ReqMoto		moto;	
	ReqBootMoto bootMoto;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__packed struct ReqTrm01	
//{
//	byte 	len;
//	byte 	func;
//	byte 	n; 
//	word 	crc;  
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__packed struct ReqTrm02	
//{
//	byte 	len;
//	byte 	f;
//	word 	crc;  
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__packed struct RspTrm02	
//{
//	byte f; 
//	u16 hv; 
//	u16 crc;
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__packed struct ReqTrm03	
//{
//	byte 	len;
//	byte	f; 
//	byte	fireCountM; 
//	byte	fireCountXY; 
//	u16		hv;
//	word 	crc;  
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__packed struct RspTrm03	
//{
//	byte f; 
//	u16 crc;
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__packed struct ReqMem
//{
//	u16 rw; 
//	u32 cnt; 
//	u16 gain; 
//	u16 st; 
//	u16 len; 
//	u16 delay; 
//	u16 data[1024*4]; 
//	u16 crc;
//
//	//byte adr;
//	//byte func;
//	
//	//__packed union
//	//{
//	//	__packed struct  { word crc; } f1;  // ����� ����� ������
//	//	__packed struct  { word crc; } f3;  
//	//};
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__packed struct RspMem
//{
//	u16 rw; 
//	u16 crc; 
//
//	//byte	adr;
//	//byte	func;
//	
//	//__packed union
//	//{
//	//	__packed struct  { word crc; } f1;  // ����� ����� ������
//	//	__packed struct  { word crc; } f2;  // ������ �������
//	//	__packed struct  { word crc; } f3;  // 
//	//	__packed struct  { word crc; } fFE;  // ������ CRC
//	//	__packed struct  { word crc; } fFF;  // ������������ ������
//	//};
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//__packed struct RspMan60
//{
//	u16 rw; 
//	u32 cnt; 
//	u16 maxAmp[96]; 
//	u16 power[96];
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct REQ : public PtrItem<REQ>
{
	enum CRC_TYPE { CRC16 = 0, CRC16_CCIT, CRC16_NOT_VALID };

	bool		ready;
	bool		crcOK;
	bool		checkCRC;
	bool		updateCRC;

	typedef bool tRsp(Ptr<REQ> &q);

	u16			tryCount;

	CRC_TYPE	crcType;
	
	//REQ *next;

	tRsp		*CallBack;
	Ptr<UNIBUF>	rsp;

	ComPort::WriteBuffer wb;
	ComPort::ReadBuffer rb;

	u32		preTimeOut, postTimeOut;

	byte	reqData[(sizeof(ReqUnion)+64) & ~3];

protected:

	virtual void _FreeCallBack() { rsp.Free(); }

public:

	//void	Free() { if (this != 0) rsp.Free(), PtrItem<REQ>::Free(); }

	REQ() : tryCount(0) { }
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class RequestQuery
{
	//REQ*			_first;
	//REQ*			_last;
	Ptr<REQ>		_req;

	ListPtr<REQ>	reqList;
	
	byte			_state;

	u16				_crc;
	u16 			_crcLen;

	byte*			_crcPtr;


	ComPort			*com;

	//u32			count;

	bool			_run;

public:

				RequestQuery(ComPort *p) : _run(true), _state(0), com(p) {}
	void		Add(const Ptr<REQ>& req)	{ reqList.Add(req); }
	Ptr<REQ>	Get()						{ return reqList.Get(); }
	//bool Empty() { return reqList.Empty(); }
	//bool Idle() { return (_first == 0) && (_req == 0); }
	bool Stoped() { return !_req.Valid(); }
	void Update();
	void Stop() { _run = false; }
	void Start() { _run = true; }
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//struct R01
//{
//	R01*	next;
//
//protected:
//
//	u32		count;
//
//	static List<R01> freeList;
//
//public:
//
////	bool memNeedSend;
//	//ComPort::WriteBuffer	wb;
//	//ComPort::ReadBuffer		rb;
//	REQ			q;
//	ReqDsp01	req;
//	RspDsp01	rsp;
//
//	R01() : next(0) { freeList.Add(this); }
//
//	static	R01*	Alloc()	{ R01* p = freeList.Get(); if (p != 0) p->count = 1; return p; };
//			void	Free()	{ if (this != 0 && count != 0) { count--; if (count == 0) freeList.Add(this); }; }
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*struct RMEM
{
	RMEM* next;

//	R01*	r02;

	ComPort::WriteBuffer	wb;
	ComPort::ReadBuffer		rb;
	REQ		q;
	ReqMem	req;
	RspMem	rsp;
};*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++






#endif //REQ_H__09_10_2014__10_31
