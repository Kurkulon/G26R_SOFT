#include "common.h"
#include "emac.h"
#include "types.h"
#include "main.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u32 reg_BMCR		;
u32 reg_BMSR		;
u32 reg_PHYSID1		;
u32 reg_PHYSID2		;
u32 reg_ADVERTISE	;
u32 reg_LPA			;
u32 reg_EXPANSION  ;
u32 reg_DCOUNTER   ;
u32 reg_FCSCOUNTER ;
u32 reg_NWAYTEST   ;
u32 reg_RERRCOUNTER;
u32 reg_SREVISION  ;
u32 reg_RESV1      ;
u32 reg_LBRERROR   ;
u32 reg_PHYADDR    ;
u32 reg_RESV2      ;
u32 reg_TPISTATUS  ;
u32 reg_NCONFIG    ;

#define READ_PHY_REG(reg) read_phy(AT91C_PHY_ADDR, MII_##reg, &reg_##reg)

AT91S_EMAC emac_regs __attribute__((at(0xFFFDC000)));


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/***********************/
#define NB_RX_BUFFERS			20		//* Number of receive buffers
#define ETH_RX_BUFFER_SIZE		128         
#define NB_TX_BUFFERS			8		//* Number of Transmit buffers // реально их надо 2-3
#define ETH_TX_BUFFER_SIZE		(1524)  	// Чтобы уместить 1480 байта данных

#define ID_EMAC_B			16

#define NO_IPPACKET			0
#define IPPACKET	        	1

#define ARP_REQUEST			0x0001
#define ARP_REPLY			0x0002
#define PROT_ARP			0x0806
#define PROT_IP				0x0800
#define PROT_ICMP			0x01
#define PROT_UDP			17
#define PROT_TCP			6
#define ICMP_ECHO_REQUEST		0x08
#define ICMP_ECHO_REPLY			0x00

#define RCV_OFFSET			0
#define OWNERSHIP_BIT			0x00000001

/* Receive status defintion */
#define AT91C_BROADCAST_ADDR	((unsigned int) (1 << 31))	//* Broadcat address detected
#define AT91C_MULTICAST_HASH 	((unsigned int) (1 << 30))	//* MultiCast hash match
#define AT91C_UNICAST_HASH 		((unsigned int) (1 << 29))	//* UniCast hash match
#define AT91C_EXTERNAL_ADDR		((unsigned int) (1 << 28))	//* External Address match
#define AT91C_SA1_ADDR	    	((unsigned int) (1 << 26))	//* Specific address 1 match
#define AT91C_SA2_ADDR	    	((unsigned int) (1 << 25))	//* Specific address 2 match
#define AT91C_SA3_ADDR	    	((unsigned int) (1 << 24))	//* Specific address 3 match
#define AT91C_SA4_ADDR	    	((unsigned int) (1 << 23))	//* Specific address 4 match
#define AT91C_TYPE_ID	    	((unsigned int) (1 << 22))	//* Type ID match
#define AT91C_VLAN_TAG	    	((unsigned int) (1 << 21))	//* VLAN tag detected
#define AT91C_PRIORITY_TAG    	((unsigned int) (1 << 20))	//* PRIORITY tag detected
#define AT91C_VLAN_PRIORITY    	((unsigned int) (7 << 17))  //* PRIORITY Mask
#define AT91C_CFI_IND        	((unsigned int) (1 << 16))  //* CFI indicator
#define AT91C_EOF           	((unsigned int) (1 << 15))  //* EOF
#define AT91C_SOF           	((unsigned int) (1 << 14))  //* SOF
#define AT91C_RBF_OFFSET     	((unsigned int) (3 << 12))  //* Receive Buffer Offset Mask
#define AT91C_LENGTH_FRAME     	((unsigned int) 0x07FF)     //* Length of frame

/* Transmit Status definition */
#define AT91C_TRANSMIT_OK		((unsigned int) (1 << 31))	//*
#define AT91C_TRANSMIT_WRAP		((unsigned int) (1 << 30))	//* Wrap bit: mark the last descriptor
#define AT91C_TRANSMIT_ERR		((unsigned int) (1 << 29))	//* RLE:transmit error
#define AT91C_TRANSMIT_UND		((unsigned int) (1 << 28))	//* Transmit Underrun
#define AT91C_BUF_EX     		((unsigned int) (1 << 27))	//* Buffers exhausted in mid frame
#define AT91C_TRANSMIT_NO_CRC	((unsigned int) (1 << 16))	//* No CRC will be appended to the current frame
#define AT91C_LAST_BUFFER    	((unsigned int) (1 << 15))	//*

typedef struct  _AT91S_RxTdDescriptor {	//* Receive Transfer descriptor structure
	unsigned int addr;
	union
	{
		unsigned int status;
		struct {
			unsigned int Length:11;
			unsigned int Res0:1;
			unsigned int Rxbuf_off:2;
			unsigned int StartOfFrame:1;
			unsigned int EndOfFrame:1;
			unsigned int Cfi:1;
			unsigned int VlanPriority:3;
			unsigned int PriorityTag:1;
			unsigned int VlanTag:1;
			unsigned int TypeID:1;
			unsigned int Sa4Match:1;
			unsigned int Sa3Match:1;
			unsigned int Sa2Match:1;
			unsigned int Sa1Match:1;
			unsigned int Res1:1;
			unsigned int ExternalAdd:1;
			unsigned int UniCast:1;
			unsigned int MultiCast:1;
			unsigned int BroadCast:1;
		}S_Status;		
	}U_Status;
}AT91S_RxTdDescriptor, *AT91PS_RxTdDescriptor;

typedef struct _AT91S_TxTdDescriptor {	//* Transmit Transfer descriptor structure
	unsigned int addr;
	union
	{
		unsigned int status;
		struct {
			unsigned int Length:11;
			unsigned int Res0:4;
			unsigned int LastBuff:1;
			unsigned int NoCrc:1;
			unsigned int Res1:10;
			unsigned int BufExhausted:1;
			unsigned int TransmitUnderrun:1;
			unsigned int TransmitError:1;
			unsigned int Wrap:1;
			unsigned int BuffUsed:1;
		}S_Status;		
	}U_Status;
}AT91S_TxTdDescriptor, *AT91PS_TxTdDescriptor;


typedef struct _AT91S_EthHdr
{
	unsigned char		et_dest[6];	/* Destination node		*/
	unsigned char		et_src[6];	/* Source node			*/
	unsigned short		et_protlen;	/* Protocol or length		*/
} AT91S_EthHdr, *AT91PS_EthHdr;

