#include "common.h"
#include "flash.h"
#include "fram.h"
#include "k9k8g08u.h"
#include "vector.h"


/*****************************************************************************/
flash_status_type		flash_read_status = FLASH_STATUS_WAIT;          // १���� ��᫥���� ����樨
flash_status_type 		flash_write_status = FLASH_STATUS_WAIT;
flash_status_type 		flash_verify_status = FLASH_STATUS_WAIT;

u64 					flash_current_adress;

unsigned int 	       	flash_vectors_errors = 0;
unsigned int 	        flash_vectors_saved = 0;

unsigned short 			flash_save_size = 0;
unsigned char			flash_save_buffer[FLASH_SAVE_BUFFER_SIZE];
unsigned char			flash_save_repeat_counter = 0;
flash_save_repeat_type	flash_save_repeat = FLASH_SAVE_REPEAT_NONE;

unsigned char			flash_read_buffer[FLASH_READ_BUFFER_SIZE];
long long 				flash_read_vector_adress;
unsigned short 			*flash_read_vector_size_p;
bool	 				*flash_read_vector_ready_p;
unsigned char			*flash_read_vector_p;

flash_status_operation_type 	flash_status_operation = FLASH_STATUS_OPERATION_WAIT;
/*****************************************************************************/
unsigned int inline FLASH_Vectors_Errors_Get()
{
	return flash_vectors_errors;
}

unsigned int inline FLASH_Vectors_Saved_Get()
{
	return flash_vectors_saved;
}

void FLASH_Vectors_Errors_Reset()
{
	flash_vectors_errors = 0;
}

void FLASH_Vectors_Saved_Reset()
{
	flash_vectors_saved = 0;
}
/*****************************************************************************/


