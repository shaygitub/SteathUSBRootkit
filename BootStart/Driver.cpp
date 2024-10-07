#include "mitigations.h"
#define EXAMPLE_IOCTL 0x40008000
#pragma warning(disable : 6305)


// Global variables:
PDEVICE_OBJECT GlobalDeviceObject = NULL;
UNICODE_STRING SymbolicLink = RTL_CONSTANT_STRING(L"\\DosDevices\\BootStart");
UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\BootStart");


NTSTATUS CreateCloseCallback(PDEVICE_OBJECT DeviceObject,
	PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	PAGED_CODE();
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS DeviceControlCallback(PDEVICE_OBJECT DeviceObject,
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


VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	IoDeleteSymbolicLink(&SymbolicLink);
	if (GlobalDeviceObject != NULL) {
		IoDeleteDevice(GlobalDeviceObject);
		GlobalDeviceObject = NULL;
	}
	DbgPrintEx(0, 0, "BootStart general - DriverUnload() called\n");
}


NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT  DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	NTSTATUS Status = STATUS_SUCCESS;
	const char* HelloMessage =
		"\n----------\n"
		"_____                   _______                  _______               _____                    _____                _____                    _____                    _____                _____          \n"
		"         /\\    \\                 /::\\    \\                /::\\    \\             /\\    \\                  /\\    \\              /\\    \\                  /\\    \\                  /\\    \\              /\\    \\         \n"
		"        /::\\    \\               /::::\\    \\              /::::\\    \\           /::\\    \\                /::\\    \\            /::\\    \\                /::\\    \\                /::\\    \\            /::\\    \\        \n"
		"       /::::\\    \\             /::::::\\    \\            /::::::\\    \\          \\:::\\    \\              /::::\\    \\           \\:::\\    \\              /::::\\    \\              /::::\\    \\           \\:::\\    \\       \n"
		"      /::::::\\    \\           /::::::::\\    \\          /::::::::\\    \\          \\:::\\    \\            /::::::\\    \\           \\:::\\    \\            /::::::\\    \\            /::::::\\    \\           \\:::\\    \\      \n"
		"     /:::/\\:::\\    \\         /:::/~~\\:::\\    \\        /:::/~~\\:::\\    \\          \\:::\\    \\          /:::/\\:::\\    \\           \\:::\\    \\          /:::/\\:::\\    \\          /:::/\\:::\\    \\           \\:::\\    \\     \n"
		"    /:::/__\\:::\\    \\       /:::/    \\:::\\    \\      /:::/    \\:::\\    \\          \\:::\\    \\        /:::/__\\:::\\    \\           \\:::\\    \\        /:::/__\\:::\\    \\        /:::/__\\:::\\    \\           \\:::\\    \\    \n"
		"   /::::\\   \\:::\\    \\     /:::/    / \\:::\\    \\    /:::/    / \\:::\\    \\         /::::\\    \\       \\:::\\   \\:::\\    \\          /::::\\    \\      /::::\\   \\:::\\    \\      /::::\\   \\:::\\    \\          /::::\\    \\   \n"
		"  /::::::\\   \\:::\\    \\   /:::/____/   \\:::\\____\\  /:::/____/   \\:::\\____\\       /::::::\\    \\    ___\\:::\\   \\:::\\    \\        /::::::\\    \\    /::::::\\   \\:::\\    \\    /::::::\\   \\:::\\    \\        /::::::\\    \\  \n"
		" /:::/\\:::\\   \\:::\\ ___\\ |:::|    |     |:::|    ||:::|    |     |:::|    |     /:::/\\:::\\    \\  /\\   \\:::\\   \\:::\\    \\      /:::/\\:::\\    \\  /:::/\\:::\\   \\:::\\    \\  /:::/\\:::\\   \\:::\\____\\      /:::/\\:::\\    \\ \n"
		"/:::/__\\:::\\   \\:::|    ||:::|____|     |:::|    ||:::|____|     |:::|    |    /:::/  \\:::\\____\\/::\\   \\:::\\   \\:::\\____\\    /:::/  \\:::\\____\\/:::/  \\:::\\   \\:::\\____\\/:::/  \\:::\\   \\:::|    |    /:::/  \\:::\\____\\\n"
		"\\:::\\   \\:::\\  /:::|____| \\:::\\    \\   /:::/    /  \\:::\\    \\   /:::/    /    /:::/    \\::/    /\\:::\\   \\:::\\   \\::/    /   /:::/    \\::/    /\\::/    \\:::\\  /:::/    /\\::/   |::::\\  /:::|____|   /:::/    \\::/    /\n"
		" \\:::\\   \\:::\\/:::/    /   \\:::\\    \\ /:::/    /    \\:::\\    \\ /:::/    /    /:::/    / \\/____/  \\:::\\   \\:::\\   \\/____/   /:::/    / \\/____/  \\/____/ \\:::\\\\/:::/    /  \\/____|:::::\\/:::/    /   /:::/    / \\/____/ \n"
		"  \\:::\\   \\::::::/    /     \\:::\\    /:::/    /      \\:::\\    /:::/    /    /:::/    /            \\:::\\   \\:::\\    \\      /:::/    /                    \\::::::/    /         |:::::::::/    /   /:::/    /          \n"
		"   \\:::\\   \\::::/    /       \\:::\\__/:::/    /        \\:::\\__/:::/    /    /:::/    /              \\:::\\   \\:::\\____\\    /:::/    /                      \\::::/    /          |::|\\::::/    /   /:::/    /           \n"
		"    \\:::\\  /:::/    /         \\::::::::/    /          \\::::::::/    /     \\::/    /                \\:::\\  /:::/    /    \\::/    /                       /:::/    /           |::| \\::/____/    \\::/    /            \n"
		"     \\:::\\/:::/    /           \\::::::/    /            \\::::::/    /       \\/____/                  \\:::\\/:::/    /      \\/____/                       /:::/    /            |::|  ~|           \\/____/             \n"
		"      \\::::::/    /             \\::::/    /              \\::::/    /                                  \\::::::/    /                                    /:::/    /             |::|   |                               \n"
		"       \\::::/    /               \\::/____/                \\::/____/                                    \\::::/    /                                    /:::/    /              \\::|   |                               \n"
		"        \\::/____/                 ~~                       ~~                                           \\::/    /                                     \\::/    /                \\:|   |                               \n"
		"         ~~                                                                                              \\/____/                                       \\/____/                  \\|___|\n\n"
		"Discord: bldysis#0868  GitHub: shaygitub\n"
		"\n----------\n";
	UNREFERENCED_PARAMETER(RegistryPath);
	DbgPrintEx(0, 0, "%s", HelloMessage);


	// Register IOCTL callbacks:
	Status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN, FALSE, &GlobalDeviceObject);
	if (!NT_SUCCESS(Status)) {
		DbgPrintEx(0, 0, "BootStart - IoCreateDevice() failed with status 0x%x\n", Status);
		return Status;
	}
	Status = IoCreateSymbolicLink(&SymbolicLink, &DeviceName);
	if (!NT_SUCCESS(Status)) {
		DbgPrintEx(0, 0, "BootStart - IoCreateSymbolicLink() failed with status 0x%x\n", Status);
		IoDeleteDevice(GlobalDeviceObject);
		return Status;
	}
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCloseCallback;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateCloseCallback;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControlCallback;
	DriverObject->DriverUnload = DriverUnload;


	// Clean all logs of driver service from machine:
	HANDLE MitigationThread = NULL;
	Status = PsCreateSystemThread(
		&MitigationThread,
		GENERIC_ALL,
		NULL,
		NULL,
		NULL,
		(PKSTART_ROUTINE)ServiceLogs::MitigateDataStructures,
		GlobalDeviceObject);

	DbgPrintEx(0, 0, "BootStart - initialized boot process\n");
	return Status;
}