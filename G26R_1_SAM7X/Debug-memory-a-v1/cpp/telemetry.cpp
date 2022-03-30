#include "common.h"
#include "main.h"
#include "telemetry.h"

/*********************************************************************/
unsigned short telemetry_rx_buffer[TELEMETRY_RX_BUFFER_SIZE];

void Telemetry_Init()
{

}

unsigned short *Telemetry_RX_Buffer()
{
	return telemetry_rx_buffer;
}

unsigned short Telemetry_RX_Buffer_Size()
{
	return sizeof(telemetry_rx_buffer);
}

/*********************************************************************/

