/****************************************************************************
*
*  Copyright (c) 2003  Sonix Technology Co., Ltd.
*
*  Module Name: DevDep.h
*
*  Abstract:    Define the structure of device dependent information.
*
*  Revision History:
*
*    03-24-2003 :	Created [Peter]
*	 04-17-2003 :	Modify the architecture of the including files [Peter]
*
*****************************************************************************/

#include "SnCam.h"

#if !defined(__DevDep_h__)
#define __DevExt_h__

class CCaptureDevice;
class CCaptureFilter;
class CCapturePin;
class CStillPin;

class CVideoStream;
class CVideoDevice;

typedef enum {
	STREAM_Capture,		// we always assume vidoe stream is stream 0
	STREAM_Still, 
	STREAM_Capture_MP2TS, 
	STREAM_Counts
};

//#define STREAM_Capture_MP2TS 3
typedef enum {
//	KSEVENT_VIDCAPTOSTI_EXT_TRIGGER,	// already define in ksmedia.h
//	KSEVENT_VIDCAP_AUTO_UPDATE,			// already define in ksmedia.h
//	KSEVENT_VIDCAP_SEARCH				// already define in ksmedia.h
	KSEVENT_VIDCAPTOSTI_BEGIN = 0x10,	// just the count, not the real event item
	KSEVENT_VIDCAPTOSTI_AUTO_LANUCH = KSEVENT_VIDCAPTOSTI_BEGIN,
	KSEVENT_VIDCAPTOSTI_OPEN_STREAM,
	KSEVENT_VIDCAPTOSTI_CLOSE_STREAM,
	KSEVENT_VIDCAPTOSTI_DEVICE_REMOVAL,
	KSEVENT_VIDCAPTOSTI_END				// just the count, not the real event item
} KSEVENT_VIDCAPTOSTI_EX;

#define KSEVENT_VIDCAPTOSTI_COUNT (KSEVENT_VIDCAPTOSTI_END - KSEVENT_VIDCAPTOSTI_BEGIN)

#define MAX_STREAM_FORMAT 40
#define FRAME_RATE_LIST_SIZE 240

typedef struct FrameRatesInfo {
	LONGLONG		CurrentActualFrameRate;						//KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE
	LONGLONG		CurrentMaxAvailableFrameRate;
	LONGLONG		SupportedFrameRates[FRAME_RATE_LIST_SIZE];	//KSPROPERTY_VIDEOCONTROL_FRAME_RATES
	UCHAR			m_FrameRatesIndex[MAX_STREAM_FORMAT];
} FRAMERATEINFO, *PFRAMERATEINFO;

#define SONIX_BUS_BW_ARRAY_SIZE     20

typedef struct {
    UCHAR           AlternateSetting;	// Alt. interface number.
    ULONG           MaxPktSize;			// Max. Pkt size for this alt. interface
	UCHAR			NumberOfTransaction;
} MaxPktSizePerInterface;

// format type
// shawn 2011/05/25 rearrange for sync with AP +++++
/*enum FORMATTYPE {
	IMG_FMT_RGB24,
	IMG_FMT_I420,
	IMG_FMT_YUY2,
	IMG_FMT_MJPEG,
	IMG_FMT_COUNT
};*/

enum FORMATTYPE {
	IMG_FMT_YUY2 = 1,
	IMG_FMT_MJPEG,
	IMG_FMT_RGB24,
	IMG_FMT_I420,
	IMG_FMT_H264,	// shawn 2011/06/27 add
	IMG_FMT_COUNT
};
// shawn 2011/05/25 rearrange for sync with AP -----

// video stream info
struct VIDEOSTREAMINFO {
	UCHAR  		bFormatIndex;
	UCHAR  		bFrameIndex;
	USHORT		wWidth;
	USHORT		wHeight;
	ULONG		biCompression;
	ULONG		ImageSize;
	FORMATTYPE	ImageFormat;
	//2010/5/13 02:36下午
	UCHAR		FrameRatesCount;
	LONGLONG	SupportedFrameRates[10];	
};

#define DEFAULT_REGISTRY_NUMBER 50

struct PROPERTYINFO {
	ULONG	DefValue;
	ULONG	DefValueHW;
	ULONG	CurrentValue;
};