typedef struct _AT91S_ArpHdr
{
	unsigned short		ar_hrd;		/* Format of hardware address	*/
	unsigned short		ar_pro;		/* Format of protocol address	*/
	unsigned char		ar_hln;		/* Length of hardware address	*/
	unsigned char		ar_pln;		/* Length of protocol address	*/
	unsigned short		ar_op;		/* Operation			*/
	unsigned char		ar_sha[6];	/* Sender hardware address	*/
	unsigned char		ar_spa[4];	/* Sender protocol address	*/
	unsigned char		ar_tha[6];	/* Target hardware address	*/
	unsigned char		ar_tpa[4];	/* Target protocol address	*/
} AT91S_ArpHdr, *AT91PS_ArpHdr;

//* IP Header structure
typedef struct _AT91S_IPheader {
	unsigned char	ip_hl_v;	/* header length and version	*/
	unsigned char	ip_tos;		/* type of service		*/
	unsigned short	ip_len;		/* total length			*/
	unsigned short	ip_id;		/* identification		*/
	unsigned short	ip_off;		/* fragment offset field	*/
	unsigned char	ip_ttl;		/* time to live			*/
	unsigned char	ip_p;		/* protocol			*/
	unsigned short	ip_sum;		/* checksum			*/
	unsigned char	ip_src[4];	/* Source IP address		*/
	unsigned char	ip_dst[4];	/* Destination IP address	*/
	unsigned short	udp_src;	/* UDP source port		*/
	unsigned short	udp_dst;	/* UDP destination port		*/
	unsigned short	udp_len;	/* Length of UDP packet		*/
	unsigned short	udp_xsum;	/* Checksum			*/
} AT91S_IPheader, *AT91PS_IPheader;

//* Preudo IP Header
typedef struct _AT91S_IPPseudoheader{
        unsigned char	ip_src[4];	/* Source IP address		*/
	unsigned char	ip_dst[4];	/* Destination IP address	*/
	unsigned char   zero;
	unsigned char   proto;
	unsigned short  size;
} AT91S_IPPseudoheader, *AT91PS_IPPseudoheader;

//* ICMP echo header structure
typedef struct _AT91S_IcmpEchoHdr {
    unsigned char   type;       /* type of message */
    unsigned char   code;       /* type subcode */
    unsigned short  cksum;      /* ones complement cksum of struct */
    unsigned short  id;         /* identifier */
    unsigned short  seq;        /* sequence number */
}AT91S_IcmpEchoHdr, *AT91PS_IcmpEchoHdr;

//* UDP header structure
typedef struct _AT91S_UDPHdr {
	unsigned short	udp_src;	/* UDP source port		*/
	unsigned short	udp_dst;	/* UDP destination port		*/
	unsigned short	udp_len;	/* Length of UDP packet		*/
	unsigned short	udp_xsum;	/* Checksum			*/
}AT91S_UDPHdr, *AT91PS_UDPHdr;

typedef struct _AT91S_EthPack
{
	AT91S_EthHdr	EthHdr;
	AT91S_ArpHdr	ArpHdr;
} AT91S_EthPack, *AT91PS_EthPack;

/***********************/
static char OurEmacAddr[6] = OUR_EMAC_ADDR;	// Our Ethernet MAC address and IP address
static unsigned char OurIpAddr[4]  = OUR_IP_ADDR;
static unsigned short OurUDPPort = OUR_UDP_PORT;

static char ComputerEmacAddr[6] = {0,0,0,0,0,0};	// Our Ethernet MAC address and IP address
static unsigned char ComputerIpAddr[4]  = {0,0,0,0};
static unsigned short ComputerUDPPort = 0;
static bool ComputerFind = false;
bool EmacIsConnected = false;

#define EMAC_RX_TDLIST_BASE	0		// +4 - для выравнивания по 4х байтной сетке
#define EMAC_TX_TDLIST_BASE	(16 + EMAC_RX_TDLIST_BASE + NB_RX_BUFFERS * sizeof(AT91S_RxTdDescriptor))
#define EMAC_RX_PACKET_BASE	(16 + EMAC_TX_TDLIST_BASE + NB_TX_BUFFERS * sizeof(AT91S_TxTdDescriptor))
#define EMAC_TX_PACKET_BASE	(16 + EMAC_RX_PACKET_BASE + NB_RX_BUFFERS * ETH_RX_BUFFER_SIZE)
#define EMAC_TX_PACKET_END	(16 + EMAC_TX_PACKET_BASE + NB_TX_BUFFERS * ETH_TX_BUFFER_SIZE)
#define EMAX_RX_TX_BUFFER_SIZE  (16 + EMAC_TX_PACKET_END - EMAC_RX_TDLIST_BASE)

static char RxTxBuffer[EMAX_RX_TX_BUFFER_SIZE];
char *RxPacket = (char *)(RxTxBuffer + EMAC_RX_PACKET_BASE);
char *TxPacket = (char *)(RxTxBuffer + EMAC_TX_PACKET_BASE);

#define MAX_OUR_RX_PACKET_SIZE		2048
static char OurRxPacketBuffer[MAX_OUR_RX_PACKET_SIZE];
char *OurRxPacket = (char *)(OurRxPacketBuffer);
AT91PS_IPheader OurRxPacketIpHeader = (AT91PS_IPheader)(OurRxPacketBuffer + 14);
AT91PS_EthHdr OurRxPacketEthHeader = (AT91PS_EthHdr)(OurRxPacketBuffer);
static bool FirstFragment = false;

AT91PS_RxTdDescriptor RxtdList = (AT91PS_RxTdDescriptor)(RxTxBuffer + EMAC_RX_TDLIST_BASE);
AT91PS_TxTdDescriptor TxtdList = (AT91PS_TxTdDescriptor)(RxTxBuffer + EMAC_TX_TDLIST_BASE);

static unsigned int speedEmac, duplexEmac;
static unsigned int TxBuffIndex = 0;

unsigned int EmacRxCounter;
unsigned int EmacTxCounter;
unsigned int EmacRxError;
unsigned int EmacTxError;

/*******************************/
/*** EMAC lowlevel functions ***/
/*******************************/
void AT91F_Enable_Mdi()	// Enable the MDIO bit in MAC control register
{
	AT91C_BASE_EMAC->EMAC_NCR |= AT91C_EMAC_MPE;	// enable management port
        AT91C_BASE_EMAC->EMAC_NCFGR |= (2)<<10;	        // MDC = MCK/32
}

void AT91F_Disable_Mdi()	// Disable the MDIO bit in the MAC control register
{
	AT91C_BASE_EMAC->EMAC_NCR &= ~AT91C_EMAC_MPE;	// disable management port
}

void write_phy(unsigned char phy_addr, unsigned char address, unsigned int value) // Write value to the a PHY register
{	// Note: MDI interface is assumed to already have been enabled.
	AT91C_BASE_EMAC->EMAC_MAN = ((AT91C_EMAC_SOF & (0x01<<30)) | (2 << 16) | (1 << 28)
		| ((phy_addr & 0x1f) << 23) | (address << 18)) | (value & 0xffff);

	/* Wait until IDLE bit in Network Status register is cleared */
	while (!(AT91C_BASE_EMAC->EMAC_NSR & AT91C_EMAC_IDLE));
}

