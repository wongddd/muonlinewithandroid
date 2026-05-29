// Android stub for CErrorReport — the original ErrorReport.cpp is deeply
// Win32-specific (registry, DirectSound, IME, SEH, etc.) and not useful on Android.
// This provides empty implementations so the rest of the codebase compiles.

#include "Utilities/Log/ErrorReport.h"
#include <stdarg.h>
#include <stdio.h>

CErrorReport g_ErrorReport;

CErrorReport::CErrorReport()
    : m_hFile(nullptr)
    , m_iKey(0)
{
    m_lpszFileName[0] = '\0';
}

CErrorReport::~CErrorReport() {}

void CErrorReport::Clear() {}

void CErrorReport::Create(char*) {}

void CErrorReport::Destroy() {}

void CErrorReport::CutHead() {}

char* CErrorReport::CheckHeadToCut(char*, DWORD) { return nullptr; }

BOOL CErrorReport::WriteFile(HANDLE, void*, DWORD, LPDWORD, LPOVERLAPPED) { return 0; }

void CErrorReport::WriteDebugInfoStr(char*) {}

void CErrorReport::Write(const char*, ...) {}

void CErrorReport::HexWrite(void*, int) {}

void CErrorReport::AddSeparator() {}

void CErrorReport::WriteLogBegin() {}

void CErrorReport::WriteCurrentTime(BOOL) {}

void CErrorReport::WriteSystemInfo(ER_SystemInfo*) {}

void CErrorReport::WriteOpenGLInfo() {}

void CErrorReport::WriteImeInfo(HWND) {}

void CErrorReport::WriteSoundCardInfo() {}

void GetSystemInfo(ER_SystemInfo*) {}
