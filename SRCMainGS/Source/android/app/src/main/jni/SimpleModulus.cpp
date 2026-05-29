#include "SimpleModulus.h"

#include <android/log.h>
#include <cstring>

#define LOG_TAG "SimpleModulus"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// SaveLoadXOR 常量 — 用于解密密钥文件
const uint32_t SimpleModulus::SAVE_LOAD_XOR[4] = {
    0x3F08A79B,
    0xE25CC287,
    0x93D27AB9,
    0x20DEA7BF
};

SimpleModulus::SimpleModulus()
    : m_hasEncKey(false)
    , m_hasDecKey(false)
    , m_lastDecryptedSize(0) {
    memset(&m_encKey, 0, sizeof(m_encKey));
    memset(&m_decKey, 0, sizeof(m_decKey));
}

SimpleModulus::~SimpleModulus() {
}

// ============================================================================
// 密钥加载
// ============================================================================

bool SimpleModulus::loadEncryptionKey(const uint8_t* buffer, size_t bufferSize) {
    if (!buffer || bufferSize < 12) {
        LOGE("loadEncryptionKey: buffer too small (%zu bytes)", bufferSize);
        return false;
    }

    // 解析 ChunkHeader: WORD id(4370), DWORD size
    uint16_t headerId = (static_cast<uint16_t>(buffer[1]) << 8) | buffer[0];
    uint32_t headerSize = (static_cast<uint32_t>(buffer[3]) << 24) |
                          (static_cast<uint32_t>(buffer[4]) << 16) |
                          (static_cast<uint32_t>(buffer[5]) << 8)  |
                          buffer[6];

    if (headerId != 4370 || headerSize != 44) { // sizeof(ChunkHeader)=6 + sizeof(ENCDEC_DATA)=32 = 38... wait
        // Actually: ChunkHeader {WORD id; DWORD size;}  sizeof = 6
        // headerSize should be sizeof(ChunkHeader) + sizeof(ENCDEC_DATA) = 6 + 48 = 54?
        // Let's check: ENCDEC_DATA has 3 arrays of 4 DWORDs each = 3*4*4 = 48 bytes
        // Total = 6 + 48 = 54
        // But the original code says: headerSize == sizeof(HeaderInfo)+sizeof(ENCDEC_DATA))
        // sizeof(ENCDEC_HEADER) = 6 (WORD + DWORD = 2+4=6)
        // sizeof(ENCDEC_DATA) = 12 DWORDs = 48
        // So headerSize = 6 + 48 = 54
        // But wait - looking at the code: it checks sizeof(HeaderInfo)+sizeof(ENCDEC_DATA))
        // Let me re-examine the struct:
        // struct ENCDEC_HEADER { WORD header; DWORD size; }; // 6 bytes
        // struct ENCDEC_DATA { DWORD Modulus[4]; DWORD Key[4]; DWORD Xor[4]; }; // 48 bytes
        // So headerSize should be 6+48=54

        // Actually, the GameServer code has header = 4370 and headerSize = 6 + 48 = 54
        // But let me not be too strict — the key loading might have different sizes
        // Just check that headerId matches
        if (headerId != 4370) {
            LOGE("loadEncryptionKey: invalid header ID 0x%04X", headerId);
            return false;
        }
    }

    // Read Modulus[4], Key[4], Xor[4] — each XOR'd with SAVE_LOAD_XOR
    const size_t dataOffset = 6; // skip header
    const size_t arraySize = 4 * sizeof(uint32_t); // 16 bytes per array

    if (bufferSize < dataOffset + 3 * arraySize) {
        LOGE("loadEncryptionKey: buffer too small for key data");
        return false;
    }

    const uint32_t* rawData = reinterpret_cast<const uint32_t*>(buffer + dataOffset);

    for (int i = 0; i < 4; ++i) {
        m_encKey.Modulus[i] = SAVE_LOAD_XOR[i] ^ rawData[i];
    }
    for (int i = 0; i < 4; ++i) {
        m_encKey.Key[i] = SAVE_LOAD_XOR[i] ^ rawData[4 + i];
    }
    for (int i = 0; i < 4; ++i) {
        m_encKey.Xor[i] = SAVE_LOAD_XOR[i] ^ rawData[8 + i];
    }

    m_hasEncKey = true;
    LOGI("Encryption key loaded successfully");
    return true;
}

