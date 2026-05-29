#pragma once
// ============================================================================
// MuPlatform.h — Windows compatibility layer for Android/NDK
//
// Provides Windows type shims so original Main5.2 source files can compile

// ImGui API export macro (empty on Android — no DLL import/export needed)
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif
// on Android without <windows.h>. Include this BEFORE any original headers.
// ============================================================================

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <mutex>
#include <string>

// ============================================================================
// Fundamental Windows types
// ============================================================================

typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned int        DWORD;
typedef unsigned int        UINT;
typedef int                 BOOL;
typedef long                LONG;
typedef unsigned long       ULONG;
typedef unsigned long long  ULONGLONG;
typedef long long           LONGLONG;

// Pointer-sized integers (match platform)
typedef intptr_t            INT_PTR;
typedef uintptr_t           UINT_PTR;
typedef intptr_t            LONG_PTR;
typedef uintptr_t           ULONG_PTR;
typedef uintptr_t           DWORD_PTR;

typedef unsigned short      USHORT;
typedef signed short        SHORT;
typedef unsigned char       UCHAR;
typedef char                CHAR;
typedef signed char         SCHAR;
typedef long long           __int64;
typedef unsigned long long  __uint64;
typedef void                VOID;
typedef void*               LPVOID;
typedef void*               PVOID;
typedef int                 INT;
typedef LONG                HRESULT;

// MSVC-specific CRT functions
inline char* itoa(int value, char* str, int base) {
    // Android NDK doesn't have itoa; use snprintf for base 10 (the only base used)
    snprintf(str, 12, "%d", value);
    return str;
}

// SAL annotations (used as parameter decorations)
#define IN
#define OUT
#define OPTIONAL

// MSVC built-in type shims
// Note: unsigned __int8 cannot be shimmed via macro (unsigned + signed char conflict).
// Source files using unsigned __int8 are patched directly.
#define __int8              signed char
#define __int16             short
#define __int32             int
#define __int64             long long

// Floating point
typedef float               FLOAT;

// ============================================================================
// Boolean constants (Windows-style)
// ============================================================================

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE  1
#endif

// ============================================================================
// Handle types (all opaque pointers on Android — no actual OS handles)
// ============================================================================

typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HGLRC;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HFONT;
typedef void* HBITMAP;
typedef void* HPEN;
typedef void* HBRUSH;
typedef void* HGDIOBJ;
typedef void* HMENU;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HACCEL;
typedef void* HWAVEOUT;
typedef void* HMIDIOUT;

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#ifndef NULL
#define NULL 0
#endif

// ============================================================================
// String types
// ============================================================================

typedef const char*     LPCSTR;
typedef char*           LPSTR;
typedef wchar_t         WCHAR;
typedef const wchar_t*  LPCWSTR;
typedef wchar_t*        LPWSTR;
typedef char            TCHAR;
typedef char*           LPTSTR;
typedef const char*     LPCTSTR;

// Unicode macros — we use ANSI on Android
#define _T(x)       x
#define TEXT(x)     x
#define _tcslen     strlen
#define _tcscpy     strcpy
#define _tcsncpy    strncpy
#define _tcscmp     strcmp
#define _tcsicmp    strcasecmp
#define _tcsnicmp   strncasecmp
#define strnicmp    strncasecmp
#define stricmp     strcasecmp
#define _stricmp    strcasecmp
#define strcmpi     strcasecmp
#define _tcsstr     strstr
#define _tcsrchr    strrchr
#define _tcschr     strchr
#define _stprintf   sprintf
#define _sntprintf  snprintf
#define _ttoi       atoi
#define _ttol       atol
#define _tstof      atof
#define _tcstod     strtod
#define _tcstoul    strtoul
#define _fgetts     fgets
#define _tcsftime   strftime

// ============================================================================
// Message / Window types
// ============================================================================

typedef UINT_PTR  WPARAM;
typedef LONG_PTR  LPARAM;
typedef LONG_PTR  LRESULT;
typedef void*     LPARAM_PTR;
typedef LRESULT (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

// ============================================================================
// Common Windows structs
// ============================================================================

// Windows BMP file headers (used in ZzzLodTerrain.cpp for heightmap save/load)
#pragma pack(push, 1)
struct BITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
};

struct BITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
};
#pragma pack(pop)

struct POINT {
    LONG x;
    LONG y;
};
typedef POINT tagPOINT;  // MSVC compat

struct RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

struct SIZE {
    LONG cx;
    LONG cy;
};

typedef SIZE* LPSIZE;
typedef size_t* PSIZE_T;
typedef size_t S_SIZE_T;

// MessageBox constants
#define MB_OK                   0x00000000
#define MB_OKCANCEL             0x00000001
#define MB_YESNO                0x00000004
#define MB_ICONEXCLAMATION      0x00000030
#define MB_ICONINFORMATION      0x00000040
#define MB_ICONQUESTION         0x00000020
#define IDOK                    1
#define IDCANCEL                2
#define IDYES                   6
#define IDNO                    7

// Code page constants
#define CP_UTF8                 65001
#define CP_ACP                  0

