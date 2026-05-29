#include "RenderBatcher.h"

#include <android/log.h>
#include <cstring>

// GL constants not in GLES 3.0 (used by drawImmediate for primitive decomposition)
#ifndef GL_QUADS
#define GL_QUADS          0x0007
#endif
#ifndef GL_TRIANGLE_FAN
#define GL_TRIANGLE_FAN   0x0006
#endif
#ifndef GL_TRIANGLE_STRIP
#define GL_TRIANGLE_STRIP 0x0005
#endif

#define LOG_TAG "RenderBatcher"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

RenderBatcher::RenderBatcher()
    : m_vao(0), m_vbo(0), m_vboSize(0), m_ebo(0), m_eboSize(0)
    , m_initialized(false)
    , m_inBegin(false)
    , m_currentPrimitive(MuPrimitive::Triangles)
    , m_currentTexture(0)
    , m_mvpLoc(-1), m_mvLoc(-1), m_textureLoc(-1)
    , m_fogStartLoc(-1), m_fogEndLoc(-1), m_fogColorLoc(-1)
    , m_alphaTestLoc(-1), m_useTextureLoc(-1) {
}

RenderBatcher::~RenderBatcher() {
    if (m_initialized) {
        destroy();
    }
}

void RenderBatcher::initialize() {
    if (m_initialized) return;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    setupVAO();

    m_vertices.reserve(MAX_BATCH_VERTICES);
    m_indices.reserve(MAX_BATCH_VERTICES * 2);

    m_initialized = true;
    LOGI("RenderBatcher initialized (VAO=%u, VBO=%u)", m_vao, m_vbo);
}

void RenderBatcher::destroy() {
    flush();

    if (m_ebo) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
        m_eboSize = 0;
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
        m_vboSize = 0;
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }

    m_vertices.clear();
    m_indices.clear();
    m_initialized = false;
}

void RenderBatcher::setUniformLocations(GLint mvpLoc, GLint mvLoc, GLint textureLoc,
                                         GLint fogStartLoc, GLint fogEndLoc, GLint fogColorLoc,
                                         GLint alphaTestLoc, GLint useTextureLoc) {
    m_mvpLoc = mvpLoc;
    m_mvLoc = mvLoc;
    m_textureLoc = textureLoc;
    m_fogStartLoc = fogStartLoc;
    m_fogEndLoc = fogEndLoc;
    m_fogColorLoc = fogColorLoc;
    m_alphaTestLoc = alphaTestLoc;
    m_useTextureLoc = useTextureLoc;
}

void RenderBatcher::setupVAO() {
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, POSITION_SIZE, GL_FLOAT, GL_FALSE, VERTEX_STRIDE,
                          reinterpret_cast<void*>(OFFSET_POSITION));

    // Color (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, COLOR_SIZE, GL_FLOAT, GL_FALSE, VERTEX_STRIDE,
                          reinterpret_cast<void*>(OFFSET_COLOR));

    // TexCoord (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, TEXCOORD_SIZE, GL_FLOAT, GL_FALSE, VERTEX_STRIDE,
                          reinterpret_cast<void*>(OFFSET_TEXCOORD));

    // Element buffer for indexed draws
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

    glBindVertexArray(0);
}

void RenderBatcher::begin(MuPrimitive primitive, GLuint textureId) {
    if (!m_initialized) initialize();

    if (m_inBegin) {
        flush();
    }

    m_currentPrimitive = primitive;
    m_currentTexture = textureId;
    m_inBegin = true;
    m_vertices.clear();
    m_indices.clear();
}

void RenderBatcher::vertex(const MuVertex& v) {
    m_vertices.push_back(v);
}

void RenderBatcher::vertex(float x, float y, float z,
                            float r, float g, float b, float a,
                            float u, float v) {
    m_vertices.emplace_back(x, y, z, r, g, b, a, u, v);
}

void RenderBatcher::end() {
    if (!m_inBegin || m_vertices.empty()) {
        m_inBegin = false;
        return;
    }

    m_inBegin = false;

    switch (m_currentPrimitive) {
    case MuPrimitive::Triangles:
        drawBatch(m_vertices.size(), m_currentTexture);
        break;

    case MuPrimitive::TriangleFan: {
        // Decompose fan into triangles: vertices (0,1,2), (0,2,3), (0,3,4)...
        size_t numTris = m_vertices.size() - 2;
        m_indices.clear();
        for (size_t i = 0; i < numTris; ++i) {
            m_indices.push_back(0);
            m_indices.push_back(i + 1);
            m_indices.push_back(i + 2);
        }
        drawBatch(m_indices.size(), m_currentTexture);
        break;
    }

    case MuPrimitive::Quads: {
        // Decompose quads: each 4-vertex group → 2 triangles (0,1,2), (0,2,3)
        size_t numQuads = m_vertices.size() / 4;
        m_indices.clear();
        for (size_t q = 0; q < numQuads; ++q) {
            GLushort base = q * 4;
            m_indices.push_back(base + 0);
            m_indices.push_back(base + 1);
            m_indices.push_back(base + 2);
            m_indices.push_back(base + 0);
            m_indices.push_back(base + 2);
            m_indices.push_back(base + 3);
        }
        drawBatch(m_indices.size(), m_currentTexture);
        break;
    }

    case MuPrimitive::Lines:
        drawBatch(m_vertices.size(), m_currentTexture);
        break;
    }

    m_vertices.clear();
    m_indices.clear();
    m_currentTexture = 0;
}

