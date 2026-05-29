#pragma once
// ============================================================================
// MuGLCompat.h — Desktop OpenGL → GLES 3.0 compatibility layer
//
// Provides:
//   1. GL constant definitions for removed desktop GL enums
//   2. glBegin/glEnd capture adapter → RenderBatcher::drawImmediate()
//   3. Matrix stack (glRotatef/glTranslatef/glMatrixMode/glPushMatrix/glPopMatrix)
//   4. Fixed-function state wrappers → RenderState
//   5. gluPerspective → glm::perspective
//
// Include this AFTER <GLES3/gl3.h> and BEFORE any original rendering code.
// ============================================================================

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "MuRendererTypes.h"
#include "RenderState.h"
#include "RenderBatcher.h"

// ============================================================================
// GL constants absent from GLES 3.0
// ============================================================================

// Primitive types
#ifndef GL_QUADS
#define GL_QUADS            0x0007
#endif
#ifndef GL_QUAD_STRIP
#define GL_QUAD_STRIP       0x0008
#endif
#ifndef GL_POLYGON
#define GL_POLYGON          0x0009
#endif

// Matrix modes
#ifndef GL_PROJECTION
#define GL_PROJECTION       0x1701
#endif
#ifndef GL_MODELVIEW
#define GL_MODELVIEW        0x1700
#endif
#ifndef GL_TEXTURE
#define GL_TEXTURE          0x1702
#endif
#ifndef GL_COLOR
#define GL_COLOR            0x1800
#endif

// Matrix query constants (not in GLES)
#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX  0x0BA6
#endif
#ifndef GL_PROJECTION_MATRIX
#define GL_PROJECTION_MATRIX 0x0BA7
#endif

// Fixed-function state (not in GLES core, we map to RenderState)
#ifndef GL_FOG
#define GL_FOG              0x0B60
#endif
#ifndef GL_FOG_MODE
#define GL_FOG_MODE         0x0B65
#endif
#ifndef GL_FOG_DENSITY
#define GL_FOG_DENSITY      0x0B62
#endif
#ifndef GL_FOG_START
#define GL_FOG_START        0x0B63
#endif
#ifndef GL_FOG_END
#define GL_FOG_END          0x0B64
#endif
#ifndef GL_FOG_COLOR
#define GL_FOG_COLOR        0x0B66
#endif
#ifndef GL_FOG_HINT
#define GL_FOG_HINT         0x0C54
#endif
#ifndef GL_LINEAR
#define GL_LINEAR           0x2601
#endif

#ifndef GL_ALPHA_TEST
#define GL_ALPHA_TEST       0x0BC0
#endif

// Texture environment (GL_MODULATE → shader handles this)
#ifndef GL_TEXTURE_ENV
#define GL_TEXTURE_ENV       0x2300
#endif
#ifndef GL_TEXTURE_ENV_MODE
#define GL_TEXTURE_ENV_MODE  0x2200
#endif
#ifndef GL_MODULATE
#define GL_MODULATE          0x2100
#endif
#ifndef GL_DECAL
#define GL_DECAL             0x2101
#endif
#ifndef GL_REPLACE
#define GL_REPLACE           0x1E01
#endif

// Texture coord generation
#ifndef GL_TEXTURE_GEN_S
#define GL_TEXTURE_GEN_S     0x0C60
#endif
#ifndef GL_TEXTURE_GEN_T
#define GL_TEXTURE_GEN_T     0x0C61
#endif

// Light model
#ifndef GL_LIGHTING
#define GL_LIGHTING          0x0B50
#endif
#ifndef GL_LIGHT_MODEL_AMBIENT
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#endif

// Blend modes
#ifndef GL_ADD
#define GL_ADD                0x0104
#endif

// Shading model
#ifndef GL_SMOOTH
#define GL_SMOOTH            0x1D01
#endif
#ifndef GL_FLAT
#define GL_FLAT              0x1D00
#endif

// Color material
#ifndef GL_COLOR_MATERIAL
#define GL_COLOR_MATERIAL    0x0B57
#endif

// Polygon mode (not in GLES)
#ifndef GL_LINE
#define GL_LINE               0x1B01
#endif
#ifndef GL_FILL
#define GL_FILL               0x1B02
#endif

