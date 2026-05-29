#pragma once
// Shim for <mbstring.h> (MSVC multi-byte string functions) on Android
// Most functions map to standard C string functions
#include <cstring>
#include <cstdlib>
#include "MuPlatform.h"