// NOTIFYICONDATA — stub for system tray (not used on Android)
struct NOTIFYICONDATA {
    DWORD cbSize;
    HWND  hWnd;
    UINT  uID;
    UINT  uFlags;
    UINT  uCallbackMessage;
    HICON hIcon;
    char  szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    char  szInfo[256];
    UINT  uTimeoutOrVersion;
    char  szInfoTitle[64];
    DWORD dwInfoFlags;
};

// MSG (depends on POINT being declared first)
typedef struct tagMSG {
    HWND   hwnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD  time;
    POINT  pt;
} MSG;

struct FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

struct SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

// RTL_OSVERSIONINFOW stub (Windows NT version info, not used on Android)
struct RTL_OSVERSIONINFOW {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR szCSDVersion[128];
};
typedef RTL_OSVERSIONINFOW* PRTL_OSVERSIONINFOW;

// ============================================================================
// Common macros
// ============================================================================

#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))

#define MAX_PATH            260
#define MAX_PATH_LEN        260

// Mixed-type min/max — legacy code uses bare min()/max() with mixed argument types.
// std::min/std::max require identical types; custom templates handle mixed types.
// Must come before any code that calls bare min()/max().
template<typename T, typename U>
inline auto min(const T& a, const U& b) {
    return a < b ? a : b;
}
template<typename T, typename U>
inline auto max(const T& a, const U& b) {
    return a > b ? a : b;
}

// ============================================================================
// Calling convention stubs
// ============================================================================

#define CALLBACK
#define WINAPI
#define APIENTRY
#define STDMETHODCALLTYPE
#define __stdcall
#define __cdecl
#define __fastcall
#define FAR
#define PASCAL
#define NEAR

// ============================================================================
// MSVC-specific stubs
// ============================================================================

#define __declspec(x)
#define __unaligned
#define __forceinline inline __attribute__((always_inline))
#define __restrict __restrict__

// __try/__except → stub (only used in anti-debug code, not needed on Android)
#define __try           if (true)
#define __except(x)     if (false)
#define __finally

// SEH macros
#define EXCEPTION_EXECUTE_HANDLER 1

// ============================================================================
// #pragma pack replacement
// ============================================================================

// Use standard C++11 alignment control instead of #pragma pack
// (original code uses #pragma pack(push,1) for network structs)
#define PACKED __attribute__((packed))

// ============================================================================
// CRITICAL_SECTION → std::mutex wrapper
// ============================================================================

struct CRITICAL_SECTION {
    std::mutex m_mutex;
    std::mutex* operator->() { return &m_mutex; }
};

inline void InitializeCriticalSection(CRITICAL_SECTION* cs) {
    new (cs) CRITICAL_SECTION();
}

inline void DeleteCriticalSection(CRITICAL_SECTION* cs) {
    cs->~CRITICAL_SECTION();
}

inline void EnterCriticalSection(CRITICAL_SECTION* cs) {
    cs->m_mutex.lock();
}

inline void LeaveCriticalSection(CRITICAL_SECTION* cs) {
    cs->m_mutex.unlock();
}

// ============================================================================
// Common Windows API stubs (enough to satisfy linker)
// ============================================================================

inline int wsprintfA(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vsprintf(buf, fmt, args);
    va_end(args);
    return ret;
}

#define wsprintf wsprintfA

inline void OutputDebugStringA(const char* str) {
    // Maps to Android logcat via __android_log_print
    // (callers should use LOGI/LOGE macros instead)
}

#define OutputDebugString OutputDebugStringA

// GetTickCount → std::chrono (used by original code for timing)
#include <chrono>
inline DWORD GetTickCount() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return static_cast<DWORD>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
    );
}

// timeGetTime — WinMM multimedia timer, same as GetTickCount for our purposes
inline DWORD timeGetTime() {
    return GetTickCount();
}

// Sleep
#include <thread>
inline void Sleep(DWORD ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// MessageBox stub
inline int MessageBoxA(HWND, LPCSTR, LPCSTR, UINT) { return IDOK; }
inline int MessageBoxW(HWND, LPCWSTR, LPCWSTR, UINT) { return IDOK; }
#define MessageBox MessageBoxA

// GetFocus stub
inline HWND GetFocus() { return nullptr; }

// Memory
inline void ZeroMemory(void* ptr, size_t size) {
    memset(ptr, 0, size);
}

inline void CopyMemory(void* dst, const void* src, size_t size) {
    memcpy(dst, src, size);
}

// Interlocked operations
#include <atomic>
inline LONG InterlockedIncrement(LONG volatile* addend) {
    return __sync_add_and_fetch(addend, 1);
}
inline LONG InterlockedDecrement(LONG volatile* addend) {
    return __sync_sub_and_fetch(addend, 1);
}
inline LONG InterlockedExchange(LONG volatile* target, LONG value) {
    return __sync_lock_test_and_set(target, value);
}

// ============================================================================
// Socket stubs (if original code references WinSock types)
// ============================================================================

typedef int SOCKET;
typedef DWORD* LPDWORD;
typedef BYTE*  LPBYTE;
typedef void*  LPOVERLAPPED;
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR    (-1)

// WinSock stubs
inline int WSAGetLastError() { return 0; }
#define WSAEWOULDBLOCK 10035

// ============================================================================
// Multi-byte / Unicode conversion stubs
// ============================================================================

inline int MultiByteToWideChar(DWORD, DWORD, const char*, int, wchar_t*, int) {
    return 0; // stub — no wide char conversion needed on Android
}

inline int WideCharToMultiByte(DWORD, DWORD, const wchar_t*, int, char*, int, const char*, BOOL*) {
    return 0; // stub
}

// ============================================================================
// File API constants (for MuFile compatibility)
// ============================================================================

#define GENERIC_READ    0x80000000
#define GENERIC_WRITE   0x40000000
#define FILE_SHARE_READ  0x00000001
#define FILE_SHARE_WRITE 0x00000002
#define OPEN_EXISTING    3
#define CREATE_ALWAYS    2
#define OPEN_ALWAYS      4
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_BEGIN       0
#define FILE_CURRENT     1
#define FILE_END         2
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF)

