package com.mu.client

import android.content.res.AssetFileDescriptor
import android.media.MediaPlayer
import android.util.Log
import android.view.Surface

/**
 * JNI bridge to the native Mu Online client engine.
 * All game logic runs in C++ via the NDK.
 */
object MuJNI {
    init {
        System.loadLibrary("mu-client")
    }

    // The active SurfaceView that receives keyboard show/hide callbacks
    var activeView: MuSurfaceView? = null

    // Called from native (render thread) to show/hide the soft keyboard
    @JvmStatic
    fun showKeyboard(show: Boolean) {
        activeView?.onTextEditModeChanged(show)
    }

    // BGM player instance (lazy, created on first PlayMp3 call)
    private var bgmPlayer: MediaPlayer? = null
    private var bgmCurrentPath: String? = null
    private var bgmEnabled: Boolean = true
    private var bgmVolume: Float = 0.7f
    private var bgmAssetManager: android.content.res.AssetManager? = null
    private var dataDir: String = "/data/user/0/com.mu.client/files/Data"

    fun setBgmAssetManager(am: android.content.res.AssetManager) {
        bgmAssetManager = am
    }

    // Called from native code to set the actual data directory
    @JvmStatic
    fun setDataDir(path: String) {
        dataDir = path
        Log.d("MuBGM", "dataDir set to: $dataDir")
    }

    @JvmStatic
    fun bgmPlay(path: String) {
        Log.d("MuBGM", "bgmPlay: $path (enabled=$bgmEnabled)")
        if (!bgmEnabled) return
        if (path == bgmCurrentPath) return  // Already playing this track

        // Stop current playback
        bgmStopInternal()

        try {
            val mp = MediaPlayer()
            // Try filesystem first (extracted from APK assets to app internal storage)
            val filePath = "$dataDir/$path"
            val file = java.io.File(filePath)
            if (file.exists()) {
                mp.setDataSource(filePath)
                Log.d("MuBGM", "bgmPlay: using file path '$filePath'")
            } else {
                // Fallback to APK assets (for small files bundled in APK)
                val am = bgmAssetManager ?: run {
                    Log.e("MuBGM", "bgmPlay: AssetManager not set and file not found: $filePath")
                    mp.release()
                    return
                }
                val afd: AssetFileDescriptor = am.openFd(path)
                mp.setDataSource(afd.fileDescriptor, afd.startOffset, afd.length)
                afd.close()
                Log.d("MuBGM", "bgmPlay: using asset path '$path'")
            }
            mp.isLooping = true
            mp.setVolume(bgmVolume, bgmVolume)
            mp.prepare()
            mp.start()
            bgmPlayer = mp
            bgmCurrentPath = path
            Log.d("MuBGM", "bgmPlay: started '$path'")
        } catch (e: Exception) {
            Log.e("MuBGM", "bgmPlay failed for '$path': ${e.message}")
        }
    }

    @JvmStatic
    fun bgmStop() {
        Log.d("MuBGM", "bgmStop")
        bgmStopInternal()
    }

    @JvmStatic
    fun bgmSetVolume(volume: Float) {
        bgmVolume = volume.coerceIn(0f, 1f)
        bgmPlayer?.setVolume(bgmVolume, bgmVolume)
    }

    @JvmStatic
    fun bgmSetEnabled(enabled: Boolean) {
        bgmEnabled = enabled
        if (!enabled) {
            bgmStopInternal()
        }
    }

    private fun bgmStopInternal() {
        bgmPlayer?.apply {
            if (isPlaying) stop()
            reset()
            release()
        }
        bgmPlayer = null
        bgmCurrentPath = null
    }

    // Lifecycle
    external fun nativeInit(assetManager: android.content.res.AssetManager, internalPath: String): Boolean
    external fun nativeDestroy()

    // Surface (EGL context managed natively)
    external fun nativeSurfaceCreated(surface: Surface)
    external fun nativeSurfaceDestroyed()

    // Input
    external fun nativeTouchDown(x: Float, y: Float, pointerId: Int)
    external fun nativeTouchMove(x: Float, y: Float, pointerId: Int)
    external fun nativeTouchUp(x: Float, y: Float, pointerId: Int)
    external fun nativeKeyDown(keyCode: Int)
    external fun nativeKeyUp(keyCode: Int)

    // Screen
    external fun nativeSetScreenSize(width: Int, height: Int)

    // App lifecycle
    external fun nativePause()
    external fun nativeResume()

    // Network
    external fun nativeConnectAndLogin(ip: String, port: Int, account: String, password: String)
    external fun nativeGetState(): String

    // IME text input
    external fun nativeCommitText(text: String)
    external fun nativeSetTextEditMode(editing: Boolean)
    external fun nativeHideKeyboard()

    // Get current text from the focused text input box (for rendering on Android)
    external fun nativeGetCurrentText(): String?

    // Android-side rendered text texture upload
    external fun nativeUpdateTextPixels(pixels: IntArray, width: Int, height: Int)

    // BGM (background music) — MediaPlayer-based MP3 playback
    external fun nativeBgmPlay(path: String)
    external fun nativeBgmStop()
    external fun nativeBgmSetVolume(volume: Float)  // 0.0 .. 1.0
    external fun nativeBgmSetEnabled(enabled: Boolean)
}
