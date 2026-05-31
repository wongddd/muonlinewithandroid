#include "ProtocolDispatch.h"
#include "TimerManager.h"

#include <android/log.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <algorithm>
#include <ctime>

#define LOG_TAG "ProtocolDispatch"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Original TranslateProtocol (from WSclient.cpp) — global scope, not in namespace
extern BOOL TranslateProtocol(int HeadCode, BYTE* ReceiveBuffer, int Size, BOOL bEncrypted);

// SceneFlag from ZzzScene.cpp
extern int SceneFlag;
#define LOADING_SCENE   3
#define CHARACTER_SCENE 4
#define MAIN_SCENE      5

// PC engine globals — must be updated to match ProtocolDispatch state
// so ZzzScene.cpp's MoveMainScene() initializes correctly.
extern int  CurrentProtocolState;   // WSclient.h:83
extern int  SelectedHero;           // ZzzInterface.h:59
extern int  LoadingWorld;           // ZzzInterface.h:64
extern bool EnableMainRender;       // ZzzScene.h:16

// Audio stubs (MuBridge.cpp) — stop login music on scene transition
extern void StopMp3(char* Name, int bEnforce = 0);

// Scene data cleanup (ZzzOpenData.cpp) — free textures/objects on scene transitions
extern void ReleaseLogoSceneData();
extern void ReleaseCharacterSceneData();

// NewUI window control (NewUICommon.cpp)
extern "C" void NewUI_ShowWindow(int ifaceId, bool show);

#define PC_RECEIVE_JOIN_MAP_SERVER  61
#define PC_RECEIVE_LOG_IN_SUCCESS   20

