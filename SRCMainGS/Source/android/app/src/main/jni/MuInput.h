#pragma once

#include <cstdint>

// ============================================================================
// MuInput — 虚拟输入系统（替代 Win32 键盘/鼠标 + CInput + CNewKeyInput）
// ============================================================================
//
// 触摸屏映射布局 (横屏 1920×1080):
//   ┌────────────────┬──────────────────────────────┐
//   │  虚拟摇杆      │  [1] [2] [3] [4] [5]         │ 技能栏
//   │  (WASD 移动)   │  [6] [7] [8] [9] [0]         │
//   │  屏幕左 40%    ├──────────────────────────────┤
//   │                │  [Tab][I] [C] [Q] [M]        │ UI 快捷键
//   │                │  [Ctrl][Spc][F][P] [G]       │ 操作键
//   └────────────────┴──────────────────────────────┘
//                      ← 屏幕右 60% →
//
// 状态机 (等价于 CNewKeyInput):
//   KEY_NONE → KEY_PRESS → KEY_REPEAT → KEY_RELEASE → KEY_NONE
//
// 等价原始类:
//   - CInput (Input.h) — 鼠标/键盘状态查询接口
//   - CNewKeyInput (NewUICommon.h) — 按键状态机 ScanAsyncKeyState
//   - SEASON3B::IsPress/IsRepeat/IsRelease/IsNone — 对外查询函数

namespace MuInput {

// 按键状态机 (匹配 CNewKeyInput::KEY_STATE)
enum KeyState : uint8_t {
    KEY_NONE    = 0,  // 未按下
    KEY_RELEASE = 1,  // 本帧刚释放
    KEY_PRESS   = 2,  // 本帧刚按下
    KEY_REPEAT  = 3   // 持续按住中
};

struct InputStateInfo {
    KeyState state = KEY_NONE;
    uint32_t pressTime = 0;  // 按下时刻 (用于重复延迟)
};

// ============================================================================
// 公开接口
// ============================================================================

// 初始化 (设置屏幕尺寸)
void init(int screenWidth, int screenHeight);

// 更新屏幕尺寸 (旋转/多窗口)
void setScreenSize(int width, int height);

// 每帧调用: 更新按键状态机 (NONE→PRESS→REPEAT, RELEASE→NONE)
void update();

// 每帧末调用: 清零瞬时鼠标标志 (down/up/dbl/wheel)
// 必须在 ImGui/Scene 渲染完成后调用，确保 UI 能读取到本帧的点击事件
void resetOneShotMouseFlags();

// ============ Touch 事件 (从 JNI 调用) ============

void onTouchDown(float x, float y, int pointerId);
void onTouchMove(float x, float y, int pointerId);
void onTouchUp(float x, float y, int pointerId);

// ============ 物理键盘事件 (从 JNI 调用，用于调试) ============

void onKeyDown(int keyCode);
void onKeyUp(int keyCode);

// ============ 键盘状态查询 (等价于 CInput + CNewKeyInput) ============

bool isKeyDown(int vkCode);      // 等价 IsPress  (刚按下)
bool isKeyHeldDown(int vkCode);  // 等价 IsRepeat (持续按住)
bool isKeyUp(int vkCode);        // 等价 IsRelease (刚释放)
bool isKeyNone(int vkCode);      // 等价 IsNone (未按下)
KeyState getKeyState(int vkCode); // 原始 KeyState (供 CNewKeyInput 同步)

// IsKeyDown / IsKeyHeldDown by name (matches original Input.h interface)
// isKeyDown = just pressed this frame
// isKeyHeldDown = being held (after initial press)

// ============ 鼠标状态查询 (等价于 CInput) ============

int  getMouseX();
int  getMouseY();
int  getMouseDX();        // 鼠标 X 位移 (本帧)
int  getMouseDY();        // 鼠标 Y 位移 (本帧)
int  getMouseWheel();     // 滚轮增量 (本帧, 清除后为0)

bool isMouseLButtonDown();     // 左键刚按下
bool isMouseLButtonHeld();     // 左键按住中
bool isMouseLButtonUp();       // 左键刚释放
bool isMouseLButtonDbl();      // 左键双击

bool isMouseRButtonDown();
bool isMouseRButtonHeld();
bool isMouseRButtonUp();
bool isMouseRButtonDbl();

bool isMouseMButtonDown();     // 中键
bool isMouseMButtonHeld();
bool isMouseMButtonUp();

// ============ 文本编辑模式 (匹配 CInput::SetTextEditMode) ============

void setTextEditMode(bool editMode);
bool isTextEditMode();

// ============ IME 文本输入缓冲 ============

void appendIMEText(const char* text);
bool hasIMEText();
const char* consumeIMEText();

// ============ 虚拟按键触发 (供 UI 按钮调用) ============

void pressVirtualKey(int vkCode);    // 模拟按键按下
void releaseVirtualKey(int vkCode);  // 模拟按键释放

// ============ 调试 ============

int  getScreenWidth();
int  getScreenHeight();
int  getActiveTouchCount();

// 常用虚拟键码 (Windows VK_* 常量)
namespace VK {
    constexpr int LBUTTON = 0x01;
    constexpr int RBUTTON = 0x02;
    constexpr int MBUTTON = 0x04;
    constexpr int BACK    = 0x08;
    constexpr int TAB     = 0x09;
    constexpr int RETURN  = 0x0D;
    constexpr int SHIFT   = 0x10;
    constexpr int CONTROL = 0x11;
    constexpr int MENU    = 0x12;
    constexpr int ESCAPE  = 0x1B;
    constexpr int SPACE   = 0x20;
    constexpr int LEFT    = 0x25;
    constexpr int UP      = 0x26;
    constexpr int RIGHT   = 0x27;
    constexpr int DOWN    = 0x28;
    constexpr int KEY_0   = 0x30;
    constexpr int KEY_1   = 0x31;
    constexpr int KEY_2   = 0x32;
    constexpr int KEY_3   = 0x33;
    constexpr int KEY_4   = 0x34;
    constexpr int KEY_5   = 0x35;
    constexpr int KEY_6   = 0x36;
    constexpr int KEY_7   = 0x37;
    constexpr int KEY_8   = 0x38;
    constexpr int KEY_9   = 0x39;
    constexpr int A       = 0x41;
    constexpr int B       = 0x42;
    constexpr int C       = 0x43;
    constexpr int D       = 0x44;
    constexpr int E       = 0x45;
    constexpr int F       = 0x46;
    constexpr int G       = 0x47;
    constexpr int H       = 0x48;
    constexpr int I       = 0x49;
    constexpr int M       = 0x4D;
    constexpr int P       = 0x50;
    constexpr int Q       = 0x51;
    constexpr int S       = 0x53;
    constexpr int T       = 0x54;
    constexpr int V       = 0x56;
    constexpr int W       = 0x57;
    constexpr int X       = 0x58;
    constexpr int Y       = 0x59;
    constexpr int Z       = 0x5A;
}

} // namespace MuInput
