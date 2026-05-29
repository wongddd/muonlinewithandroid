#include "SpriteRenderer.h"

#include <android/log.h>
#include <cmath>

#define LOG_TAG "SpriteRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static constexpr float PI = 3.14159265358979323846f;
static constexpr float DEG2RAD = PI / 180.0f;

SpriteRenderer::SpriteRenderer()
    : m_batcher(nullptr) {
}

SpriteRenderer::~SpriteRenderer() {
    destroy();
}

void SpriteRenderer::initialize(RenderBatcher* batcher) {
    m_batcher = batcher;
}

void SpriteRenderer::destroy() {
    m_batcher = nullptr;
}

void SpriteRenderer::drawBitmap(float x, float y, float width, float height,
                                 float u0, float v0, float u1, float v1,
                                 GLuint textureId, float alpha,
                                 float sx, float sy) {
    if (!m_batcher) return;

    float x1 = x + width * sx;
    float y1 = y + height * sy;

    m_batcher->begin(MuPrimitive::TriangleFan, textureId);
    m_batcher->vertex(x,  y,  0, 1,1,1,alpha, u0, v0); // top-left
    m_batcher->vertex(x1, y,  0, 1,1,1,alpha, u1, v0); // top-right
    m_batcher->vertex(x1, y1, 0, 1,1,1,alpha, u1, v1); // bottom-right
    m_batcher->vertex(x,  y1, 0, 1,1,1,alpha, u0, v1); // bottom-left
    m_batcher->end();
}

void SpriteRenderer::drawColorBitmap(float x, float y, float width, float height,
                                      float u0, float v0, float u1, float v1,
                                      GLuint textureId,
                                      float r, float g, float b, float alpha) {
    if (!m_batcher) return;

    float x1 = x + width;
    float y1 = y + height;

    m_batcher->begin(MuPrimitive::TriangleFan, textureId);
    m_batcher->vertex(x,  y,  0, r,g,b,alpha, u0, v0);
    m_batcher->vertex(x1, y,  0, r,g,b,alpha, u1, v0);
    m_batcher->vertex(x1, y1, 0, r,g,b,alpha, u1, v1);
    m_batcher->vertex(x,  y1, 0, r,g,b,alpha, u0, v1);
    m_batcher->end();
}

void SpriteRenderer::drawBitmapRotate(float x, float y, float width, float height,
                                       float u0, float v0, float u1, float v1,
                                       GLuint textureId, float angle, float alpha) {
    if (!m_batcher) return;

    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float cx = x + hw;
    float cy = y + hh;

    MuVertex verts[4] = {
        MuVertex(-hw, -hh, 0, 1,1,1,alpha, u0, v0),
        MuVertex( hw, -hh, 0, 1,1,1,alpha, u1, v0),
        MuVertex( hw,  hh, 0, 1,1,1,alpha, u1, v1),
        MuVertex(-hw,  hh, 0, 1,1,1,alpha, u0, v1),
    };

    rotateVertices(verts, cx, cy, angle * DEG2RAD);

    m_batcher->begin(MuPrimitive::TriangleFan, textureId);
    m_batcher->vertex(verts[0]);
    m_batcher->vertex(verts[1]);
    m_batcher->vertex(verts[2]);
    m_batcher->vertex(verts[3]);
    m_batcher->end();
}

void SpriteRenderer::drawBitmapAlpha(float x, float y, float width, float height,
                                      float u0, float v0, float u1, float v1,
                                      GLuint textureId, float alpha, float edgeSize) {
    if (!m_batcher) return;

    // 4x4 sub-divided grid (16 vertices, 9 quads = 18 triangles)
    // Edge vertices have alpha scaled by distance from border
    float ex = edgeSize;
    float ey = edgeSize;
    float x1 = x + width;
    float y1 = y + height;

    // UV deltas
    float du = (u1 - u0) / 4.0f;
    float dv = (v1 - v0) / 4.0f;

    // X and Y positions for the 5 grid lines
    float px[5] = {x, x + ex, x + width*0.25f, x1 - ex, x1};
    float py[5] = {y, y + ey, y + height*0.25f, y1 - ey, y1};

    // Alpha for each grid position (edges = 0, interior = alpha)
    float ax[5] = {0.0f, alpha*0.3f, alpha, alpha, alpha*0.3f};
    float ay[5] = {0.0f, alpha*0.3f, alpha, alpha, alpha*0.3f};

    // Build 4x4 grid as triangle strips or fan per cell
    // Simplified: render each of the 9 cells as a quad
    m_batcher->begin(MuPrimitive::Quads, textureId);

    for (int iy = 0; iy < 4; ++iy) {
        for (int ix = 0; ix < 4; ++ix) {
            float a00 = std::min(ax[ix],   ay[iy])   * alpha;
            float a10 = std::min(ax[ix+1], ay[iy])   * alpha;
            float a11 = std::min(ax[ix+1], ay[iy+1]) * alpha;
            float a01 = std::min(ax[ix],   ay[iy+1]) * alpha;

            float tu0 = u0 + du * ix;
            float tv0 = v0 + dv * iy;
            float tu1 = u0 + du * (ix+1);
            float tv1 = v0 + dv * (iy+1);

            m_batcher->vertex(px[ix],   py[iy],   0, 1,1,1,a00, tu0, tv0);
            m_batcher->vertex(px[ix+1], py[iy],   0, 1,1,1,a10, tu1, tv0);
            m_batcher->vertex(px[ix+1], py[iy+1], 0, 1,1,1,a11, tu1, tv1);
            m_batcher->vertex(px[ix],   py[iy+1], 0, 1,1,1,a01, tu0, tv1);
        }
    }

    m_batcher->end();
}

void SpriteRenderer::drawColorQuad(float x, float y, float width, float height,
                                    float r, float g, float b, float alpha) {
    if (!m_batcher) return;

    float x1 = x + width;
    float y1 = y + height;

    // No texture
    m_batcher->begin(MuPrimitive::TriangleFan, 0);
    m_batcher->vertex(x,  y,  0, r,g,b,alpha, 0,0);
    m_batcher->vertex(x1, y,  0, r,g,b,alpha, 1,0);
    m_batcher->vertex(x1, y1, 0, r,g,b,alpha, 1,1);
    m_batcher->vertex(x,  y1, 0, r,g,b,alpha, 0,1);
    m_batcher->end();
}

void SpriteRenderer::flush() {
    if (m_batcher) {
        m_batcher->flush();
    }
}

void SpriteRenderer::rotateVertices(MuVertex verts[4], float cx, float cy, float angleRad) {
    float cosA = cosf(angleRad);
    float sinA = sinf(angleRad);

    for (int i = 0; i < 4; ++i) {
        float dx = verts[i].x;
        float dy = verts[i].y;
        verts[i].x = cx + dx * cosA - dy * sinA;
        verts[i].y = cy + dx * sinA + dy * cosA;
    }
}
