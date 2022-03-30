#ifndef MEMORY_AT91SAM7X256_USART1_H
#define MEMORY_AT91SAM7X256_USART1_H

/*****************************************************************************/
typedef enum __attribute__ ((packed))
{
	USART1_TX_STATUS_STOP = 0,
	USART1_TX_STATUS_START,
	USART1_TX_STATUS_READY,
	USART1_TX_STATUS_ERROR,
} usart1_tx_status_type;

typedef enum __attribute__ ((packed))
{
	USART1_RX_STATUS_STOP = 0,
	USART1_RX_STATUS_START,
	USART1_RX_STATUS_READY,
	USART1_RX_STATUS_ERROR,
} usart1_rx_status_type;
/*****************************************************************************/
extern void USART1_Init();
extern void USART1_Idle();
extern void USART1_Enable(unsigned int tx_freq, unsigned int rx_freq, bool sync);
extern void USART1_Disable();
/*****************************************************************************/
extern void USART1_TX_Stop();
extern bool USART1_TX_Start(unsigned short size, unsigned char *data);
extern bool USART1_TX_Ready();
extern bool USART1_TX_Stopped();
extern bool USART1_TX_Error();
extern unsigned short USART1_TX_Get();
/*****************************************************************************/
extern void USART1_RX_Stop();
extern bool USART1_RX_Start(unsigned int timeout, unsigned short pause, unsigned short size, unsigned char *data, unsigned short max_size);
extern bool USART1_RX_Ready();
extern bool USART1_RX_Stopped();
extern bool USART1_RX_Error();
extern unsigned short USART1_RX_Get();
/*****************************************************************************/

#endif
