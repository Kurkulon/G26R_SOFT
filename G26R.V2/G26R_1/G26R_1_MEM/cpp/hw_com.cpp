//#pragma O3
//#pragma Otime

//#include <stdio.h>
//#include <conio.h>

#include <ComPort.h>
#include "hw_conf.h"
#include "hw_com.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef CPU_SAME53

ComPort comdsp	(UART2_DSP, 0, PIO_UTXD2, PIO_URXD2, PIO_URXD2, 0, PIN_UTXD2, PIN_URXD2, 0, 0, PMUX_UTXD2, PMUX_URXD2, UART2_TXPO, UART2_RXPO, UART2_GEN_SRC, UART2_GEN_CLK, &UART2_DMA);
ComPort commoto	(UART0_LPC, 0, PIO_UTXD0, PIO_URXD0, PIO_URXD0, 0, PIN_UTXD0, PIN_URXD0, 0, 0, PMUX_UTXD0, PMUX_URXD0, UART0_TXPO, UART0_RXPO, UART0_GEN_SRC, UART0_GEN_CLK, &UART0_DMA);

#elif defined(CPU_XMC48)

#elif defined(WIN32)

ComPort comdsp	;
ComPort commoto	;

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <ComPort_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
