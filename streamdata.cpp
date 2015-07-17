/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

	File:

		StreamData.cpp

	Abstract:


	History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/

#include "SnCam.h"

LONG ImageFormatControl=0x7e03;
ULONG g_lValue=0;

LONG StreamProperties[NUMOFSTREAMPROPERTIES]={0};

KS_DATARANGE_VIDEO drImageFormat[MAX_STREAM_FORMAT]={0};
//2010/4/20 11:43上午
KS_DATARANGE_VIDEO drImageFormat_MP2TS[MAX_STREAM_FORMAT]={0};

KS_DATARANGE_VIDEO drStillImageFormat[MAX_STREAM_FORMAT]={0};
PKSDATARANGE StreamFormatsArray[MAX_STREAM_FORMAT]={0};
//2010/4/20 11:24上午
PKSDATARANGE StreamFormatsArray_MP2TS[MAX_STREAM_FORMAT]={0};
PKSDATAFORMAT StillStreamFormatsArray[MAX_STREAM_FORMAT] = {0};
KSPROPERTY_ITEM ExtensionControlProperties[8][8]={0};
KSPROPERTY_SET DevicePropertySets[8]={0};

ULONG BUS_BW_ARRAY_SIZE = 7;

MaxPktSizePerInterface BusBWArray[SONIX_BUS_BW_ARRAY_SIZE] = {
    {0, 0, 0},		// Interface # 0, Max. Pkt size 0, 1 transaction
    {1, 0x80, 1},	// Interface # 1, Max. Pkt size 0x100, 1 transaction
    {2, 0x100, 1},	// Interface # 2, Max. pkt size 0x200, 1 transaction
    {3, 0x320, 1},	// Interface # 3, Max. Pkt size 0x320, 1 transaction
    {4, 0x320, 2},	// Interface # 4, Max. Pkt size 0xB20, 2 transaction
    {5, 0x320, 3},	// Interface # 5, Max. pkt size 0x320, 3 transaction
    {6, 0x3E8, 3},	// Interface # 6, Max. Pkt size 0x3E8, 3 transaction
	NULL
};

//2011/4/26 05:30下午
MaxPktSizePerInterface M2TS_BusBWArray[SONIX_BUS_BW_ARRAY_SIZE] = {
    NULL
};

//
// CapturePinDispatch:
//
// This is the dispatch table for the capture pin.  It provides notifications
// about creation, closure, processing, data formats, etc...
//
const
KSPIN_DISPATCH
CapturePinDispatch = {
    CCapturePin::DispatchCreate,            // Pin Create
    CCapturePin::DispatchClose,				// Pin Close
    CCapturePin::DispatchProcess,           // Pin Process
    NULL,                                   // Pin Reset
    CCapturePin::DispatchSetFormat,         // Pin Set Data Format
    CCapturePin::DispatchSetState,          // Pin Set Device State
    NULL,									// Pin Connect
    NULL,								    // Pin Disconnect
    NULL,                                   // Clock Dispatch
    NULL                                    // Allocator Dispatch
};

//
// StillPinDispatch:
//
// This is the dispatch table for the Still pin.  It provides notifications
// about creation, closure, processing, data formats, etc...
//
const
KSPIN_DISPATCH
StillPinDispatch = {
    CStillPin::DispatchCreate,				// Pin Create
    CStillPin::DispatchClose,				// Pin Close
    CStillPin::DispatchProcess,		        // Pin Process
    NULL,                                   // Pin Reset
    CStillPin::DispatchSetFormat,		    // Pin Set Data Format
    CStillPin::DispatchSetState,			// Pin Set Device State
    NULL,									// Pin Connect
    NULL,								    // Pin Disconnect
    NULL,                                   // Clock Dispatch
    NULL                                    // Allocator Dispatch
};

//
// CapturePinAllocatorFraming:
//
// This is the simple framing structure for the capture pin.  Note that this
// will be modified via KsEdit when the actual capture format is determined.
//
DECLARE_SIMPLE_FRAMING_EX (
    CapturePinAllocatorFraming,
    STATICGUIDOF (KSMEMORY_TYPE_KERNEL_NONPAGED),
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY | KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    2,
    0,
    2 * PAGE_SIZE,
    2 * PAGE_SIZE
    );

//------------------------------------------------------------------------------------//
//------------------------------- Declare Property Set -------------------------------//
//------------------------------------------------------------------------------------//
KSPROPERTY_STEPPING_LONG RangeAndStepProp [] = 
{
	//
	//	VideoProcAmpProperties
	//
    {	// Brightness
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
    },
	{	// Contrast
        1,						// SteppingDelta (range / steps)
		0,						// Reserved
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
    },
    {	// Hue
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
    },
	{	// Saturation
        1,						// SteppingDelta (range / steps)
		0,						// Reserved
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
    },
    {	// Sharpness
		1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
    },
    {	// Gamma 
        1,						// SteppingDelta (range / steps)
        0,						// Reserved  
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
    },
    {	// Whitebalance
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
    },
	{	// Backlight
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
  	},
    {	// PowerlineFrequency
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
    },
    {	// Gain
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units        
		100						// Maximum in (IRE * 100) units
    },
	//
	//	CameraControlProperties
	//
	{	// Exposure
		1,						// SteppingDelta (range / steps)
		0,						// Reserved
		0,						// Minimum in (IRE * 100) units
		100						// Maximum in (IRE * 100) units
	},
	{	// FOCUS
		1,						// SteppingDelta (range / steps)
		0,						// Reserved
		0,						// Minimum in (IRE * 100) units
		100						// Maximum in (IRE * 100) units
	},
	{	// IRIS
		1,						// SteppingDelta (range / steps)
		0,						// Reserved
		0,						// Minimum in (IRE * 100) units
		100						// Maximum in (IRE * 100) units
	},
	{	// ZOOM
		1,						// SteppingDelta (range / steps)
		0,						// Reserved
		0,						// Minimum in (IRE * 100) units
		100						// Maximum in (IRE * 100) units
	},
	{	// PAN
		1,						// SteppingDelta (range / steps)
		0,						// Reserved
		0,						// Minimum in (IRE * 100) units
		100						// Maximum in (IRE * 100) units
	},
	{	// ROLL
		1,						// SteppingDelta (range / steps)
		0,						// Reserved
		0,						// Minimum in (IRE * 100) units
		100						// Maximum in (IRE * 100) units
	},
	{	// Privacy
		1,						// SteppingDelta (range / steps)
		0,						// Reserved
		0,						// Minimum in (IRE * 100) units
		100						// Maximum in (IRE * 100) units
	},
	//2010/12/1 06:03下午
	{	// TILT
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units
        100						// Maximum in (IRE * 100) units
	},
	{	// AEPriority
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units
        100						// Maximum in (IRE * 100) units
	},	
	//2011/1/5 05:34下午
	{	
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units
        100						// Maximum in (IRE * 100) units
	},	
    {	
        1,						// SteppingDelta (range / steps)
        0,						// Reserved
        0,						// Minimum in (IRE * 100) units
        100						// Maximum in (IRE * 100) units
	},	
};

