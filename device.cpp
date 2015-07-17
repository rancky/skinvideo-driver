/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

    File:

        device.cpp

    Abstract:

        This file contains the device level implementation of the AVStream
        hardware sample.  Note that this is not the "fake" hardware.  The
        "fake" hardware is in hwsim.cpp.

    History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/

#include "SnCam.h"



CCaptureDevice::CCaptureDevice (IN PKSDEVICE Device) :  m_Device (Device)
{
	// initialize all members
	DBGU_TRACE("CCaptureDevice()\n");
	RtlZeroMemory(&pdx,sizeof(pdx));
	RtlZeroMemory(m_PinsWithResources,sizeof(m_PinsWithResources));
	RtlZeroMemory(m_PinsConnects,sizeof(m_PinsConnects));
	RtlZeroMemory(m_VideoInfoHeader,sizeof(m_VideoInfoHeader));
}

CCaptureDevice::~CCaptureDevice ()
{
	DBGU_TRACE("~CCaptureDevice()\n");
	
	if (pdx->m_pStillFrameBuffer)
		delete pdx->m_pStillFrameBuffer;
	
	pdx->m_pStillFrameBuffer=NULL;
}


/*************************************************

    Global Variables

*************************************************/

NTSTATUS
CCaptureDevice::DispatchCreate (
    IN PKSDEVICE Device
    )

/*++

Routine Description:

    Create the capture device.  This is the creation dispatch for the
    capture device.

Arguments:

    Device -
        The AVStream device being created.

Return Value:

    Success / Failure

--*/

{
    PAGED_CODE();

	DBGU_TRACE("CCaptureDevice::DispatchCreate()\n");

    NTSTATUS Status;
//	SnPrint(DEBUGLVL_VERBOSE, ("Enter CCaptureDevice::DispatchCreate\n"));

    CCaptureDevice *pCapDevice = new (NonPagedPool) CCaptureDevice(Device);
	CVideoStream *pVideoStream = new (NonPagedPool) CVideoStream(Device);

	if (!pVideoStream || !pCapDevice) {
        //
        // Return failure if we couldn't create the object.
        //
		if (pCapDevice)
			delete pCapDevice;
		
		if (pVideoStream)
			delete pVideoStream;

        Status = STATUS_INSUFFICIENT_RESOURCES;
	} else	{
        //
        // Add the item to the object bag if we were successful.
        // Whenever the device goes away, the bag is cleaned up and
        // we will be freed.
        //
        // For backwards compatibility with DirectX 8.0, we must grab
        // the device mutex before doing this.  For Windows XP, this is
        // not required, but it is still safe.
        //
        KsAcquireDevice (Device);
        Status = KsAddItemToObjectBag (
            Device -> Bag,
            reinterpret_cast <PVOID> (pCapDevice),
            reinterpret_cast <PFNKSFREE> (CCaptureDevice::Cleanup)
            );
        KsReleaseDevice (Device);

        if (!NT_SUCCESS (Status)) {
            delete pCapDevice;
			return Status;
        } else {
            Device -> Context = reinterpret_cast <PVOID> (pCapDevice);
        }

		//
		//	add CStreamHandler Object into bag
		//
        KsAcquireDevice (Device);
        Status = KsAddItemToObjectBag (
            Device -> Bag,
            reinterpret_cast <PVOID> (pVideoStream),
            reinterpret_cast <PFNKSFREE> (CVideoStream::Cleanup)
            );
        KsReleaseDevice (Device);

        if (!NT_SUCCESS (Status)) {
            delete pVideoStream;
			return Status;
        } else {
			pCapDevice->pdx->pVideoStream = pVideoStream;
		}
	}

	SnPrint(DEBUGLVL_VERBOSE, ("Leave CCaptureDevice::DispatchCreate return %X\n",Status));
	return Status;
}

VOID CCaptureDevice::RemoveDevice(
	)
/*++

Routine Description:

	routine is called when an IRP_MN_REMOVE_DEVICE is dispatched by the device.

Arguments:

	None

--*/
{
//	SnPrint(DEBUGLVL_VERBOSE, ("Enter CCaptureDevice::RemoveDevice\n"));
	DBGU_TRACE("Enter CCaptureDevice::RemoveDevice\n");

	//2010/8/12 03:41¤U¤È
	/*KsAcquireDevice(m_Device);
	
	if (pdx->pVideoStream)
	{
		pdx->pVideoStream->UninitializeDevice();
	}

	KsReleaseDevice(m_Device);
	*/
	SnPrint(DEBUGLVL_VERBOSE, ("Leave CCaptureDevice::RemoveDevice\n"));
}

