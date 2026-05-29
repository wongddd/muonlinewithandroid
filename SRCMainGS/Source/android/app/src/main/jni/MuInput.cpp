#include "MuInput.h"

#include <android/log.h>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <string>
#include <mutex>

#define LOG_TAG "MuInput"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace MuInput {

// ============================================================================
// 内部常量
// ============================================================================

static constexpr int   MAX_TOUCHES           = 10;
static constexpr float JOYSTICK_ZONE_FRAC    = 0.50f;   // 左 50% = 摇杆区 (增大)
static constexpr float JOYSTICK_DEAD_ZONE    = 30.0f;   // 摇杆死区 (像素)
static constexpr float JOYSTICK_MAX_RADIUS   = 200.0f;  // 摇杆最大半径 (适配高分辨率)
static constexpr float PINCH_WHEEL_FACTOR    = 0.08f;   // 缩放→滚轮系数 (更灵敏)
static constexpr float DBL_CLICK_TIME_MS     = 400.0f;  // 双击间隔
static constexpr float DBL_CLICK_DIST        = 30.0f;   // 双击最大位移

// ============================================================================
// 状态变量
// ============================================================================

// 虚拟键盘状态表 (256 键, 等价于 INPUTSTATEINFO m_pInputInfo[256])
static InputStateInfo g_keyState[256];

// 屏幕尺寸
static int g_screenWidth  = 1920;
static int g_screenHeight = 1080;

// 文本编辑模式 (禁止游戏按键)
static bool g_textEditMode = false;

// ============================================================================
// Touch 状态
// ============================================================================

enum TouchRole : uint8_t {
    ROLE_NONE    = 0,
    ROLE_JOYSTICK = 1,
    ROLE_BUTTON   = 2,
    ROLE_MOUSE    = 3,
};

struct TouchPoint {
    int   pointerId = -1;
    float startX    = 0.0f;   // 按下时的坐标
    float startY    = 0.0f;
    float x         = 0.0f;   // 当前坐标
    float y         = 0.0f;
    float prevX     = 0.0f;   // 上一帧坐标
    float prevY     = 0.0f;
    bool  active    = false;
    TouchRole role  = ROLE_NONE;
    int   vkCode    = 0;      // 如果是按钮, 映射的虚拟键码
};

static TouchPoint g_touches[MAX_TOUCHES];

// ============================================================================
// 虚拟摇杆
// ============================================================================

static bool  g_joystickActive   = false;
static int   g_joystickPointer  = -1;
static float g_joystickOriginX  = 0.0f;
static float g_joystickOriginY  = 0.0f;
static float g_joystickAngle    = 0.0f;
static float g_joystickDistance = 0.0f;

// 上一帧的 WASD 状态 (用于检测变化)
static uint8_t g_prevWASD[4] = {0}; // W, A, S, D

// ============================================================================
// 鼠标状态 (等价于 CInput 的 m_ptCursor, m_lDX, m_lDY, m_b*Btn*)
// ============================================================================

static int  g_mouseX       = 0;
static int  g_mouseY       = 0;
static int  g_mousePrevX   = 0;
static int  g_mousePrevY   = 0;
static int  g_mouseDX      = 0;
static int  g_mouseDY      = 0;
static int  g_mouseWheel   = 0;

// 左键
static bool g_mouseLButtonDn     = false;
static bool g_mouseLButtonHeld   = false;
static bool g_mouseLButtonUp     = false;
static bool g_mouseLButtonDbl    = false;
static bool g_mouseLPrevButton   = false;

// 右键
static bool g_mouseRButtonDn     = false;
static bool g_mouseRButtonHeld   = false;
static bool g_mouseRButtonUp     = false;
static bool g_mouseRButtonDbl    = false;
static bool g_mouseRPrevButton   = false;

// 中键
static bool g_mouseMButtonDn     = false;
static bool g_mouseMButtonHeld   = false;
static bool g_mouseMButtonUp     = false;
static bool g_mouseMPrevButton   = false;

// 双击检测
static int   g_lastClickButton   = 0;
static float g_lastClickTimeMs   = 0.0f;
static float g_lastClickX        = 0.0f;
static float g_lastClickY        = 0.0f;
static uint64_t g_frameTimeMs    = 0;

