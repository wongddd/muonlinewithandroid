#pragma once
#include "imgui.h"

// GLES 3.0 rendering backend using VBO/VAO + GLSL ES 3.0 shaders.
// Pair with imgui_impl_android.cpp for platform input.

IMGUI_IMPL_API bool     ImGui_ImplGLES3_Init();
IMGUI_IMPL_API void     ImGui_ImplGLES3_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplGLES3_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplGLES3_RenderDrawData(ImDrawData* draw_data);

IMGUI_IMPL_API bool     ImGui_ImplGLES3_CreateFontsTexture();
IMGUI_IMPL_API void     ImGui_ImplGLES3_DestroyFontsTexture();
IMGUI_IMPL_API bool     ImGui_ImplGLES3_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplGLES3_DestroyDeviceObjects();