/*************************************************/

NTSTATUS CCaptureDevice::PnpStart (
    IN PCM_RESOURCE_LIST TranslatedResourceList,
    IN PCM_RESOURCE_LIST UntranslatedResourceList
    )

/*++

Routine Description:

    Called at Pnp start.  We start up our hardware.

Arguments:

    TranslatedResourceList -
        The translated resource list from Pnp

    UntranslatedResourceList -
        The untranslated resource list from Pnp

Return Value:

    Success / Failure

--*/

{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;
	SnPrint(DEBUGLVL_VERBOSE, ("Enter CCaptureDevice::PnpStart\n"));

	//
	// Device is already started.
	//
	if (m_Device -> Started)
		return Status;

	if (!pdx->pVideoStream)
		return STATUS_INSUFFICIENT_RESOURCES;

	//
	// Initialize Device
	//
	KsAcquireDevice(m_Device);
	
	if (!NT_SUCCESS(pdx->pVideoStream->InitializeDevice()))
	{
		DBGU_TRACE("pVideoStream->InitializeDevice() return failed!!!\n");
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}
    
	KsReleaseDevice(m_Device);
	
	SnPrint(DEBUGLVL_VERBOSE, ("Leave CCaptureDevice::PnpStart\n"));
    return Status;
}

//
// DispatchPostStart
//
NTSTATUS CCaptureDevice::PostStart ()
{
    PAGED_CODE();
	ASSERT(m_Device);

	NTSTATUS ntStatus = STATUS_SUCCESS;
	
	if (!pdx->pVideoStream)
		return STATUS_INSUFFICIENT_RESOURCES;

	SnPrint(DEBUGLVL_VERBOSE, ("Enter CCaptureDevice::PostStart\n"));
	KsAcquireDevice(m_Device);
	ntStatus = pdx->pVideoStream->InitializationComplete();
	KsReleaseDevice(m_Device);
	
    //
    // Normally, we'd do things here like parsing the resource lists and
    // connecting our interrupt.  The parsing and connection should be the same as
    // any WDM driver.  The sections that will differ are illustrated below
    // in setting up a simulated DMA.
    //
	if (NT_SUCCESS(ntStatus)) {
		// Create the Filter for the device
		KsAcquireDevice(m_Device);
		ntStatus = KsCreateFilterFactory( m_Device->FunctionalDeviceObject,
										&pdx->m_CaptureFilterDescriptor,	// shawn 2011/06/21 modify for multi-devices
										L"GLOBAL",
										NULL,
										KSCREATE_ITEM_FREEONSTOP,
										NULL,
										NULL,
										NULL );
		KsReleaseDevice(m_Device);

	}

	SnPrint(DEBUGLVL_VERBOSE, ("Leave CCaptureDevice::PostStart\n"));

	return ntStatus;
}


/*************************************************/


void
CCaptureDevice::PnpStop ()

/*++

Routine Description:

    This is the pnp stop dispatch for the capture device.  It releases any
    adapter object previously allocated by IoGetDmaAdapter during Pnp Start.

Arguments:

    None

Return Value:

    None

--*/

{
	SnPrint(DEBUGLVL_VERBOSE, ("Enter CCaptureDevice::PnpStop\n"));
	//2010/8/12 03:41¤U¤È
	KsAcquireDevice(m_Device);
	
	if(pdx->pVideoStream)
	{
		pdx->pVideoStream->UninitializeDevice();
	} 
    
	KsReleaseDevice(m_Device);
	SnPrint(DEBUGLVL_VERBOSE, ("Leave CCaptureDevice::PnpStop\n"));
}

void
CCaptureDevice::SurpriseRemoval ()

/*++

Routine Description:

    This is the pnp stop dispatch for the capture device.  It releases any
    adapter object previously allocated by IoGetDmaAdapter during Pnp Start.

Arguments:

    None

Return Value:

    None

--*/

{
	PAGED_CODE();

	SnPrint(DEBUGLVL_VERBOSE, ("Enter CCaptureDevice::SurpriseRemoval\n"));

	KsAcquireDevice(m_Device);
	
	if (pdx->pVideoStream)
		pdx->pVideoStream->SurpriseRemoval();
	
	KsReleaseDevice(m_Device);

	SnPrint(DEBUGLVL_VERBOSE, ("Leave CCaptureDevice::SurpriseRemoval\n"));
}

/*************************************************/


