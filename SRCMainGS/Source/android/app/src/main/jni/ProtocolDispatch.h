#pragma once

#include "MuNetwork.h"
#include <cstdint>
#include <vector>
#include <string>

// ============================================================================
// ProtocolDispatch — 协议状态机和数据包分发
// ============================================================================
//
// PC 标准流程:
//   DISCONNECTED
//     ↓ connectToServer(CS_IP, CS_PORT)
//   REQUEST_JOIN_SERVER → RECEIVE_JOIN_SERVER_WAITING
//     ↓ CS 返回服务器列表 (F4/06)，UI 展示
//     ↓ 用户选择服务器 → selectServer(index)
//     ↓ CS 返回 GS 地址 (F4/03)
//   RECEIVE_JOIN_SERVER_SUCCESS → 自动重连到 GS
//   GS_JOIN_REQUESTED
//     ↓ GS 验证通过 (F1/00) → UI 显示登录框
//     ↓ 用户输入账号密码 → sendLogin()
//   REQUEST_LOG_IN
//     ↓ GS 返回登录结果 (F1/01)
//   RECEIVE_LOG_IN_SUCCESS
//     ↓ UI 切到 CHARACTER_SCENE, 用户选择角色
//     ↓ 用户点击进入 → selectCharacter(slot) → LOADING_SCENE
//   RECEIVE_JOIN_MAP_SERVER → MAIN_SCENE

namespace ProtocolDispatch {

// ========== 服务器列表 ==========

struct SubServerInfo {
    int     subIndex;
    int     connectIndex;
    int     percent;
    int     nonPvP;
    char    name[64];
};

struct ServerGroup {
    int     groupIndex;
    int     widthPos;
    char    name[32];
    std::vector<SubServerInfo> servers;
};

// ========== 角色列表 ==========

struct CharacterInfo {
    int     slot = 0;
    char    name[11] = {};
    int     level = 0;
    uint8_t classCode = 0;
    uint8_t ctlCode = 0;
};

// ========== 公开接口 ==========

void init();
void update(float deltaTime);
void processPacket(const Packet& packet);
void connectToServer(const char* ip, uint16_t port);
void disconnect();
ProtocolState getState();
const char* getStateName();

// 错误信息
const char* getLastError();
void clearLastError();

// 服务器列表
const std::vector<ServerGroup>& getServerGroups();
void selectServer(int groupIdx, int subIdx);
void refreshServerList();

// 登录
void setCredentials(const char* account, const char* password);
void sendLogin();

// 记住账号 (Task 3 — 持久化到文件)
void setCredPath(const char* internalPath);
void saveCredentials(const char* account, const char* password);
void loadCredentials();
const char* getSavedAccount();
const char* getSavedPassword();

// 角色列表
const std::vector<CharacterInfo>& getCharacterList();
void selectCharacter(int slotIndex);

// 角色创建/删除
void createCharacter(const char* name, uint8_t classCode);
void deleteCharacter(int slotIndex);
const char* getCreateCharResult();
int getCreateCharResultColor();

// 账号注册
void sendRegister(const char* account, const char* password);
const char* getRegResult();
int getRegResultColor();

// 自动重连 (Task 5)
void enableAutoReconnect(bool enable);
bool isAutoReconnectEnabled();

} // namespace ProtocolDispatch
