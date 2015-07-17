/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

	File:

		VideoDevice.cpp

	Abstract:


	History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/

#include "SnCam.h"

CVideoDevice::CVideoDevice(void)
:m_DataBufSize(DEFAULT_DATA_BUFFER_SIZE), m_UsbIoCount(0), m_MPEG2UsbIoCount(0)
{
	//SnapShotState
	RtlZeroMemory (m_InstanceTable, sizeof(m_InstanceTable)); 
	m_InstanceTable[0].Name			= L"SnapShotState";
	m_InstanceTable[0].Flags		= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
	m_InstanceTable[0].EntryContext	= &m_SnapShotState;

	wcscpy(m_FrameFileName, L"\\DosDevices\\");
	m_FrameTransIfo.rgb.r = 0;
	m_FrameTransIfo.rgb.g = 0;
	m_FrameTransIfo.rgb.b = 255;
	m_FrameTransIfo.trans = 0;

	m_iIsoInterfaceIndex = 0;	// james try M2TS.

	KeInitializeMutex(&m_EffectMutex, 0);
	KeInitializeEvent(&m_DataReadyEvent, NotificationEvent, FALSE);
	
	//2010/6/11 04:50と
	KeInitializeEvent(&m_MEPG2DataReadyEvent, NotificationEvent, FALSE);

	KeInitializeSemaphore(&m_EventHandlerSemaphore,0,MAXLONG);

	TSMuxEnable = FALSE;
	BeST5R286 = FALSE;

	//2011/4/12 11:09と
	m_TimeIncrement = KeQueryTimeIncrement();

	m_PipeMode = USB_ENDPOINT_TYPE_ISOCHRONOUS; // RBK bulkmode, init pipe as ISO

}

CVideoDevice::~CVideoDevice(void)
{
	DBGU_TRACE("CVideoDevice::~CVideoDevice()..\n");
	UINT i = 0;
	//2010/8/26 03:25と
	m_EventHandlerShutdown = TRUE;
	KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);

	//2010/8/26 03:41と
	// Shutdown system thread if suspend, autostart, or ep0_polling is enabled (ymwu, 2007/5/25)
	if(m_pDevExt->m_C3Enable || m_pDevExt->m_Autostart_Enable)	// shawn 2011/07/20 modify
	{
		//m_EventHandlerShutdown = TRUE;
		while(m_EventHandlerShutdown)
		{
			DBGU_TRACE("Delay 100 ms ! i=%d\n",i);
			KeDelay(100);
			
			if(i++>50) 
				break;
		}
	}

	//2010/3/9 07:05と
	m_pDevExt->m_bDeviceIdle = FALSE;
	
	//2010/8/26 03:28と
	if(m_pDevExt->m_C3Enable)
		CancelSelectiveSuspend();

	// Free ISO pipe buffer
	for(i=0;i<MAX_IRP;i++)
	{
		if(m_IsoTransferObj[i].pIrp){
			IoFreeIrp(m_IsoTransferObj[i].pIrp);
			m_IsoTransferObj[i].pIrp = NULL;
		}

		if(m_IsoTransferObj[i].pUrb){
			ExFreePoolWithTag(m_IsoTransferObj[i].pUrb,'11US');
			m_IsoTransferObj[i].pUrb = NULL; 
		}

		if(m_IsoTransferObj[i].pUsbBuffer){
			ExFreePoolWithTag(m_IsoTransferObj[i].pUsbBuffer,'11US');
			m_IsoTransferObj[i].pUsbBuffer = NULL; 
		}
	}

	//2010/7/30 05:04と
	//2010/6/21 04:58と H264
	// Free ISO pipe buffer
	if(m_pDevExt->m_bMPEG2TSExist)
	{
		for(i=0;i<MAX_IRP;i++)
		{
			if(m_MPEG2IsoTransferObj[i].pIrp){
				IoFreeIrp(m_MPEG2IsoTransferObj[i].pIrp);
				m_MPEG2IsoTransferObj[i].pIrp = NULL;
			}

			if(m_MPEG2IsoTransferObj[i].pUrb){
				ExFreePoolWithTag(m_MPEG2IsoTransferObj[i].pUrb,'11US');
				m_MPEG2IsoTransferObj[i].pUrb = NULL; 
			}

			if(m_MPEG2IsoTransferObj[i].pUsbBuffer){
				ExFreePoolWithTag(m_MPEG2IsoTransferObj[i].pUsbBuffer,'11US');
				m_MPEG2IsoTransferObj[i].pUsbBuffer = NULL; 
			}
		}
	}

	// Free INT pipe buffer
	if(m_IntTransferObj.pIrp){
		IoFreeIrp(m_IntTransferObj.pIrp);
		m_IntTransferObj.pIrp = NULL;
	}

	if(m_IntTransferObj.pUrb){
		ExFreePoolWithTag(m_IntTransferObj.pUrb,'11US');
		m_IntTransferObj.pUrb = NULL; 
	}

	if(m_IntTransferObj.pUsbBuffer){
		ExFreePoolWithTag(m_IntTransferObj.pUsbBuffer,'11US');
		m_IntTransferObj.pUsbBuffer = NULL; 
	}

	/* RBK move buffer allocation from initial device to RUN +++
	for(i=0;i<NUM_OF_DATA_BUFFER;i++){
		if(m_DataBufObj[i].Content.pDataBuf){
			ExFreePoolWithTag(m_DataBufObj[i].Content.pDataBuf,'11US');

			m_DataBufObj[i].Content.pDataBuf = NULL;
			DBGU_TRACE("ExFreePool pDataBuf[%d]\n",i);
		}

		m_DataBufObj[i].State = NO_DATA;
	}

	//mux 2011/4/1 06:20と
	if(TSMuxEnable || BeST5R286)
	{
		//2010/6/21 05:01と H264
		for(i=0;i<NUM_OF_DATA_BUFFER;i++){
			if(m_MPEG2DataBufObj[i].Content.pDataBuf){
				ExFreePoolWithTag(m_MPEG2DataBufObj[i].Content.pDataBuf,'11US');
				m_MPEG2DataBufObj[i].Content.pDataBuf = NULL;
				DBGU_TRACE("ExFreePoolWithTag H264 pDataBuf[%d]\n",i);
			}
			
			m_MPEG2DataBufObj[i].State = NO_DATA;
		}
	}

	if(pMP2TSDataBuf)
	{
		ExFreePoolWithTag(pMP2TSDataBuf, '11US');
		pMP2TSDataBuf = NULL;
	}
	
	if(pTSMuxTempBuf)
	{
		ExFreePoolWithTag(pTSMuxTempBuf, '11US');
		pTSMuxTempBuf = NULL;
	}

	RBK move buffer allocation from initial device to RUN ---
	*/
	if (m_pShellBufferPtr)
		delete m_pShellBufferPtr;

	// delete CShell
	if(m_pCShell)
	{
		CShell__CShell(m_pCShell);
		m_pCShell = NULL;
	}

	//2010/4/23 02:39と H264
	if(pMPEG2TSFileBuffer)
	{
		ExFreePoolWithTag(pMPEG2TSFileBuffer, 'FREE');
		pMPEG2TSFileBuffer = NULL;
	}

	//2010/11/22 03:12と
	if(S420Buffer)
	{
		ExFreePoolWithTag(S420Buffer, '11US');
		S420Buffer = NULL;
	}

	// shawn 2011/06/01 +++++
	if(m_pEffectBufferPtr) {
		ExFreePoolWithTag(m_pEffectBufferPtr, 'FREE');
		m_pEffectBufferPtr = NULL;
	}
	// shawn 2011/06/01 -----
	
	//2010/8/23 03:11と
	UninitializeUsbDevice();
}


NTSTATUS 
CVideoDevice::AllocateStramingBuffer()
{

	NTSTATUS ntStatus = STATUS_SUCCESS;
	// RBK move buffer allocation from initial device to RUN	+++ 				
	//
	// Allocate memory for data buffer.
	//
	for(int i=0;i<NUM_OF_DATA_BUFFER;i++)
	{
		m_DataBufObj[i].Content.pDataBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '61US');

		if(!m_DataBufObj[i].Content.pDataBuf)
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		else
		{
			RtlZeroMemory(m_DataBufObj[i].Content.pDataBuf, m_DataBufSize);
			m_DataBufObj[i].Content.DataLength = 0;
		}
	}
		
	//2010/7/30 03:46と
	if(m_pDevExt->m_bMPEG2TSExist)
	{		
		//2011/4/1 05:36と mux
		pMP2TSDataBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '41US');
			
		if(TSMuxEnable)
		{
			//2011/4/11 03:58と
			pTSMuxTempBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '51US');

			//2010/6/21 04:29と H264
			for(int i=0;i<NUM_OF_DATA_BUFFER;i++)
			{
				m_MPEG2DataBufObj[i].Content.pDataBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '71US');
					
				if(!m_MPEG2DataBufObj[i].Content.pDataBuf)
				{
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}
				else
				{
					RtlZeroMemory(m_MPEG2DataBufObj[i].Content.pDataBuf, m_DataBufSize);
					m_MPEG2DataBufObj[i].Content.DataLength = 0;
				}
			}
		}
	}				
	else if(BeST5R286)
	{		
		for(int i=0;i<NUM_OF_DATA_BUFFER;i++)
		{
			m_MPEG2DataBufObj[i].Content.pDataBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '71US');
				
			if(!m_MPEG2DataBufObj[i].Content.pDataBuf)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}
			else
			{
				RtlZeroMemory(m_MPEG2DataBufObj[i].Content.pDataBuf, m_DataBufSize);
				m_MPEG2DataBufObj[i].Content.DataLength = 0;
			}
		}			
	}

	return ntStatus;
}


void 
CVideoDevice::FreeStramingBuffer()
{

		// RBK move buffer allocation from initial device to RUN	+++ 
		for(int i=0;i<NUM_OF_DATA_BUFFER;i++){
			if(m_DataBufObj[i].Content.pDataBuf){
				ExFreePoolWithTag(m_DataBufObj[i].Content.pDataBuf,'61US');

				m_DataBufObj[i].Content.pDataBuf = NULL;
				DBGU_TRACE("ExFreePool pDataBuf[%d]\n",i);
			}

			m_DataBufObj[i].State = NO_DATA;
		}

		if(TSMuxEnable || BeST5R286)
		{
			//2010/6/21 05:01と H264
			for(int i=0;i<NUM_OF_DATA_BUFFER;i++){
				if(m_MPEG2DataBufObj[i].Content.pDataBuf){
					ExFreePoolWithTag(m_MPEG2DataBufObj[i].Content.pDataBuf,'71US');
					m_MPEG2DataBufObj[i].Content.pDataBuf = NULL;
					DBGU_TRACE("ExFreePoolWithTag H264 pDataBuf[%d]\n",i);
				}
			
				m_MPEG2DataBufObj[i].State = NO_DATA;
			}
		}


		if(pMP2TSDataBuf)
		{
			ExFreePoolWithTag(pMP2TSDataBuf, '41US');
			pMP2TSDataBuf = NULL;
		}
	
		if(pTSMuxTempBuf)
		{
			ExFreePoolWithTag(pTSMuxTempBuf, '51US');
			pTSMuxTempBuf = NULL;
		}
	
	// RBK move buffer allocation from initial device to RUN	---
}


//RBK BULKMODE

NTSTATUS 
CVideoDevice::PreparingStreamIrps(PDEVEXT pDevExt,PDEVICE_OBJECT pdo)
{
	UCHAR StackSize = 0;
	int i = 0;			
	NTSTATUS ntStatus = STATUS_SUCCESS;

	if(!pdo || !m_pDevExt){
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	StackSize = (pdo->StackSize+1);

	if (this->m_PipeMode == USB_ENDPOINT_TYPE_BULK)
	{
		for(i=0;i<MAX_IRP;i++)
		{
			//
			// Initialize Iso transfer object
			//
			m_BulkTransferObj[i].pIrp = IoAllocateIrp(StackSize,FALSE);
			
			if(!m_BulkTransferObj[i].pIrp)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				goto _exit_init;
			}

			IoInitializeIrp(m_BulkTransferObj[i].pIrp,IoSizeOfIrp(StackSize),StackSize);


			m_BulkTransferObj[i].pUrb =	(PURB) ExAllocatePoolWithTag(NonPagedPool, sizeof(URB), '11US');

			
			if(!m_BulkTransferObj[i].pUrb)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				goto _exit_init;
			}

			RtlZeroMemory(m_BulkTransferObj[i].pUrb,  sizeof(URB));



			DbgPrint("Bulk MAX Payload size : 0x%X\n",m_dwMaxPayloadTransferSizeTmp);

			m_BulkTransferObj[i].pUsbBuffer = (PUCHAR) ExAllocatePoolWithTag(NonPagedPool,m_dwMaxPayloadTransferSizeTmp, '11US');
			m_BulkTransferObj[i].usUsbBufferLength = m_dwMaxPayloadTransferSizeTmp;

			if(!m_BulkTransferObj[i].pUsbBuffer)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				goto _exit_init;
			}

			RtlZeroMemory(m_BulkTransferObj[i].pUsbBuffer, m_dwMaxPayloadTransferSizeTmp);

			m_BulkTransferObj[i].pContext = this;
			//2010/6/22 11:14と H264
			m_BulkTransferObj[i].StreamNumber = STREAM_Capture;
			//2010/10/13 03:22と
			m_BulkTransferObj[i].PipeNumber = 0;	
		}
	}
	

_exit_init:
	return ntStatus;
}

//RBK BULKMODE

NTSTATUS 
CVideoDevice::UnPreparingStreamIrps(PDEVEXT pDevExt)
{
	int i = 0;

	

	if (this->m_PipeMode == USB_ENDPOINT_TYPE_BULK)
	{
		// Free Bulk pipe buffer
		for(i=0;i<MAX_IRP;i++)
		{
			if(m_BulkTransferObj[i].pIrp){
				IoFreeIrp(m_BulkTransferObj[i].pIrp);
				m_BulkTransferObj[i].pIrp = NULL;
			}

			if(m_BulkTransferObj[i].pUrb){
				ExFreePoolWithTag(m_BulkTransferObj[i].pUrb,'11US');
				m_BulkTransferObj[i].pUrb = NULL; 
			}

			if(m_BulkTransferObj[i].pUsbBuffer){
				ExFreePoolWithTag(m_BulkTransferObj[i].pUsbBuffer,'11US');
				m_BulkTransferObj[i].pUsbBuffer = NULL; 
			}
		}
	}

	
	return STATUS_SUCCESS;
}



NTSTATUS CVideoDevice::CheckForSonix()
{
	NTSTATUS ntStatus;
	UCHAR Request = 0x00;
	UCHAR Data=0;
	ULONG BufferLen = sizeof(Data);

	for (int i=0;i<3;i++) // if device io error, we allow retry for 3 times
	{
		//for chip ID identify, make it Sonix Only
		ntStatus = UsbControlVendorTransfer(
			URB_FUNCTION_VENDOR_INTERFACE,	// Function
			TRUE,							// Read/Write(write=false,read=true),
			Request,						// Request
			0x101f,							// Value
			0x0c45,							//2010/3/4 02:37と 		// Index
			&Data,							// buffer
			BufferLen						// buffer length
			);

		if(NT_SUCCESS(ntStatus))
		{
			// shawn 2011/05/25 +++++
			m_pDevExt->m_ChipID = Data;
			// shawn 2011/05/25 -----
			
			DBGU_TRACE("ChipID = %x\n", Data);
			
			if (Data == 0x90 ||Data == 0x91)	// ST5021x
			{
				return STATUS_SUCCESS;
			}

			//2011/2/22 02:33と for 286
			if (Data == 0x86 || Data == 0x88 || Data == 0x89)	// shawn 2011/08/22 for 289
			{
				BeST5R286 = TRUE;
				DBGU_TRACE("BeST5R286 1\n");
				return STATUS_SUCCESS;
			}

			// shawn 2011/06/01 for testing +++++
			if (Data == 0x22 || 
				Data == 0x25 || 
				Data == 0x15 || 
				Data == 0x23 || 
				Data == 0x32 || 
				Data == 0x56 || 
				Data == 0x33 || 
				Data == 0x76 || 
				Data == 0x75 || 
				Data == 0x70 || 
				Data == 0x71 || 
				Data == 0x16 || 
				Data == 0x80 || 
				Data == 0x81 || 
				Data == 0x88 )
			{
				return STATUS_SUCCESS;
			}
			// shawn 2011/06/01 testing -----

			/*if (Data == 0x22)	// ST5021x
			{
				return STATUS_SUCCESS;
			}

			if (Data == 0x23)	// ST5023x
			{
				return STATUS_SUCCESS;
			}

			if (Data == 0x25)	// ST5025x
			{
				return STATUS_SUCCESS;
			}

			//2010/3/3 05:19と
			//2011/2/23 09:46と
			if (Data == 0x15 ||Data == 0x32 ||Data == 0x56||Data == 0x33 ||Data == 0x76||Data == 0x90||Data == 0x91)
			{
				return STATUS_SUCCESS;
			}

			//2011/2/22 02:33と for 286
			Be50286 = FALSE;
			if (Data == 0x86)
			{
				Be50286 = TRUE;
				return STATUS_SUCCESS;
			}*/
		}
		else {
			KeDelay(33);
		}
	}

	return STATUS_UNSUCCESSFUL;
}

LONG CVideoDevice::GetUsbIoCount()
{
	LONG  UsbIoCount = m_UsbIoCount + m_MPEG2UsbIoCount;
	DBGU_TRACE("Usb I/O Count : %d\n", UsbIoCount);
	return UsbIoCount;
}

NTSTATUS 
CVideoDevice::USB_ControlClassCommand(
	IN UCHAR Request,
	IN USHORT Value,
	IN USHORT Index,
	IN PVOID pBuffer,
	IN OUT ULONG BufferLength,
	IN BOOLEAN GetData
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UINT MaxTry = 8;//3;2011/4/28 10:02と

	for (int i=0;i<MaxTry;i++)
	{
		ntStatus = UsbControlVendorTransfer(
			URB_FUNCTION_CLASS_INTERFACE,	// Function
			GetData,						// Read/Write(write=false,read=true),
			Request,						// Request
			Value,							// Value
			Index,							// Index
			pBuffer,						// buffer
			BufferLength					// buffer length
			);

		//KeDelay(25);
		
		if (NT_SUCCESS(ntStatus))
			break;

		//2011/4/28 10:02と
		if (ntStatus != STATUS_UNSUCCESSFUL && ntStatus != STATUS_PENDING && ntStatus != STATUS_POWER_STATE_INVALID )
		{
			DBGU_WARNING("USB_ControlClassCommand return Error(%X)\n",ntStatus);
			RtlZeroMemory(pBuffer, BufferLength);
			break;
		}

		//2010/8/25 03:30と
		KeDelay(50);
	}

	return ntStatus;
}

NTSTATUS 
CVideoDevice::InitializeDevice(PDEVEXT pDevExt,PDEVICE_OBJECT pdo,PDEVICE_OBJECT rpdo)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UINT i;
	UCHAR StackSize;
	USBD_PIPE_INFORMATION PipeInfo;
	RtlZeroMemory(&PipeInfo,sizeof(USBD_PIPE_INFORMATION));
	int Y = 0;

	m_pDevExt = pDevExt;

	if(!pdo || !m_pDevExt){
		ntStatus = STATUS_UNSUCCESSFUL;
	}
	else
	{
		//
		// Initialize Usb
		//

		UsbDev::SetUsbPDO(pdo);//Store pdo
		//2011/5/16 03:57と

		if (!NT_SUCCESS( CheckForSonix() ))
			return STATUS_UNSUCCESSFUL;

		ntStatus = InitializeUsbDevice(pDevExt);	//initialize USB	// shawn 2011/05/06 modify
		
		if(NT_SUCCESS(ntStatus))
		{
			BYTE bUSBSpeed=0;
			PUSB_CONFIGURATION_DESCRIPTOR pConfigDesc;
			/*
			if (!NT_SUCCESS( CheckForSonix() ))
				return STATUS_UNSUCCESSFUL;
			*/			
			
			// shawn 2011/08/01 modify for USB 1.1 YUY2 issue +++++
			//2010/4/28 02:07と

			//m_fUSBHighSpeed = 1;

			// RBK temperally comment out this speed check for implementation
	
			// Check USB speed to decide USBCAMD_NUM_ISO_PACKETS_PER_REQUEST
			ntStatus = ASIC_Communication(CTL_R_NONE, 1, 0x130d, &bUSBSpeed);

			if (NT_SUCCESS(ntStatus))
				m_fUSBHighSpeed = ((bUSBSpeed & 0x80) == 0x80);
			else
			{
				DBGU_TRACE("InitializationComplete: Check USB Speed return %X\n",ntStatus);
				ntStatus = STATUS_IO_DEVICE_ERROR;
				goto _exit_init;
			}
			// shawn 2011/08/01 modify for USB 1.1 YUY2 issue -----

			// Get instance key
			if(!NT_SUCCESS(GetInstanceKey(rpdo))){
				ntStatus = STATUS_UNSUCCESSFUL;
				DBGU_TRACE("Fail to get instance key !\n");
				goto _exit_init;
			}
			
			if (m_fUSBHighSpeed)
				DBGU_TRACE("Device is on High Speed! Michael\n");
			else
				DBGU_TRACE("Device is on Full Speed!\n");

			pConfigDesc = ReturnConfigDesc(0);

			if (pConfigDesc)
				ntStatus = UVC_ParseConfigurationDescriptor(pConfigDesc);
			
			if(!NT_SUCCESS(ntStatus && pConfigDesc)){
				ntStatus = STATUS_UNSUCCESSFUL;
				DBGU_TRACE("Fail to Parse Configuration Descriptor !\n");
				goto _exit_init;
			}

			// shawn 2011/06/23 for adding still pin or not +++++
			if (!m_pDevExt->m_StillPin_Enable)
			{
				m_pDevExt->m_STREAM_Capture_MP2TS_Idx = 1;
				m_pDevExt->m_STREAM_Still_Idx = -1;
			}
			else
			{
				m_pDevExt->m_STREAM_Capture_MP2TS_Idx = STREAM_Capture_MP2TS;
				m_pDevExt->m_STREAM_Still_Idx = STREAM_Still;
			}
			// shawn 2011/06/23 for adding still pin or not -----

			ntStatus = UsbDev::SetAlternate(0, SELECTED_DATA_INTERFACE_INDEX);//Select alternate 0 
			
			if(!NT_SUCCESS(ntStatus)){
				ntStatus = STATUS_UNSUCCESSFUL;
				DBGU_TRACE("Fail to Select alternate 0 !\n");
				goto _exit_init;
			}
		}
		else
		{
			ntStatus = STATUS_UNSUCCESSFUL;
			DBGU_TRACE("Fail to initialize USB device !\n");
			goto _exit_init;
		}
		
		m_EventHandlerShutdown = false;
		m_EventType = 0;
		//2011/2/25 10:59と
		m_pDevExt->m_C3Enable = 0;
		m_pDevExt->m_Autostart_Enable = 0;	// shawn 2011/07/20 add
		m_pDevExt->m_TSMUX_Enable = 0;		// shawn 2011/07/21 add
					
		/* RBK move buffer allocation to RUN
		//
		// Allocate memory for data buffer.
		//
		for(i=0;i<NUM_OF_DATA_BUFFER;i++)
		{
			m_DataBufObj[i].Content.pDataBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '11US');

			if(!m_DataBufObj[i].Content.pDataBuf)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				goto _exit_init;
			}
			else
			{
				RtlZeroMemory(m_DataBufObj[i].Content.pDataBuf, m_DataBufSize);
				m_DataBufObj[i].Content.DataLength = 0;
			}
		}
		
		//2010/7/30 03:46と
		if(m_pDevExt->m_bMPEG2TSExist)
		{		
			//2011/4/1 05:36と mux
			pMP2TSDataBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '11US');
			
			if(TSMuxEnable)
			{
				//2011/4/11 03:58と
				pTSMuxTempBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '11US');

				//2010/6/21 04:29と H264
				for(i=0;i<NUM_OF_DATA_BUFFER;i++)
				{
					m_MPEG2DataBufObj[i].Content.pDataBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '11US');
					
					if(!m_MPEG2DataBufObj[i].Content.pDataBuf)
					{
						ntStatus = STATUS_INSUFFICIENT_RESOURCES;
						goto _exit_init;
					}
					else
					{
						RtlZeroMemory(m_MPEG2DataBufObj[i].Content.pDataBuf, m_DataBufSize);
						m_MPEG2DataBufObj[i].Content.DataLength = 0;
					}
				}
			}
		}				
		else if(BeST5R286)
		{		
			for(i=0;i<NUM_OF_DATA_BUFFER;i++)
			{
				m_MPEG2DataBufObj[i].Content.pDataBuf = (UCHAR *) ExAllocatePoolWithTag(NonPagedPool,m_DataBufSize, '11US');
				
				if(!m_MPEG2DataBufObj[i].Content.pDataBuf)
				{
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
					goto _exit_init;
				}
				else
				{
					RtlZeroMemory(m_MPEG2DataBufObj[i].Content.pDataBuf, m_DataBufSize);
					m_MPEG2DataBufObj[i].Content.DataLength = 0;
				}
			}			
		}
		*/
		//
		//	Allocate memory for Irps and Urbs of Iso,Int pipe. 
		//
		StackSize = (pdo->StackSize+1);
		
		for(i=0;i<MAX_IRP;i++)
		{
			//
			// Initialize Iso transfer object
			//
			m_IsoTransferObj[i].pIrp = IoAllocateIrp(StackSize,FALSE);
			
			if(!m_IsoTransferObj[i].pIrp)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				goto _exit_init;
			}

			IoInitializeIrp(m_IsoTransferObj[i].pIrp,IoSizeOfIrp(StackSize),StackSize);

			if (m_fUSBHighSpeed)
				m_IsoTransferObj[i].pUrb =	(PURB) ExAllocatePoolWithTag(NonPagedPool, GET_ISO_URB_SIZE(MAX_NUM_ISO_PACKETS_2), '11US');
			else
				m_IsoTransferObj[i].pUrb =	(PURB) ExAllocatePoolWithTag(NonPagedPool, GET_ISO_URB_SIZE(MAX_NUM_ISO_PACKETS), '11US');
			
			if(!m_IsoTransferObj[i].pUrb)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				goto _exit_init;
			}

			if (m_fUSBHighSpeed)
				RtlZeroMemory(m_IsoTransferObj[i].pUrb, GET_ISO_URB_SIZE(MAX_NUM_ISO_PACKETS_2));
			else
				RtlZeroMemory(m_IsoTransferObj[i].pUrb, GET_ISO_URB_SIZE(MAX_NUM_ISO_PACKETS));

			if (m_fUSBHighSpeed)
                        {
				m_IsoTransferObj[i].pUsbBuffer = (PUCHAR) ExAllocatePoolWithTag(NonPagedPool, (MAX_TRANSFER_SIZE_2 + 64), '11US');
                                m_IsoTransferObj[i].usUsbBufferLength =  (MAX_TRANSFER_SIZE_2 + 64);
                        }
			else
			{
                               	m_IsoTransferObj[i].pUsbBuffer = (PUCHAR) ExAllocatePoolWithTag(NonPagedPool, (MAX_TRANSFER_SIZE + 64), '11US');
                                m_IsoTransferObj[i].usUsbBufferLength =  (MAX_TRANSFER_SIZE + 64);
                        }
			if(!m_IsoTransferObj[i].pUsbBuffer)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				goto _exit_init;
			}

			if (m_fUSBHighSpeed)
				RtlZeroMemory(m_IsoTransferObj[i].pUsbBuffer, (MAX_TRANSFER_SIZE_2 + 64));
			else
				RtlZeroMemory(m_IsoTransferObj[i].pUsbBuffer, (MAX_TRANSFER_SIZE + 64));

			m_IsoTransferObj[i].pContext = this;
			//2010/6/22 11:14と H264
			m_IsoTransferObj[i].StreamNumber = STREAM_Capture;
			//2010/10/13 03:22と
			m_IsoTransferObj[i].PipeNumber = 0;	
		}


		//2010/7/30 03:46と
		if(m_pDevExt->m_bMPEG2TSExist || BeST5R286)
		{
			//2010/6/21 04:36と for H264
			for(i=0;i<MAX_IRP;i++)
			{
				//
				// Initialize Iso transfer object
				//
				m_MPEG2IsoTransferObj[i].pIrp = IoAllocateIrp(StackSize,FALSE);
				
				if(!m_MPEG2IsoTransferObj[i].pIrp)
				{
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
					goto _exit_init;
				}
				
				IoInitializeIrp(m_MPEG2IsoTransferObj[i].pIrp,IoSizeOfIrp(StackSize),StackSize);
	
				if (m_fUSBHighSpeed)
					m_MPEG2IsoTransferObj[i].pUrb =	(PURB) ExAllocatePoolWithTag(NonPagedPool, GET_ISO_URB_SIZE(MAX_NUM_ISO_PACKETS_2), '11US');
				else
					m_MPEG2IsoTransferObj[i].pUrb =	(PURB) ExAllocatePoolWithTag(NonPagedPool, GET_ISO_URB_SIZE(MAX_NUM_ISO_PACKETS), '11US');
				
				if(!m_MPEG2IsoTransferObj[i].pUrb)
				{
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
					goto _exit_init;
				}
				
				if (m_fUSBHighSpeed)
					RtlZeroMemory(m_MPEG2IsoTransferObj[i].pUrb, GET_ISO_URB_SIZE(MAX_NUM_ISO_PACKETS_2));
				else
					RtlZeroMemory(m_MPEG2IsoTransferObj[i].pUrb, GET_ISO_URB_SIZE(MAX_NUM_ISO_PACKETS));
	
				if (m_fUSBHighSpeed)
				{
	                                m_MPEG2IsoTransferObj[i].pUsbBuffer = (PUCHAR) ExAllocatePoolWithTag(NonPagedPool, (MAX_TRANSFER_SIZE_2 + 64), '11US');
					m_MPEG2IsoTransferObj[i].usUsbBufferLength =  (MAX_TRANSFER_SIZE_2 + 64);
				}
				else
                                {
					m_MPEG2IsoTransferObj[i].pUsbBuffer = (PUCHAR) ExAllocatePoolWithTag(NonPagedPool, (MAX_TRANSFER_SIZE + 64), '11US');
					m_MPEG2IsoTransferObj[i].usUsbBufferLength =  (MAX_TRANSFER_SIZE + 64);
				}				
				if(!m_MPEG2IsoTransferObj[i].pUsbBuffer)
				{
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
					goto _exit_init;
				}
				
				if (m_fUSBHighSpeed)
					RtlZeroMemory(m_MPEG2IsoTransferObj[i].pUsbBuffer, (MAX_TRANSFER_SIZE_2 + 64));
				else
					RtlZeroMemory(m_MPEG2IsoTransferObj[i].pUsbBuffer, (MAX_TRANSFER_SIZE + 64));
	
				m_MPEG2IsoTransferObj[i].pContext = this;
				
				//2011/5/11 05:10と
				//2010/6/22 11:14と H264
				if(m_pDevExt->m_bMPEG2TSExist)
				{
					m_MPEG2IsoTransferObj[i].StreamNumber = m_pDevExt->m_STREAM_Capture_MP2TS_Idx;	// shawn 2011/06/23 modify			
					m_MPEG2IsoTransferObj[i].PipeNumber = 0;	
				}
				else if(BeST5R286)
				{
					m_MPEG2IsoTransferObj[i].StreamNumber = STREAM_Capture;
					m_MPEG2IsoTransferObj[i].PipeNumber = 1;
					DBGU_TRACE("BeST5R286 2\n");
				}
				
				//2010/6/22 11:14と H264
				//2010/10/13 03:22と
				//m_MPEG2IsoTransferObj[i].PipeNumber = 1;	
			}		
		}
		//
		// Initialize interrupt transfer object
		//
		m_IntTransferObj.pIrp = IoAllocateIrp(StackSize,FALSE);
		
		if(!m_IntTransferObj.pIrp)
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			goto _exit_init;
		}
		
		IoInitializeIrp(m_IntTransferObj.pIrp,IoSizeOfIrp(StackSize),StackSize);

		m_IntTransferObj.pUrb =	(PURB) ExAllocatePoolWithTag(NonPagedPool, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER), '11US');
		
		if(!m_IntTransferObj.pUrb)
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			goto _exit_init;
		}

		m_IntTransferObj.pUsbBuffer = (PUCHAR) ExAllocatePoolWithTag(NonPagedPool, 0x10, '11US');
		
		if(!m_IntTransferObj.pUsbBuffer)
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			goto _exit_init;
		}

		m_IntTransferObj.pContext = this;

		//
		// start interrupt transfer
		//
		ntStatus = StartInterruptTransfer();
		
		if(NT_SUCCESS(ntStatus))
		{
			DBGU_TRACE("InitializeDevice::start interrupt transfer OK!\n");
			WriteSnapShotStateToReg();	//SnapShotState		
		}

		// shawn 2011/06/01 +++++
	#ifndef	_NOEFFECT
		//2007/11/28 02:41と for effect convert
		if(!m_pEffectBufferPtr) 
		{
	  		m_pEffectBufferPtr = (unsigned char *)ExAllocatePoolWithTag(NonPagedPool, (((ImgWidList[isCount-1])*(ImgHgtList[isCount-1])*14)*sizeof(unsigned char)), '8sSF');
		}
	#endif

		for (Y = 0; Y < 256; Y++)
		{
			Table[Y] = (int)(log(Y / 255.0 *(8 + 1) + 1) / (log((double)(8 + 2))) * 255);		//WhiteLevel 8	//	该方法对应的论文为：A Two-Stage Contrast Enhancement Algorithm for Digital Images
		}//
		// shawn 2011/06/01 -----
	}	
	
	//2010/4/23 02:25と Read MPEG2TS file
	// james try M2TS.
	/*if(ReadMPEG2TSFiles())
		DBGU_TRACE("ReadMPEG2TSFiles OK!\n");
	*/
	
_exit_init:

	return ntStatus;
}

