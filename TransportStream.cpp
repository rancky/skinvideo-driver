//michael
//#include "TransportStream.h"
//#include "bitops.h"
#include "SnCam.h"

#define _CRC32_MPEG2
//#define _CRC32_STD


#ifdef _CRC32_MPEG2
unsigned long crc32_table[256] = 
{
	0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
	0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61,
	0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD, 0x4C11DB70, 0x48D0C6C7,
	0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
	0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3,
	0x709F7B7A, 0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
	0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58, 0xBAEA46EF,
	0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
	0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB,
	0xCEB42022, 0xCA753D95, 0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1,
	0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077, 0x30476DC0,
	0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
	0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4,
	0x0808D07D, 0x0CC9CDCA, 0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE,
	0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08,
	0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
	0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC,
	0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6,
	0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A, 0xE0B41DE7, 0xE4750050,
	0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
	0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34,
	0xDC3ABDED, 0xD8FBA05A, 0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637,
	0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB, 0x4F040D56, 0x4BC510E1,
	0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
	0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5,
	0x3F9B762C, 0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
	0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E, 0xF5EE4BB9,
	0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
	0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD,
	0xCDA1F604, 0xC960EBB3, 0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7,
	0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 0x9B3660C6, 0x9FF77D71,
	0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
	0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2,
	0x470CDD2B, 0x43CDC09C, 0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
	0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E,
	0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
	0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A,
	0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0,
	0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C, 0xE3A1CBC1, 0xE760D676,
	0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
	0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
	0x933EB0BB, 0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,
	0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};
#endif

#ifdef _CRC32_STD
unsigned long crc32_table[256] = 
{		// poly = 0x04C11DB7L 
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
        0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
        0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
        0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
        0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
        0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
        0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
        0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
        0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
        0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
        0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
        0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
        0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
        0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
        0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
        0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
        0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
        0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
        0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
        0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
        0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
        0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
        0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
        0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
        0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
        0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
        0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
        0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
        0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
        0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
        0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
        0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
        0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
        0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
        0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
        0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
        0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
        0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
        0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
        0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
        0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
        0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
        0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
        0x2d02ef8d
};
#endif
// Global Counter for each program (most 10 programs)
int g_VideoFrameIdx;
int g_PROGRAM_COUNT;
STRUCT_PROGRAM g_Counter[10];
long long g_llPCR_Accumulator;
long long g_llPTS_Accumulator;
long long g_llDTS_Accumulator;
char g_InputFileDirectory[200];
char g_FrameNamePrefix[200];
char g_OutputFileName[200];
int g_InputFrameNumber;
int g_PCRInterval;
int g_PTS_DTS_Flags;

//void Construct_TS(const BYTE *pVideoES, int NumBytes)
void Construct_TS(int FrameIdx, const BYTE *pVideoES, int NumBytes,
				  BYTE *OutputData_Buffer, int &OutputBufSize)
{
	g_VideoFrameIdx = FrameIdx;

	// PAT/PMT send once at begin
	//if(g_VideoFrameIdx == 0 )
	if(g_VideoFrameIdx == 0 || g_VideoFrameIdx == 895 || g_VideoFrameIdx == 1787)	// N1: PAT/PMT send at I Frame and before H264 AUD, SEI
	//if(g_VideoFrameIdx == 0 || g_VideoFrameIdx == 882 || g_VideoFrameIdx == 1754)	// FreeTalk: PAT/PMT send at I Frame and before H264 AUD, SEI
	//if(g_VideoFrameIdx == 0 || g_VideoFrameIdx == 1 || g_VideoFrameIdx == 3 || g_VideoFrameIdx == 5 )	// FreeTalk: PAT/PMT send at I Frame and before H264 AUD, SEI
	{
		// initial global variables
		//InitGlobalVar();

		// construct PAT_TS
		Construct_PAT_TO_TS(OutputData_Buffer);

		// construct PMT_TS
		Construct_PMT_TO_TS(OutputData_Buffer);

		OutputBufSize += TS_PACKET_LENGTH * 2;
	}	
	// Construct PES_TS
	//Construct_PES_TO_TS(pVideoES, NumBytes, Elementary_ID);
	Construct_PES_TO_TS(pVideoES, NumBytes, Elementary_ID, OutputData_Buffer, OutputBufSize);

}
void UpdateProgramTable(WORD PID)
{
	// Search PID in Global Counter
	int i=0;
	while(i<10)
	{
		if(g_Counter[i].PID == PID)		// Old program, continuity_counter++
		{
			g_Counter[i].continuity_counter = (g_Counter[i].continuity_counter+1)%16; // 0~15
			break;
		}
		else if(g_Counter[i].PID == 0xFFFF )	// empty column for new program
		{
			g_Counter[i].PID = PID;
			g_Counter[i].continuity_counter = 0;
			break;
		}
		else
			i++;
	}
}

void Set_TS_Header(STRUCT_TS *pts, WORD PID, int Adap_field_ctrl, int PayloadStart, bool PCR_Flag)
{
	pts->TS_Length = TS_PACKET_LENGTH ;				// fixed 188 bytes per TS
	//pts->TS_Output = new BYTE [pts->TS_Length];	// Houston 2011/04/15
	memset(pts->TS_Output, 0x0, pts->TS_Length);

	pts->TS_Data_Length = TS_PACKET_LENGTH - 4;		// Packet length - Header length(4 bytes) 
	//pts->bytes_left = 184;
	//pts->load = 0;

	// clear TS_header
	memset(pts->TS_Header,0x0,5);
	// set sync_byte = 0x47 at header[0](7:0)
	BitSet(pts->TS_Header[0], 0x47);
	
	// set payload_unit_start_indicator:1 at header[1](6:6)
	if(PayloadStart)
		BitSet(pts->TS_Header[1], 0x40);

	// set elementary_PID = 0x100 at header[1](4:0)+header[2](7:0)
	BitSet(pts->TS_Header[1], (PID & 0x1F00)>>8 );	// retrieve (12:8)
	BitSet(pts->TS_Header[2], (PID & 0x00FF)    );	// retrieve (7:0)
	
	// Update Global Program Table
	UpdateProgramTable(PID);
	// Calc. continuity_counter -> at header[3](3:0)
	BYTE Contin_Counter = Return_Contin_Counter(PID);
	BitSetVal(pts->TS_Header[3],0x0f, Contin_Counter);
	

	// set adaptation_field_ctrl
	if( Adap_field_ctrl == PES_ADAPTATION_FIELD )
	{
		pts->TS_Data_Length = MAX_REMAINBYTES;

		// '11' at header[3](5:4) in TS(PES)
		BitSet(pts->TS_Header[3], 0x30);

		// set adpa_field_len at header[4](7:0)
//#ifdef _TEST_MODE
		if(PayloadStart)
		{
			BitSet(pts->TS_Header[4], 0x09);	// 9 bytes : 1 byte ( flags ) + 6 bytes( PCR / Stuffing ) + 2 bytes (Stuffing)
			pts->TS_Data_Length -= 2;			// stuffing 2 bytes
		}
		else
			BitSet(pts->TS_Header[4], 0x07);	
//#else
//		BitSet(pts->TS_Header[4], 0x07);	// 7 bytes :  1 byte ( flags ) 
											//	 		+ 6 bytes( PCR / Stuffing )
//#endif
		
		int Adap_Field_len = pts->TS_Header[4];

		// Adaptation_Field_Length > 0 
		if( Adap_Field_len > 0 )
		{		
			// allocate space for Adaptation Field
			//pts->TS_Adaptation_Field = new BYTE [Adap_Field_len];		// Houston 2011/04/15 remove
			memset(pts->TS_Adaptation_Field,0x0,Adap_Field_len);

			if( PCR_Flag == 1 )	// Set PCR
			{
				// -- Set Adaptation field flags --
				// set PCR_flag:0x1 at Field[0](4:4)
				BitSet(pts->TS_Adaptation_Field[0],0x10);
				
				// declare: PCR_Base:33bits, PCR_Extesion:9 bits
				// PCR_Base[0:3](32:1) + PCR_Base[4](0:0)
				// PCR_Ext [0](8:1) + PCR_Ext[1](0:0)
				BYTE PCR_Base[5], PCR_Ext[2];
				memset(PCR_Base, 0x0, 5);
				memset(PCR_Ext , 0x0, 2);

				// Calc. PCR
				Calc_PCR(PCR_Base, PCR_Ext);

				// -- assign PCR Field to Field[1~6] --
				// assign PCR_Base[0~4] to Field[1~5]
				ByteToByte(PCR_Base[0],7,0,pts->TS_Adaptation_Field[1],7,0);
				ByteToByte(PCR_Base[1],7,0,pts->TS_Adaptation_Field[2],7,0);
				ByteToByte(PCR_Base[2],7,0,pts->TS_Adaptation_Field[3],7,0);
				ByteToByte(PCR_Base[3],7,0,pts->TS_Adaptation_Field[4],7,0);
				ByteToByte(PCR_Base[4],7,0,pts->TS_Adaptation_Field[5],7,0);

				// Reserve 6 bits as 1, at Field[5](6:1) -> (7E):0111 1110
				BYTE ReserveByte = 0x7E;
				ByteToByte(ReserveByte,6,1,pts->TS_Adaptation_Field[5],6,1);

				// assign PCR_Ext[0](7:7) 0x80 to Field[5](0:0) 0x01
				ByteToByte(PCR_Ext[0],7,7,pts->TS_Adaptation_Field[5],0,0);
				// assign PCR_Ext[0](6:0) 0x7F to Field[6](7:1) 0xFE
				ByteToByte(PCR_Ext[0],6,0,pts->TS_Adaptation_Field[6],7,1);
				// assign PCR_Ext[1](7:7) 0x80 to Field[6](0:0) 0x01
				ByteToByte(PCR_Ext[1],7,7,pts->TS_Adaptation_Field[6],0,0);	

			}// end of PCR_Flag == 1
			else	// PCR_Flag == 0 => NO PCR , Set all 0xFF @ Field[1~6] or Padded by video data
			{
				// Replace the PCR with Stuffing bytes 0xFF
				memset(&(pts->TS_Adaptation_Field[1]), 0xFF, 6);
			}
			// if PES first packet, stuffing 2 bytes in the end of Adaptation field , 
			// for HW 4N bytes restriction
//#ifdef _TEST_MODE
			if(PayloadStart)
			{
				BitSet(pts->TS_Adaptation_Field[7],0xFF);
				BitSet(pts->TS_Adaptation_Field[8],0xFF);
			}
//#endif

		}// end of Adap_Field_len > 0		
	}
	else // set adaptation_field_ctrl = 01 at header[3](5:4) in TS(PAT/PMT)
	{
		//'01' at header[3](5:4) in TS(PAT/PMT)
		BitSet(pts->TS_Header[3], 0x10);
		// set pointer_field: 0x0
		BitSet(pts->TS_Header[4], 0x00);
		// No Adaptation field
		//pts->TS_Adaptation_Field = NULL;	// Houston 2011/04/15 remove
	}
}