namespace ProtocolDispatch {

// ============================================================================
// StreamPacketEngine XorData — forward chained XOR
// ============================================================================
static void XorData(uint8_t* buf, int start, int end) {
    static const uint8_t filter[32] = {
        0xAB, 0x11, 0xCD, 0xFE, 0x18, 0x23, 0xC5, 0xA3,
        0xCA, 0x33, 0xC1, 0xCC, 0x66, 0x67, 0x21, 0xF3,
        0x32, 0x12, 0x15, 0x35, 0x29, 0xFF, 0xFE, 0x1D,
        0x44, 0xEF, 0xCD, 0x41, 0x26, 0x3C, 0x4E, 0x4D
    };
    for (int i = start; i < end; i++) {
        buf[i] ^= (buf[i - 1] ^ filter[i % 32]);
    }
}

static void BuxConvert(uint8_t* buf, int size) {
    static const uint8_t XorTable[3] = { 0xFC, 0xCF, 0xAB };
    for (int n = 0; n < size; n++) {
        buf[n] ^= XorTable[n % 3];
    }
}

static constexpr int SIZE_PROTOCOLVERSION = 5;
static constexpr int SIZE_PROTOCOLSERIAL = 16;
static uint8_t g_version[SIZE_PROTOCOLVERSION] = {
    static_cast<uint8_t>('1' + 1),
    static_cast<uint8_t>('0' + 2),
    static_cast<uint8_t>('4' + 3),
    static_cast<uint8_t>('0' + 4),
    static_cast<uint8_t>('5' + 5)
};
static const char g_serial[SIZE_PROTOCOLSERIAL + 1] = "TbYehR2hFUPBKgZj";

// ============================================================================
// 状态
// ============================================================================
static ProtocolState g_currentState = ProtocolState::DISCONNECTED;
static std::string g_serverIP;
static uint16_t g_serverPort = 0;
static bool g_stateChanged = true;

// 登录凭据
static std::string g_loginAccount;
static std::string g_loginPassword;

// 上次登录错误信息 (UI 展示用)
static std::string g_lastError;

// Timer for delayed sends in ConnectServer flow
static float g_stateTimer = 0.0f;
static bool g_serverListRequested = false;

// 超时检测 (Task 4)
static float g_timeoutAccum = 0.0f;
static constexpr float CONNECT_TIMEOUT = 45.0f;   // 连接超时 45s (某些服务器响应慢)
static constexpr float LOGIN_TIMEOUT  = 30.0f;   // 登录超时 30s
static constexpr float JOIN_TIMEOUT   = 20.0f;   // 加入地图超时 20s

// 自动重连 (Task 5)
static bool g_autoReconnect = false;
static std::string g_reconnectIP;
static uint16_t g_reconnectPort = 0;
static std::string g_reconnectAccount;
static std::string g_reconnectPassword;
static float g_reconnectDelay = 0.0f;
static constexpr float RECONNECT_DELAY = 3.0f;   // 断开后 3s 自动重连

// 服务器列表 — PC 层级: ServerGroup → SubServerInfo
static std::vector<ServerGroup> g_serverGroups;

// 角色列表
static std::vector<CharacterInfo> g_characterList;

// 当前选中角色 (用于 sendJoinMapServerRequestPacket)
static int g_selectedSlot = 0;
static char g_selectedCharName[11] = {0};

// ============================================================================
// 内部函数声明
// ============================================================================
static void onEnterState(ProtocolState state);
static void sendHWIDRequest();
static void sendServerListRequest();
static void sendServerSelectRequest(uint16_t serverIndex);
static void sendLoginRequestPacket();
static void sendCharacterListRequestPacket();
static void sendJoinMapServerRequestPacket();
static void sendRegistrationPacket(const char* account, const char* password);

// 注册结果 (Task 2)
static std::string g_regResult;
static int g_regResultColor = 0;

// 角色创建/删除结果 (Task 3)
static char g_createResult[128] = {0};
static int  g_createResultColor = 0;

// 协议处理
static void handleProtocol0xF1(const Packet& packet);
static void handleProtocol0xF3(const Packet& packet);
static void handleProtocol0xF4(const Packet& packet);
static void handleProtocol0x00(const Packet& packet);

// ============================================================================
// 状态机 — 完整 PC 流程
// ============================================================================

static const char* stateName(ProtocolState s) {
    switch (s) {
        case ProtocolState::DISCONNECTED:                return "DISCONNECTED";
        case ProtocolState::REQUEST_JOIN_SERVER:         return "REQUEST_JOIN_SERVER";
        case ProtocolState::RECEIVE_JOIN_SERVER_WAITING: return "RECEIVE_JOIN_SERVER_WAITING";
        case ProtocolState::RECEIVE_JOIN_SERVER_SUCCESS: return "RECEIVE_JOIN_SERVER_SUCCESS";
        case ProtocolState::RECEIVE_JOIN_SERVER_FAIL:    return "RECEIVE_JOIN_SERVER_FAIL";
        case ProtocolState::GS_JOIN_REQUESTED:           return "GS_JOIN_REQUESTED";
        case ProtocolState::REQUEST_LOG_IN:              return "REQUEST_LOG_IN";
        case ProtocolState::RECEIVE_LOG_IN_SUCCESS:      return "RECEIVE_LOG_IN_SUCCESS";
        case ProtocolState::REQUEST_CHARACTERS_LIST:     return "REQUEST_CHARACTERS_LIST";
        case ProtocolState::RECEIVE_CHARACTERS_LIST:     return "RECEIVE_CHARACTERS_LIST";
        case ProtocolState::REQUEST_JOIN_MAP_SERVER:     return "REQUEST_JOIN_MAP_SERVER";
        case ProtocolState::RECEIVE_JOIN_MAP_SERVER:     return "RECEIVE_JOIN_MAP_SERVER";
        default: return "UNKNOWN";
    }
}

static void setState(ProtocolState newState) {
    if (g_currentState != newState) {
        LOGI("State: %s → %s", stateName(g_currentState), stateName(newState));
        g_currentState = newState;
        g_stateChanged = true;
        MuNetwork::setState(newState);
    }
}

static void onEnterState(ProtocolState state) {
    LOGI("Entering state: %s", stateName(state));

    switch (state) {
        case ProtocolState::REQUEST_JOIN_SERVER:
            // PC Phase 2: Connect to ConnectServer, send HWID
            g_serverGroups.clear();
            g_characterList.clear();
            g_serverListRequested = false;
            g_stateTimer = 0.0f;
            if (!g_serverIP.empty()) {
                if (MuNetwork::connect(g_serverIP.c_str(), g_serverPort)) {
                    sendHWIDRequest();
                    setState(ProtocolState::RECEIVE_JOIN_SERVER_WAITING);
                } else {
                    g_lastError = "无法连接到服务器";
                    LOGE("Failed to connect to %s:%u", g_serverIP.c_str(), g_serverPort);
                    setState(ProtocolState::RECEIVE_JOIN_SERVER_FAIL);
                }
            }
            break;

        case ProtocolState::RECEIVE_JOIN_SERVER_WAITING:
            // PC Phase 2: waiting for server list from CS.
            // After 0.3s in update(), we auto-request the server list.
            break;

        case ProtocolState::RECEIVE_JOIN_SERVER_SUCCESS:
            // PC Phase 3 end: Got GameServer IP:Port redirect. Reconnect to GS.
            // After this, GS will send F1/00 (join result) — we wait for it.
            LOGI("Reconnecting to GameServer at %s:%u", g_serverIP.c_str(), g_serverPort);
            MuNetwork::disconnect();
            if (MuNetwork::connect(g_serverIP.c_str(), g_serverPort)) {
                g_stateTimer = 0.0f;
                setState(ProtocolState::GS_JOIN_REQUESTED);
            } else {
                g_lastError = "无法连接到游戏服务器";
                LOGE("Failed to reconnect to GameServer");
                setState(ProtocolState::RECEIVE_JOIN_SERVER_FAIL);
            }
            break;

        case ProtocolState::GS_JOIN_REQUESTED:
            // PC Phase 3: Connected to GS, waiting for F1/00 join result.
            //  — UI shows "Connecting to game server..."
            //  — When F1/00 arrives with result=0x01 → UI shows login form
            //  — User enters credentials and clicks Login → sendLogin()
            break;

        case ProtocolState::REQUEST_LOG_IN:
            // PC Phase 4: User clicked Login — send F1/01 login packet
            sendLoginRequestPacket();
            break;

        case ProtocolState::RECEIVE_LOG_IN_SUCCESS:
            // PC Phase 4 end: Login OK. UI transitions to CHARACTER_SCENE.
            LOGI("*** Login successful! SceneFlag=%d → CHARACTER_SCENE(4)", SceneFlag);

            // Task 2: 释放登录场景资源
            ReleaseLogoSceneData();
            LOGI("*** ReleaseLogoSceneData() called");

            // Task 5: 保存连接信息用于自动重连
            g_reconnectIP = g_serverIP;
            g_reconnectPort = g_serverPort;
            g_reconnectAccount = g_loginAccount;
            g_reconnectPassword = g_loginPassword;

            // Stop login music (PC behaviour: StopMp3 in LoadingScene)
            StopMp3(nullptr, 1);

            // Tell the PC engine that we've logged in (prevents re-init attempts)
            CurrentProtocolState = PC_RECEIVE_LOG_IN_SUCCESS;
            SceneFlag = CHARACTER_SCENE;
            LOGI("*** SceneFlag is now %d, transitioning to REQUEST_CHARACTERS_LIST", SceneFlag);
            setState(ProtocolState::REQUEST_CHARACTERS_LIST);
            break;

        case ProtocolState::REQUEST_CHARACTERS_LIST:
            // PC Phase 5: Request character list from GS
            LOGI("*** Requesting character list...");
            sendCharacterListRequestPacket();
            break;

        case ProtocolState::RECEIVE_CHARACTERS_LIST:
            // PC Phase 5 end: Character list received. UI shows character selection.
            LOGI("*** Character list received! count=%zu, scene=%d",
                 g_characterList.size(), SceneFlag);

            // Task 2: Update PC engine CurrentProtocolState to enable
            // 3D character rendering in NewMoveCharacterScene().
            // PC condition: CurrentProtocolState >= RECEIVE_CHARACTERS_LIST (51)
            CurrentProtocolState = 51;

            for (size_t ci = 0; ci < g_characterList.size(); ci++) {
                LOGI("  Char[%zu]: '%s' Lv.%d class=0x%02X",
                     ci, g_characterList[ci].name,
                     g_characterList[ci].level,
                     g_characterList[ci].classCode);
            }
            // Tell the PC engine which character is selected for rendering.
            // ZzzScene.cpp's NewMoveCharacterScene/NewRenderCharacterScene use
            // SelectedHero to know which character model to highlight/display.
            if (!g_characterList.empty()) {
                SelectedHero = g_characterList[0].slot;
                LOGI("*** Set SelectedHero=%d ('%s') for PC engine",
                     SelectedHero, g_characterList[0].name);
            }
            break;

        case ProtocolState::REQUEST_JOIN_MAP_SERVER:
            // PC Phase 6: User selected a character — send join map request
            // Task 1: Show loading screen while waiting for F3/03 response
            SceneFlag = LOADING_SCENE;
            LOGI("*** SceneFlag = LOADING_SCENE, sending join map request");
            sendJoinMapServerRequestPacket();
            break;

        case ProtocolState::RECEIVE_JOIN_MAP_SERVER:
            LOGI("*** ENTERED GAME WORLD ***");
            break;

        case ProtocolState::RECEIVE_JOIN_SERVER_FAIL:
            LOGE("Join server failed. Disconnecting.");
            disconnect();
            break;

        default:
            break;
    }
}

// ============================================================================
// 公开接口
// ============================================================================

void init() {
    if (g_currentState == ProtocolState::REQUEST_JOIN_SERVER) {
        LOGI("ProtocolDispatch init — preserving pending connection to %s:%u",
             g_serverIP.c_str(), g_serverPort);
        return;
    }
    g_currentState = ProtocolState::DISCONNECTED;
    g_stateChanged = true;
    g_serverIP.clear();
    g_serverPort = 0;
    g_serverGroups.clear();
    g_characterList.clear();
    g_timeoutAccum = 0.0f;
    g_autoReconnect = false;
    LOGI("ProtocolDispatch initialized");
}

void update(float deltaTime) {
    if (g_stateChanged) {
        g_stateChanged = false;
        onEnterState(g_currentState);
    }

    // Timer-based: 0.3s after connecting to CS, request server list
    if (g_currentState == ProtocolState::RECEIVE_JOIN_SERVER_WAITING &&
        MuNetwork::isConnected()) {
        g_stateTimer += deltaTime;
        if (!g_serverListRequested && g_stateTimer >= 0.3f) {
            g_serverListRequested = true;
            sendServerListRequest();
        }
    }

    // ======== Task 4: 超时检测 ========
    g_timeoutAccum += deltaTime;
    if (MuNetwork::isConnected()) {
        float timeout = 0.0f;
        switch (g_currentState) {
            case ProtocolState::REQUEST_JOIN_SERVER:
            case ProtocolState::RECEIVE_JOIN_SERVER_WAITING:
            case ProtocolState::RECEIVE_JOIN_SERVER_SUCCESS:
            case ProtocolState::GS_JOIN_REQUESTED:
                timeout = CONNECT_TIMEOUT;
                break;
            case ProtocolState::REQUEST_LOG_IN:
                timeout = LOGIN_TIMEOUT;
                break;
            case ProtocolState::REQUEST_JOIN_MAP_SERVER:
                timeout = JOIN_TIMEOUT;
                break;
            default:
                g_timeoutAccum = 0.0f;  // 其他状态不检测超时
                break;
        }
        if (timeout > 0.0f && g_timeoutAccum >= timeout) {
            g_lastError = "连接超时";
            LOGE("Timeout in state %s after %.0fs", stateName(g_currentState), timeout);
            g_timeoutAccum = 0.0f;
            disconnect();
        }
    } else {
        g_timeoutAccum = 0.0f;
    }

    // ======== Task 5: 自动重连 ========
    if (g_autoReconnect &&
        g_currentState == ProtocolState::DISCONNECTED &&
        !g_reconnectIP.empty()) {
        g_reconnectDelay += deltaTime;
        if (g_reconnectDelay >= RECONNECT_DELAY) {
            LOGI("Auto-reconnecting to %s:%u as %s",
                 g_reconnectIP.c_str(), g_reconnectPort, g_reconnectAccount.c_str());
            g_autoReconnect = false;
            g_reconnectDelay = 0.0f;
            g_serverIP = g_reconnectIP;
            g_serverPort = g_reconnectPort;
            if (!g_reconnectAccount.empty()) {
                g_loginAccount = g_reconnectAccount;
                g_loginPassword = g_reconnectPassword;
            }
            ProtocolDispatch::connectToServer(g_serverIP.c_str(), g_serverPort);
        }
    }
}

void processPacket(const Packet& packet) {
    if (packet.size < 3) { LOGE("Packet too small: %zu bytes", packet.size); return; }

    uint8_t packetType = packet.data[0];
    uint8_t headCode = 0;
    uint16_t packetSize = 0;

    // Normalize C2 to C1 format for uniform subcode access
    uint8_t normBuf[MAX_PACKET_SIZE];
    const uint8_t* pktData = packet.data;
    size_t pktDataSize = packet.size;

    if (packetType == 0xC1) {
        headCode = packet.data[2];
        packetSize = packet.data[1];
    } else if (packetType == 0xC2) {
        headCode = packet.data[3];
        packetSize = (static_cast<uint16_t>(packet.data[1]) << 8) | packet.data[2];
        size_t newLen = packet.size - 1;
        if (newLen <= MAX_PACKET_SIZE) {
            normBuf[0] = 0xC2;
            normBuf[1] = static_cast<uint8_t>(packetSize & 0xFF);
            memcpy(normBuf + 2, packet.data + 3, packet.size - 3);
            pktData = normBuf;
            pktDataSize = newLen;
        }
    } else {
        LOGE("Unexpected packet type 0x%02X in dispatch", packetType);
        return;
    }

    // Once in game world, route ALL packets through original TranslateProtocol
    if (g_currentState >= ProtocolState::RECEIVE_JOIN_MAP_SERVER) {
        BOOL bEnc = packet.encrypted ? TRUE : FALSE;
        TranslateProtocol(headCode, (BYTE*)packet.data, (int)packet.size, bEnc);
        return;
    }

    // Login flow: dispatch via our simplified state machine
    Packet normalized;
    memcpy(normalized.data, pktData, pktDataSize);
    normalized.size = pktDataSize;
    normalized.encrypted = packet.encrypted;

    // Dump raw F4/06 server list packets for debugging
    if (headCode == 0xF4 && normalized.size >= 5 && normalized.data[3] == 0x06) {
        char hexBuf[384] = {0};
        int dumpLen = (normalized.size < 128) ? (int)normalized.size : 128;
        for (int di = 0; di < dumpLen; di++)
            sprintf(hexBuf + di*3, "%02X ", normalized.data[di]);
        LOGI("Raw F4/06 (norm) [0..%d] sz=%zu: %s", dumpLen-1, normalized.size, hexBuf);
    }

    switch (headCode) {
        case 0xF1: handleProtocol0xF1(normalized); break;
        case 0xF3: handleProtocol0xF3(normalized); break;
        case 0xF4: handleProtocol0xF4(normalized); break;
        case 0x00: handleProtocol0x00(normalized); break;
        default:
            LOGI("Unhandled protocol head code: 0x%02X (state: %s)",
                 headCode, stateName(g_currentState));
            break;
    }
}

void connectToServer(const char* ip, uint16_t port) {
    g_serverIP = ip;
    g_serverPort = port;
    setState(ProtocolState::REQUEST_JOIN_SERVER);
}

void disconnect() {
    MuNetwork::disconnect();
    g_serverGroups.clear();
    g_characterList.clear();
    g_timeoutAccum = 0.0f;
    setState(ProtocolState::DISCONNECTED);
}

ProtocolState getState() { return g_currentState; }
const char* getStateName() { return stateName(g_currentState); }

// ========== 服务器列表 (PC: CServerListManager) ==========

const std::vector<ServerGroup>& getServerGroups() {
    return g_serverGroups;
}

void selectServer(int groupIdx, int subIdx) {
    if (g_currentState != ProtocolState::RECEIVE_JOIN_SERVER_WAITING) return;
    if (groupIdx < 0 || groupIdx >= (int)g_serverGroups.size()) return;

    auto& group = g_serverGroups[groupIdx];
    if (subIdx < 0 || subIdx >= (int)group.servers.size()) return;

    uint16_t connectIdx = (uint16_t)group.servers[subIdx].connectIndex;
    sendServerSelectRequest(connectIdx);
    LOGI("Selected server: group=%d '%s' sub=%d '%s' (connectIdx=%u)",
         groupIdx, group.name, subIdx, group.servers[subIdx].name, connectIdx);
}

void refreshServerList() {
    if (g_currentState == ProtocolState::RECEIVE_JOIN_SERVER_WAITING) {
        g_serverListRequested = true;
        sendServerListRequest();
    }
}

// ========== 登录 ==========

const char* getLastError() {
    return g_lastError.c_str();
}

void clearLastError() {
    g_lastError.clear();
}

void setCredentials(const char* account, const char* password) {
    g_loginAccount = account;
    g_loginPassword = password;
    g_lastError.clear();
    LOGI("Login credentials stored for account: %s", account);
}

void sendLogin() {
    // PC: user clicked OK in LoginWin → SendRequestLogIn
    if (g_currentState == ProtocolState::GS_JOIN_REQUESTED) {
        // Only proceed if F1/00 was received (GS validated the connection)
        setState(ProtocolState::REQUEST_LOG_IN);
    } else {
        LOGI("sendLogin() ignored: current state is %s (need GS_JOIN_REQUESTED)",
             stateName(g_currentState));
    }
}

// ========== 角色列表 ==========

const std::vector<CharacterInfo>& getCharacterList() {
    return g_characterList;
}

void selectCharacter(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= (int)g_characterList.size()) return;
    if (g_currentState != ProtocolState::RECEIVE_CHARACTERS_LIST) return;

