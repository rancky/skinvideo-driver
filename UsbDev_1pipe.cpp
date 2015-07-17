/****************************************************************************
*
*  Copyright (c) 2003  Sonix Technology Co., Ltd.
*
*  Module Name: UsbDev.cpp
*
*  Abstract:    Provide basic functions to access USB device thought USB bus.
*
*  Revision History:
*
*    03-24-2003 :	Created [Peter]
*	 04-17-2003 :	Modify the architecture of the including files [Peter]
*
*****************************************************************************/

#include <SnCam.h>
#define USBDEV_POOLTAG 'EDNS'

// UsbDev Constructor
UsbDev::UsbDev()
:m_UsbConfigurationNo(0), m_hConfigureation(NULL), m_NumberOfInterfaces(0),
 m_SuspendIrp(NULL)
{
	RtlZeroMemory(&m_DevDesc, sizeof(USB_DEVICE_DESCRIPTOR));
	for(int i=0;i<MAX_CONFIGURATION_NO;i++)
		m_pConfigDesc[i] = NULL;
	//2010/3/9 11:20上午
	KeInitializeSpinLock(&IdleReqStateLock);	
	//2010/8/17 03:50下午
//	KeInitializeSpinLock(&SetConfigSpinLock);
	  KeInitializeSemaphore(&CallUSBSemaphore, 1, 1);
		//KeInitializeSpinLock(&m_IdleReqStateLock);
}

// UsbDev deconstructor
UsbDev::~UsbDev()
{
	
//	SetToUnconfigure();
	DBGU_TRACE("UsbDev::~UsbDev()\n");

	for(int i=0;i<MAX_CONFIGURATION_NO;i++)
		if(m_pConfigDesc[i]) ExFreePoolWithTag(m_pConfigDesc[i],USBDEV_POOLTAG);
	
	//if(m_pInterfaceListEntry)
	{ // m_pInterfaceListEntry != NULL
		for(int i=0;i<m_NumberOfInterfaces;i++)
			if(m_pInterface[i]) 
			{
				ExFreePoolWithTag(m_pInterface[i],USBDEV_POOLTAG);
				//2010/8/16 06:31下午
				m_pInterface[i] = NULL;
				DBGU_TRACE("ExFreePoolWithTag m_pInterface[%d]\n",i);
			}
		//ExFreePoolWithTag(m_pInterfaceListEntry,USBDEV_POOLTAG);
		//m_pInterfaceListEntry = NULL;
	} // m_pInterfaceListEntry != NULL
	
}

// Initialize usb device.
NTSTATUS UsbDev::InitializeUsbDevice()
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	DBGU_TRACE(">>>UsbDev::InitializeUsbDevice !\n");
	ntStatus = GetDeviceDescriptor();
	if(NT_SUCCESS(ntStatus))
	{
		ntStatus = GetConfigurationDescriptor();
		if(NT_SUCCESS(ntStatus))
		{
			ntStatus = SetConfiguration();
		}
	}
	DBGU_TRACE("UsbDev::InitializeUsbDevice return %X\n", ntStatus);
	return ntStatus;
}

// Set the alternate setting of the interface.
NTSTATUS UsbDev::SetAlternate(UCHAR AlternateNo, UCHAR InterfaceNo)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PURB pUrb = NULL;
	PUSBD_INTERFACE_INFORMATION pInterfaceInfo = NULL;
	int i;

	DBGU_TRACE(">>>UsbDev::SetAlternate !\n");
	if (m_pInterface[InterfaceNo])
		pInterfaceInfo = m_pInterface[InterfaceNo];
	if(pInterfaceInfo)
	{//pInterfaceInfo != NULL
#ifdef __VIDEO_CLASS__
		// For Video Class Device
		DBGU_TRACE("Change information for ISO Interface\n");
		/*
		if (InterfaceNo == 1)
			pInterfaceInfo->NumberOfPipes = 1;
		if ((InterfaceNo == 1)&&(AlternateNo > 0))
			pInterfaceInfo->Length = sizeof(USBD_INTERFACE_INFORMATION);
		*/
		// james try M2TS.
		if (InterfaceNo == 1 || InterfaceNo == 2)
		{
			pInterfaceInfo->NumberOfPipes = 1;
		
			if (AlternateNo > 0)
				pInterfaceInfo->Length = sizeof(USBD_INTERFACE_INFORMATION);
		}

#endif
		pUrb = (PURB) ExAllocatePoolWithTag(NonPagedPool, GET_SELECT_INTERFACE_REQUEST_SIZE(pInterfaceInfo->NumberOfPipes), USBDEV_POOLTAG);

		if(pUrb) 
		{//pUrb != NULL
			pInterfaceInfo->AlternateSetting = AlternateNo;
			for(i=0;i<pInterfaceInfo->NumberOfPipes;i++)
			{
				if (m_fUSBHighSpeed)
					pInterfaceInfo->Pipes[i].MaximumTransferSize = MAX_TRANSFER_SIZE_2;
				else
					pInterfaceInfo->Pipes[i].MaximumTransferSize = MAX_TRANSFER_SIZE;
				pInterfaceInfo->Pipes[i].PipeFlags = 0;
			}
			RtlCopyMemory(&pUrb->UrbSelectInterface.Interface, pInterfaceInfo, pInterfaceInfo->Length);
			pUrb->UrbHeader.Length   = GET_SELECT_INTERFACE_REQUEST_SIZE(pInterfaceInfo->NumberOfPipes);
			pUrb->UrbHeader.Function = URB_FUNCTION_SELECT_INTERFACE;
			pUrb->UrbSelectInterface.ConfigurationHandle = m_hConfigureation;
			ntStatus = SendAwaitUrb(pUrb);
			if(!NT_SUCCESS(ntStatus))
			{//ntStatus != STATUS_SUCCESS
				DBGU_TRACE("ERR: Fail to set alternate setting !!\n");
				ntStatus = STATUS_UNSUCCESSFUL;
			}//ntStatus != STATUS_SUCCESS
			else
			{//ntStatus == STATUS_SUCCESS
				if(pInterfaceInfo->Length > pUrb->UrbSelectInterface.Interface.Length) 
					pInterfaceInfo->Length = pUrb->UrbSelectInterface.Interface.Length;
				RtlCopyMemory(pInterfaceInfo, &pUrb->UrbSelectInterface.Interface, pUrb->UrbSelectInterface.Interface.Length);
				//Display interface information
#if DBG
				DBGU_TRACE(" Interface Information\n");
				DBGU_TRACE(" -----------------------------------\n");
				DBGU_TRACE(" Length: %x\n",pUrb->UrbSelectInterface.Interface.Length);
				DBGU_TRACE(" InterfaceNumber: %x\n",pUrb->UrbSelectInterface.Interface.InterfaceNumber);
				DBGU_TRACE(" AlternateSetting: %x\n", pUrb->UrbSelectInterface.Interface.AlternateSetting);
				DBGU_TRACE(" NumberOfPipes: %x\n",pUrb->UrbSelectInterface.Interface.NumberOfPipes);
				DBGU_TRACE(" Class: %x\n",pUrb->UrbSelectInterface.Interface.Class);
				DBGU_TRACE(" SubClass: %x\n",	pUrb->UrbSelectInterface.Interface.SubClass);
				DBGU_TRACE(" Protocol: %x\n",pUrb->UrbSelectInterface.Interface.Protocol);
				DBGU_TRACE(" InterfaceHandle: %x\n",pUrb->UrbSelectInterface.Interface.InterfaceHandle);
				DBGU_TRACE(" -----------------------------------\n");	
				for(i=0;i<pInterfaceInfo->NumberOfPipes;i++) 
				{// for loop i
					DBGU_TRACE(" Pipes (%x) Information\n",i);
					DBGU_TRACE(" -----------------------------------\n");	
					DBGU_TRACE(" EndpointAddress: %x\n",pUrb->UrbSelectInterface.Interface.Pipes[i].EndpointAddress);     
					DBGU_TRACE(" PipeType: %x\n",pUrb->UrbSelectInterface.Interface.Pipes[i].PipeType);
					DBGU_TRACE(" PipeHandle: %x\n",pUrb->UrbSelectInterface.Interface.Pipes[i].PipeHandle);
					DBGU_TRACE(" MaximumPacketSize: %x\n",pUrb->UrbSelectInterface.Interface.Pipes[i].MaximumPacketSize);
				}// for loop i
				DBGU_TRACE(" -----------------------------------\n");	
#endif
			}//ntStatus == STATUS_SUCCESS
		}//pUrb != NULL
		else
		{//pUrb == NULL
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}//pUrb == NULL
	}//pInterfaceInfo != NULL
	else
	{//pInterfaceInfo == NULL
		DBGU_TRACE("pInterfaceInfo == NULL !\n");
		ntStatus = STATUS_UNSUCCESSFUL;
	}//pInterfaceInfo == NULL
	if(pUrb)
		ExFreePoolWithTag(pUrb,USBDEV_POOLTAG);
	return ntStatus;
}

