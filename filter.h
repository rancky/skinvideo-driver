/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

    File:

        filter.h

    Abstract:

        This file contains the filter level header for the capture filter.

    History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/

#ifndef _UVCAV_FILTER_H_
#define _UVCAV_FILTER_H_

class CCaptureFilter {

private:

	//
	// Cleanup():
	//
	// This is the bag cleanup callback for the CCaptureFilter.  Not providing
	// one would cause ExFreePool to be used.  This is not good for C++
	// constructed objects.  We simply delete the object here.
	//
	static void 
	Cleanup (
		IN CCaptureFilter *CapFilter
		)
	{
		if (CapFilter)
			delete CapFilter; CapFilter = NULL;
	}

public:

	//
	// CCaptureFilter():
	//
	// The capture filter object constructor.  Since the new operator will
	// have zeroed the memory, do not bother initializing any NULL or 0
	// fields.  Only initialize non-NULL, non-0 fields.
	//
	CCaptureFilter (
		IN PKSFILTER Filter
		) : m_Filter (Filter)
	{
		m_Device = KsGetDevice(Filter);
	}

	//
	// ~CCaptureFilter():
	//
	// The capture filter destructor.
	//
	~CCaptureFilter (
		)
	{
	}

	//
	// DispatchCreate():
	//
	// This is the filter creation dispatch for the capture filter.  It
	// creates the CCaptureFilter object, associates it with the AVStream
	// object, and bags it for easy cleanup later.
	//
	static NTSTATUS 
	DispatchCreate (
		IN PKSFILTER Filter,
		IN PIRP Irp
		);

	//
	// DispatchClose():
	//
	// This is the filter clsoing dispatch for the capture filter.  It
	// creates the CCaptureFilter object, associates it with the AVStream
	// object, and bags it for easy cleanup later.
	//
	static NTSTATUS 
	DispatchClose (
		IN OUT PKSFILTER Filter,
		IN PIRP Irp
		);

	//
	//	Handle Device Property Set
	//
	static STDMETHODIMP_(NTSTATUS) 
	SetProperty(
		IN     PIRP        irp,
		IN     PKSPROPERTY property,
		IN OUT PVOID       data
	);

	//
	//	Handle Device Property Get
	//
	static STDMETHODIMP_(NTSTATUS) 
	GetProperty(
		IN     PIRP        irp,
		IN     PKSPROPERTY property,
		IN OUT PVOID       data
	);

private:

	//
	// The AVStream filter object associated with this CCaptureFilter.
	//
	PKSFILTER m_Filter;
	//
	// The AVStream device we're associated with.
	//
	PKSDEVICE m_Device;
};


#endif //_UVCAV_FILTER_H_