NTSTATUS CVideoDevice::GetInstanceKey(PDEVICE_OBJECT pdo)
{
//	DBGU_FN("CVideoDevice::GetInstanceKey()");
	DBGU_TRACE("CVideoDevice::GetInstanceKey()\n");

	NTSTATUS ntStatus = STATUS_SUCCESS;
	ULONG Reslen;
	//2011/5/17 04:59と
	//WCHAR DriverKeyName[MAX_PATH];
	WCHAR DriverKeyName[256];

	RtlZeroMemory(DriverKeyName,sizeof(DriverKeyName));
	RtlZeroMemory(m_InstanceKey,sizeof(m_InstanceKey));
	RtlZeroMemory(m_DefValuePath,sizeof(m_DefValuePath));
	RtlZeroMemory(m_CurValuePath,sizeof(m_CurValuePath));
	//2011/5/18 10:40と
	RtlZeroMemory(m_pDevExt->pVideoStream->m_DevInstanceKey,512);
	ntStatus = IoGetDeviceProperty(	pdo,
									DevicePropertyDriverKeyName,
									sizeof(DriverKeyName),
									DriverKeyName,
									&Reslen);

	if(!NT_SUCCESS(ntStatus))
	{
		DBGU_TRACE("Cannot get \"DevicePropertyDriverKeyName\" !\n");
		ntStatus = STATUS_INSUFFICIENT_RESOURCES; 
		goto _exit_GetInstanceKey;
	}

	if(IoIsWdmVersionAvailable(1,0x20) || IoIsWdmVersionAvailable(1,0x10))
	{//Windows XP or Windows 2000
		DBGU_TRACE("Windows XP or 2000 System !\n");
		swprintf((wchar_t *)m_InstanceKey, L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\Class\\%ws\\Settings",DriverKeyName);
		
		// shawn 2011/05/31 modify +++++
		//swprintf((wchar_t *)m_CurValuePath,L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\Class\\%ws\\CurrentSettings",DriverKeyName);
		swprintf((wchar_t *)m_CurValuePath,L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\Class\\%ws\\CustomDefaultSettings",DriverKeyName);
		// shawn 2011/05/31 modify -----
		
		swprintf((wchar_t *)m_DefValuePath,L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\Class\\%ws\\DefaultSettings",DriverKeyName);
		//2011/5/18 10:42と
		swprintf((wchar_t *)m_pDevExt->pVideoStream->m_DevInstanceKey, L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Control\\Class\\%ws\\Settings",DriverKeyName);

	}//Windows XP or Windows 2000
	else if(IoIsWdmVersionAvailable(1,0x05) || IoIsWdmVersionAvailable(1,0))
	{//Windows ME or Windows 98/SE
		DBGU_TRACE("Windows ME or 98/SE System !\n");
		swprintf((wchar_t *)m_InstanceKey, L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Services\\Class\\%ws\\Settings",DriverKeyName);
		swprintf((wchar_t *)m_CurValuePath, L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Services\\Class\\%ws\\CurrentSettings",DriverKeyName);
		swprintf((wchar_t *)m_DefValuePath, L"\\Registry\\MACHINE\\System\\CurrentControlSet\\Services\\Class\\%ws\\DefaultSettings",DriverKeyName);
	}//Windows ME or Windows 98/SE
	else
	{
		DBGU_TRACE("Unknow Windows System !! \n");
		ntStatus = STATUS_UNSUCCESSFUL; 
	}

	DBGU_TRACE("Instance Key : %ws\n",m_InstanceKey);

	if(NT_SUCCESS(ntStatus))
		RtlInitUnicodeString(&m_InstanceRegPath, (PWSTR) m_InstanceKey);

_exit_GetInstanceKey:

	return (ntStatus);
}

NTSTATUS CVideoDevice::WriteDeviceInfoToReg(LONG InstanceIndex)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UNICODE_STRING	RegPath, DevPath;
	WCHAR temp[16], BasePath[MAX_PATH];

	// Save m_InstanceIndex
	m_InstanceIndex = InstanceIndex;

	RtlInitUnicodeString(&DevPath, (PWSTR) m_InstanceKey);
	swprintf(temp, L"ii%04d", m_InstanceIndex);

	// add to get exact length of m_InstanceKey
	swprintf(BasePath, SN9C220_KEY_PATH);
	RtlInitUnicodeString(&RegPath, (PWSTR) BasePath);

	// Instance key path
	ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
							RegPath.Buffer,
							temp,
							REG_SZ,
							DevPath.Buffer,
							DevPath.Length);

	if (NT_SUCCESS(ntStatus))
	{
		ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
								RegPath.Buffer,
								L"InstanceIndex",
								REG_DWORD,
								&m_InstanceIndex,
								sizeof(ULONG));
	}

	if (NT_SUCCESS(ntStatus))
		m_fWriteDefValue=TRUE;
	else
		m_fWriteDefValue=FALSE;

	return ntStatus;
}

// ask for still image
NTSTATUS
CVideoDevice::StillPinTrigger()
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

	ASSERT(m_pDevExt);
	DBGU_TRACE("Enter StillPinTrigger()\n");
	DBGU_TRACE("pStillPin = %x\n", m_pDevExt->pStillPin);
	
	if (!m_pDevExt->pStillPin)
	{
		// trigger only for size over VGA
		if ((m_SnapShotSize <= SNAPSHOT_640X480) || (m_pDevExt->m_StillCaptureMethod < 2))
		{
			DBGU_TRACE("Leave StillPinTrigger() without request still frame.\n");
			m_SaveSnapShotFile = TRUE;
			m_dSnapShotRetry = 0;
			return ntStatus;
		}

		BOOL bFormatChanged = FALSE;
		VIDEOSTREAMINFO m_OldSnapShotInfo;
		RtlCopyMemory(&m_OldSnapShotInfo, &m_SnapShotInfo, sizeof(VIDEOSTREAMINFO));

		// request as capture pin compression
		if (m_ZoomParam.biCompression == MEDIASUBTYPE_MJPG.Data1)
			m_SnapShotInfo.biCompression = MEDIASUBTYPE_MJPG.Data1;
		else
			m_SnapShotInfo.biCompression = MEDIASUBTYPE_YUY2.Data1;

		switch (m_SnapShotSize)
		{
		// shawn 2011/07/19 +++++
		case SNAPSHOT_800X600:
			m_SnapShotInfo.wWidth = 800;	// Output image width
			m_SnapShotInfo.wHeight = 600;	// Output image height
			m_SnapShotInfo.ImageSize = 1024*768*2;
			break;
		// shawn 2011/07/19 -----

		case SNAPSHOT_1024X768:
			m_SnapShotInfo.wWidth = 1024;	// Output image width
			m_SnapShotInfo.wHeight = 768;	// Output image height
			m_SnapShotInfo.ImageSize = 1024*768*2;
			break;

		case SNAPSHOT_1280X960:
			m_SnapShotInfo.wWidth = 1280;	// Output image width
			m_SnapShotInfo.wHeight = 960;	// Output image height
			m_SnapShotInfo.ImageSize = 1280*960*2;
			break;

		case SNAPSHOT_1280X1024:
			m_SnapShotInfo.wWidth = 1280;	// Output image width
			m_SnapShotInfo.wHeight = 1024;	// Output image height
			m_SnapShotInfo.ImageSize = 1280*1024*2;
			break;

		case SNAPSHOT_1600X1200:
			m_SnapShotInfo.wWidth = 1600;	// Output image width
			m_SnapShotInfo.wHeight = 1200;	// Output image height
			m_SnapShotInfo.ImageSize = 1600*1200*2;
			break;

		// shawn 2011/07/19 +++++
		case SNAPSHOT_2304x1728:
		case SNAPSHOT_2560X2048:
		case SNAPSHOT_3200X2400:
		case SNAPSHOT_2816X2253:
		case SNAPSHOT_3520X2640:
		case SNAPSHOT_2048X1536:
			m_SnapShotInfo.wWidth = 2048;	// Output image width
			m_SnapShotInfo.wHeight = 1536;	// Output image height
			m_SnapShotInfo.ImageSize = 2048*1536*2;
			break;
			break;

		case SNAPSHOT_960X720:
			m_SnapShotInfo.wWidth = 960;	// Output image width
			m_SnapShotInfo.wHeight = 720;	// Output image height
			m_SnapShotInfo.ImageSize = 960*720*2;
			break;

		case SNAPSHOT_1280X800:
			m_SnapShotInfo.wWidth = 1280;	// Output image width
			m_SnapShotInfo.wHeight = 800;	// Output image height
			m_SnapShotInfo.ImageSize = 1280*800*2;
			break;

		case SNAPSHOT_1600X900:
			m_SnapShotInfo.wWidth = 1600;	// Output image width
			m_SnapShotInfo.wHeight = 900;	// Output image height
			m_SnapShotInfo.ImageSize = 1600*900*2;
			break;

		case SNAPSHOT_1280X720:
			m_SnapShotInfo.wWidth = 1280;	// Output image width
			m_SnapShotInfo.wHeight = 720;	// Output image height
			m_SnapShotInfo.ImageSize = 1280*720*2;
			break;

		case SNAPSHOT_640X400:
			m_SnapShotInfo.wWidth = 640;	// Output image width
			m_SnapShotInfo.wHeight = 400;	// Output image height
			m_SnapShotInfo.ImageSize = 640*400*2;
			break;

		case SNAPSHOT_1920X1080:
			m_SnapShotInfo.wWidth = 1920;	// Output image width
			m_SnapShotInfo.wHeight = 1080;	// Output image height
			m_SnapShotInfo.ImageSize = 1920*1080*2;
			break;
		// shawn 2011/07/19 -----

		default:
			m_SnapShotInfo.wWidth = 640;	// Output image width
			m_SnapShotInfo.wHeight = 480;	// Output image height
			m_SnapShotInfo.ImageSize = 640*480*2;
			break;
		}

		ULONG maxWidth=0, i,j=0;

		for (i=0;i<MAX_STREAM_FORMAT;i++)
		{
			if (m_SnapShotInfo.biCompression == m_pDevExt->StillFormatArray[i].biCompression)
				if (m_pDevExt->StillFormatArray[i].wWidth > maxWidth)
				{
					maxWidth = m_pDevExt->StillFormatArray[i].wWidth;
					j=i;
				}
		}

		if (maxWidth < m_SnapShotInfo.wWidth)
		{
			m_SnapShotInfo.wWidth = m_pDevExt->StillFormatArray[j].wWidth;
			m_SnapShotInfo.wHeight = m_pDevExt->StillFormatArray[j].wHeight;
			m_SnapShotInfo.ImageSize = m_SnapShotInfo.wWidth*m_SnapShotInfo.wHeight*2;
		}

		DBGU_TRACE("Ready to get Still with %dX%d\n", m_SnapShotInfo.wWidth, m_SnapShotInfo.wHeight);

		KS_VIDEOINFOHEADER VideoInfoHeader;
		VideoInfoHeader.bmiHeader.biWidth = m_SnapShotInfo.wWidth;
		VideoInfoHeader.bmiHeader.biHeight = m_SnapShotInfo.wHeight;
		VideoInfoHeader.bmiHeader.biSizeImage = m_SnapShotInfo.ImageSize;
		VideoInfoHeader.bmiHeader.biCompression = m_SnapShotInfo.biCompression;

		// check if we need to inform device that still format is changed
		if ((m_OldSnapShotInfo.wWidth != m_SnapShotInfo.wWidth) ||
			(m_OldSnapShotInfo.wHeight != m_SnapShotInfo.wHeight) ||
			(m_OldSnapShotInfo.biCompression != m_SnapShotInfo.biCompression))
			ntStatus = CheckStillPinBandwidth(VideoInfoHeader);

		if (!NT_SUCCESS(ntStatus))
		{
			DBGU_WARNING("StillPinTrigger: call CheckStillPinBandwidth return %X\n", ntStatus);
			return ntStatus;
		}
		else if (m_enImageOut == m_enImageOutStill) // preview size = snapshot size
		{
			DBGU_TRACE("StillPinTrigger: preview size = snapshot size\n");
			return ntStatus;
		}
	}

	ULONG Value = VS_STILL_IMAGE_TRIGGER_CONTROL<<8;
    ULONG Index = m_pDevExt->m_InputEntityID;
	ULONG Buffer = 1;

	if (m_pDevExt->m_pStillFrameBuffer)
		RtlZeroMemory(m_pDevExt->m_pStillFrameBuffer, sizeof(/*1600*1200*3*/2048*1536*3));	// shawn 2011/07/12 modify for 3M

	// now we have got current setting, then set back ..
	ntStatus = USB_ControlClassCommand(
		SET_CUR,
		Value,
		Index,
		&Buffer,
		sizeof(Buffer),
		FALSE);

	m_SaveSnapShotFile = TRUE;
	m_dSnapShotRetry = 0;

	DBGU_TRACE("Exit StillPinTrigger() with %X\n",ntStatus);
	return ntStatus;
}

//
// tell device to preper still pin transfer
// this rountine must be called after CapturePinProbeCommit() is called.
//
NTSTATUS
CVideoDevice::StillPinProbeCommit(
	IN UCHAR bFormatIdex,
	IN UCHAR bFrameIndex
	)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

	VIDEO_STILL_PROBE_AND_COMMIT_CONTROL VSCctrl;
	ULONG Value;
    ULONG Index;
	ULONG BufferLength;
	ULONG dwMaxVideoFrameSize = 0;
	ULONG dwMaxPayloadTransferSize = 0;

	SnPrint (DEBUGLVL_VERBOSE, ("Start Still Probe and Commit\n"));
	RtlZeroMemory(&VSCctrl,sizeof(VIDEO_STILL_PROBE_AND_COMMIT_CONTROL));

	VSCctrl.bFormatIdex = bFormatIdex;
	VSCctrl.bFrameIndex = bFrameIndex;

	Value = VS_STILL_PROBE_CONTROL<<8;
	Index = m_pDevExt->m_InputEntityID;
	BufferLength = sizeof(VIDEO_STILL_PROBE_AND_COMMIT_CONTROL);

	// now we have got current setting, then set back ..
	ntStatus = USB_ControlClassCommand(
		SET_CUR,
		Value,
		Index,
		&VSCctrl,
		BufferLength,
		FALSE);

	// to get the dwMaxVideoFrameSize and dwMaxPayloadTransferSize
	if ( NT_SUCCESS(ntStatus)) {
		ntStatus = USB_ControlClassCommand(
			GET_CUR,
			Value,
			Index,
			&VSCctrl,
			BufferLength,
			TRUE);

		if ( NT_SUCCESS(ntStatus)) {
			// for AllocateBandwidth using
			dwMaxVideoFrameSize = VSCctrl.dwMaxVideoFrameSize;
			dwMaxPayloadTransferSize = VSCctrl.dwMaxPayloadTransferSize;
		    DBGU_TRACE("still dwMaxVideoFrameSize=%d, dwMaxPayloadTransferSize=%d\n",
				dwMaxVideoFrameSize,dwMaxPayloadTransferSize);

			Value = VS_STILL_COMMIT_CONTROL<<8;
			ntStatus = USB_ControlClassCommand(
				SET_CUR,
				Value,
				Index,
				&VSCctrl,
				BufferLength,
				FALSE);
		}
	}

	//2010/8/2 06:52と
	/*m_dwMaxVideoFrameSize = 
		(m_dwMaxVideoFrameSize>dwMaxVideoFrameSize) ? m_dwMaxVideoFrameSize:dwMaxVideoFrameSize;
	m_dwMaxPayloadTransferSize = 
		(m_dwMaxPayloadTransferSize>dwMaxPayloadTransferSize) ? m_dwMaxPayloadTransferSize:dwMaxPayloadTransferSize;
	*/
	m_dwMaxVideoFrameSize = dwMaxVideoFrameSize;
	
	// shawn 2011/06/24 modify +++++
	//m_dwMaxPayloadTransferSize = dwMaxPayloadTransferSize;

	// shawn 2011/08/01 modify +++++
	m_dwMaxPayloadTransferSizeTmp = 
		(m_dwMaxPayloadTransferSizeTmp>dwMaxPayloadTransferSize) ? m_dwMaxPayloadTransferSizeTmp:dwMaxPayloadTransferSize;
	// shawn 2011/08/01 modify -----
	// shawn 2011/06/24 modify -----

	SnPrint (DEBUGLVL_VERBOSE, ("Start StillPinProbeCommit return 0x%x\n", ntStatus));
	return ntStatus;
}

// tell device to start capture pin transfer
NTSTATUS
CVideoDevice::CapturePinProbeCommit(
	IN ULONG AvgTimePerFrame,
	IN UCHAR bFormatIdex,
	IN UCHAR bFrameIndex,
	IN ULONG StreamNumber 
	)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

	VIDEO_PROBE_AND_COMMIT_CONTROL VPCctrl;
	ULONG Value;
    ULONG Index;
	ULONG BufferLength;

	// shawn 2011/07/19 modify +++++
	//m_dwMaxVideoFrameSize = 0;
	//m_dwMaxPayloadTransferSize = 0;
	ULONG dwMaxVideoFrameSize = 0;
	ULONG dwMaxPayloadTransferSize = 0;
	// shawn 2011/07/19 modify -----

	SnPrint (DEBUGLVL_VERBOSE, ("Start Video Probe and Commit\n"));
	RtlZeroMemory(&VPCctrl,sizeof(VIDEO_PROBE_AND_COMMIT_CONTROL));

	VPCctrl.bmHint		= 0x100;
	VPCctrl.bFormatIdex = bFormatIdex;
	VPCctrl.bFrameIndex = bFrameIndex;
	VPCctrl.dwFrameInterval = AvgTimePerFrame;

	Value = VS_PROBE_CONTROL<<8;
	//Index = m_pDevExt->m_InputEntityID;	
	DBGU_TRACE("StreamNumber = %d \n", StreamNumber);
	Index = (StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx) ? 0x2 : m_pDevExt->m_InputEntityID;	// james try M2TS.	// shawn 2011/06/23 modify
	BufferLength = sizeof(VIDEO_PROBE_AND_COMMIT_CONTROL);

    DBGU_TRACE("AvgTimePerFrame=%d \n", AvgTimePerFrame);
	// now we have got current setting, then set back ..
	ntStatus = USB_ControlClassCommand(
		SET_CUR,
		Value,
		Index,
		&VPCctrl,
		BufferLength,
		FALSE);

	DBGU_TRACE("First ProbeCommit = 0x%x \n", ntStatus);

	// to get the dwMaxVideoFrameSize and dwMaxPayloadTransferSize
	if ( NT_SUCCESS(ntStatus)) {
		ntStatus = USB_ControlClassCommand(
			GET_CUR,
			Value,
			Index,
			&VPCctrl,
			BufferLength,
			TRUE);

		DBGU_TRACE("ProbeCommit GetCur = 0x%x \n", ntStatus);

		if ( NT_SUCCESS(ntStatus)) {
			// for AllocateBandwidth using
			// shawn 2011/07/19 modify +++++
			//m_dwMaxVideoFrameSize = VPCctrl.dwMaxVideoFrameSize;
			//m_dwMaxPayloadTransferSize = VPCctrl.dwMaxPayloadTransferSize;
			dwMaxVideoFrameSize = VPCctrl.dwMaxVideoFrameSize;
			dwMaxPayloadTransferSize = VPCctrl.dwMaxPayloadTransferSize;
			// shawn 2011/07/19 modify -----

		    DBGU_TRACE("dwFrameInterval=%d, dwMaxVideoFrameSize=%d, dwMaxPayloadTransferSize=%d\n",
				VPCctrl.dwFrameInterval,dwMaxVideoFrameSize,dwMaxPayloadTransferSize);

			Value = VS_COMMIT_CONTROL<<8;
			ntStatus = USB_ControlClassCommand(
				SET_CUR,
				Value,
				Index,
				&VPCctrl,
				BufferLength,
				FALSE);

			DBGU_TRACE("Second ProbeCommit = 0x%x \n", ntStatus);
		}
	}

	// shawn 2011/07/19 +++++
	m_dwMaxVideoFrameSize = dwMaxVideoFrameSize;
	
	if (StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/07/20 add
		m_dwMaxPayloadTransferSize_MP2TS = dwMaxPayloadTransferSize;
	else
	{
		// shawn 2011/08/01 modify +++++
		m_dwMaxPayloadTransferSizeTmp = 
			(m_dwMaxPayloadTransferSizeTmp>dwMaxPayloadTransferSize) ? m_dwMaxPayloadTransferSizeTmp:dwMaxPayloadTransferSize;
		// shawn 2011/08/01 modify -----
	}
	// shawn 2011/07/19 -----

	SnPrint (DEBUGLVL_VERBOSE, ("Start CapturePinProbeCommit return 0x%x\n", ntStatus));
	return ntStatus;
}

NTSTATUS CVideoDevice::ReadSnapShotStateFromReg()
{
	DBGU_FN("ReadSnapShotStateFromReg()");

	NTSTATUS ntStatus = STATUS_SUCCESS;

	ntStatus = RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
										m_InstanceRegPath.Buffer,
										m_InstanceTable,
										NULL, 
										NULL);

	DBGU_TRACE("Get from registry: m_SnapShotState %d (%x)\n", m_SnapShotState,ntStatus); 
	return ntStatus;
}

NTSTATUS CVideoDevice::WriteSnapShotStateToReg()
{
	DBGU_FN("WriteSnapShotStateToReg()");

	NTSTATUS ntStatus = STATUS_SUCCESS;

	ntStatus = RtlWriteRegistryValue(	RTL_REGISTRY_ABSOLUTE,
										m_InstanceRegPath.Buffer,
										L"SnapShotState",
										REG_DWORD,
										&m_SnapShotState,
										sizeof(ULONG));

	DBGU_TRACE("Set to registry: m_SnapShotState %d (%x)\n", m_SnapShotState,ntStatus);
	return ntStatus;
}

