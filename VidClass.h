/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

	File:

		VidClass.h

	Abstract:
		Define and declare video class items

	History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/

#pragma once

#ifndef   __VIDEO_CLASS_H__
#define   __VIDEO_CLASS_H__

//
// Video Class Sonix Structs.
//

#pragma pack(push, Old, 1)

// USB IAD Descriptor
typedef struct _USB_IAD_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x0B;
	UCHAR	bFirstInterface;
	UCHAR	bInterfaceCount;
	UCHAR	bFunctionClass;
	UCHAR	bFunctionSubClass;
	UCHAR	bFunctionProtocol;
	UCHAR	iFunction;
} USB_IAD_DESCRIPTOR, *PUSB_IAD_DESCRIPTOR;

// Video Class Control Interface Header Descriptor
typedef struct _Video_Class_Control_Interface_Header_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x01;
	USHORT	bcdVDC;
	USHORT	wTotalLength;
	DWORD	dwClockFrequency;
	UCHAR	bInCollection;
	UCHAR	baInterfaceNr[10];	// baInterfaceNr[bInCollection]
} VIDCLASS_INTERFACE_HEADER_DESCRIPTOR, *PVIDCLASS_INTERFACE_HEADER_DESCRIPTOR;

// Video Class Input Terminal Descriptor
typedef struct _Video_Class_Input_Terminal_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x02;
	UCHAR	bTerminalID;
	USHORT	wTerminalType;
	UCHAR	bAssocTerminal;
	UCHAR	iTerminal;
	USHORT	wObjectiveFocalLengthMin;
	USHORT	wObjectiveFocalLengthMax;
	USHORT	wOcularFocalLength;
	UCHAR	bControlSize;
	ULONG	bmControls;
} VIDCLASS_INPUT_TERMINAL_DESCRIPTOR, *PVIDCLASS_INPUT_TERMINAL_DESCRIPTOR;

// Video Class Output Terminal Descriptor
typedef struct _Video_Class_Output_Terminal_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x03;
	UCHAR	bTerminalID;
	USHORT	wTerminalType;
	UCHAR	bAssocTerminal;
	UCHAR	bSourceID;
	UCHAR	iTerminal;
} VIDCLASS_OUTPUT_TERMINAL_DESCRIPTOR, *PVIDCLASS_OUTPUT_TERMINAL_DESCRIPTOR;

// Video Class Processing Unit Descriptor
typedef struct _Video_Class_Processing_Unit_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x05;
	UCHAR	bUnitID;
	UCHAR	bSourceID;
	USHORT	wMaxMultiplier;
	UCHAR	bControlSize;
	USHORT	bmControls;
	UCHAR	iProcessing;
} VIDCLASS_PROCESSING_UNIT_DESCRIPTOR, *PVIDCLASS_PROCESSING_UNIT_DESCRIPTOR;

// Video Class Interrupt Endpoint Descriptor
typedef struct _Video_Class_Interrupt_Endpoint_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x25;
	UCHAR	bDescriptorSubtype;	// 0x03;
	USHORT	wMaxTransferSize;
} VIDCLASS_INTERRUPT_ENDPOINT_DESCRIPTOR, *PVIDCLASS_INTERRUPT_ENDPOINT_DESCRIPTOR;

// Video Class Video Input Header Descriptor
typedef struct _Video_Class_Video_Input_Header_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x01;
	UCHAR	bNumFormats;
	USHORT	wTotalLength;
	UCHAR	bEndpointAddress;
	UCHAR	bmInfo;
	UCHAR	bTerminalLink;
	UCHAR	bStillCaptureMethod;
	UCHAR	bTriggerUsage;
	UCHAR	bControlSize;
	UCHAR	bVideoFormat[10];	// bVideoFormat[bNumFormats]
} VIDCLASS_VIDEO_INPUT_HEADER_DESCRIPTOR, *PVIDCLASS_VIDEO_INPUT_HEADER_DESCRIPTOR;

