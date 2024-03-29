#if !defined(__CUSTOMPROP_H__)
#define __CUSTOMPROP_H__

#include <ks.h>

//
// Customer defined properties
//
typedef enum {
	KSPROPERTY_CUSTOM_PROP_MIRROR,
	KSPROPERTY_CUSTOM_PROP_FLIP,
	KSPROPERTY_CUSTOM_PROP_SNAPSHOTIMAGESIZE,
	KSPROPERTY_CUSTOM_PROP_RESETSETTINGS,
	KSPROPERTY_CUSTOM_PROP_SAVESETTINGS,
	KSPROPERTY_CUSTOM_PROP_RESTORESETTINGS,				// 5
	KSPROPERTY_CUSTOM_PROP_BW_MODE,
	KSPROPERTY_CUSTOM_PROP_IMAGEFORMAT,
	KSPROPERTY_CUSTOM_PROP_EFFECT,
	KSPROPERTY_CUSTOM_PROP_FRAME_FILENAME,
	KSPROPERTY_CUSTOM_PROP_CAPTURE_WIDTH,				// 10
	KSPROPERTY_CUSTOM_PROP_CAPTURE_HEIGHT,
	KSPROPERTY_CUSTOM_PROP_PROPERTY_PAGE_CONTROL,
	KSPROPERTY_CUSTOM_PROP_GETSENSORINFO,				// obsolete
	KSPROPERTY_CUSTOM_PROP_SNAPSHOTIMAGESIZE2,			// obsolete
	KSPROPERTY_CUSTOM_PROP_WBTemperatureControlAuto,	// 15
	KSPROPERTY_CUSTOM_PROP_WBComponentControl,
	KSPROPERTY_CUSTOM_PROP_WBComponentControlAuto,
	KSPROPERTY_CUSTOM_PROP_AEModeControl,
	KSPROPERTY_CUSTOM_PROP_STILLMETHOD,
	KSPROPERTY_CUSTOM_PROP_AEPriorityControl,			// 20
	KSPROPERTY_CUSTOM_PROP_ROTATION,
	KSPROPERTY_CUSTOM_PROP_AUXLED_MODE,
	KSPROPERTY_CUSTOM_PROP_SNAPSHOT_CONTROL,
	
	// shawn 2011/05/25 +++++
	//0: Disable LED auto control on property page.
	//1: Enable  LED auto control on property page.
	KSPROPERTY_CUSTOM_PROP_AUXLED_AUTO_CONTROL,
	KSPROPERTY_CUSTOM_PROP_DENOISE_MODE,
	KSPROPERTY_CUSTOM_PROP_WBComponentControl_Green,
	KSPROPERTY_CUSTOM_PROP_EECOM_LED,					// for EECOM customized
	KSPROPERTY_CUSTOM_PROP_EECOM_IR,					// for EECOM customized
	// shawn 2011/05/25 -----

	KSPROPERTY_CUSTOM_PROP_END
} KSPROPERTY_CUSTOM_PROP;

typedef enum {
	KSPROPERTY_ZOOM_PROP_LEFTDOWN,
	KSPROPERTY_ZOOM_PROP_LEFT, 
	KSPROPERTY_ZOOM_PROP_LEFTUP, 
	KSPROPERTY_ZOOM_PROP_UP,
	KSPROPERTY_ZOOM_PROP_RIGHTUP,
	KSPROPERTY_ZOOM_PROP_RIGHT,
	KSPROPERTY_ZOOM_PROP_RIGHTDOWN,
	KSPROPERTY_ZOOM_PROP_DOWN, 
	KSPROPERTY_ZOOM_PROP_ZOOM_IN,
	KSPROPERTY_ZOOM_PROP_ZOOM_IN_MAX,	// get the maximum value for zoom in
	KSPROPERTY_ZOOM_PROP_STATUS,		// get zoom status
	KSPROPERTY_ZOOM_PROP_ENABLE_OPTION,	// 0 : All Disable, 1 : Zoom, 2 : Face
	KSPROPERTY_ZOOM_PROP_WIDTH,			// get preview width
	KSPROPERTY_ZOOM_PROP_UI_CONTROL,	// 0 : Hide Face Tracking UI, 1 : Show Face Tracking UI
	KSPROPERTY_ZOOM_PROP_PAN_MAX,		// get Pan max
	KSPROPERTY_ZOOM_PROP_PAN_POS,		// get/set Pan absolute position
	KSPROPERTY_ZOOM_PROP_TILT_MAX,		// get tilt max
	KSPROPERTY_ZOOM_PROP_TILT_POS,		// get/set Tilt absolute position
	KSPROPERTY_ZOOM_PROP_MIRROR_FLIP,
	KSPROPERTY_ZOOM_PROP_END
} KSPROPERTY_ZOOM_PROP;

