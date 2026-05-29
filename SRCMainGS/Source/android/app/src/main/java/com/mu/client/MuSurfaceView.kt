package com.mu.client

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Rect
import android.os.Handler
import android.os.Looper
import android.text.InputType
import android.util.Log
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.view.inputmethod.BaseInputConnection
import android.view.inputmethod.EditorInfo
import android.view.inputmethod.InputConnection
import android.view.inputmethod.InputMethodManager

class MuSurfaceView(context: Context) : SurfaceView(context), SurfaceHolder.Callback {

    companion object {
        private const val TAG = "MuSurfaceView"
    }

    private val imm: InputMethodManager =
        context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
    private val mainHandler = Handler(Looper.getMainLooper())

    // Track whether text editing mode is active (set via native callback)
    private var isTextEditing = false

    init {
        holder.addCallback(this)
        isFocusable = true
        isFocusableInTouchMode = true
        MuJNI.activeView = this
    }

    // --- SurfaceHolder.Callback ---

    override fun surfaceCreated(holder: SurfaceHolder) {
        MuJNI.nativeSurfaceCreated(holder.surface)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        MuJNI.nativeSetScreenSize(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        MuJNI.nativeSurfaceDestroyed()
    }

    // --- Touch input (forwarded to native) ---

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val action = event.actionMasked
        val pointerIndex = event.actionIndex
        val pointerId = event.getPointerId(pointerIndex)
        val x = event.getX(pointerIndex)
        val y = event.getY(pointerIndex)

        val actionName = when (action) {
            MotionEvent.ACTION_DOWN -> "DOWN"
            MotionEvent.ACTION_UP -> "UP"
            MotionEvent.ACTION_MOVE -> "MOVE"
            MotionEvent.ACTION_POINTER_DOWN -> "PTR_DOWN"
            MotionEvent.ACTION_POINTER_UP -> "PTR_UP"
            MotionEvent.ACTION_CANCEL -> "CANCEL"
            else -> "?$action"
        }
        Log.d(TAG, "onTouchEvent: $actionName id=$pointerId x=$x y=$y (count=${event.pointerCount})")

        when (action) {
            MotionEvent.ACTION_DOWN,
            MotionEvent.ACTION_POINTER_DOWN -> MuJNI.nativeTouchDown(x, y, pointerId)
            MotionEvent.ACTION_MOVE -> {
                for (i in 0 until event.pointerCount) {
                    MuJNI.nativeTouchMove(event.getX(i), event.getY(i), event.getPointerId(i))
                }
            }
            MotionEvent.ACTION_UP,
            MotionEvent.ACTION_POINTER_UP,
            MotionEvent.ACTION_CANCEL -> MuJNI.nativeTouchUp(x, y, pointerId)
        }
        return true
    }

    // --- Keyboard events (hardware keyboard) ---

    override fun onKeyDown(keyCode: Int, event: KeyEvent): Boolean {
        MuJNI.nativeKeyDown(keyCode)
        // After key event, refresh text texture if in editing mode
        if (isTextEditing) {
            refreshTextTexture()
        }
        return true
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent): Boolean {
        MuJNI.nativeKeyUp(keyCode)
        return true
    }

    // Refreshes the text texture by reading current text from native and rendering it.
    // Used for hardware keyboard path where text changes happen via key events (not IME commitText).
    private fun refreshTextTexture() {
        val fullText = MuJNI.nativeGetCurrentText()
        if (!fullText.isNullOrEmpty()) {
            Log.d(TAG, "refreshTextTexture: '$fullText' (${fullText.length} chars)")
            renderTextToTexture(fullText)
        }
    }

    // --- IME (soft keyboard) support ---

    override fun onCreateInputConnection(outAttrs: EditorInfo): InputConnection {
        outAttrs.inputType = InputType.TYPE_CLASS_TEXT
        outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_FULLSCREEN
        return object : BaseInputConnection(this, false) {
            override fun commitText(text: CharSequence?, newCursorPosition: Int): Boolean {
                text?.let {
                    Log.d(TAG, "commitText IME: '$it'")
                    MuJNI.nativeCommitText(it.toString())
                    // Render the full accumulated text from native buffer
                    val fullText = MuJNI.nativeGetCurrentText()
                    Log.d(TAG, "commitText fullText after commit: '$fullText'")
                    if (!fullText.isNullOrEmpty()) {
                        renderTextToTexture(fullText)
                    }
                }
                return true
            }

            override fun deleteSurroundingText(beforeLength: Int, afterLength: Int): Boolean {
                Log.d(TAG, "deleteSurroundingText: before=$beforeLength")
                for (i in 0 until beforeLength) {
                    MuJNI.nativeKeyDown(KeyEvent.KEYCODE_DEL)
                    MuJNI.nativeKeyUp(KeyEvent.KEYCODE_DEL)
                }
                // After delete, render the remaining text
                val fullText = MuJNI.nativeGetCurrentText()
                Log.d(TAG, "deleteSurroundingText fullText after delete: '$fullText'")
                if (!fullText.isNullOrEmpty()) {
                    renderTextToTexture(fullText)
                }
                return true
            }

            override fun sendKeyEvent(event: KeyEvent): Boolean {
                if (event.action == KeyEvent.ACTION_DOWN) {
                    MuJNI.nativeKeyDown(event.keyCode)
                } else {
                    MuJNI.nativeKeyUp(event.keyCode)
                }
                return true
            }
        }
    }

    // Renders text to a Bitmap using Android Canvas and uploads to native GL texture.
    // Called from commitText, onKeyDown, and deleteSurroundingText whenever the user types.
    private fun renderTextToTexture(text: String) {
        if (text.isEmpty()) return

        Log.d(TAG, "renderTextToTexture: rendering '$text' (${text.length} chars)")

        val paint = Paint().apply {
            color = Color.WHITE
            textSize = 36f
            isAntiAlias = true
            typeface = android.graphics.Typeface.DEFAULT
            textAlign = Paint.Align.LEFT
        }

        // Measure text dimensions
        val bounds = Rect()
        paint.getTextBounds(text, 0, text.length, bounds)
        val width = (bounds.width() + 8).coerceAtLeast(1)
        val height = (bounds.height() + 8).coerceAtLeast(1)

        // Round up to power-of-2 for GL texture
        val texWidth = Integer.highestOneBit(width - 1) shl 1
        val texHeight = Integer.highestOneBit(height - 1) shl 1

        val bitmap = Bitmap.createBitmap(texWidth, texHeight, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        canvas.drawColor(Color.TRANSPARENT, android.graphics.PorterDuff.Mode.CLEAR)
        canvas.drawText(text, 2f, (texHeight - paint.descent() - paint.ascent()) / 2f, paint)

        // Extract pixels as int array
        val pixels = IntArray(texWidth * texHeight)
        bitmap.getPixels(pixels, 0, texWidth, 0, 0, texWidth, texHeight)
        bitmap.recycle()

        MuJNI.nativeUpdateTextPixels(pixels, texWidth, texHeight)
    }

    // Called from native via JNI when text editing mode changes
    fun onTextEditModeChanged(editing: Boolean) {
        mainHandler.post {
            isTextEditing = editing
            Log.d(TAG, "onTextEditModeChanged: editing=$editing")
            if (editing) {
                requestFocus()
                imm.showSoftInput(this, InputMethodManager.SHOW_IMPLICIT)
            } else {
                imm.hideSoftInputFromWindow(windowToken, 0)
            }
        }
    }
}
