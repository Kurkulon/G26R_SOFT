#include "common.h"
#include "main.h"
#include "usart1.h"

unsigned int usart1_freq = 1000000;
/*****************************************************************************/
usart1_tx_status_type usart1_tx_status = USART1_TX_STATUS_STOP;
unsigned short usart1_tx_size = 0;
unsigned short usart1_tx_count = 0;
unsigned char *usart1_tx_buf;

void USART1_TX_Stop()
{
	AT91C_BASE_US1->US_CR = AT91C_US_TXDIS;
	AT91C_BASE_US1->US_CR = AT91C_US_RSTTX;
	AT91F_PDC_DisableTx((AT91PS_PDC) & (AT91C_BASE_US1->US_RPR));
	usart1_tx_status = USART1_TX_STATUS_STOP;
}

bool USART1_TX_Start(unsigned short size, unsigned char *data)            
{
	usart1_tx_size = size;
	usart1_tx_count = 0;
	usart1_tx_buf = data;
	AT91F_PDC_EnableTx((AT91PS_PDC) & (AT91C_BASE_US1->US_RPR));
	AT91C_BASE_US1->US_CR = AT91C_US_TXDIS;
	AT91C_BASE_US1->US_CR = AT91C_US_RSTTX;
	if(AT91F_US_SendFrame (AT91C_BASE_US1, (char *)usart1_tx_buf, size, 0, 0) == 2)
	{
		usart1_tx_status = USART1_TX_STATUS_START;
		AT91C_BASE_US1->US_CR = AT91C_US_TXEN;
		return true;
	}
	else
	{
		usart1_tx_status = USART1_TX_STATUS_ERROR;
		return false;
	}
}

bool USART1_TX_Stopped()
{
	if(usart1_tx_status == USART1_TX_STATUS_STOP) return true;
	return false;
}

bool USART1_TX_Ready()
{
	if(usart1_tx_status == USART1_TX_STATUS_READY) return true;
	return false;
}

bool USART1_TX_Error()
{
	if(usart1_tx_status == USART1_TX_STATUS_ERROR) return true;
	return false;
}

unsigned short USART1_TX_Get()
{
	return usart1_tx_count;
}

void USART1_TX_Idle()
{
        unsigned int status = AT91C_BASE_US1->US_CSR;
	if((status & (AT91C_US_ENDTX | AT91C_US_TXEMPTY | AT91C_US_TXBUFE)) == (AT91C_US_ENDTX | AT91C_US_TXEMPTY | AT91C_US_TXBUFE)) 
	{
		if(usart1_tx_status == USART1_TX_STATUS_START)
		{
			usart1_tx_status = USART1_TX_STATUS_READY;
		}
	}
}
/*****************************************************************************/
usart1_rx_status_type usart1_rx_status = USART1_RX_STATUS_STOP;
unsigned short usart1_rx_size = 0;
unsigned short usart1_rx_max_size = 0;
unsigned short usart1_rx_count = 0;
unsigned short usart1_rx_timeout_count = 0;
unsigned short usart1_rx_timeout = 0;
unsigned short usart1_rx_pause = 0;
unsigned char *usart1_rx_buf;

void USART1_RX_Stop()
{
	AT91C_BASE_US1->US_CR = AT91C_US_RXDIS;
	AT91C_BASE_US1->US_CR = AT91C_US_RSTRX;
	AT91F_PDC_DisableRx((AT91PS_PDC) & (AT91C_BASE_US1->US_RPR));
	usart1_rx_status = USART1_RX_STATUS_STOP;
}

bool USART1_RX_Start(unsigned int timeout, unsigned short pause, unsigned short size, unsigned char *data, unsigned short max_size) 
{
	usart1_rx_size = size;
	usart1_rx_max_size = max_size;
	usart1_rx_count = 0;
	usart1_rx_buf = data;
	AT91F_PDC_EnableRx((AT91PS_PDC) & (AT91C_BASE_US1->US_RPR));
	AT91C_BASE_US1->US_CR = AT91C_US_RXDIS;
	AT91C_BASE_US1->US_CR = AT91C_US_RSTRX;
	float temp = (float)timeout / (1000000.0f / (float)usart1_freq);
	usart1_rx_timeout_count = (unsigned int)temp / 0x8000;
	usart1_rx_timeout = (unsigned int)temp % 0x8000;
	if(!usart1_rx_timeout) usart1_rx_timeout = 1;
	if(usart1_rx_timeout_count) AT91C_BASE_US1->US_RTOR = 0x8000; else AT91C_BASE_US1->US_RTOR = usart1_rx_timeout;
	temp = (float)pause / (1000000.0f / (float)usart1_freq);
	if(temp > 0xFFFF) temp = 0xFFFF;
	if(temp == 0) temp = 1;
	usart1_rx_pause = (unsigned short)temp;
	AT91C_BASE_US1->US_CR = AT91C_US_STTTO;
	AT91C_BASE_US1->US_CR = AT91C_US_RETTO;
	if(AT91F_US_ReceiveFrame (AT91C_BASE_US1, (char *)usart1_rx_buf, max_size, 0, 0) == 2)
	{
		usart1_rx_status = USART1_RX_STATUS_START;
		AT91C_BASE_US1->US_CR = AT91C_US_RXEN;
		return true;
	}
	else
	{
		usart1_rx_status = USART1_RX_STATUS_ERROR;
		return false;
	}
}

