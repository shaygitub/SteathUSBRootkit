#include "helpers.h"
#pragma warning(disable : 4996)


PVOID GetKernelModuleAddress(char* ModuleName) {
	void* InfoBuffer = nullptr;
	DWORD InfoBufferSize = 0;
	PRTL_PROCESS_MODULES ActualModules = NULL;
	char CurrentModule[1024] = { 0 };
	PVOID ModuleBase = NULL;


	// Query information about all system modules:
	NTSTATUS Status = ZwQuerySystemInformation(SystemModuleInformation, InfoBuffer, InfoBufferSize, &InfoBufferSize);
	while (Status == STATUS_INFO_LENGTH_MISMATCH) {
		if (InfoBuffer != NULL) {
			ExFreePool(InfoBuffer);
		}
		InfoBuffer = ExAllocatePoolWithTag(NonPagedPool, InfoBufferSize, 0x90807060);
		Status = ZwQuerySystemInformation(SystemModuleInformation, InfoBuffer, InfoBufferSize, &InfoBufferSize);
	}

	if (!NT_SUCCESS(Status)) {
		if (InfoBuffer != NULL) {
			ExFreePool(InfoBuffer);
		}
		return NULL;
	}


	// Find module of searched module:
	ActualModules = (PRTL_PROCESS_MODULES)InfoBuffer;
	if (ActualModules == NULL) {
		return 0;
	}
	for (DWORD ModuleIndex = 0; ModuleIndex < ActualModules->NumberOfModules; ++ModuleIndex) {
		RtlZeroMemory(CurrentModule, 1024);
		strcat_s(CurrentModule, (char*)ActualModules->Modules[ModuleIndex].FullPathName + ActualModules->Modules[ModuleIndex].OffsetToFileName);
		if (!_stricmp(CurrentModule, ModuleName)) {
			ModuleBase = ActualModules->Modules[ModuleIndex].ImageBase;
			ExFreePool(InfoBuffer);
			return ModuleBase;
		}
	}
	ExFreePool(InfoBuffer);
	return 0;
}


PVOID FindSection(const char* SectionName, PVOID ModulePointer, PULONG OutputSize) {
	SIZE_T SectionNameLength = strlen(SectionName);
	PIMAGE_NT_HEADERS Headers = (PIMAGE_NT_HEADERS)((ULONG64)ModulePointer + ((PIMAGE_DOS_HEADER)ModulePointer)->e_lfanew);
	PIMAGE_SECTION_HEADER Section = NULL;
	PIMAGE_SECTION_HEADER Sections = IMAGE_FIRST_SECTION(Headers);
	for (DWORD SectionIndex = 0; SectionIndex < Headers->FileHeader.NumberOfSections; ++SectionIndex) {
		Section = &Sections[SectionIndex];
		if (memcmp(Section->Name, SectionName, SectionNameLength) == 0 &&
			SectionNameLength == strlen((char*)Section->Name)) {
			if (Section->VirtualAddress == NULL) {
				return NULL;
			}
			if (OutputSize) {
				*OutputSize = Section->Misc.VirtualSize;
			}
			return (PVOID)((ULONG64)ModulePointer + Section->VirtualAddress);
		}
	}
	return 0;
}


PVOID FindSectionAtKernel(const char* SectionName, PVOID ModulePointer, PULONG OutputSize) {
	BYTE Headers[0x1000] = { 0 };
	ULONG SectionSize = 0;
	PVOID ActualSection = FindSection(SectionName, (PVOID)Headers, &SectionSize);
	if (!ActualSection || !SectionSize || ModulePointer == NULL) {
		return NULL;
	}
	RtlCopyMemory(Headers, ModulePointer, 0x1000);
	if (OutputSize) {
		*OutputSize = SectionSize;
	}
	return (PVOID)((ULONG64)ActualSection - (ULONG64)Headers + (ULONG64)ModulePointer);
}


BOOLEAN bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask) {
	for (; *szMask; ++szMask, ++pData, ++bMask) {
		if (*szMask == 'x' && *pData != *bMask) {
			return 0;
		}
	}
	return (*szMask) == 0;
}


PVOID FindPattern(PVOID SectionAddress, uintptr_t dwLen, BYTE* bMask, const char* szMask) {
	size_t MaxLength = dwLen - strlen(szMask);
	for (uintptr_t i = 0; i < MaxLength; i++) {
		if (bDataCompare((BYTE*)((ULONG64)SectionAddress + i), bMask, szMask)) {
			return (PVOID)((ULONG64)SectionAddress + i);
		}
	}
	return NULL;
}


PVOID FindPatternAtKernel(PVOID ModuleAddress, uintptr_t dwLen, BYTE* bMask, const char* szMask) {
	PVOID SectionData = NULL;
	PVOID Pattern = NULL;
	if (!ModuleAddress) {
		return NULL;
	}
	if (dwLen > 1024 * 1024 * 1024) {
		return NULL;
	}
	SectionData = ExAllocatePoolWithTag(NonPagedPool, dwLen, 0x50403020);
	if (SectionData == NULL) {
		return NULL;
	}
	RtlCopyMemory(SectionData, ModuleAddress, dwLen);
	Pattern = FindPattern(SectionData, dwLen, bMask, szMask);
	if (Pattern <= 0) {
		ExFreePool(SectionData);
		return 0;
	}
	return (PVOID)((ULONG64)ModuleAddress - (ULONG64)SectionData + (ULONG64)Pattern);
}


PVOID FindPatternInSectionAtKernel(const char* SectionName, PVOID ModulePointer, BYTE* bMask, const char* szMask) {
	ULONG SectionSize = 0;
	PVOID Section = FindSectionAtKernel(SectionName, ModulePointer, &SectionSize);
	return FindPatternAtKernel(Section, SectionSize, bMask, szMask);
}


PVOID ResolveRelativeAddress(_In_ PVOID Instruction, _In_ ULONG OffsetOffset, _In_ ULONG InstructionSize) {
	ULONG_PTR Instr = (ULONG_PTR)Instruction;
	LONG RipOffset = 0;
	RtlCopyMemory(&RipOffset, (PVOID)(Instr + OffsetOffset), sizeof(LONG));
	PVOID ResolvedAddr = (PVOID)(Instr + InstructionSize + RipOffset);
	return ResolvedAddr;
}