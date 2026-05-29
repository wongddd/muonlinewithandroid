#include "MuNetwork.h"
#include "SimpleModulus.h"

#include <android/log.h>
#include <cstring>
#include <cstdio>
#include <cerrno>

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define LOG_TAG "MuNetwork"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global stubs — defined in wsctlc_stubs.cpp, used to bridge MuNetwork's BSD
// socket to the original CWsctlc::sSend() code path (MySend → ::send).
extern SOCKET g_stubSendSocket;

namespace MuNetwork {

// ============================================================================
// EncDecKey[2] — socket-layer XOR+SUB cipher
// Default values from CGMProtect::CreateKeyEnv()
// ============================================================================
static uint8_t g_encDecKey[2] = { 0xE2, 0x02 };

// ============================================================================
// Socket state
// ============================================================================
static int g_socket = -1;
static uint16_t g_port = 0;
static bool g_connected = false;
static ProtocolState g_state = ProtocolState::DISCONNECTED;
static std::string g_lastError;

// SimpleModulus instance for C3/C4 packet encryption
static SimpleModulus g_simpleModulus;
static uint8_t g_packetSerialSend = 0;

// Encryption is only applied on GameServer ports (55901-56000).
// ConnectServer (44404) traffic is plain C1/C2.
static bool isGameServerPort() {
    return g_port >= 55901 && g_port <= 56000;
}

// ============================================================================
// Receive buffer and packet queue
// ============================================================================
static constexpr size_t RECV_BUF_SIZE = 65536;
static uint8_t g_recvBuf[RECV_BUF_SIZE];
static size_t g_recvBufLen = 0;

static std::queue<Packet> g_packetQueue;
static std::mutex g_queueMutex;

// ============================================================================
// XorData scramble — reverses server-side PacketManager::XorData unscramble.
// Equivalent to CStreamPacketEngine::XorData (StreamPacketEngine.h:64).
// Applied to C1/C2 body bytes BEFORE XOR+SUB encryption.
//   C1: scrambles bytes [3..size-1] forward  (head at pos 2)
//   C2: scrambles bytes [4..size-1] forward  (head at pos 3)
// ============================================================================
static const uint8_t g_xorFilter[32] = {
    0xAB, 0x11, 0xCD, 0xFE, 0x18, 0x23, 0xC5, 0xA3,
    0xCA, 0x33, 0xC1, 0xCC, 0x66, 0x67, 0x21, 0xF3,
    0x32, 0x12, 0x15, 0x35, 0x29, 0xFF, 0xFE, 0x1D,
    0x44, 0xEF, 0xCD, 0x41, 0x26, 0x3C, 0x4E, 0x4D
};

static void xorDataScramble(uint8_t* buf, size_t size) {
    int start = (buf[0] == 0xC2) ? 4 : 3;
    for (int i = start; i < (int)size; ++i) {
        buf[i] ^= (buf[i - 1] ^ g_xorFilter[i % 32]);
    }
}

// ============================================================================
// XOR+SUB cipher (equivalent to wsctlc.cpp EncryptData/DecryptData)
// ============================================================================

void encryptData(uint8_t* buf, size_t size) {
    for (size_t n = 0; n < size; ++n) {
        buf[n] = (buf[n] + g_encDecKey[1]) ^ g_encDecKey[0];
    }
}

void decryptData(uint8_t* buf, size_t size) {
    for (size_t n = 0; n < size; ++n) {
        buf[n] = (buf[n] ^ g_encDecKey[0]) - g_encDecKey[1];
    }
}

// ============================================================================
// Key initialization
// ============================================================================

void initKeys(uint8_t key0, uint8_t key1) {
    g_encDecKey[0] = key0;
    g_encDecKey[1] = key1;
    LOGI("EncDecKey initialized: [0]=0x%02X, [1]=0x%02X", key0, key1);
}

// ============================================================================
// Socket connection management
// ============================================================================

static bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

bool connect(const char* ip, uint16_t port) {
    disconnect();

    g_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_socket < 0) {
        g_lastError = "socket() failed: " + std::string(strerror(errno));
        LOGE("%s", g_lastError.c_str());
        return false;
    }

    if (!setNonBlocking(g_socket)) {
        g_lastError = "fcntl(O_NONBLOCK) failed";
        LOGE("%s", g_lastError.c_str());
        close(g_socket);
        g_socket = -1;
        return false;
    }

    int flag = 1;
    setsockopt(g_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        g_lastError = "inet_pton() failed for: " + std::string(ip);
        LOGE("%s", g_lastError.c_str());
        close(g_socket);
        g_socket = -1;
        return false;
    }

