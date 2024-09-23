#pragma once
#include <ntddk.h>
#include <wdm.h>


namespace IoctlCallbacks {
    NTSTATUS CreateCloseCallback(PDEVICE_OBJECT DeviceObject, PIRP Irp);
    NTSTATUS DeviceControlCallback(PDEVICE_OBJECT DeviceObject, PIRP Irp);
}