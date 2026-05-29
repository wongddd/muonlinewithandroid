#pragma once
// Shim for imgui_impl_opengl2.h on Android
// We use imgui_impl_gles3.cpp instead
#include "imgui.h"

inline void ImGui_ImplOpenGL2_NewFrame() {}
inline void ImGui_ImplOpenGL2_RenderDrawData(ImDrawData*) {}