// Get the alternate setting number of the interface. 
int UsbDev::GetNoOfAlternateSetting(UCHAR InterfaceNo)
{
	PUSB_INTERFACE_DESCRIPTOR pAlternateSetting = NULL;
	int i;
	for(i=0;;i++) 
	{// for loop i
		if (!m_pConfigDesc)
			break;

		pAlternateSetting = (PUSB_INTERFACE_DESCRIPTOR)USBD_ParseConfigurationDescriptorEx(
								m_pConfigDesc[m_UsbConfigurationNo],// Pointer to configuration descriptor
								m_pConfigDesc[m_UsbConfigurationNo],// Start position
								InterfaceNo,					// Interface number
								i,								// Alternate setting
								-1,								// Class
								-1,								// Subclass
								-1);							// Protocol
		if(!pAlternateSetting) 
			break;
	}//for loop i
	return (i);
}

//Get pipe information.
NTSTATUS UsbDev::GetPipeInfo(
	UCHAR InterfaceNo,
	UCHAR PipeNo,
	PUSBD_PIPE_INFORMATION pPipeInfo
	)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	if (m_pInterface[InterfaceNo]) {//if (m_pInterfaceListEntry) {
		if(pPipeInfo)
			*pPipeInfo = m_pInterface[InterfaceNo]->Pipes[PipeNo];
		else
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	}
	else {
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	if(!NT_SUCCESS(ntStatus))
		DBGU_TRACE("GetPipeInfo Failure!\n");

	return ntStatus;
}

// Send vendor specific command to usb device by control pipe.
NTSTATUS UsbDev::UsbControlVendorTransfer(
	USHORT Functions,
	BOOL  Rw,
	UCHAR Request,
	USHORT Value,
	USHORT Index,
	PVOID TransferBuffer,
	ULONG TransferBufferLength
	)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
    PURB		Urb;

	Urb = (PURB) ExAllocatePoolWithTag(NonPagedPool,
			sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST), USBDEV_POOLTAG);

    if (Urb) 
	{// if Urb != NULL
		RtlZeroMemory((void *) Urb, sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST));
		UsbBuildVendorRequest(
					Urb,																// Urb
					Functions,															// Functions
					(USHORT)sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),		// Length 
					(Rw ? USBD_TRANSFER_DIRECTION_IN : USBD_TRANSFER_DIRECTION_OUT),	// TransferFlags
					0,																	// ReservedBits 
					Request,															// Request
					Value,														// Value 
					Index,														// Index
					TransferBuffer,												// TransferBuffer
					NULL,														// TransferBufferMDL
					TransferBufferLength,										// TransferBufferLength 
					NULL);														// Link 
        ntStatus = SendAwaitUrb(Urb);
	    ExFreePoolWithTag(Urb,USBDEV_POOLTAG);
//		if (!NT_SUCCESS(ntStatus)) 
//		{
//			DBGU_TRACE("Cannot WriteCommandToASIC %X!!\n",ntStatus);
//		}
    }// if Urb != NULL
	else 
	{// if Urb == NULL	
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }// if Urb == NULL

    return ntStatus;
}

//Bulk or Interrupt transfer (asynchronous)
NTSTATUS UsbDev::UsbBulkOrInterruptTransfer(
	PIRP pIrp,
	PURB pUrb,
	PUSBD_PIPE_INFORMATION	pPipeInfo,
	BOOL Rw,
	PVOID TransferBuffer,
	PMDL TransferBufferMDL,
	ULONG TransferBufferLength,
	PVOID BulkOrInterruptCompletion,
	PVOID BulkOrInterruptContext
	)
{
	NTSTATUS	ntStatus;
	PIO_STACK_LOCATION	pStack;
	
	if(pIrp && pUrb && pPipeInfo && (TransferBuffer || TransferBufferMDL) && BulkOrInterruptCompletion && BulkOrInterruptContext)
	{// pIrp != NULL && pUrb != NULL
		pStack=IoGetNextIrpStackLocation(pIrp);
		pStack->Parameters.Others.Argument1 = pUrb;	
		pStack->Parameters.DeviceIoControl.IoControlCode=IOCTL_INTERNAL_USB_SUBMIT_URB;
		pStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;	
		UsbBuildInterruptOrBulkTransferRequest(pUrb,
											   sizeof(_URB_BULK_OR_INTERRUPT_TRANSFER),
											   pPipeInfo->PipeHandle,
											   TransferBuffer,
											   TransferBufferMDL,
											   TransferBufferLength,
											   ((Rw ? USBD_TRANSFER_DIRECTION_IN : 0) | USBD_SHORT_TRANSFER_OK),
											   NULL);
		IoSetCompletionRoutine(pIrp,(PIO_COMPLETION_ROUTINE)BulkOrInterruptCompletion,BulkOrInterruptContext,TRUE,TRUE,TRUE);
		ntStatus = IoCallDriver(m_pLdo,pIrp);
	}// pIrp != NULL && pUrb != NULL
	else
	{// pIrp == NULL || pUrb == NULL
		ntStatus = STATUS_INVALID_PARAMETER;//((NTSTATUS)0xC000000DL)
	}// pIrp == NULL || pUrb == NULL
	return ntStatus;
}