KSPROPERTY_MEMBERSLIST BrightnessMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[BRIGHTNESS],
	},
	{
        {
            KSPROPERTY_MEMBER_VALUES,
			sizeof(StreamProperties[BRIGHTNESS]),	// Default value
            1,	// sizeof default value
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
		(PVOID) &StreamProperties[BRIGHTNESS],
    }    
};

KSPROPERTY_VALUES BrightnessValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (BrightnessMembersList),
    BrightnessMembersList
};

// Contrast
KSPROPERTY_MEMBERSLIST ContrastMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[CONTRAST],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
			sizeof(StreamProperties[CONTRAST]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[CONTRAST],
    }    
};

KSPROPERTY_VALUES ContrastValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ContrastMembersList),
    ContrastMembersList
};

// Hue
KSPROPERTY_MEMBERSLIST HueMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[HUE],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[HUE]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[HUE],
    }    
};

KSPROPERTY_VALUES HueValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (HueMembersList),
    HueMembersList
};

// Saturation
KSPROPERTY_MEMBERSLIST SaturationMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[SATURATION],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[SATURATION]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[SATURATION],
    }    
};

KSPROPERTY_VALUES SaturationValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (SaturationMembersList),
    SaturationMembersList
};

// Sharpness
KSPROPERTY_MEMBERSLIST SharpnessMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[SHARPNESS],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[SHARPNESS]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[SHARPNESS],
    }    
};

KSPROPERTY_VALUES SharpnessValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (SharpnessMembersList),
    SharpnessMembersList
};

// Gamma
KSPROPERTY_MEMBERSLIST GammaMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[GAMMA],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[GAMMA]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[GAMMA],
    }    
};

KSPROPERTY_VALUES GammaValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (GammaMembersList),
    GammaMembersList
};

// White balance
KSPROPERTY_MEMBERSLIST WhitebalanceMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[WHITEBALANCE],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[WHITEBALANCE]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[WHITEBALANCE],
    }    
};

KSPROPERTY_VALUES WhitebalanceValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (WhitebalanceMembersList),
    WhitebalanceMembersList
};

// Backlight
KSPROPERTY_MEMBERSLIST BacklightMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[BACKLIGHT],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[BACKLIGHT]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[BACKLIGHT],
    }    
};

KSPROPERTY_VALUES BacklightValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (BacklightMembersList),
    BacklightMembersList
};

//powerline frequency
KSPROPERTY_MEMBERSLIST PowerlineFrequencyMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[POWERLINEFREQUENCY],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[POWERLINEFREQUENCY]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[POWERLINEFREQUENCY],
    }    
};

KSPROPERTY_VALUES PowerlineFrequencyValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (PowerlineFrequencyMembersList),
    PowerlineFrequencyMembersList
};

//Gain
KSPROPERTY_MEMBERSLIST GainMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[GAIN],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[GAIN]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[GAIN],
    }    
};

KSPROPERTY_VALUES GainValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (GainMembersList),
    GainMembersList
};

// Exposure
KSPROPERTY_MEMBERSLIST ExposureMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[EXPOSURE],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[EXPOSURE]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[EXPOSURE],
    }    
};

KSPROPERTY_VALUES ExposureValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ExposureMembersList),
    ExposureMembersList
};

// FOCUS
KSPROPERTY_MEMBERSLIST FocusMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[FOCUS],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[FOCUS]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[FOCUS],
    }    
};

KSPROPERTY_VALUES FocusValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (FocusMembersList),
    FocusMembersList
};

// IRIS
KSPROPERTY_MEMBERSLIST IrisMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[IRIS],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[IRIS]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[IRIS],
    }    
};

KSPROPERTY_VALUES IrisValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (IrisMembersList),
    IrisMembersList
};

// Zoom
KSPROPERTY_MEMBERSLIST ZoomMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[ZOOM],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[ZOOM]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[ZOOM],
    }    
};

KSPROPERTY_VALUES ZoomValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ZoomMembersList),
    ZoomMembersList
};

// PAN
KSPROPERTY_MEMBERSLIST PanMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[PAN],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[PAN]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[PAN],
    }    
};

KSPROPERTY_VALUES PanValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (PanMembersList),
    PanMembersList
};

//2010/12/1 06:00下午
// TILT
KSPROPERTY_MEMBERSLIST TiltMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[TILT]
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[TILT]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[TILT],
    }    
};

KSPROPERTY_VALUES TiltValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (TiltMembersList),
    TiltMembersList
};

// ROLL
KSPROPERTY_MEMBERSLIST RollMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[ROLL],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[ROLL]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[ROLL],
    }    
};