NTSTATUS
CCaptureDevice::AcquireHardwareResources (
    IN PKS_VIDEOINFOHEADER VideoInfoHeader,
	IN ULONG StreamNumber
    )

/*++

Routine Description:

    Acquire hardware resources for the capture hardware.  If the 
    resources are already acquired, this will return an error.
    The hardware configuration must be passed as a VideoInfoHeader.

Arguments:

    VideoInfoHeader -
        Information about the capture stream.  This **MUST** remain
        stable until the caller releases hardware resources.  Note
        that this could also be guaranteed by bagging it in the device
        object bag as well.

Return Value:

    Success / Failure

--*/

{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;
	DBGU_TRACE("Enter CCaptureDevice::AcquireHardwareResources\n");

    //
    // If we're the first pin to go into acquire (remember we can have
    // a filter in another graph going simultaneously), grab the resources.
    //
    if (InterlockedCompareExchange(&m_PinsWithResources[StreamNumber], 1, 0) == 0)
	{
		//2010/4/21 02:53¤U¤È
		if(StreamNumber == pdx->m_STREAM_Capture_MP2TS_Idx)	// shawn 2011/06/23 modify
		{
			DBGU_TRACE("Format M2TS Match!\n");
			Status = STATUS_SUCCESS;
		}
		else
		{	
			m_VideoInfoHeader[StreamNumber] = VideoInfoHeader;

			DBGU_TRACE("X=%d, Y=%d, BI=%d, CP=%d\n",m_VideoInfoHeader[StreamNumber]->bmiHeader.biWidth,
				m_VideoInfoHeader[StreamNumber]->bmiHeader.biHeight,
				m_VideoInfoHeader[StreamNumber]->bmiHeader.biBitCount,
				m_VideoInfoHeader[StreamNumber]->bmiHeader.biCompression);
			
			//
			// Create the necessary type of image synthesizer.
			//
			if (m_VideoInfoHeader[StreamNumber] -> bmiHeader.biBitCount == 24 &&
				m_VideoInfoHeader[StreamNumber] -> bmiHeader.biCompression == KS_BI_RGB)
			{
				DBGU_TRACE("Format RGB24 Match!\n");
				Status = STATUS_SUCCESS;
			}
			else if(m_VideoInfoHeader[StreamNumber] -> bmiHeader.biBitCount == 12 &&
				m_VideoInfoHeader[StreamNumber] -> bmiHeader.biCompression == MEDIASUBTYPE_I420.Data1)
			{
				DBGU_TRACE("Format I420 Match!\n");
				Status = STATUS_SUCCESS;
			}
			else if(m_VideoInfoHeader[StreamNumber] -> bmiHeader.biBitCount == 16 &&
				m_VideoInfoHeader[StreamNumber] -> bmiHeader.biCompression == MEDIASUBTYPE_YUY2.Data1)
			{
				DBGU_TRACE("Format YUY2 Match!\n");
				Status = STATUS_SUCCESS;
			}
			else if(m_VideoInfoHeader[StreamNumber] -> bmiHeader.biCompression == MEDIASUBTYPE_MJPG.Data1)
			{
				DBGU_TRACE("Format MJPG Match!\n");
				Status = STATUS_SUCCESS;
			}
			else
			{
				DBGU_WARNING("Failure : Synthesize none of RGB24, I420, YUY2 and MJPG!\n");
				Status = STATUS_INVALID_PARAMETER;
			}

			if (NT_SUCCESS(Status) && StreamNumber == STREAM_Capture) {
				ASSERT(pdx->m_pStillFrameBuffer == NULL);	// m_pStillFrameBuffer not be freed before re-allocate

				//RBK move huge data buffer to PagedPool
				pdx->m_pStillFrameBuffer = new (PagedPool) BYTE[/*1600*1200*3*/2048*1536*3];	// shawn 2011/07/12 modify for 3M
				
				if (!pdx->m_pStillFrameBuffer)
					Status = STATUS_INSUFFICIENT_RESOURCES;

				DBGU_TRACE("Allocate Still Frame Buffer\n");
			}
		}
	} else {
        //
        // TODO: Better status code?
        //
        Status = STATUS_SHARING_VIOLATION;
    }

	DBGU_TRACE("Leave CCaptureDevice::AcquireHardwareResources with status = %X\n",Status);
    return Status;
}

/*************************************************/


void
CCaptureDevice::ReleaseHardwareResources (
	IN ULONG StreamNumber
    )

/*++

Routine Description:

    Release hardware resources.  This should only be called by
    an object which has acquired them.

Arguments:

    None

Return Value:

    None

--*/