NTSTATUS IsoCompleteRoutine(PDEVICE_OBJECT pDeviceObject,PIRP Irp,PVOID Context)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	TRANSFEROBJ *pTransferObj = (TRANSFEROBJ *)Context;
	PURB pUrb = pTransferObj->pUrb;
	USBD_PIPE_INFORMATION PipeInfo;
	CVideoDevice *pVideoDevice = (CVideoDevice *) pTransferObj->pContext;
	int	PktIdx, NumPackets = pUrb->UrbIsochronousTransfer.NumberOfPackets;
	PUCHAR pXferBuf;
	ULONG XferLen;
	bool bNewFrmae;
		
	RtlZeroMemory(&PipeInfo,sizeof(USBD_PIPE_INFORMATION));

	if(!NT_SUCCESS(Irp->IoStatus.Status)) {
		DBGU_WARNING("[ISO]!!! Irp Error !!! (ErrCode: 0x%08X)\n", Irp->IoStatus.Status);

		if ((Irp->IoStatus.Status == STATUS_UNSUCCESSFUL) &&
			(pUrb->UrbHeader.Status == USBD_STATUS_ISOCH_REQUEST_FAILED))
		{
			DBGU_WARNING("[ISO]!!! Urb Error !!! ReTry !!!\n");
			goto _Start_Next_ISO_Xfer;
		}
		else
			goto _exit_Iso_Complete;
	}

	if (USBD_ERROR(pUrb->UrbHeader.Status)) {
		DBGU_WARNING("[ISO]!!! Urb Error !!! (ErrCode: 0x%08X)\n", pUrb->UrbHeader.Status);
		goto _exit_Iso_Complete;
	}

	if (pUrb->UrbIsochronousTransfer.ErrorCount == NumPackets) {
		DBGU_WARNING("[ISO]!!! UrbIsochronousTransfer.ErrorCount == NumPackets !!!\n");
		goto _exit_Iso_Complete;
	}
	
	//2010/6/29 05:50と H264
	if((pVideoDevice->m_pDevExt->m_StreamState != KSSTATE_RUN && pTransferObj->StreamNumber == STREAM_Capture)
	||(pVideoDevice->m_pDevExt->m_MPEG2StreamState != KSSTATE_RUN && pTransferObj->StreamNumber == pVideoDevice->m_pDevExt->m_STREAM_Capture_MP2TS_Idx))	// shawn 2011/06/23 modify
	{
		DBGU_WARNING("[ISO]!! Reset iso pipe !!\n");
		goto _exit_Iso_Complete;
	}

	//if(pTransferObj->StreamNumber == STREAM_Capture_MP2TS)
	//		DBGU_TRACE("IsoCompleteRoutine  StreamNumber = %d\n",pTransferObj->StreamNumber);
			
	//
	// process all iso packets
	//
	XferLen = 0;
	//2010/7/21 05:59と
	bNewFrmae = FALSE;
	//2011/4/7 06:11と
	ULONG TotalValidDataLength = 0;
	
	for (PktIdx = 0; PktIdx < NumPackets; ++PktIdx) {
		pXferBuf = pTransferObj->pUsbBuffer + pUrb->UrbIsochronousTransfer.IsoPacket[PktIdx].Offset;
		XferLen = pUrb->UrbIsochronousTransfer.IsoPacket[PktIdx].Length;

		//DBGU_TRACE("[ISO]STreamNum=%d, Packet[%d]XferLen=%d, Status=0x%08X\n", pTransferObj->StreamNumber, PktIdx, XferLen, pUrb->UrbIsochronousTransfer.IsoPacket[PktIdx].Status);
		if (!XferLen || USBD_ERROR(pUrb->UrbIsochronousTransfer.IsoPacket[PktIdx].Status))
			continue;

		//2010/7/21 05:40と skype
		if(pTransferObj->StreamNumber == pVideoDevice->m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
		{
			if(pVideoDevice->TSMuxEnable)
				pVideoDevice->ParseTransferBuffer(pXferBuf, XferLen, pTransferObj->StreamNumber, pTransferObj->PipeNumber);
			else
			{
				PPAYLOAD_HEADER PayloadHeader = (PPAYLOAD_HEADER)pXferBuf;
				ULONG	dValidDataLength=0;
	
				//DBGU_TRACE("[ISO]STreamNum=%d, Packet[%d]XferLen=%d, Status=0x%08X\n", pTransferObj->StreamNumber, PktIdx, XferLen, pUrb->UrbIsochronousTransfer.IsoPacket[PktIdx].Status);
		
				//2010/9/13 03:22と
				//if((XferLen < PayloadHeader->bLength)||(PayloadHeader->bLength != 0x0C))
				if((XferLen < PayloadHeader->bLength)||(PayloadHeader->bLength != 0x0C && PayloadHeader->bLength != 0x02))
					continue;		

				dValidDataLength = XferLen - PayloadHeader->bLength;
				
				if(dValidDataLength>0)
				{
					//2011/4/7 06:12と
					//if(!bNewFrmae && (pVideoDevice->m_MPEG2ValidDataLength + dValidDataLength > pVideoDevice->m_APBufferLen[STREAM_Capture_MP2TS]))
					//	bNewFrmae = TRUE;
	
					//2010/7/23 06:01と ring
					if((pVideoDevice->m_MPEG2IsoBufferAddr + dValidDataLength) > DEFAULT_DATA_BUFFER_SIZE)
					{
						UINT Offset = DEFAULT_DATA_BUFFER_SIZE - pVideoDevice->m_MPEG2IsoBufferAddr;
						RtlCopyMemory(pVideoDevice->pMP2TSDataBuf + pVideoDevice->m_MPEG2IsoBufferAddr, pXferBuf+PayloadHeader->bLength, Offset);
						RtlCopyMemory(pVideoDevice->pMP2TSDataBuf, pXferBuf+PayloadHeader->bLength + Offset, dValidDataLength - Offset);
						pVideoDevice->m_MPEG2IsoBufferAddr = dValidDataLength - Offset;
					}
					else
					{
						RtlCopyMemory(pVideoDevice->pMP2TSDataBuf + pVideoDevice->m_MPEG2IsoBufferAddr, pXferBuf+PayloadHeader->bLength, dValidDataLength);
						pVideoDevice->m_MPEG2IsoBufferAddr += dValidDataLength;
						
						if(pVideoDevice->m_MPEG2IsoBufferAddr >= DEFAULT_DATA_BUFFER_SIZE)
							pVideoDevice->m_MPEG2IsoBufferAddr = 0;
					}
					
					//2011/4/7 06:09と
					TotalValidDataLength += dValidDataLength;
					//pVideoDevice->m_MPEG2ValidDataLength += dValidDataLength;
										
				}
			}
		}	
    else
	    pVideoDevice->ParseTransferBuffer(pXferBuf, XferLen, pTransferObj->StreamNumber,pTransferObj->PipeNumber);

	} //PktIdx
	
	if(!pVideoDevice->TSMuxEnable &&(pTransferObj->StreamNumber == pVideoDevice->m_pDevExt->m_STREAM_Capture_MP2TS_Idx))	// shawn 2011/06/23 modify
	{
		//2011/4/7 06:12と
		pVideoDevice->m_MPEG2ValidDataLength += TotalValidDataLength;
		//DBGU_TRACE("pVideoDevice->m_MPEG2ValidDataLength =%d TotalValidDataLength=%d\n", pVideoDevice->m_MPEG2ValidDataLength,TotalValidDataLength);
	
		if(pVideoDevice->m_MPEG2ValidDataLength > pVideoDevice->m_APBufferLen[pVideoDevice->m_pDevExt->m_STREAM_Capture_MP2TS_Idx])	// shawn 2011/06/23 modify
		{
			DBGU_TRACE("KeSetEvent(&pVideoDevice->m_MEPG2DataReadyEvent\n");
			KeSetEvent(&pVideoDevice->m_MEPG2DataReadyEvent, 0, FALSE);
		}

		TotalValidDataLength = 0;
	}
_Start_Next_ISO_Xfer:
	//
	// start next Iso transfer
	//

	//2010/6/22 02:19と H264
	//ntStatus = pVideoDevice->GetPipeInfo(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE_INDEX, &PipeInfo);
	if(pTransferObj->StreamNumber == STREAM_Capture)
	{
		//2011/5/11 05:21と
		if(pTransferObj->PipeNumber == 0)
			ntStatus = pVideoDevice->GetPipeInfo(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE_INDEX, &PipeInfo);
		else if(pTransferObj->PipeNumber == 1)
			ntStatus = pVideoDevice->GetPipeInfo(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE2_INDEX, &PipeInfo);
	}
	else if(pTransferObj->StreamNumber == pVideoDevice->m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
		ntStatus = pVideoDevice->GetPipeInfo(SELECTED_MPEG2_INTERFACE_INDEX, ISO_PIPE_INDEX, &PipeInfo);

	if(NT_SUCCESS(ntStatus))
	{
		ntStatus = pVideoDevice->UsbIsochronousTransfer(	pTransferObj->pIrp,	
												pTransferObj->pUrb,
												&PipeInfo,	
												TRUE,
												pTransferObj->pUsbBuffer,	
												NULL,
												(PVOID) IsoCompleteRoutine,	
												(PVOID) pTransferObj);
	}

	if(NT_SUCCESS(ntStatus))
	{
		//2010/6/22 02:52と H264
		if(pTransferObj->StreamNumber == STREAM_Capture)
		{
			if(pTransferObj->PipeNumber == 0)
				InterlockedIncrement(&pVideoDevice->m_UsbIoCount);
			else if(pTransferObj->PipeNumber == 1)
				InterlockedIncrement(&pVideoDevice->m_MPEG2UsbIoCount);
		}
		else if(pTransferObj->StreamNumber == pVideoDevice->m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
			InterlockedIncrement(&pVideoDevice->m_MPEG2UsbIoCount);
	}
	else
	{
		DBGU_WARNING("[ISO]Fail to setup Iso transfer (ErrCode: 0x%08X, UsbIoCount: %d) !\n", ntStatus, pVideoDevice->m_UsbIoCount);
		return STATUS_MORE_PROCESSING_REQUIRED;
	}

_exit_Iso_Complete:

	Irp->PendingReturned=0;
	
	//2010/6/22 02:52と H264
	if(pTransferObj->StreamNumber == STREAM_Capture)
	{
		if(pTransferObj->PipeNumber == 0)
			InterlockedDecrement(&pVideoDevice->m_UsbIoCount);
		else if(pTransferObj->PipeNumber == 1)
			InterlockedDecrement(&pVideoDevice->m_MPEG2UsbIoCount);
	}
	else if(pTransferObj->StreamNumber == pVideoDevice->m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
		InterlockedDecrement(&pVideoDevice->m_MPEG2UsbIoCount);

	return STATUS_MORE_PROCESSING_REQUIRED;
}


//RBK BULKMODE

NTSTATUS BulkCompleteRoutine(PDEVICE_OBJECT pDeviceObject,PIRP Irp,PVOID Context)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	TRANSFEROBJ *pTransferObj = (TRANSFEROBJ *)Context;
	PURB pUrb = pTransferObj->pUrb;
	USBD_PIPE_INFORMATION PipeInfo;
	CVideoDevice *pVideoDevice = (CVideoDevice *) pTransferObj->pContext;

	PUCHAR pXferBuf;
	ULONG XferLen;

	pXferBuf = pTransferObj->pUsbBuffer;
	XferLen  = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength ;
		
	RtlZeroMemory(&PipeInfo,sizeof(USBD_PIPE_INFORMATION));

	if(!NT_SUCCESS(Irp->IoStatus.Status)) {
		DBGU_WARNING("[BULK]!!! Irp Error !!! (ErrCode: 0x%08X)\n", Irp->IoStatus.Status);

		if (Irp->IoStatus.Status == STATUS_UNSUCCESSFUL)
			goto _exit_Bulk_Complete;
	}

	if (USBD_ERROR(pUrb->UrbHeader.Status)) {
		DBGU_WARNING("[BULK]!!! Urb Error !!! (ErrCode: 0x%08X)\n", pUrb->UrbHeader.Status);
		goto _exit_Bulk_Complete;
	}

	
	//2010/6/29 05:50と H264
	if(pVideoDevice->m_pDevExt->m_StreamState != KSSTATE_RUN && pTransferObj->StreamNumber == STREAM_Capture)
	{
		DBGU_WARNING("[BULK]!! Reset BULK pipe !!\n");
		goto _exit_Bulk_Complete;
	}


	ntStatus = pVideoDevice->ParseTransferBuffer(pXferBuf, 
												 XferLen, 
												  pTransferObj->StreamNumber,
												  pTransferObj->PipeNumber);

	if(pTransferObj->StreamNumber == STREAM_Capture)
		ntStatus = pVideoDevice->GetPipeInfo(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE_INDEX, &PipeInfo);

	
	if(NT_SUCCESS(ntStatus))
	{
		ntStatus = pVideoDevice->UsbBulkOrInterruptTransfer(	pTransferObj->pIrp,	
													pTransferObj->pUrb,
													&PipeInfo,	
													TRUE,
													pTransferObj->pUsbBuffer,	
													NULL,
													pTransferObj->usUsbBufferLength,
													(PVOID) BulkCompleteRoutine,	
													(PVOID) pTransferObj);
	}

	if(NT_SUCCESS(ntStatus))
	{
		if(pTransferObj->StreamNumber == STREAM_Capture)
		{
				InterlockedIncrement(&pVideoDevice->m_UsbIoCount);
		}
	}
	else
	{
		DBGU_WARNING("[BULK]Fail to setup Bulk transfer (ErrCode: 0x%08X, UsbIoCount: %d) !\n", ntStatus, pVideoDevice->m_UsbIoCount);
	}



_exit_Bulk_Complete:

	Irp->PendingReturned=0;
	
	//2010/6/22 02:52と H264
	if(pTransferObj->StreamNumber == STREAM_Capture)
	{
		if(pTransferObj->PipeNumber == 0)
			InterlockedDecrement(&pVideoDevice->m_UsbIoCount);

	}

	return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS IntCompleteRoutine(PDEVICE_OBJECT pDeviceObject,PIRP Irp,PVOID Context)
{
	DBGU_INFO("!IntCompleteRoutine\n");
	NTSTATUS ntStatus = STATUS_SUCCESS;
	TRANSFEROBJ *pTransferObj = (TRANSFEROBJ *)Context;
	PURB pUrb = pTransferObj->pUrb;
	CVideoDevice *pVideoDevice = (CVideoDevice *) pTransferObj->pContext;
	PUCHAR pUsbBuf = pTransferObj->pUsbBuffer;
	USBD_PIPE_INFORMATION PipeInfo;

	RtlZeroMemory(&PipeInfo,sizeof(USBD_PIPE_INFORMATION));

	if(!NT_SUCCESS(Irp->IoStatus.Status)) {
		DBGU_TRACE("!! Interrupt Irp Error (Error Code : %x)\n",Irp->IoStatus.Status);
		goto _exit_Int_Complete;
	}

	if (USBD_ERROR(pUrb->UrbHeader.Status)) {
		DBGU_TRACE("!! Interrupt Urb Error (Error Code : %x)\n",pUrb->UrbHeader.Status);
		goto _exit_Int_Complete;  
	}

	if(pVideoDevice->m_pDevExt->DevicePowerState != PowerDeviceD0)
	{
		DBGU_TRACE("IntCompleteRoutine (Device enter suspend mode !)\n");
		goto _exit_Int_Complete;
	}

	if ((pTransferObj->pUsbBuffer[0] == 2) &&	// VideoStreaming interface
		(pTransferObj->pUsbBuffer[1] == 1) &&	// Interface 1
		(pTransferObj->pUsbBuffer[2] == 0))		// Button
	{
		if (pTransferObj->pUsbBuffer[3] == 1)	// Button pressed
		{
			DBGU_TRACE("Hardware button pressed!\n");
			pTransferObj->pUsbBuffer[3] = 0;
			
			if (!(pVideoDevice->m_EventType&SONIX_EVENT_INTERRUPT))
			{
				// set bit of SONIX_EVENT_INTERRUPT to 1
				pVideoDevice->m_EventType |= SONIX_EVENT_INTERRUPT;
				KeReleaseSemaphore(&pVideoDevice->m_EventHandlerSemaphore,0,1,FALSE);
			}
		}
		else if (pTransferObj->pUsbBuffer[3] == 0)	// Button released
		{
			DBGU_TRACE("Hardware button released!\n");
		}
		else
		{
			DBGU_TRACE("Invalid Interrupt Data !\n");
			goto _exit_Int_Complete;  
		}
	}
	else
	{
		DBGU_TRACE("Invalid Interrupt Data !\n");
		goto _exit_Int_Complete;  
	}

	ntStatus = pVideoDevice->StartInterruptTransfer();

	if(!NT_SUCCESS(ntStatus))
	{
		DBGU_TRACE("Fail to setup interrupt transfer(error : %x), (%d) !\n",ntStatus,pVideoDevice->m_UsbIoCount);
		return STATUS_MORE_PROCESSING_REQUIRED;
	}

_exit_Int_Complete:

	Irp->PendingReturned=0;
	//2010/6/22 02:54と H264
	//InterlockedDecrement(&pVideoDevice->m_UsbIoCount);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

PDATABUFCONTENT	CVideoDevice::GetDataBuffer(DATABUFOP opCode, ULONG DataBufObjNumber)
{
	PDATABUFCONTENT pContent = NULL;
	int i;
	ULONG State;
	
	//2010/6/21 05:18と H264
	if(DataBufObjNumber == 0)
	{
		if (DATAOP_READ == opCode) {
			//
			// Find the buffer of data ae ready
			//
			for(i=0;i<NUM_OF_DATA_BUFFER;i++){
				State = InterlockedCompareExchange((LONG volatile *)&(m_DataBufObj[i].State), DATA_READ, DATA_READY);
				
				if(State == DATA_READY) {
					pContent = &(m_DataBufObj[i].Content);
					DBGU_INFO("DataBuf[%d]: State from DATA_READY to DATA_READ\n", i);
					break;
				}
			}
		}
		else if (DATAOP_WRITE == opCode) {
			//
			// Find the buffer of no data
			//
			for(i=0;i<NUM_OF_DATA_BUFFER;i++){
				State = InterlockedCompareExchange((LONG volatile *)&(m_DataBufObj[i].State), DATA_WRITE, NO_DATA);
				
				if(State == NO_DATA){
					pContent = &(m_DataBufObj[i].Content);
					DBGU_INFO("DataBuf[%d]: State from NO_DATA to DATA_WRITE\n", i);
					break;
				}
			}
	
			if (!pContent)
			{// Cannot find a buffer to write
				//
				// Overwrite the buffer of data ready 
				//
				for(i=0;i<NUM_OF_DATA_BUFFER;i++){
					State = InterlockedCompareExchange((LONG volatile *)&(m_DataBufObj[i].State), DATA_WRITE, DATA_READY);
					
					if(State == DATA_READY){
						pContent = &(m_DataBufObj[i].Content);
						DBGU_INFO("DataBuf[%d]: State from DATA_READY to DATA_WRITE\n", i);
						break;
					}
				}
			}
		}
	}
	else if(DataBufObjNumber == 1)
	{
		if (DATAOP_READ == opCode) {
			//
			// Find the buffer of data ae ready
			//
			for(i=0;i<NUM_OF_DATA_BUFFER;i++){
				State = InterlockedCompareExchange((LONG volatile *)&(m_MPEG2DataBufObj[i].State), DATA_READ, DATA_READY);
				
				if(State == DATA_READY) {
					pContent = &(m_MPEG2DataBufObj[i].Content);
					DBGU_INFO("m_MPEG2DataBufObj[%d]: State from DATA_READY to DATA_READ\n", i);
					break;
				}
			}
		}
		else if (DATAOP_WRITE == opCode) {
			//
			// Find the buffer of no data
			//
			for(i=0;i<NUM_OF_DATA_BUFFER;i++){
				State = InterlockedCompareExchange((LONG volatile *)&(m_MPEG2DataBufObj[i].State), DATA_WRITE, NO_DATA);
				
				if(State == NO_DATA){
					pContent = &(m_MPEG2DataBufObj[i].Content);
					DBGU_INFO("m_MPEG2DataBufObj[%d]: State from NO_DATA to DATA_WRITE\n", i);
					break;
				}
			}
	
			if (!pContent)
			{// Cannot find a buffer to write
				//
				// Overwrite the buffer of data ready 
				//
				for(i=0;i<NUM_OF_DATA_BUFFER;i++){
					State = InterlockedCompareExchange((LONG volatile *)&(m_MPEG2DataBufObj[i].State), DATA_WRITE, DATA_READY);
					
					if(State == DATA_READY){
						pContent = &(m_MPEG2DataBufObj[i].Content);
						DBGU_INFO("m_MPEG2DataBufObj[%d]: State from DATA_READY to DATA_WRITE\n", i);
						break;
					}
				}
			}
		}
	}

	return pContent;
}

void CVideoDevice::ReturnDataBuffer(DATABUFOP opCode, ULONG DataBufObjNumber)
{
	int i;
	ULONG State;

	//2010/6/22 11:21と H264
	if(DataBufObjNumber == 0)
	{
		if (DATAOP_READ == opCode) {
			for(i=0;i<NUM_OF_DATA_BUFFER;i++){
				if(m_DataBufObj[i].State == DATA_READ){
					State = InterlockedCompareExchange((LONG volatile *)&(m_DataBufObj[i].State), NO_DATA, DATA_READ);
					DBGU_INFO("DataBuf[%d]: State from DATA_READ to NO_DATA\n", i);
					break;
				}
			}
		}
		else if (DATAOP_WRITE == opCode) {
			for(i=0;i<NUM_OF_DATA_BUFFER;i++){
				if(m_DataBufObj[i].State == DATA_WRITE){
					State = InterlockedCompareExchange((LONG volatile *)&(m_DataBufObj[i].State), DATA_READY, DATA_WRITE);		
					KeSetEvent(&m_DataReadyEvent, 0, FALSE);
					DBGU_INFO("[DBG]DataBuf[%d]: State from DATA_WRITE to DATA_READY\n", i);
	
					m_dwFrameSN++;
					m_DataBufObj[i].Content.FrameSN = m_dwFrameSN;
					DBGU_INFO("[DBG]*** NewFrame[%d] from DataBuf[%d], DataCount: %d ***\n", m_dwFrameSN, i, m_DataBufObj[i].Content.DataLength);
					break;
				}
			}
		}
	}
	else if(DataBufObjNumber == 1)
	{
		if (DATAOP_READ == opCode) {
			for(i=0;i<NUM_OF_DATA_BUFFER;i++){
				if(m_MPEG2DataBufObj[i].State == DATA_READ){
					State = InterlockedCompareExchange((LONG volatile *)&(m_MPEG2DataBufObj[i].State), NO_DATA, DATA_READ);	
					DBGU_INFO("m_MPEG2DataBufObj[%d]: State from DATA_READ to NO_DATA\n", i);
					break;
				}
			}
		}
		else if (DATAOP_WRITE == opCode) {
			for(i=0;i<NUM_OF_DATA_BUFFER;i++){
				if(m_MPEG2DataBufObj[i].State == DATA_WRITE){
					State = InterlockedCompareExchange((LONG volatile *)&(m_MPEG2DataBufObj[i].State), DATA_READY, DATA_WRITE);		
					//KeSetEvent(&TsMuxSemaphore, 0, FALSE);
					
					//2011/5/11 05:48と
					if(TSMuxEnable)
					{
						KeReleaseSemaphore(&TsMuxSemaphore,0,1,FALSE);				
						m_dwFrameSN++;
					}
					else if(BeST5R286)
						KeSetEvent(&m_MEPG2DataReadyEvent, 0, FALSE);	
	//				DBGU_INFO("[DBG]m_MPEG2DataBufObj[%d]: State from DATA_WRITE to DATA_READY\n", i);
	
//					m_dwFrameSN++;
					m_MPEG2DataBufObj[i].Content.FrameSN = m_dwFrameSN;
					DBGU_INFO("[DBG]*** NewFrame[%d] from m_MPEG2DataBufObj, DataCount: %d ***\n", m_dwFrameSN, i, m_MPEG2DataBufObj[i].Content.DataLength);
					break;
				}
			}
		}
	}
		
}

NTSTATUS CVideoDevice::ParseTransferBuffer(PUCHAR pXferBuf, ULONG XferLen, ULONG StreamNumber, ULONG PipeNumber)
{
  NTSTATUS ntStatus=STATUS_SUCCESS;
	PPAYLOAD_HEADER PayloadHeader = (PPAYLOAD_HEADER)pXferBuf;
	ULONG	dValidDataLength=0;
	//2010/6/23 03:08と
	BOOLEAN fValidHeader ;		// flag to indicate whether payload header is valid (ymwu, 2007/5/25)

	BOOLEAN		bNewFrame;		// james try M2TS.
	BOOLEAN		bNewFrame_pipe2;	
	
	fValidHeader = TRUE;
	bNewFrame = FALSE;
	bNewFrame_pipe2 = FALSE;
	
	if (!pXferBuf || (XferLen == 0))
	{
		DBGU_TRACE("(!pXferBuf || (XferLen == 0)) StreamNum=%d\n",StreamNumber);

		return STATUS_INVALID_PARAMETER;
	}

	if (XferLen < PayloadHeader->bLength)
	{
		DBGU_TRACE("(XferLen < PayloadHeader->bLength) StreamNumber=%d\n",StreamNumber);

		return STATUS_INVALID_PARAMETER;
	}
	
	//DBGU_TRACE("PayloadHeader->bLength=%d, StreamNumber=%d \n",PayloadHeader->bLength, StreamNumber);

	//2010/5/14 06:27と
	if (PayloadHeader->bLength != 0x0C)
	{
		DBGU_TRACE("Invalid payload header length =%d StreamNumber=%d\n",PayloadHeader->bLength, StreamNumber);
		fValidHeader = FALSE;
		dValidDataLength = XferLen;
		//test 2010/6/23 03:48と
		return STATUS_INVALID_PARAMETER;
	}
	else
		dValidDataLength = XferLen - PayloadHeader->bLength;

	//2010/6/22 11:18と H264
	if(fValidHeader)
	{
		if(PipeNumber == 0)
			bNewFrame = (StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx) ? (m_FrameFlag_MP2TS != (PayloadHeader->bBFH&1)) : (m_FrameFlag != (PayloadHeader->bBFH&1));	// shawn 2011/06/23 modify
		else if(PipeNumber == 1)
			bNewFrame_pipe2 = (m_FrameFlag_Pipe2 != (PayloadHeader->bBFH&1));
	}
	
	//	bNewFrame = (StreamNumber == STREAM_Capture_MP2TS) ? (m_pMPEG2DataContent->DataLength + dValidDataLength > m_APBufferLen[STREAM_Capture_MP2TS]) : (m_FrameFlag != (PayloadHeader->bBFH&1));

	//if(StreamNumber == STREAM_Capture_MP2TS)
	//	DBGU_TRACE("ParseTransferBuffer StreamNumber = %d dValidDataLength = %d bNewFrame=%d\n",StreamNumber, dValidDataLength, bNewFrame);

	//2010/6/23 07:11とH264
	if(StreamNumber == STREAM_Capture)
	{
		if(PipeNumber == 0)
		{
			//2010/11/9 07:13と
			//DBGU_TRACE("Pipe 1 PayloadHeader->bBFH =%X, frame id = %X, dValidDataLength = %d",PayloadHeader->bBFH, PayloadHeader->bBFH&1, dValidDataLength);
				
			if(bNewFrame)
			{	// a new frame come
				//DBGU_WARNING("pipe 1 new frame! DataLength = %d PTS = %x, %x, %x, %x\n",m_pDataContent->DataLength,
				//PayloadHeader->bPTS[0],PayloadHeader->bPTS[1],PayloadHeader->bPTS[2],PayloadHeader->bPTS[3]);

				DBGU_TRACE("New frame come !!\n");	// shawn 2011/08/25 add
		
				m_FrameFlag = PayloadHeader->bBFH&1;
					
				//2010/10/13 03:29と
				//ReturnDataBuffer(DATAOP_WRITE);
				ReturnDataBuffer(DATAOP_WRITE, PipeNumber);
					
				//m_pDataContent = GetDataBuffer(DATAOP_WRITE);
				m_pDataContent = GetDataBuffer(DATAOP_WRITE, PipeNumber);
					
				if (m_pDataContent)
				{
					m_pDataContent->DataLength = 0;
					
					if(PayloadHeader->bBFH&0x20)
					{
						m_pDataContent->isStillImage = TRUE;
						DBGU_TRACE("This Packet is Still Frame. \n");
					}
					else
					{
						m_pDataContent->isStillImage = FALSE;
					}
				}
			}
			
			//DBGU_WARNING("pipe 1 dValidDataLength = %d\n",dValidDataLength);	
			if (m_pDataContent && dValidDataLength>0)
			{
				if ((m_pDataContent->DataLength + dValidDataLength) > DEFAULT_DATA_BUFFER_SIZE)
				{
					DBGU_WARNING("[DBG]!! Image Size is over the size of data buffer !! (Receive Size: %d, Buffer Size: %d)\n",m_pDataContent->DataLength, DEFAULT_DATA_BUFFER_SIZE);
					// drop this frame
					m_pDataContent->DataLength = 0;
				}
			
				//2010/11/15 02:46と
				if(m_pDataContent->DataLength == 0)
				{
					m_pDataContent->PTS = PayloadHeader->bPTS[0] 
										+ (PayloadHeader->bPTS[1]<<8) 
										+ (PayloadHeader->bPTS[2]<<16) 
										+ (PayloadHeader->bPTS[3]<<24);
					DBGU_TRACE("Pipe 1 PTS = %X\n",m_pDataContent->PTS);
				}
			
				RtlCopyMemory((m_pDataContent->pDataBuf + m_pDataContent->DataLength), (pXferBuf+PayloadHeader->bLength), dValidDataLength);
				m_pDataContent->DataLength += dValidDataLength;
				//DBGU_WARNING("pipe 1 RtlCopyMemory dValidDataLength = %d DataLength=%d\n",dValidDataLength,m_pDataContent->DataLength);
	
			}
			else if (!m_pDataContent)
				DBGU_WARNING("No available buffer to store ISO packet data!!\n");
		}
		else
		{
			//2010/11/9 07:13と
			//DBGU_TRACE("Pipe 2 PayloadHeader->bBFH =%X, frame id = %X, dValidDataLength = %d",PayloadHeader->bBFH, PayloadHeader->bBFH&1, dValidDataLength);
				
			if(bNewFrame_pipe2)
			{	// a new frame come
				m_FrameFlag_Pipe2 = PayloadHeader->bBFH&1;
		
				//DBGU_WARNING("pipe 2 new frame! DataLength = %d PTS = %x, %x, %x, %x\n",m_pMPEG2DataContent->DataLength,
				//PayloadHeader->bPTS[0],PayloadHeader->bPTS[1],PayloadHeader->bPTS[2],PayloadHeader->bPTS[3]);
		
				//2010/10/13 03:29と
				//ReturnDataBuffer(DATAOP_WRITE);
				ReturnDataBuffer(DATAOP_WRITE, PipeNumber);
					
				//m_pDataContent = GetDataBuffer(DATAOP_WRITE);
				m_pMPEG2DataContent = GetDataBuffer(DATAOP_WRITE, PipeNumber);
					
				if (m_pMPEG2DataContent)
				{
					m_pMPEG2DataContent->DataLength = 0;
		
					if(PayloadHeader->bBFH&0x20)
					{
						m_pMPEG2DataContent->isStillImage = TRUE;
						DBGU_TRACE("This Packet is Still Frame. \n");
					}
					else
					{
						m_pMPEG2DataContent->isStillImage = FALSE;
					}
				}
			}
				
			//2010/11/8 07:18と
			//DBGU_WARNING("pipe 2 dValidDataLength = %d\n",dValidDataLength);

			if (m_pMPEG2DataContent && dValidDataLength>0)
			{
				if ((m_pMPEG2DataContent->DataLength + dValidDataLength) > DEFAULT_DATA_BUFFER_SIZE)
				{
					DBGU_WARNING("[DBG]!! Image Size is over the size of data buffer !! (Receive Size: %d, Buffer Size: %d)\n",m_pMPEG2DataContent->DataLength, DEFAULT_DATA_BUFFER_SIZE);
					// drop this frame
					m_pMPEG2DataContent->DataLength = 0;
				}

				//2010/11/15 02:46と
				if(m_pMPEG2DataContent->DataLength == 0)
				{
					m_pMPEG2DataContent->PTS = PayloadHeader->bPTS[0] 
											+ (PayloadHeader->bPTS[1]<<8) 
											+ (PayloadHeader->bPTS[2]<<16) 
											+ (PayloadHeader->bPTS[3]<<24);
					DBGU_TRACE("Pipe 2 PTS = %X\n",m_pMPEG2DataContent->PTS);
				}			
		
				RtlCopyMemory((m_pMPEG2DataContent->pDataBuf + m_pMPEG2DataContent->DataLength), (pXferBuf+PayloadHeader->bLength), dValidDataLength);
		
				m_pMPEG2DataContent->DataLength += dValidDataLength;
				//DBGU_WARNING("pipe 2 RtlCopyMemory dValidDataLength = %d DataLength=%d\n",dValidDataLength,m_pMPEG2DataContent->DataLength);
			}
			else if (!m_pMPEG2DataContent)
				DBGU_WARNING("No available buffer to store ISO packet data!!\n");
		}		
		
	}
	else if(StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
	{
		if(bNewFrame)
		{	// a new frame come

			m_FrameFlag_MP2TS = PayloadHeader->bBFH&1;
			ReturnDataBuffer(DATAOP_WRITE, 1);
			m_pMPEG2DataContent = GetDataBuffer(DATAOP_WRITE, 1);
			
			if (m_pMPEG2DataContent)
			{
				m_pMPEG2DataContent->DataLength = 0;
				m_pMPEG2DataContent->isStillImage = FALSE;
			}
		}
	
		if (m_pMPEG2DataContent && dValidDataLength>0)
		{
			if ((m_pMPEG2DataContent->DataLength + dValidDataLength) > DEFAULT_DATA_BUFFER_SIZE)
			{
				DBGU_WARNING("[DBG]!! Image Size is over the size of data buffer !! (Receive Size: %d, Buffer Size: %d)\n",m_pMPEG2DataContent->DataLength, DEFAULT_DATA_BUFFER_SIZE);
				// drop this frame
				m_pMPEG2DataContent->DataLength = 0;
			}
	
			RtlCopyMemory((m_pMPEG2DataContent->pDataBuf + m_pMPEG2DataContent->DataLength), (pXferBuf+PayloadHeader->bLength), dValidDataLength);
			m_pMPEG2DataContent->DataLength += dValidDataLength;
		}
		else if (!m_pMPEG2DataContent)
			DBGU_WARNING("No available buffer to store ISO packet data!!\n");
	}
	return ntStatus;
}

//2010/7/12 05:22と
VOID ProcessBuffer(PVOID Context)
{
	/*PProcessBuf workItem = (PProcessBuf)Context;
	CVideoDevice *pVideoDevice = workItem->pVideoDevice;
  
	DBGU_TRACE("ProcessBuffer workItem->XferLen = %d\n",workItem->XferLen);
	pVideoDevice->ParseTransferBuffer(workItem->pXferBuf, 
									workItem->XferLen, 
									workItem->StreamNumber);
	ExFreePoolWithTag(workItem, 'FREE');*/
}

UInt32 CVideoDevice::SetASICShellSize(UINT iWidth, UINT iHeight,UINT uiTargetFrameRate, int iZoom, int iHPaneStep, int iVPaneStep, bool zoom, BOOL AbsValueFlag)
{	
	//2010/12/16 02:09と
	//int iBufHSize = ImgWidList[isUXGA] + 32;//16;
	//2010/10/21 04:07と
	//int iBufVSize = ImgHgtList[isUXGA] +16;	
	//int iBufVSize = ImgHgtList[isUXGA]*5/3 +16;	// shawn 2010/09/16 modify for ZoomIn when resolution is bigger than VGA

	int iBufHSize = ImgWidList[is3M] +16;		// shawn 2011/07/12 for 3M
	int iBufVSize = ImgHgtList[is3M]*5/3 +16;	// shawn 2011/07/12 for 3M

	UInt32 iZoomInvalid = 0;
	
	int iASICHSize;
	int iASICVSize;
	int iASICHiStart = 0;
	int iASICViStart = 0;
	int iASICSubSample = 0;
	int bIsJPG = 0;

	DBGU_TRACE("SetASICShellSize iWidth = %d, iHeight = %d\n",iWidth , iHeight);

	// shawn 2010/03/09 fix crash bug when setting shell related properties in property page and not in preview state
	if (iWidth == 0 || iHeight == 0)
		return 0;
		
	if((m_ImageFormat[STREAM_Capture] == IMG_FMT_MJPEG)||(!m_fUSBHighSpeed && m_ImageFormat[STREAM_Capture] != IMG_FMT_YUY2))
		bIsJPG = 1;

	//20070625
	/*if(deviceContext->m_SXGA2_Still_Format == IMG_FMT_MJPEG)
	{
		if((deviceContext->m_MJPEG2RGB24)||(deviceContext->m_MJPEG2YUY2))
			bIsJPG = 1;
	}
	else
	{
		if(((deviceContext->m_MJPEG2RGB24)||(deviceContext->m_MJPEG2YUY2))&&(deviceContext->m_StillProcessState != CURRENTFRAMEISSTILL))
			bIsJPG = 1;
	}*/
		
	UINT	iHeight_limit;
		
	/*if(deviceContext->m_AddFormat == TRUE)
		iHeight_limit = 960;
	else*/
		iHeight_limit = 480;

	// If current format is MJPG, we do not use zoom/scaling functions.
	// Thus, output size is the same with input size (ymwu, 2007/4/16)
	if(m_ImageFormat[STREAM_Capture] == IMG_FMT_MJPEG)
	{			
		iASICHSize = iWidth;//ImgWidList[eImageOut];
		iASICVSize = iHeight;//ImgHgtList[eImageOut];
	}
	else if(iHeight > iHeight_limit)
	{
		iASICHSize = iWidth<<(1-bIsJPG);//ImgWidList[eImageOut]<<(1-bIsJPG);
		iASICVSize = iHeight;//ImgHgtList[eImageOut];
	}
	else
	{
		iASICHSize = 1280>>bIsJPG;
		iASICVSize = 480;
	}
	
	//The following caculations are based on the scaling method is x pixel->128
	//mPCamParam.eImageIn  = isVGA;
	//mPCamParam.eImageOut = eImageOut;

	int iStrmHSize = iASICHSize;
	int iStrmVSize = iASICVSize;
	int iHoSize = iWidth;//ImgWidList[eImageOut];
	int iVoSize = iHeight;//ImgHgtList[eImageOut];
	//      S/W Subsample   SrcBlkSize
	//SXGA      0               64
	//XGA       0               80
	//SVGA      0               102
	//VGA       0               128
	//CIF       1               106
	//QVGA      1               128
	//QCIF      2               106
	//QQVGA     2               128
	int iMaxZoom = 0;
	int iMaxSrcBlkSize = 0;

	//2010/10/21 04:09と
	// shawn 2010/09/16 modify for CIF view angle +++++
	//int iSWSubSample = 0;
	int a32SWSubSample[2];
	int	a32SrcBlkSize[2];
	int iHSrcBlkSize = 0;
	int iVSrcBlkSize = 0;		
	
	// If current format is MJPG, we do not use zoom/scaling functions. (ymwu, 2007/4/16)
	if(m_ImageFormat[STREAM_Capture] == IMG_FMT_MJPEG)
	{
		//2010/10/21 04:11と			
		//iSWSubSample = 0;
		a32SWSubSample[0]	= 0;
		a32SWSubSample[1]	= 0;
		//2010/10/27 04:24と
		iMaxSrcBlkSize = iHSrcBlkSize = iVSrcBlkSize = 128;			
		//iMaxSrcBlkSize = 128;
		m_ZoomParam.iMaxZoom = iMaxZoom = 0;
	}
	else if(iHeight > iHeight_limit)
	{
		// shawn 2010/09/16 modify for CIF view angle +++++
		//iSWSubSample = 0;
		a32SWSubSample[0]	= 0;
		a32SWSubSample[1]	= 0;			
		//2010/10/27 04:24と
		iMaxSrcBlkSize = iHSrcBlkSize = iVSrcBlkSize = 128;		
		//iMaxSrcBlkSize = 128;
		// shawn 2010/09/16 modify for ZoomIn when resolution is bigger than VGA +++++
		//m_ZoomParam.iMaxZoom = iMaxZoom = 0;
		m_ZoomParam.iMaxZoom = iMaxZoom = iMaxSrcBlkSize -42;
	}
	else
	{
		//2010/10/21 04:09と
		iHSrcBlkSize = 81920/iHoSize;
		iVSrcBlkSize = 61440/iVoSize;
		iMaxSrcBlkSize = (iHSrcBlkSize>iVSrcBlkSize)?iVSrcBlkSize:iHSrcBlkSize;
		//2010/10/21 04:13と
		/*iSWSubSample = 0;

		while((iMaxSrcBlkSize-1)>>7)
		{
			iMaxSrcBlkSize>>=1;
			iSWSubSample++;
		}*/

		//test 2010/10/25 02:46と
		m_CIFViewAngle = 0;

		if (m_CIFViewAngle && ((iWidth == 352 && iHeight == 288) || (iWidth == 176 && iHeight == 144)))
		{
			a32SWSubSample[0]	= 0;

			while((iHSrcBlkSize-1)>>7)
			{
				iHSrcBlkSize>>=1;
				a32SWSubSample[0]++;
			}
				
			a32SWSubSample[1]	= 0;

			while((iVSrcBlkSize-1)>>7)
			{
				iVSrcBlkSize>>=1;
				a32SWSubSample[1]++;
			}

			iHSrcBlkSize = iHSrcBlkSize - m_HSclOffset;
		}
		else
		{
			a32SWSubSample[0]	= 0;
			a32SWSubSample[1]	= 0;

			while((iMaxSrcBlkSize-1)>>7)
			{
				iMaxSrcBlkSize>>=1;
				a32SWSubSample[0]++;
				a32SWSubSample[1]++;
			}
				
			iHSrcBlkSize	= iMaxSrcBlkSize;
			iVSrcBlkSize	= iMaxSrcBlkSize;
		}			

		//vivi 2005/11/14 3X zoom
		//deviceContext->m_ZoomParam.iMaxZoom = iMaxZoom = (eImageOut > isVGA)?0:iMaxSrcBlkSize -64;
		m_ZoomParam.iMaxZoom = iMaxZoom = iMaxSrcBlkSize -42;
	}

	int iHPaneIdx, iVPaneIdx;
	
	//if ((iZoomStep==0) && (iHPaneStep==0) && (iVPaneStep==0))
	//{
	//	deviceContext->m_ZoomParam.iZoom  = iZoom = 0;//mPCamParam.iZoom  = iZoom = 0;
	//	deviceContext->m_ZoomParam.iHPaneIdx = iHPaneIdx = 0;//mPCamParam.iHPaneIdx = iHPaneIdx = 0;
	//	deviceContext->m_ZoomParam.iVPaneIdx = iVPaneIdx = 0;//mPCamParam.iVPaneIdx = iVPaneIdx = 0;
	//}
	//else
	{
		int iNewZoom = 	iZoom;//deviceContext->m_ZoomParam.iZoom +iZoomStep;//int iNewZoom = mPCamParam.iZoom +iZoomStep;
		//deviceContext->m_ZoomParam.iZoom = iZoom;//mPCamParam.iZoom = iZoom = iNewZoom;
		
		if (iNewZoom >= iMaxZoom) 
		{
			iZoom = iMaxZoom;//iZoom = mPCamParam.iZoom;
			iZoomInvalid |= 0x20;
		}
		
		if(iNewZoom <= 0)
		{
			iZoom = 0;//iZoom = mPCamParam.iZoom;
			iZoomInvalid |= 0x10;
		}

		if(AbsValueFlag)
		{
			iHPaneIdx = m_ZoomParam.iHPaneIdx = iHPaneStep;
			iVPaneIdx = m_ZoomParam.iVPaneIdx = iVPaneStep;
		}
		else
		{
			iHPaneIdx = m_ZoomParam.iHPaneIdx +iHPaneStep;//iHPaneIdx = mPCamParam.iHPaneIdx +iHPaneStep;
			iVPaneIdx = m_ZoomParam.iVPaneIdx +iVPaneStep;//iVPaneIdx = mPCamParam.iVPaneIdx +iVPaneStep;
		}
	}

	// shawn 2010/09/16 modify for CIF view angle +++++
	//int iSrcBlkSize = iMaxSrcBlkSize - iZoom;
	a32SrcBlkSize[0] = iHSrcBlkSize - iZoom;	
	a32SrcBlkSize[1] = iVSrcBlkSize - iZoom;

	//int iHtmp = (iHoSize<<iSWSubSample) *iSrcBlkSize;
	//int iVtmp = (iVoSize<<iSWSubSample) *iSrcBlkSize;
	int iHtmp = (iHoSize << a32SWSubSample[0]) * a32SrcBlkSize[0];
	int iVtmp = (iVoSize << a32SWSubSample[1]) * a32SrcBlkSize[1];
	// shawn 2010/09/16 modify for CIF view angle -----

	int iHiSize = iHtmp/iDstBlkSize + ((iHtmp%iDstBlkSize)? 1: 0);
	int iViSize = iVtmp/iDstBlkSize + ((iVtmp%iDstBlkSize)? 1: 0);

	int iHPaneIdxRange = (iStrmHSize>>(2-bIsJPG)) - ((iHiSize+1) >>1);
	int iVPaneIdxRange = (iStrmVSize>>1) - ((iViSize+1) >>1);
	
	iHPaneIdxRange = ((iHPaneIdxRange >>1) <<1);//vivi
	iVPaneIdxRange = ((iVPaneIdxRange >>1) <<1);//vivi

	//vivi 2005/12/6 get the pan/tilt absolute position
	m_ZoomParam.iHPaneIdxRange = iHPaneIdxRange;
	m_ZoomParam.iVPaneIdxRange = iVPaneIdxRange;

	if (iHPaneIdx <= -iHPaneIdxRange)
	{
		iHPaneIdx = -iHPaneIdxRange;
		iZoomInvalid |= 0x02;
	}
	
	if (iHPaneIdx >= iHPaneIdxRange)
	{
		iHPaneIdx = iHPaneIdxRange;
		iZoomInvalid |= 0x1;
	}
	
	m_ZoomParam.iHPaneIdx = iHPaneIdx;//mPCamParam.iHPaneIdx = iHPaneIdx;

	if (iVPaneIdx <= -iVPaneIdxRange)
	{
		iVPaneIdx = -iVPaneIdxRange;
		iZoomInvalid |= 0x08;
	}
	
	if (iVPaneIdx >= iVPaneIdxRange)
	{
		iVPaneIdx = iVPaneIdxRange;
		iZoomInvalid |= 0x04;
	}
	
	m_ZoomParam.iVPaneIdx = iVPaneIdx;//mPCamParam.iVPaneIdx = iVPaneIdx;

	int iHiStart = iStrmHSize/(4>>bIsJPG) +iHPaneIdx -(iHiSize+1)/2;
	int iViStart = iStrmVSize/2 +iVPaneIdx -(iViSize+1)/2;
	
	iHiStart = ((iHiStart>>1) <<1);
	iViStart = ((iViStart>>1) <<1);

	//2010/10/21 04:05と
	//CShell_SetSize(m_pCShell, iStrmHSize, iStrmVSize, iBufHSize, iBufVSize,
	//	iHiStart, iViStart, iHiSize, iViSize, iHoSize,  iVoSize, iSWSubSample, iSrcBlkSize, zoom);
	CShell_SetSize(m_pCShell, iStrmHSize, iStrmVSize, iBufHSize, iBufVSize,
		iHiStart, iViStart, iHiSize, iViSize, iHoSize,  iVoSize, a32SWSubSample, a32SrcBlkSize, zoom);

	return iZoomInvalid;
}

NTSTATUS CVideoDevice::CheckCapturePinBandwidth(KS_VIDEOINFOHEADER &VideoInfoHeader)
{
    NTSTATUS ntStatus=STATUS_SUCCESS;

	// james try M2TS.
	if(&VideoInfoHeader == NULL)
	{
		DBGU_TRACE("M2TS CapturePinProbeCommit\n");
		m_ImageFormat[m_pDevExt->m_STREAM_Capture_MP2TS_Idx] = IMG_FMT_H264;	// shawn 2011/06/27 add

		ntStatus = CapturePinProbeCommit(
						0, //m_pDevExt->m_FrameRateInfo.CurrentActualFrameRate,
						1, //m_pDevExt->StreamFormatArray[i].bFormatIndex,
						0, //m_pDevExt->StreamFormatArray[i].bFrameIndex
						m_pDevExt->m_STREAM_Capture_MP2TS_Idx);//2010/6/22 05:23と H264	// shawn 2011/06/23 modify

		return ntStatus;
	}

	int i;

	ULONG X=VideoInfoHeader.bmiHeader.biWidth;
	ULONG Y=VideoInfoHeader.bmiHeader.biHeight;

#ifdef _VGA_OUTPUT_
	///////////////////////////////////////////////////////////////
	// Request VGA format output even user request format < VGA  //
	///////////////////////////////////////////////////////////////
	//2010/8/25 11:25と
	//if((X<640 || Y<480)&&(VideoInfoHeader.bmiHeader.biCompression != MEDIASUBTYPE_MJPG.Data1))	// preview is lower than VGA
	//2011/1/7 02:25と
	if((X<640 || Y<480)&&(VideoInfoHeader.bmiHeader.biCompression != MEDIASUBTYPE_MJPG.Data1)&&(m_fUSBHighSpeed))	// shawn 2011/08/01 modify
	//&&(VideoInfoHeader.bmiHeader.biCompression != 0x32595559))	// preview is lower than VGA
	{
		X=640;
		Y=480;
	}
#endif

	//
	// Find format index and frame index
	//
	for (i=0;i<MAX_STREAM_FORMAT;i++)
	{
		if (X == m_pDevExt->StreamFormatArray[i].wWidth &&
			Y == m_pDevExt->StreamFormatArray[i].wHeight &&
			VideoInfoHeader.bmiHeader.biCompression == m_pDevExt->StreamFormatArray[i].biCompression)
		{
			DBGU_TRACE("Find FormatIndex=%d and FrameIndex=%d\n",
				m_pDevExt->StreamFormatArray[i].bFormatIndex,
				m_pDevExt->StreamFormatArray[i].bFrameIndex);

			m_pDevExt->m_dwFormatIndex[STREAM_Capture] = i;

			// fix AvgTimePerFrame to lower than MaxAvailableFrameRate
			m_pDevExt->m_FrameRateInfo.CurrentActualFrameRate = VideoInfoHeader.AvgTimePerFrame;

			// shawn 2011/11/15 recover +++++
			//2010/10/13 10:49と
			if (VideoInfoHeader.AvgTimePerFrame < m_pDevExt->m_FrameRateInfo.CurrentMaxAvailableFrameRate)
			{
				DBGU_WARNING("User sets frame rate larger than Max Available FrameRate!!\n");
				m_pDevExt->m_FrameRateInfo.CurrentActualFrameRate = m_pDevExt->m_FrameRateInfo.CurrentMaxAvailableFrameRate;
				DBGU_WARNING("User wish FPS=%d\n",10000000/VideoInfoHeader.AvgTimePerFrame);
				DBGU_WARNING("Current Max FPS=%d\n",10000000/m_pDevExt->m_FrameRateInfo.CurrentMaxAvailableFrameRate);
			}
			// shawn 2011/11/15 recover -----

			ntStatus = CapturePinProbeCommit(
				m_pDevExt->m_FrameRateInfo.CurrentActualFrameRate,
				m_pDevExt->StreamFormatArray[i].bFormatIndex,
				m_pDevExt->StreamFormatArray[i].bFrameIndex,
				STREAM_Capture);//2010/6/22 05:23と H264

			if (NT_SUCCESS(ntStatus))
			{
				DBGU_TRACE("x=%d,y=%d",VideoInfoHeader.bmiHeader.biWidth,VideoInfoHeader.bmiHeader.biHeight);
				DBGU_TRACE("%d : %d\n",m_dwMaxVideoFrameSize, VideoInfoHeader.bmiHeader.biSizeImage);
				
				if (m_dwMaxVideoFrameSize >= m_pDevExt->StreamFormatArray[i].ImageSize)
				{
					m_dwCurVideoFrameSize = m_pDevExt->StreamFormatArray[i].ImageSize;
					DBGU_TRACE("m_dwCurVideoFrameSize = %d\n", m_dwCurVideoFrameSize);
					m_ImageFormat[STREAM_Capture] = m_pDevExt->StreamFormatArray[i].ImageFormat;
					DBGU_TRACE("m_ImageFormat[STREAM_Capture] = %d\n", m_ImageFormat[STREAM_Capture]);
					break;
				}
			}
			else
			{
				DBGU_WARNING("CapturePinProbeCommit return 0x%X\n",ntStatus);
			}
		}
	}

	if (i == MAX_STREAM_FORMAT)
	{
		DBGU_WARNING("No supported format Matched !!\n");
		m_pDevExt->m_dwFormatIndex[STREAM_Capture] = 0;
		return STATUS_INVALID_PARAMETER;
	}

	DBGU_TRACE("CheckCapturePinBandwidth return %X\n", ntStatus);
	return ntStatus;
}

NTSTATUS CVideoDevice::CheckStillPinBandwidth(KS_VIDEOINFOHEADER &VideoInfoHeader)
{
    NTSTATUS ntStatus=STATUS_SUCCESS;

	int i;
	ULONG X=VideoInfoHeader.bmiHeader.biWidth;
	ULONG Y=VideoInfoHeader.bmiHeader.biHeight;
	DWORD biCompression = VideoInfoHeader.bmiHeader.biCompression;
	ULONG biSizeImage = VideoInfoHeader.bmiHeader.biSizeImage;

	DBGU_TRACE("CheckStillPinBandwidth Find biCompression=%d, SizeImage=%d\n",
				biCompression,biSizeImage);

#ifdef _VGA_OUTPUT_
	///////////////////////////////////////////////////////////////
	// Request VGA format output even user request format < VGA  //
	///////////////////////////////////////////////////////////////
	if ((X<640 || Y<480)&&(VideoInfoHeader.bmiHeader.biCompression != MEDIASUBTYPE_MJPG.Data1)&&(m_fUSBHighSpeed))	// preview is lower than VGA	// shawn 2011/08/01 modify
	{
		X=640;
		Y=480;
	}
#endif

	//
	// Find format index and frame index
	//
	for (i=0;i<MAX_STREAM_FORMAT;i++)
	{
		DBGU_TRACE("(%d) X= %d : %d\n",i, X, m_pDevExt->StillFormatArray[i].wWidth);
		DBGU_TRACE("(%d) Y= %d : %d\n",i, Y, m_pDevExt->StillFormatArray[i].wHeight);
		
		if (X == m_pDevExt->StillFormatArray[i].wWidth &&
			Y == m_pDevExt->StillFormatArray[i].wHeight &&
			VideoInfoHeader.bmiHeader.biCompression == m_pDevExt->StillFormatArray[i].biCompression)	// shawn 2011/08/11 modify for fixing method 2 snapshot bug
		{
			DBGU_TRACE("Find FormatIndex=%d and FrameIndex=%d\n",
				m_pDevExt->StillFormatArray[i].bFormatIndex,
				m_pDevExt->StillFormatArray[i].bFrameIndex);

			m_pDevExt->m_dwFormatIndex[m_pDevExt->m_STREAM_Still_Idx] = i;	// shawn 2011/06/23 modify

			ntStatus = StillPinProbeCommit(
				m_pDevExt->StillFormatArray[i].bFormatIndex,
				m_pDevExt->StillFormatArray[i].bFrameIndex
				);

			if (NT_SUCCESS(ntStatus))
			{
				if (m_dwMaxVideoFrameSize >= VideoInfoHeader.bmiHeader.biSizeImage)
				{
					m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] = m_pDevExt->StillFormatArray[i].ImageFormat;	// shawn 2011/06/23 modify
					DBGU_TRACE("Still Format=%d",m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx]);	// shawn 2011/06/23 modify
					DBGU_TRACE("x=%d,y=%d",X,Y);
					DBGU_TRACE("%d : %d\n",m_dwMaxVideoFrameSize, VideoInfoHeader.bmiHeader.biSizeImage);
					break;
				}
				else if (m_pDevExt->StillFormatArray[i].ImageFormat == IMG_FMT_MJPEG)	// no checking image size for MJPEG
				{
					m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] = IMG_FMT_MJPEG;	// shawn 2011/06/23 modify
					DBGU_TRACE("Still Format=%d",m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx]);	// shawn 2011/06/23 modify
					DBGU_TRACE("x=%d,y=%d",X,Y);
					DBGU_TRACE("%d : %d\n",m_dwMaxVideoFrameSize, VideoInfoHeader.bmiHeader.biSizeImage);
					break;
				}
			}
			else
			{
				DBGU_WARNING("StillPinProbeCommit return 0x%X\n",ntStatus);
			}
		}
	}

	if (i == MAX_STREAM_FORMAT)
	{
		DBGU_WARNING("No supported still format Matched !!\n");
		m_pDevExt->m_dwFormatIndex[m_pDevExt->m_STREAM_Still_Idx] = 0;	// shawn 2011/06/23 modify
		return STATUS_INVALID_PARAMETER;
	}

	for (i = (int)isQQVGA; i < (int)isCount; ++i) {
		if (ImgWidList[i] == VideoInfoHeader.bmiHeader.biWidth &&
			ImgHgtList[i] == VideoInfoHeader.bmiHeader.biHeight) {
			break;
		}
	}

	m_enImageOutStill = (ImageSize)i;
	
	if (m_enImageOutStill >= isCount) {
        DBGU_ERROR("Invalid output format\n");
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	DBGU_TRACE("CheckStillPinBandwidth return %X\n", ntStatus);
	return ntStatus;
}

NTSTATUS CVideoDevice::Run(KS_VIDEOINFOHEADER &VideoInfoHeader, PKSPIN pin)
{
	DBGU_TRACE("CVideoDevice::Run()\n");
	NTSTATUS ntStatus = STATUS_SUCCESS;
	//2010/11/4 10:32と
	USBD_PIPE_INFORMATION PipeInfo, PipeInfo2;//PipeInfo;
	int i;
	BYTE iAltSettingIndex = 0;	// james try M2TS.
	
	//2010/6/18 03:48と
	ULONG StreamNumber = pin->Id;
	
	RtlZeroMemory(&PipeInfo,sizeof(USBD_PIPE_INFORMATION));
	RtlZeroMemory(&PipeInfo2,sizeof(USBD_PIPE_INFORMATION));
	
	if (!m_pCShell)
		m_pCShell = CShell_CShell();

	if (m_pShellBufferPtr)
		delete m_pShellBufferPtr;

	//RBK move huge data buffer to PagedPool
	if (m_fUSBHighSpeed)
	{
		m_pShellBufferPtr = new (PagedPool) UCHAR[/*1600*1200*3*/2048*1536*3];	// shawn 2011/07/12 modify for 3M
	}
	else
	{
		m_pShellBufferPtr = new (PagedPool) UCHAR[640*480*3];
	}

	if (!m_pShellBufferPtr || !m_pCShell)
	{
		DBGU_TRACE("Exit CVideoDevice::Run() return STATUS_INSUFFICIENT_RESOURCES\n");
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	//Stop interrupt transfer because user may change the alternate settings.
	ntStatus = PipeAbortOrReset(SELECTED_CTRL_INTERFACE_INDEX,INT_PIPE_INDEX,TRUE);

	if (!NT_SUCCESS(ntStatus))
	{
		DBGU_TRACE("PipeAbortOrReset return status = %X\n",ntStatus);
		return ntStatus;
	}


	//
	// Allocate Bandwaidth
	//
	
	// 2010/6/18 03:51と
	iAltSettingIndex = (StreamNumber == STREAM_Capture) ? SELECTED_DATA_INTERFACE_INDEX : SELECTED_MPEG2_INTERFACE_INDEX;

	//UINT AlternateSettingCount = GetNoOfAlternateSetting(SELECTED_DATA_INTERFACE_INDEX);
	UINT AlternateSettingCount = GetNoOfAlternateSetting(iAltSettingIndex);	// james try M2TS.

	//2010/10/13 11:06と two pins
	TwoPipeEnable = FALSE;

	// shawn 2011/08/01 +++++
	m_dwMaxPayloadTransferSize = m_dwMaxPayloadTransferSizeTmp;
	m_dwMaxPayloadTransferSizeTmp = 0;
	// shawn 2011/08/01 -----
	
	if(m_dwMaxPayloadTransferSize > 3072 && BeST5R286)
	{
		TwoPipeEnable = TRUE;
	
		//2010/11/22 02:59と
		//RBK move huge data buffer to PagedPool
		if(S420Buffer == NULL)
			S420Buffer = (PUCHAR) ExAllocatePoolWithTag(PagedPool,/*1600*1200*3/2*/2048*1536*3/2, '11US');	// shawn 2011/07/12 modify for 3M

		//2011/2/25 02:52と
		m_IncorrectLengthFrameCount = 0;
		m_IncorrectPTSCount = 0;	
		DBGU_TRACE("BeST5R286 3\n");
	}	
	
	//2011/4/26 05:13と
	//2011/3/7 05:01と
	if(StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
	{
		for (m_AlternateSettingNo=1; m_AlternateSettingNo < AlternateSettingCount; m_AlternateSettingNo++)
		//for (m_AlternateSettingNo = AlternateSettingCount -1; m_AlternateSettingNo > 0 ; m_AlternateSettingNo--)
		{
			if ((M2TS_BusBWArray[m_AlternateSettingNo].MaxPktSize*M2TS_BusBWArray[m_AlternateSettingNo].NumberOfTransaction)
				>= m_dwMaxPayloadTransferSize_MP2TS)	// shawn 2011/07/20 modify
			{
				m_AlternateSettingNo = M2TS_BusBWArray[m_AlternateSettingNo].AlternateSetting;
				//ntStatus = SetAlternate(m_AlternateSettingNo,SELECTED_DATA_INTERFACE_INDEX);
					ntStatus = SetAlternate(m_AlternateSettingNo, iAltSettingIndex);	// james try M2TS.
				//else
				//	ntStatus = SetAlternate(1, iAltSettingIndex);	// james try M2TS.
			
				if(NT_SUCCESS(ntStatus) && (m_AlternateSettingNo > 0))
				{
					DBGU_TRACE("!! SetAlternate ISO Transfer (%d) OK!!\n",m_AlternateSettingNo);
					break;
				}
				else
				{
					DBGU_TRACE("Fail to set USB Bandwidth : %d !\n",m_AlternateSettingNo);
				}
			}
		}

		if(m_AlternateSettingNo >= AlternateSettingCount)
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			DBGU_WARNING("Fail to set USB Bandwidth.\n ===> Video Stops!\n");
			goto _Exit_RUN;
		}
	}
	else
	{	
	//2010/11/8 05:03と
		// Select USB Bandwidth from Min
		for (m_AlternateSettingNo=1; m_AlternateSettingNo < AlternateSettingCount; m_AlternateSettingNo++)
		//for (m_AlternateSettingNo = AlternateSettingCount -1; m_AlternateSettingNo > 0 ; m_AlternateSettingNo--)
		{
			if ((BusBWArray[m_AlternateSettingNo].MaxPktSize*BusBWArray[m_AlternateSettingNo].NumberOfTransaction)
				>= m_dwMaxPayloadTransferSize)
			{
				m_AlternateSettingNo = BusBWArray[m_AlternateSettingNo].AlternateSetting;
				//ntStatus = SetAlternate(m_AlternateSettingNo,SELECTED_DATA_INTERFACE_INDEX);
					ntStatus = SetAlternate(m_AlternateSettingNo, iAltSettingIndex);	// james try M2TS.
				//else
				//	ntStatus = SetAlternate(1, iAltSettingIndex);	// james try M2TS.
			
				if(NT_SUCCESS(ntStatus) && (m_AlternateSettingNo > 0))
				{
					DBGU_TRACE("!! SetAlternate ISO Transfer (%d) OK!!\n",m_AlternateSettingNo);
					break;
				}
				else
				{
					DBGU_TRACE("Fail to set USB Bandwidth : %d !\n",m_AlternateSettingNo);
				}
			}
		}
		if (this->m_PipeMode == USB_ENDPOINT_TYPE_ISOCHRONOUS)
		{			
		        if(m_AlternateSettingNo >= AlternateSettingCount)
	        	{
			        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			        DBGU_WARNING("Fail to set USB Bandwidth.\n ===> Video Stops!\n");
			        goto _Exit_RUN;
                        }
		}
		else //RBK bulkmode
		{
			ntStatus = SetAlternate(0, 1);	
		}		
	}
		
	DBGU_TRACE("set USB Bandwidth : %d/(max=%d) !\n",m_AlternateSettingNo,AlternateSettingCount-1);

	if(&VideoInfoHeader != NULL)	// james try M2TS.
	{
		ImageSize enImageOut;
		int iZoom = 0;

		for (i = (int)isQQVGA; i < (int)isCount; ++i) {
			if (ImgWidList[i] == VideoInfoHeader.bmiHeader.biWidth &&
				ImgHgtList[i] == VideoInfoHeader.bmiHeader.biHeight) {
				break;
			}
		}

		enImageOut = (ImageSize)i;
		
		if (enImageOut >= isCount) {
			DBGU_ERROR("Invalid output format\n");
			ntStatus = STATUS_UNSUCCESSFUL;
			goto _Exit_RUN;
		}
		
		m_enImageOut = enImageOut;

		CShell_Reset(m_pCShell);
		//SetASICShellSize(m_ZoomParam.iZoomStep, 0, 0, FALSE, FALSE);
		SetASICShellSize(ImgWidList[m_enImageOut], ImgHgtList[m_enImageOut],0, 
						m_ZoomParam.iZoomStep, 
						0, 
						0,
						0,
						0);
	}

	//2010/6/21 05:26と H264
	if(StreamNumber == STREAM_Capture)
	{
		for(i=0;i<NUM_OF_DATA_BUFFER;i++)
			m_DataBufObj[i].State = NO_DATA;

		if(TwoPipeEnable)
			for(i=0;i<NUM_OF_DATA_BUFFER;i++)
				m_MPEG2DataBufObj[i].State = NO_DATA;			
	}
	else if(StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
	{
		if(TSMuxEnable)
		{
			for(i=0;i<NUM_OF_DATA_BUFFER;i++)
				m_MPEG2DataBufObj[i].State = NO_DATA;
		}
	}		
	
	//2010/6/21 05:15と H264		
	//m_pDataContent = GetDataBuffer(DATAOP_WRITE);
		
	if(StreamNumber == STREAM_Capture)
	{
		m_pDataContent = GetDataBuffer(DATAOP_WRITE, 0);
		m_pDataContent->DataLength = 0;
		
		//2010/10/13 11:28と
		if(TwoPipeEnable)
		{
			m_pMPEG2DataContent = GetDataBuffer(DATAOP_WRITE, 1);
			m_pMPEG2DataContent->DataLength = 0;
		}		
	}
	else if(StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
	{
		LARGE_INTEGER TmpTime;
		
		//mux 2011/4/1 06:22と
		if(TSMuxEnable)
		{
			//2011/4/1 06:15と
			StartMuxThread();
			InitGlobalVar();
			FrameSN = 0;
			//m_Time = PcGetTimeInterval(0);
			//m_pMPEG2DataContent = &m_MPEG2DataBufObj[0].Content;
			m_pMPEG2DataContent = GetDataBuffer(DATAOP_WRITE, 1);
			m_pMPEG2DataContent->DataLength = 0;
			m_byTmpTSCounter = 0;	// shawn 2011/08/04 add
			m_byTmpTSCounter2 = 0;	// shawn 2011/08/04 add
		}
		
		//2010/7/14 02:23と skype
		m_MPEG2IsoBufferAddr = 0;
		m_MPEG2ReadBufferAddr = 0;
		m_MPEG2ValidDataLength = 0;		
		m_FirstFrame = TRUE;
		//2011/4/20 11:46と
		KeQuerySystemTime(&TmpTime);
		m_Time = TmpTime.QuadPart;
		DBGU_TRACE("Run m_Time =%d \n",m_Time);
	}
	
	// Restart interrupt transfer	
	/*ntStatus = StartInterruptTransfer();
	
	if(!NT_SUCCESS(ntStatus))
		DBGU_TRACE("Fail to restart interrupt transfer !\n");
	*/
	// start iso transfer
	//PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX,ISO_PIPE_INDEX,FALSE);
	//ntStatus = GetPipeInfo(SELECTED_DATA_INTERFACE_INDEX,ISO_PIPE_INDEX,&PipeInfo);
	PipeAbortOrReset(iAltSettingIndex, ISO_PIPE_INDEX,FALSE);	// james try M2TS.
	
	//2010/10/13 04:12と
	if(TwoPipeEnable)		
		PipeAbortOrReset(iAltSettingIndex, 1,FALSE);	
			
	ntStatus = GetPipeInfo(iAltSettingIndex, ISO_PIPE_INDEX, &PipeInfo);
	
	if (NT_SUCCESS(ntStatus))
	{
		for(i=0;i<MAX_IRP;i++){
			
			//2010/6/21 05:59と H264
			if(StreamNumber == STREAM_Capture)
			{
				if (this->m_PipeMode == USB_ENDPOINT_TYPE_ISOCHRONOUS) //RBK ISOMODE
					ntStatus = UsbIsochronousTransfer(	m_IsoTransferObj[i].pIrp,
													m_IsoTransferObj[i].pUrb,
													&PipeInfo,
													TRUE,
													m_IsoTransferObj[i].pUsbBuffer,
													NULL,
													(PVOID)IsoCompleteRoutine,
													(PVOID) &m_IsoTransferObj[i]);
				else //RBK BULKMODE
					ntStatus = UsbBulkOrInterruptTransfer(m_BulkTransferObj[i].pIrp,
													m_BulkTransferObj[i].pUrb,
													&PipeInfo,
													TRUE,
													m_BulkTransferObj[i].pUsbBuffer,
													NULL,
													m_BulkTransferObj[i].usUsbBufferLength,
													(PVOID)BulkCompleteRoutine,
													(PVOID) &m_BulkTransferObj[i]);	
			}
			else if(StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
				ntStatus = UsbIsochronousTransfer(	m_MPEG2IsoTransferObj[i].pIrp,
												m_MPEG2IsoTransferObj[i].pUrb,
												&PipeInfo,
												TRUE,
												m_MPEG2IsoTransferObj[i].pUsbBuffer,
												NULL,
												(PVOID)IsoCompleteRoutine,
												(PVOID) &m_MPEG2IsoTransferObj[i]);
			
			if(!NT_SUCCESS(ntStatus))
			{
				DBGU_TRACE("Fail to setup Iso transfer(%d) !\n",i);
				break;
			}
			else
			{
				DBGU_TRACE("Iso Transfer OK ! StreamNumber=%d\n", StreamNumber);
			}

			//2010/6/21 05:59と H264
			if(StreamNumber == STREAM_Capture)		
				InterlockedIncrement(&m_UsbIoCount);
			else if(StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify		
				InterlockedIncrement(&m_MPEG2UsbIoCount);
		}
	}
	else
	{
		DBGU_TRACE("Exit CVideoDevice::Run() status = %X (%x)\n",ntStatus);
		goto _Exit_RUN;
	}

	//2010/10/13 02:27と
	if(TwoPipeEnable)		
	{
		ntStatus = GetPipeInfo(iAltSettingIndex, 1, &PipeInfo2);
		//2010/11/4 02:58と
		DBGU_TRACE(" Pipes Information\n");
					DBGU_TRACE(" -----------------------------------\n");	
					DBGU_TRACE(" EndpointAddress: %x\n",PipeInfo2.EndpointAddress);     
					DBGU_TRACE(" PipeType: %x\n",PipeInfo2.PipeType);
					DBGU_TRACE(" PipeHandle: %x\n",PipeInfo2.PipeHandle);
					DBGU_TRACE(" MaximumPacketSize: %x\n",PipeInfo2.MaximumPacketSize);
		
		if (NT_SUCCESS(ntStatus))
		{
			for(i=0;i<MAX_IRP;i++){
				
				//2010/6/21 05:59と H264
				if(StreamNumber == STREAM_Capture)
				{
					ntStatus = UsbIsochronousTransfer(	m_MPEG2IsoTransferObj[i].pIrp,
													m_MPEG2IsoTransferObj[i].pUrb,
													&PipeInfo2,
													TRUE,
													m_MPEG2IsoTransferObj[i].pUsbBuffer,
													NULL,
													(PVOID)IsoCompleteRoutine,
													(PVOID) &m_MPEG2IsoTransferObj[i]);
				}

				if(!NT_SUCCESS(ntStatus))
				{
					DBGU_TRACE("2nd pipe Fail to setup Iso transfer(%d) !\n",i);
					break;
				}
				else
				{
					DBGU_TRACE("2nd pipe Iso Transfer OK ! StreamNumber=%d\n", StreamNumber);
				}
	
				//2010/6/21 05:59と H264
				InterlockedIncrement(&m_MPEG2UsbIoCount);
			}
		}
		else
		{
			DBGU_TRACE("Exit CVideoDevice::Run() status = %X (%x)\n",ntStatus);
			goto _Exit_RUN;
		}	
	}

	// shawn 2011/06/14 +++++
	if(m_ImageFormat[STREAM_Capture] == IMG_FMT_RGB24)
	{
		DBGU_TRACE("IMG_FMT_RGB24 m_pDevExt->m_Flip.CurrentValue ^ 1 \n");
		CShell_SetVMirror(m_pCShell, (m_pDevExt->m_Flip.CurrentValue ^ 1));
	}
	else
	{
		DBGU_TRACE("IMG_FMT_others m_pDevExt->m_Flip.CurrentValue  \n");
		CShell_SetVMirror(m_pCShell, m_pDevExt->m_Flip.CurrentValue);
	}
	// shawn 2011/06/14 -----

#ifndef _NOEFFECT
	if (m_EffectParam == 0xfe)
	{
		// shawn 2011/06/01 modify +++++
		if(m_FrameFileName != NULL)
		{
			if (!SetFrameFile(m_FrameFileName, 255, 1, &m_FrameTransIfo, m_pEffectBufferPtr))
			{
				DBGU_INFO("KSPROPERTY_CUSTOM_PROP_FRAME_FILENAME - SetFrameFile(%ws) is failed\n", m_FrameFileName);
				m_EffectParam = 0;
			}
		}
		// shawn 2011/06/01 modify -----
	}
#endif

	// shawn 2011/06/14 +++++
	if(m_pDevExt->m_DefectMode)
	{
		m_pCShell->Param.iDefectMode = m_pDevExt->m_DefectMode;
		m_pCShell->Param.iDefectYThd = m_pDevExt->m_iYThd;
		m_pCShell->Param.iDefectCThd = m_pDevExt->m_iCThd;	
	}

	if(m_pDevExt->m_DeNoiseMode.CurrentValue)
	{
		m_pCShell->Param.bTemporalNR = m_pDevExt->m_TemporalNR;
		m_pCShell->Param.bEdgeDtn = m_pDevExt->m_EdgeDtn;
		m_pCShell->Param.iTempStrgLumThd = m_pDevExt->m_StrgLThd;
		m_pCShell->Param.iTempStrgChrThd = m_pDevExt->m_StrgCThd;	
		m_pCShell->Param.iTempNzPwLumThd = m_pDevExt->m_NzPwrLThd;
		m_pCShell->Param.iTempNzPwChrThd = m_pDevExt->m_NzPwrCThd;
		m_pCShell->Param.iLowLightThd = m_pDevExt->m_LowLightThd;	
		m_pCShell->Param.iMeanDiffYThd = m_pDevExt->m_MeanDiffYThd;
		m_pCShell->Param.iMeanDiffCThd = m_pDevExt->m_MeanDiffCThd;			
	}
	// shawn 2011/06/14 -----

_Exit_RUN:

	if(NT_SUCCESS(ntStatus))
	{
		DBGU_TRACE("Change Stream State to KSSTATE_RUN\n");
		
		//2010/6/29 05:52と H264
		if(StreamNumber == STREAM_Capture)	
			m_pDevExt->m_StreamState = KSSTATE_RUN;
		else
			m_pDevExt->m_MPEG2StreamState = KSSTATE_RUN;
		
		KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);

		m_SaveSnapShotFileReady = FALSE;
		m_SaveSnapShotFile = FALSE;

		m_FrameErrorCount=0;
		m_SnapShotState = AP_IS_RUNNING;
		WriteSnapShotStateToReg();

		if(&VideoInfoHeader != NULL)	// james try M2TS.
		{
			//  keep and compare image format
			if ((m_ZoomParam.iPreWidth != VideoInfoHeader.bmiHeader.biWidth) || 
				(m_ZoomParam.iPreHeight != VideoInfoHeader.bmiHeader.biHeight) ||
				(m_ZoomParam.biCompression != VideoInfoHeader.bmiHeader.biCompression))
			{
				DBGU_TRACE("image format changed\n");
				m_ZoomParam.iPreWidth = VideoInfoHeader.bmiHeader.biWidth;
				m_ZoomParam.iPreHeight = VideoInfoHeader.bmiHeader.biHeight;
				m_ZoomParam.biCompression = VideoInfoHeader.bmiHeader.biCompression;
				m_ZoomParam.bZoomEnable = FALSE;
				m_ZoomParam.iZoomStep = 0;
				m_ZoomParam.iZoomStatus = 		SetASICShellSize(ImgWidList[m_enImageOut], ImgHgtList[m_enImageOut],0, 
						m_ZoomParam.iZoomStep, 
						0, 
						0,
						0,
						0);
				//SetASICShellSize(0, 0, 0, FALSE, FALSE);
			}
			
			// effect and zoom not support SVGA and upper resolution
			if ((VideoInfoHeader.bmiHeader.biWidth > 640) || (VideoInfoHeader.bmiHeader.biHeight > 480))
			{
				DBGU_TRACE("image format > VGA, disable Zomm function\n");
				m_ZoomParam.bZoomEnable = FALSE;
				m_ZoomParam.iZoomStep = 0;
				m_ZoomParam.iZoomStatus = SetASICShellSize(ImgWidList[m_enImageOut], ImgHgtList[m_enImageOut],0, 
						m_ZoomParam.iZoomStep, 
						0, 
						0,
						0,
						0);
				//SetASICShellSize(0, 0, 0, FALSE, FALSE);

				if ((m_EffectParam<17 || m_EffectParam>26) && m_EffectParam!=0xfe)
					m_EffectParam = 0;
			}
		}
	}

	DBGU_TRACE("Exit CVideoDevice::Run() status = %X\n",ntStatus);
	return ntStatus;
}


NTSTATUS CVideoDevice::Pause(ULONG StreamNumber)
{
	DBGU_TRACE("CVideoDevice::Pause()\n");
	NTSTATUS ntStatus = STATUS_SUCCESS;
	
	//2010/6/29 05:52と H264
	if(StreamNumber == STREAM_Capture)	
		m_pDevExt->m_StreamState = KSSTATE_PAUSE;
	else
		m_pDevExt->m_MPEG2StreamState = KSSTATE_PAUSE;

	//2010/6/22 02:55と H264
	if (m_UsbIoCount >= 1 && StreamNumber == STREAM_Capture) {
		// abort iso transfer
		//ntStatus = PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE_INDEX, TRUE);
		ntStatus = PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE_INDEX, TRUE);	// james try M2TS.
	}

	//2011/5/12 11:00と
	if (m_MPEG2UsbIoCount >= 1 && StreamNumber == STREAM_Capture && TwoPipeEnable) {
		// abort iso transfer
		ntStatus = PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE2_INDEX, TRUE);	// james try M2TS.
	}

	//2010/6/22 02:57と H264
	if (m_MPEG2UsbIoCount >= 1 && StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx) {	// shawn 2011/06/23 modify
		// abort iso transfer
		//ntStatus = PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE_INDEX, TRUE);
		ntStatus = PipeAbortOrReset(SELECTED_MPEG2_INTERFACE_INDEX, ISO_PIPE_INDEX, TRUE);	// james try M2TS.
	}
	
	if (ntStatus == STATUS_PENDING)
		ntStatus = STATUS_SUCCESS;

	return ntStatus;
}

NTSTATUS CVideoDevice::Stop(ULONG StreamNumber)
{
	DBGU_TRACE("CVideoDevice::Stop()\n");
	NTSTATUS ntStatus = STATUS_SUCCESS;

	//2010/6/29 05:52と H264
	if(StreamNumber == STREAM_Capture)	
		m_pDevExt->m_StreamState = KSSTATE_STOP;
	else
		m_pDevExt->m_MPEG2StreamState = KSSTATE_STOP;

	//
	// Stop all transfers
	//
	//2010/6/29 04:16と H264
	//ntStatus = StopInterruptPipe();
	
	//if ((!NT_SUCCESS(ntStatus)) && ntStatus!=STATUS_PENDING)	return ntStatus;
	
	//2010/6/22 02:55と H264
	if (m_UsbIoCount >= 1 && StreamNumber == STREAM_Capture) {
		// abort iso transfer

		//ntStatus = PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX,ISO_PIPE_INDEX,TRUE);
		ntStatus = PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE_INDEX,TRUE);	// james try M2TS.
	}

	if (m_MPEG2UsbIoCount >= 1 && StreamNumber == STREAM_Capture && TwoPipeEnable) {
		// abort iso transfer
		ntStatus = PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE2_INDEX, TRUE);	// james try M2TS.
	}
			
	if (m_MPEG2UsbIoCount >= 1  && StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx) {	// shawn 2011/06/23 modify
		// abort iso transfer

		//ntStatus = PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX,ISO_PIPE_INDEX,TRUE);
		ntStatus = PipeAbortOrReset(SELECTED_MPEG2_INTERFACE_INDEX, ISO_PIPE_INDEX,TRUE);	// james try M2TS.
	}

	if ((!NT_SUCCESS(ntStatus)) && ntStatus!=STATUS_PENDING)	return ntStatus;

	m_SnapShotState = AP_IS_NOT_RUNNING;
	WriteSnapShotStateToReg();

	// Free Usb Bandwidth
	//ntStatus = SetAlternate(0, SELECTED_DATA_INTERFACE_INDEX);
	//if (!NT_SUCCESS(ntStatus))
	//	DBGU_WARNING("Free USB Bandwidth fail! (0x%X)\n", ntStatus);

	//2010/6/24 11:35と H264
	// Free Usb Bandwidth   james try M2TS.
	if(StreamNumber == STREAM_Capture)
		ntStatus = SetAlternate(0, SELECTED_DATA_INTERFACE_INDEX);

	if(StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
		ntStatus = SetAlternate(0, SELECTED_MPEG2_INTERFACE_INDEX);

	if (!NT_SUCCESS(ntStatus))
		DBGU_WARNING("Free USB Bandwidth fail! (0x%X)\n", ntStatus);

	m_AlternateSettingNo = 0;
	
	//2010/6/30 10:34と H264
	if(StreamNumber == STREAM_Capture)
		m_dwCurVideoFrameSize = 0;
		
	//2011/4/6 05:21と mux
	if(TSMuxEnable && !TerminateMuxThread && StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
	{
		int i=0;
		
		StopMuxThread = TRUE;
		TerminateMuxThread = TRUE;
		KeReleaseSemaphore(&TsMuxSemaphore,0,1,FALSE);
		
		while(StopMuxThread)
		{
			DBGU_TRACE("Delay 100 ms ! i=%d StopMuxThread = %d\n",i,StopMuxThread);
			KeDelay(100);
			
			if(i++>50) 
				break;
		}
	}	
	//RBK remove not sync memory free
	/*
	//2010/11/22 03:12と
	if(S420Buffer)
	{
		ExFreePoolWithTag(S420Buffer, '11US');
		S420Buffer = NULL;
	}
	*/
	//2010/6/29 04:40と
	// Restart interrupt transfer
	/*if (m_pDevExt->DevicePowerState == PowerDeviceD0)
		ntStatus = StartInterruptTransfer();
	
	if (ntStatus == STATUS_PENDING)
		ntStatus = STATUS_SUCCESS;
	*/
	return ntStatus;
}

NTSTATUS
CVideoDevice::ReadStill(
	PUCHAR pDisplayBuf,
	PKS_VIDEOINFOHEADER pVideoInfoHeader
	)
{
	DBGU_TRACE("CVideoDevice::ReadStill()\n");
	NTSTATUS ntStatus = STATUS_SUCCESS;

	// shawn 2011/08/03 +++++
	USHORT wWidthTmp = m_SnapShotInfo.wWidth;
	USHORT wHeightTmp = m_SnapShotInfo.wHeight;

	if ((m_SnapShotInfo.wWidth < 640 || m_SnapShotInfo.wHeight < 480) && m_fUSBHighSpeed)	// shawn 2011/08/11 modify for fixing method 2 snapshot bug
	{
		wWidthTmp = 640;
		wHeightTmp = 480;
	}
	// shawn 2011/08/03 -----
	
	if(!m_pCShell || !m_pDevExt)
		return (STATUS_UNSUCCESSFUL);

	if (!m_pDevExt->m_pStillFrameBuffer)
		return STATUS_UNSUCCESSFUL;

	DBGU_TRACE("Still Capture Method = %d, m_dwStillFrameLength = %d\n",m_pDevExt->m_StillCaptureMethod, m_dwStillFrameLength);

	for (int i=0;i<50;i++)
	{
		if (!m_SaveSnapShotFileReady)
		{
			DBGU_TRACE("still read(): Retry %d ..\n",i);
			KeDelay(50);
		}
		else if (m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] == IMG_FMT_MJPEG)	// shawn 2011/06/23 modify
		{
			DBGU_TRACE("copy one image as still frame for output..\n");
			// MJPEG just copy it into display buffer
			RtlCopyMemory(pDisplayBuf,
				m_pDevExt->m_pStillFrameBuffer,
				m_dwStillFrameLength);

			// shawn 2011/06/22 fix 290 MJPG loss data issue +++++
			if (m_pDevExt->m_ChipID == 0x90)
			{
				if (m_dwStillFrameLength >= 2)
				{
					if (pDisplayBuf[m_dwStillFrameLength - 1] != 0xd9)
					{
						if (pDisplayBuf[m_dwStillFrameLength - 2] != 0xff)
						{
							int i = 0;

							for (i = 0; i < 18; i++)
								pDisplayBuf[m_dwStillFrameLength + i] = 0x00;
							
							m_dwStillFrameLength += 20;
							pDisplayBuf[m_dwStillFrameLength - 2] = 0xff;
							pDisplayBuf[m_dwStillFrameLength - 1] = 0xd9;
						}
						else
						{
							m_dwStillFrameLength++;
							pDisplayBuf[m_dwStillFrameLength - 1] = 0xd9;
						}
					}
				}
			}
			// shawn 2011/06/22 fix 290 MJPG loss data issue -----

			m_MJPEGReadBufferLength = m_dwStillFrameLength;	// shawn 2011/07/26 add

			/*// shawn 2011/07/27 for testing +++++
			//
			// Write image data to file.
			//
			IO_STATUS_BLOCK		ioStatusBlock;
			HANDLE				fh;	
			LARGE_INTEGER		ByteOffset;
			UNICODE_STRING		uSnapShotFileName;
			OBJECT_ATTRIBUTES	oa;
			UCHAR				p[SNAPSHOT_FILE_HDR_SIZE];
			WCHAR				*wssfn=NULL;

			wssfn = new (NonPagedPool) WCHAR[MAX_PATH];
			
			if(!wssfn)
				break;

			DBGU_TRACE("m_SnapShotState : Start Save Image file! \n");
			DBGU_TRACE("Save snapshot to file! %dX%d\n",m_SnapShotInfo.wWidth,m_SnapShotInfo.wHeight);
			DBGU_TRACE("Save Still image file size = %d\n", m_dwStillFrameLength);
			
			swprintf(wssfn, L"\\DosDevices\\C:\\%ws",m_SnapShotFileName);
			RtlInitUnicodeString(&uSnapShotFileName, wssfn);
			InitializeObjectAttributes(&oa,&uSnapShotFileName, OBJ_CASE_INSENSITIVE,NULL,NULL);	

			if (NT_SUCCESS(ZwCreateFile(	&fh,
										GENERIC_WRITE | SYNCHRONIZE,
										&oa,
										&ioStatusBlock,
										0, 
										FILE_ATTRIBUTE_NORMAL,
										FILE_SHARE_WRITE,
										FILE_OVERWRITE_IF,
										FILE_SYNCHRONOUS_IO_NONALERT,
										NULL,
										0)))
			{
				ByteOffset.QuadPart = 0;
				
				ZwWriteFile(fh,
							NULL,
							NULL,
							NULL,
							&ioStatusBlock,
							pDisplayBuf,
							m_dwStillFrameLength,
							&ByteOffset,
							NULL);

				ZwClose(fh);
			}//zwCreateFile is ok
			
			if (wssfn)
				delete wssfn;
			// shawn 2011/07/27 for testing -----*/

			break;
		}
		// shawn 2011/08/03 modify +++++
		//else if (m_dwStillFrameLength != pVideoInfoHeader->bmiHeader.biSizeImage)
		
		else if(((m_dwStillFrameLength != wWidthTmp*wHeightTmp*3)
				&&(m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] == IMG_FMT_RGB24))
				 ||((m_dwStillFrameLength != wWidthTmp*wHeightTmp*3/2)
				 &&(m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] == IMG_FMT_I420))
				 ||((m_dwStillFrameLength != wWidthTmp*wHeightTmp*2)
				 &&(m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] == IMG_FMT_YUY2)))
		// shawn 2011/08/03 modify -----
		{
			m_SaveSnapShotFileReady = FALSE;
			ntStatus = StillPinTrigger();
			
			if (!NT_SUCCESS(ntStatus))
				return STATUS_PENDING;
		}
		else
		{
			if (m_pDevExt->m_StillCaptureMethod<2)
			{
				DBGU_TRACE("copy one image as still frame for output..\n");
				// Method 0,1 just copy it into display buffer
				RtlCopyMemory(pDisplayBuf,
					m_pDevExt->m_pStillFrameBuffer,
					m_dwStillFrameLength);
				break;
			}
			else
			{
				DBGU_TRACE("Receive one still frame for output..\n");
				//Video class has no AE count
				m_pCShell->pFACE->count = 1;

				//
				// Change shell param for still
				//
				CShell_Reset(m_pCShell);
				ImageSize tempSize = m_enImageOut;
				m_enImageOut = m_enImageOutStill;
				SetASICShellSize(ImgWidList[m_enImageOut], ImgHgtList[m_enImageOut],0, 
						m_ZoomParam.iZoomStep, 
						0, 
						0,
						0,
						0);
				//SetASICShellSize(m_ZoomParam.iZoomStep, 0, 0, FALSE, FALSE);

				if (m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] == IMG_FMT_MJPEG)	// shawn 2011/06/23 modify
				{
					SnPrint(DEBUGLVL_VERBOSE, ("MJPG: Pass without processing..\n"));
					RtlCopyMemory(pDisplayBuf,m_pDevExt->m_pStillFrameBuffer,pVideoInfoHeader->bmiHeader.biSizeImage);
				}
				// shawn 2011/08/01 +++++
				else if(m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] == IMG_FMT_YUY2 && !m_fUSBHighSpeed)
				{	
					SnPrint(DEBUGLVL_VERBOSE, ("Full Speed, YUY2: Pass without processing..\n"));
					RtlCopyMemory(pDisplayBuf,m_pDevExt->m_pStillFrameBuffer,pVideoInfoHeader->bmiHeader.biSizeImage);
				}
				// shawn 2011/08/01 -----
				else
				{
					CShell_SetProcessInfo(
						m_pCShell,
						pVideoInfoHeader->bmiHeader.biSizeImage,
						m_pDevExt->m_pStillFrameBuffer,
						//m_pShellBufferPtr,
						pDisplayBuf,
						0);

					switch (m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx])	// shawn 2011/06/23 modify
					{
					case IMG_FMT_RGB24:
						//CShell_Process(m_pCShell, 0, false, csRGB24, 0);
						CShell_Process(m_pCShell, 0, diYUY2, csRGB24);
						break;

					case IMG_FMT_I420:
						//CShell_Process(m_pCShell, 0, false, csI420, 0);
						CShell_Process(m_pCShell, 0, diYUY2, csI420);
						break;

					case IMG_FMT_YUY2:
						//CShell_Process(m_pCShell, 0, false, csYUY2, 0);
						CShell_Process(m_pCShell, 0, diYUY2, csYUY2);
						break;

					default:
						DBGU_WARNING("Unexpected Error occure!!\n");
						goto _READ_STILL_EXIT;
						break;
					}

					JPEGReport *pJPEGReport = CShell_GetJPEGReport(m_pCShell);
					
					if (pJPEGReport->bJPEGError == TRUE)
					{
						SnPrint(DEBUGLVL_VERBOSE, ("Image Size is wrong..\n"));
						ntStatus = STATUS_CANCELLED;
						goto _READ_STILL_EXIT;
					}
				}

//2010/10/6 03:49と
#ifndef	_NOEFFECT
				//	Effect
				if (m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] != IMG_FMT_MJPEG)	// shawn 2011/06/23 modify
				{
					LARGE_INTEGER timeout;
					NTSTATUS waitStatus;

					timeout.QuadPart = -10000000;
					waitStatus = KeWaitForSingleObject(&m_EffectMutex, Executive, KernelMode, FALSE, &timeout);

					// shawn 2011/06/01 modify +++++
					switch (m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx])	// shawn 2011/06/23 modify
					{
					case IMG_FMT_RGB24:
						EffectConvert((PBYTE)pDisplayBuf,
										pVideoInfoHeader->bmiHeader.biWidth,
										pVideoInfoHeader->bmiHeader.biHeight,
										0,
										m_pEffectBufferPtr);

						EffectConvert((PBYTE)pDisplayBuf,
										pVideoInfoHeader->bmiHeader.biWidth,
										pVideoInfoHeader->bmiHeader.biHeight,
										m_EffectParam,
										m_pEffectBufferPtr);
						
						break;

					case IMG_FMT_I420:
						EffectConvertI420((PBYTE)pDisplayBuf,
											pVideoInfoHeader->bmiHeader.biWidth,
											pVideoInfoHeader->bmiHeader.biWidth,
											pVideoInfoHeader->bmiHeader.biHeight,
											pVideoInfoHeader->bmiHeader.biHeight,
											0,
											m_pEffectBufferPtr);		
											
						EffectConvertI420((PBYTE)pDisplayBuf,
											pVideoInfoHeader->bmiHeader.biWidth,
											pVideoInfoHeader->bmiHeader.biWidth,
											pVideoInfoHeader->bmiHeader.biHeight,
											pVideoInfoHeader->bmiHeader.biHeight,
											m_EffectParam,
											m_pEffectBufferPtr);
						
						break;

					case IMG_FMT_YUY2:
						EffectConvertYUY2((PBYTE)pDisplayBuf,
											pVideoInfoHeader->bmiHeader.biWidth,
											pVideoInfoHeader->bmiHeader.biWidth,
											pVideoInfoHeader->bmiHeader.biHeight,
											pVideoInfoHeader->bmiHeader.biHeight,
											0,
											m_pEffectBufferPtr);		
											
						EffectConvertYUY2((PBYTE)pDisplayBuf,
											pVideoInfoHeader->bmiHeader.biWidth,
											pVideoInfoHeader->bmiHeader.biWidth,
											pVideoInfoHeader->bmiHeader.biHeight,
											pVideoInfoHeader->bmiHeader.biHeight,
											m_EffectParam,
											m_pEffectBufferPtr);
						
						break;

					default:
						DBGU_WARNING("Unexpected Error occure!!\n");
						goto _READ_STILL_EXIT;
						break;
					}
					
					/*EffectConvert( m_ImageFormat[STREAM_Still], // 0=RGB24, 1=I420, 2=YUY2
								pDisplayBuf,
								pVideoInfoHeader->bmiHeader.biWidth,
								pVideoInfoHeader->bmiHeader.biHeight,
								m_EffectParam
								);*/
					// shawn 2011/06/01 modify -----

					if (STATUS_SUCCESS == waitStatus)
						KeReleaseMutex(&m_EffectMutex, FALSE);
				}
				//
