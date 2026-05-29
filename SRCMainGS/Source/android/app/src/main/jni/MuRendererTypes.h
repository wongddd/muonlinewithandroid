#pragma once

// Common vertex format for all rendering in the Android port.
// Matches the attribute layout in shader_es.vert:
//   location 0: aPosition (vec4)
//   location 1: aColor    (vec4)
//   location 2: aTexCoord (vec2)

struct MuVertex {
    float x, y, z;      // position
    float r, g, b, a;   // color (modulation)
    float u, v;         // texture coordinates

    MuVertex() : x(0), y(0), z(0), r(1), g(1), b(1), a(1), u(0), v(0) {}

    MuVertex(float px, float py, float pz,
             float cr, float cg, float cb, float ca,
             float tu, float tv)
        : x(px), y(py), z(pz)
        , r(cr), g(cg), b(cb), a(ca)
        , u(tu), v(tv) {}
};

// Attribute sizes in floats
static constexpr int POSITION_SIZE = 3;
static constexpr int COLOR_SIZE = 4;
static constexpr int TEXCOORD_SIZE = 2;
static constexpr int VERTEX_STRIDE = sizeof(MuVertex);

// Attribute offsets within MuVertex
static constexpr size_t OFFSET_POSITION = offsetof(MuVertex, x);
static constexpr size_t OFFSET_COLOR = offsetof(MuVertex, r);
static constexpr size_t OFFSET_TEXCOORD = offsetof(MuVertex, u);

enum class MuPrimitive {
    Triangles,
    TriangleFan,   // converted to indexed triangles internally
    Quads,         // converted to 2 triangles internally
    Lines
};