// ============================================================================
// Misc Windows constants used in original code
// ============================================================================

#define WM_USER                 0x0400
#define WM_QUIT                 0x0012
#define WM_TIMER                0x0113
#define WM_KEYDOWN              0x0100
#define WM_KEYUP                0x0101
#define WM_LBUTTONDOWN          0x0201
#define WM_LBUTTONUP            0x0202
#define WM_RBUTTONDOWN          0x0204
#define WM_RBUTTONUP            0x0205
#define WM_MOUSEMOVE            0x0200
#define WM_MOUSEWHEEL           0x020A
#define WM_SIZE                 0x0005
#define WM_PAINT                0x000F
#define WM_DESTROY              0x0002
#define WM_CLOSE                0x0010
#define WM_CHAR                 0x0102
#define WM_SYSKEYDOWN           0x0104

#define VK_TAB                  0x09
#define VK_RETURN               0x0D
#define VK_SHIFT                0x10
#define VK_CONTROL              0x11
#define VK_MENU                 0x12
#define VK_ESCAPE               0x1B
#define VK_SPACE                0x20
#define VK_LEFT                 0x25
#define VK_UP                   0x26
#define VK_RIGHT                0x27
#define VK_DOWN                 0x28
#define VK_DELETE               0x2E
#define VK_F1                   0x70
#define VK_F2                   0x71
#define VK_F3                   0x72
#define VK_F4                   0x73
#define VK_F5                   0x74
#define VK_F6                   0x75
#define VK_F7                   0x76
#define VK_F8                   0x77
#define VK_F9                   0x78
#define VK_F10                  0x79
#define VK_F11                  0x7A
#define VK_F12                  0x7B
#define VK_LBUTTON              0x01
#define VK_RBUTTON              0x02
#define VK_MBUTTON              0x04
#define VK_XBUTTON1             0x05
#define VK_XBUTTON2             0x06
#define VK_LCONTROL             0xA2
#define VK_RCONTROL             0xA3
#define VK_PRIOR                0x21
#define VK_NEXT                 0x22
#define VK_HOME                 0x24
#define VK_BACK                 0x08
#define SW_SHOW                 5

// Color macros
#define RGB(r,g,b)      ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define GetRValue(rgb)  ((BYTE)(rgb))
#define GetGValue(rgb)  ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)  ((BYTE)((rgb)>>16))

// ============================================================================
// GDI function stubs (needed by unicode namespace in _types.h)
// These are never actually called on Android — the original code's text
// rendering is replaced by ImGui / bitmap fonts.
// ============================================================================

inline BOOL GetTextExtentPointA(HDC, LPCSTR, int, LPSIZE) { return 0; }
inline BOOL GetTextExtentPoint32A(HDC, LPCSTR, int, LPSIZE) { return 0; }
inline BOOL TextOutA(HDC, int, int, LPCSTR, int) { return 0; }

// SetRect — sets the coordinates of a RECT structure
inline BOOL SetRect(RECT* rc, int left, int top, int right, int bottom) {
    rc->left = left; rc->top = top; rc->right = right; rc->bottom = bottom;
    return 0;
}

__attribute__((unused)) inline BOOL IntersectRect(RECT* dst, const RECT* a, const RECT* b) {
    dst->left   = a->left   > b->left   ? a->left   : b->left;
    dst->top    = a->top    > b->top    ? a->top    : b->top;
    dst->right  = a->right  < b->right  ? a->right  : b->right;
    dst->bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
    return (dst->left < dst->right) && (dst->top < dst->bottom);
}

__attribute__((unused)) inline BOOL OffsetRect(RECT* rc, int dx, int dy) {
    rc->left += dx; rc->top += dy;
    rc->right += dx; rc->bottom += dy;
    return 0;
}

// ============================================================================
// Timer stubs (SetTimer/KillTimer from Win32, used by w_BuffTimeControl.cpp)
// ============================================================================

typedef void (*TIMERPROC)(HWND, UINT, UINT_PTR, DWORD);
__attribute__((unused)) inline UINT_PTR SetTimer(HWND, UINT_PTR, UINT, TIMERPROC) { return 1; }
__attribute__((unused)) inline BOOL KillTimer(HWND, UINT_PTR) { return 0; }

// ============================================================================
// Safe string functions (MSVC template versions that deduce size from array)
#define fopen_s(fp, path, mode)   (*(fp) = fopen(path, mode), *(fp) != nullptr ? 0 : -1)
#define localtime_s(tm, time)     (*(tm) = *localtime(time), 0)
#define _tzset                    tzset
#define _vsntprintf               vsnprintf
#define _ultoa_s(v, b, r)         snprintf(b, sizeof(b), "%lu", (unsigned long)(v))
inline char* _itoa(int v, char* b, int r) { snprintf(b, 32, "%d", v); return b; }
#define GetKeyState(x)            (0)