#endif
_READ_STILL_EXIT:
				//
				// Restore shell param for capture
				//
				CShell_Reset(m_pCShell);
				m_enImageOut = tempSize;
				//SetASICShellSize(m_ZoomParam.iZoomStep, 0, 0, FALSE, FALSE);
				SetASICShellSize(ImgWidList[m_enImageOut], ImgHgtList[m_enImageOut],0, 
						m_ZoomParam.iZoomStep, 
						0, 
						0,
						0,
						0);
			}
			
			break;
		}
	}

	if (!m_SaveSnapShotFileReady)
		ntStatus = STATUS_PENDING;

	m_SaveSnapShotFileReady = FALSE;
	m_SaveSnapShotFile = FALSE;

	SnPrint(DEBUGLVL_VERBOSE, ("Exit CVideoDevice::ReadStill() with status %X\n",ntStatus));
	return ntStatus;
}

NTSTATUS
CVideoDevice::Read(
	PUCHAR pDisplayBuf,
	PKS_VIDEOINFOHEADER pVideoInfoHeader,
	REFERENCE_TIME aft_prefer,
	BOOL bFirstFrame
	)
{
	DBGU_TRACE("CVideoDevice::Read() aft_prefer = %d",aft_prefer);
	NTSTATUS ntStatus = STATUS_SUCCESS;
	
	//2010/6/22 03:41と
	ULONG StreamNumber;
	
	StreamNumber = (pVideoInfoHeader != NULL) ? STREAM_Capture : m_pDevExt->m_STREAM_Capture_MP2TS_Idx;	// shawn 2011/06/23 modify
	
	//2010/7/13 11:10と skype
	if (StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
	{
		LARGE_INTEGER timeout;
		//2011/4/7 03:51と
		//if(m_FirstFrame)
		//	timeout.QuadPart = - 20000000;
		//else
			//timeout.QuadPart = - 333333*100;
		timeout.QuadPart = - 1000000;			// shawn 2011/07/25 for resolving 290 HW bug (re-start H264 stream)
		
		//TS mux 2011/4/1 05:15と
			
		//while((m_MPEG2ValidDataLength) < m_APBufferLen[STREAM_Capture_MP2TS])
		if((m_MPEG2ValidDataLength) < m_APBufferLen[m_pDevExt->m_STREAM_Capture_MP2TS_Idx])	// shawn 2011/06/23 modify
		{
			SnPrint(DEBUGLVL_VERBOSE, ("[TEST]KeWaitForSingleObject.. \n"));
			KeResetEvent(&m_MEPG2DataReadyEvent);
			ntStatus = KeWaitForSingleObject(&m_MEPG2DataReadyEvent, Executive, KernelMode, FALSE, &timeout);
			//ntStatus = KeWaitForSingleObject(&m_MEPG2DataReadyEvent, Executive, KernelMode, FALSE, NULL);

			if(ntStatus == STATUS_TIMEOUT)
			{
				// shawn 2011/07/25 for resolving 290 HW bug (re-start H264 stream) +++++
				//return STATUS_CANCELLED;

				BYTE abyTmp[188] = {0};
				LARGE_INTEGER TmpTime;

				memset(abyTmp, 0xFF, 188);
				abyTmp[0] = 0x47;
				abyTmp[1] = 0x1F;
				abyTmp[2] = 0xFF;
				abyTmp[3] = 0x10;
				
				RtlCopyMemory(pDisplayBuf, abyTmp, 188);
				m_MPEG2ReadBufferLength = 188;	//shawn 2011/08/05 add
				SnPrint(DEBUGLVL_VERBOSE, ("Shawn => Read : M2TS Timeout => Add dummy TS packet ...\n"));

				KeQuerySystemTime(&TmpTime);
				m_Time = TmpTime.QuadPart;
								
				return STATUS_SUCCESS;
				// shawn 2011/07/25 for resolving 290 HW bug (re-start H264 stream) -----
			}
		}

		SnPrint(DEBUGLVL_VERBOSE, ("m_MPEG2ValidDataLength = %d\n",m_MPEG2ValidDataLength));

		if(m_MPEG2ValidDataLength > m_APBufferLen[m_pDevExt->m_STREAM_Capture_MP2TS_Idx])	// shawn 2011/06/23 modify
		{
			if((m_MPEG2ReadBufferAddr + m_APBufferLen[m_pDevExt->m_STREAM_Capture_MP2TS_Idx]) > DEFAULT_DATA_BUFFER_SIZE)	// shawn 2011/06/23 modify
			{
				UINT Offset = DEFAULT_DATA_BUFFER_SIZE - m_MPEG2ReadBufferAddr;
				RtlCopyMemory(pDisplayBuf, pMP2TSDataBuf + m_MPEG2ReadBufferAddr, Offset);
				RtlCopyMemory(pDisplayBuf + Offset, pMP2TSDataBuf, m_APBufferLen[m_pDevExt->m_STREAM_Capture_MP2TS_Idx] - Offset);	// shawn 2011/06/23 modify
				m_MPEG2ReadBufferAddr = m_APBufferLen[m_pDevExt->m_STREAM_Capture_MP2TS_Idx] - Offset;	// shawn 2011/06/23 modify
			}
			else
			{
				RtlCopyMemory(pDisplayBuf, pMP2TSDataBuf + m_MPEG2ReadBufferAddr, m_APBufferLen[m_pDevExt->m_STREAM_Capture_MP2TS_Idx]);	// shawn 2011/06/23 modify
				m_MPEG2ReadBufferAddr += m_APBufferLen[m_pDevExt->m_STREAM_Capture_MP2TS_Idx];	// shawn 2011/06/23 modify
			
				if(m_MPEG2ReadBufferAddr >= DEFAULT_DATA_BUFFER_SIZE)
					m_MPEG2ReadBufferAddr = 0;
			}

			m_MPEG2ReadBufferLength = m_APBufferLen[m_pDevExt->m_STREAM_Capture_MP2TS_Idx];	// shawn 2011/06/23 modify
			SnPrint(DEBUGLVL_VERBOSE, ("M2TS(1): DataLength(%d) m_MPEG2ReadBufferAddr(%d)\n", m_MPEG2ReadBufferLength,m_MPEG2ReadBufferAddr));
			m_MPEG2ValidDataLength -= m_APBufferLen[m_pDevExt->m_STREAM_Capture_MP2TS_Idx];	// shawn 2011/06/23 modify

			if(m_FirstFrame)
				m_FirstFrame = FALSE;
		}
		else //if((m_MPEG2ValidDataLength + 760) > m_APBufferLen[STREAM_Capture_MP2TS])
		{
			if((m_MPEG2ReadBufferAddr + m_MPEG2ValidDataLength) > DEFAULT_DATA_BUFFER_SIZE)
			{
				UINT Offset = DEFAULT_DATA_BUFFER_SIZE - m_MPEG2ReadBufferAddr;
				RtlCopyMemory(pDisplayBuf, pMP2TSDataBuf + m_MPEG2ReadBufferAddr, Offset);
				RtlCopyMemory(pDisplayBuf + Offset, pMP2TSDataBuf, m_MPEG2ValidDataLength - Offset);
				m_MPEG2ReadBufferAddr = m_MPEG2ValidDataLength - Offset;
			}
			else
			{
				RtlCopyMemory(pDisplayBuf, pMP2TSDataBuf + m_MPEG2ReadBufferAddr, m_MPEG2ValidDataLength);
				m_MPEG2ReadBufferAddr += m_MPEG2ValidDataLength;

				if(m_MPEG2ReadBufferAddr >= DEFAULT_DATA_BUFFER_SIZE)
					m_MPEG2ReadBufferAddr = 0;
			}

			m_MPEG2ReadBufferLength = m_MPEG2ValidDataLength;
			SnPrint(DEBUGLVL_VERBOSE, ("M2TS(2): DataLength(%d) m_MPEG2ReadBufferAddr(%d)\n", m_MPEG2ReadBufferLength,m_MPEG2ReadBufferAddr));
			m_MPEG2ValidDataLength = 0;

			if(m_FirstFrame)
				m_FirstFrame = FALSE;
		}

		SnPrint(DEBUGLVL_VERBOSE, ("Left m_MPEG2ValidDataLength = %d\n",m_MPEG2ValidDataLength));

		/*// shawn 2011/08/04 for testing +++++
		for (int i = 0; i < m_MPEG2ReadBufferLength; i += 188)
		{
			if (pDisplayBuf[i] == 0x47 && 
				((pDisplayBuf[i+1] == 0x01) || (pDisplayBuf[i+1] == 0x41)) && 
				pDisplayBuf[i+2] == 0x00)
			{
				if ((i + 3) < m_MPEG2ReadBufferLength)
				{
					if (((pDisplayBuf[i+3])&0x0F) != m_byTmpTSCounter2)
					{
						DBGU_TRACE("Shawn => Read : TS Counter Failed => %d -> %d\n", m_byTmpTSCounter2, ((pDisplayBuf[i+3])&0x0F));
						m_byTmpTSCounter2 = ((pDisplayBuf[i+3])&0x0F);
					}
					
					m_byTmpTSCounter2 = (m_byTmpTSCounter2 + 1) % 16;
				}
			}
		}
		// shawn 2011/08/04 for testing -----*/

		return STATUS_SUCCESS;
	}
	
	//2010/4/22 10:13と
	if(StreamNumber == STREAM_Capture)
	{
		if(!m_pCShell || !m_pDevExt)
		{
			DBGU_TRACE("!m_pCShell || !m_pDevExt return (STATUS_UNSUCCESSFUL)\n");
			return (STATUS_UNSUCCESSFUL);
		}
	}
	
	if (bFirstFrame)
	{
		m_pLastDisplayBuf = NULL;
		//SetASICShellSize(m_ZoomParam.iZoomStep, 0, 0, FALSE, FALSE);
		SetASICShellSize(ImgWidList[m_enImageOut], ImgHgtList[m_enImageOut],0, 
						m_ZoomParam.iZoomStep, 
						0, 
						0,
						0,
						0);
	}

	// shawn 2011/06/02 +++++
#ifndef	_NOEFFECT

	DbgPrint("m_EffectParam=%d\n",m_EffectParam);

	//2008/12/15 12:03と modify
	//vivi 2005/8/26 if zoom enable, or image size > VGA, or VideoEffect is 0, disable Video Effect and Demo
	if(m_ZoomParam.bZoomEnable || pVideoInfoHeader->bmiHeader.biHeight > 480)
		if( ((1 <= m_EffectParam) && (m_EffectParam <= 16 )) || 
			(m_EffectParam == 0x30) || 
			(m_EffectParam == 0xff) )
		{
			//2008/1/11 06:14と modify for supporting effect when snapshot size>VGA
			//20070621
			/*if(deviceContext->m_StillProcessing)
			{
				if(deviceContext->m_StillProcessState == CURRENTFRAMEISSTILL)
				{
					Temp_EffectParam = deviceContext->m_EffectParam;
					deviceContext->m_EffectParam = 0;
					DbgPrint("deviceContext->m_EffectParam = 0;\n");;
				}
			}
			else*/
			
			//if(!deviceContext->m_StillProcessing)
				m_EffectParam = 0;
			
			//DbgPrint("Temp_EffectParam = %d deviceContext->m_StillProcessState=%d deviceContext->m_EffectParam=%d\n",Temp_EffectParam,deviceContext->m_StillProcessState,deviceContext->m_EffectParam);
		}
#endif
	// shawn 2011/06/02 -----

	// shawn 2011/05/24 modify +++++
	if(m_pDevExt->m_SetMirrorOrBW)
	{
		m_pDevExt->m_SetMirrorOrBW = FALSE;
		
		// for Mirror, Flip and BWMode
		CShell_SetBW(m_pCShell, m_pDevExt->m_BWMode.CurrentValue);
		CShell_SetHMirror(m_pCShell,m_pDevExt->m_Mirror.CurrentValue);
			
		if(/*VideoInfoHeader.bmiHeader.biBitCount == 24*/m_ImageFormat[STREAM_Capture] == IMG_FMT_RGB24)
			CShell_SetVMirror(m_pCShell, !m_pDevExt->m_Flip.CurrentValue);
		else
			CShell_SetVMirror(m_pCShell, m_pDevExt->m_Flip.CurrentValue);

		CShell_SetRotateState(m_pCShell, (RotateState)m_pDevExt->m_Rotation.CurrentValue);
	}
	// shawn 2011/05/24 modify -----

	PUCHAR pLastDisplayBuf = m_pLastDisplayBuf;
	m_pLastDisplayBuf = pDisplayBuf;

	UINT iRetry = 0;
	//2010/3/10 02:25と
	//UINT iRetryMax = (bFirstFrame || (pDisplayBuf != pLastDisplayBuf)) ? 20 : 2;//50 : 2;
	UINT iRetryMax;
	DBGU_TRACE("bFirstFrame = %d\n",bFirstFrame);
	
	if(bFirstFrame)
	{
		//DBGU_TRACE("iRetryMax = 30 bFirstFrame\n");
		iRetryMax = 100;
		//iRetryMax = 500;
	}
	else if (StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)//MPEG2 TS	// shawn 2011/06/23 modify
	{
		//DBGU_TRACE("iRetryMax = 2 pDisplayBuf == pLastDisplayBuf\n");
		iRetryMax = 10;
	}
	else if(pDisplayBuf == pLastDisplayBuf)
	{
		//DBGU_TRACE("iRetryMax = 2 pDisplayBuf == pLastDisplayBuf\n");
		//2010/11/16 05:53と test
		iRetryMax = 10;//2;
		
		//2010/8/5 02:35と
		if(pVideoInfoHeader->bmiHeader.biHeight == 216)
			iRetryMax = 5;
	}
	else 
	{
		//DBGU_TRACE("iRetryMax = 2\n");
		//2010/11/16 05:54と
		iRetryMax = 10;//2;	
		
		if(pVideoInfoHeader->bmiHeader.biHeight == 216)
			iRetryMax = 5;	
	}

	//2010/10/13 05:24と
	if(TwoPipeEnable && S420Buffer!= NULL)
	{
		PDATABUFCONTENT pContentPipe1=GetDataBuffer(DATAOP_READ, 0);
		//PUCHAR RawBuffer= NULL;
		ULONG TwoPipeModeFrameLen = pVideoInfoHeader->bmiHeader.biWidth * pVideoInfoHeader->bmiHeader.biHeight *3/4;
		
		while (!pContentPipe1)
		{
			//2010/11/22 06:36と
			if(m_pDevExt->m_StreamState != KSSTATE_RUN && StreamNumber == STREAM_Capture)
				break;
				
			if (iRetry < iRetryMax) {
				LARGE_INTEGER timeout;
				timeout.QuadPart = - aft_prefer*20;
	
				//2010/6/22 03:58と H264
				if (StreamNumber == STREAM_Capture)
				{
					KeResetEvent(&m_DataReadyEvent);
					KeWaitForSingleObject(&m_DataReadyEvent, Executive, KernelMode, FALSE, &timeout);	// shawn 2011/08/01 modify
				}

				iRetry++;
			}
			else
				break;

			pContentPipe1=GetDataBuffer(DATAOP_READ, 0);
			
			if(pContentPipe1)
			{
				//2010/11/23 04:25と
				/*
					//2010/11/15 04:00と
				if(pContentPipe1->DataLength != TwoPipeModeFrameLen)
				{
					DBGU_TRACE("pContentPipe1->DataLength != 230400 \n");
					ReturnDataBuffer(DATAOP_READ, 0);
					pContentPipe1 = NULL;
					//2010/11/23 04:20と
					break;
				}
				else*/
				
				break;
			}
			else
				DBGU_TRACE("pContentPipe1= 0 \n");
		}

		PDATABUFCONTENT pContentPipe2=GetDataBuffer(DATAOP_READ, 1);
		
		while (!pContentPipe2)
		{
			//2010/11/22 06:36と
			if(m_pDevExt->m_StreamState != KSSTATE_RUN && StreamNumber == STREAM_Capture)
				break;
							
			if (iRetry < iRetryMax) {
				LARGE_INTEGER timeout;
				timeout.QuadPart = - aft_prefer*20;
	
				//2010/6/22 03:58と H264
				if (StreamNumber == STREAM_Capture)
				{
					KeResetEvent(&m_MEPG2DataReadyEvent);
					KeWaitForSingleObject(&m_MEPG2DataReadyEvent, Executive, KernelMode, FALSE, &timeout);	// shawn 2011/08/01 modify
				}

				iRetry++;
			}
			else
				break;

			pContentPipe2=GetDataBuffer(DATAOP_READ, 1);

			if(pContentPipe2)
			{
				//2010/11/23 04:25と
				/*
				//2010/11/15 04:00と
				if(pContentPipe2->DataLength != TwoPipeModeFrameLen)
				{
					DBGU_TRACE("pContentPipe2->DataLength != 230400 \n");
					ReturnDataBuffer(DATAOP_READ, 1);
					pContentPipe2 = NULL;
					//2010/11/23 04:20と
					break;
				}
				else*/
				
				break;
			}
			else
				DBGU_TRACE("pContentPipe2 = 0 \n");
		}
				
		if (!pContentPipe1 || !pContentPipe2) {
			DBGU_WARNING("[DBG]Fail to get data buffer.\n");
			return STATUS_CANCELLED;
		}	

		if (StreamNumber == STREAM_Capture)	// james try M2TS.
		{
			//Video class has no AE count
			m_pCShell->pFACE->count = 1;
		}

		//2010/11/4 03:48と
		if (pContentPipe1 && pContentPipe2)
		{
			DBGU_TRACE("Pipe1->DataLength = %d Pipe2->DataLength = %d Pipe 1 PTS = %X, Pipe 2 PTS = %X\n",
			pContentPipe1->DataLength,pContentPipe2->DataLength,pContentPipe1->PTS,pContentPipe2->PTS);
			
			//2011/2/25 03:06と
			if(m_dwFrameSN%100 == 0)
				DBGU_TRACE("[mj]Pipe1->Length = %d Pipe2->Length = %d Pipe1 PTS = %X, Pipe2 PTS = %X FrameSN= %d ErrLen=%d ErrPTS=%d\n",
				pContentPipe1->DataLength,pContentPipe2->DataLength,pContentPipe1->PTS,pContentPipe2->PTS,m_dwFrameSN,m_IncorrectLengthFrameCount,m_IncorrectPTSCount);
			
		}

		//2010/11/15 04:50と
		if(pContentPipe1->DataLength == TwoPipeModeFrameLen &&pContentPipe2->DataLength == TwoPipeModeFrameLen)
		{
			if(pContentPipe1->PTS != pContentPipe2->PTS)
			{
				//2011/2/25 03:08と
				DBGU_TRACE("[error][mj]Incorrect PTS P1Length = %d P2Length = %d P1PTS = %X, P2PTS = %X FrameSN= %d ErrLen=%d ErrPTS=%d\n",
				pContentPipe1->DataLength,pContentPipe2->DataLength,pContentPipe1->PTS,pContentPipe2->PTS,m_dwFrameSN,m_IncorrectLengthFrameCount,m_IncorrectPTSCount);
				m_IncorrectPTSCount++;
				
				if(pContentPipe1->PTS > pContentPipe2->PTS)
				{
					DBGU_TRACE("Pipe1->PTS > Pipe2->PTS\n");
					//ReturnDataBuffer(DATAOP_READ, 1);
				}
				else if(pContentPipe1->PTS < pContentPipe2->PTS)
				{
					DBGU_TRACE("Pipe2->PTS > Pipe1->PTS \n");
					//ReturnDataBuffer(DATAOP_READ, 0);
				}

				ReturnDataBuffer(DATAOP_READ, 0);
				ReturnDataBuffer(DATAOP_READ, 1);
				ntStatus = STATUS_SUCCESS;
				return ntStatus;
			}
		}
		else
		{
			//2010/12/3 02:37と
			m_IncorrectLengthFrameCount++;
			DBGU_TRACE("[error][mj]Incorrect Length P1Length = %d P2Length = %d P1 PTS = %X, P2 PTS = %X FrameSN= %d ErrLen=%d ErrPTS=%d\n",
			pContentPipe1->DataLength,pContentPipe2->DataLength,pContentPipe1->PTS,pContentPipe2->PTS,m_dwFrameSN,m_IncorrectLengthFrameCount,m_IncorrectPTSCount);
				
			ReturnDataBuffer(DATAOP_READ, 0);
			ReturnDataBuffer(DATAOP_READ, 1);
			
			//2011/2/23 02:42と
			// james 2011/03/01 try the first frame being green issue.
			if(bFirstFrame) {
				//pVideoInfoHeader->bmiHeader.biSizeImage = 0;
				Report_Black_Image(pVideoInfoHeader, pDisplayBuf, m_ImageFormat[STREAM_Capture]);
				ntStatus = STATUS_SUCCESS;
			}
			else {
				//pVideoInfoHeader->bmiHeader.biSizeImage = 0;	// shawn 2011/08/03 remove
				//2011/2/23 02:42と
				ntStatus = STATUS_CANCELLED;//STATUS_SUCCESS;
			}

			return ntStatus;
				
		}
		//RawBuffer = (PUCHAR) ExAllocatePoolWithTag(NonPagedPool,1600*1200*3, '11US');
	
		if(m_ImageFormat[STREAM_Capture] == IMG_FMT_YUY2)
		{
			SnPrint(DEBUGLVL_VERBOSE, ("S420->YUY2 w=%d, h=%d\n",pVideoInfoHeader->bmiHeader.biWidth,pVideoInfoHeader->bmiHeader.biHeight));
			
			//2010/7/21 02:30と
			if(pDisplayBuf)
			{
				//2010/10/8 02:23と for 2 pipe
				int w = pVideoInfoHeader->bmiHeader.biWidth;
				int h = pVideoInfoHeader->bmiHeader.biHeight;
				int	offset = 0;
				
				for(int i=0;i<h;i++)
				{
					RtlCopyMemory(S420Buffer + offset*3/2,pContentPipe1->pDataBuf + offset*3/4, w*3/4);
					RtlCopyMemory(S420Buffer + offset*3/2 + w*3/4,pContentPipe2->pDataBuf + offset*3/4, w*3/4);
					offset += w;
					
					if(offset > ((w*h*3/2) - 2*w))
						break;
				}
				//RtlCopyMemory(pDisplayBuf,pContent->pDataBuf,pContent->DataLength);
			 }

			 //2010/11/10 09:47と
			/*if(m_dwFrameSN >= 10 && m_dwFrameSN <= 20)
			{
				DumpImageFiles(pContentPipe1->pDataBuf,pContentPipe1->DataLength,m_dwFrameSN);		
				DumpImageFiles(pContentPipe2->pDataBuf,pContentPipe2->DataLength,m_dwFrameSN+100);
				DumpImageFiles(S420Buffer,pContentPipe1->DataLength + pContentPipe2->DataLength,m_dwFrameSN+200);
			}*/

			 CShell_SetProcessInfo(
							m_pCShell,
							pContentPipe1->DataLength + pContentPipe2->DataLength,
							S420Buffer,
							//m_pShellBufferPtr,
							pDisplayBuf,
							0);
	
			switch (m_ImageFormat[STREAM_Capture])
			{
				case IMG_FMT_RGB24:
					//CShell_Process(m_pCShell, 0, false, csRGB24, 0);
					CShell_Process(m_pCShell, 0, diYUYV_S420, csRGB24);				
					break;

				case IMG_FMT_I420:
					//CShell_Process(m_pCShell, 0, false, csI420, 0);
					CShell_Process(m_pCShell, 0, diYUYV_S420, csI420);
					break;

				case IMG_FMT_YUY2:
					//CShell_Process(m_pCShell, 0, false, csYUY2, 0);
					CShell_Process(m_pCShell, 0, diYUYV_S420, csYUY2);
					break;

				default:
					DBGU_WARNING("Unexpected Error occure!!\n");
					//goto _READ_EXIT;
					break;
			}
		
			//2010/11/10 09:47と
			/*if(m_dwFrameSN >= 10 && m_dwFrameSN <= 20)
			{
				DumpImageFiles(pDisplayBuf,1280*720*2,m_dwFrameSN+300);		
			}*/	 
	
			ReturnDataBuffer(DATAOP_READ, 0);
			ReturnDataBuffer(DATAOP_READ, 1);
			SnPrint(DEBUGLVL_VERBOSE, ("Exit 2Pipes CVideoDevice::Read() with status %X\n",ntStatus));
			//ExFreePoolWithTag(RawBuffer, '11US');
				
			return ntStatus;		 
		}
		else
		{
			ReturnDataBuffer(DATAOP_READ, 0);
			ReturnDataBuffer(DATAOP_READ, 1);
			return ntStatus;		 
		}
	}//two pipe read end
	
	PDATABUFCONTENT pContent;

	if (StreamNumber == STREAM_Capture)		
		pContent=GetDataBuffer(DATAOP_READ, 0);
	else
		pContent=GetDataBuffer(DATAOP_READ, 1);

	while (!pContent) {
		/*
		//2010/11/22 06:36と
		if((m_pDevExt->m_StreamState != KSSTATE_RUN && StreamNumber == STREAM_Capture)
		||(m_pDevExt->m_MPEG2StreamState != KSSTATE_RUN && StreamNumber == STREAM_Capture_MP2TS))
			break;
		*/				
		
		//2011/5/16 06:30と
		if(m_pDevExt->m_StreamState != KSSTATE_RUN && StreamNumber == STREAM_Capture)
			break;
		
		if (iRetry < iRetryMax) {
			LARGE_INTEGER timeout;
			
			//timeout.QuadPart = - aft_prefer;
			if (StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// james try M2TS.	// shawn 2011/06/23 modify
				timeout.QuadPart = - 333333;
			else
				timeout.QuadPart = - aft_prefer;

			DBGU_TRACE("KeWaitForSingleObject StreamNumber=%d\n",StreamNumber);
			
			//2010/6/22 03:58と H264
			if (StreamNumber == STREAM_Capture)
			{
				KeResetEvent(&m_DataReadyEvent);
				KeWaitForSingleObject(&m_DataReadyEvent, Executive, KernelMode, FALSE, /*&timeout*/NULL);	// shawn 2011/06/02 modify

			}
			/*else if (StreamNumber == STREAM_Capture_MP2TS)
			{
				KeResetEvent(&m_MEPG2DataReadyEvent);
				KeWaitForSingleObject(&m_MEPG2DataReadyEvent, Executive, KernelMode, FALSE, &timeout);
			}*/

			iRetry++;
		}
		else
			break;

		if (StreamNumber == STREAM_Capture)		
			 pContent=GetDataBuffer(DATAOP_READ, 0);
		else
			 pContent=GetDataBuffer(DATAOP_READ, 1);

		if (StreamNumber == STREAM_Capture)// james try M2TS.
		{
			if (pContent)
			{
				DBGU_TRACE("1. pStillPin = %x\n", m_pDevExt->pStillPin);
				
				if (m_SaveSnapShotFileReady && m_pDevExt->pStillPin)
				{
					DBGU_TRACE("processing Still frame Ready, so wait for it to be complete..\n");
					ReturnDataBuffer(DATAOP_READ, 0);
					pContent = NULL;
				}
				else if (pContent->isStillImage)
				{
					DBGU_WARNING("Bypass Still frames..\n");
					if (m_pDevExt->m_pStillFrameBuffer)
					{
						// shawn 2011/08/03 modify +++++
						//if ((pContent->DataLength == m_SnapShotInfo.ImageSize) ||
						//	(m_SnapShotInfo.biCompression == MEDIASUBTYPE_MJPG.Data1))

						USHORT wWidthTmp = m_SnapShotInfo.wWidth;
						USHORT wHeightTmp = m_SnapShotInfo.wHeight;

						if ((m_SnapShotInfo.wWidth < 640 || m_SnapShotInfo.wHeight < 480) && m_fUSBHighSpeed)	// shawn 2011/08/11 modify for fixing method 2 snapshot bug
						{
							wWidthTmp = 640;
							wHeightTmp = 480;
						}
						
						if(((pContent->DataLength == wWidthTmp*wHeightTmp*3)
							 &&(m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] == IMG_FMT_RGB24))
							 ||((pContent->DataLength == wWidthTmp*wHeightTmp*3/2)
							 &&(m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] == IMG_FMT_I420))
							 ||((pContent->DataLength == wWidthTmp*wHeightTmp*2)
							 &&(m_ImageFormat[m_pDevExt->m_STREAM_Still_Idx] == IMG_FMT_YUY2))
							 ||(m_SnapShotInfo.biCompression == MEDIASUBTYPE_MJPG.Data1))
						// shawn 2011/08/03 modify -----
						{
							DBGU_TRACE("Copy this frame into still buffer %d\n",pContent->DataLength);
							RtlCopyMemory(m_pDevExt->m_pStillFrameBuffer, pContent->pDataBuf, pContent->DataLength);
							m_dwStillFrameLength = pContent->DataLength;
							m_SnapShotInfo.ImageSize = pContent->DataLength;

							m_SaveSnapShotFileReady = TRUE;
							DBGU_TRACE("2. pStillPin = %x\n", m_pDevExt->pStillPin);
							
							if (!m_pDevExt->pStillPin)
								goto _SAVE_TO_FILE;
							else
							{
								m_EventType |= SONIX_EVENT_START_STILL_XFER;
								KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);
							}
						}
						else
							DBGU_WARNING("Receive broken Still frame (%d/%d)..\n", pContent->DataLength, m_SnapShotInfo.ImageSize);
					}
					else
						DBGU_WARNING("Receive Still frame but no buffer to store..\n");

					ReturnDataBuffer(DATAOP_READ, 0);
					pContent = NULL;
				}
				/*else if (m_pDevExt->pStillPin && m_SaveSnapShotFile && m_pDevExt->m_StillCaptureMethod == 2)
				{
					DBGU_TRACE("processing Still frame, so wait for it to be complete..%d\n", m_dSnapShotRetry);
					ReturnDataBuffer(DATAOP_READ);
					pContent = NULL;
					
					// failed to get still image.. forget it
					m_dSnapShotRetry++;
					if (m_dSnapShotRetry >= iRetryMax)
					{
						StillPinTrigger();
						m_SaveSnapShotFile = FALSE;
					}
				}
				*/
				
				// can't get enough data, drop this and wait next..
				//else if ((m_ImageFormat[STREAM_Capture] != IMG_FMT_MJPEG)&&(pContent->DataLength != m_dwCurVideoFrameSize))
				else if ((m_ImageFormat[STREAM_Capture] != IMG_FMT_MJPEG) && (pContent->DataLength != m_dwCurVideoFrameSize) && (StreamNumber == STREAM_Capture))	// james try M2TS.
				{
					ReturnDataBuffer(DATAOP_READ, 0);
					DBGU_WARNING("[DBG]Fail to read enough image data.(%d < %d)\n",pContent->DataLength, m_dwCurVideoFrameSize);
					m_FrameErrorCount++;
					DBGU_WARNING("Frame Error count = %d\n",m_FrameErrorCount);
					DBGU_WARNING("m_AlternateSettingNo = %d\n",m_AlternateSettingNo);
					
					//2011/4/29 11:46と
					if(bFirstFrame) {
						//pVideoInfoHeader->bmiHeader.biSizeImage = 0;
						DBGU_WARNING("Report_Black_Image\n");
						Report_Black_Image(pVideoInfoHeader, pDisplayBuf, m_ImageFormat[STREAM_Capture]);
						ntStatus = STATUS_SUCCESS;
					}

					pContent = NULL;
				}
				else
					break;
			}
		}
	}

	if (!pContent) {
		DBGU_WARNING("[DBG]Fail to get data buffer.\n");
		//2010/5/14 06:15と
		return STATUS_CANCELLED;//STATUS_DEVICE_NOT_READY;
	}	

	if (StreamNumber == STREAM_Capture)	// james try M2TS.
	{
		//Video class has no AE count
		m_pCShell->pFACE->count = 1;
	}

	if (m_ImageFormat[STREAM_Capture] == IMG_FMT_MJPEG)
	{
		SnPrint(DEBUGLVL_VERBOSE, ("MJPG: Pass without processing..\n"));

		// shawn 2011/06/22 fix 290 MJPG loss data issue +++++
		if (m_pDevExt->m_ChipID == 0x90)
		{
			if (pContent->DataLength >= 2)
			{
				if (pContent->pDataBuf[pContent->DataLength - 1] != 0xd9)
				{
					if (pContent->pDataBuf[pContent->DataLength - 2] != 0xff)
					{
						SnPrint(DEBUGLVL_VERBOSE, ("290 MJPG Case2: loss 0xff, 0xd9\n"));
						int i = 0;

						for (i = 0; i < 18; i++)
							pContent->pDataBuf[pContent->DataLength + i] = 0x00;
						
						pContent->DataLength += 20;
						pContent->pDataBuf[pContent->DataLength - 2] = 0xff;
						pContent->pDataBuf[pContent->DataLength - 1] = 0xd9;
					}
					else
					{
						SnPrint(DEBUGLVL_VERBOSE, ("290 MJPG Case1: loss 0xd9\n"));
						pContent->DataLength++;
						pContent->pDataBuf[pContent->DataLength - 1] = 0xd9;
					}
				}
				else
				{
					SnPrint(DEBUGLVL_VERBOSE, ("290 MJPG Case3: Pass\n"));
				}
			}
			else
			{
				SnPrint(DEBUGLVL_VERBOSE, ("290 MJPG Length < 2\n"));
			}
		}
		/*// shawn 2011/08/22 for testing +++++
		else if (m_pDevExt->m_ChipID == 0x88 || m_pDevExt->m_ChipID == 0x89)
		{
			int i = 0;

			if (pContent->DataLength >= 2)
			{
				while (1)
				{
					SnPrint(DEBUGLVL_VERBOSE, ("288/289 MJPG Handle ==>\n"));

					if (pContent->pDataBuf[pContent->DataLength - 1] == 0x00 && i < 7)
					{
						SnPrint(DEBUGLVL_VERBOSE, ("288/289 MJPG Handle ==> (%d) DataLength-- !\n", i));
						pContent->DataLength--;
						i++;
					}
					else
					{
						if (pContent->pDataBuf[pContent->DataLength - 1] != 0xd9 || 
							pContent->pDataBuf[pContent->DataLength - 2] != 0xff)
						{
							SnPrint(DEBUGLVL_VERBOSE, ("288/289 MJPG Handle ==> Lost 0xff 0xd9 !\n", i));
						}

						break;
					}

					//if (pContent->pDataBuf[pContent->DataLength - 1] == 0xd9 && 
					//	pContent->pDataBuf[pContent->DataLength - 2] == 0xff)
					//{
					//	SnPrint(DEBUGLVL_VERBOSE, ("288 MJPG Handle ==> OK !\n"));
					//	break;
					//}
					//else
					//{
					//	i++;
					//						
					//	if (pContent->DataLength > 0)
					//	{
					//		SnPrint(DEBUGLVL_VERBOSE, ("288 MJPG Handle ==> (%d) DataLength-- !\n", i));
					//		pContent->DataLength--;
					//	}
					//	else
					//	{
					//		SnPrint(DEBUGLVL_VERBOSE, ("288 MJPG Handle ==> DataLength = 0 !\n", i));
					//		break;
					//	}
					//}
				}
			}
		}
		// shawn 2011/08/22 for testing -----*/
		else
		{
			// shawn 2011/06/09 for checking MJPG frame +++++
			/*int iJFIFLen = m_pCShell->iStreamLen;
			CJPEG  *pJPEG   = m_pCShell->pJPEG;
			CJPEG_JPEGDecodeCheckError(pJPEG, pContent->pDataBuf, iJFIFLen);
			JPEGReport *pJPEGReport = CShell_GetJPEGReport(m_pCShell);
			
			if (pJPEGReport->bJPEGError == TRUE)
			{
				SnPrint(DEBUGLVL_VERBOSE, ("pJPEGReport->bJPEGError == TRUE image size is incorrect: %d\n", pJPEGReport->iErrorCode));
				ntStatus = STATUS_CANCELLED;
				goto _READ_EXIT;
			}*/
			// shawn 2011/06/09 for checking MJPG frame -----
		}
		// shawn 2011/06/22 fix 290 MJPG loss data issue -----
		
		//2010/7/21 02:30と
		if(pDisplayBuf && pContent->DataLength <= m_APBufferLen[STREAM_Capture])
		{
			RtlCopyMemory(pDisplayBuf,pContent->pDataBuf,pContent->DataLength);
		}
		
		/*m_MPEG2ReadBufferLength*/m_MJPEGReadBufferLength = pContent->DataLength;	// shawn 2011/07/04 modify
	}
	else if (StreamNumber == m_pDevExt->m_STREAM_Capture_MP2TS_Idx)	// james try M2TS.	// shawn 2011/06/23 modify
	{
		SnPrint(DEBUGLVL_VERBOSE, ("M2TS: Pass without processing.. DataLength(%d)\n", pContent->DataLength));
		RtlCopyMemory(pDisplayBuf,pContent->pDataBuf,pContent->DataLength);
		//RtlCopyMemory(pDisplayBuf,pContent->pDataBuf, 0x2000);	// james try
		m_MPEG2ReadBufferLength = pContent->DataLength;
		goto _READ_EXIT;
	}
	//2010/11/10 02:49と
	else if(0)//(m_ImageFormat[STREAM_Capture] == IMG_FMT_YUY2)
	{
		SnPrint(DEBUGLVL_VERBOSE, ("YUY2: Pass without processing..m_dwFrameSN = %d\n",m_dwFrameSN));
		RtlCopyMemory(pDisplayBuf,pContent->pDataBuf,pContent->DataLength);
	}
	// shawn 2011/08/01 +++++
	else if(m_ImageFormat[STREAM_Capture] == IMG_FMT_YUY2 && !m_fUSBHighSpeed)
	{	
		SnPrint(DEBUGLVL_VERBOSE, ("Full Speed, YUY2: Pass without processing..m_dwFrameSN = %d\n",m_dwFrameSN));
		RtlCopyMemory(pDisplayBuf,pContent->pDataBuf,pContent->DataLength);
	}
	// shawn 2011/08/01 -----
	else
	{
		CShell_SetProcessInfo(
			m_pCShell,
			pContent->DataLength,
			pContent->pDataBuf,
			//m_pShellBufferPtr,
			pDisplayBuf,
			0);

		switch (m_ImageFormat[STREAM_Capture])
		{
			case IMG_FMT_RGB24:
				//CShell_Process(m_pCShell, 0, false, csRGB24, 0);
				CShell_Process(m_pCShell, 0, diYUY2, csRGB24);
				break;

			case IMG_FMT_I420:
				//CShell_Process(m_pCShell, 0, false, csI420, 0);
				CShell_Process(m_pCShell, 0, diYUY2, csI420);
				break;

			case IMG_FMT_YUY2:
				//CShell_Process(m_pCShell, 0, false, csYUY2, 0);
				CShell_Process(m_pCShell, 0, diYUY2, csYUY2);
				break;

			default:
				DBGU_WARNING("Unexpected Error occure!!\n");
				goto _READ_EXIT;
				break;
		}

		JPEGReport *pJPEGReport = CShell_GetJPEGReport(m_pCShell);
		
		if (pJPEGReport->bJPEGError == TRUE)
		{
			SnPrint(DEBUGLVL_VERBOSE, ("Image Size is wrong..\n"));
			ntStatus = STATUS_CANCELLED;
			goto _READ_EXIT;
		}
	}

	//	Effect
	//2010/10/6 03:50と
