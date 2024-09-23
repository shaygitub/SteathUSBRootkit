#include "Ioctl.h"
#define EXAMPLE_IOCTL 0x40008000
#pragma warning(disable : 6305)


NTSTATUS IoctlCallbacks::CreateCloseCallback(PDEVICE_OBJECT DeviceObject,
    PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    PAGED_CODE();
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS IoctlCallbacks::DeviceControlCallback(PDEVICE_OBJECT DeviceObject,
    PIRP Irp) {
    PIO_STACK_LOCATION ParamStackLocation = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG InputBufferSize = 0;
    ULONG OutputBufferSize = 0;
    PUCHAR InputBuffer = NULL;
    PUCHAR OutputBuffer = NULL;
    UNREFERENCED_PARAMETER(DeviceObject);
    PAGED_CODE();
    ParamStackLocation = IoGetCurrentIrpStackLocation(Irp);
    InputBufferSize = ParamStackLocation->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferSize = ParamStackLocation->Parameters.DeviceIoControl.OutputBufferLength;


    // Determine which I/O control code was specified:
    switch (ParamStackLocation->Parameters.DeviceIoControl.IoControlCode) {
    case EXAMPLE_IOCTL:

        // Verify correct parameters:
        InputBufferSize = ParamStackLocation->Parameters.DeviceIoControl.InputBufferLength;
        OutputBufferSize = ParamStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
        InputBuffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
        OutputBuffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
        if (InputBufferSize != 16 || OutputBufferSize != 16 || InputBuffer == NULL) {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }
        DbgPrintEx(0, 0, "BootStart IOCTL - IOCTL 0x%X executed\n", EXAMPLE_IOCTL);
        break;

    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}