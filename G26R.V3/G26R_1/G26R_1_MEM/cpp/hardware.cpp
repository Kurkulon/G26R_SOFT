//#include "types.h"
#include "core.h"
//#include "time.h"
//#include "COM_DEF.h"
//#include "CRC\CRC16_8005.h"
//#include "list.h"
//#include "PointerCRC.h"

#include "hardware.h"
//#include "SEGGER_RTT\SEGGER_RTT.h"
//#include "hw_conf.h"
//#include "hw_rtm.h"
//#include "hw_nand.h"
//#include "DMA\DMA.h"
//#include "MANCH\manch.h"

#ifdef WIN32

#include <windows.h>
#include <Share.h>
#include <conio.h>
#include <stdarg.h>
#include <stdio.h>
#include <intrin.h>
//#include "CRC\CRC16_CCIT.h"
#include "list.h"

//static HANDLE handleNandFile;
//static const char nameNandFile[] = "NAND_FLASH_STORE.BIN";
//
//static HANDLE handleWriteThread;
//static HANDLE handleReadThread;
//
//static byte nandChipSelected = 0;
//
//static u64 curNandFilePos = 0;
////static u64 curNandFileBlockPos = 0;
//static u32 curBlock = 0;
//static u32 curRawBlock = 0;
//static u16 curPage = 0;
//static u16 curCol = 0;
//
//static OVERLAPPED	_overlapped;
//static u32			_ovlReadedBytes = 0;
//static u32			_ovlWritenBytes = 0;
//
//static void* nandEraseFillArray;
//static u32 nandEraseFillArraySize = 0;
//static byte nandReadStatus = 0x41;
//static u32 lastError = 0;
//
//
//static byte fram_I2c_Mem[0x10000];
//static byte fram_SPI_Mem[0x40000];
//
//static bool fram_spi_WREN = false;

//static u16 crc_ccit_result = 0;


//struct BlockBuffer { BlockBuffer *next; u32 block; u32 prevBlock; u32 writed; u32 data[((NAND_PAGE_SIZE+NAND_SPARE_SIZE) << NAND_PAGE_BITS) >> 2]; };

//static BlockBuffer _blockBuf[16];
//
//static List<BlockBuffer> freeBlockBuffer;
//static List<BlockBuffer> rdBlockBuffer;
//static List<BlockBuffer> wrBlockBuffer;
//
//static BlockBuffer *curNandBlockBuffer[4] = { 0 };
//
//static volatile bool busyWriteThread = false;

#elif defined(CPU_SAME53)

//__align(16) T_HW::DMADESC DmaTable[32];
//__align(16) T_HW::DMADESC DmaWRB[32];

#elif defined(CPU_XMC48)

//#pragma O3
//#pragma Otime

#endif 

#define GEAR_RATIO 12.25

//const u16 pulsesPerHeadRoundFix4 = GEAR_RATIO * 6 * 16;

//const u16 testNandChipMask = 0xFFFF;

static volatile u32 shaftCounter = 0;
static volatile u32 shaftPrevTime = 0;
static volatile u32 shaftCount = 0;
static volatile u32 shaftTime = 0;
u16 shaftRPS = 0;
volatile u16 curShaftCounter = 0;

//static bool busy_CRC_CCITT_DMA = false;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//static void InitVectorTable();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

__forceinline 	void EnableVCORE()	{ PIO_ENVCORE->SET(ENVCORE); 	}
__forceinline 	void DisableVCORE()	{ PIO_ENVCORE->CLR(ENVCORE); 	}


#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <ARM\system_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <Clock_imp.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <CRC\CRC_CCITT_DMA_IMP.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void WDT_Init()
{
	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "WDT Init ... ");

	#ifdef CPU_SAME53	

		//HW::MCLK->APBAMASK |= APBA_WDT;

		//HW::WDT->CONFIG = WDT_WINDOW_CYC512|WDT_PER_CYC1024;
	
		#ifndef _DEBUG
		HW::WDT->CTRLA = WDT_ENABLE;
		#else
		HW::WDT->CTRLA = 0;
		#endif

		//while(HW::WDT->SYNCBUSY);

	#elif defined(CPU_XMC48)

		#ifndef _DEBUG
	
		//HW::WDT_Enable();

		//HW::WDT->WLB = OFI_FREQUENCY/2;
		//HW::WDT->WUB = (3 * OFI_FREQUENCY)/2;
		//HW::SCU_CLK->WDTCLKCR = 0|SCU_CLK_WDTCLKCR_WDTSEL_OFI;

		//HW::WDT->CTR = WDT_CTR_ENB_Msk|WDT_CTR_DSP_Msk;

		#else

		HW::WDT_Disable();

		#endif

	#endif

	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "OK\n");
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef WIN32

