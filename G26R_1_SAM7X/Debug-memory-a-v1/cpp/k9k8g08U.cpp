#include "common.h"
#include "main.h"
#include "k9k8g08u.h"


#define K9K8G08U_COMMAND_LATCH_CYCLE(cmd) \
{                                    \
	K9K8G08U_BASE_PIO_DATA->PIO_OER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_CLE->PIO_SODR = K9K8G08U_PIO_CLE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = ((cmd & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~cmd) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_CLE->PIO_CODR = K9K8G08U_PIO_CLE;	\
}

#define K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(col, row) \
{                                    \
	K9K8G08U_BASE_PIO_DATA->PIO_OER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_ALE->PIO_SODR = K9K8G08U_PIO_ALE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = ((col & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~col) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = (((col >> 8) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~(col >> 8)) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = ((row & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~row) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = (((row >> 8) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~(row >> 8)) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = (((row >> 16) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~(row >> 16)) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	 \
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_ALE->PIO_CODR = K9K8G08U_PIO_ALE;	\
}

#define K9K8G08U_ADRESS_LATCH_COL_CYCLE(col) \
{                                    \
	K9K8G08U_BASE_PIO_DATA->PIO_OER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_ALE->PIO_SODR = K9K8G08U_PIO_ALE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = ((col & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~col) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = (((col >> 8) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~(col >> 8)) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_ALE->PIO_CODR = K9K8G08U_PIO_ALE;	\
}

#define K9K8G08U_ADRESS_LATCH_ROW_CYCLE(row) \
{                                    \
	K9K8G08U_BASE_PIO_DATA->PIO_OER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_ALE->PIO_SODR = K9K8G08U_PIO_ALE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = ((row & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~row) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = (((row >> 8) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~(row >> 8)) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = (((row >> 16) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~(row >> 16)) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	 \
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_ALE->PIO_CODR = K9K8G08U_PIO_ALE;	\
}

#define K9K8G08U_READ_CYCLE_PREPARE() \
	K9K8G08U_BASE_PIO_DATA->PIO_ODR = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);	\

#define K9K8G08U_READ_CYCLE(data) \
{       				\
	K9K8G08U_BASE_PIO_RE->PIO_CODR = K9K8G08U_PIO_RE;	\
	data = ((K9K8G08U_BASE_PIO_DATA->PIO_PDSR >> K9K8G08U_PIO_DATA_OFFSET) & K9K8G08U_PIO_DATA_MASK); \
	K9K8G08U_BASE_PIO_RE->PIO_SODR = K9K8G08U_PIO_RE;	\
}       

#define K9K8G08U_WRITE_CYCLE_PREPARE() \
	K9K8G08U_BASE_PIO_DATA->PIO_OER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);	\

#define K9K8G08U_WRITE_CYCLE(data) \
{       				\
	K9K8G08U_BASE_PIO_DATA->PIO_SODR = ((data & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (((~data) & K9K8G08U_PIO_DATA_MASK) << K9K8G08U_PIO_DATA_OFFSET);	\
	K9K8G08U_BASE_PIO_WE->PIO_CODR = K9K8G08U_PIO_WE;	\
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;	\
}       

#define K9K8G08U_BUSY() (!(K9K8G08U_BASE_PIO_RB->PIO_PDSR & K9K8G08U_PIO_RB))

void K9K8G08U_Chip_Select(unsigned char chip) 
{                                    
	if(chip == 0)                   
	{ 				
		K9K8G08U_BASE_PIO_CE2->PIO_SODR = K9K8G08U_PIO_CE2;
		K9K8G08U_BASE_PIO_CE->PIO_CODR = K9K8G08U_PIO_CE; 
	}                                                          
	else                                                        
	if(chip == 1)                                                
	{                                                             
		K9K8G08U_BASE_PIO_CE->PIO_SODR = K9K8G08U_PIO_CE;      
		K9K8G08U_BASE_PIO_CE2->PIO_CODR = K9K8G08U_PIO_CE2;     
	}                                                                
	else                                                              
	{                                                                  
		K9K8G08U_BASE_PIO_CE->PIO_SODR = K9K8G08U_PIO_CE;           
		K9K8G08U_BASE_PIO_CE2->PIO_SODR = K9K8G08U_PIO_CE2;          
	}                                                                     
}                                                                              

/*********************************************************************************/
k9k8g08u_status_operation_type k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WAIT;
k9k8g08u_memory_size_type k9k8g08u_memory_size;

unsigned short k9k8g08u_data_col;
unsigned int k9k8g08u_data_row;
unsigned char k9k8g08u_data_chip;
unsigned short k9k8g08u_data_size;
unsigned short k9k8g08u_data_count;
unsigned char *k9k8g08u_data;
unsigned short k9k8g08u_data_verify_errors;
/*********************************************************************************/

const k9k8g08u_memory_size_type& K9K8G08U_Size_Get()
{
	return k9k8g08u_memory_size;
}

void K9K8G08U_Adress_Set(long long a)
{
	while(a < 0) a += k9k8g08u_memory_size.fullSize * (((-a) >> (k9k8g08u_memory_size.fullBits)) + 1);
	a &= k9k8g08u_memory_size.fullSize-1;
	k9k8g08u_data_chip = a >> k9k8g08u_memory_size.chipBits;
	a &= k9k8g08u_memory_size.chipSize-1;
	k9k8g08u_data_row = a >> k9k8g08u_memory_size.pageBits;
	a &= k9k8g08u_memory_size.pageSize-1;
	k9k8g08u_data_col = a;
}

void K9K8G08U_Adress_Set_Next(long long a)
{
	while(a < 0) a += k9k8g08u_memory_size.fullSize * (((-a) >> (k9k8g08u_memory_size.fullBits)) + 1);
	a &= k9k8g08u_memory_size.fullSize-1;
	a += (k9k8g08u_memory_size.blockSize) - (a & (k9k8g08u_memory_size.blockSize-1));
	a &= k9k8g08u_memory_size.fullSize-1;
	k9k8g08u_data_chip = a >> k9k8g08u_memory_size.chipBits;
	a &= k9k8g08u_memory_size.chipSize-1;
	k9k8g08u_data_row = a >> k9k8g08u_memory_size.pageBits;
	k9k8g08u_data_col = 0;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline u64 Mul64(u32 v1, u32 v2)
{
	return (u64)v1*v2;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u64 K9K8G08U_Adress_Get()
{
//	return (k9k8g08u_data_chip * k9k8g08u_memory_size.chipSize) + Mul64(k9k8g08u_data_row, k9k8g08u_memory_size.pageSize) + k9k8g08u_data_col;
	return ((u64)((k9k8g08u_data_chip << k9k8g08u_memory_size.blockBits) + k9k8g08u_data_row) << k9k8g08u_memory_size.pageBits) + k9k8g08u_data_col;
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void K9K8G08U_Status_Operation_Reset()
{
	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WAIT;
}

k9k8g08u_status_operation_type K9K8G08U_Status_Operation_Get()
{
	return k9k8g08u_status_operation;
}

/*********************************************************************************/
bool K9K8G08U_Reset()
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	while(K9K8G08U_BUSY());
	K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_RESET);
	while(K9K8G08U_BUSY());
	return true;
}

bool K9K8G08U_Read_ID(k9k8g08u_id_type *id)
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_ID);
	K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(0, 0);
	char i;
	K9K8G08U_READ_CYCLE_PREPARE();
	for(i = 0; i < sizeof(k9k8g08u_id_type); i++) 
	{ 
		K9K8G08U_READ_CYCLE(((unsigned char *)id)[i]);
	}
	return true;
}

bool K9K8G08U_Read_Data(long long *adr, unsigned char *data, unsigned short size)
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	K9K8G08U_Adress_Set(*adr);	
	k9k8g08u_data_size = size;
	k9k8g08u_data_count = 0;
	k9k8g08u_data = data;
	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_READ_START;
	return true;
}

bool K9K8G08U_Verify_Data(long long *adr, unsigned char *data, unsigned short size)
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	K9K8G08U_Adress_Set(*adr);	
	k9k8g08u_data_size = size;
	k9k8g08u_data_count = 0;
	k9k8g08u_data = data;
	k9k8g08u_data_verify_errors = 0;
	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_START;
	return true;
}

bool K9K8G08U_Write_Data(bool next, long long *adr, unsigned char *data, unsigned short size)
{
	if(k9k8g08u_status_operation != K9K8G08U_STATUS_OPERATION_WAIT) return false;
	if(next)
	{
		K9K8G08U_Adress_Set_Next(*adr);
		*adr = K9K8G08U_Adress_Get();
	}
	else K9K8G08U_Adress_Set(*adr);
	k9k8g08u_data_size = size;
	k9k8g08u_data_count = 0;
	k9k8g08u_data = data;         
	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_START;
	return true;
}
/*********************************************************************************/
bool K9K8G08U_Idle()
{
	if(K9K8G08U_BUSY()) return false;
        
	switch (k9k8g08u_status_operation)
	{
		case K9K8G08U_STATUS_OPERATION_WAIT: 
			break;
		case K9K8G08U_STATUS_OPERATION_READ_START:
			K9K8G08U_Chip_Select(k9k8g08u_data_chip);
		        K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_1);
			K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(k9k8g08u_data_col, k9k8g08u_data_row);
			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_2);
			k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_READ_IDLE;
			break;
		case K9K8G08U_STATUS_OPERATION_READ_IDLE:	
			nop();
			unsigned short x_r_i;
			if(k9k8g08u_data_size - k9k8g08u_data_count > K9K8G08U_READ_PACK_BYTES) x_r_i = K9K8G08U_READ_PACK_BYTES; else x_r_i = k9k8g08u_data_size - k9k8g08u_data_count;
			if((x_r_i + k9k8g08u_data_col) > k9k8g08u_memory_size.pageSize) x_r_i = k9k8g08u_memory_size.pageSize - k9k8g08u_data_col;
			unsigned short i_r_i = k9k8g08u_data_count;      // сделано для того чтобы вче переменные вращались по регистрам, не заходя в память. так реально бысрее
			unsigned short e_r_i = i_r_i + x_r_i;
			K9K8G08U_READ_CYCLE_PREPARE();
			while(i_r_i < e_r_i)
			{ 
				K9K8G08U_READ_CYCLE(k9k8g08u_data[i_r_i]);
				i_r_i ++;
			}
			k9k8g08u_data_count = i_r_i;
			k9k8g08u_data_col += x_r_i;
			k9k8g08u_data_col &= k9k8g08u_memory_size.pageSize-1;
			if(k9k8g08u_data_col == 0) 
			{
				k9k8g08u_data_row ++;
				k9k8g08u_data_row &= (1 << (k9k8g08u_memory_size.chipBits - k9k8g08u_memory_size.pageBits))-1;
				if(k9k8g08u_data_row == 0)
				{
					do
					{
						k9k8g08u_data_chip ++;	
						k9k8g08u_data_chip %= K9K8G08U_MAX_CHIP;
					} while(!(k9k8g08u_memory_size.mask & (1 << k9k8g08u_data_chip)));
				}
				if(k9k8g08u_data_size > k9k8g08u_data_count) k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_READ_START;
			}
			if(k9k8g08u_data_size <= k9k8g08u_data_count) k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_READ_READY;
			break;
			// дочитываем до конца, чтобы адрес попал куда надо
		case K9K8G08U_STATUS_OPERATION_VERIFY_START:
			K9K8G08U_Chip_Select(k9k8g08u_data_chip);
		        K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_1);
			K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(k9k8g08u_data_col, k9k8g08u_data_row);
			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_2);
			k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_IDLE;
			break; 
		case K9K8G08U_STATUS_OPERATION_VERIFY_IDLE:	
			nop();
			unsigned short x_v_i;
			if(k9k8g08u_data_size - k9k8g08u_data_count > K9K8G08U_READ_PACK_BYTES) x_v_i = K9K8G08U_READ_PACK_BYTES; else x_v_i = k9k8g08u_data_size - k9k8g08u_data_count;
			if((x_v_i + k9k8g08u_data_col) > k9k8g08u_memory_size.pageSize) x_v_i = k9k8g08u_memory_size.pageSize - k9k8g08u_data_col;
			unsigned short i_v_i = k9k8g08u_data_count;       // сделано для того чтобы вче переменные вращались по регистрам, не заходя в память. так реально бысрее
			unsigned short e_v_i = i_v_i + x_v_i;
			unsigned char s_v_i;
			unsigned short q_v_i = k9k8g08u_data_verify_errors;
			K9K8G08U_READ_CYCLE_PREPARE();
			while(i_v_i < e_v_i)
			{ 
                		K9K8G08U_READ_CYCLE(s_v_i);
				if(s_v_i != k9k8g08u_data[i_v_i]) q_v_i ++;
				i_v_i ++;
			}
			k9k8g08u_data_count = i_v_i;
			k9k8g08u_data_verify_errors = q_v_i;
			k9k8g08u_data_col += x_v_i;
			k9k8g08u_data_col &= k9k8g08u_memory_size.pageSize-1;
			if(k9k8g08u_data_col == 0) 
			{
				k9k8g08u_data_row ++;
				k9k8g08u_data_row &= (1 << (k9k8g08u_memory_size.chipBits - k9k8g08u_memory_size.pageBits))-1;
				if(k9k8g08u_data_row == 0)
				{
					do
					{
						k9k8g08u_data_chip ++;	
						k9k8g08u_data_chip %= K9K8G08U_MAX_CHIP;
					} while(!(k9k8g08u_memory_size.mask & (1 << k9k8g08u_data_chip)));
				}
				if(k9k8g08u_data_size > k9k8g08u_data_count) k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_START;
			}
			if(k9k8g08u_data_size <= k9k8g08u_data_count)
			{
				if(k9k8g08u_data_verify_errors) k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_ERROR;
				else k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_VERIFY_READY;
			}
			break;

		case K9K8G08U_STATUS_OPERATION_WRITE_START:
			K9K8G08U_Chip_Select(k9k8g08u_data_chip);
			if(k9k8g08u_data_col == 0 && (k9k8g08u_data_row & ((1 << (k9k8g08u_memory_size.blockBits - k9k8g08u_memory_size.pageBits))-1)) == 0)	// новый блок
			{
				K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_BLOCK_ERASE_1);
				K9K8G08U_ADRESS_LATCH_ROW_CYCLE(k9k8g08u_data_row);
				K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_BLOCK_ERASE_2);
	                 	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_ERASE_CHECK;
				break;
			}
		case K9K8G08U_STATUS_OPERATION_WRITE_START_ERASED:
			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_PAGE_PROGRAM_1);
			K9K8G08U_ADRESS_LATCH_COL_ROW_CYCLE(k9k8g08u_data_col, k9k8g08u_data_row);
			k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_IDLE;
			//break; // не надо, т.к. можно обработать сразу IDLE (START_ERASED не занимает много времени)
		case K9K8G08U_STATUS_OPERATION_WRITE_IDLE:
			nop();
			unsigned short x_w_i;
			if(k9k8g08u_data_size - k9k8g08u_data_count > K9K8G08U_WRITE_PACK_BYTES) x_w_i = K9K8G08U_WRITE_PACK_BYTES; else x_w_i = k9k8g08u_data_size - k9k8g08u_data_count;
			if((x_w_i + k9k8g08u_data_col) > k9k8g08u_memory_size.pageSize) x_w_i = k9k8g08u_memory_size.pageSize - k9k8g08u_data_col;
			unsigned short i_w_i = k9k8g08u_data_count;   // сделано для того чтобы вче переменные вращались по регистрам, не заходя в память. так реально бысрее
			unsigned short e_w_i = i_w_i + x_w_i;
			K9K8G08U_WRITE_CYCLE_PREPARE();
			while(i_w_i < e_w_i)
			{ 
				K9K8G08U_WRITE_CYCLE(k9k8g08u_data[i_w_i]);
				i_w_i ++;
			}
			k9k8g08u_data_count = i_w_i;
			k9k8g08u_data_col += x_w_i;
			k9k8g08u_data_col &= k9k8g08u_memory_size.pageSize-1;
			if(k9k8g08u_data_col == 0)  //end of page
			{
				k9k8g08u_data_row ++;
				k9k8g08u_data_row &= (1 << (k9k8g08u_memory_size.chipBits - k9k8g08u_memory_size.pageBits))-1;
				if(k9k8g08u_data_row == 0)
				{
					do
					{
						k9k8g08u_data_chip ++;	
						k9k8g08u_data_chip %= K9K8G08U_MAX_CHIP;
					} while(!(k9k8g08u_memory_size.mask & (1 << k9k8g08u_data_chip)));
				}
				K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_PAGE_PROGRAM_2);
				k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_CHECK;
			}
			if(k9k8g08u_data_size <= k9k8g08u_data_count) 
			{
				k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_CHECK;
				K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_PAGE_PROGRAM_2);
			}
			break;
		case K9K8G08U_STATUS_OPERATION_WRITE_CHECK:
			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_STATUS);
			unsigned char s_w_c;
			K9K8G08U_READ_CYCLE_PREPARE();
			K9K8G08U_READ_CYCLE(s_w_c);
			if(s_w_c & 0x01)	// сбой
			{
                        	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_ERROR;	
			}
			else
			{
		        	if(k9k8g08u_data_size <= k9k8g08u_data_count)
				{
					k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_READY; 
				}
				else 
				{
					k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_START;
				}
			} 
			break;
		case K9K8G08U_STATUS_OPERATION_WRITE_ERASE_CHECK:
			K9K8G08U_COMMAND_LATCH_CYCLE(K9K8G08U_COMMAND_READ_STATUS);
			unsigned char s_e_c;
			K9K8G08U_READ_CYCLE_PREPARE();
			K9K8G08U_READ_CYCLE(s_e_c);
			if(s_e_c & 0x01)	// сбой
			{
                        	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_ERROR;	
			}
			else
			{
		        	k9k8g08u_status_operation = K9K8G08U_STATUS_OPERATION_WRITE_START_ERASED; 
			} 
			break;
	}
	return true;
}

