// Foundation headers (must come before any original game code)
#include "stdafx_port.h"

#include "MuGameLoop.h"
#include "MuGLContext.h"
#include "MuInput.h"
#include "MuAudio.h"
#include "MuFile.h"
#include "MuAssetExtractor.h"
#include "MuNetwork.h"
#include "TimerManager.h"
#include "ProtocolDispatch.h"
#include "ShaderLoader.h"
#include "RenderBatcher.h"
#include "RenderState.h"
#include "TerrainBatcher.h"
#include "SpriteRenderer.h"
#include "MuGLCompat.h"

// Game headers (Main5.2 original code)
#include "ZzzScene.h"
#include "ZzzOpenglUtil.h"
#include "ZzzCharacter.h"
#include "CGMCharacter.h"
#include "CGMModelManager.h"
#include "CharacterManager.h"

// Bridge functions for NewUI window debugging (compiled in NewUICommon.cpp)
extern "C" {
    void NewUI_ShowWindow(int ifaceId, bool show);
    bool NewUI_IsWindowOpen(int ifaceId);
    void NewUI_CloseAllWindows();
    // Computes EncDecKey from Connect.msil, pushes to MuNetwork (defined in CGMProtectStub.cpp)
    void CGMProtect_InitEncDecKey();
}

// Global state (defined in MuGlobalState.cpp)
extern void MuGlobalState_initGL();
extern RenderBatcher* g_glBatcher;

// Full singleton initialization (defined in MuGameInit.cpp)
extern void MuGameInit_FullInit();

#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_gles3.h"

#include <EGL/egl.h>
#include <android/log.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <atomic>
#include <mutex>
#include <algorithm>

#define LOG_TAG "MuGameLoop"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// External globals (defined in ZzzOpenglUtil.h / ZzzOpenglUtil.cpp)
extern unsigned int WindowWidth;
extern unsigned int WindowHeight;
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

// Keyboard show/hide (defined in MuBridge.cpp, overrides weak symbol from UIControls.cpp)
extern void Android_ShowKeyboard(bool show);

// ImGui text delivery helper (exposed for MuBridge.cpp to route IME text to ImGui)
bool ImGui_DeliverText(const char* utf8) {
    if (!utf8 || !utf8[0]) return false;
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput) {
        io.AddInputCharactersUTF8(utf8);
        LOGI("ImGui_DeliverText: delivered '%s' (wantTextInput=%d)", utf8, io.WantTextInput);
        return true;
    }
    LOGI("ImGui_DeliverText: skipped '%s' (wantTextInput=%d)", utf8, io.WantTextInput);
    return false;
}
extern float  MouseX;
extern float  MouseY;
extern float  MouseRenderX;
extern float  MouseRenderY;
extern bool   MouseLButton;
extern bool   MouseLButtonPop;
extern bool   MouseLButtonPush;
extern bool   MouseLButtonDBClick;
extern bool   MouseRButton;
extern bool   MouseRButtonPop;
extern bool   MouseRButtonPush;
extern bool   MouseMButton;
extern bool   MouseMButtonPop;
extern bool   MouseMButtonPush;
extern int    MouseWheel;

// Bridge MuInput → global mouse variables (normally set by WINHANDLE.cpp Win32 message pump)
// Emulates the Windows message pump behavior:
//   - MouseLButtonPush = true on the FIRST frame of press, stays true until release
//     (or until game code consumes it by setting it to false)
//   - MouseLButtonPop = true only on the release frame
//   - MouseLButton = true while held (same as MouseLButtonPush but not consumed)
static void syncMouseFromTouch() {
    // --- Left button ---
    // Track transition state so MouseLButtonPush is only asserted on initial press,
    // allowing game code to consume it (set to false) without re-assertion.
    static bool s_prevLButton = false;
    bool curLButton = MuInput::isMouseLButtonDown() || MuInput::isMouseLButtonHeld();

    if (MuInput::isMouseLButtonDown()) {
        ::MouseLButtonPush = true;       // Just pressed — assert
    } else if (MuInput::isMouseLButtonUp()) {
        ::MouseLButtonPush = false;      // Just released — clear
    }
    // else: held (not first frame) — leave MouseLButtonPush as-is (game may have consumed)

    s_prevLButton = curLButton;
    ::MouseLButton    = curLButton;
    ::MouseLButtonPop = MuInput::isMouseLButtonUp();
    ::MouseLButtonDBClick = MuInput::isMouseLButtonDbl();

    // --- Right button ---
    static bool s_prevRButton = false;
    bool curRButton = MuInput::isMouseRButtonDown() || MuInput::isMouseRButtonHeld();

    if (MuInput::isMouseRButtonDown()) {
        ::MouseRButtonPush = true;
    } else if (MuInput::isMouseRButtonUp()) {
        ::MouseRButtonPush = false;
    }

    s_prevRButton = curRButton;
    ::MouseRButton    = curRButton;
    ::MouseRButtonPop = MuInput::isMouseRButtonUp();

    // --- Middle button ---
    static bool s_prevMButton = false;
    bool curMButton = MuInput::isMouseMButtonDown() || MuInput::isMouseMButtonHeld();

    if (MuInput::isMouseMButtonDown()) {
        ::MouseMButtonPush = true;
    } else if (MuInput::isMouseMButtonUp()) {
        ::MouseMButtonPush = false;
    }

    s_prevMButton = curMButton;
    ::MouseMButton    = curMButton;
    ::MouseMButtonPop = MuInput::isMouseMButtonUp();

    // --- Mouse wheel ---
    ::MouseWheel = MuInput::getMouseWheel();

    // --- Mouse position ---
    ::MouseRenderX = (float)MuInput::getMouseX();
    ::MouseRenderY = (float)MuInput::getMouseY();
    if (g_fScreenRate_x > 0.0f) ::MouseX = ::MouseRenderX / g_fScreenRate_x;
    if (g_fScreenRate_y > 0.0f) ::MouseY = ::MouseRenderY / g_fScreenRate_y;
}