// Normalization
#ifndef GL_NORMALIZE
#define GL_NORMALIZE         0x0BA1
#endif

// Line smooth (not in GLES 3.0)
#ifndef GL_LINE_SMOOTH
#define GL_LINE_SMOOTH        0x0B20
#endif

// Hint targets
#ifndef GL_PERSPECTIVE_CORRECTION_HINT
#define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#endif
#ifndef GL_POINT_SMOOTH_HINT
#define GL_POINT_SMOOTH_HINT  0x0C51
#endif
#ifndef GL_LINE_SMOOTH_HINT
#define GL_LINE_SMOOTH_HINT   0x0C52
#endif

// Current color query
#ifndef GL_CURRENT_COLOR
#define GL_CURRENT_COLOR     0x0B00
#endif

// Clip planes
#ifndef GL_CLIP_PLANE0
#define GL_CLIP_PLANE0       0x3000
#endif

// Light model color control
#ifndef GL_SEPARATE_SPECULAR_COLOR
#define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#endif

// Client state (for glEnableClientState — removed in GLES)
#ifndef GL_VERTEX_ARRAY
#define GL_VERTEX_ARRAY      0x8074
#endif
#ifndef GL_COLOR_ARRAY
#define GL_COLOR_ARRAY       0x8076
#endif
#ifndef GL_TEXTURE_COORD_ARRAY
#define GL_TEXTURE_COORD_ARRAY 0x8078
#endif
#ifndef GL_NORMAL_ARRAY
#define GL_NORMAL_ARRAY      0x8075
#endif

// glReadBuffer mode
#ifndef GL_BACK
#define GL_BACK              0x0405
#endif
#ifndef GL_FRONT
#define GL_FRONT             0x0404
#endif

// ============================================================================
// Global capture state (defined in MuGlobalState.cpp)
// ============================================================================

extern thread_local std::vector<MuVertex> g_glCaptureVerts;
extern thread_local GLenum g_glCaptureMode;
extern thread_local MuVertex g_glCurrentVert;
extern thread_local GLuint g_glBoundTexture;
extern thread_local bool g_glCaptureActive;
extern RenderBatcher* g_glBatcher;

// Matrix stacks (max depth 32, matching original OpenGL spec)
extern thread_local std::vector<glm::mat4> g_projectionStack;
extern thread_local std::vector<glm::mat4> g_modelviewStack;
extern thread_local int g_currentMatrixMode;
extern thread_local std::vector<glm::mat4>* g_currentStack;

// ============================================================================
// Immediate-mode capture wrappers
// ============================================================================

inline void glBegin(GLenum mode) {
    g_glCaptureMode = mode;
    g_glCaptureVerts.clear();
    g_glCaptureActive = true;
}

inline void glEnd() {
    if (!g_glCaptureActive || g_glCaptureVerts.empty()) {
        g_glCaptureActive = false;
        return;
    }
    g_glCaptureActive = false;

    if (g_glBatcher) {
        g_glBatcher->drawImmediate(g_glCaptureMode,
                                    g_glCaptureVerts.data(),
                                    g_glCaptureVerts.size(),
                                    g_glBoundTexture);
    }
    g_glCaptureVerts.clear();
}

inline void glColor4f(float r, float g, float b, float a) {
    g_glCurrentVert.r = r;
    g_glCurrentVert.g = g;
    g_glCurrentVert.b = b;
    g_glCurrentVert.a = a;
}

inline void glColor4fv(const float* v) {
    g_glCurrentVert.r = v[0];
    g_glCurrentVert.g = v[1];
    g_glCurrentVert.b = v[2];
    g_glCurrentVert.a = v[3];
}

inline void glColor3f(float r, float g, float b) {
    g_glCurrentVert.r = r;
    g_glCurrentVert.g = g;
    g_glCurrentVert.b = b;
    g_glCurrentVert.a = 1.0f;
}

inline void glColor3fv(const float* v) {
    g_glCurrentVert.r = v[0];
    g_glCurrentVert.g = v[1];
    g_glCurrentVert.b = v[2];
    g_glCurrentVert.a = 1.0f;
}

