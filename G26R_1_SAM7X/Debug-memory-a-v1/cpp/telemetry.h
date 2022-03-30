#ifndef MEMORY_AT91SAM7X256_TELEMETRY_H
#define MEMORY_AT91SAM7X256_TELEMETRY_H

/*******************************************************************************/
#define TELEMETRY_RX_BUFFER_SIZE		4200

enum
{
	TELEMETRY_COMMAND_MODE_ERROR_STOP_SESSION_BIT = 0,	// �� ���������� ��� ������� � ������ � ������ ������
	TELEMETRY_COMMAND_MODE_READY_STOP_SESSION_BIT,		// �� ���������� ��� ������� � ������ ��� ������������� ����������
	TELEMETRY_COMMAND_MODE_ERROR_STOP_DEVICE_BIT,		// �� ���������� ��������� ������� ������� � ������ ������ 
	TELEMETRY_COMMAND_MODE_READY_STOP_DEVICE_BIT,			// �� ���������� ��������� ������� ������� � ������ �������������� ���������� ������ (������� ������������ � READY_STOP_SESSION)
};

typedef enum __attribute__ ((packed)) 
{
	TELEMETRY_MANCHESTER = 0,
	TELEMETRY_USART,
	TELEMETRY_UART,
	TELEMETRY_UNKNOWN,
} telemetry_type;

enum
{
	TELEMETRY_FLAG_DEVICES_DISABLE_BIT = 0,
	TELEMETRY_FLAG_TX_ERROR_BIT,
	TELEMETRY_FLAG_RX_ERROR_BIT,
};

typedef struct __attribute__ ((packed))
{
	unsigned short request;
	unsigned short data[];	
} telemetry_tx_type;

typedef struct __attribute__ ((packed))
{
	unsigned short response;
	unsigned short data[];	
} telemetry_rx_type;

/*****************************************************************************/
extern void Telemetry_Init();
extern unsigned short *Telemetry_RX_Buffer();
extern unsigned short Telemetry_RX_Buffer_Size();

/*****************************************************************************/


#endif
