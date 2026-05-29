#include "MuAssetExtractor.h"

#include <android/asset_manager.h>
#include <android/log.h>

#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <errno.h>

#define LOG_TAG "MuAssetExtractor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static AAssetManager* g_assetMgr = nullptr;
static std::string    g_basePath;

// ============================================================================
// Known data subdirectories under assets/Data/ — walked for bulk extraction.
// Must match the PC client's data directory layout.
// ============================================================================
static const char* g_dataDirs[] = {
    "Data/Custom",
    "Data/Effect",
    "Data/InGameShopBanner",
    "Data/InGameShopScript",
    "Data/Interface",
    "Data/Item",
    "Data/Launcher",
    "Data/Local",
    "Data/Local0",
    "Data/LocalS16",
    "Data/Logo",
    "Data/Monster",
    "Data/Music",
    "Data/NPC",
    "Data/Player",
    "Data/Skill",
    "Data/Sound",
    "Data/World1", "Data/World2", "Data/World3", "Data/World4", "Data/World5",
    "Data/World7", "Data/World8", "Data/World9",
    "Data/World10", "Data/World11", "Data/World12", "Data/World19",
    "Data/World25", "Data/World31", "Data/World32", "Data/World34", "Data/World35",
    "Data/World38", "Data/World39", "Data/World40", "Data/World41", "Data/World42",
    "Data/World43", "Data/World47", "Data/World52", "Data/World55", "Data/World56",
    "Data/World57", "Data/World58", "Data/World59", "Data/World63", "Data/World64",
    "Data/World65", "Data/World66", "Data/World67", "Data/World68", "Data/World69",
    "Data/World70", "Data/World71", "Data/World72", "Data/World73", "Data/World74",
    "Data/World74(SelectServerS6)",
    "Data/World75", "Data/World75(SelectCharS13)", "Data/World75(SelectCharS6)",
    "Data/World76", "Data/World77", "Data/World80", "Data/World81", "Data/World82",
    "Data/World83", "Data/World84", "Data/World85", "Data/World86", "Data/World87",
    "Data/World88", "Data/World89", "Data/World90", "Data/World91", "Data/World92",
    "Data/World93", "Data/World94", "Data/World95",
    "Data/World100",
    "Data/Object1", "Data/Object2", "Data/Object3", "Data/Object4", "Data/Object5",
    "Data/Object7", "Data/Object8", "Data/Object9",
    "Data/Object10", "Data/Object11", "Data/Object12", "Data/Object19",
    "Data/Object25", "Data/Object31", "Data/Object32", "Data/Object34", "Data/Object35",
    "Data/Object38", "Data/Object39", "Data/Object40", "Data/Object41", "Data/Object42",
    "Data/Object43", "Data/Object47", "Data/Object52", "Data/Object55", "Data/Object56",
    "Data/Object57", "Data/Object58", "Data/Object59", "Data/Object63", "Data/Object64",
    "Data/Object65", "Data/Object66", "Data/Object67", "Data/Object68", "Data/Object69",
    "Data/Object70", "Data/Object71", "Data/Object72", "Data/Object73", "Data/Object74",
    "Data/Object74(SelectServerS6)",
    "Data/Object75", "Data/Object75(SelectCharS13)", "Data/Object75(SelectCharS6)",
    "Data/Object76", "Data/Object77", "Data/Object80", "Data/Object81", "Data/Object82",
    "Data/Object83", "Data/Object84", "Data/Object85", "Data/Object86", "Data/Object87",
    "Data/Object88", "Data/Object89", "Data/Object90", "Data/Object91", "Data/Object92",
    "Data/Object93", "Data/Object94", "Data/Object95",
    nullptr  // sentinel
};

// ============================================================================
// Initialize
// ============================================================================

void mu_asset_extractor_init(void* assetManager, const char* internalPath) {
    g_assetMgr = static_cast<AAssetManager*>(assetManager);
    if (internalPath) {
        g_basePath = internalPath;
        if (!g_basePath.empty() && g_basePath.back() != '/') {
            g_basePath += '/';
        }
    }
    LOGI("Asset extractor initialized: basePath=%s, assetMgr=%s",
         g_basePath.c_str(), assetManager ? "yes" : "no");
}

// ============================================================================
// Helpers
// ============================================================================

static std::string normalizePath(const char* path) {
    std::string result;
    while (*path) {
        result += (*path == '\\') ? '/' : *path;
        ++path;
    }
    return result;
}

// Ensure all parent directories exist for a file path
static void ensureParentDir(const char* filePath) {
    std::string path(filePath);
    size_t lastSlash = path.rfind('/');
    if (lastSlash == std::string::npos) return;

    std::string dir = path.substr(0, lastSlash);
    size_t pos = 1;
    while (pos < dir.size()) {
        size_t nextSlash = dir.find('/', pos);
        if (nextSlash == std::string::npos) {
            mkdir(dir.c_str(), 0755);
            break;
        }
        std::string sub = dir.substr(0, nextSlash);
        mkdir(sub.c_str(), 0755);
        pos = nextSlash + 1;
    }
    mkdir(dir.c_str(), 0755);
}