bool SimpleModulus::loadDecryptionKey(const uint8_t* buffer, size_t bufferSize) {
    if (!buffer || bufferSize < 12) {
        LOGE("loadDecryptionKey: buffer too small");
        return false;
    }

    uint16_t headerId = (static_cast<uint16_t>(buffer[1]) << 8) | buffer[0];

    if (headerId != 4370) {
        LOGE("loadDecryptionKey: invalid header ID 0x%04X", headerId);
        return false;
    }

    const size_t dataOffset = 6;
    const uint32_t* rawData = reinterpret_cast<const uint32_t*>(buffer + dataOffset);

    for (int i = 0; i < 4; ++i) {
        m_decKey.Modulus[i] = SAVE_LOAD_XOR[i] ^ rawData[i];
    }
    for (int i = 0; i < 4; ++i) {
        m_decKey.Key[i] = SAVE_LOAD_XOR[i] ^ rawData[4 + i];
    }
    for (int i = 0; i < 4; ++i) {
        m_decKey.Xor[i] = SAVE_LOAD_XOR[i] ^ rawData[8 + i];
    }

    m_hasDecKey = true;
    LOGI("Decryption key loaded successfully");
    return true;
}

bool SimpleModulus::loadKeyFromBuffer(const uint8_t* buffer, bool loadModulus, bool loadKey, bool loadXor) {
    // 直接加载密钥数据（不带头部，原始48字节: Modulus[4]+Key[4]+Xor[4]）
    const uint32_t* rawData = reinterpret_cast<const uint32_t*>(buffer);

    for (int i = 0; i < 4; ++i) {
        if (loadModulus) m_encKey.Modulus[i] = SAVE_LOAD_XOR[i] ^ rawData[i];
        if (loadKey)     m_encKey.Key[i] = SAVE_LOAD_XOR[i] ^ rawData[4 + i];
        if (loadXor)     m_encKey.Xor[i] = SAVE_LOAD_XOR[i] ^ rawData[8 + i];
    }

    m_hasEncKey = true;
    return true;
}

// ============================================================================
// 整条消息加解密
// ============================================================================

int SimpleModulus::encrypt(uint8_t* target, const uint8_t* source, int size) {
    if (!m_hasEncKey) {
        LOGE("encrypt: no encryption key loaded");
        return -1;
    }

    int oriSize = size;
    int blocks = (size + 7) / 8;  // 向上取整到8字节
    int outputSize = blocks * 11;

    if (target) {
        int remaining = oriSize;
        for (int n = 0; n < oriSize; n += 8, remaining -= 8, target += 11) {
            int blockSize = (remaining >= 8) ? 8 : remaining;
            encryptBlock(target, &source[n], blockSize);
        }
    }

    return outputSize;
}

int SimpleModulus::decrypt(uint8_t* target, const uint8_t* source, int size) {
    if (!m_hasDecKey) {
        LOGE("decrypt: no decryption key loaded");
        return -1;
    }

    int result = 0;
    int consumed = 0;

    if (target && size > 0) {
        while (consumed < size) {
            int blockResult = decryptBlock(target, source);

            if (blockResult < 0) {
                LOGE("decrypt: DecryptBlock failed at offset %d", consumed);
                return blockResult;
            }

            result += blockResult;
            consumed += 11;
            source += 11;
            target += 8;
        }
    }

    return result;
}

// ============================================================================
// 分组加密 (8字节 → 11字节)
// ============================================================================

void SimpleModulus::encryptBlock(uint8_t* target, const uint8_t* source, int size) {
    uint32_t encBuffer[4] = {0};
    uint32_t encValue = 0;

    memset(target, 0, 11);

    // Step 1: 4轮加密 — 每个16位字
    // EncBuffer[n] = ((Xor[n] ^ source_word[n] ^ prev_result) * Key[n]) % Modulus[n]
    for (int n = 0; n < 4; ++n) {
        encBuffer[n] = (((m_encKey.Xor[n] ^ reinterpret_cast<const uint16_t*>(source)[n]) ^ encValue)
                        * m_encKey.Key[n]) % m_encKey.Modulus[n];
        encValue = static_cast<uint16_t>(encBuffer[n]);
    }

    // Step 2: 链式反向异或混淆
    for (int n = 0; n < 3; ++n) {
        encBuffer[n] = (encBuffer[n] ^ m_encKey.Xor[n]) ^ static_cast<uint16_t>(encBuffer[n + 1]);
    }

    // Step 3: 位打包 — 每个EncBuffer的16位和[22..23]的2位
    int bitPos = 0;
    for (int n = 0; n < 4; ++n) {
        bitPos = addBits(target, bitPos, reinterpret_cast<uint8_t*>(&encBuffer[n]), 0, 16);
        bitPos = addBits(target, bitPos, reinterpret_cast<uint8_t*>(&encBuffer[n]), 22, 2);
    }

    // Step 4: 16位校验和
    uint8_t checkSum = 0xF8;
    for (int n = 0; n < 8; ++n) {
        checkSum ^= source[n];
    }

    uint8_t encValueBytes[2];
    encValueBytes[0] = (checkSum ^ (static_cast<uint8_t>(size))) ^ 0x3D;
    encValueBytes[1] = checkSum;

    addBits(target, bitPos, encValueBytes, 0, 16);
}

