#include "MuConfig.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

static const char* CONFIG_FILE = ".mu_config";

bool config_load(GameConfig& cfg, const char* basePath) {
    if (!basePath || !basePath[0]) return false;
    char path[260];
    snprintf(path, sizeof(path), "%s%s", basePath, CONFIG_FILE);

    FILE* fp = fopen(path, "r");
    if (!fp) return false;

    char line[128];
    while (fgets(line, sizeof(line), fp)) {
        // Strip newline
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';

        char key[64] = {}, val[96] = {};
        if (sscanf(line, "%63[^=]=%95s", key, val) >= 1) {
            if      (strcmp(key, "language") == 0)        cfg.language = atoi(val);
            else if (strcmp(key, "autoReconnect") == 0)   cfg.autoReconnect = atoi(val);
            else if (strcmp(key, "saveAccount") == 0)     cfg.saveAccount = atoi(val);
            else if (strcmp(key, "bgmVolume") == 0)       cfg.bgmVolume = atoi(val);
            else if (strcmp(key, "sfxVolume") == 0)       cfg.sfxVolume = atoi(val);
            else if (strcmp(key, "brightness") == 0)      cfg.screenBrightness = atoi(val);
            else if (strcmp(key, "account") == 0) {
                strncpy(cfg.account, val, sizeof(cfg.account) - 1);
            } else if (strcmp(key, "password") == 0) {
                strncpy(cfg.password, val, sizeof(cfg.password) - 1);
            }
        }
    }
    fclose(fp);
    return true;
}

bool config_save(const GameConfig& cfg, const char* basePath) {
    if (!basePath || !basePath[0]) return false;
    char path[260];
    snprintf(path, sizeof(path), "%s%s", basePath, CONFIG_FILE);

    FILE* fp = fopen(path, "w");
    if (!fp) return false;

    fprintf(fp, "language=%d\n", cfg.language);
    fprintf(fp, "autoReconnect=%d\n", cfg.autoReconnect);
    fprintf(fp, "saveAccount=%d\n", cfg.saveAccount);
    fprintf(fp, "bgmVolume=%d\n", cfg.bgmVolume);
    fprintf(fp, "sfxVolume=%d\n", cfg.sfxVolume);
    fprintf(fp, "brightness=%d\n", cfg.screenBrightness);
    if (cfg.account[0]) fprintf(fp, "account=%s\n", cfg.account);
    if (cfg.password[0]) fprintf(fp, "password=%s\n", cfg.password);
    fclose(fp);
    return true;
}