struct PROPERTYINFO_WBC {
	ULONG	DefValue;
	ULONG	DefValueHW;
	ULONG	MaxValue;
	ULONG	MinValue;
	ULONG	CurrentValue;
	ULONG	ResValue;
};

//
// Property Information
//
enum {
	PROPERTIESFORVIDEOPROCAMP,
	BRIGHTNESS = PROPERTIESFORVIDEOPROCAMP,
	CONTRAST, 
	HUE, 
	SATURATION, 
	SHARPNESS, 
	GAMMA, 
	WHITEBALANCE,
	BACKLIGHT, 
	POWERLINEFREQUENCY,
	GAIN,
	NUM_VIDEO_AMP_PROPERTY,
	EXPOSURE = NUM_VIDEO_AMP_PROPERTY,
	FOCUS,
	//2010/3/16 03:34下午
	FOCUS_AUTO,
	IRIS,
	ZOOM,
	PAN,
	ROLL,
	PRIVACY,
	TILT, 
	AEMODE,
	AEPRIORITY,	
	//2011/1/5 05:57下午 test
	//PANTILT,
	NUMOFSTREAMPROPERTIES
};

enum {	// don't use below within StreamProperties[]
	MIRROR = NUMOFSTREAMPROPERTIES,	// 17
	FLIP,
	BW_MODE,
	AE_MODE,						// 20
	AE_PRIORITY,
	WB_COMPONENT,					// Whitebalance Component
	WB_TEMPERATURE_AUTO,			// Whitebalance Temperature Auto
	WB_COMPONENT_AUTO,
	ROTATION,						// shawn 2011/05/25 add
	SNAPSHOT_CTRL,					// shawn 2011/05/27 add
	DENOISE,						// shawn 2011/05/31 add
};

enum {
	KSPROPERTY_EXTENSION_PROP_1 = 1,
	KSPROPERTY_EXTENSION_PROP_2,
	KSPROPERTY_EXTENSION_PROP_3,
	KSPROPERTY_EXTENSION_PROP_4,
	KSPROPERTY_EXTENSION_PROP_5,
	KSPROPERTY_EXTENSION_PROP_6,
	KSPROPERTY_EXTENSION_PROP_7,
	KSPROPERTY_EXTENSION_PROP_8
};

#define NUM_CAMERA_CONTROL_PROPERTY (NUMOFSTREAMPROPERTIES - NUM_VIDEO_AMP_PROPERTY)

// shawn 2011/05/25 +++++
#define XU_CONTROL6_BUFFER_LEN		7
#define XU_CONTROL6_NORMAL_WRITE	0x00			
#define	XU_CONTROL6_DUMMY_WRITE		0x01

//XU_SF_CONTROL_ID
#define SF_BUFFER_LEN				11
#define SF_DRIVER_PARAM_ADDR_L		0x00
#define SF_DRIVER_PARAM_ADDR_H		0x84
#define SF_213B_DRIVER_PARAM_ADDR_L	0x00
#define SF_213B_DRIVER_PARAM_ADDR_H	0xE0
//2008/12/16 03:07下午
#define SF_233_DRIVER_PARAM_ADDR_L	0x00
#define SF_233_DRIVER_PARAM_ADDR_H	0x55
#define SF_232_DRIVER_PARAM_ADDR_L	0x00	// shawn 2010/01/21 add
#define SF_232_DRIVER_PARAM_ADDR_H	0xC4	// shawn 2010/01/21 add
#define SF_DRIVER_PARAM_LEN_NOW		32//25  //2008/8/20 02:03下午
#define SF_CMD_NORMAL_WRITE			0x00			
#define	SF_CMD_DUMMY_WRITE			0x80
#define	SF_CMD_CHIP_ERASE			0xc0

//XU_AUXLED_CONTROL_ID
#define AUXLED_ADDR_L				0x00
#define AUXLED_ADDR_H				0x00
#define AUXLED_DATA_ATTRIBUTE		2
#define AUXLED_DATA_BRIGHT			3
#define	AUXLED_OFF					0x00
#define	AUXLED_ON					0x01
#define	AUXLED_AUTO					0x02

