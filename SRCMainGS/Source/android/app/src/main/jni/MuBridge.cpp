#include <jni.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/log.h>

#include <cstring>
#include <string>
#include <mutex>
#include <strings.h>
#include <vector>
#include <GLES3/gl3.h>

#include "MuGameLoop.h"
#include "MuGLContext.h"
#include "MuInput.h"
#include "ProtocolDispatch.h"

#define LOG_TAG "MuClient"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static bool g_initialized = false;

// --- JNI callback support ---

static JavaVM* g_jvm = nullptr;
static jclass  g_muJniClass = nullptr;
static jmethodID g_showKeyboardMethod = nullptr;
static jmethodID g_bgmPlayMethod = nullptr;
static jmethodID g_bgmStopMethod = nullptr;
static jmethodID g_bgmSetVolumeMethod = nullptr;
static jmethodID g_bgmSetEnabledMethod = nullptr;

// Forward declaration from UIControls.cpp (Android_DeliverIMEText)
extern "C" void Android_DeliverIMEText(const char* utf8);

// Forward declaration from MuGameLoop.cpp (ImGui text delivery helper)
extern bool ImGui_DeliverText(const char* utf8);

// --- Android-side text texture (Canvas-rendered, uploaded to GL) ---
// Pixel data arrives from Java UI thread; texture is created on render thread.
static std::mutex          g_textMutex;
static std::vector<uint8_t> g_textPixels;  // RGBA bytes (swizzled from ARGB)
static int                  g_textWidth  = 0;
static int                  g_textHeight = 0;
static bool                 g_textPending = false;

// Called from UIControls.cpp (render thread) to draw the cached text texture
void Android_RenderTextTexture(float x, float y, float maxWidth, float maxHeight);

// Forward-declare: defined in ZzzOpenglUtil.cpp (compiled for Android)
extern void RenderBitmap(int Texture, float x, float y, float Width, float Height,
                         float u, float v, float uWidth, float vHeight,
                         bool Scale, bool StartScale, float Alpha);

// Called automatically by the JVM when the native library is loaded
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /* reserved */) {
    g_jvm = vm;
    JNIEnv* env;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_VERSION_1_6;
    }
    jclass localClass = env->FindClass("com/mu/client/MuJNI");
    if (localClass) {
        g_muJniClass = (jclass)env->NewGlobalRef(localClass);
        g_showKeyboardMethod = env->GetStaticMethodID(g_muJniClass, "showKeyboard", "(Z)V");
        g_bgmPlayMethod      = env->GetStaticMethodID(g_muJniClass, "bgmPlay", "(Ljava/lang/String;)V");
        g_bgmStopMethod      = env->GetStaticMethodID(g_muJniClass, "bgmStop", "()V");
        g_bgmSetVolumeMethod = env->GetStaticMethodID(g_muJniClass, "bgmSetVolume", "(F)V");
        g_bgmSetEnabledMethod = env->GetStaticMethodID(g_muJniClass, "bgmSetEnabled", "(Z)V");
        env->DeleteLocalRef(localClass);
    }
    return JNI_VERSION_1_6;
}

// --- BGM bridge (C++ → Java MediaPlayer) ---
// Called from PlayMp3 stubs to delegate to Android MediaPlayer.
// Thread-safe: can be called from any thread (JNI attaches if needed).

static void callStaticVoidMethod_String(jmethodID method, const char* str) {
    if (!g_jvm || !g_muJniClass || !method) return;
    JNIEnv* env;
    bool attached = false;
    int status = g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if (g_jvm->AttachCurrentThread(&env, nullptr) == 0) attached = true;
        else return;
    }
    if (env) {
        jstring jstr = env->NewStringUTF(str);
        env->CallStaticVoidMethod(g_muJniClass, method, jstr);
        env->DeleteLocalRef(jstr);
        if (attached) g_jvm->DetachCurrentThread();
    }
}

