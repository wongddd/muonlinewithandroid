#pragma once

#include <android/asset_manager.h>
#include <android/native_window.h>

namespace MuGameLoop {

bool init(AAssetManager* assetManager, const char* internalPath);
void destroy();

// Surface lifecycle — creates/destroys EGL and starts/stops render thread
void onSurfaceCreated(ANativeWindow* window);
void onSurfaceDestroyed();

// App lifecycle
void pause();
void resume();

// Update game window dimensions to match surface (fullscreen support)
void updateScreenSize(int width, int height);

// Accessors for the game loop
bool isRunning();

} // namespace MuGameLoop
