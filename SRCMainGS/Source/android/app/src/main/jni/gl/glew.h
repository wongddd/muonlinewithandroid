#pragma once
// Shim for <GL/glew.h> on Android
// GLEW is not needed — GLES 3.0 provides all required entry points natively
// Core GL functions come from <GLES3/gl3.h> (already in stdafx_port.h)
#include <GLES3/gl3.h>

// Stub GLEW types/macros that original code may reference
#define GLenum unsigned int
#define GLboolean unsigned char

// GLEW function pointers — stub (not used with GLES 3.0 native entry points)
#define glewInit() (0)
#define glewIsSupported(x) (1)
#define GLEW_OK 0

// GLEW extension IDs (used in #ifdef blocks)
#define GLEW_EXT_texture_filter_anisotropic 0

// GL extension constants (anisotropic filtering)
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT  0x84FF
#define GL_TEXTURE_MAX_ANISOTROPY_EXT      0x84FE
