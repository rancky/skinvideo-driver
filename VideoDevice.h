/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

	File:

		VideoDevice.h

	Abstract:


	History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/
#pragma once

#ifndef __VideoDevice_H__
#define __VideoDevice_H__

#include "usbdev.h"

//#define MAX_IRP	2
#define MAX_IRP	8	// james try M2TS.
#define SELECTED_CTRL_INTERFACE_INDEX	0
#define SELECTED_DATA_INTERFACE_INDEX	1
//2010/6/18 03:51下午
#define SELECTED_MPEG2_INTERFACE_INDEX	2
#define ISO_PIPE_INDEX				0
//2010/11/22 04:20下午
#define ISO_PIPE2_INDEX				1
#define INT_PIPE_INDEX				0
#define SNAPSHOT_FILE_NAME_SIZE		32
#define SNAPSHOT_FILE_HDR_SIZE		24
#define NUM_OF_DATA_BUFFER			2
#define DEFAULT_DATA_BUFFER_SIZE	/*(1600*1200*3+PAGE_SIZE)*/(2048*1536*3+PAGE_SIZE)	// shawn 2011/07/12 modify for 3M

#define VENDOR_REQUEST_WRITE		8
#define VENDOR_REQUEST_READ			0
#define CTL_R_NONE  0x00
#define CTL_W_NONE  0x01
//2010/12/28 10:45上午
#define VENDOR_REQUEST_SF  3
#define VENDOR_BUFFER_SIZE 64
#define VENDOR_GET_SECTORTABLE_ADDR  9

enum STILL_SIZE{
	Dummy,
	SNAPSHOT_160X120,	
	SNAPSHOT_176X144,
	SNAPSHOT_320X240,
	SNAPSHOT_352X288,
	SNAPSHOT_640X480,	
	SNAPSHOT_800X600,	
	SNAPSHOT_1024X768,						
	SNAPSHOT_1280X960,			// 1.3M
	SNAPSHOT_1280X1024,			// 1.3M
	SNAPSHOT_1600X1200,			// 2M
	SNAPSHOT_2304x1728,
	SNAPSHOT_2560X2048,
	// shawn 2011/07/19 +++++
	SNAPSHOT_3200X2400,
	SNAPSHOT_960X720,
	SNAPSHOT_1280X800,
	SNAPSHOT_2816X2253,
	SNAPSHOT_3520X2640,
	SNAPSHOT_1600X900,
	SNAPSHOT_1280X720,
	SNAPSHOT_640X400,
	SNAPSHOT_1920X1080,
	SNAPSHOT_2048X1536
	// shawn 2011/07/19 -----
};

typedef struct	TransferObj {
	PIRP		pIrp;
	PURB		pUrb;
	PVOID		pContext;
	PUCHAR		pUsbBuffer;
	USHORT      usUsbBufferLength;
	//2010/6/21 06:40下午 H264
	ULONG 	StreamNumber;
	//2010/10/13 03:16下午 two pipes
	ULONG 	PipeNumber;	
} TRANSFEROBJ, *PTRANSFEROBJ;

typedef enum {
	NO_DATA,
	DATA_READ,
	DATA_WRITE,
	DATA_READY,
} DATABUFSTATE;

typedef enum {
	DATAOP_READ,
	DATAOP_WRITE,
} DATABUFOP;

typedef enum{
	AP_IS_NOT_RUNNING=0,
	AUTO_START=1,
	AP_IS_RUNNING=2,
	SNAP_SHOT=3,
	TWAIN_IS_RUNNING=4,
	WAITING_FOR_AP_OPEN=5
} SNAPSHOTSTATE;

enum WindowsVersion
{
	WindowsUnknown,
    WindowsXP,
    WindowsVista7,
	Windows8
};
//
// Sonix event type
//
#define SONIX_EVENT_IMG_AE				0x00000001
#define SONIX_EVENT_INTERRUPT			0x00000002
#define SONIX_EVENT_START_INT_XFER		0x00000004
#define SONIX_EVENT_ISO_BULK_RESTART	0x00000008
#define SONIX_EVENT_ASIC_COMMUNICATION	0x00000010
#define SONIX_EVENT_START_STILL_XFER	0x00000020

typedef struct DataBufContent {
	PUCHAR		pDataBuf;
	ULONG		DataLength;
	DWORD		FrameSN; // serial number of received frame
	BOOLEAN		isStillImage;
	//2010/11/15 02:46下午
	ULONG		PTS;
} DATABUFCONTENT, *PDATABUFCONTENT;

typedef struct DataBufObj {
	DATABUFSTATE	State;
	DATABUFCONTENT	Content;
} DATABUFOBJ, *PDATABUFOBJ;

