#include "ShaderLoader.h"

#include <android/log.h>
#include <cstdlib>
#include <cstring>

#define LOG_TAG "ShaderLoader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace ShaderLoader {

// Embedded GLES 3.0 vertex shader
// Supports: position, color, uv attributes; modelViewProjection matrix; fog
static const char* DEFAULT_VERT_SRC = R"(#version 300 es

// Mobile-optimized: mediump for colors/texcoords, highp for positions
layout(location = 0) in highp vec4 aPosition;
layout(location = 1) in mediump vec4 aColor;
layout(location = 2) in mediump vec2 aTexCoord;

uniform highp mat4 uModelViewProjection;
uniform highp mat4 uModelView;
uniform highp float uFogStart;
uniform highp float uFogEnd;

out mediump vec4 vColor;
out mediump vec2 vTexCoord;
out mediump float vFogFactor;
out highp vec3 vWorldPos;

void main() {
    gl_Position = uModelViewProjection * aPosition;

    vColor = aColor;
    vTexCoord = aTexCoord;

    // Linear fog factor
    highp vec4 worldPos = uModelView * aPosition;
    vWorldPos = worldPos.xyz;
    highp float dist = length(worldPos.xyz);
    vFogFactor = clamp((uFogEnd - dist) / (uFogEnd - uFogStart), 0.0, 1.0);
}
)";

// Embedded GLES 3.0 fragment shader
// Supports: texture sampling, color modulation, alpha test, fog
static const char* DEFAULT_FRAG_SRC = R"(#version 300 es

precision mediump float;

uniform sampler2D uTexture;
uniform vec4 uFogColor;
uniform float uAlphaTest;
uniform bool uUseTexture;

in vec4 vColor;
in vec2 vTexCoord;
in float vFogFactor;
in vec3 vWorldPos;

out vec4 fragColor;

void main() {
    vec4 texColor = uUseTexture ? texture(uTexture, vTexCoord) : vec4(1.0);

    // Modulate: vertex color * texture color (replaces glTexEnvf MODULATE)
    vec4 color = vColor * texColor;

    // Alpha test (replaces glAlphaFunc)
    if (color.a < uAlphaTest) {
        discard;
    }

    // Fog blending (replaces glEnable(GL_FOG))
    color.rgb = mix(uFogColor.rgb, color.rgb, vFogFactor);

    fragColor = color;
}
)";

static GLuint g_defaultProgram = 0;

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    if (!shader) {
        LOGE("glCreateShader failed (type=%d)", type);
        return 0;
    }

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* buf = (char*)malloc(infoLen);
            glGetShaderInfoLog(shader, infoLen, nullptr, buf);
            LOGE("Shader compile error: %s", buf);
            free(buf);
        }
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint linkProgram(GLuint vertShader, GLuint fragShader) {
    GLuint program = glCreateProgram();
    if (!program) {
        LOGE("glCreateProgram failed");
        return 0;
    }

    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            char* buf = (char*)malloc(infoLen);
            glGetProgramInfoLog(program, infoLen, nullptr, buf);
            LOGE("Program link error: %s", buf);
            free(buf);
        }
        glDeleteProgram(program);
        return 0;
    }

    // Shaders can be detached and deleted after linking
    glDetachShader(program, vertShader);
    glDetachShader(program, fragShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    return program;
}

GLuint loadDefaultProgram() {
    GLuint vert = compileShader(GL_VERTEX_SHADER, DEFAULT_VERT_SRC);
    if (!vert) {
        LOGE("Failed to compile default vertex shader");
        return 0;
    }

    GLuint frag = compileShader(GL_FRAGMENT_SHADER, DEFAULT_FRAG_SRC);
    if (!frag) {
        LOGE("Failed to compile default fragment shader");
        glDeleteShader(vert);
        return 0;
    }

    GLuint program = linkProgram(vert, frag);
    if (!program) {
        LOGE("Failed to link default program");
        return 0;
    }

    g_defaultProgram = program;
    LOGI("Default shader program compiled and linked (id=%u)", program);

    // Cache uniform locations
    GLint mvpLoc = glGetUniformLocation(program, "uModelViewProjection");
    GLint mvLoc = glGetUniformLocation(program, "uModelView");
    GLint fogStartLoc = glGetUniformLocation(program, "uFogStart");
    GLint fogEndLoc = glGetUniformLocation(program, "uFogEnd");
    GLint fogColorLoc = glGetUniformLocation(program, "uFogColor");
    GLint alphaTestLoc = glGetUniformLocation(program, "uAlphaTest");
    GLint textureLoc = glGetUniformLocation(program, "uTexture");
    GLint useTextureLoc = glGetUniformLocation(program, "uUseTexture");

    // Set default uniforms
    glUseProgram(program);
    glUniform1f(fogStartLoc, 2000.0f);
    glUniform1f(fogEndLoc, 8000.0f);
    glUniform4f(fogColorLoc, 0.05f, 0.05f, 0.08f, 1.0f);
    glUniform1f(alphaTestLoc, 0.1f);
    glUniform1i(textureLoc, 0); // texture unit 0
    glUniform1i(useTextureLoc, 1);

    return program;
}

GLuint loadProgram(const char* vertAssetPath, const char* fragAssetPath) {
    // Phase 9: Load shader sources from assets via AAssetManager
    // For now, all shaders are embedded
    (void)vertAssetPath;
    (void)fragAssetPath;
    return 0;
}

void shutdown() {
    if (g_defaultProgram) {
        glDeleteProgram(g_defaultProgram);
        g_defaultProgram = 0;
    }
}

} // namespace ShaderLoader
