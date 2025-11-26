#ifndef REQ_H__09_10_2014__10_31
#define REQ_H__09_10_2014__10_31

#pragma once

#include "hw_conf.h"

#include <RequestQuery.h>
#include <ComPort\ComPort.h>

#include <list.h>
//#include <FLASH\flash.h>
#include <BOOT\boot_req.h>

//struct Request
//{
//	byte adr;
//	byte func;
//	
//	union
//	{
//		struct  { byte n; word crc; } f1;  // старт оцифровки
//		struct  { byte n; byte chnl; word crc; } f2;  // чтение вектора
//		struct  { byte dt[3]; byte ka[3]; word crc; } f3;  // установка периода дискретизации вектора и коэффициента усиления
//	};
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqMoto
{
	u16 	rw;
	u16 	reqHV;
	u16		freq;
	u16		sampleTime;
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

struct ReqDsp01	// чтение вектора
{
	u16 	rw;

	u16		repeatResponse;
	u16		gain; 
	u16		sampleTime; 
	u16		sampleLen; 
	u16		sampleDelay; 
	u16		firePeriod; //ms
	u16		verDevice;

	u16 	crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct RspDsp01	// чтение вектора
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
		u16 verDevice;
		u16 gain; 
		u16 st; 
		u16 sl; 
		u16 sd; 
	} h; 
	u16 data[1024+32]; 
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  ReqDsp05	// запрос контрольной суммы и длины программы во флэш-памяти
{ 
	u16		rw; 
	u16 	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  RspDsp05	// запрос контрольной суммы и длины программы во флэш-памяти
{ 
	u16		rw; 

	__packed union
	{
		__packed struct { u16 flashLen; u16 flashCRC; u16 crc; } v1;
		__packed struct { u16 flashLen; u32 startAdr; u16 flashCRC; u16 crc; } v2;
	};
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  ReqDsp06	// запись страницы во флэш
{ 
	u16		rw; 
	u16		stAdr; 
	u16		count; 
	byte	data[258]; 
	u16		crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  RspDsp06	// запись страницы во флэш
{ 
	u16		rw; 
	u16		res; 
	word	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  ReqDsp07	// перезагрузить блэкфин
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






#endif //REQ_H__09_10_2014__10_31
