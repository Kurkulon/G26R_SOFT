#include "at25df021.h"

#include "types.h"
#include "core.h"
#include "CRC16.h"
#include "SPI.h"
#include "hw_conf.h"

#define FLASH_START_ADR 0 	
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

static S_SPIM	spimem(DSP_SERCOM_NUM, PIO_DSP_SPCK, PIO_DSP_MOSI, PIO_DSP_MISO, PIO_DSP_FS, DSP_SPCK, DSP_MOSI, DSP_MISO, DSP_PMUX_SPCK, DSP_PMUX_MOSI, DSP_PMUX_MISO,
					DSP_FS_MASK, ArraySize(DSP_FS_MASK), DSP_DIPO_BITS, DSP_DOPO_BITS, DSP_GEN_SRC, DSP_GEN_CLK, &DSP_DMATX, &DSP_DMARX);

#elif defined(CPU_XMC48)

#endif 

static u16 flashCRC = 0;
static u32 flashLen = 0;
static u16 lastErasedBlock = ~0;
static bool flashOK = false;
static bool flashChecked = false;
static bool flashCRCOK = false;
static byte buf[SECTOR_SIZE];


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <at25df021_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool at25df021_Init(u32 baudrate)
{
	return spimem.Connect(baudrate);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void at25df021_Destroy()
{
	return spimem.Disconnect();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static byte ReadStatusRegister(void)
{
	spimem.ChipSelect(0);

	spimem.WriteReadByte(SPI_RDSR);

	byte usStatus = spimem.WriteReadByte(0);

	spimem.ChipDisable();

	return usStatus;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static ERROR_CODE Wait_For_Status(byte Statusbit)
{
	volatile int n, i;
	char status_register = 0xFF;
	ERROR_CODE ErrorCode = NO_ERR;	// tells us if there was an error erasing flash

	for(i = 0; i < TIMEOUT; i++)
	{
		status_register = ReadStatusRegister();
		if( !(status_register & Statusbit) )
		{
			ErrorCode = NO_ERR;	// tells us if there was an error erasing flash
			break;
		}

		delay(DELAY);

		ErrorCode = POLL_TIMEOUT;	// Time out error
 
		HW::WDT->Update();
	};

	return ErrorCode;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static ERROR_CODE Wait_For_WEL()
{
	volatile int n, i;
	char status_register = 0;
	ERROR_CODE ErrorCode = NO_ERR;	// tells us if there was an error erasing flash

	for(i = 0; i < 35; i++)
	{
		status_register = ReadStatusRegister();

		if( (status_register & WEL) )
		{
			ErrorCode = NO_ERR;	// tells us if there was an error erasing flash
			break;
		};

		delay(DELAY);

		ErrorCode = POLL_TIMEOUT;	// Time out error

		HW::WDT->Update();
	};

	return ErrorCode;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void __SendSingleCommand(byte iCommand)
{
	spimem.ChipSelect(0);

	spimem.WriteReadByte(iCommand);

	spimem.ChipDisable();

	delay(100);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CmdWriteEnable()
{
	__SendSingleCommand(SPI_WREN);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CmdWriteDisable()
{
	__SendSingleCommand(SPI_WRDI);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CmdEraseSector(u32 sec)
{
	spimem.ChipSelect(0);

	sec *= SECTOR_SIZE;

	spimem.WriteReadByte(SPI_SE);

	spimem.WriteReadByte(sec >> 16);
	spimem.WriteReadByte(sec >> 8);
	spimem.WriteReadByte(sec);

	spimem.ChipDisable();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CmdWriteStatusReg(byte stat)
{
	spimem.ChipSelect(0);

	spimem.WriteReadByte(SPI_WRSR);
	spimem.WriteReadByte(stat);

	spimem.ChipDisable();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void GlobalUnProtect()
{
	CmdWriteEnable();

	CmdWriteStatusReg(0);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u32 GetNumSectors()
{
	return gNumSectors;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ERROR_CODE EraseBlock(u32 sec)
{
	ERROR_CODE 	  ErrorCode   = NO_ERR;		//tells us if there was an error erasing flash

	GlobalUnProtect();
	GlobalUnProtect();

	CmdWriteEnable();

	ErrorCode = Wait_For_WEL();

    if (ErrorCode == NO_ERR)
	{
		CmdEraseSector(sec);

		ErrorCode = Wait_For_Status(RDY_BSY);
	};

	return ErrorCode;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static ERROR_CODE WritePage(void *data, u32 stAdr, u16 count )
{
	static byte buf[4];

    ERROR_CODE Result = NO_ERR;

	if ((stAdr & 0xFF) != 0 || count > 256 || count == 0)
	{
		return INVALID_BLOCK;
	};

	u16 block = stAdr/SECTOR_SIZE;

	if (lastErasedBlock != block)
	{
		Result = EraseBlock(block);

		if (Result != NO_ERR) return Result;

		lastErasedBlock = block;
	};

	CmdWriteEnable();

	Result = Wait_For_WEL();

    if(Result != NO_ERR)
	{
		return Result;
	}
    else
    {
		spimem.ChipSelect(0);

		spimem.WriteReadByte(SPI_PP);
		spimem.WriteReadByte(stAdr >> 16);
		spimem.WriteReadByte(stAdr >> 8);
		spimem.WriteReadByte(stAdr);

		spimem.WriteSyncDMA(data, count);

		spimem.ChipDisable();
    };

	return Wait_For_Status(RDY_BSY);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static ERROR_CODE VerifyPage(const byte *data, u32 stAdr, u16 count )
{
    ERROR_CODE Result = NO_ERR;

	if ((stAdr & 0xFF) != 0 || count > 256 || count == 0)
	{
		return INVALID_BLOCK;
	};

	spimem.ChipSelect(0);

    spimem.WriteReadByte(SPI_FAST_READ);
    spimem.WriteReadByte((stAdr) >> 16);
    spimem.WriteReadByte((stAdr) >> 8);
    spimem.WriteReadByte(stAdr);
    spimem.WriteReadByte(0);

	for ( ; count > 0; count--)
	{
		if (spimem.WriteReadByte(0) != *data)
		{
			Result = VERIFY_WRITE;
			break;
		};

		data++;
	};

	spimem.ChipDisable();

	return Result;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ERROR_CODE at25df021_Read(void *data, u32 stAdr, u16 count )
{
	static byte buf[5];

    buf[0] = SPI_FAST_READ;
    buf[1] = stAdr >> 16;
    buf[2] = stAdr >> 8;
    buf[3] = stAdr;
    buf[4] = 0;

	spimem.ChipSelect(0);

	spimem.WritePIO(buf, sizeof(buf));

	spimem.ReadSyncDMA(data, count);

	spimem.ChipDisable();

	return NO_ERR;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 at25df021_GetCRC16(u32 stAdr, u16 count)
{
	DataCRC crc;

	crc.w = 0xFFFF;

	byte t = 0;

	static byte buf[5];

    buf[0] = SPI_FAST_READ;
    buf[1] = stAdr >> 16;
    buf[2] = stAdr >> 8;
    buf[3] = stAdr;
    buf[4] = 0;

	spimem.ChipSelect(0);

	spimem.WritePIO(buf, sizeof(buf));

	for ( ; count > 0; count--)
	{
		t = spimem.WriteReadByte(0);

		crc.w = tableCRC[crc.b[0] ^ t] ^ crc.b[1];
		
		HW::WDT->Update();
	};
	
	spimem.ChipDisable();

	return crc.w;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ERROR_CODE at25df021_Write(const byte *data, u32 stAdr, u32 count, bool verify)
{
    ERROR_CODE Result = NO_ERR;

	u32 c;

	while (count > 0)
	{
		u16 c = (count >= 256) ? 256 : count;

		count -= c;

		Result = WritePage((void*)data, stAdr, c);

		if (Result != NO_ERR) break;

		if (verify)
		{
			Result = VerifyPage(data, stAdr, c);
			if (Result != NO_ERR) break;
		};

		data += c;
		stAdr += c;

		HW::WDT->Update();
    };

    return(Result);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool BlackFin_CheckFlash(u16 *pcrc, u16 *plen)
{
	static ADI_BOOT_HEADER bh;
	static u16 crc = 0;

//	if (FlashBusy()) return;

	u32 *p = (u32*)&bh;

	u32 adr = 0;
	
	flashOK = flashChecked = flashCRCOK = false;

	//at25df021_Read(buf, FLASH_START_ADR, sizeof(buf));

	while (1)
	{
		at25df021_Read(&bh, FLASH_START_ADR + adr, sizeof(bh));

		u32 x = p[0] ^ p[1] ^ p[2] ^ p[3];
		x ^= x >> 16; 
		x = (x ^ (x >> 8)) & 0xFF; 

		if (((u32)(bh.dBlockCode) >> 24) == 0xAD && x == 0)
		{
			adr += sizeof(bh);

			if ((bh.dBlockCode & BFLAG_FILL) == 0)
			{
				adr += bh.dByteCount;	
			};

			if (bh.dBlockCode & BFLAG_FINAL)
			{
				flashOK = true;

				break;
			};
		}
		else
		{
			break;
		};
	};

	flashLen = adr;

	at25df021_Read(&crc, FLASH_START_ADR + adr, sizeof(crc));

	if (flashLen > 0) flashCRC = at25df021_GetCRC16(FLASH_START_ADR, flashLen), flashCRCOK = (flashCRC == crc);

	if (!flashCRCOK) flashLen = 0;

	flashChecked = true;

	if (pcrc != 0) *pcrc = flashCRC;
	if (plen != 0) *plen = flashLen;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
