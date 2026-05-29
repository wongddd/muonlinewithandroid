#include "MuAudio.h"

#include <android/log.h>
#include <android/asset_manager.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <cstring>
#include <vector>
#include <mutex>

#define LOG_TAG "MuAudio"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace MuAudio {

// ============================================================================
// 内部常量
// ============================================================================

static constexpr int MAX_SOUNDS      = 512;   // 最大音效 ID
static constexpr int MAX_PLAYERS     = 16;    // 并发播放器槽位
static constexpr int MAX_SOUND_NAME  = 128;   // 文件路径最大长度

// ============================================================================
// WAV 文件头结构
// ============================================================================

#pragma pack(push, 1)
struct WavRiffHeader {
    char     riff[4];       // "RIFF"
    uint32_t fileSize;      // 文件总大小 - 8
    char     wave[4];       // "WAVE"
};

struct WavChunkHeader {
    char     id[4];         // "fmt " or "data"
    uint32_t size;          // 块大小 (不含头)
};

struct WavFmtChunk {
    uint16_t audioFormat;   // 1 = PCM
    uint16_t numChannels;   // 1 = mono, 2 = stereo
    uint32_t sampleRate;    // 22050, 44100, etc.
    uint32_t byteRate;      // sampleRate * numChannels * bitsPerSample/8
    uint16_t blockAlign;    // numChannels * bitsPerSample/8
    uint16_t bitsPerSample; // 8, 16
};
#pragma pack(pop)

// ============================================================================
// 音效数据
// ============================================================================

struct SoundData {
    bool     loaded       = false;
    char     filename[MAX_SOUND_NAME] = {0};
    uint8_t* pcmData      = nullptr;
    uint32_t pcmSize      = 0;
    int      numChannels  = 1;
    int      sampleRate   = 22050;
    int      bitsPerSample = 16;
    int      volume       = 100;  // 0..100
};

// ============================================================================
// 播放器槽位
// ============================================================================

struct PlayerSlot {
    bool          inUse     = false;
    int           soundId   = -1;
    bool           loop      = false;

    // OpenSL ES 对象
    SLObjectItf   playerObj    = nullptr;
    SLPlayItf     playItf      = nullptr;
    SLVolumeItf   volumeItf    = nullptr;
    SLBufferQueueItf bufQueue  = nullptr;
};

// ============================================================================
// 全局状态
// ============================================================================

static std::mutex g_mutex;

static AAssetManager* g_assetManager = nullptr;

// OpenSL ES 引擎
static SLObjectItf  g_engineObj   = nullptr;
static SLEngineItf  g_engineItf   = nullptr;
static SLObjectItf  g_outputMix   = nullptr;

// 音效数据表
static SoundData g_sounds[MAX_SOUNDS];

// 播放器池
static PlayerSlot g_players[MAX_PLAYERS];

// 主音量
static int g_masterVolume = 100;
static bool g_paused = false;
static bool g_initialized = false;

// ============================================================================
// WAV 文件解析
// ============================================================================

static bool parseWav(const uint8_t* data, size_t dataSize,
                     uint8_t*& outPcm, uint32_t& outPcmSize,
                     int& outChannels, int& outSampleRate, int& outBits) {

    if (dataSize < sizeof(WavRiffHeader) + sizeof(WavChunkHeader) + sizeof(WavFmtChunk)) {
        LOGE("WAV file too small: %zu bytes", dataSize);
        return false;
    }

    size_t offset = 0;

    // --- RIFF header ---
    const auto* riff = reinterpret_cast<const WavRiffHeader*>(data + offset);
    if (memcmp(riff->riff, "RIFF", 4) != 0 || memcmp(riff->wave, "WAVE", 4) != 0) {
        LOGE("Invalid RIFF/WAVE header");
        return false;
    }
    offset += sizeof(WavRiffHeader);

    // --- Chunk loop ---
    bool hasFmt = false;
    bool hasData = false;

    while (offset + sizeof(WavChunkHeader) <= dataSize) {
        const auto* chunk = reinterpret_cast<const WavChunkHeader*>(data + offset);
        offset += sizeof(WavChunkHeader);

        uint32_t chunkSize = chunk->size;

        if (memcmp(chunk->id, "fmt ", 4) == 0) {
            if (offset + sizeof(WavFmtChunk) > dataSize) break;

            const auto* fmt = reinterpret_cast<const WavFmtChunk*>(data + offset);

            if (fmt->audioFormat != 1) {
                LOGE("Unsupported audio format: %u (only PCM=1 supported)", fmt->audioFormat);
                return false;
            }

            outChannels    = fmt->numChannels;
            outSampleRate  = fmt->sampleRate;
            outBits        = fmt->bitsPerSample;
            hasFmt         = true;

            offset += chunkSize;
        }
        else if (memcmp(chunk->id, "data", 4) == 0) {
            if (!hasFmt) {
                LOGE("WAV data chunk before fmt chunk");
                return false;
            }

            if (offset + chunkSize > dataSize) break;

            // 复制 PCM 数据
            outPcmSize = chunkSize;
            outPcm = new uint8_t[chunkSize];
            memcpy(outPcm, data + offset, chunkSize);
            hasData = true;

            break; // data 块是最后一个
        }
        else {
            // 跳过未知块 (如 "fact", "LIST", "INFO" 等)
            offset += chunkSize;
        }
    }

    if (!hasFmt || !hasData) {
        LOGE("WAV missing fmt or data chunk");
        return false;
    }

    return true;
}

