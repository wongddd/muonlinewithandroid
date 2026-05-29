#include "MuFile.h"

#include <android/log.h>
#include <android/asset_manager.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <errno.h>

#define LOG_TAG "MuFile"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace MuFile {

// ============================================================================
// 文件句柄类型
// ============================================================================

enum class HandleType {
    NONE,
    ASSET,   // AAsset* (只读)
    DISK     // FILE* (读写)
};

struct FileHandle {
    HandleType type = HandleType::NONE;
    AAsset*    asset = nullptr;
    FILE*      file  = nullptr;
    long       fsize = -1; // 缓存文件大小

    bool valid() const { return type != HandleType::NONE && (asset || file); }
};

// ============================================================================
// 全局状态
// ============================================================================

static AAssetManager* g_assetManager = nullptr;
static std::string    g_internalPath;

// ============================================================================
// 初始化
// ============================================================================

void init(void* assetManager, const char* internalPath) {
    g_assetManager = static_cast<AAssetManager*>(assetManager);
    if (internalPath) {
        g_internalPath = internalPath;
        // 确保路径以 / 结尾
        if (!g_internalPath.empty() && g_internalPath.back() != '/') {
            g_internalPath += '/';
        }
    }
    LOGI("MuFile initialized: internalPath=%s, assetManager=%s",
         internalPath ? internalPath : "(null)",
         assetManager ? "yes" : "no");
}

// ============================================================================
// 路径路由
// ============================================================================

// 判断是否为潜在 asset 路径 (读模式 + 相对路径)
static bool isAssetPath(const char* path, const char* mode) {
    if (!path) return false;

    // 只读模式才考虑 asset
    if (mode && strchr(mode, 'w')) return false;
    if (mode && strchr(mode, 'a')) return false;

    // 绝对路径 → 磁盘
    if (path[0] == '/') return false;

    // 相对路径 → 先试磁盘，再试 asset
    if (strncmp(path, "Data/", 5) == 0) return true;
    if (strncmp(path, "Data\\", 5) == 0) return true;

    return true;
}

// 构建磁盘完整路径
static std::string buildDiskPath(const char* path) {
    if (!path) return "";

    // 绝对路径 → 直接使用
    if (path[0] == '/') return path;

    // 相对路径 → 基于 internalPath
    std::string full = g_internalPath;
    // 标准化反斜杠
    const char* p = path;
    while (*p) {
        if (*p == '\\') full += '/';
        else full += *p;
        ++p;
    }
    return full;
}

// 标准化 asset 路径 (反斜杠 → 正斜杠)
static std::string normalizeAssetPath(const char* path) {
    std::string result;
    while (*path) {
        if (*path == '\\') result += '/';
        else result += *path;
        ++path;
    }
    return result;
}

// ============================================================================
// 公开接口
// ============================================================================

void* open(const char* path, const char* mode) {
    if (!path) return nullptr;

    auto* handle = new FileHandle();
    const char* fopenMode = mode ? mode : "rb";

    // --- Try disk first for ALL paths ---
    // This allows /sdcard/Data/... to override APK-bundled assets at runtime,
    // which is critical for large game assets that don't fit in the APK on device.
    std::string diskPath = buildDiskPath(path);
    handle->file = (fopen)(diskPath.c_str(), fopenMode);

    if (handle->file) {
        handle->type = HandleType::DISK;
        fseek(handle->file, 0, SEEK_END);
        handle->fsize = ftell(handle->file);
        fseek(handle->file, 0, SEEK_SET);
        return handle;
    }

    // --- Not on disk, try APK assets ---
    if (isAssetPath(path, mode) && g_assetManager) {
        std::string assetPath = normalizeAssetPath(path);
        handle->type  = HandleType::ASSET;
        handle->asset = AAssetManager_open(g_assetManager, assetPath.c_str(), AASSET_MODE_BUFFER);

        if (!handle->asset) {
            LOGE("open(asset) failed: %s", assetPath.c_str());
            delete handle;
            return nullptr;
        }

        handle->fsize = static_cast<long>(AAsset_getLength(handle->asset));
        return handle;
    }

    LOGE("open failed: %s (disk=%s)", path, diskPath.c_str());
    delete handle;
    return nullptr;
}