static void callStaticVoidMethod_Float(jmethodID method, float val) {
    if (!g_jvm || !g_muJniClass || !method) return;
    JNIEnv* env;
    bool attached = false;
    int status = g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if (g_jvm->AttachCurrentThread(&env, nullptr) == 0) attached = true;
        else return;
    }
    if (env) {
        env->CallStaticVoidMethod(g_muJniClass, method, val);
        if (attached) g_jvm->DetachCurrentThread();
    }
}

static void callStaticVoidMethod_Bool(jmethodID method, bool val) {
    if (!g_jvm || !g_muJniClass || !method) return;
    JNIEnv* env;
    bool attached = false;
    int status = g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if (g_jvm->AttachCurrentThread(&env, nullptr) == 0) attached = true;
        else return;
    }
    if (env) {
        env->CallStaticVoidMethod(g_muJniClass, method, val ? JNI_TRUE : JNI_FALSE);
        if (attached) g_jvm->DetachCurrentThread();
    }
}

static void callStaticVoidMethod_Void(jmethodID method) {
    if (!g_jvm || !g_muJniClass || !method) return;
    JNIEnv* env;
    bool attached = false;
    int status = g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if (g_jvm->AttachCurrentThread(&env, nullptr) == 0) attached = true;
        else return;
    }
    if (env) {
        env->CallStaticVoidMethod(g_muJniClass, method);
        if (attached) g_jvm->DetachCurrentThread();
    }
}

void Android_BgmPlay(const char* path) {
    callStaticVoidMethod_String(g_bgmPlayMethod, path);
}

void Android_BgmStop() {
    callStaticVoidMethod_Void(g_bgmStopMethod);
}

void Android_BgmSetVolume(float volume) {
    callStaticVoidMethod_Float(g_bgmSetVolumeMethod, volume);
}

void Android_BgmSetEnabled(bool enabled) {
    callStaticVoidMethod_Bool(g_bgmSetEnabledMethod, enabled);
}