// 鼠标模式下的 pointerId
static int   g_mousePointer      = -1;

// 双指缩放
static int   g_pinchPointer1     = -1;
static int   g_pinchPointer2     = -1;
static float g_pinchLastDist     = 0.0f;

// ============================================================================
// 触摸按钮布局 (屏幕右 60% → 5列 × 4行)
// ============================================================================

struct TouchButton {
    int   vkCode;       // 虚拟键码
    int   col;          // 列 (0-4)
    int   row;          // 行 (0-3)
    float widthFrac;    // 按钮宽度 (相对列宽)
    float heightFrac;   // 按钮高度 (相对行高)
};

// 右侧按钮面板: 4列 × 4行 (更少的列 = 更大的按钮)
// 面板区域: x=[50%..100%], y=[0..100%]
// 列宽 = 面板宽 / 4, 行高 = 面板高 / 4
static const TouchButton g_buttonLayout[] = {
    // Row 0: 技能快捷键
    { VK::KEY_1, 0, 0 }, { VK::KEY_2, 1, 0 }, { VK::KEY_3, 2, 0 }, { VK::KEY_4, 3, 0 },
    // Row 1: 技能 + 药水
    { VK::KEY_5, 0, 1 }, { VK::KEY_6, 1, 1 }, { VK::KEY_7, 2, 1 }, { VK::KEY_8, 3, 1 },
    // Row 2: UI (I=背包, C=角色, M=地图, Q=指令)
    { VK::I, 0, 2 }, { VK::C, 1, 2 }, { VK::M, 2, 2 }, { VK::Q, 3, 2 },
    // Row 3: 操作 (Ctrl=攻击, Space=跳跃/拾取, Tab=小地图, T=组队)
    { VK::CONTROL, 0, 3 }, { VK::SPACE, 1, 3 }, { VK::TAB, 2, 3 }, { VK::T, 3, 3 },
};

static constexpr int BUTTON_COUNT = sizeof(g_buttonLayout) / sizeof(g_buttonLayout[0]);

// ============================================================================
// 内部函数
// ============================================================================

static int findFreeTouchSlot() {
    for (int i = 0; i < MAX_TOUCHES; ++i) {
        if (!g_touches[i].active) return i;
    }
    return -1;
}

static int findTouchByPointer(int pointerId) {
    for (int i = 0; i < MAX_TOUCHES; ++i) {
        if (g_touches[i].active && g_touches[i].pointerId == pointerId) return i;
    }
    return -1;
}

static int findTouchByRole(TouchRole role) {
    for (int i = 0; i < MAX_TOUCHES; ++i) {
        if (g_touches[i].active && g_touches[i].role == role) return i;
    }
    return -1;
}

// 查找哪个触摸按钮被按下 (x, y 在屏幕坐标)
static int hitTestButton(float x, float y) {
    float panelX      = g_screenWidth * JOYSTICK_ZONE_FRAC;
    float panelWidth  = g_screenWidth - panelX;
    float panelHeight = static_cast<float>(g_screenHeight);

    if (x < panelX) return -1; // 不在按钮面板区域

    float colW = panelWidth / 4.0f;  // 4 columns for bigger buttons
    float rowH = panelHeight / 4.0f;

    float relX = x - panelX;
    int   col  = static_cast<int>(relX / colW);
    int   row  = static_cast<int>(y / rowH);

    if (col < 0 || col >= 4 || row < 0 || row >= 4) return -1;

    // 线性查找匹配的按钮
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        if (g_buttonLayout[i].col == col && g_buttonLayout[i].row == row) {
            return i;
        }
    }
    return -1;
}

// ============================================================================
// 摇杆 → WASD 映射
// ============================================================================

