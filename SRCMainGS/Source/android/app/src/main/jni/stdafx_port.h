#pragma once
// ============================================================================
// stdafx_port.h — Android replacement for stdafx.h (precompiled header)
//
// Replaces the original stdafx.h which includes <windows.h>, <gl/glew.h>,
// <gl/gl.h>, and other Windows/desktop-specific headers.
//
// Original Main5.2 source files should replace:
//   #include "stdafx.h"
// with:
//   #include "stdafx_port.h"
// ============================================================================

// ============================================================================
// Platform shims (MUST come first — before any original code)
// ============================================================================
#include "MuPlatform.h"

// ============================================================================
// Standard C/C++ (matching original stdafx.h includes)
// ============================================================================
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdarg>
#include <ctime>
#include <cassert>
#include <string>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <vector>
#include <queue>
#include <thread>
#include <chrono>
#include <random>
#include <fstream>
#include <unordered_map>
#include <mutex>

// ============================================================================
// OpenGL (Android GLES 3.0 replaces desktop GL + GLEW)
// ============================================================================
#include <GLES3/gl3.h>
#include "MuGLCompat.h"

// ============================================================================
// Android logging (replaces OutputDebugString)
// ============================================================================
#include <android/log.h>

// ============================================================================
// Original project includes (portable headers — no Windows dependencies)
// These are included as they're brought into the Android build wave-by-wave.
// ============================================================================

// Wave 1 headers are included directly by the files that need them.
// stdafx_port.h provides only the platform/foundation layer.

// ============================================================================
// Compiler warning suppressions (matching original stdafx.h)
// ============================================================================
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wformat-security"

// ============================================================================
// Core game types — order matches original stdafx.h
// ============================================================================
#include "_define.h"
#include "_enum.h"
#include "_types.h"
#include "_struct.h"

// ============================================================================
// Utility headers (from original stdafx.h, required by game classes)
// ============================================================================

// Abstract handler interface for BuffStateSystem, MapProcess, PetProcess base
#include "w_WindowMessageHandler.h"

// ============================================================================
// Common defines
// ============================================================================

// These are set via CMake but repeated here for safety:
// #define ANDROID
// #define SHADER_VERSION_TEST
// #define MAIN_UPDATE 603
// #define LANG_CHN 1

// Win32-only subsystems are NOT implemented (IMPLEMENT_IMGUI_WIN32 not defined,
// so #ifdef blocks in original code are skipped)

// Stub VM_START/VM_END (anti-debug _asm blocks)
#define VM_START
#define VM_END
#define VM_START2
#define VM_END2

// Note: min/max are provided by `using std::min; using std::max;` in MuPlatform.h
// (included above). This avoids macro conflicts with GLM and other template libraries.