// Video_Class_Uncompressed_Format_Type_Descriptor
typedef struct _Video_Class_Uncompressed_Format_Type_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x04;
	UCHAR	bFormatIndex;
	UCHAR	bNumFrameDescriptors;
	GUID	guidFormat;
	UCHAR	bBitPerPixel;
	UCHAR	bDefaultFrameIndex;
	UCHAR	bAspectRationX;
	UCHAR	bAspectRationY;
	UCHAR	bmInterfaceFlags;
	UCHAR	bCopyProtect;
} VIDCLASS_UNCOMPRESSED_FORMAT_DESCRIPTOR, *PVIDCLASS_UNCOMPRESSED_FORMAT_DESCRIPTOR;

// Video_Class_Uncompressed_Frame_Type_Descriptor
typedef struct _Video_Class_Uncompressed_Frame_Type_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x05;
	UCHAR	bFrameIndex;
	UCHAR	bCapabilities;
	USHORT	wWidth;
	USHORT	wHeight;
	DWORD	dwMinBitRate;
	DWORD	dwMaxBitRate;
	DWORD	dwMaxVideoFrameBufferSize;
	DWORD	DefaultFrameInterval;
	UCHAR	bFrameIntervalType;
	DWORD	dwFrameInterval[30];	//dwFrameInterval[bFrameIntervalType]
} VIDCLASS_UNCOMPRESSED_FRAME_DESCRIPTOR, *PVIDCLASS_UNCOMPRESSED_FRAME_DESCRIPTOR;

// Video_Class_Still_Image_Frame_Type_Descriptor
typedef struct _Video_Class_Still_Image_Frame_Type_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x03;
	UCHAR	bEndpointAddress;
	UCHAR	bNumImageSizePatterns;
	USHORT	pImageSizePattern[30];	// pImageSizePattern[bNumImageSizePatterns]
	UCHAR	bNumCompressionPattern;
	UCHAR	bCompression[30];		// bCompression[bNumCompressionPattern]
} VIDCLASS_STILL_FRAME_DESCRIPTOR, *PVIDCLASS_STILL_FRAME_DESCRIPTOR;

// Video_Class_Color_Matching_Descriptor
typedef struct _Video_Class_Color_Matching_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x0D;
	UCHAR	bColorPrimaries;
	UCHAR	bTransferCharacteristics;
	UCHAR	bMatrixCoefficients;
} VIDCLASS_COLOR_MATCHING_DESCRIPTOR, *PVIDCLASS_COLOR_MATCHING_DESCRIPTOR;

// Video_Class_MJPEG_Format_Type_Descriptor
typedef struct _Video_Class_MJPEG_Format_Type_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x06;
	UCHAR	bFormatIndex;
	UCHAR	bNumFrameDescriptors;
	UCHAR	bmFlags;
	UCHAR	bDefaultFrameIndex;
	UCHAR	bAspectRatioX;
	UCHAR	bAspectRatioY;
	UCHAR	bmInterfaceFlags;
	UCHAR	bCopyProtect;
} VIDCLASS_MJPEG_FORMAT_DESCRIPTOR, *PVIDCLASS_MJPEG_FORMAT_DESCRIPTOR;

// Video Class MJPEG Frame Type Descriptor
typedef struct _Video_Class_MJPEG_Frame_Type_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x07;
	UCHAR	bFrameIndex;
	UCHAR	bmCapabilities;
	USHORT	wWidth;
	USHORT	wHeight;
	DWORD	dwMinBitRate;
	DWORD	dwMaxBitRate;
	DWORD	dwMaxVideoFrameBufferSize;
	DWORD	dwDefaultFrameInterval;
	UCHAR	bFrameIntervalType;
	DWORD	dwFrameInterval[30];	//dwFrameInterval[bFrameIntervalType]
} VIDCLASS_MPEG_FRAME_DESCRIPTOR, *PVIDCLASS_MPEG_FRAME_DESCRIPTOR;


