#include "common.h"
#include "fram.h"
#include "ds32c35.h"
#include "telemetry.h"

#define FRAM_VARIABLE_CHECK(n, x) (fram_ ## n ## _buf.x != fram_ ## n .x) 
#define FRAM_VARIABLE_WRITE(n, x) \
	{                                                 \
 		if(FRAM_Write(offsetof(fram_ ## n ## _type, x) + offsetof(fram_type, n), (unsigned char *)&fram_ ## n ## _buf.x, sizeof(fram_ ## n ## _buf.x))) \
		{ \
			fram_ ## n .x = fram_ ## n ## _buf.x; \
		} \
	}

/*****************************************************************************/

bool FRAM_Write(unsigned short adr, unsigned char *data, unsigned short size)
{
	if(adr + size > DS32C35_FRAM_SIZE) return false;
	return DS32C35_FRAM_Write(adr, data, size);
}

bool FRAM_Read(unsigned short adr, unsigned char *data, unsigned short size)
{
	if(adr + size > DS32C35_FRAM_SIZE) return false;
	return DS32C35_FRAM_Read(adr, data, size);
}

/*****************************************************************************/
fram_main_type fram_main;
fram_main_type fram_main_buf;
bool fram_main_change = false;

void FRAM_Main_Init()
{
	FRAM_Read(offsetof(fram_type, main), (unsigned char *)&fram_main, sizeof(fram_main_type));
	fram_main_buf = fram_main;
	fram_main_change = false;
}

void FRAM_Main_Idle()
{
	if(!fram_main_change) return;
	if(FRAM_VARIABLE_CHECK(main, device_number)) FRAM_VARIABLE_WRITE(main, device_number)
	else 
	if(FRAM_VARIABLE_CHECK(main, device_type)) FRAM_VARIABLE_WRITE(main, device_type)
	else 
	if(FRAM_VARIABLE_CHECK(main, device_telemetry)) FRAM_VARIABLE_WRITE(main, device_telemetry)
	else fram_main_change = false;
}

unsigned short FRAM_Main_Device_Number_Get()
{
	return fram_main_buf.device_number;	
}

void FRAM_Main_Device_Number_Set(unsigned short n)
{	
	fram_main_buf.device_number = n;
	fram_main_change = true;
}

unsigned char FRAM_Main_Device_Type_Get()
{
	return fram_main_buf.device_type;	
}

void FRAM_Main_Device_Type_Set(unsigned char n)
{	
	fram_main_buf.device_type = n;
	fram_main_change = true;
}

unsigned char FRAM_Main_Device_Telemetry_Get()
{
	return fram_main_buf.device_telemetry;	
}

void FRAM_Main_Device_Telemetry_Set(unsigned char n)
{	
	fram_main_buf.device_telemetry = n;
	fram_main_change = true;
}

/*****************************************************************************/
fram_memory_type fram_memory;
fram_memory_type fram_memory_buf;
bool fram_memory_change = false;

void FRAM_Memory_Init()
{
	FRAM_Read(offsetof(fram_type, memory), (unsigned char *)&fram_memory, sizeof(fram_memory_type));
	fram_memory_buf = fram_memory;
	fram_memory_change = false;
}

void FRAM_Memory_Idle()
{
	if(!fram_memory_change) return;
	if(FRAM_VARIABLE_CHECK(memory, current_adress)) FRAM_VARIABLE_WRITE(memory, current_adress)
	else
	if(FRAM_VARIABLE_CHECK(memory, current_vector_adress)) FRAM_VARIABLE_WRITE(memory, current_vector_adress)
	else
	if(FRAM_VARIABLE_CHECK(memory, current_session)) FRAM_VARIABLE_WRITE(memory, current_session)
	else
	if(FRAM_VARIABLE_CHECK(memory, start_adress)) FRAM_VARIABLE_WRITE(memory, start_adress)
	else fram_memory_change = false;
}

unsigned short FRAM_Memory_Current_Session_Get()
{
	return fram_memory_buf.current_session;	
}

void FRAM_Memory_Current_Session_Set(unsigned short s)
{	
	fram_memory_buf.current_session = s;	
	fram_memory_change = true;
}

long long FRAM_Memory_Current_Adress_Get()
{
	return fram_memory_buf.current_adress;	
}

void FRAM_Memory_Current_Adress_Set(long long a)
{	
	fram_memory_buf.current_adress = a;
	fram_memory_change = true;
}

long long FRAM_Memory_Current_Vector_Adress_Get()
{
	return fram_memory_buf.current_vector_adress;	
}

void FRAM_Memory_Current_Vector_Adress_Set(long long a)
{	
	fram_memory_buf.current_vector_adress = a;
	fram_memory_change = true;
}

long long FRAM_Memory_Start_Adress_Get()
{
	return fram_memory_buf.start_adress;	
}

void FRAM_Memory_Start_Adress_Set(long long a)
{	
	fram_memory_buf.start_adress = a;
	fram_memory_change = true;
}

/*****************************************************************************/
fram_power_type fram_power;
fram_power_type fram_power_buf;
bool fram_power_change = false;

void FRAM_Power_Init()
{
	FRAM_Read(offsetof(fram_type, power), (unsigned char *)&fram_power, sizeof(fram_power_type));
	fram_power_buf = fram_power;
	fram_power_change = false;
}

void FRAM_Power_Idle()
{
	if(!fram_power_change) return;
	if(FRAM_VARIABLE_CHECK(power, battery_setup_voltage)) FRAM_VARIABLE_WRITE(power, battery_setup_voltage) 
	else
	if(FRAM_VARIABLE_CHECK(power, battery_min_voltage)) FRAM_VARIABLE_WRITE(power, battery_min_voltage)
	else
	if(FRAM_VARIABLE_CHECK(power, battery_max_voltage)) FRAM_VARIABLE_WRITE(power, battery_max_voltage)
	else
	if(FRAM_VARIABLE_CHECK(power, battery_min_shutdown)) FRAM_VARIABLE_WRITE(power, battery_min_shutdown)
	else
	if(FRAM_VARIABLE_CHECK(power, battery_max_shutdown)) FRAM_VARIABLE_WRITE(power, battery_max_shutdown)
	else
	if(FRAM_VARIABLE_CHECK(power, battery_coeffs.k)) FRAM_VARIABLE_WRITE(power, battery_coeffs.k)
	else
	if(FRAM_VARIABLE_CHECK(power, battery_coeffs.b)) FRAM_VARIABLE_WRITE(power, battery_coeffs.b)
	else
	if(FRAM_VARIABLE_CHECK(power, line_setup_voltage)) FRAM_VARIABLE_WRITE(power, line_setup_voltage) 
	else
	if(FRAM_VARIABLE_CHECK(power, line_min_voltage)) FRAM_VARIABLE_WRITE(power, line_min_voltage)
	else
	if(FRAM_VARIABLE_CHECK(power, line_max_voltage)) FRAM_VARIABLE_WRITE(power, line_max_voltage)
	else
	if(FRAM_VARIABLE_CHECK(power, line_min_shutdown)) FRAM_VARIABLE_WRITE(power, line_min_shutdown)
	else
	if(FRAM_VARIABLE_CHECK(power, line_max_shutdown)) FRAM_VARIABLE_WRITE(power, line_max_shutdown)
	else
	if(FRAM_VARIABLE_CHECK(power, line_coeffs.k)) FRAM_VARIABLE_WRITE(power, line_coeffs.k)
	else
	if(FRAM_VARIABLE_CHECK(power, line_coeffs.b)) FRAM_VARIABLE_WRITE(power, line_coeffs.b)
	else fram_power_change = false;
}

void FRAM_Power_Battery_Voltages_Get(short *setup, short *min, short *max, unsigned char *min_shutdown, unsigned char *max_shutdown)
{
	*setup = fram_power_buf.battery_setup_voltage;
	*min = fram_power_buf.battery_min_voltage;
	*max = fram_power_buf.battery_max_voltage;
	*min_shutdown = fram_power_buf.battery_min_shutdown;
	*max_shutdown = fram_power_buf.battery_max_shutdown;
	
}

void FRAM_Power_Battery_Voltages_Set(short setup, short min, short max, unsigned char min_shutdown, unsigned char max_shutdown)
{
	fram_power_buf.battery_setup_voltage = setup;
 	fram_power_buf.battery_min_voltage = min;
	fram_power_buf.battery_max_voltage = max;
	fram_power_buf.battery_min_shutdown = min_shutdown;
	fram_power_buf.battery_max_shutdown = max_shutdown;
	fram_power_change = true;
}

void FRAM_Power_Battery_Coeffs_Get(float *k, float *b)
{
	COPY((char *)&fram_power_buf.battery_coeffs.k, (char *)k, sizeof(float));
	COPY((char *)&fram_power_buf.battery_coeffs.b, (char *)b, sizeof(float));
}

void FRAM_Power_Battery_Coeffs_Set(float *k, float *b)
{
	COPY((char *)k, (char *)&fram_power_buf.battery_coeffs.k, sizeof(float));
	COPY((char *)b, (char *)&fram_power_buf.battery_coeffs.b, sizeof(float));
	fram_power_change = true;
}

void FRAM_Power_Line_Voltages_Get(short *setup, short *min, short *max, unsigned char *min_shutdown, unsigned char *max_shutdown)
{
	*setup = fram_power_buf.line_setup_voltage;
	*min = fram_power_buf.line_min_voltage;
	*max = fram_power_buf.line_max_voltage;
	*min_shutdown = fram_power_buf.line_min_shutdown;
	*max_shutdown = fram_power_buf.line_max_shutdown;
}

void FRAM_Power_Line_Voltages_Set(short setup, short min, short max, unsigned char min_shutdown, unsigned char max_shutdown)
{
	fram_power_buf.line_setup_voltage = setup;
	fram_power_buf.line_min_voltage = min;
	fram_power_buf.line_max_voltage = max;
	fram_power_buf.line_min_shutdown = min_shutdown;
	fram_power_buf.line_max_shutdown = max_shutdown;
	fram_power_change = true;
}

void FRAM_Power_Line_Coeffs_Get(float *k, float *b)
{
	COPY((char *)&fram_power_buf.line_coeffs.k, (char *)k, sizeof(float));
	COPY((char *)&fram_power_buf.line_coeffs.b, (char *)b, sizeof(float));
}

void FRAM_Power_Line_Coeffs_Set(float *k, float *b)
{
	COPY((char *)k, (char *)&fram_power_buf.line_coeffs.k, sizeof(float));
	COPY((char *)b, (char *)&fram_power_buf.line_coeffs.b, sizeof(float));
	fram_power_change = true;
}

/*****************************************************************************/
fram_sensors_type fram_sensors;
fram_sensors_type fram_sensors_buf;
bool fram_sensors_change = false;

void FRAM_Sensors_Init()
{
	FRAM_Read(offsetof(fram_type, sensors), (unsigned char *)&fram_sensors, sizeof(fram_sensors_type));
	fram_sensors_buf = fram_sensors;
	fram_sensors_change = false;
}

void FRAM_Sensors_Idle()
{
	if(!fram_sensors_change) return;
	if(FRAM_VARIABLE_CHECK(sensors, ax_coeffs.k)) FRAM_VARIABLE_WRITE(sensors, ax_coeffs.k)
	else
	if(FRAM_VARIABLE_CHECK(sensors, ax_coeffs.b)) FRAM_VARIABLE_WRITE(sensors, ax_coeffs.b)
	else
	if(FRAM_VARIABLE_CHECK(sensors, ay_coeffs.k)) FRAM_VARIABLE_WRITE(sensors, ay_coeffs.k)
	else
	if(FRAM_VARIABLE_CHECK(sensors, ay_coeffs.b)) FRAM_VARIABLE_WRITE(sensors, ay_coeffs.b)
	else
	if(FRAM_VARIABLE_CHECK(sensors, az_coeffs.k)) FRAM_VARIABLE_WRITE(sensors, az_coeffs.k)
	else
	if(FRAM_VARIABLE_CHECK(sensors, az_coeffs.b)) FRAM_VARIABLE_WRITE(sensors, az_coeffs.b)
	else fram_sensors_change = false;
}

void FRAM_Sensors_Ax_Coeffs_Get(float *k, float *b)
{
	COPY((char *)&fram_sensors_buf.ax_coeffs.k, (char *)k, sizeof(float));
	COPY((char *)&fram_sensors_buf.ax_coeffs.b, (char *)b, sizeof(float));
}

void FRAM_Sensors_Ax_Coeffs_Set(float *k, float *b)
{
	COPY((char *)k, (char *)&fram_sensors_buf.ax_coeffs.k, sizeof(float));
	COPY((char *)b, (char *)&fram_sensors_buf.ax_coeffs.b, sizeof(float));
	fram_sensors_change = true;
}

void FRAM_Sensors_Ay_Coeffs_Get(float *k, float *b)
{
	COPY((char *)&fram_sensors_buf.ay_coeffs.k, (char *)k, sizeof(float));
	COPY((char *)&fram_sensors_buf.ay_coeffs.b, (char *)b, sizeof(float));
}

void FRAM_Sensors_Ay_Coeffs_Set(float *k, float *b)
{
	COPY((char *)k, (char *)&fram_sensors_buf.ay_coeffs.k, sizeof(float));
	COPY((char *)b, (char *)&fram_sensors_buf.ay_coeffs.b, sizeof(float));
	fram_sensors_change = true;
}

void FRAM_Sensors_Az_Coeffs_Get(float *k, float *b)
{
	COPY((char *)&fram_sensors_buf.az_coeffs.k, (char *)k, sizeof(float));
	COPY((char *)&fram_sensors_buf.az_coeffs.b, (char *)b, sizeof(float));
}

void FRAM_Sensors_Az_Coeffs_Set(float *k, float *b)
{
	COPY((char *)k, (char *)&fram_sensors_buf.az_coeffs.k, sizeof(float));
	COPY((char *)b, (char *)&fram_sensors_buf.az_coeffs.b, sizeof(float));
	fram_sensors_change = true;
}
     
/*****************************************************************************/
unsigned char fram_autonom_buffer[DS32C35_FRAM_SIZE - offsetof(fram_type, autonom)]; // последняя в структуре
fram_autonom_type *fram_autonom = (fram_autonom_type *)fram_autonom_buffer;
fram_autonom_validation_type fram_autonom_validation = FRAM_AUTONOM_VALIDATION_NONE;
bool fram_autonom_change = false;

fram_autonom_result_type fram_autonom_result[256]; //отработавшие команды

unsigned char Fram_Automon_CRC(unsigned char *data, unsigned short size)
{
    	unsigned char crc = 0xFF;
    	unsigned char i;
	while(size --)
    	{
        	crc ^= *data++;
 	        for (i = 0; i < 8; i++) crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
	}
        return crc;
}

fram_autonom_validation_type FRAM_Autonom_Validation()
{
	unsigned char crc_header = fram_autonom->header.crc_header;
	fram_autonom->header.crc_header = 0;
	if(Fram_Automon_CRC((unsigned char *)fram_autonom, sizeof(fram_autonom->header)) != crc_header) 
	{
	        fram_autonom->header.crc_header = crc_header;
		return FRAM_AUTONOM_VALIDATION_ERROR_CRC_HEADER;
	}
        fram_autonom->header.crc_header = crc_header;
	if(Fram_Automon_CRC((unsigned char *)fram_autonom + sizeof(fram_autonom->header), fram_autonom->header.size - sizeof(fram_autonom->header)) != fram_autonom->header.crc_data) return FRAM_AUTONOM_VALIDATION_ERROR_CRC_DATA;
	if(fram_autonom->header.version != FRAM_AUTONOM_VERSION) return FRAM_AUTONOM_VALIDATION_ERROR_VERSION;
	if(fram_autonom->control.special_period_ms >= 1000 * 60 * 60 * 24) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
	unsigned char i, j;
	for(i = 0; i < fram_autonom->control.session_count; i ++)
	{
		if(((fram_autonom_session_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->period_ms >= 1000 * 60 * 60 * 24) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
		for(j = 0; j < ((fram_autonom_session_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->device_count; j ++)
			if(((fram_autonom_session_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->device[j] >= fram_autonom->control.device_count) return FRAM_AUTONOM_VALIDATION_ERROR_STRUCT;
		if(j == 0) return FRAM_AUTONOM_VALIDATION_ERROR_COUNT;
	}
	for(i = fram_autonom->control.session_count; i < fram_autonom->control.session_count + fram_autonom->control.device_count; i ++)
	{
		if(((fram_autonom_device_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->delay_ms >= 1000 * 60 * 60 * 24) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
		if(((fram_autonom_device_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->period_min_ms >= 1000 * 60 * 60 * 24) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
		for(j = 0; j < ((fram_autonom_device_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->command_count; j ++)
			if(((fram_autonom_device_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->command[j] >= fram_autonom->control.command_count) return FRAM_AUTONOM_VALIDATION_ERROR_STRUCT;
		if(j == 0) return FRAM_AUTONOM_VALIDATION_ERROR_COUNT;
	}
	for(i = fram_autonom->control.session_count + fram_autonom->control.device_count; i < fram_autonom->control.session_count + fram_autonom->control.device_count + fram_autonom->control.command_count; i ++)
	{
		if(((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->telemetry >= TELEMETRY_UNKNOWN) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
		if(((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->tx_freq_hz < 1000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
		if(((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->rx_freq_hz < 1000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
		if(((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->telemetry == TELEMETRY_MANCHESTER)
		{
			if(((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->tx_freq_hz > 50000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
			if(((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->rx_freq_hz > 50000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
		}
		if((((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->telemetry == TELEMETRY_USART) || (((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->telemetry == TELEMETRY_UART))
		{
			if(((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->tx_freq_hz > 1000000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
			if(((fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[i]))->rx_freq_hz > 1000000) return FRAM_AUTONOM_VALIDATION_ERROR_PARAMETR;
		}
		
	}
	return FRAM_AUTONOM_VALIDATION_OK;
}

//-------------------------------------------------------------

void FRAM_Autonom_Write_Begin()
{
	FRAM_Autonom_Result_Reset();
       	fram_autonom_validation = FRAM_AUTONOM_VALIDATION_NONE;
}

void FRAM_Autonom_Write_End()
{
	fram_autonom_validation = FRAM_Autonom_Validation(/*(unsigned char *)fram_autonom*/);
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK) fram_autonom_change = true; else fram_autonom_change = false;
}

void FRAM_Autonom_Write_Block(unsigned short offset, unsigned short size, unsigned char *data)
{
	if((offsetof(fram_type, autonom) + offset + size) < sizeof(fram_autonom_buffer)) memmove((char *)fram_autonom + offset, (char *)data, size);
}

void FRAM_Autonom_Read_Block(unsigned short offset, unsigned short size, unsigned char *data)
{
	if((offsetof(fram_type, autonom) + offset + size) < sizeof(fram_autonom_buffer)) memmove((char *)data, (char *)fram_autonom + offset, size);
}

//-------------------------------------------------------------

void FRAM_Autonom_Reset()
{
	FRAM_Autonom_Result_Reset();
}

void FRAM_Autonom_Init()
{
	FRAM_Read(offsetof(fram_type, autonom), (unsigned char *)fram_autonom, sizeof(fram_autonom_buffer));		
	fram_autonom_validation = FRAM_Autonom_Validation(/*(unsigned char *)fram_autonom*/);
	fram_autonom_change = false;
	FRAM_Autonom_Reset();
}

bool FRAM_Autonom_Idle()
{
	if(!fram_autonom_change) return false;
	if(fram_autonom_validation != FRAM_AUTONOM_VALIDATION_OK) return false; 
	FRAM_Write(offsetof(fram_type, autonom), (unsigned char *)fram_autonom, fram_autonom->header.size);
	fram_autonom_change = false;
	return true;
}

//-----------------------------------------------
unsigned short FRAM_Autonom_Size_Get()
{
	return sizeof(fram_autonom_buffer);
}

unsigned char FRAM_Autonom_Version_Get()
{
	return FRAM_AUTONOM_VERSION;
}

fram_autonom_validation_type FRAM_Autonom_Validation_Get()
{
	return fram_autonom_validation;
}

unsigned int FRAM_Autonom_Special_Period_MS_Get()
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK) return fram_autonom->control.special_period_ms;
	return 0;
}

unsigned char FRAM_Autonom_Session_Count_Get()
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK) return fram_autonom->control.session_count;
	return 0;
}
//-----------------------------------------------
inline fram_autonom_session_type *FRAM_Autonom_Session_Pointer_Get(unsigned char session)
{
	return (fram_autonom_session_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[session]);
}

inline fram_autonom_device_type *FRAM_Autonom_Device_Pointer_Get(unsigned char device)
{
	return (fram_autonom_device_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[fram_autonom->control.session_count + device]);
}

inline fram_autonom_command_type *FRAM_Autonom_Command_Pointer_Get(unsigned char command)
{
	return (fram_autonom_command_type *)((unsigned int)fram_autonom + (unsigned int)fram_autonom->control.pointer[fram_autonom->control.session_count + fram_autonom->control.device_count + command]);
}

//-----------------------------------------------
RTC_type FRAM_Autonom_Session_Start_Get(unsigned char session)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
        return FRAM_Autonom_Session_Pointer_Get(session)->start;
	RTC_type rtc;
	rtc.date = 0;
	rtc.time = 0;
	return rtc;
}

RTC_type FRAM_Autonom_Session_Stop_Get(unsigned char session)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
        return FRAM_Autonom_Session_Pointer_Get(session)->stop;
	RTC_type rtc;
	rtc.date = 0;
	rtc.time = 0;
	return rtc;
}

bool FRAM_Autonom_Session_RTC_Check(unsigned char session)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	{
		fram_autonom_session_type* p = FRAM_Autonom_Session_Pointer_Get(session);

        return RTC_Check(p->start, p->stop);
	}
	else
	{
		return false;
	};
}

unsigned int FRAM_Autonom_Session_Period_MS_Get(unsigned char session)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Session_Pointer_Get(session)->period_ms;
	return 0;
}

unsigned char FRAM_Autonom_Session_Device_Count_Get(unsigned char session)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Session_Pointer_Get(session)->device_count;
	return 0;
}

//-----------------------------------------------
unsigned int FRAM_Autonom_Session_Device_Delay_MS_Get(unsigned char session, unsigned char device)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->delay_ms;
	return 0;
}

unsigned int FRAM_Autonom_Session_Device_Period_Min_MS_Get(unsigned char session, unsigned char device)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->period_min_ms;
	return 0;
}

unsigned char FRAM_Autonom_Session_Device_Command_Count_Get(unsigned char session, unsigned char device)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command_count;
	return 0;
}

//-----------------------------------------------
unsigned char FRAM_Autonom_Session_Device_Command_Telemetry_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->telemetry;
	return 0;
}

unsigned char FRAM_Autonom_Session_Device_Command_Mode_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->mode;
	return 0;
}

unsigned short FRAM_Autonom_Session_Device_Command_Offset_MS_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->offset_ms;
	return 0;
}

unsigned char FRAM_Autonom_Session_Device_Command_TX_Flags_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->tx_flags;
	return 0;
}

unsigned int FRAM_Autonom_Session_Device_Command_TX_Freq_HZ_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->tx_freq_hz;
	return 0;
}

unsigned short FRAM_Autonom_Session_Device_Command_TX_Size_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->tx_size;
	return 0;
}

unsigned char FRAM_Autonom_Session_Device_Command_RX_Flags_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_flags;
	return 0;
}

unsigned int FRAM_Autonom_Session_Device_Command_RX_Freq_HZ_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_freq_hz;
	return 0;
}

unsigned int FRAM_Autonom_Session_Device_Command_RX_Timeout_MKS_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_timeout_mks;
	return 0;
}

unsigned short FRAM_Autonom_Session_Device_Command_RX_Pause_MKS_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_pause_mks;
	return 0;
}

unsigned short FRAM_Autonom_Session_Device_Command_RX_Size_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->rx_size;
	return 0;
}

unsigned short *FRAM_Autonom_Session_Device_Command_TX_Data_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	return FRAM_Autonom_Command_Pointer_Get(FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command])->tx_data;
	return 0;
}
//-------------------------------------------------------------

void FRAM_Autonom_Result_Reset()
{
	unsigned short i;
	for(i = 0; i < sizeof(fram_autonom_result) / sizeof(fram_autonom_result_type); i ++) fram_autonom_result[i] = FRAM_AUTONOM_RESULT_NONE;
}

void FRAM_Autonom_Result_Set(unsigned char session, unsigned char device, unsigned char command, bool result)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	{
		if(result) fram_autonom_result[FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command]] = FRAM_AUTONOM_RESULT_READY;
		else fram_autonom_result[FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command]] = FRAM_AUTONOM_RESULT_ERROR;
	}
}

bool FRAM_Autonom_Result_Ready_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	{
		if(fram_autonom_result[FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command]] == FRAM_AUTONOM_RESULT_READY) return true;
	}
	return false;
}

bool FRAM_Autonom_Result_Error_Get(unsigned char session, unsigned char device, unsigned char command)
{
	if(fram_autonom_validation == FRAM_AUTONOM_VALIDATION_OK)
	{
		if(fram_autonom_result[FRAM_Autonom_Device_Pointer_Get(FRAM_Autonom_Session_Pointer_Get(session)->device[device])->command[command]] == FRAM_AUTONOM_RESULT_ERROR) return true;
	}
	return false;
}

/*****************************************************************************/

void FRAM_Init()
{
	FRAM_Main_Init();
	FRAM_Memory_Init();
	FRAM_Power_Init();
	FRAM_Sensors_Init();
	FRAM_Autonom_Init();
}

void FRAM_Idle()
{
	static unsigned char i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL(	FRAM_Memory_Idle(); 	);
		CALL( 	FRAM_Power_Idle();		);
		CALL( 	FRAM_Sensors_Idle();	);
		CALL( 	FRAM_Autonom_Idle();	);
		CALL( 	FRAM_Main_Idle();		);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;
}