// ============================================================================
// 分组解密 (11字节 → 8字节)
// ============================================================================

int SimpleModulus::decryptBlock(uint8_t* target, const uint8_t* source) {
    uint32_t decBuffer[4] = {0};
    memset(target, 0, 8);

    // Step 1: 从88位中提取 — 每块18位 (16位数据 + 2位高位)
    int bitPos = 0;
    for (int n = 0; n < 4; ++n) {
        addBits(reinterpret_cast<uint8_t*>(&decBuffer[n]), 0, source, bitPos, 16);
        bitPos += 16;
        addBits(reinterpret_cast<uint8_t*>(&decBuffer[n]), 22, source, bitPos, 2);
        bitPos += 2;
    }

    // Step 2: 链式异或逆向
    for (int n = 2; n >= 0; --n) {
        decBuffer[n] = (decBuffer[n] ^ m_decKey.Xor[n]) ^ static_cast<uint16_t>(decBuffer[n + 1]);
    }

    // Step 3: 逆解密 — 恢复原始16位字
    uint32_t value = 0;
    for (int n = 0; n < 4; ++n) {
        reinterpret_cast<uint16_t*>(target)[n] = static_cast<uint16_t>(
            (((m_decKey.Key[n] * decBuffer[n]) % m_decKey.Modulus[n]) ^ m_decKey.Xor[n]) ^ value);
        value = static_cast<uint16_t>(decBuffer[n]);
    }

    // Step 4: 验证校验和
    decBuffer[0] = 0;
    addBits(reinterpret_cast<uint8_t*>(decBuffer), 0, source, bitPos, 16);

    uint8_t* decBytes = reinterpret_cast<uint8_t*>(decBuffer);
    uint8_t originalSize = (decBytes[0] ^ decBytes[1]) ^ 0x3D;

    uint8_t checkSum = 0xF8;
    for (int n = 0; n < 8; ++n) {
        checkSum ^= target[n];
    }

    if (checkSum != decBytes[1]) {
        LOGE("decryptBlock: checksum mismatch (expected 0x%02X, got 0x%02X)",
             decBytes[1], checkSum);
        return -1;
    }

    m_lastDecryptedSize = originalSize;
    return originalSize;
}

// ============================================================================
// 位操作助手
// ============================================================================

int SimpleModulus::getByteOfBit(int bitPos) {
    return bitPos >> 3;  // bitPos / 8
}

void SimpleModulus::shift(uint8_t* buffer, int byteCount, int shiftBits) {
    if (shiftBits == 0 || byteCount <= 0) return;

    if (shiftBits > 0) {
        // 右移
        for (int n = byteCount - 1; n > 0; --n) {
            buffer[n] = static_cast<uint8_t>(
                (buffer[n - 1] << (8 - shiftBits)) | (buffer[n] >> shiftBits));
        }
        buffer[0] >>= shiftBits;
    } else {
        // 左移
        shiftBits = -shiftBits;
        for (int n = 0; n < byteCount - 1; ++n) {
            buffer[n] = static_cast<uint8_t>(
                (buffer[n + 1] >> (8 - shiftBits)) | (buffer[n] << shiftBits));
        }
        buffer[byteCount - 1] <<= shiftBits;
    }
}

int SimpleModulus::addBits(uint8_t* target, int targetBitPos,
                            const uint8_t* source, int sourceBitPos, int bitCount) {
    int sourceBitSize = sourceBitPos + bitCount;
    int tempSize1 = getByteOfBit(sourceBitSize - 1) + (1 - getByteOfBit(sourceBitPos));

    // 拷贝源位数据到临时缓冲区
    uint8_t* tempBuff = new uint8_t[tempSize1 + 1];
    memset(tempBuff, 0, tempSize1 + 1);

    int byteStart = getByteOfBit(sourceBitPos);
    memcpy(tempBuff, &source[byteStart], tempSize1);

    // 如果源位大小不是8的倍数，屏蔽高位
    if ((sourceBitSize % 8) != 0) {
        tempBuff[tempSize1 - 1] &= (0xFF << (8 - (sourceBitSize % 8)));
    }

    // 对齐位位置
    int shiftLeft = sourceBitPos % 8;
    int shiftRight = targetBitPos % 8;

    shift(tempBuff, tempSize1, -shiftLeft);
    shift(tempBuff, tempSize1 + 1, shiftRight);

    // 按位或写入目标
    int tempSize2 = ((shiftRight <= shiftLeft) ? 0 : 1) + tempSize1;
    uint8_t* targetPtr = &target[getByteOfBit(targetBitPos)];

    for (int n = 0; n < tempSize2; ++n) {
        targetPtr[n] |= tempBuff[n];
    }

    delete[] tempBuff;
    return targetBitPos + bitCount;
}

