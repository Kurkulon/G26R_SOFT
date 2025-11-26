#ifndef G_HW_CONF_H__29_07_2024__12_45
#define G_HW_CONF_H__29_07_2024__12_45

#pragma once

#include <types.h>
#include <core.h>

#define MCK_MHz 200
#define MCK (MCK_MHz*1000000)
#define NS2CLK(x) (((x)*MCK_MHz+500)/1000)
#define US2CLK(x) ((x)*MCK_MHz)
#define MS2CLK(x) ((x)*MCK_MHz*1000)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define FRAM_SPI_MAINVARS_ADR 0
#define FRAM_SPI_SESSIONS_ADR 0x200

#define FRAM_I2C_MAINVARS_ADR 0
#define FRAM_I2C_SESSIONS_ADR 0x200

#define REAL_TIME_CLOCK_ENABLE
#define TRAP_PACKET_VERSION		7

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_SAME53	

	// Test Pins
	// 13	- PB06	- ManTrmIRQ2
	// 14	- PB07	
	// 22	- PC06	
	// 23	- PC07	
	// 29	- PA11	
	// 37	- PB15	- SPI_Handler
	// 41	- PC11
	// 42	- PC12
	// 52	- PA16
	// 56	- PC16
	// 57	- PC17
	// 58	- PC18
	// 59	- PC19
	// 61	- PC21
	// 64	- PB16
	// 65	- PB17
	// 66	- PB18	- ManRcvIRQ sync true
	// 74	- PA24	- ManRcvIRQ
	// 75	- PA25	- main loop
	// 82	- PC24
	// 84	- PC26
	// 87	- PA27


	// ++++++++++++++	GEN	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define GEN_MCK				0
	#define GEN_32K				1
	#define GEN_25M				2
	#define GEN_1M				3
	//#define GEN_500K			4
	#define GEN_EXT32K			5

	#define GEN_MCK_CLK			MCK
	#define GEN_32K_CLK			32768
	#define GEN_25M_CLK			25000000
	#define GEN_1M_CLK			1000000
	#define GEN_EXT32K_CLK		32768

	// ++++++++++++++	SERCOM	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define SPI_SERCOM_NUM		0
	#define UART2_DSP			1