KSPROPERTY_VALUES RollValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (RollMembersList),
    RollMembersList
};

// Privacy
KSPROPERTY_MEMBERSLIST PrivacyMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[PRIVACY],
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[PRIVACY]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[PRIVACY],
    }    
};

KSPROPERTY_VALUES PrivacyValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (PrivacyMembersList),
    PrivacyMembersList
};

//2010/12/1 06:01下午
// AEPriorityValues
KSPROPERTY_MEMBERSLIST AEPriorityMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (KSPROPERTY_STEPPING_LONG),
            1,
            0
        },
        (PVOID) &RangeAndStepProp[AEPRIORITY]
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof(StreamProperties[AEPRIORITY]),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &StreamProperties[AEPRIORITY],
    }    
};

KSPROPERTY_VALUES AEPriorityValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (AEPriorityMembersList),
    AEPriorityMembersList
};

DEFINE_KSPROPERTY_TABLE(FrameRateProperties)
{
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_CAPS,
        CCaptureFilter::GetProperty,							// GetSupported or Handler
        sizeof(PKSPROPERTY_VIDEOCONTROL_CAPS_S),				// MinProperty
        sizeof(PKSPROPERTY_VIDEOCONTROL_CAPS_S),				// MinData
        CCaptureFilter::SetProperty,							// SetSupported or Handler
        NULL,													// Values
        0,														// RelationsCount
        NULL,													// Relations
        NULL,													// SupportHandler
        0														// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_MODE,
        CCaptureFilter::GetProperty,							// GetSupported or Handler
        sizeof(PKSPROPERTY_VIDEOCONTROL_MODE_S),				// MinProperty
        sizeof(PKSPROPERTY_VIDEOCONTROL_MODE_S),				// MinData
        CCaptureFilter::SetProperty,							// SetSupported or Handler
        NULL,													// Values
        0,														// RelationsCount
        NULL,													// Relations
        NULL,													// SupportHandler
        0														// SerializedSize
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE,
        CCaptureFilter::GetProperty,							// GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S),	// MinProperty
        sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S),	// MinData
        FALSE,													// SetSupported or Handler
        NULL,													// Values
        0,														// RelationsCount
        NULL,													// Relations
        NULL,													// SupportHandler
        0														// SerializedSize
    ),
	
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_FRAME_RATES,
        CCaptureFilter::GetProperty,							// GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOCONTROL_FRAME_RATES_S),			// MinProperty
        0 ,														// MinData
        FALSE,													// SetSupported or Handler
        NULL,													// Values
        0,														// RelationsCount
        NULL,													// Relations
        NULL,													// SupportHandler
        0														// SerializedSize
    ),
};

DEFINE_KSPROPERTY_TABLE(VideoProcAmpProperty)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&BrightnessValues,						// Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_CONTRAST,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&ContrastValues,						// Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_HUE,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&HueValues,								// Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_SATURATION,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&SaturationValues,                      // Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_SHARPNESS,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&SharpnessValues,						// Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_GAMMA,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&GammaValues,							// Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&WhitebalanceValues,                    // Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&BacklightValues,						// Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&PowerlineFrequencyValues,				// Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOPROCAMP_GAIN,
		CCaptureFilter::GetProperty,            // GetSupported or Handler
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
		CCaptureFilter::SetProperty,            // SetSupported or Handler
		&GainValues,							// Values
		0,                                      // RelationsCount
		NULL,                                   // Relations
		NULL,                                   // SupportHandler
		sizeof(ULONG)                           // SerializedSize
	),
};

DEFINE_KSPROPERTY_TABLE(CameraControlProperty)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_EXPOSURE,
        CCaptureFilter::GetProperty,            // GetSupported or Handler
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinData
        CCaptureFilter::SetProperty,            // SetSupported or Handler
        &ExposureValues,						// Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_FOCUS,
        CCaptureFilter::GetProperty,            // GetSupported or Handler
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinData
        CCaptureFilter::SetProperty,            // SetSupported or Handler
        &FocusValues,							// Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_IRIS,
        CCaptureFilter::GetProperty,            // GetSupported or Handler
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinData
        CCaptureFilter::SetProperty,            // SetSupported or Handler
        &IrisValues,							// Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_ZOOM,
        CCaptureFilter::GetProperty,            // GetSupported or Handler
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinData
        CCaptureFilter::SetProperty,            // SetSupported or Handler
        &ZoomValues,							// Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_PAN,
        CCaptureFilter::GetProperty,            // GetSupported or Handler
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinData
        CCaptureFilter::SetProperty,            // SetSupported or Handler
        &PanValues,								// Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_ROLL,
        CCaptureFilter::GetProperty,            // GetSupported or Handler
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinData
        CCaptureFilter::SetProperty,            // SetSupported or Handler
        &RollValues,							// Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_PRIVACY,
        CCaptureFilter::GetProperty,            // GetSupported or Handler
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinData
        CCaptureFilter::SetProperty,            // SetSupported or Handler
        &PrivacyValues,							// Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    //2010/12/1 05:59下午
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_TILT,
        CCaptureFilter::GetProperty,            // GetSupported or Handler
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinData
        CCaptureFilter::SetProperty,			// SetSupported or Handler
        &TiltValues,							// Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY,
        CCaptureFilter::GetProperty,            // GetSupported or Handler
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinProperty
        sizeof(KSPROPERTY_CAMERACONTROL_S),     // MinData
        CCaptureFilter::SetProperty,			// SetSupported or Handler
        &AEPriorityValues,						// Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),  
};

