#pragma once
// Shim for <tchar.h> on Android
// TCHAR macros already defined in MuPlatform.h
// On Android we use ANSI (char), not wide char
#include "MuPlatform.h"