bool Construct_PES_TO_TS(const BYTE *pVideoES, int NumBytes, WORD Elementary_PID, 
						 BYTE *OutputData_Buffer, int & OutputBufSize)
{
	STRUCT_TS *pTS_PES = new (NonPagedPool) STRUCT_TS;
	STRUCT_PES *pPES = new (NonPagedPool) STRUCT_PES;
	int PES_NumBytes = 0;

	// construct payload data section : PES
	// Houston 2011/04/15 +++
	// Allocate space for constructing PES	
	pPES->PES_Packet_Data = new (NonPagedPool) BYTE [NumBytes];		
	pPES->PES_Output = new (NonPagedPool) BYTE [NumBytes + TS_PACKET_LENGTH];
	// Houston 2011/04/15 ---
	PES_NumBytes = Construct_PES(pPES, pVideoES, NumBytes);

	// Divide PES_output into 184-? bytes TS payloads, 
	// if adapt. 176/174 = 188 - header(4) - Adap_len(1) - Adap_field(7) / - Stuffing(2)
	// if No adapt. 184 = 188 - header(4)
	int TotalPadded_Bytes = 0;
	BYTE Payload[MAX_PAYLOAD_SIZE];
	memset(Payload, 0x0, MAX_PAYLOAD_SIZE);

	if(PES_NumBytes > MAX_REMAINBYTES)
	{
		// +++++++++++++++++++++++++ First TS packet of PES ++++++++++++++++++++++++++++++++++++++++++
		// Header{ PAYLOAD_START + Adapt. field + PCR  }
		// Set TS Header : elementary_PID = 0x100 , adap_f_ctrl = 11, A&P
		{
			Set_TS_Header(pTS_PES, Elementary_PID, PES_ADAPTATION_FIELD, PAYLOAD_START, PCR_SET);		
			// copy part of PES to Payload
			memcpy(Payload,&(pPES->PES_Output[0]),pTS_PES->TS_Data_Length);
			Comine_Packet_TO_TS(pTS_PES, Payload, PES_ADAPTATION_FIELD);
			
			// copy output data to OutputBuf
			memcpy(&(OutputData_Buffer[OutputBufSize]), pTS_PES->TS_Output, TS_PACKET_LENGTH);
			OutputBufSize += TS_PACKET_LENGTH;

			TotalPadded_Bytes += pTS_PES->TS_Data_Length;
		}
		int RemainBytes = PES_NumBytes - TotalPadded_Bytes;
		// ------------------------- First TS packet of PES ------------------------------------------

		// +++++++++++++++++++++++++ Middle TS packets of PES ++++++++++++++++++++++++++++++++++++++++
		while( RemainBytes > MAX_REMAINBYTES )
		{
			memset(Payload, 0x0, MAX_PAYLOAD_SIZE);

			// Because Stuffing Size <= 176 bytes, so
			// if 184 > RemainBytes > 176, Set an Adaptation Field for this packet. 
			if( RemainBytes < MAX_PAYLOAD_SIZE )	// if remain bytes = 183, need 2 TS packets 182 , 1	
			{
				Set_TS_Header(pTS_PES, Elementary_PID, PES_ADAPTATION_FIELD, NON_PAYLOAD_START, PCR_NO_SET);
				
				int Exception_Handle = LAST_ONE_EXCEPTION_PCR_STUFF2BYTES;

				switch(Exception_Handle)
				{
					case LAST_ONE_EXCEPTION_PCR_REMAIN :		// PCR: set Time or 0xff, never data
						{
							memcpy(Payload,&(pPES->PES_Output[TotalPadded_Bytes]),pTS_PES->TS_Data_Length);
							break;
						}
					case LAST_ONE_EXCEPTION_PCR_UNREMAIN :		// PCR: set 0xff..ff + data
						{
							int StuffingPCRNum = 6 - (RemainBytes - MAX_REMAINBYTES);
							int DataInPCRNum;
							if(StuffingPCRNum > 0)
								DataInPCRNum = 6 - StuffingPCRNum;
							else
							{
								StuffingPCRNum = 0;
								DataInPCRNum = 6;
							}
							// copy Video data to Adapt. field as Payload
							memcpy( &(pTS_PES->TS_Adaptation_Field[7-DataInPCRNum]), &(pPES->PES_Output[TotalPadded_Bytes]), DataInPCRNum );
							// modify adapt. field leng = 1 byte(flag) + StuffingPCRNum
							pTS_PES->TS_Header[4] = 1 + StuffingPCRNum;;

							memcpy(Payload,&(pPES->PES_Output[TotalPadded_Bytes+DataInPCRNum]),pTS_PES->TS_Data_Length);
							TotalPadded_Bytes += DataInPCRNum ;
							break;
						}
					case LAST_ONE_EXCEPTION_PCR_STUFF2BYTES :	// PCR: set 0xffff(2 bytes) + data
						{
							int StuffingPCRNum = 4 - (RemainBytes - MAX_REMAINBYTES);
							int DataInPCRNum;
							if(StuffingPCRNum > 0)
								DataInPCRNum = 4 - StuffingPCRNum;
							else
							{
								StuffingPCRNum = 0;
								DataInPCRNum = 4;
							}
							// copy Video data to Adapt. field as Payload
							memcpy( &(pTS_PES->TS_Adaptation_Field[7-DataInPCRNum]), &(pPES->PES_Output[TotalPadded_Bytes]), DataInPCRNum );
							// modify adapt. field leng = 1 byte(flag) + 2 byte(stuff) + StuffingPCRNum
							pTS_PES->TS_Header[4] = 1 + 2 + StuffingPCRNum;

							memcpy(Payload,&(pPES->PES_Output[TotalPadded_Bytes+DataInPCRNum]),pTS_PES->TS_Data_Length);
							TotalPadded_Bytes += DataInPCRNum ;
							break;
						}
				}

				// Add Payload followed by TS_Header
				Comine_Packet_TO_TS(pTS_PES, Payload, PES_ADAPTATION_FIELD);

			}
			else
			{
				Set_TS_Header(pTS_PES, Elementary_PID, PES_NO_ADAPTATION_FIELD, NON_PAYLOAD_START, PCR_NO_SET);
				memcpy(Payload,&(pPES->PES_Output[TotalPadded_Bytes]),pTS_PES->TS_Data_Length);
				// Add Payload followed by TS_Header
				Comine_Packet_TO_TS(pTS_PES, Payload, PES_NO_ADAPTATION_FIELD);
			}

			// copy output data to OutputBuf
			memcpy(&(OutputData_Buffer[OutputBufSize]), pTS_PES->TS_Output, TS_PACKET_LENGTH);
			OutputBufSize += TS_PACKET_LENGTH;

			TotalPadded_Bytes += pTS_PES->TS_Data_Length;
			RemainBytes = PES_NumBytes - TotalPadded_Bytes;
		}
		// ------------------------- Middle TS packets of PES ----------------------------------------
		
		// +++++++++++++++++++++++++ Last TS packet of PES +++++++++++++++++++++++++++++++++++++++++++
		
		BYTE StuffingBuf[MAX_REMAINBYTES];
		memset(StuffingBuf, 0xFF, MAX_REMAINBYTES);

		if( RemainBytes > 0 )	// Padding remain bytes
		{
			// Construct TS header for remain PES.part & Set PCR for last TS packect of PES
			Set_TS_Header(pTS_PES, Elementary_PID, PES_ADAPTATION_FIELD, NON_PAYLOAD_START, PCR_NO_SET);
			
			// stuffing 0xff before PES payload
			// Buf : [Stuffing...ff...ff|...Remain Bytes...]
			int StuffingBytes = MAX_REMAINBYTES - RemainBytes;
			
			// Update Adaptation Field Length with plus Stuffing length
			pTS_PES->TS_Header[4] += StuffingBytes;

			memcpy( &(StuffingBuf[StuffingBytes]),
					&(pPES->PES_Output[TotalPadded_Bytes]),
					RemainBytes);
			// Add Payload followed by TS_Header
			Comine_Packet_TO_TS(pTS_PES, StuffingBuf, PES_ADAPTATION_FIELD);

			// copy output data to OutputBuf
			memcpy(&(OutputData_Buffer[OutputBufSize]), pTS_PES->TS_Output, TS_PACKET_LENGTH);
			OutputBufSize += TS_PACKET_LENGTH;
		}
		// ------------------------- Last TS packet of PES -------------------------------------------
	}

	else	// PES_NumBytes < MAX_REMAINBYTES
	{
		Set_TS_Header(pTS_PES, Elementary_PID, PES_ADAPTATION_FIELD, PAYLOAD_START, PCR_SET);		
		
		// stuffing 0xff before PES payload
		// Buf : [Stuffing...ff...ff|...Remain Bytes...]
		BYTE StuffingBuf[MAX_REMAINBYTES];
		memset(StuffingBuf, 0xFF, MAX_REMAINBYTES);
		int StuffingBytes = MAX_REMAINBYTES - PES_NumBytes - 2;			

		// Update Adaptation Field Length with plus Stuffing length
		pTS_PES->TS_Header[4] += StuffingBytes;

		memcpy( &(StuffingBuf[StuffingBytes]),
				&(pPES->PES_Output[0]),//&(pPES->PES_Output[TotalPadded_Bytes]),
				PES_NumBytes);

		// copy part of PES to Payload
		Comine_Packet_TO_TS(pTS_PES, StuffingBuf, PES_ADAPTATION_FIELD);
		
		// copy output data to OutputBuf
		memcpy(&(OutputData_Buffer[OutputBufSize]), pTS_PES->TS_Output, TS_PACKET_LENGTH);
		OutputBufSize += TS_PACKET_LENGTH;

		TotalPadded_Bytes += PES_NumBytes;
	}

	// Free Memory
	FreeTS(pTS_PES);
	FreePES(pPES);
	
	return true;
}