DEFINE_KSPROPERTY_TABLE(CustomProperties)
{
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_EFFECT,						// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_FRAME_FILENAME,				// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_RESETSETTINGS,				// PropertyId
        NULL,												// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_SAVESETTINGS,				// PropertyId
        NULL,												// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_RESTORESETTINGS,				// PropertyId
        NULL,												// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_MIRROR,						// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_FLIP,						// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_BW_MODE,						// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_WBTemperatureControlAuto,	// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_WBComponentControl,			// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_WBComponentControlAuto,		// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_AEModeControl,				// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_CAPTURE_WIDTH,				// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_CAPTURE_HEIGHT,				// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_IMAGEFORMAT,					// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_SNAPSHOTIMAGESIZE,			// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_STILLMETHOD,					// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_AEPriorityControl,			// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),

	// shawn 2011/05/25 +++++
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_ROTATION,					// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ), 

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_SNAPSHOT_CONTROL,			// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ), 

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_AUXLED_MODE,					// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ), 

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_AUXLED_AUTO_CONTROL,			// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ), 

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_DENOISE_MODE,				// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ), 

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_WBComponentControl_Green,	// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        CCaptureFilter::SetProperty,						// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ), 
	// shawn 2011/05/25 -----

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_PROPERTY_PAGE_CONTROL,		// PropertyId
        CCaptureFilter::GetProperty,						// GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),					// MinData
        NULL,												// SetSupported or Handler
        &g_lValue,											// Values
        0,													// RelationsCount
        NULL,												// Relations
        NULL,												// SupportHandler
        sizeof(ULONG)										// SerializedSize
    ),
};


DEFINE_KSPROPERTY_TABLE(ClientProperties)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_CLIENT_PROP_ASICREAD,			// PropertyId
		CCaptureFilter::GetProperty,			    // GetSupported or Handler
		sizeof(KSPROPERTY_CLIENT_PROP_S),		    // MinProperty
		sizeof(KSPROPERTY_CLIENT_PROP_S),			// MinData
		NULL,										// SetSupported or Handler
		&g_lValue,									// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_CLIENT_PROP_ASICWRITE,			// PropertyId
		NULL,										// GetSupported or Handler
		sizeof(KSPROPERTY_CLIENT_PROP_S),		    // MinProperty
		sizeof(KSPROPERTY_CLIENT_PROP_S),			// MinData
		CCaptureFilter::SetProperty,		        // SetSupported or Handler
		&g_lValue,									// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	),
};

