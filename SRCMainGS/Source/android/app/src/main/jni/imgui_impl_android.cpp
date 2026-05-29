#include "imgui_impl_android.h"
#include "imgui.h"
#include "MuInput.h"

#include <android/log.h>
#include <chrono>

#define LOG_TAG "ImGuiAndroid"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static std::chrono::steady_clock::time_point g_lastTime;
static bool g_initialized = false;

bool ImGui_ImplAndroid_Init() {
    ImGuiIO& io = ImGui::GetIO();

    // 配置功能标志
    io.BackendPlatformName = "imgui_impl_android";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    // 键盘映射 (Android KeyEvent keycodes → ImGuiKey)
    // 常用按键: 使用 ImGui 标准映射
    io.KeyMap[ImGuiKey_Tab]       = MuInput::VK::TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = MuInput::VK::LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = MuInput::VK::RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = MuInput::VK::UP;
    io.KeyMap[ImGuiKey_DownArrow]  = MuInput::VK::DOWN;
    io.KeyMap[ImGuiKey_Enter]      = MuInput::VK::RETURN;
    io.KeyMap[ImGuiKey_Escape]     = MuInput::VK::ESCAPE;
    io.KeyMap[ImGuiKey_Space]      = MuInput::VK::SPACE;
    io.KeyMap[ImGuiKey_Backspace]  = MuInput::VK::BACK;
    io.KeyMap[ImGuiKey_Delete]     = 0x2E; // VK_DELETE
    io.KeyMap[ImGuiKey_A]          = MuInput::VK::A;
    io.KeyMap[ImGuiKey_C]          = MuInput::VK::C;
    io.KeyMap[ImGuiKey_V]          = MuInput::VK::V;
    io.KeyMap[ImGuiKey_X]          = MuInput::VK::X;
    io.KeyMap[ImGuiKey_Y]          = 0x59; // VK_Y
    io.KeyMap[ImGuiKey_Z]          = 0x5A; // VK_Z

    g_lastTime = std::chrono::steady_clock::now();
    g_initialized = true;

    LOGI("ImGui Android backend initialized");
    return true;
}

void ImGui_ImplAndroid_Shutdown() {
    g_initialized = false;
    LOGI("ImGui Android backend shutdown");
}

void ImGui_ImplAndroid_NewFrame(int displayWidth, int displayHeight) {
    if (!g_initialized) return;

    ImGuiIO& io = ImGui::GetIO();

    // 屏幕尺寸
    io.DisplaySize = ImVec2(static_cast<float>(displayWidth), static_cast<float>(displayHeight));
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Delta time
    auto now = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(now - g_lastTime).count();
    g_lastTime = now;
    io.DeltaTime = (deltaTime > 0.0f) ? deltaTime : (1.0f / 60.0f);

    // --- 键盘状态 (从 MuInput 虚拟键表) ---
    // 遍历已知按键并更新 io.KeysDown
    static const int trackedKeys[] = {
        MuInput::VK::TAB, MuInput::VK::LEFT, MuInput::VK::RIGHT,
        MuInput::VK::UP, MuInput::VK::DOWN, MuInput::VK::RETURN,
        MuInput::VK::ESCAPE, MuInput::VK::SPACE, MuInput::VK::BACK,
        MuInput::VK::A, MuInput::VK::C, MuInput::VK::V, MuInput::VK::X,
        MuInput::VK::SHIFT, MuInput::VK::CONTROL, MuInput::VK::MENU,
    };
    static const int numTrackedKeys = sizeof(trackedKeys) / sizeof(trackedKeys[0]);

    for (int i = 0; i < numTrackedKeys; ++i) {
        int key = trackedKeys[i];
        io.KeysDown[key] = MuInput::isKeyDown(key) || MuInput::isKeyHeldDown(key);
    }

    // Modifier keys
    io.KeyCtrl  = io.KeysDown[MuInput::VK::CONTROL];
    io.KeyShift = io.KeysDown[MuInput::VK::SHIFT];
    io.KeyAlt   = io.KeysDown[MuInput::VK::MENU];
    io.KeySuper = false;

    // --- 鼠标状态 (从 MuInput) ---
    io.MousePos = ImVec2(
        static_cast<float>(MuInput::getMouseX()),
        static_cast<float>(MuInput::getMouseY()));

    io.MouseDown[0] = MuInput::isMouseLButtonDown() || MuInput::isMouseLButtonHeld();
    io.MouseDown[1] = MuInput::isMouseRButtonDown() || MuInput::isMouseRButtonHeld();
    io.MouseDown[2] = MuInput::isMouseMButtonDown() || MuInput::isMouseMButtonHeld();

    io.MouseWheel = static_cast<float>(MuInput::getMouseWheel());

    // 如果 ImGui 想捕获鼠标，隐藏系统光标
    // Android 上不需要处理光标可见性
}