namespace MuGameLoop {

static AAssetManager* g_assetManager = nullptr;
static std::string    g_internalPath;

static std::atomic<bool> g_running{false};
static std::atomic<bool> g_paused{false};
static std::atomic<bool> g_hasSurface{false};

static std::thread g_renderThread;
static std::mutex g_stateMutex;

// Timing
static std::chrono::steady_clock::time_point g_lastFrameTime;
static float g_deltaTime = 0.0f;
static int g_frameCount = 0;
static float g_fpsAccum = 0.0f;
static int g_currentFPS = 0;
static constexpr float TARGET_FRAME_TIME = 1.0f / 60.0f;

// Shader
static GLuint g_defaultProgram = 0;

// Phase 3: Rendering infrastructure
static RenderBatcher* g_batcher = nullptr;
static TerrainBatcher* g_terrainBatcher = nullptr;
static SpriteRenderer* g_spriteRenderer = nullptr;

// ============================================================================
// MuGameInit() — initialization mirroring WinMain's startup sequence
// Creates essential singletons and sets initial game state.
// Called once after GL context, audio, file I/O, and network are ready.
// ============================================================================

static void MuGameInit() {
    LOGI("MuGameInit: Starting game initialization...");

    // Step 1: Create essential singleton instances (originally static locals in WinMain.cpp)
    // CGMCharacter singleton — provides gmCharacters->GetCharacter(N)
    static CGMCharacter gcCharacter;
    gcCharacter.Init();
    LOGI("  CGMCharacter initialized");

    // CGMModelManager singleton — provides model/texture loading
    static CGMModelManager gcModel;
    LOGI("  CGMModelManager initialized");

    // Step 2: Hero pointer — must exist before CAIController(Hero) in FullInit
    Hero = gmCharacters->GetCharacter(0);
    LOGI("  Hero pointer set: %p", (void*)Hero);

    // Step 3: Full singleton initialization (mirrors WinMain lines 1250-1327)
    // Includes GateAttribute, SkillAttribute, CharacterMachine, g_pTimer,
    // g_pChatRoomSocketList, g_pUIManager, g_pUIMapName, CAIController,
    // g_BuffSystem, g_MapProcess, g_petProcess, CUIMng, gTextClien
    MuGameInit_FullInit();

    // Step 4: Set initial scene flag — start at login screen (skip Webzen movie)
    SceneFlag = LOG_IN_SCENE;
    LOGI("  SceneFlag = LOG_IN_SCENE");

    LOGI("MuGameInit: Game initialization complete");
}

// ============================================================================
// 真实 Scene() — 每帧调用原始游戏主循环
// Scene(HDC) 内部调用 BeginOpengl → MainScene → EndOpengl → SwapBuffers
// SwapBuffers 已 stub 为空操作；我们通过 MuGLContext 交换。
// ============================================================================

static void runGameFrame() {
    Scene(nullptr); // HDC is unused in GLES3 path
}

// ============================================================================
// 协议数据包处理 (等价于 ProtocolCompiler + TranslateProtocol)
// ============================================================================

static void processPackets() {
    Packet packet;
    while (MuNetwork::getNextPacket(packet)) {
        ProtocolDispatch::processPacket(packet);
    }
}

// renderTestScene() removed — full game Scene() is now the default rendering path.
// See runGameFrame() below. The sprite/terrain batchers are retained for potential
// use by game UI overlay rendering (HUD elements, minimap terrain previews, etc.).
// ============================================================================
// 主渲染线程 (等价于 winLoop() + Scene() + ProtocolCompiler())
// ============================================================================

static void renderLoop() {
    LOGI("Render thread started");

    while (g_running) {
        if (g_paused || !g_hasSurface) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        auto frameStart = std::chrono::steady_clock::now();
        g_deltaTime = std::chrono::duration<float>(frameStart - g_lastFrameTime).count();
        g_lastFrameTime = frameStart;

        // FPS 统计
        g_frameCount++;
        g_fpsAccum += g_deltaTime;
        if (g_fpsAccum >= 1.0f) {
            g_currentFPS = static_cast<int>(g_frameCount / g_fpsAccum);
            g_frameCount = 0;
            g_fpsAccum = 0.0f;
        }

        // === 原始循环结构映射 ===
        //
        // winLoop() {                          renderLoop() {
        //   while (msg != WM_QUIT) {             while (g_running) {
        //     PeekMessage → GetMessage...          (无消息泵 — 直接调用)
        //     else: Scene(hDC);                    sceneUpdate(deltaTime);
        //     ProtocolCompiler();                  MuNetwork::update();
        //   }                                      processPackets();
        // }                                        TimerManager::update();
        //                                          ProtocolDispatch::update(deltaTime);
        //                                          renderTestScene();  // Phase 7→Scene render
        //                                        }
        //                                      }

        // 1. 定时器更新
        TimerManager::update();

        // 1.5. 将上一帧的触控状态同步到全局鼠标变量
        // 必须在 MuInput::update() 之前调用（update 会重置每帧瞬时状态）
        syncMouseFromTouch();

        // 2. 输入处理 (Touch → 虚拟按键, 重置每帧状态)
        MuInput::update();

        // 3. 网络接收 (等价于 nRecv + 包头解析)
        MuNetwork::update();

        // 4. 数据包分发 (等价于 ProtocolCompiler + TranslateProtocol)
        processPackets();

        // 5. 协议状态机更新
        ProtocolDispatch::update(g_deltaTime);

        // 6. 渲染 (等价于原始 Scene(hDC) + ImGui overlay)
        {
            std::lock_guard<std::mutex> lock(g_stateMutex);
            if (!g_hasSurface) continue;

            // Make EGL context current on render thread
            if (!MuGLContext::makeCurrent()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            // Game scene: update + render
            runGameFrame();

            // ImGui debug + login UI (rendered on top of game scene)
            ImGui_ImplAndroid_NewFrame(MuGLContext::g_width, MuGLContext::g_height);
            ImGui_ImplGLES3_NewFrame();
            ImGui::NewFrame();

            // Task 2: 数据提取进度条 (first launch or when extracting)
            {
                float prog = mu_asset_extractor_get_progress();
                if (prog >= 0.0f) {
                    float scrW = (float)MuGLContext::g_width;
                    float scrH = (float)MuGLContext::g_height;
                    ImGui::SetNextWindowPos(ImVec2((scrW - 600) * 0.5f, scrH * 0.45f), ImGuiCond_Once);
                    ImGui::SetNextWindowSize(ImVec2(600, 120), ImGuiCond_Once);
                    ImGui::Begin("Extracting", nullptr,
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoTitleBar);
                    ImGui::TextColored(ImVec4(1,1,0.6f,1),
                                       "\xe6\xad\xa3\xe5\x9c\xa8\xe6\x8f\x90\xe5\x8f\x96\xe6\xb8\xb8\xe6\x88\x8f\xe6\x95\xb0\xe6\x8d\xae...");
                    // "正在提取游戏数据..."
                    ImGui::ProgressBar(prog, ImVec2(560, 30), "");
                    ImGui::Text("%.0f%%", prog * 100.0f);
                    ImGui::End();
                }
            }

            // SENTINEL_NEW_CODE_ACTIVE
            // Login UI — PC-matching three-phase flow:
            //   Phase 1 (LOG_IN_SCENE): Server selection  (CServerSelWin equivalent)
            //   Phase 2 (LOG_IN_SCENE): Login form         (CLoginWin equivalent)
            //   Phase 3 (CHARACTER_SCENE): Char selection   (CCharSelMainWin equivalent)
            if (SceneFlag == LOG_IN_SCENE) {
                static char serverIp[32] = "197.159.75.59";
                static int  serverPort = 44404;
                static char loginAccount[32] = {};
                static char loginPassword[32] = {};
                static bool fieldsInitialized = false;
                if (!fieldsInitialized) {
                    // Auto-fill saved credentials if available
                    const char* savedAcc = ProtocolDispatch::getSavedAccount();
                    const char* savedPw = ProtocolDispatch::getSavedPassword();
                    if (savedAcc && savedAcc[0]) {
                        strncpy(loginAccount, savedAcc, sizeof(loginAccount) - 1);
                        strncpy(loginPassword, savedPw ? savedPw : "", sizeof(loginPassword) - 1);
                    } else {
                        strncpy(loginAccount, "louismk", sizeof(loginAccount) - 1);
                        strncpy(loginPassword, "1212", sizeof(loginPassword) - 1);
                    }
                    fieldsInitialized = true;
                }

                float scrW = (float)MuGLContext::g_width;
                float scrH = (float)MuGLContext::g_height;
                ProtocolState state = ProtocolDispatch::getState();

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 14));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));

                // =====================================================================
                // Phase 1a: DISCONNECTED — show "Connect" button + server settings
                // =====================================================================
                if (state == ProtocolState::DISCONNECTED ||
                    state == ProtocolState::RECEIVE_JOIN_SERVER_FAIL) {
                    int panelW = (int)(scrW * 0.56f);
                    ImGui::SetNextWindowPos(ImVec2((scrW - panelW) * 0.5f,
                                                   (scrH - 260) * 0.45f), ImGuiCond_Once);
                    ImGui::SetNextWindowSize(ImVec2((float)panelW, 0.0f), ImGuiCond_Once);
                    ImGui::Begin("Connect", nullptr,
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoTitleBar);

                    ImGui::TextColored(ImVec4(1,1,0.6f,1), "MU Online");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::Text("ConnectServer");
                    ImGui::SameLine(560);
                    ImGui::PushItemWidth(400);
                    ImGui::InputText("##csip", serverIp, sizeof(serverIp));
                    ImGui::SameLine();
                    ImGui::PushItemWidth(200);
                    ImGui::InputInt("##csport", &serverPort, 0, 0);

                    ImGui::Spacing();
                    if (ImGui::Button("Connect", ImVec2(panelW - 128, 144))) {
                        LOGI("Connecting to CS %s:%d", serverIp, serverPort);
                        ProtocolDispatch::connectToServer(serverIp, (uint16_t)serverPort);
                    }

                    // Task 6: 资源下载按钮 + 进度
                    {
                        static bool downloadStarted = false;
                        if (!downloadStarted) {
                            if (ImGui::Button("Download Game Data", ImVec2(panelW - 128, 80))) {
                                downloadStarted = true;
                                // Will be implemented with a URL
                                LOGI("Download started");
                            }
                        } else {
                            float prog = -1.0f;
                            if (prog < 0 || prog >= 200) {
                                if (prog >= 200) {
                                    ImGui::TextColored(ImVec4(0.3f,1,0.3f,1), "Extraction complete!");
                                }
                            } else {
                                char pctLabel[32];
                                if (prog <= 100)
                                    snprintf(pctLabel, sizeof(pctLabel), "Downloading... %.0f%%", prog);
                                else
                                    snprintf(pctLabel, sizeof(pctLabel), "Extracting... %.0f%%", prog - 100);
                                ImGui::ProgressBar(prog / 200.0f, ImVec2(panelW - 128, 30), "");
                                ImGui::Text("%s", pctLabel);
                            }
                        }
                    }

                    if (state == ProtocolState::RECEIVE_JOIN_SERVER_FAIL) {
                        const char* err = ProtocolDispatch::getLastError();
                        if (!err || !err[0]) err = "Connection failed — check IP/Port";
                        ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "!! %s", err);
                    }

                    ImGui::End();
                }

