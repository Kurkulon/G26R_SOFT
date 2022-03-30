#include "common.h"     
#include "main.h"
#include "manchester.h"

/*****************************************************************************/

unsigned char manchester_tx_bit = 0;
unsigned short manchester_tx_data = 0;
bool manchester_tx_command = false;
bool manchester_tx_parity = true;
unsigned short manchester_tx_data_temp = 0;
bool manchester_tx_command_temp = false;
bool manchester_tx_parity_temp = true;

manchester_tx_status_type manchester_tx_status = MANCHESTER_TX_STATUS_WAIT;
unsigned short *manchester_tx_data_buffer;
unsigned short 	manchester_tx_data_count = 0;
unsigned short 	manchester_tx_data_size = 0;
unsigned char 	manchester_tx_flags = 0;

//#pragma push
//#pragma arm

inline void Manchester_TX_Set_Data(unsigned short data)
{
	manchester_tx_data = data;
}

inline void Manchester_TX_Set_Command(bool command)
{
	manchester_tx_command = command;			
}

inline void Manchester_TX_Set_Parity(bool parity)
{
	manchester_tx_parity = parity;			
}
	
manchester_tx_status_type Manchester_TX_Parse()
{
	bool out = false;
	if(manchester_tx_bit < 3) 
	{ 
		manchester_tx_parity_temp = manchester_tx_parity;
		manchester_tx_command_temp = manchester_tx_command;
		manchester_tx_data_temp = manchester_tx_data;
		out = manchester_tx_command_temp; 
	}
	else
	if(manchester_tx_bit < 6) 
	{
		out = !manchester_tx_command_temp; 
	}
	else
        if(manchester_tx_bit < 6 + 16 * 2)
	{
		if(manchester_tx_data_temp & 0x8000) 
		{ 
			out = !(manchester_tx_bit & 1);
			if(manchester_tx_bit & 1) 
			{ 
				manchester_tx_parity_temp ^= 1; 
				manchester_tx_data_temp <<= 1; 
			}
		}
		else 
		{
			out = (manchester_tx_bit & 1);
			if(manchester_tx_bit & 1) 
			{ 
				manchester_tx_data_temp <<= 1; 
			}
		}
	} 
	else
	if(manchester_tx_bit < 6 + 16 * 2 + 2) 
	{
		if(manchester_tx_parity_temp) 
		{ 
			out = !(manchester_tx_bit & 1);
		}
		else 
		{
			out = (manchester_tx_bit & 1);
		}
	}
	else
	{
		manchester_tx_bit = 0;
		return MANCHESTER_TX_STATUS_ME;
	}
	manchester_tx_bit ++;
	if(!out)
	{
		return MANCHESTER_TX_STATUS_LO;
	}
	else
	{
		return MANCHESTER_TX_STATUS_HI;
	}
}

bool inline Manchester_TX_Busy()
{
	if((manchester_tx_status != MANCHESTER_TX_STATUS_WAIT) && (manchester_tx_status != MANCHESTER_TX_STATUS_READY)) return true;
	return false;
}

//#pragma pop

//-------------------------------------------

