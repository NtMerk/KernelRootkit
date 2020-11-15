#include "ntddk.h"

/*      Name for the symbolic link.
        DosDevices refers to the serial port COM1,
        so we can now reference \\\\.\\MyDevice         */
const WCHAR deviceLinkBuffer[] = L"\\DosDevices\\MyDevice";

// Name for the device driver
const WCHAR deviceNameBuffer[] = L"\\Device\\MyDevice";

// Global pointer to our device object
PDEVICE_OBJECT g_RootkitDevice;

// Make Major Functions do nothing
NTSTATUS OnStubDispatch(IN PDEVICE_OBJECT pDriverObject, IN PIRP irp)
{
    irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// Callback function to unload the device driver
VOID OnUnload(IN PDRIVER_OBJECT pDriverObject)
{
    DbgPrint("OnUnload called!");
}

// Since we're using C++, it mangles the name to account for overloading. extern "C" tells the compiler to stop mangling
extern "C"
{
    // "Entry point" of our kernel device driver
    NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING registryPath)
    {
        DbgPrint("Driver loaded!");

        NTSTATUS ntStatus;
        UNICODE_STRING deviceLinkUnicodeString;
        UNICODE_STRING deviceNameUnicodeString;

        // Set up name and symbolic link
        RtlInitUnicodeString(&deviceLinkUnicodeString, deviceLinkBuffer);
        RtlInitUnicodeString(&deviceNameUnicodeString, deviceNameBuffer);

        // Set up the device
        ntStatus = IoCreateDevice(pDriverObject, 0, &deviceNameUnicodeString, 0x00001234, 0, TRUE, &g_RootkitDevice);
        DbgPrint("Device created!");

        if ( NT_SUCCESS(ntStatus) )
        {
            // Create Symbolic Link
            ntStatus = IoCreateSymbolicLink(&deviceLinkUnicodeString, &deviceNameUnicodeString);
        }

        // Call the unload function
        pDriverObject->DriverUnload = OnUnload;

        for (int c = 0; c < IRP_MJ_MAXIMUM_FUNCTION; c++)
        {
            // Set all Major Functions to point to OnStubDispatch
            pDriverObject->MajorFunction[c] = OnStubDispatch;
        }

        return STATUS_SUCCESS;
    }
}