#ifndef	_NOEFFECT
	//if ((!bFirstFrame)&&(m_ImageFormat[STREAM_Capture] != IMG_FMT_MJPEG))
	if ((!bFirstFrame)&&(m_ImageFormat[STREAM_Capture] != IMG_FMT_MJPEG)&&(pVideoInfoHeader != NULL))	// james try M2TS.
	{
		LARGE_INTEGER timeout;
		NTSTATUS waitStatus;

		timeout.QuadPart = -10000000;
		waitStatus = KeWaitForSingleObject(&m_EffectMutex, Executive, KernelMode, FALSE, &timeout);

		KeEnterCriticalRegion();
		
		// shawn 2011/06/01 modify +++++
		switch (m_ImageFormat[STREAM_Capture])
		{
		case IMG_FMT_RGB24:
			EffectConvert((PBYTE)pDisplayBuf,
							pVideoInfoHeader->bmiHeader.biWidth,
							pVideoInfoHeader->bmiHeader.biHeight,
							0,
							m_pEffectBufferPtr);

			EffectConvert((PBYTE)pDisplayBuf,
							pVideoInfoHeader->bmiHeader.biWidth,
							pVideoInfoHeader->bmiHeader.biHeight,
							m_EffectParam,
							m_pEffectBufferPtr);
					
			break;

		case IMG_FMT_I420:
			EffectConvertI420((PBYTE)pDisplayBuf,
								pVideoInfoHeader->bmiHeader.biWidth,
								pVideoInfoHeader->bmiHeader.biWidth,
								pVideoInfoHeader->bmiHeader.biHeight,
								pVideoInfoHeader->bmiHeader.biHeight,
								0,
								m_pEffectBufferPtr);		
										
			EffectConvertI420((PBYTE)pDisplayBuf,
								pVideoInfoHeader->bmiHeader.biWidth,
								pVideoInfoHeader->bmiHeader.biWidth,
								pVideoInfoHeader->bmiHeader.biHeight,
								pVideoInfoHeader->bmiHeader.biHeight,
								m_EffectParam,
								m_pEffectBufferPtr);
					
			break;

		case IMG_FMT_YUY2:
			EffectConvertYUY2((PBYTE)pDisplayBuf,
								pVideoInfoHeader->bmiHeader.biWidth,
								pVideoInfoHeader->bmiHeader.biWidth,
								pVideoInfoHeader->bmiHeader.biHeight,
								pVideoInfoHeader->bmiHeader.biHeight,
								0,
								m_pEffectBufferPtr);		
										
			EffectConvertYUY2((PBYTE)pDisplayBuf,
								pVideoInfoHeader->bmiHeader.biWidth,
								pVideoInfoHeader->bmiHeader.biWidth,
								pVideoInfoHeader->bmiHeader.biHeight,
								pVideoInfoHeader->bmiHeader.biHeight,
								m_EffectParam,
								m_pEffectBufferPtr);
					
			break;

		default:
			DBGU_WARNING("Unexpected Error occure!!\n");
			goto _READ_EXIT;
			break;
		}

		/*EffectConvert( m_ImageFormat[STREAM_Capture], // 0=RGB24, 1=I420, 2=YUY2
					pDisplayBuf,
					pVideoInfoHeader->bmiHeader.biWidth,
					pVideoInfoHeader->bmiHeader.biHeight,
					m_EffectParam
					);*/
		// shawn 2011/06/01 modify -----

		KeLeaveCriticalRegion();

		if (STATUS_SUCCESS == waitStatus)
			KeReleaseMutex(&m_EffectMutex, FALSE);
	}
	//
