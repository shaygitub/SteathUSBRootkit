#pragma once
#include "definitions.h"


PVOID GetKernelModuleAddress(char* ModuleName);
PVOID FindSection(const char* SectionName, PVOID ModulePointer, PULONG OutputSize);
PVOID FindSectionAtKernel(const char* SectionName, PVOID ModulePointer, PULONG OutputSize);
BOOLEAN bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask);
PVOID FindPattern(PVOID SectionAddress, uintptr_t dwLen, BYTE* bMask, const char* szMask);
PVOID FindPatternAtKernel(PVOID ModuleAddress, uintptr_t dwLen, BYTE* bMask, const char* szMask);
PVOID FindPatternInSectionAtKernel(const char* SectionName, PVOID ModulePointer, BYTE* bMask, const char* szMask);
PVOID ResolveRelativeAddress(_In_ PVOID Instruction, _In_ ULONG OffsetOffset, _In_ ULONG InstructionSize);