#include "at25df021.h"

#include "types.h"
#include "core.h"
#include "CRC16.h"
#include "SPI.h"
#include "hw_conf.h"

#define NUM_SECTORS 	64			/* number of sectors in the flash device */
#define SECTOR_SIZE		4096

static char 	*pFlashDesc =		"Atmel AT25DF021";
static char 	*pDeviceCompany	=	"Atmel Corporation";

static int		gNumSectors = NUM_SECTORS;

#ifndef WIN32
static u32 DSP_FS_MASK[] = { DSP_FS };
#endif

#ifdef WIN32

#elif defined(CPU_SAME53)

static S_SPIM	spimem(PIO_DSP_SPCK, PIO_DSP_MOSI, PIO_DSP_MISO, PIO_DSP_FS, DSP_SPCK, DSP_MOSI, DSP_MISO, DSP_PMUX_SPCK, DSP_PMUX_MOSI, DSP_PMUX_MISO,
					DSP_FS_MASK, ArraySize(DSP_FS_MASK), DSP_DIPO_BITS, DSP_DOPO_BITS, DSP_GEN_SRC, DSP_GEN_CLK, &DSP_DMATX, &DSP_DMARX);

#elif defined(CPU_XMC48)

#endif 

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <at25df021_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
