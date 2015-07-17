/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

    File:

        still.cpp

    Abstract:

        This file contains source for the video capture pin on the capture
        filter.  The capture sample performs "fake" DMA directly into
        the capture buffers.  Common buffer DMA will work slightly differently.

        For common buffer DMA, the general technique would be DPC schedules
        processing with KsPinAttemptProcessing.  The processing routine grabs
        the leading edge, copies data out of the common buffer and advances.
        Cloning would not be necessary with this technique.  It would be 
        similiar to the way "AVSSamp" works, but it would be pin-centric.

    History:

		created 2007/05/03 [Saxen Ko]

**************************************************************************/

#include "SnCam.h"

/**************************************************************************

    PAGEABLE CODE

**************************************************************************/


#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif // ALLOC_PRAGMA

CStillPin::CStillPin (
    IN PKSPIN Pin
    ) :
	m_Pin (Pin),
//    ,m_SurfaceType (KS_CAPTURE_ALLOC_SYSTEM)
	m_PresentationTime (0)

/*++

Routine Description:

    Construct a new capture pin.

Arguments:

    Pin -
        The AVStream pin object corresponding to the capture pin

Return Value:

    None

--*/

{
	PAGED_CODE();

	PKSDEVICE Device = KsPinGetDevice (Pin);

	//
	// Set up our device pointer.  This gives us access to "hardware I/O"
	// during the capture routines.
	//
	m_Device = reinterpret_cast <CCaptureDevice *> (Device -> Context);
}

/*************************************************/


NTSTATUS
CStillPin::
DispatchCreate (
    IN PKSPIN Pin,
    IN PIRP Irp
    )

/*++

Routine Description:

    Create a new capture pin.  This is the creation dispatch for
    the video capture pin.

Arguments:

    Pin -
        The pin being created

    Irp -
        The creation Irp

Return Value:

    Success / Failure

--*/