static void updateJoystickKeys(float angle, float distance) {
    uint8_t wasd[4] = {0}; // W, A, S, D

    if (distance > JOYSTICK_DEAD_ZONE) {
        const float PI = 3.14159265359f;

        // 8 方向映射:
        //     W (上):     -135° .. -45°   (即 -PI*3/4 .. -PI/4)
        //     S (下):     45° .. 135°     (即 PI/4 .. PI*3/4)
        //     A (左):     135° .. -135°   (即 > PI/2 或 < -PI/2)
        //     D (右):     -45° .. 45°     (即 > -PI/2 且 < PI/2)

        // 上 (W) : angle ∈ (-135°, -45°)
        if (angle > -PI * 0.75f && angle < -PI * 0.25f) wasd[0] = 1;

        // 下 (S) : angle ∈ (45°, 135°)
        if (angle > PI * 0.25f && angle < PI * 0.75f) wasd[1] = 1;

        // 左 (A) : angle ∈ (135°, 180°] or [-180°, -135°)
        if (angle > PI * 0.5f || angle < -PI * 0.5f) wasd[2] = 1;

        // 右 (D) : angle ∈ (-45°, 45°)
        if (angle > -PI * 0.5f && angle < PI * 0.5f) wasd[3] = 1;
    }

    static const int vkMap[4] = { VK::W, VK::S, VK::A, VK::D };

    for (int i = 0; i < 4; ++i) {
        if (wasd[i] != g_prevWASD[i]) {
            if (wasd[i]) {
                pressVirtualKey(vkMap[i]);
            } else {
                releaseVirtualKey(vkMap[i]);
            }
            g_prevWASD[i] = wasd[i];
        }
    }
}

// ============================================================================
// 双指缩放 → 鼠标滚轮
// ============================================================================

static void updatePinchZoom() {
    int idx1 = findTouchByPointer(g_pinchPointer1);
    int idx2 = findTouchByPointer(g_pinchPointer2);

    if (idx1 < 0 || idx2 < 0) {
        g_pinchPointer1 = -1;
        g_pinchPointer2 = -1;
        g_pinchLastDist = 0.0f;
        return;
    }

    float dx = g_touches[idx1].x - g_touches[idx2].x;
    float dy = g_touches[idx1].y - g_touches[idx2].y;
    float dist = sqrtf(dx * dx + dy * dy);

    if (g_pinchLastDist > 0.0f) {
        float delta = (dist - g_pinchLastDist) * PINCH_WHEEL_FACTOR;
        g_mouseWheel += static_cast<int>(delta);
    }
    g_pinchLastDist = dist;
}

// ============================================================================
// 公开接口实现
// ============================================================================

void init(int screenWidth, int screenHeight) {
    g_screenWidth  = screenWidth;
    g_screenHeight = screenHeight;

    memset(g_keyState, 0, sizeof(g_keyState));
    memset(g_touches, 0, sizeof(g_touches));

    g_mouseX = screenWidth / 2;
    g_mouseY = screenHeight / 2;
    g_mousePrevX = g_mouseX;
    g_mousePrevY = g_mouseY;

    LOGI("MuInput initialized: %dx%d", screenWidth, screenHeight);
}

void setScreenSize(int width, int height) {
    g_screenWidth  = width;
    g_screenHeight = height;
}

// ============================================================================
// 每帧更新
// ============================================================================

void update() {
    // 递增帧时间 (用于双击检测)
    // 每帧 ≈ 16.67ms @ 60fps
    g_frameTimeMs += 17;

    // --- 保存上一帧鼠标位置 ---
    g_mousePrevX = g_mouseX;
    g_mousePrevY = g_mouseY;

    // --- 更新按键状态机 (等价于 ScanAsyncKeyState) ---
    for (int key = 0; key < 256; ++key) {
        switch (g_keyState[key].state) {
            case KEY_PRESS:
                // 刚按下 → 下一帧转为 REPEAT (持续按住)
                g_keyState[key].state = KEY_REPEAT;
                break;

            case KEY_RELEASE:
                // 刚释放 → 下一帧转为 NONE
                g_keyState[key].state = KEY_NONE;
                g_keyState[key].pressTime = 0;
                break;

            case KEY_NONE:
            case KEY_REPEAT:
            default:
                // 保持当前状态
                break;
        }
    }

    // --- 更新触摸位置 (用于拖拽检测) ---
    for (int i = 0; i < MAX_TOUCHES; ++i) {
        if (g_touches[i].active) {
            g_touches[i].prevX = g_touches[i].x;
            g_touches[i].prevY = g_touches[i].y;
        }
    }

    // --- 双指缩放更新 ---
    if (g_pinchPointer1 >= 0 && g_pinchPointer2 >= 0) {
        updatePinchZoom();
    }

    // 鼠标位移清零 (每次 update 都重置)
    g_mouseDX = 0;
    g_mouseDY = 0;
}