//	#define SERCOM_2			2
	#define I2C_SERCOM_NUM		3
	#define DSP_SERCOM_NUM		4
	//#define SERCOM_5			5
	//#define SERCOM_6			6
	#define UART0_LPC			7


	// ++++++++++++++	DMA	0...31	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define	NAND_DMA_CH			0
	#define	UART0_DMA_CH		1
	#define	UART2_DMA_CH		2
	//#define	COM3_DMA		3
	#define	SPI_DMACH_TX_CH		4
	#define	SPI_DMACH_RX_CH		5
	#define	NAND_MEMCOPY_DMA_CH	6
	#define	I2C_DMACH_CH		7
	#define	DSP_DMATX_CH		8
	#define	DSP_DMARX_CH		9
	#define	CRC_DMA_CH			31

	// ++++++++++++++	EVENT 0...31	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define EVENT_NAND_1		0
	//#define EVENT_NAND_2		1
	//#define EVENT_NAND_3		2
	#define EVENT_MANR_1		3
	#define EVENT_MANR_2		4

	// ++++++++++++++	TC	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	//#define	NAND_TC			TC0
	//#define					TC1
	//#define MANT_TC				TC2
	#define MANI_TC				TC3
	//#define 					TC4
	//#define 					TC5
	//#define 					TC6
	//#define NAND_TC			TC7

	#define GEN_TC0_TC1			GEN_MCK
	#define GEN_TC2_TC3			GEN_MCK
	#define GEN_TC4_TC5			GEN_MCK
	#define GEN_TC6_TC7			GEN_MCK

	#define CLK_TC0_TC1			GEN_MCK_CLK
	#define CLK_TC2_TC3			GEN_MCK_CLK
	#define CLK_TC4_TC5			GEN_MCK_CLK
	#define CLK_TC6_TC7			GEN_MCK_CLK

	#define GEN_TC0				GEN_TC0_TC1
	#define GEN_TC1				GEN_TC0_TC1
	#define GEN_TC2				GEN_TC2_TC3
	#define GEN_TC3				GEN_TC2_TC3
	#define GEN_TC4				GEN_TC4_TC5
	#define GEN_TC5				GEN_TC4_TC5
	#define GEN_TC6				GEN_TC6_TC7
	#define GEN_TC7				GEN_TC6_TC7

	#define GCLK_TC0			GCLK_TC0_TC1
	#define GCLK_TC1			GCLK_TC0_TC1
	#define GCLK_TC2			GCLK_TC2_TC3
	#define GCLK_TC3			GCLK_TC2_TC3
	#define GCLK_TC4			GCLK_TC4_TC5
	#define GCLK_TC5			GCLK_TC4_TC5
	#define GCLK_TC6			GCLK_TC6_TC7
	#define GCLK_TC7			GCLK_TC6_TC7

	#define CLK_TC0				CLK_TC0_TC1
	#define CLK_TC1				CLK_TC0_TC1
	#define CLK_TC2				CLK_TC2_TC3
	#define CLK_TC3				CLK_TC2_TC3
	#define CLK_TC4				CLK_TC4_TC5
	#define CLK_TC5				CLK_TC4_TC5
	#define CLK_TC6				CLK_TC6_TC7
	#define CLK_TC7				CLK_TC6_TC7

	// ++++++++++++++	TCC	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define MANT_TCC			TCC0
	#define MANR_TCC			TCC1
	//#define					TCC2
	//#define					TCC3
	#define NAND_TCC			TCC4

	#define GEN_TCC0_TCC1		GEN_MCK
	#define GEN_TCC2_TCC3		GEN_MCK
	#define GEN_TCC4			GEN_MCK

	#define CLK_TCC0_TCC1		GEN_MCK_CLK
	#define CLK_TCC2_TCC3		GEN_MCK_CLK
	#define CLK_TCC4			GEN_MCK_CLK

	#define GEN_TCC0			GEN_TCC0_TCC1
	#define GEN_TCC1			GEN_TCC0_TCC1
	#define GEN_TCC2			GEN_TCC2_TCC3
	#define GEN_TCC3			GEN_TCC2_TCC3


	#define GCLK_TCC0			GCLK_TCC0_TCC1
	#define GCLK_TCC1			GCLK_TCC0_TCC1
	#define GCLK_TCC2			GCLK_TCC2_TCC3
	#define GCLK_TCC3			GCLK_TCC2_TCC3


	#define CLK_TCC0			CLK_TCC0_TCC1
	#define CLK_TCC1			CLK_TCC0_TCC1
	#define CLK_TCC2			CLK_TCC2_TCC3
	#define CLK_TCC3			CLK_TCC2_TCC3

	// ++++++++++++++	I2C	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//	#define I2C					CONCAT2(HW::I2C,I2C_SERCOM_NUM)
	#define PIO_I2C				HW::PIOA 
	#define PIN_SDA				22 
	#define PIN_SCL				23 
	#define SDA					(1<<PIN_SDA) 
	#define SCL					(1<<PIN_SCL) 
	#define I2C_PMUX_SDA		PORT_PMUX_C 
	#define I2C_PMUX_SCL		PORT_PMUX_C 
	#define I2C_GEN_SRC			GEN_MCK
	#define I2C_GEN_CLK			GEN_MCK_CLK
	#define I2C_BAUDRATE		400000 

	//#define I2C_TRIGSRC_RX		CONCAT3(DMCH_TRIGSRC_SERCOM,I2C_SERCOM_NUM,_RX)
	//#define I2C_TRIGSRC_TX		CONCAT3(DMCH_TRIGSRC_SERCOM,I2C_SERCOM_NUM,_TX)

	// ++++++++++++++	SPI	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define PIO_SPCK			HW::PIOA
	#define PIO_MOSI			HW::PIOA
	#define PIO_MISO			HW::PIOA
	#define PIO_CS				HW::PIOB

	#define PIN_SPCK			9
	#define PIN_MOSI			8 
	#define PIN_MISO			10 
	#define PIN_CS0				10 
	#define PIN_CS1				11

	#define SPCK				(1<<PIN_SPCK) 
	#define MOSI				(1<<PIN_MOSI) 
	#define MISO				(1<<PIN_MISO) 
	#define CS0					(1<<PIN_CS0) 
	#define CS1					(1<<PIN_CS1) 

	#define SPI_PMUX_SPCK		PORT_PMUX_C 
	#define SPI_PMUX_MOSI		PORT_PMUX_C 
	#define SPI_PMUX_MISO		PORT_PMUX_C 
	#define SPI_DIPO_BITS		SPI_DIPO(2)
	#define SPI_DOPO_BITS		SPI_DOPO(0) 

	#define SPI_GEN_SRC			GEN_MCK
	#define SPI_GEN_CLK			GEN_MCK_CLK
	#define SPI_BAUDRATE		8000000

	//#define SPI_IRQ				SERCOM0_0_IRQ
	//#define SPI_PID			PID_USIC1

	#define Pin_SPI_IRQ_Set() HW::PIOB->BSET(15)		
	#define Pin_SPI_IRQ_Clr() HW::PIOB->BCLR(15)		

	// ++++++++++++++	DSP SPI	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define PIO_DSP_SPCK		HW::PIOB
	#define PIO_DSP_MOSI		HW::PIOB
	#define PIO_DSP_MISO		HW::PIOB
	#define PIO_DSP_FS			HW::PIOC

	#define PIN_DSP_SPCK		13
	#define PIN_DSP_MOSI		12 
	#define PIN_DSP_MISO		14
	#define PIN_DSP_FS			0 

	#define DSP_SPCK			(1<<PIN_DSP_SPCK) 
	#define DSP_MOSI			(1<<PIN_DSP_MOSI) 
	#define DSP_MISO			(1<<PIN_DSP_MISO) 
	#define DSP_FS				(1<<PIN_DSP_FS) 

	#define DSP_PMUX_SPCK		(PORT_PMUX_C)	//|PORT_DRVSTR 
	#define DSP_PMUX_MOSI		(PORT_PMUX_C)	//|PORT_DRVSTR
	#define DSP_PMUX_MISO		(PORT_PMUX_C)	//|PORT_DRVSTR
	#define DSP_DIPO_BITS		SPI_DIPO(2)
	#define DSP_DOPO_BITS		SPI_DOPO(0) 

	#define DSP_GEN_SRC			GEN_MCK
	#define DSP_GEN_CLK			GEN_MCK_CLK
	#define DSP_BAUDRATE		50000000

	//#define SPI_IRQ				SERCOM0_0_IRQ
	//#define SPI_PID			PID_USIC1

	#define Pin_SPI_IRQ_Set() HW::PIOB->BSET(15)		
	#define Pin_SPI_IRQ_Clr() HW::PIOB->BCLR(15)		

	// ++++++++++++++	MANCH	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	//#define MANR_PRESC_NUM		64
	//#define MANI_PRESC_NUM		64
	//#define MANR_EXTINT			7
	//#define MANR_EXTINT		15
	
	#define PIO_MANCH		HW::PIOC
	#define PIN_L1			13 
	#define PIN_L2			14 
	#define MANCH_PMUX		PORT_PMUX_F
	#define L1_WO_NUM		3
	#define L2_WO_NUM		4

	#define L1				(1UL<<PIN_L1)
	#define L2				(1UL<<PIN_L2)
	#define H1				0
	#define H2				0

	#define PIO_RXD			HW::PIOB
	#define PIN_RXD			23
	#define RXD				(1UL<<PIN_RXD)

	//#define PIO_MANCH		HW::PIOC
	//#define PIN_MAN_TX1		13
	//#define PIN_MAN_TX2		14 

	//#define MAN_TX1			(1UL<<PIN_MAN_TX1)
	//#define MAN_TX2			(1UL<<PIN_MAN_TX2)

	//#define PIO_MANCHRX		HW::PIOA



	//#define ManRxd()		((PIO_MANCH->IN >> PIN_MANCHRX) & 1)

	#define Pin_ManRcvIRQ_Set()	HW::PIOA->BSET(24)
	#define Pin_ManRcvIRQ_Clr()	HW::PIOA->BCLR(24)

	#define Pin_ManTrmIRQ_Set()	HW::PIOB->BSET(6)		
	#define Pin_ManTrmIRQ_Clr()	HW::PIOB->BCLR(6)		

	#define Pin_ManRcvSync_Set()	HW::PIOB->BSET(18)		
	#define Pin_ManRcvSync_Clr()	HW::PIOB->BCLR(18)		

	// ++++++++++++++	NAND Flash	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define PIO_NAND_DATA	HW::PIOA
	#define PIO_ALE			HW::PIOB 
	#define PIO_CLE			HW::PIOB 
	#define PIO_WE_RE		HW::PIOB 
	#define PIO_WP			HW::PIOB 
	#define PIO_FLREADY		HW::PIOB
	#define PIO_FCS			HW::PIOB

	#define PIN_WP			24 
	#define PIN_FLREADY		25 
	#define PIN_FCS0		2 
	#define PIN_FCS1		3 
	#define PIN_FCS2		4 
	#define PIN_FCS3		5 
	#define PIN_WE			30 
	#define PIN_RE			31 
	#define PIN_ALE			0 
	#define PIN_CLE			1 

	#define WP				(1<<PIN_WP) 
	#define FLREADY			(1UL<<PIN_FLREADY) 
	#define FCS0			(1<<PIN_FCS0) 
	#define FCS1			(1<<PIN_FCS1) 
	#define FCS2			(1<<PIN_FCS2) 
	#define FCS3			(1<<PIN_FCS3) 
	#define WE				(1UL<<PIN_WE) 
	#define RE				(1UL<<PIN_RE) 
	#define ALE				(1UL<<PIN_ALE) 
	#define CLE				(1UL<<PIN_CLE) 

	#define PIN_WE_CFG		PINGFG_DRVSTR 
	#define PIN_RE_CFG		PINGFG_DRVSTR 
	#define PIN_ALE_CFG		PINGFG_DRVSTR 
	#define PIN_CLE_CFG		PINGFG_DRVSTR 

	#define NAND_DELAY_WP()		{ delay(4);		}
	#define NAND_DELAY_WH()		{ delay(4);		}
	#define NAND_DELAY_RP()		{ delay(2);		}
	#define NAND_DELAY_REH()	{ delay(2);		}
	#define NAND_DELAY_WHR()	{ delay(10);	}
	#define NAND_DELAY_ADL()	{ delay(20);	}
	#define NAND_DELAY_PR()		{ delay(4);		}

	#define NAND_WE_PER		NS2CLK(60)-1	
	#define NAND_WE_CC0		NS2CLK(40) 
	#define NAND_WE_CC1		NS2CLK(40)

	#define nandTCC			HW::NAND_TCC
	//#define nandTC			HW::NAND_TC

	#ifdef nandTCC
	
		#define NAND_RE_PER_SLOW	(NS2CLK(200)-1)
		#define NAND_RE_CC0_SLOW	NS2CLK(110) 
		#define NAND_RE_CC1_SLOW	NS2CLK(100)

		#define NAND_RE_PER_FAST	(NS2CLK(100)-1)
		#define NAND_RE_CC0_FAST	NS2CLK(55) 
		#define NAND_RE_CC1_FAST	NS2CLK(50)

		#define WE_PORT_PMUX	(PORT_PMUX_F) 
		#define RE_PORT_PMUX	(PORT_PMUX_F) 

		inline void NAND_ClockEnable()  { HW::GCLK->PCHCTRL[CONCAT2(GCLK_,NAND_TCC)] = CONCAT2(GEN_,NAND_TCC)|GCLK_CHEN; HW::MCLK->ClockEnable(CONCAT2(PID_,NAND_TCC)); }

		#define NAND_TRIGSRC_MC0  CONCAT3(DMCH_TRIGSRC_,NAND_TCC,_MC0)
		#define NAND_TRIGSRC_MC1  CONCAT3(DMCH_TRIGSRC_,NAND_TCC,_MC1)

		#define NAND_EVENT_GEN		EVGEN_DMAC_CH_0
		#define NAND_EVSYS_USER		CONCAT3(EVSYS_USER_,NAND_TCC,_EV_1)

	#else

		#define NAND_RE_PER		250	
		//#define NAND_RE_CC0		NS2CLK(35) 
		#define NAND_RE_CC1		NS2CLK(25)

		#define WE_PORT_PMUX	(PORT_PMUX_E) 
		#define RE_PORT_PMUX	(PORT_PMUX_E) 

		inline void NAND_ClockEnable()  { HW::GCLK->PCHCTRL[CONCAT2(GCLK_,NAND_TC)] = CONCAT2(GEN_,NAND_TC)|GCLK_CHEN; HW::MCLK->ClockEnable(CONCAT2(PID_,NAND_TC)); }

		#define NAND_TRIGSRC_MC0	CONCAT3(DMCH_TRIGSRC_,NAND_TC,_MC0)
		#define NAND_TRIGSRC_MC1	CONCAT3(DMCH_TRIGSRC_,NAND_TC,_MC1)

		#define NAND_EVENT_GEN		EVGEN_DMAC_CH_0
		#define NAND_EVSYS_USER		CONCAT3(EVSYS_USER_,NAND_TC,_EVU)

	#endif

	// ++++++++++++++	VCORE	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define PIO_ENVCORE		HW::PIOC
	#define PIN_ENVCORE		5 
	#define ENVCORE			(1<<PIN_ENVCORE) 
	
	// ++++++++++++++	RESET	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define PIN_RESET		10
	#define PIO_RESET		HW::PIOC
	#define RESET			(1<<PIN_RESET)


	// ++++++++++++++	DSP RESET	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define PIN_DSP_RESET		1
	#define PIO_DSP_RESET		HW::PIOC
	#define DSP_RESET			(1<<PIN_DSP_RESET)

	// ++++++++++++++	DSP BOOT MODE	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	//#define PIN_DSP_BMODE1		1
	//#define PIO_DSP_BMODE1		HW::PIOC
	//#define DSP_BMODE1			(1<<PIN_DSP_BMODE1)

	// ++++++++++++++	USART	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define PIO_UTXD0			HW::PIOB 
	#define PIO_URXD0			HW::PIOB 
	#define PIO_UTXD2			HW::PIOC 
	#define PIO_URXD2			HW::PIOC 

	#define PMUX_UTXD0			PORT_PMUX_D
	#define PMUX_URXD0			PORT_PMUX_D 
	#define PMUX_UTXD2			PORT_PMUX_C 
	#define PMUX_URXD2			PORT_PMUX_C 

	#define UART0_TXPO			USART_TXPO_0 
	#define UART0_RXPO			USART_RXPO_1 

	#define UART2_TXPO			USART_TXPO_0 
	#define UART2_RXPO			USART_RXPO_1 

	#define PIN_UTXD0			21 
	#define PIN_URXD0			20 
	#define PIN_UTXD2			27 
	#define PIN_URXD2			28

	#define UTXD0				(1<<PIN_UTXD0) 
	#define URXD0				(1<<PIN_URXD0) 
	#define UTXD2				(1<<PIN_UTXD2) 
	#define URXD2				(1<<PIN_URXD2) 

	#define UART0_GEN_SRC		GEN_MCK
	#define UART0_GEN_CLK		GEN_MCK_CLK

	#define UART2_GEN_SRC		GEN_MCK
	#define UART2_GEN_CLK		GEN_MCK_CLK


	// ++++++++++++++	CLOCK	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define PIO_RTCINT		HW::PIOC
	#define PIN_RTCINT		25 
	#define RTCINT			(1UL<<PIN_RTCINT)
	#define PIO_32kHz		HW::PIOB 
	#define PIN_32kHz		19 
	#define CLOCK_EXTINT	(PIN_RTCINT & 15)
	#define CLOCK_IRQ		(EIC_0_IRQ+CLOCK_EXTINT)

	// ++++++++++++++	EMAC	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define EMAC_PHYA 0

	#define PIO_RESET_PHY	HW::PIOC
	#define PIN_RESET_PHY	15

	#define PIO_GMD			HW::PIOA
	#define PIN_GMDC		20
	#define PIN_GMDIO		21

	#define GMDC			(1UL<<PIN_GMDC) 
	#define GMDIO			(1UL<<PIN_GMDIO) 

	// ++++++++++++++	PIO INIT	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define PIOA_INIT_DIR		(0xFF|PA11|PA16|PA24|PA25|PA27)
	#define PIOA_INIT_SET		(0)
	#define PIOA_INIT_CLR		(0xFF|PA11|PA16|PA24|PA25|PA27)

	#define PIOB_INIT_DIR		(PB06|PB07|PB08|PB18)
	#define PIOB_INIT_SET		(0)
	#define PIOB_INIT_CLR		(PB06|PB07|PB08|PB18)

	#define PIOC_INIT_DIR		(PC00|DSP_RESET|PC02|PC03|ENVCORE|PC06|PC07|RESET|PC11|PC12|PC15|PC17|PC18|PC19|PC21|PC24|PC26|L1|H1|L2|H2)
	#define PIOC_INIT_SET		(DSP_RESET|PC02|H1|H2|PC15)
	#define PIOC_INIT_CLR		(PC00|PC03|ENVCORE|PC06|PC07|RESET|PC11|PC12|PC17|PC18|PC19|PC21|PC24|PC26|L1|L2)

	#define Pin_MainLoop_Set()	HW::PIOA->BSET(25)
	#define Pin_MainLoop_Clr()	HW::PIOA->BCLR(25)
	#define Pin_MainLoop_Tgl()	HW::PIOA->BTGL(25)

#elif defined(WIN32) //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define BAUD2CLK(x)				(x)
	#define MT(v)					(v)
	#define Pin_MainLoop_Set()	
	#define Pin_MainLoop_Clr()	

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




#endif // G_HW_CONF_H__29_07_2024__12_45
