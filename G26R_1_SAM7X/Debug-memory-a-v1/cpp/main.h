#ifndef MEMORY_AT91SAM7X256_H
#define MEMORY_AT91SAM7X256_H

#define VERSION			0x0103

#define MAIN_MODE_CHECK_MS	500

typedef enum __attribute__ ((packed)) 
{
	MAIN_MODE_NONE = 0,
	MAIN_MODE_EMAC,
	MAIN_MODE_MEMORY,
	MAIN_MODE_AUTONOM,
} main_mode_type;

#define PIOA	AT91C_BASE_PIOA
#define PIOB	AT91C_BASE_PIOB

extern void ErrorHalt(u32 code);

inline void delay(u32 d) { for (u32 i = 0; i < d; i++) __nop(); }

enum 
{
	ERROR_PIO_PIN_LOW		= 0	,	
	ERROR_PIO_PIN_HI			,	
	ERROR_PIO_PIN_SHORT			,	
	ERROR_EMAC_PHY_ID				// 3
};								



#endif
