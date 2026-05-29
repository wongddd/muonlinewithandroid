#pragma once

// ============================================================================
// ImGui Android 触摸输入后端 (替代 imgui_impl_win32.cpp)
// ============================================================================
//
// 将 MuInput 的触摸/键盘/鼠标状态注入 ImGuiIO:
//   - 触摸 → ImGui 鼠标位置 + 左键
//   - 物理键盘 → ImGui 按键事件
//   - Delta time → ImGuiIO::DeltaTime
//
// 用法:
//   ImGui_ImplAndroid_Init();
//   // 每帧:
//   ImGui_ImplAndroid_NewFrame(width, height);
//   ImGui::NewFrame();
//   // ... ImGui 调用 ...
//   ImGui::Render();
//   ImGui_ImplGLES3_RenderDrawData(ImGui::GetDrawData());
//   ImGui_ImplAndroid_Shutdown();

struct ANativeWindow;
struct AInputEvent;

bool ImGui_ImplAndroid_Init();
void ImGui_ImplAndroid_Shutdown();
void ImGui_ImplAndroid_NewFrame(int displayWidth, int displayHeight);