    // Set the selected char name and trigger join map server
    const CharacterInfo& ci = g_characterList[slotIndex];
    memcpy(g_selectedCharName, ci.name, 10);
    g_selectedCharName[10] = '\0';
    g_selectedSlot = ci.slot;
    LOGI("Selected character '%s' (slot %d), joining map server", g_selectedCharName, g_selectedSlot);

    setState(ProtocolState::REQUEST_JOIN_MAP_SERVER);
}

// ========== Task 3: 记住账号 (持久化到文件) ==========

static std::string g_savedAccount;
static std::string g_savedPassword;
static std::string g_credPath;  // 配置文件路径

const char* getSavedAccount() { return g_savedAccount.c_str(); }
const char* getSavedPassword() { return g_savedPassword.c_str(); }

void setCredPath(const char* internalPath) {
    if (internalPath) {
        g_credPath = internalPath;
        if (!g_credPath.empty() && g_credPath.back() != '/') g_credPath += '/';
        g_credPath += ".mu_credentials";
    }
}

void saveCredentials(const char* account, const char* password) {
    g_savedAccount = account ? account : "";
    g_savedPassword = password ? password : "";
    LOGI("Credentials saved for account: '%s'", g_savedAccount.c_str());
    // 写入文件
    if (!g_credPath.empty()) {
        FILE* fp = (fopen)(g_credPath.c_str(), "w");
        if (fp) {
            fprintf(fp, "%s\n%s\n", g_savedAccount.c_str(), g_savedPassword.c_str());
            fclose(fp);
        }
    }
}