#endif

	// shawn 2011/06/01 +++++
	if(m_ImageFormat[STREAM_Capture] != IMG_FMT_MJPEG && m_FaceTrackParams[0] == 2)
	{
		//2008/12/15 01:40と
		//if(deviceContext->m_enImageOut <= isVGA)
		if(pVideoInfoHeader->bmiHeader.biHeight <= 480)
		{
			FaceReport *pFaceReport = CShell_GetFaceReport(m_pCShell);

			int p,t,z;
			p = pFaceReport->pan;
			t = pFaceReport->tile;

			//vivi 2005/11/16 3X zoom
			//z = min(max(pFaceReport->zoom + pFaceReport->PCam_zoom, 0), 64);
			z = min(max(pFaceReport->zoom + pFaceReport->PCam_zoom, 0), 84);

			//2008/4/25 03:17と fix system crash
			//SetASICShellSize(deviceContext, deviceContext->m_enImageOut, 0, z, p, t, 1, 0);
			SetASICShellSize(ImgWidList[m_enImageOut], ImgHgtList[m_enImageOut],0, z, p, t, 0, 0);
			
			pFaceReport->PCam_pan  = m_ZoomParam.iHPaneIdx;
			pFaceReport->PCam_tile = m_ZoomParam.iVPaneIdx;
			pFaceReport->PCam_zoom = z;		
			DBGU_WARNING("Face Tracking\n");
		}
	}
	// shawn 2011/06/01 -----

_SAVE_TO_FILE:
	// save file only when still pin not connected
	DBGU_TRACE("3. pStillPin = %x\n", m_pDevExt->pStillPin);

	if (m_SaveSnapShotFile && !m_pDevExt->pStillPin)
	{
		if (m_pDevExt->m_pStillFrameBuffer)
		{
			// just copy one frmae as output
			if (!m_SaveSnapShotFileReady)
			{
				if ((m_SnapShotSize <= SNAPSHOT_640X480) || (m_enImageOut == m_enImageOutStill))
				{
					// Copy this frame into still buffer
					RtlCopyMemory(m_pDevExt->m_pStillFrameBuffer, pDisplayBuf, pVideoInfoHeader->bmiHeader.biSizeImage);
					m_SnapShotInfo.wWidth = pVideoInfoHeader->bmiHeader.biWidth;
					m_SnapShotInfo.wHeight = pVideoInfoHeader->bmiHeader.biHeight;
					m_SnapShotInfo.ImageSize = pVideoInfoHeader->bmiHeader.biSizeImage;
					m_SnapShotInfo.biCompression = pVideoInfoHeader->bmiHeader.biCompression;

					m_SaveSnapShotFileReady = TRUE;
					m_dwStillFrameLength = pVideoInfoHeader->bmiHeader.biSizeImage;

					m_EventType |= SONIX_EVENT_START_STILL_XFER;
					KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);
				}
			}
		}
		else
			goto _READ_EXIT;

		// Still Pin not connected, save it into file
		if (m_SaveSnapShotFileReady)
		{
			//
			// Write image data to file.
			//
			IO_STATUS_BLOCK		ioStatusBlock;
			HANDLE				fh;	
			LARGE_INTEGER		ByteOffset;
			UNICODE_STRING		uSnapShotFileName;
			OBJECT_ATTRIBUTES	oa;
			UCHAR				p[SNAPSHOT_FILE_HDR_SIZE];
			WCHAR				*wssfn=NULL;

			wssfn = new (NonPagedPool) WCHAR[MAX_PATH];
			
			if(!wssfn)
				goto _READ_EXIT;

			DBGU_TRACE("m_SnapShotState : Start Save Image file! \n");
			DBGU_TRACE("Save snapshot to file! %dX%d\n",m_SnapShotInfo.wWidth,m_SnapShotInfo.wHeight);
			DBGU_TRACE("Save Still image file size = %d\n", m_dwStillFrameLength);
			m_SaveSnapShotFile = FALSE;

			swprintf(wssfn, L"\\DosDevices\\C:\\%ws",m_SnapShotFileName);
			RtlInitUnicodeString(&uSnapShotFileName, wssfn);
			InitializeObjectAttributes(&oa,&uSnapShotFileName, OBJ_CASE_INSENSITIVE,NULL,NULL);	

			if (NT_SUCCESS(ZwCreateFile(	&fh,
										GENERIC_WRITE | SYNCHRONIZE,
										&oa,
										&ioStatusBlock,
										0, 
										FILE_ATTRIBUTE_NORMAL,
										FILE_SHARE_WRITE,
										FILE_OVERWRITE_IF,
										FILE_SYNCHRONOUS_IO_NONALERT,
										NULL,
										0)))
			{
				*(WORD *)(&p[0]) = SNAPSHOT_FILE_HDR_SIZE;  // Header size
				
				if (m_SnapShotInfo.biCompression == MEDIASUBTYPE_YUY2.Data1)
					p[2] = 2;    // YUY2
				else if (m_SnapShotInfo.biCompression == MEDIASUBTYPE_I420.Data1)
					p[2] = 1;    // I420
				else
					p[2] = 0;    // RGB24 or MJPEG

				p[3] = 0; // Reserved
				*(DWORD *)(&p[4]) = (DWORD) m_SnapShotInfo.wWidth;		// Input image width
				*(DWORD *)(&p[8]) = (DWORD) m_SnapShotInfo.wHeight;		// Input image height
				*(DWORD *)(&p[12]) = (DWORD) m_SnapShotInfo.ImageSize;	// Input image size (bytes)

				if (m_SnapShotInfo.wWidth < 640)
				{
					*(DWORD *)(&p[16]) = (DWORD) m_SnapShotInfo.wWidth;		// Output image width
					*(DWORD *)(&p[20]) = (DWORD) m_SnapShotInfo.wHeight;	// Output image height
				}
				else
				{
					switch (m_SnapShotSize)
					{
					case SNAPSHOT_800X600:
						*(DWORD *)(&p[16]) = 640;	// Output image width
						*(DWORD *)(&p[20]) = 480;	// Output image height
						break;

					case SNAPSHOT_1024X768:
						*(DWORD *)(&p[16]) = 1024;	// Output image width
						*(DWORD *)(&p[20]) = 768;	// Output image height
						break;

					case SNAPSHOT_1280X960:
						*(DWORD *)(&p[16]) = 1280;	// Output image width
						*(DWORD *)(&p[20]) = 960;	// Output image height
						break;

					case SNAPSHOT_1280X1024:
						*(DWORD *)(&p[16]) = 1280;	// Output image width
						*(DWORD *)(&p[20]) = 1024;	// Output image height
						break;

					case SNAPSHOT_1600X1200:
						*(DWORD *)(&p[16]) = 1600;	// Output image width
						*(DWORD *)(&p[20]) = 1200;	// Output image height
						break;

					case SNAPSHOT_2304x1728:
						*(DWORD *)(&p[16]) = 2304;	// Output image width
						*(DWORD *)(&p[20]) = 1728;	// Output image height
						break;

					case SNAPSHOT_2560X2048:
						*(DWORD *)(&p[16]) = 2560;	// Output image width
						*(DWORD *)(&p[20]) = 2048;	// Output image height
						break;

					// shawn 2011/07/19 +++++
					case SNAPSHOT_3200X2400:
						*(DWORD *)(&p[16]) = 3200;	// Output image width
						*(DWORD *)(&p[20]) = 2400;	// Output image height
						break;

					case SNAPSHOT_960X720:
						*(DWORD *)(&p[16]) = 960;	// Output image width
						*(DWORD *)(&p[20]) = 720;	// Output image height
						break;

					case SNAPSHOT_1280X800:
						*(DWORD *)(&p[16]) = 1280;	// Output image width
						*(DWORD *)(&p[20]) = 800;	// Output image height
						break;

					case SNAPSHOT_2816X2253:
						*(DWORD *)(&p[16]) = 2816;	// Output image width
						*(DWORD *)(&p[20]) = 2253;	// Output image height
						break;

					case SNAPSHOT_3520X2640:
						*(DWORD *)(&p[16]) = 3520;	// Output image width
						*(DWORD *)(&p[20]) = 2640;	// Output image height
						break;

					case SNAPSHOT_1600X900:
						*(DWORD *)(&p[16]) = 1600;	// Output image width
						*(DWORD *)(&p[20]) = 900;	// Output image height
						break;

					case SNAPSHOT_1280X720:
						*(DWORD *)(&p[16]) = 1280;	// Output image width
						*(DWORD *)(&p[20]) = 720;	// Output image height
						break;

					case SNAPSHOT_640X400:
						*(DWORD *)(&p[16]) = 640;	// Output image width
						*(DWORD *)(&p[20]) = 400;	// Output image height
						break;

					case SNAPSHOT_1920X1080:
						*(DWORD *)(&p[16]) = 1920;	// Output image width
						*(DWORD *)(&p[20]) = 1080;	// Output image height
						break;

					case SNAPSHOT_2048X1536:
						*(DWORD *)(&p[16]) = 2048;	// Output image width
						*(DWORD *)(&p[20]) = 1536;	// Output image height
						break;
					// shawn 2011/07/19 -----

					default:
						*(DWORD *)(&p[16]) = 640;	// Output image width
						*(DWORD *)(&p[20]) = 480;	// Output image height
						break;
					}
				}

				// Write Header
				ByteOffset.QuadPart = 0;
				ZwWriteFile(fh,
							NULL,
							NULL,
							NULL,
							&ioStatusBlock,
							p,
							SNAPSHOT_FILE_HDR_SIZE,
							&ByteOffset,
							NULL);
				
				// Write Image frame data
				ByteOffset.QuadPart = SNAPSHOT_FILE_HDR_SIZE;
				
				// Convert MJPEG to RGB24
				if ((m_ImageFormat[STREAM_Capture] == IMG_FMT_MJPEG)&&(m_pDevExt->m_pStillFrameBuffer))
				{
					ULONG ImgSize = m_SnapShotInfo.wWidth*m_SnapShotInfo.wHeight*3;
					CShell_SetProcessInfo(
						m_pCShell,
						ImgSize,
						pContent->pDataBuf,
						//m_pShellBufferPtr,
						m_pDevExt->m_pStillFrameBuffer,
						0);

//					CShell_SetVMirror(m_pCShell, (m_pDevExt->m_Flip.CurrentValue ^ 1));
					//CShell_Process(m_pCShell, 0, true, csRGB24, 0);
						CShell_Process(m_pCShell, 0, diMJPG, csRGB24);
//					CShell_SetVMirror(m_pCShell, m_pDevExt->m_Flip.CurrentValue);

					ZwWriteFile(fh,
								NULL,
								NULL,
								NULL,
								&ioStatusBlock,
								m_pDevExt->m_pStillFrameBuffer,
								ImgSize,
								&ByteOffset,
								NULL);
				}
				else
				{
					ZwWriteFile(fh,
								NULL,
								NULL,
								NULL,
								&ioStatusBlock,
								m_pDevExt->m_pStillFrameBuffer,
								m_dwStillFrameLength,
								&ByteOffset,
								NULL);
				}

				ZwClose(fh);

				DBGU_TRACE("m_SnapShotState : Save Image file ok! \n");
				m_SaveSnapShotFileReady = TRUE;
				KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);
			}//zwCreateFile is ok
			
			if (wssfn)
				delete wssfn;
		}
	}

_READ_EXIT:
	//2010/3/17 03:51と
	// for MJPEG bugs
	if(0)//if((m_ImageFormat[STREAM_Capture] == IMG_FMT_MJPEG)&&(bFirstFrame == TRUE))
	{
		DBGU_TRACE("Endpoint reset at MJPEG \n");
		UCHAR data = 0;
		ASIC_Communication(CTL_W_NONE, 1, 0x1061, &data);
		data = 3;
		ASIC_Communication(CTL_W_NONE, 1, 0x1061, &data);
	}

	if (StreamNumber == STREAM_Capture)		
		ReturnDataBuffer(DATAOP_READ, 0);
	else
		ReturnDataBuffer(DATAOP_READ, 1);
		
	//ReturnDataBuffer(DATAOP_READ, 0);

	//SnPrint(DEBUGLVL_VERBOSE, ("Exit CVideoDevice::Read() with status %X\n",ntStatus));
	return ntStatus;
}

NTSTATUS CVideoDevice::StartInterruptTransfer()
{
	DBGU_TRACE("CVideoDevice::StartInterruptTransfer()\n");
	NTSTATUS ntStatus = STATUS_SUCCESS;
	USBD_PIPE_INFORMATION PipeInfo;	
	RtlZeroMemory(&PipeInfo,sizeof(USBD_PIPE_INFORMATION));

	DBGU_TRACE("Start interrupt transfer (%d)\n",m_UsbIoCount);
	return STATUS_SUCCESS;
	
	ntStatus = GetPipeInfo(SELECTED_CTRL_INTERFACE_INDEX,INT_PIPE_INDEX,&PipeInfo);
	
	if(NT_SUCCESS(ntStatus))
	{
		ntStatus = UsbBulkOrInterruptTransfer(
			m_IntTransferObj.pIrp,	
			m_IntTransferObj.pUrb,
			&PipeInfo,	
			TRUE,
			m_IntTransferObj.pUsbBuffer,	
			NULL,
			0x10,
			(PVOID) IntCompleteRoutine,	
			(PVOID) &m_IntTransferObj
		);

		if(NT_SUCCESS(ntStatus))
		{
			//2010/6/22 02:56と H264
			//InterlockedIncrement(&m_UsbIoCount);
			DBGU_TRACE("m_UsbIoCount = (%d)\n",m_UsbIoCount);
		}
	}

	DBGU_TRACE("Start interrupt transfer return Status = (%X)\n",ntStatus);
	return ntStatus;
}

NTSTATUS CVideoDevice::StopInterruptPipe()
{
	DBGU_TRACE("CVideoDevice::StopInterruptPipe()\n");
	NTSTATUS ntStatus = STATUS_SUCCESS;

	ntStatus = PipeAbortOrReset(SELECTED_CTRL_INTERFACE_INDEX,INT_PIPE_INDEX,TRUE);
	DBGU_TRACE("CVideoDevice::StopInterruptPipe() return = %X\n",ntStatus);
	return ntStatus;
}

NTSTATUS CVideoDevice::SurpriseRemoval()
{
	DBGU_INFO("!!! CVideoDevice::SurpriseRemoval !!!\n");
	NTSTATUS ntStatus = STATUS_SUCCESS;
	int i = 0;
	
	StopInterruptPipe();
	
	// shawn 2011/07/05 modify for AMCAP hang issue +++++
	//2010/6/22 02:55と H264
	/*if (m_UsbIoCount >= 1) {
		// abort iso transfer
		//PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX,ISO_PIPE_INDEX,TRUE);
		PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE_INDEX,TRUE);	// james try M2TS.
	}
	
	if (m_MPEG2UsbIoCount >= 1) {
		// abort iso transfer
		//PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX,ISO_PIPE_INDEX,TRUE);
		PipeAbortOrReset(SELECTED_MPEG2_INTERFACE_INDEX, ISO_PIPE_INDEX,TRUE);	// james try M2TS.
	}*/

	PipeAbortOrReset(SELECTED_DATA_INTERFACE_INDEX, ISO_PIPE_INDEX,TRUE);
	PipeAbortOrReset(SELECTED_MPEG2_INTERFACE_INDEX, ISO_PIPE_INDEX,TRUE);
	// shawn 2011/07/05 modify for AMCAP hang issue -----

	m_SnapShotState = AP_IS_NOT_RUNNING;
	WriteSnapShotStateToReg();
	
_EXIT_SURPRISE_REMOVAL:
	DBGU_INFO("!!! Exit CVideoDevice::SurpriseRemoval=%X !!!\n",ntStatus);
	return ntStatus;
}

void CVideoDevice::EventHandler()
{
	DBGU_INFO("Launch Thread CVideoDevice::EventHandler() !\n");
	NTSTATUS ntStatus=STATUS_SUCCESS;
	LARGE_INTEGER timeout;
	timeout.QuadPart = -50000000;//-50000000; //5 sec
	//ULONG	IdleCounter=0;
	LONG SuspendCancelledMax = 8 * 12; // 480s or 8min
	
	//m_pDevExt->m_bDeviceIdle = FALSE;
	//test
	//DBGU_TRACE("m_C3Enable == %X\n", m_pDevExt->m_C3Enable);
	//m_pDevExt->m_C3Enable = 0;
	
	//2010/8/26 03:29と
	//while (!m_EventHandlerShutdown)
	while(TRUE)
	{
		if ((!m_fWriteDefValue) || (m_pDevExt->m_C3Enable))
			ntStatus = KeWaitForSingleObject(&m_EventHandlerSemaphore, Executive, KernelMode, FALSE, &timeout);
		else
			ntStatus = KeWaitForSingleObject(&m_EventHandlerSemaphore, Executive, KernelMode, FALSE, NULL);
		
		DBGU_TRACE("PowerState == %x, m_bDeviceIdle = %x, m_pDevExt->m_cRef = %x\n", m_pDevExt->DevicePowerState,m_pDevExt->m_bDeviceIdle,m_pDevExt->m_cRef);
		
		//2010/8/13 06:16と
		if(m_EventHandlerShutdown)
		{
			m_EventType = 0;
			break;
		}
		
		if (!m_fWriteDefValue)
		{
			WriteDeviceInfoToReg(m_InstanceIndex);
			DBGU_TRACE("call WriteDeviceInfoToReg and return 0x%X\n",m_fWriteDefValue);
			continue;
		}
		
		if (m_bSyncLock)
		{
			DBGU_TRACE("we are doing zoom ctrl that will do asic cmd. we need to avoid it.\n");
			continue;
		}

		// for selective suspend
		if (ntStatus == STATUS_TIMEOUT)
		{
			//IdleCounter++;
			//if ((m_pDevExt->DevicePowerState == PowerDeviceD0)&&(IdleCounter>20))
			//2010/8/26 02:53と test
			if(m_pDevExt->m_C3Enable && m_pDevExt->m_bDeviceIdle)
			{
				if (m_pDevExt->DevicePowerState == PowerDeviceD0)
				{
					if (m_pDevExt->m_cRef == 0) // when m_cRef == 0
					{
						//2010/8/26 02:54と test
						ntStatus = SubmitIdleRequestIrp();
						//if(NT_SUCCESS(ntStatus))
						//	m_pDevExt->m_bDeviceIdle = TRUE;
						//else
						//	DBGU_WARNING("SubmitIdleRequestIrp return %X\n", ntStatus);*/
					}
					//IdleCounter=0;
				}
				else if ((m_pDevExt->DevicePowerState == PowerDeviceD2 || m_pDevExt->DevicePowerState == PowerDeviceUnspecified) && m_SuspendCancelled)
				{
					// if time interval after SuspendIrp is cancelled is greater than 3 miniutes (5 sec * 12),
					// set device power state back to D0
					// this prevents incorrect power state transition during S0 -> S3 (ymwu, 2005/7/16)
					//20061226 modify
				
					//if (pDC->m_SuspendCancelled++ > 36)
					if (m_SuspendCancelled++ > SuspendCancelledMax)
					{ 
						DbgPrint("Return to D0\n");
						SendDeviceSetPower(PowerDeviceD0, TRUE);
					}
				}
			}

			continue;
		}

		if (m_EventType & SONIX_EVENT_START_STILL_XFER)
		{
			m_EventType &= ~SONIX_EVENT_START_STILL_XFER;
			DBGU_TRACE("EventHandler 1. => pStillPin = %x\n", m_pDevExt->pStillPin);

			if (m_pDevExt->pStillPin)
			{
				DBGU_TRACE("Invoke Still Pin Process..\n");
				m_pDevExt->pStillPin->InvokePinProcess();
			}
		}
		else
		{
			UINT Count=0;
			
			//
			// Implement AutoStart/SnapShot State Machine
			//
			switch(m_SnapShotState)
			{
			case AP_IS_NOT_RUNNING:
				if(m_EventType & SONIX_EVENT_INTERRUPT)
				{
					DBGU_TRACE("m_SnapShotState -> AP_IS_NOT_RUNNING\n");
					m_EventType &= ~SONIX_EVENT_INTERRUPT;
					m_SnapShotState = AUTO_START;
					WriteSnapShotStateToReg();
					KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);
				}

				break;

			case AUTO_START:
				DBGU_TRACE("m_SnapShotState -> AUTO_START\n");
				ReadSnapShotStateFromReg();
				
				// AP change the state, so AP is aware of this interrupt.
				if (m_SnapShotState != AUTO_START)
					m_EventType &= ~SONIX_EVENT_INTERRUPT;
				
				//2010/6/29 05:56と H264	
				if(m_pDevExt->m_StreamState==KSSTATE_RUN || m_pDevExt->m_MPEG2StreamState==KSSTATE_RUN)
				{
					DBGU_TRACE("AP is Opened !\n");
					Count = 0;
					m_EventType &= ~SONIX_EVENT_INTERRUPT;
					m_SnapShotState = AP_IS_RUNNING;
					WriteSnapShotStateToReg();
				}
				else
				{
					if(Count++ < 100)
					{
						DBGU_TRACE("m_SnapShotState : Waiting for AP open !\n");
						KeDelay(50);
						if (m_SnapShotState != AUTO_START) Count = 0;
						KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);
					}
					else
					{
						DBGU_TRACE("m_SnapShotState : Time Out !\n");
						Count=0;
						m_SnapShotState = AP_IS_NOT_RUNNING;
						WriteSnapShotStateToReg();
					}
				}

				break;

			case AP_IS_RUNNING:
				DBGU_TRACE("m_SnapShotState -> AP_IS_RUNNING\n");
				
				if(m_EventType & SONIX_EVENT_INTERRUPT)
				{
					DBGU_TRACE("m_SnapShotState : Set SaveSnapShotFile \n");
					m_EventType &= ~SONIX_EVENT_INTERRUPT;
					//2010/8/3 02:49と
					/*
					m_SaveSnapShotFile = FALSE;
					m_SaveSnapShotFileReady = FALSE;
					m_pDevExt->pVideoDevice->StillPinTrigger();*/
				}
				else if(m_SaveSnapShotFileReady)
				{
					DBGU_TRACE("m_SnapShotState: m_SaveSnapShotFileReady !\n");
					m_SaveSnapShotFileReady = FALSE;
					DBGU_TRACE("EventHandler 2. => pStillPin = %x\n", m_pDevExt->pStillPin);
					
					if (!m_pDevExt->pStillPin)	// no work while still pin connecting
					{
						m_SnapShotState = SNAP_SHOT;
						WriteSnapShotStateToReg();
						KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);
					}
				}

				break;

			case SNAP_SHOT:
				DBGU_TRACE("m_SnapShotState -> SNAP_SHOT\n");
				ReadSnapShotStateFromReg();
				
				// AP change the state, so AP is aware of this interrupt ,2006/08/30
				if (m_SnapShotState != AUTO_START)
				{
					m_EventType &= ~SONIX_EVENT_INTERRUPT;
					m_SaveSnapShotFileReady = FALSE;
				}

				KeDelay(50);
				KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);
				break;

			case TWAIN_IS_RUNNING:
				DBGU_TRACE("m_SnapShotState -> TWAIN_IS_RUNNING\n");
				break;

			case WAITING_FOR_AP_OPEN:
				DBGU_TRACE("m_SnapShotState -> WAITING_FOR_AP_OPEN\n");
				
				//2010/6/29 05:56と H264	
				if(m_pDevExt->m_StreamState==KSSTATE_RUN || m_pDevExt->m_MPEG2StreamState==KSSTATE_RUN)
				{
					DBGU_TRACE("AP is Opened !\n");
					Count = 0;
					m_EventType &= ~SONIX_EVENT_INTERRUPT;
					m_SnapShotState = AP_IS_RUNNING;
					WriteSnapShotStateToReg();
				}
				else
				{
					if(Count++ < 100)
					{
						DBGU_TRACE("m_SnapShotState : Waiting for AP start !\n");
						KeDelay(100);
						ReadSnapShotStateFromReg();
						KeReleaseSemaphore(&m_EventHandlerSemaphore,0,1,FALSE);
					}
					else
					{
						DBGU_TRACE("m_SnapShotState : Time Out !\n");
						Count=0;
						m_SnapShotState = AP_IS_NOT_RUNNING;
						WriteSnapShotStateToReg();
					}
				}

				break;
			}
		}
	}
	
	//2010/8/26 07:06と
	//m_EventType = 0;
	//m_EventHandlerShutdown = false;
	DBGU_INFO("Exit Thread CVideoDevice::EventHandler() !\n");
}

NTSTATUS CVideoDevice::UpdateImageControlProperties(IN ULONG Id, IN BOOL fWrite, IN LONG buffer)
{
	NTSTATUS ntStatus=STATUS_SUCCESS;
	UCHAR	Request = (fWrite)? SET_CUR : GET_CUR;
	ULONG	Index;
	ULONG	Value;
	ULONG	BufferLength=sizeof(buffer);

	if (!m_pDevExt)
		return STATUS_UNSUCCESSFUL;

	if (!fWrite)	// [Get] clear buffer
		buffer = 0;
		//RtlZeroMemory(&buffer,sizeof(buffer));
	else	// [Set]
	{
		// below command only support for Set
		switch (Id)
		{
		case MIRROR:
			{
				// shawn 2011/05/25 remove
				//if (m_pDevExt->m_StreamState == KSSTATE_RUN && m_pCShell)
				//	CShell_SetHMirror(m_pCShell, buffer);

				m_pDevExt->m_Mirror.CurrentValue = buffer;
			}

			break;

		case FLIP:
			{
				// shawn 2011/05/25 remove
				/*if (m_pDevExt->m_StreamState == KSSTATE_RUN && m_pCShell)
				{
					if (m_ImageFormat[0] == IMG_FMT_RGB24)
						CShell_SetVMirror(m_pCShell, !buffer);
					else
						CShell_SetVMirror(m_pCShell, buffer);
				}*/

				m_pDevExt->m_Flip.CurrentValue = buffer;
			}

			break;

		case BW_MODE:
			{
				// shawn 2011/05/25 remove
				//if (m_pDevExt->m_StreamState == KSSTATE_RUN && m_pCShell)
				//	CShell_SetBW(m_pCShell, buffer);

				m_pDevExt->m_BWMode.CurrentValue = buffer;
			}

			break;

			// shawn 2011/05/25 +++++
		case ROTATION:
			{
				m_pDevExt->m_Rotation.CurrentValue = buffer;
			}

			break;

		case SNAPSHOT_CTRL:
			{
				m_pDevExt->m_Snapshot_Ctrl.CurrentValue = buffer;
			}

			break;

		case DENOISE:
			{
				m_pDevExt->m_DeNoiseMode.CurrentValue = buffer;

				switch(m_pDevExt->m_DeNoiseMode.CurrentValue)
				{
					case 0:
						m_pCShell->Param.bTemporalNR = m_pDevExt->m_TemporalNR = TEMPORAL_LEVEL_0;
						m_pCShell->Param.bEdgeDtn = m_pDevExt->m_EdgeDtn = EDGEDETECT_LEVEL_0;
						break;

					case 1:
						m_pCShell->Param.bTemporalNR = m_pDevExt->m_TemporalNR = TEMPORAL_LEVEL_1;
						m_pCShell->Param.bEdgeDtn = m_pDevExt->m_EdgeDtn = EDGEDETECT_LEVEL_1;
						m_pCShell->Param.iTempStrgLumThd = m_pDevExt->m_StrgLThd = STRGLTHD_LEVEL_1;
						m_pCShell->Param.iTempStrgChrThd = m_pDevExt->m_StrgCThd = STRGCTHD_LEVEL_1;	
						m_pCShell->Param.iTempNzPwLumThd = m_pDevExt->m_NzPwrLThd = NZPWRLTHD_LEVEL_1;
						m_pCShell->Param.iTempNzPwChrThd = m_pDevExt->m_NzPwrCThd = NZPWRCTHD_LEVEL_1;
						m_pCShell->Param.iLowLightThd = m_pDevExt->m_LowLightThd = LOWLIGHTTHD_LEVEL_1;	
						m_pCShell->Param.iMeanDiffYThd = m_pDevExt->m_MeanDiffYThd = MEANDIFFYTHD_LEVEL_1;
						m_pCShell->Param.iMeanDiffCThd = m_pDevExt->m_MeanDiffCThd = MEANDIFFCTHD_LEVEL_1;			
						break;

					case 2:
						m_pCShell->Param.bTemporalNR = m_pDevExt->m_TemporalNR = TEMPORAL_LEVEL_2;
						m_pCShell->Param.bEdgeDtn = m_pDevExt->m_EdgeDtn = EDGEDETECT_LEVEL_2;
						m_pCShell->Param.iTempStrgLumThd = m_pDevExt->m_StrgLThd = STRGLTHD_LEVEL_2;
						m_pCShell->Param.iTempStrgChrThd = m_pDevExt->m_StrgCThd = STRGCTHD_LEVEL_2;	
						m_pCShell->Param.iTempNzPwLumThd = m_pDevExt->m_NzPwrLThd = NZPWRLTHD_LEVEL_2;
						m_pCShell->Param.iTempNzPwChrThd = m_pDevExt->m_NzPwrCThd = NZPWRCTHD_LEVEL_2;
						m_pCShell->Param.iLowLightThd = m_pDevExt->m_LowLightThd = LOWLIGHTTHD_LEVEL_2;	
						m_pCShell->Param.iMeanDiffYThd = m_pDevExt->m_MeanDiffYThd = MEANDIFFYTHD_LEVEL_2;
						m_pCShell->Param.iMeanDiffCThd = m_pDevExt->m_MeanDiffCThd = MEANDIFFCTHD_LEVEL_2;			
						break;

					case 3:
						m_pCShell->Param.bTemporalNR = m_pDevExt->m_TemporalNR = TEMPORAL_LEVEL_3;
						m_pCShell->Param.bEdgeDtn = m_pDevExt->m_EdgeDtn = EDGEDETECT_LEVEL_3;						
						m_pCShell->Param.iTempStrgLumThd = m_pDevExt->m_StrgLThd = STRGLTHD_LEVEL_3;
						m_pCShell->Param.iTempStrgChrThd = m_pDevExt->m_StrgCThd = STRGCTHD_LEVEL_3;	
						m_pCShell->Param.iTempNzPwLumThd = m_pDevExt->m_NzPwrLThd = NZPWRLTHD_LEVEL_3;
						m_pCShell->Param.iTempNzPwChrThd = m_pDevExt->m_NzPwrCThd = NZPWRCTHD_LEVEL_3;
						m_pCShell->Param.iLowLightThd = m_pDevExt->m_LowLightThd = LOWLIGHTTHD_LEVEL_3;	
						m_pCShell->Param.iMeanDiffYThd = m_pDevExt->m_MeanDiffYThd = MEANDIFFYTHD_LEVEL_3;
						m_pCShell->Param.iMeanDiffCThd = m_pDevExt->m_MeanDiffCThd = MEANDIFFCTHD_LEVEL_3;			
						break;

					default:
						break;
				}
			}

			break;
			// shawn 2011/05/25 -----
		}
	}

	switch (Id)
	{
	case EXPOSURE:
	case FOCUS:
	case FOCUS_AUTO:
	case IRIS:
	case ZOOM:
	//case PANTILT:
	//2011/1/6 02:44と
	case PAN:
	case TILT:
	case ROLL:
	case PRIVACY:
	case AE_MODE:
	case AE_PRIORITY:
	//2010/12/16 03:40と
	case AEPRIORITY:
		Index = m_pDevExt->m_InputEntityID<<8;
		break;

	case BRIGHTNESS:
	case CONTRAST:
	case HUE:
	case SATURATION:
	case SHARPNESS:
	case GAMMA:
	case WHITEBALANCE:
	case BACKLIGHT:
	case POWERLINEFREQUENCY:
	case GAIN:
	case WB_COMPONENT:
	case WB_TEMPERATURE_AUTO:
	case WB_COMPONENT_AUTO:
		Index = m_pDevExt->m_PropertyEntityID<<8;
		break;

	case MIRROR:
	case FLIP:
	case BW_MODE:
		return ntStatus;
		break;

	default:
		DBGU_WARNING("UpdateImageControlProperties(%d,%d,%d) return 0x%X\n",Id,fWrite,buffer,ntStatus);
		return STATUS_NOT_SUPPORTED;
	}

	BufferLength = 2;

	switch (Id)
	{
	case EXPOSURE:
		Value=CT_EXPOSURE_TIME_ABSOLUTE_CONTROL<<8;
		BufferLength = 4;
		break;

	case FOCUS:
		Value=CT_FOCUS_ABSOLUTE_CONTROL<<8;
		BufferLength = 2;
		break;

	//2010/3/16 03:32と
	case FOCUS_AUTO:
		Value=CT_FOCUS_AUTO_CONTROL<<8;
		BufferLength = 1;
		break;

	case IRIS:
		Value=CT_IRIS_ABSOLUTE_CONTROL<<8;
		BufferLength = 2;
		break;

	case ZOOM:
		Value=CT_ZOOM_ABSOLUTE_CONTROL<<8;
		BufferLength = 2;
		break;

	//case PANTILT:
	case PAN:
	case TILT:
		Value=CT_PANTILT_ABSOLUTE_CONTROL<<8;
		BufferLength = 8;
		break;

	case ROLL:
		Value=CT_ROLL_ABSOLUTE_CONTROL<<8;
		BufferLength = 2;
		break;

	case PRIVACY:
		Value=CT_PRIVACY_CONTROL<<8;
		BufferLength = 1;
		break;

	case AE_MODE:
		Value=CT_AE_MODE_CONTROL<<8;
		BufferLength = 1;
		break;

	case AE_PRIORITY:
		Value=CT_AE_PRIORITY_CONTROL<<8;
		BufferLength = 1;
		break;

	case BRIGHTNESS:
		Value=PU_BRIGHTNESS_CONTROL<<8;
		BufferLength = 2;
		break;

	case CONTRAST:
		Value=PU_CONTRAST_CONTROL<<8;
		BufferLength = 2;
		break;

	case HUE:
		Value=PU_HUE_CONTROL<<8;
		BufferLength = 2;
		break;

	case SATURATION:
		Value=PU_SATURATION_CONTROL<<8;
		BufferLength = 2;
		break;

	case SHARPNESS:
		Value=PU_SHARPNESS_CONTROL<<8;
		BufferLength = 2;
		break;

	case GAMMA:
		Value=PU_GAMMA_CONTROL<<8;
		BufferLength = 2;
		break;

	case WHITEBALANCE:
		Value=PU_WHITE_BALANCE_TEMPERATURE_CONTROL<<8;
		break;

	case WB_COMPONENT:
		Value=PU_WHITE_BALANCE_COMPONENT_CONTROL<<8;
		break;

	case WB_TEMPERATURE_AUTO:
		Value=PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL<<8;
		BufferLength = 1;
		break;

	case WB_COMPONENT_AUTO:
		Value=PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL<<8;
		BufferLength = 1;
		break;

	case BACKLIGHT:
		Value=PU_BACKLIGHT_COMPENSATION_CONTROL<<8;
		break;

	case POWERLINEFREQUENCY:
		Value=PU_POWER_LINE_FREQUENCY_CONTROL<<8;
		BufferLength = 1;
		break;

	case GAIN:
		Value=PU_GAIN_CONTROL<<8;
		break;

	//2010/12/16 03:42と
	case AEPRIORITY:
		Value=CT_AE_PRIORITY_CONTROL<<8;
		BufferLength = 1;
		break;		
	}

	//2011/1/6 11:19と
	if(Id == PAN || Id == TILT)
	{
		struct _PanTilt
		{
			long dwPan;
			long dwTilt;
		} PanTiltValue;

		if(fWrite)
		{
			if(Id == PAN)
			{
				PanTiltValue.dwPan = buffer * 3600;		// james
				PanTiltValue.dwTilt = m_pDevExt->m_PropertyCurrentValues[TILT];
			}
			else 
			{
				PanTiltValue.dwPan = m_pDevExt->m_PropertyCurrentValues[PAN];
				PanTiltValue.dwTilt = buffer * 3600;	// james
			}	
		}

		ntStatus = USB_ControlClassCommand(Request, Value, Index, &PanTiltValue, BufferLength, (!fWrite));
		
		if (NT_SUCCESS(ntStatus))
		{
			m_pDevExt->m_PropertyCurrentValues[PAN] = PanTiltValue.dwPan/3600;
			m_pDevExt->m_PropertyCurrentValues[TILT] = PanTiltValue.dwTilt/3600;
		}
	}
	// shawn 2011/06/29 +++++
	else if (Id == FOCUS_AUTO && fWrite)
	{
		if(m_pDevExt->m_ResetAutoFocus == TRUE)
		{
			m_pDevExt->m_ResetAutoFocus = FALSE;
			buffer = m_pDevExt->m_StreamProperties[FOCUS_AUTO];
		}

		ntStatus = USB_ControlClassCommand(Request, Value, Index, &buffer, BufferLength, (!fWrite));
	}
	else if (Id == FOCUS && fWrite)
	{
		if(m_pDevExt->m_PropertyCurrentValues[FOCUS_AUTO] != KSPROPERTY_CAMERACONTROL_FLAGS_AUTO)
		{
			ntStatus = USB_ControlClassCommand(Request, Value, Index, &buffer, BufferLength, (!fWrite));
		}
	}
	else if (Id == WB_TEMPERATURE_AUTO && fWrite)
	{
		if(m_pDevExt->m_ResetAutoWhiteBalance == TRUE)
		{
			m_pDevExt->m_ResetAutoWhiteBalance = FALSE;
			buffer = m_pDevExt->m_WBTAuto.DefValue;
			DbgPrint("****Set WHITEBALANCETEMP AUTO Current =%d\n",buffer);
		}

		ntStatus = USB_ControlClassCommand(Request, Value, Index, &buffer, BufferLength, (!fWrite));
	}
	else if (Id == WHITEBALANCE && fWrite)
	{
		if (m_pDevExt->m_WBTAuto.CurrentValue != KSPROPERTY_CAMERACONTROL_FLAGS_AUTO)
		{
			ntStatus = USB_ControlClassCommand(Request, Value, Index, &buffer, BufferLength, (!fWrite));
		}
	}
	else if (Id == AE_MODE && fWrite)
	{
		if(m_pDevExt->m_ResetAutoExposure == TRUE)
		{
			m_pDevExt->m_ResetAutoExposure = FALSE;
			buffer = m_pDevExt->m_AEMode.DefValue;
		}

		ntStatus = USB_ControlClassCommand(Request, Value, Index, &buffer, BufferLength, (!fWrite));
	}
	// shawn 2011/06/29 -----
	else if(Id == EXPOSURE && fWrite)
	{
		if(m_pDevExt->m_PropertyCurrentValues[AEMODE] != 8)	// shawn 2011/06/29 add
		{
			LONG exposure_time = 10000 * pow(2.0, (int)buffer);  
			ntStatus = USB_ControlClassCommand(Request, Value, Index, &exposure_time, BufferLength, (!fWrite));
			DBGU_TRACE("EXPOSURE %d\n",exposure_time);
		}
	}
	else
		ntStatus = USB_ControlClassCommand(Request, Value, Index, &buffer, BufferLength, (!fWrite));

	if (!NT_SUCCESS(ntStatus))
		DBGU_WARNING("UpdateImageControlProperties(%d,%d,%d) return 0x%X\n",Id,fWrite,buffer,ntStatus);
	else	// command is success, so save it!
	{
		DBGU_TRACE("UpdateImageControlProperties(%d,%d,%d) return 0x%X\n",Id,fWrite,buffer,ntStatus);
		
		switch (Id)
		{
		case EXPOSURE:
			//2011/1/6 06:13と
			// james 2011/03/23.
			//m_pDevExt->m_PropertyCurrentValues[Id] = GetExposureValue(buffer);
			m_pDevExt->m_PropertyCurrentValues[Id] = fWrite ? buffer : GetExposureValue(buffer);
			break;

		case FOCUS:
		case FOCUS_AUTO:
		case IRIS:
		case ZOOM:
		//case PANTILT:
		case ROLL:
		case PRIVACY:
		case BRIGHTNESS:
		case CONTRAST:
		case HUE:
		case SATURATION:
		case SHARPNESS:
		case GAMMA:
		case WHITEBALANCE:
		case BACKLIGHT:
		case POWERLINEFREQUENCY:
		case GAIN:
		//2010/12/16 03:45と

			m_pDevExt->m_PropertyCurrentValues[Id] = (short)buffer;
			break;

		case AE_MODE:
			m_pDevExt->m_AEMode.CurrentValue = buffer;
			m_pDevExt->m_PropertyCurrentValues[AEMODE] = buffer;
			break;

		case AEPRIORITY:
		case AE_PRIORITY:
			m_pDevExt->m_PropertyCurrentValues[AEPRIORITY] = (short)buffer;
			m_pDevExt->m_AEPriority.CurrentValue = buffer;
			break;

		case WB_COMPONENT:
			m_pDevExt->m_WBC.CurrentValue = buffer;
			break;

		case WB_TEMPERATURE_AUTO:
			m_pDevExt->m_WBTAuto.CurrentValue = buffer;
			break;

		case WB_COMPONENT_AUTO:
			m_pDevExt->m_WBCAuto.CurrentValue = buffer;
			break;

		default:
			break;
		}
	}

	return ntStatus;
}

