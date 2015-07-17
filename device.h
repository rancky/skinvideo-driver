/**************************************************************************

	Sonix AVStream For UVC

	Copyright (c) 2007, Sonix Corporation.

    File:

        device.h

    Abstract:

        The header for the device level of the hardware.
        
    History:

		created 2007/01/26 [Saxen Ko]

**************************************************************************/

#ifndef _UVCAV_DEVICE_H_
#define _UVCAV_DEVICE_H_


class CCaptureDevice
{

private:

	//
	// The AVStream device we're associated with.
	//
	PKSDEVICE m_Device;

	//
	// Number of pins with resources acquired.  This is used as a locking
	// mechanism for resource acquisition on the device.
	//
	LONG m_PinsWithResources[STREAM_Counts];
	LONG m_PinsConnects[STREAM_Counts];

	//
	// The video info header we're basing hardware settings on.  The pin
	// provides this to us when acquiring resources and must guarantee its
	// stability until resources are released.
	//
	PKS_VIDEOINFOHEADER m_VideoInfoHeader[STREAM_Counts];

	//
	// Cleanup():
	//
	// This is the free callback for the bagged capture device.  Not providing
	// one will call ExFreePool, which is not what we want for a constructed
	// C++ object.  This simply deletes the capture device.
	//
	static void	
	Cleanup (
		IN CCaptureDevice *pCapDevice
		)
	{
		if (pCapDevice)
			delete pCapDevice; pCapDevice = NULL;
	}

	//
	// PnpStart():
	//
	// This is the Pnp start routine for our hardware.  Note that
	// DispatchStart bridges to here in the context of the CCaptureDevice.
	//
	NTSTATUS PnpStart (
		IN PCM_RESOURCE_LIST TranslatedResourceList,
		IN PCM_RESOURCE_LIST UntranslatedResourceList
		);

	//
	NTSTATUS PostStart();

	NTSTATUS QueryCapabilities(
		IN OUT PDEVICE_CAPABILITIES  pCapabilities
	);

	//
	// SetPowerState():
	//
	// AVStream calls SetPowerState routine when it receives an
	// IRP_MN_SET_POWER.
	//
	void SetPowerState (
		IN DEVICE_POWER_STATE  To,
		IN DEVICE_POWER_STATE  From
		);
	//
	// PnpStop():
	//
	// This is the Pnp stop routine for our hardware.  Note that
	// DispatchStop bridges to here in the context of the CCaptureDevice.
	//
	void PnpStop();

	//
	// SurpriseRemoval():
	//
	// This is the Pnp SurpriseRemoval routine for our hardware.  Note that
	// SurpriseRemoval bridges to here in the context of the CCaptureDevice.
	//
	void SurpriseRemoval();

	//
	// RemoveDevice():
	//
	// Pnp Remove device
	//
	void RemoveDevice();
	
public:

	DEVEXT pdx[1];

    //
    // CCaptureDevice():
    //
    // The capture device class constructor.  Since everything should have
    // been zero'ed by the new operator, don't bother setting anything to
    // zero or NULL.  Only initialize non-NULL, non-0 fields.
    //
	CCaptureDevice (IN PKSDEVICE Device);

    //
    // ~CCaptureDevice():
    //
    // The capture device destructor.
    //
    ~CCaptureDevice ();

    //
    // DispatchCreate():
    //
    // This is the Add Device dispatch for the capture device.  It creates
    // the CCaptureDevice and associates it with the device via the bag.
    //
    static NTSTATUS 
	DispatchCreate (
        IN PKSDEVICE Device
        );

    //
    // DispatchRemove():
    //
    // This is the Remove Device dispatch for the capture device.
    //
	static VOID
	DispatchRemove(
		IN PKSDEVICE  Device,
		IN PIRP  Irp
    )
	{
        (reinterpret_cast <CCaptureDevice *> (Device -> Context)) -> RemoveDevice ();
	}

    //
    // DispatchPnpStart():
    //
    // This is the Pnp Start dispatch for the capture device.
    //
    static NTSTATUS
    DispatchPnpStart (
        IN PKSDEVICE Device,
        IN PIRP Irp,
        IN PCM_RESOURCE_LIST TranslatedResourceList,
        IN PCM_RESOURCE_LIST UntranslatedResourceList
        )
    {
        return 
            (reinterpret_cast <CCaptureDevice *> (Device -> Context)) ->
            PnpStart (
                TranslatedResourceList,
                UntranslatedResourceList
                );
    }

	//
	// DispatchPostStart():
	//
    static NTSTATUS
	DispatchPostStart(
		IN PKSDEVICE  Device
		)
    {
        return 
            (reinterpret_cast <CCaptureDevice *> (Device -> Context)) -> PostStart ();
    }

    //
    // DispatchPnpStart():
    //
    // This is the Pnp Start dispatch for the capture device.
    //
    static void
    DispatchSetPower (
		IN PKSDEVICE  Device,
		IN PIRP  Irp,
		IN DEVICE_POWER_STATE  To,
		IN DEVICE_POWER_STATE  From
        )
    {
        (reinterpret_cast <CCaptureDevice *> (Device -> Context)) ->
        SetPowerState (
            To,
            From
            );
    }
	//
    // DispatchPnpStop():
    //
    // This is the Pnp stop dispatch for the capture device.
    //
    static void
    DispatchPnpStop (
        IN PKSDEVICE Device,
        IN PIRP Irp
        )
    {
        (reinterpret_cast <CCaptureDevice *> (Device -> Context)) -> PnpStop ();
    }

    //
    // DispatchPnpSurpriseRemoval():
    //
    // This is the Pnp SurpriseRemoval dispatch for the capture device.
    //
    static void
    DispatchPnpSurpriseRemoval (
        IN PKSDEVICE Device,
        IN PIRP Irp
        )
    {
        (reinterpret_cast <CCaptureDevice *> (Device -> Context)) -> SurpriseRemoval ();
    }

    //
    // DispatchQueryCapabilities():
    //
	static 
	NTSTATUS DispatchQueryCapabilities(
		IN PKSDEVICE  Device,
		IN PIRP  Irp,
		IN OUT PDEVICE_CAPABILITIES  Capabilities
		)
	{
        return (reinterpret_cast <CCaptureDevice *> (Device -> Context)) -> QueryCapabilities (Capabilities);
	}



	//
    // AcquireHardwareResources():
    //
    // Called to acquire hardware resources for the device based on a given
    // video info header.  This will fail if another object has already
    // acquired hardware resources since we emulate a single capture
    // device.
    //
    NTSTATUS
    AcquireHardwareResources (
        IN PKS_VIDEOINFOHEADER VideoInfoHeader,
		IN ULONG StreamNumber
        );

    //
    // ReleaseHardwareResources():
    //
    // Called to release hardware resources for the device.
    //
    void ReleaseHardwareResources (
		IN ULONG StreamNumber
		);

    //
    // Start():
    //
    // Called to start the hardware.
    //
    NTSTATUS
    Start (
		IN PKSPIN pin
        );

    //
    // Pause():
    //
    // Called to pause or unpause the hardware.
    //
    NTSTATUS
    Pause (
		IN PKSPIN pin,
        IN BOOLEAN Pausing
        );

    //
    // Stop():
    //
    // Called to stop the hardware.
    //
    NTSTATUS
    Stop (
		IN PKSPIN pin
        );

	NTSTATUS 
	FrameReadingProcess(
		IN PKSPIN pin,
		IN PKSSTREAM_HEADER pDataPacket
		);

	NTSTATUS StreamOpen(BOOL fOpen, IN PKSPIN pin);
};

#endif //_UVCAV_DEVICE_H_