void K9K8G08U_Init()
{
	K9K8G08U_BASE_PIO_RB->PIO_PER = K9K8G08U_PIO_RB;
	K9K8G08U_BASE_PIO_RB->PIO_ODR = K9K8G08U_PIO_RB;
	K9K8G08U_BASE_PIO_RB->PIO_PPUER = K9K8G08U_PIO_RB;

	K9K8G08U_BASE_PIO_RE->PIO_PER = K9K8G08U_PIO_RE;
	K9K8G08U_BASE_PIO_RE->PIO_OER = K9K8G08U_PIO_RE;
	K9K8G08U_BASE_PIO_RE->PIO_OWER = K9K8G08U_PIO_RE;
	K9K8G08U_BASE_PIO_RE->PIO_SODR = K9K8G08U_PIO_RE;

	K9K8G08U_BASE_PIO_CE->PIO_PER = K9K8G08U_PIO_CE;
	K9K8G08U_BASE_PIO_CE->PIO_OER = K9K8G08U_PIO_CE;
	K9K8G08U_BASE_PIO_CE->PIO_OWER = K9K8G08U_PIO_CE;
	K9K8G08U_BASE_PIO_CE->PIO_SODR = K9K8G08U_PIO_CE;

	K9K8G08U_BASE_PIO_CE2->PIO_PER = K9K8G08U_PIO_CE2;
	K9K8G08U_BASE_PIO_CE2->PIO_OER = K9K8G08U_PIO_CE2;
	K9K8G08U_BASE_PIO_CE2->PIO_OWER = K9K8G08U_PIO_CE2;
	K9K8G08U_BASE_PIO_CE2->PIO_SODR = K9K8G08U_PIO_CE2;

	K9K8G08U_BASE_PIO_CLE->PIO_PER = K9K8G08U_PIO_CLE;
	K9K8G08U_BASE_PIO_CLE->PIO_OER = K9K8G08U_PIO_CLE;
	K9K8G08U_BASE_PIO_CLE->PIO_OWER = K9K8G08U_PIO_CLE;
	K9K8G08U_BASE_PIO_CLE->PIO_CODR = K9K8G08U_PIO_CLE;

	K9K8G08U_BASE_PIO_ALE->PIO_PER = K9K8G08U_PIO_ALE;
	K9K8G08U_BASE_PIO_ALE->PIO_OER = K9K8G08U_PIO_ALE;
	K9K8G08U_BASE_PIO_ALE->PIO_OWER = K9K8G08U_PIO_ALE;
	K9K8G08U_BASE_PIO_ALE->PIO_CODR = K9K8G08U_PIO_ALE;

	K9K8G08U_BASE_PIO_WE->PIO_PER = K9K8G08U_PIO_WE;
	K9K8G08U_BASE_PIO_WE->PIO_OER = K9K8G08U_PIO_WE;
	K9K8G08U_BASE_PIO_WE->PIO_OWER = K9K8G08U_PIO_WE;
	K9K8G08U_BASE_PIO_WE->PIO_SODR = K9K8G08U_PIO_WE;

	K9K8G08U_BASE_PIO_WP->PIO_PER = K9K8G08U_PIO_WP;
	K9K8G08U_BASE_PIO_WP->PIO_OER = K9K8G08U_PIO_WP;
	K9K8G08U_BASE_PIO_WP->PIO_OWER = K9K8G08U_PIO_WP;
	K9K8G08U_BASE_PIO_WP->PIO_SODR = K9K8G08U_PIO_WP;

	K9K8G08U_BASE_PIO_DATA->PIO_PER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);
	K9K8G08U_BASE_PIO_DATA->PIO_OER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);
	K9K8G08U_BASE_PIO_DATA->PIO_OWER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);
	K9K8G08U_BASE_PIO_DATA->PIO_CODR = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);
	K9K8G08U_BASE_PIO_DATA->PIO_PPUER = (K9K8G08U_PIO_DATA_MASK << K9K8G08U_PIO_DATA_OFFSET);
	
	k9k8g08u_memory_size.pageBits = 0;
	k9k8g08u_memory_size.blockBits = 0;
	k9k8g08u_memory_size.chipBits = 0;
	k9k8g08u_memory_size.fullBits = 0;
	k9k8g08u_memory_size.mask = 0;

	for(unsigned char chip = 0; chip < K9K8G08U_MAX_CHIP; chip ++)
	{
		K9K8G08U_Chip_Select(chip);
		K9K8G08U_Reset();
		k9k8g08u_id_type k9k8g08u_id;
		K9K8G08U_Read_ID(&k9k8g08u_id);

		if((k9k8g08u_id.marker == 0xEC) && (k9k8g08u_id.device == 0xD3))
		{
			k9k8g08u_memory_size.pageBits = ((k9k8g08u_id.data[1] >> 0) & 0x03) + 10;
			k9k8g08u_memory_size.blockBits = ((k9k8g08u_id.data[1] >> 4) & 0x03) + 16;
			k9k8g08u_memory_size.chipBits = (((k9k8g08u_id.data[2] >> 4) & 0x07) + 23) + (((k9k8g08u_id.data[2] >> 2) & 0x03) + 0);

			if(k9k8g08u_memory_size.fullBits) k9k8g08u_memory_size.fullBits ++; else k9k8g08u_memory_size.fullBits = k9k8g08u_memory_size.chipBits;

			k9k8g08u_memory_size.mask |= (1 << chip);
		};
	};

	k9k8g08u_memory_size.pageSize = 1 << k9k8g08u_memory_size.pageBits;
	k9k8g08u_memory_size.blockSize = 1 << k9k8g08u_memory_size.blockBits;
	k9k8g08u_memory_size.chipSize = ((u64)1) << k9k8g08u_memory_size.chipBits; 
	k9k8g08u_memory_size.fullSize = ((u64)1) << k9k8g08u_memory_size.fullBits; 
	
	K9K8G08U_Chip_Select(0); ///пока один
		
}

