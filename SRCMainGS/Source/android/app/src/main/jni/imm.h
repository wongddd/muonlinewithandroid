#pragma once
// Shim for <imm.h> (MSVC Input Method Manager) on Android/Clang
// IME is not supported via Win32 API on Android
// The needed IME function stubs are in MuPlatform.h
#include "MuPlatform.h"
