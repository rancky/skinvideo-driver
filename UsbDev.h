/****************************************************************************
*
*  Copyright (c) 2003  Sonix Technology Co., Ltd.
*
*  Module Name: UsbDev.h
*
*  Abstract:    Define UsbDev Class
*
*  Revision History:
*
*    03-24-2003 :	Created [Peter]
*	 04-17-2003 :	Modify the architecture of the including files [Peter]
*
*****************************************************************************/

#if !defined(__UsbDev_h__)
#define __UsbDev_h__

//
// Constant definition
//

#define MAX_CONFIGURATION_NO 1
#define USB_COMMAND_TIMEOUT 3000

// For USB 1.1 Setting
#define MAX_NUM_ISO_PACKETS	 24
#define MAX_TRANSFER_SIZE	(MAX_NUM_ISO_PACKETS*1023)
// For USB 2.0
#define MAX_NUM_ISO_PACKETS_2	 24
#define MAX_TRANSFER_SIZE_2		(MAX_NUM_ISO_PACKETS_2*3072)

//
//Define a class/structure for UsbDev
//
typedef struct _SDSP_CONTEXT {
	PKEVENT pEvent;				// event to signal when request complete
	NTSTATUS ntStatus;			// ending status
} USB_SDSP_CONTEXT, *PUSB_SDSP_CONTEXT;

class UsbDev
{
public:

	//
	// Constructor/Deconstructor
	//

	UsbDev();
	~UsbDev();

	//
	// USB device handlers
	//
	NTSTATUS InitializeUsbDevice(PVOID pDevExt);	// shawn 2011/05/06 modify
	NTSTATUS GetDeviceDescriptor(PUSB_DEVICE_DESCRIPTOR pdd);
	NTSTATUS SetAlternate(UCHAR AlternateNo, UCHAR InterfaceNo);
	int GetNoOfAlternateSetting(UCHAR InterfaceNo);
	NTSTATUS GetPipeInfo(UCHAR InterfaceNo,UCHAR PipeNo,PUSBD_PIPE_INFORMATION pPipeInfo);
	NTSTATUS UsbControlVendorTransfer(USHORT Functions,BOOL Rw,UCHAR Request,USHORT Value,USHORT Index,PVOID TransferBuffer,ULONG TransferBufferLength);
	NTSTATUS UsbBulkOrInterruptTransfer(PIRP pIrp,PURB pUrb,PUSBD_PIPE_INFORMATION pPipeInfo,BOOL Rw,PVOID TransferBuffer,PMDL TransferBufferMDL,ULONG TransferBufferLength,PVOID BulkOrInterruptCompletion,PVOID BulkOrInterruptContext);
	NTSTATUS UsbIsochronousTransfer(PIRP pIrp,PURB pUrb,PUSBD_PIPE_INFORMATION pPipeInfo,BOOL Rw,PVOID TransferBuffer,PMDL TransferBufferMDL,PVOID IsochronousCompletion,PVOID IsochronousContext);
	NTSTATUS PipeAbortOrReset(UCHAR InterfaceNo,UCHAR PipeNo,BOOLEAN Abort);
	NTSTATUS SelectUsbBandwidth(PINT pSelectedBandwidth, BOOL fHasAudio);
	PUSB_CONFIGURATION_DESCRIPTOR ReturnConfigDesc(UCHAR index);
	int GetNoOfInterfaces() { return m_NumberOfInterfaces; };

	// for Selective Suspend
	NTSTATUS SubmitIdleRequestIrp();
	VOID	 CancelSelectiveSuspend();
	NTSTATUS SendDeviceSetPower(DEVICE_POWER_STATE devpower, BOOL wait);
	
	//2010/8/23 03:10下午
	NTSTATUS UninitializeUsbDevice();
	
	BOOLEAN	m_fUSBHighSpeed;
	// for Selective Suspend
	PIRP	m_SuspendIrp;
	//2010/8/30 04:00下午
	//BOOLEAN	m_SuspendCancelled;
	UINT	m_SuspendCancelled;
	KEVENT	m_SuspendIrpCancelEvent;
	//2010/3/9 11:13上午
	KSPIN_LOCK IdleReqStateLock;
	//2010/8/25 03:16下午
	KSEMAPHORE CallUSBSemaphore;
	//2010/10/13 04:09下午
	BOOLEAN TwoPipeEnable;  
	//2011/2/22 02:37下午
	BOOLEAN BeST5R286;	
	//2010/11/18 02:54下午
	KEVENT	m_DataReadyEvent;
	KEVENT	m_MEPG2DataReadyEvent;	
	BOOLEAN m_StopIsoThread[2];
	PVOID	m_pDevExtTmp;	// shawn 2011/05/06 add
protected:

	//
	// General protected functions
	//
	void SetUsbPDO(PDEVICE_OBJECT pdo);
	NTSTATUS SendAwaitUrb(PURB Urb);
	NTSTATUS GetDeviceDescriptor();
	NTSTATUS GetConfigurationDescriptor();
	NTSTATUS SetConfiguration();
	NTSTATUS SetToUnconfigure();
	//PDEVICE_OBJECT					m_pLdo;	//Ponter to the device object of the lower level

private:

	PDEVICE_OBJECT					m_pLdo;	//Ponter to the device object of the lower level
	USB_DEVICE_DESCRIPTOR			m_DevDesc;
	PUSB_CONFIGURATION_DESCRIPTOR	m_pConfigDesc[MAX_CONFIGURATION_NO];
	int								m_UsbConfigurationNo;
	int								m_NumberOfInterfaces;
	HANDLE							m_hConfigureation;

	//PUSBD_INTERFACE_LIST_ENTRY		m_pInterfaceListEntry;
	PUSBD_INTERFACE_INFORMATION m_pInterface[3];
	// for Selective Suspend
	USB_IDLE_CALLBACK_INFO			m_cbInfo;
};

#endif	//__UsbDev_h__