inline void glColor3ub(GLubyte r, GLubyte g, GLubyte b) {
    g_glCurrentVert.r = r / 255.0f;
    g_glCurrentVert.g = g / 255.0f;
    g_glCurrentVert.b = b / 255.0f;
    g_glCurrentVert.a = 1.0f;
}

inline void glColor4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a) {
    g_glCurrentVert.r = r / 255.0f;
    g_glCurrentVert.g = g / 255.0f;
    g_glCurrentVert.b = b / 255.0f;
    g_glCurrentVert.a = a / 255.0f;
}

inline void glTexCoord2f(float u, float v) {
    g_glCurrentVert.u = u;
    g_glCurrentVert.v = v;
}

inline void glTexCoord2fv(const float* v) {
    g_glCurrentVert.u = v[0];
    g_glCurrentVert.v = v[1];
}

inline void glVertex3f(float x, float y, float z) {
    g_glCurrentVert.x = x;
    g_glCurrentVert.y = y;
    g_glCurrentVert.z = z;
    g_glCaptureVerts.push_back(g_glCurrentVert);
}

inline void glVertex3fv(const float* v) {
    g_glCurrentVert.x = v[0];
    g_glCurrentVert.y = v[1];
    g_glCurrentVert.z = v[2];
    g_glCaptureVerts.push_back(g_glCurrentVert);
}

inline void glVertex2f(float x, float y) {
    glVertex3f(x, y, 0.0f);
}

inline void glVertex2i(int x, int y) {
    glVertex3f(static_cast<float>(x), static_cast<float>(y), 0.0f);
}

inline void glNormal3f(float nx, float ny, float nz) {
    // Normals aren't in MuVertex; we approximate by encoding into color
    // (lighting is pre-computed in the original pipeline)
    (void)nx; (void)ny; (void)nz;
}

inline void glNormal3fv(const float* v) {
    (void)v;
}

inline void glMultiTexCoord2f(GLenum, float u, float v) {
    g_glCurrentVert.u = u;
    g_glCurrentVert.v = v;
}

// ============================================================================
// Matrix stack wrappers (CPU-side using glm::mat4)
// ============================================================================

inline void glMatrixMode(GLenum mode) {
    g_currentMatrixMode = mode;
    g_currentStack = (mode == GL_PROJECTION) ? &g_projectionStack : &g_modelviewStack;
}

inline void glLoadIdentity() {
    if (g_currentStack && !g_currentStack->empty()) {
        g_currentStack->back() = glm::mat4(1.0f);
    }
}

inline void glPushMatrix() {
    if (g_currentStack) {
        if (g_currentStack->empty()) {
            g_currentStack->push_back(glm::mat4(1.0f));
        } else {
            g_currentStack->push_back(g_currentStack->back());
        }
    }
}

inline void glPopMatrix() {
    if (g_currentStack && g_currentStack->size() > 1) {
        g_currentStack->pop_back();
    }
}

inline void glRotatef(float angle, float x, float y, float z) {
    if (g_currentStack && !g_currentStack->empty()) {
        g_currentStack->back() = glm::rotate(g_currentStack->back(),
                                              glm::radians(angle),
                                              glm::vec3(x, y, z));
    }
}

inline void glTranslatef(float x, float y, float z) {
    if (g_currentStack && !g_currentStack->empty()) {
        g_currentStack->back() = glm::translate(g_currentStack->back(),
                                                 glm::vec3(x, y, z));
    }
}

inline void glScalef(float x, float y, float z) {
    if (g_currentStack && !g_currentStack->empty()) {
        g_currentStack->back() = glm::scale(g_currentStack->back(),
                                             glm::vec3(x, y, z));
    }
}

// Get the current top-of-stack matrix (used by BeginOpengl to upload to RenderState)
inline const float* glGetCurrentMatrix() {
    static float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    if (g_currentStack && !g_currentStack->empty()) {
        return glm::value_ptr(g_currentStack->back());
    }
    return identity;
}

// ============================================================================
// gluOrtho2D
// ============================================================================