typedef struct
{
	BOOL		bZoomEnable;
	int			iZoomStatus;
	int			iZoomStep;
    int			iMaxZoom, iHPaneIdx, iVPaneIdx;
	int			iPreWidth, iPreHeight, iPreZoomValue;
	int			iHPaneIdxRange, iVPaneIdxRange;		// for Pan / Tilt
	DWORD		biCompression;
	UCHAR		bShowUIControl;
} ZoomParam;

//2010/7/12 05:08下午
typedef struct _ProcessBuf_WORKITEM {
	WORK_QUEUE_ITEM WorkItem;
	CVideoDevice *pVideoDevice;
	PUCHAR pXferBuf;
	ULONG XferLen; 
	ULONG StreamNumber;
}ProcessBuf, *PProcessBuf;	

class CVideoDevice : public UsbDev
{
public:
	//##### Constructor #####//
	CVideoDevice(void);
	~CVideoDevice(void);

	//##### Functions #####//
	NTSTATUS    AllocateStramingBuffer();
	void        FreeStramingBuffer();
	NTSTATUS	InitializeDevice(PDEVEXT pDevExt,PDEVICE_OBJECT pdo,PDEVICE_OBJECT rpdo);
	NTSTATUS	WriteDeviceInfoToReg(LONG InstanceIndex);

	//2010/6/29 04:27下午 H264
	NTSTATUS	Stop(ULONG StreamNumber);
	NTSTATUS	Pause(ULONG StreamNumber);
	//2010/6/18 03:37下午
//	NTSTATUS	Run(KS_VIDEOINFOHEADER &VideoInfoHeader);
	NTSTATUS	Run(KS_VIDEOINFOHEADER &VideoInfoHeader, PKSPIN pin);
	NTSTATUS	CheckCapturePinBandwidth(KS_VIDEOINFOHEADER &VideoInfoHeader);
	NTSTATUS	CheckStillPinBandwidth(KS_VIDEOINFOHEADER &VideoInfoHeader);
	//RBK BULKMODE
	NTSTATUS    PreparingStreamIrps(PDEVEXT pDevExt,PDEVICE_OBJECT pdo);
	NTSTATUS    UnPreparingStreamIrps(PDEVEXT pDevExt);

	NTSTATUS
	Read(
		PUCHAR pDisplayBuf,
		PKS_VIDEOINFOHEADER pVideoInfoHeader,
		REFERENCE_TIME aft_prefer,
		BOOL bFirstFrame = FALSE
		);

	NTSTATUS
	ReadStill(
		PUCHAR pDisplayBuf,
		PKS_VIDEOINFOHEADER pVideoInfoHeader
		);

	NTSTATUS
	ASIC_Communication(
		IN UCHAR Control,
		IN UCHAR Number,
		IN USHORT Address,
		IN UCHAR *pData
		);

	NTSTATUS 
	USB_ControlClassCommand(
		IN UCHAR Request,
		IN USHORT Value,
		IN USHORT Index,
		IN PVOID pBuffer,
		IN OUT ULONG BufferLength,
		IN BOOLEAN GetData
		);
	
	//2010/12/28 10:43上午
	NTSTATUS 
	SF_Communication(
		UCHAR	Control,
		USHORT	Number,
		USHORT	Address,
		UCHAR	*pData
		);

	NTSTATUS GetSectorTableAddr(
		USHORT	*pwAddr);
	
	//2011/1/3 05:46下午
	NTSTATUS GetExtensionFrameRate(void);
	//2011/1/6 05:26下午
	short		GetExposureValue(IN ULONG fw_Exposure_us);
	
	NTSTATUS	StartInterruptTransfer();
	NTSTATUS	StopInterruptPipe();
	NTSTATUS	StillPinTrigger();

	LONG		GetUsbIoCount();
	NTSTATUS	SurpriseRemoval();

	//2010/7/12 05:30下午
	friend VOID ProcessBuffer(PVOID Context);
	
	friend NTSTATUS IsoCompleteRoutine(PDEVICE_OBJECT pDeviceObject,PIRP Irp,PVOID Context);
	friend NTSTATUS IntCompleteRoutine(PDEVICE_OBJECT pDeviceObject,PIRP Irp,PVOID Context);
	friend NTSTATUS BulkCompleteRoutine(PDEVICE_OBJECT pDeviceObject,PIRP Irp,PVOID Context);

	friend void EventHandlerRoutine(IN CVideoDevice *pVideoDevice);
	//mux 2011/3/31 02:52下午
	friend void TSMuxRoutine(IN CVideoDevice *pVideoDevice);

	//2010/10/14 02:53下午
	//ULONG SetASICShellSize(int iZoomStep, int iHPaneStep, int iVPaneStep, BOOL bSyncChange, BOOL AbsValueFlag);
	UInt32 SetASICShellSize(UINT iWidth, UINT iHeight,UINT uiTargetFrameRate, int iZoom, int iHPaneStep, int iVPaneStep, bool zoom, BOOL AbsValueFlag);

