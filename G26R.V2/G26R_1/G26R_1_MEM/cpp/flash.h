#ifndef FLASH_H__09_08_2019__08_17
#define FLASH_H__09_08_2019__08_17

#include "hardware.h"

//#include "rtc.h"
//#include "vector.h"
#include "trap_def.h"
#include "list.h"
#include <stddef.h>
#include "CRC16.h"


//#define FLWB_LEN (3072)
//#define FLRB_LEN 1536

#define UNIBUF_LEN (3072)


enum  
{
	FLASH_STATUS_NONE = 0,
	FLASH_STATUS_BUSY,				// ����� ����������� ��� ���������� ��������� (�������� ������� � ������)
	FLASH_STATUS_STOP,
	FLASH_STATUS_PAUSE,
	FLASH_STATUS_RESUME,
	FLASH_STATUS_ERASE,
	FLASH_STATUS_UNERASE,
	FLASH_STATUS_READ_SESSION_START,
	FLASH_STATUS_READ_SESSION_IDLE,
	FLASH_STATUS_READ_SESSION_FIND, // ����� ������� �� ����������� �������
	FLASH_STATUS_READ_SESSION_ERROR,
	FLASH_STATUS_READ_SESSION_READY,
	FLASH_STATUS_READ_VECTOR_START,
	FLASH_STATUS_READ_VECTOR_IDLE,
	FLASH_STATUS_READ_VECTOR_FIND, // ����� ������� �� ����������� �������
	FLASH_STATUS_READ_VECTOR_ERROR,
	FLASH_STATUS_READ_VECTOR_READY
};