inline void gluOrtho2D(float left, float right, float bottom, float top) {
    // Maps to a 2D orthographic projection using glm
    // Near/far default to -1/1 (standard GL convention)
    glm::mat4 proj = glm::ortho(left, right, bottom, top);
    if (!g_projectionStack.empty()) {
        g_projectionStack.back() = proj;
    } else {
        g_projectionStack.push_back(proj);
    }
    RenderState::setProjectionMatrix(glm::value_ptr(proj));
}

// ============================================================================
// gluPerspective → glm::perspective
// ============================================================================

inline void gluPerspective(float fovy, float aspect, float zNear, float zFar) {
    glm::mat4 proj = glm::perspective(glm::radians(fovy), aspect, zNear, zFar);
    if (!g_projectionStack.empty()) {
        g_projectionStack.back() = proj;
    } else {
        g_projectionStack.push_back(proj);
    }
    RenderState::setProjectionMatrix(glm::value_ptr(proj));
}

inline void gluLookAt(float eyeX, float eyeY, float eyeZ,
                       float centerX, float centerY, float centerZ,
                       float upX, float upY, float upZ) {
    glm::mat4 view = glm::lookAt(glm::vec3(eyeX, eyeY, eyeZ),
                                  glm::vec3(centerX, centerY, centerZ),
                                  glm::vec3(upX, upY, upZ));
    if (!g_modelviewStack.empty()) {
        g_modelviewStack.back() = view;
    } else {
        g_modelviewStack.push_back(view);
    }
    RenderState::setModelViewMatrix(glm::value_ptr(view));
}

// ============================================================================
// Fixed-function state wrappers → RenderState
// ============================================================================

// Override glEnable/glDisable to intercept desktop-only GL enums that do not
// exist in GLES.  Valid GLES enums (GL_BLEND, GL_DEPTH_TEST, etc.) pass through.
// The original game code calls glEnable(GL_TEXTURE_2D), glEnable(GL_FOG),
// glEnable(GL_ALPHA_TEST), etc. — all of which are GL_INVALID_ENUM in GLES.

inline void mu_glEnable(GLenum cap) {
    switch (cap) {
        case 0x0DE1: return;               // GL_TEXTURE_2D — always on in GLES
        case 0x0B60: RenderState::setFogEnabled(true); return;       // GL_FOG
        case 0x0BC0: RenderState::setAlphaTestEnabled(true); return; // GL_ALPHA_TEST
        case 0x0B50: return;               // GL_LIGHTING
        case 0x0BA1: return;               // GL_NORMALIZE
        case 0x0B20: return;               // GL_LINE_SMOOTH
        case 0x0B57: return;               // GL_COLOR_MATERIAL
        case 0x0C60: return;               // GL_TEXTURE_GEN_S
        case 0x0C61: return;               // GL_TEXTURE_GEN_T
        case 0x0B10: return;               // GL_POINT_SMOOTH
        default: break;
    }
    glEnable(cap);
}

inline void mu_glDisable(GLenum cap) {
    switch (cap) {
        case 0x0DE1: return;               // GL_TEXTURE_2D
        case 0x0B60: RenderState::setFogEnabled(false); return;      // GL_FOG
        case 0x0BC0: RenderState::setAlphaTestEnabled(false); return;// GL_ALPHA_TEST
        case 0x0B50: return;               // GL_LIGHTING
        case 0x0BA1: return;               // GL_NORMALIZE
        case 0x0B20: return;               // GL_LINE_SMOOTH
        case 0x0B57: return;               // GL_COLOR_MATERIAL
        case 0x0C60: return;               // GL_TEXTURE_GEN_S
        case 0x0C61: return;               // GL_TEXTURE_GEN_T
        case 0x0B10: return;               // GL_POINT_SMOOTH
        default: break;
    }
    glDisable(cap);
}

#define glEnable  mu_glEnable
#define glDisable mu_glDisable

inline void muEnableFog()       { RenderState::setFogEnabled(true); }
inline void muDisableFog()      { RenderState::setFogEnabled(false); }
inline void muEnableAlphaTest() { RenderState::setAlphaTestEnabled(true); }
inline void muDisableAlphaTest(){ RenderState::setAlphaTestEnabled(false); }