//Isochronous transfer (asynchronous)
NTSTATUS UsbDev::UsbIsochronousTransfer(
	PIRP pIrp,
	PURB pUrb,
	PUSBD_PIPE_INFORMATION	pPipeInfo,
	BOOL Rw,
	PVOID TransferBuffer,
	PMDL TransferBufferMDL,
	PVOID IsochronousCompletion,
	PVOID IsochronousContext
	)
{
	NTSTATUS ntStatus;
	PIO_STACK_LOCATION	pStack;
	ULONG siz,NumOfIsoPackets;
	int i;

	if(pIrp && pUrb && pPipeInfo && (TransferBuffer || TransferBufferMDL) && IsochronousCompletion && IsochronousContext)
	{// pIrp != NULL && pUrb != NULL
		pStack=IoGetNextIrpStackLocation(pIrp);
		pStack->Parameters.Others.Argument1 = pUrb;	
		pStack->Parameters.DeviceIoControl.IoControlCode=IOCTL_INTERNAL_USB_SUBMIT_URB;
		pStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
		//2010/8/4 09:55上午
		//NumOfIsoPackets = (pPipeInfo->MaximumTransferSize/pPipeInfo->MaximumPacketSize);
		//DBGU_TRACE("pPipeInfo->MaximumTransferSize = %d, MaximumPacketSize = %d\n",pPipeInfo->MaximumTransferSize, pPipeInfo->MaximumPacketSize);

		{
			if (m_fUSBHighSpeed)
			{
				//2010/8/4 09:55上午
				//if(NumOfIsoPackets > MAX_NUM_ISO_PACKETS_2)
					NumOfIsoPackets = MAX_NUM_ISO_PACKETS_2;
			}
			else
			{
				//if(NumOfIsoPackets > MAX_NUM_ISO_PACKETS)
					NumOfIsoPackets = MAX_NUM_ISO_PACKETS;
			}
		}
		siz = GET_ISO_URB_SIZE(NumOfIsoPackets);
		RtlZeroMemory(pUrb, siz);
		pUrb->UrbIsochronousTransfer.Hdr.Length = siz;
		pUrb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;
		pUrb->UrbIsochronousTransfer.PipeHandle = pPipeInfo->PipeHandle;
		pUrb->UrbIsochronousTransfer.TransferFlags = ((Rw ? USBD_TRANSFER_DIRECTION_IN : 0) | USBD_SHORT_TRANSFER_OK | USBD_START_ISO_TRANSFER_ASAP);
		pUrb->UrbIsochronousTransfer.TransferBufferLength = (NumOfIsoPackets * pPipeInfo->MaximumPacketSize);//pPipeInfo->MaximumTransferSize
		pUrb->UrbIsochronousTransfer.TransferBuffer	= TransferBuffer;
		pUrb->UrbIsochronousTransfer.TransferBufferMDL = TransferBufferMDL; 
		pUrb->UrbIsochronousTransfer.StartFrame = 0;
		pUrb->UrbIsochronousTransfer.NumberOfPackets = NumOfIsoPackets;
		pUrb->UrbIsochronousTransfer.UrbLink = NULL;
		for (i = 0; i<NumOfIsoPackets; i++) 
		{
			pUrb->UrbIsochronousTransfer.IsoPacket[i].Offset = (i*pPipeInfo->MaximumPacketSize);
			pUrb->UrbIsochronousTransfer.IsoPacket[i].Length = pPipeInfo->MaximumPacketSize;
		}
		IoSetCompletionRoutine(pIrp,(PIO_COMPLETION_ROUTINE)IsochronousCompletion,IsochronousContext,TRUE,TRUE,TRUE);
		ntStatus = IoCallDriver(m_pLdo,pIrp);
	}// pIrp != NULL && pUrb != NULL
	else
	{// pIrp == NULL || pUrb == NULL
		ntStatus = STATUS_INVALID_PARAMETER;//((NTSTATUS)0xC000000DL)
	}// pIrp == NULL || pUrb == NULL
	return ntStatus;
}

// Abort or Reset pipe
NTSTATUS UsbDev::PipeAbortOrReset(UCHAR InterfaceNo,UCHAR PipeNo,BOOLEAN Abort)
{
    NTSTATUS	ntStatus;
	PURB		pUrb;

	if (m_pInterface[InterfaceNo]) {//if (m_pInterfaceListEntry) {
		pUrb = (PURB) ExAllocatePoolWithTag(NonPagedPool, sizeof(struct _URB_PIPE_REQUEST), USBDEV_POOLTAG);
		if (pUrb) 
		{
			pUrb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
    		pUrb->UrbHeader.Function = (USHORT) (Abort ? URB_FUNCTION_ABORT_PIPE : URB_FUNCTION_RESET_PIPE);
			pUrb->UrbPipeRequest.PipeHandle = m_pInterface[InterfaceNo]->Pipes[PipeNo].PipeHandle;
			ntStatus = SendAwaitUrb(pUrb);
			ExFreePoolWithTag(pUrb,USBDEV_POOLTAG);
		} 
		else 
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;		
		}	
	}
	else {
		ntStatus = STATUS_UNSUCCESSFUL;
	}
	
#if DBG
	if(Abort)
		DBGU_TRACE("UsbDev::PipeAbortOrReset ! Num = %d, (ABORT PIPE), Status : %x\n",InterfaceNo, ntStatus);
	else
		DBGU_TRACE("UsbDev::PipeAbortOrReset ! Num = %d, (RESET PIPE), Status : %x\n",InterfaceNo, ntStatus);
#endif
    
	return ntStatus;
}

// Get Usb Device Descriptor
NTSTATUS UsbDev::GetDeviceDescriptor(PUSB_DEVICE_DESCRIPTOR pdd)
{
	NTSTATUS	ntStatus = STATUS_SUCCESS;
	if(pdd)
	{
		RtlCopyMemory(pdd, &m_DevDesc, sizeof(USB_DEVICE_DESCRIPTOR));
	}
	else
	{
		ntStatus = STATUS_UNSUCCESSFUL;
	}
	return ntStatus;
}

// ------------------------ Protected member functions ------------------------

// Set PDO to private variable "m_pLdo"
void UsbDev::SetUsbPDO(PDEVICE_OBJECT pdo)
{
	m_pLdo = pdo;
}

