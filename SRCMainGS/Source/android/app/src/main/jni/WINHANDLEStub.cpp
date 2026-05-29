// ============================================================================
// WINHANDLEStub.cpp — Implement CWINHANDLE methods for Android
//
// WINHANDLE.cpp is excluded from build (pure Win32). Implements the class
// methods declared in WINHANDLE.h. We include the original header directly
// since stdafx_port.h does not pull it in.
// ============================================================================

#include "stdafx_port.h"
#include "WINHANDLE.h"

CWINHANDLE::CWINHANDLE()
    : hWnd(nullptr)
    , hInstance(nullptr)
    , wndIndex(0)
    , WndMode(false)
    , WndActive(true)
    , WndIconic(false)
    , iWinWidth(1920.0f)
    , iWinHight(1080.0f)
{
}

CWINHANDLE::~CWINHANDLE() {}

void CWINHANDLE::Release() {}

HWND CWINHANDLE::Create(HINSTANCE hCurrentInst, mu_uint32, mu_uint32) {
    hInstance = hCurrentInst;
    hWnd = (HWND)0x1;
    return hWnd;
}

void CWINHANDLE::Destroyer() {}

void CWINHANDLE::InitSize(mu_uint32 RenderSizeX, mu_uint32 RenderSizeY) {
    iWinWidth = (mu_float)RenderSizeX;
    iWinHight = (mu_float)RenderSizeY;
}

void CWINHANDLE::SetFontSize(mu_uint32) {}

void CWINHANDLE::Resize(mu_uint32, mu_uint32) {}

HWND CWINHANDLE::GethWnd() { return hWnd ? hWnd : (HWND)0x1; }
void CWINHANDLE::SetInstance(HINSTANCE hInst) { hInstance = hInst; }
HINSTANCE CWINHANDLE::GetInstance() { return hInstance; }
void CWINHANDLE::SetWndActive(mu_boolean bActive) { WndActive = bActive; }
mu_boolean CWINHANDLE::CheckWndActive() { return true; }
void CWINHANDLE::UpdateWndActive() {}
void CWINHANDLE::SetWndMode(mu_boolean bActive) { WndMode = bActive; }
mu_boolean CWINHANDLE::CheckWndMode() { return false; }
void CWINHANDLE::SetDisplayIndex(mu_uint8 index, mu_boolean) { wndIndex = index; }
mu_uint8 CWINHANDLE::GetDisplayIndex() { return wndIndex; }
mu_uint8 CWINHANDLE::GetDisplayIndex(const std::string) { return 0; }
mu_float CWINHANDLE::GetScreenX() { return iWinWidth; }
mu_float CWINHANDLE::GetScreenY() { return iWinHight; }
ResolutionConfig* CWINHANDLE::LoadCurrentConfig() { return nullptr; }
void CWINHANDLE::Check_State() {}
void CWINHANDLE::Change_State(mu_boolean) {}
mu_boolean CWINHANDLE::CheckWndIconic() { return false; }
mu_boolean CWINHANDLE::CheckPerformance() { return true; }
void CWINHANDLE::SendWindowMessage(const char*, bool, ...) {}
void CWINHANDLE::SendNowMessage(UINT, WPARAM, LPARAM) {}
void CWINHANDLE::SendPostMessage(UINT, WPARAM, LPARAM) {}

CWINHANDLE* CWINHANDLE::Instance() {
    static CWINHANDLE inst;
    return &inst;
}

MSG CWINHANDLE::winLoop() { MSG m = {}; return m; }

LONG CWINHANDLE::WndProc(HWND, UINT, WPARAM, LPARAM) { return 0; }

// Global variables declared in WINHANDLE.h
int g_iInactiveWarning = 0;
BOOL g_bMinimizedEnabled = FALSE;
float g_iMousePopPosition_x = 0.0f;
float g_iMousePopPosition_y = 0.0f;
// g_pChatRoomSocketList moved to MuGlobalState.cpp (single definition)
