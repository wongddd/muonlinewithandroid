#include "TerrainBatcher.h"

#include <android/log.h>

#define LOG_TAG "TerrainBatcher"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

TerrainBatcher::TerrainBatcher()
    : m_initialized(false)
    , m_vao(0), m_vbo(0), m_ebo(0)
    , m_currentTexture(0)
    , m_tileCount(0) {
}

TerrainBatcher::~TerrainBatcher() {
    if (m_initialized) {
        destroy();
    }
}

void TerrainBatcher::initialize() {
    if (m_initialized) return;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, VERTEX_STRIDE,
                          reinterpret_cast<void*>(OFFSET_POSITION));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, COLOR_SIZE, GL_FLOAT, GL_FALSE, VERTEX_STRIDE,
                          reinterpret_cast<void*>(OFFSET_COLOR));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, TEXCOORD_SIZE, GL_FLOAT, GL_FALSE, VERTEX_STRIDE,
                          reinterpret_cast<void*>(OFFSET_TEXCOORD));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBindVertexArray(0);

    m_vertices.reserve(MAX_TILES_PER_BATCH * 4);
    m_indices.reserve(MAX_TILES_PER_BATCH * 6);

    m_initialized = true;
    LOGI("TerrainBatcher initialized (capacity=%zu tiles)", MAX_TILES_PER_BATCH);
}

void TerrainBatcher::destroy() {
    if (m_ebo) { glDeleteBuffers(1, &m_ebo); m_ebo = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    m_initialized = false;
}

void TerrainBatcher::beginPass() {
    m_vertices.clear();
    m_indices.clear();
    m_tileCount = 0;
    m_currentTexture = 0;
}

void TerrainBatcher::addTile(const MuVertex& v0, const MuVertex& v1,
                              const MuVertex& v2, const MuVertex& v3,
                              GLuint textureId) {
    // Flush if texture changes or buffer full
    if ((!m_vertices.empty() && textureId != m_currentTexture) || needsFlush(textureId)) {
        flushBatch();
    }

    m_currentTexture = textureId;

    // Add 4 vertices
    GLushort baseIndex = static_cast<GLushort>(m_vertices.size());
    m_vertices.push_back(v0);
    m_vertices.push_back(v1);
    m_vertices.push_back(v2);
    m_vertices.push_back(v3);

    // Fan decomposition: (0,1,2), (0,2,3)
    m_indices.push_back(baseIndex + 0);
    m_indices.push_back(baseIndex + 1);
    m_indices.push_back(baseIndex + 2);
    m_indices.push_back(baseIndex + 0);
    m_indices.push_back(baseIndex + 2);
    m_indices.push_back(baseIndex + 3);

    ++m_tileCount;
}

void TerrainBatcher::endPass() {
    if (!m_vertices.empty()) {
        flushBatch();
    }
}

bool TerrainBatcher::needsFlush(GLuint /*textureId*/) const {
    return m_tileCount >= MAX_TILES_PER_BATCH;
}

void TerrainBatcher::flushBatch() {
    if (m_vertices.empty() || m_indices.empty()) return;

    glBindVertexArray(m_vao);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(MuVertex),
                 m_vertices.data(), GL_STREAM_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLushort),
                 m_indices.data(), GL_STREAM_DRAW);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, m_currentTexture);

    // Draw all tiles in one call
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()),
                   GL_UNSIGNED_SHORT, nullptr);

    glBindVertexArray(0);

    // Clear for next batch (keep texture — next batch might use same)
    m_vertices.clear();
    m_indices.clear();
    m_tileCount = 0;
}