// IsBadReadPtr stub (Windows memory validation, always return "OK" on Android)
__attribute__((unused)) inline BOOL IsBadReadPtr(const void*, UINT_PTR) { return 0; }

// GDI font constants
#define FW_BOLD                  700
#define DEFAULT_CHARSET          1
#define OUT_DEFAULT_PRECIS       0
#define CLIP_DEFAULT_PRECIS      0
#define NONANTIALIASED_QUALITY   3
#define DEFAULT_PITCH            0
#define FF_DONTCARE              (0<<4)

// ============================================================================
// Additional GDI stubs
// ============================================================================

struct BITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    // RGBQUAD bmiColors[1]; — not needed for stub
};

typedef struct tagBITMAPINFOHEADER2 {
    DWORD biSize;
    LONG  biWidth, biHeight;
    WORD  biPlanes, biBitCount;
    DWORD biCompression, biSizeImage;
    LONG  biXPelsPerMeter, biYPelsPerMeter;
    DWORD biClrUsed, biClrImportant;
} BITMAPINFOHEADER2;

__attribute__((unused)) inline BOOL DeleteObject(HGDIOBJ) { return 0; }
__attribute__((unused)) inline DWORD SetBkColor(HDC, DWORD) { return 0; }
__attribute__((unused)) inline DWORD SetTextColor(HDC, DWORD) { return 0; }
__attribute__((unused)) inline HDC CreateCompatibleDC(HDC) { return nullptr; }

#define GWL_WNDPROC             (-4)
#define BI_RGB                  0
#define DIB_RGB_COLORS          0
#define OBJ_FONT                6

// MCI (Media Control Interface) constants — not supported on Android
#define MCI_SEQ_MAPPER          ((DWORD)-1)
// WinSock async select event flags
#define FD_CONNECT              0x00000010
#define FD_READ                 0x00000001
#define FD_WRITE                0x00000002
#define FD_CLOSE                0x00000020

// WM_IME messages
#define WM_IME_STARTCOMPOSITION 0x010D
#define WM_IME_ENDCOMPOSITION   0x010E
#define WM_IME_COMPOSITION      0x010F

// GDI / window long constants
#define GWL_USERDATA            (-21)
#define CF_TEXT                 1

// Clipboard stubs
typedef void* HGLOBAL;
__attribute__((unused)) inline BOOL OpenClipboard(HWND) { return 0; }
__attribute__((unused)) inline BOOL CloseClipboard() { return 0; }

// GDI palette entry
typedef struct tagPALETTEENTRY {
    BYTE peRed, peGreen, peBlue, peFlags;
} PALETTEENTRY;

// More GDI stubs
__attribute__((unused)) inline HGDIOBJ GetCurrentObject(HDC, UINT) { return nullptr; }
__attribute__((unused)) inline HBITMAP CreateDIBSection(HDC, const BITMAPINFO*, UINT, void**, HANDLE, DWORD) { return nullptr; }
__attribute__((unused)) inline LONG SetWindowLongW(HWND, int, LONG) { return 0; }
#define SetWindowLongPtr SetWindowLongW
__attribute__((unused)) inline BOOL DestroyWindow(HWND) { return 0; }
__attribute__((unused)) inline HWND CreateWindowW(LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, void*) { return (HWND)0x1; }
#define CreateWindow CreateWindowA
__attribute__((unused)) inline HWND CreateWindowA(LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, void*) { return (HWND)0x1; }

// Clipboard stubs
__attribute__((unused)) inline HANDLE GetClipboardData(UINT) { return nullptr; }
__attribute__((unused)) inline void* GlobalLock(HANDLE) { return nullptr; }
__attribute__((unused)) inline BOOL GlobalUnlock(HANDLE) { return 0; }

// Window text stubs
__attribute__((unused)) inline LONG GetWindowLongW(HWND, int) { return 0; }
#define GetWindowLongPtr GetWindowLongW
__attribute__((unused)) inline int GetWindowTextW(HWND, LPWSTR, int) { return 0; }
__attribute__((unused)) inline BOOL SetWindowTextW(HWND, LPCWSTR) { return 0; }
__attribute__((unused)) inline LRESULT CallWindowProcW(WNDPROC, HWND, UINT, WPARAM, LPARAM) { return 0; }

// Caret stub
__attribute__((unused)) inline BOOL GetCaretPos(POINT*) { return 0; }

// IME / edit control constants
#define WM_IME_NOTIFY            0x0282
#define IMN_SETOPENSTATUS        0x0005
#define CFS_FORCE_POSITION       0x0020
#define GCS_COMPSTR              0x0008
#define _TRUNCATE                ((size_t)-1)
#define EM_SETLIMITTEXT          0x00C5
#define SWP_NOMOVE               0x0002
#define SWP_NOZORDER             0x0004
#define SWP_NOSIZE               0x0001
#define SWP_HIDEWINDOW           0x0080
#define HWND_TOPMOST             ((HWND)-1)

