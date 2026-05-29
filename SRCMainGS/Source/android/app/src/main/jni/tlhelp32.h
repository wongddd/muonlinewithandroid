#pragma once
// Shim for <tlhelp32.h> (MSVC Toolhelp API) on Android/Clang
// Process snapshots, module enumeration — not supported on Android
#include "MuPlatform.h"

typedef void* HANDLE;
#define TH32CS_SNAPPROCESS  0x00000002
#define TH32CS_SNAPMODULE   0x00000008

struct PROCESSENTRY32 {
    DWORD dwSize;
    DWORD cntUsage;
    DWORD th32ProcessID;
    ULONG_PTR th32DefaultHeapID;
    DWORD th32ModuleID;
    DWORD cntThreads;
    DWORD th32ParentProcessID;
    LONG  pcPriClassBase;
    DWORD dwFlags;
    char  szExeFile[260];
};
typedef PROCESSENTRY32* LPPROCESSENTRY32;

struct MODULEENTRY32 {
    DWORD dwSize;
    DWORD th32ModuleID;
    DWORD th32ProcessID;
    DWORD GlblcntUsage;
    DWORD ProccntUsage;
    BYTE* modBaseAddr;
    DWORD modBaseSize;
    HINSTANCE hModule;
    char  szModule[256];
    char  szExePath[260];
};
typedef MODULEENTRY32* LPMODULEENTRY32;

__attribute__((unused)) inline HANDLE CreateToolhelp32Snapshot(DWORD, DWORD) { return (HANDLE)-1; }
__attribute__((unused)) inline BOOL Process32First(HANDLE, LPPROCESSENTRY32) { return 0; }
__attribute__((unused)) inline BOOL Process32Next(HANDLE, LPPROCESSENTRY32) { return 0; }
__attribute__((unused)) inline BOOL Module32First(HANDLE, LPMODULEENTRY32) { return 0; }
__attribute__((unused)) inline BOOL Module32Next(HANDLE, LPMODULEENTRY32) { return 0; }
