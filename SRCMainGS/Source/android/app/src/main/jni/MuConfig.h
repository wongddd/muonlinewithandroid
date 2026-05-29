#pragma once
#include <cstdint>

// ============================================================================
// MuConfig — 设置持久化 (P1-4)
// 保存/加载配置文件到 internal storage
// ============================================================================

struct GameConfig {
    int  language = 1;         // 0=EN, 1=CN
    int  autoReconnect = 0;    // 0=off, 1=on
    int  saveAccount = 0;
    int  bgmVolume = 70;       // 0-100
    int  sfxVolume = 70;
    int  screenBrightness = 100;
    char account[32] = {};
    char password[32] = {};
};

bool config_load(GameConfig& cfg, const char* basePath);
bool config_save(const GameConfig& cfg, const char* basePath);