bool USART1_RX_Stopped()
{
	if(usart1_rx_status == USART1_RX_STATUS_STOP) return true;
	return false;
}

bool USART1_RX_Ready()
{
	if(usart1_rx_status == USART1_RX_STATUS_READY) return true;
	return false;
}

bool USART1_RX_Error()
{
	if(usart1_rx_status == USART1_RX_STATUS_ERROR) return true;
	return false;
}

unsigned short USART1_RX_Get()
{
	return usart1_rx_count;
}

void USART1_RX_Idle()
{
	if(usart1_rx_status == USART1_RX_STATUS_START)
	{
		unsigned short temp = AT91C_BASE_US1->US_RCR;	
		if(temp == 0)
		{
			usart1_rx_status = USART1_RX_STATUS_ERROR;
		}
		else
		{
			if(AT91C_BASE_US1->US_CSR & AT91C_US_TIMEOUT)
			{
				if(temp < usart1_rx_max_size)
				{
					usart1_rx_count = usart1_rx_max_size - temp;
					if(usart1_rx_count >= usart1_rx_size) usart1_rx_status = USART1_RX_STATUS_READY;
					else usart1_rx_status = USART1_RX_STATUS_ERROR;
				} 
				else
				{
					if(usart1_rx_timeout_count > 0)
					{
						usart1_rx_timeout_count --;
						if(usart1_rx_timeout_count) AT91C_BASE_US1->US_RTOR = 0x8000;
 						else AT91C_BASE_US1->US_RTOR = usart1_rx_timeout;
						AT91C_BASE_US1->US_CR = AT91C_US_STTTO;
						AT91C_BASE_US1->US_CR = AT91C_US_RETTO;
					}
					else
					{
						usart1_rx_status = USART1_RX_STATUS_ERROR;
					}
				} 
			}
			else
			{
				if(temp < usart1_rx_max_size)
				{
	                		if(AT91C_BASE_US1->US_RTOR != usart1_rx_pause)
					{	
						AT91C_BASE_US1->US_RTOR = usart1_rx_pause;
						AT91C_BASE_US1->US_CR = AT91C_US_RETTO;
					}
				}
			}
		}			
	}

}

/***********************************************************************/
void USART1_Enable(unsigned int tx_freq, unsigned int rx_freq, bool sync)
{
	usart1_freq = tx_freq; // работаем всегда на частоте запроса, железно
	if(sync)
	{
		AT91C_BASE_US1->US_BRGR = CLOCK_MCK / usart1_freq;
		AT91C_BASE_US1->US_MR |= AT91C_US_SYNC | AT91C_US_CKLO;
	}
	else
	{
		AT91C_BASE_US1->US_BRGR = (CLOCK_MCK / 16) / usart1_freq;
		AT91C_BASE_US1->US_MR &= ~(AT91C_US_SYNC | AT91C_US_CKLO);
	}
	AT91F_PDC_Open ((AT91PS_PDC) & (AT91C_BASE_US1->US_RPR));
}

void USART1_Disable()
{
	AT91C_BASE_US1->US_MR &= ~(AT91C_US_SYNC | AT91C_US_CKLO);
	AT91F_PDC_Close ((AT91PS_PDC) & (AT91C_BASE_US1->US_RPR));
}

void USART1_Idle()
{
	USART1_TX_Idle();
	USART1_RX_Idle();
}

void USART1_Init()
{
       	AT91F_PIO_CfgPeriph( AT91C_BASE_PIOA, ((unsigned int) AT91C_PA5_RXD1) | ((unsigned int) AT91C_PA6_TXD1) | ((unsigned int) AT91C_PA7_SCK1), 0);
    	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_US1 ) ;   	// First, enable the clock of the USART
	AT91F_US_SetTimeguard(AT91C_BASE_US1, 0);
	AT91F_PDC_Close ((AT91PS_PDC) & (AT91C_BASE_US1->US_RPR));
	AT91C_BASE_US1->US_MR = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_NBSTOP_1_BIT | AT91C_US_PAR_EVEN;
}
