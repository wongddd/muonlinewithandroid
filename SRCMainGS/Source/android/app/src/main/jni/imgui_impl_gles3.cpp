#include "imgui_impl_gles3.h"
#include "imgui.h"

#include <GLES3/gl3.h>
#include <android/log.h>

#define LOG_TAG "ImGuiGLES3"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// GLSL ES 1.00 shader (forward-compatible, works on both GLES 2.0 and 3.0)
static const char* g_vertexShaderSrc = R"GLSL(#version 100
attribute vec2 aPosition;
attribute vec2 aUV;
attribute vec4 aColor;
uniform mat4 uProjMtx;
varying vec2 vUV;
varying vec4 vColor;
void main() {
    vUV = aUV;
    vColor = aColor;
    gl_Position = uProjMtx * vec4(aPosition, 0.0, 1.0);
}
)GLSL";

static const char* g_fragmentShaderSrc = R"GLSL(#version 100
precision mediump float;
varying vec2 vUV;
varying vec4 vColor;
uniform sampler2D uTexture;
void main() {
    gl_FragColor = vColor * texture2D(uTexture, vUV);
}
)GLSL";

struct ImGui_ImplGLES3_Data {
    GLuint fontTexture;
    GLuint shaderProgram;
    GLuint vertHandle;
    GLuint fragHandle;
    GLuint attribPos;
    GLuint attribUV;
    GLuint attribColor;
    GLuint uniformProjMtx;
    GLuint uniformTex;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    ImGui_ImplGLES3_Data() { memset(this, 0, sizeof(*this)); }
};

static ImGui_ImplGLES3_Data* GetBackendData() {
    return ImGui::GetCurrentContext()
        ? (ImGui_ImplGLES3_Data*)ImGui::GetIO().BackendRendererUserData
        : nullptr;
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    if (!shader) {
        LOGE("glCreateShader(%d) returned 0 (GL error: %d)", type, glGetError());
        return 0;
    }
    glShaderSource(shader, 1, &src, nullptr);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOGE("glShaderSource failed: GL error 0x%X", err);
    }
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLchar buf[512] = {};
        glGetShaderInfoLog(shader, sizeof(buf) - 1, nullptr, buf);
        const char* typeName = (type == GL_VERTEX_SHADER) ? "vert" : "frag";
        LOGE("Shader compile error (%s): %s", typeName, buf[0] ? buf : "(empty log)");
        LOGE("Shader source [%s]:\n%s", typeName, src);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint linkProgram(GLuint vert, GLuint frag) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    GLint status;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        GLchar buf[512];
        glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
        LOGE("Program link error: %s", buf);
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

bool ImGui_ImplGLES3_Init() {
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendRendererUserData == nullptr && "Already initialized a renderer backend!");

    auto* bd = IM_NEW(ImGui_ImplGLES3_Data)();
    io.BackendRendererUserData = (void*)bd;
    io.BackendRendererName = "imgui_impl_gles3";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    LOGI("GLES3 renderer backend initialized");
    return true;
}

void ImGui_ImplGLES3_Shutdown() {
    auto* bd = GetBackendData();
    IM_ASSERT(bd != nullptr);
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGLES3_DestroyDeviceObjects();
    io.BackendRendererName = nullptr;
    io.BackendRendererUserData = nullptr;
    IM_DELETE(bd);

    LOGI("GLES3 renderer backend shutdown");
}

void ImGui_ImplGLES3_NewFrame() {
    auto* bd = GetBackendData();
    IM_ASSERT(bd != nullptr);
    if (!bd->shaderProgram)
        ImGui_ImplGLES3_CreateDeviceObjects();
}

