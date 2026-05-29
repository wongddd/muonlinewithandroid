#pragma once
// Shim for <gl/glu.h> on Android
// The desktop GLU library is not available — our MuGLCompat.h provides inline stubs
// for gluPerspective, gluLookAt, etc.
#include "MuGLCompat.h"