int Construct_PES(STRUCT_PES *ppes, const BYTE *pVideoES, int NumBytes)
{
	int Combine_Length=0;
	memset(ppes->Packet_Start_Code_Prefix,0x0,3);
	BitNset( ppes->Packet_Start_Code_Prefix[2], B_BitPos(BYTE_SIZE-1)); // last bit = 1
	ppes->Stream_ID = 0xE0;
	ppes->PES_Packet_Length = 0x0;

	// check Stream_ID != XXXX
	if(ppes->Stream_ID == 0xE0)
	{
		ppes->PES_Header_Flags = 0x0;
		// Set '10' at first 2 bits
		BitNset(ppes->PES_Header_Flags, W_BitPos(0) );

#if SET_DATA_ALIGNMENT_INDICATOR

		// data alignment indicator :'1' =>  at 6th. bit
		BitNset(ppes->PES_Header_Flags, W_BitPos(5) );
#endif

		// Default Set PTS_DTS_flags = '10' -> set PES_Header_Flags[8]:1
		if(g_PTS_DTS_Flags == 10)
			BitNset(ppes->PES_Header_Flags, W_BitPos(8) );
		else	//PTS_DTS_flags = '11'
		{
			BitNset(ppes->PES_Header_Flags, W_BitPos(8) );
			BitNset(ppes->PES_Header_Flags, W_BitPos(9) );
		}

		// check PTS_DTS_flags to allocate space for PTS/DTS
		// if PTS_DTS_flags == '10' => header_data_length = 0x5
		// if PTS_DTS_flags == '11' => header_data_length = 0xA
		if( BitNtst(ppes->PES_Header_Flags, W_BitPos(8)) && 
			!BitNtst(ppes->PES_Header_Flags, W_BitPos(9))   )
		{	
			// new space for PTS
			ppes->PES_Header_Data_Length = 0x5;
			//ppes->PES_Header_Fields = new BYTE [ppes->PES_Header_Data_Length];	// Houston 2011/04/15 remove
			memset(ppes->PES_Header_Fields,0x0,ppes->PES_Header_Data_Length);

			// set '0010'
			BitNset(ppes->PES_Header_Fields[0],B_BitPos(2));
			
			// Calc. PTS
			BYTE PTS[5];		// Calc. PTS by equation (2-11)
			memset(PTS,0x0,5);
			
			Calc_PTS(PTS);

			BYTE temp =0x0;

			// Set Field[] <= PTS[32~30]
			ByteToByte(PTS[0],7,5,ppes->PES_Header_Fields[0],3,1);
			// set marker_bit : 1
			ppes->PES_Header_Fields[0] = ( ppes->PES_Header_Fields[0] | 0x1 );

			// Set Field[] <= PTS[29~15]
			temp = (GetByteVal(PTS[0],4,0)<<3) + GetByteVal(PTS[1],7,5);	// Fields[1] <- PTS[29~22]
			ByteToByte(temp,7,0,ppes->PES_Header_Fields[1],7,0);
			temp = (GetByteVal(PTS[1],4,0)<<2) + GetByteVal(PTS[2],7,6);  // Fields[2] <- PTS[21~15]
			ByteToByte(temp,6,0,ppes->PES_Header_Fields[2],7,1);
			// set marker_bit : 1
			ppes->PES_Header_Fields[2] = ( ppes->PES_Header_Fields[2] | 0x1 );

			// Set Field[] <= PTS[14~0]
			temp = (GetByteVal(PTS[2],5,0)<<2) + GetByteVal(PTS[3],7,6);	// Fields[3] <- PTS[14~7]
			ByteToByte(temp,7,0,ppes->PES_Header_Fields[3],7,0);
			temp = (GetByteVal(PTS[3],5,0)<<1) + GetByteVal(PTS[4],7,7);  // Fields[4] <- PTS[6~0]
			ByteToByte(temp,6,0,ppes->PES_Header_Fields[4],7,1);
			// set marker_bit : 1
			ppes->PES_Header_Fields[4] = ( ppes->PES_Header_Fields[4] | 0x1 );

		}
		else if( BitNtst(ppes->PES_Header_Flags, W_BitPos(8)) && 
			     BitNtst(ppes->PES_Header_Flags, W_BitPos(9))    )
		{
			// new space for PTS and DTS
			ppes->PES_Header_Data_Length = 0xA;
			//ppes->PES_Header_Fields = new BYTE [ppes->PES_Header_Data_Length];		// Houston 2011/04/15 remove
			memset(ppes->PES_Header_Fields,0x0,ppes->PES_Header_Data_Length);

			// set '0011'
			BitNset(ppes->PES_Header_Fields[0],B_BitPos(2));
			BitNset(ppes->PES_Header_Fields[0],B_BitPos(3));
			
			// Calc. PTS 33bits , DTS 33bits
			BYTE PTS[5],DTS[5];		// Calc. PTS by equation (2-11)
			memset(PTS,0x0,5);
			memset(DTS,0x0,5);

			Calc_PTS(PTS);
			Calc_DTS(DTS);
			
			BYTE temp = 0x0;

			// ---------------------- PTS begin ----------------------
			// Set Field[] <= PTS[32~30]
			ByteToByte(PTS[0],7,5,ppes->PES_Header_Fields[0],3,1);
			// set marker_bit : 1
			ppes->PES_Header_Fields[0] = ( ppes->PES_Header_Fields[0] | 0x1 );

			// Set Field[] <= PTS[29~15]
			temp = (GetByteVal(PTS[0],4,0)<<3) + GetByteVal(PTS[1],7,5);	// Fields[1] <- PTS[29~22]
			ByteToByte(temp,7,0,ppes->PES_Header_Fields[1],7,0);
			temp = (GetByteVal(PTS[1],4,0)<<2) + GetByteVal(PTS[2],7,6);	// Fields[2] <- PTS[21~15]
			ByteToByte(temp,6,0,ppes->PES_Header_Fields[2],7,1);
			// set marker_bit : 1
			ppes->PES_Header_Fields[2] = ( ppes->PES_Header_Fields[2] | 0x1 );

			// Set Field[] <= PTS[14~0]
			temp = (GetByteVal(PTS[2],5,0)<<2) + GetByteVal(PTS[3],7,6);	// Fields[3] <- PTS[14~7]
			ByteToByte(temp,7,0,ppes->PES_Header_Fields[3],7,0);
			temp = (GetByteVal(PTS[3],5,0)<<1) + GetByteVal(PTS[4],7,7);	// Fields[4] <- PTS[6~0]
			ByteToByte(temp,6,0,ppes->PES_Header_Fields[4],7,1);
			// set marker_bit : 1
			ppes->PES_Header_Fields[4] = ( ppes->PES_Header_Fields[4] | 0x1 );
			// ---------------------- PTS end ----------------------

			// set '0001' at Field[5]
// remove 0528 1500 bug fixed
//			BitNset(ppes->PES_Header_Fields[5],B_BitPos(2));
			BitNset(ppes->PES_Header_Fields[5],B_BitPos(3));

			// ---------------------- DTS begin --------------------
			// Set Field[] <= DTS[32~30]
			ByteToByte(DTS[0],7,5,ppes->PES_Header_Fields[5],3,1);
			// set marker_bit : 1
			ppes->PES_Header_Fields[5] = ( ppes->PES_Header_Fields[5] | 0x1 );

			// Set Field[] <= DTS[29~15]
			temp = (GetByteVal(DTS[0],4,0)<<3) + GetByteVal(DTS[1],7,5);	// Fields[6] <- DTS[29~22]
			ByteToByte(temp,7,0,ppes->PES_Header_Fields[6],7,0);
			temp = (GetByteVal(DTS[1],4,0)<<2) + GetByteVal(DTS[2],7,6);  // Fields[7] <- DTS[21~15]
			ByteToByte(temp,6,0,ppes->PES_Header_Fields[7],7,1);
			// set marker_bit : 1
			ppes->PES_Header_Fields[7] = ( ppes->PES_Header_Fields[7] | 0x1 );

			// Set Field[] <= PTS[14~0]
			temp = (GetByteVal(DTS[2],5,0)<<2) + GetByteVal(DTS[3],7,6);	// Fields[8] <- DTS[14~7]
			ByteToByte(temp,7,0,ppes->PES_Header_Fields[8],7,0);
			temp = (GetByteVal(DTS[3],5,0)<<1) + GetByteVal(DTS[4],7,7);  // Fields[9] <- DTS[6~0]
			ByteToByte(temp,6,0,ppes->PES_Header_Fields[9],7,1);
			// set marker_bit : 1
			ppes->PES_Header_Fields[9] = ( ppes->PES_Header_Fields[9] | 0x1 );
			// ---------------------- DTS end ----------------------
		}

		// video data length
		ppes->VideoESData_len = NumBytes ;		
		// Allocate Data space & copy data from VideoES
		//ppes->PES_Packet_Data = new BYTE [NumBytes];		// Houston 2011/04/15 remove
		memcpy(ppes->PES_Packet_Data, pVideoES, NumBytes);
		
		// Combine all fields into PES Packet
		Combine_Length = Combine_PES(ppes);

		// Dump PES packet data to File
		//Write_To_File("PES_dump.bin",ppes->PES_Output, Combine_Length);
	}
	return Combine_Length;
}