void read_phy(unsigned char phy_addr, unsigned char address, unsigned int *value)	// Read value stored in a PHY register.
{	// Note: MDI interface is assumed to already have been enabled.
	AT91C_BASE_EMAC->EMAC_MAN = (AT91C_EMAC_SOF & (0x01<<30)) | (2 << 16) | (2 << 28)
		| ((phy_addr & 0x1f) << 23) | (address << 18);

	/* Wait until IDLE bit in Network Status register is cleared */
	while (!(AT91C_BASE_EMAC->EMAC_NSR & AT91C_EMAC_IDLE));
	*value = (AT91C_BASE_EMAC->EMAC_MAN & 0x0000ffff);	
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Read_All_PHY_REGS()
{
	AT91F_Enable_Mdi(/*AT91C_BASE_EMAC*/);	//Enable com between EMAC PHY

	READ_PHY_REG(BMCR		);
	READ_PHY_REG(BMSR		);
	READ_PHY_REG(PHYSID1	);
	READ_PHY_REG(PHYSID2	);
	READ_PHY_REG(ADVERTISE	);
	READ_PHY_REG(LPA		);
	READ_PHY_REG(EXPANSION  );
	READ_PHY_REG(DCOUNTER   );
	READ_PHY_REG(FCSCOUNTER );
	READ_PHY_REG(NWAYTEST   );
	READ_PHY_REG(RERRCOUNTER);
	READ_PHY_REG(SREVISION  );
	READ_PHY_REG(RESV1      );
	READ_PHY_REG(LBRERROR   );
	READ_PHY_REG(PHYADDR    );
	READ_PHY_REG(RESV2      );
	READ_PHY_REG(TPISTATUS  );
	READ_PHY_REG(NCONFIG    );
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/********************************/
/*** EMAC handler functions ***/
/********************************/

unsigned short IcmpChksum(unsigned short *header, unsigned short size)
{
	unsigned int sum = 0;
	unsigned int i;
	for(i=0;i<size;i++)
	{
		sum += SWAP16(*header);
		if(sum&0x00010000){sum&=0xFFFF;sum+=0x1;}
		header++;
	}
	return ~sum;
}

unsigned short NetChksum(unsigned short *sdata, unsigned short len )
{
	unsigned short acc = 0;
	while(len > 1)
	{
		acc += *sdata;	
		if(acc < *sdata) acc++;
		++sdata;
		len -= 2;
	}
  	if(len == 1) 
	{
		acc += SWAP16(((unsigned short)(*(unsigned char *)sdata)) << 8);
		if(acc < SWAP16(((unsigned short)(*(unsigned char *)sdata)) << 8)) 
		{
			++acc;
		}
	}
	return acc;
}

// посчитать контрольную сумму IP-псевдозаголовка
unsigned short PseudoChksum(unsigned char *ip_src, unsigned char *ip_dst, unsigned short size)
{
	AT91S_IPPseudoheader ph;
        ph.ip_src[0] = *(ip_src + 0); 
        ph.ip_src[1] = *(ip_src + 1); 
        ph.ip_src[2] = *(ip_src + 2); 
        ph.ip_src[3] = *(ip_src + 3); 
        ph.ip_dst[0] = *(ip_dst + 0); 
        ph.ip_dst[1] = *(ip_dst + 1); 
        ph.ip_dst[2] = *(ip_dst + 2); 
        ph.ip_dst[3] = *(ip_dst + 3);
	ph.zero = 0;
	ph.proto = PROT_UDP;
	ph.size = SWAP16( size );
        return NetChksum( (unsigned short *)(&ph), sizeof(AT91S_IPPseudoheader) );
}

// сложить две контрольных суммы
unsigned short NetChksumAdd(unsigned short s1, unsigned short s2)
{
	unsigned int s = ((unsigned int)s1<<16)|s2;
	return NetChksum((unsigned short *)(&s), 4 );
}


// посчитать контрольную сумму UDP:
unsigned short UDPChksum(unsigned char *ip_src, unsigned char *ip_dst, unsigned short *pkt, unsigned short size )
{
	return NetChksumAdd(NetChksum((unsigned short *)(pkt), size), PseudoChksum((unsigned char *)ip_src, (unsigned char *)ip_dst, size));
}

// проверить контрольную сумму UDP:
bool UDPChksumVerify(unsigned char *ip_src, unsigned char *ip_dst, unsigned short *pkt, unsigned short size )
{
	AT91PS_UDPHdr h = (AT91PS_UDPHdr)pkt;
	unsigned short tmp = h->udp_xsum;
	unsigned short tmp2 = 0;
	if( tmp == 0 ) { return true; }
	h->udp_xsum = 0;
	tmp2 = NetChksumAdd(NetChksum((unsigned short *)(pkt), size), PseudoChksum((unsigned char *)ip_src, (unsigned char *)ip_dst, size));
        if( NetChksumAdd(tmp, tmp2) == 0xFFFF ) return true;
	return false;
}

unsigned short IPChksum(unsigned short *header, unsigned short size)
{
	return IcmpChksum((unsigned short *)(header), size);
}

// проверить контрольную сумму IP Header:
bool IPChksumVerify(unsigned short *header, unsigned short size )
{
//	AT91PS_IPheader h = (AT91PS_IPheader)header;
	unsigned short tmp;
	tmp = IPChksum((unsigned short *)(header), size);
        if( tmp == 0 ) return true;
	return false;
}

bool ProcessTxEmacPacket();

bool TransmitPacket(char *pData, unsigned short length)	// Send a packet
{
	bool ok = ProcessTxEmacPacket();
	TxtdList[TxBuffIndex].addr = (unsigned int)pData;
	TxtdList[TxBuffIndex].U_Status.S_Status.Length = length;
	TxtdList[TxBuffIndex].U_Status.S_Status.LastBuff = 1;
	if (TxBuffIndex == (NB_TX_BUFFERS - 1))	TxBuffIndex = 0; else TxBuffIndex ++;
	AT91C_BASE_EMAC->EMAC_NCR |= AT91C_EMAC_TSTART;
	return ok;
}

unsigned short TxDataID = 0;

bool EMAC_SendData(char *pData, unsigned short length)
{
	if(!EmacIsConnected) return false; 
	if(!ComputerFind)  return false;
	length += sizeof(AT91S_UDPHdr);
#define BLOK_LEN		1480	// Split to max IPlen=1500
#define MAX_TIME_TO_TRANSMIT_MS 10	// ms, do not write zero!
	unsigned int blok = 0;
	unsigned int sended_len = 0;
	unsigned short checksum;
	unsigned int i,j;
	bool ready = true;
        TxDataID ++;
	AT91S_EthHdr *OurTxPacketEthHeader;
	AT91S_IPheader *OurTxPacketIpHeader;
	while(sended_len < length) 
	{
        	OurTxPacketEthHeader = (AT91S_EthHdr *)((unsigned int)((unsigned int)TxPacket + TxBuffIndex*ETH_TX_BUFFER_SIZE)&0xFFFFFFFC);
		OurTxPacketIpHeader = (AT91S_IPheader *)((unsigned int)OurTxPacketEthHeader + 14);
		// EthHeader
		for(i=0;i<6;i++) 
		{ 
			OurTxPacketEthHeader->et_dest[i] = ComputerEmacAddr[i];
       		        OurTxPacketEthHeader->et_src[i] = OurEmacAddr[i];
		}
		OurTxPacketEthHeader->et_protlen = SWAP16(PROT_IP);
		// IpHeader
		for(i=0;i<4;i++) 
		{ 
			OurTxPacketIpHeader->ip_dst[i] = ComputerIpAddr[i];
			OurTxPacketIpHeader->ip_src[i] = OurIpAddr[i];
		}
		OurTxPacketIpHeader->ip_hl_v 	= OurRxPacketIpHeader->ip_hl_v;	// may be fix  = 0x45
		OurTxPacketIpHeader->ip_tos 	= OurRxPacketIpHeader->ip_tos;  // may be fix  = 0x00
		OurTxPacketIpHeader->ip_id 	= SWAP16(TxDataID);
		OurTxPacketIpHeader->ip_ttl	= 128;
		OurTxPacketIpHeader->ip_p	= PROT_UDP;
		unsigned short offset = 0x0000;
		if(length - sended_len > BLOK_LEN) offset |= 0x2000;
		offset |= (sended_len/8)&0x1FFF;
		OurTxPacketIpHeader->ip_off 	= SWAP16(offset);
		// UDP header
		char *data;
        	if(blok==0)	
		{
			OurTxPacketIpHeader->udp_src 	= SWAP16(OurUDPPort);
			OurTxPacketIpHeader->udp_dst 	= SWAP16(ComputerUDPPort);
			OurTxPacketIpHeader->udp_len 	= SWAP16(length);
			OurTxPacketIpHeader->udp_xsum	= 0;
			checksum = ~NetChksumAdd(NetChksumAdd(NetChksum((unsigned short *)(&(OurTxPacketIpHeader->udp_src)), sizeof(AT91S_UDPHdr)), 
							   PseudoChksum((unsigned char *)(OurTxPacketIpHeader->ip_src), (unsigned char *)(OurTxPacketIpHeader->ip_dst), SWAP16(OurTxPacketIpHeader->udp_len))),
						NetChksum((unsigned short *)(pData), length-sizeof(AT91S_UDPHdr)));
			OurTxPacketIpHeader->udp_xsum	= checksum;
			sended_len += sizeof(AT91S_UDPHdr);
			data = (char *)(&OurTxPacketIpHeader->udp_xsum) + sizeof(OurTxPacketIpHeader->udp_xsum);
			i = sizeof(AT91S_UDPHdr);
		}
		else
		{
			i = 0;
			data = (char *)(&OurTxPacketIpHeader->udp_src);
		}

		i = sended_len + BLOK_LEN - i;
		if (i>=length) i=length;
		while(sended_len < i)
		{
	        	*((unsigned short *)data) = (*((unsigned short *)pData));
			data+=2;
			pData+=2;
			sended_len+=2;
		}
		sended_len = i;
		// IP Header
		unsigned short ip_len		= sended_len - blok*BLOK_LEN + (OurTxPacketIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int);
		OurTxPacketIpHeader->ip_len 	= SWAP16(ip_len);
                OurTxPacketIpHeader->ip_sum	= 0;
		checksum = SWAP16(IPChksum((unsigned short *)OurTxPacketIpHeader, (OurTxPacketIpHeader->ip_hl_v & 0x0F) * sizeof(unsigned int)/sizeof(unsigned short)));
		OurTxPacketIpHeader->ip_sum 	= checksum;
		// Transmit
		ready = ready & ProcessTxEmacPacket();
		TxtdList[TxBuffIndex].addr = (unsigned int)OurTxPacketEthHeader;
		TxtdList[TxBuffIndex].U_Status.S_Status.Length = SWAP16(OurTxPacketIpHeader->ip_len) + 14;
		TxtdList[TxBuffIndex].U_Status.S_Status.LastBuff = 1;
		if (TxBuffIndex == (NB_TX_BUFFERS - 1))	TxBuffIndex = 0; else TxBuffIndex ++;
		AT91C_BASE_EMAC->EMAC_NCR |= AT91C_EMAC_TSTART;
		blok++;
	}
	EmacTxCounter++;
	return ready;
}


unsigned int UDPNeedLength = 0xFFFFFFFF;
unsigned int UDPBuildLength = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int ProcessRxEmacPacket()	// Process packets...
{
	static u32 curIndex = 0;

	//unsigned int i;
	// Receive one packet		


	if (curIndex >= NB_RX_BUFFERS)
	{
		curIndex = 0;
	};

	//u32 process = curIndex;

	if(RxtdList[curIndex].addr & OWNERSHIP_BIT)
	{
		if ((AT91C_BASE_EMAC->EMAC_RSR & AT91C_EMAC_REC) == AT91C_EMAC_REC)
		{
			AT91C_BASE_EMAC->EMAC_RSR |= AT91C_EMAC_REC;							
		};
	}
	else
	{
		curIndex++;

		return NO_IPPACKET;
	};

	TestPin2_Set();

	int status = NO_IPPACKET;
	bool need_handle_data = false;
	unsigned int icmp_len;
	char *pData;
	AT91PS_EthHdr	pEth;
	AT91PS_ArpHdr	pArp;
	AT91PS_IPheader pIpHeader;
	AT91PS_IcmpEchoHdr pIcmpEcho;
	// Process this packet
	pData      = (char *)(RxtdList[curIndex].addr & 0xFFFFFFFC);
	pEth = (AT91PS_EthHdr)(pData + RCV_OFFSET);

	switch (SWAP16(pEth->et_protlen))
	{
		case PROT_ARP: // ARP Packet format

			pArp = (AT91PS_ArpHdr)(pData + 14 + RCV_OFFSET);
            
			if (SWAP16(pArp->ar_op) == ARP_REQUEST) 
			{       
				// ARP REPLY operation

				pArp->ar_op =  SWAP16(ARP_REPLY);				
				for (byte i = 0; i<6; i++) // Fill the dest address and src address
				{
					pEth->et_dest[i] = pEth->et_src[i];	// swap ethernet dest address and ethernet src address			
					pEth->et_src[i]  = OurEmacAddr[i];
					pArp->ar_tha[i]   = pArp->ar_sha[i];
					pArp->ar_sha[i] = OurEmacAddr[i];
				};

				for (byte i = 0; i<4; i++) // swap sender IP address and target IP address
				{				
					pArp->ar_tpa[i] = pArp->ar_spa[i];
					pArp->ar_spa[i] = OurIpAddr[i];
				};	
       			
				TransmitPacket((pData + RCV_OFFSET), 0x40);
			};

			break; 

		case PROT_IP:	// IP protocol frame

			pIpHeader = (AT91PS_IPheader)(pData + 14 + RCV_OFFSET);			
            
			if(!IPChksumVerify((unsigned short *)pIpHeader, (pIpHeader->ip_hl_v & 0x0F) * sizeof(unsigned int)/sizeof(unsigned short)))
			{
//				EMAC_HandleRxError();
				EmacRxError++;
				break;
			};

			pIcmpEcho = (AT91PS_IcmpEchoHdr)((char *)pIpHeader + 20);

			status = IPPACKET;

			switch(pIpHeader->ip_p)
			{
				case PROT_ICMP:		

					if(pIcmpEcho->type == ICMP_ECHO_REQUEST)
					{
						pIcmpEcho->type = ICMP_ECHO_REPLY;
						pIcmpEcho->code = 0;
						pIcmpEcho->cksum = 0;
   						icmp_len = (SWAP16(pIpHeader->ip_len) - 20);	// Checksum of the ICMP Message
						if (icmp_len % 2)
						{
							*((unsigned char *)pIcmpEcho + icmp_len) = 0;
							icmp_len ++;
						}
						icmp_len = icmp_len / sizeof(unsigned short);
						pIcmpEcho->cksum = SWAP16(IcmpChksum((unsigned short *) pIcmpEcho, icmp_len));

						for(byte i = 0; i<4; i++) // Swap IP Dest address and IP Source address
						{
							pIpHeader->ip_dst[i] = pIpHeader->ip_src[i];
							pIpHeader->ip_src[i] = OurIpAddr[i];
						}
                        
						for(byte i = 0; i<6; i++)	// Swap Eth Dest address and Eth Source address
						{
							pEth->et_dest[i] = pEth->et_src[i];
							pEth->et_src[i]  = OurEmacAddr[i];
						}
						TransmitPacket((pData + RCV_OFFSET), SWAP16(pIpHeader->ip_len) + 14 + RCV_OFFSET);
					};

					break; 

				case PROT_UDP:		
				{

					char *adrRxTxBuf;
					char *adrOurBuf;
					int len;
                    bool NewPacket = false;

					if(pIpHeader->ip_id != OurRxPacketIpHeader->ip_id) 
					{
						if (((SWAP16(pIpHeader->ip_off))&0x1FFF)==0)
						{
                			if(pIpHeader->udp_dst != SWAP16(OurUDPPort)) { break; }
						};

						NewPacket = true;		
        			};

					if(!NewPacket)
					{
						adrRxTxBuf = (char *)(pIpHeader->ip_src);
						adrOurBuf = (char *)(OurRxPacketIpHeader->ip_src);
						len = 8;
					        while(len)
						{
							if(*adrOurBuf != *adrRxTxBuf) NewPacket = true;
							adrRxTxBuf++;
							adrOurBuf++;
							len --;
						}
					}
					else //	if(NewPacket)
					{
						UDPBuildLength = 0;
						FirstFragment = false;
						// Copy EthHeader and IPHeader	
						adrRxTxBuf = (char *)(pEth);
						adrOurBuf = (char *)(OurRxPacketEthHeader);
						len = ((pIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int)) + 14;
						while(len)
						{
							*adrOurBuf = *adrRxTxBuf;
							adrRxTxBuf++;
							adrOurBuf++;
							len --;
						}
					}
					
					unsigned short offset;			
					offset = SWAP16(pIpHeader->ip_off);

					bool IPFragmentation = (offset & 0x4000) == 0;

//					bool IPFragmentLast;

					//if(((offset)&0x4000)) IPFragmentation = false;//IP Fragmentation  = no
					//	else IPFragmentation = true;

					//if(((offset)&0x2000)) IPFragmentLast = false; //IP Fragment last = no
					//	else  IPFragmentLast = true; //IP Fragment last = yes

					adrRxTxBuf = ((char *)(pIpHeader) + (pIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int));
					adrOurBuf = (char *)(OurRxPacketIpHeader) + ((pIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int));
					
					if(IPFragmentation) adrOurBuf += 8*(offset&0x1FFF);

					char *startAdr = (char *)(RxtdList[0].addr & 0xFFFFFFFC);
					char *endAdr = (char *)((RxtdList[NB_RX_BUFFERS-1].addr & 0xFFFFFFFC) + ETH_RX_BUFFER_SIZE);

					len =  (SWAP16(pIpHeader->ip_len)) - ((pIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int));
					UDPBuildLength+=len;
                    
					char *endOurAdr = (char *)(&OurRxPacket[MAX_OUR_RX_PACKET_SIZE-sizeof(unsigned int)]);
					
					while(len > 0)
					{
						if(adrOurBuf >= endOurAdr) break;	// Buf is full
						if(adrRxTxBuf >= endAdr) adrRxTxBuf = startAdr;
                                                
						*adrOurBuf = *adrRxTxBuf;
						
						adrRxTxBuf++;
						adrOurBuf++;
						len --;
					};

					if(adrOurBuf >= endOurAdr) 
					{ 
						UDPNeedLength = 0xFFFFFFFF;
						UDPBuildLength = 0;
						break;
					};

					if((offset&0x1FFF) == 0)
					{
						FirstFragment = true;
						UDPNeedLength = SWAP16(pIpHeader->udp_len);
					};

					if((!IPFragmentation)||(FirstFragment&&(UDPBuildLength == UDPNeedLength)))
					{
						UDPNeedLength = 0xFFFFFFFF;
						UDPBuildLength = 0;
						need_handle_data =true;
					};

					break; 

				}
				default:
				break;
			}
		break; // case PROT_IP
		default:
		break;
	};

	// The packet has been processed ==> release the corresponding buffers descriptors

	RxtdList[curIndex].addr &= ~(OWNERSHIP_BIT);

	if((RxtdList[curIndex].U_Status.status & (AT91C_SOF|AT91C_EOF)) == AT91C_SOF)	
	{
		u32 p = curIndex;

		do
		{
			if (p == (NB_RX_BUFFERS-1)) p = 0; else p++;

			RxtdList[p].addr &= ~(OWNERSHIP_BIT);
		}
		while((!(RxtdList[p].U_Status.status & AT91C_EOF)) && (p != curIndex));	

//		curIndex = p;

	};

	curIndex++;

	//if(need_handle_data) HandleRxPacket(); 

	TestPin2_Clr();

	return status;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ProcessTxEmacPacket()
{
	bool ok = false;
	bool no_timeout = true;
	unsigned int t = 0xFFFF;
	while(!(AT91C_BASE_EMAC->EMAC_TSR & AT91C_EMAC_COMP))
	{
		if(!(t--)) { no_timeout = false; break; }
	}
	unsigned int i;
	if( (AT91C_BASE_EMAC->EMAC_TSR & AT91C_EMAC_COMP) == AT91C_EMAC_COMP) AT91C_BASE_EMAC->EMAC_TSR |= AT91C_EMAC_COMP;
	if( (AT91C_BASE_EMAC->EMAC_TSR & AT91C_EMAC_UBR) == AT91C_EMAC_UBR ) AT91C_BASE_EMAC->EMAC_TSR |= AT91C_EMAC_UBR;		
	if( (AT91C_BASE_EMAC->EMAC_TSR & AT91C_EMAC_UND) == AT91C_EMAC_UND ) AT91C_BASE_EMAC->EMAC_TSR |= AT91C_EMAC_UND;		
	if( (AT91C_BASE_EMAC->EMAC_TSR & AT91C_EMAC_COL) == AT91C_EMAC_COL ) AT91C_BASE_EMAC->EMAC_TSR |= AT91C_EMAC_COL;		
	if( (AT91C_BASE_EMAC->EMAC_TSR & AT91C_EMAC_RLES) == AT91C_EMAC_RLES ) AT91C_BASE_EMAC->EMAC_TSR |= AT91C_EMAC_RLES;		
	if( (AT91C_BASE_EMAC->EMAC_TSR & AT91C_EMAC_BEX) == AT91C_EMAC_BEX ) AT91C_BASE_EMAC->EMAC_TSR |= AT91C_EMAC_BEX;		
	
       	for (i = 0; i < NB_TX_BUFFERS; i++)
       	{
		ok = true;
               	if(TxtdList[i].U_Status.S_Status.BuffUsed == 1)
               	{
			if(TxtdList[i].U_Status.S_Status.TransmitUnderrun == 1) 
			{ 
				TxtdList[i].U_Status.S_Status.TransmitUnderrun = 0;
				ok = false;
			}
			if(TxtdList[i].U_Status.S_Status.TransmitError == 1) 
			{ 
				TxtdList[i].U_Status.S_Status.TransmitError = 0;
				ok = false;
			}
			if(TxtdList[i].U_Status.S_Status.BufExhausted == 1) 
			{ 
				TxtdList[i].U_Status.S_Status.BufExhausted = 0;
				ok = false;
			}
                       	TxtdList[i].U_Status.S_Status.Length = 0;
               		TxtdList[i].U_Status.S_Status.BuffUsed = 0;
             	}
	}
	return (ok&no_timeout);
}
	

void EMAC_Idle()
{
	if(EmacIsConnected)
	{
        //while(ProcessRxEmacPacket()!=NO_IPPACKET);
        ProcessRxEmacPacket();
	}
	else
	{
		EmacIsConnected = EMAC_GetConnection();
	}
}	

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/***************************/
/*** EMAC Init functions ***/
/***************************/

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int AT91F_GetLinkSpeed()      //This function configure the link speed // \arg Pointer to AT91PS_EMAC service
{
	unsigned int bmsr, bmcr, lpa, mac_cfg;
	read_phy(AT91C_PHY_ADDR, MII_BMSR, &bmsr);		/* Link status is latched, so read twice to get current value */
	read_phy(AT91C_PHY_ADDR, MII_BMSR, &bmsr);
	if (!(bmsr & BMSR_LSTATUS)) return -1;			/* no link */
	read_phy(AT91C_PHY_ADDR, MII_BMCR, &bmcr);
	if (bmcr & BMCR_ANENABLE) 
	{				/* AutoNegotiation is enabled */
		if (!(bmsr & BMSR_ANEGCOMPLETE)) return -2;						/* auto-negotitation in progress */
		read_phy(AT91C_PHY_ADDR, MII_LPA, &lpa);
		if ((lpa & LPA_100FULL) || (lpa & LPA_100HALF))	speedEmac = SPEED_100;
		else speedEmac = SPEED_10;
		if ((lpa & LPA_100FULL) || (lpa & LPA_10FULL)) duplexEmac = DUPLEX_FULL;
		else duplexEmac = DUPLEX_HALF;
	} else 
	{
		speedEmac = (bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10;
		duplexEmac = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
	}
        mac_cfg = AT91C_BASE_EMAC->EMAC_NCFGR & ~(AT91C_EMAC_SPD | AT91C_EMAC_FD); /* Update the MAC */
	if (speedEmac == SPEED_100) 
	{
		if (duplexEmac == DUPLEX_FULL) AT91C_BASE_EMAC->EMAC_NCFGR = mac_cfg | AT91C_EMAC_SPD | AT91C_EMAC_FD; /* 100 Full Duplex */
		else AT91C_BASE_EMAC->EMAC_NCFGR = mac_cfg | AT91C_EMAC_SPD;	/* 100 Half Duplex */
	} else 
	{
		if (duplexEmac == DUPLEX_FULL) AT91C_BASE_EMAC->EMAC_NCFGR = mac_cfg | AT91C_EMAC_FD; /* 10 Full Duplex */
		else AT91C_BASE_EMAC->EMAC_NCFGR = mac_cfg;		/* 10 Half Duplex */
	}
	return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

unsigned int EMAC_GetSpeed() { return speedEmac; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

unsigned int EMAC_GetDuplex() { return duplexEmac; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int AT91F_Ether_Probe()	//Detect MAC and PHY and perform initialization
{
	unsigned int phyid1, phyid2;
	unsigned int status;
	AT91F_Enable_Mdi();	// Read the PHY ID registers
	read_phy(AT91C_PHY_ADDR, MII_PHYSID1, &phyid1);
	read_phy(AT91C_PHY_ADDR, MII_PHYSID2, &phyid2);
	unsigned int id = ((phyid1 << 16) | (phyid2 & 0xfff0));
	if ((id!= MII_DM9161_ID) &&(id!=MII_DM9161A_ID))
	{
		ErrorHalt(ERROR_EMAC_PHY_ID);
		AT91F_Disable_Mdi();
		return -1;
	}
	status = AT91F_GetLinkSpeed();
	AT91F_Disable_Mdi();
	return status;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int AT91F_EMACInit( //This function initialise the ethernet
		unsigned int pRxTdList,// \arg Pointer to AT91PS_EMAC service
		unsigned int pTxTdList)
{
	if (AT91F_Ether_Probe())
	{
		return -1;
	}
	// the sequence write EMAC_SA1L and write EMAC_SA1H must be respected
	AT91C_BASE_EMAC->EMAC_SA1L = ((int)OurEmacAddr[3] << 24) | ((int)OurEmacAddr[2] << 16) | ((int)OurEmacAddr[1] << 8) | OurEmacAddr[0];
	AT91C_BASE_EMAC->EMAC_SA1H = ((int)OurEmacAddr[5] << 8) | OurEmacAddr[4];
	AT91C_BASE_EMAC->EMAC_RBQP = pRxTdList;
	AT91C_BASE_EMAC->EMAC_TBQP = pTxTdList;
	//Clear receive status register
	AT91C_BASE_EMAC->EMAC_RSR  = (AT91C_EMAC_OVR | AT91C_EMAC_REC | AT91C_EMAC_BNA);
	AT91C_BASE_EMAC->EMAC_NCFGR  |= (AT91C_EMAC_CAF /*| AT91C_EMAC_NBC*/ );// | AT91C_EMAC_RBOF_OFFSET_2);
	AT91C_BASE_EMAC->EMAC_NCR  |= (AT91C_EMAC_TE | AT91C_EMAC_RE | AT91C_EMAC_WESTAT);
 	return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int AT91F_EmacEntry(void)	// Initialise Emac to receive packets
{
	unsigned int i;
	unsigned int val;
	for (i = 0; i < NB_RX_BUFFERS; ++i) 	// Initialise RxtdList descriptor
	{
		val = (unsigned int)(RxPacket + (i * ETH_RX_BUFFER_SIZE));
		RxtdList[i].addr = val & 0xFFFFFFF8;
		RxtdList[i].U_Status.status = 0;
	}	
	RxtdList[NB_RX_BUFFERS-1].addr |= 0x02;	// Set the WRAP bit at the end of the list descriptor
	for (i = 0; i < NB_TX_BUFFERS; ++i) 	// Initialise TxtdList descriptor
	{
		val = (unsigned int)(TxPacket + (i * ETH_TX_BUFFER_SIZE));
		TxtdList[i].addr = val & 0xFFFFFFF8;
		TxtdList[i].U_Status.status = 0;
	}	
	TxtdList[NB_TX_BUFFERS-1].U_Status.S_Status.Wrap = 1;	// Set the WRAP bit at the end of the list descriptor
	return(AT91F_EMACInit((unsigned int) RxtdList, (unsigned int) TxtdList));
}	

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool EMAC_GetConnection()	// Wait for PHY auto negotiation completed
{
	int status = 0;
	read_phy(AT91C_PHY_ADDR, MII_BMSR, (unsigned int *)&status);
	read_phy(AT91C_PHY_ADDR, MII_BMSR, (unsigned int *)&status);
  	
	if (!(status & BMSR_ANEGCOMPLETE)) return false;
	
	AT91F_Disable_Mdi(/*AT91C_BASE_EMAC*/);
    AT91C_BASE_EMAC->EMAC_USRIO= AT91C_EMAC_CLKEN;	//Enable EMAC in MII mode, enable clock ERXCK and ETXCK
    
	Read_All_PHY_REGS();

	if ( AT91F_EmacEntry())	
	{
		return false;	//EMAC conf
	}

	AT91F_Enable_Mdi(/*AT91C_BASE_EMAC*/);
	
	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool EMAC_GetLink()     // очень медленная хрень, порядка 100мкс, 
{
	int status = 0;
	read_phy(AT91C_PHY_ADDR, MII_BMSR, (unsigned int *)&status);
	read_phy(AT91C_PHY_ADDR, MII_BMSR, (unsigned int *)&status);
  	if (!(status & BMSR_LSTATUS))
	{
		EmacIsConnected = false;
		return false;
	}
	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void EMAC_Power_Off()
{
	AT91F_PIO_SetOutput( AT91C_BASE_PIOB,  1<<18 ); 			// power down
	AT91F_PIO_ClearOutput( AT91C_BASE_PIOA,  1<<18 ); 			// disable osc
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool Test_Pin2Pin_Short(AT91PS_PIO pPio, u32 testMask, u32 pinMask)
{
	pPio->PIO_PER = testMask;
	pPio->PIO_ODR = testMask; 
	pPio->PIO_PPUER = testMask;

	pPio->PIO_OER = pinMask;
	pPio->PIO_CODR = pinMask;

	delay(100); 

	u32 v1 = pPio->PIO_PDSR;

	pPio->PIO_SODR = pinMask;
	
	delay(100); 

	u32 v2 = pPio->PIO_PDSR;

	pPio->PIO_PER = testMask;
	pPio->PIO_ODR = testMask; 
	pPio->PIO_PPUER = testMask;

	return ((v1 ^ v2) & testMask & ~pinMask);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool EMAC_Init()
{
	EmacIsConnected = false;
	int control = 0;
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOA ) ;
	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOB ) ;
	AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_EMAC ) ;

	const u32 testMask = AT91C_PB5_ERX0 | AT91C_PB6_ERX1 | AT91C_PB13_ERX2 | AT91C_PB14_ERX3 | AT91C_PB7_ERXER | 1<<18 | AT91C_PB17_ERXCK | AT91C_PB15_ERXDV_ECRSDV 
						| AT91C_PB8_EMDC | AT91C_PB9_EMDIO | AT91C_PB2_ETX0 | AT91C_PB3_ETX1 | AT91C_PB10_ETX2 | AT91C_PB11_ETX3 
						| AT91C_PB0_ETXCK_EREFCK | AT91C_PB1_ETXEN | AT91C_PB12_ETXER;

	AT91C_BASE_PIOB->PIO_PPUER = testMask;
	AT91C_BASE_PIOB->PIO_PER = testMask;

	__dsb(15);

	delay(100);

	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB5_ERX0				) == 0)		ErrorHalt(ERROR_EMAC_ERX0_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB6_ERX1				) == 0)		ErrorHalt(ERROR_EMAC_ERX1_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB13_ERX2			) == 0) 	ErrorHalt(ERROR_EMAC_ERX2_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB14_ERX3			) == 0) 	ErrorHalt(ERROR_EMAC_ERX3_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB7_ERXER			) == 0) 	ErrorHalt(ERROR_EMAC_ERXER_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & (1<<18)					) == 0)		ErrorHalt(ERROR_EMAC_PWRDN_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB17_ERXCK			) == 0)		ErrorHalt(ERROR_EMAC_ERXCK_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB15_ERXDV_ECRSDV	) == 0)		ErrorHalt(ERROR_EMAC_ERXDV_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB8_EMDC				) == 0)		ErrorHalt(ERROR_EMAC_EMDC_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB9_EMDIO			) == 0)		ErrorHalt(ERROR_EMAC_EMDIO_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB2_ETX0				) == 0)		ErrorHalt(ERROR_EMAC_ETX0_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB3_ETX1				) == 0)		ErrorHalt(ERROR_EMAC_ETX1_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB10_ETX2			) == 0)		ErrorHalt(ERROR_EMAC_ETX2_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB11_ETX3			) == 0)		ErrorHalt(ERROR_EMAC_ETX3_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB0_ETXCK_EREFCK		) == 0)		ErrorHalt(ERROR_EMAC_ETXCK_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB1_ETXEN			) == 0)		ErrorHalt(ERROR_EMAC_ETXEN_LOW);
	if ((AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB12_ETXER			) == 0)		ErrorHalt(ERROR_EMAC_ETXER_LOW);

	AT91F_PIO_CfgOutput( AT91C_BASE_PIOB, testMask);
	AT91C_BASE_PIOB->PIO_CODR = testMask; 

	delay(100);

	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB5_ERX0			)	ErrorHalt(ERROR_EMAC_ERX0_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB6_ERX1			)	ErrorHalt(ERROR_EMAC_ERX1_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB13_ERX2			) 	ErrorHalt(ERROR_EMAC_ERX2_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB14_ERX3			) 	ErrorHalt(ERROR_EMAC_ERX3_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB7_ERXER			) 	ErrorHalt(ERROR_EMAC_ERXER_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB17_ERXCK		)	ErrorHalt(ERROR_EMAC_ERXCK_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB15_ERXDV_ECRSDV	)	ErrorHalt(ERROR_EMAC_ERXDV_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB8_EMDC			)	ErrorHalt(ERROR_EMAC_EMDC_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB9_EMDIO			)	ErrorHalt(ERROR_EMAC_EMDIO_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB2_ETX0			)	ErrorHalt(ERROR_EMAC_ETX0_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB3_ETX1			)	ErrorHalt(ERROR_EMAC_ETX1_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB10_ETX2			)	ErrorHalt(ERROR_EMAC_ETX2_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB11_ETX3			)	ErrorHalt(ERROR_EMAC_ETX3_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB0_ETXCK_EREFCK	)	ErrorHalt(ERROR_EMAC_ETXCK_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB1_ETXEN			)	ErrorHalt(ERROR_EMAC_ETXEN_HI	);
	if (AT91C_BASE_PIOB->PIO_PDSR & AT91C_PB12_ETXER		)	ErrorHalt(ERROR_EMAC_ETXER_HI	);


	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB5_ERX0			))	ErrorHalt(ERROR_EMAC_ERX0_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB6_ERX1			))	ErrorHalt(ERROR_EMAC_ERX1_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB13_ERX2			)) 	ErrorHalt(ERROR_EMAC_ERX2_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB14_ERX3			)) 	ErrorHalt(ERROR_EMAC_ERX3_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB7_ERXER			)) 	ErrorHalt(ERROR_EMAC_ERXER_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB17_ERXCK			))	ErrorHalt(ERROR_EMAC_ERXCK_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB15_ERXDV_ECRSDV	))	ErrorHalt(ERROR_EMAC_ERXDV_SHORT);
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB8_EMDC			))	ErrorHalt(ERROR_EMAC_EMDC_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB9_EMDIO			))	ErrorHalt(ERROR_EMAC_EMDIO_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB2_ETX0			))	ErrorHalt(ERROR_EMAC_ETX0_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB3_ETX1			))	ErrorHalt(ERROR_EMAC_ETX1_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB10_ETX2			))	ErrorHalt(ERROR_EMAC_ETX2_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB11_ETX3			))	ErrorHalt(ERROR_EMAC_ETX3_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB0_ETXCK_EREFCK	))	ErrorHalt(ERROR_EMAC_ETXCK_SHORT);
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB1_ETXEN			))	ErrorHalt(ERROR_EMAC_ETXEN_SHORT);		
	if (Test_Pin2Pin_Short(AT91C_BASE_PIOB, testMask, AT91C_PB12_ETXER			))	ErrorHalt(ERROR_EMAC_ETXER_SHORT);		


	AT91C_BASE_PIOB->PIO_PER = testMask;
	AT91C_BASE_PIOB->PIO_ODR = testMask; 
	AT91C_BASE_PIOB->PIO_PPUER = testMask;

	AT91C_BASE_PIOB->PIO_PPUDR = (1<<15);   //disable pull up on RXDV => PHY normal mode (not in test mode),PHY has internal pull down
    AT91C_BASE_PIOB->PIO_PPUDR = (1<<16);	//PHY has internal pull down : set MII mode

	AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, 1<<18 );
	AT91F_PIO_SetOutput( AT91C_BASE_PIOA,  1<<18 ); 			//enable OSC
	AT91F_PIO_CfgOutput( AT91C_BASE_PIOB, 1<<18 );
	AT91F_PIO_ClearOutput( AT91C_BASE_PIOB,  1<<18 ); 			//clear PB18 <=> PHY powerdown
  	AT91C_BASE_RSTC->RSTC_RMR = 0xA5000000 | AT91C_RSTC_ERSTL&(0x01<<8) | AT91C_RSTC_URSTEN;
 	AT91C_BASE_RSTC->RSTC_RCR = 0xA5000000 | AT91C_RSTC_EXTRST;		//After PHY power up, hardware reset
  	while(!(AT91C_BASE_RSTC->RSTC_RSR & AT91C_RSTC_NRSTL));			//Wait for hardware reset end
 	AT91F_PIO_CfgPeriph(	//EMAC IO init for EMAC-PHY com, Remove EF100 config
              AT91C_BASE_PIOB, // PIO controller base address
              ((unsigned int) AT91C_PB2_ETX0) |
              ((unsigned int) AT91C_PB12_ETXER) |
              ((unsigned int) AT91C_PB16_ECOL) |
              ((unsigned int) AT91C_PB11_ETX3) |
              ((unsigned int) AT91C_PB6_ERX1) |
              ((unsigned int) AT91C_PB15_ERXDV_ECRSDV) |
              ((unsigned int) AT91C_PB13_ERX2) |
              ((unsigned int) AT91C_PB3_ETX1) |
              ((unsigned int) AT91C_PB8_EMDC) |
              ((unsigned int) AT91C_PB5_ERX0) |
              ((unsigned int) AT91C_PB14_ERX3) |
              ((unsigned int) AT91C_PB4_ECRS) |
              ((unsigned int) AT91C_PB1_ETXEN) |
              ((unsigned int) AT91C_PB10_ETX2) |
              ((unsigned int) AT91C_PB0_ETXCK_EREFCK) |
              ((unsigned int) AT91C_PB9_EMDIO) |
              ((unsigned int) AT91C_PB7_ERXER) |
              ((unsigned int) AT91C_PB17_ERXCK),
              0);
	AT91F_Enable_Mdi(/*AT91C_BASE_EMAC*/);	//Enable com between EMAC PHY

	read_phy(AT91C_PHY_ADDR, MII_BMCR, (unsigned int *)&control); 	//PHY has internal pull down : disable MII isolate
	read_phy(AT91C_PHY_ADDR, MII_BMCR, (unsigned int *)&control);
	control&=~BMCR_ISOLATE;
	write_phy(AT91C_PHY_ADDR, MII_BMCR, control);
    EmacRxCounter = 0;
	EmacTxCounter = 0;
	EmacRxError = 0;
	EmacTxError = 0;
        return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
