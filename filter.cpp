/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

    File:

        filter.cpp

    Abstract:

        This file contains the filter level implementation for the 
        capture filter.

    History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/

#include "SnCam.h"
//#include "StreamData.h"


/**************************************************************************

    PAGEABLE CODE

**************************************************************************/

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif // ALLOC_PRAGMA


NTSTATUS
CCaptureFilter::
DispatchCreate (
	IN PKSFILTER Filter,
	IN PIRP Irp
    )

/*++

Routine Description:

    This is the creation dispatch for the capture filter.  It creates
    the CCaptureFilter object, associates it with the AVStream filter
    object, and bag the CCaptureFilter for later cleanup.

Arguments:

    Filter -
        The AVStream filter being created

    Irp -
        The creation Irp

Return Value:
    
    Success / failure

--*/

{
	PAGED_CODE();

	NTSTATUS Status = STATUS_SUCCESS;

	DBGU_TRACE("Enter CCaptureFilter::DispatchCreate\n");
	CCaptureFilter *CapFilter = new (NonPagedPool) CCaptureFilter (Filter);

	if (!CapFilter) {
		//
		// Return failure if we couldn't create the filter.
		//
		Status = STATUS_INSUFFICIENT_RESOURCES;
	} else {
		//
		// Add the item to the object bag if we we were successful. 
		// Whenever the filter closes, the bag is cleaned up and we will be
		// freed.
		//
		Status = KsAddItemToObjectBag (
			Filter -> Bag,
			reinterpret_cast <PVOID> (CapFilter),
			reinterpret_cast <PFNKSFREE> (CCaptureFilter::Cleanup)
			);

		if (!NT_SUCCESS (Status)) {
			delete CapFilter;
		} else {
			Filter -> Context = reinterpret_cast <PVOID> (CapFilter);

			CCaptureDevice *pCaptureDevice = reinterpret_cast<CCaptureDevice *>(CapFilter->m_Device->Context);
			
			if (!pCaptureDevice)
				return STATUS_UNSUCCESSFUL;
			
			InterlockedIncrement(&pCaptureDevice->pdx->m_cRef);
			
			//2010/8/26 11:05上午
			//if (pCaptureDevice->pdx->m_bDeviceIdle)
			if (pCaptureDevice->pdx->m_cRef > 0&& pCaptureDevice->pdx->m_bDeviceIdle && pCaptureDevice->pdx->DevicePowerState != PowerDeviceD0)
			{
				pCaptureDevice->pdx->pVideoDevice->CancelSelectiveSuspend();
				pCaptureDevice->pdx->pVideoDevice->SendDeviceSetPower(PowerDeviceD0, FALSE);
				pCaptureDevice->pdx->m_bDeviceIdle = FALSE;
			}
		}
	}
	
	DBGU_TRACE("Leave CCaptureFilter::DispatchCreate with %d\n", Status);

	return Status;
}

//
// FilterClose() method of the CFilter class
//
//    Deletes the previously created filter object.
//
STDMETHODIMP_(NTSTATUS)
CCaptureFilter::DispatchClose(
    IN OUT PKSFILTER Filter,
    IN PIRP Irp
    )
{
	CCaptureFilter *pFilter = reinterpret_cast <CCaptureFilter *> (Filter->Context);
	ASSERT(pFilter);

	CCaptureDevice *pCaptureDevice = reinterpret_cast<CCaptureDevice *>(pFilter->m_Device->Context);
	ASSERT(pCaptureDevice);

	InterlockedDecrement(&pCaptureDevice->pdx->m_cRef);
	
	//2010/5/3 01:49下午
	//2010/3/9 11:40上午
	if((pCaptureDevice->pdx->DevicePowerState == PowerDeviceD0)&&(pCaptureDevice->pdx->m_cRef == 0)
	    &&(!pCaptureDevice->pdx->m_bDeviceIdle)&&(pCaptureDevice->pdx->m_C3Enable))
	{
		NTSTATUS ntStatus = STATUS_SUCCESS;
		//2010/8/30 04:40下午
		//ntStatus = pCaptureDevice->pdx->pVideoDevice->SubmitIdleRequestIrp();
		//if(NT_SUCCESS(ntStatus))
			pCaptureDevice->pdx->m_bDeviceIdle = TRUE;
		//else
		//	DBGU_WARNING("SubmitIdleRequestIrp return %X\n", ntStatus);
	}	

	DBGU_TRACE("Enter CCaptureFilter::DispatchClose (%d)\n", pCaptureDevice->pdx->m_cRef);

	return STATUS_SUCCESS;
}

//**************************************************************************************************
NTSTATUS CCaptureFilter::SetProperty
(
	IN     PIRP        irp,
	IN     PKSPROPERTY property,
	IN OUT PVOID       data
)
{
	PAGED_CODE ();

	NTSTATUS    ntStatus = STATUS_UNSUCCESSFUL;

	ASSERT(irp);
	ASSERT(property);
	DBGU_TRACE("Enter CCaptureFilter::SetProperty\n");

	//
	// Get hold of our FilterContext via pIrp
	//
	CCaptureFilter *pCaptureFilter = reinterpret_cast<CCaptureFilter *>(KsGetFilterFromIrp(irp)->Context);

	//
	// Assert that we have a valid filter context
	//
	ASSERT (pCaptureFilter);

	CCaptureDevice *pCaptureDevice = reinterpret_cast<CCaptureDevice *>(pCaptureFilter->m_Device->Context);
	ASSERT(pCaptureDevice);

	if (KSPROPERTY_TYPE_SET & property->Flags)
		ntStatus = pCaptureDevice->pdx->pVideoStream->SetDeviceProperty(irp, property, data);

	DBGU_TRACE("Enter CCaptureFilter::SetProperty return =%X\n",ntStatus);
	return ntStatus;
}


NTSTATUS CCaptureFilter::GetProperty
(
	IN     PIRP        irp,
	IN     PKSPROPERTY property,
	IN OUT PVOID       data
)
{
	PAGED_CODE ();

	NTSTATUS    ntStatus = STATUS_UNSUCCESSFUL;

	ASSERT(irp);
	ASSERT(property);
	DBGU_TRACE("Enter CCaptureFilter::GetProperty\n");

	//
	// Get hold of our FilterContext via pIrp
	//
	CCaptureFilter *pCaptureFilter = reinterpret_cast<CCaptureFilter *>(KsGetFilterFromIrp(irp)->Context);

	//
	// Assert that we have a valid filter context
	//
	ASSERT (pCaptureFilter);

	CCaptureDevice *pCaptureDevice = reinterpret_cast<CCaptureDevice *>(pCaptureFilter->m_Device->Context);
	ASSERT(pCaptureDevice);

	if (KSPROPERTY_TYPE_GET & property->Flags)
		ntStatus = pCaptureDevice->pdx->pVideoStream->GetDeviceProperty(irp, property, data);

	DBGU_TRACE("Enter CCaptureFilter::GetProperty return =%X\n",ntStatus);
	return ntStatus;
}
//**************************************************************************************************