#define WBC_BLUE_ADDR_L				0x01
#define WBC_BLUE_ADDR_H				0x00
#define WBC_BLUE_DATA				2
#define WBC_RED_ADDR_L				0x02
#define WBC_RED_ADDR_H				0x00
#define WBC_RED_DATA				2
#define WBC_GREEN_ADDR_L			0x05	//2008/8/20 11:49上午
#define WBC_GREEN_ADDR_H			0x00
#define WBC_GREEN_DATA				2
#define WBC_AUTO_ADDR_L				0x03
#define WBC_AUTO_ADDR_H				0x00
#define WBC_AUTO_DATA				2
#define PRIVACY_ADDR_L				0x04
#define PRIVACY_ADDR_H				0x00
#define PRIVACY_DATA				2

#define	DIRECTION_OUT				0
#define	DIRECTION_IN				1

#define	EXUNIT_MAX_NUM				8
#define EXUNITCONTROL_MAX_BYTES		8

static const GUID PROPSETID_VIDCAP_EXTENSION_UNIT = 
{ 0x28f03370, 0x6311, 0x4a2e, { 0xba, 0x2c, 0x68, 0x90, 0xeb, 0x33, 0x40, 0x16 } };

typedef enum {
	AUXLED_DISABLE_ADDR	= 0,
	AUXLED_ATTRIBUTE_DEF_ADDR,
	AUXLED_BRIGHT_DEF_ADDR,
	AUXLED_BRIGHT_RES_ADDR,
	AUXLED_BRIGHT_MAX_ADDR,
	AUXLED_BRIGHT_MIN_ADDR,
	PRIVACY_DISABLE_ADDR,
	PRIVACY_DEF_ADDR,
	WBC_DISABLE_ADDR,
	WBC_BLUE_DEF_ADDR_H,
	WBC_BLUE_DEF_ADDR_L,
	WBC_RED_DEF_ADDR_H,
	WBC_RED_DEF_ADDR_L,
	WBC_BLUE_RES_ADDR,
	WBC_RED_RES_ADDR,
	WBC_BLUE_MAX_ADDR_H,
	WBC_BLUE_MAX_ADDR_L,
	WBC_RED_MAX_ADDR_H,
	WBC_RED_MAX_ADDR_L,
	WBC_BLUE_MIN_ADDR_H,
	WBC_BLUE_MIN_ADDR_L,
	WBC_RED_MIN_ADDR_H,
	WBC_RED_MIN_ADDR_L,		
	WBCAUTO_DISABLE_ADDR,
	WBCAUTO_DEF_ADDR,
	WBC_GREEN_DEF_ADDR_H,	//2008/8/20 11:46上午
	WBC_GREEN_DEF_ADDR_L,	
	WBC_GREEN_RES_ADDR,
	WBC_GREEN_MAX_ADDR_H,
	WBC_GREEN_MAX_ADDR_L,		
	WBC_GREEN_MIN_ADDR_H,
	WBC_GREEN_MIN_ADDR_L,		
	Data_0x8400_END
}Data_0x8400_ADDR;

typedef enum {
	XU_ASIC_CONTROL_ID	= 1,
	XU_I2C_CONTROL_ID,
	XU_SF_CONTROL_ID,
	XU_ROM_CONTROL_ID,
	XU_RUNTIME_VAR_CONTROL_ID,
	XU_AUXLED_CONTROL_ID,
	XU_END
}EXTENSION_UNIT_CONTROL_SELECTOR;

