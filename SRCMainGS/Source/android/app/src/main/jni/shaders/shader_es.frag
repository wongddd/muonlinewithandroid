#version 300 es

precision mediump float;

// GLES 3.0 fragment shader — replaces fixed-function texture/color pipeline
// Supports: texture sampling, color modulation, alpha test, fog blending

uniform sampler2D uTexture;
uniform vec4 uFogColor;
uniform float uAlphaTest;
uniform bool uUseTexture;

in vec4 vColor;
in vec2 vTexCoord;
in float vFogFactor;

out vec4 fragColor;

void main() {
    // Texture sampling (replaces glTexEnvf GL_MODULATE)
    vec4 texColor = uUseTexture ? texture(uTexture, vTexCoord) : vec4(1.0);
    vec4 color = vColor * texColor;

    // Alpha test (replaces glAlphaFunc + glEnable(GL_ALPHA_TEST))
    if (color.a < uAlphaTest) {
        discard;
    }

    // Fog blending (replaces glEnable(GL_FOG))
    color.rgb = mix(uFogColor.rgb, color.rgb, vFogFactor);

    fragColor = color;
}
