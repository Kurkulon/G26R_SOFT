#ifndef WIN32

#include "SPI.h"
#include "hw_conf.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_SAME53

static u32 DSP_FS_MASK[] = { DSP_FS };

static S_SPIM	spi4(DSP_SERCOM_NUM, PIO_DSP_SPCK, PIO_DSP_MOSI, PIO_DSP_MISO, PIO_DSP_FS, DSP_SPCK, DSP_MOSI, DSP_MISO, DSP_PMUX_SPCK, DSP_PMUX_MOSI, DSP_PMUX_MISO,
					DSP_FS_MASK, ArraySize(DSP_FS_MASK), DSP_DIPO_BITS, DSP_DOPO_BITS, DSP_GEN_SRC, DSP_GEN_CLK, &DSP_DMATX, &DSP_DMARX);

#elif defined(CPU_XMC48)

#endif 

#define FLASH_START_ADR 0
#define ADSP_CHECKFLASH
//#define ADSP_CRC_PROTECTION
#define AT25_SPI_BAUD_RATE 	5000000
#define FLASHSPI_WRITESYNC

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <FLASH\FlashSPI_imp_v2.h>

FlashSPI dspFlash(spi4, 0);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void at25df021_Init()
{
	dspFlash.Init(); 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void at25df021_Destroy()
{
	return spi4.Disconnect();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool at25df021_Write(const byte *data, u32 stAdr, u32 count, bool verify)
{
	ERROR_CODE Result = dspFlash.WriteSync(data, stAdr, count, verify);

    return Result == NO_ERR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool BlackFin_CheckFlash(u16 *pcrc, u16 *plen)
{
	if (pcrc != 0) *pcrc = dspFlash.flashCRC;
	if (plen != 0) *plen = dspFlash.flashLen;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // #ifndef WIN32