int Combine_PES(STRUCT_PES *ppes)
{
	// Total output size
	int Output_Size = (9 + ppes->VideoESData_len) + ppes->PES_Header_Data_Length;
	//ppes->PES_Output = new BYTE [Output_Size];	// Houston 2011/04/15 remove
	memset(ppes->PES_Output,0x0,Output_Size);

	// combine all to PES_Output
	ppes->PES_Output[0] = ppes->Packet_Start_Code_Prefix[0];
	ppes->PES_Output[1] = ppes->Packet_Start_Code_Prefix[1];
	ppes->PES_Output[2] = ppes->Packet_Start_Code_Prefix[2];
	ppes->PES_Output[3] = ppes->Stream_ID;
	ppes->PES_Output[4] = (ppes->PES_Packet_Length & 0xff00) >> 8;
	ppes->PES_Output[5] =  ppes->PES_Packet_Length & 0x00ff;
	ppes->PES_Output[6] = (ppes->PES_Header_Flags  & 0xff00) >> 8;
	ppes->PES_Output[7] =  ppes->PES_Header_Flags  & 0x00ff;
	ppes->PES_Output[8] = ppes->PES_Header_Data_Length;

	// PES Header Fields
	int i;
	for(i=0; i<ppes->PES_Header_Data_Length; i++)
		ppes->PES_Output[9+i] = ppes->PES_Header_Fields[i];
	// PES Packet data
	for(i=0; i<ppes->VideoESData_len; i++)
		ppes->PES_Output[9+ppes->PES_Header_Data_Length+i] = ppes->PES_Packet_Data[i];

	return Output_Size;
}

void Construct_PAT_TO_TS(BYTE *OutputBuffer)
{
	STRUCT_TS *pTS_PAT = new (NonPagedPool) STRUCT_TS;
	STRUCT_PAT *pPAT = new (NonPagedPool) STRUCT_PAT;
	WORD PAT_PID = 0x0;
	
	// Set TS Header	
	Set_TS_Header(pTS_PAT, PAT_PID, PAT_PMT, PAYLOAD_START, PCR_NO_SET);
	//DBG_Print(pTS_PAT);

	// Set program association section
	Construct_PAT(pPAT);

	// Add PAT_Output followed by TS_Header
	Comine_Packet_TO_TS(pTS_PAT, pPAT->PAT_Output, PAT_PMT);

	// copy PAT_Output data to OutputBuffer
	memcpy(OutputBuffer, pTS_PAT->TS_Output, TS_PACKET_LENGTH);

	// Free Memory
	FreeTS(pTS_PAT);
	FreePAT(pPAT);
}

void Construct_PAT(STRUCT_PAT *ppat)
{
	ppat->TABLE_ID = 0x00;			// 0x00 for prog. ass. section
	
	// ++++++++++ Setction ++++++++++
	memset(ppat->Section,0x0,2);
	// set section_syntax_indicator:0x1 at Section[0](7:7)
	BitSet(ppat->Section[0],0x80);

	// set '0' at Section[0](6:6)
	BitClr(ppat->Section[0],0x40);

	// reserved 2 bits : 11 at Section[0](5:4)
	BitSet(ppat->Section[0],0x30);

	// Section Length = TS_ID(2) + VerN_CurNextIdx(1) + Ver #(1)+ Last_Ver#(1) + PATable(?) + CRC32(4)
	// section length(12 bits) 0x00D : at Section[0](3:0)+[1](7:0)
	WORD Section_Length = 0x000D;
	BitSet(ppat->Section[0],((Section_Length&0x0f00)>>8) );
	BitSet(ppat->Section[1],(Section_Length&0x00ff));
	// ---------- Setction ----------
	
	// TS_ID(16 bits) : 0x00
	memset(ppat->TS_ID,0x0,2);

	// ++++++++++ Version # and Current Next Indicator ++++++++++
	ppat->VerN_CurNextIdx = 0x0;
	// reserved(2 bits) : '11' at VerN_CurNextIdx(7:6)
	BitSet(ppat->VerN_CurNextIdx, 0xC0);

	// version number(5 bits): 0x0 at VerN_CurNextIdx(5:1)
	BYTE Version_Number = 0x0;
	BitSet(ppat->VerN_CurNextIdx, (Version_Number&0x1F) );

	// current next indicator(1 bit): 0x1 at VerN_CurNextIdx(0:0)
	BitSet(ppat->VerN_CurNextIdx, 0x1);
	// ---------- Version # and Current Next Indicator ----------

	// Section Number(8 bits): 0x0
	ppat->Section_Number=0x0;

	// Last Section Number(8 bits): 0x0
	ppat->Last_Section_Number=0x0;
	
	// Allocate space for PATable
	int PAT_Size = Section_Length - 9;		// 13-9=4
	//ppat->PATable= new BYTE [PAT_Size];	// Houston 2011/04/15 remove
	memset(ppat->PATable, 0x0, PAT_Size);
	// ++++++++++ PATable ++++++++++
	int Program_Count = 1;
	int i;
	for(i=0; i<Program_Count; i++)
	{
		// Program_Number(16 bits):0x01 at PAT[0] + PAT[1]
		WORD Program_Number = 0x01;
		BitSet(ppat->PATable[i+0],(Program_Number&0xFF00)>>8);
		BitSet(ppat->PATable[i+1],Program_Number);

		// reserved 3 bits: '111' at PAT[2](7:5)
		BitSet(ppat->PATable[i+2],0xE0);

		// Program_Map_PID(13 bits):0x30 at PAT[2](4:0)+PAT[3](7:0)
		WORD Program_Map_PID = PROGRAM_MAP_PID;
		BitSet( ppat->PATable[i+2], (Program_Map_PID&0x1F00)>>8);
		BitSet( ppat->PATable[i+3], (Program_Map_PID&0xFF));
	}
	// ---------- PATable ----------

	// ++++++++++ CRC32 ++++++++++
	memset(ppat->CRC32, 0xFF, 4);
	
	// Houston 2011/04/15 +++
	//unsigned char *CRCInput_PAT = GenerateCRCInput_PAT(ppat,Program_Count);
	BYTE *CRCInput_PAT = new (NonPagedPool) BYTE [8+Program_Count*4];
	GenerateCRCInput_PAT(ppat,Program_Count, CRCInput_PAT);
	// Houston 2011/04/15 ---
	
	unsigned long CRC32Val = crc32(CRCInput_PAT, (8+Program_Count*4) );
	ppat->CRC32[0] = (CRC32Val & 0xFF000000) >> 24;
	ppat->CRC32[1] = (CRC32Val & 0x00FF0000) >> 16;
	ppat->CRC32[2] = (CRC32Val & 0x0000FF00) >> 8;
	ppat->CRC32[3] = (CRC32Val & 0x000000FF);
	// ---------- CRC32 ----------

	// ++++++++++ Stuffing Data ++++++++++
	int Stuffing_length = 188 - 21 ; 
	//ppat->Stuffing_Data = new BYTE [Stuffing_length];		// Houston 2011/04/15
	memset(ppat->Stuffing_Data, 0xff, Stuffing_length);
	// ---------- Stuffing Data ----------

	// save all PAT to Output
	Combine_PAT(ppat,Program_Count,Stuffing_length);

	//DBG_Print(ppat);
	if(CRCInput_PAT != NULL)
	{
		delete [] CRCInput_PAT;
		CRCInput_PAT = NULL;
	}
}

// Houston 2011/04/15
void GenerateCRCInput_PAT(STRUCT_PAT *ppat, int Program_Count, BYTE *reCRCInput)
//unsigned char *GenerateCRCInput_PAT(STRUCT_PAT *ppat, int Program_Count)
{
	//unsigned char *reCRCInput = new unsigned char [8+Program_Count*4];	// Houston 2011/04/15
	memset(reCRCInput,0x0,8+Program_Count*4);

	reCRCInput[0] = ppat->TABLE_ID;
	reCRCInput[1] = ppat->Section[0]; 
	reCRCInput[2] = ppat->Section[1];
	reCRCInput[3] = ppat->TS_ID[0];
	reCRCInput[4] = ppat->TS_ID[1];
	reCRCInput[5] = ppat->VerN_CurNextIdx;
	reCRCInput[6] = ppat->Section_Number;
	reCRCInput[7] = ppat->Last_Section_Number;
	for(int i=0; i<Program_Count; i++)
	{
		reCRCInput[8+i] = ppat->PATable[0+i];
		reCRCInput[9+i] = ppat->PATable[1+i];
		reCRCInput[10+i] = ppat->PATable[2+i];
		reCRCInput[11+i] = ppat->PATable[3+i];
	}
	//return reCRCInput;		// Houston 2011/04/15
}
// Houston 2011/04/15
void GenerateCRCInput_PMT(STRUCT_PMT *ppmt, int Program_Elementary_Count, BYTE *reCRCInput)
//unsigned char *GenerateCRCInput_PMT(STRUCT_PMT *ppmt, int Program_Elementary_Count)
{
	//unsigned char *reCRCInput = new unsigned char [12+Program_Elementary_Count*5];	// Houston 2011/04/15
	memset(reCRCInput,0x0,12+Program_Elementary_Count*5);

	reCRCInput[0] = ppmt->TABLE_ID;
	reCRCInput[1] = ppmt->Section[0]; 
	reCRCInput[2] = ppmt->Section[1];
	reCRCInput[3] = ppmt->Porgram_Number[0];
	reCRCInput[4] = ppmt->Porgram_Number[1];
	reCRCInput[5] = ppmt->VerN_CurNextIdx;
	reCRCInput[6] = ppmt->Section_Number;
	reCRCInput[7] = ppmt->Last_Section_Number;
	reCRCInput[8] = ppmt->PCR_PID[0];
	reCRCInput[9] = ppmt->PCR_PID[1];
	reCRCInput[10] = ppmt->Prog_Info_Length[0];
	reCRCInput[11] = ppmt->Prog_Info_Length[1];
	for(int i=0; i<Program_Elementary_Count; i++)
	{
		reCRCInput[12+i] = ppmt->PMTable[0+i];
		reCRCInput[13+i] = ppmt->PMTable[1+i];
		reCRCInput[14+i] = ppmt->PMTable[2+i];
		reCRCInput[15+i] = ppmt->PMTable[3+i];
		reCRCInput[16+i] = ppmt->PMTable[4+i];
	}
	//return reCRCInput;	// Houston 2011/04/15
}