//extern dword DI;
//extern dword DO;
char pressedKey;
//extern  dword maskLED[16];

//byte _array1024[0x60000]; 

HWND  hMainWnd;

HCURSOR arrowCursor;
HCURSOR handCursor;

HBRUSH	redBrush;
HBRUSH	yelBrush;
HBRUSH	grnBrush;
HBRUSH	gryBrush;

RECT rectLed[10] = { {20, 41, 33, 54}, {20, 66, 33, 79}, {20, 91, 33, 104}, {21, 117, 22, 118}, {20, 141, 33, 154}, {218, 145, 219, 146}, {217, 116, 230, 129}, {217, 91, 230, 104}, {217, 66, 230, 79}, {217, 41, 230, 54}  }; 
HBRUSH* brushLed[10] = { &yelBrush, &yelBrush, &yelBrush, &gryBrush, &grnBrush, &gryBrush, &redBrush, &redBrush, &redBrush, &redBrush };

//int x,y,Depth;

HFONT font1;
HFONT font2;

HDC memdc;
HBITMAP membm;

//HANDLE facebitmap;

static const u32 secBufferWidth = 80;
static const u32 secBufferHeight = 12;
static const u32 fontWidth = 12;
static const u32 fontHeight = 16;


static char secBuffer[secBufferWidth*secBufferHeight*2];

static u32 pallete[16] = {	0x000000,	0x800000,	0x008000,	0x808000,	0x000080,	0x800080,	0x008080,	0xC0C0C0,
							0x808080,	0xFF0000,	0x00FF00,	0xFFFF00,	0x0000FF,	0xFF00FF,	0x00FFFF,	0xFFFFFF };

const char lpAPPNAME[] = "¿—76÷_”œ–";

int screenWidth = 0, screenHeight = 0;

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static __int64		tickCounter = 0;

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

static void InitDisplay()
{
	WNDCLASS		    wcl;

	wcl.hInstance		= NULL;
	wcl.lpszClassName	= lpAPPNAME;
	wcl.lpfnWndProc		= WindowProc;
	wcl.style	    	= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	wcl.hIcon	    	= NULL;
	wcl.hCursor	    	= NULL;
	wcl.lpszMenuName	= NULL;

	wcl.cbClsExtra		= 0;
	wcl.cbWndExtra		= 0;
	wcl.hbrBackground	= NULL;

	RegisterClass (&wcl);

	int sx = screenWidth = GetSystemMetrics (SM_CXSCREEN);
	int sy = screenHeight = GetSystemMetrics (SM_CYSCREEN);

	hMainWnd = CreateWindowEx (0, lpAPPNAME, lpAPPNAME,	WS_DLGFRAME|WS_POPUP, 0, 0,	640, 480, NULL,	NULL, NULL, NULL);

	if(!hMainWnd) 
	{
		cputs("Error creating window\r\n");
		exit(0);
	};

	RECT rect;

	if (GetClientRect(hMainWnd, &rect))
	{
		MoveWindow(hMainWnd, 0, 0, 641 + 768 - rect.right - 2, 481 - rect.bottom - 1 + 287, true);
	};

	ShowWindow (hMainWnd, SW_SHOWNORMAL);

	font1 = CreateFont(30, 14, 0, 0, 100, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_DONTCARE, "Lucida Console");
	font2 = CreateFont(fontHeight, fontWidth-2, 0, 0, 100, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_DONTCARE, "Lucida Console");

	if (font1 == 0 || font2 == 0)
	{
		cputs("Error creating font\r\n");
		exit(0);
	};

	GetClientRect(hMainWnd, &rect);
	HDC hdc = GetDC(hMainWnd);
    memdc = CreateCompatibleDC(hdc);
	membm = CreateCompatibleBitmap(hdc, rect.right - rect.left + 1, rect.bottom - rect.top + 1 + secBufferHeight*fontHeight);
    SelectObject(memdc, membm);
	ReleaseDC(hMainWnd, hdc);


	arrowCursor = LoadCursor(0, IDC_ARROW);
	handCursor = LoadCursor(0, IDC_HAND);

	if (arrowCursor == 0 || handCursor == 0)
	{
		cputs("Error loading cursors\r\n");
		exit(0);
	};

	LOGBRUSH lb;

	lb.lbStyle = BS_SOLID;
	lb.lbColor = RGB(0xFF, 0, 0);

	redBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0xFF, 0xFF, 0);

	yelBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0x7F, 0x7F, 0x7F);

	gryBrush = CreateBrushIndirect(&lb);

	lb.lbColor = RGB(0, 0xFF, 0);

	grnBrush = CreateBrushIndirect(&lb);

	for (u32 i = 0; i < sizeof(secBuffer); i+=2)
	{
		secBuffer[i] = 0x20;
		secBuffer[i+1] = 0xF0;
	};
}