typedef enum {
	KSPROPERTY_CLIENT_PROP_ASICREAD, 
	KSPROPERTY_CLIENT_PROP_ASICWRITE,
	KSPROPERTY_CLIENT_PROP_END
} KSPROPERTY_CLIENT_PROP;

typedef enum {
	DISABLE_RIGHT=1,
	DISABLE_LEFT=2, 
	DISABLE_DOWN=4, 
	DISABLE_UP=8, 
	DISABLE_ZOOM_OUT=16,
	DISABLE_ZOOM_IN=32,
	DISABLE_END,
} KSPROPERTY_ZOOM_PROP_DISABLE;

//--------------------------------------------------------------------------//
//						Define ST50220 Device GUID							//
//--------------------------------------------------------------------------//
DEFINE_GUID(PROPSETID_CUSTOM_PROP_ST50220, 
0xf1cafc1d, 0xd7d6, 0x400f, 0xa7, 0xfd, 0x96, 0x71, 0x5e, 0xb, 0xd9, 0x59);
DEFINE_GUID(PROPSETID_ZOOM_PROP_ST50220, 
0xf1cafc1d, 0xd7d6, 0x400f, 0xa7, 0xfd, 0x96, 0x71, 0x5e, 0xb, 0xd9, 0x5a);
DEFINE_GUID(PROPSETID_CLIENT_PROP_ST50220, 
0xf1cafc1d, 0xd7d6, 0x400f, 0xa7, 0xfd, 0x96, 0x71, 0x5e, 0xb, 0xd9, 0x58);
//2010/8/6 03:50�U��
DEFINE_GUID(SonixAVstreamFilter, 
0xBBEFB6C7, 0x2FC4, 0x4139, 0xBB, 0x8B, 0xA5, 0x8B, 0xBA, 0x72, 0x40, 0x84);


#define PRODUCT_BASENAME	L"snp2uvc"
#define SN9C220_KEY_PATH	L"\\Registry\\MACHINE\\SOFTWARE\\Sonix\\PCCAM\\" PRODUCT_BASENAME

typedef struct {
    KSPROPERTY Property;
    ULONG   Value;
    ULONG   MinValue;
    ULONG   MaxValue;
    ULONG   DefValue;
    ULONG   ResValue;
    CHAR	FileName[MAX_PATH];
} KSPROPERTY_CUSTOM_PROP_S, *PKSPROPERTY_CUSTOM_PROP_S;

typedef struct {
    KSPROPERTY Property;
    LONG   Value;                       
    ULONG  Flags;                       
    ULONG  Capabilities;                     
} KSPROPERTY_ZOOM_PROP_S, *PKSPROPERTY_ZOOM_PROP_S;

typedef struct {
    KSPROPERTY Property;
    ULONG	index;
    LONG	value;
    ULONG   valueSize;
	ULONG   common;
} KSPROPERTY_CLIENT_PROP_S, *PKSPROPERTY_CLIENT_PROP_S;

//
// Control request IDs.
//

#endif //__CUSTOMPROP_H__