#ifndef MEMORY_AT91SAM7X256_PIN_H
#define MEMORY_AT91SAM7X256_PIN_H

/************ TestPin I/O *******************/
#define TESTPIN1_PORT 	AT91C_BASE_PIOA
#define TESTPIN2_PORT 	AT91C_BASE_PIOA
#define TESTPIN1	AT91C_PIO_PA25
#define TESTPIN2	AT91C_PIO_PA26
/************ TestPin functions *************/
#define TestPin1_Set() TESTPIN1_PORT->PIO_SODR = TESTPIN1
#define TestPin1_Clr() TESTPIN1_PORT->PIO_CODR = TESTPIN1
#define TestPin2_Set() TESTPIN2_PORT->PIO_SODR = TESTPIN2
#define TestPin2_Clr() TESTPIN2_PORT->PIO_CODR = TESTPIN2

inline void TestPins_Init()
{
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOA ) ;
	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOB ) ;

	TESTPIN1_PORT->PIO_PER 	|= TESTPIN1;
	TESTPIN1_PORT->PIO_OER 	|= TESTPIN1;
	TESTPIN1_PORT->PIO_OWER |= TESTPIN1;
	TESTPIN2_PORT->PIO_PER 	|= TESTPIN2;
	TESTPIN2_PORT->PIO_OER 	|= TESTPIN2;
	TESTPIN2_PORT->PIO_OWER |= TESTPIN2;
	TestPin1_Clr();
	TestPin2_Clr();
}
/*****************************************************************************/

#endif
