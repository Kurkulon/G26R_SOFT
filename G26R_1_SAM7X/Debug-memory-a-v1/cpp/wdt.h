#ifndef MEMORY_AT91SAM7X256_WDT_H
#define MEMORY_AT91SAM7X256_WDT_H

#define WDT_PERIOD_MS	1000u		// 4..16000

/************ WDT functions *************/
extern void WDT_Init(bool enable);

inline void WDT_Restart() {	AT91F_WDTRestart(AT91C_BASE_WDTC); }

/*****************************************************************************/

#endif
