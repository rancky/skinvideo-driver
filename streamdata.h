/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

	File:

		StreamData.h

	Abstract:


	History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/
#pragma once

#ifndef _StreamData_H_
#define _StreamData_H_

/**************************************************************************

    DISPATCH AND DESCRIPTOR LAYOUT

**************************************************************************/

#define MAX_NUM_STREAM_INFO 3//2
#define NUM_OF_POSSIBLE_STREAM_INSTANCES 1

#define FCC_FORMAT_YUV12A mmioFOURCC('I','4','2','0')

//--------------------------------------------------------------------------//
//							Define Image GUID								//
//--------------------------------------------------------------------------//
// e436eb7d-524f-11ce-9f53-0020af0ba770			MEDIASUBTYPE_RGB24
DEFINE_GUID(MEDIASUBTYPE_RGB24,
0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70);
// 32595559-0000-0010-8000-00AA00389B71			MEDIASUBTYPE_YUY2
DEFINE_GUID(MEDIASUBTYPE_YUY2,
0x32595559, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 47504A4D-0000-0010-8000-00AA00389B71			MEDIASUBTYPE_MJPG
DEFINE_GUID(MEDIASUBTYPE_MJPG,
0x47504A4D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
// 30323449-0000-0010-8000-00AA00389B71			MEDIASUBTYPE_I420
DEFINE_GUID(MEDIASUBTYPE_I420,
0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

/*************************************************

    Externed information

*************************************************/
//const PWSTR g_szUseVRAM = L"UseVRAM";

extern ULONG g_lValue;
extern LONG ImageFormatControl;
extern ULONG BUS_BW_ARRAY_SIZE;
extern MaxPktSizePerInterface BusBWArray[];
extern MaxPktSizePerInterface M2TS_BusBWArray[];
extern LONG StreamProperties[];

//
// define image output format
//

enum {RGB24, I420, YUY2, MJPEG};
//2010/3/29 03:02¤U¤È
///////////////////////// Video stream information //////////////////////
extern KS_DATARANGE_VIDEO drImageFormat[];
extern KS_DATARANGE_VIDEO drImageFormat_MP2TS[];
//extern KS_DATARANGE_MPEG2_VIDEO drImageFormat[];
///////////////////////// Video Still information //////////////////////
extern KS_DATARANGE_VIDEO drStillImageFormat[];

extern HW_STREAM_INFORMATION Streams[];
extern PKSDATARANGE StreamFormatsArray[];
extern PKSDATARANGE StreamFormatsArray_MP2TS[];
extern PKSDATAFORMAT StillStreamFormatsArray[];

extern KSPROPERTY_STEPPING_LONG RangeAndStepProp[];
//
// CapturePinDispatch:
//
// This is the dispatch table for the capture pin.  It provides notifications
// about creation, closure, processing, data formats, etc...
//
extern const KSPIN_DISPATCH CapturePinDispatch;
extern const KSPIN_DISPATCH StillPinDispatch;

//
// CapturePinAllocatorFraming:
//
// This is the simple framing structure for the capture pin.  Note that this
// will be modified via KsEdit when the actual capture format is determined.
//
extern const KSALLOCATOR_FRAMING_EX CapturePinAllocatorFraming;

// FilterAutomationTable
extern KSPROPERTY_SET DevicePropertySetsTable[];

extern KSPROPERTY_ITEM VideoProcAmpProperties[];
extern KSPROPERTY_ITEM CameraControlProperties[];
extern const KSPROPERTY_ITEM CameraControlProperty[];
extern const KSPROPERTY_ITEM VideoProcAmpProperty[];
extern const KSPROPERTY_ITEM FrameRateProperties[];
extern const KSPROPERTY_ITEM CustomProperties[];
extern const KSPROPERTY_ITEM MotorProperties[];
extern const KSPROPERTY_ITEM ClientProperties[];
extern const KSPROPERTY_ITEM ZoomProperties[];

// PinAutomationTable
extern const KSPROPERTY_SET VideoStreamPropertySetsTable[];
extern const KSPROPERTY_ITEM VideoStreamConnectionProperties[];
extern const KSPROPERTY_ITEM VideoStreamDroppedFramesProperties[];

extern const KSPROPERTY_SET CapturePinPropertySets[];
extern const KSPROPERTY_ITEM PinConnectionPropertyItems[];

extern KSPIN_MEDIUM StandardMedium;

//
// CaptureFilterDispatch:
//
// This is the dispatch table for the capture filter.  It provides notification
// of creation, closure, processing (for filter-centrics, not for the capture
// filter), and resets (for filter-centrics, not for the capture filter).
//
extern const KSFILTER_DISPATCH CaptureFilterDispatch;

// change not const for extension unit
#define DEFAULT_KSAUTOMATION_PROPERTIES 6
#define MAX_XU_KSAUTOMATION_PROPERTIES	8
extern KSAUTOMATION_TABLE FilterAutomationTable;
//
extern KSAUTOMATION_TABLE NodeAutomationTable;
//
// CaptureFilterDescription:
//
// The descriptor for the capture filter.  We don't specify any topology
// since there's only one pin on the filter.  Realistically, there would
// be some topological relationships here because there would be input 
// pins from crossbars and the like.
//
extern KSFILTER_DESCRIPTOR CaptureFilterDescriptor;

extern GUID g_PINNAME_VIDEO_CAPTURE;
extern GUID g_PINNAME_VIDEO_STILL;

//
// CaptureFilterCategories:
//
// The list of category GUIDs for the capture filter.
//
extern const GUID CaptureFilterCategories [CAPTURE_FILTER_CATEGORIES_COUNT];

extern const KSPIN_INTERFACE StandardPinInterfaces[];

extern const KSPIN_MEDIUM StandardPinMediums[];

//
// CaptureFilterPinDescriptors:
//
// The list of pin descriptors on the capture filter.  
//

// Note: We can add pin define into KSPIN_DESCRIPTOR_EX like STATIC_PINNAME_XXXX, etc.
extern KSPIN_DESCRIPTOR_EX CaptureFilterPinDescriptors [3];
//2010/7/28 04:13¤U¤È
extern KSPIN_DESCRIPTOR_EX STREAMCapturePinDescriptor;
extern KSPIN_DESCRIPTOR_EX STREAMCaptureMP2TSPinDescriptor;
extern KSPIN_DESCRIPTOR_EX STREAMStillPinDescriptor;

//2010/4/29 05:08¤U¤È
extern KSNODE_DESCRIPTOR CaptureFilterNodeDescriptors [CAPTURE_FILTER_NODE_COUNT];
extern KSPROPERTY_SET ExUnitPropertySetsTable[8];

extern KSEVENT_ITEM VIDCAPTOSTIItem[];
extern GUID SONIX_KSEVENTSETID_VIDCAPTOSTI;
extern KSEVENT_SET VIDCAPTOSTIEventSet[];
extern NTSTATUS __stdcall Sonix_DeviceEventProc(PHW_EVENT_DESCRIPTOR pEvent);
extern VOID Filter_NotifyStiMonitor(PDEVEXT pdx, PKSFILTER filter, ULONG EventId);
extern VOID Pin_NotifyStiMonitor(PDEVEXT pdx, PKSPIN pin, ULONG EventId);

// Extension Unit
extern const KSPROPERTY_ITEM ExtensionControlProperty[];
extern KSPROPERTY_ITEM ExtensionControlProperties[8][8];

extern KSAUTOMATION_TABLE PinAutomationTable;

// fb6c4281-0353-11d1-905f-0000c0cc16ba
DEFINE_GUID(PIN_CATEGORY_CAPTURE,
0xfb6c4281, 0x0353, 0x11d1, 0x90, 0x5f, 0x00, 0x00, 0xc0, 0xcc, 0x16, 0xba);
// fb6c428a-0353-11d1-905f-0000c0cc16ba
DEFINE_GUID(PIN_CATEGORY_STILL,
0xfb6c428a, 0x0353, 0x11d1, 0x90, 0x5f, 0x00, 0x00, 0xc0, 0xcc, 0x16, 0xba);

#endif	// _StreamData_H_