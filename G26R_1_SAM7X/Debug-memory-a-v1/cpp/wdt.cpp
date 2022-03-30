#include "common.h"
#include "wdt.h"

/************ WDT functions *************/

void WDT_Init(bool enable)
{
	unsigned int t = AT91F_WDTGetPeriod(WDT_PERIOD_MS);
	if(enable)	AT91F_WDTSetMode(AT91C_BASE_WDTC, AT91C_WDTC_WDRSTEN | ((AT91C_WDTC_WDD&(t<<16))) | (AT91C_WDTC_WDV&t));
		else	AT91F_WDTSetMode(AT91C_BASE_WDTC, AT91C_WDTC_WDDIS);
	AT91F_WDTGetStatus(AT91C_BASE_WDTC);
}
/*****************************************************************************/