	NTSTATUS UpdateImageControlProperties(IN ULONG Id, IN BOOL fWrite, IN LONG buffer=0);

	//2011/4/12 11:10上午
	ULONGLONG GetSystemTime(void);
	//##### Variables #####//
	WCHAR	m_InstanceKey[MAX_PATH];
	// add for keep default value path
	WCHAR	m_DefValuePath[MAX_PATH];
	WCHAR	m_CurValuePath[MAX_PATH];

	ULONG	m_SnapShotSize;
	VIDEOSTREAMINFO m_SnapShotInfo;			// store still image output when method 2
	ULONG	m_dSnapShotRetry;
	BOOLEAN	m_SaveSnapShotFile;
	WCHAR	m_SnapShotFileName[SNAPSHOT_FILE_NAME_SIZE];
	WCHAR	m_FrameFileName[MAX_PATH+12];	// prefix: \DosDevices\ 

	CShell *m_pCShell;
	// add for Zoom
	ZoomParam	m_ZoomParam;
	DWORD	m_FaceTrackParams[ftpCount];
	LONG	m_bSyncLock;
	// add for Effect
	ULONG	m_EffectParam;
	KMUTEX	m_EffectMutex;
	TRANSIFO	m_FrameTransIfo;
	//2010/4/23 02:47下午 H264
	PUCHAR	pMPEG2TSFileBuffer;	
	ULONG		pMPEG2TSFileBufferStart;
	ULONG		MPEG2TSFileBufferLen;
	//2011/3/29 03:57下午
	ULONG		m_MPEG2ReadBufferLength;
	ULONG		m_MJPEGReadBufferLength;	// shawn 2011/07/04 add
		
	//2010/6/10 05:14下午
	ULONG		m_APBufferLen[3];
	// james try M2TS.
	ULONG	m_iIsoInterfaceIndex;
	//2010/10/13 04:09下午
	//BOOLEAN TwoPipeEnable;

	//2010/10/19 03:55下午
	ImageSize	m_enImageOut;
	ImageSize	m_enImageOutStill;
	//2010/10/21 04:16下午
	ULONG m_HSclOffset;
	BOOLEAN m_CIFViewAngle;
	
	//2011/3/31 02:30下午
	BOOLEAN	TSMuxEnable;
	BOOLEAN	StopMuxThread;
	BOOLEAN	TerminateMuxThread;
	KSEMAPHORE	TsMuxSemaphore;
	PUCHAR pMP2TSDataBuf;	
	PUCHAR pTSMuxTempBuf;
	short	 FrameSN;
	//2011/4/12 11:10上午
	ULONGLONG m_Time;
	ULONG			m_TimeIncrement;
	//ULONGLONG TSFrameInterval;
	FORMATTYPE	m_ImageFormat[MAX_NUM_STREAM_INFO];	// shawn 2011/05/25 move from private to public
	unsigned char *m_pEffectBufferPtr;				// shawn 2011/06/01 add
	LONG	m_InitDropFrames;
	USHORT RunningWindowsVersion();
protected:
	//##### Functions #####//
	void EventHandler();

private:
	//##### Functions #####//
	NTSTATUS	CheckForSonix();
	NTSTATUS	GetInstanceKey(PDEVICE_OBJECT pdo);

	NTSTATUS	ReadSnapShotStateFromReg();
	NTSTATUS	WriteSnapShotStateToReg();

	NTSTATUS
	CapturePinProbeCommit(
		IN ULONG AvgTimePerFrame,
		IN UCHAR bFormatIdex,
		IN UCHAR bFrameIndex,
		IN ULONG StreamNumber	//2010/6/22 05:43下午 H264
		);

	NTSTATUS
	StillPinProbeCommit(
		IN UCHAR bFormatIdex,
		IN UCHAR bFrameIndex
		);

	NTSTATUS
	UVC_ParseConfigurationDescriptor(
		IN PUSB_CONFIGURATION_DESCRIPTOR configDesc
		);
	
	//2010/6/22 11:17上午 H264
	//	NTSTATUS	ParseTransferBuffer(PUCHAR pXferBuf, ULONG XferLen);
	NTSTATUS CVideoDevice::ParseTransferBuffer(PUCHAR pXferBuf, ULONG XferLen, ULONG StreamNumber, ULONG PipeNumber);

	//2010/6/21 05:24下午 H264
	//PDATABUFCONTENT		GetDataBuffer(DATABUFOP opCode);
	PDATABUFCONTENT	CVideoDevice::GetDataBuffer(DATABUFOP opCode, ULONG DataBufObjNumber);
	void CVideoDevice::ReturnDataBuffer(DATABUFOP opCode, ULONG DataBufObjNumber);