// ============================================================================
// Post-frame reset — called AFTER ImGui has read mouse state.
// Resets one-shot mouse button flags (down/up/dbl) and wheel accumulator.
// ============================================================================

void resetOneShotMouseFlags() {
    g_mouseLButtonDn  = false;
    g_mouseLButtonUp  = false;
    g_mouseLButtonDbl = false;
    g_mouseRButtonDn  = false;
    g_mouseRButtonUp  = false;
    g_mouseRButtonDbl = false;
    g_mouseMButtonDn  = false;
    g_mouseMButtonUp  = false;
    g_mouseWheel = 0;
}

// ============================================================================
// Touch 事件
// ============================================================================

void onTouchDown(float x, float y, int pointerId) {
    int slot = findFreeTouchSlot();
    if (slot < 0) return; // 触摸点已满

    TouchPoint& tp = g_touches[slot];
    tp.pointerId = pointerId;
    tp.startX    = x;
    tp.startY    = y;
    tp.x         = x;
    tp.y         = y;
    tp.prevX     = x;
    tp.prevY     = y;
    tp.active    = true;

    // --- 分配角色 ---
    if (x < g_screenWidth * JOYSTICK_ZONE_FRAC && !g_joystickActive) {
        // 左侧 40% → 虚拟摇杆
        tp.role = ROLE_JOYSTICK;
        g_joystickActive   = true;
        g_joystickPointer  = pointerId;
        g_joystickOriginX  = x;
        g_joystickOriginY  = y;
        g_joystickDistance = 0.0f;
        g_joystickAngle    = 0.0f;
        memset(g_prevWASD, 0, sizeof(g_prevWASD));
    } else {
        // Check if touch is in the game UI zone (center area)
        // Touches here are routed as mouse clicks, not button presses,
        // so the game's dialogs (login, inventory, etc.) receive input.
        bool inUIZone = (x > g_screenWidth * 0.35f && x < g_screenWidth * 0.78f &&
                         y > g_screenHeight * 0.12f && y < g_screenHeight * 0.82f);

        int btnIdx = inUIZone ? -1 : hitTestButton(x, y);
        if (btnIdx >= 0) {
            // 右侧按钮区域 → 虚拟按键
            tp.role   = ROLE_BUTTON;
            tp.vkCode = g_buttonLayout[btnIdx].vkCode;
            pressVirtualKey(tp.vkCode);
        } else {
            // 游戏区域 (中央) → 鼠标事件
            tp.role = ROLE_MOUSE;

            // 如果没有其他鼠标触摸点，设置为当前鼠标点
            if (g_mousePointer < 0) {
                g_mousePointer = pointerId;
            }

            // 更新鼠标位置
            g_mouseX = static_cast<int>(x);
            g_mouseY = static_cast<int>(y);

            // 双击检测
            float dx = x - g_lastClickX;
            float dy = y - g_lastClickY;
            float dist = sqrtf(dx * dx + dy * dy);

            if (g_lastClickButton == VK::LBUTTON &&
                (g_frameTimeMs - g_lastClickTimeMs) < DBL_CLICK_TIME_MS &&
                dist < DBL_CLICK_DIST) {
                g_mouseLButtonDbl = true;
                g_lastClickButton = 0;
            } else {
                g_mouseLButtonDn  = true;
                g_mouseLButtonHeld = true;
            }

            g_lastClickTimeMs = static_cast<float>(g_frameTimeMs);
            g_lastClickX      = x;
            g_lastClickY      = y;
            g_lastClickButton = VK::LBUTTON;
        }
    }

    // --- 管理双指缩放 ---
    if (g_pinchPointer1 < 0) {
        g_pinchPointer1 = pointerId;
    } else if (g_pinchPointer2 < 0) {
        g_pinchPointer2 = pointerId;
        // 初始化双指距离
        int idx1 = findTouchByPointer(g_pinchPointer1);
        int idx2 = slot; // 当前触摸点
        float dx2 = g_touches[idx1].x - g_touches[idx2].x;
        float dy2 = g_touches[idx1].y - g_touches[idx2].y;
        g_pinchLastDist = sqrtf(dx2 * dx2 + dy2 * dy2);
    }
}

