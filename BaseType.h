#ifndef __BASETYPE_H__
#define __BASETYPE_H__

#if defined(__LINUX__)
    #define DELAY(Tms) udelay(Tms)
#else
    #if defined(__KERNEL_MODE__)
        #define DELAY(Tms)  KeDelay((Tms))
	#define FLOATING KFLOATING_SAVE
	#if defined(__PREFAST__)
		#define START_FLOATING(FloatSave,NtStatus)	NtStatus = KeSaveFloatingPointState(&FloatSave);
		#define END_FLOATING(FloatSave,NtStatus)	NtStatus = KeRestoreFloatingPointState(&FloatSave);
		#define CHECK_FLOATING_BOOL(ntsatus)		if(ntsatus != STATUS_SUCCESS) return NULL;
		#define CHECK_FLOATING_VOID(ntsatus)		if(ntsatus != STATUS_SUCCESS) return;
		#define CHECK_FLOATING_VAR(ntsatus,value)	if(ntsatus != STATUS_SUCCESS) return value;
	#else
		#define ExFreePoolWithTag(mp,a) ExFreePool(mp)
		#define START_FLOATING
		#define END_FLOATING
		#define CHECK_FLOATING_BOOL
		#define CHECK_FLOATING_VOID
		#define CHECK_FLOATING_VAR
	#endif
    #else
        #include "stdafx.h"
        #define DELAY(Tms)  ::SleepEx((Tms), TRUE)
		#define FLOATING long
		#define NTSTATUS long
		#define START_FLOATING
		#define END_FLOATING
		#define CHECK_FLOATING_BOOL
		#define CHECK_FLOATING_VOID
		#define CHECK_FLOATING_VAR
    #endif

	#ifndef _WIN64
	    #define __MMX__
	#endif
#endif

//
#define _ISP_SENSOR_
//
typedef unsigned char           UInt8;
typedef char                    SInt8;
typedef unsigned short          UInt16;
typedef signed short            SInt16;
typedef unsigned long           UInt32;
typedef signed long             SInt32;
typedef unsigned __int64        UInt64;

typedef struct
{
	UInt16 X;
	UInt16 Y;
} DefectPos;
//2010/5/10 11:42¤W¤È
enum ImageSize
{
    isQQVGA,
    isN1_1,//N1
    isQCIF,
    isCUSTOM3,
    isQVGA,
    isCIF,
    isN1_2,//N1
    isCUSTOM4,
    isVGA,
    isN1_3,//N1
    isSVGA,
	isCUSTOM1,
    isXGA,
    isCUSTOM5,
	isCUSTOM2,
    isSXGA,
	isSXGA2,
	isUXGA,
	isFULLHD,	// shawn 2011/07/12 add
	is3M,		// shawn 2011/07/12 add
    isCount
};
/*enum ImageSize
{
    isQQVGA,
    isQCIF,
    isQVGA,
    isCIF,
    isVGA,
    isSVGA,
    isXGA,
    isSXGA,
	isSXGA2,
	isUXGA,
    isCount
};*/
extern const int ImgWidList[isCount], ImgHgtList[isCount], ImgWidList2[isCount];

enum USBSpeed
{
    usbsFull,
    usbsHi,
    usbsCount
};

enum ImageFormat
{
    ifJPEG,
    ifRaw,
    ifYUV422,
    ifYUV420,
    ifCount
};

enum InterMode
{
    imBilinear,
    imEdge,
    imCount
};

enum ASICName
{
    an9C201,
    an9C202,
    anCount
};

enum SensorName
{
	snMI0360,
	snMT9V111,
	snHV7131R,
	snOV7660,
	snOV7670,
	snMI1300,
	snMI1310,
	snMI1320,
	snMT9M011,
	snOV9650,
	snOV9655,
	snSOI968,
	snICM107B,
	snCX1332,
	snS5K53BEB,
	snS5K4AAFX,
	//OV7670 ISP must be the 1st ISP sensor here, don't move it!
	snOV7670ISP,
	snCount
};
extern const char* SensorNameList[snCount];

enum USBBandWidth
{
    USB_BULK=0,
    ISO_1=1,      
    ISO_2,
    ISO_3,
    ISO_4,
    ISO_5,
    ISO_6,
    ISO_7,
    ISO_8,
    bwCount
};
extern const int USBBandWidthList[bwCount];
extern const int USB2BandWidthList[bwCount];

extern const unsigned char Y_QTAB_FINE[64];
extern const unsigned char UV_QTAB_FINE[64];
enum {QTableNum = 30};
typedef struct
{
    unsigned char *pYQTable[2];
    unsigned char *pUVQTable[2];
    UInt32 iTableIndex[2];
    UInt32 iSelQTableBuffer;
    BOOL bQTableSync;
} QTableInfo;


extern int iUSBBandWidth[usbsCount][bwCount];
extern double dDataSizeMul[ifCount];

//2011/4/11 06:18¤U¤È
extern long long g_llPCR_Accumulator ;
extern long long g_llPTS_Accumulator ;
extern long long g_llDTS_Accumulator ;
#endif

#if defined(__MMX__)
    extern BOOL gbSSE, gbSSE2;
#endif