                // =====================================================================
                // Phase 1b: WAITING FOR SERVER LIST — show status + servers from CS
                // =====================================================================
                else if (state == ProtocolState::RECEIVE_JOIN_SERVER_WAITING) {
                    const auto& groups = ProtocolDispatch::getServerGroups();
                    int panelW = (int)(scrW * 0.78f);

                    float listH = 80.0f;
                    for (auto& g : groups)
                        listH += 60.0f + (float)g.servers.size() * 48.0f;
                    listH = std::min(listH, 800.0f);
                    if (groups.empty()) listH = 160.0f;
                    float panelH = 140.0f + listH;

                    ImGui::SetNextWindowPos(ImVec2((scrW - panelW) * 0.5f,
                                                   (scrH - panelH) * 0.35f), ImGuiCond_Once);
                    ImGui::SetNextWindowSize(ImVec2((float)panelW, panelH), ImGuiCond_Once);
                    ImGui::Begin("Server Selection", nullptr,
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoTitleBar);

                    ImGui::TextColored(ImVec4(0.3f,0.9f,0.3f,1),
                                       "Connected to ConnectServer");
                    ImGui::Separator();
                    ImGui::Spacing();

                    if (groups.empty()) {
                        ImGui::Text("Requesting server list...");
                    } else {
                        ImGui::Text("Select a server group, then a sub-server:");
                        ImGui::Spacing();
                        for (size_t gi = 0; gi < groups.size(); gi++) {
                            const auto& grp = groups[gi];
                            char groupLabel[80];
                            // Show group name with server count
                            int online = 0, total = (int)grp.servers.size();
                            for (auto& s : grp.servers) if (s.percent < 128) online++;
                            snprintf(groupLabel, sizeof(groupLabel),
                                     "%s  [%d/%d] ##grp%zu", grp.name, online, total, gi);

                            bool groupOpen = ImGui::TreeNodeEx(
                                groupLabel,
                                ImGuiTreeNodeFlags_DefaultOpen |
                                (grp.servers.empty() ? ImGuiTreeNodeFlags_Leaf : 0));

                            if (groupOpen) {
                                for (size_t si = 0; si < grp.servers.size(); si++) {
                                    const auto& sv = grp.servers[si];

                                    // Status icon + color
                                    const char* statusIcon = "";
                                    ImVec4 btnColor = ImVec4(0.3f, 0.7f, 0.3f, 0.5f); // green=normal
                                    if (sv.percent >= 128) {
                                        statusIcon = "\xe2\x9c\x97"; // ✗
                                        btnColor = ImVec4(0.5f, 0.5f, 0.5f, 0.3f); // grey=closed
                                    } else if (sv.percent >= 100) {
                                        statusIcon = "\xe2\x96\xa0"; // ■
                                        btnColor = ImVec4(0.8f, 0.3f, 0.3f, 0.5f); // red=full
                                    } else if (sv.percent >= 80) {
                                        statusIcon = "\xe2\x96\xb2"; // ▲
                                        btnColor = ImVec4(0.8f, 0.6f, 0.2f, 0.5f); // yellow=busy
                                    } else {
                                        statusIcon = "\xe2\x96\xbc"; // ▼
                                    }

                                    ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
                                    char subLabel[80];
                                    snprintf(subLabel, sizeof(subLabel), "%s %s##sv%zu_%zu",
                                             statusIcon, sv.name, gi, si);

                                    if (ImGui::Button(subLabel, ImVec2(panelW - 144, 58))) {
                                        LOGI("Selected: group=%d '%s' sub=%d '%s'",
                                             grp.groupIndex, grp.name,
                                             sv.subIndex, sv.name);
                                        ProtocolDispatch::selectServer((int)gi, (int)si);
                                    }
                                    ImGui::PopStyleColor();
                                }
                                ImGui::TreePop();
                            }
                        }

                        ImGui::Spacing();
                        if (ImGui::Button("Refresh List", ImVec2(panelW - 144, 64))) {
                            ProtocolDispatch::refreshServerList();
                        }
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                    if (ImGui::Button("Disconnect", ImVec2(panelW - 144, 75))) {
                        ProtocolDispatch::disconnect();
                    }

                    ImGui::End();
                }

                // =====================================================================
                // Phase 1c-2: GS_JOIN_REQUESTED — connected to GS, waiting for user input
                //   When F1/00 arrives, show login form (PC LoginWin equivalent)
                // =====================================================================
                else if (state == ProtocolState::GS_JOIN_REQUESTED) {
                    int panelW = (int)(scrW * 0.60f);
                    ImGui::SetNextWindowPos(ImVec2((scrW - panelW) * 0.5f,
                                                   (scrH - 280) * 0.45f), ImGuiCond_Once);
                    ImGui::SetNextWindowSize(ImVec2((float)panelW, 0.0f), ImGuiCond_Once);
                    ImGui::Begin("Login", nullptr,
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoTitleBar);

                    ImGui::TextColored(ImVec4(0.3f,0.9f,0.3f,1),
                                       "Game Server connected");
                    ImGui::Separator();
                    ImGui::Spacing();

                    float labelX = 96.0f;
                    float inputW = panelW - labelX - 128.0f;
                    ImGui::Text("ID");
                    ImGui::SameLine(labelX);
                    ImGui::PushItemWidth(inputW);
                    ImGui::InputText("##id", loginAccount, sizeof(loginAccount));

                    ImGui::Text("PW");
                    ImGui::SameLine(labelX);
                    ImGui::PushItemWidth(inputW - 80);
                    static bool pwShow = false;
                    int pwFlags = pwShow ? 0 : ImGuiInputTextFlags_Password;
                    ImGui::InputText("##pw", loginPassword, sizeof(loginPassword), pwFlags);
                    ImGui::SameLine();
                    if (ImGui::Button(pwShow ? "\xe9\x9a\x90\xe8\x97\x8f" : "\xe6\x98\xbe\xe7\xa4\xba", ImVec2(80, 80))) { // 隐藏/显示
                        pwShow = !pwShow;
                    }

                    {
                        const char* err = ProtocolDispatch::getLastError();
                        if (err && err[0]) {
                            ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "!! %s", err);
                        }
                    }

                    ImGui::Spacing();

                    // Save account checkbox (Task 3)
                    static bool saveAccount = false;
                    ImGui::Checkbox("Save Account", &saveAccount);
                    ImGui::SameLine(280);
                    bool autoRecon = ProtocolDispatch::isAutoReconnectEnabled();
                    if (ImGui::Checkbox("Auto-Reconnect", &autoRecon)) {
                        ProtocolDispatch::enableAutoReconnect(autoRecon);
                    }

                    ImGui::Separator();
                    ImGui::Spacing();

                    float btnWide = panelW - 128.0f - 40.0f;
                    float btnHalf = (btnWide - 12.0f) * 0.5f;
                    if (ImGui::Button("\xe7\x99\xbb\xe5\xbd\x95", ImVec2(btnHalf, 136))) { // 登录
                        LOGI("Sending login: account='%s'", loginAccount);
                        ProtocolDispatch::setCredentials(loginAccount, loginPassword);
                        if (saveAccount) {
                            ProtocolDispatch::saveCredentials(loginAccount, loginPassword);
                        }
                        ProtocolDispatch::sendLogin();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("\xe5\x8f\x96\xe6\xb6\x88", ImVec2(btnHalf, 136))) { // 取消
                        ProtocolDispatch::disconnect();
                    }
                    ImGui::Spacing();
                    // Task 4: 账号注册 — opens a popup modal
                    if (ImGui::Button("\xe6\xb3\xa8\xe5\x86\x8c\xe8\xb4\xa6\xe5\x8f\xb7", ImVec2(btnWide, 80))) {
                        ImGui::OpenPopup("\xe6\xb3\xa8\xe5\x86\x8c\xe8\xb4\xa6\xe5\x8f\xb7");
                    }

                    ImGui::End();
                }