void Combine_PAT(STRUCT_PAT *ppat,int Program_Count,int Stuffing_length)
{
	ppat->Output_length = 188 - 5;			// 5 bytes : TS header length
	//ppat->PAT_Output = new BYTE [ppat->Output_length];	// Houston 2011/04/15
	memset(ppat->PAT_Output,0x0, ppat->Output_length);
	
	ppat->PAT_Output[0] = ppat->TABLE_ID;
	ppat->PAT_Output[1] = ppat->Section[0]; 
	ppat->PAT_Output[2] = ppat->Section[1];
	ppat->PAT_Output[3] = ppat->TS_ID[0];
	ppat->PAT_Output[4] = ppat->TS_ID[1];
	ppat->PAT_Output[5] = ppat->VerN_CurNextIdx;
	ppat->PAT_Output[6] = ppat->Section_Number;
	ppat->PAT_Output[7] = ppat->Last_Section_Number;
	// PATable 4 bytes for each program: 1 prog.
	ppat->PAT_Output[8] = ppat->PATable[0];
	ppat->PAT_Output[9] = ppat->PATable[1];
	ppat->PAT_Output[10] = ppat->PATable[2];
	ppat->PAT_Output[11] = ppat->PATable[3];
	// CRC32
	ppat->PAT_Output[12] = ppat->CRC32[0];
	ppat->PAT_Output[13] = ppat->CRC32[1];
	ppat->PAT_Output[14] = ppat->CRC32[2];
	ppat->PAT_Output[15] = ppat->CRC32[3];
	// Stuffing
	for(int i = 0; i<Stuffing_length; i++)
		ppat->PAT_Output[16+i] = ppat->Stuffing_Data[i];
}

void Comine_Packet_TO_TS(STRUCT_TS *pts, BYTE *output, int Mode)
{
	if(Mode == PES_ADAPTATION_FIELD)	// 11 : has adaptation field for PES
	{
		// copy TS Header + adapt_field length (4 bytes + 1 byte) to output 
		memcpy(pts->TS_Output, pts->TS_Header, 5);

		int Adapt_Field_Length = 7;	// flags(1 byte) + PCR(6 bytes)

//#ifdef _TEST_MODE
//		int Adapt_Field_Length = 7;	// flags(1 byte) + PCR(6 bytes) + Stuffing(2 bytes)
		bool PayloadStart = pts->TS_Header[1] & 0x40 ;
		if(PayloadStart)
			Adapt_Field_Length = 0x9;
//#else
//		int Adapt_Field_Length = 7;	// flags(1 byte) + PCR(6 bytes)
//#endif
		// copy adap. field to output
		memcpy(&(pts->TS_Output[5]), pts->TS_Adaptation_Field, Adapt_Field_Length);
		// copy PES to output
		memcpy(&(pts->TS_Output[5+Adapt_Field_Length]), output, (TS_PACKET_LENGTH-5-Adapt_Field_Length));
	}	
	// No Adaptation Feild  
	else if(Mode == PES_NO_ADAPTATION_FIELD)
	{
		// copy TS Header (4 bytes) to output
		memcpy(pts->TS_Output, pts->TS_Header, 4);
		// copy parts of reamined PES to output
		memcpy( &(pts->TS_Output[4]) , output, TS_PACKET_LENGTH-4);
	}
	else if(Mode == PAT_PMT)
	{
		// copy TS Header + pointer field(4 bytes + 1 byte) to output
		memcpy(pts->TS_Output, pts->TS_Header, 5);
		// copy PAT/PMT_Output to output
		memcpy( &(pts->TS_Output[5]) , output, TS_PACKET_LENGTH-5);
	}

	Write_To_File(pts->TS_Output, TS_PACKET_LENGTH);
}

void Construct_PMT_TO_TS(BYTE *OutputBuffer)
{
	STRUCT_TS *pTS_PMT = new (NonPagedPool) STRUCT_TS;
	STRUCT_PMT *pPMT = new (NonPagedPool) STRUCT_PMT;
	WORD ProgMap_PID = PROGRAM_MAP_PID;
	
	// Set TS Header	
	Set_TS_Header(pTS_PMT, ProgMap_PID, PAT_PMT, PAYLOAD_START, PCR_NO_SET);
	//DBG_Print(pTS_PMT);

	// Set program map section
	Construct_PMT(pPMT);

	// Add PMT_Output followed by TS_Header
	Comine_Packet_TO_TS(pTS_PMT, pPMT->PMT_Output, PAT_PMT);

	// copy PMT_Output data to OutputBuffer, after PAT Packet
	memcpy(&(OutputBuffer[TS_PACKET_LENGTH]), pTS_PMT->TS_Output, TS_PACKET_LENGTH);

	// Free Memory
	FreeTS(pTS_PMT);
	FreePMT(pPMT);
}

void Construct_PMT(STRUCT_PMT *ppmt)
{
	ppmt->TABLE_ID = 0x02;
	
	// ++++++++++ Setction ++++++++++
	memset(ppmt->Section,0x0,2);
	// set section_syntax_indicator:0x1 at Section[0](7:7)
	BitSet(ppmt->Section[0],0x80);

	// set '0' at Section[0](6:6)
	BitClr(ppmt->Section[0],0x40);

	// reserved 2 bits : 11 at Section[0](5:4)
	BitSet(ppmt->Section[0],0x30);

	// Section Length = Program_Number(2) + VerN_CurNextIdx(1) + Sec #(1) 
	// + Last_Ver#(1) + PCR_PID(2)+ Prog. info len(2) + PMTable(?) + CRC32(4)
	// section length(12 bits) 0x012 : at Section[0](3:0)+[1](7:0)
	WORD Section_Length = 0x0012;
	BitSet(ppmt->Section[0],((Section_Length&0x0f00)>>8) );
	BitSet(ppmt->Section[1],(Section_Length&0x00ff));
	// ---------- Setction ----------
	
	// Program_Number(16 bits) : 0x0001
	memset(ppmt->Porgram_Number,0x0,2);
	BitSet(ppmt->Porgram_Number[1],0x01);

	// ++++++++++ Version # and Current Next Indicator ++++++++++
	ppmt->VerN_CurNextIdx = 0x0;
	// reserved(2 bits) : '11' at VerN_CurNextIdx(7:6)
	BitSet(ppmt->VerN_CurNextIdx, 0xC0);

	// version number(5 bits): 0x0 at VerN_CurNextIdx(5:1)
	BYTE Version_Number = 0x0;
	BitSet(ppmt->VerN_CurNextIdx, (Version_Number&0x1F) );

	// current next indicator(1 bit): 0x1 at VerN_CurNextIdx(0:0)
	BitSet(ppmt->VerN_CurNextIdx, 0x1);
	// ---------- Version # and Current Next Indicator ----------

	// Section Number(8 bits): 0x0
	ppmt->Section_Number=0x0;

	// Last Section Number(8 bits): 0x0
	ppmt->Last_Section_Number=0x0;
	
	// reserved 3 bits: '111' at PCR_PID[0](7:5)
	memset(ppmt->PCR_PID, 0x0, 2);
	BitSet(ppmt->PCR_PID[0], 0xE0);

	// PCR_PID(13 bits): 0x100 at PCR_PID[0](4:0) + PCR_PID[1](7:0)
	WORD PCR_PID = Elementary_ID;
	BitSet(ppmt->PCR_PID[0],((PCR_PID&0x1F00)>>8) );
	BitSet(ppmt->PCR_PID[1],(PCR_PID&0x00FF));

	// reserved 4 bits: '1111' at Prog_Info_Length[0](7:4)
	memset(ppmt->Prog_Info_Length, 0x0, 2);
	BitSet(ppmt->Prog_Info_Length[0], 0xF0);

	// Program Info Length(12 bits):0x000 at Prog_Info_Length[0](3:0)+[1](7:0)
	WORD Program_Info_Length = 0x000;
	BitSet(ppmt->Prog_Info_Length[0],((Program_Info_Length&0x0F00)>>8) );
	BitSet(ppmt->Prog_Info_Length[1],(Program_Info_Length&0x00FF));

	// Allocate space for PMTable
	int PMT_Size = Section_Length - 13;		// 18-13=5
	//ppmt->PMTable= new BYTE [PMT_Size];	// Houston 2011/04/15
	memset(ppmt->PMTable, 0x0, PMT_Size);
	// ++++++++++ PATable (5 bytes/prog.) ++++++++++
	int Program_Element_Count = 1;
	int i;
	for(i=0; i<Program_Element_Count; i++)
	{
		// stream type(8 bits) 0x1b at PMT[0]
		BitSet(ppmt->PMTable[i+0], 0x1b);			// 0x1b : H.264

		// reserved 3 bits: '111' at PMT[1](7:5)
		BitSet(ppmt->PMTable[i+1],0xE0);

		// Elementary_PID(13 bits):0x100 at PMT[1](4:0)+PMT[2](7:0)
		WORD Elementary_PID = Elementary_ID;
		BitSet( ppmt->PMTable[i+1], (Elementary_PID&0x1F00)>>8);
		BitSet( ppmt->PMTable[i+2], (Elementary_PID&0xFF));

		// reserved 4 bits: '1111' at PMT[3](7:4)
		BitSet(ppmt->PMTable[i+3],0xF0);

		// ES_info_length(12 bits):0x000 at PMT[3](3:0)+PAT[4](7:0)
		WORD ES_info_length = 0x000;
		BitSet( ppmt->PMTable[i+3], (ES_info_length&0x0F00)>>8);
		BitSet( ppmt->PMTable[i+4], (ES_info_length&0xFF));
	}
	// ---------- PATable ----------

	// ++++++++++ CRC32 ++++++++++
	memset(ppmt->CRC32, 0x00, 4);

	// Houston 2011/04/15 +++
	//unsigned char *CRCInput_PMT = GenerateCRCInput_PMT(ppmt,Program_Element_Count);
	BYTE *CRCInput_PMT = new (NonPagedPool) BYTE[12+Program_Element_Count*5];
	GenerateCRCInput_PMT(ppmt,Program_Element_Count, CRCInput_PMT);
	// Houston 2011/04/15 ---
	
	unsigned long CRC32Val = crc32(CRCInput_PMT, (12+Program_Element_Count*5) );
	ppmt->CRC32[0] = (CRC32Val & 0xFF000000) >> 24;
	ppmt->CRC32[1] = (CRC32Val & 0x00FF0000) >> 16;
	ppmt->CRC32[2] = (CRC32Val & 0x0000FF00) >> 8;
	ppmt->CRC32[3] = (CRC32Val & 0x000000FF);
	// ---------- CRC32 ----------

	// ++++++++++ Stuffing Data ++++++++++
	int Stuffing_length = 188 - 26 ;  
	//ppmt->Stuffing_Data = new BYTE [Stuffing_length];		// Houston 2011/04/15 
	memset(ppmt->Stuffing_Data, 0xff, Stuffing_length);
	// ---------- Stuffing Data ----------

	// save all PMT to Output
	Combine_PMT(ppmt,Program_Element_Count,Stuffing_length);

	//DBG_Print(ppmt);
	if(CRCInput_PMT != NULL)
	{
		delete [] CRCInput_PMT;
		CRCInput_PMT = NULL;
	}
}

