#pragma once
// ============================================================================
// windows.h — Android/NDK shim
//
// The original Main5.2 source uses #include <windows.h> in several headers
// (Timer.h, etc.). On Android there is no <windows.h>, so we provide one
// that pulls in our MuPlatform.h type shims.
// ============================================================================

#include "MuPlatform.h"
