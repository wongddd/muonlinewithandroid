#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace MuGLContext {

bool init(ANativeWindow* window);
void destroy();
void onSurfaceChanged(int width, int height);
bool makeCurrent();
void swapBuffers();

extern EGLDisplay g_display;
extern EGLSurface g_surface;
extern EGLContext g_context;
extern int g_width;
extern int g_height;

} // namespace MuGLContext