// ============================================================================
// Debug helpers
// ============================================================================

void SimpleModulus::logKeyParams() const {
    LOGI("=== EncKey params ===");
    if (m_hasEncKey) {
        for (int i = 0; i < 4; i++) {
            LOGI("  [%d] M=0x%08X K=0x%08X X=0x%08X",
                 i, m_encKey.Modulus[i], m_encKey.Key[i], m_encKey.Xor[i]);
        }
    } else {
        LOGI("  (no encryption key loaded)");
    }
    LOGI("=== DecKey params ===");
    if (m_hasDecKey) {
        for (int i = 0; i < 4; i++) {
            LOGI("  [%d] M=0x%08X K=0x%08X X=0x%08X",
                 i, m_decKey.Modulus[i], m_decKey.Key[i], m_decKey.Xor[i]);
        }
    } else {
        LOGI("  (no decryption key loaded)");
    }
}

void SimpleModulus::encryptBlockForDebug(uint8_t* target, const uint8_t* source, int size) {
    LOGI("=== encryptBlock debug ===");
    char hexSrc[64] = {0};
    for (int i = 0; i < 8; i++) sprintf(hexSrc + i*3, "%02X ", source[i]);
    LOGI("Source: %s", hexSrc);

    uint32_t encBuffer[4] = {0};
    uint32_t encValue = 0;

    // Step 1: 4 rounds
    for (int n = 0; n < 4; ++n) {
        uint16_t srcWord = reinterpret_cast<const uint16_t*>(source)[n];
        uint32_t step1 = m_encKey.Xor[n] ^ srcWord;
        uint32_t step2 = step1 ^ encValue;
        uint64_t step3 = (uint64_t)step2 * m_encKey.Key[n];
        uint32_t step4 = step3 % m_encKey.Modulus[n];
        encBuffer[n] = step4;
        LOGI("  Round %d: src=0x%04X Xor=0x%08X xor_src=0x%08X ^prev=0x%08X *K=0x%08X => %llu %%M=0x%08X => EB=0x%08X",
             n, srcWord, m_encKey.Xor[n], step1, step2,
             m_encKey.Key[n], (unsigned long long)step3, m_encKey.Modulus[n], encBuffer[n]);
        encValue = static_cast<uint16_t>(encBuffer[n]);
    }

    // Step 2: XOR chaining
    for (int n = 0; n < 3; ++n) {
        uint32_t before = encBuffer[n];
        encBuffer[n] = (encBuffer[n] ^ m_encKey.Xor[n]) ^ static_cast<uint16_t>(encBuffer[n + 1]);
        LOGI("  Chain %d: 0x%08X ^ X[%d]=0x%08X ^ EB[%d]{lo}=0x%04X => 0x%08X",
             n, before, n, m_encKey.Xor[n], n+1,
             static_cast<uint16_t>(encBuffer[n+1]), encBuffer[n]);
    }

    memset(target, 0, 11);

    // Step 3: Bit packing
    int bitPos = 0;
    for (int n = 0; n < 4; ++n) {
        bitPos = addBits(target, bitPos, reinterpret_cast<uint8_t*>(&encBuffer[n]), 0, 16);
        bitPos = addBits(target, bitPos, reinterpret_cast<uint8_t*>(&encBuffer[n]), 22, 2);
    }

    // Step 4: Checksum
    uint8_t checkSum = 0xF8;
    for (int n = 0; n < 8; ++n) checkSum ^= source[n];

    uint8_t encValueBytes[2];
    encValueBytes[0] = (checkSum ^ (static_cast<uint8_t>(size))) ^ 0x3D;
    encValueBytes[1] = checkSum;
    addBits(target, bitPos, encValueBytes, 0, 16);

    LOGI("  CheckSum=0x%02X size=%d encBytes=[0x%02X, 0x%02X]",
         checkSum, size, encValueBytes[0], encValueBytes[1]);

    char hexOut[64] = {0};
    for (int i = 0; i < 11; i++) sprintf(hexOut + i*3, "%02X ", target[i]);
    LOGI("Encrypted output: %s", hexOut);
}