DEFINE_KSPROPERTY_TABLE(ZoomProperties)
{
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_LEFTDOWN,				// PropertyId
        NULL,									    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_LEFT,					// PropertyId
        NULL,									    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_LEFTUP,				// PropertyId
        NULL,									    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_UP,					// PropertyId
        NULL,										// GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_RIGHTUP,				// PropertyId
        NULL,										// GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_RIGHT,					// PropertyId
        NULL,										// GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_RIGHTDOWN,				// PropertyId
        NULL,										// GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_DOWN,					// PropertyId
        NULL,										// GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
	
	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_ZOOM_IN,				// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_ZOOM_PROP_ZOOM_IN_MAX,			// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        NULL,								        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM 
    (
        KSPROPERTY_ZOOM_PROP_STATUS,				// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        NULL,								        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM 
    (
        KSPROPERTY_ZOOM_PROP_ENABLE_OPTION,			// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
	DEFINE_KSPROPERTY_ITEM 
    (
        KSPROPERTY_ZOOM_PROP_WIDTH,					// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        NULL,										// SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM 
    (
        KSPROPERTY_ZOOM_PROP_UI_CONTROL,			// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        NULL,								        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM 
    (
        KSPROPERTY_ZOOM_PROP_PAN_MAX,				// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        FALSE,										// SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM 
    (
        KSPROPERTY_ZOOM_PROP_PAN_POS,				// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM 
    (
        KSPROPERTY_ZOOM_PROP_TILT_MAX,				// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        FALSE,										// SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM 
    (
        KSPROPERTY_ZOOM_PROP_TILT_POS,				// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        CCaptureFilter::SetProperty,		        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),

	DEFINE_KSPROPERTY_ITEM 
    (
        KSPROPERTY_ZOOM_PROP_MIRROR_FLIP,			// PropertyId
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinProperty
        sizeof(KSPROPERTY_ZOOM_PROP_S),				// MinData
        NULL,								        // SetSupported or Handler
        &g_lValue,									// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ),
};

// Extension Unit
DEFINE_KSPROPERTY_TABLE(ExtensionControlProperty)
{
	 /*DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_EXTENSION_PROP_1,				// PropertyId
        TRUE,										// GetSupported or Handler
        sizeof(KSPROPERTY),							// MinProperty
        0,											// MinData
        TRUE,										// SetSupported or Handler
        NULL,										// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        sizeof(ULONG)								// SerializedSize
    ), 
	 */
   DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_EXTENSION_PROP_1,				// PropertyId
		CCaptureFilter::GetProperty,				// GetSupported or Handler
		sizeof(KSPROPERTY),							// MinProperty
		0,											// MinData
		CCaptureFilter::SetProperty,				// SetSupported or Handler
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	),	

	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_EXTENSION_PROP_2,				// PropertyId
		CCaptureFilter::GetProperty,				// GetSupported or Handler
		sizeof(KSPROPERTY),							// MinProperty
		0,											// MinData
		CCaptureFilter::SetProperty,				// SetSupported or Handler
		NULL,//&g_lValue,							// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	), 

	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_EXTENSION_PROP_3,				// PropertyId
		CCaptureFilter::GetProperty,				// GetSupported or Handler
		sizeof(KSPROPERTY),							// MinProperty
		0,											// MinData
		CCaptureFilter::SetProperty,				// SetSupported or Handler
		NULL,//&g_lValue,							// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	), 

	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_EXTENSION_PROP_4,				// PropertyId
		CCaptureFilter::GetProperty,				// GetSupported or Handler
		sizeof(KSPROPERTY),							// MinProperty
		0,											// MinData
		CCaptureFilter::SetProperty,				// SetSupported or Handler
		NULL,//&g_lValue,							// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	), 

	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_EXTENSION_PROP_5,				// PropertyId
		CCaptureFilter::GetProperty,				// GetSupported or Handler
		sizeof(KSPROPERTY),							// MinProperty
		0,											// MinData
		CCaptureFilter::SetProperty,				// SetSupported or Handler
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	), 

	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_EXTENSION_PROP_6,				// PropertyId
		CCaptureFilter::GetProperty,				// GetSupported or Handler
		sizeof(KSPROPERTY),							// MinProperty
		0,											// MinData
		CCaptureFilter::SetProperty,				// SetSupported or Handler
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	), 

	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_EXTENSION_PROP_7,				// PropertyId
		CCaptureFilter::GetProperty,				// GetSupported or Handler
		sizeof(KSPROPERTY),							// MinProperty
		0,											// MinData
		CCaptureFilter::SetProperty,				// SetSupported or Handler
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	), 

	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_EXTENSION_PROP_8,				// PropertyId
		CCaptureFilter::GetProperty,				// GetSupported or Handler
		sizeof(KSPROPERTY),							// MinProperty
		0,											// MinData
		CCaptureFilter::SetProperty,				// SetSupported or Handler
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		NULL,										// SupportHandler
		sizeof(ULONG)								// SerializedSize
	), 

};

KSPROPERTY_ITEM VideoProcAmpProperties[NUM_VIDEO_AMP_PROPERTY]={0};
KSPROPERTY_ITEM CameraControlProperties[NUM_CAMERA_CONTROL_PROPERTY]={0};

KSPROPERTY_SET DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+MAX_XU_KSAUTOMATION_PROPERTIES] =
{
	DEFINE_KSPROPERTY_SET
	( 
		&PROPSETID_VIDCAP_CAMERACONTROL,			// Set
		SIZEOF_ARRAY(CameraControlProperties),		// PropertiesCount
		CameraControlProperties,					// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),

	DEFINE_KSPROPERTY_SET
	( 
		&PROPSETID_VIDCAP_VIDEOPROCAMP,				// Set
		SIZEOF_ARRAY(VideoProcAmpProperties),		// PropertiesCount
		VideoProcAmpProperties,						// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),

	DEFINE_KSPROPERTY_SET
	( 
		&PROPSETID_VIDCAP_VIDEOCONTROL,				// Set
		SIZEOF_ARRAY(FrameRateProperties),			// PropertiesCount
		FrameRateProperties,						// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),

	DEFINE_KSPROPERTY_SET
	( 
		&PROPSETID_CUSTOM_PROP_ST50220,				// Set
		SIZEOF_ARRAY(CustomProperties),				// PropertiesCount
		CustomProperties,							// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),

	DEFINE_KSPROPERTY_SET
	( 
		&PROPSETID_CLIENT_PROP_ST50220,				// Set
		SIZEOF_ARRAY(ClientProperties),				// PropertiesCount
		ClientProperties,							// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),

	DEFINE_KSPROPERTY_SET
	( 
		&PROPSETID_ZOOM_PROP_ST50220,				// Set
		SIZEOF_ARRAY(ZoomProperties),				// PropertiesCount
		ZoomProperties,								// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),
};

//
// CaptureFilterDispatch:
//
// This is the dispatch table for the capture filter.  It provides notification
// of creation, closure, processing (for filter-centrics, not for the capture
// filter), and resets (for filter-centrics, not for the capture filter).
//
const 
KSFILTER_DISPATCH
CaptureFilterDispatch = {
    CCaptureFilter::DispatchCreate,         // Filter Create
    CCaptureFilter::DispatchClose,          // Filter Close
    NULL,                                   // Filter Process
    NULL                                    // Filter Reset
};

// modify for not const
KSAUTOMATION_TABLE FilterAutomationTable = 
{	// change the number of properties for extension unit
	DEFAULT_KSAUTOMATION_PROPERTIES,	//ULONG PropertySetsCount;
	sizeof(KSPROPERTY_ITEM),			//ULONG PropertyItemSize;
	DevicePropertySetsTable,			//const KSPROPERTY_SET* PropertySets;
	0,									//ULONG MethodSetsCount;
	0,									//ULONG MethodItemSize;
	NULL,								//const KSMETHOD_SET* MethodSets;
	0,									//ULONG EventSetsCount;
	0,									//ULONG EventItemSize;
	NULL,								//const KSEVENT_SET* EventSets;
#if !defined(_WIN64)
	NULL								//PVOID Alignment;
#endif // !defined(_WIN64)
};

//2010/4/29 07:41下午
KSPROPERTY_SET ExUnitPropertySetsTable[8] =
{

};

// modify for not const
KSAUTOMATION_TABLE NodeAutomationTable = 
{	// change the number of properties for extension unit
	1,							//ULONG PropertySetsCount;
	sizeof(KSPROPERTY_ITEM),	//ULONG PropertyItemSize;
	ExUnitPropertySetsTable,	//DevicePropertySetsTable,//const KSPROPERTY_SET* PropertySets;
	0,							//ULONG MethodSetsCount;
	0,							//ULONG MethodItemSize;
	NULL,						//const KSMETHOD_SET* MethodSets;
	0,							//ULONG EventSetsCount;
	0,							//ULONG EventItemSize;
	NULL,						//const KSEVENT_SET* EventSets;
#if !defined(_WIN64)
	NULL						//PVOID Alignment;
#endif // !defined(_WIN64)
};
/*
DEFINE_KSAUTOMATION_TABLE (FilterAutomationTable)
{
    DEFINE_KSAUTOMATION_PROPERTIES (DevicePropertySetsTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};
*/
//
// CaptureFilterDescription:
//
// The descriptor for the capture filter.  We don't specify any topology
// since there's only one pin on the filter.  Realistically, there would
// be some topological relationships here because there would be input 
// pins from crossbars and the like.
//
KSFILTER_DESCRIPTOR 
CaptureFilterDescriptor = {
    &CaptureFilterDispatch,									// Dispatch Table
    &FilterAutomationTable,									// Automation Table add by Saxen 2006/7/24
    KSFILTER_DESCRIPTOR_VERSION,							// Version
    0,														// Flags
    &KSNAME_Filter,											// Reference GUID
    //2010/8/4 03:12下午
    //DEFINE_KSFILTER_PIN_DESCRIPTORS (CaptureFilterPinDescriptors[0]),	// KSFILTER_PIN_DESCRIPTORS
    2,														//PinDescriptorsCount 
    sizeof(CaptureFilterPinDescriptors[0]),					//PinDescriptorSize 
    CaptureFilterPinDescriptors,							//PinDescriptors 
    DEFINE_KSFILTER_CATEGORIES (CaptureFilterCategories),	// KSFILTER_CATEGORIES

	//2010/8/25 02:00下午
	//DEFINE_KSFILTER_NODE_DESCRIPTORS_NULL,
	DEFINE_KSFILTER_NODE_DESCRIPTORS(CaptureFilterNodeDescriptors),
										// NodeDescriptorsCount; in this case, 0
										// NodeDescriptorSize; in this case, 0
										// NodeDescriptors; in this case, NULL
	DEFINE_KSFILTER_DEFAULT_CONNECTIONS,
	// Automatically fills in the connections table for a filter which defines no explicit connections
										// ConnectionsCount; number of connections in the table
										// Connections; table of connections
	NULL								// ComponentId; in this case, no ID is provided
	};

GUID g_PINNAME_VIDEO_CAPTURE = {STATIC_PINNAME_VIDEO_CAPTURE};
GUID g_PINNAME_VIDEO_STILL = {STATIC_PINNAME_VIDEO_STILL};

//
// CaptureFilterCategories:
//
// The list of category GUIDs for the capture filter.
//
const
GUID CaptureFilterCategories [CAPTURE_FILTER_CATEGORIES_COUNT] = {
    STATICGUIDOF (KSCATEGORY_VIDEO),
    STATICGUIDOF (KSCATEGORY_CAPTURE)
};

// All of the property sets supported by video streams.
DEFINE_KSPROPERTY_TABLE(PinConnectionPropertyItems){
	DEFINE_KSPROPERTY_ITEM_CONNECTION_ALLOCATORFRAMING(
		CCaptureFilter::GetProperty)
};

DEFINE_KSPROPERTY_SET_TABLE(VideoStreamPropertySetsTable)
{
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_Connection,								// Set
        2,//SIZEOF_ARRAY(VideoStreamConnectionProperties),		// PropertiesCount
        VideoStreamConnectionProperties,						// PropertyItem
        0,														// FastIoCount
        NULL													// FastIoTable
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_DROPPEDFRAMES,						// Set
       	1,//SIZEOF_ARRAY(VideoStreamDroppedFramesProperties),	// PropertiesCount
        VideoStreamDroppedFramesProperties,						// PropertyItem
        0,														// FastIoCount
        NULL													// FastIoTable
    ),
};

/*DEFINE_KSAUTOMATION_TABLE(PinAutomationTable) {
    DEFINE_KSAUTOMATION_PROPERTIES(VideoStreamPropertySetsTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};*/

KSAUTOMATION_TABLE PinAutomationTable = {
	DEFINE_KSAUTOMATION_PROPERTIES(VideoStreamPropertySetsTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

DEFINE_KSPIN_INTERFACE_TABLE(StandardPinInterfaces) {
    {
        STATICGUIDOF(KSINTERFACESETID_Standard),
        KSINTERFACE_STANDARD_STREAMING,
        0
    }
};

DEFINE_KSPIN_MEDIUM_TABLE(StandardPinMediums) {
    {
        STATICGUIDOF(KSMEDIUMSETID_Standard),
        KSMEDIUM_TYPE_ANYINSTANCE,
        0
    }
};

//2010/7/28 02:13下午
KSPIN_DESCRIPTOR_EX
STREAMCapturePinDescriptor = 
	{
		&CapturePinDispatch,														// Dispatch Table
		&PinAutomationTable,														// Pin Automation Table
		{
			DEFINE_KSPIN_DEFAULT_INTERFACES,										// default interfaces
			DEFINE_KSPIN_DEFAULT_MEDIUMS,											// default mediums
			SIZEOF_ARRAY((PKSDATARANGE *)StreamFormatsArray),						// DataRangesCount
			(PKSDATARANGE *)StreamFormatsArray,										// DataRanges
			KSPIN_DATAFLOW_OUT,														// Dataflow
			KSPIN_COMMUNICATION_BOTH,												// Communication
			&PIN_CATEGORY_CAPTURE,													// Category
			&g_PINNAME_VIDEO_CAPTURE,												// Name
			0																		// Reserved
		},
		KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY | KSPIN_FLAG_ASYNCHRONOUS_PROCESSING,	// Flags
		1,																			// Instances Possible
		0,																			// Instances Necessary
		&CapturePinAllocatorFraming,												// Allocator Framing
		reinterpret_cast <PFNKSINTERSECTHANDLEREX> 
			(CCapturePin::IntersectHandler)
	};

KSPIN_DESCRIPTOR_EX
STREAMCaptureMP2TSPinDescriptor = 
	{
		&CapturePinDispatch,														// Dispatch Table
		&PinAutomationTable,														// Pin Automation Table
		{
			DEFINE_KSPIN_DEFAULT_INTERFACES,										// default interfaces
			DEFINE_KSPIN_DEFAULT_MEDIUMS,											// default mediums
			1,//SIZEOF_ARRAY((PKSDATARANGE *)StreamFormatsArray),					// DataRangesCount
			(PKSDATARANGE *)StreamFormatsArray_MP2TS,								// DataRanges
			KSPIN_DATAFLOW_OUT,														// Dataflow
			KSPIN_COMMUNICATION_BOTH,												// Communication
			&PIN_CATEGORY_CAPTURE,													// Category
			&g_PINNAME_VIDEO_CAPTURE,												// Name
			0																		// Reserved
		},
		KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY | KSPIN_FLAG_ASYNCHRONOUS_PROCESSING,	// Flags
		1,																			// Instances Possible
		0,																			// Instances Necessary
		&CapturePinAllocatorFraming,												// Allocator Framing
		reinterpret_cast <PFNKSINTERSECTHANDLEREX> 
			(CCapturePin::IntersectHandler)
	};

KSPIN_DESCRIPTOR_EX
STREAMStillPinDescriptor = 
	{
		&StillPinDispatch,															// Dispatch Table
		NULL,																		// NULL Automation Table
		{
			DEFINE_KSPIN_DEFAULT_INTERFACES,										// default interfaces
			DEFINE_KSPIN_DEFAULT_MEDIUMS,											// default mediums
			SIZEOF_ARRAY((PKSDATARANGE *)StillStreamFormatsArray),					// DataRangesCount
			(PKSDATARANGE *)StillStreamFormatsArray,								// DataRanges
			KSPIN_DATAFLOW_OUT,														// Dataflow
			KSPIN_COMMUNICATION_BOTH,												// Communication
			&PIN_CATEGORY_STILL,													// Category
			&g_PINNAME_VIDEO_STILL,													// Name
			0																		// Reserved
		},
		KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING | KSPIN_FLAG_ASYNCHRONOUS_PROCESSING | 
		KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY,										// Flags
		1,																			// Instances Possible
		0,																			// Instances Necessary
		&CapturePinAllocatorFraming,												// Allocator Framing
		reinterpret_cast <PFNKSINTERSECTHANDLEREX> 
			(CStillPin::IntersectHandler)
	};	
//
// CaptureFilterPinDescriptors:
//
// The list of pin descriptors on the capture filter.
//
// Note: We can add pin define into KSPIN_DESCRIPTOR_EX like STATIC_PINNAME_XXXX, etc.
KSPIN_DESCRIPTOR_EX
CaptureFilterPinDescriptors [3] = {
	0
};

//2010/4/29 05:09下午
KSNODE_DESCRIPTOR
CaptureFilterNodeDescriptors [CAPTURE_FILTER_NODE_COUNT] = {
	//Ex-Unit
	{
		&NodeAutomationTable,					// Pin Automation Table
		&KSNODETYPE_DEV_SPECIFIC,
		NULL,									//const GUID*  Name;
	},
};
// Property sets for all video capture streams.
DEFINE_KSPROPERTY_TABLE(VideoStreamConnectionProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CONNECTION_ALLOCATORFRAMING,
        CCaptureFilter::GetProperty,		    // GetSupported or Handler
        sizeof(KSALLOCATOR_FRAMING),            // MinProperty
        sizeof(KSALLOCATOR_FRAMING),            // MinData
        FALSE,                                  // SetSupported or Handler
        NULL,                                   // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    //2010/7/20 06:00下午 skype
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CONNECTION_ALLOCATORFRAMING_EX,
        CCaptureFilter::GetProperty,		    // GetSupported or Handler
        sizeof(KSALLOCATOR_FRAMING_EX),         // MinProperty
        sizeof(KSALLOCATOR_FRAMING_EX),         // MinData
        FALSE,                                  // SetSupported or Handler
        NULL,                                   // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),    
};

// Property set for the dropped frames of the video stream.
DEFINE_KSPROPERTY_TABLE(VideoStreamDroppedFramesProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_DROPPEDFRAMES_CURRENT,
        CCaptureFilter::GetProperty,			    // GetSupported or Handler
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S),	// MinProperty
        sizeof(KSPROPERTY_DROPPEDFRAMES_CURRENT_S),	// MinData
        FALSE,										// SetSupported or Handler
        NULL,										// Values
        0,											// RelationsCount
        NULL,										// Relations
        NULL,										// SupportHandler
        0											// SerializedSize
    ),
};

#define NUM_VIDEO_STREAM_PROPERTY_SET (SIZEOF_ARRAY(VideoStreamPropertySetsTable))
KSPIN_MEDIUM StandardMedium = {STATIC_KSMEDIUMSETID_Standard, 0, 0};

HW_STREAM_INFORMATION Streams [] =
{
	// -----------------------------------------------------------------
	// VideoStream
	// -----------------------------------------------------------------
	{
	// HW_STREAM_INFORMATION 
	NUM_OF_POSSIBLE_STREAM_INSTANCES,			// NumberOfPossibleInstances
	KSPIN_DATAFLOW_OUT,							// DataFlow
	TRUE,										// DataAccessible
	MAX_STREAM_FORMAT,							// NumberOfFormatArrayEntries
	StreamFormatsArray,							// StreamFormatsArray
	NULL,										// ClassReserved[0]
	NULL,										// ClassReserved[1]
	NULL,										// ClassReserved[2]
	NULL,										// ClassReserved[3]
	NUM_VIDEO_STREAM_PROPERTY_SET,				// NumStreamPropArrayEntries
	(PKSPROPERTY_SET) DevicePropertySetsTable,	// StreamPropertiesArray
	0,											// NumStreamEventArrayEntries;
	0,											// StreamEventsArray;
	(GUID *)&PINNAME_VIDEO_CAPTURE,				// Category;
	(GUID *)&PINNAME_VIDEO_CAPTURE,				// Name;
	1,											// MediumsCount
	&StandardMedium,							// Mediums
	FALSE,
	{0,0}										// BridgeStream
	},
	// -----------------------------------------------------------------
	// VideoStream MPEG2TS
	// -----------------------------------------------------------------
	{
	// HW_STREAM_INFORMATION 
	NUM_OF_POSSIBLE_STREAM_INSTANCES,			// NumberOfPossibleInstances
	KSPIN_DATAFLOW_OUT,							// DataFlow
	TRUE,										// DataAccessible
	MAX_STREAM_FORMAT,							// NumberOfFormatArrayEntries
	StreamFormatsArray_MP2TS,				    // StreamFormatsArray
	NULL,										// ClassReserved[0]
	NULL,										// ClassReserved[1]
	NULL,										// ClassReserved[2]
	NULL,										// ClassReserved[3]
	NUM_VIDEO_STREAM_PROPERTY_SET,				// NumStreamPropArrayEntries
	(PKSPROPERTY_SET) DevicePropertySetsTable,	// StreamPropertiesArray
	0,											// NumStreamEventArrayEntries;
	0,											// StreamEventsArray;
	(GUID *)&PINNAME_VIDEO_CAPTURE,				// Category;
	(GUID *)&PINNAME_VIDEO_CAPTURE,				// Name;
	1,											// MediumsCount
	&StandardMedium,							// Mediums
	FALSE,
	{0,0}										// BridgeStream
	},
	// -----------------------------------------------------------------
	// StillStream
	// -----------------------------------------------------------------
	{
	// HW_STREAM_INFORMATION -------------------------------------------
	NUM_OF_POSSIBLE_STREAM_INSTANCES,			// NumberOfPossibleInstances
	KSPIN_DATAFLOW_OUT,							// DataFlow
	TRUE,										// DataAccessible
	MAX_STREAM_FORMAT,							// NumberOfFormatArrayEntries
	StillStreamFormatsArray,					// StreamFormatsArray
	NULL,										// ClassReserved[0]
	NULL,										// ClassReserved[1]
	NULL,										// ClassReserved[2]
	NULL,										// ClassReserved[3]
	NUM_VIDEO_STREAM_PROPERTY_SET,				// NumStreamPropArrayEntries
	(PKSPROPERTY_SET) DevicePropertySetsTable,	// StreamPropertiesArray
	0,											// NumStreamEventArrayEntries;
	0,											// StreamEventsArray;
	(GUID *) &PINNAME_VIDEO_STILL,				// Category
	(GUID *) &PINNAME_VIDEO_STILL,				// Name
	0,											// MediumsCount
	&StandardMedium,							// Mediums
	FALSE,
	{0,0}										// BridgeStream 
	},
};

KSEVENT_ITEM VIDCAPTOSTIItem[] =
{
    {
        KSEVENT_VIDCAPTOSTI_EXT_TRIGGER,
        0,
        0,
        NULL,
        NULL,
        NULL
    },
    {
        KSEVENT_VIDCAPTOSTI_AUTO_LANUCH,
        0,
        0,
        NULL,
        NULL,
        NULL
    },
    {
        KSEVENT_VIDCAPTOSTI_OPEN_STREAM,
        0,
        0,
        NULL,
        NULL,
        NULL
    },
    {
        KSEVENT_VIDCAPTOSTI_CLOSE_STREAM,
        0,
        0,
        NULL,
        NULL,
        NULL
    },
    {
        KSEVENT_VIDCAPTOSTI_DEVICE_REMOVAL,
        0,
        0,
        NULL,
        NULL,
        NULL
    },
};

GUID SONIX_KSEVENTSETID_VIDCAPTOSTI = {STATIC_KSEVENTSETID_VIDCAPTOSTI};

KSEVENT_SET VIDCAPTOSTIEventSet[] =
{
    {
        &SONIX_KSEVENTSETID_VIDCAPTOSTI,
        SIZEOF_ARRAY(VIDCAPTOSTIItem),
        VIDCAPTOSTIItem,
    }
};

/*++

Routine Description:

    This routine will notify STI stack that a trigger button has been pressd

Arguments:



Return Value:

    NT status code

--*/
VOID Filter_NotifyStiMonitor(PDEVEXT pdx, PKSFILTER filter, ULONG EventId)
{
	ULONG Id = EventId - KSEVENT_VIDCAPTOSTI_BEGIN;
    if ((Id < KSEVENT_VIDCAPTOSTI_COUNT) && pdx->EventCount[Id])
    {
		DBGU_TRACE("call SONIX_NotifyStiMonitor Event ID = %d\n",EventId);
		KsFilterGenerateEvents(
			filter,
			&SONIX_KSEVENTSETID_VIDCAPTOSTI,
			EventId,
			0,
			NULL,
			NULL,
			NULL);
    }
}

VOID Pin_NotifyStiMonitor(PDEVEXT pdx, PKSPIN pin, ULONG EventId)
{
	ULONG Id = EventId - KSEVENT_VIDCAPTOSTI_BEGIN;
    if ((Id < KSEVENT_VIDCAPTOSTI_COUNT) && pdx->EventCount[Id])
    {
		DBGU_TRACE("call SONIX_NotifyStiMonitor Event ID = %d\n",EventId);
		KsPinGenerateEvents(
			pin,
			&SONIX_KSEVENTSETID_VIDCAPTOSTI,
			EventId,
			0,
			NULL,
			NULL,
			NULL);
    }
}

/*++

Routine Description:

    This routine will get called by stream class to enable/disable device events.

Arguments:



Return Value:

    NT status code

--*/

NTSTATUS STREAMAPI Sonix_DeviceEventProc (PHW_EVENT_DESCRIPTOR pEvent)
{
    PDEVEXT deviceExtension=(PDEVEXT)(pEvent->DeviceExtension);
	ULONG EventId = pEvent->EventEntry->EventItem->EventId;
	ULONG Id = EventId - KSEVENT_VIDCAPTOSTI_BEGIN;

    if (pEvent->Enable)
    {
	    if (Id < KSEVENT_VIDCAPTOSTI_COUNT)
			deviceExtension->EventCount[Id]++;
    }
    else
    {
	    if (Id < KSEVENT_VIDCAPTOSTI_COUNT)
			deviceExtension->EventCount[Id]--;
    }
    return STATUS_SUCCESS;
}