void loadCredentials() {
    if (g_credPath.empty()) return;
    FILE* fp = (fopen)(g_credPath.c_str(), "r");
    if (fp) {
        char buf[64];
        if (fgets(buf, sizeof(buf), fp)) {
            buf[strcspn(buf, "\n")] = '\0';
            g_savedAccount = buf;
        }
        if (fgets(buf, sizeof(buf), fp)) {
            buf[strcspn(buf, "\n")] = '\0';
            g_savedPassword = buf;
        }
        fclose(fp);
        if (!g_savedAccount.empty())
            LOGI("Credentials loaded for account: '%s'", g_savedAccount.c_str());
    }
}

// ========== Task 5: 自动重连 ==========

void enableAutoReconnect(bool enable) {
    g_autoReconnect = enable;
    LOGI("Auto-reconnect %s", enable ? "enabled" : "disabled");
}

bool isAutoReconnectEnabled() { return g_autoReconnect; }

// ============================================================================
// 数据包发送
// ============================================================================

static void sendHWIDRequest() {
    uint8_t buf[42] = {0};
    buf[0] = 0xC1; buf[1] = 42; buf[2] = 0xF4; buf[3] = 0x04;
    MuNetwork::send(buf, buf[1]);
    LOGI("Sent HWID request (0xF4 sub 0x04)");
}

static void sendServerListRequest() {
    uint8_t buf[5] = {0};
    buf[0] = 0xC1; buf[1] = 5; buf[2] = 0xF4; buf[3] = 0x06;
    MuNetwork::send(buf, buf[1]);
    LOGI("Sent server list request (0xF4 sub 0x06)");
}

static void sendServerSelectRequest(uint16_t serverIndex) {
    uint8_t buf[6] = {0};
    buf[0] = 0xC1; buf[1] = 6; buf[2] = 0xF4; buf[3] = 0x03;
    buf[4] = static_cast<uint8_t>(serverIndex & 0xFF);
    buf[5] = static_cast<uint8_t>((serverIndex >> 8) & 0xFF);
    MuNetwork::send(buf, buf[1]);
    LOGI("Sent server select request for server %u (0xF4 sub 0x03)", serverIndex);
}