enum flash_save_repeat_type
{
	FLASH_SAVE_REPEAT_NONE = 0,
	FLASH_SAVE_REPEAT_NORMAL = 2,	// ���� ����������, ������ - � ����� ����
	FLASH_SAVE_REPEAT_HIGH = 4,	// .... 3 � ����� ����
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct VecData
{
	__packed struct Hdr
	{
		u16 		session;
		u16 		device;
		RTC_type	rtc;
		byte		flags;		// ������
		u64			prVecAdr;	// ����� ����������� ������������
		u16			dataLen;
		u16			crc;		// CRC16 ���� ���� �����
	} h;

	byte		data[/*FLWB_LEN*/]; // ��������� 2 ����� CRC16
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//struct _FLWB
//{
//	friend class Ptr<FLWB>;
//	friend class List<FLWB>;
//	friend class ListPtr<FLWB>;
//
//	FLWB*	next;
//	u32		count;
//
//protected:
//
//	static List<FLWB> freeBufList;
//
//	static	FLWB*	Alloc()	{ FLWB* p = freeBufList.Get(); if (p != 0) p->count = 1; return p; };
//			void	Free()	{ if (this != 0 && count != 0) { count--; if (count == 0) freeBufList.Add(this); }; }
//
//public:
//
//	FLWB() : next(0), count(0) { freeBufList.Add(this); }
//
//	//void* operator new(size_t i) { return ((FLWB*)0)->Alloc(); }
//
//	u16 	dataLen;
//
//	VecData vd;
//};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct UNIBUF : public PtrItem<UNIBUF>
{
	UNIBUF() : dataOffset(sizeof(VecData::Hdr)), dataLen(0)  { /*freeBufList.Add(this);*/ }

	u16		dataOffset;
	u16 	dataLen;

	byte	data[UNIBUF_LEN]; // ��������� 2 ����� CRC16

	void*	GetDataPtr() { return data+dataOffset; }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//struct UNIBUF
//{
//	friend class Ptr<UNIBUF>;
//	friend class List<UNIBUF>;
//	friend class ListPtr<UNIBUF>;
//
//	UNIBUF*	next;
//	u32		count;
//
//protected:
//
//	static List<UNIBUF> freeBufList;
//
//	static	UNIBUF*	Alloc()	{ UNIBUF* p = freeBufList.Get(); if (p != 0) p->count = 1; return p; };
//			void	Free()	{ if (this != 0 && count != 0) { count--; if (count == 0) freeBufList.Add(this); }; }
//
//public:
//
//	UNIBUF() : next(0), count(0), dataOffset(sizeof(VecData::Hdr)), dataLen(0)  { freeBufList.Add(this); }
//
//	u16		dataOffset;
//	u16 	dataLen;
//
//	byte	data[UNIBUF_LEN]; // ��������� 2 ����� CRC16
//
//	void*	GetDataPtr() { return data+dataOffset; }
//};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//class PtrFLWB
//{
//	friend class List<PtrFLWB>;
//
//protected:
//
//	FLWB* buf;
//
//public:
//
//	PtrFLWB() : buf(0) {}
//	PtrFLWB(const PtrFLWB& p) : buf(p.buf) { buf->count++; }
//
//	PtrFLWB& operator=(const PtrFLWB& p) { if (buf != p.buf) { buf->Free(); buf = p.buf; buf->count++; }; return *this; }
//	~PtrFLWB() { buf->Free(); }
//	bool Valid() { return buf != 0; }
//
//	void AllocBuf() { buf->Free(); buf = buf->Alloc(); }
//	void FreeBuf() { buf->Free(); buf = 0; }
//
//	//FLWB* operator->() { return buf; }
//};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct FLRB
{
	FLRB *next;

	bool	ready;
	bool	vecStart; // ����� ������ ������� � ������� hdr

	bool	useAdr;
	u64		adr;

	VecData::Hdr hdr;

	u16		maxLen;
	u16		len;

	byte	*data;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct FileDsc
{
	u16			session;	// file number
	i64			size;		//���� 0 �� ������ ������� ���������
	RTC_type	start_rtc;	//���� 0 �� ������ ������� ���������
	RTC_type	stop_rtc;  
	u32			startPage; 
	u32			lastPage;
	byte		flags;
};	

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct SpareArea
{
	u16		validPage;	// 0xFFFF - good page
	u16		validBlock;	// 0xFFFF - good block
	u16		badPages;	// 0xFFFF - all pages good

	enum { CRC_SKIP = 6 };

	__packed union
	{
		__packed struct //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			u16		file;  			// file number

			u32		start;			// start page of file
			u32		fpn;			// file page number
			u32		prev;			// start page of previos file
			u32		rawPage;		// raw page num

			u32		vectorCount;	// vector count in file

			u16		vecFstOff;		// first vector header offset. 0xFFFF - no vector header 
			u16		vecFstLen;		// first vector lenght. 0 - no vector

			u16		vecLstOff;		// last vector header offset. 0xFFFF - no vector header 
			u16		vecLstLen;		// last vector lenght. 0 - no vector

			u16		fbb;			// file bad blocks count
			u16		fbp;			// file bad pages count

			byte	chipMask;	

			u16		crc;

			void CheckCRC() { crc = GetCRC16(this, sizeof(*this)); }
			void UpdateCRC() { crc = GetCRC16(this, sizeof(*this) - sizeof(crc)); }

		} v1;

		__packed struct //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			u16		len;			// struct lenght
			u16		version;		// struct version

			u16		file;  			// file number

			u32		start;			// start page of file
			u32		fpn;			// file page number
			u32		prev;			// start page of previos file
			u32		rawPage;		// raw page num

			u32		vectorCount;	// vector count in file

			u16		vecFstOff;		// first vector header offset. 0xFFFF - no vector header 
			u16		vecFstLen;		// first vector lenght. 0 - no vector

			u16		vecLstOff;		// last vector header offset. 0xFFFF - no vector header 
			u16		vecLstLen;		// last vector lenght. 0 - no vector

			//u32		vecPrPg;	// previos vector raw page
			//u16		vecPrOff;	// previos vector offset

			u16		fbb;			// file bad blocks count
			u16		fbp;			// file bad pages count

			byte	chipMask;	

			u16		crc;

			void CheckCRC() { crc = GetCRC16(this, sizeof(*this)); }
			void UpdateCRC() { crc = GetCRC16(this, sizeof(*this) - sizeof(crc)); }

		} v2;
	};
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//extern FLWB*	AllocFlashWriteBuffer();
extern FLRB*	AllocFlashReadBuffer();
//extern void		FreeFlashWriteBuffer(FLWB* b);
extern void		FreeFlashReadBuffer(FLRB* b);
extern bool		RequestFlashRead(FLRB* b);
extern bool		RequestFlashWrite(Ptr<UNIBUF> &b, u16 devID);

extern void NAND_Idle();
extern void NAND_FullErase();
//extern void NAND_NextSession();

inline u16 GetDeviceID() { extern u16 deviceID; return deviceID; }

extern void StartSendSession();
extern FileDsc* GetSessionInfo(u16 session, u64 adr);

//extern const SessionInfo* GetLastSessionInfo();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern void FLASH_Init();
extern bool FLASH_Reset();
extern void FLASH_Update();
extern bool FLASH_Busy();

extern bool FLASH_WriteEnable();
extern void FLASH_WriteDisable();
extern byte FLASH_Status();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
//extern bool FLASH_Erase_Full();
//extern bool FLASH_UnErase_Full();
//extern bool FLASH_Write_Vector(u16 session, u16 device, RTC_type rtc, byte flags, byte *data, u16 size, flash_save_repeat_type repeat);
//extern bool FLASH_Read_Vector(u64 adr, u16 *size, bool *ready, byte **vector);

extern u32 FLASH_Vectors_Errors_Get();
extern u32 FLASH_Vectors_Saved_Get();
extern u32 FLASH_Vectors_Recieved_Get();
extern u32 FLASH_Vectors_Rejected_Get();
extern u32 FLASH_Session_Get();


//extern void FLASH_Vectors_Errors_Reset();
//extern void FLASH_Vectors_Saved_Reset();

extern u64 FLASH_Current_Adress_Get();
extern u64 FLASH_Full_Size_Get();
extern u16 FLASH_Chip_Mask_Get();
extern u64 FLASH_Used_Size_Get();

extern bool FLASH_SendStatus(u32 progress, byte status);


//extern i64 FLASH_Empty_Size_Get();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // FLASH_H__09_08_2019__08_17