size_t read(void* handlePtr, void* buffer, size_t size) {
    if (!handlePtr || !buffer) return 0;
    auto* handle = static_cast<FileHandle*>(handlePtr);

    switch (handle->type) {
        case HandleType::ASSET:
            return AAsset_read(handle->asset, buffer, size);
        case HandleType::DISK:
            return fread(buffer, 1, size, handle->file);
        default:
            return 0;
    }
}

long size(void* handlePtr) {
    if (!handlePtr) return -1;
    auto* handle = static_cast<FileHandle*>(handlePtr);
    return handle->fsize;
}

int seek(void* handlePtr, long offset, int origin) {
    if (!handlePtr) return -1;
    auto* handle = static_cast<FileHandle*>(handlePtr);

    switch (handle->type) {
        case HandleType::ASSET:
            return AAsset_seek(handle->asset, offset, origin);
        case HandleType::DISK:
            return fseek(handle->file, offset, origin);
        default:
            return -1;
    }
}

long tell(void* handlePtr) {
    if (!handlePtr) return -1;
    auto* handle = static_cast<FileHandle*>(handlePtr);

    switch (handle->type) {
        case HandleType::ASSET:
            return static_cast<long>(AAsset_getRemainingLength(handle->asset))
                   - (handle->fsize - AAsset_seek(handle->asset, 0, SEEK_CUR));
        case HandleType::DISK:
            return ftell(handle->file);
        default:
            return -1;
    }
}

void close(void* handlePtr) {
    if (!handlePtr) return;
    auto* handle = static_cast<FileHandle*>(handlePtr);

    switch (handle->type) {
        case HandleType::ASSET:
            if (handle->asset) AAsset_close(handle->asset);
            break;
        case HandleType::DISK:
            if (handle->file) fclose(handle->file);
            break;
        default:
            break;
    }

    delete handle;
}

bool exists(const char* path) {
    if (!path) return false;

    // Check disk first
    std::string diskPath = buildDiskPath(path);
    struct stat st;
    if (stat(diskPath.c_str(), &st) == 0) return true;

    // Fall back to APK assets
    if (isAssetPath(path, "rb") && g_assetManager) {
        std::string assetPath = normalizeAssetPath(path);
        AAsset* asset = AAssetManager_open(g_assetManager, assetPath.c_str(), AASSET_MODE_BUFFER);
        if (asset) {
            AAsset_close(asset);
            return true;
        }
    }
    return false;
}

uint8_t* readAll(const char* path, size_t& outSize) {
    outSize = 0;
    void* f = open(path, "rb");
    if (!f) return nullptr;

    long sz = size(f);
    if (sz <= 0) {
        close(f);
        return nullptr;
    }

    auto* buffer = new uint8_t[sz];
    size_t bytesRead = read(f, buffer, static_cast<size_t>(sz));
    close(f);

    if (bytesRead != static_cast<size_t>(sz)) {
        LOGE("readAll: short read (%zu of %ld bytes) for %s", bytesRead, sz, path);
        delete[] buffer;
        outSize = 0;
        return nullptr;
    }

    outSize = static_cast<size_t>(sz);
    return buffer;
}

void freeBuffer(uint8_t* buffer) {
    delete[] buffer;
}

size_t write(void* handlePtr, const void* buffer, size_t size) {
    if (!handlePtr || !buffer) return 0;
    auto* handle = static_cast<FileHandle*>(handlePtr);

    if (handle->type != HandleType::DISK) {
        LOGE("write: handle is not a disk file");
        return 0;
    }

    return fwrite(buffer, 1, size, handle->file);
}

bool makeDir(const char* path) {
    if (!path) return false;
    std::string fullPath = buildDiskPath(path);
    return mkdir(fullPath.c_str(), 0755) == 0 || errno == EEXIST;
}

const char* getInternalPath() {
    return g_internalPath.c_str();
}

} // namespace MuFile