#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

void UpdateDisplay()
{
	static byte curChar = 0;
	//static byte c = 0, i = 0;
	//static byte flashMask = 0;
	static const byte a[4] = { 0x80, 0x80+0x40, 0x80+20, 0x80+0x40+20 };

	MSG msg;

	static dword pt = 0;
	static TM32 tm;

	u32 t = GetTickCount();

	if ((t-pt) >= 2)
	{
		pt = t;

		while(PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			GetMessage (&msg, NULL, 0, 0);

			TranslateMessage (&msg);

			DispatchMessage (&msg);
		};
	};

	static char buf[80];
	static char buf1[sizeof(secBuffer)];

	if (tm.Check(20))
	{
		bool rd = true;

		if (rd)
		{
			//SelectObject(memdc, font1);
			//SetBkColor(memdc, 0x074C00);
			//SetTextColor(memdc, 0x00FF00);
			//TextOut(memdc, 443, 53, buf, 20);
			//TextOut(memdc, 443, 30*1+53, buf+20, 20);
			//TextOut(memdc, 443, 30*2+53, buf+40, 20);
			//TextOut(memdc, 443, 30*3+53, buf+60, 20);

			SelectObject(memdc, font2);

			for (u32 j = 0; j < secBufferHeight; j++)
			{
				for (u32 i = 0; i < secBufferWidth; i++)
				{
					u32 n = (j*secBufferWidth+i)*2;

					u8 t = secBuffer[n+1];

					SetBkColor(memdc, pallete[(t>>4)&0xF]);
					SetTextColor(memdc, pallete[t&0xF]);
					TextOut(memdc, i*fontWidth, j*fontHeight+287, secBuffer+n, 1);
				};
			};

			RedrawWindow(hMainWnd, 0, 0, RDW_INVALIDATE);

		}; // if (rd)

	}; // if ((t-pt) > 10)

}

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

LRESULT CALLBACK WindowProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;
	HDC hdc;
	PAINTSTRUCT ps;
	bool c;
	static int x, y;
	int x0, y0, r0;
	static char key = 0;
	static char pkey = 0;

	RECT rect;

	static bool move = false;
	static int movex, movey = 0;

//	char *buf = (char*)screenBuffer;

    switch (message)
	{
        case WM_CREATE:

            break;

	    case WM_DESTROY:

		    break;

        case WM_PAINT:

			if (GetUpdateRect(hWnd, &rect, false)) 
			{
				hdc = BeginPaint(hWnd, &ps);

				//printf("Update RECT: %li, %li, %li, %li\r\n", ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);

				c = BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left + 1, ps.rcPaint.bottom - ps.rcPaint.top + 1, memdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
	 
				EndPaint(hWnd, &ps);
			};

            break;

        case WM_CHAR:

			pressedKey = wParam;

			if (pressedKey == '`')
			{
				GetWindowRect(hWnd, &rect);

				if ((rect.bottom-rect.top) > 340)
				{
					MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top-fontHeight*secBufferHeight, true); 
				}
				else
				{
					MoveWindow(hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top+fontHeight*secBufferHeight, true); 
				};
			};

            break;

		case WM_MOUSEMOVE:

			x = LOWORD(lParam); y = HIWORD(lParam);

			if (move)
			{
				GetWindowRect(hWnd, &rect);
				SetWindowPos(hWnd, HWND_TOP, rect.left+x-movex, rect.top+y-movey, 0, 0, SWP_NOSIZE); 
			};

			return 0;

			break;

		case WM_MOVING:

			return TRUE;

			break;

		case WM_SYSCOMMAND:

			return 0;

			break;

		case WM_LBUTTONDOWN:

			move = true;
			movex = x; movey = y;

			SetCapture(hWnd);

			return 0;

			break;

		case WM_LBUTTONUP:

			move = false;

			ReleaseCapture();

			return 0;

			break;

		case WM_MBUTTONDOWN:

			ShowWindow(hWnd, SW_MINIMIZE);

			return 0;

			break;

		case WM_ACTIVATE:

			if (HIWORD(wParam) != 0 && LOWORD(wParam) != 0)
			{
				ShowWindow(hWnd, SW_NORMAL);
			};

			break;

		case WM_CLOSE:

			//run = false;

			break;
	};
    
	return DefWindowProc(hWnd, message, wParam, lParam);
}

