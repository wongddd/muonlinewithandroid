#pragma once

// ============================================================================
// MuStringTable — 多语言字符串表 (P2-7)
// 使用方式: MuText(MUSTR_CONNECT)  → "Connect" / "连接"
// ============================================================================

enum MuStringID {
    MUSTR_CONNECT,
    MUSTR_CONNECTING_CS,
    MUSTR_SERVER_LIST,
    MUSTR_SELECT_SERVER,
    MUSTR_GAME_SERVER,
    MUSTR_LOGIN,
    MUSTR_CANCEL,
    MUSTR_REGISTER,
    MUSTR_LOGOUT,
    MUSTR_ID,
    MUSTR_PW,
    MUSTR_SAVE_ACCOUNT,
    MUSTR_AUTO_RECONNECT,
    MUSTR_SHOW_PW,
    MUSTR_HIDE_PW,
    MUSTR_CHAR_SELECT,
    MUSTR_NO_CHARS,
    MUSTR_CREATE_CHAR,
    MUSTR_DELETE,
    MUSTR_DISCONNECT,
    MUSTR_DOWNLOAD_DATA,
    MUSTR_EXTRACTING,
    MUSTR_LOADING,
    MUSTR_ENTERING_WORLD,
    MUSTR_LOGIN_PROGRESS,

    MUSTR_ERR_PASSWORD,
    MUSTR_ERR_INVALID_ID,
    MUSTR_ERR_ALREADY_ON,
    MUSTR_ERR_SERVER_BUSY,
    MUSTR_ERR_BLOCKED,
    MUSTR_ERR_VERSION,
    MUSTR_ERR_CANT_CONNECT,
    MUSTR_ERR_CANT_CONNECT_GS,
    MUSTR_ERR_TIMEOUT,
    MUSTR_ERR_REJECTED,
    MUSTR_ERR_UNKNOWN,
    MUSTR_ERR_REG_FAIL,
    MUSTR_ERR_REG_PW_SHORT,
    MUSTR_ERR_REG_MISMATCH,
    MUSTR_ERR_EMPTY_NAME,
    MUSTR_ERR_NAME_TOO_SHORT,

    MUSTR_CLASS_DW,
    MUSTR_CLASS_DK,
    MUSTR_CLASS_FE,
    MUSTR_CLASS_MG,
    MUSTR_CLASS_DL,
    MUSTR_CLASS_SU,
    MUSTR_CLASS_RF,

    MUSTR_COUNT  // must be last
};

// 语言列表
enum MuLanguage { LANG_EN = 0, LANG_CN = 1 };

// 设置/获取语言
void muSetLanguage(MuLanguage lang);
MuLanguage muGetLanguage();

// 获取字符串 (中文或英文)
const char* muText(MuStringID id);

// 设置语言选择器回调 (由 UI 调用)
typedef void (*MuLangCallback)(MuLanguage lang);
void muSetLangCallback(MuLangCallback cb);