// Override the weak symbol in UIControls.cpp
void Android_ShowKeyboard(bool show) {
    if (!g_jvm || !g_muJniClass || !g_showKeyboardMethod) return;

    JNIEnv* env;
    bool attached = false;
    int status = g_jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (status == JNI_EDETACHED) {
        if (g_jvm->AttachCurrentThread(&env, nullptr) == 0) {
            attached = true;
        } else {
            return;
        }
    }
    if (env) {
        env->CallStaticVoidMethod(g_muJniClass, g_showKeyboardMethod, show ? JNI_TRUE : JNI_FALSE);
        if (attached) {
            g_jvm->DetachCurrentThread();
        }
    }
}

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_mu_client_MuJNI_nativeInit(JNIEnv* env, jclass /* clazz */,
                                     jobject assetManager, jstring internalPath) {
    LOGI("nativeInit called");

    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) {
        LOGE("Failed to get AAssetManager");
        return JNI_FALSE;
    }

    const char* path = env->GetStringUTFChars(internalPath, nullptr);

    if (!MuGameLoop::init(mgr, path)) {
        LOGE("MuGameLoop::init failed");
        env->ReleaseStringUTFChars(internalPath, path);
        return JNI_FALSE;
    }

    // Notify Kotlin side of the actual data directory for MediaPlayer BGM
    jclass clazz = env->FindClass("com/mu/client/MuJNI");
    if (clazz) {
        jmethodID setDataDir = env->GetStaticMethodID(clazz, "setDataDir", "(Ljava/lang/String;)V");
        if (setDataDir) {
            std::string dataDir = std::string(path) + "/Data";
            jstring dataDirStr = env->NewStringUTF(dataDir.c_str());
            env->CallStaticVoidMethod(clazz, setDataDir, dataDirStr);
            env->DeleteLocalRef(dataDirStr);
        }
        env->DeleteLocalRef(clazz);
    }

    env->ReleaseStringUTFChars(internalPath, path);
    g_initialized = true;
    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeDestroy(JNIEnv* /* env */, jclass /* clazz */) {
    LOGI("nativeDestroy called");
    MuGameLoop::destroy();
    g_initialized = false;
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeSurfaceCreated(JNIEnv* env, jclass /* clazz */,
                                               jobject surface) {
    if (!g_initialized) return;
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) {
        LOGE("ANativeWindow_fromSurface returned null");
        return;
    }
    MuGameLoop::onSurfaceCreated(window);
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeSurfaceDestroyed(JNIEnv* /* env */, jclass /* clazz */) {
    if (!g_initialized) return;
    MuGameLoop::onSurfaceDestroyed();
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeTouchDown(JNIEnv* /* env */, jclass /* clazz */,
                                          jfloat x, jfloat y, jint pointerId) {
    if (!g_initialized) return;
    MuInput::onTouchDown(x, y, pointerId);
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeTouchMove(JNIEnv* /* env */, jclass /* clazz */,
                                          jfloat x, jfloat y, jint pointerId) {
    if (!g_initialized) return;
    MuInput::onTouchMove(x, y, pointerId);
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeTouchUp(JNIEnv* /* env */, jclass /* clazz */,
                                        jfloat x, jfloat y, jint pointerId) {
    if (!g_initialized) return;
    MuInput::onTouchUp(x, y, pointerId);
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeKeyDown(JNIEnv* /* env */, jclass /* clazz */,
                                        jint keyCode) {
    if (!g_initialized) return;
    MuInput::onKeyDown(keyCode);
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeKeyUp(JNIEnv* /* env */, jclass /* clazz */,
                                      jint keyCode) {
    if (!g_initialized) return;
    MuInput::onKeyUp(keyCode);
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeSetScreenSize(JNIEnv* /* env */, jclass /* clazz */,
                                              jint width, jint height) {
    if (!g_initialized) return;
    MuInput::init(width, height);
    MuGLContext::onSurfaceChanged(width, height);
    MuGameLoop::updateScreenSize(width, height);
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativePause(JNIEnv* /* env */, jclass /* clazz */) {
    LOGI("nativePause called");
    MuGameLoop::pause();
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeResume(JNIEnv* /* env */, jclass /* clazz */) {
    LOGI("nativeResume called");
    MuGameLoop::resume();
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeConnectAndLogin(JNIEnv* env, jclass /* clazz */,
                                                jstring ip, jint port,
                                                jstring account, jstring password) {
    const char* ipStr = env->GetStringUTFChars(ip, nullptr);
    const char* accStr = env->GetStringUTFChars(account, nullptr);
    const char* pwdStr = env->GetStringUTFChars(password, nullptr);

    LOGI("nativeConnectAndLogin: %s:%d / %s", ipStr, port, accStr);

    ProtocolDispatch::setCredentials(accStr, pwdStr);
    ProtocolDispatch::connectToServer(ipStr, static_cast<uint16_t>(port));

    env->ReleaseStringUTFChars(ip, ipStr);
    env->ReleaseStringUTFChars(account, accStr);
    env->ReleaseStringUTFChars(password, pwdStr);
}

JNIEXPORT jstring JNICALL
Java_com_mu_client_MuJNI_nativeGetState(JNIEnv* env, jclass /* clazz */) {
    return env->NewStringUTF(ProtocolDispatch::getStateName());
}

// --- IME text input ---

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeCommitText(JNIEnv* env, jclass /* clazz */, jstring text) {
    const char* str = env->GetStringUTFChars(text, nullptr);
    if (str) {
        // Route to ImGui first if it wants text input (ImGui login panel)
        bool consumedByImGui = ImGui_DeliverText(str);

        if (!consumedByImGui) {
            // Deliver directly to the focused CUITextInputBox (OldUI)
            Android_DeliverIMEText(str);
        }
        // Also store in MuInput's buffer for any other consumers
        MuInput::appendIMEText(str);
        env->ReleaseStringUTFChars(text, str);
    }
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeSetTextEditMode(JNIEnv* /* env */, jclass /* clazz */, jboolean editing) {
    MuInput::setTextEditMode(editing);
}

JNIEXPORT jstring JNICALL
Java_com_mu_client_MuJNI_nativeGetCurrentText(JNIEnv* env, jclass /* clazz */) {
    // Read current text from the focused Android text box via helper
    extern void Android_TextBox_GetText(void* pBox, char* out, int maxLen);
    extern void* g_pFocusedAndroidTextBox;
    char szText[256] = { 0 };
    Android_TextBox_GetText(g_pFocusedAndroidTextBox, szText, 255);
    if (szText[0] != '\0') {
        return env->NewStringUTF(szText);
    }
    return nullptr;
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeHideKeyboard(JNIEnv* /* env */, jclass /* clazz */) {
    // Keyboard will be hidden when focus is lost (handled by Android_ShowKeyboard(false))
    Android_ShowKeyboard(false);
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeUpdateTextPixels(JNIEnv* env, jclass /* clazz */,
                                                 jintArray pixels, jint width, jint height) {
    if (!pixels || width <= 0 || height <= 0) return;

    jsize len = env->GetArrayLength(pixels);
    jint* data = env->GetIntArrayElements(pixels, nullptr);
    if (!data) return;

    {
        std::lock_guard<std::mutex> lock(g_textMutex);

        g_textPixels.resize(width * height * 4); // RGBA bytes
        g_textWidth = width;
        g_textHeight = height;

        // Swizzle ARGB (Android Bitmap) → RGBA (OpenGL)
        // ARGB int 0xAARRGGBB on little-endian = bytes [BB, GG, RR, AA]
        // RGBA needs bytes [RR, GG, BB, AA]
        uint8_t* dst = g_textPixels.data();
        for (int i = 0; i < len; ++i) {
            uint32_t p = static_cast<uint32_t>(data[i]);
            // p = 0xAARRGGBB; swizzle to 0xAABBGGRR (bytes become [RR, GG, BB, AA])
            uint32_t sw = (p & 0xFF00FF00) | ((p & 0x00FF0000) >> 16) | ((p & 0x000000FF) << 16);
            dst[i * 4 + 0] = static_cast<uint8_t>(sw & 0xFF);       // R
            dst[i * 4 + 1] = static_cast<uint8_t>((sw >> 8) & 0xFF); // G
            dst[i * 4 + 2] = static_cast<uint8_t>((sw >> 16) & 0xFF);// B
            dst[i * 4 + 3] = static_cast<uint8_t>((sw >> 24) & 0xFF);// A
        }

        g_textPending = true;
    }

    env->ReleaseIntArrayElements(pixels, data, JNI_ABORT);
}

// --- BGM JNI native implementations ---

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeBgmPlay(JNIEnv* env, jclass /* clazz */, jstring path) {
    const char* str = env->GetStringUTFChars(path, nullptr);
    if (str) {
        Android_BgmPlay(str);
        env->ReleaseStringUTFChars(path, str);
    }
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeBgmStop(JNIEnv* /* env */, jclass /* clazz */) {
    Android_BgmStop();
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeBgmSetVolume(JNIEnv* /* env */, jclass /* clazz */, jfloat volume) {
    Android_BgmSetVolume(volume);
}

JNIEXPORT void JNICALL
Java_com_mu_client_MuJNI_nativeBgmSetEnabled(JNIEnv* /* env */, jclass /* clazz */, jboolean enabled) {
    Android_BgmSetEnabled(enabled);
}

} // extern "C"

// ============================================================================
// PlayMp3 / StopMp3 — Android implementations of wzAudio BGM API
// These are declared in Winmain.h and called by game scene code.
// WinMain.cpp is excluded from Android build, so we provide replacements.
// ============================================================================

// Music on/off state (matches original WinMain.cpp m_MusicOnOff)
static bool g_musicEnabled = true;
// Currently playing MP3 filename (matches original Mp3FileName)
static char g_currentMp3File[260] = {0};

// PlayMp3 replacement — delegates to Android MediaPlayer via JNI
void PlayMp3(char* Name, BOOL bEnforce) {
    if (!Name || !Name[0]) return;
    if (!g_musicEnabled && !bEnforce) return;

    // Already playing this track — skip
    if (strcmp(Name, g_currentMp3File) == 0) return;

    // Stop any currently playing BGM
    Android_BgmStop();

    strncpy(g_currentMp3File, Name, sizeof(g_currentMp3File) - 1);
    g_currentMp3File[sizeof(g_currentMp3File) - 1] = '\0';

    // Normalize path: backslash→forward, strip "data/" prefix, fix directory case
    char normalizedPath[260];
    strncpy(normalizedPath, Name, sizeof(normalizedPath) - 1);
    normalizedPath[sizeof(normalizedPath) - 1] = '\0';
    for (char* p = normalizedPath; *p; ++p) {
        if (*p == '\\') *p = '/';
    }

    // Strip "data/" or "Data/" prefix (game stores paths relative to Data/ dir)
    const char* stripped = normalizedPath;
    if (strncasecmp(stripped, "data/", 5) == 0) {
        stripped += 5;
    }

    // Fix case: original paths use lowercase "music/", backup uses "Music/"
    char finalPath[260];
    if (strncasecmp(stripped, "music/", 6) == 0) {
        finalPath[0] = 'M';
        finalPath[1] = 'u';
        finalPath[2] = 's';
        finalPath[3] = 'i';
        finalPath[4] = 'c';
        finalPath[5] = '/';
        strncpy(finalPath + 6, stripped + 6, sizeof(finalPath) - 7);
        finalPath[sizeof(finalPath) - 1] = '\0';
    } else {
        strncpy(finalPath, stripped, sizeof(finalPath) - 1);
        finalPath[sizeof(finalPath) - 1] = '\0';
    }

    // Request playback via Android MediaPlayer (tries /sdcard/Data/ first)
    Android_BgmPlay(finalPath);
}

// StopMp3 replacement
void StopMp3(char* Name, BOOL /* bEnforce */) {
    if (Name && Name[0]) {
        // Only stop if the named track is the one currently playing
        if (strcmp(Name, g_currentMp3File) != 0) return;
    }
    g_currentMp3File[0] = '\0';
    Android_BgmStop();
}

// IsEndMp3 — return true if the current BGM finished playing.
// MediaPlayer is set to looping, so this always returns false.
bool IsEndMp3() {
    return false;
}

// GetMp3PlayPosition — return current playback position in some unit.
// Game code checks > 0 to mean "is playing". Return 1 if a track is active.
int GetMp3PlayPosition() {
    return (g_currentMp3File[0] != '\0') ? 1 : 0;
}

// ============================================================================
// Android_RenderTextTexture — called from UIControls.cpp::Render() on render thread
// ============================================================================
void Android_RenderTextTexture(float x, float y, float maxWidth, float maxHeight) {
    static GLuint texId = 0;
    static int    texW = 0;
    static int    texH = 0;

    // Check for new pixel data from Java side
    if (g_textPending) {
        std::lock_guard<std::mutex> lock(g_textMutex);
        g_textPending = false;

        if (texId == 0) {
            glGenTextures(1, &texId);
        }

        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_textWidth, g_textHeight,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, g_textPixels.data());

        texW = g_textWidth;
        texH = g_textHeight;
    }

    if (texId == 0 || texW == 0 || texH == 0) return;

    // Fit the texture into maxWidth x maxHeight while preserving aspect ratio
    float scaleX = maxWidth / (float)texW;
    float scaleY = maxHeight / (float)texH;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;
    float w = texW * scale;
    float h = texH * scale;

    // RenderBitmap: Texture, x, y, Width, Height, u, v, uWidth, vHeight, Scale, StartScale, Alpha
    RenderBitmap((int)texId, x, y, w, h, 0.f, 0.f, 1.f, 1.f, true, true, 0.f);
}
