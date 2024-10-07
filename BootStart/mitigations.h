#pragma once
#include "helpers.h"
#pragma warning(disable : 4996)


namespace ServiceLogs {
	VOID MitigateDataStructures(_In_ PDEVICE_OBJECT GlobalDeviceObject);
	NTSTATUS MitigateWdFilter();
	NTSTATUS MitigateUnloadedDrivers(PDEVICE_OBJECT GlobalDeviceObject);
}