// ============================================================================
// OpenSL ES 初始化 / 销毁
// ============================================================================

static int millibelFromPercent(int percent) {
    // 0..100 → millibel
    // 0% → -4000 (SL_MILLIBEL_MIN), 100% → 0 (max)
    if (percent <= 0) return SL_MILLIBEL_MIN;
    if (percent >= 100) return 0;

    // 对数映射: millibel = 2000 * log10(percent/100)
    // 简化线性映射: -4000 + (percent * 40)
    return -4000 + (percent * 40);
}

bool init(void* assetManager) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_initialized) {
        LOGI("MuAudio already initialized");
        return true;
    }

    g_assetManager = static_cast<AAssetManager*>(assetManager);

    // --- 创建 OpenSL ES 引擎 ---
    SLresult result;

    result = slCreateEngine(&g_engineObj, 0, nullptr, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("slCreateEngine failed: %d", result);
        return false;
    }

    result = (*g_engineObj)->Realize(g_engineObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Engine Realize failed: %d", result);
        return false;
    }

    result = (*g_engineObj)->GetInterface(g_engineObj, SL_IID_ENGINE, &g_engineItf);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("GetInterface ENGINE failed: %d", result);
        return false;
    }

    // --- 创建 Output Mix ---
    result = (*g_engineItf)->CreateOutputMix(g_engineItf, &g_outputMix, 0, nullptr, nullptr);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateOutputMix failed: %d", result);
        return false;
    }

    result = (*g_outputMix)->Realize(g_outputMix, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("OutputMix Realize failed: %d", result);
        return false;
    }

    // --- 初始化音效表 ---
    for (int i = 0; i < MAX_SOUNDS; ++i) {
        g_sounds[i] = SoundData{};
    }

    // --- 初始化播放器池 ---
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        g_players[i] = PlayerSlot{};
    }

    g_masterVolume = 100;
    g_paused = false;
    g_initialized = true;

    LOGI("MuAudio initialized (OpenSL ES)");
    return true;
}

void destroy() {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (!g_initialized) return;

    stopAllSounds();

    // 释放播放器
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (g_players[i].playerObj) {
            (*g_players[i].playerObj)->Destroy(g_players[i].playerObj);
            g_players[i] = PlayerSlot{};
        }
    }

    // 释放音效 PCM 数据
    for (int i = 0; i < MAX_SOUNDS; ++i) {
        if (g_sounds[i].pcmData) {
            delete[] g_sounds[i].pcmData;
            g_sounds[i] = SoundData{};
        }
    }

    // 销毁 Output Mix
    if (g_outputMix) {
        (*g_outputMix)->Destroy(g_outputMix);
        g_outputMix = nullptr;
    }

    // 销毁引擎
    if (g_engineObj) {
        (*g_engineObj)->Destroy(g_engineObj);
        g_engineObj = nullptr;
        g_engineItf = nullptr;
    }

    g_assetManager = nullptr;
    g_initialized = false;
    LOGI("MuAudio destroyed");
}

// ============================================================================
// 音效加载
// ============================================================================

