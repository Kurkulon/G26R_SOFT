#ifndef MEMORY_AT91SAM7X256_FRAM_H
#define MEMORY_AT91SAM7X256_FRAM_H

#include "rtc.h"

/*************************/
typedef struct __attribute__ ((packed))   
{
	float k;
	float b;
} fram_linear_coeffs_type;
/*************************/
typedef enum __attribute__ ((packed))   
{
	FRAM_MAIN_DEVICE_TYPE_MODULE_MEMORY = 0,
	FRAM_MAIN_DEVICE_TYPE_MODULE_AUTONOM,
} fram_main_device_type;

enum
{
	FRAM_MAIN_DEVICE_TELEMETRY_MANCHESTER_BIT = 0,
	FRAM_MAIN_DEVICE_TELEMETRY_USART_BIT,
	FRAM_MAIN_DEVICE_TELEMETRY_UART_BIT,
};

typedef struct __attribute__ ((packed))   
{
	unsigned short device_number;
	unsigned char device_type;	// тип (0 = память, 1 =автономка)
	unsigned char device_telemetry;	// режим работы  (0bit = manch, 1bit = USART, 2bit = UART)
} fram_main_type;
/*************************/
typedef struct __attribute__ ((packed))   
{
	unsigned short current_session;     // всё что казасется записи векторов и сессий
	long long current_adress;
	long long current_vector_adress;
	long long start_adress;
} fram_memory_type;
/*************************/

typedef struct __attribute__ ((packed))   // всё что касаемо питания
{
	short battery_setup_voltage;	//0.1V
	short battery_min_voltage;	//0.1V
	short battery_max_voltage;	//0.1V
	unsigned char battery_min_shutdown;
	unsigned char battery_max_shutdown;
	short line_setup_voltage;	//0.1V
	short line_min_voltage;	//0.1V
	short line_max_voltage;	//0.1V
	unsigned char line_min_shutdown;	
	unsigned char line_max_shutdown;	
	fram_linear_coeffs_type battery_coeffs;
	fram_linear_coeffs_type line_coeffs;
} fram_power_type;
/*************************/
typedef struct __attribute__ ((packed))   // всё что касаемо питания
{
	fram_linear_coeffs_type ax_coeffs;
	fram_linear_coeffs_type ay_coeffs;
	fram_linear_coeffs_type az_coeffs;
} fram_sensors_type;
/*************************/
typedef enum __attribute__ ((packed)) 
{
        FRAM_AUTONOM_RESULT_NONE = 0, // не производился опрос
        FRAM_AUTONOM_RESULT_ERROR,
        FRAM_AUTONOM_RESULT_READY,
} fram_autonom_result_type;

typedef enum __attribute__ ((packed)) 
{
        FRAM_AUTONOM_VALIDATION_NONE = 0, // не производилась
        FRAM_AUTONOM_VALIDATION_OK,
        FRAM_AUTONOM_VALIDATION_ERROR_CRC_HEADER,
        FRAM_AUTONOM_VALIDATION_ERROR_CRC_DATA,
        FRAM_AUTONOM_VALIDATION_ERROR_VERSION,
        FRAM_AUTONOM_VALIDATION_ERROR_STRUCT,
        FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR,
        FRAM_AUTONOM_VALIDATION_ERROR_COUNT,
} fram_autonom_validation_type;

#define FRAM_AUTONOM_VERSION	0x1

typedef struct __attribute__ ((packed))       
{
	unsigned char telemetry;
	unsigned char mode;   
	unsigned short offset_ms; // смещение относительно предыдущей команды
	unsigned char tx_flags;
	unsigned int tx_freq_hz;
	unsigned short tx_size;
	unsigned char rx_flags;
	unsigned int rx_freq_hz;
	unsigned int rx_timeout_mks; 
	unsigned short rx_pause_mks;
	unsigned short rx_size;
	unsigned short tx_data[]; //команды в прибор
} fram_autonom_command_type;

typedef struct __attribute__ ((packed))   
{
	unsigned int delay_ms; // задержка с момента подачи питания
	unsigned int period_min_ms; // минимальный период опроса
	unsigned char command_count; // команд в приборе
	unsigned char command[]; // номера команд в приборе
} fram_autonom_device_type;

typedef struct __attribute__ ((packed))   
{
	RTC_type start;
	RTC_type stop;
	unsigned int period_ms;
	unsigned char device_count; // приборов в сессии
	unsigned char device[]; // номера приборов в сессии
} fram_autonom_session_type;

typedef struct __attribute__ ((packed))   
{
	unsigned int special_period_ms;
	unsigned char session_count;
	unsigned char device_count;
	unsigned char command_count;
	unsigned short pointer[];
} fram_autonom_control_type;

typedef struct __attribute__ ((packed))   
{
	unsigned char version;
	unsigned short size;		// размер данных вместе с header
	unsigned char crc_data;		// = crc(данные[size - header])
	unsigned char crc_header; 	// = crc(size, version, crc_data)
} fram_autonom_header_type;

typedef struct __attribute__ ((packed))   // управляющая память автономки
{
	fram_autonom_header_type header;
	fram_autonom_control_type control;
} fram_autonom_type;
/*************************************************/
typedef struct __attribute__ ((packed))   
{
	fram_main_type main;
	fram_memory_type memory;
	fram_power_type power;
	fram_sensors_type sensors;
	fram_autonom_type autonom;
} fram_type;              
/*************************************************/
extern void 		FRAM_Init();
extern void 		FRAM_Idle();