NTSTATUS
SyncCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
/*++

Routine Description:

    If the Irp is one we allocated ourself, DeviceObject is NULL.

--*/
{
    PKEVENT kevent = (PKEVENT)Context;

    KeSetEvent(kevent, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

// Send a USB command to device and then waiting for this command to complete.
NTSTATUS UsbDev::SendAwaitUrb(PURB Urb)
{
		//2010/8/25 03:13下午
	  /*NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP irp;
    KEVENT TimeoutEvent;
    PIO_STACK_LOCATION nextStack;

    KeWaitForSingleObject(&CallUSBSemaphore,Executive,KernelMode,FALSE,NULL);

    // Initialize the event we'll wait on
    //
    KeInitializeEvent(&TimeoutEvent,SynchronizationEvent,FALSE);

    // Allocate the Irp
    //
    irp = IoAllocateIrp(m_pLdo->StackSize, FALSE);

    if (irp == NULL){
        ntStatus =  STATUS_UNSUCCESSFUL;
        goto Exit_CallUSB;
    }
    //
    // Set the Irp parameters
    //
    nextStack = IoGetNextIrpStackLocation(irp);
    //ASSERT(nextStack != NULL);
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.DeviceIoControl.IoControlCode =  IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->Parameters.Others.Argument1 = Urb;
    //
    // Set the completion routine.
    //
    IoSetCompletionRoutine(irp,SyncCompletionRoutine,&TimeoutEvent, TRUE, TRUE,TRUE);   
    //
    // pass the irp down usb stack
    //
    ntStatus = IoCallDriver(m_pLdo,irp);

    if (ntStatus == STATUS_PENDING) {
        // Irp i spending. we have to wait till completion..
        LARGE_INTEGER timeout;

        // Specify a timeout of 5 seconds to wait for this call to complete.
        //
        timeout.QuadPart = -10000 * 5000;

        ntStatus = KeWaitForSingleObject(&TimeoutEvent, Executive,KernelMode,FALSE, &timeout);
        if (ntStatus == STATUS_TIMEOUT) {
           ntStatus = STATUS_IO_TIMEOUT;

            // Cancel the Irp we just sent.
            //
            IoCancelIrp(irp);

            // And wait until the cancel completes
            //
            KeWaitForSingleObject(&TimeoutEvent,Executive, KernelMode, FALSE,NULL);
        }
        else {
            ntStatus = irp->IoStatus.Status;
        }
    }

    // Done with the Irp, now free it.
    //
    IoFreeIrp(irp);

Exit_CallUSB:

    KeReleaseSemaphore(&CallUSBSemaphore,LOW_REALTIME_PRIORITY,1,FALSE);

    if (NT_ERROR(ntStatus)) {
       DBGU_TRACE("***Error*** SendAwaitUrb (%x) (%x)\n", ntStatus,Urb->UrbHeader.Status);
    }

    return ntStatus;
	*/
	NTSTATUS ntStatus = STATUS_SUCCESS;
	IO_STATUS_BLOCK ioStatus;
	KEVENT event;
	PIRP Irp;
	PIO_STACK_LOCATION	nextStack;
	LARGE_INTEGER		dueTime;
//	UINT retry = 0;

	KeInitializeEvent(&event, NotificationEvent, FALSE);

	//	------------------  2006/09/27 [Saxen Ko]  ------------------
	// fix DELL bug, that we might re-use the Urb to sent commands!

//	for (retry = 0; retry < 3; ++retry)
	{
		Irp = IoBuildDeviceIoControlRequest(
					IOCTL_INTERNAL_USB_SUBMIT_URB,
					m_pLdo,
					NULL,
					0,
					NULL,
					0,
					TRUE, 
					&event,
					&ioStatus);
		if((Irp != NULL) && ((nextStack = IoGetNextIrpStackLocation(Irp)) != NULL))
		{//(Irp != NULL) && (nextStack != NULL)
			nextStack->Parameters.Others.Argument1 = Urb;

			//
			// Set the completion routine, which will signal the event
			//
			IoSetCompletionRoutine(Irp,
								   SyncCompletionRoutine,
								   &event,
								   FALSE,	// InvokeOnSuccess
								   FALSE,	// InvokeOnError
								   TRUE);	// InvokeOnCancel

			ntStatus = IoCallDriver(m_pLdo, Irp);
			if (ntStatus == STATUS_PENDING) 
			{//ntStatus == STATUS_PENDING
				dueTime.QuadPart = (-10000 * USB_COMMAND_TIMEOUT);
				ntStatus = KeWaitForSingleObject(&event, Suspended,	KernelMode,	FALSE, &dueTime);
				if (ntStatus == STATUS_TIMEOUT)
				{
					ntStatus = STATUS_IO_TIMEOUT;

					DBGU_TRACE("UsbDev::SendAwaitUrb STATUS_IO_TIMEOUT, We cancel this IRP!\n");
					// Reset the Event, then Cancel the Irp we just sent.
					KeResetEvent(&event);
					IoCancelIrp(Irp);

					// And wait until the cancel completes
					KeWaitForSingleObject(&event,
										  Executive,
										  KernelMode,
										  FALSE,
										  NULL);
				}
				else
					ntStatus = ioStatus.Status;
			}//ntStatus == STATUS_PENDING 
//			else 
//			{	
//				ioStatus.Status = ntStatus;
//			}
		}//(Irp != NULL) && (nextStack != NULL)
		else
		{//Irp == NULL
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}//Irp == NULL
		DBGU_TRACE("SendAwaitUrb: ntStatus(%X) !\n", ntStatus);
		if(!NT_SUCCESS(ntStatus))
		{
			DBGU_TRACE("SendAwaitUrb: UrbStatus(%X) !\n", Urb->UrbHeader.Status);
			// make it more significant
			if (ntStatus == STATUS_UNSUCCESSFUL)
				ntStatus = Urb->UrbHeader.Status;
		}

//		if ((STATUS_UNSUCCESSFUL == ntStatus) && (USBD_STATUS_DEV_NOT_RESPONDING == Urb->UrbHeader.Status))
//		{
//			KeResetEvent(&event);
//			DBGU_TRACE("SendAwaitUrb: retry(%d)\n", (retry + 1));
//		}
//		else
//		{
//			break;
//		}
	}
	return ntStatus;
}

// Get Usb Device Descriptor
NTSTATUS UsbDev::GetDeviceDescriptor()
{
	NTSTATUS	ntStatus;
    PURB		pUrb;

	DBGU_TRACE(">>>UsbDev::GetDeviceDescriptor !\n");
    pUrb = (PURB) ExAllocatePoolWithTag(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST), USBDEV_POOLTAG);
    if (pUrb) 
	{//pUrb != NULL
		RtlZeroMemory((void *) pUrb, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
        UsbBuildGetDescriptorRequest(
			pUrb,
			(USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
			USB_DEVICE_DESCRIPTOR_TYPE,
			0,
			0,
			&m_DevDesc,
			NULL,
			sizeof(USB_DEVICE_DESCRIPTOR),
			NULL);
        ntStatus = SendAwaitUrb(pUrb);
	    ExFreePoolWithTag(pUrb,USBDEV_POOLTAG);
#if DBG
        if (NT_SUCCESS(ntStatus)) {

			//Print the infomation of device descriptor
            DBGU_TRACE("  Device Descriptor:\n");
            DBGU_TRACE("  -------------------------\n");
            DBGU_TRACE("  bLength %x\n", m_DevDesc.bLength);
            DBGU_TRACE("  bDescriptorType 0x%x\n", m_DevDesc.bDescriptorType);
            DBGU_TRACE("  bcdUSB 0x%x\n", m_DevDesc.bcdUSB);
            DBGU_TRACE("  bDeviceClass 0x%x\n", m_DevDesc.bDeviceClass);
            DBGU_TRACE("  bDeviceSubClass 0x%x\n", m_DevDesc.bDeviceSubClass);
            DBGU_TRACE("  bDeviceProtocol 0x%x\n", m_DevDesc.bDeviceProtocol);
            DBGU_TRACE("  bMaxPacketSize0 0x%x\n", m_DevDesc.bMaxPacketSize0);
            DBGU_TRACE("  idVendor 0x%x\n", m_DevDesc.idVendor);
            DBGU_TRACE("  idProduct 0x%x\n", m_DevDesc.idProduct);
            DBGU_TRACE("  bcdDevice 0x%x\n", m_DevDesc.bcdDevice);
            DBGU_TRACE("  iManufacturer 0x%x\n", m_DevDesc.iManufacturer);
            DBGU_TRACE("  iProduct 0x%x\n", m_DevDesc.iProduct);
            DBGU_TRACE("  iSerialNumber 0x%x\n", m_DevDesc.iSerialNumber);
            DBGU_TRACE("  bNumConfigurations 0x%x\n", m_DevDesc.bNumConfigurations);
            DBGU_TRACE("  -------------------------\n");
			
        } 
		else 
		{
			DBGU_TRACE("ERR: Cannot get device descriptor !!\n");
		}
#endif
    }//pUrb != NULL 
	else 
	{//pUrb == NULL	
		DBGU_TRACE("ERR: Fail to allocate memory for pUrb !!\n");
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }//pUrb == NULL
    return ntStatus;
}

// Get Usb Configuration Descriptor
NTSTATUS UsbDev::GetConfigurationDescriptor()
{
	NTSTATUS						ntStatus = STATUS_SUCCESS;
    PURB							pUrb;
    ULONG							siz;
	UCHAR							index;
	PUSB_CONFIGURATION_DESCRIPTOR	pTemp_pConfigDesc = NULL;

	DBGU_TRACE(">>>UsbDev::GetConfigurationDescriptor !\n");
	pUrb = (PURB) ExAllocatePoolWithTag(NonPagedPool, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST), USBDEV_POOLTAG);
	if(pUrb)
	{//pUrb != NULL	
		RtlZeroMemory(pUrb, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
		for(index=0;index<m_DevDesc.bNumConfigurations;index++)
		{//travel all configurations
			siz = 64;
get_config_descriptor_retry:
			if(pTemp_pConfigDesc) ExFreePoolWithTag(pTemp_pConfigDesc,USBDEV_POOLTAG);
			if ((pTemp_pConfigDesc = (PUSB_CONFIGURATION_DESCRIPTOR) ExAllocatePoolWithTag(NonPagedPool, siz, USBDEV_POOLTAG)))
			{//pTemp_pConfigDesc != NULL
				RtlZeroMemory(pTemp_pConfigDesc,siz);
				UsbBuildGetDescriptorRequest(
					pUrb,
					(USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
					USB_CONFIGURATION_DESCRIPTOR_TYPE,
					index,
					0,
					pTemp_pConfigDesc,
					NULL,
					siz,
					NULL);
				ntStatus = SendAwaitUrb(pUrb);
				if(!NT_SUCCESS(ntStatus))
				{//ntStatus != STATUS_SUCCESS
					DBGU_TRACE("ERR: Fail to get configuration descriptor !!\n");
					ExFreePoolWithTag(pTemp_pConfigDesc,USBDEV_POOLTAG);
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}//ntStatus != STATUS_SUCCESS
				if ((pUrb->UrbControlDescriptorRequest.TransferBufferLength>0) && \
					(pTemp_pConfigDesc->wTotalLength>siz)) 
				{
					siz = pTemp_pConfigDesc->wTotalLength;
					goto get_config_descriptor_retry;
				} 
				m_pConfigDesc[index] = pTemp_pConfigDesc;
				pTemp_pConfigDesc = NULL;
				// Print configuration descriptor
				DBGU_TRACE("  Configuration Descriptor: %x\n",index);
				DBGU_TRACE("  -------------------------\n");
				DBGU_TRACE("  bLength %x\n",m_pConfigDesc[index]->bLength);
				DBGU_TRACE("  bDescriptorType %x\n",m_pConfigDesc[index]->bDescriptorType);
				DBGU_TRACE("  wTotalLength %x\n",m_pConfigDesc[index]->wTotalLength);
				DBGU_TRACE("  bNumInterfaces %x\n",m_pConfigDesc[index]->bNumInterfaces);
				DBGU_TRACE("  bConfigurationValue %x\n",m_pConfigDesc[index]->bConfigurationValue);
				DBGU_TRACE("  iConfiguration %x\n",m_pConfigDesc[index]->iConfiguration);
				DBGU_TRACE("  MaxPower %x\n",m_pConfigDesc[index]->MaxPower);
				DBGU_TRACE("  -------------------------\n");
			}//pTemp_pConfigDesc != NULL
			else
			{//pTemp_pConfigDesc == NULL
				DBGU_TRACE("ERR: Fail to allocate memory for m_pConfigDesc !!\n");
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}//pTemp_pConfigDesc == NULL
		}//travel all configurations
	}//pUrb != NULL	
	else 
	{//pUrb == NULL	
		DBGU_TRACE("ERR: Fail to allocate memory for pUrb !!\n");
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	}//pUrb == NULL	
	if(pUrb) ExFreePoolWithTag(pUrb,USBDEV_POOLTAG);
    return ntStatus;
}

// Set the configuration of the usb device.
NTSTATUS UsbDev::SetConfiguration()
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
    PUSBD_INTERFACE_LIST_ENTRY	pTempInterfaceList = NULL, pTempInterfaceList_First = NULL;
	PUSB_CONFIGURATION_DESCRIPTOR pTemp_pConfigDesc = m_pConfigDesc[m_UsbConfigurationNo];
	int i,j,NoOfInterface;
	PUSB_INTERFACE_DESCRIPTOR pTempInterfaceDesc = NULL;
	PURB pUrb = NULL;
	//2010/8/17 03:49下午
	KIRQL oldIrql;
	PUSBD_INTERFACE_INFORMATION interface_temp;
	
	//2010/8/16 06:17下午
	NoOfInterface = pTemp_pConfigDesc->bNumInterfaces;
	DBGU_TRACE(">>>UsbDev::SetConfiguration ! NoOfInterface = %d\n",NoOfInterface);
	pTempInterfaceList_First = pTempInterfaceList = (PUSBD_INTERFACE_LIST_ENTRY) ExAllocatePoolWithTag( NonPagedPool,	sizeof(USBD_INTERFACE_LIST_ENTRY) * (NoOfInterface + 1), USBDEV_POOLTAG);
	
	//2010/8/23 11:51上午
	m_NumberOfInterfaces = NoOfInterface;
	
	if(pTempInterfaceList)
	{// pTempInterfaceList != NULL
		RtlZeroMemory(pTempInterfaceList, sizeof(USBD_INTERFACE_LIST_ENTRY) * (NoOfInterface + 1));
		for(i=0;i<NoOfInterface;i++) 
		{// for loop i
			pTempInterfaceDesc = (PUSB_INTERFACE_DESCRIPTOR) USBD_ParseConfigurationDescriptorEx( 
														m_pConfigDesc[0],		// Pointer to configuration descriptor
														m_pConfigDesc[0],		// Start position
														i,					// Interface number
														-1,//0,					// Alternate setting
														-1,
														-1,
														-1);
			if(!pTempInterfaceDesc)
				break;
			pTempInterfaceList->InterfaceDescriptor = pTempInterfaceDesc;
			pTempInterfaceList++;
		}// for loop i
	    pTempInterfaceList->InterfaceDescriptor = NULL;	// Mark the end of the list
		//pTempInterfaceList = pTempInterfaceList_First; // Pointer to the first entry of the interface list.
		pUrb = USBD_CreateConfigurationRequestEx(pTemp_pConfigDesc, pTempInterfaceList_First);
		if(pUrb) 
		{// pUrb != NULL
			for(i=0;i<NoOfInterface;i++) 
			{// for loop i
				// Set to interface i, alternater 0
				pTempInterfaceList_First[i].Interface->InterfaceNumber	= i;
				pTempInterfaceList_First[i].Interface->AlternateSetting	= 0;	
				DBGU_TRACE("No of endpoints : %d\n", pTempInterfaceList_First[i].InterfaceDescriptor->bNumEndpoints);
				for(j=0;j<pTempInterfaceList_First[i].InterfaceDescriptor->bNumEndpoints;j++) 
					pTempInterfaceList_First[i].Interface->Pipes[j].MaximumTransferSize = USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;
			}// for loop i
			//2010/8/24 05:51下午
			interface_temp = &pUrb->UrbSelectConfiguration.Interface;
			
			ntStatus = SendAwaitUrb(pUrb);
			if(!NT_SUCCESS(ntStatus)) 
			{// ntStatus != STATUS_SUCCESS
				ntStatus = STATUS_IO_DEVICE_ERROR;
				DBGU_TRACE("Fail to configure usb device !\n");
			}// ntStatus != STATUS_SUCCESS
			else
			{// ntStatus == STATUS_SUCCESS
				m_hConfigureation = pUrb->UrbSelectConfiguration.ConfigurationHandle;
				//Release memory
				//if(m_pInterfaceListEntry)
				{ // m_pInterfaceListEntry != NULL
					for(i=0;i<m_NumberOfInterfaces;i++)
						if(m_pInterface[i])
						{
							ExFreePoolWithTag(m_pInterface[i],USBDEV_POOLTAG);
							m_pInterface[i] = NULL;
							DBGU_TRACE("ExFreePoolWithTag m_pInterface[%d]\n",i);
						}
				//	ExFreePoolWithTag(m_pInterfaceListEntry,USBDEV_POOLTAG);
				//	m_pInterfaceListEntry = NULL;
				} // m_pInterfaceListEntry != NULL
				//2010/8/23 11:51上午
				//m_NumberOfInterfaces = NoOfInterface;
				//Copy interface list
				//m_pInterfaceListEntry = (PUSBD_INTERFACE_LIST_ENTRY) ExAllocatePoolWithTag(NonPagedPool,(m_NumberOfInterfaces * sizeof(USBD_INTERFACE_LIST_ENTRY)), USBDEV_POOLTAG);

				//if(m_pInterfaceListEntry)
				{// m_pInterfaceListEntry != NULL
					
					for(i=0;i<m_NumberOfInterfaces;i++) 
					{//for loop i
						//m_pInterfaceListEntry[i].InterfaceDescriptor = pTempInterfaceList_First[i].InterfaceDescriptor;
#ifdef __VIDEO_CLASS__
						// For Video Class Device
						if (pTempInterfaceList_First[i].Interface->InterfaceNumber==1) // UVC has 2 interfaces
						{
							pTempInterfaceList_First[i].Interface->NumberOfPipes = 1;
							pTempInterfaceList_First[i].Interface->Length = sizeof(USBD_INTERFACE_INFORMATION);
						}
						// james try M2TS.
						if (pTempInterfaceList_First[i].Interface->InterfaceNumber==2)
						{
							pTempInterfaceList_First[i].Interface->NumberOfPipes = 1;
							pTempInterfaceList_First[i].Interface->Length = sizeof(USBD_INTERFACE_INFORMATION);
						}
#endif
						//2010/8/24 05:53下午
//						m_pInterface[i] = (PUSBD_INTERFACE_INFORMATION) ExAllocatePoolWithTag(NonPagedPool, pTempInterfaceList_First[i].Interface->Length, USBDEV_POOLTAG);
						m_pInterface[i] = (PUSBD_INTERFACE_INFORMATION) ExAllocatePoolWithTag(NonPagedPool, interface_temp->Length, USBDEV_POOLTAG);
						//2010/8/17 05:42下午
						//KeAcquireSpinLock(&SetConfigSpinLock, &oldIrql);

						//2010/8/13 05:20下午
						if (m_pInterface[i] && pTempInterfaceList_First[i].Interface && pTempInterfaceList_First[i].Interface->Length >0)
						//if (m_pInterfaceListEntry[i].Interface)
						{
								//2010/8/25 10:52上午
								//DBGU_TRACE("RtlCopyMemory Interface[%d] len = %d\n",i,pTempInterfaceList_First[i].Interface->Length);
								//RtlCopyMemory(m_pInterface[i], pTempInterfaceList_First[i].Interface, pTempInterfaceList_First[i].Interface->Length);
								DBGU_TRACE("MJ RtlCopyMemory Interface[%d] len = %d\n",i,interface_temp->Length);
								RtlCopyMemory(m_pInterface[i], interface_temp, interface_temp->Length);
								m_pInterface[i]->InterfaceNumber	= i;
								m_pInterface[i]->AlternateSetting	= 0;	
								if(i==0)
									m_pInterface[i]->Pipes[0].MaximumTransferSize = USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;
								else
										m_pInterface[i]->NumberOfPipes = 1;

						}
						else
						{
							DBGU_TRACE("RtlCopyMemory Interface[%d] STATUS_INSUFFICIENT_RESOURCES\n",i);
							ntStatus = STATUS_INSUFFICIENT_RESOURCES;
						}	
						//2010/8/17 03:42下午
						//KeReleaseSpinLock(&SetConfigSpinLock, oldIrql);
					}//for loop i
					

				}// m_pInterfaceListEntry != NULL
				/*else
				{// m_pInterfaceListEntry == NULL
					ntStatus = STATUS_INSUFFICIENT_RESOURCES;
				}*/// m_pInterfaceListEntry == NULL
			}// ntStatus == STATUS_SUCCESS
		}// pUrb != NULL
		else
		{// pUrb == NULL
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}// pUrb == NULL
	}// pTempInterfaceList != NULL
	else
	{// pTempInterfaceList == NULL
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	}// pTempInterfaceList == NULL
	if(pUrb) 
		ExFreePool(pUrb);

	if(pTempInterfaceList_First)
	{
		ExFreePoolWithTag(pTempInterfaceList_First,USBDEV_POOLTAG);
		//2010/8/16 07:00下午
		pTempInterfaceList_First = NULL;
	}
	return ntStatus;
}

PUSB_CONFIGURATION_DESCRIPTOR UsbDev::ReturnConfigDesc(UCHAR index)
{
	return m_pConfigDesc[index];
}

// Set usb device to unconfiguration state.
NTSTATUS UsbDev::SetToUnconfigure()
{
	NTSTATUS	ntStatus;
    PURB		pUrb;
    ULONG		siz;

	DBGU_TRACE(">>>UsbDev::Unconfigure !\n");
	siz = sizeof(struct _URB_SELECT_CONFIGURATION);
	pUrb = (PURB) ExAllocatePoolWithTag(NonPagedPool, siz, USBDEV_POOLTAG);
	if (pUrb) {
		UsbBuildSelectConfigurationRequest(pUrb, (USHORT) siz,	NULL);				
		ntStatus = SendAwaitUrb(pUrb);
	} 
	else 
	{	
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
	}
	if(!NT_SUCCESS(ntStatus)) 
		DBGU_TRACE("Fail to Unconfigure ! (Error : %x)\n",ntStatus);
	if(pUrb)
		ExFreePoolWithTag(pUrb,USBDEV_POOLTAG);
	return ntStatus;
}

//2006/3/7 support Choice Full Speed Bandwidth
NTSTATUS UsbDev::SelectUsbBandwidth(PINT pSelectedBandwidth, BOOL fHasAudio)
{
 	NTSTATUS ntStatus=STATUS_SUCCESS;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;
	PUSB_BUS_NOTIFICATION pBandwidthInfo;
	LARGE_INTEGER timeout = {(ULONG)(USB_COMMAND_TIMEOUT * 1000 * -10), -1};
	UINT AudioBandwidth=0;
	UINT AvailableBandwidth;

	if (fHasAudio)
		AudioBandwidth = 64;
	// ISO0, ISO1, ISO2, ISO3, ISO4, ISO5, ISO6, ISO7, ISO8
	UINT Bandwith_Require[] = { 0, 128, 256, 384, 512, 680, 800, 900, 1023};

	//2006/3/14 check pSelectedBandwidth
	if (pSelectedBandwidth == NULL) return STATUS_UNSUCCESSFUL;

	//2006/03/22 free bandwidth before query it
	ntStatus = SetAlternate(0,0);
	if (!NT_SUCCESS(ntStatus))
		return ntStatus;

    if((pBandwidthInfo = (PUSB_BUS_NOTIFICATION)ExAllocatePoolWithTag(NonPagedPool, sizeof(USB_BUS_NOTIFICATION), USBDEV_POOLTAG))==NULL)
		return STATUS_UNSUCCESSFUL;

	memset(pBandwidthInfo, 0, sizeof(USB_BUS_NOTIFICATION));
	pBandwidthInfo->NotificationType = AcquireBusInfo;

    //
    // issue a synchronous request
    //

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
            IOCTL_INTERNAL_USB_GET_BUS_INFO,
            m_pLdo,
            NULL,
            0,
            NULL,
            0,
            TRUE, 
            &event,
            &ioStatus);

    if (irp == NULL ) {
		if (pBandwidthInfo)
			ExFreePoolWithTag(pBandwidthInfo,USBDEV_POOLTAG);
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Call the class driver to perform the operation.  If the returned status
    // is PENDING, wait for the request to complete.
    //

    nextStack= IoGetNextIrpStackLocation(irp);

	 if (nextStack == NULL ) {
		if (pBandwidthInfo)
			ExFreePoolWithTag(pBandwidthInfo,USBDEV_POOLTAG);
        return STATUS_UNSUCCESSFUL;
    }

    //
    // pass the URB to the USB driver stack
    //
    nextStack->Parameters.Others.Argument1 = pBandwidthInfo;

    ntStatus= IoCallDriver(m_pLdo,irp);

    DBGU_TRACE ("return from IoCallDriver USBD %x\n", ntStatus);

    if (ntStatus == STATUS_PENDING) {

        DBGU_TRACE ( "Wait for single object\n");

        ntStatus = KeWaitForSingleObject(&event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       &timeout);

        if (ntStatus == STATUS_TIMEOUT) {                        //
            //
            // USBD did not complete this request in 30 milliseconds, assume
            // that the USBD is hung and return an
            // error.                        
            //
			if (pBandwidthInfo)
				ExFreePoolWithTag(pBandwidthInfo,USBDEV_POOLTAG);
            return(STATUS_UNSUCCESSFUL);                    
        }

        DBGU_TRACE ("Wait for single object, returned %x\n", ntStatus);
        
    } else {
        ioStatus.Status = ntStatus;
    }

    //
    // USBD maps the error code for us
    //
    ntStatus = ioStatus.Status;

	if(ntStatus == STATUS_SUCCESS)
	{
		DBGU_TRACE ("System Usb Total Bandwidth is %d, ConsumedBandwidth is %d\n",pBandwidthInfo->TotalBandwidth, pBandwidthInfo->ConsumedBandwidth);
		AvailableBandwidth = (pBandwidthInfo->TotalBandwidth - pBandwidthInfo->ConsumedBandwidth)/10 - AudioBandwidth;
		DBGU_TRACE ("System Usb Available ISO Bandwidth is %d bytes\n",AvailableBandwidth);
		for(int i=8; i > 0 ; i--)
		{
			if (AvailableBandwidth > Bandwith_Require[i])
			{
				*pSelectedBandwidth = i;
				break;
			}
		}
	}
	if (pBandwidthInfo)
		ExFreePoolWithTag(pBandwidthInfo,USBDEV_POOLTAG);

    return ntStatus;
}

VOID SendDeviceSetPowerComplete(PDEVICE_OBJECT pdo, UCHAR fcn, POWER_STATE state, PUSB_SDSP_CONTEXT context, PIO_STATUS_BLOCK pStatus)
{
	context->ntStatus = pStatus->Status;
	KeSetEvent(context->pEvent, EVENT_INCREMENT, FALSE);
}

NTSTATUS UsbDev::SendDeviceSetPower(
	DEVICE_POWER_STATE devpower, 
	BOOL bWait)
{ 
	POWER_STATE poState;
	NTSTATUS ntStatus;

	poState.DeviceState = devpower;
	if (bWait)
	{
		KEVENT Event;
		USB_SDSP_CONTEXT context;
		
		KeInitializeEvent(&Event, NotificationEvent, FALSE);
		context.pEvent = &Event;
		ntStatus = PoRequestPowerIrp(m_pLdo, IRP_MN_SET_POWER, poState,
			(PREQUEST_POWER_COMPLETE) SendDeviceSetPowerComplete, &context, NULL);
		if (ntStatus == STATUS_PENDING)
		{
			KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
			ntStatus = context.ntStatus;
		}
	}
	else
		ntStatus = PoRequestPowerIrp(m_pLdo, IRP_MN_SET_POWER, poState, NULL, NULL, NULL);

	DBGU_WARNING("SendDeviceSetPower D%d return %X\n", (devpower-1), ntStatus);
	return ntStatus;
}

void SelectiveSuspendCallback(UsbDev *pUsbDev)
{
	DBGU_TRACE("Device goes into Selective Suspend..");

	// Vista cannot accept PowerDeviceD3, that XP does.
	pUsbDev->SendDeviceSetPower(PowerDeviceD2, TRUE);
}

NTSTATUS SelectiveSuspendCompletionRoutine(PDEVICE_OBJECT pdo, PIRP Irp, UsbDev *pUsbDev)
{
	NTSTATUS ntStatus = Irp->IoStatus.Status;
	DBGU_TRACE("SelectiveSuspendCompletionRoutine: 0x%08X\n",ntStatus);
		
	//test 					
	/*if (InterlockedExchangePointer((PVOID *)&pUsbDev->m_SuspendIrp, NULL))
	{
		DBGU_TRACE("IoFreeIrp Suspend IRP at Completion Routine..\n");
		IoFreeIrp(Irp);
	}*/
	InterlockedExchangePointer((PVOID *)&pUsbDev->m_SuspendIrp, NULL);
	DBGU_TRACE("IoFreeIrp Suspend IRP at Completion Routine..\n");
	IoFreeIrp(Irp);

	// fix incorrect power state transition during S0 -> S3 (ymwu, 2005/7/16)
	// this irp will be cancelled when system power state is changed to S3
	// device power state should not be set to D0 in this situation
	// if you make incorrect power state transition, the system sometimes hangs
	{
		DBGU_TRACE("Cancel Selective Suspend..\n");
		pUsbDev->m_SuspendCancelled = 1;
	}
	KeSetEvent(&pUsbDev->m_SuspendIrpCancelEvent, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS UsbDev::SubmitIdleRequestIrp()
{
	NTSTATUS ntStatus=STATUS_UNSUCCESSFUL;
	PIRP	Irp;
	PIO_STACK_LOCATION nextStack;
	//KSPIN_LOCK	m_IdleReqStateLock;
    KIRQL	oldIrql;

	DBGU_TRACE("Enter IssueSelectiveSuspendRequest\n");
	//2010/8/25 06:31下午
	//ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	//KeAcquireSpinLock(&m_IdleReqStateLock, &oldIrql);
	if (!(Irp = IoAllocateIrp(m_pLdo->StackSize, FALSE)))
		return STATUS_INSUFFICIENT_RESOURCES;

	if (InterlockedCompareExchangePointer((PVOID *)&m_SuspendIrp, (PVOID)Irp, NULL) != NULL)
	{
		DBGU_TRACE("Previous SelectiveSuspendRequest exists.. Stop idle!\n");

		//KeReleaseSpinLock(&m_IdleReqStateLock, oldIrql);
		IoFreeIrp(Irp);

		return STATUS_UNSUCCESSFUL;
	}

	m_cbInfo.IdleCallback = (USB_IDLE_CALLBACK)SelectiveSuspendCallback;
	m_cbInfo.IdleContext = (PVOID)this;

	nextStack = IoGetNextIrpStackLocation(Irp);
	nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION;
	nextStack->Parameters.DeviceIoControl.Type3InputBuffer = &m_cbInfo;

	m_SuspendCancelled = 0;
	KeInitializeEvent(&m_SuspendIrpCancelEvent, SynchronizationEvent, FALSE);

	IoSetCompletionRoutine(Irp,
		(PIO_COMPLETION_ROUTINE)SelectiveSuspendCompletionRoutine,
		(PVOID)this, TRUE, TRUE, TRUE);

//	KeReleaseSpinLock(&m_IdleReqStateLock, oldIrql);

	ntStatus = IoCallDriver(m_pLdo, Irp);

	DBGU_TRACE("IssueSelectiveSuspendRequest:STATUS_SUCCESS\n");
	return STATUS_SUCCESS;
}

VOID UsbDev::CancelSelectiveSuspend()
{
	PIRP Irp;
	LARGE_INTEGER timeout;
	KIRQL oldIrql;
	DBGU_TRACE("CancelSelectiveSuspend()\n");
	//test
	//Irp = (PIRP)InterlockedExchangePointer((PVOID *)&m_SuspendIrp, NULL);

	timeout.QuadPart = -600000000; //60 sec
	
	KeAcquireSpinLock(&IdleReqStateLock, &oldIrql);
	
	if (m_SuspendIrp)//(Irp)
	{
   		KeInitializeEvent(&m_SuspendIrpCancelEvent, SynchronizationEvent, FALSE);
			DBGU_TRACE("IoCancelIrp(Irp)\n");
			IoCancelIrp(m_SuspendIrp);
			KeReleaseSpinLock(&IdleReqStateLock, oldIrql);
			// waiting for IRP been cancelled.
			KeWaitForSingleObject(
				&m_SuspendIrpCancelEvent,
				Executive,
				KernelMode,
				FALSE,
				&timeout);//NULL);
			DBGU_TRACE("IoCancelIrp(Irp) Complete\n");	
	}
	else
		KeReleaseSpinLock(&IdleReqStateLock, oldIrql);
}

//2010/8/23 03:09下午
// Initialize usb device.
NTSTATUS UsbDev::UninitializeUsbDevice()
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	/*DBGU_TRACE(">>>UsbDev::UninitializeUsbDevice !\n");
	for(int i=0;i<MAX_CONFIGURATION_NO;i++)
		if(m_pConfigDesc[i]) ExFreePoolWithTag(m_pConfigDesc[i],USBDEV_POOLTAG);
	
	if(m_pInterfaceListEntry)
	{ // m_pInterfaceListEntry != NULL
		for(int i=0;i<m_NumberOfInterfaces;i++)
			if(m_pInterfaceListEntry[i].Interface) 
			{
				ExFreePoolWithTag(m_pInterfaceListEntry[i].Interface,USBDEV_POOLTAG);
				//2010/8/16 06:31下午
				m_pInterfaceListEntry[i].Interface = NULL;
			}
		ExFreePoolWithTag(m_pInterfaceListEntry,USBDEV_POOLTAG);
		m_pInterfaceListEntry = NULL;
	} // m_pInterfaceListEntry != NULL*/
	return ntStatus;
}