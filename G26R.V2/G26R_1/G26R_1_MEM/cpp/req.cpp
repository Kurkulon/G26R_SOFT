#include "req.h"

#include "CRC16.h"
#include "time.h"
#include "hardware.h"

//#pragma O3
//#pragma Otime

//List<R01> R01::freeList;
//static R01 r02[8];

//List< PtrItem<REQ> > PtrItem<REQ>::_freeList;
template <class T> List< PtrItem<T> > PtrItem<T>::_freeList;
static REQ reqArray[8];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void REQ::_FreeCallBack() 

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void RequestQuery::Add(REQ* req)
//{
//	if (req != 0)
//	{
//		req->ready = false;
//
//		if (_first == 0)
//		{
//			_first = _last = req;
//		}
//		else
//		{
//			_last->next = req;
//			_last = req;
//		};
//
//		req->next = 0;
//
//		count++;
//	};
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//REQ* RequestQuery::Get()
//{
//	REQ* r = _first;
//
//	if (_first != 0)
//	{
//		_first = _first->next;
//		r->next = 0;
//
//		count--;
//	};
//
//	return r;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RequestQuery::Update()
{
//	static RTM32 tm;

	enum { WAIT = 0, UPDATE_CRC_1, UPDATE_CRC_2, UPDATE_CRC_3, WRITE, READ, CHECK_CRC_1, CHECK_CRC_2, CALLBACK };

	switch(_state)
	{
		case WAIT:

			if (_run)
			{
				_req = Get();

				if (_req.Valid()) _state = UPDATE_CRC_1;

				if ((_req->updateCRC || _req->checkCRC) && _req->crcType >= _req->CRC16_NOT_VALID)
				{
					_req->updateCRC = false;
					_req->checkCRC = false;
				};
			};

			break;

		case UPDATE_CRC_1:

			if (_req->wb.data != 0 && _req->wb.len != 0)
			{
				if (_req->updateCRC)
				{
					_crc = 0xFFFF;
					_crcLen = _req->wb.len;
					_crcPtr = (byte*)_req->wb.data;

					_state = (_req->crcType == _req->CRC16) ? UPDATE_CRC_3 : UPDATE_CRC_2;
				}
				else
				{
					com->Write(&_req->wb);
					_state = WRITE;
				};
			}
			else
			{
				_state = 0;
			}; 

			break;

		case UPDATE_CRC_2:

			if (CRC_CCITT_DMA_Async(_crcPtr, _crcLen, _crc))
			{
				_state = UPDATE_CRC_3;
			};

			break;

		case UPDATE_CRC_3:
		{
			if (_req->crcType == _req->CRC16)
			{
				u16 len = 105;

				if (_crcLen < len) len = _crcLen;

				_crcLen -= len;

				_crc = GetCRC16(_crcPtr, len, _crc, 0);

				_crcPtr += len;
			}
			else if (CRC_CCITT_DMA_CheckComplete(&_crc))
			{
				_crcPtr += _crcLen;
				_crcLen = 0;	
			};

			if (_crcLen == 0)
			{
				DataPointer p(_crcPtr);
				*p.w = _crc;
				_req->wb.len += 2;
				_req->updateCRC = false;

				com->Write(&_req->wb);
				_state = WRITE;
			};

			break;
		};

		case WRITE:

			if (!com->Update())
			{
				if (_req->rb.data != 0 && _req->rb.maxLen != 0)
				{
					com->Read(&_req->rb, _req->preTimeOut, _req->postTimeOut); 

					_state = READ;
				}
				else
				{
					_state = CALLBACK;
				};
			};

			break;

		case READ:

			if (!com->Update())
			{
				if (_req->checkCRC && _req->rb.recieved)
				{
					_crc = 0xFFFF;
					_crcLen = _req->rb.len;
					_crcPtr = (byte*) _req->rb.data;

					_state = (_req->crcType == _req->CRC16) ? CHECK_CRC_2 : CHECK_CRC_1;
				}
				else
				{
					_req->crcOK = false;
					_state = CALLBACK;
				};
			};

			break;

		case CHECK_CRC_1:

			if (CRC_CCITT_DMA_Async(_crcPtr, _crcLen, _crc))
			{
				_state = CHECK_CRC_2;
			};

		case CHECK_CRC_2:

			if (_req->crcType == _req->CRC16)
			{
				u16 len = 105;

				if (_crcLen < len) len = _crcLen;

				_crcLen -= len;

				_crc = GetCRC16(_crcPtr, len, _crc, 0);

				_crcPtr += len;
			}
			else if (CRC_CCITT_DMA_CheckComplete(&_crc))
			{
				_crcLen = 0;	
			};

			if (_crcLen == 0)
			{
				_req->crcOK = _crc == 0;
				_state = CALLBACK;
			};

			break;

		case CALLBACK:

			if (_req->CallBack != 0)
			{
				_req->ready = _req->CallBack(_req);
			}
			else
			{
				_req->ready = true;
			};

			_state = WAIT;

			break;

		default:

			_state = WAIT;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
