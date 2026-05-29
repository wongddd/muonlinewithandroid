#include "RenderState.h"

#include <android/log.h>
#include <cstring>

#define LOG_TAG "RenderState"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace RenderState {

// Current state
static float g_projectionMatrix[16] = {0};
static float g_modelViewMatrix[16] = {0};
static float g_mvpMatrix[16] = {0};
static bool g_mvpDirty = true;
static bool g_mvDirty = true;

static GLuint g_currentTexture = 0;

static bool g_fogEnabled = false;
static float g_fogStart = 2000.0f;
static float g_fogEnd = 8000.0f;
static float g_fogColor[4] = {0.05f, 0.05f, 0.08f, 1.0f};

static bool g_alphaTestEnabled = true;
static float g_alphaThreshold = 0.1f;

static bool g_blendEnabled = false;
static GLenum g_blendSrc = GL_SRC_ALPHA;
static GLenum g_blendDst = GL_ONE_MINUS_SRC_ALPHA;

static bool g_depthTestEnabled = true;
static bool g_depthWrite = true;

// Cached uniform locations
static GLint g_program = 0;
static GLint g_mvpLoc = -1;
static GLint g_mvLoc = -1;
static GLint g_textureLoc = -1;
static GLint g_fogStartLoc = -1;
static GLint g_fogEndLoc = -1;
static GLint g_fogColorLoc = -1;
static GLint g_alphaTestLoc = -1;
static GLint g_useTextureLoc = -1;

void init() {
    // Identity matrices
    float identity[16] = {
        1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1
    };
    memcpy(g_projectionMatrix, identity, sizeof(g_projectionMatrix));
    memcpy(g_modelViewMatrix, identity, sizeof(g_modelViewMatrix));
    memcpy(g_mvpMatrix, identity, sizeof(g_mvpMatrix));

    // Cache uniform locations
    glGetIntegerv(GL_CURRENT_PROGRAM, &g_program);
    if (g_program) {
        g_mvpLoc = glGetUniformLocation(g_program, "uModelViewProjection");
        g_mvLoc = glGetUniformLocation(g_program, "uModelView");
        g_textureLoc = glGetUniformLocation(g_program, "uTexture");
        g_fogStartLoc = glGetUniformLocation(g_program, "uFogStart");
        g_fogEndLoc = glGetUniformLocation(g_program, "uFogEnd");
        g_fogColorLoc = glGetUniformLocation(g_program, "uFogColor");
        g_alphaTestLoc = glGetUniformLocation(g_program, "uAlphaTest");
        g_useTextureLoc = glGetUniformLocation(g_program, "uUseTexture");
    }

    // Apply defaults to GL
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
}

void setProjectionMatrix(const float* mat4) {
    memcpy(g_projectionMatrix, mat4, 16 * sizeof(float));
    g_mvpDirty = true;
}

void setModelViewMatrix(const float* mat4) {
    memcpy(g_modelViewMatrix, mat4, 16 * sizeof(float));
    g_mvpDirty = true;
    g_mvDirty = true;
}

void updateMVP() {
    if (g_mvpDirty) {
        const float* P = g_projectionMatrix;
        const float* M = g_modelViewMatrix;
        float* mvp = g_mvpMatrix;

        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                mvp[i + j * 4] = P[i + 0 * 4] * M[0 + j * 4]
                               + P[i + 1 * 4] * M[1 + j * 4]
                               + P[i + 2 * 4] * M[2 + j * 4]
                               + P[i + 3 * 4] * M[3 + j * 4];
            }
        }

        // Upload to shader
        if (g_mvpLoc >= 0) {
            glUniformMatrix4fv(g_mvpLoc, 1, GL_FALSE, g_mvpMatrix);
        }
        g_mvpDirty = false;
    }

    if (g_mvDirty && g_mvLoc >= 0) {
        glUniformMatrix4fv(g_mvLoc, 1, GL_FALSE, g_modelViewMatrix);
        g_mvDirty = false;
    }
}

void setTexture(GLuint textureId) {
    if (g_currentTexture == textureId) return;  // Skip redundant rebind (saves GPU state change)
    g_currentTexture = textureId;
    glBindTexture(GL_TEXTURE_2D, textureId);
    if (g_useTextureLoc >= 0) {
        glUniform1i(g_useTextureLoc, textureId != 0 ? 1 : 0);
    }
}

void setFogEnabled(bool enabled) {
    g_fogEnabled = enabled;
    if (g_fogStartLoc >= 0) {
        glUniform1f(g_fogStartLoc, enabled ? g_fogStart : 99999.0f); // huge end = no fog
    }
}

void setFogParams(float start, float end, float r, float g, float b, float a) {
    g_fogStart = start;
    g_fogEnd = end;
    g_fogColor[0] = r;
    g_fogColor[1] = g;
    g_fogColor[2] = b;
    g_fogColor[3] = a;

    if (g_fogStartLoc >= 0) {
        glUniform1f(g_fogStartLoc, start);
    }
    if (g_fogEndLoc >= 0) {
        glUniform1f(g_fogEndLoc, end);
    }
    if (g_fogColorLoc >= 0) {
        glUniform4f(g_fogColorLoc, r, g, b, a);
    }
}

void setAlphaTestEnabled(bool enabled) {
    g_alphaTestEnabled = enabled;
    if (g_alphaTestLoc >= 0) {
        glUniform1f(g_alphaTestLoc, enabled ? g_alphaThreshold : 0.0f);
    }
}

void setAlphaTestThreshold(float threshold) {
    g_alphaThreshold = threshold;
    if (g_alphaTestEnabled && g_alphaTestLoc >= 0) {
        glUniform1f(g_alphaTestLoc, threshold);
    }
}

void setBlendEnabled(bool enabled) {
    g_blendEnabled = enabled;
    if (enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(g_blendSrc, g_blendDst);
    } else {
        glDisable(GL_BLEND);
    }
}

void setBlendFunc(GLenum src, GLenum dst) {
    g_blendSrc = src;
    g_blendDst = dst;
    if (g_blendEnabled) {
        glBlendFunc(src, dst);
    }
}

void setDepthTestEnabled(bool enabled) {
    g_depthTestEnabled = enabled;
    if (enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void setDepthMask(bool write) {
    g_depthWrite = write;
    glDepthMask(write ? GL_TRUE : GL_FALSE);
}

void apply() {
    // Apply all state at once (useful after a shader switch)
    if (g_depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(g_depthWrite ? GL_TRUE : GL_FALSE);

    if (g_blendEnabled) {
        glEnable(GL_BLEND);
        glBlendFunc(g_blendSrc, g_blendDst);
    } else {
        glDisable(GL_BLEND);
    }

    setFogEnabled(g_fogEnabled);
    setAlphaTestEnabled(g_alphaTestEnabled);
    updateMVP();
}

GLuint getProgram() {
    return g_program;
}

} // namespace RenderState