// james try M2TS.
// Video_Class_M2TS_Format_Type_Descriptor
typedef struct _Video_Class_M2TS_Format_Type_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x0A;
	UCHAR	bFormatIndex;
	UCHAR	bDataOffset;
	UCHAR	bPacketLength;
	UCHAR	bStrideLength;
} VIDCLASS_M2TS_FORMAT_DESCRIPTOR, *PVIDCLASS_M2TS_FORMAT_DESCRIPTOR;


// Video Probe and Commit Controls
typedef struct _Video_Probe_and_Commit_Controls {
	USHORT	bmHint;
	UCHAR	bFormatIdex;
	UCHAR	bFrameIndex;
	DWORD	dwFrameInterval;
	USHORT	wKeyFrameRate;
	USHORT	wPFrameRate;
	WORD	wCompQuality;
	WORD	wCompWindowSize;
	USHORT	wDelay;
	DWORD	dwMaxVideoFrameSize;
	DWORD	dwMaxPayloadTransferSize;
	/*DWORD	dwClockFrequency;
	UCHAR	bmFramingInfo;
	UCHAR	bmPreferedVersion;
	UCHAR	bMinVersion;
	UCHAR	bMaxVersion;*/
} VIDEO_PROBE_AND_COMMIT_CONTROL, *PVIDEO_PROBE_AND_COMMIT_CONTROL;

// Video Still Probe Control and Still Commit Control
typedef struct _Video_Still_Probe_and_Commit_Controls {
	UCHAR	bFormatIdex;
	UCHAR	bFrameIndex;
	UCHAR	bCompressionIndex;
	DWORD	dwMaxVideoFrameSize;
	DWORD	dwMaxPayloadTransferSize;
} VIDEO_STILL_PROBE_AND_COMMIT_CONTROL, *PVIDEO_STILL_PROBE_AND_COMMIT_CONTROL;

// Payload Header structure
typedef struct _FRAME_PAYLOAD_HEADER {
	BYTE bLength;
	BYTE bBFH;
	//2010/11/11 04:14¤U¤È
	UCHAR bPTS[4];
	//PUCHAR pbSCR;
} PAYLOAD_HEADER,*PPAYLOAD_HEADER;

// Video Class Extension Unit Descriptor
typedef struct _Video_Class_Extension_Unit_Descriptor {
	UCHAR	bLength;
	UCHAR	bDescriptorType;	// 0x24;
	UCHAR	bDescriptorSubtype;	// 0x06;
	UCHAR	bUnitID;
	GUID	guidExtensionCode;
	UCHAR	bNumControls;
	UCHAR	bNrInPins;
	UCHAR	baSourceID;
	UCHAR	bControlSize;
	//2010/4/30 02:20¤U¤È
	UCHAR	bmControls;//UCHAR bmControls[EXUNITCONTROL_MAX_BYTES];//UCHAR	bmControls;
	UCHAR	iExtension;
} VIDCLASS_EXTENSION_UNIT_DESCRIPTOR, *PVIDCLASS_EXTENSION_UNIT_DESCRIPTOR;

#pragma pack(pop, Old)

//
// Video Class Defines
//

// Video Interface Class Code
#define CC_VIDEO	0x0E
#define USB_VIDEO_CLASS_INPUT_HEADER_DESCRIPTOR_TYPE	0x24

// Video Interface Subclass Codes
enum {
	SC_UNDEFINED,					// 0x00
	SC_VIDEOCONTROL,				// 0x01
	SC_VIDEOSTREAMING,				// 0x02
	SC_VIDEO_INTERFACE_COLLECTION	// 0x03
};

// Video Class-Specific Descriptor Types
enum {
	CS_UNDEFINED = 0x20,
	CS_DEVICE,			// 0x21
	CS_CONFIGURATION,	// 0x22
	CS_STRING,			// 0x23
	CS_INTERFACE,		// 0x24
	CS_ENDPOINT			// 0x25
};