void Combine_PMT(STRUCT_PMT *ppmt,int Program_Element_Count,int Stuffing_length)
{
	ppmt->Output_length = 188 - 5;			// 5 bytes : TS header length
	//ppmt->PMT_Output = new BYTE [ppmt->Output_length];	// Houston 2011/04/15
	memset(ppmt->PMT_Output,0x0, ppmt->Output_length);
	
	ppmt->PMT_Output[0] = ppmt->TABLE_ID;
	ppmt->PMT_Output[1] = ppmt->Section[0]; 
	ppmt->PMT_Output[2] = ppmt->Section[1];
	ppmt->PMT_Output[3] = ppmt->Porgram_Number[0];
	ppmt->PMT_Output[4] = ppmt->Porgram_Number[1];
	ppmt->PMT_Output[5] = ppmt->VerN_CurNextIdx;
	ppmt->PMT_Output[6] = ppmt->Section_Number;
	ppmt->PMT_Output[7] = ppmt->Last_Section_Number;
	ppmt->PMT_Output[8] = ppmt->PCR_PID[0];
	ppmt->PMT_Output[9] = ppmt->PCR_PID[1];
	ppmt->PMT_Output[10] = ppmt->Prog_Info_Length[0];
	ppmt->PMT_Output[11] = ppmt->Prog_Info_Length[1];
	// PMTable 5 bytes for each program: 1 prog.
	ppmt->PMT_Output[12] = ppmt->PMTable[0];
	ppmt->PMT_Output[13] = ppmt->PMTable[1];
	ppmt->PMT_Output[14] = ppmt->PMTable[2];
	ppmt->PMT_Output[15] = ppmt->PMTable[3];
	ppmt->PMT_Output[16] = ppmt->PMTable[4];
	// CRC32
	ppmt->PMT_Output[17] = ppmt->CRC32[0];
	ppmt->PMT_Output[18] = ppmt->CRC32[1];
	ppmt->PMT_Output[19] = ppmt->CRC32[2];
	ppmt->PMT_Output[20] = ppmt->CRC32[3];
	// Stuffing
	for(int i = 0; i<Stuffing_length; i++)
		ppmt->PMT_Output[21+i] = ppmt->Stuffing_Data[i];
}

bool ReadParam(const char *parafilename)
{
/* michael
	char PARASection[256];
	memset(PARASection,0x00,sizeof(char)*256);
	strcpy(PARASection,"Parameter");
	char para[50]={0};
	char buf[40]={0};
	DWORD cch;

	sprintf(para,"InputFileDirectory");
	cch = GetPrivateProfileStringA(PARASection,para,NULL,g_InputFileDirectory,200,parafilename);
	if(cch==NULL){printf("Can't Find Param File!\n"); return false;}

	sprintf(para,"FrameNamePrefix");
	cch = GetPrivateProfileStringA(PARASection,para,"ff_", g_FrameNamePrefix,200,parafilename);
	sprintf(para,"OutputFileName");
	cch = GetPrivateProfileStringA(PARASection,para,"Default.ts", g_OutputFileName,200,parafilename);

#ifdef _TEST_MODE
	char tempFileName[200]; memset(tempFileName,'\0',200);
	memcpy(tempFileName, g_OutputFileName, 200);
	sprintf(&(tempFileName[strlen(tempFileName)-3]),"_TestMode.ts");
	memcpy(g_OutputFileName,tempFileName,200);
#endif

	sprintf(para,"FrameNumber");
	cch = GetPrivateProfileStringA(PARASection,para,"30", buf,40,parafilename);
	g_InputFrameNumber = atoi(buf);
	sprintf(para,"PCRInterval");
	cch = GetPrivateProfileStringA(PARASection,para,"5", buf,40,parafilename);
	g_PCRInterval= atoi(buf);
	sprintf(para,"PTS_DTS_Flag");
	cch = GetPrivateProfileStringA(PARASection,para,"10", buf,40,parafilename);
	g_PTS_DTS_Flags= atoi(buf);
	*/
	return true;
}

void InitGlobalVar()
{
	// clear program counter
	g_VideoFrameIdx = 0;
	g_PROGRAM_COUNT = 0;
	for(int i=0; i<10 ; i++)
	{
		g_Counter[i].PID = 0xFFFF;
		g_Counter[i].continuity_counter = 0x0;
	}
	g_llPCR_Accumulator = 0;//216969;//0;
	g_llPTS_Accumulator = 0;
	g_llDTS_Accumulator = 0;
	memset(g_InputFileDirectory,0,200);
	memset(g_FrameNamePrefix,0,200);
	memset(g_OutputFileName,0,200);
	// default value
	sprintf(g_OutputFileName,"TS.ts");
	g_InputFrameNumber=30;
	g_PCRInterval=3;
	g_PTS_DTS_Flags=10;	
}