inline void glFogf(GLenum pname, float param) {
    switch (pname) {
    case GL_FOG_START:  RenderState::setFogParams(param, -1, -1,-1,-1,-1); break;
    case GL_FOG_END:    RenderState::setFogParams(-1, param, -1,-1,-1,-1); break;
    case GL_FOG_DENSITY: break; // not used in linear mode
    default: break;
    }
}

inline void glFogfv(GLenum pname, const float* params) {
    switch (pname) {
    case GL_FOG_COLOR:
        RenderState::setFogParams(-1, -1, params[0], params[1], params[2], params[3]);
        break;
    default: break;
    }
}

inline void glAlphaFunc(GLenum, float ref) {
    RenderState::setAlphaTestThreshold(ref);
}

// glTexEnvf — no-op (shader handles texture modulation)
inline void glTexEnvf(GLenum, GLenum, float) {}
inline void glTexEnvi(GLenum, GLenum, int) {}

// glPolygonMode — no-op (not available in GLES)
inline void glPolygonMode(GLenum, GLenum) {}

// glShadeModel — no-op
inline void glShadeModel(GLenum) {}

// glHint — no-op
inline void glHint(GLenum, GLenum) {}

// glColorMaterial — no-op
inline void glColorMaterial(GLenum, GLenum) {}

// glGetFloatv — GL_PROJECTION_MATRIX and GL_MODELVIEW_MATRIX
// (used by original code to read back matrices)
inline void glGetFloatv(GLenum pname, float* params) {
    if (pname == 0x0BA7) { // GL_PROJECTION_MATRIX
        if (!g_projectionStack.empty()) {
            memcpy(params, glm::value_ptr(g_projectionStack.back()), 16 * sizeof(float));
        }
    } else if (pname == 0x0BA6) { // GL_MODELVIEW_MATRIX
        if (!g_modelviewStack.empty()) {
            memcpy(params, glm::value_ptr(g_modelviewStack.back()), 16 * sizeof(float));
        }
    }
}

// ============================================================================
// GL state query wrappers (used by original code)
// ============================================================================

// glGetIntegerv with GL_MAX_TEXTURE_SIZE → GL_MAX_TEXTURE_SIZE (exists in GLES)
// glGetIntegerv with other enums → passthrough

// ============================================================================
// Texture bind tracking (for capture adapter)
// ============================================================================

// NOTE: Do NOT override glBindTexture (it exists in GLES and handles the actual binding).
// Instead, use muBindTexture to track the current texture for the capture adapter.

inline void muBindTexture(GLenum target, GLuint texture) {
    g_glBoundTexture = texture;
    glBindTexture(target, texture);
}

// ============================================================================
// Client-state wrappers (glEnableClientState doesn't exist in GLES)
// ============================================================================

inline void glEnableClientState(GLenum) {}
inline void glDisableClientState(GLenum) {}

// glVertexPointer / glTexCoordPointer / glColorPointer / glNormalPointer
// These are used in the original code's non-shader BMD path and terrain grass path.
// In our port, BMD uses VAO (SHADER_VERSION_TEST), and terrain uses TerrainBatcher.
// So these can be no-ops initially.

inline void glVertexPointer(int, GLenum, int, const void*) {}
inline void glTexCoordPointer(int, GLenum, int, const void*) {}
inline void glColorPointer(int, GLenum, int, const void*) {}
inline void glNormalPointer(GLenum, int, const void*) {}
inline void glDrawArrays_Compat(GLenum mode, int first, int count) {
    // Used by original code's non-shader path. In our port, rendering goes through
    // the capture adapter (glBegin/glEnd) or VAO path (SHADER_VERSION_TEST).
    (void)mode; (void)first; (void)count;
}

// glViewport2 is defined in original code (ZzzOpenglUtil.cpp:611).
// Our MuGLCompat.h must NOT redefine it.

// ============================================================================
// OpenGL error check helper
// ============================================================================

inline const char* muGLErrorString(GLenum err) {
    switch (err) {
    case GL_NO_ERROR:          return "GL_NO_ERROR";
    case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
    default:                   return "UNKNOWN";
    }
}

inline void muCheckGLError(const char* tag) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        // Log via android log (callers must have LOG_TAG defined)
        // __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s: GL error %s (0x%X)", tag, muGLErrorString(err), err);
    }
}

