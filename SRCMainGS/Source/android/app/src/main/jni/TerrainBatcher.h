#pragma once

#include <GLES3/gl3.h>
#include <vector>

#include "MuRendererTypes.h"

// Batches terrain tiles into large VBO draws.
// Each terrain tile is a triangle fan (4 vertices → 2 triangles).
// Tiles are accumulated and flushed periodically.
//
// In the original code, each terrain tile calls glBegin(GL_TRIANGLE_FAN)
// → 4x glVertex3f/glColor3f/glTexCoord2f → glEnd(). For a visible area
// of 64x64 tiles, that's 4096 draw calls per frame.
//
// This batcher reduces that to ~4-8 draw calls by batching.

class TerrainBatcher {
public:
    static constexpr size_t MAX_TILES_PER_BATCH = 16384; // 256x256 = 65536 max tiles

    TerrainBatcher();
    ~TerrainBatcher();

    void initialize();
    void destroy();

    // Begin a new terrain pass (clears internal buffers)
    void beginPass();

    // Add a terrain tile as a triangle fan (4 vertices, top-left winding)
    // v0 = top-left, v1 = top-right, v2 = bottom-right, v3 = bottom-left
    void addTile(const MuVertex& v0, const MuVertex& v1,
                 const MuVertex& v2, const MuVertex& v3,
                 GLuint textureId);

    // Flush all accumulated tiles to GPU
    void endPass();

    // Check if we should flush (based on texture change or buffer size)
    bool needsFlush(GLuint textureId) const;

private:
    void flushBatch();

    bool m_initialized;

    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;

    GLuint m_currentTexture;
    std::vector<MuVertex> m_vertices;
    std::vector<GLushort> m_indices;
    size_t m_tileCount;
};