	//2010/4/23 02:59下午
	BOOL ReadMPEG2TSFiles();
	//2010/10/25 05:34下午
	BOOL DumpImageFiles(PUCHAR Buffer, ULONG BufLen, ULONG FrameNo);

	//2011/3/30 05:35下午 mux
	NTSTATUS StartMuxThread();
	//2011/4/29 11:48上午
	BOOL Report_Black_Image(PKS_VIDEOINFOHEADER pVideoInfoHeader, PBYTE FrameBuffer, BYTE ImgFormat);
	
	//##### Variables #####//
	PDEVEXT	m_pDevExt;
	//ImageSize	m_enImageOut;
	//ImageSize	m_enImageOutStill;
	//FORMATTYPE	m_ImageFormat[MAX_NUM_STREAM_INFO];
	UCHAR	m_AlternateSettingNo;

	BOOLEAN	m_EventHandlerShutdown;
	KSEMAPHORE	m_EventHandlerSemaphore;				// Semaphore for event
	ULONG   m_EventType;

	SNAPSHOTSTATE	m_SnapShotState;
	ULONG	m_dwStillFrameLength;
	BOOLEAN	m_SaveSnapShotFileReady;

	PUCHAR	m_pShellBufferPtr;
	PUCHAR	m_pLastDisplayBuf;
	
	//2010/11/18 02:54下午
	/*
	KEVENT	m_DataReadyEvent;
	//2010/6/11 04:50下午
	KEVENT	m_MEPG2DataReadyEvent;
	*/

	ULONG	m_DataBufSize;								// the size of data buffer.
	DATABUFOBJ	m_DataBufObj[NUM_OF_DATA_BUFFER];		// data buffer object
	//2010/6/21 04:28下午 H264
	DATABUFOBJ	m_MPEG2DataBufObj[NUM_OF_DATA_BUFFER];	// data buffer object
	PDATABUFCONTENT	m_pDataContent;						// pointer to current data content
	//2010/6/23 06:30下午 H264
	PDATABUFCONTENT	m_pMPEG2DataContent;				// pointer to current data content
	
	UCHAR	m_FrameFlag;
	//2010/10/13 03:31下午
	UCHAR	m_FrameFlag_MP2TS;
	UCHAR m_FrameFlag_Pipe2;
	
	ULONG	m_dwFrameSN;
	LONGLONG	m_FrameErrorCount;
	
	LONG	m_UsbIoCount;
	//2010/6/22 02:44下午 H264
	LONG	m_MPEG2UsbIoCount;
	
	//2010/7/23 06:08下午 ring
	//2010/7/14 02:22下午 skype
	//ULONG		m_MPEG2DataOffset;
	ULONG		m_MPEG2IsoBufferAddr;
	ULONG		m_MPEG2ReadBufferAddr;
	//ULONG		m_MPEG2DataLength;
	ULONG		m_MPEG2ValidDataLength;
	//2010/7/27 10:56上午
	//ULONG		m_MPEG2ReadBufferLength;
	BOOLEAN	m_FirstFrame;
	
	TRANSFEROBJ	m_IsoTransferObj[MAX_IRP];		// Iso transfer object
	//2010/6/21 04:36下午 H264
	TRANSFEROBJ	m_MPEG2IsoTransferObj[MAX_IRP];	// Iso transfer object
	TRANSFEROBJ m_IntTransferObj;				// interrupt transfer object

	ULONG	m_dwCurVideoFrameSize;
	ULONG	m_dwMaxVideoFrameSize;
	ULONG	m_dwMaxPayloadTransferSize;
	ULONG	m_dwMaxPayloadTransferSizeTmp;		// shawn 2011/08/01 add
	ULONG	m_dwMaxPayloadTransferSize_MP2TS;	// shawn 2011/07/20 add
	//RBK BULKMODE
	UCHAR m_PipeMode;
	TRANSFEROBJ	m_BulkTransferObj[MAX_IRP];		// Iso transfer object

	LONG	m_InstanceIndex;
	UNICODE_STRING	m_InstanceRegPath;
	BOOLEAN	m_fWriteDefValue;

	RTL_QUERY_REGISTRY_TABLE	m_InstanceTable[2];
	//2010/11/22 03:10下午
	PUCHAR S420Buffer;
	//2011/2/25 03:10下午
	ULONG	m_IncorrectLengthFrameCount;
	ULONG	m_IncorrectPTSCount;
	BYTE	m_byTmpTSCounter;	// shawn 2011/08/04 add
	BYTE	m_byTmpTSCounter2;	// shawn 2011/08/04 add

	BYTE	Table[256];
};

#endif	// __VideoDevice_H__