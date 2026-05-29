#pragma once
// Shim for <conio.h> on Android
// Console I/O functions (_getch, _kbhit, etc.) are not available
// These are only used in debug/console code paths, not in rendering
#include "MuPlatform.h"
