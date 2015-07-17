/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

	File:

		VideoStream.cpp

	Abstract:


	History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/

#include "SnCam.H"

static LONG InstanceIndex=0, InstanceCount=0;

CVideoStream::CVideoStream(IN PKSDEVICE Device)
: m_Device (Device), m_PictureNumber(0), m_DropCount(0)
{
	RtlZeroMemory(&m_VideoInfoHeader, sizeof(KS_VIDEOINFOHEADER));
	m_pVideoInfoHeader[0] = NULL;
	m_pVideoInfoHeader[1] = NULL;
	m_pVideoInfoHeader[2] = NULL;	// james try M2TS.
}

CVideoStream::~CVideoStream(void)
{
}

NTSTATUS CVideoStream::InitializeDevice()
{
    NTSTATUS ntStatus=STATUS_SUCCESS;
	PDEVEXT pDevExt = CurrentDevExt();

	if (!m_Device || !pDevExt) {
		DBGU_ERROR("Null m_Device or Device extension at InitializeDevice().\n");
		ntStatus = STATUS_INVALID_HANDLE;

		return ntStatus;
	}

	UNICODE_STRING	RegPath;

	// shawn 2011/06/21 for multi-devices +++++
	// VideoProcAmpProperties +++++
	pDevExt->m_RangeAndStepProp[0].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[0].Reserved = 0;
	pDevExt->m_RangeAndStepProp[0].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[0].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[1].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[1].Reserved = 0;
	pDevExt->m_RangeAndStepProp[1].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[1].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[2].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[2].Reserved = 0;
	pDevExt->m_RangeAndStepProp[2].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[2].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[3].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[3].Reserved = 0;
	pDevExt->m_RangeAndStepProp[3].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[3].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[4].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[4].Reserved = 0;
	pDevExt->m_RangeAndStepProp[4].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[4].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[5].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[5].Reserved = 0;
	pDevExt->m_RangeAndStepProp[5].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[5].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[6].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[6].Reserved = 0;
	pDevExt->m_RangeAndStepProp[6].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[6].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[7].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[7].Reserved = 0;
	pDevExt->m_RangeAndStepProp[7].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[7].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[8].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[8].Reserved = 0;
	pDevExt->m_RangeAndStepProp[8].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[8].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[9].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[9].Reserved = 0;
	pDevExt->m_RangeAndStepProp[9].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[9].Bounds.SignedMaximum = 100;

		// Brightness +++++
	pDevExt->m_BrightnessMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_BrightnessMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_BrightnessMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_BrightnessMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_BrightnessMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[BRIGHTNESS - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_BrightnessMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_BrightnessMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[BRIGHTNESS]);
	pDevExt->m_BrightnessMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_BrightnessMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_BrightnessMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[BRIGHTNESS];
	
	pDevExt->m_BrightnessValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_BrightnessValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_BrightnessValues.PropTypeSet.Flags = 0;
	pDevExt->m_BrightnessValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_BrightnessMembersList);
	pDevExt->m_BrightnessValues.MembersList = pDevExt->m_BrightnessMembersList;
		// Brightness -----

		// Contrast +++++
	pDevExt->m_ContrastMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_ContrastMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_ContrastMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_ContrastMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_ContrastMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[CONTRAST - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_ContrastMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_ContrastMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[CONTRAST]);
	pDevExt->m_ContrastMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_ContrastMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_ContrastMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[CONTRAST];
	
	pDevExt->m_ContrastValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_ContrastValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_ContrastValues.PropTypeSet.Flags = 0;
	pDevExt->m_ContrastValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_ContrastMembersList);
	pDevExt->m_ContrastValues.MembersList = pDevExt->m_ContrastMembersList;
		// Contrast -----

		// Hue +++++
	pDevExt->m_HueMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_HueMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_HueMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_HueMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_HueMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[HUE - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_HueMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_HueMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[HUE]);
	pDevExt->m_HueMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_HueMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_HueMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[HUE];
	
	pDevExt->m_HueValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_HueValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_HueValues.PropTypeSet.Flags = 0;
	pDevExt->m_HueValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_HueMembersList);
	pDevExt->m_HueValues.MembersList = pDevExt->m_HueMembersList;
		// Hue -----

		// Saturation +++++
	pDevExt->m_SaturationMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_SaturationMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_SaturationMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_SaturationMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_SaturationMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[SATURATION - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_SaturationMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_SaturationMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[SATURATION]);
	pDevExt->m_SaturationMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_SaturationMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_SaturationMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[SATURATION];
	
	pDevExt->m_SaturationValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_SaturationValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_SaturationValues.PropTypeSet.Flags = 0;
	pDevExt->m_SaturationValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_SaturationMembersList);
	pDevExt->m_SaturationValues.MembersList = pDevExt->m_SaturationMembersList;
		// Saturation -----

		// Sharpness +++++
	pDevExt->m_SharpnessMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_SharpnessMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_SharpnessMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_SharpnessMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_SharpnessMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[SHARPNESS - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_SharpnessMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_SharpnessMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[SHARPNESS]);
	pDevExt->m_SharpnessMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_SharpnessMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_SharpnessMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[SHARPNESS];
	
	pDevExt->m_SharpnessValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_SharpnessValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_SharpnessValues.PropTypeSet.Flags = 0;
	pDevExt->m_SharpnessValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_SharpnessMembersList);
	pDevExt->m_SharpnessValues.MembersList = pDevExt->m_SharpnessMembersList;
		// Sharpness -----

		// Gamma +++++
	pDevExt->m_GammaMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_GammaMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_GammaMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_GammaMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_GammaMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[GAMMA - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_GammaMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_GammaMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[GAMMA]);
	pDevExt->m_GammaMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_GammaMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_GammaMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[GAMMA];
	
	pDevExt->m_GammaValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_GammaValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_GammaValues.PropTypeSet.Flags = 0;
	pDevExt->m_GammaValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_GammaMembersList);
	pDevExt->m_GammaValues.MembersList = pDevExt->m_GammaMembersList;
		// Gamma -----

		// Whitebalance +++++
	pDevExt->m_WhitebalanceMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_WhitebalanceMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_WhitebalanceMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_WhitebalanceMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_WhitebalanceMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[WHITEBALANCE - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_WhitebalanceMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_WhitebalanceMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[WHITEBALANCE]);
	pDevExt->m_WhitebalanceMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_WhitebalanceMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_WhitebalanceMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[WHITEBALANCE];
	
	pDevExt->m_WhitebalanceValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_WhitebalanceValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_WhitebalanceValues.PropTypeSet.Flags = 0;
	pDevExt->m_WhitebalanceValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_WhitebalanceMembersList);
	pDevExt->m_WhitebalanceValues.MembersList = pDevExt->m_WhitebalanceMembersList;
		// Whitebalance -----

		// Backlight +++++
	pDevExt->m_BacklightMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_BacklightMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_BacklightMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_BacklightMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_BacklightMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[BACKLIGHT - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_BacklightMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_BacklightMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[BACKLIGHT]);
	pDevExt->m_BacklightMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_BacklightMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_BacklightMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[BACKLIGHT];
	
	pDevExt->m_BacklightValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_BacklightValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_BacklightValues.PropTypeSet.Flags = 0;
	pDevExt->m_BacklightValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_BacklightMembersList);
	pDevExt->m_BacklightValues.MembersList = pDevExt->m_BacklightMembersList;
		// Backlight -----

		// PowerlineFrequency +++++
	pDevExt->m_PowerlineFrequencyMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_PowerlineFrequencyMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_PowerlineFrequencyMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_PowerlineFrequencyMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_PowerlineFrequencyMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[POWERLINEFREQUENCY - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_PowerlineFrequencyMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_PowerlineFrequencyMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[POWERLINEFREQUENCY]);
	pDevExt->m_PowerlineFrequencyMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_PowerlineFrequencyMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_PowerlineFrequencyMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[POWERLINEFREQUENCY];
	
	pDevExt->m_PowerlineFrequencyValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_PowerlineFrequencyValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_PowerlineFrequencyValues.PropTypeSet.Flags = 0;
	pDevExt->m_PowerlineFrequencyValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_PowerlineFrequencyMembersList);
	pDevExt->m_PowerlineFrequencyValues.MembersList = pDevExt->m_PowerlineFrequencyMembersList;
		// PowerlineFrequency -----

		// Gain +++++
	pDevExt->m_GainMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_GainMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_GainMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_GainMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_GainMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[GAIN - PROPERTIESFORVIDEOPROCAMP];
	pDevExt->m_GainMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_GainMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[GAIN]);
	pDevExt->m_GainMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_GainMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_GainMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[GAIN];
	
	pDevExt->m_GainValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_GainValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_GainValues.PropTypeSet.Flags = 0;
	pDevExt->m_GainValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_GainMembersList);
	pDevExt->m_GainValues.MembersList = pDevExt->m_GainMembersList;
		// Gain -----
	// VideoProcAmpProperties -----

	// CameraControlProperties +++++
	pDevExt->m_RangeAndStepProp[10].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[10].Reserved = 0;
	pDevExt->m_RangeAndStepProp[10].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[10].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[11].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[11].Reserved = 0;
	pDevExt->m_RangeAndStepProp[11].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[11].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[12].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[12].Reserved = 0;
	pDevExt->m_RangeAndStepProp[12].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[12].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[13].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[13].Reserved = 0;
	pDevExt->m_RangeAndStepProp[13].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[13].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[14].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[14].Reserved = 0;
	pDevExt->m_RangeAndStepProp[14].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[14].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[15].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[15].Reserved = 0;
	pDevExt->m_RangeAndStepProp[15].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[15].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[16].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[16].Reserved = 0;
	pDevExt->m_RangeAndStepProp[16].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[16].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[17].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[17].Reserved = 0;
	pDevExt->m_RangeAndStepProp[17].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[17].Bounds.SignedMaximum = 100;

	pDevExt->m_RangeAndStepProp[18].SteppingDelta = 1;
	pDevExt->m_RangeAndStepProp[18].Reserved = 0;
	pDevExt->m_RangeAndStepProp[18].Bounds.SignedMinimum = 0;
	pDevExt->m_RangeAndStepProp[18].Bounds.SignedMaximum = 100;

		// Exposure +++++
	pDevExt->m_ExposureMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_ExposureMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_ExposureMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_ExposureMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_ExposureMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[EXPOSURE];
	pDevExt->m_ExposureMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_ExposureMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[EXPOSURE]);
	pDevExt->m_ExposureMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_ExposureMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_ExposureMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[EXPOSURE];
	
	pDevExt->m_ExposureValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_ExposureValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_ExposureValues.PropTypeSet.Flags = 0;
	pDevExt->m_ExposureValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_ExposureMembersList);
	pDevExt->m_ExposureValues.MembersList = pDevExt->m_ExposureMembersList;
		// Exposure -----

		// Focus +++++
	pDevExt->m_FocusMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_FocusMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_FocusMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_FocusMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_FocusMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[FOCUS];
	pDevExt->m_FocusMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_FocusMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[FOCUS]);
	pDevExt->m_FocusMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_FocusMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_FocusMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[FOCUS];
	
	pDevExt->m_FocusValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_FocusValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_FocusValues.PropTypeSet.Flags = 0;
	pDevExt->m_FocusValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_FocusMembersList);
	pDevExt->m_FocusValues.MembersList = pDevExt->m_FocusMembersList;
		// Focus -----

		// Iris +++++
	pDevExt->m_IrisMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_IrisMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_IrisMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_IrisMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_IrisMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[IRIS - 1];
	pDevExt->m_IrisMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_IrisMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[IRIS]);
	pDevExt->m_IrisMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_IrisMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_IrisMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[IRIS];
	
	pDevExt->m_IrisValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_IrisValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_IrisValues.PropTypeSet.Flags = 0;
	pDevExt->m_IrisValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_IrisMembersList);
	pDevExt->m_IrisValues.MembersList = pDevExt->m_IrisMembersList;
		// Iris -----

		// Zoom +++++
	pDevExt->m_ZoomMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_ZoomMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_ZoomMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_ZoomMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_ZoomMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[ZOOM - 1];
	pDevExt->m_ZoomMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_ZoomMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[ZOOM]);
	pDevExt->m_ZoomMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_ZoomMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_ZoomMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[ZOOM];
	
	pDevExt->m_ZoomValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_ZoomValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_ZoomValues.PropTypeSet.Flags = 0;
	pDevExt->m_ZoomValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_ZoomMembersList);
	pDevExt->m_ZoomValues.MembersList = pDevExt->m_ZoomMembersList;
		// Zoom -----

		// Pan +++++
	pDevExt->m_PanMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_PanMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_PanMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_PanMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_PanMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[PAN - 1];
	pDevExt->m_PanMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_PanMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[PAN]);
	pDevExt->m_PanMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_PanMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_PanMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[PAN];
	
	pDevExt->m_PanValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_PanValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_PanValues.PropTypeSet.Flags = 0;
	pDevExt->m_PanValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_PanMembersList);
	pDevExt->m_PanValues.MembersList = pDevExt->m_PanMembersList;
		// Pan -----

		// Tilt +++++
	pDevExt->m_TiltMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_TiltMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_TiltMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_TiltMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_TiltMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[TILT - 1];
	pDevExt->m_TiltMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_TiltMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[TILT]);
	pDevExt->m_TiltMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_TiltMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_TiltMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[TILT];
	
	pDevExt->m_TiltValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_TiltValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_TiltValues.PropTypeSet.Flags = 0;
	pDevExt->m_TiltValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_TiltMembersList);
	pDevExt->m_TiltValues.MembersList = pDevExt->m_TiltMembersList;
		// Tilt -----

		// Roll +++++
	pDevExt->m_RollMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_RollMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_RollMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_RollMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_RollMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[ROLL - 1];
	pDevExt->m_RollMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_RollMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[ROLL]);
	pDevExt->m_RollMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_RollMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_RollMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[ROLL];
	
	pDevExt->m_RollValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_RollValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_RollValues.PropTypeSet.Flags = 0;
	pDevExt->m_RollValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_RollMembersList);
	pDevExt->m_RollValues.MembersList = pDevExt->m_RollMembersList;
		// Roll -----

		// Privacy +++++
	pDevExt->m_PrivacyMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_PrivacyMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_PrivacyMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_PrivacyMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_PrivacyMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[PRIVACY - 1];
	pDevExt->m_PrivacyMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_PrivacyMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[PRIVACY]);
	pDevExt->m_PrivacyMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_PrivacyMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_PrivacyMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[PRIVACY];
	
	pDevExt->m_PrivacyValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_PrivacyValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_PrivacyValues.PropTypeSet.Flags = 0;
	pDevExt->m_PrivacyValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_PrivacyMembersList);
	pDevExt->m_PrivacyValues.MembersList = pDevExt->m_PrivacyMembersList;
		// Privacy -----

		// AEPriority +++++
	pDevExt->m_AEPriorityMembersList[0].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_STEPPEDRANGES;
	pDevExt->m_AEPriorityMembersList[0].MembersHeader.MembersSize = sizeof (KSPROPERTY_STEPPING_LONG);
	pDevExt->m_AEPriorityMembersList[0].MembersHeader.MembersCount = 1;
	pDevExt->m_AEPriorityMembersList[0].MembersHeader.Flags = 0;
	pDevExt->m_AEPriorityMembersList[0].Members = (PVOID)&pDevExt->m_RangeAndStepProp[AEPRIORITY - 2];
	pDevExt->m_AEPriorityMembersList[1].MembersHeader.MembersFlags = KSPROPERTY_MEMBER_VALUES;
	pDevExt->m_AEPriorityMembersList[1].MembersHeader.MembersSize = sizeof(pDevExt->m_StreamProperties[AEPRIORITY]);
	pDevExt->m_AEPriorityMembersList[1].MembersHeader.MembersCount = 1;
	pDevExt->m_AEPriorityMembersList[1].MembersHeader.Flags = KSPROPERTY_MEMBER_FLAG_DEFAULT;
	pDevExt->m_AEPriorityMembersList[1].Members = (PVOID)&pDevExt->m_StreamProperties[AEPRIORITY];
	
	pDevExt->m_AEPriorityValues.PropTypeSet.Set = KSPROPTYPESETID_General;
	pDevExt->m_AEPriorityValues.PropTypeSet.Id = VT_I4;
	pDevExt->m_AEPriorityValues.PropTypeSet.Flags = 0;
	pDevExt->m_AEPriorityValues.MembersListCount = SIZEOF_ARRAY(pDevExt->m_AEPriorityMembersList);
	pDevExt->m_AEPriorityValues.MembersList = pDevExt->m_AEPriorityMembersList;
		// AEPriority -----
	// CameraControlProperties -----

	// DevicePropertiesArray +++++
	pDevExt->m_DevicePropertySetsTable[0].Set = &PROPSETID_VIDCAP_CAMERACONTROL;
	pDevExt->m_DevicePropertySetsTable[0].PropertiesCount = SIZEOF_ARRAY(pDevExt->m_CameraControlProperties);
	pDevExt->m_DevicePropertySetsTable[0].PropertyItem = pDevExt->m_CameraControlProperties;
	pDevExt->m_DevicePropertySetsTable[0].FastIoCount = 0;
	pDevExt->m_DevicePropertySetsTable[0].FastIoTable = NULL;
	
	pDevExt->m_DevicePropertySetsTable[1].Set = &PROPSETID_VIDCAP_VIDEOPROCAMP;
	pDevExt->m_DevicePropertySetsTable[1].PropertiesCount = SIZEOF_ARRAY(pDevExt->m_VideoProcAmpProperties);
	pDevExt->m_DevicePropertySetsTable[1].PropertyItem = pDevExt->m_VideoProcAmpProperties;
	pDevExt->m_DevicePropertySetsTable[1].FastIoCount = 0;
	pDevExt->m_DevicePropertySetsTable[1].FastIoTable = NULL;

	pDevExt->m_DevicePropertySetsTable[2].Set = &PROPSETID_VIDCAP_VIDEOCONTROL;
	pDevExt->m_DevicePropertySetsTable[2].PropertiesCount = 4;
	pDevExt->m_DevicePropertySetsTable[2].PropertyItem = FrameRateProperties;
	pDevExt->m_DevicePropertySetsTable[2].FastIoCount = 0;
	pDevExt->m_DevicePropertySetsTable[2].FastIoTable = NULL;

	pDevExt->m_DevicePropertySetsTable[3].Set = &PROPSETID_CLIENT_PROP_ST50220;
	pDevExt->m_DevicePropertySetsTable[3].PropertiesCount = 2;
	pDevExt->m_DevicePropertySetsTable[3].PropertyItem = ClientProperties;
	pDevExt->m_DevicePropertySetsTable[3].FastIoCount = 0;
	pDevExt->m_DevicePropertySetsTable[3].FastIoTable = NULL;

	pDevExt->m_DevicePropertySetsTable[4].Set = &PROPSETID_CUSTOM_PROP_ST50220;
	pDevExt->m_DevicePropertySetsTable[4].PropertiesCount = 25;
	pDevExt->m_DevicePropertySetsTable[4].PropertyItem = CustomProperties;
	pDevExt->m_DevicePropertySetsTable[4].FastIoCount = 0;
	pDevExt->m_DevicePropertySetsTable[4].FastIoTable = NULL;

	pDevExt->m_DevicePropertySetsTable[5].Set = &PROPSETID_ZOOM_PROP_ST50220;
	pDevExt->m_DevicePropertySetsTable[5].PropertiesCount = 15;
	pDevExt->m_DevicePropertySetsTable[5].PropertyItem = ZoomProperties;
	pDevExt->m_DevicePropertySetsTable[5].FastIoCount = 0;
	pDevExt->m_DevicePropertySetsTable[5].FastIoTable = NULL;
	// DevicePropertiesArray -----

	// KSAUTOMATION_TABLE +++++
	pDevExt->m_FilterAutomationTable.PropertySetsCount = DEFAULT_KSAUTOMATION_PROPERTIES;
	pDevExt->m_FilterAutomationTable.PropertyItemSize = sizeof(KSPROPERTY_ITEM);
	pDevExt->m_FilterAutomationTable.PropertySets = pDevExt->m_DevicePropertySetsTable;
	pDevExt->m_FilterAutomationTable.MethodSetsCount = 0;
	pDevExt->m_FilterAutomationTable.MethodItemSize = 0;
	pDevExt->m_FilterAutomationTable.MethodSets = NULL;
	pDevExt->m_FilterAutomationTable.EventSetsCount = 0;
	pDevExt->m_FilterAutomationTable.EventItemSize = 0;
	pDevExt->m_FilterAutomationTable.EventSets = NULL;
#if !defined(_WIN64)
	pDevExt->m_FilterAutomationTable.Alignment = NULL;
#endif
	// KSAUTOMATION_TABLE -----

	// KSFILTER_DESCRIPTOR +++++
	pDevExt->m_CaptureFilterDescriptor.Dispatch = &CaptureFilterDispatch;
	pDevExt->m_CaptureFilterDescriptor.AutomationTable = &pDevExt->m_FilterAutomationTable;
	pDevExt->m_CaptureFilterDescriptor.Version = KSFILTER_DESCRIPTOR_VERSION;
	pDevExt->m_CaptureFilterDescriptor.Flags = 0;
	pDevExt->m_CaptureFilterDescriptor.ReferenceGuid = &KSNAME_Filter;
	pDevExt->m_CaptureFilterDescriptor.PinDescriptorsCount = 2;
	pDevExt->m_CaptureFilterDescriptor.PinDescriptorSize = sizeof(pDevExt->m_CaptureFilterPinDescriptors[0]);
	pDevExt->m_CaptureFilterDescriptor.PinDescriptors = pDevExt->m_CaptureFilterPinDescriptors;
	pDevExt->m_CaptureFilterDescriptor.CategoriesCount = 2;
	pDevExt->m_CaptureFilterDescriptor.Categories = CaptureFilterCategories;
	pDevExt->m_CaptureFilterDescriptor.NodeDescriptorsCount = 3;
	pDevExt->m_CaptureFilterDescriptor.NodeDescriptorSize = sizeof (KSNODE_DESCRIPTOR);
	pDevExt->m_CaptureFilterDescriptor.NodeDescriptors = pDevExt->m_CaptureFilterNodeDescriptors;
	pDevExt->m_CaptureFilterDescriptor.ConnectionsCount = 0;
	pDevExt->m_CaptureFilterDescriptor.Connections = NULL;
	pDevExt->m_CaptureFilterDescriptor.ComponentId = NULL;
	// KSFILTER_DESCRIPTOR -----

	// KSPIN_DESCRIPTOR_EX +++++
	pDevExt->m_STREAMCapturePinDescriptor.Dispatch = &CapturePinDispatch;
	pDevExt->m_STREAMCapturePinDescriptor.AutomationTable = &PinAutomationTable;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.InterfacesCount = 0;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.Interfaces = NULL;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.MediumsCount = 0;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.Mediums = NULL;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.DataRangesCount = SIZEOF_ARRAY((PKSDATARANGE *)pDevExt->m_StreamFormatsArray);
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.DataRanges = (PKSDATARANGE *)pDevExt->m_StreamFormatsArray;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.DataFlow = KSPIN_DATAFLOW_OUT;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.Communication = KSPIN_COMMUNICATION_BOTH;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.Category = &PIN_CATEGORY_CAPTURE;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor.Name = &g_PINNAME_VIDEO_CAPTURE;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor. ConstrainedDataRangesCount = 0;
	pDevExt->m_STREAMCapturePinDescriptor.PinDescriptor. ConstrainedDataRanges = NULL;
	pDevExt->m_STREAMCapturePinDescriptor.Flags = KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY | KSPIN_FLAG_ASYNCHRONOUS_PROCESSING;
	pDevExt->m_STREAMCapturePinDescriptor.InstancesPossible = 1;
	pDevExt->m_STREAMCapturePinDescriptor.InstancesNecessary = 0;
	pDevExt->m_STREAMCapturePinDescriptor.AllocatorFraming = &CapturePinAllocatorFraming;
	pDevExt->m_STREAMCapturePinDescriptor.IntersectHandler = reinterpret_cast <PFNKSINTERSECTHANDLEREX> (CCapturePin::IntersectHandler);

	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.Dispatch = &CapturePinDispatch;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.AutomationTable = &PinAutomationTable;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.InterfacesCount = 0;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.Interfaces = NULL;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.MediumsCount = 0;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.Mediums = NULL;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.DataRangesCount = 1;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.DataRanges = (PKSDATARANGE *)pDevExt->m_StreamFormatsArray_MP2TS;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.DataFlow = KSPIN_DATAFLOW_OUT;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.Communication = KSPIN_COMMUNICATION_BOTH;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.Category = &PIN_CATEGORY_CAPTURE;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.Name = &g_PINNAME_VIDEO_CAPTURE;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor. ConstrainedDataRangesCount = 0;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor. ConstrainedDataRanges = NULL;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.Flags = KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY | KSPIN_FLAG_ASYNCHRONOUS_PROCESSING;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.InstancesPossible = 1;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.InstancesNecessary = 0;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.AllocatorFraming = &CapturePinAllocatorFraming;
	pDevExt->m_STREAMCaptureMP2TSPinDescriptor.IntersectHandler = reinterpret_cast <PFNKSINTERSECTHANDLEREX> (CCapturePin::IntersectHandler);

	pDevExt->m_STREAMStillPinDescriptor.Dispatch = &StillPinDispatch;
	pDevExt->m_STREAMStillPinDescriptor.AutomationTable = NULL;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.InterfacesCount = 0;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.Interfaces = NULL;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.MediumsCount = 0;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.Mediums = NULL;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.DataRangesCount = SIZEOF_ARRAY((PKSDATARANGE *)pDevExt->m_StillStreamFormatsArray);
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.DataRanges = (PKSDATARANGE *)pDevExt->m_StillStreamFormatsArray;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.DataFlow = KSPIN_DATAFLOW_OUT;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.Communication = KSPIN_COMMUNICATION_BOTH;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.Category = &PIN_CATEGORY_STILL;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor.Name = &g_PINNAME_VIDEO_STILL;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor. ConstrainedDataRangesCount = 0;
	pDevExt->m_STREAMStillPinDescriptor.PinDescriptor. ConstrainedDataRanges = NULL;
	pDevExt->m_STREAMStillPinDescriptor.Flags = KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING | KSPIN_FLAG_ASYNCHRONOUS_PROCESSING | KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY;
	pDevExt->m_STREAMStillPinDescriptor.InstancesPossible = 1;
	pDevExt->m_STREAMStillPinDescriptor.InstancesNecessary = 0;
	pDevExt->m_STREAMStillPinDescriptor.AllocatorFraming = &CapturePinAllocatorFraming;
	pDevExt->m_STREAMStillPinDescriptor.IntersectHandler = reinterpret_cast <PFNKSINTERSECTHANDLEREX> (CCapturePin::IntersectHandler);
	// KSPIN_DESCRIPTOR_EX -----

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
		pDevExt->m_StreamFormatsArray,				// StreamFormatsArray
		NULL,										// ClassReserved[0]
		NULL,										// ClassReserved[1]
		NULL,										// ClassReserved[2]
		NULL,										// ClassReserved[3]
		/*NUM_VIDEO_STREAM_PROPERTY_SET*/2,				// NumStreamPropArrayEntries
		(PKSPROPERTY_SET) VideoStreamPropertySetsTable,	// StreamPropertiesArray
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
		pDevExt->m_StreamFormatsArray_MP2TS,	    // StreamFormatsArray
		NULL,										// ClassReserved[0]
		NULL,										// ClassReserved[1]
		NULL,										// ClassReserved[2]
		NULL,										// ClassReserved[3]
		/*NUM_VIDEO_STREAM_PROPERTY_SET*/2,				// NumStreamPropArrayEntries
		(PKSPROPERTY_SET) VideoStreamPropertySetsTable,	// StreamPropertiesArray
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
		pDevExt->m_StillStreamFormatsArray,			// StreamFormatsArray
		NULL,										// ClassReserved[0]
		NULL,										// ClassReserved[1]
		NULL,										// ClassReserved[2]
		NULL,										// ClassReserved[3]
		/*NUM_VIDEO_STREAM_PROPERTY_SET*/2,				// NumStreamPropArrayEntries
		(PKSPROPERTY_SET) VideoStreamPropertySetsTable,	// StreamPropertiesArray
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

	RtlCopyMemory(pDevExt->m_Streams, Streams, sizeof(Streams));

	// KSAUTOMATION_TABLE +++++
	pDevExt->m_NodeAutomationTable[0].PropertySetsCount = 1;
	pDevExt->m_NodeAutomationTable[0].PropertyItemSize = sizeof(KSPROPERTY_ITEM);
	pDevExt->m_NodeAutomationTable[0].PropertySets = &pDevExt->m_DevicePropertySetsTable[0];
	pDevExt->m_NodeAutomationTable[0].MethodSetsCount = 0;
	pDevExt->m_NodeAutomationTable[0].MethodItemSize = 0;
	pDevExt->m_NodeAutomationTable[0].MethodSets = NULL;
	pDevExt->m_NodeAutomationTable[0].EventSetsCount = 0;
	pDevExt->m_NodeAutomationTable[0].EventItemSize = 0;
	pDevExt->m_NodeAutomationTable[0].EventSets = NULL;
#if !defined(_WIN64)
	pDevExt->m_NodeAutomationTable[0].Alignment = NULL;
#endif
	
	pDevExt->m_NodeAutomationTable[1].PropertySetsCount = 1;
	pDevExt->m_NodeAutomationTable[1].PropertyItemSize = sizeof(KSPROPERTY_ITEM);
	pDevExt->m_NodeAutomationTable[1].PropertySets = &pDevExt->m_ExUnitPropertySetsTable[0];
	pDevExt->m_NodeAutomationTable[1].MethodSetsCount = 0;
	pDevExt->m_NodeAutomationTable[1].MethodItemSize = 0;
	pDevExt->m_NodeAutomationTable[1].MethodSets = NULL;
	pDevExt->m_NodeAutomationTable[1].EventSetsCount = 0;
	pDevExt->m_NodeAutomationTable[1].EventItemSize = 0;
	pDevExt->m_NodeAutomationTable[1].EventSets = NULL;
#if !defined(_WIN64)
	pDevExt->m_NodeAutomationTable[1].Alignment = NULL;
#endif
	pDevExt->m_NodeAutomationTable[2].PropertySetsCount = 1;
	pDevExt->m_NodeAutomationTable[2].PropertyItemSize = sizeof(KSPROPERTY_ITEM);
	pDevExt->m_NodeAutomationTable[2].PropertySets = &pDevExt->m_ExUnitPropertySetsTable[0];
	pDevExt->m_NodeAutomationTable[2].MethodSetsCount = 0;
	pDevExt->m_NodeAutomationTable[2].MethodItemSize = 0;
	pDevExt->m_NodeAutomationTable[2].MethodSets = NULL;
	pDevExt->m_NodeAutomationTable[2].EventSetsCount = 0;
	pDevExt->m_NodeAutomationTable[2].EventItemSize = 0;
	pDevExt->m_NodeAutomationTable[2].EventSets = NULL;
#if !defined(_WIN64)
	pDevExt->m_NodeAutomationTable[2].Alignment = NULL;