/*
#if 0
int main(int argc, char *argv[])
{
	BYTE *InputData_Buffer = NULL;
	int InputBufSize = 0; 
	BYTE *OutputData_Buffer = NULL;
	int OutputBufSize = 0; 

	int i=0;
	
	//
	//	initial global variables
	//
	InitGlobalVar();

	
	char ParamFileName[200];
	memset(ParamFileName,0,200);

	if(argc!=1)	// execute by cmd
	{	
		sprintf(ParamFileName,"%s",argv[1]);
	}
	else		// execute by IDE
	{
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\qcif_300f_I\\param_qcif_300f_I.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\Salesman\\param_Salesman.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\320x240\\param_320x240.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\640x480\\param_640x480.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\cif_300f_I\\param_cif_300_I.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\cif_300f_IP\\param_cif_300_IP.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\720p_240f_I\\param_720p_240f_I.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\720p_188f_IP\\param_720p_188f_IP.txt");
		//sprintf(ParamFileName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\vga_100f_I\\param_vga_100f_I.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\vga_100f_IP\\param_vga_100f_IP.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\vga_100f_IP\\param_vga_100f_IP2.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\xga_100f_I\\param_xga_100f_I.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\xga_100f_IP\\param_xga_100f_IP.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\1080p_100f_I\\param_1080p_100f_I.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\1080p_100f_IP\\param_1080p_100f_IP.txt");
		
		// Change H264 Setting
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame3\\param_N1_Frame3.txt");
		sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame4\\param_N1_Frame4.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame5\\param_N1_Frame5.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame6\\param_N1_Frame6.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame7_AUD_SEI_Separate_F0\\param_N1_Frame7_AUD_SEI_Separate_F0.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame8_AUD_SEI_Merge\\param_N1_Frame8_AUD_SEI_Merge.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame9_ChangeSEI_Others\\param_N1_Frame9_ChangeSEI_Others.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame10_Change_First_SEI\\param_N1_Frame10_Change_First_SEI.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\FreeTalk_Frame1\\param_FreeTalk_Frame1.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\FreeTalk_Frame2_SEI_Add3bytes_4alignment\\param_FreeTalk_Frame2_SEI_Add3bytes_4alignment.txt");
		//sprintf(ParamFileName, "D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\FreeTalk_Frame3_all_I_frame\\param_FreeTalk_Frame3_all_I_frame.txt");
	}

	if(!ReadParam(ParamFileName))
	{
		printf("error:%s\n",ParamFileName);
		exit(1);
	}

	char VideoFrameName[200];
	memset(VideoFrameName,0,200);

	for(int idx = 0; idx<g_InputFrameNumber ; idx++)
	{
		//if(idx%30==0)
			printf("%d ",idx);
		
		// Read H.264 video sequence..
#ifdef _EXEC_BY_FILE

		sprintf(VideoFrameName,"%s%s%03d.264",g_InputFileDirectory,g_FrameNamePrefix,idx);
#else
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\320x240\\ff_%03d.264",g_VideoFrameIdx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\640x480\\ff_%03d.264",g_VideoFrameIdx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\vga_100f_IP\\ff_%03d.264",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\720p_240f\\ff_%03d.264",g_VideoFrameIdx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\720p_188f_IP\\ff_%03d.264",g_VideoFrameIdx);
		
		// for N1's H.264
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\smallsize\\ff_%03d.264",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame3\\Frame_%04d.bin",idx);
		sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame4\\Frame_ (%d).bin",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame5\\Frame_ (%d).bin",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame6\\Frame_ (%d).bin",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame7_AUD_SEI_Separate_F0\\Frame_ (%d).bin",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame8_AUD_SEI_Merge\\Frame_ (%d).bin",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame9_ChangeSEI_Others\\Frame_ (%d).bin",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\N1_Frame10_Change_First_SEI\\Frame_ (%d).bin",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\FreeTalk_Frame1\\Frame_ (%d).bin",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\FreeTalk_Frame2_SEI_Add3bytes_4alignment\\Frame_ (%d).bin",idx);
		//sprintf(VideoFrameName,"D:\\Work Directory\\MPEG2-TS\\H.264 Video Sequence\\FreeTalk_Frame3_all_I_frame\\Frame_ (%d).bin",idx);
#endif
		
		InputData_Buffer = ReadVideoSequence(VideoFrameName, InputBufSize);

		// Allocate Memory for Output data buffer
		// Preallocte size = (inSize / minTSPacketSize = 174 )*188 + 188*11 (PAT and PMT packet + 9 TS Null packets)
		int tempBufsize = InputBufSize*2 + 188*11;		// double size for N1's H264 input
		OutputData_Buffer = new BYTE [tempBufsize];
		memset(OutputData_Buffer,0,tempBufsize);
		OutputBufSize = 0;

		// Construct Transport Streams for input H.264 video
		Construct_TS(idx, InputData_Buffer, InputBufSize, OutputData_Buffer, OutputBufSize);

		// Save TS file generated from each single input Frame
#ifdef _TEST_MODE
		Write_To_IdxFile(idx, OutputData_Buffer, OutputBufSize);
#endif

/*
		g_llPCR_Accumulator += 3003;
		g_llPTS_Accumulator += 3013;	// 3003 -> 33ms
		g_llDTS_Accumulator += 3023;	// 3003 -> 33ms
*/
/*
		if(idx==0)
		{
			g_llPCR_Accumulator += 82187;
			g_llPTS_Accumulator += 82207;	// 3003 -> 33ms
			g_llDTS_Accumulator += 82197;	// 3003 -> 33ms
		}
		else if( idx!=0 && idx%2==0)		// Frame Packet
		{
			g_llPCR_Accumulator += 3;
			g_llPTS_Accumulator += 3;	
			g_llDTS_Accumulator += 3;	
		}
		else	// H264 AUD, SEI Packet
		{
			g_llPCR_Accumulator += 4040;
			g_llPTS_Accumulator += 4040;	
			g_llDTS_Accumulator += 4040;	
		}

		// Append NULL TS Packets
		int NULLPacketNumber = 9;
		Append_TS_NULL_Packet(NULLPacketNumber, idx, OutputData_Buffer, OutputBufSize);

		// Free memory space for each Frame
		delete [] InputData_Buffer;
		InputData_Buffer = NULL;
		delete [] OutputData_Buffer;
		OutputData_Buffer = NULL;

	}

	return 0;
}
#endif

BYTE *ReadVideoSequence(const char* filename, int &FileSize)
{
	BYTE *filebuffer = NULL;
	FILE *fp = fopen(filename,"rb");
	if (fp==NULL) {fputs ("Read File error",stderr); exit(1);}
	// obtain file size:
	fseek (fp , 0 , SEEK_END);
	long lSize = ftell (fp);
	rewind(fp);
	// allocate memory to contain the whole file:
	filebuffer = new BYTE [lSize];
	if (filebuffer == NULL) {fputs ("Memory error",stderr); exit(1);}
	// copy the file into the buffer:
	size_t result = fread (filebuffer,1,lSize,fp);
	if (result != lSize) {
		fputs ("Reading error",stderr); 
		// Houston 2011/04/15 fixed memory leak +++
		if(filebuffer)
		{
			delete [] filebuffer;
			filebuffer = NULL;
		}
		fclose(fp);
		// Houston 2011/04/15 fixed memory leak ---
		exit(1);
	}
	fclose(fp);

	FileSize = lSize;
	return filebuffer;
}

BYTE *ReadVideoSequence4N(const char* filename, int &FileSize)
{
	BYTE *filebuffer = NULL;
	FILE *fp = fopen(filename,"rb");
	if (fp==NULL) {fputs ("Read File error",stderr); exit(1);}
	// obtain file size:
	fseek (fp , 0 , SEEK_END);
	long lSize = ftell (fp);
	rewind(fp);

	// input size should be 4N bytes
	int StuffNum = 4 - (lSize % 4) ;
	if(StuffNum < 4)
		FileSize = lSize + StuffNum;
	else
		FileSize = lSize;

	// allocate memory to contain the whole file:
	filebuffer = new BYTE [FileSize];
	if (filebuffer == NULL) {fputs ("Memory error",stderr); exit(1);}
	// copy the file into the buffer:
	size_t result = fread (filebuffer,1,lSize,fp);
	if (result != lSize) {fputs ("Reading error",stderr); exit(1);}
	fclose(fp);
	
	// stuffing 0x00 in the end of input data
	while(StuffNum > 0 && StuffNum < 4)
	{
		filebuffer[lSize++] = 0x00;
		StuffNum--;
	}

	return filebuffer;
}*/
void Append_TS_NULL_Packet(int num, 
						   int idx, 
						   BYTE *OutputData_Buffer,
						   int &OutputBufSize)
{
	BYTE TS_NULL[TS_PACKET_LENGTH];
	memset(TS_NULL,0xff,TS_PACKET_LENGTH);
	TS_NULL[0] = 0x47;
	TS_NULL[1] = 0x1F;
	TS_NULL[2] = 0xFF;
	TS_NULL[3] = 0x10;

	for(int i=0; i<num ; i++)
	{
#ifdef _TEST_MODE
	//	Write_To_IdxFile(idx, TS_NULL, TS_PACKET_LENGTH);
#endif
	//	Write_To_File(TS_NULL, TS_PACKET_LENGTH);	
		// copy data to output_buffer
		memcpy(&OutputData_Buffer[OutputBufSize], TS_NULL, TS_PACKET_LENGTH);
		OutputBufSize += TS_PACKET_LENGTH;
	}
}

BYTE BitMaskRange(int HighBit, int LowBit)
{
	BYTE Mask = 0x0;
	for(int i = LowBit; i<= HighBit; i++)
		Mask |= 0x1 << i;
	return Mask;
}

BYTE GetByteVal(BYTE B, int HighBit, int LowBit)
{
	BYTE Mask = BitMaskRange(HighBit,LowBit);
	return (B&Mask)>>LowBit;
}

bool ByteToByte(BYTE Bsrc, int Hsrc, int Lsrc, BYTE &Bdst, int Hdst, int Ldst)
{
	if( (Hsrc-Lsrc) == (Hdst-Ldst) )
	{
		BYTE src_temp = GetByteVal(Bsrc,Hsrc,Lsrc);		// get B1's bits
		BYTE Bdst_Mask = BitMaskRange(Hdst,Ldst);		// set B2's Mask
		src_temp = src_temp << Ldst;	// shift B1's bits to match B2's Low bit
		BitSetVal(Bdst, Bdst_Mask,src_temp);			// set to B2	
		return true;
	}
	else
	{	
		//printf("\n *** Error in BytetoByte : # bits are equal !\n");
		return false;
	}
}

bool Write_To_File(BYTE *output, int len)
{
	/*
#if defined (__CAlCULATE_MUX_TIME__)
	return true;
#endif

	FILE *fp;
	fp = fopen(g_OutputFileName,"a+b");
	if(fp==NULL) {printf("Open %s error! \n",g_OutputFileName); return false;}
	fwrite(output, 1, len, fp);	// write 1 byte each time
	fclose(fp);
	*/
	return true;
}

bool Write_To_IdxFile(int idx, BYTE *output, int len)
{
	// Write TS file (binary)
/*	FILE *fp;
	int Corresponding_File_idx = 0;
	char OutputFileName[256] = {0};

	if(idx == 0)
		sprintf(OutputFileName,"%d.ts",idx);
	else
	{
		// file idx = ceil(idx/2.0)
		Corresponding_File_idx = ((double)idx/2.0) > (idx/2) ? ((idx/2)+1) : (idx/2) ;
		sprintf(OutputFileName,"%d.ts",Corresponding_File_idx );
	}

	fp = fopen(OutputFileName,"a+b");
	if(fp==NULL) {printf("Open %s error! \n",g_OutputFileName); return false;}
	fwrite(output, 1, len, fp);	// write 1 byte each time
	fclose(fp);
	
	// Write TS file (Text)
	FILE *fp2;
	sprintf(OutputFileName,"%s.txt",OutputFileName);
	fp2 = fopen(OutputFileName,"a+b");
	if(fp2==NULL) {printf("Open %s error! \n",g_OutputFileName); return false;}
	
	for(int i=0; i<len; i++)
		fprintf(fp2,"%02x\n",output[i]);
	fclose(fp2);
*/
	return true;
}

BYTE Return_Contin_Counter(WORD PID)
{
	// Search PID in Global Counter
	int i=0;
	while(i<10)
	{
		if(g_Counter[i].PID == PID)
			break;
		else
			i++;
	}
	return g_Counter[i].continuity_counter;
}


void Calc_PCR(BYTE *PCR_Base, BYTE *PCR_Ext)
{
	long long pcr_b	= 0;

#ifdef _TEST_MODE
	pcr_b = 0x0;		//Set 0 to all bits of PCR_Base(33 bits)
#else
	pcr_b = g_llPCR_Accumulator;
#endif

	long long Mask33_25 = 0x00000001FE000000;
	long long Mask24_17 = 0x0000000001FE0000;
	long long Mask16_9  = 0x000000000001FE00;
	long long Mask8_1	= 0x00000000000001FE;
	long long Mask0		= 0x0000000000000001;

	BitSet(PCR_Base[0],(pcr_b & Mask33_25)>>25	);		// 33~25 @[0](7:0)
	BitSet(PCR_Base[1],(pcr_b & Mask24_17)>>17	);		// 24~17 @[1](7:0)
	BitSet(PCR_Base[2],(pcr_b & Mask16_9) >>9	);		// 16~9  @[2](7:0)
	BitSet(PCR_Base[3],(pcr_b & Mask8_1)  >>1	);		// 8~1	 @[3](7:0)
	BitSet(PCR_Base[4],(pcr_b & Mask0)	  <<7	);		// 0	 @[4](7:7)

	// Set 1 to all bits of PCR_Ext(9 bits)
	BitSet(PCR_Ext[0],0x00);	// 8~1
	BitSet(PCR_Ext[1],0x00);	// 0
}

void Calc_PTS(BYTE *PTS)
{
	long long pts = 0;

#ifdef _TEST_MODE
	pts = 0xFFFFFFFFFFFFFFFF;		//Set 1 to all bits of PTS
#else
	pts = g_llPTS_Accumulator ;
#endif

	long long Mask33_25 = 0x00000001FE000000;
	long long Mask24_17 = 0x0000000001FE0000;
	long long Mask16_9  = 0x000000000001FE00;
	long long Mask8_1	= 0x00000000000001FE;
	long long Mask0		= 0x0000000000000001;

	BitSet(PTS[0],(pts & Mask33_25)>>25	);	// 33~25 @[0](7:0)
	BitSet(PTS[1],(pts & Mask24_17)>>17	);	// 24~17 @[1](7:0)
	BitSet(PTS[2],(pts & Mask16_9) >>9	);	// 16~9  @[2](7:0)
	BitSet(PTS[3],(pts & Mask8_1)  >>1	);	// 8~1	 @[3](7:0)
	BitSet(PTS[4],(pts & Mask0)	   <<7	);	// 0	 @[4](7:7)
}