// ============================================================================
// VBO/EBO buffer management (mobile-optimized)
//
// glBufferSubData for in-place updates when data fits within the current
// allocation. Only orphan (glBufferData nullptr) on growth.
// Mobile GPUs (Mali, Adreno) prefer SubData over per-frame reallocation.
// ============================================================================

void RenderBatcher::uploadVertices(const void* data, size_t size) {
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    if (size > static_cast<size_t>(m_vboSize)) {
        GLsizeiptr newSize = static_cast<GLsizeiptr>(size) * 2;
        glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW);
        m_vboSize = newSize;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
}

void RenderBatcher::uploadIndices(const void* data, size_t size) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    if (size > static_cast<size_t>(m_eboSize)) {
        GLsizeiptr newSize = static_cast<GLsizeiptr>(size) * 2;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW);
        m_eboSize = newSize;
    }
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(size), data);
}

void RenderBatcher::drawImmediate(GLenum mode, const MuVertex* verts, size_t count, GLuint textureId) {
    if (count == 0) return;

    if (!m_initialized) initialize();

    std::vector<GLushort> indices;

    switch (mode) {
    case GL_TRIANGLES:
        // Direct triangles
        break;

    case GL_TRIANGLE_FAN: {
        size_t numTris = count - 2;
        indices.reserve(numTris * 3);
        for (size_t i = 0; i < numTris; ++i) {
            indices.push_back(0);
            indices.push_back(static_cast<GLushort>(i + 1));
            indices.push_back(static_cast<GLushort>(i + 2));
        }
        break;
    }

    case GL_QUADS: {
        size_t numQuads = count / 4;
        indices.reserve(numQuads * 6);
        for (size_t q = 0; q < numQuads; ++q) {
            GLushort base = static_cast<GLushort>(q * 4);
            indices.push_back(base + 0);
            indices.push_back(base + 1);
            indices.push_back(base + 2);
            indices.push_back(base + 0);
            indices.push_back(base + 2);
            indices.push_back(base + 3);
        }
        break;
    }

    case GL_TRIANGLE_STRIP: {
        size_t numTris = count - 2;
        indices.reserve(numTris * 3);
        for (size_t i = 0; i < numTris; ++i) {
            if (i & 1) {
                indices.push_back(static_cast<GLushort>(i + 1));
                indices.push_back(static_cast<GLushort>(i + 2));
                indices.push_back(static_cast<GLushort>(i));
            } else {
                indices.push_back(static_cast<GLushort>(i));
                indices.push_back(static_cast<GLushort>(i + 1));
                indices.push_back(static_cast<GLushort>(i + 2));
            }
        }
        break;
    }

    case GL_LINES:
        break;

    default:
        return;
    }

    glBindVertexArray(m_vao);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(m_useTextureLoc, textureId != 0 ? 1 : 0);

    uploadVertices(verts, count * sizeof(MuVertex));

    if (indices.empty()) {
        GLenum drawMode = (mode == GL_LINES) ? GL_LINES : GL_TRIANGLES;
        glDrawArrays(drawMode, 0, static_cast<GLsizei>(count));
    } else {
        uploadIndices(indices.data(), indices.size() * sizeof(GLushort));
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_SHORT, nullptr);
    }

    glBindVertexArray(0);
}

void RenderBatcher::flush() {
    if (m_inBegin) {
        end();
    }
}

void RenderBatcher::drawBatch(GLsizei vertexCount, GLuint textureId) {
    if (vertexCount == 0) return;

    glBindVertexArray(m_vao);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(m_useTextureLoc, textureId != 0 ? 1 : 0);

    uploadVertices(m_vertices.data(), m_vertices.size() * sizeof(MuVertex));

    if (m_indices.empty()) {
        GLenum mode = (m_currentPrimitive == MuPrimitive::Lines) ? GL_LINES : GL_TRIANGLES;
        glDrawArrays(mode, 0, vertexCount);
    } else {
        uploadIndices(m_indices.data(), m_indices.size() * sizeof(GLushort));
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, nullptr);
    }

    glBindVertexArray(0);
}
