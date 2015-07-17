#ifndef _SONIX_VIDEO_CLASS_INCLUDE_
#define _SONIX_VIDEO_CLASS_INCLUDE_

extern "C" {
#include <wdm.h>
#include <usbdi.h>
#include <usb100.h>
#include <usbdlib.h>
}

#include <windef.h>
#include <ntstrsafe.h>
#include <stdlib.h>
#include <unknown.h>

#include <ks.h>
//2010/12/1 06:12下午
//#include <ksmedia.h>
#include "ksmedia.h"
#include <strmini.h>

#include <kcom.h>

#include <setjmp.h>
#include <stdarg.h>
#include <math.h>

/*************************************************

    Misc Definitions

*************************************************/
#define STR_MODULENAME "ST50220 AVStream: "
#define DEBUGLVL_VERBOSE 2
#define DEBUGLVL_TERSE 1
#define DEBUGLVL_ERROR 0

const DebugLevel = DEBUGLVL_ERROR;
//const int g_PhysicalVRAMAddress = 0xABCDEF;

#define KS_SIZE_PREHEADER2 (FIELD_OFFSET(KS_VIDEOINFOHEADER2,bmiHeader))
#define KS_SIZE_VIDEOHEADER2(pbmi) ((pbmi)->bmiHeader.biSize + KS_SIZE_PREHEADER2)

#if (DBG)
#define SnPrint(lvl, strings) \
{ \
    if (lvl > DebugLevel) {\
        DbgPrint(STR_MODULENAME);\
        DbgPrint##strings;\
        if ((lvl) == DEBUGLVL_ERROR) {\
            DbgBreakPoint();\
        } \
    }\
}
#else // !DBG
   #define SnPrint(lvl, strings)
#endif // !DBG

#define ABS(x) ((x) < 0 ? (-(x)) : (x))

#ifndef mmioFOURCC    
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#define FOURCC_YUY2         mmioFOURCC('Y', 'U', 'Y', '2')
#define FCC_FORMAT_YUV12A	mmioFOURCC('I','4','2','0')

//
// CAPTURE_PIN_DATA_RANGE_COUNT:
//
// The number of ranges supported on the capture pin.
//
#define CAPTURE_PIN_DATA_RANGE_COUNT 2

//
// CAPTURE_FILTER_PIN_COUNT:
//
// The number of pins on the capture filter.
//
//2010/4/29 05:04下午
#define CAPTURE_FILTER_PIN_COUNT 3

//2010/4/29 05:09下午
#define CAPTURE_FILTER_NODE_COUNT  3

//
// CAPTURE_FILTER_CATEGORIES_COUNT:
//
// The number of categories for the capture filter.
//
#define CAPTURE_FILTER_CATEGORIES_COUNT 2

#define AVSHWS_POOLTAG '1VAS'
#define ST50220DEV_POOLTAG 'vaNS'
//2010/4/30 01:56下午
//#define EXUNIT_MAX_NUM 8
//#define EXUNITCONTROL_MAX_BYTES 8
extern "C" {
#include "effectlib.h"
}

#include "VidClass.h"

#include "DebugUtil.h"
#include "BaseType.h"

#include "FaceTracking.h"

#include "JPEG.h"
#include "Scale.h"
#include "Shell.h"

#include "UsbDev.h"

#include "DevDep.h"	//Device dependent information
#include "CustomProp.h"
#include "StreamData.h"

#include "device.h"
#include "filter.h"
#include "capture.h"
#include "still.h"
#include "VideoStream.h"
#include "VideoDevice.h"
//2011/4/11 03:30下午
#include "TransportStream.h"
#include "bitops.h"
#endif