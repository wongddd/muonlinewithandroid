#pragma once

#include <GLES3/gl3.h>

#include "MuRendererTypes.h"
#include "RenderBatcher.h"

// Replaces all RenderBitmap variants in ZzzOpenglUtil.cpp with
// VBO-based rendering on GLES 3.0.
//
// The original code has 15+ overloads handling:
//   - Basic 2D bitmaps (position, size, UV rect, alpha, scaling flags)
//   - Rotated bitmaps (around center or arbitrary point)
//   - Locally-rotated bitmaps (with cos/sin angle matrices)
//   - Alpha-edge bitmaps (4x4 sub-divided grid with edge fading)
//   - Projected 3D bitmaps (Euler-angle rotation in world space)
//   - UV-adjusted bitmaps (25% bottom crop)
//
// All of these emit 4-vertex triangle fans. SpriteRenderer bakes
// them into a shared VBO and flushes periodically.

class SpriteRenderer {
public:
    SpriteRenderer();
    ~SpriteRenderer();

    void initialize(RenderBatcher* batcher);
    void destroy();

    // --- Core 2D bitmap (replaces RenderBitmap) ---
    // Draws a textured quad at (x,y) with given size in ortho projection.
    // sx/sy: scaling flags (1.0 = normal, -1.0 = flip)
    // u0,v0,u1,v1: texture UV rect
    // alpha: overall opacity (0.0 - 1.0)
    void drawBitmap(float x, float y, float width, float height,
                    float u0, float v0, float u1, float v1,
                    GLuint textureId, float alpha = 1.0f,
                    float sx = 1.0f, float sy = 1.0f);

    // --- Color-tinted bitmap (replaces RenderColorBitmap) ---
    void drawColorBitmap(float x, float y, float width, float height,
                         float u0, float v0, float u1, float v1,
                         GLuint textureId,
                         float r, float g, float b, float alpha);

    // --- Rotated bitmap (replaces RenderBitmapRotate) ---
    // Rotation angle in degrees, around the bitmap center
    void drawBitmapRotate(float x, float y, float width, float height,
                          float u0, float v0, float u1, float v1,
                          GLuint textureId, float angle, float alpha = 1.0f);

    // --- Alpha-edge bitmap (replaces RenderBitmapAlpha) ---
    // 4x4 sub-divided grid with smooth edge fading
    void drawBitmapAlpha(float x, float y, float width, float height,
                         float u0, float v0, float u1, float v1,
                         GLuint textureId, float alpha, float edgeSize);

    // --- Simple colored quad without texture (replaces RenderColor/RenderNoColor) ---
    void drawColorQuad(float x, float y, float width, float height,
                       float r, float g, float b, float alpha);

    // Flush any pending batched draw calls
    void flush();

private:
    RenderBatcher* m_batcher; // not owned

    // Helper: apply rotation to 4 vertices around center point
    void rotateVertices(MuVertex verts[4], float cx, float cy, float angleRad);
};
