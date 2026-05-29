// ============================================================================
// wsctlc_stubs.cpp — CWsctlc method stubs for Android
//
// wsctlc.cpp is excluded from build (pure Win32 WSAAsyncSelect).
// Our Android networking uses MuNetwork + ProtocolDispatch instead.
// TranslateProtocol() doesn't use CWsctlc at all — it only processes BYTE* buffers.
// However, WSclient.cpp instantiates a global CWsctlc SocketClient and calls
// ProtocolCompiler (which we bypass). These stubs satisfy the linker.
// ============================================================================

#include "stdafx_port.h"
#include "wsctlc.h"
#include <sys/socket.h>
#include <unistd.h>

CWsctlc::CWsctlc()
    : m_hWnd(nullptr)
    , m_bGame(false)
    , m_iMaxSockets(0)
    , m_nSendBufLen(0)
    , m_nRecvBufLen(0)
    , m_LogPrint(0)
    , m_logfp(nullptr)
    , m_socket(0)
    , m_pPacketQueue(nullptr)
{
    m_SendBuf[0] = '\0';
    m_RecvBuf[0] = '\0';
}

CWsctlc::~CWsctlc() {}

BYTE* CWsctlc::GetReadMsg() { return nullptr; }
SOCKET CWsctlc::GetSocket() { return 0; }

// The following methods are referenced by wsctlc.h virtual table but never called
// in our code path (we bypass ProtocolCompiler). Stubbed as no-ops.
void CWsctlc::LogPrintOn() {}
void CWsctlc::LogPrintOff() {}

BOOL CWsctlc::Startup() { return FALSE; }
int CWsctlc::Create(void*, int) { return 0; }
BOOL CWsctlc::Connect(char*, unsigned short, unsigned int) { return FALSE; }
BOOL CWsctlc::Close() { return FALSE; }
void CWsctlc::LogHexPrintS(unsigned char*, int) {}
int CWsctlc::FDWriteSend() { return 0; }
int CWsctlc::nRecv() { return 0; }

// ============================================================================
// Free functions from wsctlc.cpp — stubbed (not used in our code path)
// ============================================================================

// Socket to use for outgoing packets (set by MuNetwork after connect).
// CWsctlc::sSend() → MySend() → ::send() needs a valid socket.
SOCKET g_stubSendSocket = INVALID_SOCKET;

void DecryptData(BYTE* lpMsg, int size) {
    for (int n = 0; n < size; n++) {
        lpMsg[n] = lpMsg[n] - 1; // dummy
    }
}

void EncryptData(BYTE* lpMsg, int size) {
    for (int n = 0; n < size; n++) {
        lpMsg[n] = lpMsg[n] + 1; // dummy
    }
}

int WINAPI MySend(SOCKET s, char* buf, int len, int flags) {
    SOCKET sock = (g_stubSendSocket != INVALID_SOCKET) ? g_stubSendSocket : s;
    return ::send(sock, buf, len, flags);
}

int WINAPI MyRecvEnc(SOCKET s, char* buf, int len, int flags) {
    return ::recv(s, buf, len, flags);
}