static void sendLoginRequestPacket() {
    static constexpr int ID_SIZE = 10;
    static constexpr int PW_SIZE = 20;
    static constexpr int HWID_SIZE = 36;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint32_t tick = static_cast<uint32_t>(ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL);

    size_t bodySize = ID_SIZE + PW_SIZE + HWID_SIZE + sizeof(tick)
                     + SIZE_PROTOCOLVERSION + SIZE_PROTOCOLSERIAL;
    size_t totalSize = 4 + bodySize;
    std::vector<uint8_t> rawBuf(totalSize);

    rawBuf[0] = 0xC1;
    rawBuf[1] = static_cast<uint8_t>(totalSize);
    rawBuf[2] = 0xF1;
    rawBuf[3] = 0x01;

    size_t off = 4;
    size_t accLen = g_loginAccount.size();
    if (accLen > ID_SIZE) accLen = ID_SIZE;
    memcpy(rawBuf.data() + off, g_loginAccount.c_str(), accLen);
    off += ID_SIZE;

    size_t pwdLen = g_loginPassword.size();
    if (pwdLen > PW_SIZE) pwdLen = PW_SIZE;
    memcpy(rawBuf.data() + off, g_loginPassword.c_str(), pwdLen);
    off += PW_SIZE;

    off += HWID_SIZE;

    BuxConvert(rawBuf.data() + 4, ID_SIZE);
    BuxConvert(rawBuf.data() + 4 + ID_SIZE, PW_SIZE);
    BuxConvert(rawBuf.data() + 4 + ID_SIZE + PW_SIZE, HWID_SIZE);

    memcpy(rawBuf.data() + off, &tick, sizeof(tick));
    off += sizeof(tick);

    for (int i = 0; i < SIZE_PROTOCOLVERSION; i++)
        rawBuf[off++] = static_cast<uint8_t>(g_version[i] - (i + 1));
    memcpy(rawBuf.data() + off, g_serial, SIZE_PROTOCOLSERIAL);

    XorData(rawBuf.data(), 3, static_cast<int>(totalSize));

    MuNetwork::send(rawBuf.data(), rawBuf[1], true);
    LOGI("Sent login request for account: %s", g_loginAccount.c_str());
}

static void sendCharacterListRequestPacket() {
    uint8_t buf[5] = {0};
    buf[0] = 0xC1; buf[1] = 5; buf[2] = 0xF3; buf[3] = 0x00; buf[4] = 0x00;
    MuNetwork::send(buf, buf[1]);
    LOGI("Sent character list request");
}

static void sendJoinMapServerRequestPacket() {
    uint8_t buf[14] = {0};
    buf[0] = 0xC1; buf[1] = 14; buf[2] = 0xF3; buf[3] = 0x03;
    memcpy(buf + 4, g_selectedCharName, 10);
    XorData(buf, 3, 14);
    MuNetwork::send(buf, buf[1], true);
    LOGI("Sent join map server request for character: '%s'", g_selectedCharName);
}

// ========== Task 2: 角色创建/删除 (PC: 0xF3 sub 0x01/0x02) ==========

static void sendCharacterCreateRequestPacket(const char* name, uint8_t classCode) {
    // PC: SendRequestCreateCharacter — C1 [size] F3 01 [name(10)] [classCode]
    uint8_t buf[16] = {0};
    buf[0] = 0xC1; buf[1] = 16; buf[2] = 0xF3; buf[3] = 0x01;
    memcpy(buf + 4, name, std::min(strlen(name), (size_t)10));
    buf[14] = classCode;
    XorData(buf, 3, 16);
    MuNetwork::send(buf, buf[1], true);
    LOGI("Sent create character request: '%s' class=0x%02X", name, classCode);
}

static void sendCharacterDeleteRequestPacket(const char* name) {
    // PC: SendRequestDeleteCharacter — C1 [size] F3 02 [name(10)] [code]
    uint8_t buf[16] = {0};
    buf[0] = 0xC1; buf[1] = 16; buf[2] = 0xF3; buf[3] = 0x02;
    memcpy(buf + 4, name, std::min(strlen(name), (size_t)10));
    buf[14] = 0x00; // code
    XorData(buf, 3, 16);
    MuNetwork::send(buf, buf[1], true);
    LOGI("Sent delete character request: '%s'", name);
}

// ========== 公开接口: 角色创建/删除 ==========

void createCharacter(const char* name, uint8_t classCode) {
    if (g_currentState != ProtocolState::RECEIVE_CHARACTERS_LIST) {
        LOGI("createCharacter ignored: state=%s", stateName(g_currentState));
        return;
    }
    sendCharacterCreateRequestPacket(name, classCode);
}

void deleteCharacter(int slotIndex) {
    if (slotIndex < 0 || slotIndex >= (int)g_characterList.size()) return;
    if (g_currentState != ProtocolState::RECEIVE_CHARACTERS_LIST) return;
    sendCharacterDeleteRequestPacket(g_characterList[slotIndex].name);
}

// ========== Task 2: 账号注册 ==========

static void sendRegistrationPacket(const char* account, const char* password) {
    // Send registration request to ConnectServer.
    // Format (common private server): C1 [size] F4 01 [account(10)] [password(20)]
    // The server responds with F4/01 result.
    uint8_t buf[64] = {0};
    buf[0] = 0xC1;
    buf[1] = 36;  // size: 4 + 10 + 20 + 2
    buf[2] = 0xF4;
    buf[3] = 0x01;  // subcode: register
    size_t accLen = strlen(account);
    size_t pwdLen = strlen(password);
    memcpy(buf + 4, account, accLen > 10 ? 10 : accLen);
    memcpy(buf + 14, password, pwdLen > 20 ? 20 : pwdLen);
    MuNetwork::send(buf, buf[1]);
    LOGI("Sent registration request for account: '%s'", account);
}

const char* getRegResult() { return g_regResult.c_str(); }
int getRegResultColor() { return g_regResultColor; }
const char* getCreateCharResult() { return g_createResult; }
int getCreateCharResultColor() { return g_createResultColor; }

void sendRegister(const char* account, const char* password) {
    g_regResult.clear();
    g_regResultColor = 0;
    if (MuNetwork::isConnected()) {
        sendRegistrationPacket(account, password);
        g_regResult = "\xe6\xb3\xa8\xe5\x86\x8c\xe8\xaf\xb7\xe6\xb1\x82\xe5\xb7\xb2\xe5\x8f\x91\xe9\x80\x81";
        g_regResultColor = 1;
    } else {
        g_regResult = "\xe6\x9c\xaa\xe8\xbf\x9e\xe6\x8e\xa5\xe5\x88\xb0\xe6\x9c\x8d\xe5\x8a\xa1\xe5\x99\xa8";
        g_regResultColor = 2;
    }
}

// ============================================================================
// 协议处理器
// ============================================================================