bool loadSound(int id, const char* filename) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (!g_initialized) {
        LOGE("loadSound: MuAudio not initialized");
        return false;
    }

    if (id < 0 || id >= MAX_SOUNDS) {
        LOGE("loadSound: invalid id %d", id);
        return false;
    }

    if (!g_assetManager) {
        LOGE("loadSound: no asset manager");
        return false;
    }

    // 如果已加载，先释放旧数据
    if (g_sounds[id].loaded) {
        delete[] g_sounds[id].pcmData;
        g_sounds[id] = SoundData{};
    }

    // 打开 asset 文件
    AAsset* asset = AAssetManager_open(g_assetManager, filename, AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("loadSound: failed to open %s", filename);
        return false;
    }

    size_t fileSize = AAsset_getLength(asset);
    const uint8_t* fileData = static_cast<const uint8_t*>(AAsset_getBuffer(asset));

    if (!fileData || fileSize == 0) {
        LOGE("loadSound: empty or null asset %s", filename);
        AAsset_close(asset);
        return false;
    }

    // 解析 WAV
    uint8_t* pcm = nullptr;
    uint32_t pcmSize = 0;
    int channels = 0, sampleRate = 0, bits = 0;

    // 需要拷贝数据因为 parseWav 需要非 const 访问 (解析后复制 PCM)
    std::vector<uint8_t> fileCopy(fileData, fileData + fileSize);
    AAsset_close(asset);

    if (!parseWav(fileCopy.data(), fileCopy.size(), pcm, pcmSize, channels, sampleRate, bits)) {
        LOGE("loadSound: failed to parse WAV %s", filename);
        return false;
    }

    g_sounds[id].loaded        = true;
    g_sounds[id].pcmData       = pcm;
    g_sounds[id].pcmSize       = pcmSize;
    g_sounds[id].numChannels   = channels;
    g_sounds[id].sampleRate    = sampleRate;
    g_sounds[id].bitsPerSample = bits;
    g_sounds[id].volume        = 100;
    strncpy(g_sounds[id].filename, filename, MAX_SOUND_NAME - 1);

    LOGI("Loaded sound %d: %s (%dHz %dch %dbit, %u bytes PCM)",
         id, filename, sampleRate, channels, bits, pcmSize);
    return true;
}

// ============================================================================
// 播放器管理
// ============================================================================

// 播放器回调：播放结束时触发 (用于循环或回收)
static void playerCallback(SLBufferQueueItf caller, void* context) {
    // context = player slot index
    // Android Simple Buffer Queue: 播放完所有已入队缓冲区后调用
    // 对于一次性音效，播放器需要被回收
    // 对于循环音效，需要重新入队
    int slotIndex = static_cast<int>(reinterpret_cast<intptr_t>(context));

    PlayerSlot& slot = g_players[slotIndex];
    if (slot.loop && slot.inUse) {
        // 重新入队数据实现循环
        SoundData& sound = g_sounds[slot.soundId];
        if (sound.loaded && sound.pcmData) {
            (*caller)->Enqueue(caller, sound.pcmData, sound.pcmSize);
        }
    }
    // 非循环: 播放完成后槽位由外部回收 (stopSound 或下次 playSound)
}

static int findFreePlayer() {
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (!g_players[i].inUse) return i;
    }
    return -1; // 无空闲槽位
}

static void destroyPlayer(int slotIndex) {
    PlayerSlot& slot = g_players[slotIndex];
    if (slot.playerObj) {
        // 先停止播放
        if (slot.playItf) {
            (*slot.playItf)->SetPlayState(slot.playItf, SL_PLAYSTATE_STOPPED);
        }
        (*slot.playerObj)->Destroy(slot.playerObj);
    }
    slot = PlayerSlot{};
}

