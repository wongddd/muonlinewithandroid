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

static const char* g_externalRoots[] = {
    "/sdcard/",
    "/storage/emulated/0/",
    "/sdcard/MU/",
    "/storage/emulated/0/MU/",
    nullptr
};

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
    "Data/World6", "Data/World7", "Data/World8", "Data/World9",
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
    "Data/World100", "Data/World101", "Data/World111",
    "Data/World134", "Data/World135", "Data/World136", "Data/World137",
    "Data/Object1", "Data/Object2", "Data/Object3", "Data/Object4", "Data/Object5",
    "Data/Object6", "Data/Object7", "Data/Object8", "Data/Object9",
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
    "Data/Object100", "Data/Object101", "Data/Object111",
    "Data/Object134", "Data/Object135", "Data/Object136", "Data/Object137",
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
    while (result.compare(0, 2, "./") == 0) {
        result.erase(0, 2);
    }
    return result;
}

static bool isReadOnlyMode(const char* mode) {
    return !(mode && (strchr(mode, 'w') || strchr(mode, 'a') || strchr(mode, '+')));
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

static bool tryOpenFile(const std::string& path, const char* mode, FILE** out) {
    FILE* fp = (fopen)(path.c_str(), mode);
    if (!fp) return false;
    *out = fp;
    return true;
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

static bool extractAssetTree(const std::string& assetDir,
                             const std::string& diskDir,
                             int* fileCount,
                             int64_t* byteCount) {
    if (!g_assetMgr) return false;

    AAssetDir* dir = AAssetManager_openDir(g_assetMgr, assetDir.c_str());
    if (!dir) return false;

    bool extractedAny = false;
    const char* name;
    while ((name = AAssetDir_getNextFileName(dir)) != nullptr) {
        std::string assetPath = assetDir.empty() ? name : assetDir + "/" + name;
        std::string diskPath = diskDir.empty() ? name : diskDir + "/" + name;

        if (extractAsset(assetPath.c_str(), diskPath.c_str())) {
            extractedAny = true;
            if (fileCount) (*fileCount)++;
            if (byteCount) {
                struct stat st;
                if (stat(diskPath.c_str(), &st) == 0) *byteCount += st.st_size;
            }
            continue;
        }

        std::string childDiskDir = diskPath;
        if (extractAssetTree(assetPath, childDiskDir, fileCount, byteCount)) {
            extractedAny = true;
        }
    }

    AAssetDir_close(dir);
    return extractedAny;
}

// ============================================================================
// Progress tracking for extraction
// ============================================================================

static volatile float g_extractProgress = -1.0f;  // -1 = not extracting
static int g_extractTotalDirs = 0;

float mu_asset_extractor_get_progress() {
    return g_extractProgress;
}

// ============================================================================
// Bulk extraction — extract all game data from APK to disk on first launch
// ============================================================================

void mu_asset_extractor_extract_all() {
    if (!g_assetMgr || g_basePath.empty()) {
        LOGE("extract_all: not initialized");
        return;
    }

    // Count total directories for progress
    if (g_extractTotalDirs == 0) {
        int count = 0;
        for (int i = 0; g_dataDirs[i] != nullptr; i++) count++;
        g_extractTotalDirs = count + 1 + 139 * 2; // +1 for Data/, plus World/Object0..138
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
    g_extractProgress = 0.0f;

    int totalFiles = 0;
    int64_t totalBytes = 0;
    int dirIndex = 0;

    // Extract root-level files and any directory tree discoverable in assets/Data.
    {
        extractAssetTree("Data", g_basePath + "Data", &totalFiles, &totalBytes);
        dirIndex++;
        g_extractProgress = (float)dirIndex / (float)g_extractTotalDirs;
    }

    // Extract files from each known subdirectory
    for (int i = 0; g_dataDirs[i] != nullptr; i++) {
        AAssetDir* dir = AAssetManager_openDir(g_assetMgr, g_dataDirs[i]);
        if (!dir) {
            dirIndex++;
            g_extractProgress = (float)dirIndex / (float)g_extractTotalDirs;
            continue;
        }

        extractAssetTree(g_dataDirs[i], g_basePath + g_dataDirs[i], &totalFiles, &totalBytes);
        AAssetDir_close(dir);

        dirIndex++;
        g_extractProgress = (float)dirIndex / (float)g_extractTotalDirs;
    }

    // The PC client derives map resource directories as World%d/Object%d from
    // map ids. Keep this broad so newly added maps in the copied Data tree or
    // APK assets are not missed by a static whitelist.
    for (int map = 0; map <= 138; ++map) {
        char worldDir[32];
        char objectDir[32];
        snprintf(worldDir, sizeof(worldDir), "Data/World%d", map);
        snprintf(objectDir, sizeof(objectDir), "Data/Object%d", map);

        extractAssetTree(worldDir, g_basePath + worldDir, &totalFiles, &totalBytes);
        dirIndex++;
        g_extractProgress = (float)dirIndex / (float)g_extractTotalDirs;

        extractAssetTree(objectDir, g_basePath + objectDir, &totalFiles, &totalBytes);
        dirIndex++;
        g_extractProgress = (float)dirIndex / (float)g_extractTotalDirs;
    }

    // Write sentinel
    FILE* sfp = (fopen)(sentinelPath.c_str(), "w");
    if (sfp) {
        fprintf(sfp, "extracted %d files, %lld bytes", totalFiles, (long long)totalBytes);
        fclose(sfp);
    }

    g_extractProgress = -1.0f;
    LOGI("Bulk extraction complete: %d files, %lld bytes (%.1f MB)",
         totalFiles, (long long)totalBytes, totalBytes / (1024.0 * 1024.0));
}

// ============================================================================
// mu_fopen_android — fopen replacement
// ============================================================================

FILE* mu_fopen_android(const char* path, const char* mode) {
    if (!path) return nullptr;

    std::string normPath = normalizePath(path);
    if (normPath.empty()) return nullptr;

    const char* fopenMode = mode ? mode : "rb";
    bool isWrite = !isReadOnlyMode(fopenMode);

    // --- Absolute path: pass through ---
    if (normPath[0] == '/') {
        if (isWrite) {
            ensureParentDir(normPath.c_str());
        }
        return (fopen)(normPath.c_str(), fopenMode);
    }

    // --- Relative write path: prepend base path ---
    if (isWrite) {
        std::string diskPath = g_basePath + normPath;
        ensureParentDir(diskPath.c_str());
        return (fopen)(diskPath.c_str(), fopenMode);
    }

    // --- Relative read-only path ---
    FILE* fp = nullptr;
    std::string canonicalPath = canonicalizeResourcePath(normPath);

    // Step 1: Try public external data roots populated from the PC client.
    for (int i = 0; g_externalRoots[i] != nullptr; ++i) {
        std::string candidate = std::string(g_externalRoots[i]) + normPath;
        if (tryOpenFile(candidate, fopenMode, &fp)) return fp;
        candidate = std::string(g_externalRoots[i]) + canonicalPath;
        if (tryOpenFile(candidate, fopenMode, &fp)) return fp;
        if (!hasDataPrefix(normPath)) {
            candidate = std::string(g_externalRoots[i]) + "Data/" + normPath;
            if (tryOpenFile(candidate, fopenMode, &fp)) return fp;
            candidate = std::string(g_externalRoots[i]) + "Data/" + canonicalPath;
            if (tryOpenFile(candidate, fopenMode, &fp)) return fp;
        }
    }

    // Step 2: Try internal disk cache (already extracted from a previous run).
    std::string diskPath = g_basePath + normPath;
    fp = (fopen)(diskPath.c_str(), fopenMode);
    if (fp) return fp;
    diskPath = g_basePath + canonicalPath;
    fp = (fopen)(diskPath.c_str(), fopenMode);
    if (fp) return fp;
    if (!hasDataPrefix(normPath)) {
        diskPath = g_basePath + "Data/" + normPath;
        fp = (fopen)(diskPath.c_str(), fopenMode);
        if (fp) return fp;
        diskPath = g_basePath + "Data/" + canonicalPath;
        fp = (fopen)(diskPath.c_str(), fopenMode);
        if (fp) return fp;
    }

    // Step 3: Try extracting from APK assets.
    diskPath = g_basePath + normPath;
    if (g_assetMgr && extractAsset(normPath.c_str(), diskPath.c_str())) {
        LOGI("Extracted from APK: %s -> %s", normPath.c_str(), diskPath.c_str());
        fp = (fopen)(diskPath.c_str(), fopenMode);
        if (fp) return fp;
    }
    diskPath = g_basePath + canonicalPath;
    if (g_assetMgr && canonicalPath != normPath && extractAsset(canonicalPath.c_str(), diskPath.c_str())) {
        LOGI("Extracted from APK: %s -> %s", canonicalPath.c_str(), diskPath.c_str());
        fp = (fopen)(diskPath.c_str(), fopenMode);
        if (fp) return fp;
    }
    if (g_assetMgr && !hasDataPrefix(normPath)) {
        std::string assetPath = "Data/" + normPath;
        diskPath = g_basePath + assetPath;
        if (extractAsset(assetPath.c_str(), diskPath.c_str())) {
            LOGI("Extracted from APK: %s -> %s", assetPath.c_str(), diskPath.c_str());
            fp = (fopen)(diskPath.c_str(), fopenMode);
            if (fp) return fp;
        }
        assetPath = "Data/" + canonicalPath;
        diskPath = g_basePath + assetPath;
        if (canonicalPath != normPath && extractAsset(assetPath.c_str(), diskPath.c_str())) {
            LOGI("Extracted from APK: %s -> %s", assetPath.c_str(), diskPath.c_str());
            fp = (fopen)(diskPath.c_str(), fopenMode);
            if (fp) return fp;
        }
    }

    // Step 4: Final fallback for callers that already use a process-relative path.
    return (fopen)(path, fopenMode);
}