{
    PAGED_CODE();
    m_VideoInfoHeader[StreamNumber] = NULL;
	DBGU_TRACE("Enter CCaptureDevice::ReleaseHardwareResources\n");

	if (pdx->m_pStillFrameBuffer)
		delete pdx->m_pStillFrameBuffer;

	pdx->m_pStillFrameBuffer=NULL;

	//
	// Release our "lock" on hardware resources.  This will allow another
	// pin (perhaps in another graph) to acquire them.
	//
	InterlockedExchange (&m_PinsWithResources[StreamNumber], 0);
}

/*************************************************/
NTSTATUS CCaptureDevice::StreamOpen(BOOL fOpen, IN PKSPIN pin)
{
    PAGED_CODE();
	NTSTATUS ntStatus = STATUS_SUCCESS;

	if (!pdx->pVideoStream)
		return STATUS_INSUFFICIENT_RESOURCES;

	if (fOpen)
	{
		// check if capture pin was opened before still pin
		if ((pin->Id == pdx->m_STREAM_Still_Idx)&&(m_PinsConnects[STREAM_Capture] == 0))	// shawn 2011/06/23 modify
		{
			DBGU_WARNING("Error: Open still pin before capture opened..\n");
			return STATUS_UNSUCCESSFUL;
		}

		ntStatus = pdx->pVideoStream->OpenStream(pin);
		
		if (NT_SUCCESS(ntStatus))
			InterlockedIncrement(&m_PinsConnects[pin->Id]);
	}
	else
	{
		ntStatus = pdx->pVideoStream->CloseStream(pin);
		
		if (NT_SUCCESS(ntStatus))
			InterlockedDecrement(&m_PinsConnects[pin->Id]);
	}

	return ntStatus;
}

NTSTATUS
CCaptureDevice::Start (IN PKSPIN pin)

/*++

Routine Description:

    Start the capture device based on the video info header we were told
    about when resources were acquired.

Arguments:

    None

Return Value:

    Success / Failure

--*/

{
    PAGED_CODE();
	DBGU_TRACE("Enter CCaptureDevice::Start\n");
	
	if (!pdx->pVideoStream)
		return STATUS_INSUFFICIENT_RESOURCES;

	return pdx->pVideoStream->SetStreamState(pin, KSSTATE_RUN);
}

/*************************************************/


NTSTATUS
CCaptureDevice::Pause(
	IN PKSPIN pin,
    IN BOOLEAN Pausing
    )

/*++

Routine Description:

    Pause or unpause the hardware.  This is an effective start
    or stop without resetting counters and formats.  Note that this can
    only be called to transition from started -> paused -> started.  Calling
    this without starting the hardware with Start() does nothing.

Arguments:

    Pausing -
        An indicatation of whether we are pausing or unpausing

        TRUE -
            Pause the hardware

        FALSE -
            Unpause the hardware

Return Value:

    Success / Failure

--*/

{
    PAGED_CODE();
	DBGU_TRACE("Enter CCaptureDevice::Pause\n");
	
	if (!pdx->pVideoStream)
		return STATUS_INSUFFICIENT_RESOURCES;

	if (Pausing)
		return pdx->pVideoStream->SetStreamState(pin, KSSTATE_PAUSE);
	else
		return pdx->pVideoStream->SetStreamState(pin, KSSTATE_RUN);
}

/*************************************************/


NTSTATUS
CCaptureDevice::Stop (IN PKSPIN pin)

/*++

Routine Description:

    Stop the capture device.

Arguments:

    None

Return Value:

    Success / Failure

--*/

{
    PAGED_CODE();
	DBGU_TRACE("Enter CCaptureDevice::Stop\n");

	if (!pdx->pVideoStream)
		return STATUS_INSUFFICIENT_RESOURCES;

	return pdx->pVideoStream->SetStreamState(pin, KSSTATE_STOP);
}

/*************************************************/

/*++

Routine Description:

    Indicate frame processing.

Arguments:

     ByteCount - total buffer size

Return Value:

	Return STATUS_SUCCESS to continue processing.
	Return STATUS_PENDING to stop processing until the next triggering event.
	The minidriver may return an error code, but this will be treated as described for STATUS_PENDING.

--*/
NTSTATUS CCaptureDevice::FrameReadingProcess(
	IN PKSPIN pin,
	IN PKSSTREAM_HEADER pDataPacket
	)
{
	PAGED_CODE();
	NTSTATUS ntStatus = STATUS_SUCCESS;
	
	if (!pdx->pVideoStream)
		return STATUS_INSUFFICIENT_RESOURCES;

	if (pin->DeviceState == KSSTATE_RUN)
	{
		ntStatus = pdx->pVideoStream->ReadData(pin, pDataPacket, 1);
	}

    //
    // DEVICE_NOT_READY indicates that the advancement ran off the end
    // of the queue.  We couldn't lock the leading edge.
    //
    if (ntStatus == STATUS_DEVICE_NOT_READY)
		ntStatus = STATUS_SUCCESS;

	return ntStatus;
}