// Edit control styles
#define ES_PASSWORD              0x0020
#define ES_MULTILINE             0x0004
#define ES_AUTOVSCROLL           0x0040
#define ES_AUTOHSCROLL           0x0080
#define WS_VSCROLL               0x00200000
#define WS_CHILD                 0x40000000
#define EM_SETSEL                0x00B1
#define EM_GETLINECOUNT          0x00BA
#define EM_SCROLL                0x00B5
#define EM_LINESCROLL            0x00B6
#define WM_SETFONT               0x0030
#define WM_ERASEBKGND            0x0014
#define WS_VISIBLE               0x10000000
#define SW_HIDE                  0
#define SB_VERT                  1
#define SB_LINEUP                0
#define SB_LINEDOWN              1
#define SB_PAGEUP                2
#define SB_PAGEDOWN              3

__attribute__((unused)) inline BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT) { return TRUE; }
__attribute__((unused)) inline BOOL ShowWindow(HWND, int) { return TRUE; }
__attribute__((unused)) inline BOOL IsWindowVisible(HWND) { return TRUE; }
__attribute__((unused)) inline int GetScrollPos(HWND, int) { return 0; }
__attribute__((unused)) inline int SetScrollPos(HWND, int, int, BOOL) { return 0; }
#define PostMessageW PostMessageA
#define PostMessage  PostMessageA

// Registry types
typedef void* HKEY;
#define HKEY_CLASSES_ROOT        ((HKEY)0x80000000)
#define HKEY_CURRENT_CONFIG      ((HKEY)0x80000005)
#define HKEY_CURRENT_USER        ((HKEY)0x80000001)
#define HKEY_LOCAL_MACHINE       ((HKEY)0x80000002)
#define HKEY_USERS               ((HKEY)0x80000003)
#define REG_DWORD                4
#define REG_OPTION_NON_VOLATILE  0
#define KEY_ALL_ACCESS           0xF003F
#define KEY_QUERY_VALUE          0x0001
#define ERROR_SUCCESS            0
#define ERROR_ALREADY_EXISTS     183

__attribute__((unused)) inline LONG RegQueryValueEx(HKEY, LPCSTR, DWORD*, DWORD*, BYTE*, DWORD*) { return ERROR_SUCCESS; }
__attribute__((unused)) inline LONG RegCloseKey(HKEY) { return ERROR_SUCCESS; }
#define REG_NONE                 0
#define REG_EXPAND_SZ            2
#define REG_BINARY               3
#define REG_SZ                   1
__attribute__((unused)) inline LONG RegCreateKeyEx(HKEY, LPCSTR, DWORD, LPSTR, DWORD, DWORD, void*, HKEY*, DWORD*) { return ERROR_SUCCESS; }
__attribute__((unused)) inline LONG RegSetValueEx(HKEY, LPCSTR, DWORD, DWORD, const BYTE*, DWORD) { return ERROR_SUCCESS; }
__attribute__((unused)) inline LONG RegDeleteKey(HKEY, LPCSTR) { return ERROR_SUCCESS; }
__attribute__((unused)) inline LONG RegOpenKeyEx(HKEY, LPCSTR, DWORD, DWORD, HKEY*) { return ERROR_SUCCESS; }
__attribute__((unused)) inline LONG RegDeleteValue(HKEY, LPCSTR) { return ERROR_SUCCESS; }

// SendMessage stubs
__attribute__((unused)) inline LRESULT SendMessageA(HWND, UINT, WPARAM, LPARAM) { return 0; }
__attribute__((unused)) inline LRESULT SendMessageW(HWND, UINT, WPARAM, LPARAM) { return 0; }
#define SendMessage SendMessageA

// CreateFont stub
__attribute__((unused)) inline HFONT CreateFontA(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, LPCSTR) { return nullptr; }
#define CreateFont CreateFontA

template <size_t N>
inline int strcpy_s(char (&dst)[N], const char* src) {
    strncpy(dst, src, N - 1);
    dst[N - 1] = '\0';
    return 0;
}

template <size_t N>
inline int strcat_s(char (&dst)[N], const char* src) {
    strncat(dst, src, N - strlen(dst) - 1);
    return 0;
}

inline int strncat_s(char *dst, size_t dst_size, const char* src, size_t count) {
    if (count == ((size_t)-1)) count = dst_size - strlen(dst) - 1;
    size_t max_copy = dst_size - strlen(dst) - 1;
    if (count > max_copy) count = max_copy;
    strncat(dst, src, count);
    return 0;
}

template <size_t N>
inline int sprintf_s(char (&buf)[N], const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(buf, N, fmt, args);
    va_end(args);
    return ret;
}

#define _snprintf_s(buf, size, count, ...) snprintf(buf, count, __VA_ARGS__)

// wchar safe stubs
#define _tcscpy_s(dst, size, src)    strcpy(dst, src)
#define _tcsncpy_s(dst, size, src, n) strncpy(dst, src, n)
#define wcscpy_s(dst, size, src)     ((void)(size), (void)0)
#define swprintf_s(buf, size, ...)   swprintf(buf, size, __VA_ARGS__)

// ============================================================================
// Additional MessageBox constants
// ============================================================================

#define MB_ICONERROR            0x00000010
#define MB_ICONWARNING          0x00000030
#define MB_ICONASTERISK         0x00000040

// PostMessage stub
__attribute__((unused)) inline LRESULT PostMessageA(HWND, UINT, WPARAM, LPARAM) { return 0; }