    int ret = ::connect(g_socket, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0 && errno != EINPROGRESS) {
        g_lastError = "connect() failed: " + std::string(strerror(errno));
        LOGE("%s", g_lastError.c_str());
        close(g_socket);
        g_socket = -1;
        return false;
    }

    // Non-blocking connect: wait up to 3s for the TCP handshake to complete.
    // Until the socket is writable, send() will fail with ENOTCONN.
    if (ret < 0 && errno == EINPROGRESS) {
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(g_socket, &wfds);
        struct timeval tv = {3, 0};
        ret = select(g_socket + 1, nullptr, &wfds, nullptr, &tv);
        if (ret <= 0) {
            g_lastError = "connect() timeout or error";
            LOGE("%s (errno=%d)", g_lastError.c_str(), errno);
            close(g_socket);
            g_socket = -1;
            return false;
        }
        // Verify the socket actually connected (check SO_ERROR)
        int sockErr = 0;
        socklen_t sockErrLen = sizeof(sockErr);
        if (getsockopt(g_socket, SOL_SOCKET, SO_ERROR, &sockErr, &sockErrLen) < 0 || sockErr != 0) {
            g_lastError = "connect() async failed: " + std::string(strerror(sockErr));
            LOGE("%s", g_lastError.c_str());
            close(g_socket);
            g_socket = -1;
            return false;
        }
    }

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(g_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(g_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    g_connected = true;
    g_port = port;
    g_state = ProtocolState::REQUEST_JOIN_SERVER;
    g_recvBufLen = 0;

    ::g_stubSendSocket = g_socket;

    {
        std::lock_guard<std::mutex> lock(g_queueMutex);
        while (!g_packetQueue.empty()) g_packetQueue.pop();
    }

    LOGI("Connected to %s:%u (fd=%d)", ip, port, g_socket);
    return true;
}

void disconnect() {
    if (g_socket >= 0) {
        shutdown(g_socket, SHUT_RDWR);
        close(g_socket);
        g_socket = -1;
    }
    g_connected = false;
    g_port = 0;
    g_state = ProtocolState::DISCONNECTED;
    g_recvBufLen = 0;

    {
        std::lock_guard<std::mutex> lock(g_queueMutex);
        while (!g_packetQueue.empty()) g_packetQueue.pop();
    }

    LOGI("Disconnected");
}

bool isConnected() {
    return g_connected;
}

ProtocolState getState() {
    return g_state;
}

void setState(ProtocolState state) {
    g_state = state;
}

const char* getLastError() {
    return g_lastError.c_str();
}

// ============================================================================
// Data send
// ============================================================================

bool send(const uint8_t* data, size_t size, bool encryptBody) {
    if (!g_connected || g_socket < 0) return false;

    std::vector<uint8_t> buf(data, data + size);

    if (isGameServerPort()) {
        // Hex dump BEFORE transforms
        char hexPre[128] = {0};
        size_t dumpLen = size < 20 ? size : 20;
        for (size_t i = 0; i < dumpLen; i++) {
            sprintf(hexPre + i * 3, "%02X ", buf[i]);
        }
        LOGI("Send pre-transform: %s(%zu bytes)", hexPre, size);

        // Apply encryption layers (XorData for C1/C2, SimpleModulus for C3/C4)
        if (encryptBody && g_simpleModulus.hasEncKey()) {
            // C3/C4 SimpleModulus path: add random padding, encrypt body, wrap in C3/C4
            // (mirrors SendPacket in wsclientinline.h:88-140)
            buf.push_back(static_cast<uint8_t>(rand() % 256));

            int iSkip = (buf[0] == 0xC1) ? 2 : 3;
            buf[iSkip - 1] = g_packetSerialSend++;
            --iSkip;

            // XorData is NOT applied here — the caller (ProtocolDispatch) already
            // applied it via StreamPacketEngine-style forward chained XOR.
            // Double-scrambling would corrupt data after server's ExtractPacket
            // undoes only one layer.
            int bodyLen = static_cast<int>(buf.size() - iSkip - 1);
            int encSize = g_simpleModulus.encrypt(nullptr, buf.data() + iSkip, bodyLen);

            std::vector<uint8_t> wrapped;
            if (encSize < 256) {
                // C3 short format: {0xC3, size, encrypted_body}
                wrapped.resize(encSize + 2);
                wrapped[0] = 0xC3;
                wrapped[1] = static_cast<uint8_t>(encSize + 2);
                g_simpleModulus.encrypt(wrapped.data() + 2, buf.data() + iSkip, bodyLen);
            } else {
                // C4 long format: {0xC4, sizeH, sizeL, encrypted_body}
                uint16_t wireLen = static_cast<uint16_t>(encSize + 3);
                wrapped.resize(wireLen);
                wrapped[0] = 0xC4;
                wrapped[1] = static_cast<uint8_t>((wireLen >> 8) & 0xFF);
                wrapped[2] = static_cast<uint8_t>(wireLen & 0xFF);
                g_simpleModulus.encrypt(wrapped.data() + 3, buf.data() + iSkip, bodyLen);
            }
            buf.swap(wrapped);
            LOGI("SimpleModulus encrypted: C1(%zu) → C%c(%zu)", size,
                 buf[0] == 0xC3 ? '3' : '4', buf.size());
            // Apply XOR+SUB after SimpleModulus wrapping — server DecryptData runs on ALL
            // incoming data before packet type detection (SocketManager.cpp:590)
            encryptData(buf.data(), buf.size());
        } else {
            // Plain C1/C2 path: xorDataScramble + encryptData (XOR+SUB)
            xorDataScramble(buf.data(), buf.size());
            encryptData(buf.data(), buf.size());
        }

        // Hex dump AFTER transforms
        char hexPost[128] = {0};
        dumpLen = buf.size() < 20 ? buf.size() : 20;
        for (size_t i = 0; i < dumpLen; i++) {
            sprintf(hexPost + i * 3, "%02X ", buf[i]);
        }
        LOGI("Send post-transform: %s", hexPost);
    }

    size_t totalSent = 0;
    while (totalSent < buf.size()) {
        ssize_t n = ::send(g_socket, buf.data() + totalSent, buf.size() - totalSent, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            LOGE("send() failed: %s", strerror(errno));
            disconnect();
            return false;
        }
        totalSent += n;
    }

    return totalSent == buf.size();
}

// ============================================================================
// SimpleModulus instance (C3/C4 packet cipher)
// ============================================================================
bool initSimpleModulusDecKey(const uint8_t* buffer, size_t bufferSize) {
    return g_simpleModulus.loadDecryptionKey(buffer, bufferSize);
}

bool initSimpleModulusEncKey(const uint8_t* buffer, size_t bufferSize) {
    return g_simpleModulus.loadEncryptionKey(buffer, bufferSize);
}

bool isSimpleModulusReady() {
    return g_simpleModulus.hasDecKey();
}

void runSimpleModulusSelfTest() {
    // Log loaded key parameters
    g_simpleModulus.logKeyParams();

    // Encrypt the first 8 bytes of the login XorData'd body and log intermediate values
    // Original body bytes (positions 1-8 of encrypted region, AFTER XorData):
    // These come from the pre-transform log: C1 5F F1 0E 7A 36 86 4C F5 AB 01 CD AB CC DC 1D 1E 3E 2B 1E
    // After XorData: position 1=size(XorData'd=0x5F→0x5F doesn't change since XorData starts at pos 3)
    // Actually the encrypted region starts at position 1 (the size/serial byte)
    // Position 1 after serial replacement = packetSerial, so the test block should be
    // the bytes that get encrypted.
    uint8_t testBlock[8] = {0x0E, 0x7A, 0x36, 0x86, 0x4C, 0xF5, 0xAB, 0x01};
    uint8_t encrypted[11];
    g_simpleModulus.encryptBlockForDebug(encrypted, testBlock, 8);
}

// ============================================================================
// parsePackets — decode and enqueue complete packets from the recv buffer.
//
// Encryption layers on the wire:
//   ConnectServer (port 44404):  plain C1/C2 only
//   GameServer   (ports 55901+): ALL packets (C1/C2/C3/C4) are XOR+SUB encrypted.
//                                Server applies EncryptData to everything before send.
// ============================================================================

static void parsePackets() {
    size_t offset = 0;

    while (offset + 3 <= g_recvBufLen) {
        uint8_t* ptr = g_recvBuf + offset;
        uint16_t packetSize = 0;

        if (isGameServerPort()) {
            // Server EncryptData runs on ALL outgoing data. Decrypt header first
            // to reveal the real packet type.
            size_t hdrLen = (offset + 4 <= g_recvBufLen) ? 4 : 3;
            uint8_t decHdr[4];
            memcpy(decHdr, ptr, hdrLen);
            decryptData(decHdr, hdrLen);

            if (decHdr[0] == 0xC3) {
                // C3 short packet: XOR+SUB decrypt ALL bytes, then SimpleModulus decrypt body
                packetSize = decHdr[1];
                if (packetSize < 3 || packetSize > MAX_PACKET_SIZE) {
                    LOGE("Invalid decrypted C3 size: %u", packetSize);
                    g_recvBufLen = 0; return;
                }
                if (offset + packetSize > g_recvBufLen) break;

                decryptData(ptr, packetSize); // undo server's EncryptData

                if (!g_simpleModulus.hasDecKey()) {
                    LOGE("C3 packet but no SimpleModulus decryption key");
                    g_recvBufLen = 0; return;
                }
                uint8_t decrypted[2048];
                int decSize = g_simpleModulus.decrypt(decrypted, ptr + 2, packetSize - 2);
                if (decSize < 0) {
                    LOGE("C3 SimpleModulus decrypt failed");
                    g_recvBufLen = 0; return;
                }

                // First byte of decrypted data is server send serial — skip it
                uint8_t rebuilt[2048 + 3];
                rebuilt[0] = 0xC1;
                rebuilt[1] = static_cast<uint8_t>(decSize + 1);
                memcpy(rebuilt + 2, decrypted + 1, decSize - 1);

                Packet pkt;
                memcpy(pkt.data, rebuilt, decSize + 1);
                pkt.size = decSize + 1;
                pkt.encrypted = true;
                { std::lock_guard<std::mutex> lock(g_queueMutex); g_packetQueue.push(pkt); }
                offset += packetSize;
            }
            else if (decHdr[0] == 0xC4) {
                // C4 long packet: XOR+SUB decrypt ALL bytes, then SimpleModulus decrypt body
                if (hdrLen < 4) break;
                packetSize = (static_cast<uint16_t>(decHdr[1]) << 8) | decHdr[2];
                if (packetSize < 4 || packetSize > MAX_PACKET_SIZE) {
                    LOGE("Invalid decrypted C4 size: %u", packetSize);
                    g_recvBufLen = 0; return;
                }
                if (offset + packetSize > g_recvBufLen) break;

                decryptData(ptr, packetSize); // undo server's EncryptData

                if (!g_simpleModulus.hasDecKey()) {
                    LOGE("C4 packet but no SimpleModulus decryption key");
                    g_recvBufLen = 0; return;
                }
                uint8_t decrypted[4096];
                int decSize = g_simpleModulus.decrypt(decrypted, ptr + 3, packetSize - 3);
                if (decSize < 0) {
                    LOGE("C4 SimpleModulus decrypt failed");
                    g_recvBufLen = 0; return;
                }

                // First byte of decrypted data is server send serial — skip it
                int bodyLen = decSize - 1;
                uint8_t rebuilt[4096 + 4];
                rebuilt[0] = 0xC2;
                rebuilt[1] = static_cast<uint8_t>(((bodyLen + 3) >> 8) & 0xFF);
                rebuilt[2] = static_cast<uint8_t>((bodyLen + 3) & 0xFF);
                memcpy(rebuilt + 3, decrypted + 1, bodyLen);

                Packet pkt;
                memcpy(pkt.data, rebuilt, bodyLen + 3);
                pkt.size = bodyLen + 3;
                pkt.encrypted = true;
                { std::lock_guard<std::mutex> lock(g_queueMutex); g_packetQueue.push(pkt); }
                offset += packetSize;
            }
            else if (decHdr[0] == 0xC1) {
                packetSize = decHdr[1];
                size_t avail = g_recvBufLen - offset;
                if (packetSize < 3 || packetSize > MAX_PACKET_SIZE) {
                    LOGE("Invalid decrypted C1 size: %u", packetSize);
                    g_recvBufLen = 0; return;
                }
                if (packetSize < 5 && avail >= 12 && decHdr[2] == 0xF1 && decHdr[3] == 0x00) {
                    packetSize = static_cast<uint16_t>(avail);
                }
                if (offset + packetSize > g_recvBufLen) break;

                decryptData(ptr, packetSize);

                Packet pkt;
                memcpy(pkt.data, ptr, packetSize);
                pkt.size = packetSize;
                pkt.encrypted = false;
                { std::lock_guard<std::mutex> lock(g_queueMutex); g_packetQueue.push(pkt); }
                offset += packetSize;
            }
            else if (decHdr[0] == 0xC2) {
                if (hdrLen < 4) break;
                packetSize = (static_cast<uint16_t>(decHdr[1]) << 8) | decHdr[2];
                if (packetSize < 4 || packetSize > MAX_PACKET_SIZE) {
                    LOGE("Invalid decrypted C2 size: %u", packetSize);
                    g_recvBufLen = 0; return;
                }
                if (offset + packetSize > g_recvBufLen) break;

                decryptData(ptr, packetSize);

                Packet pkt;
                memcpy(pkt.data, ptr, packetSize);
                pkt.size = packetSize;
                pkt.encrypted = false;
                { std::lock_guard<std::mutex> lock(g_queueMutex); g_packetQueue.push(pkt); }
                offset += packetSize;
            }
            else {
                size_t dumpLen = (offset + 16 <= g_recvBufLen) ? 16 : (g_recvBufLen - offset);
                char hexDump[64] = {0};
                for (size_t i = 0; i < dumpLen; i++) {
                    sprintf(hexDump + i * 3, "%02X ", ptr[i]);
                }
                LOGE("Encrypted unknown: raw=0x%02X dec=0x%02X at offset %zu, raw bytes: %s",
                     ptr[0], decHdr[0], offset, hexDump);
                g_recvBufLen = 0;
                return;
            }
        }
        // --- ConnectServer: plain C1 ---
        else if (ptr[0] == 0xC1) {
            packetSize = ptr[1];
            if (packetSize < 3 || packetSize > MAX_PACKET_SIZE) {
                LOGE("Invalid C1 packet size: %u", packetSize);
                g_recvBufLen = 0;
                return;
            }
            if (offset + packetSize > g_recvBufLen) break;

            Packet pkt;
            memcpy(pkt.data, ptr, packetSize);
            pkt.size = packetSize;
            pkt.encrypted = false;
            {
                std::lock_guard<std::mutex> lock(g_queueMutex);
                g_packetQueue.push(pkt);
            }
            offset += packetSize;
        }
        // --- ConnectServer: plain C2 ---
        else if (ptr[0] == 0xC2) {
            packetSize = (static_cast<uint16_t>(ptr[1]) << 8) | ptr[2];
            if (packetSize < 4 || packetSize > MAX_PACKET_SIZE) {
                LOGE("Invalid C2 packet size: %u", packetSize);
                g_recvBufLen = 0;
                return;
            }
            if (offset + packetSize > g_recvBufLen) break;

            Packet pkt;
            memcpy(pkt.data, ptr, packetSize);
            pkt.size = packetSize;
            pkt.encrypted = false;
            {
                std::lock_guard<std::mutex> lock(g_queueMutex);
                g_packetQueue.push(pkt);
            }
            offset += packetSize;
        }
        else {
            LOGE("Unknown packet type: 0x%02X at offset %zu", ptr[0], offset);
            g_recvBufLen = 0;
            return;
        }
    }

    // Compact remaining data to front of buffer
    if (offset > 0 && offset < g_recvBufLen) {
        memmove(g_recvBuf, g_recvBuf + offset, g_recvBufLen - offset);
        g_recvBufLen -= offset;
    } else if (offset >= g_recvBufLen) {
        g_recvBufLen = 0;
    }
}

void update() {
    if (!g_connected || g_socket < 0) return;

    if (g_recvBufLen >= RECV_BUF_SIZE - 1024) {
        LOGE("Receive buffer overflow, clearing");
        g_recvBufLen = 0;
        return;
    }

    ssize_t n = recv(g_socket, g_recvBuf + g_recvBufLen,
                     RECV_BUF_SIZE - g_recvBufLen, 0);

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        LOGE("recv() error: %s", strerror(errno));
        disconnect();
        return;
    }

    if (n == 0) {
        LOGI("Server closed connection");
        disconnect();
        return;
    }

    // Raw bytes added to buffer. parsePackets() handles decryption per-packet:
    // C3/C4 → SimpleModulus only (no XOR+SUB)
    // C1/C2 on GS ports → XOR+SUB decrypt then queue
    // C1/C2 on CS ports → plain, queue directly
    g_recvBufLen += static_cast<size_t>(n);

    // Hex dump raw received data
    if (isGameServerPort()) {
        size_t dumpLen = g_recvBufLen < 32 ? g_recvBufLen : 32;
        char hex[128] = {0};
        for (size_t i = 0; i < dumpLen; i++) {
            sprintf(hex + i * 3, "%02X ", g_recvBuf[i]);
        }
        LOGI("Recv raw %zd bytes: %s", n, hex);
    }

    parsePackets();
}

bool getNextPacket(Packet& outPacket) {
    std::lock_guard<std::mutex> lock(g_queueMutex);
    if (g_packetQueue.empty()) return false;
    outPacket = g_packetQueue.front();
    g_packetQueue.pop();
    return true;
}

} // namespace MuNetwork
