#pragma once
// Shim for imgui_impl_win32.h on Android
// We use imgui_impl_android.cpp instead
#include "imgui.h"

inline void ImGui_ImplWin32_NewFrame() {}