void onTouchMove(float x, float y, int pointerId) {
    int slot = findTouchByPointer(pointerId);
    if (slot < 0) return;

    TouchPoint& tp = g_touches[slot];
    tp.x = x;
    tp.y = y;

    switch (tp.role) {
        case ROLE_JOYSTICK: {
            float dx = x - g_joystickOriginX;
            float dy = y - g_joystickOriginY;
            float dist = sqrtf(dx * dx + dy * dy);

            // 限制摇杆最大半径
            if (dist > JOYSTICK_MAX_RADIUS) {
                dx *= JOYSTICK_MAX_RADIUS / dist;
                dy *= JOYSTICK_MAX_RADIUS / dist;
                dist = JOYSTICK_MAX_RADIUS;
            }

            g_joystickDistance = dist;
            g_joystickAngle    = atan2f(dy, dx);
            updateJoystickKeys(g_joystickAngle, g_joystickDistance);
            break;
        }
        case ROLE_BUTTON: {
            // 检查手指是否滑出按钮
            int btnIdx = hitTestButton(x, y);
            if (btnIdx < 0 || g_buttonLayout[btnIdx].vkCode != tp.vkCode) {
                // 滑出 → 释放按键
                releaseVirtualKey(tp.vkCode);
            } else {
                // 滑回 → 重新按下
                // (保持按下状态，不需要额外操作)
            }
            break;
        }
        case ROLE_MOUSE: {
            if (pointerId == g_mousePointer) {
                g_mouseX = static_cast<int>(x);
                g_mouseY = static_cast<int>(y);
            }
            break;
        }
        default:
            break;
    }
}

void onTouchUp(float x, float y, int pointerId) {
    int slot = findTouchByPointer(pointerId);
    if (slot < 0) return;

    TouchPoint& tp = g_touches[slot];

    switch (tp.role) {
        case ROLE_JOYSTICK:
            releaseVirtualKey(VK::W);
            releaseVirtualKey(VK::S);
            releaseVirtualKey(VK::A);
            releaseVirtualKey(VK::D);
            memset(g_prevWASD, 0, sizeof(g_prevWASD));
            g_joystickActive  = false;
            g_joystickPointer = -1;
            break;

        case ROLE_BUTTON:
            releaseVirtualKey(tp.vkCode);
            break;

        case ROLE_MOUSE:
            if (pointerId == g_mousePointer) {
                g_mousePointer = -1;
                g_mouseLButtonUp   = true;
                g_mouseLButtonHeld = false;
            }
            break;

        default:
            break;
    }

    // 清理
    tp.active    = false;
    tp.pointerId = -1;
    tp.role      = ROLE_NONE;
    tp.vkCode    = 0;

    // 清理双指缩放
    if (g_pinchPointer1 == pointerId) {
        g_pinchPointer1 = g_pinchPointer2;
        g_pinchPointer2 = -1;
        g_pinchLastDist = 0.0f;
    } else if (g_pinchPointer2 == pointerId) {
        g_pinchPointer2 = -1;
        g_pinchLastDist = 0.0f;
    }
}

// ============================================================================
// 物理键盘事件
// ============================================================================

void onKeyDown(int keyCode) {
    if (keyCode >= 0 && keyCode < 256) {
        pressVirtualKey(keyCode);
    }
}

void onKeyUp(int keyCode) {
    if (keyCode >= 0 && keyCode < 256) {
        releaseVirtualKey(keyCode);
    }
}

// ============================================================================
// 虚拟按键触发
// ============================================================================