NTSTATUS	CCaptureDevice::QueryCapabilities(
	IN OUT PDEVICE_CAPABILITIES  pCapabilities
)
{
	PAGED_CODE();

	//
	// We don't want "safely remove hardware" icon appeared at System Tray
	//
	DBGU_TRACE("Enter CCaptureDevice::QueryCapabilities\n");
	pCapabilities->SurpriseRemovalOK = TRUE;
	DBGU_TRACE("Leave CCaptureDevice::QueryCapabilities\n");

	return STATUS_SUCCESS;
}



void
CCaptureDevice::SetPowerState (
	IN DEVICE_POWER_STATE  To,
	IN DEVICE_POWER_STATE  From
    )
{
	PAGED_CODE();
	ASSERT(m_Device);
	
	if (!pdx->pVideoStream)
		return;

	SnPrint(DEBUGLVL_VERBOSE, ("Enter CCaptureDevice::SetPowerState from D%d\n", (From-1)));
	KsAcquireDevice(m_Device);
	pdx->pVideoStream->ChangePowerState(To,From);
	KsReleaseDevice(m_Device);
	SnPrint(DEBUGLVL_VERBOSE, ("Leave CCaptureDevice::SetPowerState to D%d\n", (To-1)));
}


/**************************************************************************

    DESCRIPTOR AND DISPATCH LAYOUT

**************************************************************************/
/*
//
// CaptureFilterDescriptor:
//
// The filter descriptor for the capture device.
DEFINE_KSFILTER_DESCRIPTOR_TABLE (FilterDescriptors) { 
    &CaptureFilterDescriptor
};
*/
//
// CaptureDeviceDispatch:
//
// This is the dispatch table for the capture device.  Plug and play
// notifications as well as power management notifications are dispatched
// through this table.
//
const
KSDEVICE_DISPATCH CaptureDeviceDispatch = {
    CCaptureDevice::DispatchCreate,         // Pnp Add Device
    CCaptureDevice::DispatchPnpStart,       // Pnp Start
    CCaptureDevice::DispatchPostStart,		// Post-Start
    NULL,                                   // Pnp Query Stop
    NULL,                                   // Pnp Cancel Stop
    CCaptureDevice::DispatchPnpStop,        // Pnp Stop
    NULL,                                   // Pnp Query Remove
    NULL,                                   // Pnp Cancel Remove
    CCaptureDevice::DispatchRemove,			// Pnp Remove
    CCaptureDevice::DispatchQueryCapabilities,		// Pnp Query Capabilities
    CCaptureDevice::DispatchPnpSurpriseRemoval,		// Pnp Surprise Removal
    NULL,                                   // Power Query Power
    CCaptureDevice::DispatchSetPower,		// Power Set Power
    NULL                                    // Pnp Query Interface
};

//
// CaptureDeviceDescriptor:
//
// This is the device descriptor for the capture device.  It points to the
// dispatch table and contains a list of filter descriptors that describe
// filter-types that this device supports.  Note that the filter-descriptors
// can be created dynamically and the factories created via 
// KsCreateFilterFactory as well.  
//

const
KSDEVICE_DESCRIPTOR CaptureDeviceDescriptor = {
    &CaptureDeviceDispatch,
    0,
    NULL
};

/**************************************************************************

    INITIALIZATION CODE

**************************************************************************/


extern "C"
NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

    Driver entry point.  Pass off control to the AVStream initialization
    function (KsInitializeDriver) and return the status code from it.

Arguments:

    DriverObject -
        The WDM driver object for our driver

    RegistryPath -
        The registry path for our registry info

Return Value:

    As from KsInitializeDriver

--*/

{
	//
	// Simply pass the device descriptor and parameters off to AVStream
	// to initialize us.  This will cause filter factories to be set up
	// at add & start.  Everything is done based on the descriptors passed
	// here.
	//
	return  KsInitializeDriver (
				DriverObject,
				RegistryPath,
				&CaptureDeviceDescriptor
			);
}