static void handleProtocol0xF1(const Packet& packet) {
    if (packet.size < 4) return;

    uint8_t subCode = packet.data[3];
    uint8_t value = (packet.size > 4) ? packet.data[4] : 0;

    LOGI("Protocol 0xF1: subcode=0x%02X value=0x%02X state=%s",
         subCode, value, stateName(g_currentState));

    switch (subCode) {
        case 0x00: {
            // PC: ReceiveJoinServer — GS validates our connection (F1/00)
            // result=0x01 means OK. UI should now show login form.
            uint8_t result = value;
            if (result == 0x01) {
                if (packet.size >= 7) {
                    uint16_t heroKey = (static_cast<uint16_t>(packet.data[5]) << 8) | packet.data[6];
                    LOGI("Join server success! HeroKey=%u", heroKey);
                }
                // Stay in GS_JOIN_REQUESTED — UI will show login form.
                // User clicks Login → sendLogin() → transitions to REQUEST_LOG_IN.
                LOGI("GameServer connection OK — waiting for user login input");
            } else {
                g_lastError = "游戏服务器拒绝连接";
                LOGE("Join server failed: result=%u", result);
                setState(ProtocolState::RECEIVE_JOIN_SERVER_FAIL);
            }
            break;
        }
        case 0x01: {
            // PC: ReceiveLoginResult — server responds to our F1/01 login packet
            LOGI("Login result: value=0x%02X state=%s size=%zu",
                 value, stateName(g_currentState), packet.size);
            for (size_t di = 0; di < packet.size && di < 16; di++) {
                __android_log_print(ANDROID_LOG_INFO, LOG_TAG,
                    "  F1/01[%zu] = 0x%02X", di, packet.data[di]);
            }
            switch (value) {
                case 0x00:
                    g_lastError = "密码错误";
                    LOGE("Login failed: wrong password");
                    setState(ProtocolState::GS_JOIN_REQUESTED);
                    break;
                case 0x01:
                case 0x20:
                    g_lastError.clear();
                    LOGI("*** Login success! value=0x%02X", value);
                    setState(ProtocolState::RECEIVE_LOG_IN_SUCCESS);
                    break;
                case 0x02:
                    g_lastError = "账号不存在";
                    LOGE("Login failed: invalid ID");
                    setState(ProtocolState::GS_JOIN_REQUESTED);
                    break;
                case 0x03:
                    g_lastError = "账号已在线";
                    LOGE("Login failed: ID already connected");
                    setState(ProtocolState::GS_JOIN_REQUESTED);
                    break;
                case 0x04:
                    g_lastError = "服务器繁忙";
                    LOGE("Login failed: server busy");
                    setState(ProtocolState::GS_JOIN_REQUESTED);
                    break;
                case 0x05:
                    g_lastError = "账号已被封停";
                    LOGE("Login failed: ID blocked");
                    setState(ProtocolState::GS_JOIN_REQUESTED);
                    break;
                case 0x06:
                    g_lastError = "版本不匹配";
                    LOGE("Login failed: version mismatch");
                    setState(ProtocolState::GS_JOIN_REQUESTED);
                    break;
                default:
                    g_lastError = "未知错误";
                    LOGE("Login failed: unknown result 0x%02X", value);
                    setState(ProtocolState::GS_JOIN_REQUESTED);
                    break;
            }
            break;
        }
        default:
            LOGI("0xF1 unhandled subcode: 0x%02X", subCode);
            break;
    }
}