{
	PAGED_CODE();

	NTSTATUS Status = STATUS_SUCCESS;
	DBGU_TRACE("Enter CStillPin::DispatchCreate\n");

	CStillPin *StiPin = new (NonPagedPool) CStillPin (Pin);

	if (!StiPin) {
		//
		// Return failure if we couldn't create the pin.
		//
		Status = STATUS_INSUFFICIENT_RESOURCES;
	} else {
		//
		// Add the item to the object bag if we we were successful. 
		// Whenever the pin closes, the bag is cleaned up and we will be
		// freed.
		//
		Status = KsAddItemToObjectBag (
			Pin -> Bag,
			reinterpret_cast <PVOID> (StiPin),
			reinterpret_cast <PFNKSFREE> (CStillPin::Cleanup)
			);

		if (!NT_SUCCESS (Status)) {
			delete StiPin;
		} else {
			Pin -> Context = reinterpret_cast <PVOID> (StiPin);
		}
	}

	//
	// If we succeeded so far, stash the video info header away and change
	// our allocator framing to reflect the fact that only now do we know
	// the framing requirements based on the connection format.
	//
	PKS_VIDEOINFOHEADER VideoInfoHeader = NULL;		// for Vista change to KS_VIDEOINFOHEADER2

	if (NT_SUCCESS (Status)) {

		VideoInfoHeader = StiPin -> CaptureVideoInfoHeader ();
		
		if (!VideoInfoHeader) {
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

	if (NT_SUCCESS (Status)) {

		//
		// We need to edit the descriptor to ensure we don't mess up any other
		// pins using the descriptor or touch read-only memory.
		//
		Status = KsEdit (
			Pin, 
			&Pin -> Descriptor, 
			AVSHWS_POOLTAG);

		if (NT_SUCCESS (Status)) {
			do {
				PKSFILTER Filter = (PKSFILTER)KsGetParent(Pin);
	            
				if (!Filter) {
					Status = STATUS_UNSUCCESSFUL;
					break;
				}

				CCaptureFilter* ParentFilter = static_cast<CCaptureFilter*>(Filter -> Context);

				if (!ParentFilter) {
					Status = STATUS_UNSUCCESSFUL;
					break;
				}
			} while (FALSE);
		}

		//
		// If the edits proceeded without running out of memory, adjust 
		// the framing based on the video info header.
		//
		Status = KsEdit (
			Pin, 
			&Pin -> Descriptor -> AllocatorFraming, 
			AVSHWS_POOLTAG);

		if (NT_SUCCESS (Status)) {

			//
			// We've KsEdit'ed this...  I'm safe to cast away constness as
			// long as the edit succeeded.
			//
			PKSALLOCATOR_FRAMING_EX Framing =
				const_cast <PKSALLOCATOR_FRAMING_EX> (
					Pin -> Descriptor -> AllocatorFraming
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
				VideoInfoHeader -> bmiHeader.biSizeImage;

			Framing -> FramingItem [0].PhysicalRange.Stepping = 
				Framing -> FramingItem [0].FramingRange.Range.Stepping =
				0;
		}
	}

	PKSDEVICE pDevice = KsPinGetDevice (Pin);
	CCaptureDevice *pCapDevice = reinterpret_cast <CCaptureDevice *> (pDevice -> Context);

	if (!pCapDevice)
		Status = STATUS_UNSUCCESSFUL;

	if (NT_SUCCESS (Status))
	{
		Status = pCapDevice->StreamOpen(TRUE,Pin);
	}

	DBGU_TRACE("Exit CStillPin::DispatchCreate Status= %X\n",Status);
	return Status;
}

NTSTATUS
CStillPin::
DispatchClose (
    IN PKSPIN Pin,
    IN PIRP Irp
    )
{
	PAGED_CODE();

	NTSTATUS Status = STATUS_SUCCESS;
	DBGU_TRACE("Enter CStillPin::DispatchClose\n");

	PKSDEVICE pDevice = KsPinGetDevice (Pin);
	CCaptureDevice *pCapDevice = reinterpret_cast <CCaptureDevice *> (pDevice -> Context);

	if (!pCapDevice)
		Status = STATUS_UNSUCCESSFUL;

	if (NT_SUCCESS (Status))
	{
		Status = pCapDevice->StreamOpen(FALSE,Pin);
	}

	return Status;
}
/*************************************************/


PKS_VIDEOINFOHEADER 
CStillPin::
CaptureVideoInfoHeader (
    )

/*++

Routine Description:

    Capture the video info header out of the connection format.  This
    is what we use to base synthesized images off.

Arguments:

    None

Return Value:

    The captured video info header or NULL if there is insufficient
    memory.

--*/

{

	PAGED_CODE();
	DBGU_TRACE("Enter CStillPin::CaptureVideoInfoHeader\n");

	PKS_VIDEOINFOHEADER ConnectionHeader =
		&((reinterpret_cast <PKS_DATAFORMAT_VIDEOINFOHEADER> 
			(m_Pin -> ConnectionFormat)) -> VideoInfoHeader);	// for KS_VIDEOINFOHEADER2

	m_VideoInfoHeader = reinterpret_cast <PKS_VIDEOINFOHEADER> (
		ExAllocatePoolWithTag (
			NonPagedPool,
			KS_SIZE_VIDEOHEADER (ConnectionHeader),
			AVSHWS_POOLTAG
			)
		);

	if (!m_VideoInfoHeader)
		return NULL;

	//
	// Bag the newly allocated header space.  This will get cleaned up
	// automatically when the pin closes.
	//
	NTSTATUS Status =
		KsAddItemToObjectBag (
			m_Pin -> Bag,
			reinterpret_cast <PVOID> (m_VideoInfoHeader),
			NULL
			);

	if (!NT_SUCCESS (Status)) {
		ExFreePoolWithTag (m_VideoInfoHeader);	m_VideoInfoHeader = NULL;
		return NULL;
	} else {
		//
		// Copy the connection format video info header into the newly 
		// allocated "captured" video info header.
		//
		RtlCopyMemory (
			m_VideoInfoHeader,
			ConnectionHeader,
			KS_SIZE_VIDEOHEADER (ConnectionHeader)
			);
	}

	return m_VideoInfoHeader;
}

void CStillPin::InvokePinProcess ()
{
    PAGED_CODE();
    NTSTATUS Status = STATUS_SUCCESS;

	DBGU_TRACE("CStillPin::InvokePinProcess\n");
	ASSERT(m_Pin);
	ASSERT(m_Device);

	//
	// Initialize the PinGate to allow processing
	//
	PKSGATE pGate = KsPinGetAndGate(m_Pin);
	if (pGate)
	{
		//KsGateTurnInputOn(pGate);
		KsGateInitializeAnd(pGate, NULL);
	}

	//
	// Kick processing to happen.
	//
	KsPinAttemptProcessing (m_Pin, TRUE);
}


NTSTATUS CStillPin::Process ()

/*++

Routine Description:

    The process dispatch for the pin bridges to this location.
    We handle setting up scatter gather mappings, etc...

Arguments:

    None

Return Value:

    Success / Failure

--*/

{
    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;
    PKSSTREAM_POINTER Leading=NULL;

	SnPrint(DEBUGLVL_VERBOSE, ("Enter CStillPin::Process\n"));
	ASSERT(m_Pin);

	if (!m_Device)
	{
		DBGU_ERROR("DeviceState is not KSSTATE_RUN or m_Device of CStillPin is NULL!\n");
		return STATUS_UNSUCCESSFUL;
	}

	if (m_Pin->DeviceState != KSSTATE_RUN)
	{
		DBGU_WARNING("Stream State is not KSSTATE_RUN ..\n");
		return STATUS_UNSUCCESSFUL;
	}

	KsPinAcquireProcessingMutex(m_Pin);

    Leading = KsPinGetLeadingEdgeStreamPointer (
        m_Pin,
        KSSTREAM_POINTER_STATE_LOCKED
        );

	//
	// Find stream pointer that has Data buffer
	//
	while (NT_SUCCESS (Status) && Leading)
	{
		//
		// If no data is present in the Leading edge stream pointer, just 
		// move on to the next frame
		//
		if ( NULL == Leading -> StreamHeader -> Data ) {
			Status = KsStreamPointerAdvance(Leading);
			continue;
		}

		break;
	}

	if (Leading && m_VideoInfoHeader)
	{
		//
		// fill Stream header
		//
		Leading->StreamHeader->DataUsed = 0;
		DBGU_TRACE("buffer Remaining = %d\n",Leading->OffsetOut.Remaining);

		if (Leading->OffsetOut.Remaining >= m_VideoInfoHeader->bmiHeader.biSizeImage)
		{
			Leading -> StreamHeader -> Duration =
				m_VideoInfoHeader -> AvgTimePerFrame;
            Leading -> StreamHeader -> PresentationTime.Numerator =
                Leading -> StreamHeader -> PresentationTime.Denominator = 1;

			Status = m_Device->FrameReadingProcess(
				m_Pin,
				Leading->StreamHeader
				);

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
			
			//if(!wssfn)
			//	break;

			DBGU_TRACE("Still Process: Start Save Image file! \n");
			
			swprintf(wssfn, L"\\DosDevices\\C:\\Still_%ws",m_Device->pdx->pVideoDevice->m_SnapShotFileName);
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
							Leading->StreamHeader->Data,
							Leading->StreamHeader->DataUsed,
							&ByteOffset,
							NULL);

				ZwClose(fh);
			}//zwCreateFile is ok
			
			if (wssfn)
				delete wssfn;
			// shawn 2011/07/27 for testing -----*/

			// shawn 2011/07/28 remove for fixing method 2 MJPEG snapshot issue
			/*if (m_Clock) {
				LONGLONG ClockTime = m_Clock -> GetTime ();
				Leading -> StreamHeader -> PresentationTime.Time = ClockTime;
				Leading -> StreamHeader -> OptionsFlags |=
					KSSTREAM_HEADER_OPTIONSF_FLUSHONPAUSE |
					KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
					KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
			} else*/ {
				//
				// If there is no clock, don't time stamp the packets.
				//
				Leading -> StreamHeader -> PresentationTime.Time = 0;
				Leading -> StreamHeader -> OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_FLUSHONPAUSE;
			}

			//
			// Advances StreamPointer the specified number of bytes into the stream 
			// and unlocks it.
			//
			KsStreamPointerAdvanceOffsetsAndUnlock(
								Leading,
								0,
								/*m_VideoInfoHeader->bmiHeader.biSizeImage*/Leading->StreamHeader->DataUsed,
								TRUE
								);
		}
		else
		{
			KeDelay(33);
			KsStreamPointerUnlock(Leading, FALSE);
			Status = STATUS_BUFFER_OVERFLOW;
		}
	}
	//
	// Turn Off the PinGate to avoid processing
	//
	PKSGATE pGate = KsPinGetAndGate(m_Pin);
	if (pGate)
	{
		KsGateTurnInputOff(pGate);
	}

	KsPinReleaseProcessingMutex(m_Pin);

	SnPrint(DEBUGLVL_VERBOSE, ("Leave CStillPin::Process (0x%X)\n",Status));
	return Status;
}

/*************************************************/


NTSTATUS
CStillPin::
CleanupReferences (
    )

/*++

Routine Description:

    Clean up any references we're holding on frames after we abruptly
    stop the hardware.

Arguments:

    None

Return Value:

    Success / Failure

--*/

{
	PAGED_CODE();
	DBGU_TRACE("Enter CStillPin::CleanupReferences\n");

	PKSSTREAM_POINTER Clone = KsPinGetFirstCloneStreamPointer (m_Pin);
	PKSSTREAM_POINTER NextClone = NULL;

	//
	// Walk through the clones, deleting them, and setting DataUsed to
	// zero since we didn't use any data!
	//
	while (Clone) {
		NextClone = KsStreamPointerGetNextClone (Clone);

		Clone -> StreamHeader -> DataUsed = 0;
		KsStreamPointerDelete (Clone);

		Clone = NextClone;
	}

	return STATUS_SUCCESS;
}

/*************************************************/


NTSTATUS
CStillPin::
SetState (
    IN KSSTATE ToState,
    IN KSSTATE FromState
    )

/*++

Routine Description:

    This is called when the caputre pin transitions state.  The routine
    attempts to acquire / release any hardware resources and start up
    or shut down capture based on the states we are transitioning to
    and away from.

Arguments:

    ToState -
        The state we're transitioning to

    FromState -
        The state we're transitioning away from

Return Value:

    Success / Failure

--*/

{
	PAGED_CODE();
	DBGU_TRACE("Enter CStillPin::SetState from %d to %d\n",FromState,ToState);

	NTSTATUS Status = STATUS_SUCCESS;
	PKSGATE pGate = KsPinGetAndGate(m_Pin);

	if (!m_Device)
	{
		DBGU_ERROR("m_Device of CStillPin is NULL!\n");
		return STATUS_UNSUCCESSFUL;
	}

	switch (ToState) {
		case KSSTATE_STOP:
			//
			// First, stop the hardware if we actually did anything to it.
			//
			if (m_StreamState != KSSTATE_RUN) {
				Status = m_Device -> Stop (m_Pin);
				//ASSERT (NT_SUCCESS (Status));

				m_StreamState = KSSTATE_STOP;
			}

			//
			// We've stopped the "fake hardware".  It has cleared out
			// it's scatter / gather tables and will no longer be 
			// completing clones.  We had locks on some frames that were,
			// however, in hardware.  This will clean them up.  An
			// alternative location would be in the reset dispatch.
			// Note, however, that the reset dispatch can occur in any
			// state and this should be understood.
			//
			// Some hardware may fill all S/G mappings before stopping...
			// in this case, you may not have to do this.  The 
			// "fake hardware" here simply stops filling mappings and 
			// cleans its scatter / gather tables out on the Stop call.
			//
			Status = CleanupReferences ();

			//
			// Release any hardware resources related to this pin.
			//
			if (m_AcquiredResources) {
				//
				// If we got an interface to the clock, we must release it.
				//
				if (m_Clock) {
					m_Clock -> Release ();
					m_Clock = NULL;
				}

				m_Device -> ReleaseHardwareResources (
					m_Pin->Id
					);

				m_AcquiredResources = FALSE;
				DBGU_TRACE("Enter CStillPin::SetState to KSSTATE_STOP => pStillPin = NULL\n");
				m_Device->pdx->pStillPin = NULL;
			}

			break;

		case KSSTATE_ACQUIRE:
			//
			// Acquire any hardware resources related to this pin.  We should
			// only acquire them here -- **NOT** at filter create time. 
			// This means we do not fail creation of a filter because of
			// limited hardware resources.
			//
			if (FromState == KSSTATE_STOP) {
				Status = m_Device -> AcquireHardwareResources (
					m_VideoInfoHeader,
					m_Pin->Id
					);

				if (NT_SUCCESS (Status)) {
					m_AcquiredResources = TRUE;
					DBGU_TRACE("Enter CStillPin::SetState to KSSTATE_ACQUIRE => pStillPin = this\n");
					m_Device->pdx->pStillPin = this;

					//
					// Attempt to get an interface to the master clock.
					// This will fail if one has not been assigned.  Since
					// one must be assigned while the pin is still in 
					// KSSTATE_STOP, this is a guranteed method of getting
					// the clock should one be assigned.
					//
					if (!NT_SUCCESS (
						KsPinGetReferenceClockInterface (
							m_Pin,
							&m_Clock
							)
						)) {

						//
						// If we could not get an interface to the clock,
						// don't use one.  
						//
						m_Clock = NULL;
					}
				} else {
					m_AcquiredResources = FALSE;
				}
			} else {
				//
				// Standard transport pins will always receive transitions in
				// +/- 1 manner.  This means we'll always see a PAUSE->ACQUIRE
				// transition before stopping the pin.  
				//
				// The below is done because on DirectX 8.0, when the pin gets
				// a message to stop, the queue is inaccessible.  The reset 
				// which comes on every stop happens after this (at which time
				// the queue is inaccessible also).  So, for compatibility with
				// DirectX 8.0, I am stopping the "fake" hardware at this
				// point and cleaning up all references we have on frames.  See
				// the comments above regarding the CleanupReferences call.
				//
				// If this sample were targeting XP only, the below code would
				// not be here.  Again, I only do this so the sample does not
				// hang when it is stopped running on a configuration such as
				// Win2K + DX8. 
				//
				if (m_StreamState != KSSTATE_STOP) {
					Status = m_Device -> Stop (m_Pin);
					//ASSERT (NT_SUCCESS (Status));

					m_StreamState = KSSTATE_STOP;
				}

				Status = CleanupReferences ();
			}

			break;

		case KSSTATE_PAUSE:
			//
			// Stop the hardware if we're coming down from run.
			//
			if (FromState == KSSTATE_RUN) {

				m_PresentationTime = 0;
				Status = m_Device -> Pause (m_Pin, TRUE);

				if (NT_SUCCESS (Status)) {
					m_StreamState = KSSTATE_PAUSE;
				}

			}

			break;

		case KSSTATE_RUN:
			//
			// Start the hardware or unpause it depending on
			// whether we're initially running or we've paused and restarted.
			//
			if (m_StreamState == KSSTATE_PAUSE) {
				Status = m_Device -> Pause (m_Pin, FALSE);
			} else {
				Status = m_Device -> Start (m_Pin);
			}

			//
			// Turn Off the PinGate to avoid processing
			//
			if (pGate)
			{
				KsGateTurnInputOff(pGate);
			}

			if (NT_SUCCESS (Status)) {
				m_StreamState = KSSTATE_RUN;
			}

			break;
	}

	DBGU_TRACE("Leave CStillPin::SetState with status = %X\n",Status);
	return Status;
}

/*************************************************/

NTSTATUS
CStillPin::
IntersectHandler (
    IN PKSFILTER Filter,
    IN PIRP Irp,
    IN PKSP_PIN PinInstance,
    IN PKSDATARANGE CallerDataRange,
    IN PKSDATARANGE DescriptorDataRange,
    IN ULONG BufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    )

/*++

Routine Description:

    This routine handles video pin intersection queries by determining the
    intersection between two data ranges.

Arguments:

    Filter -
        Contains a void pointer to the  filter structure.

    Irp -
        Contains a pointer to the data intersection property request.

    PinInstance -
        Contains a pointer to a structure indicating the pin in question.

    CallerDataRange -
        Contains a pointer to one of the data ranges supplied by the client
        in the data intersection request.  The format type, subtype and
        specifier are compatible with the DescriptorDataRange.

    DescriptorDataRange -
        Contains a pointer to one of the data ranges from the pin descriptor
        for the pin in question.  The format type, subtype and specifier are
        compatible with the CallerDataRange.

    BufferSize -
        Contains the size in bytes of the buffer pointed to by the Data
        argument.  For size queries, this value will be zero.

    Data -
        Optionally contains a pointer to the buffer to contain the data 
        format structure representing the best format in the intersection 
        of the two data ranges.  For size queries, this pointer will be 
        NULL.

    DataSize -
        Contains a pointer to the location at which to deposit the size 
        of the data format.  This information is supplied by the function 
        when the format is actually delivered and in response to size 
        queries.

Return Value:

    STATUS_SUCCESS if there is an intersection and it fits in the supplied
    buffer, STATUS_BUFFER_OVERFLOW for successful size queries, 
    STATUS_NO_MATCH if the intersection is empty, or 
    STATUS_BUFFER_TOO_SMALL if the supplied buffer is too small.

--*/

{
	PAGED_CODE();
	DBGU_TRACE("Enter CStillPin::IntersectHandler\n");

	const GUID VideoInfoSpecifier = 
		{STATICGUIDOF(KSDATAFORMAT_SPECIFIER_VIDEOINFO)};

	ASSERT(Filter);
	ASSERT(Irp);
	ASSERT(PinInstance);
	ASSERT(CallerDataRange);
	ASSERT(DescriptorDataRange);
	ASSERT(DataSize);

	ULONG DataFormatSize;

	//
	// Specifier FORMAT_VideoInfo for VIDEOINFOHEADER2
	//
	if (IsEqualGUID(CallerDataRange->Specifier, VideoInfoSpecifier) &&
		CallerDataRange -> FormatSize >= sizeof (KS_DATARANGE_VIDEO)) {

		PKS_DATARANGE_VIDEO callerDataRange = 
			reinterpret_cast <PKS_DATARANGE_VIDEO> (CallerDataRange);

		PKS_DATARANGE_VIDEO descriptorDataRange = 
			reinterpret_cast <PKS_DATARANGE_VIDEO> (DescriptorDataRange);

		PKS_DATAFORMAT_VIDEOINFOHEADER FormatVideoInfoHeader;

		//
		// Check that the other fields match
		//
		if ((callerDataRange->bFixedSizeSamples != 
				descriptorDataRange->bFixedSizeSamples) ||
			(callerDataRange->bTemporalCompression != 
				descriptorDataRange->bTemporalCompression) ||
			(callerDataRange->StreamDescriptionFlags != 
				descriptorDataRange->StreamDescriptionFlags) ||
			(callerDataRange->MemoryAllocationFlags != 
				descriptorDataRange->MemoryAllocationFlags) ||
			(RtlCompareMemory (&callerDataRange->ConfigCaps,
					&descriptorDataRange->ConfigCaps,
					sizeof (KS_VIDEO_STREAM_CONFIG_CAPS)) != 
					sizeof (KS_VIDEO_STREAM_CONFIG_CAPS))) 
		{
			DBGU_TRACE("Check that the other fields match, return STATUS_NO_MATCH\n");
			return STATUS_NO_MATCH;
		}

		//
		// KS_SIZE_VIDEOHEADER() below is relying on bmiHeader.biSize from
		// the caller's data range.  This **MUST** be validated; the
		// extended bmiHeader size (biSize) must not extend past the end
		// of the range buffer.  Possible arithmetic overflow is also
		// checked for.
		//
		{
			ULONG VideoHeaderSize = KS_SIZE_VIDEOHEADER (
				&callerDataRange->VideoInfoHeader
				);

			ULONG DataRangeSize = 
				FIELD_OFFSET (KS_DATARANGE_VIDEO, VideoInfoHeader) +
				VideoHeaderSize;

			DBGU_TRACE("X=%d, Y=%d, BI=%d\n",callerDataRange->VideoInfoHeader.bmiHeader.biWidth,
				callerDataRange->VideoInfoHeader.bmiHeader.biHeight,
				callerDataRange->VideoInfoHeader.bmiHeader.biBitCount);
			//
			// Check that biSize does not extend past the buffer.  The 
			// first two checks are for arithmetic overflow on the 
			// operations to compute the alleged size.  (On unsigned
			// math, a+b < a iff an arithmetic overflow occurred).
			//
			if (VideoHeaderSize < callerDataRange->VideoInfoHeader.bmiHeader.biSize ||
				DataRangeSize < VideoHeaderSize ||
				DataRangeSize > callerDataRange -> DataRange.FormatSize
			) {
				DBGU_TRACE("Check that biSize, return STATUS_INVALID_PARAMETER\n");
				return STATUS_INVALID_PARAMETER;
			}
		}

		DataFormatSize = 
			sizeof (KSDATAFORMAT) + 
			KS_SIZE_VIDEOHEADER (&callerDataRange->VideoInfoHeader);

		//
		// If the passed buffer size is 0, it indicates that this is a size
		// only query.  Return the size of the intersecting data format and
		// pass back STATUS_BUFFER_OVERFLOW.
		//
		if (BufferSize == 0) {
			*DataSize = DataFormatSize;
			DBGU_TRACE("the passed buffer size is 0, return STATUS_BUFFER_OVERFLOW\n");
			return STATUS_BUFFER_OVERFLOW;
		}

		//
		// Verify that the provided structure is large enough to
		// accept the result.
		//
		if (BufferSize < DataFormatSize) 
		{
			DBGU_TRACE("Verify buffer large enough, return STATUS_BUFFER_TOO_SMALL\n");
			return STATUS_BUFFER_TOO_SMALL;
		}

		//
		// Copy over the KSDATAFORMAT, followed by the actual VideoInfoHeader
		//
		*DataSize = DataFormatSize;

		FormatVideoInfoHeader = PKS_DATAFORMAT_VIDEOINFOHEADER( Data );

		//
		// Copy over the KSDATAFORMAT.  This is precisely the same as the
		// KSDATARANGE (it's just the GUIDs, etc...  not the format information
		// following any data format.
		// 
		RtlCopyMemory (
			&FormatVideoInfoHeader->DataFormat, 
			DescriptorDataRange, 
			sizeof (KSDATAFORMAT));

		FormatVideoInfoHeader->DataFormat.FormatSize = DataFormatSize;

		//
		// Copy over the callers requested VIDEOINFOHEADER
		//

		RtlCopyMemory (
			&FormatVideoInfoHeader->VideoInfoHeader,
			&callerDataRange->VideoInfoHeader,
			KS_SIZE_VIDEOHEADER (&callerDataRange->VideoInfoHeader) 
			);

		//
		// Calculate biSizeImage for this request, and put the result in both
		// the biSizeImage field of the bmiHeader AND in the SampleSize field
		// of the DataFormat.
		//
		// Note that for compressed sizes, this calculation will probably not
		// be just width * height * bitdepth
		//
		FormatVideoInfoHeader->VideoInfoHeader.bmiHeader.biSizeImage =
			FormatVideoInfoHeader->DataFormat.SampleSize = 
			KS_DIBSIZE (FormatVideoInfoHeader->VideoInfoHeader.bmiHeader);

		//
		// REVIEW - Perform other validation such as cropping and scaling checks
		// 
		DBGU_TRACE("REVIEW - Perform other validation such as cropping and scaling checks\n");
		return STATUS_SUCCESS;

	} // End of VIDEOINFOHEADER specifier
	
	DBGU_TRACE("End of VIDEOINFOHEADER specifier, return STATUS_NO_MATCH\n");
	return STATUS_NO_MATCH;
}


/*************************************************/


NTSTATUS CStillPin::DispatchSetFormat
(
    IN PKSPIN Pin,
    IN PKSDATAFORMAT OldFormat OPTIONAL,
    IN PKSMULTIPLE_ITEM OldAttributeList OPTIONAL,
    IN const KSDATARANGE *DataRange,
    IN const KSATTRIBUTE_LIST *AttributeRange OPTIONAL
)

/*++

Routine Description:

    This is the set data format dispatch for the capture pin.  It is called
    in two circumstances.

        1: before Pin's creation dispatch has been made to verify that
           Pin -> ConnectionFormat is an acceptable format for the range
           DataRange.  In this case OldFormat is NULL.

        2: after Pin's creation dispatch has been made and an initial format
           selected in order to change the format for the pin.  In this case,
           OldFormat will not be NULL.

    Validate that the format is acceptible and perform the actions necessary
    to change format if appropriate.

Arguments:

    Pin -
        The pin this format is being set on.  The format itself will be in
        Pin -> ConnectionFormat.

    OldFormat -
        The previous format used on this pin.  If this is NULL, it is an
        indication that Pin's creation dispatch has not yet been made and
        that this is a request to validate the initial format and not to
        change formats.

    OldAttributeList -
        The old attribute list for the prior format

    DataRange -
        A range out of our list of data ranges which was determined to be
        at least a partial match for Pin -> ConnectionFormat.  If the format
        there is unacceptable for the range, STATUS_NO_MATCH should be
        returned.

    AttributeRange -
        The attribute range

Return Value:

    Success / Failure

        STATUS_SUCCESS -
            The format is acceptable / the format has been changed

        STATUS_NO_MATCH -
            The format is not-acceptable / the format has not been changed

--*/

{
	PAGED_CODE();
	DBGU_TRACE("Enter CStillPin::DispatchSetFormat\n");

	NTSTATUS Status = STATUS_NO_MATCH;

	const GUID VideoInfoSpecifier = 
		{STATICGUIDOF(KSDATAFORMAT_SPECIFIER_VIDEOINFO)};

	CStillPin *StiPin = NULL;

	//
	// Find the pin, if it exists yet.  OldFormat will be an indication of 
	// this.  If we're changing formats, OldFormat will be non-NULL.
	//
	// You cannot use Pin -> Context to make the determination.  AVStream
	// preinitializes this to the filter's context.
	//
	if (OldFormat) {
		StiPin = reinterpret_cast <CStillPin *> (Pin -> Context);
	}

	if (IsEqualGUID (Pin -> ConnectionFormat -> Specifier, VideoInfoSpecifier) &&
		Pin -> ConnectionFormat -> FormatSize >= sizeof (KS_DATAFORMAT_VIDEOINFOHEADER)) {
		PKS_DATAFORMAT_VIDEOINFOHEADER ConnectionFormat =
			reinterpret_cast <PKS_DATAFORMAT_VIDEOINFOHEADER> (Pin -> ConnectionFormat);

		//
		// DataRange comes out of OUR data range list.  I know the range
		// is valid as such.
		//
		const KS_DATARANGE_VIDEO *VIRange =
			reinterpret_cast <const KS_DATARANGE_VIDEO *> (DataRange);

		//
		// Check that bmiHeader.biSize is valid since we use it later.
		//
		ULONG VideoHeaderSize = KS_SIZE_VIDEOHEADER (
			&ConnectionFormat -> VideoInfoHeader
			);

		ULONG DataFormatSize = FIELD_OFFSET (
			KS_DATAFORMAT_VIDEOINFOHEADER, VideoInfoHeader
			) + VideoHeaderSize;

		if (
			VideoHeaderSize < ConnectionFormat->VideoInfoHeader.bmiHeader.biSize ||
			DataFormatSize < VideoHeaderSize ||
			DataFormatSize > ConnectionFormat -> DataFormat.FormatSize
			) {
			Status = STATUS_INVALID_PARAMETER;
		}
		//
		// Check that the format is a match for the selected range. 
		//
		else if (
			(ConnectionFormat -> VideoInfoHeader.bmiHeader.biWidth !=
				VIRange -> VideoInfoHeader.bmiHeader.biWidth) ||

			(ConnectionFormat -> VideoInfoHeader.bmiHeader.biHeight !=
				VIRange -> VideoInfoHeader.bmiHeader.biHeight) ||

			(ConnectionFormat -> VideoInfoHeader.bmiHeader.biCompression !=
				VIRange -> VideoInfoHeader.bmiHeader.biCompression) 

			) {
			Status = STATUS_NO_MATCH;
		} else {
			//
			// Compute the minimum size of our buffers to validate against.
			// The image synthesis routines synthesize |biHeight| rows of
			// biWidth pixels in either RGB24 or UYVY.  In order to ensure
			// safe synthesis into the buffer, we need to know how large an
			// image this will produce.
			//
			// I do this explicitly because of the method that the data is
			// synthesized.  A variation of this may or may not be necessary
			// depending on the mechanism the driver in question fills the 
			// capture buffers.  The important thing is to ensure that they
			// aren't overrun during capture.
			//
			ULONG ImageSize;

			if (!MultiplyCheckOverflow (
				(ULONG)ConnectionFormat->VideoInfoHeader.bmiHeader.biWidth,
				(ULONG)abs (ConnectionFormat->
					VideoInfoHeader.bmiHeader.biHeight),
				&ImageSize
				)) {
				Status = STATUS_INVALID_PARAMETER;
			}
			//
			// We only support KS_BI_RGB (24) and KS_BI_YUV422 (16), so
			// this is valid for those formats.
			//
			else if (!MultiplyCheckOverflow (
				ImageSize,
				(ULONG)(ConnectionFormat->
					VideoInfoHeader.bmiHeader.biBitCount / 8),
				&ImageSize
				)) {
				Status = STATUS_INVALID_PARAMETER;
			}
			//
			// Valid for the formats we use.  Otherwise, this would be
			// checked later.
			//
			else if (ConnectionFormat->VideoInfoHeader.bmiHeader.biSizeImage <
					ImageSize) {
				Status = STATUS_INVALID_PARAMETER;
			} else {
				//
				// We can accept the format. 
				//
				Status = STATUS_SUCCESS;

				//
				// OldFormat is an indication that this is a format change.  
				// Since I do not implement the 
				// KSPROPERTY_CONNECTION_PROPOSEDATAFORMAT, by default, I do 
				// not handle dynamic format changes.
				//
				// If something changes while we're in the stop state, we're 
				// fine to handle it since we haven't "configured the hardware"
				// yet.
				//
				if (OldFormat) {
					//
					// If we're in the stop state, we can handle just about any
					// change.  We don't support dynamic format changes. 
					//
					if (Pin -> DeviceState == KSSTATE_STOP) {
						if (!StiPin -> CaptureVideoInfoHeader ()) {
							Status = STATUS_INSUFFICIENT_RESOURCES;
						}
					} else {
						//
						// Because we don't accept dynamic format changes, we
						// should never get here.  Just being over-protective.
						//
						Status = STATUS_INVALID_DEVICE_STATE;
					}
				}
			}
		}
	}

	return Status;

}

/**************************************************************************

    LOCKED CODE

**************************************************************************/

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif // ALLOC_PRAGMA
