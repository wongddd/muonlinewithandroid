#pragma once
// Shim for Wininet.h (Windows Internet API) on Android
// FTP/HTTP functions are not available; GameShop subsystem stubbed
#include "MuPlatform.h"

typedef void* HINTERNET;
typedef WORD INTERNET_PORT;

#define INTERNET_MAX_URL_LENGTH          2083
#define INTERNET_MAX_USER_NAME_LENGTH    128
#define INTERNET_MAX_PASSWORD_LENGTH     128
#define INTERNET_MAX_PATH_LENGTH         2048
#define INTERNET_MAX_SCHEME_LENGTH       32
#define INTERNET_DEFAULT_HTTP_PORT       80
#define INTERNET_DEFAULT_HTTPS_PORT      443

#define INTERNET_OPEN_TYPE_DIRECT        1
#define INTERNET_FLAG_RELOAD             0x80000000
#define INTERNET_FLAG_SECURE             0x00800000
#define INTERNET_SERVICE_HTTP            3
#define INTERNET_SERVICE_FTP             1
#define HTTP_QUERY_STATUS_CODE           19
#define HTTP_STATUS_OK                   200

__attribute__((unused)) inline HINTERNET InternetOpenA(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD) { return nullptr; }
#define InternetOpen InternetOpenA
__attribute__((unused)) inline HINTERNET InternetConnectA(HINTERNET, LPCSTR, INTERNET_PORT, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR) { return nullptr; }
#define InternetConnect InternetConnectA
__attribute__((unused)) inline HINTERNET HttpOpenRequestA(HINTERNET, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR*, DWORD, DWORD_PTR) { return nullptr; }
#define HttpOpenRequest HttpOpenRequestA
__attribute__((unused)) inline BOOL InternetCloseHandle(HINTERNET) { return 0; }
__attribute__((unused)) inline BOOL InternetReadFile(HINTERNET, LPVOID, DWORD, LPDWORD) { return 0; }
__attribute__((unused)) inline BOOL HttpSendRequestA(HINTERNET, LPCSTR, DWORD, LPVOID, DWORD) { return 0; }
#define HttpSendRequest HttpSendRequestA
__attribute__((unused)) inline BOOL HttpQueryInfoA(HINTERNET, DWORD, LPVOID, LPDWORD, LPDWORD) { return 0; }
#define HttpQueryInfo HttpQueryInfoA
__attribute__((unused)) inline BOOL InternetQueryDataAvailable(HINTERNET, LPDWORD, DWORD, DWORD_PTR) { return 0; }
__attribute__((unused)) inline BOOL InternetSetOptionA(HINTERNET, DWORD, LPVOID, DWORD) { return 0; }
#define InternetSetOption InternetSetOptionA
__attribute__((unused)) inline BOOL InternetCrackUrlA(LPCSTR, DWORD, DWORD, void*) { return 0; }
#define InternetCrackUrl InternetCrackUrlA