NTSTATUS
CVideoDevice::UVC_ParseConfigurationDescriptor(
	IN PUSB_CONFIGURATION_DESCRIPTOR configDesc
	)
{
	NTSTATUS	ntStatus=STATUS_SUCCESS;
	BOOL		bFormatEnable=false;
	ULONG		remain = configDesc->wTotalLength;
	UCHAR		length,type,i;
	UCHAR		bVideoCtrl;
	PUCHAR		head = (PUCHAR) configDesc;
	GUID		uncompressedGUID=GUID_NULL;
	UCHAR		bBitPerPixel=16;
	int			nVideoImage=0;
	UCHAR		nVideoImageCnt=0;
	int			nStillImage=0;
	UCHAR		nStillImageCnt=0;
	USHORT		nFrameRateCnt=0;
	ULONG		nFormatIndex=0;
	LONGLONG	SupportedFrameRates[FRAME_RATE_LIST_SIZE];
	UCHAR		m_FrameRatesIndex[MAX_STREAM_FORMAT];
	UCHAR		nStart;
	int			AlternateSettingCnt=-1;
	UCHAR		nExUnit = 0;
	//2011/4/26 05:36と
	int			M2TS_AlternateSettingCnt=-1;
	
	m_pDevExt->m_ExUnitNum = 0;
	m_pDevExt->m_StillCaptureMethod = 0;	// shawn 2011/07/19 add

	RtlZeroMemory(m_pDevExt->m_FrameRateInfo.m_FrameRatesIndex,MAX_STREAM_FORMAT);
	RtlZeroMemory(BusBWArray,sizeof(MaxPktSizePerInterface)*SONIX_BUS_BW_ARRAY_SIZE);
	//2011/4/26 05:43と
	RtlZeroMemory(M2TS_BusBWArray,sizeof(MaxPktSizePerInterface)*SONIX_BUS_BW_ARRAY_SIZE);

	m_pDevExt->m_DefaultFrameIndex[STREAM_Capture] = 1;
	
	m_pDevExt->m_bMPEG2TSExist = FALSE;

	// shawn 2011/06/23 for adding still pin or not +++++
	UNICODE_STRING	BasePath;
	RTL_QUERY_REGISTRY_TABLE *Table=NULL;

	RtlInitUnicodeString(&BasePath, (PWSTR)m_DefValuePath);
	Table = new (NonPagedPool) RTL_QUERY_REGISTRY_TABLE[2];
	RtlZeroMemory(Table, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));

	//StillPin_Enable
	m_pDevExt->m_StillPin_Enable = 0;

	Table[0].Name			= L"StillPin_Enable";
	Table[0].Flags			= RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
	Table[0].EntryContext	= &m_pDevExt->m_StillPin_Enable;
	
	RtlQueryRegistryValues(	RTL_REGISTRY_ABSOLUTE,
							BasePath.Buffer,
							Table,
							NULL, 
							NULL);

	if(Table)
		delete [] Table;
	// shawn 2011/06/23 for adding still pin or not -----
	
	DBGU_TRACE("enter CAM_ParseConfigurationDescriptor remain = %d\n",remain);
	
	while (remain)
	{
		length = head[0];
		type = head[1];
		DBGU_TRACE("DESC, Read length = 0x%X, type = 0x%X\n",length,type);
		
		if (type == USB_CONFIGURATION_DESCRIPTOR_TYPE)	// 0x02
		{
			PUSB_CONFIGURATION_DESCRIPTOR desc = (PUSB_CONFIGURATION_DESCRIPTOR)head;
			m_pDevExt->m_NumOfInterface=desc->bNumInterfaces;
		}
		else if (type == USB_INTERFACE_DESCRIPTOR_TYPE)	// 0x04
		{
			PUSB_INTERFACE_DESCRIPTOR desc = (PUSB_INTERFACE_DESCRIPTOR )head;
			bVideoCtrl = desc->bInterfaceSubClass;
			
			if (bVideoCtrl == 0x02)	// video stream interface
			{
				//2011/4/26 05:36と
				if(m_pDevExt->m_bMPEG2TSExist)
				{
					M2TS_AlternateSettingCnt++;
					M2TS_BusBWArray[M2TS_AlternateSettingCnt].AlternateSetting=desc->bAlternateSetting;
					DBGU_TRACE("M2TS_BusBWArray[%d] Iso AlternateSetting = %d\n",M2TS_AlternateSettingCnt,M2TS_BusBWArray[M2TS_AlternateSettingCnt].AlternateSetting);

				}
				else
				{
					AlternateSettingCnt++;
					BusBWArray[AlternateSettingCnt].AlternateSetting=desc->bAlternateSetting;
					DBGU_TRACE("BusBWArray[%d] Iso AlternateSetting = %d\n",AlternateSettingCnt,BusBWArray[AlternateSettingCnt].AlternateSetting);
				}
			}
		}
		else if (type == USB_ENDPOINT_DESCRIPTOR_TYPE)
		{
			PUSB_ENDPOINT_DESCRIPTOR desc = (PUSB_ENDPOINT_DESCRIPTOR )head;
			
			if (desc->bmAttributes == USB_ENDPOINT_TYPE_BULK)
			{
				DBGU_TRACE("Bulk pipe reported, Enable Bulk Mode\n");
				m_PipeMode = USB_ENDPOINT_TYPE_BULK;
			}
			else if ((desc->bmAttributes&USB_ENDPOINT_TYPE_ISOCHRONOUS) && (M2TS_AlternateSettingCnt>=0))	
			{
				DBGU_TRACE("wMaxPacketSize=0x%X\n",desc->wMaxPacketSize);
				
				//2010/11/29 04:46と
				if(M2TS_BusBWArray[M2TS_AlternateSettingCnt].MaxPktSize != 0)
				{
					if(M2TS_BusBWArray[M2TS_AlternateSettingCnt].MaxPktSize == (desc->wMaxPacketSize&0x7FF))
						M2TS_BusBWArray[M2TS_AlternateSettingCnt].NumberOfTransaction += (1+((desc->wMaxPacketSize>>11)&3));
				}
				else
				{
					M2TS_BusBWArray[M2TS_AlternateSettingCnt].MaxPktSize=(desc->wMaxPacketSize&0x7FF);
					M2TS_BusBWArray[M2TS_AlternateSettingCnt].NumberOfTransaction=1+((desc->wMaxPacketSize>>11)&3);
				}

				DBGU_TRACE("MaxPktSize = %d, NumberOfTransaction=%d\n",
					M2TS_BusBWArray[M2TS_AlternateSettingCnt].MaxPktSize,M2TS_BusBWArray[M2TS_AlternateSettingCnt].NumberOfTransaction);
			}
			else if ((desc->bmAttributes&USB_ENDPOINT_TYPE_ISOCHRONOUS) && (AlternateSettingCnt>=0))
			{
				DBGU_TRACE("wMaxPacketSize=0x%X\n",desc->wMaxPacketSize);
				
				//2010/11/29 04:46と
				if(BusBWArray[AlternateSettingCnt].MaxPktSize != 0)
				{
					if(BusBWArray[AlternateSettingCnt].MaxPktSize == (desc->wMaxPacketSize&0x7FF))
						BusBWArray[AlternateSettingCnt].NumberOfTransaction += (1+((desc->wMaxPacketSize>>11)&3));
				}
				else
				{
					BusBWArray[AlternateSettingCnt].MaxPktSize=(desc->wMaxPacketSize&0x7FF);
					BusBWArray[AlternateSettingCnt].NumberOfTransaction=1+((desc->wMaxPacketSize>>11)&3);
				}
				
				DBGU_TRACE("MaxPktSize = %d, NumberOfTransaction=%d\n",
					BusBWArray[AlternateSettingCnt].MaxPktSize,BusBWArray[AlternateSettingCnt].NumberOfTransaction);
			}
		}
		else if (type == USB_VIDEO_CLASS_INPUT_HEADER_DESCRIPTOR_TYPE)
		{
			DBGU_TRACE("DESC, sub type = 0x%X\n",head[2]);
			
			switch (head[2])
			{
			case 1:
				{
					if (bVideoCtrl == 0x01)
					{
						PVIDCLASS_INTERFACE_HEADER_DESCRIPTOR desc = (PVIDCLASS_INTERFACE_HEADER_DESCRIPTOR)head;
						DBGU_TRACE("==> VIDCLASS_INTERFACE_HEADER_DESCRIPTOR");
						DBGU_TRACE("==> bLength = 0x%X\n",desc->bLength);
						DBGU_TRACE("==> type = 0x%X, subtype = 0x%X\n",desc->bDescriptorType,desc->bDescriptorSubtype);
						DBGU_TRACE("sibtype addr = %X, head[2] = %X\n",&(desc->bDescriptorSubtype),head+2);
						DBGU_TRACE("==> bcdVDC = 0x%X, Addr = %X\n",desc->bcdVDC,&(desc->bcdVDC));
						DBGU_TRACE("head[3] = %X, head[4] = %X\n",head+3,head+4);
						DBGU_TRACE("==> TotalLength = 0x%X\n",desc->wTotalLength);
						DBGU_TRACE("==> clock frequence = 0x%X\n",desc->dwClockFrequency);
						DBGU_TRACE("==> inColection = 0x%X\n",desc->bInCollection);
						m_pDevExt->m_dwClockFrequence=desc->dwClockFrequency;
					}
					else
					{
						PVIDCLASS_VIDEO_INPUT_HEADER_DESCRIPTOR desc = (PVIDCLASS_VIDEO_INPUT_HEADER_DESCRIPTOR)head;
						DBGU_TRACE("==> VIDCLASS_VIDEO_INPUT_HEADER_DESCRIPTOR");
						DBGU_TRACE("==> ");
						
						// shawn 2011/07/19 +++++
						// for fix YouCam connect to device failed issue
						DBGU_TRACE("bStillCaptureMethod = %x", desc->bStillCaptureMethod);

						if (desc->bStillCaptureMethod > m_pDevExt->m_StillCaptureMethod)
							m_pDevExt->m_StillCaptureMethod = desc->bStillCaptureMethod;
						// shawn 2011/07/19 -----
					}
				}

				break;

			case 2:
				{
					if (bVideoCtrl == 0x01)
					{
						PVIDCLASS_INPUT_TERMINAL_DESCRIPTOR desc = (PVIDCLASS_INPUT_TERMINAL_DESCRIPTOR)head;
						DBGU_TRACE("==> VIDCLASS_INPUT_TERMINAL_DESCRIPTOR");
						DBGU_TRACE("==> bmControls = %X\n",desc->bmControls);
						m_pDevExt->m_InputControls=desc->bmControls;
						m_pDevExt->m_InputEntityID=desc->bTerminalID;
					}
				}

				break;

			case 3:
				{
					if (bVideoCtrl == 0x01)
					{
						PVIDCLASS_OUTPUT_TERMINAL_DESCRIPTOR desc = (PVIDCLASS_OUTPUT_TERMINAL_DESCRIPTOR)head;
						DBGU_TRACE("==> VIDCLASS_OUTPUT_TERMINAL_DESCRIPTOR");
						DBGU_TRACE("==> Terminal Type = %X\n",desc->wTerminalType);
					}
					else
					{
						PVIDCLASS_STILL_FRAME_DESCRIPTOR desc = (PVIDCLASS_STILL_FRAME_DESCRIPTOR)head;
						DBGU_TRACE("==> VIDCLASS_STILL_FRAME_DESCRIPTOR %d\n",nStillImage);
						DBGU_TRACE("==> Still Image Numbers = %d\n",desc->bNumImageSizePatterns);

						m_pDevExt->m_DefaultFrameIndex_Still=0;
						m_pDevExt->m_VideoImageFrameNumber_Still=desc->bNumImageSizePatterns;
						nStillImageCnt += desc->bNumImageSizePatterns;

						m_pDevExt->m_StillPin_Enable = TRUE;	// shawn 2011/06/23 for adding still pin or not
						
						for(int i=0;i<desc->bNumImageSizePatterns;i++)
						{
							m_pDevExt->m_drStillImageFormat[nStillImage].DataRange.FormatSize=sizeof (KS_DATARANGE_VIDEO);
							m_pDevExt->m_drStillImageFormat[nStillImage].DataRange.Flags=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].DataRange.Reserved=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].DataRange.MajorFormat=KSDATAFORMAT_TYPE_VIDEO;
							m_pDevExt->m_drStillImageFormat[nStillImage].DataRange.SubFormat = uncompressedGUID;
							m_pDevExt->m_drStillImageFormat[nStillImage].DataRange.Specifier=KSDATAFORMAT_SPECIFIER_VIDEOINFO;

							m_pDevExt->m_drStillImageFormat[nStillImage].bFixedSizeSamples=TRUE;
							m_pDevExt->m_drStillImageFormat[nStillImage].bTemporalCompression=FALSE;
							m_pDevExt->m_drStillImageFormat[nStillImage].StreamDescriptionFlags=KS_VIDEOSTREAM_CAPTURE;
							m_pDevExt->m_drStillImageFormat[nStillImage].MemoryAllocationFlags=0;

							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.guid=KSDATAFORMAT_SPECIFIER_VIDEOINFO;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.VideoStandard=KS_AnalogVideo_None;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.CropAlignX=1;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.CropAlignY=1;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.CropGranularityX=1;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.CropGranularityY=1;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.OutputGranularityX=1;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.OutputGranularityY=1;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.ShrinkTapsX=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.ShrinkTapsY=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.StretchTapsX=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.StretchTapsY=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.InputSize.cx=desc->pImageSizePattern[i*2];
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.InputSize.cy=desc->pImageSizePattern[i*2+1];
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.MinCroppingSize.cx=desc->pImageSizePattern[i*2];
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.MinCroppingSize.cy=desc->pImageSizePattern[i*2+1];
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.MaxCroppingSize.cx=desc->pImageSizePattern[i*2];
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.MaxCroppingSize.cy=desc->pImageSizePattern[i*2+1];
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.MinOutputSize.cx=desc->pImageSizePattern[i*2];
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.MinOutputSize.cy=desc->pImageSizePattern[i*2+1];
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.MaxOutputSize.cx=desc->pImageSizePattern[i*2];
							m_pDevExt->m_drStillImageFormat[nStillImage].ConfigCaps.MaxOutputSize.cy=desc->pImageSizePattern[i*2+1];

							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.rcSource.left=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.rcSource.right=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.rcSource.bottom=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.rcSource.top=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.rcTarget.left=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.rcTarget.right=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.rcTarget.bottom=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.rcTarget.top=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.dwBitErrorRate=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biSize=sizeof (KS_BITMAPINFOHEADER);
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biWidth=desc->pImageSizePattern[i*2];
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biHeight=desc->pImageSizePattern[i*2+1];
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biPlanes=1;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biCompression=uncompressedGUID.Data1;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biXPelsPerMeter=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biYPelsPerMeter=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biClrUsed=0;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biClrImportant=0;

							//RBK fix WHCK usage indicator test fail while still pin is declaired 20130516 +++
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.AvgTimePerFrame = 10000000;
							m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.dwBitRate = desc->pImageSizePattern[i*2]*desc->pImageSizePattern[i*2+1]*2*8;
							m_pDevExt->m_drStillImageFormat[nStillImage].StreamDescriptionFlags=0;
							//RBK fix WHCK usage indicator test fail while still pin is declaired 20130516 ---
							//2010/8/4 11:34と
							m_pDevExt->StillFormatArray[nStillImage].bFormatIndex = nFormatIndex;//nStillImage+1;
							m_pDevExt->StillFormatArray[nStillImage].biCompression = uncompressedGUID.Data1;
							m_pDevExt->StillFormatArray[nStillImage].bFrameIndex = i+1;
							m_pDevExt->StillFormatArray[nStillImage].ImageSize = m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biSizeImage;
							m_pDevExt->StillFormatArray[nStillImage].wWidth = desc->pImageSizePattern[i*2];
							m_pDevExt->StillFormatArray[nStillImage].wHeight = desc->pImageSizePattern[i*2+1];
							
							if (uncompressedGUID.Data1 == MEDIASUBTYPE_YUY2.Data1)
								m_pDevExt->StillFormatArray[nStillImage].ImageFormat = IMG_FMT_YUY2;
							else
								m_pDevExt->StillFormatArray[nStillImage].ImageFormat = IMG_FMT_MJPEG;

							if(uncompressedGUID.Data1 == MEDIASUBTYPE_YUY2.Data1)
							{
								m_pDevExt->m_drStillImageFormat[nStillImage].DataRange.SampleSize=2*desc->pImageSizePattern[i*2]*desc->pImageSizePattern[i*2+1];
								m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biSizeImage=2*desc->pImageSizePattern[i*2]*desc->pImageSizePattern[i*2+1];
								m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biBitCount=16;
								m_pDevExt->StillFormatArray[nStillImage].ImageFormat = IMG_FMT_YUY2;
							}
							else
							{
								m_pDevExt->m_drStillImageFormat[nStillImage].DataRange.SampleSize=3*desc->pImageSizePattern[i*2]*desc->pImageSizePattern[i*2+1];
								m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biSizeImage=3*desc->pImageSizePattern[i*2]*desc->pImageSizePattern[i*2+1];
								m_pDevExt->m_drStillImageFormat[nStillImage].VideoInfoHeader.bmiHeader.biBitCount=24;				
								m_pDevExt->StillFormatArray[nStillImage].ImageFormat = IMG_FMT_MJPEG;
							}

							nStillImage++;
						}
					}
				}

				break;

			case 4:
				{
					if (bVideoCtrl == 0x02)
					{
						PVIDCLASS_UNCOMPRESSED_FORMAT_DESCRIPTOR desc = (PVIDCLASS_UNCOMPRESSED_FORMAT_DESCRIPTOR)head;
						DBGU_TRACE("==> VIDCLASS_UNCOMPRESSED_FORMAT_DESCRIPTOR");
						DBGU_TRACE("==> guidFormat: %X %X %X %X\n",desc->guidFormat.Data1,
							desc->guidFormat.Data2,desc->guidFormat.Data3,desc->guidFormat.Data4[0]);
						uncompressedGUID=desc->guidFormat;
						bBitPerPixel=desc->bBitPerPixel;
						nStart = 0;
						nFormatIndex = desc->bFormatIndex;
						
						if (bFormatEnable)
							nVideoImageCnt+=desc->bNumFrameDescriptors;
						
						DBGU_TRACE("desc->bNumFrameDescriptors = %d\n",desc->bNumFrameDescriptors);
						m_pDevExt->m_YUY2_FormatCount = desc->bNumFrameDescriptors;
						m_pDevExt->m_VideoImageFrameNumber[STREAM_Capture]=desc->bNumFrameDescriptors;
						m_pDevExt->m_DefaultFrameIndex[STREAM_Capture]=desc->bDefaultFrameIndex;
					}
				}

				break;

			case 5:
				{
					bFormatEnable = true;
					
					if (bVideoCtrl == 0x01)
					{
						PVIDCLASS_PROCESSING_UNIT_DESCRIPTOR desc = (PVIDCLASS_PROCESSING_UNIT_DESCRIPTOR)head;
						DBGU_TRACE("==> VIDCLASS_PROCESSING_UNIT_DESCRIPTOR");
						DBGU_TRACE("==> bmControls = %X\n",desc->bmControls);
						m_pDevExt->m_PropertyControls=desc->bmControls;
						m_pDevExt->m_PropertyEntityID=desc->bUnitID;
					}
					else
					{
						PVIDCLASS_UNCOMPRESSED_FRAME_DESCRIPTOR desc = (PVIDCLASS_UNCOMPRESSED_FRAME_DESCRIPTOR)head;
						DBGU_TRACE("==> VIDCLASS_UNCOMPRESSED_FRAME_DESCRIPTOR =%d\n",nVideoImage);
						DBGU_TRACE("==> ID=%d, %d x %d, FrameIntervaltype = %d\n",desc->bFrameIndex,
							desc->wWidth,desc->wHeight,desc->bFrameIntervalType);
							
						m_pDevExt->m_drImageFormat[nVideoImage].DataRange.FormatSize=sizeof (KS_DATARANGE_VIDEO);
						m_pDevExt->m_drImageFormat[nVideoImage].DataRange.Flags=0;
						
						m_pDevExt->m_drImageFormat[nVideoImage].DataRange.SampleSize = desc->dwMaxVideoFrameBufferSize;
						m_pDevExt->m_drImageFormat[nVideoImage].DataRange.Reserved=0;
						m_pDevExt->m_drImageFormat[nVideoImage].DataRange.MajorFormat=KSDATAFORMAT_TYPE_VIDEO;
						m_pDevExt->m_drImageFormat[nVideoImage].DataRange.SubFormat = uncompressedGUID;
						m_pDevExt->m_drImageFormat[nVideoImage].DataRange.Specifier=KSDATAFORMAT_SPECIFIER_VIDEOINFO;

						m_pDevExt->m_drImageFormat[nVideoImage].bFixedSizeSamples=TRUE;
						m_pDevExt->m_drImageFormat[nVideoImage].bTemporalCompression=FALSE;
						m_pDevExt->m_drImageFormat[nVideoImage].StreamDescriptionFlags=KS_VIDEOSTREAM_CAPTURE;
						m_pDevExt->m_drImageFormat[nVideoImage].MemoryAllocationFlags=0;

						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.guid=KSDATAFORMAT_SPECIFIER_VIDEOINFO;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.VideoStandard=KS_AnalogVideo_None;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.CropAlignX=1;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.CropAlignY=1;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.CropGranularityX=1;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.CropGranularityY=1;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.OutputGranularityX=1;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.OutputGranularityY=1;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.ShrinkTapsX=0;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.ShrinkTapsY=0;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.StretchTapsX=0;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.StretchTapsY=0;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.InputSize.cx=desc->wWidth;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.InputSize.cy=desc->wHeight;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinCroppingSize.cx=desc->wWidth;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinCroppingSize.cy=desc->wHeight;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxCroppingSize.cx=desc->wWidth;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxCroppingSize.cy=desc->wHeight;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinOutputSize.cx=desc->wWidth;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinOutputSize.cy=desc->wHeight;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxOutputSize.cx=desc->wWidth;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxOutputSize.cy=desc->wHeight;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinBitsPerSecond=desc->dwMinBitRate;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxBitsPerSecond=desc->dwMaxBitRate;
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinFrameInterval=desc->dwFrameInterval[0];
						m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxFrameInterval=desc->dwFrameInterval[desc->bFrameIntervalType-1];

						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcSource.left=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcSource.right=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcSource.bottom=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcSource.top=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcTarget.left=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcTarget.right=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcTarget.bottom=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcTarget.top=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.dwBitRate=desc->dwMaxBitRate;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.dwBitErrorRate=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.AvgTimePerFrame=desc->dwFrameInterval[0];
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biSize=sizeof (KS_BITMAPINFOHEADER);
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biWidth=desc->wWidth;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biHeight=desc->wHeight;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biPlanes=1;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biBitCount=bBitPerPixel;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biCompression=uncompressedGUID.Data1;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biSizeImage = desc->dwMaxVideoFrameBufferSize;
						
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biXPelsPerMeter=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biYPelsPerMeter=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biClrUsed=0;
						m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biClrImportant=0;

						m_pDevExt->StreamFormatArray[nVideoImage].bFormatIndex = nFormatIndex;
						m_pDevExt->StreamFormatArray[nVideoImage].biCompression = uncompressedGUID.Data1;
						m_pDevExt->StreamFormatArray[nVideoImage].bFrameIndex = desc->bFrameIndex;
						m_pDevExt->StreamFormatArray[nVideoImage].ImageSize = m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biSizeImage;
						
						m_pDevExt->StreamFormatArray[nVideoImage].wWidth = desc->wWidth;
						m_pDevExt->StreamFormatArray[nVideoImage].wHeight = desc->wHeight;
						m_pDevExt->StreamFormatArray[nVideoImage].ImageFormat = IMG_FMT_YUY2;
						//2010/5/13 02:37と
						m_pDevExt->StreamFormatArray[nVideoImage].FrameRatesCount = desc->bFrameIntervalType;
						
						for(i=0;i<desc->bFrameIntervalType;i++)
						{
							m_pDevExt->StreamFormatArray[nVideoImage].SupportedFrameRates[i] = desc->dwFrameInterval[i];
							DBGU_TRACE("Format[%d]  X= %d Y= %d SupportedFrameRates=%d\n",nVideoImage,m_pDevExt->StreamFormatArray[nVideoImage].wWidth, 
							m_pDevExt->StreamFormatArray[nVideoImage].wHeight, m_pDevExt->StreamFormatArray[nVideoImage].SupportedFrameRates[i]);
						}

						if (nVideoImage<m_pDevExt->m_DefaultFrameIndex[STREAM_Capture])
						{
							// all copy to temp..
							for (i=0;i<desc->bFrameIntervalType;i++)
								SupportedFrameRates[nFrameRateCnt+i]=desc->dwFrameInterval[i];

							nVideoImage++;

							// write frame rate settings end ..
							m_FrameRatesIndex[nVideoImage] = nFrameRateCnt+desc->bFrameIntervalType;
							nFrameRateCnt += desc->bFrameIntervalType;
						}
						else if (nVideoImage==m_pDevExt->m_DefaultFrameIndex[STREAM_Capture])
						{
							for (i=0;i<desc->bFrameIntervalType;i++)
								m_pDevExt->m_FrameRateInfo.SupportedFrameRates[i]=desc->dwFrameInterval[i];

							// write frame rate settings end ..
							m_pDevExt->m_FrameRateInfo.m_FrameRatesIndex[1+nStart] = desc->bFrameIntervalType;

							// Copy back
							for (i=0;i<nFrameRateCnt;i++)
							{
								m_pDevExt->m_FrameRateInfo.SupportedFrameRates[i+desc->bFrameIntervalType]=SupportedFrameRates[i];
							}

							for (i=0;i<m_pDevExt->m_DefaultFrameIndex[STREAM_Capture];i++)
							{
								m_pDevExt->m_FrameRateInfo.m_FrameRatesIndex[2+nStart+i]=desc->bFrameIntervalType+m_FrameRatesIndex[i+1];
							}

							nFrameRateCnt += desc->bFrameIntervalType;
							nVideoImage++;
						}
						else
						{
							// write frame rate settings start ..
							for (i=0;i<desc->bFrameIntervalType;i++)
								m_pDevExt->m_FrameRateInfo.SupportedFrameRates[nFrameRateCnt+i]=desc->dwFrameInterval[i];

							nVideoImage++;

							// write frame rate settings end ..
							m_pDevExt->m_FrameRateInfo.m_FrameRatesIndex[nVideoImage] = nFrameRateCnt+desc->bFrameIntervalType;
							nFrameRateCnt += desc->bFrameIntervalType;
						}
						m_pDevExt->m_YUY2_FrmaeRateCount = nFrameRateCnt;
					}
				}

				break;

			case 6:
				if(bVideoCtrl == 2) // video stream interface
				{
					PVIDCLASS_MJPEG_FORMAT_DESCRIPTOR desc = (PVIDCLASS_MJPEG_FORMAT_DESCRIPTOR)head;
					DBGU_TRACE("==> VIDCLASS_UNCOMPRESSED_FORMAT_DESCRIPTOR");
					nFormatIndex = desc->bFormatIndex;
					nStart = nVideoImageCnt;
					uncompressedGUID=MEDIASUBTYPE_MJPG;
					
					if (bFormatEnable)
						nVideoImageCnt+=desc->bNumFrameDescriptors;
					
					DBGU_TRACE("desc->bNumFrameDescriptors = %d\n",desc->bNumFrameDescriptors);
					m_pDevExt->m_VideoImageFrameNumber_Still=desc->bNumFrameDescriptors;
					m_pDevExt->m_DefaultFrameIndex_Still=desc->bDefaultFrameIndex;
				}
				else if(bVideoCtrl == 1)//video control interface
				{
					PVIDCLASS_EXTENSION_UNIT_DESCRIPTOR desc = (PVIDCLASS_EXTENSION_UNIT_DESCRIPTOR)head;
					DBGU_TRACE("==> VIDCLASS_EXTENSION_UNIT_DESCRIPTOR");
					m_pDevExt->m_ExUnitControls[nExUnit] = desc->bmControls;
					//DBGU_TRACE("==> bmControls = %X\n",m_pDevExt->m_ExUnitControls[nExUnit]);
					RtlCopyMemory(&m_pDevExt->m_ExUnitGUID[nExUnit], &desc->guidExtensionCode, sizeof(GUID));
					DBGU_TRACE("ExUnitGUID: %X %X %X %X",m_pDevExt->m_ExUnitGUID[nExUnit].Data1,
						m_pDevExt->m_ExUnitGUID[nExUnit].Data2,
						m_pDevExt->m_ExUnitGUID[nExUnit].Data3,m_pDevExt->m_ExUnitGUID[nExUnit].Data4);
					//2010/4/29 05:59と
					//RtlCopyMemory(&CaptureFilterNodeDescriptors[0].Name, &desc->guidExtensionCode, sizeof(GUID));
					DBGU_TRACE("Michael Copy EX unit GUID to Node\n");
					
					m_pDevExt->m_ExUnitID[nExUnit] = desc->bUnitID;
					DBGU_TRACE("m_ExUnitID = %d\n",m_pDevExt->m_ExUnitID[nExUnit]);
					USHORT ParamLen = 0;
					ULONG Value;
					ULONG Index;
					ULONG BufferLength;
					USHORT PropertyCount = 0;
					//2010/4/30 01:51と
					/*UCHAR	ControlSize;
					NTSTATUS ExUnit_ntStatus=STATUS_SUCCESS;
					
					ControlSize = desc->bControlSize;
					
					for(int i=0;i<ControlSize;i++)
					{
						m_pDevExt->m_ExUnitControls[nExUnit][i] = desc->bmControls[i];
						DBGU_TRACE("==> bmControls = %X\n",m_pDevExt->m_ExUnitControls[nExUnit][i]);
					}	
						
					for(int j=0;j<ControlSize;j++)	
						for(int i=0;i<MAX_XU_KSAUTOMATION_PROPERTIES;i++)
						{
							if(m_pDevExt->m_ExUnitControls[nExUnit][j]&(1<<i))
							{
								Value = (8*j+i+1)<<8;
								Index = m_pDevExt->m_ExUnitID[nExUnit]<<8;
								BufferLength = 2;
			
								ExUnit_ntStatus = USB_ControlClassCommand(
										GET_LEN,
										Value,
										Index,
										&ParamLen,
										BufferLength,
										TRUE);
						
								if(NT_SUCCESS(ExUnit_ntStatus)) 
								{
									m_pDevExt->m_ExUnitParamLength[nExUnit][8*j+i] = ParamLen;
									DBGU_TRACE("i=%d nExUnit=%d ParamLen = %d\n",i,nExUnit,m_pDevExt->m_ExUnitParamLength[nExUnit][8*j+i]);
									ExtensionControlProperties[nExUnit][PropertyCount] = ExtensionControlProperty[i];
									//ExtensionControlProperties[nExUnit][PropertyCount].PropertyId = 8*j+i+1;
									ExtensionControlProperties[nExUnit][PropertyCount].MinProperty = sizeof(KSPROPERTY);
									ExtensionControlProperties[nExUnit][PropertyCount].MinData = ParamLen;
		
									PropertyCount++;
								}
								else
								{
									DbgPrint("Get Exunit length fail = %X\n",ExUnit_ntStatus);
									//deviceContext->m_ExUnitControlParamLength[nExUnit][i] = 0;
								}
							}
						}
					
					DBGU_TRACE("PropertyCount = %d\n",PropertyCount);
					DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].Set = &m_pDevExt->m_ExUnitGUID[nExUnit];
					DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].PropertiesCount = PropertyCount,
					DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].PropertyItem = &ExtensionControlProperties[nExUnit][0];
					DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].FastIoCount = 0;
					DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].FastIoTable = NULL;
					*/
					for(int i=0;i<MAX_XU_KSAUTOMATION_PROPERTIES;i++)
					{
						if(m_pDevExt->m_ExUnitControls[nExUnit]&(1<<i))
						{
							Value = (i+1)<<8;
							Index = m_pDevExt->m_ExUnitID[nExUnit]<<8;
							BufferLength = 2;
							ParamLen=0;

							ntStatus = USB_ControlClassCommand(
								GET_LEN,
								Value,
								Index,
								&ParamLen,
								BufferLength,
								TRUE);

							if( NT_SUCCESS(ntStatus)) 
							{
								m_pDevExt->m_ExUnitParamLength[nExUnit][i] = ParamLen;
								DBGU_TRACE("i=%d nExUnit=%d ParamLen = %d\n",i,nExUnit,m_pDevExt->m_ExUnitParamLength[nExUnit][i]);
								m_pDevExt->m_ExtensionControlProperties[nExUnit][PropertyCount] = ExtensionControlProperty[i];

								//ExtensionControlProperties[nExUnit][PropertyCount].PropertyId = i+1;

								m_pDevExt->m_ExtensionControlProperties[nExUnit][PropertyCount].MinProperty = sizeof(KSPROPERTY);
								//ExtensionControlProperties[nExUnit][PropertyCount].MinData = ParamLen;

								PropertyCount++;
							}
							else
							{
								DBGU_TRACE("Get Exunit length fail = %X\n",ntStatus);
								m_pDevExt->m_ExUnitParamLength[nExUnit][i] = sizeof(ULONG);
							}
						}
					}

					DBGU_TRACE("PropertyCount = %d\n",PropertyCount);
					
					// shawn 2011/06/21 modify for multi-devices +++++
					/*DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].Set = &m_pDevExt->m_ExUnitGUID[nExUnit];
					DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].PropertiesCount = PropertyCount,
					DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].PropertyItem = ExtensionControlProperties[nExUnit];
					DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].FastIoCount = 0;
					DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].FastIoTable = NULL;*/

					m_pDevExt->m_DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].Set = &m_pDevExt->m_ExUnitGUID[nExUnit];
					m_pDevExt->m_DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].PropertiesCount = PropertyCount,
					m_pDevExt->m_DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].PropertyItem = m_pDevExt->m_ExtensionControlProperties[nExUnit];
					m_pDevExt->m_DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].FastIoCount = 0;
					m_pDevExt->m_DevicePropertySetsTable[DEFAULT_KSAUTOMATION_PROPERTIES+nExUnit].FastIoTable = NULL;
					// shawn 2011/06/21 modify for multi-devices -----
					
					//2010/10/5 05:44と
					//2010/4/29 07:47と
					m_pDevExt->m_ExUnitPropertySetsTable[nExUnit].Set = &m_pDevExt->m_ExUnitGUID[nExUnit];
					m_pDevExt->m_ExUnitPropertySetsTable[nExUnit].PropertiesCount = PropertyCount,
					m_pDevExt->m_ExUnitPropertySetsTable[nExUnit].PropertyItem = m_pDevExt->m_ExtensionControlProperties[nExUnit];
					m_pDevExt->m_ExUnitPropertySetsTable[nExUnit].FastIoCount = 0;
					m_pDevExt->m_ExUnitPropertySetsTable[nExUnit].FastIoTable = NULL;
					
					//if F/W report it is supported
					if (PropertyCount > 0)
						nExUnit++;
				}

				break;

			case 7:
				{
					PVIDCLASS_MPEG_FRAME_DESCRIPTOR desc = (PVIDCLASS_MPEG_FRAME_DESCRIPTOR)head;
					DBGU_TRACE("==> VIDCLASS_MPEG_FRAME_DESCRIPTOR = %d\n",nVideoImage);
					DBGU_TRACE("==> ID=%d, %d x %d, FrameIntervaltype = %d\n",desc->bFrameIndex,
					desc->wWidth,desc->wHeight,desc->bFrameIntervalType);

					m_pDevExt->m_drImageFormat[nVideoImage].DataRange.FormatSize=sizeof (KS_DATARANGE_VIDEO);
					m_pDevExt->m_drImageFormat[nVideoImage].DataRange.Flags=0;
					m_pDevExt->m_drImageFormat[nVideoImage].DataRange.SampleSize = desc->dwMaxVideoFrameBufferSize;
					m_pDevExt->m_drImageFormat[nVideoImage].DataRange.Reserved=0;
					m_pDevExt->m_drImageFormat[nVideoImage].DataRange.MajorFormat=KSDATAFORMAT_TYPE_VIDEO;
					m_pDevExt->m_drImageFormat[nVideoImage].DataRange.SubFormat = uncompressedGUID;
					m_pDevExt->m_drImageFormat[nVideoImage].DataRange.Specifier=KSDATAFORMAT_SPECIFIER_VIDEOINFO;

					m_pDevExt->m_drImageFormat[nVideoImage].bFixedSizeSamples=FALSE;
					m_pDevExt->m_drImageFormat[nVideoImage].bTemporalCompression=FALSE;
					m_pDevExt->m_drImageFormat[nVideoImage].StreamDescriptionFlags=KS_VIDEOSTREAM_CAPTURE;
					m_pDevExt->m_drImageFormat[nVideoImage].MemoryAllocationFlags=0;

					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.guid=KSDATAFORMAT_SPECIFIER_VIDEOINFO;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.VideoStandard=KS_AnalogVideo_None;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.CropAlignX=1;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.CropAlignY=1;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.CropGranularityX=1;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.CropGranularityY=1;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.OutputGranularityX=1;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.OutputGranularityY=1;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.ShrinkTapsX=0;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.ShrinkTapsY=0;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.StretchTapsX=0;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.StretchTapsY=0;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.InputSize.cx=desc->wWidth;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.InputSize.cy=desc->wHeight;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinCroppingSize.cx=desc->wWidth;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinCroppingSize.cy=desc->wHeight;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxCroppingSize.cx=desc->wWidth;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxCroppingSize.cy=desc->wHeight;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinOutputSize.cx=desc->wWidth;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinOutputSize.cy=desc->wHeight;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxOutputSize.cx=desc->wWidth;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxOutputSize.cy=desc->wHeight;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinBitsPerSecond=desc->dwMinBitRate;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxBitsPerSecond=desc->dwMaxBitRate;
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MinFrameInterval=desc->dwFrameInterval[0];
					m_pDevExt->m_drImageFormat[nVideoImage].ConfigCaps.MaxFrameInterval=desc->dwFrameInterval[desc->bFrameIntervalType-1];
					
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcSource.left=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcSource.right=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcSource.bottom=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcSource.top=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcTarget.left=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcTarget.right=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcTarget.bottom=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.rcTarget.top=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.dwBitRate=desc->dwMaxBitRate;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.dwBitErrorRate=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.AvgTimePerFrame=desc->dwFrameInterval[0];
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biSize=sizeof (KS_BITMAPINFOHEADER);
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biWidth=desc->wWidth;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biHeight=desc->wHeight;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biPlanes=1;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biBitCount=bBitPerPixel;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biBitCount=24;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biCompression=uncompressedGUID.Data1;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biSizeImage=desc->dwMaxVideoFrameBufferSize;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biXPelsPerMeter=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biYPelsPerMeter=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biClrUsed=0;
					m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biClrImportant=0;

					m_pDevExt->StreamFormatArray[nVideoImage].bFormatIndex = nFormatIndex;
					m_pDevExt->StreamFormatArray[nVideoImage].biCompression = uncompressedGUID.Data1;
					m_pDevExt->StreamFormatArray[nVideoImage].bFrameIndex = desc->bFrameIndex;
					m_pDevExt->StreamFormatArray[nVideoImage].ImageSize = m_pDevExt->m_drImageFormat[nVideoImage].VideoInfoHeader.bmiHeader.biSizeImage;
					m_pDevExt->StreamFormatArray[nVideoImage].wWidth = desc->wWidth;
					m_pDevExt->StreamFormatArray[nVideoImage].wHeight = desc->wHeight;
					m_pDevExt->StreamFormatArray[nVideoImage].ImageFormat = IMG_FMT_MJPEG;
					//2010/5/13 02:37と
					m_pDevExt->StreamFormatArray[nVideoImage].FrameRatesCount = desc->bFrameIntervalType;
					
					for(i=0;i<desc->bFrameIntervalType;i++)
					{
						m_pDevExt->StreamFormatArray[nVideoImage].SupportedFrameRates[i] = desc->dwFrameInterval[i];
						DBGU_TRACE("Format[%d]  X= %d Y= %d SupportedFrameRates=%d\n",nVideoImage,m_pDevExt->StreamFormatArray[nVideoImage].wWidth, 
						m_pDevExt->StreamFormatArray[nVideoImage].wHeight, m_pDevExt->StreamFormatArray[nVideoImage].SupportedFrameRates[i]);
					}

					if (nVideoImage<m_pDevExt->m_DefaultFrameIndex_Still)
					{
						// all copy to temp..
						for (i=0;i<desc->bFrameIntervalType;i++)
							SupportedFrameRates[nFrameRateCnt+i]=desc->dwFrameInterval[i];

						nVideoImage++;

						// write frame rate settings end ..
						m_FrameRatesIndex[nVideoImage] = nFrameRateCnt+desc->bFrameIntervalType;
						nFrameRateCnt += desc->bFrameIntervalType;
					}
					else if (nVideoImage==m_pDevExt->m_DefaultFrameIndex_Still)
					{
						for (i=0;i<desc->bFrameIntervalType;i++)
							m_pDevExt->m_FrameRateInfo.SupportedFrameRates[i]=desc->dwFrameInterval[i];

						// write frame rate settings end ..
						m_pDevExt->m_FrameRateInfo.m_FrameRatesIndex[1+nStart] = desc->bFrameIntervalType;

						// Copy back
						for (i=0;i<nFrameRateCnt;i++)
						{
							m_pDevExt->m_FrameRateInfo.SupportedFrameRates[i+desc->bFrameIntervalType]=SupportedFrameRates[i];
						}

						//2010/5/10 11:59と
						for (i=0;i<m_pDevExt->m_DefaultFrameIndex_Still;i++)
						//for (i=0;i<m_pDevExt->m_DefaultFrameIndex[0];i++)
						{
							m_pDevExt->m_FrameRateInfo.m_FrameRatesIndex[2+nStart+i]=desc->bFrameIntervalType+m_FrameRatesIndex[i+1];
						}

						nFrameRateCnt += desc->bFrameIntervalType;
						nVideoImage++;
					}
					else
					{
						// write frame rate settings start ..
						for (i=0;i<desc->bFrameIntervalType;i++)
							m_pDevExt->m_FrameRateInfo.SupportedFrameRates[nFrameRateCnt+i]=desc->dwFrameInterval[i];

						nVideoImage++;

						// write frame rate settings end ..
						m_pDevExt->m_FrameRateInfo.m_FrameRatesIndex[nVideoImage] = nFrameRateCnt+desc->bFrameIntervalType;
						nFrameRateCnt += desc->bFrameIntervalType;
					}
				}

				break;

			case 0x0a:	// james try M2TS.
				{
					PVIDCLASS_M2TS_FORMAT_DESCRIPTOR desc = (PVIDCLASS_M2TS_FORMAT_DESCRIPTOR)head;
					DBGU_TRACE("==> VIDCLASS_M2TS_FORMAT_DESCRIPTOR\n");
					DBGU_TRACE("bFormatIndex = 0x%x, bDataOffset = 0x%x, bPacketLength = 0x%x, bStrideLength = 0x%x", 
						desc->bFormatIndex, desc->bDataOffset, desc->bPacketLength, desc->bStrideLength);
					
					//2011/5/13 05:56と
					m_pDevExt->m_bMPEG2TSExist = TRUE;
					TSMuxEnable = TRUE;
					KeInitializeSemaphore(&TsMuxSemaphore,0,MAXLONG);
					
					m_pDevExt->m_drImageFormat_MP2TS[0].DataRange.FormatSize=sizeof (KSDATAFORMAT);
					m_pDevExt->m_drImageFormat_MP2TS[0].DataRange.Flags=0;
					//m_pDevExt->m_drImageFormat_MP2TS[0].DataRange.SampleSize = 64 * 1024;//desc->dwMaxVideoFrameBufferSize;
					m_pDevExt->m_drImageFormat_MP2TS[0].DataRange.SampleSize = 0x10000000;//desc->dwMaxVideoFrameBufferSize;
					m_pDevExt->m_drImageFormat_MP2TS[0].DataRange.Reserved=0;
					m_pDevExt->m_drImageFormat_MP2TS[0].DataRange.MajorFormat=KSDATAFORMAT_TYPE_STREAM;
					m_pDevExt->m_drImageFormat_MP2TS[0].DataRange.SubFormat =KSDATAFORMAT_TYPE_MPEG2_TRANSPORT;// uncompressedGUID;
					m_pDevExt->m_drImageFormat_MP2TS[0].DataRange.Specifier=KSDATAFORMAT_SPECIFIER_NONE;
					m_pDevExt->m_drImageFormat_MP2TS[0].VideoInfoHeader.bmiHeader.biCompression=mmioFOURCC('H','2','6','4');//uncompressedGUID.Data1;
					/*											
					StreamFormatsArray_MP2TS[0] = (PKSDATAFORMAT) &drImageFormat_MP2TS[0];	
					CaptureFilterPinDescriptors[STREAM_Capture_MP2TS].PinDescriptor.DataRangesCount = 1;
					Streams[STREAM_Capture_MP2TS].NumberOfFormatArrayEntries = 1;
					*/
				}

				break;
			}
		}

		head += length;
		remain -= length;
	}

	// add 1 for iso 0 bandwidth
	BUS_BW_ARRAY_SIZE = AlternateSettingCnt+1;

	//
	// Change filter descriptor before create it. This is for extension unit
	// purpose. Carefully about the numbers.
	//
	m_pDevExt->m_ExUnitNum = nExUnit;
	m_pDevExt->m_FilterAutomationTable.PropertySetsCount = DEFAULT_KSAUTOMATION_PROPERTIES+m_pDevExt->m_ExUnitNum;


	//RBK , fix pin descroptor error
	USHORT usPinCount = 1; // capture
	if (m_pDevExt->m_StillPin_Enable ^ m_pDevExt->m_bMPEG2TSExist)  // add one more pin, it is xor 
		usPinCount++;
	if (m_pDevExt->m_StillPin_Enable && m_pDevExt->m_bMPEG2TSExist) // add one more pin, if both exist
		usPinCount++;

	m_pDevExt->m_CaptureFilterDescriptor.PinDescriptorsCount = usPinCount;

	DBGU_TRACE("new PropertySetsCount = %d\n", FilterAutomationTable.PropertySetsCount);

	// save to device extension
	m_pDevExt->m_VideoImageFormatCount = nVideoImageCnt;
	m_pDevExt->m_StillImageFromatCount = nStillImageCnt;
	m_pDevExt->m_VideoImageFrameNumber[STREAM_Capture] = nVideoImageCnt;
	m_pDevExt->m_VideoImageFrameNumber_Still = nStillImageCnt;