__irq void Manchester_TX_Isr()
{
	unsigned int status = AT91C_BASE_TC2->TC_SR;  	//clear Status Register

    if(AT91C_BASE_TC2->TC_CMR & AT91C_TC_WAVE) //compare
	{	
		if((status & AT91C_TC_CPCS) && (manchester_tx_status != MANCHESTER_TX_STATUS_WAIT))
		{
			if(manchester_tx_status == MANCHESTER_TX_STATUS_HI)
			{                 
				if(manchester_tx_flags & (1 << MANCHESTER_FLAG_INVERSE_POLARITY_BIT)) 
				{
					AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA1 | AT91C_PIO_PA2;
				}
				else
				{
					AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA1 | AT91C_PIO_PA2;
				}
			}
			else if(manchester_tx_status == MANCHESTER_TX_STATUS_LO)
			{
				if(manchester_tx_flags & (1 << MANCHESTER_FLAG_INVERSE_POLARITY_BIT)) 
				{
					AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA1| AT91C_PIO_PA2;
				}
				else
				{
					AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA1| AT91C_PIO_PA2;
				}
			}
			else if(manchester_tx_status == MANCHESTER_TX_STATUS_ME)
			{
				AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA1;
				AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA2;
		              	if(manchester_tx_data_count <= manchester_tx_data_size)
				{
					Manchester_TX_Set_Data(((manchester_tx_type *)manchester_tx_data_buffer)[manchester_tx_data_count].data);
					if(manchester_tx_flags & (1 << MANCHESTER_FLAG_INVERSE_COMMAND_BIT)) Manchester_TX_Set_Command(manchester_tx_data_count);
					else Manchester_TX_Set_Command(!manchester_tx_data_count);
					if(manchester_tx_flags & (1 << MANCHESTER_FLAG_INVERSE_PARITY_BIT)) Manchester_TX_Set_Parity(false);
					else Manchester_TX_Set_Parity(true);
					manchester_tx_data_count ++;
				} else manchester_tx_status = MANCHESTER_TX_STATUS_READY;
			};

			if(manchester_tx_data_count <= manchester_tx_data_size)
			{
				manchester_tx_status = Manchester_TX_Parse();
			}
		}
	};

	AT91C_BASE_AIC->AIC_EOICR = AT91C_BASE_TC2->TC_SR;  	//clear Status Register  на сулчай прихода сверхкоротких импульсов после срабатывания CPCS прихода LDRAS, это штатная ситуация
}       

//----------------------------------------------------------
void Manchester_TX_Start(unsigned char flags, unsigned int freq, unsigned short size, unsigned short *data)
{
	manchester_tx_status = MANCHESTER_TX_STATUS_ME;
	manchester_tx_data_count = 0;
	manchester_tx_data_size = size;
	manchester_tx_data_buffer = data;
	manchester_tx_flags = flags;
	AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKDIS; // Disable the Clock Counter 
	AT91F_TC_InterruptDisable(AT91C_BASE_TC2, 0xFFFFFFFF);
	unsigned int dummy = AT91C_BASE_TC2->TC_SR; //clear Status Register
	AT91C_BASE_TC2->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK | AT91C_TC_WAVE | AT91C_TC_WAVESEL_UP_AUTO;	
	AT91C_BASE_TC2->TC_RC = ((CLOCK_MCK / 2) / freq) / 2;
	AT91F_TC_InterruptEnable(AT91C_BASE_TC2, AT91C_TC_CPCS); // Validate the RC compare interrupt 
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC2);
	AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKEN;
	AT91C_BASE_TC2->TC_CCR = AT91C_TC_SWTRG;
}

void Manchester_TX_Stop()
{
	AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKDIS; // Disable the Clock Counter 
	AT91F_TC_InterruptDisable(AT91C_BASE_TC2, 0xFFFFFFFF);
	AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_TC2);
	unsigned int dummy = AT91C_BASE_TC2->TC_SR; //clear Status Register
	manchester_tx_status = MANCHESTER_TX_STATUS_WAIT;
}

bool Manchester_TX_Ready()
{
	if(manchester_tx_status == MANCHESTER_TX_STATUS_READY) return true;
	return false;
}

bool Manchester_TX_Error()
{
	return false;
}


bool Manchester_TX_Stopped()
{
	if(manchester_tx_status == MANCHESTER_TX_STATUS_WAIT) return true;
	return false;
}

