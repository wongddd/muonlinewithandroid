#pragma once

#include <GLES3/gl3.h>
#include <vector>

#include "MuRendererTypes.h"

// General-purpose VBO batcher. Accumulates vertices on CPU,
// flushes to GPU with a single glBufferData + glDrawArrays per batch.
//
// Usage mirrors OpenGL 1.x begin/end:
//   batcher.begin(MuPrimitive::TriangleFan, textureId);
//   batcher.vertex(x, y, z, r, g, b, a, u, v);
//   ... more vertices ...
//   batcher.end();
//
// end() converts primitives (fans→tris, quads→tris) and may flush
// immediately or defer until buffer is full.

class RenderBatcher {
public:
    static constexpr size_t MAX_BATCH_VERTICES = 65536; // 256KB per batch at 40 bytes/vertex

    RenderBatcher();
    ~RenderBatcher();

    void initialize();
    void destroy();

    // Begin a primitive batch
    void begin(MuPrimitive primitive, GLuint textureId = 0);

    // Add a vertex
    void vertex(const MuVertex& v);
    void vertex(float x, float y, float z, float r, float g, float b, float a, float u, float v);

    // End the batch — flushes to GPU
    void end();

    // Force flush any pending data
    void flush();

    // Capture-adapter entry point: takes raw GL primitive mode and pre-built
    // vertex array, decomposes to triangles, and draws via VBO.
    // Used by MuGLCompat.h glBegin/glEnd wrappers.
    void drawImmediate(GLenum mode, const MuVertex* verts, size_t count, GLuint textureId);

    // Set shader uniform locations (call once after shader is loaded)
    void setUniformLocations(GLint mvpLoc, GLint mvLoc, GLint textureLoc,
                             GLint fogStartLoc, GLint fogEndLoc, GLint fogColorLoc,
                             GLint alphaTestLoc, GLint useTextureLoc);

private:
    void setupVAO();
    void drawBatch(GLsizei vertexCount, GLuint textureId);

    // VBO/EBO buffer management (mobile-optimized)
    // Use glBufferSubData when data fits; orphan (glBufferData nullptr) when growth needed
    void uploadVertices(const void* data, size_t size);
    void uploadIndices(const void* data, size_t size);

    GLuint m_vao;
    GLuint m_vbo;
    GLsizeiptr m_vboSize;   // current VBO allocation size (0 = unallocated)
    GLuint m_ebo;           // element/index buffer for fan→tri / quad→tri decomposition
    GLsizeiptr m_eboSize;   // current EBO allocation size

    bool m_initialized;
    bool m_inBegin;

    MuPrimitive m_currentPrimitive;
    GLuint m_currentTexture;

    std::vector<MuVertex> m_vertices;
    std::vector<GLushort> m_indices; // for fan/quad decomposition

    // Uniform locations
    GLint m_mvpLoc;
    GLint m_mvLoc;
    GLint m_textureLoc;
    GLint m_fogStartLoc;
    GLint m_fogEndLoc;
    GLint m_fogColorLoc;
    GLint m_alphaTestLoc;
    GLint m_useTextureLoc;
};
