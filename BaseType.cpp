#if defined(__LINUX__)
    #include <snxcam.h>
#elif defined(__KERNEL_MODE__)
    #include <sncam.h>
#else
    #include "BaseType.h"
#endif

//2010/5/10 11:40¤W¤È
//const int ImgWidList[isCount] = {160, 176, 320, 352, 640, 800, 1024, 1280, 1280, 1600};
const int ImgWidList[isCount] = {160, 160, 176, 320, 320, 352, 384, 640, 640, 768, 800, 960, 1024, 1280, 1280, 1280, 1280, 1600, 1920, 2048};	// shawn 2011/07/13 modify for 3M
const int ImgWidList2[isCount] = {192, 192, 320, 384, 640, 800, 1024, 1280, 1280, 1600};
//const int ImgHgtList[isCount] = {120, 144, 240, 288, 480, 600, 768,  960, 1024, 1200};
const int ImgHgtList[isCount] = {120, 128, 144, 200, 240, 288, 216, 400, 480, 480, 600, 720, 768,   720,  800,  960, 1024, 1200, 1080, 1536};	// shawn 2011/07/13 modify for 3M

const char* SensorNameList[snCount] = 
{
	"MI0360",
	"MT9V111",
	"HV7131R",
	"OV7660",
	"OV7670",
	"MI1300",
	"MI1310",
	"MI1320",
	"MT9M011",
	"OV9650",
	"OV9655",
	"SOI968",
	"ICM107B",
	"CX1332",
	"S5K53BEB",
	"S5K4AAFX",
	"OV7670_ISP"
};

const int USBBandWidthList[bwCount]  = { 1023,  128,  256,  384,  512,  680,  800,   900,  1023};
const int USB2BandWidthList[bwCount] = {24576, 1024, 2048, 3072, 4096, 5440, 6400, 14400, 24576};

const unsigned char Y_QTAB_FINE[64] =
{
     3,   2,   2,   3,   2,   2,   3,   3,
     3,   3,   4,   3,   3,   4,   5,   8,
     5,   5,   4,   4,   5,  10,   7,   7,
     6,   8,  12,  10,  12,  12,  11,  10,
    11,  11,  13,  14,  18,  16,  13,  14,
    17,  14,  11,  11,  16,  22,  16,  17,
    19,  20,  21,  21,  21,  12,  15,  23,
    24,  22,  20,  24,  18,  20,  21,  20
};

const unsigned char UV_QTAB_FINE[64] = 
{
     3,   4,   4,   5,   4,   5,   9,   5,
     5,   9,  20,  13,  11,  13,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20
};

//[HiSpeed,FullSpeed][Bulk,ISO1,ISO2,ISO3,ISO4,ISO5,ISO6,ISO7,ISO8]
int iUSBBandWidth[usbsCount][bwCount] = {
	{0,128000 ,256000 ,384000 ,512000 ,680000 ,800000 ,900000  ,1008000 },
	{0,1024000,2048000,3072000,4096000,5440000,6400000,14400000,24576000},
};
//JPEG,Raw,YUV422,YUV420
double dDataSizeMul[ifCount] = {0.0625,1,2,1.5};

#if defined(__MMX__)
    BOOL gbSSE = FALSE, gbSSE2 = FALSE;
#endif

