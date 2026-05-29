// ============================================================================
// MuGlobalState.cpp — Global variable definitions
//
// Only defines globals NOT provided by any included Main5.2 source file.
// Original source files define their own Camera, Screen, Render, Scene globals.
// This file provides:
//   1. Thread-local GL capture state (our compatibility layer)
//   2. Globals from Winmain.cpp (excluded from build)
//   3. Globals from WSclient.cpp (excluded from build)
// ============================================================================

#include "stdafx_port.h"
#include "_define.h"
#include "RenderBatcher.h"

// ============================================================================
// GL Compatibility Adapter State (MuGLCompat.h externs — our code)
// ============================================================================

thread_local std::vector<MuVertex> g_glCaptureVerts;
thread_local GLenum g_glCaptureMode = 0;
thread_local MuVertex g_glCurrentVert;
thread_local GLuint g_glBoundTexture = 0;
thread_local bool g_glCaptureActive = false;
RenderBatcher* g_glBatcher = nullptr;

thread_local std::vector<glm::mat4> g_projectionStack;
thread_local std::vector<glm::mat4> g_modelviewStack;
thread_local int g_currentMatrixMode = 0x1700; // GL_MODELVIEW
thread_local std::vector<glm::mat4>* g_currentStack = nullptr;

// ============================================================================
// Winmain.cpp replacements (Winmain.cpp is excluded from Android build)
// These globals are declared extern in various headers.
// ============================================================================

bool      Destroy = false;
int       m_Resolution = 0;
int       m_nColorDepth = 32;
int       m_SoundOnOff = 1;
int       m_MusicOnOff = 1;
int       Time_Effect = 0;
bool      ashies = false;
int       weather = 0;

// ============================================================================
// WSclient.cpp replacements (WSclient.cpp is excluded from Android build)
// NOTE: HeroKey, CurrentProtocolState, Password, Question, QuestionID,
// First, FirstTime, MoveCount, BuyCost, SendDropItem, SummonLife,
// GuildWarIndex, GuildWarScore, SoccerTime, ChatWhisperID
// are NOW provided by compiling WSclient.cpp directly.
// ============================================================================

// ============================================================================
// Additional globals not in any included source (unique to Android port)
// ============================================================================

bool      g_battleEnable = false;
int       World = 0;
bool      EnableShadow = true;

// ============================================================================
// ZzzInventory.h globals (missing definitions)
// ============================================================================

int       GuildNumber = 0;
int       InventoryType = 0;
bool      EnableRenderInventory = false;

// ============================================================================
// ZzzScene.h globals (missing definitions)
// ============================================================================

bool      InitServerList = false;

// ============================================================================
// ZzzOpenglUtil.h globals (missing definitions)
// ============================================================================

float     g_fFrameEstimate = 0.0f;

// ============================================================================
// Winmain.cpp globals not in WSclient.cpp
// ============================================================================

char      m_Version[16] = {};

// ============================================================================
// Winmain.cpp replacements (Winmain.cpp is excluded from Android build)
// These globals are allocated/initialized in MuGameInit().
// Forward-declared to avoid pulling in UIWindows.h → UIControls.h (Windows deps)
// ============================================================================

class CChatRoomSocketList;
class CUIManager;
class CUIMapName;
class CTimer;

CChatRoomSocketList* g_pChatRoomSocketList = nullptr;
CUIManager* g_pUIManager = nullptr;
CUIMapName* g_pUIMapName = nullptr;
CTimer* g_pTimer = nullptr;

// ============================================================================
// Init helper: called once after EGL context is set up to initialize stack pointers
// ============================================================================

void MuGlobalState_initGL() {
    g_projectionStack.clear();
    g_projectionStack.push_back(glm::mat4(1.0f));
    g_modelviewStack.clear();
    g_modelviewStack.push_back(glm::mat4(1.0f));
    g_currentMatrixMode = 0x1700; // GL_MODELVIEW
    g_currentStack = &g_modelviewStack;

    g_glCaptureVerts.clear();
    g_glCaptureMode = 0;
    g_glCurrentVert = MuVertex();
    g_glBoundTexture = 0;
    g_glCaptureActive = false;
}
