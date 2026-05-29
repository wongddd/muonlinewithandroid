#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <queue>

// ============================================================================
// Packet Header Structures (mirrors Protocol.h / WSclient.h)
// ============================================================================

// C1: 普通短包 (1字节大小, 不加密)
struct PBMSG_HEAD {
    uint8_t type;  // 0xC1
    uint8_t size;  // 总包长度
    uint8_t head;  // 协议码

    void set(uint8_t h, uint8_t s) { type = 0xC1; size = s; head = h; }
};

// C1: 带子码短包
struct PSBMSG_HEAD {
    uint8_t type;   // 0xC1
    uint8_t size;   // 总包长度
    uint8_t head;   // 协议码
    uint8_t subh;   // 子码

    void set(uint8_t h, uint8_t sh, uint8_t s) { type = 0xC1; size = s; head = h; subh = sh; }
};

// C2: 普通长包 (2字节大小, 大端)
struct PWMSG_HEAD {
    uint8_t type;    // 0xC2
    uint8_t sizeH;   // size 高字节
    uint8_t sizeL;   // size 低字节
    uint8_t head;    // 协议码

    void set(uint8_t h, uint16_t s) {
        type = 0xC2;
        sizeH = static_cast<uint8_t>((s >> 8) & 0xFF);
        sizeL = static_cast<uint8_t>(s & 0xFF);
        head = h;
    }
    uint16_t getSize() const { return (static_cast<uint16_t>(sizeH) << 8) | sizeL; }
};

// C2: 带子码长包
struct PSWMSG_HEAD {
    uint8_t type;    // 0xC2
    uint8_t sizeH;
    uint8_t sizeL;
    uint8_t head;    // 协议码
    uint8_t subh;    // 子码

    void set(uint8_t h, uint8_t sh, uint16_t s) {
        type = 0xC2;
        sizeH = static_cast<uint8_t>((s >> 8) & 0xFF);
        sizeL = static_cast<uint8_t>(s & 0xFF);
        head = h;
        subh = sh;
    }
    uint16_t getSize() const { return (static_cast<uint16_t>(sizeH) << 8) | sizeL; }
};

// ============================================================================
// 连接状态机 (mirrors WSclient.h)
// ============================================================================
enum class ProtocolState : int {
    DISCONNECTED                 = -1,
    REQUEST_JOIN_SERVER          = 0,   // Connect to CS, send HWID
    RECEIVE_JOIN_SERVER_WAITING  = 1,   // Waiting for CS responses
    RECEIVE_JOIN_SERVER_SUCCESS  = 2,   // Got GS redirect, reconnecting
    RECEIVE_JOIN_SERVER_FAIL     = 3,   // Connection failed
    GS_JOIN_REQUESTED            = 10,  // Join sent to GS, waiting for result
    REQUEST_LOG_IN               = 19,  // Send login packet
    RECEIVE_LOG_IN_SUCCESS       = 20,  // Login OK
    REQUEST_CHARACTERS_LIST      = 50,
    RECEIVE_CHARACTERS_LIST      = 51,
    REQUEST_JOIN_MAP_SERVER      = 60,
    RECEIVE_JOIN_MAP_SERVER      = 61,
};

// ============================================================================
// 完整数据包（网络线程 → 游戏线程）
// ============================================================================
constexpr size_t MAX_PACKET_SIZE = 16384;

struct Packet {
    uint8_t data[MAX_PACKET_SIZE];
    size_t size = 0;
    bool encrypted = false; // 是否为 C3/C4 解密后的数据包
};

// ============================================================================
// MuNetwork — POSIX socket 网络层
// ============================================================================
namespace MuNetwork {

// 初始化：设置 socket 层加解密密钥 (EncDecKey[2])
void initKeys(uint8_t key0, uint8_t key1);

// SimpleModulus 密钥初始化（C3/C4 封包加解密）
// buffer: 原始密钥文件内容（48字节: Modulus[4]+Key[4]+Xor[4]）
bool initSimpleModulusDecKey(const uint8_t* buffer, size_t bufferSize);
bool initSimpleModulusEncKey(const uint8_t* buffer, size_t bufferSize);
bool isSimpleModulusReady();
void runSimpleModulusSelfTest();

// 连接到游戏服务器
bool connect(const char* ip, uint16_t port);

// 断开连接
void disconnect();

// 每帧调用：接收数据、解析包头、解密 C3/C4、入队完整包
void update();

// 取下一个完整数据包（返回 true 表示有数据）
bool getNextPacket(Packet& outPacket);

// 发送原始 C1/C2 数据（ConnectServer，自动应用 XOR+SUB 加密）
// encryptBody=true: 先用 SimpleModulus 加密并包裹为 C3/C4（登录/登出等敏感包）
bool send(const uint8_t* data, size_t size, bool encryptBody = false);

// 连接状态
bool isConnected();
ProtocolState getState();
void setState(ProtocolState state);

// 获取上一个错误信息
const char* getLastError();

// socket 层加解密（公开，供参考）
void encryptData(uint8_t* buf, size_t size);
void decryptData(uint8_t* buf, size_t size);

} // namespace MuNetwork