#endif
	// KSAUTOMATION_TABLE -----

	// KSNODE_DESCRIPTOR +++++
	//RBK , add extension unit control set to NODE 1 AND NODE 2 (comply with uvc)
	pDevExt->m_CaptureFilterNodeDescriptors[0].AutomationTable = &pDevExt->m_NodeAutomationTable[0];
	pDevExt->m_CaptureFilterNodeDescriptors[0].Type = &KSNODETYPE_VIDEO_STREAMING;
	pDevExt->m_CaptureFilterNodeDescriptors[0].Name = NULL;

	pDevExt->m_CaptureFilterNodeDescriptors[1].AutomationTable = &pDevExt->m_NodeAutomationTable[1];
	pDevExt->m_CaptureFilterNodeDescriptors[1].Type = &KSNODETYPE_DEV_SPECIFIC;
	pDevExt->m_CaptureFilterNodeDescriptors[1].Name = NULL;

	pDevExt->m_CaptureFilterNodeDescriptors[2].AutomationTable = &pDevExt->m_NodeAutomationTable[2];
	pDevExt->m_CaptureFilterNodeDescriptors[2].Type = &KSNODETYPE_DEV_SPECIFIC;
	pDevExt->m_CaptureFilterNodeDescriptors[2].Name = NULL;

	// KSNODE_DESCRIPTOR -----
	// shawn 2011/06/21 for multi-devices -----

	pDevExt->pVideoDevice = new (NonPagedPool) CVideoDevice();

	if(pDevExt->pVideoDevice == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;

	ntStatus = pDevExt->pVideoDevice->InitializeDevice(
		pDevExt, m_Device->NextDeviceObject, m_Device->PhysicalDeviceObject);

	if(NT_SUCCESS(ntStatus))
	{
		// shawn 2011/05/25 +++++
		UCHAR SF_driver_param_addrH = SF_DRIVER_PARAM_ADDR_H;
		UCHAR SF_driver_param_addrL = SF_DRIVER_PARAM_ADDR_L;
		int i = 0;
		int j = 0;

		pDevExt->m_is288P = FALSE;

		if(pDevExt->m_ChipID == 0x22)
		{
			ULONG BufferLen;
			UCHAR Request;
			UCHAR ControlRW;
			UCHAR pData[8];
			NTSTATUS ntStatus = STATUS_SUCCESS;
			
			DbgPrint("pDevExt->m_ChipID == 0x22\n");
			
			BufferLen = 8;
			Request = 0x4;
			ControlRW = 0x01;
			
			ntStatus = pDevExt->pVideoDevice->UsbControlVendorTransfer(
						URB_FUNCTION_VENDOR_INTERFACE,	//Function
						ControlRW,						//Read/Write(write=false,read=true),
						Request,						// Request
						0x9ff8,							// Value
						0x0C45,							// Index
						&pData,							// buffer
						BufferLen						// buffer length
						);

			if(NT_SUCCESS(ntStatus))
			{
				for(i=0;i<5;i++)
					DbgPrint("pData[%d]=%c\n",i,pData[i]);
				
				DbgPrint("pData[5]=%x\n",pData[5]);	
			  
				if((pData[0] == '2')&&(pData[1] == '2')&&(pData[2] == '0'))
				{
					SF_driver_param_addrH = SF_213B_DRIVER_PARAM_ADDR_H;
					SF_driver_param_addrL = SF_213B_DRIVER_PARAM_ADDR_L;
					DbgPrint("This is 213B or 213MR chip\n");
				}
				
				//2008/5/7 02:29下午
				if((pData[0] == '2')&&(pData[1] == '2')&&(pData[2] == '5'))
				{
					pDevExt->m_is288P = TRUE;
				}
			}
		}

		if(pDevExt->m_ChipID == 0x33)
		{
			SF_driver_param_addrH = SF_233_DRIVER_PARAM_ADDR_H;
			SF_driver_param_addrL = SF_233_DRIVER_PARAM_ADDR_L;
		}
		else if (pDevExt->m_ChipID == 0x75 || 
				 pDevExt->m_ChipID == 0x76 || 
				 pDevExt->m_ChipID == 0x16 ||
				 pDevExt->m_ChipID == 0x90 )
		{
			SF_driver_param_addrH = SF_232_DRIVER_PARAM_ADDR_H;
			SF_driver_param_addrL = SF_232_DRIVER_PARAM_ADDR_L;
		}
		else if (pDevExt->m_ChipID == 0x32 || 
				 pDevExt->m_ChipID == 0x86 || 
				 pDevExt->m_ChipID == 0x88 ||
				 pDevExt->m_ChipID == 0x89)	// shawn 2011/08/22 for 289
		{
			unsigned short wAddr = 0;
			unsigned char abyTmpBuf[4] = {0};
			BOOL bIsCompactMode = TRUE;

			pDevExt->pVideoDevice->GetSectorTableAddr(&wAddr);
			
			if (pDevExt->m_ChipID == 0x32)
			{
				ntStatus = pDevExt->pVideoDevice->ASIC_Communication(CTL_R_NONE, 4, wAddr, abyTmpBuf);

				if(ntStatus != STATUS_SUCCESS)
				{
					DBGU_TRACE("ASIC_Communication fail 1 %x", ntStatus);
				}

				if (abyTmpBuf[0] != 0x52 || 	// 'R'
					abyTmpBuf[1] != 0x53 ||		// 'S'
					abyTmpBuf[2] != 0x65 ||		// 'e'
					abyTmpBuf[3] != 0x63 )		// 'c'
					bIsCompactMode = FALSE;
			}
			
			if (bIsCompactMode)
			{
				ntStatus = pDevExt->pVideoDevice->ASIC_Communication(CTL_R_NONE, 1, wAddr+8, abyTmpBuf);

				if(ntStatus != STATUS_SUCCESS)
				{
					DBGU_TRACE("ASIC_Communication fail 2 %x", ntStatus);
				}

				wAddr = (abyTmpBuf[0] * 256) + 0x400;
				SF_driver_param_addrH = (UCHAR)(wAddr >> 8);
				SF_driver_param_addrL = (UCHAR)(wAddr & 0xff);
			}
			else
			{
				SF_driver_param_addrH = SF_232_DRIVER_PARAM_ADDR_H;
				SF_driver_param_addrL = SF_232_DRIVER_PARAM_ADDR_L;
			}
		}

		pDevExt->m_DefXUID = 0;

		for (i = 0; i < EXUNIT_MAX_NUM; i++)
		{
			if (IsEqualGUID(pDevExt->m_ExUnitGUID[i], PROPSETID_VIDCAP_EXTENSION_UNIT))
			{
				pDevExt->m_DefXUID = i;
				DbgPrint("m_DefXUID = %d\n", pDevExt->m_DefXUID);
				break;
			}
		}

		UCHAR GetBuffer[SF_BUFFER_LEN];

		RtlZeroMemory(pDevExt->m_0x8400_Data, Data_0x8400_END);

		for(i=0; i<Data_0x8400_END;)	//get 8 byte each time
		{
			//2007/9/5 11:43上午
			if(NT_SUCCESS(Get_ExUnit_Control_Value(GET_CUR, XU_SF_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], SF_BUFFER_LEN, SF_driver_param_addrH, SF_driver_param_addrL+i, GetBuffer)))	// shawn 2010/10/26 modify
				memcpy(&pDevExt->m_0x8400_Data[i], GetBuffer+3, SF_BUFFER_LEN-3);
			else
			{
				for(j=0;j<SF_BUFFER_LEN-3;j++)
					pDevExt->m_0x8400_Data[i+j] = 0xff;
			}

			i += 8;
		}

		if(pDevExt->m_ChipID == 0x15)
		{
			DbgPrint("pDevExt->m_ChipID == 0x15\n");

			if(NT_SUCCESS(Get_ExUnit_Control_Value(GET_CUR, XU_SF_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], SF_BUFFER_LEN, 0x0, 0x0, GetBuffer)))	// shawn 2010/10/26 modify
			{
					for(i=0;i<SF_BUFFER_LEN-3;i++)
						DbgPrint("GetBuffer[%d]=%c\n",i,GetBuffer[i+3]);

					if(!((GetBuffer[3]=='S')&&(GetBuffer[4]=='N')&&(GetBuffer[5]=='9')&&(GetBuffer[6]=='C')
						&&(GetBuffer[7]=='2')&&(GetBuffer[8]=='1')&&(GetBuffer[9]=='5')))
					{
						DbgPrint("Serial flash doesn't exist!!\n");
						
						//2008/5/8 10:49上午
						for(j=0;j<Data_0x8400_END;j++)
							pDevExt->m_0x8400_Data[j] = 0xff;
					}			
					else
						DbgPrint("Serial flash exist!!\n");
			}
		}

		pDevExt->m_XU_AuxLED_Dis = pDevExt->m_0x8400_Data[AUXLED_DISABLE_ADDR]&0X01;
		pDevExt->m_XU_WBC_Dis = pDevExt->m_0x8400_Data[WBC_DISABLE_ADDR]&0X01;
		pDevExt->m_XU_WBCAuto_Dis = pDevExt->m_0x8400_Data[WBCAUTO_DISABLE_ADDR]&0X01;
		pDevExt->m_XU_Privacy_Dis = pDevExt->m_0x8400_Data[PRIVACY_DISABLE_ADDR]&0X01;	
		//2008/1/4 02:50下午
		pDevExt->m_XU_AuxLEDAuto_Dis = pDevExt->m_0x8400_Data[AUXLED_DISABLE_ADDR]&0X04;

		pDevExt->m_SetMirrorOrBW = FALSE;
		// shawn 2011/05/25 -----

		// shawn 2011/06/29 +++++
		pDevExt->m_ResetAutoExposure= FALSE;
		pDevExt->m_ResetAutoFocus= FALSE;
		pDevExt->m_ResetAutoWhiteBalance = FALSE;
		// shawn 2011/06/29 -----

		//2010/8/30 04:15下午		pDevExt->DevicePowerState = PowerDeviceD0;

		//
		// Load default settings from registry
		//
		//2010/8/6 10:32上午 test
		
		if(!NT_SUCCESS(ntStatus = LoadHWDefaultValues()))
		{
			DBGU_TRACE("LoadHWDefaultValues return %X\n",ntStatus);
			// At boot time, driver will fail to get hardware default settings from device.
			ntStatus = STATUS_INVALID_PARAMETER;
			goto Exit_InitializeDevice;
		}
		if(!NT_SUCCESS(ntStatus = LoadDefaultSettings(/*pDevExt->pVideoDevice->m_DefValuePath*/FALSE, FALSE)))	// shawn 2011/05/31 modify
		{
			DBGU_TRACE("LoadDefaultSettings return %X\n",ntStatus);
			// At boot time, driver will fail to get default settings from registry.
			ntStatus = STATUS_INVALID_PARAMETER;
			goto Exit_InitializeDevice;
		}

		// shawn 2011/07/21 modify +++++
		if (!pDevExt->m_TSMUX_Enable)
			pDevExt->pVideoDevice->TSMuxEnable = FALSE;
		// shawn 2011/07/21 modify -----
		
		//2011/1/17 03:46下午 de-mark
		// Load last settings from registry
		
		//2011/5/18 04:52下午
		//2011/2/22 05:56下午 mark
		CurrentSettings(TRUE, TRUE);// Load instance default settings

		//
		// Set image format
		//
		if(!NT_SUCCESS(ntStatus = SetImageFormat()))
		{
			DBGU_TRACE("SetImageFormat return %X\n",ntStatus);
			goto Exit_InitializeDevice;
		}

		//
		// Modify/Create keys instance-index related.
		//
		InterlockedIncrement(&InstanceCount);
		m_InstanceIndex = InterlockedIncrement(&InstanceIndex);

		pDevExt->pVideoDevice->WriteDeviceInfoToReg(m_InstanceIndex);

		// USB Active
		DWORD USBActive = 1;
		RtlInitUnicodeString(&RegPath, (PWSTR) pDevExt->pVideoDevice->m_InstanceKey);
		RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
								RegPath.Buffer,
								L"USBActive",
								REG_DWORD,
								&USBActive,
								sizeof(ULONG));

		// SnapShotFileName
		swprintf(pDevExt->pVideoDevice->m_SnapShotFileName, L"img2-%03d.raw",m_InstanceIndex);
		RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
								RegPath.Buffer,
								L"SnapShotFileName",
								REG_SZ,
								pDevExt->pVideoDevice->m_SnapShotFileName,
								sizeof(pDevExt->pVideoDevice->m_SnapShotFileName));
	}

	DBGU_TRACE("===> m_FaceTrackParams[0] = %d\n",pDevExt->pVideoDevice->m_FaceTrackParams[0]);

	// shawn 2011/06/15 +++++
	// james 2009/06/08 add for avoiding conflict.
	if(pDevExt->pVideoDevice->m_ZoomParam.bZoomEnable == 1)
		pDevExt->pVideoDevice->m_FaceTrackParams[0] = pDevExt->pVideoDevice->m_ZoomParam.bShowUIControl;
	// shawn 2011/06/15 -----

	if(pDevExt->pVideoDevice->m_FaceTrackParams[0] == 0)
	{
		//pDevExt->pVideoDevice->m_ZoomParam.bZoomEnable = 0;
		pDevExt->pVideoDevice->m_ZoomParam.bShowUIControl = 0;
	}
	else if(pDevExt->pVideoDevice->m_FaceTrackParams[0] == 1)
	{
		//pDevExt->pVideoDevice->m_ZoomParam.bZoomEnable = 0;
		pDevExt->pVideoDevice->m_ZoomParam.bShowUIControl = 1;
	}
	else if(pDevExt->pVideoDevice->m_FaceTrackParams[0] == 2)	
	{
		//pDevExt->pVideoDevice->m_ZoomParam.bZoomEnable = 2;
		pDevExt->pVideoDevice->m_ZoomParam.bShowUIControl = 1;
	}

	// shawn 2011/06/15 +++++
	// james 2009/06/09 add for avoiding conflict.
	if(pDevExt->pVideoDevice->m_ZoomParam.bZoomEnable == 0)
		pDevExt->pVideoDevice->m_FaceTrackParams[0] = 0;
	// shawn 2011/06/15 -----

Exit_InitializeDevice:
	if(NT_SUCCESS(ntStatus))
		m_bInitDevFlag = TRUE;
	else
		m_bInitDevFlag = FALSE;

	DBGU_TRACE("InitializeDevice return %d\n", ntStatus);

	return ntStatus;
}

NTSTATUS CVideoStream::InitializationComplete()
{
	DBGU_TRACE("enter CVideoStream::InitializationComplete\n");
    NTSTATUS	ntStatus=STATUS_SUCCESS;
	PDEVEXT	pDevExt = CurrentDevExt();
	HANDLE	hDispatchThread;
	//2011/3/30 05:29下午
	HANDLE	hTSMuxThread;

	if (!pDevExt) {
		DBGU_ERROR("Null Device extension at InitializationComplete().\n");
		return STATUS_UNSUCCESSFUL;
	}

	//2010/8/13 06:17下午
	if(pDevExt->m_C3Enable || pDevExt->m_Autostart_Enable)	// shawn 2011/07/20 modify
	{
		ntStatus = PsCreateSystemThread(&hDispatchThread, 0L, NULL, NULL, NULL,
						(PKSTART_ROUTINE)EventHandlerRoutine, (PVOID)pDevExt->pVideoDevice);

		if (NT_SUCCESS(ntStatus)) {
			ZwClose(hDispatchThread);			
		} else {
			DBGU_TRACE("Fail to create system thread !\n");
		}
	}
	
	pDevExt->m_StreamState = KSSTATE_STOP;
	//2010/6/29 05:58下午 H264
	pDevExt->m_MPEG2StreamState = KSSTATE_STOP;
	
	InterlockedExchangePointer((PVOID *)&pDevExt->DevicePowerState, (PVOID)PowerDeviceD0);
	
	//2010/3/9 07:08下午
	pDevExt->m_bDeviceIdle = TRUE;

	//2011/2/22 05:56下午
	//CurrentSettings(TRUE, TRUE);// Load instance default settings
	//2011/5/19 10:52上午
	ReadyToSetCurSetting = false;
	
	DBGU_TRACE("InitializationComplete return %X\n",ntStatus);
	return ntStatus;
}

NTSTATUS CVideoStream::SurpriseRemoval()
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PDEVEXT pDevExt = CurrentDevExt();

	if (!pDevExt) {
		DBGU_ERROR("Null Device extension at SurpriseRemoval().\n");
		return STATUS_UNSUCCESSFUL;
	}

	pDevExt->m_StreamState = KSSTATE_STOP;
	//2010/6/29 05:55下午 H264
	pDevExt->m_MPEG2StreamState = KSSTATE_STOP;

	m_SurpriseRemoval = TRUE;

	if(pDevExt->pVideoDevice)
		ntStatus = pDevExt->pVideoDevice->SurpriseRemoval();

	return ntStatus;
}

NTSTATUS CVideoStream::UninitializeDevice()
{
	PDEVEXT pDevExt = CurrentDevExt();
	NTSTATUS ntStatus = STATUS_SUCCESS;

	if (!pDevExt) {
		DBGU_ERROR("Null Device extension at UninitializeDevice().\n");
		return STATUS_UNSUCCESSFUL;
	}
	/*
	if (m_pDevicePropertySets)
		delete m_pDevicePropertySets;
	m_pDevicePropertySets = NULL;
	*/
	UNICODE_STRING	RegPath;
	WCHAR BasePath[MAX_PATH]={0},temp[16]={0};

	// Stop interrupt transfer.
	DBGU_TRACE("GetUsbIoCount = %d\n",pDevExt->pVideoDevice->GetUsbIoCount());

	if(pDevExt->pVideoDevice->GetUsbIoCount() > 0)
	{
		pDevExt->pVideoDevice->StopInterruptPipe();
		//pDevExt->pVideoDevice->SetAlternate(0, SELECTED_DATA_INTERFACE_INDEX);
		//2010/6/24 11:36上午 H264
		pDevExt->pVideoDevice->SetAlternate(0, SELECTED_DATA_INTERFACE_INDEX);	// james try M2TS.
		pDevExt->pVideoDevice->SetAlternate(0, SELECTED_MPEG2_INTERFACE_INDEX);	// james try M2TS.
	}

	//2011/5/16 02:01下午 mark
	// Save instance settings
	//2011/2/17 06:14下午 mark
	if(m_bInitDevFlag)
		CurrentSettings(TRUE, FALSE);

	wcscpy(BasePath, SN9C220_KEY_PATH);
	RtlInitUnicodeString(&RegPath, (PWSTR) BasePath);

	// Delete the key instance-index related.
	swprintf(temp, L"ii%04d",m_InstanceIndex);
	RtlDeleteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
							RegPath.Buffer,
							temp);

	// Delete InstanceIndex key
	if(InterlockedDecrement(&InstanceCount) <= 0)
	{
		RtlDeleteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
								RegPath.Buffer,
								L"InstanceIndex");
	}

	// USB Active
	DWORD USBActive = 0;
	RtlInitUnicodeString(&RegPath, (PWSTR) pDevExt->pVideoDevice->m_InstanceKey);
	RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
							RegPath.Buffer,
							L"USBActive",
							REG_DWORD,
							&USBActive,
							sizeof(ULONG));

	//
	// waiting for the end of the usb transfer.
	//
	DBGU_TRACE("GetUsbIoCount = %d\n",pDevExt->pVideoDevice->GetUsbIoCount());
	
	while(pDevExt->pVideoDevice->GetUsbIoCount() > 0)
	{
		KeDelay(5);
	}
	
	//2010/8/26 02:14下午
	//SurpriseRemoval();
	
	if(pDevExt->pVideoDevice)
	{
		delete pDevExt->pVideoDevice;
		pDevExt->pVideoDevice = NULL;
		DBGU_TRACE("---> Delete class pVideoDevice !\n");
	}

	return ntStatus;
}

