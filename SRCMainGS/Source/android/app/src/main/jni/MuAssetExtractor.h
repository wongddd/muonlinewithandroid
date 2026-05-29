#pragma once

#include <cstdio>

// Initialize the asset extractor (must be called before any game code that uses fopen)
void mu_asset_extractor_init(void* assetManager, const char* internalPath);

// Bulk-extract all APK assets to disk (runs once, uses a sentinel file)
void mu_asset_extractor_extract_all();

// fopen replacement — normalizes paths, extracts from APK assets on demand
// Use (fopen)(...) inside this function's implementation to call real libc fopen
FILE* mu_fopen_android(const char* path, const char* mode);
