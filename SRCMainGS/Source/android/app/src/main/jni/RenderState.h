#pragma once

#include <GLES3/gl3.h>

// Manages shader uniforms and render state, replacing the fixed-function pipeline.
//
// Original → Replacement:
//   glMatrixMode(GL_PROJECTION)    → setProjectionMatrix(mat4)
//   glMatrixMode(GL_MODELVIEW)     → setModelViewMatrix(mat4)
//   glRotatef/glTranslatef         → (compose on CPU, upload via setModelViewMatrix)
//   gluPerspective                  → setProjectionMatrix(glm::perspective(...))
//   glColor3f/4f                   → vertex color attribute
//   glBindTexture                   → setTexture(id)
//   glEnable(GL_FOG)               → setFog(true, start, end, color)
//   glEnable(GL_ALPHA_TEST)        → setAlphaTest(true, threshold)
//   glEnable(GL_BLEND)             → setBlendMode(src, dst)
//   glDepthMask                     → setDepthMask(bool)
//   glEnable(GL_DEPTH_TEST)        → setDepthTest(bool)

namespace RenderState {

// Initialize state defaults
void init();

// Matrix stack emulation (single level — the original uses Push/Pop but we
// track the current matrix and upload it each frame)
void setProjectionMatrix(const float* mat4);
void setModelViewMatrix(const float* mat4);
void updateMVP(); // recomputes MVP from current PM + MVM

// Texture
void setTexture(GLuint textureId);

// Fog
void setFogEnabled(bool enabled);
void setFogParams(float start, float end, float r, float g, float b, float a);

// Alpha test
void setAlphaTestEnabled(bool enabled);
void setAlphaTestThreshold(float threshold);

// Blend
void setBlendEnabled(bool enabled);
void setBlendFunc(GLenum src, GLenum dst);

// Depth
void setDepthTestEnabled(bool enabled);
void setDepthMask(bool write);

// Apply all current state to GL (call once per frame or per state change)
void apply();

// Get the current shader program
GLuint getProgram();

} // namespace RenderState
