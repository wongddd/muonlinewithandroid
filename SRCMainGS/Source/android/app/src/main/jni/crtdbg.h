#pragma once
// Shim for <crtdbg.h> (MSVC CRT debug heap) on Android/Clang
// Not available on Android — stubs for _CrtSetDbgFlag, _CrtSetReportMode etc.
#define _CRTDBG_ALLOC_MEM_DF    0x01
#define _CRTDBG_LEAK_CHECK_DF   0x20
#define _CRT_WARN               0
#define _CRT_ERROR              1
#define _CRT_ASSERT             2
#define _CRTDBG_MODE_DEBUG      0x02
#define _CRTDBG_MODE_WNDW       0x04
#define _CRTDBG_MODE_FILE       0x08

__attribute__((unused)) static int _CrtSetDbgFlag(int) { return 0; }
__attribute__((unused)) static int _CrtSetReportMode(int, int) { return 0; }