bool inline FLASH_Read_Is_Ready()
{
	if(flash_read_status == FLASH_STATUS_READY)
	{
		flash_read_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

bool inline FLASH_Read_Is_Error()
{
	if(flash_read_status == FLASH_STATUS_ERROR)
	{
		flash_read_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

bool inline FLASH_Write_Is_Ready()
{
	if(flash_write_status == FLASH_STATUS_READY)
	{
		flash_write_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

bool inline FLASH_Write_Is_Error()
{
	if(flash_write_status == FLASH_STATUS_ERROR)
	{
		flash_write_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

bool inline FLASH_Verify_Is_Ready()
{
	if(flash_verify_status == FLASH_STATUS_READY)
	{
		flash_verify_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

bool inline FLASH_Verify_Is_Error()
{
	if(flash_verify_status == FLASH_STATUS_ERROR)
	{
		flash_verify_status = FLASH_STATUS_WAIT;
		return true;
	}	
	return false;
}

bool FLASH_Busy()
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return true;
	return false;
}
/*****************************************************************************/
u64 FLASH_Current_Adress_Get()
{
	return flash_current_adress;
}

//u64 FLASH_Full_Size_Get()
//{
//	return ((long long)1 << (K9K8G08U_Size_Get().fullSize));
//}

u64 FLASH_Used_Size_Get()
{
	long long size = FLASH_Full_Size_Get();

	if(FRAM_Memory_Start_Adress_Get() != -1)
	{
		size = FRAM_Memory_Current_Adress_Get() - FRAM_Memory_Start_Adress_Get();
		if(size < 0) size += FLASH_Full_Size_Get();
	}
	return size;
}

u64 FLASH_Empty_Size_Get()
{
	return FLASH_Full_Size_Get() - FLASH_Used_Size_Get();
}

unsigned short FLASH_Chip_Mask_Get()
{
	return (K9K8G08U_Size_Get().mask);
}


/************************************************************************************/
// 㤠���� 
bool FLASH_Erase_Full()
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	FRAM_Memory_Start_Adress_Set(FRAM_Memory_Current_Adress_Get());
	return true;
}
/************************************************************************************/
// ����⠭�����
bool FLASH_UnErase_Full()
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	FRAM_Memory_Start_Adress_Set(-1);
	return true;
}
/************************************************************************************/
// �⠥� �� 㪠������� �����
bool FLASH_Read(long long *adr, unsigned char *data, unsigned short size)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	if(!K9K8G08U_Read_Data(adr, data, size)) return false;
	return true;
}

bool FLASH_Read_Vector(long long adr, unsigned short *size, bool *ready, unsigned char **vector)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT)
	{
		return false;
	}
	flash_read_vector_adress = adr;
	flash_read_vector_size_p = size;
	*flash_read_vector_size_p = 0;
	flash_read_vector_ready_p = ready;
        *flash_read_vector_ready_p = false;
        *vector = flash_read_buffer;
	flash_status_operation = FLASH_STATUS_OPERATION_READ_WAIT;
	return true;
}

/************************************************************************************/
// �஢���� �� 㪠������� �����
bool FLASH_Verify(long long *adr, unsigned char *data, unsigned short size)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	if(!K9K8G08U_Verify_Data(adr, data, size)) return false;
	return true;
}

/************************************************************************************/
// ���� �� 㪠������� �����
bool FLASH_Write(long long *adr, unsigned char *data, unsigned short size)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	if(!K9K8G08U_Write_Data(false, adr, data, size)) return false;
	return true;
}
// ���� �� 㪠������� ����� ᬥ���� �� ���� ����, �����頥� ��ࠢ����� ����
bool FLASH_Write_Next(long long *adr, unsigned char *data, unsigned short size)
{
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	if(!K9K8G08U_Write_Data(true, adr, data, size)) return false;
	return true;
}
// ���� �����
bool FLASH_Write_Vector(unsigned short session, unsigned short device, RTC_type rtc, unsigned char flags, unsigned char *data, unsigned short size, flash_save_repeat_type repeat)
{
	if((flash_status_operation != FLASH_STATUS_OPERATION_WAIT) || (size + sizeof(vector_type) > FLASH_SAVE_BUFFER_SIZE))
	{
		flash_vectors_errors ++;
		return false;
	}
	flash_save_size = Vector_Make(session, device, rtc, flags, data, flash_save_buffer, size);
	if(flash_save_size == 0)
	{
		flash_vectors_errors ++;
		return false;
	}
	flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WAIT;
	flash_save_repeat = repeat;
	flash_save_repeat_counter = 0;
	return true;
}
/************************************************************************************/



/*****************************************************************************/

bool FLASH_Reset()
{
	if(!K9K8G08U_Reset()) return false;
	if(flash_status_operation != FLASH_STATUS_OPERATION_WAIT) return false;
	flash_current_adress = FRAM_Memory_Current_Adress_Get();
	K9K8G08U_Adress_Set(flash_current_adress);
	FRAM_Memory_Current_Adress_Set(K9K8G08U_Adress_Get()); // �஢�ઠ
	return true;
}

void FLASH_Init()
{
	FLASH_Reset();
}

void FLASH_Idle()
{

	switch (K9K8G08U_Status_Operation_Get())	// ᤥ���� ⠪, �⮡� ����� �뫮 ��� �㡫�஢��� ������ � ��砥 ��㤠�
	{
		case K9K8G08U_STATUS_OPERATION_WAIT:	// ��� ᪮���
			break;
		case K9K8G08U_STATUS_OPERATION_READ_READY: 
			flash_read_status = FLASH_STATUS_READY;
			K9K8G08U_Status_Operation_Reset();	
			break;
		case K9K8G08U_STATUS_OPERATION_READ_ERROR: 
			flash_read_status = FLASH_STATUS_ERROR;
			K9K8G08U_Status_Operation_Reset();	
			break;
		case K9K8G08U_STATUS_OPERATION_WRITE_READY: 
			flash_write_status = FLASH_STATUS_READY;
			K9K8G08U_Status_Operation_Reset();
			break;
		case K9K8G08U_STATUS_OPERATION_WRITE_ERROR: 
			flash_write_status = FLASH_STATUS_ERROR;
			K9K8G08U_Status_Operation_Reset();	
			break;
		case K9K8G08U_STATUS_OPERATION_VERIFY_READY: 
			flash_verify_status = FLASH_STATUS_READY;
			K9K8G08U_Status_Operation_Reset();
			break;
		case K9K8G08U_STATUS_OPERATION_VERIFY_ERROR: 
			flash_verify_status = FLASH_STATUS_ERROR;
			K9K8G08U_Status_Operation_Reset();	
			break;
	}

	//TestPin2_Set();

	if(K9K8G08U_Status_Operation_Get() == K9K8G08U_STATUS_OPERATION_WAIT) flash_current_adress = K9K8G08U_Adress_Get();	

	//TestPin2_Clr();

	// ������ ����� �����
	long long current_adress_old = FRAM_Memory_Current_Adress_Get();

	if(flash_status_operation == FLASH_STATUS_OPERATION_SAVE_WRITE)
	{
	    if(FLASH_Write_Is_Ready()) 
		{
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			long long adr = FRAM_Memory_Current_Adress_Get();
			if(FLASH_Verify(&adr, flash_save_buffer, flash_save_size))
			{
				flash_status_operation = FLASH_STATUS_OPERATION_SAVE_VERIFY;
			}
			else
			{	
				FRAM_Memory_Current_Adress_Set(FLASH_Current_Adress_Get());
		        flash_vectors_errors ++;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
		};

		if(FLASH_Write_Is_Error())
		{
			FRAM_Memory_Current_Adress_Set(FLASH_Current_Adress_Get());
			flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WAIT;		
		}
	}
	else if(flash_status_operation == FLASH_STATUS_OPERATION_SAVE_VERIFY)
	{
	    if(FLASH_Verify_Is_Ready()) 
		{
			FRAM_Memory_Current_Vector_Adress_Set(FRAM_Memory_Current_Adress_Get());
			FRAM_Memory_Current_Adress_Set(FLASH_Current_Adress_Get());	
	               	flash_vectors_saved ++;
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
		}
		if(FLASH_Verify_Is_Error())
		{
			FRAM_Memory_Current_Adress_Set(FLASH_Current_Adress_Get());
			flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WAIT;		
		}
	};

	//TestPin2_Set();
	
	if(flash_status_operation == FLASH_STATUS_OPERATION_SAVE_WAIT)
	{
		if(flash_save_repeat_counter <= 1)	// ������, ����� �� �.�����
		{
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			long long adr = FRAM_Memory_Current_Adress_Get();
			if(FLASH_Write(&adr, flash_save_buffer, flash_save_size))
			{
				FRAM_Memory_Current_Adress_Set(adr);
				flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WRITE;		
			} 
			else
			{
	                	flash_vectors_errors ++;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
		}
		else if(flash_save_repeat_counter <= flash_save_repeat)
		{
                	flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			long long adr = FRAM_Memory_Current_Adress_Get();
			if(FLASH_Write_Next(&adr, flash_save_buffer, flash_save_size))
			{
				FRAM_Memory_Current_Adress_Set(adr);
				flash_status_operation = FLASH_STATUS_OPERATION_SAVE_WRITE;		
			} 
			else
			{
		               	flash_vectors_errors ++;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
		}
		else
		{
	                flash_vectors_errors ++;
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
		};

		flash_save_repeat_counter ++;
	};

	if(FRAM_Memory_Start_Adress_Get() != -1) 	//check full size
	{
		long long current_adress_new = FRAM_Memory_Current_Adress_Get();
		if(current_adress_old != current_adress_new) // �⮡� �� ����� �� ���
		{
			long long size_old = current_adress_old - FRAM_Memory_Start_Adress_Get();
			if(size_old < 0) size_old += FLASH_Full_Size_Get();
			long long size_new = current_adress_new - FRAM_Memory_Start_Adress_Get();
			if(size_new < 0) size_new += FLASH_Full_Size_Get();
			if(size_new < size_old) FRAM_Memory_Start_Adress_Set(-1);
		}
	};

	//TestPin2_Clr();

	// ������ �⥭�� �����
	if(flash_status_operation == FLASH_STATUS_OPERATION_READ_HEADER)
	{
	    if(FLASH_Read_Is_Ready()) 
		{
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			unsigned short size_vector;
			if(Vector_Check_Header(flash_read_buffer, &size_vector))
			{
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;
				if((size_vector <= FLASH_READ_BUFFER_SIZE) && (FLASH_Read(&flash_read_vector_adress, flash_read_buffer, size_vector)))
				{
					*flash_read_vector_size_p = size_vector;
					flash_status_operation = FLASH_STATUS_OPERATION_READ_DATA;		
				}      	
				else
				{
					*flash_read_vector_size_p = 0;
					*flash_read_vector_ready_p = true;
					flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
				}
			}
			else
			{	
				*flash_read_vector_size_p = 0;
				*flash_read_vector_ready_p = true;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
		}
		if(FLASH_Read_Is_Error())
		{
			*flash_read_vector_size_p = 0;
			*flash_read_vector_ready_p = true;
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
		}
	}
	else if(flash_status_operation == FLASH_STATUS_OPERATION_READ_DATA)
	{
	        if(FLASH_Read_Is_Ready()) 
		{
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			if(Vector_Check_Data(flash_read_buffer))
			{
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;
				*flash_read_vector_ready_p = true;
			}
			else
			{	
				*flash_read_vector_size_p = 0;
				*flash_read_vector_ready_p = true;
				flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
			}
		}
		if(FLASH_Read_Is_Error())
		{
			*flash_read_vector_size_p = 0;
			*flash_read_vector_ready_p = true;
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
		}
	};

	//TestPin2_Set();

	if(flash_status_operation == FLASH_STATUS_OPERATION_READ_WAIT)
	{
		flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
		if(FLASH_Read(&flash_read_vector_adress, flash_read_buffer, sizeof(vector_header_type)))
		{
			*flash_read_vector_size_p = 0;
			*flash_read_vector_ready_p = false;
			flash_status_operation = FLASH_STATUS_OPERATION_READ_HEADER;		
		} 
		else
		{
			*flash_read_vector_size_p = 0;
			*flash_read_vector_ready_p = true;
			flash_status_operation = FLASH_STATUS_OPERATION_WAIT;		
		}
	};

	//TestPin2_Clr();
}