// ============================================================================
// MSVC CRT path constants (_splitpath, _MAX_DRIVE, _MAX_DIR, _MAX_FNAME)
// ============================================================================

#define _MAX_DRIVE  3
#define _MAX_DIR    256
#define _MAX_FNAME  256
#define _MAX_EXT    256

inline void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext) {
    // Minimal stub — only called by GlobalBitmap::ExchangeExt which feeds "" for ext
    if (drive) drive[0] = '\0';
    if (dir) dir[0] = '\0';
    if (fname) fname[0] = '\0';
    if (ext) ext[0] = '\0';
    if (!path) return;

    const char* lastSlash = strrchr(path, '/');
    const char* lastBackslash = strrchr(path, '\\');
    const char* sep = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;
    const char* dot = strrchr(path, '.');

    if (sep) {
        if (dir) {
            size_t len = sep - path + 1;
            if (len < _MAX_DIR) { memcpy(dir, path, len); dir[len] = '\0'; }
        }
    }

    const char* nameStart = sep ? sep + 1 : path;
    if (dot && dot > nameStart) {
        if (fname) {
            size_t len = dot - nameStart;
            if (len < _MAX_FNAME) { memcpy(fname, nameStart, len); fname[len] = '\0'; }
        }
        if (ext && *(dot + 1)) strncpy(ext, dot, _MAX_EXT - 1);
    } else {
        if (fname) strncpy(fname, nameStart, _MAX_FNAME - 1);
    }
}

// ============================================================================
// CreateDirectory stub (not functional on Android)
// ============================================================================

inline BOOL CreateDirectoryA(LPCSTR, void*) { return 0; }
#define CreateDirectory CreateDirectoryA

// ============================================================================
// ShellExecute stub
// ============================================================================

inline HINSTANCE ShellExecuteA(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, int) { return nullptr; }
#define ShellExecute ShellExecuteA

// ============================================================================
// Win32 string functions
// ============================================================================

#define lstrlen     strlen
#define _mbclen(c)  1       // Simplified — always returns 1 for single-byte

// _strupr — MSVC CRT uppercase-in-place
inline char* _strupr(char* str) {
    for (char* p = str; *p; ++p) {
        if (*p >= 'a' && *p <= 'z') *p -= 32;
    }
    return str;
}

// ============================================================================
// Win32 input / IME stubs
// ============================================================================

inline SHORT GetAsyncKeyState(int) { return 0; }

typedef void* HIMC;

inline HIMC ImmGetContext(HWND) { return nullptr; }
inline BOOL ImmGetConversionStatus(HIMC, DWORD*, DWORD*) { return 0; }
inline BOOL ImmSetConversionStatus(HIMC, DWORD, DWORD) { return 0; }
inline BOOL ImmReleaseContext(HWND, HIMC) { return 0; }
__attribute__((unused)) inline BOOL ImmGetCompositionWindow(HIMC, void*) { return 0; }
__attribute__((unused)) inline BOOL ImmSetCompositionWindow(HIMC, void*) { return 0; }

#define IME_CMODE_NATIVE  0x00000001
#define IME_SMODE_NONE    0x00000000

// ============================================================================
// GDI / OpenGL utility stubs
// ============================================================================

inline BOOL SwapBuffers(HDC) { return 0; }
inline HWND SetFocus(HWND) { return nullptr; }

// ============================================================================
// Time / system stubs
// ============================================================================

inline void GetLocalTime(SYSTEMTIME* st) {
    time_t t = time(nullptr);
    struct tm* tm = localtime(&t);
    st->wYear = tm->tm_year + 1900;
    st->wMonth = tm->tm_mon + 1;
    st->wDayOfWeek = tm->tm_wday;
    st->wDay = tm->tm_mday;
    st->wHour = tm->tm_hour;
    st->wMinute = tm->tm_min;
    st->wSecond = tm->tm_sec;
    st->wMilliseconds = 0;
}

// ============================================================================
// More virtual key codes
// ============================================================================

#define VK_SNAPSHOT             0x2C
#define VK_CAPITAL              0x14

// ============================================================================
// IME constants and structs
// ============================================================================

#define IME_CMODE_ALPHANUMERIC      0x0000
#define IME_SMODE_AUTOMATIC         0x0002

#define WM_IME_CONTROL              0x0283
#define IMC_SETCOMPOSITIONWINDOW    0x000C
#define CFS_POINT                   0x0002

struct COMPOSITIONFORM {
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
};

inline HWND ImmGetDefaultIMEWnd(HWND) { return nullptr; }
__attribute__((unused)) inline LONG ImmGetCompositionStringW(HIMC, DWORD, void*, DWORD) { return 0; }
__attribute__((unused)) inline LONG ImmGetCompositionStringA(HIMC, DWORD, void*, DWORD) { return 0; }
#define ImmGetCompositionString ImmGetCompositionStringA

// ============================================================================
// Cursor API stubs
// ============================================================================

inline BOOL SetCursorPos(int, int) { return 0; }

// ============================================================================
// _ASSERT / _ASSERTE macros (MSVC CRT debug assertions)
// ============================================================================

#define _ASSERT(x)      ((void)0)
#define _ASSERTE(x)     ((void)0)