/*******************************************************************/
bool manchester_rx_state = false;
unsigned char manchester_rx_number = 0;
unsigned char manchester_rx_length = 0;
bool manchester_rx_sync = false;
unsigned short manchester_rx_data_temp = 0;
unsigned short manchester_rx_data = 0;
bool manchester_rx_command_temp = false;
bool manchester_rx_command = false;
bool manchester_rx_parity_temp = false;
bool manchester_rx_parity = false;
unsigned short manchester_rx_bit = 0;
unsigned short manchester_rx_bit_1_4 = 0;
unsigned short manchester_rx_bit_3_4 = 0;
unsigned short manchester_rx_bit_5_4 = 0;
unsigned short manchester_rx_bit_7_4 = 0;
unsigned short manchester_rx_bit_9_4 = 0;



inline bool Manchester_RX_Get_Type()
{
       	return manchester_rx_command;
}

inline void Manchester_RX_Set_Parity(bool parity)
{
	manchester_rx_parity = parity;			
}

inline void Manchester_RX_Set_Bit(unsigned short bit)
{
	manchester_rx_bit = bit;			
	manchester_rx_bit_1_4 = (1 * manchester_rx_bit) / 4;
	manchester_rx_bit_3_4 = (3 * manchester_rx_bit) / 4;
	manchester_rx_bit_5_4 = (5 * manchester_rx_bit) / 4;
	manchester_rx_bit_7_4 = (7 * manchester_rx_bit) / 4;
	manchester_rx_bit_9_4 = (9 * manchester_rx_bit) / 4;
}

//#pragma push
//#pragma arm

inline uint16_t Manchester_RX_Get_Data()
{
       	return manchester_rx_data;
}

manchester_rx_status_type Manchester_RX_Parse(unsigned short length)
{	
	manchester_rx_state = !manchester_rx_state;
	if((length < manchester_rx_bit_1_4) || (length > manchester_rx_bit_9_4))
	{
		manchester_rx_number = 0;
		manchester_rx_length = 0;
		manchester_rx_sync = false;
		return MANCHESTER_RX_STATUS_HANDLE_ERROR_LENGTH;
	}
	else
	if(length <= manchester_rx_bit_5_4)
	{	
		manchester_rx_length ++;
		if(length > manchester_rx_bit_3_4) manchester_rx_length ++;
	}
	else
	if(length <= manchester_rx_bit_9_4)
	{
		manchester_rx_sync = true;
		manchester_rx_data_temp = 0;
		manchester_rx_parity_temp = manchester_rx_parity;
		manchester_rx_number = 0;
		manchester_rx_length = 1;
		if(length > manchester_rx_bit_7_4) manchester_rx_length = 2;
		manchester_rx_command = !manchester_rx_state;
	}
	if((manchester_rx_length == 2) && (manchester_rx_sync))
	{
  		if(manchester_rx_number < 16)
		{
			manchester_rx_data_temp <<= 1;
			manchester_rx_data_temp |= manchester_rx_state;
			manchester_rx_parity_temp ^= manchester_rx_state;
			manchester_rx_number ++;
			manchester_rx_length = 0;
		}
	 	else
		{
			manchester_rx_data = manchester_rx_data_temp;
			manchester_rx_data_temp = 0;
			manchester_rx_number = 0;
			manchester_rx_length = 0;
			manchester_rx_sync = false;
			if(manchester_rx_state != manchester_rx_parity_temp)
			{
				manchester_rx_data = (~manchester_rx_data);
				manchester_rx_state = !manchester_rx_state;
				manchester_rx_command = !manchester_rx_command;
			}
			return MANCHESTER_RX_STATUS_HANDLE_READY;
		}
	}
	if(manchester_rx_length >= 3)
	{
		manchester_rx_number = 0;
		manchester_rx_length = 0;
		manchester_rx_sync = false;
		return MANCHESTER_RX_STATUS_HANDLE_ERROR_STRUCT;
	}
	return MANCHESTER_RX_STATUS_HANDLE;
}

//#pragma pop

