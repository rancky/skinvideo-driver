/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

	File:

		VideoStream.h

	Abstract:


	History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/
#pragma once

#ifndef __VideoStream_H__
#define __VideoStream_H__

class CVideoStream
{
public:
	//##### Constructor #####//
	CVideoStream(IN PKSDEVICE Device);
	~CVideoStream(void);

	//##### Functions #####//
	static void
	Cleanup (
		IN CVideoStream *pStream
		)
	{
		if (pStream)
			delete pStream; pStream = NULL;
	}

	NTSTATUS	InitializeDevice();
	NTSTATUS	InitializationComplete();

	NTSTATUS	SurpriseRemoval();
	NTSTATUS	UninitializeDevice();

	NTSTATUS	OpenStream(IN PKSPIN pin);
	NTSTATUS	CloseStream(IN PKSPIN pin);

	NTSTATUS
	SetStreamState(
		IN PKSPIN pin,
		IN KSSTATE StreamState
		);

	NTSTATUS	GetDeviceProperty(IN PIRP irp, IN PKSPROPERTY Property, IN OUT PVOID data);
	NTSTATUS	SetDeviceProperty(IN PIRP irp, IN PKSPROPERTY Property, IN OUT PVOID data);
	NTSTATUS	GetStreamProperty(IN PIRP irp, IN PKSPROPERTY Property, IN OUT PVOID data);

	NTSTATUS	
	ReadData(
		IN PKSPIN pin,
		PKSSTREAM_HEADER pDataPacket,
		IN ULONG NumberOfBuffers
		);

	NTSTATUS
	ChangePowerState(
		IN DEVICE_POWER_STATE  CurrentState,
		IN DEVICE_POWER_STATE  PreviousState
		);

	NTSTATUS CurrentSettings(IN BOOL fInstance, IN BOOL fLoad);

	// shawn 2011/05/25 +++++
	NTSTATUS  
	Get_ExUnit_Control_Value(
		IN ULONG Cmd, 
		IN ULONG ControlID, 
		IN ULONG ExUnitID, 
		IN ULONG BufferLength, 
		IN UCHAR AddrH, 
		IN UCHAR AddrL,
		OUT PUCHAR GetBuffer);

	NTSTATUS 
	Set_ExUnit_Control_Value(
		IN ULONG Cmd, 
		IN ULONG ControlID, 
		IN ULONG ExUnitID, 
		IN ULONG BufferLength, 
		IN UCHAR AddrH, 
		IN UCHAR AddrL,
		IN PUCHAR SetBuffer);

	NTSTATUS Get_Cur_AUXLED_Value(OUT PLONG GetBuffer);
	NTSTATUS Set_Cur_AUXLED_Value(IN LONG  SetBuffer);
	NTSTATUS Get_Cur_WBC_Value(OUT PLONG GetBuffer);
	NTSTATUS Set_Cur_WBC_Value(IN LONG  SetBuffer);
	NTSTATUS Get_Cur_WBC_Green_Value(OUT PLONG GetBuffer);
	NTSTATUS Set_Cur_WBC_Green_Value(IN LONG  SetBuffer);
	NTSTATUS Get_Cur_WBCAuto_Value(OUT PLONG GetBuffer);
	NTSTATUS Set_Cur_WBCAuto_Value(IN LONG  SetBuffer);
	NTSTATUS Get_Cur_Privacy_Value(OUT PLONG GetBuffer);
	NTSTATUS Set_Cur_Privacy_Value(IN LONG  SetBuffer);
	// shawn 2011/05/25 -----
		
	//##### Variables #####//
	LONG		m_InstanceIndex;
	//2011/5/18 11:07¤W¤È
	WCHAR	m_DevInstanceKey[512];
	KTIMER Timer;
	KDPC Dpc;
	BOOLEAN StopDPC;
	
	BOOLEAN ReadyToSetCurSetting;
	LONGLONG m_FirstFrameStartTime;	// shawn 2011/12/16 move to here for fixing AMCAP capture error
protected:

	//
	// Inline helper
	//
	inline PDEVEXT	CurrentDevExt();

private:

	//##### Functions #####//
	//NTSTATUS CheckOSVersion();
	NTSTATUS LoadDefaultSettings(/*PWCHAR BasePath*/BOOL bCustom, BOOLEAN bReload);	// shawn 2011/05/31 modify
	NTSTATUS SetImageFormat();
	NTSTATUS LoadHWDefaultValues();

	//##### Variables #####//
	BOOLEAN						m_SurpriseRemoval;
	//LONGLONG					m_FirstFrameStartTime;	// shawn 2011/12/16 remove for fixing AMCAP capture error
	WCHAR						DefaultRegPath[MAX_PATH];
	PKSDEVICE					m_Device;
	LONGLONG					m_PresentationTimeLast;
	LONGLONG					m_PictureNumber;
	LONGLONG					m_DropCount;
	LONGLONG					m_DropCountFake;
	BOOLEAN						m_bInitDevFlag;
	BOOLEAN						m_WakeUp_Run;
	KS_VIDEOINFOHEADER			m_VideoInfoHeader;
	PKS_VIDEOINFOHEADER			m_pVideoInfoHeader[MAX_NUM_STREAM_INFO];	// array of pointer to the header of video info.
	BYTE						m_StreamInstances[MAX_NUM_STREAM_INFO];		// array of stream instance

	// Extension Unit
//	PKSPROPERTY_SET				m_pDevicePropertySets;
};

#endif	// __VideoStream_H__