void pressVirtualKey(int vkCode) {
    if (vkCode < 0 || vkCode >= 256) return;

    InputStateInfo& info = g_keyState[vkCode];
    if (info.state == KEY_NONE || info.state == KEY_RELEASE) {
        info.state     = KEY_PRESS;
        info.pressTime = static_cast<uint32_t>(g_frameTimeMs);
    }
}

void releaseVirtualKey(int vkCode) {
    if (vkCode < 0 || vkCode >= 256) return;

    InputStateInfo& info = g_keyState[vkCode];
    if (info.state == KEY_PRESS || info.state == KEY_REPEAT) {
        info.state     = KEY_RELEASE;
        info.pressTime = 0;
    }
}

// ============================================================================
// 键盘状态查询
// ============================================================================

bool isKeyDown(int vkCode) {
    if (g_textEditMode) return false;
    if (vkCode < 0 || vkCode >= 256) return false;
    return g_keyState[vkCode].state == KEY_PRESS;
}

bool isKeyHeldDown(int vkCode) {
    if (g_textEditMode) return false;
    if (vkCode < 0 || vkCode >= 256) return false;
    return g_keyState[vkCode].state == KEY_REPEAT;
}

bool isKeyUp(int vkCode) {
    if (vkCode < 0 || vkCode >= 256) return false;
    return g_keyState[vkCode].state == KEY_RELEASE;
}

bool isKeyNone(int vkCode) {
    if (vkCode < 0 || vkCode >= 256) return false;
    return g_keyState[vkCode].state == KEY_NONE;
}

KeyState getKeyState(int vkCode) {
    if (vkCode < 0 || vkCode >= 256) return KEY_NONE;
    return g_keyState[vkCode].state;
}

// ============================================================================
// 鼠标状态查询
// ============================================================================

int getMouseX()        { return g_mouseX; }
int getMouseY()        { return g_mouseY; }
int getMouseDX()       { return g_mouseDX; }
int getMouseDY()       { return g_mouseDY; }
int getMouseWheel()    { return g_mouseWheel; }

bool isMouseLButtonDown()  { return g_mouseLButtonDn; }
bool isMouseLButtonHeld()  { return g_mouseLButtonHeld; }
bool isMouseLButtonUp()    { return g_mouseLButtonUp; }
bool isMouseLButtonDbl()   { return g_mouseLButtonDbl; }

bool isMouseRButtonDown()  { return g_mouseRButtonDn; }
bool isMouseRButtonHeld()  { return g_mouseRButtonHeld; }
bool isMouseRButtonUp()    { return g_mouseRButtonUp; }
bool isMouseRButtonDbl()   { return g_mouseRButtonDbl; }

bool isMouseMButtonDown()  { return g_mouseMButtonDn; }
bool isMouseMButtonHeld()  { return g_mouseMButtonHeld; }
bool isMouseMButtonUp()    { return g_mouseMButtonUp; }

// ============================================================================
// 文本编辑模式
// ============================================================================

void setTextEditMode(bool editMode) { g_textEditMode = editMode; }
bool isTextEditMode()               { return g_textEditMode; }

// ============================================================================
// IME 文本缓冲
// ============================================================================

static std::string g_imeTextBuffer;
static std::mutex  g_imeTextMutex;

void appendIMEText(const char* text) {
    std::lock_guard<std::mutex> lock(g_imeTextMutex);
    g_imeTextBuffer += text;
}

bool hasIMEText() {
    std::lock_guard<std::mutex> lock(g_imeTextMutex);
    return !g_imeTextBuffer.empty();
}

const char* consumeIMEText() {
    // Returns a C string — caller must NOT hold it across frames
    // The returned pointer is valid until the next call to consumeIMEText()
    static std::string s_result;
    std::lock_guard<std::mutex> lock(g_imeTextMutex);
    s_result = std::move(g_imeTextBuffer);
    g_imeTextBuffer.clear();
    return s_result.c_str();
}

// ============================================================================
// 调试
// ============================================================================

int getScreenWidth()     { return g_screenWidth; }
int getScreenHeight()    { return g_screenHeight; }

int getActiveTouchCount() {
    int count = 0;
    for (int i = 0; i < MAX_TOUCHES; ++i) {
        if (g_touches[i].active) ++count;
    }
    return count;
}

} // namespace MuInput