void Calc_DTS(BYTE *DTS)
{
	long long dts = 0;

//#ifdef _TEST_MODE
//	dts = 0xFFFFFFFFFFFFFFFF;		//Set 1 to all bits of PTS
//#else
	dts = g_llDTS_Accumulator;
//#endif

	long long Mask33_25 = 0x00000001FE000000;
	long long Mask24_17 = 0x0000000001FE0000;
	long long Mask16_9  = 0x000000000001FE00;
	long long Mask8_1	= 0x00000000000001FE;
	long long Mask0		= 0x0000000000000001;

	BitSet(DTS[0],(dts & Mask33_25)>>25	);	// 33~25 @[0](7:0)
	BitSet(DTS[1],(dts & Mask24_17)>>17	);	// 24~17 @[1](7:0)
	BitSet(DTS[2],(dts & Mask16_9) >>9	);	// 16~9  @[2](7:0)
	BitSet(DTS[3],(dts & Mask8_1)  >>1	);	// 8~1	 @[3](7:0)
	BitSet(DTS[4],(dts & Mask0)	   <<7	);	// 0	 @[4](7:7)
}


unsigned long crc32(const unsigned char *ss, int len)
{
	unsigned long val = 0xFFFFFFFFL ; 
    const unsigned char *s = ss;
#ifdef _CRC32_MPEG2
    while (--len >= 0)
    {
		// normal form : No Refelected for MPEG2
		val = crc32_table[((val>>24) ^ *s++) & 0xff] ^ (val << 8);
    }
	return (val^0x00000000L);	// XOR OUT = 0x00000000
#endif

#ifdef _CRC32_STD
	while (--len >= 0)
    {	
		// reflected form for Standard
		val = crc32_table[(val ^ *s++) & 0xff] ^ (val >> 8);
    }
    return (val^0xFFFFFFFFL);	// XOR OUT = 0xFFFFFFFF
#endif
}

void FreeTS(STRUCT_TS *pts)
{
	if(pts != NULL)
	{
		/*	// Houston 2011/04/15
		if(pts->TS_Adaptation_Field != NULL)
		{
			delete [] pts->TS_Adaptation_Field;
			pts->TS_Adaptation_Field = NULL;
		}
		
		if(pts->TS_Output!= NULL)
		{
			delete [] pts->TS_Output;
			pts->TS_Output = NULL;
		}
		*/

		delete pts;
		pts = NULL;
	}
	else 
		return;
}

void FreePAT(STRUCT_PAT *ppat)
{
	if(ppat != NULL)
	{
		/* // Houston 2011/04/15 remove
		if(ppat->PAT_Output != NULL)
		{
			delete [] ppat->PAT_Output;
			ppat->PAT_Output = NULL;
		}
		if(ppat->PATable != NULL)
		{
			delete [] ppat->PATable;
			ppat->PATable = NULL;
		}
		if(ppat->Stuffing_Data != NULL)
		{
			delete [] ppat->Stuffing_Data;
			ppat->Stuffing_Data = NULL;
		}
		*/

		delete ppat;
		ppat = NULL;
	}
}
void FreePMT(STRUCT_PMT *ppmt)
{
	if(ppmt != NULL)
	{
		/* // Houston 2011/04/15 remove
		if(ppmt->PMT_Output != NULL)
		{
			delete [] ppmt->PMT_Output;
			ppmt->PMT_Output = NULL;
		}
		if(ppmt->PMTable != NULL)
		{
			delete [] ppmt->PMTable;
			ppmt->PMTable = NULL;
		}
		if(ppmt->Stuffing_Data != NULL)
		{
			delete [] ppmt->Stuffing_Data;
			ppmt->Stuffing_Data = NULL;
		}
		*/
		delete ppmt;
		ppmt = NULL;
	}
}

void FreePES(STRUCT_PES *ppes)
{
	if(ppes != NULL)
	{
		/* // Houston 2011/04/15 remove
		if(ppes->PES_Header_Fields != NULL)
		{
			delete [] ppes->PES_Header_Fields;
			ppes->PES_Header_Fields = NULL;
		}*/
		if(ppes->PES_Output != NULL)
		{
			delete [] ppes->PES_Output;
			ppes->PES_Output = NULL;
		}
		if(ppes->PES_Packet_Data != NULL)
		{
			delete [] ppes->PES_Packet_Data;
			ppes->PES_Packet_Data = NULL;
		}
		delete ppes;
		ppes = NULL;
	}
}

/*
void DBG_Print(STRUCT_PMT *ppmt)
{
	printf("\n -- PMT -- \n");
	printf("[0]   TABLE_ID          :%02x\n",ppmt->TABLE_ID);
	printf("[1~2] Section           :%02x%02x\n",ppmt->Section[0],ppmt->Section[1]);
	printf("[3~4] Porgram_Number    :%02x%02x\n",ppmt->Porgram_Number[0],ppmt->Porgram_Number[1]);
	printf("[5]   Ver#_CurNextIdx   :%02x\n",ppmt->VerN_CurNextIdx);
	printf("[8~9] PCR_PID           :%02x%02x\n",ppmt->PCR_PID[0],ppmt->PCR_PID[1]);
	printf("[10~11] Prog. info len  :%02x%02x\n",ppmt->Prog_Info_Length[0],ppmt->Prog_Info_Length[1]);
	printf("-PMTable- \n");
	printf("[12]    stream type     :%02x\n",ppmt->PMTable[0]);
	printf("[13~14] element PID     :%02x%02x\n",ppmt->PMTable[1],ppmt->PMTable[2]);
	printf("[15~16] ES info len     :%02x%02x\n",ppmt->PMTable[3],ppmt->PMTable[4]);
}
void DBG_Print(STRUCT_PAT *ppat)
{
	printf("\n -- PAT -- \n");
	printf("[0]   TABLE_ID          :%02x\n",ppat->TABLE_ID);
	printf("[1~2] Section           :%02x%02x\n",ppat->Section[0],ppat->Section[1]);
	printf("[3~4] TS_ID             :%02x%02x\n",ppat->TS_ID[0],ppat->TS_ID[1]);
	printf("[5]   Ver#_CurNextIdx   :%02x\n",ppat->VerN_CurNextIdx);
	printf("[8~]  PATable: prog. #  :%02x%02x\n",ppat->PATable[0],ppat->PATable[1]);
	printf("[ ~]  PATable: prog PID :%02x%02x\n",ppat->PATable[2],ppat->PATable[3]);

}
void DBG_Print(STRUCT_TS *pts)
{
	printf("\n -- TS_Header 4 bytes -- \n");
	printf("[0](7:0) sync			:%2x\n",pts->TS_Header[0]);
	printf("[1] payload	& PID(13~8)	:%2x\n",pts->TS_Header[1]);
	printf("[2]	          PID( 7~0)	:%2x\n",pts->TS_Header[2]);
	printf("[3]	ada_field & counter	:%2x\n",pts->TS_Header[3]);
	printf("\n -- Adaptation Field 6 bytes -- \n");
	if(pts->TS_Adaptation_Field != NULL)
	{
		printf("[0] flag            :%2x\n",pts->TS_Adaptation_Field[0]);
		printf("[1]                 :%2x\n",pts->TS_Adaptation_Field[1]);
		printf("[2]                 :%2x\n",pts->TS_Adaptation_Field[2]);
		printf("[3]                 :%2x\n",pts->TS_Adaptation_Field[3]);
		printf("[4]                 :%2x\n",pts->TS_Adaptation_Field[4]);
		printf("[5] reserve (6:1)   :%2x\n",pts->TS_Adaptation_Field[5]);
		printf("[6]                 :%2x\n",pts->TS_Adaptation_Field[6]);
	}
}
void DBG_Print(STRUCT_PES *p)
{
	printf("\n -- PES packet -- \n");
	printf("Packet_Start_Code_Prefix (24):%02x%02x%02x\n",p->Packet_Start_Code_Prefix[0],p->Packet_Start_Code_Prefix[1],p->Packet_Start_Code_Prefix[2]);
	printf("Stream_ID                (8) :0x%02x\n",p->Stream_ID);
	printf("PES_Packet_Length        (16):0x%04x\n",p->PES_Packet_Length);
	printf("PES_Header_Flags         (16):0x%04x\n",p->PES_Header_Flags);
	printf("PES_Header_Data_Length   (8) :0x%02x\n",p->PES_Header_Data_Length);
	printf("PTS_DTS_Flag             (2) :%d%d   -> PES_Header_Flags[8:9] \n",(p->PES_Header_Flags & 0x80)>>7 ,(p->PES_Header_Flags & 0x40)>>6 );
	//printf("input ES data  (%3d   bytes) :%s\n",  p->VideoESData_len,p->PES_Packet_Data);

	// PTS
	printf("PES_Header_Fields PTS: %02x %02x %02x %02x %02x\n",
		p->PES_Header_Fields[0],
		p->PES_Header_Fields[1],
		p->PES_Header_Fields[2],
		p->PES_Header_Fields[3],
		p->PES_Header_Fields[4]);
	// DTS
	//if((p->PES_Header_Flags & 0x40)>>6 )
	//if(BitNtst(p->PES_Header_Flags,W_BitPos(9)))
	if(BitTst(p->PES_Header_Flags,0x40) )
	{
		printf("PES_Header_Fields DTS: %02x %02x %02x %02x %02x\n",
		p->PES_Header_Fields[5],
		p->PES_Header_Fields[6],
		p->PES_Header_Fields[7],
		p->PES_Header_Fields[8],
		p->PES_Header_Fields[9]);
	}
}
void DBG_PrintPROGRAM()
{
	printf("\n--- PROGRAM TABLE --- \n");
	printf("\n      PID      Count  \n");
	for(int i=0; i<10 ; i++)
	{
		printf("      %02x      %d\n",g_Counter[i].PID,g_Counter[i].continuity_counter);
	}
}

*/
