#include "MuFile.h"

#include <android/log.h>
#include <android/asset_manager.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
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

static const char* g_externalRoots[] = {
    "/sdcard/",
    "/storage/emulated/0/",
    "/sdcard/MU/",
    "/storage/emulated/0/MU/",
    nullptr
};

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

static bool isReadOnlyMode(const char* mode) {
    return !(mode && (strchr(mode, 'w') || strchr(mode, 'a') || strchr(mode, '+')));
}

// 标准化磁盘/asset 路径 (反斜杠 → 正斜杠，去掉 ./ 前缀)
static std::string normalizePath(const char* path) {
    std::string result;
    if (!path) return result;

    while (*path) {
        if (*path == '\\') result += '/';
        else result += *path;
        ++path;
    }
    while (result.compare(0, 2, "./") == 0) {
        result.erase(0, 2);
    }
    return result;
}

static std::string joinPath(const std::string& root, const std::string& relative) {
    if (root.empty()) return relative;
    if (root.back() == '/') return root + relative;
    return root + "/" + relative;
}

static bool fileExistsOnDisk(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

static bool hasDataPrefix(const std::string& path) {
    return path.compare(0, 5, "Data/") == 0 || path.compare(0, 5, "data/") == 0;
}

static std::string lowercaseAscii(const std::string& value) {
    std::string out = value;
    for (char& ch : out) {
        if (ch >= 'A' && ch <= 'Z') ch = static_cast<char>(ch - 'A' + 'a');
    }
    return out;
}

static std::string canonicalizeResourcePath(const std::string& path) {
    std::string base;
    std::string rest = path;
    if (hasDataPrefix(rest)) {
        base = "Data/";
        rest = rest.substr(5);
    }

    size_t slash = rest.find('/');
    std::string head = (slash == std::string::npos) ? rest : rest.substr(0, slash);
    std::string tail = (slash == std::string::npos) ? "" : rest.substr(slash);
    std::string lower = lowercaseAscii(head);
    std::string fixed = head;

    if (lower.compare(0, 5, "world") == 0) fixed = "World" + head.substr(5);
    else if (lower.compare(0, 6, "object") == 0) fixed = "Object" + head.substr(6);
    else if (lower == "custom") fixed = "Custom";
    else if (lower == "effect") fixed = "Effect";
    else if (lower == "interface") fixed = "Interface";
    else if (lower == "item") fixed = "Item";
    else if (lower == "launcher") fixed = "Launcher";
    else if (lower == "local") fixed = "Local";
    else if (lower == "logo") fixed = "Logo";
    else if (lower == "monster") fixed = "Monster";
    else if (lower == "music") fixed = "Music";
    else if (lower == "npc") fixed = "NPC";
    else if (lower == "player") fixed = "Player";
    else if (lower == "skill") fixed = "Skill";
    else if (lower == "sound") fixed = "Sound";

    return base + fixed + tail;
}

static void addUniqueCandidate(std::vector<std::string>& candidates, const std::string& path) {
    for (const std::string& existing : candidates) {
        if (existing == path) return;
    }
    candidates.push_back(path);
}

// 构建默认磁盘完整路径；写入仍限制在 internalPath 内。
static std::string buildDiskPath(const char* path) {
    if (!path) return "";

    // 绝对路径 → 直接使用
    if (path[0] == '/') return path;

    // 相对路径 → 基于 internalPath
    return joinPath(g_internalPath, normalizePath(path));
}

static std::vector<std::string> buildReadCandidates(const char* path) {
    std::vector<std::string> candidates;
    if (!path) return candidates;

    std::string norm = normalizePath(path);
    if (norm.empty()) return candidates;

    if (norm[0] == '/') {
        candidates.push_back(norm);
        return candidates;
    }

    std::string canonical = canonicalizeResourcePath(norm);
    for (int i = 0; g_externalRoots[i] != nullptr; ++i) {
        addUniqueCandidate(candidates, joinPath(g_externalRoots[i], norm));
        addUniqueCandidate(candidates, joinPath(g_externalRoots[i], canonical));
        if (!hasDataPrefix(norm)) {
            addUniqueCandidate(candidates, joinPath(g_externalRoots[i], "Data/" + norm));
            addUniqueCandidate(candidates, joinPath(g_externalRoots[i], "Data/" + canonical));
        }
    }
    addUniqueCandidate(candidates, joinPath(g_internalPath, norm));
    addUniqueCandidate(candidates, joinPath(g_internalPath, canonical));
    if (!hasDataPrefix(norm)) {
        addUniqueCandidate(candidates, joinPath(g_internalPath, "Data/" + norm));
        addUniqueCandidate(candidates, joinPath(g_internalPath, "Data/" + canonical));
    }
    addUniqueCandidate(candidates, norm);
    addUniqueCandidate(candidates, canonical);
    return candidates;
}

// 标准化 asset 路径
static std::string normalizeAssetPath(const char* path) {
    return normalizePath(path);
}

// ============================================================================
// 公开接口
// ============================================================================

void* open(const char* path, const char* mode) {
    if (!path) return nullptr;

    auto* handle = new FileHandle();
    const char* fopenMode = mode ? mode : "rb";

    if (isReadOnlyMode(fopenMode)) {
        // Public Data roots let the Android build consume the same resource tree
        // as the PC client without repackaging multi-GB assets into the APK.
        std::vector<std::string> candidates = buildReadCandidates(path);
        for (const std::string& candidate : candidates) {
            handle->file = (fopen)(candidate.c_str(), fopenMode);
            if (handle->file) {
                handle->type = HandleType::DISK;
                fseek(handle->file, 0, SEEK_END);
                handle->fsize = ftell(handle->file);
                fseek(handle->file, 0, SEEK_SET);
                return handle;
            }
        }
    } else {
        std::string diskPath = buildDiskPath(path);
        handle->file = (fopen)(diskPath.c_str(), fopenMode);
        if (handle->file) {
            handle->type = HandleType::DISK;
            fseek(handle->file, 0, SEEK_END);
            handle->fsize = ftell(handle->file);
            fseek(handle->file, 0, SEEK_SET);
            return handle;
        }
    }

    // --- Not on disk, try APK assets ---
    if (isAssetPath(path, mode) && g_assetManager) {
        std::string assetPath = normalizeAssetPath(path);
        handle->type  = HandleType::ASSET;
        handle->asset = AAssetManager_open(g_assetManager, assetPath.c_str(), AASSET_MODE_BUFFER);
        if (!handle->asset) {
            assetPath = canonicalizeResourcePath(assetPath);
            handle->asset = AAssetManager_open(g_assetManager, assetPath.c_str(), AASSET_MODE_BUFFER);
        }
        if (!handle->asset && !hasDataPrefix(assetPath)) {
            assetPath = "Data/" + assetPath;
            handle->asset = AAssetManager_open(g_assetManager, assetPath.c_str(), AASSET_MODE_BUFFER);
        }

        if (!handle->asset) {
            LOGE("open(asset) failed: %s", assetPath.c_str());
            delete handle;
            return nullptr;
        }

        handle->fsize = static_cast<long>(AAsset_getLength(handle->asset));
        return handle;
    }

    LOGE("open failed: %s", path);
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

    // Check public/full-resource roots and internal cache first.
    std::vector<std::string> candidates = buildReadCandidates(path);
    for (const std::string& candidate : candidates) {
        if (fileExistsOnDisk(candidate)) return true;
    }

    // Fall back to APK assets
    if (isAssetPath(path, "rb") && g_assetManager) {
        std::string assetPath = normalizeAssetPath(path);
        AAsset* asset = AAssetManager_open(g_assetManager, assetPath.c_str(), AASSET_MODE_BUFFER);
        if (!asset) {
            assetPath = canonicalizeResourcePath(assetPath);
            asset = AAssetManager_open(g_assetManager, assetPath.c_str(), AASSET_MODE_BUFFER);
        }
        if (!asset && !hasDataPrefix(assetPath)) {
            assetPath = "Data/" + assetPath;
            asset = AAssetManager_open(g_assetManager, assetPath.c_str(), AASSET_MODE_BUFFER);
        }
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