                // =====================================================================
                // Phase 1d: LOGIN IN FLIGHT — waiting for server response
                // =====================================================================
                else if (state == ProtocolState::REQUEST_LOG_IN ||
                         state == ProtocolState::RECEIVE_LOG_IN_SUCCESS ||
                         state == ProtocolState::REQUEST_CHARACTERS_LIST ||
                         state == ProtocolState::RECEIVE_CHARACTERS_LIST) {
                    ImGui::SetNextWindowPos(ImVec2((scrW - scrW*0.52f) * 0.5f, scrH * 0.4f), ImGuiCond_Once);
                    ImGui::SetNextWindowSize(ImVec2(scrW*0.52f, 360), ImGuiCond_Once);
                    ImGui::Begin("Status", nullptr,
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoTitleBar);

                    ImGui::TextColored(ImVec4(1,1,0.6f,1), "\xe7\x99\xbb\xe5\xbd\x95\xe8\xbf\x9b\xe5\xba\xa6"); // 登录进度
                    ImGui::Separator();
                    ImGui::Spacing();

                    // Step-by-step progress display
                    struct StepInfo { const char* label; bool done; bool active; };
                    StepInfo steps[] = {
                        {"\xe8\xbf\x9e\xe6\x8e\xa5 ConnectServer...", // 连接 ConnectServer...
                         state > ProtocolState::RECEIVE_JOIN_SERVER_WAITING,
                         state == ProtocolState::RECEIVE_JOIN_SERVER_WAITING},
                        {"\xe8\x8e\xb7\xe5\x8f\x96\xe6\x9c\x8d\xe5\x8a\xa1\xe5\x99\xa8\xe5\x88\x97\xe8\xa1\xa8...", // 获取服务器列表...
                         false, false},
                        {"\xe8\xbf\x9e\xe6\x8e\xa5\xe6\xb8\xb8\xe6\x88\x8f\xe6\x9c\x8d\xe5\x8a\xa1\xe5\x99\xa8...", // 连接游戏服务器...
                         state >= ProtocolState::GS_JOIN_REQUESTED,
                         state == ProtocolState::RECEIVE_JOIN_SERVER_SUCCESS},
                        {"\xe9\xaa\x8c\xe8\xaf\x81\xe8\xb4\xa6\xe5\x8f\xb7...", // 验证账号...
                         state >= ProtocolState::REQUEST_LOG_IN,
                         state == ProtocolState::GS_JOIN_REQUESTED},
                        {"\xe8\x8e\xb7\xe5\x8f\x96\xe8\xa7\x92\xe8\x89\xb2\xe5\x88\x97\xe8\xa1\xa8...", // 获取角色列表...
                         state >= ProtocolState::RECEIVE_CHARACTERS_LIST,
                         state == ProtocolState::REQUEST_CHARACTERS_LIST},
                        {"\xe5\x8a\xa0\xe8\xbd\xbd\xe6\xb8\xb8\xe6\x88\x8f\xe4\xb8\x96\xe7\x95\x8c...", // 加载游戏世界...
                         false, false},
                    };

                    for (auto& s : steps) {
                        if (s.done) {
                            ImGui::TextColored(ImVec4(0.3f,1,0.3f,1), "\xe2\x9c\x93 %s", s.label); // ✓
                        } else if (s.active) {
                            ImGui::TextColored(ImVec4(1,1,0.3f,1), "\xe2\x96\xb6 %s", s.label); // ▶
                        } else {
                            ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1), "   %s", s.label);
                        }
                    }

                    ImGui::Spacing();
                    ImGui::Text("\xe5\xbd\x93\xe5\x89\x8d\xe7\x8a\xb6\xe6\x80\x81: %s", // 当前状态:
                               ProtocolDispatch::getStateName());
                    ImGui::End();
                }

                else if (state == ProtocolState::RECEIVE_JOIN_SERVER_SUCCESS) {
                    ImGui::SetNextWindowPos(ImVec2((scrW - scrW*0.52f) * 0.5f, scrH * 0.4f), ImGuiCond_Once);
                    ImGui::SetNextWindowSize(ImVec2(scrW*0.52f, 240), ImGuiCond_Once);
                    ImGui::Begin("Status", nullptr,
                                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoTitleBar);
                    ImGui::Text("Connecting to Game Server...");
                    ImGui::End();
                }

                ImGui::PopStyleVar(2);
            }

            // =====================================================================
            // 注册面板 (Task 4: CB_DangKyInGame 等价)
            // =====================================================================
            if (ImGui::BeginPopupModal("\xe6\xb3\xa8\xe5\x86\x8c\xe8\xb4\xa6\xe5\x8f\xb7", nullptr,
                                       ImGuiWindowFlags_NoResize)) {
                static char regAccount[32] = {};
                static char regPassword[32] = {};
                static char regConfirm[32] = {};
                static char regMsg[128] = {};
                static int  regMsgColor = 0; // 0=normal, 1=green, 2=red

                ImGui::Text("\xe8\xb4\xa6\xe5\x8f\xb7:"); ImGui::SameLine(200);
                ImGui::PushItemWidth(400);
                ImGui::InputText("##regacc", regAccount, sizeof(regAccount));

                ImGui::Text("\xe5\xaf\x86\xe7\xa0\x81:"); ImGui::SameLine(200);
                ImGui::PushItemWidth(400);
                ImGui::InputText("##regpw", regPassword, sizeof(regPassword),
                                 ImGuiInputTextFlags_Password);

                ImGui::Text("\xe7\xa1\xae\xe8\xae\xa4\xe5\xaf\x86\xe7\xa0\x81:"); ImGui::SameLine(200);
                ImGui::PushItemWidth(400);
                ImGui::InputText("##regcf", regConfirm, sizeof(regConfirm),
                                 ImGuiInputTextFlags_Password);

                // Show status message
                if (regMsg[0]) {
                    ImVec4 color = (regMsgColor == 1) ? ImVec4(0.3f,1,0.3f,1)
                                 : (regMsgColor == 2) ? ImVec4(1,0.3f,0.3f,1)
                                 : ImVec4(1,1,1,1);
                    ImGui::TextColored(color, "%s", regMsg);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Button("\xe6\x8f\x90\xe4\xba\xa4", ImVec2(280, 60))) {
                    if (!regAccount[0]) {
                        snprintf(regMsg, sizeof(regMsg), "\xe8\xaf\xb7\xe8\xbe\x93\xe5\x85\xa5\xe8\xb4\xa6\xe5\x8f\xb7"); // 请输入账号
                        regMsgColor = 2;
                    } else if (strlen(regPassword) < 4) {
                        snprintf(regMsg, sizeof(regMsg), "\xe5\xaf\x86\xe7\xa0\x81\xe8\x87\xb3\xe5\xb0\x91" "4\xe4\xbd\x8d");
                        regMsgColor = 2;
                    } else if (strcmp(regPassword, regConfirm) != 0) {
                        snprintf(regMsg, sizeof(regMsg), "\xe4\xb8\xa4\xe6\xac\xa1\xe5\xaf\x86\xe7\xa0\x81\xe4\xb8\x8d\xe4\xb8\x80\xe8\x87\xb4"); // 两次密码不一致
                        regMsgColor = 2;
                    } else {
                        LOGI("Register account: '%s'", regAccount);
                        ProtocolDispatch::sendRegister(regAccount, regPassword);
                        // Show result from ProtocolDispatch
                        const char* res = ProtocolDispatch::getRegResult();
                        if (res && res[0]) {
                            snprintf(regMsg, sizeof(regMsg), "%s", res);
                            regMsgColor = ProtocolDispatch::getRegResultColor();
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("\xe5\x8f\x96\xe6\xb6\x88", ImVec2(280, 60))) {
                    regAccount[0] = '\0';
                    regPassword[0] = '\0';
                    regConfirm[0] = '\0';
                    regMsg[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            // =====================================================================
            // Phase 3: CHARACTER_SCENE — character selection + create/delete
            // =====================================================================
            if (SceneFlag == CHARACTER_SCENE) {
                const auto& chars = ProtocolDispatch::getCharacterList();
                float scrW = (float)MuGLContext::g_width;
                float scrH = (float)MuGLContext::g_height;
                int panelW = (int)(scrW * 0.50f);

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(48, 42));
                ImGui::SetNextWindowPos(ImVec2((scrW - panelW) * 0.5f, scrH * 0.35f), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2((float)panelW, 0.0f), ImGuiCond_Once);
                ImGui::Begin("Select Character", nullptr,
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoTitleBar);

                ImGui::TextColored(ImVec4(1,1,0.6f,1), "\xe9\x80\x89\xe6\x8b\xa9\xe8\xa7\x92\xe8\x89\xb2"); // 选择角色
                ImGui::Separator();
                ImGui::Spacing();

                if (chars.empty()) {
                    ImGui::Text("\xe6\xb2\xa1\xe6\x9c\x89\xe8\xa7\x92\xe8\x89\xb2"); // 没有角色
                } else {
                    for (size_t i = 0; i < chars.size(); i++) {
                        const auto& c = chars[i];
                        const char* className = "Unknown";
                        switch (c.classCode) {
                            case 0x00: className = "\xe9\xbb\x91\xe6\x9a\x97\xe6\xb3\x95\xe5\xb8\x88"; break; // 暗黑法师
                            case 0x10: className = "\xe9\xbb\x91\xe6\x9a\x97\xe6\x88\x98\xe5\xa3\xab"; break; // 暗黑战士
                            case 0x20: className = "\xe7\xb2\xbe\xe7\x81\xb5\xe5\xb7\xab\xe5\xb8\x88"; break; // 精灵巫师
                            case 0x30: className = "\xe9\xad\x94\xe5\x89\x91\xe5\xa3\xab"; break; // 魔剑士
                            case 0x40: className = "\xe9\xbb\x91\xe6\x9a\x97\xe9\xa2\x86\xe4\xb8\xbb"; break; // 暗黑领主
                            case 0x50: className = "\xe5\x8f\xac\xe5\x94\xa4\xe5\xb7\xab\xe5\xb8\x88"; break; // 召唤巫师
                            case 0x60: className = "\xe6\x9a\xb4\xe6\x80\x92\xe6\x88\x98\xe5\xa3\xab"; break; // 暴怒战士
                        }
                        char label[64];
                        snprintf(label, sizeof(label), "%s  Lv.%d  %s",
                                 c.name, c.level, className);

                        if (c.ctlCode != 0) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f,0.6f,0.6f,1));
                            ImGui::Text("%s  [\xe5\xb0\x81\xe5\x8f\xb7]", label); // [封号]
                            ImGui::PopStyleColor();
                        } else {
                            // Select character to enter game
                            if (ImGui::Button(label, ImVec2(panelW - 360, 90))) {
                                LOGI("Selected character: '%s'", c.name);
                                ProtocolDispatch::selectCharacter((int)i);
                            }
                            // Delete button
                            ImGui::SameLine();
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f,0.2f,0.2f,0.5f));
                            char delLabel[32];
                            snprintf(delLabel, sizeof(delLabel), "\xe5\x88\xa0\xe9\x99\xa4##del%zu", i);
                            if (ImGui::Button(delLabel, ImVec2(180, 90))) {
                                LOGI("Delete character: '%s'", c.name);
                                ProtocolDispatch::deleteCharacter((int)i);
                            }
                            ImGui::PopStyleColor();
                        }
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Create character section
                static char newCharName[11] = {};
                static int  newCharClass = 0;
                static char createMsg[128] = {};
                static int  createMsgColor = 0;

                ImGui::Text("\xe5\x88\x9b\xe5\xbb\xba\xe8\xa7\x92\xe8\x89\xb2:"); // 创建角色:
                ImGui::SameLine();
                ImGui::PushItemWidth(300);
                ImGui::InputText("##newname", newCharName, sizeof(newCharName));
                ImGui::SameLine();
                ImGui::PushItemWidth(200);

                // Class selection with correct PC class codes
                // DW=0x00, DK=0x10, EE=0x20, MG=0x30, DL=0x40, SU=0x50, RF=0x60
                struct ClassItem { const char* name; uint8_t code; };
                static const ClassItem classItems[] = {
                    {"\xe9\xbb\x91\xe6\x9a\x97\xe6\xb3\x95\xe5\xb8\x88", 0x00}, // Dark Wizard
                    {"\xe9\xbb\x91\xe6\x9a\x97\xe6\x88\x98\xe5\xa3\xab", 0x10}, // Dark Knight
                    {"\xe7\xb2\xbe\xe7\x81\xb5\xe5\xb7\xab\xe5\xb8\x88", 0x20}, // Fairy Elf
                    {"\xe9\xad\x94\xe5\x89\x91\xe5\xa3\xab", 0x30}, // Magic Gladiator
                    {"\xe9\xbb\x91\xe6\x9a\x97\xe9\xa2\x86\xe4\xb8\xbb", 0x40}, // Dark Lord
                    {"\xe5\x8f\xac\xe5\x94\xa4\xe5\xb7\xab\xe5\xb8\x88", 0x50}, // Summoner
                    {"\xe6\x9a\xb4\xe6\x80\x92\xe6\x88\x98\xe5\xa3\xab", 0x60}, // Rage Fighter
                };
                const int classCount = sizeof(classItems) / sizeof(classItems[0]);
                static const char* classNames[classCount];
                for (int ci = 0; ci < classCount; ci++) classNames[ci] = classItems[ci].name;
                ImGui::Combo("##class", &newCharClass, classNames, classCount);

                if (createMsg[0]) {
                    ImVec4 cc = (createMsgColor == 1) ? ImVec4(0.3f,1,0.3f,1)
                               : (createMsgColor == 2) ? ImVec4(1,0.3f,0.3f,1)
                               : ImVec4(1,1,1,1);
                    ImGui::TextColored(cc, "%s", createMsg);
                }

                ImGui::SameLine();
                if (ImGui::Button("\xe5\x88\x9b\xe5\xbb\xba", ImVec2(180, 84))) {
                    if (!newCharName[0]) {
                        snprintf(createMsg, sizeof(createMsg), "\xe8\xaf\xb7\xe8\xbe\x93\xe5\x85\xa5\xe8\xa7\x92\xe8\x89\xb2\xe5\x90\x8d"); // 请输入角色名
                        createMsgColor = 2;
                    } else if (strlen(newCharName) < 2) {
                        snprintf(createMsg, sizeof(createMsg), "\xe8\xa7\x92\xe8\x89\xb2\xe5\x90\x8d\xe8\x87\xb3\xe5\xb0\x91" "2\xe4\xb8\xaa\xe5\xad\x97"); // 角色名至少2个字
                        createMsgColor = 2;
                    } else {
                        LOGI("Create character: '%s' class=%s(0x%02X)",
                             newCharName, classItems[newCharClass].name,
                             classItems[newCharClass].code);
                        ProtocolDispatch::createCharacter(newCharName,
                            classItems[newCharClass].code);
                        snprintf(createMsg, sizeof(createMsg),
                                 "\xe5\x88\x9b\xe5\xbb\xba\xe8\xaf\xb7\xe6\xb1\x82\xe5\xb7\xb2\xe5\x8f\x91\xe9\x80\x81"); // 创建请求已发送
                        createMsgColor = 1;
                        newCharName[0] = '\0';
                    }
                }

                // Show create/delete result from server
                {
                    const char* cr = ProtocolDispatch::getCreateCharResult();
                    if (cr && cr[0]) {
                        int cc = ProtocolDispatch::getCreateCharResultColor();
                        ImVec4 col = (cc == 1) ? ImVec4(0.3f,1,0.3f,1) : ImVec4(1,0.3f,0.3f,1);
                        ImGui::TextColored(col, "%s", cr);
                    }
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                if (ImGui::Button("\xe6\x96\xad\xe5\xbc\x80\xe8\xbf\x9e\xe6\x8e\xa5", ImVec2(panelW - 96, 84))) {
                    ProtocolDispatch::disconnect();
                    SceneFlag = LOG_IN_SCENE;
                }

                ImGui::End();
                ImGui::PopStyleVar(1);
            }

            // =====================================================================
            // Task 1: LOADING_SCENE — loading screen between char select and game
            // =====================================================================
            if (SceneFlag == LOADING_SCENE) {
                float scrW = (float)MuGLContext::g_width;
                float scrH = (float)MuGLContext::g_height;
                ImGui::SetNextWindowPos(ImVec2((scrW - 600) * 0.5f, scrH * 0.4f), ImGuiCond_Once);
                ImGui::SetNextWindowSize(ImVec2(scrW*0.26f, 160), ImGuiCond_Once);
                ImGui::Begin("Loading", nullptr,
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoTitleBar);
                ImGui::TextColored(ImVec4(0.3f,1.0f,0.3f,1), "Entering game world...");
                ImGui::Text("(%s)", ProtocolDispatch::getStateName());
                ImGui::End();
            }

            // NewUI Window Debug Panel — only in MAIN_SCENE (hide during login for performance)
            if (SceneFlag == MAIN_SCENE) {
                ImGui::SetNextWindowPos(ImVec2(MuGLContext::g_width - 370.f, 90.f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(360, 520), ImGuiCond_FirstUseEver);
                ImGui::Begin("NewUI Windows", nullptr, ImGuiWindowFlags_NoCollapse);

                // Helper lambda: toggle button for a window
                auto ToggleWin = [](const char* label, int ifaceId) {
                    bool open = NewUI_IsWindowOpen(ifaceId);
                    if (ImGui::Checkbox(label, &open)) {
                        NewUI_ShowWindow(ifaceId, open);
                    }
                };

                if (ImGui::CollapsingHeader("HUD / Main", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ToggleWin("MainFrame", SEASON3B::INTERFACE_MAINFRAME);
                    ToggleWin("MiniMap", SEASON3B::INTERFACE_MINI_MAP);
                    ToggleWin("BuffWindow", SEASON3B::INTERFACE_BUFF_WINDOW);
                    ToggleWin("SkillList", SEASON3B::INTERFACE_SKILL_LIST);
                    ToggleWin("HotKey", SEASON3B::INTERFACE_HOTKEY);
                    ToggleWin("ChatInputBox", SEASON3B::INTERFACE_CHATINPUTBOX);
                    ToggleWin("ChatLogWindow", SEASON3B::INTERFACE_CHATLOGWINDOW);
                    ToggleWin("Option", SEASON3B::INTERFACE_OPTION);
                    ToggleWin("Help", SEASON3B::INTERFACE_HELP);
                    ToggleWin("Command", SEASON3B::INTERFACE_COMMAND);
                    ToggleWin("QuickCommand", SEASON3B::INTERFACE_QUICK_COMMAND);
                    ToggleWin("Macro", SEASON3B::INTERFACE_MACRO_OFICIAL);
                    ToggleWin("WindowMenu", SEASON3B::INTERFACE_WINDOW_MENU);
                }
                if (ImGui::CollapsingHeader("Character / Inventory")) {
                    ToggleWin("Character", SEASON3B::INTERFACE_CHARACTER);
                    ToggleWin("Inventory", SEASON3B::INTERFACE_INVENTORY);
                    ToggleWin("InventoryJewel", SEASON3B::INTERFACE_INVENTORY_JEWEL);
                    ToggleWin("InventoryCore", SEASON3B::INTERFACE_INVENTORY_CORE);
                    ToggleWin("InvExtension", SEASON3B::INTERFACE_INVENTORY_EXTENSION);
                    ToggleWin("Storage", SEASON3B::INTERFACE_STORAGE);
                    ToggleWin("StorageExt", SEASON3B::INTERFACE_STORAGE_EXTENSION);
                    ToggleWin("MixInventory", SEASON3B::INTERFACE_MIXINVENTORY);
                    ToggleWin("Trade", SEASON3B::INTERFACE_TRADE);
                    ToggleWin("ItemTooltip", SEASON3B::INTERFACE_ITEM_TOOLTIP);
                    ToggleWin("ItemExplain", SEASON3B::INTERFACE_ITEM_EXPLANATION);
                    ToggleWin("SetItemExplain", SEASON3B::INTERFACE_SETITEM_EXPLANATION);
                    ToggleWin("ItemEndurance", SEASON3B::INTERFACE_ITEM_ENDURANCE_INFO);
                }
                if (ImGui::CollapsingHeader("Social / Party")) {
                    ToggleWin("Friend", SEASON3B::INTERFACE_FRIEND);
                    ToggleWin("Party", SEASON3B::INTERFACE_PARTY);
                    ToggleWin("PartyInfo", SEASON3B::INTERFACE_PARTY_INFO_WINDOW);
                    ToggleWin("GuildInfo", SEASON3B::INTERFACE_GUILDINFO);
                    ToggleWin("Duel", SEASON3B::INTERFACE_DUEL_WINDOW);
                    ToggleWin("DuelWatch", SEASON3B::INTERFACE_DUELWATCH);
                    ToggleWin("ChatBlocked", SEASON3B::INTERFACE_CHAT_BLOCKED);
                    ToggleWin("GensRanking", SEASON3B::INTERFACE_GENSRANKING);
                }
                if (ImGui::CollapsingHeader("NPC / Quest")) {
                    ToggleWin("NPCDialogue", SEASON3B::INTERFACE_NPC_DIALOGUE);
                    ToggleWin("NPCShop", SEASON3B::INTERFACE_NPCSHOP);
                    ToggleWin("NPCBreeder", SEASON3B::INTERFACE_NPCBREEDER);
                    ToggleWin("GateKeeper", SEASON3B::INTERFACE_GATEKEEPER);
                    ToggleWin("GuildMaster", SEASON3B::INTERFACE_NPCGUILDMASTER);
                    ToggleWin("Guardsman", SEASON3B::INTERFACE_GUARDSMAN);
                    ToggleWin("Senatus", SEASON3B::INTERFACE_SENATUS);
                    ToggleWin("QuestProgress", SEASON3B::INTERFACE_QUEST_PROGRESS);
                    ToggleWin("MyQuest", SEASON3B::INTERFACE_MYQUEST);
                    ToggleWin("NPCQuest", SEASON3B::INTERFACE_NPCQUEST);
                    ToggleWin("Refinery", SEASON3B::INTERFACE_REFINERY);
                    ToggleWin("RefineryInfo", SEASON3B::INTERFACE_REFINERYINFO);
                    ToggleWin("LuckyCoinReg", SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION);
                    ToggleWin("ExchangeLuckyCoin", SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);
                    ToggleWin("LuckyItemWnd", SEASON3B::INTERFACE_LUCKYITEMWND);
                }
                if (ImGui::CollapsingHeader("Events / PvP")) {
                    ToggleWin("BloodCastle", SEASON3B::INTERFACE_BLOODCASTLE);
                    ToggleWin("DevilSquare", SEASON3B::INTERFACE_DEVILSQUARE);
                    ToggleWin("ChaosCastle", SEASON3B::INTERFACE_CHAOSCASTLE_TIME);
                    ToggleWin("CryWolf", SEASON3B::INTERFACE_CRYWOLF);
                    ToggleWin("Doppelganger", SEASON3B::INTERFACE_DOPPELGANGER_FRAME);
                    ToggleWin("EmpireGuardian", SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
                    ToggleWin("SiegeWarfare", SEASON3B::INTERFACE_SIEGEWARFARE);
                    ToggleWin("KanturuInfo", SEASON3B::INTERFACE_KANTURU_INFO);
                    ToggleWin("BattleSoccer", SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE);
                    ToggleWin("CursedTemple", SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
                    ToggleWin("GoldBowman", SEASON3B::INTERFACE_GOLD_BOWMAN);
                }
                if (ImGui::CollapsingHeader("Shop / Market")) {
                    ToggleWin("InGameShop", SEASON3B::INTERFACE_INGAMESHOP);
                    ToggleWin("MyShop", SEASON3B::INTERFACE_MYSHOP_INVENTORY);
                    ToggleWin("PurchaseShop", SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
                    ToggleWin("UnitedMarket", SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
                }
                if (ImGui::CollapsingHeader("System / Misc")) {
                    ToggleWin("MoveMap", SEASON3B::INTERFACE_MOVEMAP);
                    ToggleWin("ServerDivision", SEASON3B::INTERFACE_SERVERDIVISION);
                    ToggleWin("MasterLevel", SEASON3B::INTERFACE_MASTER_LEVEL);
                    ToggleWin("Pet", SEASON3B::INTERFACE_PET);
                    ToggleWin("SlideWindow", SEASON3B::INTERFACE_SLIDEWINDOW);
                    ToggleWin("NameWindow", SEASON3B::INTERFACE_NAME_WINDOW);
                    ToggleWin("HeroPosInfo", SEASON3B::INTERFACE_HERO_POSITION_INFO);
                    ToggleWin("Reconnect", SEASON3B::INTERFACE_RECONNECT);
                    ToggleWin("CustomMenu", SEASON3B::INTERFACE_CUSTOM_MENU);
                    ToggleWin("EventTime", SEASON3B::INTERFACE_EVENT_TIME);
                    ToggleWin("RankingTop", SEASON3B::INTERFACE_RANKING_TOP);
                    ToggleWin("ShowVip", SEASON3B::INTERFACE_SHOW_VIP);
                }

                ImGui::Separator();
                if (ImGui::Button("Close All Windows", ImVec2(150, 25))) {
                    NewUI_CloseAllWindows();
                }

                ImGui::End();
            }

            // Performance: only show debug overlay in MAIN_SCENE (not during login)
            if (SceneFlag == MAIN_SCENE) {
                ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(180, 80), ImGuiCond_FirstUseEver);
                ImGui::Begin("Debug", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse);
                ImGui::Text("FPS: %d (%.1f ms)", g_currentFPS, g_deltaTime * 1000.0f);
                ImGui::Text("Scene: %d", static_cast<int>(SceneFlag));
                ImGui::Text("Net: %s", MuNetwork::isConnected() ? "OK" : "--");
                ImGui::End();
            }

            // Touch overlay: show joystick + button indicators in MAIN_SCENE
            if (SceneFlag == MAIN_SCENE) {
                float scrW = (float)MuGLContext::g_width;
                float scrH = (float)MuGLContext::g_height;

                // Joystick zone indicator (semi-transparent)
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(ImVec2(scrW * 0.5f, scrH));
                ImGui::Begin("JoyOverlay", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
                             ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
                if (MuInput::isJoystickActive()) {
                    float ox = MuInput::getJoystickOriginX();
                    float oy = MuInput::getJoystickOriginY();
                    float dist = MuInput::getJoystickDistance();
                    float ang = MuInput::getJoystickAngle();
                    float ex = ox + cosf(ang) * dist;
                    float ey = oy + sinf(ang) * dist;
                    ImGui::GetWindowDrawList()->AddCircle(ImVec2(ox, oy), 80,
                        IM_COL32(255,255,255,60), 0, 3.0f);
                    ImGui::GetWindowDrawList()->AddCircle(ImVec2(ex, ey), 20,
                        IM_COL32(100,200,255,160), 0, 2.0f);
                    ImGui::GetWindowDrawList()->AddLine(ImVec2(ox, oy), ImVec2(ex, ey),
                        IM_COL32(100,200,255,80), 2.0f);
                }
                ImGui::End();

                // Button labels (right side)
                ImGui::SetNextWindowPos(ImVec2(scrW * 0.5f, 0));
                ImGui::SetNextWindowSize(ImVec2(scrW * 0.5f, scrH));
                ImGui::Begin("BtnOverlay", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
                             ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
                const char* btnLabels[] = {"1","2","3","4","5","6","7","8","I","C","M","Q","Ctrl","Spc","Tab","T"};
                float panelW = scrW * 0.5f;
                float colW = panelW / 4.0f;
                float rowH = scrH / 4.0f;
                ImFont* font = ImGui::GetFont();
                auto* dl = ImGui::GetWindowDrawList();
                for (int i = 0; i < 16; i++) {
                    int col = i % 4;
                    int row = i / 4;
                    float x1 = col * colW + 4, y1 = row * rowH + 4;
                    float x2 = (col+1)*colW - 4, y2 = (row+1)*rowH - 4;
                    dl->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2),
                        IM_COL32(0,0,0,50), 8.0f);
                    dl->AddRect(ImVec2(x1, y1), ImVec2(x2, y2),
                        IM_COL32(255,255,255,40), 8.0f, 0, 1.5f);
                    // Draw label text centered in button
                    const char* label = btnLabels[i];
                    ImVec2 ts = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0, label);
                    float tx = (x1 + x2 - ts.x) * 0.5f;
                    float ty = (y1 + y2 - ts.y) * 0.5f;
                    dl->AddText(ImVec2(tx, ty), IM_COL32(255,255,255,180), label);
                }
                ImGui::End();
            }

            // Show/hide soft keyboard based on ImGui text input focus
            {
                static bool s_prevWantTextInput = false;
                bool wantTextInput = ImGui::GetIO().WantTextInput;
                if (wantTextInput != s_prevWantTextInput) {
                    s_prevWantTextInput = wantTextInput;
                    Android_ShowKeyboard(wantTextInput);
                }
            }

            ImGui::Render();
            ImGui_ImplGLES3_RenderDrawData(ImGui::GetDrawData());

            MuGLContext::swapBuffers();
        }

        // 7. 重置瞬时鼠标标志 — 必须在 ImGui 渲染之后
        MuInput::resetOneShotMouseFlags();

        // 帧率控制 (60fps 上限)
        auto frameEnd = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(frameEnd - frameStart).count();
        if (elapsed < TARGET_FRAME_TIME) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(TARGET_FRAME_TIME - elapsed));
        }
    }

    LOGI("Render thread stopped");
}

// ============================================================================
// 公开接口
// ============================================================================

bool init(AAssetManager* assetManager, const char* internalPath) {
    g_assetManager = assetManager;
    g_internalPath = internalPath ? internalPath : "/sdcard/";
    if (!g_internalPath.empty() && g_internalPath.back() != '/') {
        g_internalPath += '/';
    }
    LOGI("MuGameLoop internalPath: %s", g_internalPath.c_str());

    g_running = true;
    g_paused = true;
    g_hasSurface = false;
    g_lastFrameTime = std::chrono::steady_clock::now();

    // One-time ImGui context creation (survives surface destroy/recreate)
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui_ImplAndroid_Init();

    LOGI("MuGameLoop initialized");
    return true;
}

void destroy() {
    g_running = false;

    if (g_renderThread.joinable()) {
        g_renderThread.join();
    }

    // ImGui GLES3 backend is shut down in onSurfaceDestroyed() before GL teardown.
    // Only Android backend and the ImGui context need cleanup here.
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    ProtocolDispatch::disconnect();
    MuAudio::destroy();

    if (g_terrainBatcher) {
        g_terrainBatcher->destroy();
        delete g_terrainBatcher;
        g_terrainBatcher = nullptr;
    }
    if (g_spriteRenderer) {
        g_spriteRenderer->destroy();
        delete g_spriteRenderer;
        g_spriteRenderer = nullptr;
    }
    if (g_batcher) {
        g_batcher->destroy();
        delete g_batcher;
        g_batcher = nullptr;
    }

    if (g_defaultProgram) {
        glDeleteProgram(g_defaultProgram);
        g_defaultProgram = 0;
    }
    ShaderLoader::shutdown();
    MuNetwork::disconnect();
    MuGLContext::destroy();

    LOGI("MuGameLoop destroyed");
}

void onSurfaceCreated(ANativeWindow* window) {
    std::lock_guard<std::mutex> lock(g_stateMutex);

    if (!MuGLContext::init(window)) {
        LOGE("MuGLContext::init failed");
        return;
    }

    // Compile default shader
    g_defaultProgram = ShaderLoader::loadDefaultProgram();
    if (!g_defaultProgram) {
        LOGE("Failed to load default shader program");
        return;
    }

    glUseProgram(g_defaultProgram);
    LOGI("Default shader program loaded (id=%u)", g_defaultProgram);

    // Initialize GL compatibility layer matrix stacks + capture state
    MuGlobalState_initGL();
    LOGI("GL compatibility state initialized");

    // Set initial window dimensions from native window (fullscreen support)
    {
        int winW = ANativeWindow_getWidth(window);
        int winH = ANativeWindow_getHeight(window);
        if (winW > 0 && winH > 0) {
            updateScreenSize(winW, winH);
        }
    }

    // Initialize ImGui GLES3 renderer (GL context just created)
    ImGui_ImplGLES3_Init();
    LOGI("ImGui GLES3 renderer initialized");

    // Initialize timer system (replaces Windows SetTimer/KillTimer)
    TimerManager::init();
    LOGI("TimerManager initialized");

    // Initialize protocol dispatch (state machine)
    ProtocolDispatch::init();
    ProtocolDispatch::setCredPath(g_internalPath.c_str());
    ProtocolDispatch::loadCredentials();
    LOGI("ProtocolDispatch initialized");

    // Initialize audio (replaces DirectSound)
    MuAudio::init(g_assetManager);
    LOGI("MuAudio initialized (OpenSL ES)");

    // Initialize file I/O abstraction (replaces Win32 CreateFile/ReadFile)
    MuFile::init(g_assetManager, g_internalPath.c_str());
    LOGI("MuFile initialized");

    // Initialize asset extractor (transparent fopen → APK asset loading)
    mu_asset_extractor_init(g_assetManager, g_internalPath.c_str());
    LOGI("MuAssetExtractor initialized");

    // Data files are extracted on-demand from APK assets when accessed by the game.
    // No bulk extraction needed — the fopen hook (mu_fopen_android) handles it.

    // Initialize network encryption keys from Connect.msil (CustomerName XOR ClientSerial)
    CGMProtect_InitEncDecKey();

    // Phase 9: Load SimpleModulus keys from assets
    // Key files: Data/Local/Dec2.dat (decryption), Data/Local/Enc1.dat (encryption)
    // Format: WORD header(4370) + DWORD size(54) + 48 bytes XOR'd key data
    {
        size_t keySize;
        uint8_t* decKey = MuFile::readAll("Data/Dec2.dat", keySize);
        if (decKey) {
            if (MuNetwork::initSimpleModulusDecKey(decKey, keySize)) {
                LOGI("SimpleModulus decryption key loaded from Dec2.dat (%zu bytes)", keySize);
            } else {
                LOGE("Failed to parse Dec2.dat");
            }
            MuFile::freeBuffer(decKey);
        } else {
            LOGI("Dec2.dat not found — C3/C4 decryption disabled");
        }

        uint8_t* encKey = MuFile::readAll("Data/Enc1.dat", keySize);
        if (encKey) {
            if (MuNetwork::initSimpleModulusEncKey(encKey, keySize)) {
                LOGI("SimpleModulus encryption key loaded from Enc1.dat (%zu bytes)", keySize);
            } else {
                LOGE("Failed to parse Enc1.dat");
            }
            MuFile::freeBuffer(encKey);
        } else {
            LOGI("Enc1.dat not found — C3/C4 encryption disabled");
        }

        if (MuNetwork::isSimpleModulusReady()) {
            LOGI("SimpleModulus ready — C3/C4 packets enabled");
            // Self-test: encrypt then decrypt a test block to verify keys
            MuNetwork::runSimpleModulusSelfTest();
        }
    }

    // Initialize render state manager
    RenderState::init();

    // Initialize render batchers
    g_batcher = new RenderBatcher();
    g_batcher->initialize();
    g_glBatcher = g_batcher; // wire capture adapter to the real batcher

    GLint mvpLoc   = glGetUniformLocation(g_defaultProgram, "uModelViewProjection");
    GLint mvLoc    = glGetUniformLocation(g_defaultProgram, "uModelView");
    GLint texLoc   = glGetUniformLocation(g_defaultProgram, "uTexture");
    GLint fogSLoc  = glGetUniformLocation(g_defaultProgram, "uFogStart");
    GLint fogELoc  = glGetUniformLocation(g_defaultProgram, "uFogEnd");
    GLint fogCLoc  = glGetUniformLocation(g_defaultProgram, "uFogColor");
    GLint atLoc    = glGetUniformLocation(g_defaultProgram, "uAlphaTest");
    GLint useTexLoc = glGetUniformLocation(g_defaultProgram, "uUseTexture");

    g_batcher->setUniformLocations(mvpLoc, mvLoc, texLoc,
                                    fogSLoc, fogELoc, fogCLoc,
                                    atLoc, useTexLoc);

    g_terrainBatcher = new TerrainBatcher();
    g_terrainBatcher->initialize();

    g_spriteRenderer = new SpriteRenderer();
    g_spriteRenderer->initialize(g_batcher);

    // Initialize game singletons and state (mirrors WinMain startup)
    // Must be called with active GL context (before releasing to render thread)
    MuGameInit();

    // Release EGL context from main thread (render thread will acquire it)
    eglMakeCurrent(MuGLContext::g_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    g_hasSurface = true;
    g_paused = false;
    g_lastFrameTime = std::chrono::steady_clock::now();

    g_renderThread = std::thread(renderLoop);

    LOGI("Surface created, render thread started");
}

void onSurfaceDestroyed() {
    g_hasSurface = false;
    g_paused = true;

    if (g_renderThread.joinable()) {
        g_renderThread.join();
    }

    if (g_terrainBatcher) {
        g_terrainBatcher->destroy();
        delete g_terrainBatcher;
        g_terrainBatcher = nullptr;
    }
    if (g_spriteRenderer) {
        g_spriteRenderer->destroy();
        delete g_spriteRenderer;
        g_spriteRenderer = nullptr;
    }
    if (g_batcher) {
        g_batcher->destroy();
        delete g_batcher;
        g_batcher = nullptr;
    }

    // Shutdown ImGui GLES3 renderer before GL context is destroyed (only if GL was initialized)
    if (g_defaultProgram) {
        ImGui_ImplGLES3_Shutdown();
        glDeleteProgram(g_defaultProgram);
        g_defaultProgram = 0;
    }

    MuGLContext::destroy();
    LOGI("Surface destroyed");
}

void pause() {
    g_paused = true;
    MuAudio::pauseAll();
    LOGI("Game loop paused");
}

void resume() {
    if (g_hasSurface) {
        g_paused = false;
        g_lastFrameTime = std::chrono::steady_clock::now();
        MuAudio::resumeAll();
    }
    LOGI("Game loop resumed");
}

void updateScreenSize(int width, int height) {
    if (width <= 0 || height <= 0) return;

    WindowWidth = width;
    WindowHeight = height;

    // Recalculate screen rate (matching WINHANDLE.cpp InitSize logic, ScreenType==0 stretch mode)
    g_fScreenRate_x = (float)width / 640.0f;
    g_fScreenRate_y = (float)height / 480.0f;

    LOGI("Screen size updated: %dx%d, rate_x=%.3f, rate_y=%.3f",
         width, height, g_fScreenRate_x, g_fScreenRate_y);
}

bool isRunning() {
    return g_running;
}

} // namespace MuGameLoop
