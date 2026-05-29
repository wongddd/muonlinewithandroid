#pragma once
// Shim for <GL/gl.h> on Android
// Routes to GLES 3.0 + MuGLCompat for legacy immediate-mode compatibility
#include <GLES3/gl3.h>
#include "MuGLCompat.h"

// Desktop GL types not present in GLES
typedef double GLdouble;
