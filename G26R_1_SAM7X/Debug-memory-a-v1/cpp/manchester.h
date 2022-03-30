#ifndef MEMORY_AT91SAM7X256_MANCHESTER_H
#define MEMORY_AT91SAM7X256_MANCHESTER_H

#define MANCHESTER_RX_INTERRUPT_LEVEL	5
#define MANCHESTER_TX_INTERRUPT_LEVEL	5
/*****************************************************************************/
typedef enum __attribute__ ((packed)) 
{
	MANCHESTER_TX_STATUS_WAIT = 0,
	MANCHESTER_TX_STATUS_ME,
	MANCHESTER_TX_STATUS_LO,
	MANCHESTER_TX_STATUS_HI,
	MANCHESTER_TX_STATUS_READY,
} manchester_tx_status_type;

typedef enum __attribute__ ((packed)) 
{
	MANCHESTER_RX_STATUS_WAIT = 0,
	MANCHESTER_RX_STATUS_HANDLE,
	MANCHESTER_RX_STATUS_HANDLE_ERROR_LENGTH,
	MANCHESTER_RX_STATUS_HANDLE_ERROR_STRUCT,
	MANCHESTER_RX_STATUS_HANDLE_READY,
	MANCHESTER_RX_STATUS_ERROR,
	MANCHESTER_RX_STATUS_READY,
} manchester_rx_status_type;

enum
{
	MANCHESTER_FLAG_INVERSE_POLARITY_BIT = 0,
	MANCHESTER_FLAG_INVERSE_PARITY_BIT,
	MANCHESTER_FLAG_INVERSE_COMMAND_BIT,
};

typedef struct __attribute__ ((packed))
{
	unsigned short data;	
} manchester_tx_type;

typedef struct __attribute__ ((packed))
{
	unsigned short data;	
} manchester_rx_type;

/*****************************************************************************/
extern void Manchester_Idle();
extern void Manchester_Init();

extern void Manchester_TX_Start(unsigned char flags, unsigned int freq, unsigned short size, unsigned short *data);
extern void Manchester_TX_Stop();
extern bool Manchester_TX_Ready();
extern bool Manchester_TX_Error();
extern bool Manchester_TX_Busy();
extern bool Manchester_TX_Stopped();

extern void Manchester_RX_Start(unsigned char flags, unsigned int freq, unsigned int timeout, unsigned int pause, unsigned short size, unsigned short *data, unsigned short data_size);
extern void Manchester_RX_Stop();
extern unsigned short Manchester_RX_Get();
extern bool Manchester_RX_Ready();
extern bool Manchester_RX_Error();
extern bool Manchester_RX_Busy();
extern bool Manchester_RX_Stopped();
/*****************************************************************************/

#endif