NTSTATUS CVideoStream::OpenStream(IN PKSPIN pin)
{
	DBGU_TRACE("enter CVideoStream::OpenStream(), StreamNumber=%d\n",pin->Id);
    NTSTATUS ntStatus=STATUS_SUCCESS;
	PDEVEXT pDevExt = CurrentDevExt();
	PKSDATAFORMAT pKSDataFormat = pin->ConnectionFormat;
	ULONG StreamNumber = pin->Id;
	int i;

	if (!pDevExt) {
		DBGU_ERROR("Null Device extension at OpenStream().\n");
		return STATUS_UNSUCCESSFUL;
	}

	//2010/8/2 05:25下午	
	//if ((StreamNumber >= MAX_NUM_STREAM_INFO) ||
	//	(m_StreamInstances[StreamNumber] >= Streams[StreamNumber].NumberOfPossibleInstances))
	if ((StreamNumber >= MAX_NUM_STREAM_INFO) ||
		(m_StreamInstances[StreamNumber] >= pDevExt->m_CaptureFilterPinDescriptors[StreamNumber].InstancesPossible ))
	{// Invalid stream number
		DBGU_TRACE("Calling OpenStream before Close it!(We have not called CloseStream.) \n");
		DBGU_TRACE("m_StreamInstances=%d, NumberOfPossibleInstances=%d\n",
			m_StreamInstances[StreamNumber],pDevExt->m_CaptureFilterPinDescriptors[StreamNumber].InstancesPossible);
        ntStatus = STATUS_INVALID_PARAMETER;
		goto exit_OpenStream;
    }// Invalid stream number

	//2010/8/2 05:10下午
	// Check the validation of the stream format requested.
	//PKSDATAFORMAT *AvailableFormats		= Streams[StreamNumber].StreamFormatsArray;
	//ULONG NumberOfFormatArrayEntries	= Streams[StreamNumber].NumberOfFormatArrayEntries;
	PKSDATAFORMAT *AvailableFormats		= (PKSDATARANGE *)pDevExt->m_CaptureFilterPinDescriptors[StreamNumber].PinDescriptor.DataRanges;
	ULONG NumberOfFormatArrayEntries	= pDevExt->m_CaptureFilterPinDescriptors[StreamNumber].PinDescriptor.DataRangesCount;
	
	for (i = 0; i < NumberOfFormatArrayEntries; i++, AvailableFormats++) 
	{// for loop i
        //Check stream format we supported.
		if(!(IsEqualGUID ((const struct _GUID &)pKSDataFormat->MajorFormat, (const struct _GUID &)(*AvailableFormats)->MajorFormat) &&
			 IsEqualGUID ((const struct _GUID &)pKSDataFormat->SubFormat, (const struct _GUID &)(*AvailableFormats)->SubFormat) &&
			 IsEqualGUID ((const struct _GUID &)pKSDataFormat->Specifier, (const struct _GUID &)(*AvailableFormats)->Specifier)))
			 continue;

		if (IsEqualGUID ((const struct _GUID &)pKSDataFormat->Specifier, (const struct _GUID &)KSDATAFORMAT_SPECIFIER_VIDEOINFO)) 
		{// Specifier == KSDATAFORMAT_SPECIFIER_VIDEOINFO             
            PKS_VIDEOINFOHEADER	pVideoInfoHdrToVerify = (PKS_VIDEOINFOHEADER)&(((PKS_DATAFORMAT_VIDEOINFOHEADER)pKSDataFormat)->VideoInfoHeader);
           	PKS_VIDEOINFOHEADER	pVideoInfoHdr = &(((PKS_DATARANGE_VIDEO)*AvailableFormats)->VideoInfoHeader);

			if ((pVideoInfoHdrToVerify->bmiHeader.biWidth != pVideoInfoHdr->bmiHeader.biWidth) ||
				(pVideoInfoHdrToVerify->bmiHeader.biHeight != pVideoInfoHdr->bmiHeader.biHeight) ||
				(pVideoInfoHdrToVerify->bmiHeader.biCompression != pVideoInfoHdr->bmiHeader.biCompression)) {
				continue;
			}

			if((m_pVideoInfoHeader[StreamNumber]) && (m_pVideoInfoHeader[StreamNumber] != &m_VideoInfoHeader))
			{// The stream is opened more than once.
				DBGU_TRACE("The stream is opened more than once.\n");
				ntStatus = STATUS_INVALID_PARAMETER;
				goto exit_OpenStream;	
			}// The stream is opened more than once.

			// prevent crash, re-initialize to NULL
			if (m_pVideoInfoHeader[StreamNumber] == &m_VideoInfoHeader)
				m_pVideoInfoHeader[StreamNumber] = NULL;

			// m_pVideoInfoHeader[StreamNumber] should be NULL
			ASSERT(m_pVideoInfoHeader[StreamNumber] == NULL);

			if (StreamNumber == pDevExt->m_STREAM_Still_Idx)	// shawn 2011/06/23 modify
			{
				// we only allow method 2 that still pin can render different format with capture pin
				if (pDevExt->m_StillCaptureMethod != 2)
				{
					if ((m_pVideoInfoHeader[STREAM_Capture]->bmiHeader.biWidth!=pVideoInfoHdrToVerify->bmiHeader.biWidth)||
						(m_pVideoInfoHeader[STREAM_Capture]->bmiHeader.biHeight!=pVideoInfoHdrToVerify->bmiHeader.biHeight)||
						(m_pVideoInfoHeader[STREAM_Capture]->bmiHeader.biCompression!=pVideoInfoHdrToVerify->bmiHeader.biCompression))
					{
						DBGU_WARNING("Select different format with capture pin\n");
						ntStatus = STATUS_INVALID_PARAMETER;
						goto exit_OpenStream;
					}
				}

				pDevExt->pVideoDevice->m_SnapShotInfo.wWidth = pVideoInfoHdrToVerify->bmiHeader.biWidth;
				pDevExt->pVideoDevice->m_SnapShotInfo.wHeight = pVideoInfoHdrToVerify->bmiHeader.biHeight;
				pDevExt->pVideoDevice->m_SnapShotInfo.ImageSize = pVideoInfoHdrToVerify->bmiHeader.biSizeImage;
				pDevExt->pVideoDevice->m_SnapShotInfo.biCompression = pVideoInfoHdrToVerify->bmiHeader.biCompression;

				// shawn 2011/07/19 +++++
				switch(pDevExt->pVideoDevice->m_SnapShotInfo.wHeight)
				{
			  		case 120:
			  			pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_160X120;
						break;
					case 144:
						pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_176X144;
						break;	
			  	 	case 240:
			  	 		pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_320X240;
						break;
			  		case 288:
			  			pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_352X288;
						break;
					case 480:
						pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_640X480;
						break;	
			  	 	case 600:
			  	 		pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_800X600;
						break;
						//2007/9/3 03:12下午
					case 720:
			  			// shawn 2011/04/14 modify +++++
						//deviceContext->m_SnapShotSize = SNAPSHOT_960X720;
						
						if(pDevExt->pVideoDevice->m_SnapShotInfo.wHeight == 960)
				  			pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_960X720;
				  		else if(pDevExt->pVideoDevice->m_SnapShotInfo.wHeight == 1280)
				  			pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_1280X720;
						// shawn 2011/04/14 modify -----
						
						break;			
					case 800:
			  			pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_1280X800;
						break;								
			  		case 768:
			  			pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_1024X768;
						break;
					case 960:
						pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_1280X960;
						break;	
			  	 	case 1024:
			  	 		pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_1280X1024;
					  	break;
					case 1200:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_1600X1200;
						break;
					case 1728:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_2304x1728;
						break;
					case 2048:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_2560X2048;
						break;
					case 2400:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_3200X2400;
						break;	
						//2007/12/26 11:12上午
					case 2253:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_2816X2253;
						break;
					case 2640:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_3520X2640;
						break;
					// shawn 2011/04/14 +++++
					case 900:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_1600X900;
						break;
					case 400:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_640X400;
						break;
					case 1080:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_1920X1080;
						break;
					case 1536:
					  	pDevExt->pVideoDevice->m_SnapShotSize = SNAPSHOT_2048X1536;
						break;
					// shawn 2011/04/14 -----
					default:
						break;
				}
				// shawn 2011/07/19 -----
			}

		    m_pVideoInfoHeader[StreamNumber] = (PKS_VIDEOINFOHEADER) ExAllocatePoolWithTag(NonPagedPool, KS_SIZE_VIDEOHEADER(pVideoInfoHdrToVerify), 'osSS');

			if (!m_pVideoInfoHeader[StreamNumber])
			{// Fail to allocate memory 
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				goto exit_OpenStream;
			}// Fail to allocate memory 

			// Copy the VIDEOINFOHEADER requested to our storage.
			RtlCopyMemory(	m_pVideoInfoHeader[StreamNumber],
							pVideoInfoHdrToVerify,
							KS_SIZE_VIDEOHEADER(pVideoInfoHdrToVerify));

			break;
		}// Specifier == KSDATAFORMAT_SPECIFIER_VIDEOINFO
		else
		{// Specifier != KSDATAFORMAT_SPECIFIER_VIDEOINFO
			//2010/4/19 04:16下午 test
			if(StreamNumber != pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
			{
				DBGU_TRACE("Specifier != KSDATAFORMAT_SPECIFIER_VIDEOINFO\n");
				ntStatus = STATUS_INVALID_PARAMETER;
				goto exit_OpenStream;
			}
			else	// james try M2TS.
			{
				DBGU_TRACE("M2TS : Specifier != KSDATAFORMAT_SPECIFIER_VIDEOINFO\n");
			}
		}// Specifier != KSDATAFORMAT_SPECIFIER_VIDEOINFO
	}// for loop i

	// we search all possible formats but failed to find match
	//test 2010/4/20 07:02下午
	if ((i == NumberOfFormatArrayEntries)&&(StreamNumber != pDevExt->m_STREAM_Capture_MP2TS_Idx))	// shawn 2011/06/23 modify
	{
		DBGU_WARNING("we search all possible formats but failed to find match");
		ntStatus = STATUS_INVALID_PARAMETER;
	}
	else
	{
		if (StreamNumber == pDevExt->m_STREAM_Still_Idx)	// shawn 2011/06/23 modify
			pDevExt->m_bStillStreamOpen = TRUE;
		else
			pDevExt->m_bStillStreamOpen = FALSE;

		m_StreamInstances[StreamNumber]++;
	}

exit_OpenStream:
	//2010/4/23 05:21下午 H264
	pDevExt->pVideoDevice->m_MPEG2ReadBufferLength = 0;
	pDevExt->pVideoDevice->m_MJPEGReadBufferLength = 0;	// shawn 2011/07/04 add
	DBGU_TRACE("OpenStream return %X\n",ntStatus);
	return ntStatus;
}

NTSTATUS CVideoStream::CloseStream(IN PKSPIN pin)
{
 	DBGU_TRACE("enter CVideoStream::CloseStream()");

	PDEVEXT pDevExt = CurrentDevExt();
	NTSTATUS ntStatus = STATUS_SUCCESS;
	ULONG StreamNumber = pin->Id;
	
	
	if (!pDevExt) {
		DBGU_ERROR("Null Device extension at CloseStream().\n");
		return STATUS_UNSUCCESSFUL;
	}

	if (StreamNumber == pDevExt->m_STREAM_Still_Idx)	// shawn 2011/06/23 modify
		pDevExt->m_bStillStreamOpen = FALSE;
	
	//2010/8/2 05:24下午
	//if ((StreamNumber >= MAX_NUM_STREAM_INFO) || 
	//	(m_StreamInstances[StreamNumber] < Streams[StreamNumber].InstancesPossible)) 
	if ((StreamNumber >= MAX_NUM_STREAM_INFO) || 
		(m_StreamInstances[StreamNumber] < pDevExt->m_CaptureFilterPinDescriptors[StreamNumber].InstancesPossible)) 
	{// Invalid stream number
        ntStatus = STATUS_INVALID_PARAMETER;
		goto exit_SrbCloseStream;
    }// Invalid stream number

	// fix XP sp1 bluescreen when use property get/get before set stream open.
	if(m_pVideoInfoHeader[StreamNumber] && (m_pVideoInfoHeader[StreamNumber] != &m_VideoInfoHeader))
	{
		ExFreePoolWithTag(m_pVideoInfoHeader[StreamNumber],'osSS');
		m_pVideoInfoHeader[StreamNumber] = NULL;
	}

	m_StreamInstances[StreamNumber]--;

	// re-initialize default Max Available FrameRate
	pDevExt->m_FrameRateInfo.CurrentMaxAvailableFrameRate = pDevExt->m_FrameRateInfo.SupportedFrameRates[pDevExt->m_DefaultFrameIndex[STREAM_Capture]-1];
	DBGU_TRACE("reset CurrentMaxAvailableFrameRate to %d\n" , pDevExt->m_FrameRateInfo.CurrentMaxAvailableFrameRate);

	// Save instance settings to avoid new settings lost after system restart (2005/02/21)
	//2011/5/16 02:03下午 mark
	//if(m_bInitDevFlag)
	//	CurrentSettings(TRUE, FALSE);

exit_SrbCloseStream:

	DBGU_TRACE("CloseStream return %X\n",ntStatus);
	return ntStatus;
}

NTSTATUS CVideoStream::SetStreamState(IN PKSPIN pin, IN KSSTATE StreamState)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PDEVEXT pDevExt = CurrentDevExt();
	
	//2010/6/29 04:24下午 H264
	ULONG StreamNumber = pin->Id;
	
	if (!pDevExt) {
		DBGU_ERROR("Null Device extension at SetStreamState().\n");
		return STATUS_UNSUCCESSFUL;
	}

	switch(StreamState) {
	case KSSTATE_STOP:
		DBGU_TRACE("Set to Stop State !\n");
		ntStatus = pDevExt->pVideoDevice->Stop(StreamNumber);
//RBK BULKMODE		
		pDevExt->pVideoDevice->UnPreparingStreamIrps(pDevExt);

		pDevExt->pVideoDevice->FreeStramingBuffer();
		//m_FirstFrameStartTime = 0;//Reset start time of the first frame	// shawn 2011/12/16 remove for fixing AMCAP capture error
		break;

	case KSSTATE_ACQUIRE:
		DBGU_TRACE("Set to Acquire State !\n");
		break;

	case KSSTATE_PAUSE:
		DBGU_TRACE("Set to Pause State !\n");
		ntStatus = pDevExt->pVideoDevice->Pause(StreamNumber);

		// shawn 2011/05/24 fix recording value bug +++++
		if(m_bInitDevFlag)
			CurrentSettings(TRUE, FALSE);
		// shawn 2011/05/24 fix recording value bug -----

		break;

	case KSSTATE_RUN:
		DBGU_TRACE("Set to Run State !\n");

		// wake up if device is not at PowerDeviceD0
		if (pDevExt->DevicePowerState != PowerDeviceD0)
		{
			DBGU_TRACE("wake up device from PowerDeviceD%d to PowerDeviceD0\n", (pDevExt->DevicePowerState)-1);
			pDevExt->pVideoDevice->SendDeviceSetPower(PowerDeviceD0, TRUE);
			//pDevExt->DevicePowerState = PowerDeviceD0;
		}

		ntStatus = pDevExt->pVideoDevice->AllocateStramingBuffer();
		if (NT_SUCCESS(ntStatus))
		{			
			if (StreamNumber == STREAM_Capture)
			{
				pDevExt->pVideoDevice->m_iIsoInterfaceIndex = SELECTED_DATA_INTERFACE_INDEX;	// james try M2TS.
				
				// shawn 2011/07/19 move to here for fix YouCam preview failed issue +++++
				//2010/8/3 10:33上午 test
				if (pDevExt->m_bStillStreamOpen)// && pDevExt->m_StillCaptureMethod>1)
					ntStatus = pDevExt->pVideoDevice->CheckStillPinBandwidth(*m_pVideoInfoHeader[pDevExt->m_STREAM_Still_Idx]);	// shawn 2011/06/23 modify
				// shawn 2011/07/19 move to here for fix YouCam preview failed issue -----

				ntStatus = pDevExt->pVideoDevice->CheckCapturePinBandwidth(*m_pVideoInfoHeader[STREAM_Capture]);		
	//RBK BULKMODE
				if(NT_SUCCESS(ntStatus))
					ntStatus = pDevExt->pVideoDevice->PreparingStreamIrps(pDevExt,m_Device->NextDeviceObject);


				if(NT_SUCCESS(ntStatus))
					ntStatus = pDevExt->pVideoDevice->Run(*m_pVideoInfoHeader[STREAM_Capture], pin);
			}
			//2010/4/21 02:59下午 H264
			else if(StreamNumber == pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
			{
				// james try M2TS.
				DBGU_TRACE("pin->Id == STREAM_Capture_MP2TS \n");
				pDevExt->pVideoDevice->m_iIsoInterfaceIndex = SELECTED_MPEG2_INTERFACE_INDEX;
				ntStatus = pDevExt->pVideoDevice->CheckCapturePinBandwidth(*m_pVideoInfoHeader[pDevExt->m_STREAM_Capture_MP2TS_Idx]);	// shawn 2011/06/23 modify
			
				if(NT_SUCCESS(ntStatus))
					ntStatus = pDevExt->pVideoDevice->Run(*m_pVideoInfoHeader[pDevExt->m_STREAM_Capture_MP2TS_Idx], pin);	// shawn 2011/06/23 modify

				ntStatus = STATUS_SUCCESS;
			}
			
			//2011/5/19 11:34上午
			if(ReadyToSetCurSetting)
			{
				ReadyToSetCurSetting = false;
				CurrentSettings(TRUE, TRUE);
			}
		}

		break;

	default:
		break;
	}
	
	if(NT_SUCCESS(ntStatus) || ntStatus == STATUS_NO_SUCH_DEVICE) {
		//2010/6/29 06:00下午 H264
		if (StreamNumber == STREAM_Capture)
			pDevExt->m_StreamState = StreamState;
		else
			pDevExt->m_MPEG2StreamState = StreamState;
		
		ntStatus = STATUS_SUCCESS;
	} 
	else { 
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	DBGU_TRACE("CVideoStream::SetStreamState return %X\n",ntStatus);
	return ntStatus;
}

NTSTATUS CVideoStream::SetDeviceProperty(IN PIRP irp, IN PKSPROPERTY Property, IN OUT PVOID data)
{
	DBGU_FN("CVideoStream::SetDeviceProperty()");

	NTSTATUS ntStatus = STATUS_SUCCESS;
	PDEVEXT pDevExt = CurrentDevExt();
	PIO_STACK_LOCATION ioStack = IoGetCurrentIrpStackLocation(irp);

	if (!pDevExt || !ioStack || !data) {
		DBGU_ERROR("Null ioStack or Device extension at SetDeviceProperty(%X,%X,%X).\n",pDevExt,ioStack,data);
		return STATUS_UNSUCCESSFUL;
	}

	ULONG PropertyOutputSize = ioStack->Parameters.DeviceIoControl.OutputBufferLength; // size of data buffer
	ULONG Id = Property->Id;	// index of the property

	// Special handle -- if AP did not open stream first, then open filter to set property.
	if(!m_pVideoInfoHeader[0])
		m_pVideoInfoHeader[0] = &m_VideoInfoHeader;

	if (IsEqualGUID(( const struct _GUID &)PROPSETID_VIDCAP_VIDEOCONTROL, ( const struct _GUID &)Property->Set)) 
	{
		PKSPROPERTY_VIDEOCONTROL_MODE_S	pS = (PKSPROPERTY_VIDEOCONTROL_MODE_S) data;
		DBGU_TRACE("Set PROPSETID_VIDCAP_VIDEOCONTROL: %d",Id);

		switch (Id) {
		case KSPROPERTY_VIDEOCONTROL_MODE:
			DBGU_INFO("Set KSPROPERTY_VIDEOCONTROL_MODE\n");

            if (( pS->Mode & KS_VideoControlFlag_Trigger) || 
                ( pS->Mode & KS_VideoControlFlag_ExternalTriggerEnable ) ) {
					DBGU_INFO("Software button trigger On\n");
					pDevExt->pVideoDevice->StillPinTrigger();
            }

			break;

		default:
			DBGU_WARNING("!! Invalid Id == %d\n",Id);
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
		}
    } 
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_VIDCAP_CAMERACONTROL, ( const struct _GUID &)Property->Set)) 
	{
		DBGU_INFO("Set PROPSETID_VIDCAP_CAMERACONTROL : %d, ",Id);
		PKSPROPERTY_CAMERACONTROL_S	pS	= (PKSPROPERTY_CAMERACONTROL_S) data;

		switch(Id){
		case KSPROPERTY_CAMERACONTROL_EXPOSURE:
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_EXPOSURE - Set(%d), Flags(%d)\n",pS->Value,pS->Flags);
			
			//2011/1/10 10:43上午
			if ((pDevExt->m_InputControls>>1)&1)
			{
				ULONG buffer;

				if(pS->Flags == KSPROPERTY_CAMERACONTROL_FLAGS_AUTO)
					buffer = 0x08;
				else
					buffer = 0x01;

				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(AE_MODE,TRUE,buffer);
				DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_EXPOSURE_AUTO - Set(%d)\n", pS->Value);
			}

			if(pDevExt->m_AEMode.CurrentValue != 8)
			{
				if ((pS->Value < pDevExt->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMinimum) || 
					(pS->Value > pDevExt->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMaximum)) {
					DBGU_WARNING("Invalid value: %d\n", pS->Value);
					ntStatus = STATUS_INVALID_PARAMETER;
				}
				else {
					ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(EXPOSURE,TRUE,pS->Value);
				}
			}

			break;

		case KSPROPERTY_CAMERACONTROL_FOCUS:
			DBGU_INFO("case KSPROPERTY_CAMERACONTROL_FOCUS - Set(%d), Flags(%d)\n",pS->Value,pS->Flags);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[FOCUS].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[FOCUS].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(FOCUS,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_CAMERACONTROL_IRIS:
			DBGU_INFO("case KSPROPERTY_CAMERACONTROL_IRIS - Set(%d), Flags(%d)\n",pS->Value,pS->Flags);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[IRIS].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[IRIS].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(IRIS,TRUE,pS->Value);
			}

			break;
			
			//2011/1/6 10:37上午
		/*case KSPROPERTY_CAMERACONTROL_PANTILT:
			DBGU_INFO("case KSPROPERTY_CAMERACONTROL_PANTILT - Set(%d), Flags(%d)\n",pS->Value,pS->Flags);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[PANTILT].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[PANTILT].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(PANTILT,TRUE,pS->Value);
			}

			break;*/

		case KSPROPERTY_CAMERACONTROL_PAN:
			if ((pS->Value < pDevExt->m_RangeAndStepProp[PAN].Bounds.SignedMinimum) 
			&& (pS->Value > pDevExt->m_RangeAndStepProp[PAN].Bounds.SignedMaximum)) 
			{
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else 
			{
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(PAN,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_CAMERACONTROL_TILT:			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[TILT].Bounds.SignedMinimum) 
			&& (pS->Value > pDevExt->m_RangeAndStepProp[TILT].Bounds.SignedMaximum)) 			
	   		{
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else
			{
	    		ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(TILT,TRUE,pS->Value);  
			}

			break;

		case KSPROPERTY_CAMERACONTROL_ROLL:
			DBGU_INFO("case KSPROPERTY_CAMERACONTROL_ROLL - Set(%d), Flags(%d)\n",pS->Value,pS->Flags);

			if ((pS->Value < pDevExt->m_RangeAndStepProp[ROLL].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[ROLL].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(ROLL,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_CAMERACONTROL_ZOOM:
			DBGU_INFO("case KSPROPERTY_CAMERACONTROL_ZOOM - Set(%d), Flags(%d)\n",pS->Value,pS->Flags);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[ZOOM].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[ZOOM].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(ZOOM,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_CAMERACONTROL_PRIVACY:
			DBGU_INFO("case KSPROPERTY_CAMERACONTROL_PRIVACY - Set(%d), Flags(%d)\n",pS->Value,pS->Flags);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[PRIVACY].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[PRIVACY].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(PRIVACY,TRUE,pS->Value);
			}

			break;

		//2010/12/16 03:52下午
		case KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY:
			DBGU_INFO("case KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY - Set(%d), Flags(%d)\n",pS->Value,pS->Flags);
			
			if ((pS->Value < 0) || 
				(pS->Value > 1)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(AEPRIORITY,TRUE,pS->Value);
			}

			break;

		default:
			DBGU_WARNING("!! Invalid Id == %d\n",Id);
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
		}
	}
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_VIDCAP_VIDEOPROCAMP, ( const struct _GUID &)Property->Set)) 
	{
		DBGU_INFO("Set PROPSETID_VIDCAP_VIDEOPROCAMP : %d, ",Id);

		PKSPROPERTY_VIDEOPROCAMP_S	pS	= (PKSPROPERTY_VIDEOPROCAMP_S) data;

		switch(Id){
		case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS(%d)\n",pS->Value);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[BRIGHTNESS].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[BRIGHTNESS].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(BRIGHTNESS,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_VIDEOPROCAMP_CONTRAST:
			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_CONTRAST(%d)\n",pS->Value);

			if ((pS->Value < pDevExt->m_RangeAndStepProp[CONTRAST].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[CONTRAST].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(CONTRAST,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_VIDEOPROCAMP_HUE:
			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_HUE(%d)\n",pS->Value);

			if ((pS->Value < pDevExt->m_RangeAndStepProp[HUE].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[HUE].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(HUE,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_VIDEOPROCAMP_SATURATION:
			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_SATURATION(%d)\n",pS->Value);

			if ((pS->Value < pDevExt->m_RangeAndStepProp[SATURATION].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[SATURATION].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(SATURATION,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_VIDEOPROCAMP_SHARPNESS:
			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_SHARPNESS(%d)\n",pS->Value);

			if ((pS->Value < pDevExt->m_RangeAndStepProp[SHARPNESS].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[SHARPNESS].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(SHARPNESS,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_VIDEOPROCAMP_GAMMA:
			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_GAMMA(%d)\n",pS->Value);

			if ((pS->Value < pDevExt->m_RangeAndStepProp[GAMMA].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[GAMMA].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(GAMMA,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE:
			//2010/3/16 02:31下午
			if ((pDevExt->m_PropertyControls>>12)&1)
			{
				ULONG buffer;

				if(pS->Flags == KSPROPERTY_CAMERACONTROL_FLAGS_AUTO)
					buffer = 1;
				else
					buffer = 0;
				
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WB_TEMPERATURE_AUTO,TRUE,buffer);	
				DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE_AUTO(%d)\n",buffer);
			}

			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE(%d)\n",pS->Value);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[WHITEBALANCE].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[WHITEBALANCE].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WHITEBALANCE,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION:
			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION(%d)\n",pS->Value);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[BACKLIGHT].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[BACKLIGHT].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(BACKLIGHT,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY:
			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY(%d)\n",pS->Value);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[POWERLINEFREQUENCY].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[POWERLINEFREQUENCY].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(POWERLINEFREQUENCY,TRUE,pS->Value);
			}

			break;

		case KSPROPERTY_VIDEOPROCAMP_GAIN:
			DBGU_INFO("set KSPROPERTY_VIDEOPROCAMP_GAIN(%d)\n",pS->Value);
			
			if ((pS->Value < pDevExt->m_RangeAndStepProp[GAIN].Bounds.SignedMinimum) || 
				(pS->Value > pDevExt->m_RangeAndStepProp[GAIN].Bounds.SignedMaximum)) {
				DBGU_WARNING("Invalid value: %d\n", pS->Value);
				ntStatus = STATUS_INVALID_PARAMETER;
			}
			else {
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(GAIN,TRUE,pS->Value);
			}

			break;

		default:
			DBGU_WARNING("!! Invalid Id == %d\n",Id);
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
		}
	}
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_CUSTOM_PROP_ST50220, ( const struct _GUID &)Property->Set)) 
	{
		DBGU_INFO("Set PROPSETID_CUSTOM_PROP_ST50220 : %d, ",Id);
		PKSPROPERTY_CUSTOM_PROP_S	pS	= (PKSPROPERTY_CUSTOM_PROP_S) data;

		switch (Id)
		{
		case KSPROPERTY_CUSTOM_PROP_MIRROR:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(MIRROR, TRUE, pS->Value);
			pDevExt->m_SetMirrorOrBW = TRUE;	// shawn 2011/05/25 add
			break;

		case KSPROPERTY_CUSTOM_PROP_FLIP:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(FLIP, TRUE, pS->Value);
			pDevExt->m_SetMirrorOrBW = TRUE;	// shawn 2011/05/25 add
			break;

		case KSPROPERTY_CUSTOM_PROP_BW_MODE:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(BW_MODE, TRUE, pS->Value);
			pDevExt->m_SetMirrorOrBW = TRUE;	// shawn 2011/05/25 add
			break;

		case KSPROPERTY_CUSTOM_PROP_SNAPSHOTIMAGESIZE:
			pDevExt->pVideoDevice->m_SnapShotSize = pS->Value;
			break;

		case KSPROPERTY_CUSTOM_PROP_RESETSETTINGS:
			ntStatus = LoadDefaultSettings(/*pDevExt->pVideoDevice->m_DefValuePath*/FALSE,TRUE);	// shawn 2011/05/31 modify

			// shawn 2011/06/29 +++++
			pDevExt->m_ResetAutoExposure= TRUE;
			pDevExt->m_ResetAutoFocus= TRUE;
			pDevExt->m_ResetAutoWhiteBalance = TRUE;
			// shawn 2011/06/29 -----

			break;

		case KSPROPERTY_CUSTOM_PROP_SAVESETTINGS:
			ntStatus = CurrentSettings(FALSE,FALSE);
			break;

		case KSPROPERTY_CUSTOM_PROP_RESTORESETTINGS:
			ntStatus = /*CurrentSettings(FALSE,TRUE)*/LoadDefaultSettings(/*pDevExt->pVideoDevice->m_CurValuePath*/TRUE,TRUE);	// shawn 2011/05/31 modify

			// shawn 2011/06/29 +++++
			pDevExt->m_ResetAutoExposure= TRUE;
			pDevExt->m_ResetAutoFocus= TRUE;
			pDevExt->m_ResetAutoWhiteBalance = TRUE;
			// shawn 2011/06/29 -----

			break;

#ifndef _NOEFFECT
		case KSPROPERTY_CUSTOM_PROP_EFFECT:
			pDevExt->pVideoDevice->m_EffectParam = pS->Value;
			break;

		case KSPROPERTY_CUSTOM_PROP_FRAME_FILENAME:
			{
				pS->FileName[sizeof(pS->FileName) - 1] = 0;
				DBGU_INFO("KSPROPERTY_CUSTOM_PROP_FRAME_FILENAME - Set(%s)\n", pS->FileName);
				mbstowcs((pDevExt->pVideoDevice->m_FrameFileName + 12), pS->FileName, sizeof(pS->FileName));

				LARGE_INTEGER timeout;
				NTSTATUS waitStatus;

				timeout.QuadPart = -10000000;
				waitStatus = KeWaitForSingleObject(&pDevExt->pVideoDevice->m_EffectMutex, Executive, KernelMode, FALSE, &timeout);

				// shawn 2011/06/01 modify +++++
				if (!SetFrameFile(pDevExt->pVideoDevice->m_FrameFileName, 255, 1, &(pDevExt->pVideoDevice->m_FrameTransIfo),pDevExt->pVideoDevice->m_pEffectBufferPtr))
				//if (!SetFrameFile(pDevExt->pVideoDevice->m_FrameFileName, 255, 1, &pDevExt->pVideoDevice->m_FrameTransIfo))
				// shawn 2011/06/01 modify -----
				{
					DBGU_INFO("KSPROPERTY_CUSTOM_PROP_FRAME_FILENAME - SetFrameFile(%s) is failed\n", pS->FileName);
					ntStatus = STATUS_UNSUCCESSFUL;
				}
				else
				{
					DBGU_INFO("KSPROPERTY_CUSTOM_PROP_FRAME_FILENAME - SetFrameFile(%s) is successful\n", pS->FileName);
				}
				
				pDevExt->pVideoDevice->m_EffectParam = 0xfe;
				
				if (STATUS_SUCCESS == waitStatus)
					KeReleaseMutex(&pDevExt->pVideoDevice->m_EffectMutex, FALSE);
			}

			break;
#endif

		case KSPROPERTY_CUSTOM_PROP_WBTemperatureControlAuto:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WB_TEMPERATURE_AUTO, TRUE, pS->Value);
			break;

		case KSPROPERTY_CUSTOM_PROP_WBComponentControl:
			if(!pDevExt->m_XU_WBC_Dis)
			{
				if(((pS->Value>>8) >= (pDevExt->m_WBC.MinValue>>8)) &&((pS->Value&0xff) >= (pDevExt->m_WBC.MinValue&0xff))
					&&((pS->Value>>8) <= (pDevExt->m_WBC.MaxValue>>8)) &&((pS->Value&0xff) <= (pDevExt->m_WBC.MaxValue&0xff)))
				{
					ntStatus = Set_Cur_WBC_Value(pS->Value);
					//2007/8/9 05:09下午
					DbgPrint("Set WBComponentControl Current =0x%x\n",pS->Value);
					pDevExt->m_WBC.CurrentValue = pS->Value;
				}
				else
					ntStatus = STATUS_INVALID_PARAMETER;
			}
			else
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WB_COMPONENT, TRUE, pS->Value);
			
			break;

		case KSPROPERTY_CUSTOM_PROP_WBComponentControlAuto:
			// shawn 2011/06/01 modify +++++
			if(!pDevExt->m_XU_WBCAuto_Dis)
			{
				if (pS->Value <= 1)
				{
					Set_Cur_WBCAuto_Value(pS->Value);
					//2007/8/9 05:17下午
					pDevExt->m_WBCAuto.CurrentValue = pS->Value;	
				}
				else
	                ntStatus = STATUS_INVALID_PARAMETER;
			}	
			else
			{
	            if (((pDevExt->m_PropertyControls>>13)&1) && (pS->Value <= 1) ) 
	            {
	  				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WB_COMPONENT_AUTO, TRUE, pS->Value);
					pDevExt->m_WBCAuto.CurrentValue = pS->Value;
	            } 
	            else
	                ntStatus = STATUS_INVALID_PARAMETER;
			}
			// shawn 2011/06/01 modify -----
			
			break;

		case KSPROPERTY_CUSTOM_PROP_AEModeControl:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(AE_MODE, TRUE, pS->Value);
			break;

		case KSPROPERTY_CUSTOM_PROP_AEPriorityControl:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(AE_PRIORITY, TRUE, pS->Value);
			break;

			// shawn 2011/05/25 +++++
		case KSPROPERTY_CUSTOM_PROP_ROTATION:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(ROTATION, TRUE, pS->Value);
			pDevExt->m_SetMirrorOrBW = TRUE;
			break;

		case KSPROPERTY_CUSTOM_PROP_AUXLED_MODE:
			if(!pDevExt->m_XU_AuxLED_Dis)
			{
				Set_Cur_AUXLED_Value(pS->Value);
				pDevExt->m_AuxLedMode.CurrentValue = pS->Value;	
			}

			break;

		case KSPROPERTY_CUSTOM_PROP_SNAPSHOT_CONTROL:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(SNAPSHOT_CTRL, TRUE, pS->Value);
			break;

		case KSPROPERTY_CUSTOM_PROP_DENOISE_MODE:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(DENOISE, TRUE, pS->Value);
			break;

		case KSPROPERTY_CUSTOM_PROP_WBComponentControl_Green:
			if(!pDevExt->m_XU_WBC_Dis)
			{
				if(((pS->Value&0xff) >= (pDevExt->m_WBC_Green.MinValue&0xff))&&((pS->Value&0xff) <= (pDevExt->m_WBC_Green.MaxValue&0xff)))
				{
					Set_Cur_WBC_Green_Value(pS->Value);
					DbgPrint("Set WBComponentControl_Green Current =0x%x\n",pS->Value);
					pDevExt->m_WBC_Green.CurrentValue = pS->Value;	
				}	
			}

			break;
			// shawn 2011/05/25 -----

		default:
			DBGU_WARNING("!! Invalid Id == %d\n",Id);
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
		}
	}
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_ZOOM_PROP_ST50220, ( const struct _GUID &)Property->Set)) 
	{
		DBGU_TRACE("PROPSETID_ZOOM_PROP_ST50220 : %d, ",Id);

		PKSPROPERTY_ZOOM_PROP_S pS	= (PKSPROPERTY_ZOOM_PROP_S) data;
		pDevExt->pVideoDevice->m_ZoomParam.bZoomEnable = 1;
		/*
		//
		// when mirror or flip works, we need to change zoom movement direction
		//
		if (Id==KSPROPERTY_ZOOM_PROP_RIGHT||Id==KSPROPERTY_ZOOM_PROP_LEFT)
		{
			// check Mirror is ON
			if (pDevExt->m_Mirror.CurrentValue)
				Id = (Id==KSPROPERTY_ZOOM_PROP_LEFT)? KSPROPERTY_ZOOM_PROP_RIGHT:KSPROPERTY_ZOOM_PROP_LEFT;
		}
		else if (Id==KSPROPERTY_ZOOM_PROP_UP||Id==KSPROPERTY_ZOOM_PROP_DOWN)
		{
			// check Flip is ON
			if (pDevExt->m_Flip.CurrentValue)
				Id = (Id==KSPROPERTY_ZOOM_PROP_UP)? KSPROPERTY_ZOOM_PROP_DOWN:KSPROPERTY_ZOOM_PROP_UP;
		}
		*/
		pDevExt->pVideoDevice->m_bSyncLock=true;

		switch(Id)
		{
			case KSPROPERTY_ZOOM_PROP_LEFT:
				DBGU_TRACE("SET KSPROPERTY_ZOOM_PROP_LEFT");
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(ImgWidList[pDevExt->pVideoDevice->m_enImageOut], ImgHgtList[pDevExt->pVideoDevice->m_enImageOut],0, 
						pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 
						-2, 
						0,
						1,
						0);
				//pDevExt->pVideoDevice->SetASICShellSize(pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, -2, 0, TRUE, FALSE);
				break;
			
			case KSPROPERTY_ZOOM_PROP_UP:
				DBGU_TRACE("SET KSPROPERTY_ZOOM_PROP_UP");
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(ImgWidList[pDevExt->pVideoDevice->m_enImageOut], ImgHgtList[pDevExt->pVideoDevice->m_enImageOut],0, 
						pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 
						0, 
						-2,
						1,
						0);				
				//pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 0, -2, TRUE, FALSE);
				break;

			case KSPROPERTY_ZOOM_PROP_RIGHT:
				DBGU_TRACE("SET KSPROPERTY_ZOOM_PROP_RIGHT");
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(ImgWidList[pDevExt->pVideoDevice->m_enImageOut], ImgHgtList[pDevExt->pVideoDevice->m_enImageOut],0, 
						pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 
						2, 
						0,
						1,
						0);				
				//pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 2, 0, TRUE, FALSE);
				break;

			case KSPROPERTY_ZOOM_PROP_DOWN:
				DBGU_TRACE("SET KSPROPERTY_ZOOM_PROP_DOWN");
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(ImgWidList[pDevExt->pVideoDevice->m_enImageOut], ImgHgtList[pDevExt->pVideoDevice->m_enImageOut],0, 
						pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 
						0, 
						2,
						1,
						0);				
				//pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 0, 2, TRUE, FALSE);
				break;

			case KSPROPERTY_ZOOM_PROP_ZOOM_IN:
				DBGU_TRACE("SET KSPROPERTY_ZOOM_PROP_ZOOM_IN");
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStep = pS->Value;
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(ImgWidList[pDevExt->pVideoDevice->m_enImageOut], ImgHgtList[pDevExt->pVideoDevice->m_enImageOut],0, 
						pS->Value, 
						0, 
						0,
						1,
						0);				
				//pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(pS->Value, 0, 0, TRUE, 1);
				break;

				// add for setting absolute position of Pan
			case KSPROPERTY_ZOOM_PROP_PAN_POS:
				DBGU_TRACE("SET KSPROPERTY_ZOOM_PROP_PAN_POS");
				//pDevExt->pVideoDevice->m_ZoomParam.iHPaneIdx = 0;
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(ImgWidList[pDevExt->pVideoDevice->m_enImageOut], ImgHgtList[pDevExt->pVideoDevice->m_enImageOut],0, 
						pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 
						pS->Value, 
						pDevExt->pVideoDevice->m_ZoomParam.iVPaneIdx,
						1,
						1);						
				//pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, pS->Value, 0, TRUE, 1);
				break;

				// add for setting absolute position of Tilt
			case KSPROPERTY_ZOOM_PROP_TILT_POS:
				DBGU_TRACE("SET KSPROPERTY_ZOOM_PROP_TILT_POS");
				//pDevExt->pVideoDevice->m_ZoomParam.iVPaneIdx = 0;
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(ImgWidList[pDevExt->pVideoDevice->m_enImageOut], ImgHgtList[pDevExt->pVideoDevice->m_enImageOut],0, 
						pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 
						pDevExt->pVideoDevice->m_ZoomParam.iHPaneIdx,
						pS->Value,
						1,
						1);					
				//pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(pDevExt->pVideoDevice->m_ZoomParam.iZoomStep, 0, pS->Value, TRUE, 1);
				break;

			case KSPROPERTY_ZOOM_PROP_ENABLE_OPTION: 
				DBGU_TRACE("SET KSPROPERTY_ZOOM_PROP_ZOOM_ENABLE_OPTION =%d\n",pS->Value);

				// Disable Effect
				if (pS->Value != 0)
					pDevExt->pVideoDevice->m_EffectParam = 0;

				switch (pS->Value)
				{
					case 0:	//Disable All
					case 1:	//ZoomIn Enable
					{
						if(pDevExt->pVideoDevice->m_ZoomParam.bShowUIControl)
							pDevExt->pVideoDevice->m_FaceTrackParams[0] = 1;
						else
							pDevExt->pVideoDevice->m_FaceTrackParams[0] = 0;
					}

					break;

					case 2:	//Face Tracking Enable
					{	
						// 0 : hide FT UI and FT disable, 1 : show FT UI and FT disable, 2 : show FT UI and FT enable
						pDevExt->pVideoDevice->m_FaceTrackParams[0] = 2;
					}

					break;
				}
				
				//pDevExt->pVideoDevice->SetASICShellSize(0, 0, 0, FALSE, FALSE);
				pDevExt->pVideoDevice->m_ZoomParam.bZoomEnable = pS->Value;				
				
				if (pDevExt->pVideoDevice->m_pCShell)
					CShell_ReadFaceTrackParamFromReg(pDevExt->pVideoDevice->m_pCShell, pDevExt->pVideoDevice->m_FaceTrackParams);
				
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStep = 0;
				pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(ImgWidList[pDevExt->pVideoDevice->m_enImageOut], ImgHgtList[pDevExt->pVideoDevice->m_enImageOut],0, 
						0, 
						0,
						0,
						1,
						1);					
				//pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus = pDevExt->pVideoDevice->SetASICShellSize(0, 0, 0, TRUE, 1);
				break;

			default:
				DBGU_WARNING("!! Invalid Id \n");
				ntStatus = STATUS_UNSUCCESSFUL;
				break;
		}

		pDevExt->pVideoDevice->m_bSyncLock=false;
	}
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_CLIENT_PROP_ST50220, ( const struct _GUID &)Property->Set)) 
	{
		DBGU_INFO("Set PROPSETID_CLIENT_PROP_ST50220 : %d, ",Id);
		PKSPROPERTY_CLIENT_PROP_S	pS	= (PKSPROPERTY_CLIENT_PROP_S) data;
		UCHAR pData = (UCHAR)(pS->value&0xff);

		switch (Id)
		{
		case KSPROPERTY_CLIENT_PROP_ASICWRITE:
			DBGU_TRACE("SET KSPROPERTY_CLIENT_PROP_ASICWRITE\n");
			ntStatus = pDevExt->pVideoDevice->ASIC_Communication(CTL_W_NONE, 1, pS->index, &pData);
			break;

		default:
			DBGU_WARNING("!! Invalid Id == %d\n",Id);
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
		}
	}
	else
	{
		BOOLEAN bXU_Ctrl=FALSE;
		int i;

		for(i=0;i<pDevExt->m_ExUnitNum;i++)
		{
			if (IsEqualGUID(( const struct _GUID &)pDevExt->m_ExUnitGUID[i], ( const struct _GUID &)Property->Set))
			{
				PBYTE pS = (PBYTE) data;
				PUCHAR pData=NULL;
				ULONG Value;
				ULONG Index;
				ULONG BufferLength;
				UCHAR Flags = 0x01;	// for Set

				bXU_Ctrl = TRUE;

				Value = Id<<8;
				Index = pDevExt->m_ExUnitID[i]<<8;
				BufferLength = pDevExt->m_ExUnitParamLength[i][Id-1];
				pData = new (NonPagedPool) UCHAR[BufferLength];

				if (pData==NULL)
				{
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}

				RtlZeroMemory(pData, BufferLength);
				RtlCopyMemory(pData, pS, BufferLength);
				
				//2010/4/30 10:41上午
				DBGU_TRACE("Michael Set XU_Ctrl ID = %d\n", Id);
				DBGU_TRACE("Michael Set XU_Ctrl len = %d\n", BufferLength);
				
				for(int k=0;k<BufferLength;k++)
					DBGU_TRACE("Michael Set XU_Ctrl data[%d] = %X", k, pData[k]);
				
				ntStatus = pDevExt->pVideoDevice->USB_ControlClassCommand(
					Flags,
					Value,
					Index,
					pData,
					BufferLength,
					FALSE);
				
				DBGU_TRACE("Michael Set XU_Ctrl return %X\n", ntStatus);
				
				if (pData)
					delete pData;

				break;
			}
		}

		if (!bXU_Ctrl)
		{
			DBGU_WARNING("SetDeviceProperty:We did not support this device property Id:%d!\n",Property->Id);
			ntStatus = STATUS_NOT_IMPLEMENTED;
		}
    }

	DBGU_TRACE("SetDeviceProperty: return status: 0x%X!\n",ntStatus);
	return ntStatus;
}

NTSTATUS CVideoStream::GetDeviceProperty(IN PIRP irp, IN PKSPROPERTY Property, IN OUT PVOID data)
{
	DBGU_TRACE("CVideoStream::GetDeviceProperty()");

	NTSTATUS ntStatus = STATUS_SUCCESS;
	PDEVEXT pDevExt = CurrentDevExt();
	PIO_STACK_LOCATION ioStack = IoGetCurrentIrpStackLocation(irp);

	if (!pDevExt || !ioStack) {
		DBGU_ERROR("Null ioStack or Device extension at GetDeviceProperty(%X,%X).\n",ioStack,pDevExt);
		return STATUS_UNSUCCESSFUL;
	}

	if (!pDevExt->pVideoDevice) {
		DBGU_ERROR("Null pDevExt->pVideoDevice at GetDeviceProperty().\n");
		return STATUS_UNSUCCESSFUL;
	}

	PULONG_PTR pActualBytesTransferred = &(irp->IoStatus.Information);
	ULONG PropertyOutputSize = ioStack->Parameters.DeviceIoControl.OutputBufferLength; // size of data buffer
	ULONG Id = Property->Id;	// index of the property
	
	// Special handle -- if AP did not open stream first, then open filter to set property.
	if(!m_pVideoInfoHeader[STREAM_Capture])
		m_pVideoInfoHeader[STREAM_Capture] = &m_VideoInfoHeader;

	if(IsEqualGUID((const struct _GUID &)KSPROPSETID_Connection, (const struct _GUID &)Property->Set)) 
	{//KSPROPSETID_Connection
		ntStatus = GetStreamProperty(irp, Property, data);
    }//KSPROPSETID_Connection
	else if(IsEqualGUID((const struct _GUID &)PROPSETID_VIDCAP_DROPPEDFRAMES, (const struct _GUID &)Property->Set)) 
	{//PROPSETID_VIDCAP_DROPPEDFRAMES
		ntStatus = GetStreamProperty(irp, Property, data);
	}//PROPSETID_VIDCAP_DROPPEDFRAMES
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_VIDCAP_VIDEOCONTROL, ( const struct _GUID &)Property->Set)) 
	{
		DBGU_TRACE("Get PROPSETID_VIDCAP_VIDEOCONTROL: %d",Id);

		switch (Id) 
		{
		case KSPROPERTY_VIDEOCONTROL_CAPS:
		{
			DBGU_TRACE(" KSPROPERTY_VIDEOCONTROL_CAPS\n");

			PKSPROPERTY_VIDEOCONTROL_CAPS_S pS = (PKSPROPERTY_VIDEOCONTROL_CAPS_S) data;
			ASSERT (PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_CAPS_S));

			// fix for support software trigger
			pS->VideoControlCaps =  KS_VideoControlFlag_ExternalTriggerEnable|KS_VideoControlFlag_Trigger;
			*pActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCONTROL_CAPS_S);
		}

		break;

		case KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE:
		{
			DBGU_TRACE(" KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE\n");

			PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pS = (PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S) data;
	        ASSERT (PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S));

			pS->CurrentActualFrameRate = pDevExt->m_FrameRateInfo.CurrentActualFrameRate;
			pS->CurrentMaxAvailableFrameRate = pDevExt->m_FrameRateInfo.CurrentMaxAvailableFrameRate;
	        *pActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S);

			DBGU_TRACE("Actual Frame Rate : %d, MaxAvailableFrameRate :%d\n", pS->CurrentActualFrameRate, pS->CurrentMaxAvailableFrameRate);
		}

		break;

		case KSPROPERTY_VIDEOCONTROL_FRAME_RATES:
		{
			PKSPROPERTY_VIDEOCONTROL_FRAME_RATES_S pData = (PKSPROPERTY_VIDEOCONTROL_FRAME_RATES_S) Property;
			//2010/5/13 05:57下午
			USHORT count= pDevExt->StreamFormatArray[pData->RangeIndex].FrameRatesCount;//pDevExt->m_FrameRateInfo.m_FrameRatesIndex[pData->RangeIndex+1]-pDevExt->m_FrameRateInfo.m_FrameRatesIndex[pData->RangeIndex];
			ULONG size = sizeof(KSMULTIPLE_ITEM) + ((FRAME_RATE_LIST_SIZE ) * sizeof(LONGLONG));
			ULONG size2 = sizeof(KSMULTIPLE_ITEM) + sizeof(LONGLONG);
			PKSMULTIPLE_ITEM pMultipleItem = (PKSMULTIPLE_ITEM) data;
			LONGLONG *pDataBuf = (LONGLONG *) (pMultipleItem + 1);

			DBGU_TRACE("pData->RangeIndex=%d,StreamIndex=%d\n",pData->RangeIndex,pData->StreamIndex);
			
			if (PropertyOutputSize == 0)
			{
				if (pData->StreamIndex == STREAM_Capture)
					*pActualBytesTransferred = size;
				else
					*pActualBytesTransferred = size2;

				ntStatus = STATUS_BUFFER_OVERFLOW;
				DBGU_TRACE("STATUS_BUFFER_OVERFLOW !\n");
			}
			else if ((PropertyOutputSize >= size) && (pData->StreamIndex == STREAM_Capture))
			{
				//USHORT start = pDevExt->m_FrameRateInfo.m_FrameRatesIndex[pData->RangeIndex];
				//DBGU_TRACE("start = %d,count=%d\n",start,count);
				// Copy the input property info to the property info
				pMultipleItem->Size = count * sizeof(LONGLONG);
				pMultipleItem->Count = count;

				for (int i=0;i<count;i++)
				{
					//pDataBuf[count-i-1] = pDevExt->m_FrameRateInfo.SupportedFrameRates[start+i];
					pDataBuf[count-i-1] = pDevExt->StreamFormatArray[pData->RangeIndex].SupportedFrameRates[i];
					DBGU_TRACE("Frame rate : %d \n", 10000000/pDataBuf[count-i-1]);
				}

				//RBK there is no reason to update current max available framerate and actual framerate here
				//    It may cause faulty framerate value be set to device
				//pDevExt->m_FrameRateInfo.CurrentMaxAvailableFrameRate = pDevExt->StreamFormatArray[pData->RangeIndex].SupportedFrameRates[0];//pDevExt->m_FrameRateInfo.SupportedFrameRates[start];
				//pDevExt->m_FrameRateInfo.CurrentActualFrameRate = pDevExt->m_FrameRateInfo.CurrentMaxAvailableFrameRate;
				//DBGU_TRACE("CurrentMaxAvailableFrameRate = %d\n",10000000/pDevExt->m_FrameRateInfo.CurrentMaxAvailableFrameRate);

				*pActualBytesTransferred = size;
				ntStatus = STATUS_SUCCESS;
			}
			else if (PropertyOutputSize >= size2)	// still pin
			{
				pMultipleItem->Size = sizeof(LONGLONG);
				pMultipleItem->Count = 1;
				// return 0 fps
				pDataBuf[0] = 0;
				*pActualBytesTransferred = size2;
				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}
		}

		break;

		case KSPROPERTY_VIDEOCONTROL_MODE:
		{
			DBGU_TRACE(" KSPROPERTY_VIDEOCONTROL_MODE\n");

			PKSPROPERTY_VIDEOCONTROL_MODE_S pS = (PKSPROPERTY_VIDEOCONTROL_MODE_S) data;    // pointer to the data

			ASSERT (PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_MODE_S));

			pS->Mode = KS_VideoControlFlag_ExternalTriggerEnable|KS_VideoControlFlag_Trigger;
	        *pActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCONTROL_MODE_S);
		}

		break;

		default:
			DBGU_WARNING("!! Invalid Id \n");
			ntStatus = STATUS_NOT_IMPLEMENTED;
        break;
		}
	}
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_VIDCAP_CAMERACONTROL, ( const struct _GUID &)Property->Set))
	{
		DBGU_INFO("Get PROPSETID_VIDCAP_CAMERACONTROL: %d\n",Id);

		PKSPROPERTY_CAMERACONTROL_S	pS	= (PKSPROPERTY_CAMERACONTROL_S) data;

		pS->Flags = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL | KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE;
		pS->Capabilities = KSPROPERTY_CAMERACONTROL_FLAGS_MANUAL | KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE;
		*pActualBytesTransferred = sizeof(KSPROPERTY_CAMERACONTROL_S);
		
		// Check input buff size vaild
		if (PropertyOutputSize==0)
		{
			return STATUS_BUFFER_OVERFLOW;
		}
		else if (PropertyOutputSize < *pActualBytesTransferred)
		{
			return STATUS_BUFFER_TOO_SMALL;
		}

		switch(Id){
		case KSPROPERTY_CAMERACONTROL_EXPOSURE:
			//2010/3/16 03:25下午
			if ((pDevExt->m_InputControls>>1)&1)
			{
				pS->Capabilities = KSPROPERTY_CAMERACONTROL_FLAGS_AUTO | KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE;
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(AE_MODE,FALSE);
				//pS->Value = pDevExt->m_StreamProperties[WB_TEMPERATURE_AUTO];
				pS->Value = pDevExt->m_AEMode.CurrentValue;
				DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_EXPOSURE_AUTO - Get(%d)\n", pS->Value);
					
				if(!(pS->Value&0x01))
					pS->Flags = KSPROPERTY_CAMERACONTROL_FLAGS_AUTO | KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE;
			}

			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(EXPOSURE,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[EXPOSURE];
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_EXPOSURE - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_CAMERACONTROL_PRIVACY:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(PRIVACY,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[PRIVACY];
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_PRIVACY - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_CAMERACONTROL_FOCUS:
			//2010/3/16 03:25下午
			if ((pDevExt->m_InputControls>>17)&1)
			{
				pS->Capabilities = KSPROPERTY_CAMERACONTROL_FLAGS_AUTO | KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE;
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(FOCUS_AUTO,FALSE);
				pS->Value = pDevExt->m_PropertyCurrentValues[FOCUS_AUTO];
				DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_FOCUS_AUTO - Get(%d)\n", pS->Value);
				
				if(pS->Value)
					pS->Flags = KSPROPERTY_CAMERACONTROL_FLAGS_AUTO | KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE;
			}

			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(FOCUS,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[FOCUS];
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_FOCUS - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_CAMERACONTROL_IRIS:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(IRIS,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[IRIS];
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_IRIS - Get(%d)\n", pS->Value);
			break;

		//2011/1/6 02:42下午
		//case KSPROPERTY_CAMERACONTROL_PANTILT:
		case KSPROPERTY_CAMERACONTROL_PAN:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(PAN,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[PAN];
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_PAN - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_CAMERACONTROL_TILT:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(TILT,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[TILT];
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_TILT - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_CAMERACONTROL_ROLL:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(ROLL,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[ROLL];
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_ROLL - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_CAMERACONTROL_ZOOM:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(ZOOM,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[ZOOM];
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_ZOOM - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY://2010/12/16 03:51下午
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(AEPRIORITY,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[AEPRIORITY];
			DBGU_INFO("KSPROPERTY_CAMERACONTROL_AUTO_EXPOSURE_PRIORITY - Get(%d)\n", pS->Value);
			break;

		default:
			DBGU_WARNING("!! Invalid Id \n");
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
		}
	}
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_VIDCAP_VIDEOPROCAMP, ( const struct _GUID &)Property->Set))
	{
		DBGU_INFO("Get PROPSETID_VIDCAP_VIDEOPROCAMP : %d\n",Id);

		PKSPROPERTY_VIDEOPROCAMP_S	pS	= (PKSPROPERTY_VIDEOPROCAMP_S) data;

		pS->Flags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
		pS->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
		*pActualBytesTransferred = sizeof(KSPROPERTY_VIDEOPROCAMP_S);
		
		// Check input buff size vaild
		if (PropertyOutputSize==0)
		{
			return STATUS_BUFFER_OVERFLOW;
		}
		else if (PropertyOutputSize < *pActualBytesTransferred)
		{
			return STATUS_BUFFER_TOO_SMALL;
		}

		switch(Id){
		case KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(BRIGHTNESS,FALSE);
			//2010/3/16 07:07下午
			pS->Value = (signed short)pDevExt->m_PropertyCurrentValues[BRIGHTNESS];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_VIDEOPROCAMP_CONTRAST:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(CONTRAST,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[CONTRAST];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_CONTRAST - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_VIDEOPROCAMP_HUE:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(HUE,FALSE);
			pS->Value = (signed short)pDevExt->m_PropertyCurrentValues[HUE];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_HUE - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_VIDEOPROCAMP_SATURATION:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(SATURATION,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[SATURATION];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_SATURATION - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_VIDEOPROCAMP_SHARPNESS:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(SHARPNESS,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[SHARPNESS];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_SHARPNESS - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_VIDEOPROCAMP_GAMMA:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(GAMMA,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[GAMMA];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_GAMMA - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE:
			//2010/3/16 02:19下午
			if ((pDevExt->m_PropertyControls>>12)&1)
			{
				pS->Capabilities = KSPROPERTY_CAMERACONTROL_FLAGS_AUTO | KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE;
				ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WB_TEMPERATURE_AUTO,FALSE);
				//pS->Value = pDevExt->m_StreamProperties[WB_TEMPERATURE_AUTO];
				pS->Value = pDevExt->m_WBTAuto.CurrentValue;
				DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE_AUTO - Get(%d)\n", pS->Value);
				
				if(pS->Value)
					pS->Flags = KSPROPERTY_CAMERACONTROL_FLAGS_AUTO | KSPROPERTY_CAMERACONTROL_FLAGS_ABSOLUTE;
			}

			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WHITEBALANCE,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[WHITEBALANCE];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(BACKLIGHT,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[BACKLIGHT];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION - Get(%d)\n", pS->Value);
			break;

		case KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(POWERLINEFREQUENCY,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[POWERLINEFREQUENCY];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY - Get(%d)\n", pS->Value);
			break;

		case  KSPROPERTY_VIDEOPROCAMP_GAIN:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(GAIN,FALSE);
			pS->Value = pDevExt->m_PropertyCurrentValues[GAIN];
			DBGU_INFO("KSPROPERTY_VIDEOPROCAMP_GAIN - Get(%d)\n", pS->Value);
			break;

		default:
			DBGU_WARNING("!! Invalid Id \n");
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
		}
	}
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_CUSTOM_PROP_ST50220, ( const struct _GUID &)Property->Set)) 
	{
		DBGU_INFO("Get PROPSETID_CUSTOM_PROP_ST50220 : %d, ",Id);
		PKSPROPERTY_CUSTOM_PROP_S	pS	= (PKSPROPERTY_CUSTOM_PROP_S) data;

		switch (Id)
		{
		case KSPROPERTY_CUSTOM_PROP_MIRROR:
			pS->Value = pDevExt->m_Mirror.CurrentValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_FLIP:
			pS->Value = pDevExt->m_Flip.CurrentValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_BW_MODE:
			pS->Value = pDevExt->m_BWMode.CurrentValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_SNAPSHOTIMAGESIZE:
			pS->Value = pDevExt->pVideoDevice->m_SnapShotSize;
			break;

#ifndef _NOEFFECT
		case KSPROPERTY_CUSTOM_PROP_EFFECT:
			pS->Value = pDevExt->pVideoDevice->m_EffectParam;
			break;

		case KSPROPERTY_CUSTOM_PROP_FRAME_FILENAME:
			wcstombs(pS->FileName, (pDevExt->pVideoDevice->m_FrameFileName + 12), sizeof(pS->FileName));
			pS->FileName[sizeof(pS->FileName) - 1] = 0;
			break;
#endif

		case KSPROPERTY_CUSTOM_PROP_WBTemperatureControlAuto:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WB_TEMPERATURE_AUTO, FALSE);
			pS->Value = pDevExt->m_WBTAuto.CurrentValue;
			pS->DefValue = pDevExt->m_WBTAuto.DefValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_WBComponentControl:
			// shawn 2011/05/27 +++++
			if(pDevExt->m_XU_WBC_Dis)
			{
				//20061228
				if (!((pDevExt->m_PropertyControls>>7)&1))	// Support Whitebalance Component
				{
					DbgPrint("KSPROPERTY_CUSTOM_PROP_WBComponentControl NOT support\n");
					ntStatus = STATUS_NOT_SUPPORTED;
					break;
				}

				//ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WB_COMPONENT, FALSE);
			}
			// shawn 2011/05/27 -----
			
			pS->Value = pDevExt->m_WBC.CurrentValue;
			pS->DefValue = pDevExt->m_WBC.DefValue;
			pS->MaxValue = pDevExt->m_WBC.MaxValue;
			pS->MinValue = pDevExt->m_WBC.MinValue;
			pS->ResValue = pDevExt->m_WBC.ResValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_WBComponentControlAuto:
			// shawn 2011/06/01 +++++
			if(pDevExt->m_XU_WBCAuto_Dis)
			{
				//20061228
				if (!((pDevExt->m_PropertyControls>>13)&1))	// Support Whitebalance Component Auto
				{
					ntStatus = STATUS_NOT_SUPPORTED;
					break;
				}
			}
			// shawn 2011/06/01 -----
			
			//ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(WB_COMPONENT_AUTO, FALSE);
			pS->Value = pDevExt->m_WBCAuto.CurrentValue;
			pS->DefValue = pDevExt->m_WBCAuto.DefValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_AEModeControl:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(AE_MODE, FALSE);
			pS->Value = pDevExt->m_AEMode.CurrentValue;
			pS->DefValue = pDevExt->m_AEMode.DefValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_AEPriorityControl:
			ntStatus = pDevExt->pVideoDevice->UpdateImageControlProperties(AE_PRIORITY, FALSE);
			pS->Value = pDevExt->m_AEPriority.CurrentValue;
			pS->DefValue = pDevExt->m_AEPriority.DefValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_PROPERTY_PAGE_CONTROL:
			pS->Value = pDevExt->m_PageCtrl;
			break;

			// shawn 2011/05/25 +++++
		case KSPROPERTY_CUSTOM_PROP_IMAGEFORMAT:
			// shawn 2011/06/27 modify +++++
			if (pDevExt->m_MPEG2StreamState == KSSTATE_RUN && pDevExt->m_StreamState != KSSTATE_RUN)
				pS->Value = pDevExt->pVideoDevice->m_ImageFormat[pDevExt->m_STREAM_Capture_MP2TS_Idx];
			else
				pS->Value = pDevExt->pVideoDevice->m_ImageFormat[STREAM_Capture];
			// shawn 2011/06/27 modify -----

			break;

		case KSPROPERTY_CUSTOM_PROP_CAPTURE_WIDTH:
			pS->Value = ImgWidList[pDevExt->pVideoDevice->m_enImageOut];
			break;

		case KSPROPERTY_CUSTOM_PROP_CAPTURE_HEIGHT:
			pS->Value = ImgHgtList[pDevExt->pVideoDevice->m_enImageOut];
			break;

		case KSPROPERTY_CUSTOM_PROP_STILLMETHOD:
			pS->Value = pDevExt->m_StillCaptureMethod;
			break;

		case KSPROPERTY_CUSTOM_PROP_ROTATION:
			pS->Value = pDevExt->m_Rotation.CurrentValue;
			pS->DefValue = pDevExt->m_Rotation.DefValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_AUXLED_MODE:
       		if (pDevExt->m_XU_AuxLED_Dis)
			{
				ntStatus = STATUS_NOT_SUPPORTED;
				break;
			}	
       		
			pS->Value = pDevExt->m_AuxLedMode.CurrentValue;
			pS->DefValue = pDevExt->m_AuxLedMode.DefValue;
			pS->MinValue = pDevExt->m_0x8400_Data[AUXLED_BRIGHT_MIN_ADDR];
			pS->MaxValue = pDevExt->m_0x8400_Data[AUXLED_BRIGHT_MAX_ADDR];
            pS->ResValue =  pDevExt->m_0x8400_Data[AUXLED_BRIGHT_RES_ADDR];
			DbgPrint("Get KSPROPERTY_CUSTOM_PROP_AUXLED_MODE(%d)\n", pS->Value);
			break;

		case KSPROPERTY_CUSTOM_PROP_SNAPSHOT_CONTROL:
			pS->Value = pDevExt->m_Snapshot_Ctrl.CurrentValue;
			pS->DefValue = pDevExt->m_Snapshot_Ctrl.DefValue;
			break;

		case KSPROPERTY_CUSTOM_PROP_AUXLED_AUTO_CONTROL:
			if(pDevExt->m_XU_AuxLEDAuto_Dis)
				pS->Value = 0;
			else
				pS->Value = 1;

			break;

		case KSPROPERTY_CUSTOM_PROP_DENOISE_MODE:
			if(!pDevExt->m_DeNoiseMode_UIEnable)
			{
				ntStatus = STATUS_NOT_SUPPORTED;
				break;
			}

			pS->Value = pDevExt->m_DeNoiseMode.CurrentValue;
			pS->DefValue = pDevExt->m_DeNoiseMode.DefValue;
			pS->MinValue = 0;
			pS->MaxValue = 3;
			pS->ResValue = 1;
			break;

		case KSPROPERTY_CUSTOM_PROP_WBComponentControl_Green:
			if(pDevExt->m_XU_WBC_Dis)
			{
				if (!((pDevExt->m_PropertyControls>>7)&1))	// Support Whitebalance Component
				{
					DbgPrint("KSPROPERTY_CUSTOM_PROP_WBComponentControl NOT support\n");
					ntStatus = STATUS_NOT_SUPPORTED;
					break;
				}
			}
			
			pS->Value = pDevExt->m_WBC_Green.CurrentValue;
			pS->MaxValue = pDevExt->m_WBC_Green.MaxValue;
			pS->MinValue = pDevExt->m_WBC_Green.MinValue;
			pS->DefValue = pDevExt->m_WBC_Green.DefValue;
			pS->ResValue = pDevExt->m_WBC_Green.ResValue;
			break;
			// shawn 2011/05/25 -----

		default:
			DBGU_WARNING("!! Invalid Id == %d\n",Id);
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
		}
	}
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_ZOOM_PROP_ST50220, ( const struct _GUID &)Property->Set)) 
	{
		DBGU_TRACE("get PROPSETID_ZOOM_PROP_ST50220 : %d, ",Id);

		PKSPROPERTY_ZOOM_PROP_S pS	= (PKSPROPERTY_ZOOM_PROP_S) data;

		switch(Id)
		{
			case KSPROPERTY_ZOOM_PROP_STATUS:
			{
				LONG temp;
				pS->Value = pDevExt->pVideoDevice->m_ZoomParam.iZoomStatus;
				/*//
				// when mirror or flip works, we need to change zoom direction status
				//
				temp = pS->Value;
				
				// check Mirror is ON
				if (pDevExt->m_Mirror.CurrentValue)
				{
					if (temp&DISABLE_RIGHT)
						pS->Value |= DISABLE_LEFT;
					else
						pS->Value &= ~DISABLE_LEFT;

					if (temp&DISABLE_LEFT)
						pS->Value |= DISABLE_RIGHT;
					else
						pS->Value &= ~DISABLE_RIGHT;
				}
				
				// check Flip is ON
				if (pDevExt->m_Flip.CurrentValue)
				{
					if (temp&DISABLE_UP)
						pS->Value |= DISABLE_DOWN;
					else
						pS->Value &= ~DISABLE_DOWN;

					if (temp&DISABLE_DOWN)
						pS->Value |= DISABLE_UP;
					else
						pS->Value &= ~DISABLE_UP;
				}
				*/
				
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_STATUS - Get(%d)\n", pS->Value);
			}

			break;

			case KSPROPERTY_ZOOM_PROP_ZOOM_IN:
				pS->Value = pDevExt->pVideoDevice->m_ZoomParam.iZoomStep;
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_ZOOM_IN - Get(%d)\n", pS->Value);
				break;

			case KSPROPERTY_ZOOM_PROP_ZOOM_IN_MAX:
				pS->Value = pDevExt->pVideoDevice->m_ZoomParam.iMaxZoom;
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_ZOOM_IN_MAX Get(%d)\n", pS->Value);
				break;

			case KSPROPERTY_ZOOM_PROP_ENABLE_OPTION:
				pS->Value = pDevExt->pVideoDevice->m_ZoomParam.bZoomEnable;
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_ENABLE - Get(%d)\n",pS->Value);
				break;

			case KSPROPERTY_ZOOM_PROP_UI_CONTROL:
				pS->Value = pDevExt->pVideoDevice->m_ZoomParam.bShowUIControl; 
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_UI_CONTROL- Get(%d)\n", pS->Value);
				break;

			case KSPROPERTY_ZOOM_PROP_PAN_MAX:
				pS->Value = pDevExt->pVideoDevice->m_ZoomParam.iHPaneIdxRange; 
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_PAN_MAX- Get(%d)\n", pS->Value);
				break;

			case KSPROPERTY_ZOOM_PROP_PAN_POS:
				pS->Value = pDevExt->pVideoDevice->m_ZoomParam.iHPaneIdx;
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_PAN_POS- Get(%d)\n", pS->Value);
				break;

			case KSPROPERTY_ZOOM_PROP_TILT_MAX:
				pS->Value = pDevExt->pVideoDevice->m_ZoomParam.iVPaneIdxRange;
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_TILT_MAX- Get(%d)\n", pS->Value);
				break;

			case KSPROPERTY_ZOOM_PROP_TILT_POS:
				pS->Value = pDevExt->pVideoDevice->m_ZoomParam.iVPaneIdx;
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_TILT_POS- Get(%d)\n", pS->Value);
				break;

			case KSPROPERTY_ZOOM_PROP_WIDTH:
				pS->Value = m_pVideoInfoHeader[STREAM_Capture]->bmiHeader.biWidth;
				break;

			case KSPROPERTY_ZOOM_PROP_MIRROR_FLIP:
				pS->Value = pDevExt->m_Mirror.CurrentValue  | ( pDevExt->m_Flip.CurrentValue << 1);
				DBGU_TRACE("KSPROPERTY_ZOOM_PROP_MIRROR_FLIP- Get(%d)\n", pS->Value);
				break;

			default:
				DBGU_ERROR("!! Invalid Id \n");
				ntStatus = STATUS_NOT_IMPLEMENTED;
				break;
		}

		pS->Flags = 0;
		pS->Capabilities = 0;
		*pActualBytesTransferred = sizeof(KSPROPERTY_ZOOM_PROP_S);
	}
	else if (IsEqualGUID(( const struct _GUID &)PROPSETID_CLIENT_PROP_ST50220, ( const struct _GUID &)Property->Set)) 
	{
		DBGU_INFO("Get PROPSETID_CLIENT_PROP_ST50220 : %d, ",Id);
		PKSPROPERTY_CLIENT_PROP_S	pS	= (PKSPROPERTY_CLIENT_PROP_S) data;
		UCHAR pData = 0;

		switch (Id)
		{
		case KSPROPERTY_CLIENT_PROP_ASICREAD:
			DBGU_TRACE("GET KSPROPERTY_CLIENT_PROP_ASICREAD\n");
			ntStatus = pDevExt->pVideoDevice->ASIC_Communication(CTL_R_NONE, 1, pS->index, &pData);
			*pActualBytesTransferred = sizeof(KSPROPERTY_CLIENT_PROP_S);
			break;

		default:
			DBGU_WARNING("!! Invalid Id == %d\n",Id);
			ntStatus = STATUS_NOT_IMPLEMENTED;
			break;
		}
	}
	else 
	{
		DBGU_INFO("Get EXunit control : %d, ",Id);
		BOOLEAN bXU_Ctrl=FALSE;
		int i;

		for(i=0;i<pDevExt->m_ExUnitNum;i++)
		{
			if (IsEqualGUID(( const struct _GUID &)pDevExt->m_ExUnitGUID[i], ( const struct _GUID &)Property->Set))
			{
				PBYTE pS = (PBYTE) data;
				PUCHAR pData=NULL;
				ULONG Value;
				ULONG Index;
				ULONG BufferLength;
				UCHAR Flags = 0x81;	// for Get

				bXU_Ctrl = TRUE;

				Value = Id<<8;
				Index = pDevExt->m_ExUnitID[i]<<8;
				BufferLength = pDevExt->m_ExUnitParamLength[i][Id-1];
				pData = new (NonPagedPool) UCHAR[BufferLength];
				
				if ((pData==NULL)||(PropertyOutputSize == 0))
				{
					//2010/5/11 04:18下午
					DBGU_TRACE("return EX_Unit length = %d\n",BufferLength);
					*pActualBytesTransferred = BufferLength;
					
					if (pData)
						delete pData;
					
					return STATUS_BUFFER_OVERFLOW;
					//break;
				}

				RtlZeroMemory(pData, BufferLength);

				ntStatus = pDevExt->pVideoDevice->USB_ControlClassCommand(
					Flags,
					Value,
					Index,
					pData,
					BufferLength,
					TRUE);

				if( NT_SUCCESS(ntStatus))
				{
					RtlCopyMemory(pS, pData, BufferLength);
					DBGU_TRACE("Michael Get XU_Ctrl ID = %d\n", Id);
					DBGU_TRACE("Michael Get XU_Ctrl len = %d\n", BufferLength);
					
					for(int k=0;k<BufferLength;k++)
						DBGU_TRACE("Michael Get XU_Ctrl data[%d] = %X", k, pData[k]);
				}

				DBGU_TRACE("Michael Get XU_Ctrl return %X\n", ntStatus);

				if (pData)
					delete pData;
				
				//2010/5/11 04:18下午
				*pActualBytesTransferred = BufferLength;
				break;
			}
		}

		if (!bXU_Ctrl)
		{
			DBGU_WARNING("Michael GetDeviceProperty:We did not support this device property Id:%d!\n",Property->Id);
			ntStatus = STATUS_NOT_IMPLEMENTED;
		}
  }

	DBGU_TRACE("GetDeviceProperty: return status: 0x%X!\n",ntStatus);
	return ntStatus;
}

NTSTATUS CVideoStream::GetStreamProperty(IN PIRP irp, IN PKSPROPERTY Property, IN OUT PVOID data)
{
	DBGU_TRACE("CVideoStream::GetStreamProperty()");

	PDEVEXT pDevExt = CurrentDevExt();
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PKSPIN pin = KsGetPinFromIrp(irp);
	PIO_STACK_LOCATION ioStack = IoGetCurrentIrpStackLocation(irp);

	if (!pin || !pDevExt || !ioStack) {
		DBGU_ERROR("Null ioStack or pin or Device extension at GetStreamProperty().\n");
		return STATUS_UNSUCCESSFUL;
	}

	PULONG_PTR pActualBytesTransferred = &(irp->IoStatus.Information);
	ULONG PropertyOutputSize = ioStack->Parameters.DeviceIoControl.OutputBufferLength; // size of data buffer
	ULONG StreamNumber = pin->Id;

	DBGU_TRACE(" m_StreamInstances[%d]=%d\n", StreamNumber, m_StreamInstances[StreamNumber]);

	//2010/6/30 02:05下午 H264
	if(!((StreamNumber<MAX_NUM_STREAM_INFO) && m_StreamInstances[StreamNumber]))
	//if(!((StreamNumber<MAX_NUM_STREAM_INFO) && m_StreamInstances[StreamNumber] && m_pVideoInfoHeader[StreamNumber]))
	{//Check the validation of the stream
		DBGU_TRACE("ntStatus = STATUS_INVALID_PARAMETER StreamNumber = %d\n",StreamNumber);
		ntStatus = STATUS_INVALID_PARAMETER;
	}//Check the validation of the stream
	else if(IsEqualGUID((const struct _GUID &)KSPROPSETID_Connection, (const struct _GUID &)Property->Set)) 
	{//KSPROPSETID_Connection
		DBGU_TRACE("Get KSPROPSETID_Connection : %d\n",Property->Id);
		
		switch(Property->Id)
		{
			case KSPROPERTY_CONNECTION_ALLOCATORFRAMING:
			{
				if (PropertyOutputSize == 0)
				{
					ntStatus = STATUS_BUFFER_OVERFLOW;
					DBGU_TRACE("STATUS_BUFFER_OVERFLOW !\n");
				}
				else if (PropertyOutputSize < sizeof (KSALLOCATOR_FRAMING))
				{
					DBGU_TRACE("STATUS_BUFFER_TOO_SMALL !\n");
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}
				else
				{
					PKSALLOCATOR_FRAMING pFraming = (PKSALLOCATOR_FRAMING) data;
					RtlZeroMemory(pFraming, sizeof(KSALLOCATOR_FRAMING));
					pFraming->RequirementsFlags = (KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY | KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER | KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY);
					pFraming->PoolType = PagedPool;
					pFraming->Frames	= 2;
					
					//2010/6/30 03:01下午
					if(StreamNumber == STREAM_Capture)
						pFraming->FrameSize = m_pVideoInfoHeader[StreamNumber]->bmiHeader.biSizeImage;
					else
						pFraming->FrameSize = 1880;//0x13000;//792;//
				}

				*pActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING);
				break;
			}

			//2010/7/20 06:03下午 skype
			case KSPROPERTY_CONNECTION_ALLOCATORFRAMING_EX:
			{
				if (PropertyOutputSize == 0)
				{
					ntStatus = STATUS_BUFFER_OVERFLOW;
					DBGU_TRACE("STATUS_BUFFER_OVERFLOW !\n");
				}
				else if (PropertyOutputSize < sizeof (KSALLOCATOR_FRAMING_EX))
				{
					DBGU_TRACE("STATUS_BUFFER_TOO_SMALL !\n");
					ntStatus = STATUS_BUFFER_TOO_SMALL;
				}
				else
				{
					PKSALLOCATOR_FRAMING_EX Framing = (PKSALLOCATOR_FRAMING_EX) data;
					RtlZeroMemory(Framing, sizeof(KSALLOCATOR_FRAMING_EX));
					
					Framing =
					const_cast <PKSALLOCATOR_FRAMING_EX> (
						pin -> Descriptor -> AllocatorFraming
						);
					Framing -> FramingItem [0].Frames = 2;

					//
					// The physical and optimal ranges must be biSizeImage.  We only
					// support one frame size, precisely the size of each capture
					// image.
					//
					Framing -> FramingItem [0].PhysicalRange.MinFrameSize =
						Framing -> FramingItem [0].PhysicalRange.MaxFrameSize =
						Framing -> FramingItem [0].FramingRange.Range.MinFrameSize =
						Framing -> FramingItem [0].FramingRange.Range.MaxFrameSize =
						1880;//0x13000;//792;
						//0x96000;	// james define.
	
					Framing -> FramingItem [0].PhysicalRange.Stepping = 
						Framing -> FramingItem [0].FramingRange.Range.Stepping =
						0;
				}

				*pActualBytesTransferred = sizeof (KSALLOCATOR_FRAMING_EX);
				break;
			}

		default:
			ntStatus = STATUS_NOT_IMPLEMENTED;	// Did not implement this property item.
			break;
		}//switch(Property->Id)

    }//KSPROPSETID_Connection 
	else if(IsEqualGUID((const struct _GUID &)PROPSETID_VIDCAP_DROPPEDFRAMES, (const struct _GUID &)Property->Set)) 
	{//PROPSETID_VIDCAP_DROPPEDFRAMES

		DBGU_TRACE("Get PROPSETID_VIDCAP_DROPPEDFRAMES : %d\n",Property->Id);

		switch(Property->Id)
		{
		case KSPROPERTY_DROPPEDFRAMES_CURRENT:
		{
			if (PropertyOutputSize == 0)
			{
				ntStatus = STATUS_BUFFER_OVERFLOW;
				DBGU_TRACE("STATUS_BUFFER_OVERFLOW !\n");
			}
			else if (PropertyOutputSize < sizeof (KSALLOCATOR_FRAMING))
			{
				DBGU_TRACE("STATUS_BUFFER_TOO_SMALL !\n");
				ntStatus = STATUS_BUFFER_TOO_SMALL;
			}
			else
			{
				PKSPROPERTY_DROPPEDFRAMES_CURRENT_S pDroppedFrames = (PKSPROPERTY_DROPPEDFRAMES_CURRENT_S) data;
				
				// james 2011/03/22 modify.
				if(!m_FirstFrameStartTime) {
					pDroppedFrames->PictureNumber		= 0;
					pDroppedFrames->DropCount			= 0;
					
					// shawn 2011/07/04 modify +++++
					/*//2011/5/9 02:46下午
					pDroppedFrames->AverageFrameSize	= pDevExt->pVideoDevice->m_MPEG2ReadBufferLength;//m_pVideoInfoHeader[StreamNumber]->bmiHeader.biSizeImage;*/
					
					if(StreamNumber == STREAM_Capture)
					{
						if(m_pVideoInfoHeader[StreamNumber]->bmiHeader.biCompression == MEDIASUBTYPE_MJPG.Data1)
							pDroppedFrames->AverageFrameSize = pDevExt->pVideoDevice->m_MJPEGReadBufferLength;
						else
							pDroppedFrames->AverageFrameSize = m_pVideoInfoHeader[StreamNumber]->bmiHeader.biSizeImage;
					}
					else
						pDroppedFrames->AverageFrameSize = pDevExt->pVideoDevice->m_MPEG2ReadBufferLength;
					// shawn 2011/07/04 modify -----
				}
				else {
					pDroppedFrames->PictureNumber		= m_PictureNumber;
					pDroppedFrames->DropCount			= m_DropCount;
					
					// shawn 2011/07/04 modify +++++
					//pDroppedFrames->AverageFrameSize	= pDevExt->pVideoDevice->m_MPEG2ReadBufferLength;//m_pVideoInfoHeader[StreamNumber]->bmiHeader.biSizeImage;

					if(StreamNumber == STREAM_Capture)
					{
						if(m_pVideoInfoHeader[StreamNumber]->bmiHeader.biCompression == MEDIASUBTYPE_MJPG.Data1)
							pDroppedFrames->AverageFrameSize = pDevExt->pVideoDevice->m_MJPEGReadBufferLength;
						else
							pDroppedFrames->AverageFrameSize = m_pVideoInfoHeader[StreamNumber]->bmiHeader.biSizeImage;
					}
					else
						pDroppedFrames->AverageFrameSize = pDevExt->pVideoDevice->m_MPEG2ReadBufferLength;
					// shawn 2011/07/04 modify -----
				}
			}

            *pActualBytesTransferred = sizeof (KSPROPERTY_DROPPEDFRAMES_CURRENT_S);
			
			DBGU_TRACE("(DBG0922)GET DROP COUNT ---> m_PictureNumber : %d\n",m_PictureNumber);
			break;
		}

		default:
			ntStatus = STATUS_NOT_IMPLEMENTED;	// Did not implement this property item.
			break;
		}//switch(Property->Id)
    
	}//PROPSETID_VIDCAP_DROPPEDFRAMES 
	else 
	{
		DBGU_WARNING("GetStreamProperty, We did not support this stream property ID:%d!\n",Property->Id);
		ntStatus = STATUS_NOT_IMPLEMENTED;
    }

	DBGU_TRACE("GetStreamProperty: return status: 0x%X!\n",ntStatus);
	return ntStatus;
}

NTSTATUS
CVideoStream::ReadData(
	IN PKSPIN pin,
	PKSSTREAM_HEADER pDataPacket,
	IN ULONG NumberOfBuffers
	)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PDEVEXT pDevExt = CurrentDevExt();
	PKS_VIDEOINFOHEADER	pVideoInfoHeader = m_pVideoInfoHeader[pin->Id];
	REFERENCE_TIME	aft;

	DBGU_INFO("---- Enter CStreamHandler::ReadData() STreamNum=%d NumberOfBuffers=%d\n",pin->Id, NumberOfBuffers);
	
	if (!pDevExt) {
		DBGU_ERROR("Null Device extension at ReadData().\n");
		return STATUS_UNSUCCESSFUL;
	}

	// Save Pin ID ( Capture Pin or Still Pin? )
	ULONG StreamNumber = pin->Id;

	//2010/4/21 05:18下午
	if(StreamNumber == STREAM_Capture)
		aft = pVideoInfoHeader->AvgTimePerFrame;
		
	// System wake up from D3 power state - handle standby or hibernate condition
	if(m_WakeUp_Run && (pDevExt->DevicePowerState == PowerDeviceD0))// && (m_StreamState != KSSTATE_RUN))
	{// VidCap Capture / Suspend or Hibernate
		DBGU_TRACE("(ReadData) WakeUp_Run !\n");
		m_WakeUp_Run = FALSE;

		//2010/6/29 06:01下午 H264
		if(StreamNumber == STREAM_Capture)
			pDevExt->m_StreamState = KSSTATE_RUN;
		else
			pDevExt->m_MPEG2StreamState = KSSTATE_RUN;	
		
		ntStatus = pDevExt->pVideoDevice->Run(*pVideoInfoHeader, pin);
		
		if (!NT_SUCCESS(ntStatus))
			return ntStatus;
	}

	//2010/6/29 06:02下午 H264
	if((pDevExt->m_StreamState != KSSTATE_RUN && StreamNumber == STREAM_Capture)||(pDevExt->m_MPEG2StreamState != KSSTATE_RUN && StreamNumber == pDevExt->m_STREAM_Capture_MP2TS_Idx))	// shawn 2011/06/23 modify
	//if(pDevExt->m_StreamState != KSSTATE_RUN)
	{	
		// Handle surprise removal condition
		if(m_SurpriseRemoval && !pDevExt->pVideoDevice->GetUsbIoCount())
		{
			DBGU_INFO("ReadData() -> m_SurpriseRemoval == true & UsbIoCount == 0 !\n");
			return STATUS_UNSUCCESSFUL;
		}

		pDataPacket->DataUsed =	0;
	
		return STATUS_CANCELLED;

	}//if(m_StreamState != KSSTATE_RUN)

	//
	// process all data buffers
	//
    PKS_FRAME_INFO      pFrameInfo;
	LARGE_INTEGER		StartTime_DataPacket, EndTime_DataPacket, PresentTime_DataPacket, DelayTime_DataPacket;
	REFERENCE_TIME		TmpDuration;
	BOOL				bReadyToReadData;
	BOOL				bFirstFrame;
	LONGLONG			DropCountFake;

	for(ULONG i = 0; i < NumberOfBuffers; i++, pDataPacket++) {
		pFrameInfo = (PKS_FRAME_INFO) (pDataPacket + 1);

		//2010/6/10 05:18下午
		pDevExt->pVideoDevice->m_APBufferLen[StreamNumber] = pDataPacket->FrameExtent;
		DBGU_TRACE("APBufferLen[%d] = %d, pDataPacket->FrameExtent = %d\n", StreamNumber, pDevExt->pVideoDevice->m_APBufferLen[StreamNumber],pDataPacket->FrameExtent);

		//2010/4/21 04:00下午 h264
		if(StreamNumber != pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
		{
			// dismatched frame size
			if (pDataPacket->FrameExtent < pVideoInfoHeader->bmiHeader.biSizeImage)
			{
				DBGU_WARNING("[DEBUG]AP request length = %ld, Driver data length = %ld\n",pDataPacket->FrameExtent,pVideoInfoHeader->bmiHeader.biSizeImage);
				pDataPacket->DataUsed = 0;
				pDataPacket->OptionsFlags = KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
				pFrameInfo->DropCount = m_DropCount;
				pFrameInfo->PictureNumber = m_PictureNumber;
				continue;
			}
		}
		//2010/7/20 03:05下午 H264
		/*else
		{
			// dismatched frame size
			if (pDataPacket->FrameExtent < 1000)
			{
					DBGU_WARNING("[DEBUG]AP request length = %ld, Driver data length = %ld\n",pDataPacket->FrameExtent,pVideoInfoHeader->bmiHeader.biSizeImage);
					pDataPacket->DataUsed = 0;
					pDataPacket->OptionsFlags = KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
					pFrameInfo->DropCount = m_DropCount;
					pFrameInfo->PictureNumber = m_PictureNumber;
					continue;
			}
		}*/
		//
		// initialization per data buffer
		//

		// shawn 2011/06/27 fix MP2TS crash +++++
		if (pVideoInfoHeader)
			pDataPacket->Duration		= pVideoInfoHeader->AvgTimePerFrame/*0*/;	// shawn 2011/06/09 modify for fixing graphedit issue
		else
			pDataPacket->Duration		= 0;
		// shawn 2011/06/27 fix MP2TS crash -----

		pDataPacket->OptionsFlags	= 0;
		KeQuerySystemTime(&StartTime_DataPacket);

		bFirstFrame = FALSE;
		pDataPacket->PresentationTime.Numerator		= 1;//For a non-scaled value, this should be one. Numerator must not be zero.
		pDataPacket->PresentationTime.Denominator	= 1;//For a non-scaled value, this should be one. Denominator must not be zero. 
		
		if(!m_FirstFrameStartTime) {
			DBGU_TRACE("m_FirstFrameStartTime !\n");
			bFirstFrame = TRUE;
		}

		//
		// For VMR Decompressor of YUY2, it require width byte is 128*n, 
		// If format is YUY2 and resolution is 160x120, 176x144 and 352x288, 
		// the gived buffer size will large than SizeImage.
		// we need to do modification to match it.
		//
		//2010/4/21 04:00下午 h264
		if(StreamNumber == pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
		{
		}
		//2010/8/4 10:17上午
		else if (bFirstFrame && StreamNumber == STREAM_Capture)
		{
			PKS_BITMAPINFOHEADER pBmiHeader = &(pVideoInfoHeader->bmiHeader);
			ULONG height = pBmiHeader->biHeight;
			ULONG srcWByte = pBmiHeader->biWidth * pBmiHeader->biBitCount/8;
			ULONG destWByte = pDataPacket->FrameExtent/height;
			
			if ( ((srcWByte%128) != 0) && ((destWByte%128) == 0) && srcWByte<destWByte)
				pDevExt->m_ImgSizeLarger = true;
			else
				pDevExt->m_ImgSizeLarger = false;
		}

		//
		// start to read data
		//
		if (StreamNumber == STREAM_Capture)
			ntStatus = pDevExt->pVideoDevice->Read((PUCHAR) pDataPacket->Data, pVideoInfoHeader, aft, bFirstFrame);
		else if (StreamNumber == pDevExt->m_STREAM_Still_Idx)			// shawn 2011/06/23 modify
			ntStatus = pDevExt->pVideoDevice->ReadStill((PUCHAR) pDataPacket->Data, pVideoInfoHeader);
		//2010/4/21 04:00下午 h264
		else if(StreamNumber == pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
		{
			ntStatus = pDevExt->pVideoDevice->Read((PUCHAR) pDataPacket->Data, NULL, aft, 0);
		}

		if (NT_SUCCESS(ntStatus))
		{
			if(StreamNumber == pDevExt->m_STREAM_Capture_MP2TS_Idx)		// shawn 2011/06/23 modify
			{
				//pDataPacket->DataUsed = 40*188;
				pDataPacket->DataUsed = pDevExt->pVideoDevice->m_MPEG2ReadBufferLength;	// james try M2TS.
				pDataPacket->OptionsFlags = KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;	
				DBGU_WARNING("STREAM_Capture_MP2TS pDataPacket->DataUsed =%d\n",pDataPacket->DataUsed);

				goto _READDATA_EXIT;		
			}
			else
			{
				DBGU_WARNING("STREAM_Capture pDataPacket->FrameExtent =%d\n",pDataPacket->FrameExtent);

				// shawn 2011/07/04 modify +++++
				/*pDataPacket->DataUsed = pVideoInfoHeader->bmiHeader.biSizeImage;
				
				//2011/5/3 05:54下午
				if(pVideoInfoHeader->bmiHeader.biCompression == MEDIASUBTYPE_MJPG.Data1)
				{
					pDataPacket->DataUsed = pDevExt->pVideoDevice->pMPEG2TSFileBufferStart;
					DBGU_WARNING("MJ pDataPacket->DataUsed =%d\n",pDataPacket->DataUsed);				
				}*/

				if(pVideoInfoHeader->bmiHeader.biCompression == MEDIASUBTYPE_MJPG.Data1)
				{
					pDataPacket->DataUsed = pDevExt->pVideoDevice->m_MJPEGReadBufferLength;
					DBGU_WARNING("MJ pDataPacket->DataUsed =%d\n",pDataPacket->DataUsed);
				}
				else
					pDataPacket->DataUsed = pVideoInfoHeader->bmiHeader.biSizeImage;
				// shawn 2011/07/04 modify -----
				
				//if (bFirstFrame || pVideoInfoHeader->bmiHeader.biCompression == MEDIASUBTYPE_MJPG.Data1)
				pDataPacket->OptionsFlags = KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;
			}
		}
		else
		{
			DBGU_WARNING("We can't receive any good frame..\n");
			pDataPacket->DataUsed = 0;
			pDevExt->pVideoDevice->m_MJPEGReadBufferLength = 0;		// shawn 2011/07/04 add
			pDataPacket->OptionsFlags = KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
			
			// james try M2TS.
			if(StreamNumber == pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
			{
				pDevExt->pVideoDevice->m_MPEG2ReadBufferLength = 0;
				goto _READDATA_EXIT;
			}
		}

		//
		// calc picture number and drop count
		//
		KeQuerySystemTime(&PresentTime_DataPacket);
		pDataPacket->PresentationTime.Time = PresentTime_DataPacket.QuadPart;

		//2010/6/29 04:47下午 
		/*(if (StreamNumber == STREAM_Still)
		{
			if (NT_SUCCESS(ntStatus))
			{
				if (pDevExt->m_bReStartRun)
				{
					DBGU_WARNING("Restart capture pin.. \n");
					ntStatus = pDevExt->pVideoDevice->Stop();
					if (!NT_SUCCESS(ntStatus))
						return ntStatus;
					// re-start captrue pin
					ntStatus = pDevExt->pVideoDevice->Run(*m_pVideoInfoHeader[STREAM_Capture], pin);
				}
			}

			return ntStatus;
		}*/

		if (bFirstFrame) {
			pFrameInfo->PictureNumber = 0;
			pFrameInfo->DropCount = 0;
			DropCountFake = 0;

			m_FirstFrameStartTime = pDataPacket->PresentationTime.Time;
			m_PresentationTimeLast = pDataPacket->PresentationTime.Time;
			DBGU_INFO("FirstFrame!!! PresentTime:%I64d\n",m_FirstFrameStartTime);
		}
		//2010/8/4 10:18上午
		else if(StreamNumber == STREAM_Capture){
			pFrameInfo->PictureNumber = (pDataPacket->PresentationTime.Time - m_FirstFrameStartTime) / aft;
			
			if (pFrameInfo->PictureNumber < 0) {
				pFrameInfo->PictureNumber = 0;
			}

			if (pFrameInfo->PictureNumber <= m_PictureNumber) {
				pFrameInfo->PictureNumber = m_PictureNumber + 1;
			}

			pFrameInfo->DropCount = m_DropCount;
			pFrameInfo->DropCount += (pFrameInfo->PictureNumber - m_PictureNumber - 1);
			DropCountFake = pFrameInfo->DropCount - (2 * m_PictureNumber * aft) / 10000000;
			
			if (DropCountFake < m_DropCountFake)
				DropCountFake = m_DropCountFake;

			if ((m_PictureNumber + 1) == pFrameInfo->PictureNumber) {
				PresentTime_DataPacket.QuadPart = m_FirstFrameStartTime + (aft * pFrameInfo->PictureNumber);
				TmpDuration = pDataPacket->PresentationTime.Time - PresentTime_DataPacket.QuadPart;
				
				if (TmpDuration < -166667) {
					pDataPacket->PresentationTime.Time = PresentTime_DataPacket.QuadPart;

					DelayTime_DataPacket.QuadPart = TmpDuration;
					DBGU_TRACE("[DEBUG]DelayTime_DataPacket:%I64d\n", DelayTime_DataPacket.QuadPart);
					KeDelayExecutionThread(KernelMode, FALSE, &DelayTime_DataPacket);
				}
			}

			m_PresentationTimeLast = pDataPacket->PresentationTime.Time;
		}

		m_DropCount = pFrameInfo->DropCount;
		m_PictureNumber = pFrameInfo->PictureNumber;
		m_DropCountFake = DropCountFake;
		DBGU_INFO("DataUsed:%lu, aft:%I64d, PresentTime:%I64d, PictureNumber:%I64d, DropCount:%I64d, DropCountFake:%I64d\n",pDataPacket->DataUsed, aft, PresentTime_DataPacket.QuadPart, pFrameInfo->PictureNumber, pFrameInfo->DropCount, m_DropCountFake);
	} //for(i)

_READDATA_EXIT:
	DBGU_INFO("---- End CStreamHandler::ReadData() StreamNum=%d---- (%d)\n",pin->Id, pDevExt->pVideoDevice->GetUsbIoCount());

	return ntStatus;
}

VOID
SubmitCurrentSettings(PVOID Context)
{
	/*
	PDEVEXT pDevExt;
	pDevExt = (PDEVEXT)DeferredContext;
	pDevExt->pVideoStream->CurrentSettings(TRUE, TRUE);
	pDevExt->pVideoStream->StopDPC = FALSE;*/
	PInterrupt_WORKITEM workitem = (PInterrupt_WORKITEM)Context;
	PDEVEXT pDevExt = (PDEVEXT)workitem->pDevExt;
	DBGU_TRACE("SubmitCurrentSettings()");
	pDevExt->pVideoStream->CurrentSettings(TRUE, TRUE);
	pDevExt->pVideoStream->ReadyToSetCurSetting = true;
	ExFreePool(workitem);	
}

NTSTATUS
CVideoStream::ChangePowerState(
	IN DEVICE_POWER_STATE  CurrentState,
	IN DEVICE_POWER_STATE  PreviousState
	)
{
	DBGU_TRACE("CVideoStream::ChangePowerState()");

	PDEVEXT pDevExt = CurrentDevExt();
	NTSTATUS ntStatus = STATUS_SUCCESS;
	LARGE_INTEGER dueTime;
	
	if (!pDevExt) {
		DBGU_ERROR("Null Device extension at ChangePowerState().\n");
		ntStatus = STATUS_UNSUCCESSFUL;

		return ntStatus;
	}

	if(pDevExt->DevicePowerState != CurrentState)
	{
		pDevExt->DevicePowerState = CurrentState;

		switch (CurrentState) 
		{
		case PowerDeviceD0:
			//2010/8/23 05:05下午
			if(0)//(pDevExt->m_StreamState != KSSTATE_RUN)
			{
				pDevExt->pVideoDevice->StopInterruptPipe();
				pDevExt->pVideoDevice->StartInterruptTransfer();
			}
			
			//2011/5/19 10:07上午
			//CurrentSettings(TRUE, TRUE);
			ReadyToSetCurSetting = true;
			
			if(0)//(ReadyToSetCurSetting)
			{
				ReadyToSetCurSetting = false;
				PInterrupt_WORKITEM workitem;
				workitem = (PInterrupt_WORKITEM)ExAllocatePool(NonPagedPool,sizeof(Interrupt_WORKITEM));
	 			
				if (workitem) 
				{
			   		ExInitializeWorkItem(&workitem->WorkItem,
												 SubmitCurrentSettings,
												 workitem);
				
					workitem->pDevExt = pDevExt;	
					ExQueueWorkItem(&workitem->WorkItem, DelayedWorkQueue );
				} 
			}

			/*if(!StopDPC)
			{
				StopDPC = TRUE;
				KeInitializeTimerEx(&Timer,SynchronizationTimer);
				KeInitializeDpc(&Dpc,
				                 SubmitCurrentSettingsDpc,
				                 pDevExt);
												
				dueTime.QuadPart = -50000000; //5 sec
				KeSetTimer(&Timer,
										dueTime,
										&Dpc);
			}*/	

			break;

		case PowerDeviceD1:
		case PowerDeviceD2:	// for Vista selective suspend
		case PowerDeviceD3:
			if(pDevExt->m_StreamState == KSSTATE_RUN)
			{// VidCap
				DBGU_TRACE("Device is prevewing before sleep\n");
				m_WakeUp_Run = FALSE; // Metro app preview black imag after resume from S3

			}
			else
				m_WakeUp_Run = FALSE;
			
			//2010/6/29 04:47下午
			//2010/8/16 11:22上午
			if(pDevExt->m_StreamState != KSSTATE_STOP)
				ntStatus = pDevExt->pVideoDevice->Stop(STREAM_Capture);
			
			if(pDevExt->m_MPEG2StreamState != KSSTATE_STOP)
				ntStatus = pDevExt->pVideoDevice->Stop(pDevExt->m_STREAM_Capture_MP2TS_Idx);	// shawn 2011/06/23 modify
			
			break;
		}
	}
	else
		DBGU_TRACE("ChangePowerState to the same state %d\n", (CurrentState-1));

	DBGU_TRACE("CVideoStream::ChangePowerState() End");
	return ntStatus;
}

///////////////////////////////////////////////////////////////////////////////////////
//           For Sonix custom define functions                                       //
///////////////////////////////////////////////////////////////////////////////////////

NTSTATUS CVideoStream::SetImageFormat()
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PDEVEXT pdx = CurrentDevExt();
	int i,j;
	ULONG	buffer= 0;
	ULONG	BufferLength;
	ULONG	Index;
	ULONG	Value;
	LONGLONG temp;
	UINT	ImageCount=0;
	UINT	nImgNumModify=0;
	//2010/7/28 05:04下午
	UINT	nPinDescriptorCount = 0;

	// shawn 2011/06/15 +++++
	BOOL Disable_RGB24 = 0;
	BOOL Disable_I420 = 0 ;
	BOOL Disable_YUY2 = 0;
	BOOL Disable_MJPEG = 0;
	// shawn 2011/06/15 -----

	////////////////////////////////////
	//	For Images Format Settings..  //
	////////////////////////////////////
	pdx->m_Streams[STREAM_Capture].NumberOfFormatArrayEntries = pdx->m_VideoImageFormatCount;
	DBGU_TRACE("No. of format array entries : %d\n",pdx->m_Streams[STREAM_Capture].NumberOfFormatArrayEntries);

	for (i=0;i<MAX_STREAM_FORMAT;i++)
	{
		pdx->m_StreamFormatsArray[i]=NULL;
		pdx->m_StreamFormatsArray_MP2TS[i]=NULL;
		pdx->m_StillStreamFormatsArray[i]=NULL;
	}

	// initialize default Max Available FrameRate
	pdx->m_FrameRateInfo.CurrentMaxAvailableFrameRate = pdx->m_FrameRateInfo.SupportedFrameRates[pdx->m_DefaultFrameIndex[STREAM_Capture]-1];
	DBGU_TRACE("m_DefaultFrameIndex=%d, CurrentMaxAvailableFrameRate=%d\n" , pdx->m_DefaultFrameIndex[STREAM_Capture], pdx->m_FrameRateInfo.CurrentMaxAvailableFrameRate);
	
	//
	// Initialize Capture Pin Stream formats
	//

	// find default format
	pdx->m_StreamFormatsArray[0] = (PKSDATAFORMAT) &pdx->m_drImageFormat[pdx->m_DefaultFrameIndex[STREAM_Capture]-1];
	
	// other frame format
	for (i=1,j=0;i<pdx->m_VideoImageFrameNumber[STREAM_Capture];j++)
	{
		if (j==(pdx->m_DefaultFrameIndex[STREAM_Capture]-1))
			continue;

		pdx->m_StreamFormatsArray[i] = (PKSDATAFORMAT) &pdx->m_drImageFormat[j];
		DBGU_TRACE("==> %d vs %d\n",(i),(j));
		i++;
	}

	ImageCount = pdx->m_VideoImageFrameNumber[STREAM_Capture];
	nImgNumModify = ImageCount;

	DBGU_TRACE("m_YUY2_FormatCount=%d, ",pdx->m_YUY2_FormatCount);
	DBGU_TRACE("m_YUY2_FrmaeRateCount=%d\n",pdx->m_YUY2_FrmaeRateCount);
											
	/*StreamFormatsArray_MP2TS[0] = (PKSDATAFORMAT) &drImageFormat_MP2TS[0];	
	CaptureFilterPinDescriptors[STREAM_Capture_MP2TS].PinDescriptor.DataRangesCount = 1;
	Streams[STREAM_Capture_MP2TS].NumberOfFormatArrayEntries = 1;*/
	
	//2010/12/27 03:42下午 read SF for resolution and frame rate
	DBGU_TRACE("pdx->pVideoDevice->Be50286 = %d\n",pdx->pVideoDevice->BeST5R286);

	if(pdx->pVideoDevice->BeST5R286)
	{
		DBGU_TRACE("SN9C286!\n");
		pdx->pVideoDevice->GetExtensionFrameRate();
	}
	
	//2010/11/10 02:34下午 test
	// ony add RGB24 and I420 when high speed
	if/*(0)*/(pdx->pVideoDevice->m_fUSBHighSpeed)
	{
		// shawn 2011/06/15 +++++
		RTL_QUERY_REGISTRY_TABLE *Table=NULL;
		NTSTATUS ntStatus_temp = STATUS_SUCCESS;
		UNICODE_STRING	RegPath;
		LONG	buffer = 0; // james 2009/06/11 initialize the value.
		RtlInitUnicodeString(&RegPath, (PWSTR) pdx->pVideoDevice->m_DefValuePath);
		Table = (RTL_QUERY_REGISTRY_TABLE *) ExAllocatePool(NonPagedPool, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));

		RtlZeroMemory (Table, 2*sizeof(RTL_QUERY_REGISTRY_TABLE)); 
		//Disable_RGB24	
		Table[0].Name			= L"Disable_RGB24";
		Table[0].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
		Table[0].EntryContext	= &buffer;
		ntStatus = RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
												RegPath.Buffer,
												Table,
												NULL, 
												NULL);
		if(NT_SUCCESS(ntStatus))
			Disable_RGB24 = buffer;
		else
			Disable_RGB24 = 0;
			
		RtlZeroMemory (Table, 2*sizeof(RTL_QUERY_REGISTRY_TABLE)); 
		//Disable_I420	
		Table[0].Name			= L"Disable_I420";
		Table[0].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
		Table[0].EntryContext	= &buffer;
		ntStatus = RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
												RegPath.Buffer,
												Table,
												NULL, 
												NULL);
		if(NT_SUCCESS(ntStatus))		
			Disable_I420 = buffer;
		else
			Disable_I420 = 0;

		RtlZeroMemory (Table, 2*sizeof(RTL_QUERY_REGISTRY_TABLE)); 
		//Disable_YUY2	
		Table[0].Name			= L"Disable_YUY2";
		Table[0].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
		Table[0].EntryContext	= &buffer;
		ntStatus = RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
												RegPath.Buffer,
												Table,
												NULL, 
												NULL);
		if(NT_SUCCESS(ntStatus))		
			Disable_YUY2 = buffer;
		else
			Disable_YUY2 = 0;

		//2008/9/8 10:18上午
		RtlZeroMemory (Table, 2*sizeof(RTL_QUERY_REGISTRY_TABLE)); 
		//Disable_MJPEG	
		Table[0].Name			= L"Disable_MJPEG";
		Table[0].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
		Table[0].EntryContext	= &buffer;
		ntStatus = RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
												RegPath.Buffer,
												Table,
												NULL, 
												NULL);
		if(NT_SUCCESS(ntStatus))		
			Disable_MJPEG = buffer;
		else
			Disable_MJPEG = 1;				 		 			
		ExFreePoolWithTag(Table, 'FREE');
		// shawn 2011/06/15 -----
		
		j = pdx->m_YUY2_FormatCount;

		if (!Disable_RGB24)	// shawn 2011/06/15 add
		{
			for (i=0;i<ImageCount;i++)
			{
				// only add as YUY2
				if (pdx->m_drImageFormat[i].VideoInfoHeader.bmiHeader.biBitCount != 16)
					continue;

				// copy formats for RGB24
				RtlCopyMemory(&pdx->m_drImageFormat[i+ImageCount], &pdx->m_drImageFormat[i], sizeof(KS_DATARANGE_VIDEO));
				pdx->m_drImageFormat[i+ImageCount].DataRange.SubFormat = MEDIASUBTYPE_RGB24;
				pdx->m_drImageFormat[i+ImageCount].bTemporalCompression=FALSE;
				pdx->m_drImageFormat[i+ImageCount].VideoInfoHeader.bmiHeader.biBitCount=24;
				pdx->m_drImageFormat[i+ImageCount].VideoInfoHeader.bmiHeader.biCompression=KS_BI_RGB;//MEDIASUBTYPE_RGB24.Data1;
				pdx->m_drImageFormat[i+ImageCount].VideoInfoHeader.bmiHeader.biSizeImage=
					pdx->m_drImageFormat[i].VideoInfoHeader.bmiHeader.biWidth*pdx->m_drImageFormat[i].VideoInfoHeader.bmiHeader.biHeight*3;
				RtlCopyMemory(&pdx->StreamFormatArray[i+ImageCount],&pdx->StreamFormatArray[i],sizeof(VIDEOSTREAMINFO));
				pdx->StreamFormatArray[i+ImageCount].ImageSize = pdx->m_drImageFormat[i].VideoInfoHeader.bmiHeader.biSizeImage;
				pdx->StreamFormatArray[i+ImageCount].ImageFormat = IMG_FMT_RGB24;
				pdx->StreamFormatArray[i+ImageCount].biCompression = KS_BI_RGB;
				pdx->m_StreamFormatsArray[i+ImageCount] = (PKSDATAFORMAT) &pdx->m_drImageFormat[i+ImageCount];
				
				DBGU_TRACE("RGB24[%d]  X= %d Y= %d\n", i+ImageCount, pdx->m_drImageFormat[i+ImageCount].VideoInfoHeader.bmiHeader.biWidth, pdx->m_drImageFormat[i+ImageCount].VideoInfoHeader.bmiHeader.biHeight);
				nImgNumModify++;
			}
		}

#ifndef	_X64	// shawn 2011/06/15 add
		if (!Disable_I420)	// shawn 2011/06/15 add
		{
			for (i=0;i<ImageCount;i++)
			{
				// copy formats for I420
				if ((pdx->m_drImageFormat[i].VideoInfoHeader.bmiHeader.biWidth <= 640) && 
						(pdx->m_drImageFormat[i].VideoInfoHeader.bmiHeader.biHeight <= 480))
				{		
					RtlCopyMemory(&pdx->m_drImageFormat[nImgNumModify], &pdx->m_drImageFormat[i], sizeof(KS_DATARANGE_VIDEO));
					pdx->m_drImageFormat[nImgNumModify].DataRange.SubFormat = MEDIASUBTYPE_I420;
					pdx->m_drImageFormat[nImgNumModify].bTemporalCompression=FALSE;
					pdx->m_drImageFormat[nImgNumModify].VideoInfoHeader.bmiHeader.biBitCount=12;
					pdx->m_drImageFormat[nImgNumModify].VideoInfoHeader.bmiHeader.biCompression=MEDIASUBTYPE_I420.Data1;//MEDIASUBTYPE_RGB24.Data1;
					pdx->m_drImageFormat[nImgNumModify].VideoInfoHeader.bmiHeader.biSizeImage=
						pdx->m_drImageFormat[i].VideoInfoHeader.bmiHeader.biWidth*pdx->m_drImageFormat[i].VideoInfoHeader.bmiHeader.biHeight*3/2;
					RtlCopyMemory(&pdx->StreamFormatArray[nImgNumModify],&pdx->StreamFormatArray[i],sizeof(VIDEOSTREAMINFO));
					pdx->StreamFormatArray[nImgNumModify].ImageSize = pdx->m_drImageFormat[i].VideoInfoHeader.bmiHeader.biSizeImage;
					pdx->StreamFormatArray[nImgNumModify].ImageFormat = IMG_FMT_I420;
					pdx->StreamFormatArray[nImgNumModify].biCompression = MEDIASUBTYPE_I420.Data1;
					pdx->m_StreamFormatsArray[nImgNumModify] = (PKSDATAFORMAT) &pdx->m_drImageFormat[nImgNumModify];
					
					DBGU_TRACE("I420[%d]  X= %d Y= %d\n", nImgNumModify, pdx->m_drImageFormat[nImgNumModify].VideoInfoHeader.bmiHeader.biWidth, pdx->m_drImageFormat[nImgNumModify].VideoInfoHeader.bmiHeader.biHeight);
					nImgNumModify++;
				}	
			}
		}
#endif	// shawn 2011/06/15 add

		DBGU_TRACE("nImgNumModify = %d",nImgNumModify);
		
		// Add frame rates..
		j = pdx->m_FrameRateInfo.m_FrameRatesIndex[ImageCount];
		
		for (i=0;i<pdx->m_YUY2_FrmaeRateCount;i++)
		{
			// shawn 2011/06/15 modify +++++
#ifndef	_X64
			if (!Disable_RGB24 && !Disable_I420)
			{
				pdx->m_FrameRateInfo.SupportedFrameRates[i+j] = pdx->m_FrameRateInfo.SupportedFrameRates[i];
				pdx->m_FrameRateInfo.SupportedFrameRates[i+j+pdx->m_YUY2_FrmaeRateCount] = pdx->m_FrameRateInfo.SupportedFrameRates[i];
			}
			else if ( (!Disable_RGB24 && Disable_I420) || (Disable_RGB24 && !Disable_I420) )
				pdx->m_FrameRateInfo.SupportedFrameRates[i+j] = pdx->m_FrameRateInfo.SupportedFrameRates[i];
#else
			if (!Disable_RGB24)
				pdx->m_FrameRateInfo.SupportedFrameRates[i+j] = pdx->m_FrameRateInfo.SupportedFrameRates[i];
#endif
			// shawn 2011/06/15 modify -----
		}

		j = ImageCount;

		for (i=0;i<pdx->m_YUY2_FormatCount+1;i++)
		{
			// shawn 2011/06/15 modify +++++
#ifndef	_X64
			if (!Disable_RGB24 && !Disable_I420)
			{
				pdx->m_FrameRateInfo.m_FrameRatesIndex[i+j] = pdx->m_FrameRateInfo.m_FrameRatesIndex[i]+pdx->m_FrameRateInfo.m_FrameRatesIndex[j];
				pdx->m_FrameRateInfo.m_FrameRatesIndex[i+j+pdx->m_YUY2_FormatCount] = 
					pdx->m_FrameRateInfo.m_FrameRatesIndex[i]+
					pdx->m_FrameRateInfo.m_FrameRatesIndex[j]+
					pdx->m_FrameRateInfo.m_FrameRatesIndex[pdx->m_YUY2_FormatCount];
			}
			else if ( (!Disable_RGB24 && Disable_I420) || (Disable_RGB24 && !Disable_I420) )
				pdx->m_FrameRateInfo.m_FrameRatesIndex[i+j] = pdx->m_FrameRateInfo.m_FrameRatesIndex[i]+pdx->m_FrameRateInfo.m_FrameRatesIndex[j];
#else
			if (!Disable_RGB24)
				pdx->m_FrameRateInfo.m_FrameRatesIndex[i+j] = pdx->m_FrameRateInfo.m_FrameRatesIndex[i]+pdx->m_FrameRateInfo.m_FrameRatesIndex[j];
#endif
			// shawn 2011/06/15 modify -----
		}

		ImageCount = pdx->m_VideoImageFrameNumber[STREAM_Capture] = nImgNumModify;
	}
	// end of add RGB24 and I420 formats
	
	//2010/7/28 04:55下午
	pdx->m_Streams[STREAM_Capture].NumberOfFormatArrayEntries = ImageCount;
	pdx->m_STREAMCapturePinDescriptor.PinDescriptor.DataRangesCount = ImageCount;
	//CaptureFilterPinDescriptors[nPinDescriptorCount] = STREAMCapturePinDescriptor;
	pdx->m_CaptureFilterPinDescriptors[STREAM_Capture] = pdx->m_STREAMCapturePinDescriptor;
	
	//CaptureFilterDescriptor.PinDescriptorsCount ++;
	nPinDescriptorCount++;
	DBGU_TRACE("==> Streams[STREAM_Capture].NumberOfFormatArrayEntries = %d\n",ImageCount);

	//
	// Initialize Still Pin Stream formats
	//
	DBGU_TRACE("Still Capture Method = %d\n", pdx->m_StillCaptureMethod);
	
	if (pdx->m_StillPin_Enable)	// shawn 2011/06/23 add
	{
		//2010/7/28 06:41下午
		if((pdx->m_StillCaptureMethod == 0)||(pdx->m_StillCaptureMethod == 1))
		{
			// The number of Still format array will be the same with the number of Video format array (ymwu, 2007/5/15)
			for (i=0;i<pdx->m_VideoImageFrameNumber[STREAM_Capture];i++)
			{
				pdx->m_StillStreamFormatsArray[i] = (PKSDATAFORMAT) &pdx->m_drImageFormat[i];
			}

			pdx->m_VideoImageFrameNumber_Still = pdx->m_VideoImageFrameNumber[STREAM_Capture];	
		}
		else
		{
			for (i=0;i<pdx->m_VideoImageFrameNumber_Still;i++)
			{
				pdx->m_StillStreamFormatsArray[i] = (PKSDATAFORMAT) &pdx->m_drStillImageFormat[i];
			}
		}

		ImageCount = pdx->m_VideoImageFrameNumber_Still;
		pdx->m_Streams[pdx->m_STREAM_Still_Idx].NumberOfFormatArrayEntries = ImageCount;	// shawn 2011/06/23 modify
		//2010/7/28 05:12下午
	
		pdx->m_STREAMStillPinDescriptor.PinDescriptor.DataRangesCount = ImageCount;
		pdx->m_CaptureFilterPinDescriptors[pdx->m_STREAM_Still_Idx] = pdx->m_STREAMStillPinDescriptor;	// shawn 2011/06/23 modify

		nPinDescriptorCount++;
	}
	
	//2010/7/28 04:12下午
	if(pdx->m_bMPEG2TSExist)
	{
		pdx->m_StreamFormatsArray_MP2TS[0] = (PKSDATAFORMAT) &pdx->m_drImageFormat_MP2TS[0];	
		pdx->m_STREAMCaptureMP2TSPinDescriptor.PinDescriptor.DataRangesCount = 1;
		pdx->m_CaptureFilterPinDescriptors[pdx->m_STREAM_Capture_MP2TS_Idx] = pdx->m_STREAMCaptureMP2TSPinDescriptor;	// shawn 2011/06/23 modify
		nPinDescriptorCount++;
		pdx->m_Streams[pdx->m_STREAM_Capture_MP2TS_Idx].NumberOfFormatArrayEntries = 1;	// shawn 2011/06/23 modify
		
		if (pdx->m_StillPin_Enable)	// shawn 2011/06/23 add
			pdx->m_CaptureFilterDescriptor.PinDescriptorsCount = 3;
	}

	DBGU_TRACE("==> nPinDescriptorCount = %d\n",nPinDescriptorCount);
	DBGU_TRACE("==> Streams[STREAM_Still].NumberOfFormatArrayEntries = %d\n",ImageCount);

	return (ntStatus);
}

PDEVEXT	CVideoStream::CurrentDevExt()
{
	if (m_Device)
	{
		CCaptureDevice *pCapDevice = reinterpret_cast <CCaptureDevice *> (m_Device->Context);
		
		if (pCapDevice)
			return (pCapDevice->pdx);
	}

	return NULL;
}

//2011/2/25 10:54上午 mark
/*NTSTATUS
CVideoStream::CheckOSVersion()
{
	PDEVEXT pDevExt = CurrentDevExt();
	RTL_QUERY_REGISTRY_TABLE *Table=NULL;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UNICODE_STRING	RegPath;

	RtlInitUnicodeString(&RegPath, (PWSTR) DefaultRegPath);
	Table = (RTL_QUERY_REGISTRY_TABLE *) ExAllocatePoolWithTag(NonPagedPool, sizeof(RTL_QUERY_REGISTRY_TABLE), 'osSC'); 
	
	RtlZeroMemory (Table, sizeof(RTL_QUERY_REGISTRY_TABLE)); 
	//OSVersion
	Table[0].Name			= L"OSVersion";
	Table[0].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
	Table[0].EntryContext	= &pDevExt->m_C3Enable;
	ntStatus = RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
												RegPath.Buffer,
												Table,
												NULL,
												NULL);

	ExFreePoolWithTag(Table, 'osSC');
	return ntStatus;
}*/

NTSTATUS
CVideoStream::LoadHWDefaultValues()
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PDEVEXT pdx = CurrentDevExt();
	LONG	buffer= 0;
	ULONG	BufferLength=sizeof(buffer);
	ULONG	Index;
	ULONG	Value;
	UINT	i = 0;
	ULONG	Count=0;

	DBGU_TRACE("Start LoadHWDefaultValues!\n");

	RtlZeroMemory(pdx->m_PropertyDefaultValues, sizeof(pdx->m_PropertyDefaultValues));

	//deal with CameraControlProperties tables..
	// // Camera Terminal Control Selectors
	Index = pdx->m_InputEntityID<<8;

	if ((pdx->m_InputControls>>3)&1)	// Support Exposure Time Absolute || Relative
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 4;
		Value=CT_EXPOSURE_TIME_ABSOLUTE_CONTROL<<8;
		//2011/1/6 02:53下午
		
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		pdx->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMaximum = pdx->pVideoDevice->GetExposureValue(buffer);
		DBGU_TRACE("Get Exposure Max =%d\n",pdx->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMaximum);		
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);		
		pdx->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMinimum = pdx->pVideoDevice->GetExposureValue(buffer);
		DBGU_TRACE("Get Exposure Min =%d\n",pdx->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMinimum);
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);		
		pdx->m_PropertyDefaultValues[EXPOSURE] = pdx->pVideoDevice->GetExposureValue(buffer);
		DBGU_TRACE("Get Exposure Def =%d\n",pdx->m_PropertyDefaultValues[EXPOSURE]);
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Exposure Res =%d\n",buffer);
		pdx->m_RangeAndStepProp[EXPOSURE].SteppingDelta = (short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		pdx->m_PropertyCurrentValues[EXPOSURE] = pdx->pVideoDevice->GetExposureValue(buffer);
		DBGU_TRACE("Get Exposure Cur =%d\n",pdx->m_PropertyCurrentValues[EXPOSURE]);
		pdx->m_CameraControlProperties[i] = CameraControlProperty[0];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_ExposureValues;
		i++;
	}

	if ((pdx->m_InputControls>>5)&1)	// Support Focus Absolute || Relative
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=CT_FOCUS_ABSOLUTE_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Focus Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[FOCUS].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Focus Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[FOCUS].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Focus Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[FOCUS] = buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Focus Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[FOCUS].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get Focus Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[FOCUS] = (short)buffer;
		
		pdx->m_CameraControlProperties[i] = CameraControlProperty[1];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_FocusValues;
		i++;
	}

	//2011/1/6 03:03下午
	if ((pdx->m_InputControls>>17)&1)	// Support FOCUS_AUTO Absolute || Relative
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 1;
		Value=CT_FOCUS_AUTO_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get FOCUS_AUTO Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[FOCUS_AUTO] = buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get FOCUS_AUTO Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[FOCUS_AUTO] = (short)buffer;
		
		//pdx->m_CameraControlProperties[i] = CameraControlProperty[1];
		//i++;
	}

	if ((pdx->m_InputControls>>7)&1)	// Support Iris Absolute || Relative
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=CT_IRIS_ABSOLUTE_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Iris Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[IRIS].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Iris Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[IRIS].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Iris Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[IRIS] = buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Iris Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[IRIS].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get Iris Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[IRIS] = (short)buffer;		
		
		pdx->m_CameraControlProperties[i] = CameraControlProperty[2];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_IrisValues;
		i++;
	}

	if ((pdx->m_InputControls>>9)&1)	// Support Zoom Absolute || Relative
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=CT_ZOOM_ABSOLUTE_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Zoom Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[ZOOM].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Zoom Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[ZOOM].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Zoom Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[ZOOM] = buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Zoom Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[ZOOM].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get Zoom Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[ZOOM] = (short)buffer;	
				
		pdx->m_CameraControlProperties[i] = CameraControlProperty[3];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_ZoomValues;
		i++;
	}

	if((pdx->m_InputControls>>11)&1)	// Support PanTilt Absolute || Relative
	{
		struct _PanTilt
		{
			long dwPan;
			long dwTilt;
		} PanTiltValue;
		
		BufferLength = 8;
		Value=CT_PANTILT_ABSOLUTE_CONTROL<<8;

		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&PanTiltValue,BufferLength,TRUE);
		pdx->m_RangeAndStepProp[PAN].Bounds.SignedMaximum = PanTiltValue.dwPan/3600;
		pdx->m_RangeAndStepProp[TILT].Bounds.SignedMaximum = PanTiltValue.dwTilt/3600;
		DBGU_TRACE("Get PanTilt Max = %d, %d\n",pdx->m_RangeAndStepProp[PAN].Bounds.SignedMaximum, pdx->m_RangeAndStepProp[TILT].Bounds.SignedMaximum);

		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&PanTiltValue,BufferLength,TRUE);
		pdx->m_RangeAndStepProp[PAN].Bounds.SignedMinimum = PanTiltValue.dwPan/3600;
		pdx->m_RangeAndStepProp[TILT].Bounds.SignedMinimum = PanTiltValue.dwTilt/3600;
		DBGU_TRACE("Get PanTilt Min = %d, %d\n",pdx->m_RangeAndStepProp[PAN].Bounds.SignedMinimum, pdx->m_RangeAndStepProp[TILT].Bounds.SignedMinimum);

		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&PanTiltValue,BufferLength,TRUE);
		pdx->m_PropertyDefaultValues[PAN] = PanTiltValue.dwPan/3600;
		pdx->m_PropertyDefaultValues[TILT] = PanTiltValue.dwTilt/3600;
		DBGU_TRACE("Get PanTilt Def = %d, %d\n",pdx->m_PropertyDefaultValues[PAN], pdx->m_PropertyDefaultValues[TILT]);
		
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&PanTiltValue,BufferLength,TRUE);
		pdx->m_RangeAndStepProp[PAN].SteppingDelta = PanTiltValue.dwPan/3600;	
		pdx->m_RangeAndStepProp[TILT].SteppingDelta = PanTiltValue.dwTilt/3600;
		DBGU_TRACE("Get PanTilt Res = %d, %d\n",pdx->m_RangeAndStepProp[PAN].SteppingDelta, pdx->m_RangeAndStepProp[TILT].SteppingDelta);

		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&PanTiltValue,BufferLength,TRUE);
		pdx->m_PropertyCurrentValues[PAN] = PanTiltValue.dwPan/3600;
		pdx->m_PropertyCurrentValues[TILT] = PanTiltValue.dwTilt/3600;
		DBGU_TRACE("Get PanTilt Cur = %d, %d\n",pdx->m_PropertyCurrentValues[PAN], pdx->m_PropertyCurrentValues[TILT]);
					
		pdx->m_CameraControlProperties[i] = CameraControlProperty[4];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_PanValues;
		i++;
		pdx->m_CameraControlProperties[i] = CameraControlProperty[7];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_TiltValues;
		i++;		
	}

	if ((pdx->m_InputControls>>13)&1)	// Support Roll Absolute || Relative
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=CT_ROLL_ABSOLUTE_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Roll Max =%d\n",(signed short)buffer);
		pdx->m_RangeAndStepProp[ROLL].Bounds.SignedMaximum=(signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Roll Min =%d\n",(signed short)buffer);
		pdx->m_RangeAndStepProp[ROLL].Bounds.SignedMinimum=(signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Roll Def =%d\n",(signed short)buffer);
		pdx->m_PropertyDefaultValues[ROLL] = buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Roll Res =%d\n",(signed short)buffer);
		pdx->m_RangeAndStepProp[ROLL].SteppingDelta=(signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get ROLL Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[ROLL] = (signed short)buffer;	

		pdx->m_CameraControlProperties[i] = CameraControlProperty[5];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_RollValues;
		i++;
	}

	// shawn 2011/05/27 +++++
	if(!pdx->m_XU_Privacy_Dis)
	{
		pdx->m_PropertyDefaultValues[PRIVACY] = (pdx->m_0x8400_Data[PRIVACY_DEF_ADDR]);
		long buffer = 0;	// james 2009/06/11 initialize the value.
		
		if(Get_Cur_Privacy_Value(&buffer) == STATUS_SUCCESS)
			pdx->m_PropertyCurrentValues[PRIVACY] = (signed short)buffer;
		
		pdx->m_CameraControlProperties[i] = CameraControlProperty[6];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_PrivacyValues;
		i++;
	}
	// shawn 2011/05/27 -----
	else if ((pdx->m_InputControls>>18)&1)	// Support Privacy
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 1;
		Value=CT_PRIVACY_CONTROL<<8;

		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);	
		DBGU_TRACE("Get PRIVACY Def =%d\n",(signed short)buffer);
		pdx->m_PropertyDefaultValues[PRIVACY] = buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get PRIVACY Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[PRIVACY] = (signed short)buffer;	

		pdx->m_CameraControlProperties[i] = CameraControlProperty[6];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_PrivacyValues;
		i++;
	}

	if ((pdx->m_InputControls>>1)&1)	// Support AEMode Control
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 1;
		Value=CT_AE_MODE_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get CT_AE_MODE_CONTROL Def =%d\n",(BYTE)buffer);
		pdx->m_AEMode.DefValueHW=(BYTE)buffer;
		pdx->m_AEMode.CurrentValue = pdx->m_AEMode.DefValueHW;
		pdx->m_PropertyCurrentValues[AEMODE] = (signed short)buffer;	

		//2010/12/1 06:39下午
		//i++;
	}

	if((pdx->m_InputControls>>2)&1)	// Support AEPriority Control
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 1;
		Value=CT_AE_PRIORITY_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);	// shawn 2011/05/02 modify from GET_DEF to GET_CUR
		DBGU_TRACE("Get CT_AE_PRIORITY_CONTROL Def =%d\n",(BYTE)buffer);
		pdx->m_AEPriority.DefValueHW=(BYTE)buffer;
		pdx->m_AEPriority.CurrentValue = pdx->m_AEPriority.DefValueHW;
		//2010/12/1 06:40下午
		pdx->m_PropertyDefaultValues[AEPRIORITY] = buffer;
		pdx->m_PropertyCurrentValues[AEPRIORITY] = (signed short)buffer;	
		
		pdx->m_CameraControlProperties[i] = CameraControlProperty[8];
		pdx->m_CameraControlProperties[i].Values = &pdx->m_AEPriorityValues;
		i++;
	}

	// change number of DevicePropertySetsTable
	Count += i;
	pdx->m_DevicePropertySetsTable[0].PropertiesCount = i;
	DBGU_TRACE("==> number of CameraControlProperties DevicePropertySetsTable=%d\n",pdx->m_DevicePropertySetsTable[0].PropertiesCount);

	// // Processing Unit Control Selectors
	i=0;
	Index = pdx->m_PropertyEntityID<<8; 

	if ((pdx->m_PropertyControls)&1)	// Support Brightness
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=PU_BRIGHTNESS_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Brightness Max =%d\n",(signed short)buffer);
		pdx->m_RangeAndStepProp[BRIGHTNESS].Bounds.SignedMaximum=(signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Brightness Min =%d\n",(signed short)buffer);
		pdx->m_RangeAndStepProp[BRIGHTNESS].Bounds.SignedMinimum=(signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Brightness Def =%d\n",(signed short)buffer);
		pdx->m_PropertyDefaultValues[BRIGHTNESS] = (signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Brightness Res =%d\n",(signed short)buffer);
		pdx->m_RangeAndStepProp[BRIGHTNESS].SteppingDelta=(signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get BRIGHTNESS Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[BRIGHTNESS] = (signed short)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[0];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_BrightnessValues;
		DBGU_TRACE("VideoProcAmpProperties[%d]\n",i);
		i++;
	}

	if ((pdx->m_PropertyControls>>1)&1)	// Support Contrast
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=PU_CONTRAST_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Contrast Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[CONTRAST].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Contrast Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[CONTRAST].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Contrast Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[CONTRAST] = (short)buffer;	
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Contrast Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[CONTRAST].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get CONTRAST Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[CONTRAST] = (short)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[1];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_ContrastValues;
		i++;
	}

	if ((pdx->m_PropertyControls>>2)&1)	// Support Hue
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=PU_HUE_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Hue Max =%d\n",(signed short)buffer);
		pdx->m_RangeAndStepProp[HUE].Bounds.SignedMaximum=(signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Hue Min =%d\n",(signed short)buffer);
		pdx->m_RangeAndStepProp[HUE].Bounds.SignedMinimum=(signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Hue Def =%d\n",(signed short)buffer);
		pdx->m_PropertyDefaultValues[HUE] = (signed short)buffer;	
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Hue Res =%d\n",(signed short)buffer);
		pdx->m_RangeAndStepProp[HUE].SteppingDelta=(signed short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get HUE Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[HUE] = (signed short)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[2];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_HueValues;
		i++;
	}

	if ((pdx->m_PropertyControls>>3)&1)	// Support Saturation
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=PU_SATURATION_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Saturation Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[SATURATION].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Saturation Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[SATURATION].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Saturation Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[SATURATION] = (short)buffer;	
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Saturation Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[SATURATION].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get SATURATION Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[SATURATION] = (short)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[3];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_SaturationValues;
		i++;
	}

	if ((pdx->m_PropertyControls>>4)&1)	// Support Sharpness
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=PU_SHARPNESS_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Sharpness Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[SHARPNESS].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Sharpness Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[SHARPNESS].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Sharpness Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[SHARPNESS] = (short)buffer;	
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Sharpness Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[SHARPNESS].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get SHARPNESS Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[SHARPNESS] = (short)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[4];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_SharpnessValues;
		i++;
	}

	if ((pdx->m_PropertyControls>>5)&1)	// Support Gamma
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=PU_GAMMA_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Gamma Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[GAMMA].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Gamma Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[GAMMA].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Gamma Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[GAMMA] = (short)buffer;	
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Gamma Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[GAMMA].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get GAMMA Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[GAMMA] = (short)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[5];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_GammaValues;
		i++;
	}

	if ((pdx->m_PropertyControls>>6)&1)	// Support Whitebalance Temperature
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=PU_WHITE_BALANCE_TEMPERATURE_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get PU_WHITE_BALANCE_TEMPERATURE_CONTROL Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[WHITEBALANCE].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get PU_WHITE_BALANCE_TEMPERATURE_CONTROL Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[WHITEBALANCE].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get PU_WHITE_BALANCE_TEMPERATURE_CONTROL Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[WHITEBALANCE] = (short)buffer;	
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get PU_WHITE_BALANCE_TEMPERATURE_CONTROL Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[WHITEBALANCE].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get WHITEBALANCE Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[WHITEBALANCE] = (short)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[6];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_WhitebalanceValues;
		i++;
	}

	// shawn 2011/05/27 +++++
	if(!pdx->m_XU_WBC_Dis)
	{
		pdx->m_WBC.MaxValue = (pdx->m_0x8400_Data[WBC_RED_MAX_ADDR_H]<<24)
																	+ (pdx->m_0x8400_Data[WBC_RED_MAX_ADDR_L]<<16)
																	+ (pdx->m_0x8400_Data[WBC_BLUE_MAX_ADDR_H]<<8)
																	+ (pdx->m_0x8400_Data[WBC_BLUE_MAX_ADDR_L]);
		
		pdx->m_WBC.MinValue = (pdx->m_0x8400_Data[WBC_RED_MIN_ADDR_H]<<24)
																	+ (pdx->m_0x8400_Data[WBC_RED_MIN_ADDR_L]<<16)
																	+ (pdx->m_0x8400_Data[WBC_BLUE_MIN_ADDR_H]<<8)
																	+ (pdx->m_0x8400_Data[WBC_BLUE_MIN_ADDR_L]);

		pdx->m_WBC.DefValueHW = (pdx->m_0x8400_Data[WBC_RED_DEF_ADDR_H]<<24)
																	+ (pdx->m_0x8400_Data[WBC_RED_DEF_ADDR_L]<<16)
																	+ (pdx->m_0x8400_Data[WBC_BLUE_DEF_ADDR_H]<<8)
																	+ (pdx->m_0x8400_Data[WBC_BLUE_DEF_ADDR_L]);
		
		pdx->m_WBC.ResValue = (pdx->m_0x8400_Data[WBC_RED_RES_ADDR]<<16)
																	+ (pdx->m_0x8400_Data[WBC_BLUE_RES_ADDR]);
		//2008/8/20 01:33下午
		pdx->m_WBC_Green.MaxValue = (pdx->m_0x8400_Data[WBC_GREEN_MAX_ADDR_H]<<8)
																	+ (pdx->m_0x8400_Data[WBC_GREEN_MAX_ADDR_L]);
		
		pdx->m_WBC_Green.MinValue = (pdx->m_0x8400_Data[WBC_GREEN_MIN_ADDR_H]<<8)
																	+ (pdx->m_0x8400_Data[WBC_GREEN_MIN_ADDR_L]);

		pdx->m_WBC_Green.DefValueHW = (pdx->m_0x8400_Data[WBC_GREEN_DEF_ADDR_H]<<8)
																	+ (pdx->m_0x8400_Data[WBC_GREEN_DEF_ADDR_L]);
		
		pdx->m_WBC_Green.ResValue = pdx->m_0x8400_Data[WBC_GREEN_RES_ADDR];
	
		DbgPrint("WBC_RED_MAX_ADDR_H=0x%x,WBC_RED_MAX_ADDR_L=0x%x\n",pdx->m_0x8400_Data[WBC_RED_MAX_ADDR_H],
		pdx->m_0x8400_Data[WBC_RED_MAX_ADDR_L]);	

		DbgPrint("pdx->m_WBC.MaxValue=0x%x,pdx->m_WBC.MinValue=0x%x\n",pdx->m_WBC.MaxValue,pdx->m_WBC.MinValue);	
		DbgPrint("pdx->m_WBC_Green.MaxValue=0x%x,pdx->m_WBC_Green.MinValue=0x%x\n",pdx->m_WBC_Green.MaxValue,pdx->m_WBC_Green.MinValue);	
		long buffer = 0; // james 2009/06/11 initialize the value.
		
		if(Get_Cur_WBC_Value(&buffer) == STATUS_SUCCESS)
			pdx->m_WBC.CurrentValue = (ULONG)buffer;
		
		DbgPrint("pdx->m_WBC.CurrentValue=0x%x\n",pdx->m_WBC.CurrentValue);		
		
		//2008/8/20 01:38下午
		if(Get_Cur_WBC_Green_Value(&buffer) == STATUS_SUCCESS)
			pdx->m_WBC_Green.CurrentValue = (ULONG)buffer;		
		
		DbgPrint("pdx->m_WBC_Green.CurrentValue=0x%x\n", pdx->m_WBC_Green.CurrentValue);		
	}
	// shawn 2011/05/27 -----
	else if ((pdx->m_PropertyControls>>7)&1)	// Support Whitebalance Component
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 4;
		Value=PU_WHITE_BALANCE_COMPONENT_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get WHITE_BALANCE_COMPONENT Max =0x%x\n",buffer);
		pdx->m_WBC.MaxValue=buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get WHITE_BALANCE_COMPONENT Min =0x%x\n",buffer);
		pdx->m_WBC.MinValue=buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get WHITE_BALANCE_COMPONENT Def =0x%x\n",buffer);
		pdx->m_WBC.DefValueHW=buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get WHITE_BALANCE_COMPONENT Res =0x%x\n",buffer);
		pdx->m_WBC.ResValue=buffer;
		pdx->m_WBC.CurrentValue = pdx->m_WBC.DefValueHW;
		//2011/1/7 02:48下午
		//i++;
	}

	if ((pdx->m_PropertyControls>>12)&1)	// Support Whitebalance Temperature Auto
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 1;
		Value=PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get WHITE_BALANCE_TemperatureAuto Def =%d\n",(short)buffer);
		pdx->m_WBTAuto.DefValueHW=(short)buffer;
		pdx->m_WBTAuto.CurrentValue = pdx->m_WBTAuto.DefValueHW;
		//2011/1/7 02:48下午
		//i++;
	}

	// shawn 2011/05/27 +++++
	if(!pdx->m_XU_WBCAuto_Dis)
	{
		pdx->m_WBCAuto.DefValueHW = (pdx->m_0x8400_Data[WBCAUTO_DEF_ADDR]);
		long buffer = 0; // james 2009/06/11 initialize the value.
		
		if(Get_Cur_WBCAuto_Value(&buffer) == STATUS_SUCCESS)
			pdx->m_WBCAuto.CurrentValue = (short)buffer;
	}
	// shawn 2011/05/27 -----
	else if ((pdx->m_PropertyControls>>13)&1)	// Support Whitebalance Component Auto
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 1;
		Value=PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get WHITE_BALANCE_COMPONENTAuto Def =%d\n",(short)buffer);
		pdx->m_WBCAuto.DefValueHW=(short)buffer;
		pdx->m_WBCAuto.CurrentValue = pdx->m_WBCAuto.DefValueHW;
		//2011/1/7 02:48下午
		//i++;
	}

	if ((pdx->m_PropertyControls>>8)&1)	// Support Backlight
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=PU_BACKLIGHT_COMPENSATION_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Backlight Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[BACKLIGHT].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Backlight Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[BACKLIGHT].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Backlight Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[BACKLIGHT] = (short)buffer;	
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Backlight Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[BACKLIGHT].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get BACKLIGHT Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[BACKLIGHT] = (short)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[7];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_BacklightValues;
		i++;
	}

	if ((pdx->m_PropertyControls>>10)&1)	// Support PowerlineFrequency
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 1;
		Value=PU_POWER_LINE_FREQUENCY_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get PU_POWER_LINE_FREQUENCY_CONTROL Max =%d\n",(BYTE)buffer);
		pdx->m_RangeAndStepProp[POWERLINEFREQUENCY].Bounds.SignedMaximum=(BYTE)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get PU_POWER_LINE_FREQUENCY_CONTROL Min =%d\n",(BYTE)buffer);
		pdx->m_RangeAndStepProp[POWERLINEFREQUENCY].Bounds.SignedMinimum=(BYTE)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get PU_POWER_LINE_FREQUENCY_CONTROL Def =%d\n",(BYTE)buffer);

		pdx->m_PropertyDefaultValues[POWERLINEFREQUENCY] = (BYTE)buffer;	
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get PU_POWER_LINE_FREQUENCY_CONTROL Res =%d\n",(BYTE)buffer);
		pdx->m_RangeAndStepProp[POWERLINEFREQUENCY].SteppingDelta=(BYTE)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get POWERLINEFREQUENCY Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[POWERLINEFREQUENCY] = (BYTE)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[8];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_PowerlineFrequencyValues;
		i++;
	}

	if ((pdx->m_PropertyControls>>9)&1)	// Support Gain
	{
		buffer = 0; // james 2009/06/11 initialize the value.
		BufferLength = 2;
		Value=PU_GAIN_CONTROL<<8;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MAX,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Gain Max =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[GAIN].Bounds.SignedMaximum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_MIN,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Gain Min =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[GAIN].Bounds.SignedMinimum=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_DEF,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Gain Def =%d\n",(short)buffer);
		pdx->m_PropertyDefaultValues[GAIN] = (short)buffer;	
		pdx->pVideoDevice->USB_ControlClassCommand(GET_RES,Value,Index,&buffer,BufferLength,TRUE);
		DBGU_TRACE("Get Gain Res =%d\n",(short)buffer);
		pdx->m_RangeAndStepProp[GAIN].SteppingDelta=(short)buffer;
		pdx->pVideoDevice->USB_ControlClassCommand(GET_CUR,Value,Index,&buffer,BufferLength,TRUE);		
		DBGU_TRACE("Get GAIN Cur =%d\n",buffer);
		pdx->m_PropertyCurrentValues[GAIN] = (short)buffer;	

		pdx->m_VideoProcAmpProperties[i] = VideoProcAmpProperty[9];
		pdx->m_VideoProcAmpProperties[i].Values = &pdx->m_GainValues;
		i++;
	}

	// shawn 2011/05/27 +++++
	if(!pdx->m_XU_AuxLED_Dis)
	{
		long	buffer = 0; // james 2009/06/11 initialize the value.
		
		if(Get_Cur_AUXLED_Value(&buffer) == STATUS_SUCCESS) 
			pdx->m_AuxLedMode.CurrentValue = (ULONG)buffer;
		
		DbgPrint("pdx->m_AuxLedMode.CurrentValue =0x%x\n", pdx->m_AuxLedMode.CurrentValue);
	}
	// shawn 2011/05/27 -----

	// change number of DevicePropertySetsTable
	Count += i;
	pdx->m_DevicePropertySetsTable[1].PropertiesCount = i;
	pdx->m_Streams[STREAM_Capture].NumStreamPropArrayEntries=Count;
	DBGU_TRACE("==> number of VideoProcAmpProperties DevicePropertySetsTable=%d\n",pdx->m_DevicePropertySetsTable[1].PropertiesCount);

	pdx->m_StreamProperties[BRIGHTNESS] = pdx->m_PropertyDefaultValues[BRIGHTNESS];
	pdx->m_StreamProperties[CONTRAST] = pdx->m_PropertyDefaultValues[CONTRAST];
	pdx->m_StreamProperties[HUE] = pdx->m_PropertyDefaultValues[HUE];
	pdx->m_StreamProperties[SATURATION] = pdx->m_PropertyDefaultValues[SATURATION];
	pdx->m_StreamProperties[SHARPNESS] = pdx->m_PropertyDefaultValues[SHARPNESS];
	pdx->m_StreamProperties[GAMMA] = pdx->m_PropertyDefaultValues[GAMMA];
	pdx->m_StreamProperties[WHITEBALANCE] = pdx->m_PropertyDefaultValues[WHITEBALANCE];
	pdx->m_StreamProperties[BACKLIGHT] = pdx->m_PropertyDefaultValues[BACKLIGHT];
	pdx->m_StreamProperties[POWERLINEFREQUENCY] = pdx->m_PropertyDefaultValues[POWERLINEFREQUENCY];
	pdx->m_StreamProperties[GAIN] = pdx->m_PropertyDefaultValues[GAIN];
	pdx->m_StreamProperties[EXPOSURE] = pdx->m_PropertyDefaultValues[EXPOSURE];
	pdx->m_StreamProperties[FOCUS] = pdx->m_PropertyDefaultValues[FOCUS];
	pdx->m_StreamProperties[IRIS] = pdx->m_PropertyDefaultValues[IRIS];
	pdx->m_StreamProperties[ZOOM] = pdx->m_PropertyDefaultValues[ZOOM];
	//2011/1/6 11:35上午
	//pdx->m_StreamProperties[PANTILT] = pdx->m_PropertyDefaultValues[PANTILT];
	pdx->m_StreamProperties[PAN] = pdx->m_PropertyDefaultValues[PAN];
	pdx->m_StreamProperties[TILT] = pdx->m_PropertyDefaultValues[TILT];
	pdx->m_StreamProperties[ROLL] = pdx->m_PropertyDefaultValues[ROLL];
	pdx->m_StreamProperties[PRIVACY] = pdx->m_PropertyDefaultValues[PRIVACY];
	//2011/1/5 06:36下午
	pdx->m_StreamProperties[AEPRIORITY] = pdx->m_PropertyDefaultValues[AEPRIORITY];	
	pdx->m_WBTAuto.DefValue = pdx->m_WBTAuto.DefValueHW;
	pdx->m_WBC.DefValue = pdx->m_WBC.DefValueHW;
	pdx->m_WBCAuto.DefValue = pdx->m_WBCAuto.DefValueHW;
	pdx->m_Flip.DefValue = 0;
	pdx->m_Mirror.DefValue = 0;
	pdx->m_BWMode.DefValue = 0;
	pdx->m_AEMode.DefValue = pdx->m_AEMode.DefValueHW;
	pdx->m_AEPriority.DefValue = pdx->m_AEPriority.DefValueHW;
	
	// shawn 2011/06/13 +++++
	pdx->m_Rotation.DefValue = 0;
	pdx->m_AuxLedMode.DefValue = 0;
	pdx->m_Snapshot_Ctrl.DefValue = 0;
	pdx->m_DeNoiseMode.DefValue = 0;
	pdx->m_WBC_Green.DefValue = 0;
	// shawn 2011/06/13 -----

	// change the number of KSPROPERTY_SET, that for supporting Extension Unit
	/*ASSERT(m_pDevicePropertySets==NULL);

	UINT size = SIZEOF_ARRAY(DevicePropertySetsTable);
	m_pDevicePropertySets = new (NonPagedPool) KSPROPERTY_SET[size + pdx->m_ExUnitNum];

	if (m_pDevicePropertySets)
	{
		RtlCopyMemory(m_pDevicePropertySets, DevicePropertySetsTable, sizeof(DevicePropertySetsTable));

		for (i=0;i<pdx->m_ExUnitNum;i++)
		{
			RtlCopyMemory(&m_pDevicePropertySets[size+i], &DevicePropertySets[i], sizeof(KSPROPERTY_SET));
		}

		FilterAutomationTable.PropertySetsCount = size + pdx->m_ExUnitNum;//SIZEOF_ARRAY(m_pDevicePropertySets);
		FilterAutomationTable.PropertyItemSize	= sizeof(KSPROPERTY_ITEM);
		FilterAutomationTable.PropertySets		= m_pDevicePropertySets;
	}
	else
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	*/
	return ntStatus;
}

