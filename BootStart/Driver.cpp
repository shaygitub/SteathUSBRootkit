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
		" \\:::\\   \\:::\/:::/    /   \\:::\\    \\ /:::/    /    \\:::\\    \\ /:::/    /    /:::/    / \\/____/  \\:::\\   \\:::\\   \\/____/   /:::/    / \\/____/  \\/____/ \\:::\\\\/:::/    /  \\/____|:::::\\/:::/    /   /:::/    / \\/____/ \n"
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