/*******************************************************************/
unsigned int	manchester_rx_time = 0;
unsigned int	manchester_rx_pause = 0;
unsigned int	manchester_rx_timeout = 0;
manchester_rx_status_type manchester_rx_status = MANCHESTER_RX_STATUS_WAIT;
unsigned short	*manchester_rx_data_buffer;
unsigned short 	manchester_rx_data_count = 0;
unsigned short 	manchester_rx_data_size = 0;
unsigned short 	manchester_rx_data_max_size = 0;
unsigned char 	manchester_rx_flags = 0;

__irq void Manchester_RX_Isr()
{
	unsigned int status = AT91C_BASE_TC1->TC_SR;  	//clear Status Register
    
	if(!(AT91C_BASE_TC1->TC_CMR & AT91C_TC_WAVE)) // capture
	{	
		if(status & AT91C_TC_CPCS)
		{
			if(!Manchester_TX_Busy())
			{
				if(manchester_rx_time) manchester_rx_time --;
				if(!manchester_rx_time)
				{
					if(manchester_rx_data_count >= manchester_rx_data_size)
					{
						manchester_rx_status = MANCHESTER_RX_STATUS_READY;
					}
					else
					{
						manchester_rx_status = MANCHESTER_RX_STATUS_ERROR;
					}				 	
				}      	
			}
		}
		else
		if(status & (AT91C_TC_LDRAS | AT91C_TC_LOVRS))
		if(!Manchester_TX_Busy())
                if((manchester_rx_status != MANCHESTER_RX_STATUS_ERROR) && (manchester_rx_status != MANCHESTER_RX_STATUS_READY))
		{
			unsigned short length = AT91C_BASE_TC1->TC_RA;
			status = AT91C_BASE_TC1->TC_SR; 
                        manchester_rx_status = Manchester_RX_Parse(length);
			if(manchester_rx_status == MANCHESTER_RX_STATUS_HANDLE_READY)
			{
				if(manchester_rx_data_count < manchester_rx_data_max_size)
				{
					((manchester_rx_type *)manchester_rx_data_buffer)[manchester_rx_data_count].data = Manchester_RX_Get_Data();
					manchester_rx_data_count ++;
				}
				if(manchester_rx_data_count < manchester_rx_data_size)
				{
					manchester_rx_time = manchester_rx_timeout;
				}
			}
			if(manchester_rx_data_count >= manchester_rx_data_size)	manchester_rx_time = manchester_rx_pause;
		} 	
	};

	AT91C_BASE_AIC->AIC_EOICR = AT91C_BASE_TC1->TC_SR;  	//clear Status Register  на сулчай прихода сверхкоротких импульсов после срабатывания CPCS прихода LDRAS, это штатная ситуация
}       

void Manchester_RX_Start(unsigned char flags, unsigned int freq, unsigned int timeout, unsigned int pause, unsigned short size, unsigned short *data, unsigned short max_size)
{
	manchester_rx_status = MANCHESTER_RX_STATUS_HANDLE;
	manchester_rx_data_count = 0;
	manchester_rx_data_size = size;
	manchester_rx_data_max_size = max_size;
	manchester_rx_data_buffer = data;
	manchester_rx_flags = flags;
	manchester_rx_pause = pause / (((5 * 1000000) / freq) / 2);
	if(manchester_rx_pause < 1) manchester_rx_pause = 1;
	manchester_rx_timeout = timeout / (((5 * 1000000) / freq) / 2);
	if(manchester_rx_timeout < 1) manchester_rx_timeout = 1;
	manchester_rx_time = manchester_rx_timeout;
	Manchester_RX_Set_Bit(((CLOCK_MCK / 2)) / freq);
	if(manchester_rx_flags & (1 << MANCHESTER_FLAG_INVERSE_PARITY_BIT)) Manchester_RX_Set_Parity(false);
	else Manchester_RX_Set_Parity(true);
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS; // Disable the Clock Counter 
	AT91C_BASE_TC1->TC_RC = (((5 * (CLOCK_MCK / 2)) / freq) / 2); //таймер для паузы и таймаута измеряется в 2.5, периодах, т.к. это больше любой длительности
	AT91F_TC_InterruptDisable(AT91C_BASE_TC1, 0xFFFFFFFF);
	unsigned short dummya = AT91C_BASE_TC1->TC_RA;
	unsigned short dummyb = AT91C_BASE_TC1->TC_RB;
	unsigned int dummy = AT91C_BASE_TC1->TC_SR; //clear Status Register
	AT91C_BASE_TC1->TC_CMR = AT91C_TC_CLKS_TIMER_DIV1_CLOCK | AT91C_TC_LDRA_BOTH | AT91C_TC_EEVTEDG_BOTH | AT91C_TC_LDRB_BOTH | AT91C_TC_ABETRG | AT91C_TC_CPCTRG;	
	AT91F_TC_InterruptEnable(AT91C_BASE_TC1, AT91C_TC_LDRAS | AT91C_TC_CPCS | AT91C_TC_LOVRS); // Validate the RC compare interrupt 
	AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC1);
	AT91C_BASE_PIOB->PIO_ASR = AT91C_PB25_TIOA1;
	AT91C_BASE_PIOB->PIO_PDR = AT91C_PIO_PB25;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_SWTRG;
}

