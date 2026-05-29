package com.mu.client

import android.os.Build
import android.os.Bundle
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat

class MuActivity : AppCompatActivity() {

    private lateinit var muSurfaceView: MuSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Edge-to-edge fullscreen (modern API, handles notch/gesture-nav)
        WindowCompat.setDecorFitsSystemWindows(window, false)

        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        // hideSystemBars is deferred to onWindowFocusChanged, where the
        // DecorView is guaranteed to be attached to the window.
        // Calling it here on API 30+ throws NPE because insetsController
        // isn't available until the window is fully attached.

        muSurfaceView = MuSurfaceView(this)
        setContentView(muSurfaceView)

        MuJNI.nativeInit(assets, filesDir.absolutePath)

        // Pass AssetManager for BGM playback
        MuJNI.setBgmAssetManager(assets)

        // Login is handled by the ImGui-based UI panel in the native game loop.
        // Auto-login disabled — user enters credentials in the app.
        // MuJNI.nativeConnectAndLogin("192.168.1.127", 44404, "louismk", "1212")
    }

    private fun hideSystemBars() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            // API 31+ has a reliable getInsetsController on Window.
            window.insetsController?.apply {
                hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                systemBarsBehavior = WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            )
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) hideSystemBars()
    }

    override fun onPause() {
        super.onPause()
        MuJNI.nativePause()
    }

    override fun onResume() {
        super.onResume()
        MuJNI.nativeResume()
        hideSystemBars()
    }

    override fun onDestroy() {
        MuJNI.nativeDestroy()
        super.onDestroy()
    }
}
