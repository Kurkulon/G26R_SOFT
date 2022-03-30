#ifndef MEMORY_AT91SAM7X256_VECTOR_H
#define MEMORY_AT91SAM7X256_VECTOR_H

#include "rtc.h"

#define VECTOR_MAGIC		0xAA
#define VECTOR_VERSION		2
#define VECTOR_TYPE_DATA	1

typedef struct __attribute__ ((packed))   
{
	unsigned char magic;
	unsigned char version; // версия структуры 
	unsigned char type;
	unsigned char crc_header; // контрольная сумма header_type, исключая crc
	unsigned char crc_data; // контрольная сумма данных
	unsigned short size; // длина пакета в байтах включая header и control
} vector_header_type;

typedef struct __attribute__ ((packed))   
{
	unsigned char hi;
 	unsigned int lo;	//адрес предыдущего вектора
} vector_adress_type;

typedef struct __attribute__ ((packed))   
{
	unsigned short id_session;
	unsigned short id_device;
	RTC_type rtc;		
	vector_adress_type adress_vector;	// адрес предыдущего вектора
	vector_adress_type adress_session;	// адрес начала сессии
	unsigned char flags;	// erorr  т.п	
} vector_control_type;

typedef struct __attribute__ ((packed))   
{
	vector_header_type header;
	vector_control_type control;
} vector_type;
/*****************************************************************************/
extern void Vector_Init();
extern void Vector_Reset();
extern void Vector_Idle();
/*****************************************************************************/
extern unsigned short Vector_Make(unsigned short session, 
	  		 	  unsigned short device, 
				  RTC_type rtc,
		 		  unsigned char flags, 
		 		  unsigned char *in, 
 		 		  unsigned char *out, 
		 		  unsigned short size);
extern bool Vector_Check_Header(unsigned char *in, 
			 unsigned short *size);
extern bool Vector_Check_Data(unsigned char *in);

extern unsigned short Vector_Get_ID_Session(unsigned char *in);
extern unsigned short Vector_Get_ID_Device(unsigned char *in);
extern unsigned char Vector_Get_Flags(unsigned char *in);
extern void Vector_Get_RTC(unsigned char *in, RTC_type *rtc);
extern long long Vector_Get_Adress_Vector(unsigned char *in);
extern long long Vector_Get_Adress_Session(unsigned char *in);
extern unsigned short Vector_Get_Data(unsigned char *in, unsigned char **pdata);


#endif
