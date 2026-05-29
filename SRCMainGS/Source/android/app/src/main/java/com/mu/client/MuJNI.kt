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
                // Fallback to APK assets (stored under Data/ prefix)
                val am = bgmAssetManager ?: run {
                    Log.e("MuBGM", "bgmPlay: AssetManager not set and file not found: $filePath")
                    mp.release()
                    return
                }
                // Assets are at Data/Music/login_theme.mp3 (from assets/Data/)
                var assetPath = path
                if (!assetPath.startsWith("Data/")) {
                    assetPath = "Data/$assetPath"
                }
                val afd: AssetFileDescriptor = am.openFd(assetPath)
                mp.setDataSource(afd.fileDescriptor, afd.startOffset, afd.length)
                afd.close()
                Log.d("MuBGM", "bgmPlay: using asset path '$assetPath'")
                // Also extract to disk for future runs
                try {
                    val outFile = java.io.File(filePath)
                    outFile.parentFile?.mkdirs()
                    val input = am.open(assetPath)
                    val output = java.io.FileOutputStream(outFile)
                    input.copyTo(output)
                    input.close()
                    output.close()
                    Log.d("MuBGM", "bgmPlay: extracted to $filePath")
                } catch (e: Exception) {
                    Log.e("MuBGM", "bgmPlay: extract failed: ${e.message}")
                }
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

    // ======== P1-2: SFX sound effects ========
    private var sfxPlayer: MediaPlayer? = null

    @JvmStatic
    fun sfxPlay(path: String, loop: Int) {
        try {
            val filePath = "$dataDir/$path"
            val file = java.io.File(filePath)
            if (!file.exists()) {
                // Try extracting from APK assets
                val am = bgmAssetManager
                if (am != null) {
                    val assetPath = if (path.startsWith("Data/")) path else "Data/$path"
                    try {
                        file.parentFile?.mkdirs()
                        val input = am.open(assetPath)
                        val output = java.io.FileOutputStream(file)
                        input.copyTo(output)
                        input.close()
                        output.close()
                    } catch (_: Exception) {}
                }
            }
            if (file.exists()) {
                sfxPlayer?.release()
                sfxPlayer = MediaPlayer().apply {
                    setDataSource(filePath)
                    isLooping = loop != 0
                    setVolume(0.7f, 0.7f)
                    prepare()
                    start()
                }
                Log.d("MuSFX", "sfxPlay: $path")
            }
        } catch (e: Exception) {
            Log.e("MuSFX", "sfxPlay failed: ${e.message}")
        }
    }

    @JvmStatic
    fun bgmSetEnabled(enabled: Boolean) {
        bgmEnabled = enabled
        if (!enabled) {
            bgmStopInternal()
        }
    }

    // ======== 资源下载 (Task 6) ========
    @Volatile
    private var downloadProgress: Float = -1f  // -1 = not started, 0-100 = progress
    private var downloadStatus: String = ""
    private var isDownloading: Boolean = false
    private var downloadThread: Thread? = null

    @JvmStatic
    fun nativeDownloadData(url: String) {
        if (isDownloading) return
        isDownloading = true
        downloadProgress = 0f
        downloadStatus = "Connecting..."

        downloadThread = Thread {
            try {
                val dataDir = java.io.File("/data/user/0/com.mu.client/files/")
                val zipFile = java.io.File(dataDir, "gamedata.zip")
                val extractDir = java.io.File(dataDir, "Data")

                val urlObj = java.net.URL(url)
                val conn = urlObj.openConnection() as java.net.HttpURLConnection
                conn.connectTimeout = 15000
                conn.readTimeout = 30000
                conn.connect()

                val totalSize = conn.contentLengthLong
                val input = conn.inputStream
                val output = java.io.FileOutputStream(zipFile)
                val buffer = ByteArray(8192)
                var bytesRead: Int
                var totalRead: Long = 0

                while (input.read(buffer).also { bytesRead = it } != -1) {
                    output.write(buffer, 0, bytesRead)
                    totalRead += bytesRead
                    if (totalSize > 0) {
                        downloadProgress = (totalRead * 100f / totalSize)
                    }
                    downloadStatus = "Downloading ${downloadProgress.toInt()}%"
                }
                output.close()
                input.close()
                conn.disconnect()
                downloadProgress = 100f
                downloadStatus = "Extracting..."

                // Extract ZIP
                val zip = java.util.zip.ZipFile(zipFile)
                val entries = zip.entries()
                var extracted = 0
                val totalEntries = zip.size()
                while (entries.hasMoreElements()) {
                    val entry = entries.nextElement()
                    val target = java.io.File(extractDir, entry.name)
                    if (entry.isDirectory) {
                        target.mkdirs()
                    } else {
                        target.parentFile?.mkdirs()
                        val zis = zip.getInputStream(entry)
                        val fos = java.io.FileOutputStream(target)
                        zis.copyTo(fos)
                        fos.close()
                        zis.close()
                    }
                    extracted++
                    downloadProgress = 100f + (extracted * 100f / totalEntries)  // 100-200% = extracting
                }
                zip.close()
                zipFile.delete()
                downloadProgress = 200f
                downloadStatus = "Complete!"
                Log.d("MuDownload", "Download + extract complete: $totalEntries files")
            } catch (e: Exception) {
                downloadStatus = "Failed: ${e.message}"
                Log.e("MuDownload", "Download failed: ${e.message}")
            } finally {
                isDownloading = false
            }
        }.apply { start() }
    }

    @JvmStatic
    fun nativeGetDownloadProgress(): Float = downloadProgress

    @JvmStatic
    fun nativeIsDownloading(): Boolean = isDownloading

    @JvmStatic
    fun nativeGetDownloadStatus(): String = downloadStatus

    // ======== P2-6: Screenshot ========
    @JvmStatic
    fun nativeScreenshot() {
        Log.d("MuScreenshot", "nativeScreenshot called")
        Thread {
            try {
                val now = java.text.SimpleDateFormat("yyyyMMdd_HHmmss", java.util.Locale.US).format(java.util.Date())
                val dir = java.io.File("/sdcard/Pictures/MU/")
                dir.mkdirs()
                val file = java.io.File(dir, "MU_$now.png")
                val proc = Runtime.getRuntime().exec("screencap -p ${file.absolutePath}")
                proc.waitFor()
                // Notify gallery
                val values = android.content.ContentValues().apply {
                    put(android.provider.MediaStore.Images.Media.DATA, file.absolutePath)
                    put(android.provider.MediaStore.Images.Media.MIME_TYPE, "image/png")
                }
                val context = android.app.ActivityManager::class.java
                Log.d("MuScreenshot", "Saved: ${file.absolutePath}")
            } catch (e: Exception) {
                Log.e("MuScreenshot", "Failed: ${e.message}")
            }
        }.start()
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

    // Resource download (pure Kotlin, no native impl needed)
    // fun downloadData(url: String) — defined below

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