NTSTATUS 
CVideoStream::LoadDefaultSettings(/*PWCHAR BasePath*/BOOL bCustom, BOOLEAN bReload)	// shawn 2011/05/31 modify
{
    NTSTATUS ntStatus=STATUS_SUCCESS;
	DBGU_INFO("Launch CVideoStream::LoadDefaultSettings()\n");

	RTL_QUERY_REGISTRY_TABLE *Table=NULL;
	UINT i=0,j=0;
	PDEVEXT pdx = CurrentDevExt();
	WCHAR pCurStrBuf[] = L"FaceTrack0000";

	// shawn 2011/05/31 +++++
	UNICODE_STRING	BasePath;

	if (bCustom)
		RtlInitUnicodeString(&BasePath, (PWSTR) pdx->pVideoDevice->m_CurValuePath);
	else
		RtlInitUnicodeString(&BasePath, (PWSTR) pdx->pVideoDevice->m_DefValuePath);
	// shawn 2011/05/31 -----

	if (bReload)
	{
		pdx->m_StreamProperties[BRIGHTNESS] = pdx->m_PropertyDefaultValues[BRIGHTNESS];
		pdx->m_StreamProperties[CONTRAST] = pdx->m_PropertyDefaultValues[CONTRAST];
		pdx->m_StreamProperties[HUE] = pdx->m_PropertyDefaultValues[HUE];
		pdx->m_StreamProperties[SATURATION] = pdx->m_PropertyDefaultValues[SATURATION];
		pdx->m_StreamProperties[SHARPNESS] = pdx->m_PropertyDefaultValues[SHARPNESS];
		pdx->m_StreamProperties[GAMMA] = pdx->m_PropertyDefaultValues[GAMMA];
		pdx->m_StreamProperties[WHITEBALANCE] = pdx->m_PropertyDefaultValues[WHITEBALANCE];
		pdx->m_WBC.DefValue = pdx->m_WBC.DefValueHW;
		pdx->m_WBCAuto.DefValue = pdx->m_WBCAuto.DefValueHW;
		pdx->m_StreamProperties[BACKLIGHT] = pdx->m_PropertyDefaultValues[BACKLIGHT];
		pdx->m_StreamProperties[GAIN] = pdx->m_PropertyDefaultValues[GAIN];
		pdx->m_StreamProperties[POWERLINEFREQUENCY] = pdx->m_PropertyDefaultValues[POWERLINEFREQUENCY];
		pdx->m_StreamProperties[EXPOSURE] = pdx->m_PropertyDefaultValues[EXPOSURE];
		pdx->m_StreamProperties[FOCUS] = pdx->m_PropertyDefaultValues[FOCUS];
		pdx->m_StreamProperties[IRIS] = pdx->m_PropertyDefaultValues[IRIS];
		pdx->m_StreamProperties[ZOOM] = pdx->m_PropertyDefaultValues[ZOOM];
		//2011/1/6 11:36上午
		//pdx->m_StreamProperties[PANTILT] = pdx->m_PropertyDefaultValues[PANTILT];
		pdx->m_StreamProperties[PAN] = pdx->m_PropertyDefaultValues[PAN];
		pdx->m_StreamProperties[TILT] = pdx->m_PropertyDefaultValues[TILT];
		pdx->m_StreamProperties[ROLL] = pdx->m_PropertyDefaultValues[ROLL];
		pdx->m_StreamProperties[PRIVACY] = pdx->m_PropertyDefaultValues[PRIVACY];
		pdx->m_AEMode.CurrentValue = pdx->m_AEMode.DefValue;
		pdx->m_AEPriority.CurrentValue = pdx->m_AEPriority.DefValue;
		//2011/1/5 06:32下午
		pdx->m_StreamProperties[AEPRIORITY] = pdx->m_PropertyDefaultValues[AEPRIORITY];
		pdx->m_Mirror.CurrentValue = pdx->m_Mirror.DefValue;
		pdx->m_Flip.CurrentValue = pdx->m_Flip.DefValue;
		pdx->m_BWMode.CurrentValue = pdx->m_BWMode.DefValue;

		// shawn 2011/05/25 +++++
		pdx->m_Rotation.CurrentValue = pdx->m_Rotation.DefValue;

		if(!pdx->m_XU_AuxLED_Dis)
			pdx->m_AuxLedMode.CurrentValue = pdx->m_AuxLedMode.DefValue;

		pdx->m_DeNoiseMode.CurrentValue = pdx->m_DeNoiseMode.DefValue;
		pdx->m_WBC_Green.CurrentValue = pdx->m_WBC_Green.DefValue;
		// shawn 2011/05/25 -----
	}
	else
	{
		Table = new (NonPagedPool) RTL_QUERY_REGISTRY_TABLE[DEFAULT_REGISTRY_NUMBER];
		RtlZeroMemory(Table, DEFAULT_REGISTRY_NUMBER*sizeof(RTL_QUERY_REGISTRY_TABLE));

		if (Table)
		{
			i = 0;
				
			//Brightness
			pdx->m_StreamProperties[BRIGHTNESS] = pdx->m_PropertyDefaultValues[BRIGHTNESS];

			Table[i].Name			= L"Brightness";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[BRIGHTNESS];

			//Contrast
			pdx->m_StreamProperties[CONTRAST] = pdx->m_PropertyDefaultValues[CONTRAST];

			Table[i].Name			= L"Contrast";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[CONTRAST];

			//Hue
			pdx->m_StreamProperties[HUE] = pdx->m_PropertyDefaultValues[HUE];

			Table[i].Name			= L"Hue";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[HUE];

			//Saturation
			pdx->m_StreamProperties[SATURATION] = pdx->m_PropertyDefaultValues[SATURATION];

			Table[i].Name			= L"Saturation";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[SATURATION];

			//Sharpness
			pdx->m_StreamProperties[SHARPNESS] = pdx->m_PropertyDefaultValues[SHARPNESS];

			Table[i].Name			= L"Sharpness";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[SHARPNESS];

			//Gamma
			pdx->m_StreamProperties[GAMMA] = pdx->m_PropertyDefaultValues[GAMMA];

			Table[i].Name			= L"Gamma";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[GAMMA];

			//WhiteBalance
			pdx->m_StreamProperties[WHITEBALANCE] = pdx->m_PropertyDefaultValues[WHITEBALANCE];

			Table[i].Name			= L"WhiteBalance";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[WHITEBALANCE];

			//WhiteBalanceComponent
			pdx->m_WBC.DefValue = pdx->m_WBC.DefValueHW;

			Table[i].Name			= L"WBComponent";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_WBC.DefValue;

			//WhiteBalanceComponent Auto
			pdx->m_WBCAuto.DefValue = pdx->m_WBCAuto.DefValueHW;

			Table[i].Name			= L"WBComponentAuto";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_WBCAuto.DefValue;

			//BackLight
			pdx->m_StreamProperties[BACKLIGHT] = pdx->m_PropertyDefaultValues[BACKLIGHT];

			Table[i].Name			= L"BackLight";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[BACKLIGHT];

			//PowerLineFrequency
			pdx->m_StreamProperties[POWERLINEFREQUENCY] = pdx->m_PropertyDefaultValues[POWERLINEFREQUENCY];

			Table[i].Name			= L"PowerLineFrequency";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[POWERLINEFREQUENCY];

			//Gain
			pdx->m_StreamProperties[GAIN] = pdx->m_PropertyDefaultValues[GAIN];

			Table[i].Name			= L"Gain";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[GAIN];

			//Exposure
			pdx->m_StreamProperties[EXPOSURE] = pdx->m_PropertyDefaultValues[EXPOSURE];

			Table[i].Name			= L"Exposure";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[EXPOSURE];

			//Focus
			pdx->m_StreamProperties[FOCUS] = pdx->m_PropertyDefaultValues[FOCUS];

			Table[i].Name			= L"Focus";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[FOCUS];

			// shawn 2011/05/31 +++++
			//Focus Auto
			pdx->m_StreamProperties[FOCUS_AUTO] = pdx->m_PropertyDefaultValues[FOCUS_AUTO];

			Table[i].Name			= L"FocusAuto";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[FOCUS_AUTO];
			// shawn 2011/05/31 -----

			//Iris
			pdx->m_StreamProperties[IRIS] = pdx->m_PropertyDefaultValues[IRIS];

			Table[i].Name			= L"Iris";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[IRIS];

			//Zoom
			pdx->m_StreamProperties[ZOOM] = pdx->m_PropertyDefaultValues[ZOOM];

			Table[i].Name			= L"Zoom";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[ZOOM];

			//Pan
			pdx->m_StreamProperties[PAN] = pdx->m_PropertyDefaultValues[PAN];

			Table[i].Name			= L"Pan";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[PAN];

			//Tilt
			pdx->m_StreamProperties[TILT] = pdx->m_PropertyDefaultValues[TILT];

			Table[i].Name			= L"Tilt";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[TILT];

			//Roll
			pdx->m_StreamProperties[ROLL] = pdx->m_PropertyDefaultValues[ROLL];

			Table[i].Name			= L"Roll";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[ROLL];

			//AEMode Control
			pdx->m_AEMode.DefValue = pdx->m_AEMode.DefValueHW;

			Table[i].Name			= L"AEModeControl";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_AEMode.DefValue;

			//PRIVACY
			pdx->m_StreamProperties[PRIVACY] = pdx->m_PropertyDefaultValues[PRIVACY];

			Table[i].Name			= L"Privacy";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_StreamProperties[PRIVACY];

			//AEPriority Control
			pdx->m_StreamProperties[AEPRIORITY] = pdx->m_PropertyDefaultValues[AEPRIORITY];

			Table[i].Name			= L"AEPriorityControl";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			//2011/1/5 06:36下午
			Table[i++].EntryContext	= &pdx->m_StreamProperties[AEPRIORITY];//&pdx->m_AEPriority.DefValue;

			//Mirror
			//pdx->m_Mirror.DefValue = pdx->m_Mirror.DefValueHW;

			Table[i].Name			= L"Mirror";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_Mirror.DefValue;

			//Flip
			//pdx->m_Flip.DefValue = pdx->m_Flip.DefValueHW;

			Table[i].Name			= L"Flip";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_Flip.DefValue;

			//BWMode
			//pdx->m_BWMode.DefValue = pdx->m_BWMode.DefValueHW;

			Table[i].Name			= L"BWMode";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_BWMode.DefValue;

			// shawn 2011/05/25 +++++
			//Rotation
			//pdx->m_Rotation.DefValue = pdx->m_Rotation.DefValueHW;

			Table[i].Name			= L"Rotation";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_Rotation.DefValue;

			//AUXLED Mode
			if(!pdx->m_XU_AuxLED_Dis)
			{
				//pdx->m_AuxLedMode.DefValue = pdx->m_AuxLedMode.DefValueHW;

				Table[i].Name			= L"AuxLedMode";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_AuxLedMode.DefValue;
			}

			// Snapshot Control
			//pdx->m_Snapshot_Ctrl.DefValue = pdx->m_Snapshot_Ctrl.DefValueHW;

			Table[i].Name			= L"DeNoiseMode";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_Snapshot_Ctrl.DefValue;

			//DeNoiseMode
			//pdx->m_DeNoiseMode.DefValue = pdx->m_DeNoiseMode.DefValueHW;

			Table[i].Name			= L"DeNoiseMode";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_DeNoiseMode.DefValue;

			// WhoteBalanceComponentGreen
			//pdx->m_WBC_Green.DefValue = pdx->m_WBC_Green.DefValueHW;

			Table[i].Name			= L"WBComponentGreen";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
			Table[i++].EntryContext	= &pdx->m_WBC_Green.DefValue;
			// shawn 2011/05/25 -----

			// shawn 2011/05/31 +++++
			if (!bCustom)
			{
				//DefectCompensation
				Table[i].Name			= L"DefectCompensation";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_DefectMode;

				//DefectYThd
				Table[i].Name			= L"DefectYThd";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_iYThd;

				//DefectCThd
				Table[i].Name			= L"DefectCThd";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_iCThd;

				//OSVersion
				//Table[i].Name			= L"OSVersion";
				//Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				//Table[i++].EntryContext	= &pdx->m_OSVersion;

				//Suspend Enable
				Table[i].Name			= L"SuspendEnable";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_C3Enable;

				//Autostart Enable
				Table[i].Name			= L"AutoStartEnable";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_Autostart_Enable;

				//EP0 Polling
				//pdx->m_Ep0_Polling = FALSE;

				//Table[i].Name			= L"EP0Polling";
				//Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				//Table[i++].EntryContext	= &pdx->m_Ep0_Polling;
					
				//FrameFileName
				wcscpy(pdx->pVideoDevice->m_FrameFileName, L"\\DosDevices\\");

				Table[i].Name			= L"FrameFileName";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->pVideoDevice->m_FrameFileName;
					
				// SXGA2 Still Format Control
				//Table[i].Name			= L"SXGA2StillFormatControl";
				//Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				//Table[i++].EntryContext	= &pdx->m_SXGA2_Still_Format;
				
				//PropertyPageControl
				pdx->m_PageCtrl |= 0x2f;

				Table[i].Name			= L"PropertyPageControl";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_PageCtrl;

				//StrgLThd
				pdx->m_StrgLThd = 15;

				Table[i].Name			= L"StrgLThd";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_StrgLThd;

				//StrgCThd
				pdx->m_StrgCThd = 10;

				Table[i].Name			= L"StrgCThd";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_StrgCThd;

				//NzPwrLThd
				pdx->m_NzPwrLThd = 20;

				Table[i].Name			= L"NzPwrLThd";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_NzPwrLThd;

				//NzPwrCThd
				pdx->m_NzPwrCThd = 12;

				Table[i].Name			= L"NzPwrCThd";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_NzPwrCThd;

				//LowLightThd
				pdx->m_LowLightThd = 80;

				Table[i].Name			= L"LowLightThd";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_LowLightThd;

				//MeanDiffYThd
				pdx->m_MeanDiffYThd = 8;

				Table[i].Name			= L"MeanDiffYThd";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_MeanDiffYThd;

				//MeanDiffCThd
				pdx->m_MeanDiffCThd = 5;

				Table[i].Name			= L"MeanDiffCThd";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_MeanDiffCThd;

				//DeNoiseMode_UIEnable
				pdx->m_DeNoiseMode_UIEnable = 0;

				Table[i].Name			= L"DeNoiseMode_UIEnable";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_DeNoiseMode_UIEnable;

				// Interrupt
				//pdx->m_Disable_Interrupt = 0;

				//Table[i].Name			= L"InteruptDisable";
				//Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				//Table[i++].EntryContext	= &pdx->m_Disable_Interrupt;

				//SSWAITTIME
				//pdx->m_Disable_Interrupt = 480;

				//Table[i].Name			= L"SSWAITTIME";
				//Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				//Table[i++].EntryContext	= &pdx->m_SS_WaitTime;

				//SWPRIVACY
				//pdx->m_SW_Privacy = 0;

				//Table[i].Name			= L"SWPRIVACY";
				//Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				//Table[i++].EntryContext	= &pdx->m_SW_Privacy;

				// shawn 2011/07/21 +++++
				//TSMuxEnable
				Table[i].Name			= L"TSMuxEnable";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT;
				Table[i++].EntryContext	= &pdx->m_TSMUX_Enable;
				// shawn 2011/07/21 -----
			}
			// shawn 2011/05/31 -----

			ASSERT(DEFAULT_REGISTRY_NUMBER >= i);

			ntStatus = RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
												BasePath.Buffer,	// shawn 2011/05/31 modify
												Table,
												NULL, 
												NULL);

			// shawn 2011/05/31 +++++
			if (!bCustom)
			{
				RtlZeroMemory(Table, DEFAULT_REGISTRY_NUMBER*sizeof(RTL_QUERY_REGISTRY_TABLE));
				
				for(j=0; j < ftpCount; ++j)
				{
					Table[0].Name = (PWSTR)pCurStrBuf;
					swprintf(Table[0].Name, L"FaceTrack%04d", j);
					Table[0].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
					Table[0].EntryContext	= &pdx->pVideoDevice->m_FaceTrackParams[j];

					ntStatus = RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
														BasePath.Buffer,	// shawn 2011/05/31 modify
														Table,
														NULL, 
														NULL);

					if(!NT_SUCCESS(ntStatus))
					{
						delete [] Table;
						return ntStatus;
					}
				}

				// james 2009/06/08 add for avoiding conflict.
				/*if(pdx->pVideoDevice->m_ZoomParam.bZoomEnable == 1)
					pdx->pVideoDevice->m_FaceTrackParams[0] = pdx->pVideoDevice->m_ZoomParam.bShowUIControl;
				//mark 20070215
				//20061018 0 : hide Face UI and disable Face, 1 : show Face UI and disable Face, 2 : show Face UI and enable Face
				if(pdx->pVideoDevice->m_FaceTrackParams[0] == 0)
				{
					//pDevExt->m_zoom_enable_option = 0;	// james 2009/06/08 delete.
					pdx->pVideoDevice->m_ZoomParam.bShowUIControl = 0;
				}	
				else if(pdx->pVideoDevice->m_FaceTrackParams[0] == 1)
				{
					//pDevExt->m_zoom_enable_option = 0;	// james 2009/06/08 delete.
					pdx->pVideoDevice->m_ZoomParam.bShowUIControl = 1;	
				}
				else if(pdx->pVideoDevice->m_FaceTrackParams[0] == 2)	
				{
					//pDevExt->m_zoom_enable_option = 2;	// james 2009/06/08 delete.
					pdx->pVideoDevice->m_ZoomParam.bShowUIControl = 1;	
				}
				// james 2009/06/09 add for avoiding conflict.
				//pDevExt->m_show_Face_UI = (pDevExt->m_FaceTrackParams[0] != 0);
				if(pdx->pVideoDevice->m_ZoomParam.bZoomEnable == 0)
					pdx->pVideoDevice->m_FaceTrackParams[0] = 0;*/
			}
			// shawn 2011/05/31 -----
		}
		else
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		
		//2010/8/11 07:11下午
		if(Table)
			delete [] Table;	
	}
	
	// shawn 2011/05/27 remove
	/*if(NT_SUCCESS(ntStatus))
	{
		ULONG	wIndex=pdx->m_PropertyEntityID<<8;
		ULONG	wValue;
		LONG	buffer;
		ULONG	BufferLength=sizeof(buffer);

		// BRIGHTNESS
		// use default values from registry as device's initial values		
		buffer = pdx->m_PropertyDefaultValues[BRIGHTNESS];

		if ((pdx->m_PropertyControls&1)&&
			(buffer >= pdx->m_RangeAndStepProp[BRIGHTNESS].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[BRIGHTNESS].Bounds.SignedMaximum) )
		{
			wValue=PU_BRIGHTNESS_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[BRIGHTNESS] = pdx->m_PropertyDefaultValues[BRIGHTNESS];
			DBGU_TRACE("Set Brightness Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[CONTRAST];

		if (((pdx->m_PropertyControls>>1)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[CONTRAST].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[CONTRAST].Bounds.SignedMaximum) )
		{
			wValue=PU_CONTRAST_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[CONTRAST] = pdx->m_PropertyDefaultValues[CONTRAST];
			DBGU_TRACE("Set Contrast Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[HUE];

		if (((pdx->m_PropertyControls>>2)&1) &&
			(buffer >= pdx->m_RangeAndStepProp[HUE].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[HUE].Bounds.SignedMaximum) )
		{
			wValue=PU_HUE_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[HUE] = pdx->m_PropertyDefaultValues[HUE];
			DBGU_TRACE("Set Hue Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[SATURATION];

		if (((pdx->m_PropertyControls>>3)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[SATURATION].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[SATURATION].Bounds.SignedMaximum) )
		{
			wValue=PU_SATURATION_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[SATURATION] = pdx->m_PropertyDefaultValues[SATURATION];
			DBGU_TRACE("Set Saturation Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[SHARPNESS];

		if (((pdx->m_PropertyControls>>4)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[SHARPNESS].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[SHARPNESS].Bounds.SignedMaximum) )
		{
			wValue=PU_SHARPNESS_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[SHARPNESS] = pdx->m_PropertyDefaultValues[SHARPNESS];
			DBGU_TRACE("Set Sharpness Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[GAMMA];
		
		if (((pdx->m_PropertyControls>>5)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[GAMMA].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[GAMMA].Bounds.SignedMaximum) )
		{
			wValue=PU_GAMMA_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[GAMMA] = pdx->m_PropertyDefaultValues[GAMMA];
			DBGU_TRACE("Set Gmma Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[WHITEBALANCE];
		
		if (((pdx->m_PropertyControls>>6)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[WHITEBALANCE].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[WHITEBALANCE].Bounds.SignedMaximum) )
		{
			wValue=PU_WHITE_BALANCE_TEMPERATURE_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[WHITEBALANCE] = pdx->m_PropertyDefaultValues[WHITEBALANCE];
			DBGU_TRACE("Set WhiteBalance Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_WBC.CurrentValue;

		// shawn 2011/05/27 +++++
		if(!pdx->m_XU_WBC_Dis)
		{
			if(((buffer>>8) >= (pdx->m_WBC.MinValue>>8)) &&((buffer&0xff) >= (pdx->m_WBC.MinValue&0xff))&&
			   ((buffer>>8) <= (pdx->m_WBC.MaxValue>>8)) &&((buffer&0xff) <= (pdx->m_WBC.MaxValue&0xff)))
			{
				Set_Cur_WBC_Value(buffer);
				//2007/8/9 05:09下午
				DbgPrint("Set WBComponentControl Current =0x%x\n",buffer);
				pdx->m_WBC.CurrentValue = buffer;
			}
			else
			{
				DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
				//ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
		// shawn 2011/05/27 -----
		else
		{
			if (((pdx->m_PropertyControls>>7)&1)&&
				((buffer>>8) >= (pdx->m_WBC.MinValue>>8)) &&((buffer&0xff) >= (pdx->m_WBC.MinValue&0xff))&&
				((buffer>>8) <= (pdx->m_WBC.MaxValue>>8)) &&((buffer&0xff) <= (pdx->m_WBC.MaxValue&0xff)) )
			{  
				wValue=PU_WHITE_BALANCE_COMPONENT_CONTROL<<8;
				pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
				pdx->m_WBC.DefValue = pdx->m_WBC.DefValueHW;
				DBGU_TRACE("Set WhiteBalance Component Current =%d\n",buffer);
			}
			else
			{
				DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
				//ntStatus = STATUS_INVALID_PARAMETER;
			}
		}

		buffer = pdx->m_WBCAuto.CurrentValue;

		if (((pdx->m_PropertyControls>>13)&1)&&(buffer >= 0) && (buffer <= 1) ) 
		{  
			wValue=PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_WBCAuto.DefValue = pdx->m_WBCAuto.DefValueHW;
			DBGU_TRACE("Set  WhiteBalance Component Auto Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[BACKLIGHT];

		if (((pdx->m_PropertyControls>>8)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[BACKLIGHT].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[BACKLIGHT].Bounds.SignedMaximum) )
		{
			wValue=PU_BACKLIGHT_COMPENSATION_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[BACKLIGHT] = pdx->m_PropertyDefaultValues[BACKLIGHT];
			DBGU_TRACE("Set Back Light Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[GAIN];

		if (((pdx->m_PropertyControls>>9)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[GAIN].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[GAIN].Bounds.SignedMaximum) )
		{
			wValue=PU_GAIN_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[GAIN] = pdx->m_PropertyDefaultValues[GAIN];
			DBGU_TRACE("Set Gain Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[POWERLINEFREQUENCY];

		if (((pdx->m_PropertyControls>>10)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[POWERLINEFREQUENCY].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[POWERLINEFREQUENCY].Bounds.SignedMaximum) )
		{
			wValue=PU_POWER_LINE_FREQUENCY_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[POWERLINEFREQUENCY] = pdx->m_PropertyDefaultValues[POWERLINEFREQUENCY];
			DBGU_TRACE("Set Power Line Frequency Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		wIndex = pdx->m_InputEntityID<<8;

		buffer = pdx->m_PropertyDefaultValues[EXPOSURE];
		
		if (((pdx->m_InputControls>>3)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMaximum) )
		{
			wValue=CT_EXPOSURE_TIME_ABSOLUTE_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[EXPOSURE] = pdx->m_PropertyDefaultValues[EXPOSURE];
			DBGU_TRACE("Set Exposure Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[FOCUS];
		
		if (((pdx->m_InputControls>>5)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[FOCUS].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[FOCUS].Bounds.SignedMaximum) )
		{
			wValue=CT_FOCUS_ABSOLUTE_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[FOCUS] = pdx->m_PropertyDefaultValues[FOCUS];
			DBGU_TRACE("Set Focus Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[IRIS];
		
		if (((pdx->m_InputControls>>7)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[IRIS].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[IRIS].Bounds.SignedMaximum) )
		{
			wValue=CT_IRIS_ABSOLUTE_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					pdx->m_StreamProperties[IRIS] = pdx->m_PropertyDefaultValues[IRIS];
			DBGU_TRACE("Set Iris Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[ZOOM];
		
		if (((pdx->m_InputControls>>9)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[ZOOM].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[ZOOM].Bounds.SignedMaximum) )
		{
			wValue=CT_ZOOM_ABSOLUTE_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[ZOOM] = pdx->m_PropertyDefaultValues[ZOOM];
			DBGU_TRACE("Set Zoom Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}
		
		buffer = pdx->m_PropertyDefaultValues[PANTILT];
		
		if (((pdx->m_InputControls>>11)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[PANTILT].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[PANTILT].Bounds.SignedMaximum) )
		{
			wValue=CT_PANTILT_ABSOLUTE_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[PANTILT] = pdx->m_PropertyDefaultValues[PANTILT];
			DBGU_TRACE("Set PanTilt Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[ROLL];

		if (((pdx->m_InputControls>>13)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[ROLL].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[ROLL].Bounds.SignedMaximum) )
		{
			wValue=CT_ROLL_ABSOLUTE_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[ROLL] = pdx->m_PropertyDefaultValues[ROLL];
			DBGU_TRACE("Set Roll Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}

		buffer = pdx->m_PropertyDefaultValues[PRIVACY];

		if (((pdx->m_InputControls>>18)&1)&&
			(buffer >= pdx->m_RangeAndStepProp[PRIVACY].Bounds.SignedMinimum) && 
			(buffer <= pdx->m_RangeAndStepProp[PRIVACY].Bounds.SignedMaximum) )
		{
			wValue=CT_PRIVACY_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			pdx->m_StreamProperties[PRIVACY] = pdx->m_PropertyDefaultValues[PRIVACY];
			DBGU_TRACE("Set Privacy Current =%d\n",buffer);
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}
		
		//2010/8/25 04:41下午 test for freetalk
		//buffer = pdx->m_AEMode.DefValue;
		//
		//if (((pdx->m_InputControls>>1)&1)&&
		//	(buffer >= 1) && (buffer <= 8) )
		//{
		//	//2010/8/25 04:18下午
		//	buffer = 1;
		//	wValue=CT_AE_MODE_CONTROL<<8;
		//	pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
		//	pdx->m_AEMode.CurrentValue = pdx->m_AEMode.DefValue;
		//	DBGU_TRACE("Set AEModeControl Current =%d\n",buffer);
		//}
		//else
		//{
		//	DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
		//	//ntStatus = STATUS_INVALID_PARAMETER;
		//}

		//2011/1/5 06:33下午
		buffer = pdx->m_PropertyDefaultValues[AEPRIORITY];
		//buffer = pdx->m_AEPriority.DefValue;

		if (((pdx->m_InputControls>>2)&1)&&
			(buffer <= 1) )
		{
			wValue=CT_AE_PRIORITY_CONTROL<<8;
			pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
			//pdx->m_AEPriority.CurrentValue = pdx->m_AEPriority.DefValue;
			DBGU_TRACE("Set AEPriorityControl Current =%d\n",buffer);
			//2011/1/5 06:32下午
			pdx->m_StreamProperties[AEPRIORITY] = pdx->m_PropertyDefaultValues[AEPRIORITY];
		}
		else
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}
		
		if (pdx->m_Mirror.DefValue > 1)
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}
		else
			pdx->m_Mirror.CurrentValue = pdx->m_Mirror.DefValue;

		if (pdx->m_Flip.DefValue > 1)
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}
		else
			pdx->m_Flip.CurrentValue = pdx->m_Flip.DefValue;

		if (pdx->m_BWMode.DefValue > 1)
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
			//ntStatus = STATUS_INVALID_PARAMETER;
		}
		else
			pdx->m_BWMode.CurrentValue = pdx->m_BWMode.DefValue;

		// shawn 2011/05/25 +++++
		if (pdx->m_Rotation.DefValue > 4)
		{
			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
		}
		else
			pdx->m_Rotation.CurrentValue = pdx->m_Rotation.DefValue;

		if(!pdx->m_XU_AuxLED_Dis)  
			pdx->m_AuxLedMode.CurrentValue = pdx->m_AuxLedMode.DefValue;
		// shawn 2011/05/25 -----
	}*/

	DBGU_TRACE("Load Default Settings return %X\n",ntStatus);
	return ntStatus;
}