#endif		
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef WIN32

int PutString(u32 x, u32 y, byte c, const char *str)
{
	char *dst = secBuffer+(y*secBufferWidth+x)*2;
	dword i = secBufferWidth-x;

	while (*str != 0 && i > 0)
	{
		*(dst++) = *(str++);
		*(dst++) = c;
		i -= 1;
	};

	return secBufferWidth-x-i;
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef	WIN32

int Printf(u32 xx, u32 yy, byte c, const char *format, ... )
{
	char buf[1024];

	va_list arglist;

    va_start(arglist, format);
    vsprintf(buf, format, arglist);
    va_end(arglist);

	return PutString(xx, yy, c, buf);
}

#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i32	Get_FRAM_SPI_SessionsAdr() { return FRAM_SPI_SESSIONS_ADR; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

i32	Get_FRAM_I2C_SessionsAdr() { return FRAM_I2C_SESSIONS_ADR; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitHardware()
{
	SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_YELLOW "Hardware Init ... \n");

#ifdef CPU_SAME53	
	
	using namespace HW;

	#ifdef _DEBUG

		SEGGER_RTT_printf(0, "HW::WDT->CTRLA = 0x%02X\n", (u32)(HW::WDT->CTRLA));

		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_GREEN "Disable WDT ... ");

		HW::MCLK->APBAMASK |= APBA_WDT;

		while(HW::WDT->SYNCBUSY);

		if (HW::WDT->CTRLA & WDT_ENABLE)
		{
			HW::WDT->CTRLA = 0;
			
			while(HW::WDT->SYNCBUSY);
		};

		SEGGER_RTT_WriteString(0, "OK\n");

	#endif

//	HW::PIOA->BSET(13);

	HW::GCLK->GENCTRL[GEN_32K]	= GCLK_DIV(1)	|GCLK_SRC_OSCULP32K	|GCLK_GENEN;

	HW::GCLK->GENCTRL[GEN_1M]	= GCLK_DIV(25)	|GCLK_SRC_XOSC1		|GCLK_GENEN		|GCLK_OE;

	HW::GCLK->GENCTRL[GEN_25M]	= GCLK_DIV(1)	|GCLK_SRC_XOSC1		|GCLK_GENEN;

//	HW::GCLK->GENCTRL[GEN_500K] = GCLK_DIV(50)	|GCLK_SRC_XOSC1		|GCLK_GENEN;

	PIO_32kHz->SetWRCONFIG(1UL<<PIN_32kHz, PORT_PMUX_M|PORT_WRPINCFG|PORT_PMUXEN|PORT_WRPMUX|PORT_PULLEN);

	HW::GCLK->GENCTRL[GEN_EXT32K]	= GCLK_DIV(1)	|GCLK_SRC_GCLKIN	|GCLK_GENEN		;


	HW::MCLK->APBAMASK |= APBA_EIC;
	HW::GCLK->PCHCTRL[GCLK_EIC] = GCLK_GEN(GEN_MCK)|GCLK_CHEN;

	EIC->CTRLA = EIC_SWRST; while(EIC->SYNCBUSY);

	HW::MCLK->APBBMASK |= APBB_EVSYS;
	EVSYS->CTRLA = EVSYS_SWRST;

	HW::GCLK->PCHCTRL[GCLK_SERCOM_SLOW]		= GCLK_GEN(GEN_32K)|GCLK_CHEN;	// 32 kHz

#endif

	Init_time(MCK);
	RTT_Init();
	I2C_Init();
	SPI_Init();

#ifndef WIN32

	InitClock();

	//InitManTransmit();
	InitManRecieve();
	//Init_CRC_CCITT_DMA();

	InitManTransmit();

	EnableVCORE();

	WDT_Init();

#else

	InitDisplay();

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateHardware()
{
#ifndef WIN32

	static byte i = 0;

	static Deb db(false, 20);

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( SPI_Update();		);
		CALL( I2C_Update();		);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
