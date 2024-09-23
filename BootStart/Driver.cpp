#include "Driver.h"
#include "Ioctl.h"


// Global variables:
PDEVICE_OBJECT DeviceObject = NULL;
UNICODE_STRING SymbolicLink = RTL_CONSTANT_STRING(L"\\DosDevices\\BootStart");
UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\BootStart");


VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	IoDeleteSymbolicLink(&SymbolicLink);
	if (DeviceObject != NULL) {
		IoDeleteDevice(DeviceObject);
		DeviceObject = NULL;
	}
	DbgPrintEx(0, 0, "BootStart general - DriverUnload() called\n");
}


NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT  DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	NTSTATUS Status = STATUS_SUCCESS;
	const char* HelloMessage =
		"\n----------\n"
		" _____ _               _       _  ______ _ _ _            \n"
		"/  ___| |             (_)     (_) |  ___(_) | |           \n"
		"\\ `--.| |__  _ __ ___  _ _ __  _  | |_   _| | |_ ___ _ __ \n"
		" `--. \\ '_ \\| '_ ` _ \\| | '_ \\| | |  _| | | | __/ _ \\ '__|\n"
		"/\\__/ / | | | | | | | | | | | | | | |   | | | ||  __/ |   \n"
		"\\____/|_| |_|_| |_| |_|_|_| |_|_| \\_|   |_|_|\\__\\___|_|   \n\n"
		"Discord: bldysis#0868  GitHub: shaygitub\n"
		"\n----------\n";
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrintEx(0, 0, "%s", HelloMessage);


	// Register IOCTL callbacks:
	Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	if (!NT_SUCCESS(Status)) {
		DbgPrintEx(0, 0, "BootStart - IoCreateDevice() failed with status 0x%x\n", Status);
		return Status;
	}
	Status = IoCreateSymbolicLink(&SymbolicLink, &DeviceName);
	if (!NT_SUCCESS(Status)) {
		DbgPrintEx(0, 0, "BootStart - IoCreateSymbolicLink() failed with status 0x%x\n", Status);
		IoDeleteDevice(DeviceObject);
		return Status;
	}
	DriverObject->MajorFunction[IRP_MJ_CREATE] = IoctlCallbacks::CreateCloseCallback;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = IoctlCallbacks::CreateCloseCallback;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctlCallbacks::DeviceControlCallback;
	DriverObject->DriverUnload = DriverUnload;
	DbgPrintEx(0, 0, "BootStart - initialized boot process\n");
	return Status;
}