static void handleProtocol0xF4(const Packet& packet) {
    if (packet.size < 4) return;

    uint8_t subCode = packet.data[3];

    switch (subCode) {
        case 0x01: {
            // Registration response
            uint8_t result = (packet.size > 4) ? packet.data[4] : 0;
            LOGI("Registration result: 0x%02X", result);
            if (result == 0x00) {
                g_regResult = "\xe6\xb3\xa8\xe5\x86\x8c\xe6\x88\x90\xe5\x8a\x9f"; // 注册成功
                g_regResultColor = 1;
            } else {
                g_regResult = "\xe6\xb3\xa8\xe5\x86\x8c\xe5\xa4\xb1\xe8\xb4\xa5"; // 注册失败
                g_regResultColor = 2;
            }
            break;
        }
        case 0x03: {
            // PC: ReceiveServerConnect — GS address redirect from CS
            // Format: {C1, Size, F4, 03, IP[15]...Port[2]}
            LOGI("GameServer redirect packet (%zu bytes):", packet.size);
            for (size_t i = 0; i < packet.size && i < 24; i++) {
                __android_log_print(ANDROID_LOG_INFO, LOG_TAG,
                    "  [%02zu] 0x%02X '%c'", i, packet.data[i],
                    (packet.data[i] >= 32 && packet.data[i] < 127) ? (char)packet.data[i] : '.');
            }
            if (packet.size >= 22) {
                char ip[16] = {0};
                memcpy(ip, packet.data + 4, 15);
                ip[15] = '\0';
                uint16_t port = static_cast<uint16_t>(packet.data[20])
                              | (static_cast<uint16_t>(packet.data[21]) << 8);
                LOGI("GameServer at %s:%u", ip, port);
                g_serverIP = ip;
                g_serverPort = port;
                setState(ProtocolState::RECEIVE_JOIN_SERVER_SUCCESS);
            }
            break;
        }
        case 0x06: {
            // PC: ReceiveServerList — CS sends available servers (PRECEIVE_SERVER_LIST)
            //
            // Packet format matching PC client (WSclient.cpp ReceiveServerList):
            //   For C1:  [0]Code=0xC1 [1]Size [2]Head=0xF4 [3]Sub=0x06 [4]TotalH [5]TotalL
            //   For C2:  [0]Code=0xC2 [1]SizeH [2]SizeL [3]Head=0xF4 [4]Sub=0x06 [5]TotalH [6]TotalL
            //   Then TotalServer entries, each sizeof(PRECEIVE_SERVER_LIST)=36 bytes:
            //     Index[2] (LE WORD, encodes group/sub: group=idx/20, sub=idx%20+1)
            //     Percent[1] (0-100=normal, 128+=closed)
            //     Type[1] (unused)
            //     ServerName[32] (ASCII null-terminated)
            //
            // PC mapping: InsertServerGroup(Index, Percent, ServerName)
            //   → groupIndex = Index / 20  (CServerGroup.m_iServerIndex)
            //   → subIndex   = (Index % 20) + 1  (CServerInfo.m_iIndex)
            //   → display name: "%s-%d %s" (GroupName, subIndex, typeLabel)
            //   → pct>=128 means "Closed", pct>=100 means "Full"
            //   → Index stored as connectIndex for later F4/03 select

            if (g_currentState != ProtocolState::RECEIVE_JOIN_SERVER_WAITING) {
                LOGI("Server list ignored (state=%s)", stateName(g_currentState));
                break;
            }

            g_serverGroups.clear();

            // Packet is already normalized to C1-like format:
            //   [0]=Code [1]=Size [2]=Head(F4) [3]=Sub(06) [4]=CntH [5]=CntL
            // PC (C1): PHEADER_DEFAULT_SUBCODE → TotalH=Data[4], TotalL=Data[5]
            // PC (C2): PHEADER_DEFAULT_SUBCODE_WORD → TotalH=Data->Value, TotalL=Value2
            //          where Value=data[5], Value2=data[6] in ORIGINAL C2,
            //          but after normalization they're at data[4], data[5]
            const int totalH_offset = 4;  // C1-like normalized format

            if ((int)packet.size < totalH_offset + 2) break;
            uint8_t totalH = packet.data[totalH_offset];
            uint8_t totalL = packet.data[totalH_offset + 1];
            int totalServer = (static_cast<int>(totalH) << 8) | totalL;

            LOGI("Server list: total=%d servers (C1-mode offset=%d), packet size=%zu",
                 totalServer, totalH_offset, packet.size);

            // Dump raw header for debugging
            {
                char hexBuf[256] = {0};
                int dumpLen = (packet.size < 64) ? (int)packet.size : 64;
                for (int di = 0; di < dumpLen; di++) {
                    sprintf(hexBuf + di*3, "%02X ", packet.data[di]);
                }
                LOGI("Raw[0..%d]: %s", dumpLen-1, hexBuf);
            }

            if (totalServer <= 0) break;

            // Parse entries starting after the 2-byte total count
            int entryStart = totalH_offset + 2;
            constexpr int ENTRY_SIZE = 36;  // sizeof(PRECEIVE_SERVER_LIST)
            int maxEntries = (static_cast<int>(packet.size) - entryStart) / ENTRY_SIZE;
            if (maxEntries < totalServer) totalServer = maxEntries;

            for (int i = 0; i < totalServer; i++) {
                int pos = entryStart + i * ENTRY_SIZE;
                if (pos + ENTRY_SIZE > (int)packet.size) break;

                // Parse PRECEIVE_SERVER_LIST
                uint16_t connectIdx = packet.data[pos]
                    | (static_cast<uint16_t>(packet.data[pos + 1]) << 8);
                uint8_t pct   = packet.data[pos + 2];
                uint8_t type  = packet.data[pos + 3];  // unused in PC
                char svName[33] = {0};
                memcpy(svName, packet.data + pos + 4, 32);
                svName[32] = '\0';

                // PC index encoding: groupIndex = connectIdx / 20
                int groupIdx = connectIdx / 20;

                // Find or create server group
                ServerGroup* group = nullptr;
                for (auto& g : g_serverGroups) {
                    if (g.groupIndex == groupIdx) {
                        group = &g;
                        break;
                    }
                }
                if (!group) {
                    ServerGroup newGroup;
                    newGroup.groupIndex = groupIdx;
                    snprintf(newGroup.name, sizeof(newGroup.name), "%s", svName);
                    newGroup.widthPos = (groupIdx % 2 == 0) ? 0 : 1; // left/right like PC
                    g_serverGroups.push_back(newGroup);
                    group = &g_serverGroups.back();
                    LOGI("  New group[%d]: idx=%d name='%s'", (int)g_serverGroups.size()-1,
                         groupIdx, svName);
                }

                // Add sub-server
                SubServerInfo sub;
                sub.connectIndex = connectIdx;
                sub.subIndex = (connectIdx % 20) + 1;
                sub.percent = pct;
                sub.nonPvP = 0;  // would need ServerList.bmd for real nonPvP flags

                // Display name: matching PC's CServerInfo::m_bName format
                const char* pctLabel = "Normal";
                if (pct >= 128)      pctLabel = "Closed";
                else if (pct >= 100) pctLabel = "Full";
                else if (pct >= 80)  pctLabel = "High";
                snprintf(sub.name, sizeof(sub.name), "%s-%d %s",
                         group->name, sub.subIndex, pctLabel);

                group->servers.push_back(sub);
                LOGI("  Sub[%d]: idx=%d connect=%u pct=%d name='%s'",
                     i, sub.subIndex, connectIdx, pct, sub.name);
            }

            LOGI("Parsed %zu groups with %d total sub-servers",
                 g_serverGroups.size(), totalServer);

            // Task 3: 按组索引排序 (PC: ServerList.bmd 脚本顺序)
            std::sort(g_serverGroups.begin(), g_serverGroups.end(),
                [](const ServerGroup& a, const ServerGroup& b) {
                    return a.groupIndex < b.groupIndex;
                });
            for (auto& g : g_serverGroups) {
                std::sort(g.servers.begin(), g.servers.end(),
                    [](const SubServerInfo& a, const SubServerInfo& b) {
                        return a.subIndex < b.subIndex;
                    });
            }
            break;
        }
        default:
            LOGI("0xF4 unhandled subcode: 0x%02X", subCode);
            break;
    }
}

