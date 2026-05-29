#pragma once

#include <cstdint>

// SimpleModulus 加密库 — 从 GameServer/PacketManager.cpp 移植
//
// 算法：8字节明文 → 11字节密文的分组密码
//   4轮加密 (XOR + Key * value % Modulus)
//   链式异或混淆
//   位级别打包 (AddBits/Shift)
//   校验和验证 (checksum)
//
// 密钥文件格式:
//   ChunkHeader { WORD id(4370), DWORD size(36) }
//   DWORD Modulus[4] (与 SaveLoadXOR 异或存储)
//   DWORD Key[4]
//   DWORD Xor[4]

constexpr int SIMPLE_MODULUS_BLOCK_SIZE = 8;    // 输入块大小
constexpr int SIMPLE_MODULUS_ENCRYPTED_SIZE = 11; // 输出块大小
constexpr int SIMPLE_MODULUS_KEY_COUNT = 4;       // 密钥数量

struct ModulusKeyData {
    uint32_t Modulus[4];
    uint32_t Key[4];
    uint32_t Xor[4];
};

class SimpleModulus {
public:
    SimpleModulus();
    ~SimpleModulus();

    // 加载密钥
    bool loadEncryptionKey(const uint8_t* buffer, size_t bufferSize);
    bool loadDecryptionKey(const uint8_t* buffer, size_t bufferSize);
    bool loadKeyFromBuffer(const uint8_t* buffer, bool loadModulus, bool loadKey, bool loadXor);

    // 加解密整条消息
    int encrypt(uint8_t* target, const uint8_t* source, int size);
    int decrypt(uint8_t* target, const uint8_t* source, int size);

    // 获取最后一个解密块的实际大小
    int getLastDecryptedSize() const { return m_lastDecryptedSize; }

    // 密钥状态查询
    bool hasEncKey() const { return m_hasEncKey; }
    bool hasDecKey() const { return m_hasDecKey; }

    // Debug: log key parameters
    void logKeyParams() const;
    void encryptBlockForDebug(uint8_t* target, const uint8_t* source, int size);

private:
    // 分组加解密
    void encryptBlock(uint8_t* target, const uint8_t* source, int size);
    int  decryptBlock(uint8_t* target, const uint8_t* source);

    // 位操作助手
    int  addBits(uint8_t* target, int targetBitPos, const uint8_t* source, int sourceBitPos, int bitCount);
    void shift(uint8_t* buffer, int byteCount, int shiftBits);
    static int getByteOfBit(int bitPos);

    ModulusKeyData m_encKey;
    ModulusKeyData m_decKey;
    bool m_hasEncKey;
    bool m_hasDecKey;

    int m_lastDecryptedSize;

    // SaveLoad XOR 密钥 (用于解密密钥文件)
    static const uint32_t SAVE_LOAD_XOR[4];
};
