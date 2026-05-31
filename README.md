# Mu Online Android Client

基于 Mu Online PC 客户端 (Main5.2) 移植的 Android 版本，使用 NDK + Kotlin 构建。

## 架构

```
Kotlin (MuJNI.kt / MuActivity.kt)     ← Android 前端
    └── JNI Bridge (MuBridge.cpp)     ← C++ ↔ Java 交互
        └── Android JNI (34 .cpp)     ← 游戏引擎适配层
            └── PC Engine (~309 .cpp)  ← 原始 Main5.2 游戏代码
```

## 特性

- **完整登录流程**：ConnectServer → 选服 → GS 重连 → 账号登录 → 角色列表 → 进入游戏
- **C3/C4 加密**：SimpleModulus 加密链路，与 PC 客户端完全一致
- **ImGui 中文界面**：自持 CJK 字体 (fonts_ch.ttf)，登录/角色管理/设置面板全中文
- **触摸操控**：虚拟摇杆 (WASD) + 技能按钮面板 + 鼠标模拟 + 双指缩放
- **NewUI 键盘桥接**：触摸 → MuInput → CNewKeyInput → 完整游戏 UI 交互
- **BGM / SFX**：OpenSL ES 音效播放，MediaPlayer 背景音乐
- **资源按需提取**：APK 内置完整游戏资源 (464 MB)，首次运行自动提取
- **崩溃保护**：SIGSEGV/SIGABRT 异步安全处理器
- **多语言框架**：MuStringTable 中/英字符串表
- **设置持久化**：音量/语言/自动重连保存到文件

## 构建

```powershell
cd SRCMainGS\Source\android
.\gradlew assembleDebug            # 完整构建 (~5min)
.\gradlew assembleDebug --offline  # 离线构建 (~30s)
```

APK 输出：`app\build\outputs\apk\debug\app-debug.apk` (≈464 MB)

## 调试

### 本地模拟器
```powershell
emulator -avd Pixel_2
adb wait-for-device
adb install -r app\build\outputs\apk\debug\app-debug.apk
adb shell am start -n com.mu.client/.MuActivity
adb logcat -s ProtocolDispatch:* MuGameLoop:*
```

### 远程真机 (ZeroTier)
```powershell
adb connect <手机ZeroTierIP>:5555
adb install -r app\build\outputs\apk\debug\app-debug.apk
adb logcat -s ProtocolDispatch:* MuGameLoop:*
```

## 修复日志 (Fix Log)

### 2026-05-31：华为 Mate 60 Pro+ 适配

**1. 初始化挂起 (Huawei Mate 60 Pro+)**
- 问题：`PetProcess::Make()` 在麒麟 9000s 上死锁，导致 MuGameInit 挂起
- 修复：`#ifdef ANDROID` 跳过 PetProcess 初始化 (`MuGameInit.cpp`)

**2. 渲染崩溃 (Mali GPU SIGSEGV)**
- 问题：`Scene()/runGameFrame()` 在 Mali GPU 上 SIGSEGV (0x71d12b370000)
- 修复：禁用 `runGameFrame()`，登录界面仅用 ImGui 渲染

**3. jniLibs 覆盖 CMake 输出**
- 问题：`jniLibs/arm64-v8a/libmu-client.so` 旧版覆盖了 CMake 新编译版本
- 修复：删除所有 `jniLibs/*/libmu-client.so`

**4. 登录密钥不匹配**
- 问题：APK 中 `Dec2.dat`/`Enc1.dat` 与服务端不一致
  - APK 错误: `Dec2=5A79CE..` `Enc1=4F5641..`
  - PC 正确: `Dec2=CB3464..` `Enc1=E42A64..`
- 修复：替换为 `jni/Dec2_correct.dat` 和 `jni/Enc1_correct.dat`

**5. 按钮无响应**
- 问题：触摸事件未路由到 ImGui，按钮点击无反应
- 修复：`MuInput::onTouchDown()` 增加 `ImGui::GetIO().WantCaptureMouse` 检测

**6. 中文显示为问号**
- 问题：ImGui 默认字体不含 CJK 字符
- 修复：`MuGameLoop::init()` 加载 `fonts_ch.ttf` (19 MB CJK) 并调用 `GetGlyphRangesChineseFull()`

**7. 角色列表解析错误**
- 问题：entrySize 固定 34 不匹配服务器实际格式，名字带垃圾前缀
- 修复：auto-detect entrySize = bodySize / count，名字 strip 非打印字符

### 2026-05-29：初始调测

**ZeroTier 组网**
- 远端调试通过 ZeroTier VPN 组网，ADB 隧道代理端口
- `adb reverse tcp:44404 tcp:44404` 转发 ConnectServer

**服务器列表 C2 包解析**
- 问题：C2 包标准化后 offset 计算错误 (total=276 误识别)
- 修复：统一使用 offset=4 (C1 标准化格式)

**NewUI 界面常量修正**
- MINI_MAP: 98→72, BUFF_WINDOW: 58→56, HOTKEY: 108→82

## 项目结构

```
android/
├── app/
│   ├── build.gradle.kts          # Gradle 构建配置
│   ├── CMakeLists.txt            # CMake 编译列表 (309+ 源文件)
│   └── src/main/
│       ├── java/com/mu/client/
│       │   ├── MuActivity.kt     # 主 Activity
│       │   ├── MuJNI.kt          # JNI 桥接 + BGM/SFX/下载
│       │   └── MuSurfaceView.kt  # SurfaceView + 触摸
│       ├── jni/
│       │   ├── MuGameLoop.cpp    # 主循环 (1,497 行)
│       │   ├── MuBridge.cpp      # JNI 桥 (583 行)
│       │   ├── MuInput.cpp       # 触摸输入 (726 行)
│       │   ├── MuNetwork.cpp     # TCP 网络 (624 行)
│       │   ├── ProtocolDispatch.cpp # 协议状态机 (1,236 行)
│       │   ├── MuAudio.cpp       # OpenSL ES (625 行)
│       │   ├── MuAssetExtractor.cpp # 资源提取 (303 行)
│       │   └── ... (34 .cpp + 52 .h)
│       ├── assets/
│       │   ├── Data/             # 游戏资源 (1,540 files, 606 MB)
│       │   └── fonts_ch.ttf     # CJK 字体 (19 MB)
│       └── assets_Data_backup/   # 完整数据备份 (27,304 files, 2.4 GB)
└── deploy-data.ps1              # ADB 数据部署脚本
```

## 服务端

| 服务 | 端口 | 协议 |
|------|------|------|
| ConnectServer | 44404 (TCP) | C1/C2 明文 |
| GameServer | 55921 (TCP) | C3/C4 SimpleModulus 加密 |
| JoinServer | 55970 | TCP |
| DataServer | 55960 | TCP + SQL |

## 许可证

基于 Mu Online 私服项目，仅供学习研究使用。