// ============================================================================
// PtInRect stub
// ============================================================================

inline BOOL PtInRect(const RECT* rc, POINT pt) {
    return pt.x >= rc->left && pt.x < rc->right && pt.y >= rc->top && pt.y < rc->bottom;
}

// ============================================================================
// Wide-char GDI stubs (for MultiLanguage.cpp)
// ============================================================================

inline BOOL GetTextExtentPoint32W(HDC, LPCWSTR, int, LPSIZE) { return 0; }
inline BOOL TextOutW(HDC, int, int, LPCWSTR, int) { return 0; }

// ============================================================================
// GDI font stubs (for CGMFontLayer.cpp)
// ============================================================================

#define GB2312_CHARSET      134
#define GDI_ERROR           0xFFFFFFFF

struct TEXTMETRICW {
    LONG tmHeight;
    LONG tmAscent;
    LONG tmDescent;
    LONG tmInternalLeading;
    LONG tmExternalLeading;
    LONG tmAveCharWidth;
    LONG tmMaxCharWidth;
    LONG tmWeight;
    LONG tmOverhang;
    LONG tmDigitizedAspectX;
    LONG tmDigitizedAspectY;
    WCHAR tmFirstChar;
    WCHAR tmLastChar;
    WCHAR tmDefaultChar;
    WCHAR tmBreakChar;
    BYTE tmItalic;
    BYTE tmUnderlined;
    BYTE tmStruckOut;
    BYTE tmPitchAndFamily;
    BYTE tmCharSet;
};

typedef TEXTMETRICW TEXTMETRIC;

inline HGDIOBJ SelectObject(HDC, HGDIOBJ) { return nullptr; }
inline DWORD GetFontData(HDC, DWORD, DWORD, LPVOID, DWORD) { return GDI_ERROR; }
inline BOOL GetTextMetricsW(HDC, TEXTMETRICW*) { return 0; }

// ============================================================================
// INI file API stubs
// ============================================================================

inline UINT GetPrivateProfileIntA(LPCSTR, LPCSTR, int, LPCSTR) { return 0; }
#define GetPrivateProfileInt GetPrivateProfileIntA

// ============================================================================
// GDI DC stubs
// ============================================================================

inline HDC GetDC(HWND) { return nullptr; }
inline BOOL DeleteDC(HDC) { return 0; }

// ============================================================================
// Module file name stub
// ============================================================================

inline DWORD GetModuleFileNameA(HINSTANCE, LPSTR, DWORD) { return 0; }
inline DWORD GetModuleFileNameW(HINSTANCE, LPWSTR, DWORD) { return 0; }

inline DWORD GetLastError() { return 0; }

// ============================================================================
// strncpy_s (MSVC safe CRT)
// ============================================================================

template <size_t N>
inline int strncpy_s(char (&dst)[N], const char* src, size_t count) {
    size_t n = count < (N - 1) ? count : (N - 1);
    strncpy(dst, src, n);
    dst[n] = '\0';
    return 0;
}

// 4-arg version: MSVC strncpy_s(dst, dst_size, src, count)
template <size_t N>
inline int strncpy_s(char (&dst)[N], size_t dst_size, const char* src, size_t count) {
    size_t n = count < (N - 1) ? count : (N - 1);
    n = n < (dst_size - 1) ? n : (dst_size - 1);
    strncpy(dst, src, n);
    dst[n] = '\0';
    return 0;
}

inline int strncpy_s(char* dst, size_t dst_size, const char* src, size_t count) {
    size_t n = count < (dst_size - 1) ? count : (dst_size - 1);
    strncpy(dst, src, n);
    dst[n] = '\0';
    return 0;
}

// ============================================================================
// Volume / system info stubs
// ============================================================================

inline BOOL GetVolumeInformationA(LPCSTR, LPSTR, DWORD, DWORD*, DWORD*, DWORD*, LPSTR, DWORD) { return 0; }
#define GetVolumeInformation GetVolumeInformationA

typedef struct _UUID {
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
} UUID;

struct SYSTEM_INFO {
    union { DWORD dwOemId; struct { WORD wProcessorArchitecture; WORD wReserved; }; };
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
};

inline int UuidCreateSequential(UUID*) { return 0; }

inline void GetSystemInfo(SYSTEM_INFO*) {}

// ============================================================================
// vsprintf_s (for pugixml.cpp)
// ============================================================================

inline int vsprintf_s(char* buf, size_t sz, const char* fmt, va_list args) {
    return vsnprintf(buf, sz, fmt, args);
}

template <size_t N>
inline int vsprintf_s(char (&buf)[N], const char* fmt, va_list args) {
    return vsnprintf(buf, N, fmt, args);
}

// ============================================================================
// Themida VM_START/VM_END stubs (anti-tamper, not used on Android)
// Prevent ThemidaSDK.h from defining these using __asm__ / __emit__
// ============================================================================

#define __THEMIDASDK__  // block ThemidaSDK.h entirely
#define __THEMIDA_SDK__ // alternate guard some versions use
#define VM_START
#define VM_END
#define VM_START_WITHLEVEL(x)
#define CODEREPLACE_START
#define CODEREPLACE_END
#define ENCODE_START
#define ENCODE_END
#define CLEAR_START
#define CLEAR_END
#define UNPROTECTED_START
#define UNPROTECTED_END