// Extract a single file from APK assets to disk
static bool extractAsset(const char* assetPath, const char* diskPath) {
    if (!g_assetMgr) return false;

    AAsset* asset = AAssetManager_open(g_assetMgr, assetPath, AASSET_MODE_BUFFER);
    if (!asset) return false;

    const void* data = AAsset_getBuffer(asset);
    off_t length = AAsset_getLength(asset);

    if (!data || length <= 0) {
        AAsset_close(asset);
        return false;
    }

    ensureParentDir(diskPath);

    FILE* fp = (fopen)(diskPath, "wb");
    if (!fp) {
        LOGE("extractAsset: cannot write %s (errno=%d)", diskPath, errno);
        AAsset_close(asset);
        return false;
    }

    size_t written = fwrite(data, 1, static_cast<size_t>(length), fp);
    fclose(fp);
    AAsset_close(asset);

    if (written != static_cast<size_t>(length)) {
        LOGE("extractAsset: short write %zu/%d for %s", written, length, diskPath);
        return false;
    }

    return true;
}

// ============================================================================
// Bulk extraction — extract all game data from APK to disk on first launch
// ============================================================================

void mu_asset_extractor_extract_all() {
    if (!g_assetMgr || g_basePath.empty()) {
        LOGE("extract_all: not initialized");
        return;
    }

    // Sentinel: skip if already extracted
    std::string sentinelPath = g_basePath + "Data/.extracted";
    FILE* fp = (fopen)(sentinelPath.c_str(), "r");
    if (fp) {
        fclose(fp);
        LOGI("Bulk extraction already done (sentinel found)");
        return;
    }

    LOGI("Starting bulk extraction of all game data...");

    int totalFiles = 0;
    int64_t totalBytes = 0;

    // Extract root-level files in Data/
    AAssetDir* rootDir = AAssetManager_openDir(g_assetMgr, "Data");
    if (rootDir) {
        const char* fname;
        while ((fname = AAssetDir_getNextFileName(rootDir)) != nullptr) {
            std::string assetPath = std::string("Data/") + fname;
            std::string diskPath = g_basePath + assetPath;
            if (extractAsset(assetPath.c_str(), diskPath.c_str())) {
                totalFiles++;
                struct stat st;
                if (stat(diskPath.c_str(), &st) == 0) totalBytes += st.st_size;
            }
        }
        AAssetDir_close(rootDir);
    }

    // Extract files from each known subdirectory
    for (int i = 0; g_dataDirs[i] != nullptr; i++) {
        AAssetDir* dir = AAssetManager_openDir(g_assetMgr, g_dataDirs[i]);
        if (!dir) continue;

        const char* fname;
        while ((fname = AAssetDir_getNextFileName(dir)) != nullptr) {
            std::string assetPath = std::string(g_dataDirs[i]) + "/" + fname;
            std::string diskPath = g_basePath + assetPath;
            if (extractAsset(assetPath.c_str(), diskPath.c_str())) {
                totalFiles++;
                struct stat st;
                if (stat(diskPath.c_str(), &st) == 0) totalBytes += st.st_size;
            }
        }
        AAssetDir_close(dir);
    }

    // Write sentinel
    FILE* sfp = (fopen)(sentinelPath.c_str(), "w");
    if (sfp) {
        fprintf(sfp, "extracted %d files, %lld bytes", totalFiles, (long long)totalBytes);
        fclose(sfp);
    }

    LOGI("Bulk extraction complete: %d files, %lld bytes (%.1f MB)",
         totalFiles, (long long)totalBytes, totalBytes / (1024.0 * 1024.0));
}

// ============================================================================
// mu_fopen_android — fopen replacement
// ============================================================================

FILE* mu_fopen_android(const char* path, const char* mode) {
    if (!path) return nullptr;

    std::string normPath = normalizePath(path);

    bool isWrite = mode && (strchr(mode, 'w') || strchr(mode, 'a'));

    // --- Absolute path: pass through ---
    if (normPath[0] == '/') {
        if (isWrite) {
            ensureParentDir(normPath.c_str());
        }
        return (fopen)(normPath.c_str(), mode);
    }

    // --- Relative write path: prepend base path ---
    if (isWrite) {
        std::string diskPath = g_basePath + normPath;
        ensureParentDir(diskPath.c_str());
        return (fopen)(diskPath.c_str(), mode);
    }

    // --- Relative read-only path ---
    // Step 1: Try disk (already extracted from a previous run)
    std::string diskPath = g_basePath + normPath;
    FILE* fp = (fopen)(diskPath.c_str(), mode);
    if (fp) return fp;

    // Step 2: Try extracting from APK assets
    if (g_assetMgr && extractAsset(normPath.c_str(), diskPath.c_str())) {
        LOGI("Extracted from APK: %s -> %s", normPath.c_str(), diskPath.c_str());
        fp = (fopen)(diskPath.c_str(), mode);
        if (fp) return fp;
    }

    // Step 3: Final fallback
    return (fopen)(path, mode);
}