// VideoControl Interface Control Selectors
enum {
	VC_CONTROL_UNDEFINED,			// 0x00
	VC_VIDEO_POWER_MODE_CONTROL,	// 0x01
	VC_REQUEST_ERROR_CODE_CONTROL,	// 0x02
};

// Video Class-Specific Request Codes
enum {
	RC_UNDEFINED = 0x00,
	SET_CUR,	// 0x01
	GET_CUR = 0x81,
	GET_MIN,	// 0x82
	GET_MAX,	// 0x83
	GET_RES,	// 0x84
	GET_LEN,	// 0x85
	GET_INFO,	// 0x86
	GET_DEF		// 0x87
};

// VideoStreaming Interface Control Selectors
enum {
	VS_CONTROL_UNDEFINED,			// 0x00
	VS_PROBE_CONTROL,				// 0x01
	VS_COMMIT_CONTROL,				// 0x02
	VS_STILL_PROBE_CONTROL,			// 0x03
	VS_STILL_COMMIT_CONTROL,		// 0x04
	VS_STILL_IMAGE_TRIGGER_CONTROL,	// 0x05
	VS_STREAM_ERROR_CODE_CONTROL,	// 0x06
	VS_GENERATE_KEY_FRAME_CONTROL,	// 0x07
	VS_UPDATE_FRAME_SEGMENT_CONTROL,// 0x08
	VS_SYNCH_DELAY_CONTROL			// 0x09
};

// Camera Terminal Control Selectors
enum {
	CT_CONTROL_UNDEFINED,				//0x00
	CT_SCANNING_MODE_CONTROL,			//0x01
	CT_AE_MODE_CONTROL,					//0x02
	CT_AE_PRIORITY_CONTROL,				//0x03
	CT_EXPOSURE_TIME_ABSOLUTE_CONTROL,	//0x04
	CT_EXPOSURE_TIME_RELATIVE_CONTROL,	//0x05
	CT_FOCUS_ABSOLUTE_CONTROL,			//0x06
	CT_FOCUS_RELATIVE_CONTROL,			//0x07
	CT_FOCUS_AUTO_CONTROL,				//0x08
	CT_IRIS_ABSOLUTE_CONTROL,			//0x09
	CT_IRIS_RELATIVE_CONTROL,			//0x0A
	CT_ZOOM_ABSOLUTE_CONTROL,			//0x0B
	CT_ZOOM_RELATIVE_CONTROL,			//0x0C
	CT_PANTILT_ABSOLUTE_CONTROL,		//0x0D
	CT_PANTILT_RELATIVE_CONTROL,		//0x0E
	CT_ROLL_ABSOLUTE_CONTROL,			//0x0F
	CT_ROLL_RELATIVE_CONTROL,			//0x10
	CT_PRIVACY_CONTROL					//0x11
};

// Processing Unit Control Selectors
enum{
	PU_CONTROL_UNDEFINED,						//0x00
	PU_BACKLIGHT_COMPENSATION_CONTROL,			//0x01
	PU_BRIGHTNESS_CONTROL,						//0x02
	PU_CONTRAST_CONTROL,						//0x03
	PU_GAIN_CONTROL,							//0x04
	PU_POWER_LINE_FREQUENCY_CONTROL,			//0x05
	PU_HUE_CONTROL,								//0x06
	PU_SATURATION_CONTROL,						//0x07
	PU_SHARPNESS_CONTROL,						//0x08
	PU_GAMMA_CONTROL,							//0x09
	PU_WHITE_BALANCE_TEMPERATURE_CONTROL,		//0x0A
	PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL,	//0x0B
	PU_WHITE_BALANCE_COMPONENT_CONTROL,			//0x0C
	PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL,	//0x0D
	PU_DIGITAL_MULTIPLIER_CONTROL,				//0x0E
	PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL,		//0x0F
	PU_HUE_AUTO_CONTROL,						//0x10
	PU_ANALOG_VIDEO_STANDARD_CONTROL,			//0x11
	PU_ANALOG_LOCK_STATUS_CONTROL				//0x12
};

#endif	// __VIDEO_CLASS_H__