#if DBG
	DBGU_TRACE("==> Number of Video Format=%d, Still Format=%d\n",nVideoImageCnt, nStillImageCnt);
	
	for (i=0;i<nFrameRateCnt;i++)
		DBGU_TRACE("SupportedFrameRates[%d]=0x%X\n",i,m_pDevExt->m_FrameRateInfo.SupportedFrameRates[i]);
	
	for (i=0;i<MAX_STREAM_FORMAT;i++)
	{
		DBGU_TRACE("m_FrameRatesIndex[%d]=%d\n",i,m_pDevExt->m_FrameRateInfo.m_FrameRatesIndex[i]);
	}
#endif
	DBGU_TRACE("exit CAM_ParseConfigurationDescriptor : %d\n",remain);
	return STATUS_SUCCESS;
}

void EventHandlerRoutine(IN CVideoDevice *pVideoDevice)
{
	DBGU_FN("EventHandlerRoutine()");
	//2010/8/27 11:01と mark
	//	KeSetPriorityThread(KeGetCurrentThread(),LOW_REALTIME_PRIORITY);

	if (pVideoDevice) {
		pVideoDevice->EventHandler();
	}

	//2010/8/26 03:16と
	DBGU_TRACE("Terminate EventHandlerRoutine()\n");
	pVideoDevice->m_EventHandlerShutdown = FALSE;

	PsTerminateSystemThread(STATUS_SUCCESS);
}

/*++

Routine Description:

    Provide basic functino to communicate with device.

Arguments:

    Control -
        read when control = 0, and write when control = 1
	Number -
		The size of pData
	Address - 
		The ASIC address for read/write
	pData - 
		The input buffer pointer

Return Value:

    NTSTATUS code

--*/
NTSTATUS CVideoDevice::ASIC_Communication(
		UCHAR Control,
		UCHAR Number,
		USHORT Address,
		UCHAR *pData)
{
	DBGU_FN("CVideoDevice::ASIC_Communication()");

	NTSTATUS ntStatus=STATUS_SUCCESS;
	UCHAR RW;

	DBGU_INFO("CVideoDevice::ASIC_Communication - R/W=%d, Bytes to r/w=%d, Address=%d(%xh)\n",(Control&0x01),Number,Address, Address);
	
#if DBG	
	DBGU_TRACE("Data => ");
	
	for (int i = 0; i < Number; ++i) {
		DBGU_TRACE("%02X ", pData[i]);
	}

	DBGU_TRACE("\n============================================\n");
#endif

	// Initialize
	if (Control & 0x01) 
		RW = VENDOR_REQUEST_WRITE;
	else
		RW = VENDOR_REQUEST_READ;

	ntStatus = UsbControlVendorTransfer(
				URB_FUNCTION_VENDOR_INTERFACE,	//Function
				!(Control & 0x01),				//Read/Write(write=false,read=true)
				RW,								//Request
				Address,						//Value (Address)
				0x0c45,							//2010/3/4 02:37と Index
				pData,							//m_CommandBuf,//Transfer Buffer
				Number);						//TransferLength

	if (Address == 8)
		KeDelay(1);

	if (!NT_SUCCESS(ntStatus))
	{
		DBGU_ERROR("Fail to read/write ASIC ! (Error Code : %x)\n",ntStatus);
	}

	return ntStatus;
}

BOOL CVideoDevice::ReadMPEG2TSFiles(void)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES objAttrs;
	HANDLE fileHandle=0;
	IO_STATUS_BLOCK ioStatusBlock;
	UNICODE_STRING  uin_str;
	LARGE_INTEGER offset;
	PFILE_STANDARD_INFORMATION pStdInfo = NULL;
	long FileLength;
	PUCHAR buffer = NULL;
	ULONG	temp;
	WCHAR filepath[MAX_PATH + 12];
	
	offset.QuadPart = 0;

	//if(!buffer)
	{
  		wcscpy(filepath, L"\\DosDevices\\C:\\TS.ts");
		RtlInitUnicodeString( &uin_str, (PWSTR)filepath );
		InitializeObjectAttributes( &objAttrs, &uin_str, OBJ_CASE_INSENSITIVE, NULL, NULL );
			
		ntStatus = ZwCreateFile(&fileHandle,
							GENERIC_READ | SYNCHRONIZE,
							&objAttrs,
							&ioStatusBlock,
							0,
							FILE_ATTRIBUTE_NORMAL,
							0,								//FILE_SHARE_READ | FILE_SHARE_WRITE
							FILE_OPEN,
							FILE_SYNCHRONOUS_IO_NONALERT,	//FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
							NULL,
							0
							);

		if (!NT_SUCCESS(ntStatus)) 
			return 0;			
		else
		{
			pStdInfo = (PFILE_STANDARD_INFORMATION) ExAllocatePool(NonPagedPool, sizeof(FILE_STANDARD_INFORMATION));
		
			if(pStdInfo == NULL)
				return 0;
			else
			{
				if(!NT_SUCCESS(ZwQueryInformationFile(fileHandle,
										&ioStatusBlock,
										pStdInfo,
										sizeof(FILE_STANDARD_INFORMATION),
										FileStandardInformation)))
					return 0;
				else
				{
					FileLength = pStdInfo->EndOfFile.QuadPart;
					KdPrint(("Michael : file length = %d \n", FileLength));
				  	buffer = (unsigned char *)ExAllocatePoolWithTag(NonPagedPool, FileLength*sizeof(unsigned char), '8sSF');
			
					if(!buffer)
						return 0;
					else
					{
						ntStatus = ZwReadFile(fileHandle,
											NULL,
											NULL,
											NULL,
											&ioStatusBlock,
											buffer,
											FileLength,
											&offset,
											NULL
											);

						if(ntStatus != STATUS_SUCCESS ) 
							return 0;
						else
							KdPrint(("Michael : Read File Success! \n"));
					}
				}

				ExFreePool(pStdInfo);
			}
		}
		
		ZwClose(fileHandle);
	}
	
	if(buffer)
	{
		pMPEG2TSFileBuffer = buffer;
		MPEG2TSFileBufferLen = FileLength;
	}
	
	return 1;
}

//2010/10/25 05:11と Dump merge image file
BOOL CVideoDevice::DumpImageFiles(PUCHAR Buffer, ULONG BufLen, ULONG FrameNo)
{
	IO_STATUS_BLOCK		ioStatusBlock;
	HANDLE				fh;	
	LARGE_INTEGER		ByteOffset;
	UNICODE_STRING		uSnapShotFileName;
	OBJECT_ATTRIBUTES	oa;
	WCHAR				*wssfn=NULL;

	wssfn = new (NonPagedPool) WCHAR[MAX_PATH];

	if(!wssfn)
		return 0;

	DBGU_TRACE("Save merge image image file size = %d\n", BufLen);

	swprintf(wssfn, L"\\DosDevices\\C:\\%d",FrameNo);
	RtlInitUnicodeString(&uSnapShotFileName, wssfn);
	InitializeObjectAttributes(&oa,&uSnapShotFileName, OBJ_CASE_INSENSITIVE,NULL,NULL);	

	if (NT_SUCCESS(ZwCreateFile(&fh,
								GENERIC_WRITE | SYNCHRONIZE,
								&oa,
								&ioStatusBlock,
								0, 
								FILE_ATTRIBUTE_NORMAL,
								FILE_SHARE_WRITE,
								FILE_OVERWRITE_IF,
								FILE_SYNCHRONOUS_IO_NONALERT,
								NULL,
								0)))
	{
		// Write Header
		ByteOffset.QuadPart = 0;
		ZwWriteFile(fh,
					NULL,
					NULL,
					NULL,
					&ioStatusBlock,
					Buffer,
					BufLen,
					&ByteOffset,
					NULL);
	}						

	ZwClose(fh);

	if (wssfn)
		delete wssfn;

	return 1;
}

//2010/12/28 10:42と
NTSTATUS CVideoDevice::SF_Communication(
UCHAR	Control,
USHORT	Number,
USHORT	Address,
UCHAR	*pData)
{
	NTSTATUS ntStatus;
	UCHAR RW;
	ULONG BufferLen;
	ULONG Offset;
	
	DBGU_TRACE("SF_Communication - R/W=%d, Bytes to r/w=%d, Address=%x\n", (Control&0x01), Number, Address);

	// Initialize
	RW = VENDOR_REQUEST_SF;	// Get/Set SF Data
	BufferLen = Number;
	Offset = 0;
	
	//2011/1/3 11:13と
	while(BufferLen > 64)
	{
		ntStatus = UsbControlVendorTransfer(
					URB_FUNCTION_VENDOR_INTERFACE,	//Function
					!(Control & 0x01),				//Read/Write(write=false,read=true)
					RW,								//Request
					Address+Offset,					//Value (Address)
					0x0c45,							//2010/3/4 02:37と Index
					pData+Offset,					//m_CommandBuf,//Transfer Buffer
					64);							//TransferLength
		Offset += 64;			
		BufferLen -= 64;
	}

	if(BufferLen > 0)
	{
		ntStatus = UsbControlVendorTransfer(
						URB_FUNCTION_VENDOR_INTERFACE,	//Function
						!(Control & 0x01),				//Read/Write(write=false,read=true)
						RW,								//Request
						Address+Offset,					//Value (Address)
						0x0c45,							//2010/3/4 02:37と Index
						pData+Offset,					//m_CommandBuf,//Transfer Buffer
						BufferLen);						//TransferLength	
	}

	DbgPrint("SF_Communication ntStatus = %x\n", ntStatus);
						
	return ntStatus;
}


NTSTATUS CVideoDevice::GetSectorTableAddr(
USHORT	*pwAddr)
{
	NTSTATUS ntStatus;
	USHORT	Address;
	UCHAR RW;
	UCHAR aData[3] = {0};
	ULONG BufferLen;

	DbgPrint("GetSectorTableAddr ==>\n");

	// Initialize
	RW = VENDOR_GET_SECTORTABLE_ADDR;	// Get Sector Table Address
	Address = 0;
	BufferLen = 3;

	ntStatus = UsbControlVendorTransfer(
				URB_FUNCTION_VENDOR_INTERFACE,	//Function
				0x01,							//Read/Write(write=false,read=true)
				RW,								//Request
				Address,						//Value (Address)
				0x0c45,							//2010/3/4 02:37と Index
				aData,							//m_CommandBuf,//Transfer Buffer
				BufferLen);						//TransferLength
	
	DbgPrint("GetSectorTableAddr ntStatus = %x\n", ntStatus);
	*pwAddr = (aData[1] << 8) | aData[0];
						
	return ntStatus;
}

NTSTATUS CVideoDevice::GetExtensionFrameRate(void)
{
	#define TABLELEN 24*1024
	NTSTATUS ntStatus = STATUS_SUCCESS;

	typedef struct
	{
		unsigned short wFRInt;
		BYTE byCfgIdx;
	} FRData;	// Frame Rate Data

	typedef struct
	{
		unsigned short wWid;
		unsigned short wHei;
		unsigned char byNumFR;
		FRData aFRData[20];
	} ResData;	// Resolution Data
		
	unsigned short wStartAddr = 0;
	unsigned short wAddr = 0;
	unsigned short wCfgAddr = 0;
	unsigned short wHeaderLen = 0;
	unsigned char *pbyReadBuf;
	unsigned char abyTmpBuf[8] = {0};
	unsigned char byYUY2Idx = 0;
	unsigned char byNumRes = 0;
	unsigned char byNumFR = 0;
	unsigned char byCfgLen = 0;	// shawn 2011/07/14 add for 288
	unsigned char i = 0;
	unsigned char j = 0;
	ResData aResData[20];
	UINT ImageCount = m_pDevExt->m_VideoImageFrameNumber[STREAM_Capture];

	// shawn 2011/07/14 for 288 +++++
	if (m_pDevExt->m_ChipID == 0x86)
		byCfgLen = 64;
	else if (m_pDevExt->m_ChipID == 0x88 || m_pDevExt->m_ChipID == 0x89)	// shawn 2011/08/22 for 289
		byCfgLen = 96;
	// shawn 2011/07/14 for 288 -----
		
	GetSectorTableAddr(&wAddr);					// Get Sector Table Address
	//TRACE("Sector Table Address = 0x%04x", wAddr);
	//ProcReadFromASIC(wAddr+12, abyTmpBuf);	// Using ASIC Read to read Timing Table Address
	ntStatus = ASIC_Communication(CTL_R_NONE, 1, wAddr+12, abyTmpBuf);

	if(ntStatus != STATUS_SUCCESS)
	{
		DBGU_TRACE("SF_Communication fail 1 %x", ntStatus);
		return ntStatus;
	}		

	wAddr = abyTmpBuf[0] * 256;
	//TRACE("Timing Table Address = 0x%04x", wAddr);

	pbyReadBuf = (unsigned char *)ExAllocatePoolWithTag(NonPagedPool, TABLELEN, 'SF');
	memset(pbyReadBuf, 0xff, TABLELEN);

	// Read Section1 Address from STRADDR +++++
	//ReqToDriver(SF_READ, wAddr, pbyReadBuf, 3);	// argument (Control, Address, Buffer, Length)
	ntStatus =SF_Communication(CTL_R_NONE, 3, wAddr, pbyReadBuf);

	if(ntStatus != STATUS_SUCCESS)
	{
		DBGU_TRACE("SF_Communication fail 2 %x", ntStatus);
		return ntStatus;
	}				
	// Read Section1 Address from STRADDR -----

	wStartAddr = ((pbyReadBuf[1] << 8) | pbyReadBuf[2]) + wAddr;
	//TRACE("StartAddr = 0x%04x", wStartAddr);
		
	// Read HeaderLen from Section 1 +++++
	//ReqToDriver(SF_READ, wStartAddr, pbyReadBuf, 2);
	ntStatus = SF_Communication(CTL_R_NONE, 2, wStartAddr, pbyReadBuf);

	if(ntStatus != STATUS_SUCCESS)
	{
		DBGU_TRACE("SF_Communication fail 3 %x", ntStatus);
		return ntStatus;
	}		
	// Read HeaderLen from Section 1 -----

	wHeaderLen = (pbyReadBuf[0] << 8) | pbyReadBuf[1];
	//TRACE("wHeaderLen = %d", wHeaderLen);

	// Read Header Data from STRADDR +++++
	//ReqToDriver(SF_READ, wStartAddr, pbyReadBuf, wHeaderLen);
	ntStatus = SF_Communication(CTL_R_NONE, wHeaderLen, wStartAddr, pbyReadBuf);

	if(ntStatus != STATUS_SUCCESS)
	{
		DBGU_TRACE("SF_Communication fail 4 %x", ntStatus);
		return ntStatus;
	}		
	// Read Header Data from STRADDR -----

	wAddr = 7;
	byYUY2Idx = pbyReadBuf[wAddr];

	if (byYUY2Idx == 1)
		wAddr = (pbyReadBuf[wAddr + 7] << 8) | pbyReadBuf[wAddr + 8];
	else
		wAddr = (pbyReadBuf[wAddr + 9] << 8) | pbyReadBuf[wAddr + 10];;
			
	byNumRes = pbyReadBuf[wAddr];
	wAddr++;

	for (i = 0; i < byNumRes; i++)
	{
		byNumFR = pbyReadBuf[wAddr];
		wAddr++;
			
		for (j = 0; j < byNumFR; j++)
		{
			aResData[i].aFRData[j].wFRInt = (pbyReadBuf[wAddr]<<8) | (pbyReadBuf[wAddr+1]);
			wAddr += 2;
				
			aResData[i].aFRData[j].byCfgIdx = pbyReadBuf[wAddr];
			wAddr++;

			DBGU_TRACE("Res %d : FR Int = %x", i, aResData[i].aFRData[j].wFRInt);
		}
			
		wCfgAddr = wStartAddr + wHeaderLen + (byCfgLen * (aResData[i].aFRData[0].byCfgIdx - 1));	// shawn 2011/07/14 modify for 288
		//ReqToDriver(SF_READ, wCfgAddr, abyTmpBuf, 8);
		SF_Communication(CTL_R_NONE, 8, wCfgAddr, abyTmpBuf);
		aResData[i].wWid = (abyTmpBuf[4]<<8) | (abyTmpBuf[5]);
		aResData[i].wHei = (abyTmpBuf[6]<<8) | (abyTmpBuf[7]);

		DBGU_TRACE("Res %d : Wid = %d, Hei = %d", i , aResData[i].wWid, aResData[i].wHei);
		
		//2011/1/3 03:30と
		for (int k=0;k<ImageCount;k++)
		{
			if ((m_pDevExt->StreamFormatArray[k].ImageFormat == IMG_FMT_YUY2)
			&&(m_pDevExt->StreamFormatArray[k].wWidth == aResData[i].wWid)
			&&(m_pDevExt->StreamFormatArray[k].wHeight == aResData[i].wHei))
			{
				m_pDevExt->StreamFormatArray[k].FrameRatesCount = byNumFR;

				for (int m = 0; m < byNumFR; m++)
				{
					m_pDevExt->StreamFormatArray[k].SupportedFrameRates[m] = 10000000/(10000000/(aResData[i].aFRData[m].wFRInt << 8));
					DBGU_TRACE("MJ Width %d, Height = %d, interval = %d", m_pDevExt->StreamFormatArray[k].wWidth, m_pDevExt->StreamFormatArray[k].wHeight, m_pDevExt->StreamFormatArray[k].SupportedFrameRates[m]);
				}				
			}
		}
	}

	if(pbyReadBuf)
	{ 
		ExFreePoolWithTag(pbyReadBuf, 'FREE');
		pbyReadBuf = NULL;
	}

	DBGU_TRACE("SF_Communication success %x", ntStatus);
	return ntStatus;
}

short CVideoDevice::GetExposureValue(IN ULONG fw_Exposure_us)
{
	
	//RBK exposure value  
	//    no floating point 
	int iExp =0;
	ULONG iExposure_us = 10000;

	for (iExposure_us = 10000 ; (iExposure_us > fw_Exposure_us) && (iExposure_us > 0); iExposure_us =iExposure_us>>1)
	{
		iExp--;
	}

	return iExp;
}

//mux 2011/3/31 11:25と
NTSTATUS
CVideoDevice::StartMuxThread()
{
	NTSTATUS ntStatus;
	HANDLE hTSMuxThread;

	//
	// we are ready to start the thread that handle read SRb completeion 
	// after iso transfer completion routine puts them in the que.
	//
	StopMuxThread = FALSE;
	TerminateMuxThread  = FALSE;
	
	ntStatus = PsCreateSystemThread(&hTSMuxThread, 0L, NULL, NULL, NULL,
						(PKSTART_ROUTINE)TSMuxRoutine, this);

	if (NT_SUCCESS(ntStatus)) {
		ZwClose(hTSMuxThread);			
	} 
	else {
		DBGU_TRACE("Fail to create hTSMuxThread thread !\n");
	}

	return ntStatus;
}

void TSMuxRoutine(IN CVideoDevice *pVideoDevice)
{
    NTSTATUS status;
	int OutputBufSize;
	ULONGLONG Accumulator;
	//ULONGLONG TmpTime;
	LARGE_INTEGER TmpTime;
	ULONGLONG FrameInt;
    // set the thread priority
    KeSetPriorityThread(KeGetCurrentThread(),LOW_REALTIME_PRIORITY);

    while (TRUE) 
    {
		PDATABUFCONTENT pContent = pVideoDevice->GetDataBuffer(DATAOP_READ, 1);

		if(!pContent) 
		{
			DBGU_TRACE("TSMuxRoutine KeWaitForSingleObject \n");
			KeWaitForSingleObject(&pVideoDevice->TsMuxSemaphore, Executive,KernelMode,FALSE,NULL);
		}

		// 
		// We are ready to go. chk if the stop flag is on.
		//
		if (pVideoDevice->StopMuxThread ) {
			pVideoDevice->StopMuxThread = FALSE;
			DBGU_TRACE("TSMuxRoutine PsTerminateSystemThread  StopMuxThread = %d\n",pVideoDevice->StopMuxThread);
			status = PsTerminateSystemThread(STATUS_SUCCESS);
		}

		pContent = pVideoDevice->GetDataBuffer(DATAOP_READ, 1);

		if (pContent)
		{
	   		//if(pContent->DataLength>0)
	   		//if(pContent->DataLength < 300)
	   		//	DBGU_TRACE("Michael Feng pContent->DataLength =%d\n",pContent->DataLength);

			if(pContent->DataLength>100)
			{
				//2011/4/11 04:02と
				OutputBufSize = 0;

				KeQuerySystemTime(&TmpTime);
				
				FrameInt = TmpTime.QuadPart - pVideoDevice->m_Time;
				DBGU_TRACE("TmpTime.QuadPart = %ld FrameInt =%ld\n",TmpTime.QuadPart,FrameInt);
				pVideoDevice->m_Time = TmpTime.QuadPart;

				Accumulator = FrameInt;
				//Accumulator -= 40000;
				//DBGU_TRACE("FrameInt=%ld\n",FrameInt);
				DBGU_TRACE("Accumulator = %ld\n",Accumulator);
				Accumulator *=9;
				Accumulator /= 1000;
				//Accumulator += 6000;
				DBGU_TRACE("Accumulator = %ld\n",Accumulator);	
						
				//Accumulator -= 300;
				DBGU_TRACE("g_llPCR_Accumulator += %ld\n",Accumulator);
				g_llPCR_Accumulator += Accumulator;
				//Accumulator += 10;
				DBGU_TRACE("g_llPTS_Accumulator += %ld\n",Accumulator);
				g_llPTS_Accumulator += Accumulator;	// 3003 -> 33ms
				//Accumulator += 10;
				DBGU_TRACE("g_llDTS_Accumulator += %ld\n",Accumulator);
				g_llDTS_Accumulator += Accumulator;	// 3003 -> 33ms		

				Construct_TS(pVideoDevice->FrameSN,	// Input Frame index
					 pContent->pDataBuf,			// Input video data buffer
					 pContent->DataLength,			// Input video data buffer size (number of bytes)
					 pVideoDevice->pTSMuxTempBuf,	// Output video data buffer
					 OutputBufSize);				// Output video data buffer size (number of bytes)

				/*// shawn 2011/08/04 for testing +++++
				for (int i = 0; i < OutputBufSize; i += 188)
				{
					if (pVideoDevice->pTSMuxTempBuf[i]   == 0x47 && 
						((pVideoDevice->pTSMuxTempBuf[i+1] == 0x01) || (pVideoDevice->pTSMuxTempBuf[i+1] == 0x41)) && 
						pVideoDevice->pTSMuxTempBuf[i+2] == 0x00)
					{
						if ((i + 3) < OutputBufSize)
						{
							if (((pVideoDevice->pTSMuxTempBuf[i+3])&0x0F) != pVideoDevice->m_byTmpTSCounter)
							{
								DBGU_TRACE("Shawn => TSMuxRoutine : TS Counter Failed => %d -> %d\n", pVideoDevice->m_byTmpTSCounter, ((pVideoDevice->pTSMuxTempBuf[i+3])&0x0F));
								pVideoDevice->m_byTmpTSCounter = ((pVideoDevice->pTSMuxTempBuf[i+3])&0x0F);
							}
							
							pVideoDevice->m_byTmpTSCounter = (pVideoDevice->m_byTmpTSCounter + 1) % 16;
						}
					}
				}
				// shawn 2011/08/04 for testing -----*/

				/*if(1)//(Accumulator >= 10000)
				{
					//DBGU_TRACE("MJ Accumulator = %ld\n",Accumulator);
					// Accumulate Time Stamps
					g_llPCR_Accumulator += 3603;
					g_llPTS_Accumulator += 3613;;	// 3003 -> 33ms
					g_llDTS_Accumulator += 3623;;	// 3003 -> 33ms
					DBGU_TRACE("3603\n");		
				}
				else
				{
					//Accumulator -= 300;
					DBGU_TRACE("g_llPCR_Accumulator += %ld\n",Accumulator);
					g_llPCR_Accumulator += Accumulator;
					Accumulator += 10;
					DBGU_TRACE("g_llPTS_Accumulator += %ld\n",Accumulator);
					g_llPTS_Accumulator += Accumulator;	// 3003 -> 33ms
					Accumulator += 10;
					DBGU_TRACE("g_llDTS_Accumulator += %ld\n",Accumulator);
					g_llDTS_Accumulator += Accumulator;	// 3003 -> 33ms		
				}*/
					
				DBGU_TRACE("TSMuxRoutine OutputBufSize =%d pContent->DataLength =%d FrameSN=%d\n",OutputBufSize, pContent->DataLength,pVideoDevice->FrameSN);
				pVideoDevice->FrameSN ++;	
			
				//2010/7/23 06:01と ring
				if((pVideoDevice->m_MPEG2IsoBufferAddr + OutputBufSize) > DEFAULT_DATA_BUFFER_SIZE)
				{
					UINT Offset = DEFAULT_DATA_BUFFER_SIZE - pVideoDevice->m_MPEG2IsoBufferAddr;
					RtlCopyMemory(pVideoDevice->pMP2TSDataBuf + pVideoDevice->m_MPEG2IsoBufferAddr, pVideoDevice->pTSMuxTempBuf, Offset);
					RtlCopyMemory(pVideoDevice->pMP2TSDataBuf, pVideoDevice->pTSMuxTempBuf + Offset, OutputBufSize - Offset);
					pVideoDevice->m_MPEG2IsoBufferAddr = OutputBufSize - Offset;
				}
				else
				{
					RtlCopyMemory(pVideoDevice->pMP2TSDataBuf + pVideoDevice->m_MPEG2IsoBufferAddr, pVideoDevice->pTSMuxTempBuf, OutputBufSize);
					pVideoDevice->m_MPEG2IsoBufferAddr += OutputBufSize;

					if(pVideoDevice->m_MPEG2IsoBufferAddr >= DEFAULT_DATA_BUFFER_SIZE)
						pVideoDevice->m_MPEG2IsoBufferAddr = 0;
				}

				pVideoDevice->m_MPEG2ValidDataLength += OutputBufSize;
			}
			else
				DBGU_TRACE("pContent->DataLength<2000 len =%d\n",pContent->DataLength);

	    	status  = STATUS_SUCCESS;
			DBGU_TRACE("TSMuxRoutine m_MPEG2ValidDataLength =%d OutputBufSize =%d\n",pVideoDevice->m_MPEG2ValidDataLength,OutputBufSize);

			if(pVideoDevice->m_MPEG2ValidDataLength > pVideoDevice->m_APBufferLen[pVideoDevice->m_pDevExt->m_STREAM_Capture_MP2TS_Idx])	// shawn 2011/06/23 modify
	    	{
				DBGU_TRACE("KeSetEvent(&pVideoDevice->m_MEPG2DataReadyEvent, 0, FALSE); \n");
	    		KeSetEvent(&pVideoDevice->m_MEPG2DataReadyEvent, 0, FALSE);
			}
		}

		pVideoDevice->ReturnDataBuffer(DATAOP_READ, 1);
	}
}

ULONGLONG 
CVideoDevice::GetSystemTime(void)
{
	//ULONGLONG ticks;
	LARGE_INTEGER ticks;
	//KeQueryTickCount((PLARGE_INTEGER)&ticks);
    KeQueryTickCount(&ticks);
    
    DBGU_TRACE("ticks =%ld \n",ticks.QuadPart);
	//ticks *= m_TimeIncrement;
	ticks.QuadPart *= m_TimeIncrement;
		
    //return(ticks);
    return(ticks.QuadPart);
}

// james 2011/03/01
BOOL CVideoDevice::Report_Black_Image(PKS_VIDEOINFOHEADER pVideoInfoHeader, PBYTE FrameBuffer, BYTE ImgFormat)
{
	DBGU_TRACE("Report_Black_Image ImgFormat = %d, biCompression = %d", ImgFormat, pVideoInfoHeader->bmiHeader.biCompression);

	//BYTE ImgFormat = pVideoInfoHeader->bmiHeader.biCompression;
	int w = pVideoInfoHeader->bmiHeader.biWidth;
	int h = pVideoInfoHeader->bmiHeader.biHeight;
	int size;

	//if (ImgFormat == KS_BI_RGB)	// RGB24
	if (ImgFormat == IMG_FMT_RGB24)	// RGB24
	{
		size = w*h;
		// R
		RtlFillMemory((unsigned char *)FrameBuffer,size,0);
		// G
		RtlFillMemory((unsigned char *)FrameBuffer+size,size,0);
		// B
		RtlFillMemory((unsigned char *)FrameBuffer+2*size,size,0);
	}
	//else if (ImgFormat == MEDIASUBTYPE_I420.Data1)
	else if (ImgFormat == IMG_FMT_I420)
	{
		size = w*h;
		// Y
		RtlFillMemory((unsigned char *)FrameBuffer,size,0);
		// U
		size = w*h/4;
		RtlFillMemory((unsigned char *)FrameBuffer+size*4,size,128);
		// V
		RtlFillMemory((unsigned char *)FrameBuffer+size*5,size,128);
	}
	//else if (ImgFormat == MEDIASUBTYPE_YUY2.Data1)
	else if (ImgFormat == IMG_FMT_YUY2)
	{
		for(int i=0;i<w*h*2;i+=2)
		{
			*(FrameBuffer+i) = 0;
			*(FrameBuffer+i+1) = 128;
		}
	}

	return TRUE;
}

USHORT CVideoDevice::RunningWindowsVersion()
{



	if(!IoIsWdmVersionAvailable(6,0x00))
		return WindowsXP;
	

	return WindowsUnknown;
}