void ImGui_ImplGLES3_RenderDrawData(ImDrawData* draw_data) {
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    auto* bd = GetBackendData();
    if (!bd || !bd->shaderProgram) return;

    int fb_width  = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;

    // Backup GL state
    GLint lastProgram;      glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
    GLint lastTexture;      glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
    GLint lastArrayBuffer;  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
    GLint lastElemBuffer;   glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastElemBuffer);
    GLint lastVAO;          glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVAO);
    GLint lastScissor[4];   glGetIntegerv(GL_SCISSOR_BOX, lastScissor);
    GLint lastBlendSrcRGB;  glGetIntegerv(GL_BLEND_SRC_RGB, &lastBlendSrcRGB);
    GLint lastBlendDstRGB;  glGetIntegerv(GL_BLEND_DST_RGB, &lastBlendDstRGB);
    GLint lastBlendSrcA;    glGetIntegerv(GL_BLEND_SRC_ALPHA, &lastBlendSrcA);
    GLint lastBlendDstA;    glGetIntegerv(GL_BLEND_DST_ALPHA, &lastBlendDstA);
    GLint lastBlendEqRGB;   glGetIntegerv(GL_BLEND_EQUATION_RGB, &lastBlendEqRGB);
    GLint lastBlendEqA;     glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &lastBlendEqA);
    GLint lastViewport[4];  glGetIntegerv(GL_VIEWPORT, lastViewport);
    GLboolean lastBlend     = glIsEnabled(GL_BLEND);
    GLboolean lastCull      = glIsEnabled(GL_CULL_FACE);
    GLboolean lastDepth     = glIsEnabled(GL_DEPTH_TEST);
    GLboolean lastStencil   = glIsEnabled(GL_STENCIL_TEST);
    GLboolean lastScissorEn = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    glUseProgram(bd->shaderProgram);
    glUniform1i(bd->uniformTex, 0);

    // Orthographic projection matrix
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    float proj[4][4] = {
        { 2.0f/(R-L),  0.0f,         0.0f, 0.0f },
        { 0.0f,         2.0f/(T-B),   0.0f, 0.0f },
        { 0.0f,         0.0f,        -1.0f, 0.0f },
        { (R+L)/(L-R),  (T+B)/(B-T),  0.0f, 1.0f },
    };
    glUniformMatrix4fv(bd->uniformProjMtx, 1, GL_FALSE, &proj[0][0]);

    glBindVertexArray(bd->vao);

    // Render command lists
    ImVec2 clip_off   = draw_data->DisplayPos;
    ImVec2 clip_scale = draw_data->FramebufferScale;

    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers
        glBindBuffer(GL_ARRAY_BUFFER, bd->vbo);
        glBufferData(GL_ARRAY_BUFFER,
            (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
            (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bd->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
            (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

            if (pcmd->UserCallback) {
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
                    glUseProgram(bd->shaderProgram);
                    glUniform1i(bd->uniformTex, 0);
                    glUniformMatrix4fv(bd->uniformProjMtx, 1, GL_FALSE, &proj[0][0]);
                    glBindVertexArray(bd->vao);
                } else {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
            } else {
                ImVec2 clip_min(
                    (pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                    (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max(
                    (pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                    (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                glScissor(
                    (int)clip_min.x,
                    (int)((float)fb_height - clip_max.y),
                    (int)(clip_max.x - clip_min.x),
                    (int)(clip_max.y - clip_min.y));

                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
                    sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
                    (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
            }
        }
    }

    // Restore GL state
    glUseProgram(lastProgram);
    glBindTexture(GL_TEXTURE_2D, lastTexture);
    glBindBuffer(GL_ARRAY_BUFFER, lastArrayBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lastElemBuffer);
    glBindVertexArray(lastVAO);
    glBlendEquationSeparate(lastBlendEqRGB, lastBlendEqA);
    glBlendFuncSeparate(lastBlendSrcRGB, lastBlendDstRGB, lastBlendSrcA, lastBlendDstA);
    if (lastBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (lastCull) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (lastDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (lastStencil) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);
    if (lastScissorEn) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glScissor(lastScissor[0], lastScissor[1], lastScissor[2], lastScissor[3]);
    glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
}

bool ImGui_ImplGLES3_CreateFontsTexture() {
    ImGuiIO& io = ImGui::GetIO();
    auto* bd = GetBackendData();

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    GLint lastTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);

    glGenTextures(1, &bd->fontTexture);
    glBindTexture(GL_TEXTURE_2D, bd->fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    io.Fonts->SetTexID((ImTextureID)(intptr_t)bd->fontTexture);

    glBindTexture(GL_TEXTURE_2D, lastTexture);

    LOGI("Font texture created (%dx%d)", width, height);
    return true;
}

void ImGui_ImplGLES3_DestroyFontsTexture() {
    ImGuiIO& io = ImGui::GetIO();
    auto* bd = GetBackendData();
    if (bd->fontTexture) {
        glDeleteTextures(1, &bd->fontTexture);
        io.Fonts->SetTexID(0);
        bd->fontTexture = 0;
    }
}

bool ImGui_ImplGLES3_CreateDeviceObjects() {
    auto* bd = GetBackendData();

    bd->vertHandle = compileShader(GL_VERTEX_SHADER, g_vertexShaderSrc);
    bd->fragHandle = compileShader(GL_FRAGMENT_SHADER, g_fragmentShaderSrc);
    if (!bd->vertHandle || !bd->fragHandle) return false;

    bd->shaderProgram = linkProgram(bd->vertHandle, bd->fragHandle);
    if (!bd->shaderProgram) return false;

    bd->attribPos     = glGetAttribLocation(bd->shaderProgram, "aPosition");
    bd->attribUV      = glGetAttribLocation(bd->shaderProgram, "aUV");
    bd->attribColor   = glGetAttribLocation(bd->shaderProgram, "aColor");
    bd->uniformProjMtx = glGetUniformLocation(bd->shaderProgram, "uProjMtx");
    bd->uniformTex     = glGetUniformLocation(bd->shaderProgram, "uTexture");

    // Create VAO
    glGenVertexArrays(1, &bd->vao);
    glBindVertexArray(bd->vao);

    glGenBuffers(1, &bd->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, bd->vbo);
    glEnableVertexAttribArray(bd->attribPos);
    glVertexAttribPointer(bd->attribPos, 2, GL_FLOAT, GL_FALSE,
        sizeof(ImDrawVert), (void*)offsetof(ImDrawVert, pos));
    glEnableVertexAttribArray(bd->attribUV);
    glVertexAttribPointer(bd->attribUV, 2, GL_FLOAT, GL_FALSE,
        sizeof(ImDrawVert), (void*)offsetof(ImDrawVert, uv));
    glEnableVertexAttribArray(bd->attribColor);
    glVertexAttribPointer(bd->attribColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,
        sizeof(ImDrawVert), (void*)offsetof(ImDrawVert, col));

    glGenBuffers(1, &bd->ebo);

    if (!ImGui_ImplGLES3_CreateFontsTexture()) return false;

    LOGI("Device objects created (program=%u, vao=%u)", bd->shaderProgram, bd->vao);
    return true;
}

void ImGui_ImplGLES3_DestroyDeviceObjects() {
    auto* bd = GetBackendData();

    ImGui_ImplGLES3_DestroyFontsTexture();

    if (bd->vbo) { glDeleteBuffers(1, &bd->vbo); bd->vbo = 0; }
    if (bd->ebo) { glDeleteBuffers(1, &bd->ebo); bd->ebo = 0; }
    if (bd->vao) { glDeleteVertexArrays(1, &bd->vao); bd->vao = 0; }
    if (bd->shaderProgram) {
        if (bd->vertHandle) { glDetachShader(bd->shaderProgram, bd->vertHandle); glDeleteShader(bd->vertHandle); }
        if (bd->fragHandle) { glDetachShader(bd->shaderProgram, bd->fragHandle); glDeleteShader(bd->fragHandle); }
        glDeleteProgram(bd->shaderProgram);
        bd->shaderProgram = 0;
        bd->vertHandle = 0;
        bd->fragHandle = 0;
    }
}