int playSound(int id, bool loop) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (!g_initialized || g_paused) return -1;
    if (id < 0 || id >= MAX_SOUNDS) return -1;
    if (!g_sounds[id].loaded) {
        LOGE("playSound: sound %d not loaded", id);
        return -1;
    }

    SoundData& sound = g_sounds[id];
    if (!sound.pcmData || sound.pcmSize == 0) return -1;

    // 找空闲槽位
    int slotIndex = findFreePlayer();
    if (slotIndex < 0) {
        LOGE("playSound: no free player slots");
        return -1;
    }

    // 创建播放器
    PlayerSlot& slot = g_players[slotIndex];

    // 配置数据源: Android Simple Buffer Queue
    SLDataLocator_AndroidSimpleBufferQueue locBufQueue = {};
    locBufQueue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    locBufQueue.numBuffers = 2; // 双缓冲 (1 播放 + 1 入队)

    // 配置 PCM 格式
    SLDataFormat_PCM formatPcm = {};
    formatPcm.formatType    = SL_DATAFORMAT_PCM;
    formatPcm.numChannels   = static_cast<SLuint32>(sound.numChannels);
    formatPcm.samplesPerSec = static_cast<SLuint32>(sound.sampleRate) * 1000; // mHz
    formatPcm.bitsPerSample = static_cast<SLuint32>(sound.bitsPerSample);
    formatPcm.containerSize = static_cast<SLuint32>(sound.bitsPerSample);
    formatPcm.channelMask   = sound.numChannels == 2
                              ? (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT)
                              : SL_SPEAKER_FRONT_CENTER;
    formatPcm.endianness    = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSrc = {};
    audioSrc.pLocator  = &locBufQueue;
    audioSrc.pFormat   = &formatPcm;

    // 配置输出: Output Mix
    SLDataLocator_OutputMix locOutMix = {};
    locOutMix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    locOutMix.outputMix   = g_outputMix;

    SLDataSink audioSnk = {};
    audioSnk.pLocator = &locOutMix;
    audioSnk.pFormat  = nullptr;

    // 需要的接口
    SLInterfaceID ids[] = {
        SL_IID_PLAY,
        SL_IID_VOLUME,
        SL_IID_BUFFERQUEUE
    };
    SLboolean req[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

    SLresult result = (*g_engineItf)->CreateAudioPlayer(
        g_engineItf, &slot.playerObj,
        &audioSrc, &audioSnk,
        sizeof(ids) / sizeof(ids[0]), ids, req);

    if (result != SL_RESULT_SUCCESS) {
        LOGE("CreateAudioPlayer failed: %d", result);
        return -1;
    }

    result = (*slot.playerObj)->Realize(slot.playerObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
        LOGE("Player Realize failed: %d", result);
        destroyPlayer(slotIndex);
        return -1;
    }

    // 获取接口
    (*slot.playerObj)->GetInterface(slot.playerObj, SL_IID_PLAY, &slot.playItf);
    (*slot.playerObj)->GetInterface(slot.playerObj, SL_IID_VOLUME, &slot.volumeItf);
    (*slot.playerObj)->GetInterface(slot.playerObj, SL_IID_BUFFERQUEUE, &slot.bufQueue);

    // 注册回调
    (*slot.bufQueue)->RegisterCallback(slot.bufQueue, playerCallback,
                                       reinterpret_cast<void*>(static_cast<intptr_t>(slotIndex)));

    // 设置音量
    int effectiveVolume = (sound.volume * g_masterVolume) / 100;
    if (slot.volumeItf) {
        (*slot.volumeItf)->SetVolumeLevel(slot.volumeItf, millibelFromPercent(effectiveVolume));
    }

    // 入队 PCM 数据
    (*slot.bufQueue)->Enqueue(slot.bufQueue, sound.pcmData, sound.pcmSize);

    // 开始播放
    (*slot.playItf)->SetPlayState(slot.playItf, SL_PLAYSTATE_PLAYING);

    slot.inUse   = true;
    slot.soundId = id;
    slot.loop     = loop;

    return slotIndex;
}

// ============================================================================
// 停止和音量控制
// ============================================================================

void stopSound(int id) {
    std::lock_guard<std::mutex> lock(g_mutex);

    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (g_players[i].inUse && g_players[i].soundId == id) {
            destroyPlayer(i);
        }
    }
}

void stopAllSounds() {
    std::lock_guard<std::mutex> lock(g_mutex);

    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (g_players[i].inUse) {
            destroyPlayer(i);
        }
    }
}

void setVolume(int id, int volume) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (id < 0 || id >= MAX_SOUNDS) return;
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;

    g_sounds[id].volume = volume;

    // 更新正在播放该音效的播放器音量
    int effectiveVolume = (volume * g_masterVolume) / 100;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (g_players[i].inUse && g_players[i].soundId == id && g_players[i].volumeItf) {
            (*g_players[i].volumeItf)->SetVolumeLevel(
                g_players[i].volumeItf, millibelFromPercent(effectiveVolume));
        }
    }
}

void setMasterVolume(int volume) {
    std::lock_guard<std::mutex> lock(g_mutex);

    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;

    g_masterVolume = volume;

    // 更新所有播放器音量
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (g_players[i].inUse && g_players[i].volumeItf) {
            int soundId = g_players[i].soundId;
            int effectiveVolume = (g_sounds[soundId].volume * g_masterVolume) / 100;
            (*g_players[i].volumeItf)->SetVolumeLevel(
                g_players[i].volumeItf, millibelFromPercent(effectiveVolume));
        }
    }
}

// ============================================================================
// 暂停 / 恢复 (应用生命周期)
// ============================================================================

void pauseAll() {
    std::lock_guard<std::mutex> lock(g_mutex);

    g_paused = true;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (g_players[i].inUse && g_players[i].playItf) {
            (*g_players[i].playItf)->SetPlayState(g_players[i].playItf, SL_PLAYSTATE_PAUSED);
        }
    }
}

void resumeAll() {
    std::lock_guard<std::mutex> lock(g_mutex);

    g_paused = false;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (g_players[i].inUse && g_players[i].playItf) {
            (*g_players[i].playItf)->SetPlayState(g_players[i].playItf, SL_PLAYSTATE_PLAYING);
        }
    }
}

void onPause()   { pauseAll(); }
void onResume()  { resumeAll(); }

// ============================================================================
// 辅助
// ============================================================================

bool isSoundLoaded(int id) {
    if (id < 0 || id >= MAX_SOUNDS) return false;
    return g_sounds[id].loaded;
}

} // namespace MuAudio