void Manchester_RX_Stop()
{
	AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB25;
	AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS; // Disable the Clock Counter 
	AT91F_TC_InterruptDisable(AT91C_BASE_TC1, 0xFFFFFFFF);
	AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_TC1);
	unsigned int dummy = AT91C_BASE_TC1->TC_SR; //clear Status Register
	manchester_rx_status = MANCHESTER_RX_STATUS_WAIT;
}

unsigned short Manchester_RX_Get()
{
	return manchester_rx_data_count;
}

bool Manchester_RX_Ready()
{
	if(manchester_rx_status == MANCHESTER_RX_STATUS_READY) return true;
	return false;
}

bool Manchester_RX_Error()
{
	if(manchester_rx_status == MANCHESTER_RX_STATUS_ERROR) return true;
	return false;
}

bool Manchester_RX_Busy()
{
	if((manchester_rx_status != MANCHESTER_RX_STATUS_WAIT) && (manchester_rx_status != MANCHESTER_RX_STATUS_READY) && (manchester_rx_status != MANCHESTER_RX_STATUS_ERROR)) return true;
	return false;
}

bool Manchester_RX_Stopped()
{
	if(manchester_rx_status == MANCHESTER_RX_STATUS_WAIT) return true;
	return false;
}


/********************************************************************/
void Manchester_Idle()
{
}

void Manchester_Init()
{
	AT91F_TC1_CfgPMC();
	AT91F_TC2_CfgPMC();
	AT91F_PIOA_CfgPMC();
	AT91F_PIOB_CfgPMC();
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC1, MANCHESTER_RX_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, Manchester_RX_Isr);
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC2, MANCHESTER_TX_INTERRUPT_LEVEL, AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, Manchester_TX_Isr);
	AT91C_BASE_PIOA->PIO_PPUDR = AT91C_PIO_PA0;
	AT91C_BASE_PIOA->PIO_PER = AT91C_PIO_PA2;
        AT91C_BASE_PIOA->PIO_OER = AT91C_PIO_PA2;
	AT91C_BASE_PIOA->PIO_OWER = AT91C_PIO_PA2;
	AT91C_BASE_PIOA->PIO_CODR = AT91C_PIO_PA2;
	AT91C_BASE_PIOA->PIO_PER = AT91C_PIO_PA1;
        AT91C_BASE_PIOA->PIO_OER = AT91C_PIO_PA1;
	AT91C_BASE_PIOA->PIO_OWER = AT91C_PIO_PA1;
	AT91C_BASE_PIOA->PIO_SODR = AT91C_PIO_PA1;
	AT91C_BASE_PIOB->PIO_PER = AT91C_PIO_PB25;

}