extern unsigned short 	FRAM_Main_Device_Number_Get();
extern void 		FRAM_Main_Device_Number_Set(unsigned short n);
extern unsigned char 	FRAM_Main_Device_Type_Get();
extern void 		FRAM_Main_Device_Type_Set(unsigned char t);
extern unsigned char 	FRAM_Main_Device_Telemetry_Get();
extern void 		FRAM_Main_Device_Telemetry_Set(unsigned char t);

extern unsigned short 	FRAM_Memory_Current_Session_Get();      	// ид текущей сессии
extern void 		FRAM_Memory_Current_Session_Set(unsigned short s);	
extern long long 	FRAM_Memory_Current_Adress_Get();             // текущий адрес
extern void 		FRAM_Memory_Current_Adress_Set(long long a);
extern long long 	FRAM_Memory_Current_Vector_Adress_Get();	// адрес последнего записанного вектора
extern void 		FRAM_Memory_Current_Vector_Adress_Set(long long a);
extern long long 	FRAM_Memory_Start_Adress_Get();               // адрес старта сессии
extern void		FRAM_Memory_Start_Adress_Set(long long a);

extern void 		FRAM_Power_Battery_Voltages_Get(short *setup, short *min, short *max, unsigned char *min_shutdown, unsigned char *max_shutdown);
extern void 		FRAM_Power_Battery_Voltages_Set(short setup, short min, short max, unsigned char min_shutdown, unsigned char max_shutdown);
extern void 		FRAM_Power_Battery_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Power_Battery_Coeffs_Set(float *k, float *b);
extern void 		FRAM_Power_Line_Voltages_Get(short *setup, short *min, short *max, unsigned char *min_shutdown, unsigned char *max_shutdown);
extern void 		FRAM_Power_Line_Voltages_Set(short setup, short min, short max, unsigned char min_shutdown, unsigned char max_shutdown);
extern void 		FRAM_Power_Line_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Power_Line_Coeffs_Set(float *k, float *b);

extern void 		FRAM_Sensors_Ax_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Sensors_Ax_Coeffs_Set(float *k, float *b);
extern void 		FRAM_Sensors_Ay_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Sensors_Ay_Coeffs_Set(float *k, float *b);
extern void 		FRAM_Sensors_Az_Coeffs_Get(float *k, float *b);
extern void 		FRAM_Sensors_Az_Coeffs_Set(float *k, float *b);

extern unsigned short 	FRAM_Autonom_Size_Get();
extern fram_autonom_validation_type FRAM_Autonom_Validation_Get();
extern unsigned char 	FRAM_Autonom_Version_Get();
extern void		FRAM_Autonom_Write_Begin();
extern void		FRAM_Autonom_Write_End();
extern void		FRAM_Autonom_Write_Block(unsigned short offset, unsigned short size, unsigned char *data);
extern void		FRAM_Autonom_Read_Block(unsigned short offset, unsigned short size, unsigned char *data);

extern unsigned int 	FRAM_Autonom_Special_Period_MS_Get();

extern unsigned char 	FRAM_Autonom_Session_Count_Get();
extern RTC_type 	FRAM_Autonom_Session_Start_Get(unsigned char session);
extern RTC_type 	FRAM_Autonom_Session_Stop_Get(unsigned char session);
extern unsigned int 	FRAM_Autonom_Session_Period_MS_Get(unsigned char session);
extern unsigned char 	FRAM_Autonom_Session_Device_Count_Get(unsigned char session);

extern unsigned int 	FRAM_Autonom_Session_Device_Delay_MS_Get(unsigned char session, unsigned char device);
extern unsigned int 	FRAM_Autonom_Session_Device_Period_Min_MS_Get(unsigned char session, unsigned char device);
extern unsigned char	FRAM_Autonom_Session_Device_Command_Count_Get(unsigned char session, unsigned char device);

extern unsigned char 	FRAM_Autonom_Session_Device_Command_Telemetry_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned char 	FRAM_Autonom_Session_Device_Command_Mode_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned short 	FRAM_Autonom_Session_Device_Command_Offset_MS_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned char 	FRAM_Autonom_Session_Device_Command_TX_Flags_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned int 	FRAM_Autonom_Session_Device_Command_TX_Freq_HZ_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned short 	FRAM_Autonom_Session_Device_Command_TX_Size_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned char 	FRAM_Autonom_Session_Device_Command_RX_Flags_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned int 	FRAM_Autonom_Session_Device_Command_RX_Freq_HZ_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned int 	FRAM_Autonom_Session_Device_Command_RX_Timeout_MKS_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned short 	FRAM_Autonom_Session_Device_Command_RX_Pause_MKS_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned short 	FRAM_Autonom_Session_Device_Command_RX_Size_Get(unsigned char session, unsigned char device, unsigned char command);
extern unsigned short 	*FRAM_Autonom_Session_Device_Command_TX_Data_Get(unsigned char session, unsigned char device, unsigned char command);

extern void 		FRAM_Autonom_Reset();
extern void 		FRAM_Autonom_Result_Reset();
extern void 		FRAM_Autonom_Result_Set(unsigned char session, unsigned char device, unsigned char command, bool result);
extern bool		FRAM_Autonom_Result_Ready_Get(unsigned char session, unsigned char device, unsigned char command);
extern bool		FRAM_Autonom_Result_Error_Get(unsigned char session, unsigned char device, unsigned char command);

extern bool FRAM_Autonom_Session_RTC_Check(unsigned char session);

/*****************************************************************************/

#endif
