#include "MuGLContext.h"

#include <android/log.h>
#include <android/native_window.h>

#define LOG_TAG "MuGLContext"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace MuGLContext {

EGLDisplay g_display = EGL_NO_DISPLAY;
EGLSurface g_surface = EGL_NO_SURFACE;
EGLContext g_context = EGL_NO_CONTEXT;
int g_width = 0;
int g_height = 0;

bool init(ANativeWindow* window) {
    g_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (g_display == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed");
        return false;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(g_display, &major, &minor)) {
        LOGE("eglInitialize failed");
        return false;
    }
    LOGI("EGL version: %d.%d", major, minor);

    // Try progressively simpler configs for broad device/emulator compatibility
    const EGLint attribs0[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,  8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16, EGL_STENCIL_SIZE, 8,
        EGL_SAMPLE_BUFFERS, 1, EGL_SAMPLES, 4,
        EGL_NONE
    };
    const EGLint attribs1[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,  8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16, EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    const EGLint attribs2[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,  8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };
    const EGLint attribs3[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,  8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    const EGLint attribs4[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,  5, EGL_GREEN_SIZE, 6, EGL_BLUE_SIZE, 5, EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };
    const EGLint attribs5[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,  8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };

    struct { const char* name; const EGLint* ptr; } configs[] = {
        {"RGBA8888+D24S8+MSAA4", attribs0},
        {"RGBA8888+D24S8",       attribs1},
        {"RGBA8888+D16",         attribs2},
        {"RGBA8888",             attribs3},
        {"RGB565+D16",           attribs4},
        {"GLES2+RGBA8888+D16",   attribs5},
    };

    EGLConfig config = nullptr;
    EGLint numConfigs = 0;
    bool configFound = false;
    int usedGlesVersion = 3;

    for (int i = 0; i < 6; ++i) {
        if (eglChooseConfig(g_display, configs[i].ptr, &config, 1, &numConfigs) && numConfigs > 0) {
            LOGI("EGL config: %s", configs[i].name);
            configFound = true;
            if (i == 5) usedGlesVersion = 2;
            break;
        }
    }
    if (!configFound) {
        LOGE("eglChooseConfig failed - no compatible config found");
        return false;
    }

    EGLint renderableType = 0;
    eglGetConfigAttrib(g_display, config, EGL_RENDERABLE_TYPE, &renderableType);
    LOGI("EGL renderable: ES3=%d ES2=%d",
         (renderableType & EGL_OPENGL_ES3_BIT) ? 1 : 0,
         (renderableType & EGL_OPENGL_ES2_BIT) ? 1 : 0);

    g_surface = eglCreateWindowSurface(g_display, config, window, nullptr);
    if (g_surface == EGL_NO_SURFACE) {
        LOGE("eglCreateWindowSurface failed");
        return false;
    }

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, usedGlesVersion,
        EGL_NONE
    };
    g_context = eglCreateContext(g_display, config, EGL_NO_CONTEXT, contextAttribs);
    if (g_context == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed (GLES %d)", usedGlesVersion);
        eglDestroySurface(g_display, g_surface);
        g_surface = EGL_NO_SURFACE;
        return false;
    }

    if (!eglMakeCurrent(g_display, g_surface, g_surface, g_context)) {
        LOGE("eglMakeCurrent failed");
        eglDestroyContext(g_display, g_context);
        g_context = EGL_NO_CONTEXT;
        eglDestroySurface(g_display, g_surface);
        g_surface = EGL_NO_SURFACE;
        return false;
    }

    LOGI("EGL context created (GLES %d)", usedGlesVersion);
    LOGI("GL Renderer: %s", glGetString(GL_RENDERER));
    LOGI("GL Version:  %s", glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    return true;
}

void destroy() {
    if (g_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (g_context != EGL_NO_CONTEXT) {
            eglDestroyContext(g_display, g_context);
            g_context = EGL_NO_CONTEXT;
        }

        if (g_surface != EGL_NO_SURFACE) {
            eglDestroySurface(g_display, g_surface);
            g_surface = EGL_NO_SURFACE;
        }

        eglTerminate(g_display);
        g_display = EGL_NO_DISPLAY;
    }
    LOGI("EGL context destroyed");
}

void onSurfaceChanged(int width, int height) {
    g_width = width;
    g_height = height;
    if (g_context != EGL_NO_CONTEXT) {
        glViewport(0, 0, width, height);
    }
    LOGI("Surface changed: %dx%d (ctx=%s)", width, height,
         g_context != EGL_NO_CONTEXT ? "valid" : "none");
}

bool makeCurrent() {
    if (g_display == EGL_NO_DISPLAY || g_surface == EGL_NO_SURFACE || g_context == EGL_NO_CONTEXT) {
        return false;
    }
    return eglMakeCurrent(g_display, g_surface, g_surface, g_context) == EGL_TRUE;
}

void swapBuffers() {
    if (g_display != EGL_NO_DISPLAY && g_surface != EGL_NO_SURFACE) {
        eglSwapBuffers(g_display, g_surface);
    }
    static int swpCount = 0;
    if (++swpCount % 180 == 0) {
        LOGI("swapBuffers called %d times", swpCount);
    }
}

} // namespace MuGLContext