#define TEMPORAL_LEVEL_0		0
#define TEMPORAL_LEVEL_1		1
#define TEMPORAL_LEVEL_2		1
#define TEMPORAL_LEVEL_3		1
#define EDGEDETECT_LEVEL_0		0
#define EDGEDETECT_LEVEL_1		0
#define EDGEDETECT_LEVEL_2		0
#define EDGEDETECT_LEVEL_3		1 
#define STRGLTHD_LEVEL_1		13
#define STRGLTHD_LEVEL_2		15
#define STRGLTHD_LEVEL_3		1000	//2008/10/16 04:10下午
#define STRGCTHD_LEVEL_1		6
#define STRGCTHD_LEVEL_2		10
#define STRGCTHD_LEVEL_3		1000	//2008/10/16 04:10下午  
#define NZPWRLTHD_LEVEL_1		16
#define NZPWRLTHD_LEVEL_2		20
#define NZPWRLTHD_LEVEL_3		20		//2008/10/16 04:10下午
#define NZPWRCTHD_LEVEL_1		8
#define NZPWRCTHD_LEVEL_2		12
#define NZPWRCTHD_LEVEL_3		12		//2008/10/16 04:10下午 
#define MEANDIFFYTHD_LEVEL_1	5
#define MEANDIFFYTHD_LEVEL_2	8
#define MEANDIFFYTHD_LEVEL_3	8		//2008/10/16 04:10下午
#define MEANDIFFCTHD_LEVEL_1	3
#define MEANDIFFCTHD_LEVEL_2	5
#define MEANDIFFCTHD_LEVEL_3	5		//2008/10/16 04:11下午 
#define LOWLIGHTTHD_LEVEL_1		90
#define LOWLIGHTTHD_LEVEL_2		80
#define LOWLIGHTTHD_LEVEL_3		80		//2008/10/16 04:11下午 
// shawn 2011/05/25 -----

