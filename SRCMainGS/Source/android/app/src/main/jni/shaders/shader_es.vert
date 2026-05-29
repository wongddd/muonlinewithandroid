#version 300 es

// GLES 3.0 vertex shader — replaces fixed-function transform pipeline
// Inputs: vertex attributes (position, color, texcoord)
// Outputs: color, texcoord, fog factor to fragment shader

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModelViewProjection;
uniform mat4 uModelView;
uniform float uFogStart;
uniform float uFogEnd;

out vec4 vColor;
out vec2 vTexCoord;
out float vFogFactor;

void main() {
    gl_Position = uModelViewProjection * aPosition;

    vColor = aColor;
    vTexCoord = aTexCoord;

    // Linear fog: fogFactor = (fogEnd - distance) / (fogEnd - fogStart)
    vec4 worldPos = uModelView * aPosition;
    float dist = length(worldPos.xyz);
    vFogFactor = clamp((uFogEnd - dist) / (uFogEnd - uFogStart), 0.0, 1.0);
}