// ============================================================================
// CONST type (used in KeyGenerater.h)
// ============================================================================

#define CONST const

// ============================================================================
// RECT/PtInRect helpers (for ImGui hit testing)
// ============================================================================

inline BOOL ClientToScreen(HWND, POINT*) { return 0; }
inline BOOL ScreenToClient(HWND, POINT*) { return 0; }

// ============================================================================
// Win32 Performance Counter + High-Res Timer types
// ============================================================================

typedef union _LARGE_INTEGER {
    struct { DWORD LowPart; LONG HighPart; };
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER* PLARGE_INTEGER;

__attribute__((unused)) inline BOOL QueryPerformanceFrequency(LARGE_INTEGER*) { return 0; }
__attribute__((unused)) inline BOOL QueryPerformanceCounter(LARGE_INTEGER*) { return 0; }

typedef struct {
    UINT wPeriodMin;
    UINT wPeriodMax;
} TIMECAPS;

#define TIMERR_NOCANDO     0x61

__attribute__((unused)) inline UINT timeGetDevCaps(TIMECAPS*, UINT) { return 0; }
__attribute__((unused)) inline UINT timeBeginPeriod(UINT) { return 0; }
__attribute__((unused)) inline UINT timeEndPeriod(UINT) { return 0; }

// ============================================================================
// DLL loading stubs (ScaleForm.cpp uses these for flash/SWF)
// ============================================================================

typedef intptr_t (__stdcall *FARPROC)();

__attribute__((unused)) inline HINSTANCE LoadLibraryA(LPCSTR) { return nullptr; }
#define LoadLibrary LoadLibraryA
__attribute__((unused)) inline FARPROC GetProcAddress(HINSTANCE, LPCSTR) { return nullptr; }
__attribute__((unused)) inline BOOL FreeLibrary(HINSTANCE) { return 0; }

// ============================================================================
// fopen redirection — transparent APK asset loading + path normalization
// All game-code fopen() calls go through mu_fopen_android(), which:
//   1. Normalizes backslashes to forward slashes
//   2. Read-only relative paths: tries disk first, extracts from APK if missing
//   3. Write / absolute paths: passes through directly
//
// In mu_fopen_android() implementation, use (fopen)(...) to call the real libc
// fopen — the parentheses prevent macro expansion.
// ============================================================================

// Declare the wrapper
FILE* mu_fopen_android(const char* path, const char* mode);
void mu_asset_extractor_init(void* assetManager, const char* internalPath);

// Redirect fopen — all source files including MuPlatform.h route through our wrapper
#define fopen(path, mode) mu_fopen_android(path, mode)

// ============================================================================
// File API stubs (ErrorReport.cpp, some utilities use these)
// ============================================================================

__attribute__((unused)) inline HANDLE CreateFileA(LPCSTR, DWORD, DWORD, void*, DWORD, DWORD, HANDLE) { return INVALID_HANDLE_VALUE; }
#define CreateFile CreateFileA
__attribute__((unused)) inline DWORD SetFilePointer(HANDLE, LONG, LONG*, DWORD) { return 0; }
__attribute__((unused)) inline BOOL CloseHandle(HANDLE) { return 0; }
__attribute__((unused)) inline BOOL ReadFile(HANDLE, void*, DWORD, LPDWORD, LPOVERLAPPED) { return 0; }
__attribute__((unused)) inline BOOL WriteFile(HANDLE, const void*, DWORD, LPDWORD, LPOVERLAPPED) { return 0; }
__attribute__((unused)) inline BOOL DeleteFileA(LPCSTR) { return 0; }
#define DeleteFile DeleteFileA

// ============================================================================
// Keyboard layout stubs
// ============================================================================

typedef void* HKL;
__attribute__((unused)) inline HKL GetKeyboardLayout(DWORD) { return nullptr; }
__attribute__((unused)) inline BOOL GetKeyboardLayoutNameA(LPSTR) { return 0; }
#define GetKeyboardLayoutName GetKeyboardLayoutNameA

// ============================================================================
// GUID stub
// ============================================================================

typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;

// ============================================================================
// DirectSound enumeration stub
// ============================================================================

typedef BOOL (*LPDSENUMCALLBACK)(void*, void*, void*, void*);

// ============================================================================
// System directory stub
// ============================================================================

__attribute__((unused)) inline DWORD GetSystemDirectoryA(LPSTR, DWORD) { return 0; }
#define GetSystemDirectory GetSystemDirectoryA

// ============================================================================
// Version info stub
// ============================================================================

typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    char  szCSDVersion[128];
} OSVERSIONINFOA, *LPOSVERSIONINFOA;
#define OSVERSIONINFO OSVERSIONINFOA

// ============================================================================
// INI file string API stub
// ============================================================================

__attribute__((unused)) inline DWORD GetPrivateProfileStringA(LPCSTR, LPCSTR, LPCSTR, LPSTR, DWORD, LPCSTR) { return 0; }
#define GetPrivateProfileString GetPrivateProfileStringA

// ============================================================================
// Console color constants (for WindowsConsole.h)
// ============================================================================

#define FOREGROUND_BLUE         0x0001
#define FOREGROUND_GREEN        0x0002
#define FOREGROUND_RED          0x0004
#define FOREGROUND_INTENSITY    0x0008