NTSTATUS 
CVideoStream::CurrentSettings(IN BOOL fInstance, IN BOOL fLoad)
{
    NTSTATUS ntStatus=STATUS_SUCCESS;

	PDEVEXT pdx = CurrentDevExt();
	RTL_QUERY_REGISTRY_TABLE *Table=NULL;
	UNICODE_STRING	RegPath;
	int i=0;

	//2011/5/18 03:06下午 test
	/*LONG	buffer;
	ULONG	BufferLength;
	ULONG	wIndex;
	ULONG	wValue; */
	
	DBGU_TRACE("CurrentSettings()\n");
	
	if (fInstance)
		//RtlInitUnicodeString(&RegPath, (PWSTR) pdx->pVideoDevice->m_InstanceKey);
		RtlInitUnicodeString(&RegPath, (PWSTR) m_DevInstanceKey);
	else
		RtlInitUnicodeString(&RegPath, (PWSTR) pdx->pVideoDevice->m_CurValuePath);

	Table = new (NonPagedPool) RTL_QUERY_REGISTRY_TABLE[DEFAULT_REGISTRY_NUMBER];

	if (Table)
	{
		if (fLoad)	// Get current Settings
		{
			// Brightness
			Table[i].Name			= L"Brightness";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[BRIGHTNESS];

			// Contrast
			Table[i].Name			= L"Contrast";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[CONTRAST];

			// Hue
			Table[i].Name			= L"Hue";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[HUE];

			// Saturation
			Table[i].Name			= L"Saturation";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[SATURATION];

			// Sharpness
			Table[i].Name			= L"Sharpness";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[SHARPNESS];

			// Gamma
			Table[i].Name			= L"Gamma";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[GAMMA];

			// WhiteBalance
			Table[i].Name			= L"WhiteBalance";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[WHITEBALANCE];

			// WhiteBalance Auto
			Table[i].Name			= L"WBTemperatureAuto";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_WBTAuto.CurrentValue;

			// WhiteBalanceComponent
			Table[i].Name			= L"WBComponent";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_WBC.CurrentValue;

			// WhiteBalanceComponent Auto
			Table[i].Name			= L"WBComponentAuto";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_WBCAuto.CurrentValue;

			// BackLight
			Table[i].Name			= L"BackLight";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[BACKLIGHT];

			// PowerLineFrequency
			Table[i].Name			= L"PowerLineFrequency";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[POWERLINEFREQUENCY];

			// Gain
			Table[i].Name			= L"Gain";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[GAIN];

			// Exposure
			Table[i].Name			= L"Exposure";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[EXPOSURE];

			// shawn 2011/05/31 +++++
			// Focus Auto
			Table[i].Name			= L"FocusAuto";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[FOCUS_AUTO];
			// shawn 2011/05/31 -----
			
			// Focus
			Table[i].Name			= L"Focus";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[FOCUS];

			// Iris
			Table[i].Name			= L"Iris";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[IRIS];

			// Zoom
			Table[i].Name			= L"Zoom";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[ZOOM];

			// PanTilt; 		
			Table[i].Name			= L"Pan";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[PAN];
			Table[i].Name			= L"Tilt";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[TILT];
			
			// Roll
			Table[i].Name			= L"Roll";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[ROLL];

			// Privacy
			Table[i].Name			= L"Privacy";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[PRIVACY];

			// AEMode Control
			Table[i].Name			= L"AEModeControl";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[AEMODE];

			// shawn 2011/05/25 +++++
			// AEPriority Control
			Table[i].Name			= L"AEPriorityControl";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_PropertyCurrentValues[AEPRIORITY];

			// Mirror
			Table[i].Name			= L"Mirror";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_Mirror.CurrentValue;

			// Flip
			Table[i].Name			= L"Flip";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_Flip.CurrentValue;

			// BW Mode
			Table[i].Name			= L"BWMode";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_BWMode.CurrentValue;

			// Rotation
			Table[i].Name			= L"Rotation";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_Rotation.CurrentValue;

			// AuxLed Mode
			if(!pdx->m_XU_AuxLED_Dis)
			{
				Table[i].Name			= L"AuxLedMode";
				Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
				Table[i++].EntryContext	= &pdx->m_AuxLedMode.CurrentValue;
			}

			// Snapshot Control
			Table[i].Name			= L"SnapshotControl";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_Snapshot_Ctrl.CurrentValue;

			// DeNoiseMode
			Table[i].Name			= L"DeNoiseMode";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_DeNoiseMode.CurrentValue;

			// WhiteBalanceComponentGreen
			Table[i].Name			= L"WBComponentGreen";
			Table[i].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
			Table[i++].EntryContext	= &pdx->m_WBC_Green.CurrentValue;
			// shawn 2011/05/25 -----

			ASSERT(i <= DEFAULT_REGISTRY_NUMBER);

			ntStatus = RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
												RegPath.Buffer,
												Table,
												NULL, 
												NULL);

			if(NT_SUCCESS(ntStatus))
			{
				
				ULONG	wIndex=pdx->m_PropertyEntityID<<8;
				ULONG	wValue;
				LONG	buffer;
				ULONG	BufferLength=sizeof(buffer);

				// BRIGHTNESS
				// use default values from registry as device's initial values		
				buffer = pdx->m_PropertyCurrentValues[BRIGHTNESS];

				if ((pdx->m_PropertyControls&1)&&
					(buffer >= pdx->m_RangeAndStepProp[BRIGHTNESS].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[BRIGHTNESS].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=PU_BRIGHTNESS_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Brightness Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}
				
				// Contrast
				buffer = pdx->m_PropertyCurrentValues[CONTRAST];

				if (((pdx->m_PropertyControls>>1)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[CONTRAST].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[CONTRAST].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=PU_CONTRAST_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Contrast Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Hue
				buffer = pdx->m_PropertyCurrentValues[HUE];

				if (((pdx->m_PropertyControls>>2)&1) &&
					(buffer >= pdx->m_RangeAndStepProp[HUE].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[HUE].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=PU_HUE_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Hue Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Saturation
				buffer = pdx->m_PropertyCurrentValues[SATURATION];

				if (((pdx->m_PropertyControls>>3)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[SATURATION].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[SATURATION].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=PU_SATURATION_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Saturation Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Sharpness
				buffer = pdx->m_PropertyCurrentValues[SHARPNESS];

				if (((pdx->m_PropertyControls>>4)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[SHARPNESS].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[SHARPNESS].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=PU_SHARPNESS_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Sharpness Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Gamma
				buffer = pdx->m_PropertyCurrentValues[GAMMA];

				if (((pdx->m_PropertyControls>>5)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[GAMMA].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[GAMMA].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=PU_GAMMA_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Gmma Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// WhiteBalance
				buffer = pdx->m_PropertyCurrentValues[WHITEBALANCE];

				if (((pdx->m_PropertyControls>>6)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[WHITEBALANCE].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[WHITEBALANCE].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=PU_WHITE_BALANCE_TEMPERATURE_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set WhiteBalance Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// WhiteBalance Auto
				buffer = pdx->m_WBTAuto.CurrentValue;

				if (((pdx->m_PropertyControls>>12)&1)&&
					(buffer >= 0) && (buffer <= 1))
				{
					BufferLength = 1;
					wValue=PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set WhiteBalance Auto Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// WhiteBalanceComponent
				buffer = pdx->m_WBC.CurrentValue;

				// shawn 2011/05/27 +++++
				if(!pdx->m_XU_WBC_Dis)
				{
					if(((buffer>>8) >= (pdx->m_WBC.MinValue>>8)) &&((buffer&0xff) >= (pdx->m_WBC.MinValue&0xff))&&
					   ((buffer>>8) <= (pdx->m_WBC.MaxValue>>8)) &&((buffer&0xff) <= (pdx->m_WBC.MaxValue&0xff)))
					{
						ntStatus = Set_Cur_WBC_Value(buffer);
						//2008/8/27 07:22下午
						DbgPrint("Set WHITEBALANCEComponent Current =%d\n",buffer);	
					}
					else
					{
						DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
						ntStatus = STATUS_INVALID_PARAMETER;
					}
				}
				// shawn 2011/05/27 -----
				else
				{
					if (((pdx->m_PropertyControls>>7)&1)&&
						((buffer>>8) >= (pdx->m_WBC.MinValue>>8)) &&((buffer&0xff) >= (pdx->m_WBC.MinValue&0xff))&&
						((buffer>>8) <= (pdx->m_WBC.MaxValue>>8)) &&((buffer&0xff) <= (pdx->m_WBC.MaxValue&0xff)) )
					{  
						BufferLength = 4;
						wValue=PU_WHITE_BALANCE_COMPONENT_CONTROL<<8;
						pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
						DBGU_TRACE("Set WhiteBalance Component Current =%d\n",buffer);
					}
					else
					{
						DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
						ntStatus = STATUS_INVALID_PARAMETER;
					}
				}

				// WhiteBalanceComponent Auto
				buffer = pdx->m_WBCAuto.CurrentValue;

				if(!pdx->m_XU_WBCAuto_Dis)
				{
					if ((buffer >= 0) && (buffer <= 1)) 
					{  
						Set_Cur_WBCAuto_Value(buffer);
						DbgPrint("Set WHITEBALANCEComponentAuto Current =%d\n",buffer);
					}
					else
					{
						DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
						ntStatus = STATUS_INVALID_PARAMETER;
					}
				}
				else
				{
					if (((pdx->m_PropertyControls>>13)&1)&&(buffer >= 0) && (buffer <= 1) ) 
					{  
						BufferLength = 1;
						wValue=PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL<<8;
						pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
						DBGU_TRACE("Set  WhiteBalance Component Auto Current =%d\n",buffer);
					}
					else
					{
						DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
						ntStatus = STATUS_INVALID_PARAMETER;
					}
				}

				// BackLight
				buffer = pdx->m_PropertyCurrentValues[BACKLIGHT];

				if (((pdx->m_PropertyControls>>8)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[BACKLIGHT].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[BACKLIGHT].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=PU_BACKLIGHT_COMPENSATION_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Back Light Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Gain
				buffer = pdx->m_PropertyCurrentValues[GAIN];

				if (((pdx->m_PropertyControls>>9)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[GAIN].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[GAIN].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=PU_GAIN_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Gain Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// PowerLineFrequency
				buffer = pdx->m_PropertyCurrentValues[POWERLINEFREQUENCY];

				if (((pdx->m_PropertyControls>>10)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[POWERLINEFREQUENCY].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[POWERLINEFREQUENCY].Bounds.SignedMaximum) )
				{
					BufferLength = 1;
					wValue=PU_POWER_LINE_FREQUENCY_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Power Line Frequency Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				
				wIndex = pdx->m_InputEntityID<<8;

				// AEMode Control
				buffer = pdx->m_AEMode.CurrentValue = pdx->m_PropertyCurrentValues[AEMODE];

				if (((pdx->m_InputControls>>1)&1)&&
					(buffer >= 1) && (buffer <= 8) )
				{
					BufferLength = 1;
					wValue=CT_AE_MODE_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set AEModeControl Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Exposure
				buffer = pdx->m_PropertyCurrentValues[EXPOSURE];

				if (((pdx->m_InputControls>>3)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[EXPOSURE].Bounds.SignedMaximum) )
				{
					BufferLength = 4;
					wValue=CT_EXPOSURE_TIME_ABSOLUTE_CONTROL<<8;
					LONG exposure_time = 10000 * pow(2.0, (int)buffer);  
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&exposure_time,BufferLength,FALSE);
					DBGU_TRACE("Set Exposure Current =%d\n",exposure_time);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Focus Auto
				buffer = pdx->m_PropertyCurrentValues[FOCUS_AUTO];

				if (((pdx->m_InputControls>>17)&1)&&
					 (buffer >= pdx->m_RangeAndStepProp[FOCUS_AUTO].Bounds.SignedMinimum) && 
					 (buffer <= pdx->m_RangeAndStepProp[FOCUS_AUTO].Bounds.SignedMaximum) ) 
				{  
					BufferLength = 1;
					wValue=CT_FOCUS_AUTO_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DbgPrint("Set FOCUSAUTO Current =%d\n",buffer);			
				}
				else
				{
	    			DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}
				
				// Focus
				buffer = pdx->m_PropertyCurrentValues[FOCUS];

				if (((pdx->m_InputControls>>5)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[FOCUS].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[FOCUS].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=CT_FOCUS_ABSOLUTE_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Focus Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Iris
				buffer = pdx->m_PropertyCurrentValues[IRIS];

				if (((pdx->m_InputControls>>7)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[IRIS].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[IRIS].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=CT_IRIS_ABSOLUTE_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Iris Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Zoom
				buffer = pdx->m_PropertyCurrentValues[ZOOM];

				if (((pdx->m_InputControls>>9)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[ZOOM].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[ZOOM].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=CT_ZOOM_ABSOLUTE_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Zoom Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}
				
				// PanTilt
				struct _PanTilt
				{
					long dwPan;
					long dwTilt;
				} PanTiltValue, PanTiltMax, PanTiltMin;
				
				PanTiltValue.dwPan = pdx->m_PropertyCurrentValues[PAN];
				PanTiltValue.dwTilt = pdx->m_PropertyCurrentValues[TILT];	
				PanTiltMax.dwPan = pdx->m_RangeAndStepProp[PAN].Bounds.SignedMaximum;
				PanTiltMin.dwPan = pdx->m_RangeAndStepProp[PAN].Bounds.SignedMinimum;	
				PanTiltMax.dwTilt = pdx->m_RangeAndStepProp[TILT].Bounds.SignedMaximum;
				PanTiltMin.dwTilt = pdx->m_RangeAndStepProp[TILT].Bounds.SignedMinimum;	
					
				//buffer = pdx->m_PropertyCurrentValues[PANTILT];
				if (((pdx->m_InputControls>>11)&1)
					 &&(PanTiltValue.dwPan >= PanTiltMin.dwPan) && (PanTiltValue.dwPan <= PanTiltMax.dwPan)
					 &&(PanTiltValue.dwTilt >= PanTiltMin.dwTilt) && (PanTiltValue.dwTilt <= PanTiltMax.dwTilt) )
				{
					BufferLength = 8;
					wValue=CT_PANTILT_ABSOLUTE_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&PanTiltValue,8,FALSE);
					DBGU_TRACE("Set PANTILT Current =%d, %d\n",PanTiltValue.dwPan,PanTiltValue.dwTilt);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Roll
				buffer = pdx->m_PropertyCurrentValues[ROLL];

				if (((pdx->m_InputControls>>13)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[ROLL].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[ROLL].Bounds.SignedMaximum) )
				{
					BufferLength = 2;
					wValue=CT_ROLL_ABSOLUTE_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Roll Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				// Privacy
				buffer = pdx->m_PropertyCurrentValues[PRIVACY];

				if (((pdx->m_InputControls>>18)&1)&&
					(buffer >= pdx->m_RangeAndStepProp[PRIVACY].Bounds.SignedMinimum) && 
					(buffer <= pdx->m_RangeAndStepProp[PRIVACY].Bounds.SignedMaximum) )
				{
					BufferLength = 1;
					wValue=CT_PRIVACY_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DBGU_TRACE("Set Privacy Current =%d\n",buffer);
				}
				else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

	

				// shawn 2011/05/25 +++++
				// AEPriority Control
				buffer = pdx->m_AEPriority.CurrentValue = pdx->m_PropertyCurrentValues[AEPRIORITY];

				if (((pdx->m_InputControls>>2)&1)&&
					 (buffer >= 0) && 
					 (buffer <= 1) ) 
				{
  					BufferLength = 1;
					wValue=CT_AE_PRIORITY_CONTROL<<8;
					pdx->pVideoDevice->USB_ControlClassCommand(SET_CUR,wValue,wIndex,&buffer,BufferLength,FALSE);
					DbgPrint("Set CT_AE_PRIORITY_CONTROL Current =%d\n",buffer);
				} 
	   			else
				{
					DBGU_WARNING("STATUS_INVALID_PARAMETER\n");
					ntStatus = STATUS_INVALID_PARAMETER;
				}

				pdx->m_SetMirrorOrBW = TRUE;

				// AUXLED Mode
				buffer = pdx->m_AuxLedMode.CurrentValue;
				
				if(!pdx->m_XU_AuxLED_Dis)
				{
					Set_Cur_AUXLED_Value(buffer);
				}

				// WhiteBalanceComponentGreen
				buffer = pdx->m_WBC_Green.CurrentValue;
				
				if(!pdx->m_XU_WBC_Dis)
				{
					if(((buffer&0xff) >= (pdx->m_WBC_Green.MinValue&0xff))&&((buffer&0xff) <= (pdx->m_WBC_Green.MaxValue&0xff)))
					{
						ntStatus = Set_Cur_WBC_Green_Value(buffer);
						DbgPrint("Set WHITEBALANCEComponentGreen Current =%d\n",buffer);					
					}
				}
				// shawn 2011/05/25 -----
			}
		}
		else	// Save current settings
		{
			//Brightness	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Brightness",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[BRIGHTNESS],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Contrast	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Contrast",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[CONTRAST],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Hue	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Hue",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[HUE],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Saturation	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Saturation",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[SATURATION],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Sharpness	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Sharpness",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[SHARPNESS],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Gamma	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Gamma",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[GAMMA],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//WhiteBalance	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"WhiteBalance",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[WHITEBALANCE],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//WhiteBalanceComponent	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"WBComponent",
											REG_DWORD,
											&pdx->m_WBC.CurrentValue,
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//BackLight	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"BackLight",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[BACKLIGHT],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//PowerLineFrequency	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"PowerLineFrequency",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[POWERLINEFREQUENCY],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Gain	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Gain",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[GAIN],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Exposure	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Exposure",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[EXPOSURE],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Focus	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Focus",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[FOCUS],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			// shawn 2011/05/31 +++++
			//FocusAuto
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"FocusAuto",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[FOCUS_AUTO],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;
			// shawn 2011/05/31 -----

			//Iris	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Iris",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[IRIS],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Zoom	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Zoom",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[ZOOM],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Pantilt	
			/*ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Pantilt",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[PANTILT],
											sizeof(ULONG));*/
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Pan",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[PAN],
											sizeof(ULONG));											
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Tilt",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[TILT],
											sizeof(ULONG));											
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;
			
			//Roll	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Roll",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[ROLL],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;
			
			//Privacy	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Privacy",
											REG_DWORD,
											&pdx->m_PropertyCurrentValues[PRIVACY],
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;
												
			//WBTAuto	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"WBTemperatureAuto",
											REG_DWORD,
											&pdx->m_WBTAuto.CurrentValue,
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//WBCAuto	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"WBComponentAuto",
											REG_DWORD,
											&pdx->m_WBCAuto.CurrentValue,
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Mirror	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Mirror",
											REG_DWORD,
											&pdx->m_Mirror.CurrentValue,
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//Flip	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Flip",
											REG_DWORD,
											&pdx->m_Flip.CurrentValue,
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//BWMode	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"BWMode",
											REG_DWORD,
											&pdx->m_BWMode.CurrentValue,
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//AEMode	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"AEModeControl",
											REG_DWORD,
											&pdx->m_AEMode.CurrentValue,
											sizeof(ULONG));
			
			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			//AEPriority	
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"AEPriorityControl",
											REG_DWORD,
											//2011/1/5 06:34下午
											//&pdx->m_AEPriority.CurrentValue,
											&pdx->m_PropertyCurrentValues[AEPRIORITY],
											sizeof(ULONG));

			// shawn 2011/05/25 +++++
			// Rotation
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"Rotation",
											REG_DWORD,
											&pdx->m_Rotation.CurrentValue,
											sizeof(ULONG));

			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			// Snapshot Control
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"SnapshotControl",
											REG_DWORD,
											&pdx->m_Snapshot_Ctrl.CurrentValue,
											sizeof(ULONG));

			// AuxLedMode
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"AuxLedMode",
											REG_DWORD,
											&pdx->m_AuxLedMode.CurrentValue,
											sizeof(ULONG));

			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			// DeNoiseMode
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"DeNoiseMode",
											REG_DWORD,
											&pdx->m_DeNoiseMode.CurrentValue,
											sizeof(ULONG));

			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;

			// WBComponentGreen
			ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
											RegPath.Buffer,
											L"WBComponentGreen",
											REG_DWORD,
											&pdx->m_WBC_Green.CurrentValue,
											sizeof(ULONG));

			if (!NT_SUCCESS(ntStatus))	goto _EXIT_CURRENT_SETTINGS;
			// shawn 2011/05/25 -----
		}

_EXIT_CURRENT_SETTINGS:

		delete [] Table;
	}
	else
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;

	DBGU_TRACE("retrun CurrentSettings status = %X\n",ntStatus);
	return ntStatus;
}

// shawn 2011/05/25 +++++
NTSTATUS  
CVideoStream::Get_ExUnit_Control_Value(
    IN ULONG Cmd, 
	IN ULONG ControlID, 
	IN ULONG ExUnitID, 
	IN ULONG BufferLength, 
	IN UCHAR AddrH, 
	IN UCHAR AddrL,
	OUT PUCHAR GetBuffer)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PUCHAR Data= NULL;
	ULONG Value;
	ULONG Index;
	UCHAR Flags;
	BOOL  bDirect;
	PDEVEXT pdx = CurrentDevExt();
	
	Value = ControlID << 8;
	Index = ExUnitID << 8;
	Data = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, BufferLength, 'Ex');

	if(Data)	
	{
		RtlZeroMemory(Data, BufferLength);
		bDirect = DIRECTION_OUT;
		Flags = SET_CUR;

		switch(ControlID)
		{
		case XU_AUXLED_CONTROL_ID:
			Data[0] = AddrL;
			Data[1] = AddrH;
			Data[6] = XU_CONTROL6_DUMMY_WRITE;
			break;

		case XU_SF_CONTROL_ID:
			Data[0] = AddrL;
			Data[1] = AddrH;
			Data[2] = SF_CMD_DUMMY_WRITE;
			Data[2] |= BufferLength - 3;
		}
	
		ntStatus = pdx->pVideoDevice->USB_ControlClassCommand(Flags, Value, Index, Data, BufferLength, bDirect);
		
		if( NT_SUCCESS(ntStatus)) 
		{
			DbgPrint("set ExUnit dummy request ok \n");

			RtlZeroMemory(Data, BufferLength);
			bDirect = DIRECTION_IN;
			Flags = Cmd;
			Data[0] = AddrL;
			Data[1] = AddrH;

			ntStatus = pdx->pVideoDevice->USB_ControlClassCommand(Flags, Value, Index, Data, BufferLength, bDirect);
			
			if( NT_SUCCESS(ntStatus)) 
			{
				DbgPrint("get ExUnit request ok\n");

				for(int i=0; i<BufferLength; i++)
				{	
					DbgPrint("Data[%d]=%x ",i,Data[i]);
				}

				DbgPrint("\n");
				memcpy(GetBuffer, Data, BufferLength);
			}
			else
			{
				DbgPrint("get ExUnit request fail = %x\n", ntStatus);
				ntStatus = STATUS_UNSUCCESSFUL;
			}
		}
		else
		{
			DbgPrint("set ExUnit dummy request fail = %x\n", ntStatus);
			ntStatus = STATUS_UNSUCCESSFUL;
		}
		
		ExFreePoolWithTag(Data, 'Ex');
	}

	return ntStatus;		
}

NTSTATUS 
CVideoStream::Set_ExUnit_Control_Value(
    IN ULONG Cmd, 
	IN ULONG ControlID, 
	IN ULONG ExUnitID, 
	IN ULONG BufferLength, 
	IN UCHAR AddrH, 
	IN UCHAR AddrL,
	IN PUCHAR SetBuffer)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PUCHAR Data= NULL;
	ULONG Value;
	ULONG Index;
	UCHAR Flags;
	BOOL  bDirect;
	PDEVEXT pdx = CurrentDevExt();
	
	Value = ControlID << 8;
	Index = ExUnitID << 8;
	Data = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, BufferLength, 'Ex');

	if(Data)	
	{
		RtlZeroMemory(Data, BufferLength);
		memcpy(Data, SetBuffer, BufferLength);
		bDirect = DIRECTION_OUT;
		Flags = Cmd;
		Data[0] = AddrL;
		Data[1] = AddrH;
		Data[6] = XU_CONTROL6_NORMAL_WRITE;
	
		ntStatus = pdx->pVideoDevice->USB_ControlClassCommand(Flags, Value, Index, Data, BufferLength, bDirect);
		
		if( NT_SUCCESS(ntStatus)) 
		{
			DbgPrint("set ExUnit request ok \n");
		}
		else
		{
			DbgPrint("set ExUnit request fail = %x\n", ntStatus);
			ntStatus = STATUS_UNSUCCESSFUL;
		}

		ExFreePoolWithTag(Data, 'Ex');
	}

	return ntStatus;		
}

NTSTATUS  
CVideoStream::Get_Cur_AUXLED_Value(OUT PLONG GetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	
	//2008/5/7 02:27下午 for 288P without SF
	// shawn 2010/01/21 fix 288P bug +++++
	/*if (pDevExt->m_is288P)
	{
		RtlZeroMemory(buffer, SF_BUFFER_LEN);
		ntStatus = Get_ExUnit_Control_Value(pDevExt, GET_CUR, XU_SF_CONTROL_ID, pDevExt->m_ExUnitID[0], SF_BUFFER_LEN, SF_DRIVER_PARAM_ADDR_H, SF_DRIVER_PARAM_ADDR_L+1, buffer);
	}*/

	if (pDevExt->m_is288P)
	{
		DbgPrint("Get_Cur_AUXLED_Value =====> m_is288P is TRUE =====>");
		UCHAR buffer[SF_BUFFER_LEN];
		RtlZeroMemory(buffer, SF_BUFFER_LEN);
		ntStatus = Get_ExUnit_Control_Value(GET_CUR, XU_SF_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], SF_BUFFER_LEN, SF_DRIVER_PARAM_ADDR_H, SF_DRIVER_PARAM_ADDR_L, buffer);	// shawn 2010/10/26 modify
  
  		if (NT_SUCCESS(ntStatus))
		{
			*GetBuffer = buffer[AUXLED_DATA_BRIGHT+2];
			*GetBuffer = (*GetBuffer << 8) | buffer[AUXLED_DATA_ATTRIBUTE+2];
		}
 	}

	else
	{
		UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];
		RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
		ntStatus = Get_ExUnit_Control_Value(GET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, AUXLED_ADDR_H, AUXLED_ADDR_L, buffer);	// shawn 2010/10/26 modify
	
		if (NT_SUCCESS(ntStatus))
		{
			*GetBuffer = buffer[AUXLED_DATA_BRIGHT];
			*GetBuffer = (*GetBuffer << 8) | buffer[AUXLED_DATA_ATTRIBUTE];
		}
	}
	// shawn 2010/01/21 fix 288P bug -----

	return ntStatus;
}

NTSTATUS  
CVideoStream::Set_Cur_AUXLED_Value(IN LONG SetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];

	LONG	bright = (SetBuffer >> 8) & 0xff;
	LONG	max_value = pDevExt->m_0x8400_Data[AUXLED_BRIGHT_MAX_ADDR];
	LONG	min_value = pDevExt->m_0x8400_Data[AUXLED_BRIGHT_MIN_ADDR];	
	DbgPrint("Set_Cur_AUXLED_Value min_value=%x, max_value=%x bright=%x\n",
	min_value,max_value, bright);
	
	if((bright >= min_value) && (bright <= max_value))
	{
		RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
		buffer[AUXLED_DATA_ATTRIBUTE] = SetBuffer & 0xff;
		buffer[AUXLED_DATA_BRIGHT] = (SetBuffer >> 8) & 0xff;
		DbgPrint("Set_ExUnit_Control_Value\n"); 	
		ntStatus = Set_ExUnit_Control_Value(SET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, AUXLED_ADDR_H, AUXLED_ADDR_L, buffer);	// shawn 2010/10/26 modify
	} 

	return ntStatus;
}

NTSTATUS  
CVideoStream::Get_Cur_WBC_Value(OUT PLONG GetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];

	RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
	ntStatus = Get_ExUnit_Control_Value(GET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, WBC_BLUE_ADDR_H, WBC_BLUE_ADDR_L, buffer);	// shawn 2010/10/26 modify
	
	if (NT_SUCCESS(ntStatus))
	{
		*GetBuffer = (buffer[WBC_BLUE_DATA+1] << 8);
		*GetBuffer += buffer[WBC_BLUE_DATA];
	}

	RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
	ntStatus = Get_ExUnit_Control_Value(GET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, WBC_RED_ADDR_H, WBC_RED_ADDR_L, buffer);	// shawn 2010/10/26 modify
	
	if (NT_SUCCESS(ntStatus))
	{
		*GetBuffer += (buffer[WBC_RED_DATA+1] << 24);
		*GetBuffer += (buffer[WBC_RED_DATA] << 16);
	}
	
	return ntStatus;
}

NTSTATUS  
CVideoStream::Set_Cur_WBC_Value(IN LONG SetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];
	
	DbgPrint("Set_Cur_WBC_Value\n"); 
	LONG	WBC_blue_value = SetBuffer & 0xffff;
	/*LONG	max_value = pDevExt->m_0x8400_Data[WBC_BLUE_MAX_ADDR_L] + (pDevExt->m_0x8400_Data[WBC_BLUE_MAX_ADDR_H] << 8);
	LONG	min_value = pDevExt->m_0x8400_Data[WBC_BLUE_MIN_ADDR_L] + (pDevExt->m_0x8400_Data[WBC_BLUE_MIN_ADDR_H] << 8);	
	
	DbgPrint("WBC_blue_value = %x,min_value=%x,max_value=%x\n",WBC_blue_value,min_value,max_value); 
	
	if((WBC_blue_value >= min_value) && (WBC_blue_value <= max_value))
	*/
	{
		RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
		buffer[WBC_BLUE_DATA] = WBC_blue_value & 0xff;
		buffer[WBC_BLUE_DATA+1] = (WBC_blue_value >> 8) & 0xff;
		ntStatus = Set_ExUnit_Control_Value(SET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, WBC_BLUE_ADDR_H, WBC_BLUE_ADDR_L, buffer);	// shawn 2010/10/26 modify
	} 
	
	LONG	WBC_red_value = (SetBuffer >> 16) & 0xffff;
	/*max_value = pDevExt->m_0x8400_Data[WBC_RED_MAX_ADDR_L] + (pDevExt->m_0x8400_Data[WBC_RED_MAX_ADDR_H] << 8);
	min_value = pDevExt->m_0x8400_Data[WBC_RED_MIN_ADDR_L] + (pDevExt->m_0x8400_Data[WBC_RED_MIN_ADDR_H] << 8);	

	DbgPrint("WBC_red_value = %x,min_value=%x,max_value=%x\n",WBC_red_value,min_value,max_value); 
	
	if((WBC_red_value >= min_value) && (WBC_blue_value <= max_value))
	*/
	{
		RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
		buffer[WBC_RED_DATA] = WBC_red_value & 0xff;
		buffer[WBC_RED_DATA+1] = (WBC_red_value >> 8) & 0xff;
		ntStatus = Set_ExUnit_Control_Value(SET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, WBC_RED_ADDR_H, WBC_RED_ADDR_L, buffer);	// shawn 2010/10/26 modify
	} 
	
	return ntStatus;
}

NTSTATUS  
CVideoStream::Get_Cur_WBC_Green_Value(OUT PLONG GetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];

	RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
	ntStatus = Get_ExUnit_Control_Value(GET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, WBC_GREEN_ADDR_H, WBC_GREEN_ADDR_L, buffer);	// shawn 2010/10/26 modify
	
	if (NT_SUCCESS(ntStatus))
	{
		*GetBuffer = (buffer[WBC_GREEN_DATA+1] << 8);
		*GetBuffer += buffer[WBC_GREEN_DATA];
	}
	
	return ntStatus;
}

NTSTATUS  
CVideoStream::Set_Cur_WBC_Green_Value(IN LONG SetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];
	
	DbgPrint("Set_Cur_WBC_Green_Value\n"); 
	LONG	WBC_green_value = SetBuffer & 0xffff;
	
	{
		RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
		buffer[WBC_GREEN_DATA] = WBC_green_value & 0xff;
		buffer[WBC_GREEN_DATA+1] = (WBC_green_value >> 8) & 0xff;
		ntStatus = Set_ExUnit_Control_Value(SET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, WBC_GREEN_ADDR_H, WBC_GREEN_ADDR_L, buffer);	// shawn 2010/10/26 modify
	}

	return ntStatus;
}

NTSTATUS  
CVideoStream::Get_Cur_WBCAuto_Value(OUT PLONG GetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];

	RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
	ntStatus = Get_ExUnit_Control_Value(GET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, WBC_AUTO_ADDR_H, WBC_AUTO_ADDR_L, buffer);	// shawn 2010/10/26 modify
	
	if (NT_SUCCESS(ntStatus))
	{
		*GetBuffer = buffer[WBC_AUTO_DATA];
	}

	return ntStatus;
}

NTSTATUS  
CVideoStream::Set_Cur_WBCAuto_Value(IN LONG SetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];

	LONG	WBC_auto_value = SetBuffer & 0xff;

	//if((WBC_auto_value >= 0) && (WBC_auto_value <= 1))
	{
		RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
		buffer[WBC_AUTO_DATA] = WBC_auto_value & 0xff;
		ntStatus = Set_ExUnit_Control_Value(SET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, WBC_AUTO_ADDR_H, WBC_AUTO_ADDR_L, buffer);	// shawn 2010/10/26 modify
	} 
	
	return ntStatus;
}

NTSTATUS  
CVideoStream::Get_Cur_Privacy_Value(OUT PLONG GetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];

	RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
	ntStatus = Get_ExUnit_Control_Value(GET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, PRIVACY_ADDR_H, PRIVACY_ADDR_L, buffer);	// shawn 2010/10/26 modify
	
	if (NT_SUCCESS(ntStatus))
	{
		*GetBuffer = buffer[PRIVACY_DATA];
	}

	return ntStatus;
}

NTSTATUS  
CVideoStream::Set_Cur_Privacy_Value(IN LONG SetBuffer)
{
    PDEVEXT pDevExt = CurrentDevExt();
    NTSTATUS ntStatus = STATUS_SUCCESS;
	UCHAR	buffer[XU_CONTROL6_BUFFER_LEN];

	LONG	privacy_value = SetBuffer & 0x01;

	//if((privacy_value >= 0) && (privacy_value <= 1))
	{
		RtlZeroMemory(buffer, XU_CONTROL6_BUFFER_LEN);
		buffer[PRIVACY_DATA] = privacy_value & 0x01;
		ntStatus = Set_ExUnit_Control_Value(SET_CUR, XU_AUXLED_CONTROL_ID, pDevExt->m_ExUnitID[pDevExt->m_DefXUID], XU_CONTROL6_BUFFER_LEN, PRIVACY_ADDR_H, PRIVACY_ADDR_L, buffer);	// shawn 2010/10/26 modify
	} 
	
	return ntStatus;
}
// shawn 2011/05/25 -----