// Device extension
typedef struct DevExt {
	CVideoStream				*pVideoStream;
	CVideoDevice				*pVideoDevice;
	CStillPin					*pStillPin;

	FRAMERATEINFO				m_FrameRateInfo;

	USHORT						m_NumOfInterface;
	USHORT						m_StillCaptureMethod;
	DWORD						m_dwClockFrequence;
	ULONG						m_VideoImageFormatCount;
	ULONG						m_StillImageFromatCount;

	UCHAR						m_InputEntityID;
	UCHAR						m_PropertyEntityID;
	ULONG						m_InputControls;
	ULONG						m_PropertyControls;

	ULONG						m_YUY2_FormatCount;
	ULONG						m_YUY2_FrmaeRateCount;

	LONG						m_cRef;
	BOOLEAN						m_bDeviceIdle;
	DEVICE_POWER_STATE			DevicePowerState;
	KSSTATE						m_StreamState;
	//2010/6/29 05:47下午 H264
	KSSTATE						m_MPEG2StreamState;
	ULONG						m_C3Enable;
	ULONG						m_Autostart_Enable;	// shawn 2011/07/20 add
	ULONG						m_TSMUX_Enable;		// shawn 2011/07/21 add
	BOOLEAN						m_ImgSizeLarger;

	PROPERTYINFO				m_Flip;
	PROPERTYINFO				m_Mirror;
	PROPERTYINFO				m_BWMode;
	PROPERTYINFO				m_WBTAuto;
	PROPERTYINFO				m_WBCAuto;
	PROPERTYINFO				m_AEMode;
	PROPERTYINFO				m_AEPriority;
	PROPERTYINFO_WBC			m_WBC;

	// shawn 2011/05/25 +++++
	PROPERTYINFO				m_Rotation;
	PROPERTYINFO				m_AuxLedMode;
	PROPERTYINFO				m_Snapshot_Ctrl;
	PROPERTYINFO				m_DeNoiseMode;
	PROPERTYINFO_WBC			m_WBC_Green;
	BOOL						m_SetMirrorOrBW;
	BOOL						m_DeNoiseMode_UIEnable;
	BOOL						m_StillPin_Enable;			// shawn 2011/06/23 add

	BOOL						m_ResetAutoExposure;		// shawn 2011/06/29 add
	BOOL						m_ResetAutoFocus;			// shawn 2011/06/29 add
	BOOL						m_ResetAutoWhiteBalance;	// shawn 2011/06/29 add

	int							m_STREAM_Capture_MP2TS_Idx;	// shawn 2011/06/23 add
	int							m_STREAM_Still_Idx;			// shawn 2011/06/23 add

	int							m_StrgLThd;
	int							m_StrgCThd;
	int							m_NzPwrLThd;
	int							m_NzPwrCThd;	
	UInt32						m_LowLightThd;
	UInt32						m_MeanDiffYThd;
	UInt32						m_MeanDiffCThd;		
	BOOL						m_TemporalNR;
	BOOL						m_EdgeDtn;

	UCHAR						m_ChipID;
	
	UCHAR						m_DefXUID;

	UCHAR						m_0x8400_Data[SF_DRIVER_PARAM_LEN_NOW];
	UCHAR						m_XU_AuxLED_Dis;
	UCHAR						m_XU_WBC_Dis;
	UCHAR						m_XU_WBCAuto_Dis;
	UCHAR						m_XU_Privacy_Dis;
	UCHAR						m_XU_AuxLEDAuto_Dis;
	BOOL						m_is288P;

	int							m_DefectMode;
	int							m_iYThd;
	int							m_iCThd;
	// shawn 2011/05/25 -----

	UCHAR						m_DefaultFrameIndex[STREAM_Counts];
	UCHAR						m_VideoImageFrameNumber[STREAM_Counts];
//2010/8/2 03:25下午
	UCHAR						m_DefaultFrameIndex_Still;
	UCHAR						m_VideoImageFrameNumber_Still;	
//2010/3/17 05:51下午
	//ULONG						m_PropertyDefaultValues[NUMOFSTREAMPROPERTIES];
	LONG						m_PropertyDefaultValues[NUMOFSTREAMPROPERTIES];
	//2010/3/17 07:17下午
	LONG						m_PropertyCurrentValues[NUMOFSTREAMPROPERTIES];
	ULONG						m_PageCtrl;

	BOOLEAN						m_bStillStreamOpen;
	BOOLEAN						m_bReStartRun;
	PBYTE						m_pStillFrameBuffer;

	ULONG						m_dwFormatIndex[STREAM_Counts];
	VIDEOSTREAMINFO				StreamFormatArray[MAX_STREAM_FORMAT];
	VIDEOSTREAMINFO				StillFormatArray[MAX_STREAM_FORMAT];

	// add for Extension Unit
/*	UCHAR						m_ExUnitNum;
	UCHAR					    m_ExUnitControls[EXUNIT_MAX_NUM][EXUNITCONTROL_MAX_BYTES];
	GUID						m_ExUnitGUID[EXUNIT_MAX_NUM];
	USHORT						m_ExUnitID[EXUNIT_MAX_NUM];
	USHORT						m_ExUnitParamLength[EXUNIT_MAX_NUM][8*EXUNITCONTROL_MAX_BYTES];
*/
	UCHAR						m_ExUnitNum;
	ULONG					    m_ExUnitControls[8];
	GUID						m_ExUnitGUID[8];
	USHORT						m_ExUnitID[8];
	USHORT						m_ExUnitParamLength[8][8];
	
	ULONG						EventCount[KSEVENT_VIDCAPTOSTI_COUNT];
	//2010/7/28 11:21上午
	BOOLEAN						m_bMPEG2TSExist;
	
	//2011/5/18 10:37上午
	//WCHAR	m_DevInstanceKey[512];

	//2010/8/2 02:04下午
	/*UCHAR STREAM_Capture;                
	UCHAR STREAM_Capture_MP2TS; 
	UCHAR STREAM_Still;
	UCHAR STREAM_Counts; */

	// shawn 2011/06/21 for multi-devices +++++
	HW_STREAM_INFORMATION m_Streams[3];

	KSPROPERTY_STEPPING_LONG m_RangeAndStepProp[NUMOFSTREAMPROPERTIES];
	ULONG m_StreamProperties[NUMOFSTREAMPROPERTIES];

	KSPROPERTY_MEMBERSLIST m_BrightnessMembersList[2];
	KSPROPERTY_VALUES m_BrightnessValues;

	KSPROPERTY_MEMBERSLIST m_ContrastMembersList[2];
	KSPROPERTY_VALUES m_ContrastValues;

	KSPROPERTY_MEMBERSLIST m_HueMembersList[2];
	KSPROPERTY_VALUES m_HueValues;

	KSPROPERTY_MEMBERSLIST m_SaturationMembersList[2];
	KSPROPERTY_VALUES m_SaturationValues;

	KSPROPERTY_MEMBERSLIST m_SharpnessMembersList[2];
	KSPROPERTY_VALUES m_SharpnessValues;

	KSPROPERTY_MEMBERSLIST m_GammaMembersList[2];
	KSPROPERTY_VALUES m_GammaValues;

	KSPROPERTY_MEMBERSLIST m_WhitebalanceMembersList[2];
	KSPROPERTY_VALUES m_WhitebalanceValues;

	KSPROPERTY_MEMBERSLIST m_BacklightMembersList[2];
	KSPROPERTY_VALUES m_BacklightValues;

	KSPROPERTY_MEMBERSLIST m_PowerlineFrequencyMembersList[2];
	KSPROPERTY_VALUES m_PowerlineFrequencyValues;

	KSPROPERTY_MEMBERSLIST m_GainMembersList[2];
	KSPROPERTY_VALUES m_GainValues;

	KSPROPERTY_MEMBERSLIST m_ExposureMembersList[2];
	KSPROPERTY_VALUES m_ExposureValues;

	KSPROPERTY_MEMBERSLIST m_FocusMembersList[2];
	KSPROPERTY_VALUES m_FocusValues;

	KSPROPERTY_MEMBERSLIST m_IrisMembersList[2];
	KSPROPERTY_VALUES m_IrisValues;

	KSPROPERTY_MEMBERSLIST m_ZoomMembersList[2];
	KSPROPERTY_VALUES m_ZoomValues;

	KSPROPERTY_MEMBERSLIST m_PanMembersList[2];
	KSPROPERTY_VALUES m_PanValues;

	KSPROPERTY_MEMBERSLIST m_TiltMembersList[2];
	KSPROPERTY_VALUES m_TiltValues;

	KSPROPERTY_MEMBERSLIST m_RollMembersList[2];
	KSPROPERTY_VALUES m_RollValues;

	KSPROPERTY_MEMBERSLIST m_PrivacyMembersList[2];
	KSPROPERTY_VALUES m_PrivacyValues;

	KSPROPERTY_MEMBERSLIST m_AEPriorityMembersList[2];
	KSPROPERTY_VALUES m_AEPriorityValues;

	KSPROPERTY_ITEM m_VideoProcAmpProperties[NUM_VIDEO_AMP_PROPERTY];
	KSPROPERTY_ITEM m_CameraControlProperties[NUM_CAMERA_CONTROL_PROPERTY];

	KSPROPERTY_SET m_DevicePropertySetsTable[6+EXUNIT_MAX_NUM];

	KSAUTOMATION_TABLE m_FilterAutomationTable;

	KSFILTER_DESCRIPTOR m_CaptureFilterDescriptor;
	
	KSPIN_DESCRIPTOR_EX m_CaptureFilterPinDescriptors[3];

	KSPIN_DESCRIPTOR_EX m_STREAMCapturePinDescriptor;
	KSPIN_DESCRIPTOR_EX m_STREAMCaptureMP2TSPinDescriptor;
	KSPIN_DESCRIPTOR_EX m_STREAMStillPinDescriptor;

	KS_DATARANGE_VIDEO m_drImageFormat[MAX_STREAM_FORMAT];
	KS_DATARANGE_VIDEO m_drImageFormat_MP2TS[MAX_STREAM_FORMAT];
	KS_DATARANGE_VIDEO m_drStillImageFormat[MAX_STREAM_FORMAT];
	PKSDATARANGE m_StreamFormatsArray[MAX_STREAM_FORMAT];
	PKSDATARANGE m_StreamFormatsArray_MP2TS[MAX_STREAM_FORMAT];
	PKSDATAFORMAT m_StillStreamFormatsArray[MAX_STREAM_FORMAT];
	
	KSPROPERTY_ITEM m_ExtensionControlProperties[8][8];
	KSPROPERTY_SET m_ExUnitPropertySetsTable[8];
	
	KSAUTOMATION_TABLE m_NodeAutomationTable[5];
	KSNODE_DESCRIPTOR m_CaptureFilterNodeDescriptors[CAPTURE_FILTER_NODE_COUNT];
	// shawn 2011/06/21 for multi-devices -----
} DEVEXT, *PDEVEXT;
//2011/5/19 01:52下午
typedef struct _Interrupt_WORKITEM {
    PDEVEXT pDevExt;
//    BOOL Still_Done;
    WORK_QUEUE_ITEM WorkItem;
} Interrupt_WORKITEM, *PInterrupt_WORKITEM;
#endif //__DevDep_h__