static void handleProtocol0xF3(const Packet& packet) {
    if (packet.size < 4) return;

    uint8_t subCode = packet.data[3];

    switch (subCode) {
        case 0x00: {
            // PC: ReceiveCharacterList — character list response
            // The packet format varies by server version. Common layout:
            // {C1/C2, Size, F3, 00, ClassCode, MoveCount, Count/ExtWarehouse, ...chars...}
            // Each char entry: {slot, name[10], level[2], ctlCode, class, equipment[13*4], ...}
            if (g_currentState != ProtocolState::REQUEST_CHARACTERS_LIST) {
                LOGI("Char list ignored (state=%s)", stateName(g_currentState));
                return;
            }

            g_characterList.clear();

            // Determine offset: after header(4) + classCode(1) + moveCnt(1) + count(1)
            uint8_t classCode = (packet.size >= 6) ? packet.data[4] : 0;
            uint8_t moveCnt   = (packet.size >= 6) ? packet.data[5] : 0;
            uint8_t count     = (packet.size >= 7) ? packet.data[6] : 0;
            LOGI("Character list: classCode=%d moveCnt=%d count=%d", classCode, moveCnt, count);

            // Auto-detect entry size from packet body
            g_characterList.clear();
            int hdrSize = 7;
            int body = (int)packet.size - hdrSize;
            int entrySize = body / count;
            if (entrySize < 15) entrySize = 34;

            LOGI("  Auto-detect: body=%d count=%d entrySize=%d", body, count, entrySize);
            for (int ci = 0; ci < count; ci++) {
                int pos = hdrSize + ci * entrySize;
                if (pos + 14 > (int)packet.size) break;
                CharacterInfo c;
                c.slot = packet.data[pos];
                memcpy(c.name, packet.data + pos + 1, 10); c.name[10] = '\0';
                // Strip non-printable prefix byte from name
                if (c.name[0] > 0 && c.name[0] < ' ' && c.name[1] >= ' ') {
                    memmove(c.name, c.name + 1, 9); c.name[9] = '\0';
                }
                c.level    = packet.data[pos + 11] | (packet.data[pos + 12] << 8);
                c.ctlCode  = packet.data[pos + 13];
                c.classCode = packet.data[pos + 14];
                g_characterList.push_back(c);
                LOGI("  Char[%d]: slot=%d '%s' Lv.%d class=0x%02X",
                     ci, c.slot, c.name, c.level, c.classCode);
            }

            setState(ProtocolState::RECEIVE_CHARACTERS_LIST);
            break;
        }
        case 0x01: {
            // Character create response
            g_createResultColor = 2;
            if (packet.size > 4) {
                uint8_t result = packet.data[4];
                LOGI("Create character result: 0x%02X", result);
                switch (result) {
                    case 0x00:
                        snprintf(g_createResult, sizeof(g_createResult), "\xe5\x88\x9b\xe5\xbb\xba\xe6\x88\x90\xe5\x8a\x9f");
                        g_createResultColor = 1;
                        break;
                    case 0x01:
                        snprintf(g_createResult, sizeof(g_createResult), "\xe8\xa7\x92\xe8\x89\xb2\xe5\x90\x8d\xe5\xb7\xb2\xe5\xad\x98\xe5\x9c\xa8");
                        break;
                    case 0x02:
                        snprintf(g_createResult, sizeof(g_createResult), "\xe5\xb7\xb2\xe8\xbe\xbe\xe8\xa7\x92\xe8\x89\xb2\xe4\xb8\x8a\xe9\x99\x90");
                        break;
                    default:
                        snprintf(g_createResult, sizeof(g_createResult), "\xe5\x88\x9b\xe5\xbb\xba\xe5\xa4\xb1\xe8\xb4\xa5\xef\xbc\x8c\xe9\x94\x99\xe8\xaf\xaf\xe7\xa0\x81%d", result);
                        break;
                }
            }
            // Refresh char list
            setState(ProtocolState::REQUEST_CHARACTERS_LIST);
            break;
        }
        case 0x02: {
            // Character delete response
            g_createResultColor = 2;
            if (packet.size > 4) {
                uint8_t result = packet.data[4];
                LOGI("Delete character result: 0x%02X", result);
                if (result == 0x00) {
                    snprintf(g_createResult, sizeof(g_createResult), "\xe5\x88\xa0\xe9\x99\xa4\xe6\x88\x90\xe5\x8a\x9f");
                    g_createResultColor = 1;
                } else {
                    snprintf(g_createResult, sizeof(g_createResult), "\xe5\x88\xa0\xe9\x99\xa4\xe5\xa4\xb1\xe8\xb4\xa5");
                }
            }
            setState(ProtocolState::REQUEST_CHARACTERS_LIST);
            break;
        }
        case 0x03: {
            // PC: ReceiveJoinMapServer — confirm we entered the game world
            if (packet.size >= 8) {
                uint8_t x   = packet.data[4];
                uint8_t y   = packet.data[5];
                uint8_t map = packet.data[6];
                uint8_t dir = packet.data[7];
                LOGI("Join map server success! Map=%d X=%d Y=%d Dir=%d", map, x, y, dir);

                // Task 2: 释放角色选择场景资源
                ReleaseCharacterSceneData();
                LOGI("*** ReleaseCharacterSceneData() called");

                // Task 4: 重置超时
                g_timeoutAccum = 0.0f;

                // Critical: pre-set PC engine globals BEFORE changing SceneFlag.
                CurrentProtocolState = PC_RECEIVE_JOIN_MAP_SERVER;
                EnableMainRender = true;
                LoadingWorld = 30;
                LOGI("*** EnableMainRender=true, CPState=61, LoadingWorld=30");

                // Task 3: Auto-show core in-game HUD windows (NewUI)
                // Matches PC default when entering MAIN_SCENE
                NewUI_ShowWindow(53,  true); // INTERFACE_MAINFRAME  — HP/XP bar
                NewUI_ShowWindow(72,  true); // INTERFACE_MINI_MAP   — minimap
                NewUI_ShowWindow(54,  true); // INTERFACE_SKILL_LIST — skill bar
                NewUI_ShowWindow(56,  true); // INTERFACE_BUFF_WINDOW — buff status
                NewUI_ShowWindow(33,  true); // INTERFACE_CHATINPUTBOX
                NewUI_ShowWindow(41,  true); // INTERFACE_CHATLOGWINDOW
                NewUI_ShowWindow(39,  true); // INTERFACE_QUICK_COMMAND
                NewUI_ShowWindow(82,  true); // INTERFACE_HOTKEY
                LOGI("*** Default NewUI HUD windows shown");

                SceneFlag = MAIN_SCENE;
                setState(ProtocolState::RECEIVE_JOIN_MAP_SERVER);
            }
            break;
        }
        default:
            LOGI("0xF3 unhandled subcode: 0x%02X", subCode);
            break;
    }
}

static void handleProtocol0x00(const Packet& packet) {
    if (packet.size < 5) return;

    uint8_t subCode = packet.data[3];

    if (g_currentState == ProtocolState::RECEIVE_LOG_IN_SUCCESS ||
        g_currentState == ProtocolState::REQUEST_CHARACTERS_LIST) {
        // Some server versions send character list under protocol 0x00
        LOGI("Protocol 0x00 in login flow — treating as char list (sub 0x%02X)", subCode);
        if (subCode == 0x00) {
            // Parse character list (simplified)
            g_characterList.clear();
            char hexBuf[128] = {0};
            for (size_t i = 0; i < 24 && i < packet.size; i++) {
                sprintf(hexBuf + i*3, "%02X ", packet.data[i]);
            }
            LOGI("0x00 sub0 char list[0..23]: %s", hexBuf);
        }
        setState(ProtocolState::RECEIVE_CHARACTERS_LIST);
    } else if (g_currentState == ProtocolState::RECEIVE_JOIN_MAP_SERVER) {
        // Already in game — normal protocol dispatch
    } else {
        LOGI("Protocol 0x00: subcode 0x%02X in state %s (unhandled)",
             subCode, stateName(g_currentState));
    }
}

} // namespace ProtocolDispatch
