# CLAUDE.md

此文件为 Claude Code (claude.ai/code) 在此仓库中工作时提供指导。

## 交流规则

用中文回复。

## 项目概述

Mu Online 私服 —— 一个分布式的 C++ 游戏服务端模拟器（含客户端）。代码库通过编译期预处理宏支持多个游戏版本（Season 4 至 Season 8）。

## 构建系统

Visual Studio 解决方案，仅限 x86 (Win32)。每个服务器有独立的 `.sln` 文件，没有统一解决方案。

| 项目 | 解决方案 | 工具集 | 说明 |
|---------|----------|---------|-------------|
| GameServer | `GameServer\GameServer.sln` | v100/v142 | 主游戏逻辑 |
| ConnectServer | `ConnectServer\ConnectServer.sln` | v142 | 连接网关 |
| DataServer | `DataServer\DataServer.sln` | v142 | 数据库/角色操作 |
| JoinServer | `JoinServer\JoinServer.sln` | v142 | 登录验证服务 |
| Encoder | `Encoder\Encoder.sln` | v142 | 客户端封包工具 |
| Main (客户端) | `Main5.2\Main.sln` | v142 | 游戏客户端 (OpenGL/ImGui) |

每个 GameServer `.vcxproj` 通过三个预处理宏区分构建配置：
- `GAMESERVER_TYPE=0`（完整版）或 `1`（精简版）
- `GAMESERVER_UPDATE` — 协议版本：`401`、`603`、`803`
- `GAMESERVER_LANGUAGE` — 包加密变体 (0–7)
- `GAMESERVER_LICENSE` — 0 或 1 用于授权版本

常用配置：`Release_EX603`、`Release_EX803`、`Release_EX803CS`、`Release_EX401` 等。DataServer 有对应的 `DATASERVER_UPDATE` 宏。

命令行构建示例：
```
msbuild GameServer\GameServer.sln /p:Configuration="Release_EX803" /p:Platform=Win32
```

## 架构

### 网络拓扑

```
Client → ConnectServer (TCP/UDP, 服务器列表)
           ↓
Client → JoinServer (TCP, 登录/验证, 与ConnectServer通过共享内存通信)
           ↓
Client → GameServer (TCP, 基于IOCP, 处理所有游戏玩法)
           ↓
         DataServer (TCP, 持久化角色/公会/物品数据)
```

- **JoinServer**：简单 TCP，使用 `CConnection` 配合 Windows 消息泵 (`WM_JOIN_SERVER_MSG_PROC`)。处理账号登录、角色列表、服务器重定向。
- **DataServer**：TCP 后端，负责持久化数据 — 角色存储/加载、公会管理、仓库。使用 `CSocketManager`（IOCP）处理 GameServer 连接，`QueryManager` 处理 SQL。
- **GameServer**：最大的服务器。IOCP Socket 管理器 (`CSocketManager`) 处理客户端连接，普通 TCP 连接 (`CConnection`) 连接 JoinServer 和 DataServer。游戏逻辑运行在 Win32 窗口线程上，使用定时器回调。
- **ConnectServer**：客户端与 JoinServer/GameServer 之间的轻量级中继，管理服务器列表。

### GameServer 关键类

- `CServerInfo` (`ServerInfo.h`) — 巨型配置对象，读取所有 `.dat` 文件。通过 `gServerInfo` 访问单例。
- `OBJECTSTRUCT` / `LPOBJ` (`User.h`) — 玩家/怪物/NPC 对象。定长数组：`MAX_OBJECT_MONSTER` + `MAX_OBJECT_USER` + `MAX_OBJECT_BOTS`。通过全局 `gObj[]` 数组访问。
- `CUser` (`User.h`) — 封装 `OBJECTSTRUCT` 的游戏玩法方法。
- `CSocketManager` (`SocketManager.h`) — IOCP 网络管理器（8个工作线程、接受线程、队列线程）。
- `CConnection` (`Connection.h`) — 服务器间简单同步式 TCP 连接。
- `CMemScript` — 读取 `.dat` 配置文件（自定义类 INI 解析器）。

### 协议

所有服务器间使用二进制包协议：
- `PBMSG_HEAD`：`{type, size, head}` — type 为 `0xC1` 或 `0xC3`
- `PSBMSG_HEAD`：额外包含 `subh` 子头
- 包结构定义在 `Protocol.h`（客户端↔GS）、`JSProtocol.h`（Join↔GS）、`DataServerProtocol.h`（DS↔GS）

协议码 (`PROTOCOL_CODE1`–`4`) 根据 `GAMESERVER_LANGUAGE` 变化 — 构成包加密密钥。

### 共享代码模式

每个服务器项目重复复制了基础文件（`Util.h/cpp`、`MemScript.h/cpp`、`Log.h/cpp`、`CriticalSection.h/cpp`、`Queue.h/cpp`、`MiniDump.h/cpp`、`Protect.h/cpp`、`SocketManager.h/cpp`）。这些文件**不是通过库共享的** — 在一个项目中修改需要手动同步到其他项目。

### 客户端 (Main5.2)

客户端使用的依赖：
- OpenGL (GLFW/glad) 渲染
- ImGui 游戏内界面
- Lua 脚本 (Sol3 绑定)
- CryptoPP 和自定义 MuCrypto 包加密
- Freetype 字体渲染
- TurboJPEG 图片加载

关键头文件：`_define.h`、`_enum.h`、`_struct.h`、`_types.h` 定义核心类型和常量。`Protocol.h` 镜像服务端包定义。

### Lua 脚本

GameServer 包含 `lua/` 目录用于 Lua 脚本支持。

## 配置文件

服务器配置存放在 `Data\` 目录下的 `.dat` 文件中（由 `MemScript` 解析）。关键文件：
- `GameServerInfo - Common.dat` — GameServer 主配置（倍率、端口、地址、活动开关）
- 各类 `.txt`/`.dat` 文件用于物品、怪物、地图、技能等

## 输出文件

构建输出到版本对应的目录：
- `GameServer\GameServer\Release\GameServer_EX803\`
- `GameServer\GameServer\Release\GameServer_EX603CS\`

部署输出路径在各 `.vcxproj` 的 `OutDir` 中定义。

## Android 构建与部署

Android 客户端位于 `android/` 目录，通过 Gradle + CMake + NDK 交叉编译。

### 环境要求

| 组件 | 版本/路径 | 说明 |
|------|-----------|------|
| Android SDK | `D:\android-sdk-windows` | `sdk.dir` 在 `local.properties` |
| Android NDK | `25.1.8937393` | CMake 3.22.1, ninja |
| JDK | 17 | `compileOptions` 指定 |
| Kotlin | 1.9.22 | |
| Gradle | wrapper (8.5) | `./gradlew` |

### 构建命令

```powershell
cd android

# 完整 Debug 构建（首次编译 ~20 分钟，增量 ~2-5 分钟）
.\gradlew assembleDebug

# 跳过 CMake 原生编译（使用预编译的 jniLibs/*.so 直接打包，~20 秒）
# 适用场景：只改了 Java/Kotlin 代码或 APK assets，.so 未变
.\gradlew assembleDebug -x externalNativeBuildDebug

# 清理 CMake 缓存后重建（解决 "ninja: premature end of file" 等缓存损坏问题）
Remove-Item -Recurse -Force app\.cxx
.\gradlew assembleDebug
```

### 目标 ABI

`build.gradle.kts` 中 `abiFilters` 配置了四种 ABI：

| ABI | 适用设备 | .so 大小 |
|-----|---------|---------|
| `arm64-v8a` | 现代真机（99% 设备） | ~89 MB |
| `armeabi-v7a` | 旧设备 | ~72 MB |
| `x86` | 模拟器 | ~75 MB |
| `x86_64` | 64 位模拟器 | ~86 MB |

### CMake 缓存问题

CMake 构建缓存位于 `app/.cxx/`。偶尔出现以下问题：
- `ninja: premature end of file` — 缓存损坏，删除 `.cxx/` 目录重试
- `CMAKE_CXX_COMPILER not set` — x86_64 ABI 在首次 configure 时偶发失败，删除对应 ABI 缓存重试
- `turbojpeg/libturbojpeg.a not found` — 同上，缓存损坏

**应急方案**：`jniLibs/` 目录存有预编译的 `libmu-client.so` 和 `libc++_shared.so`（四个 ABI）。遇到 CMake 构建失败可用 `assembleDebug -x externalNativeBuildDebug` 直接打包。

### 部署到设备

**1. 安装 APK**
```powershell
adb install -r app\build\outputs\apk\debug\app-debug.apk
```

**2. 部署游戏数据**（必须，~2.4 GB 资源不在 APK 内）
```powershell
.\deploy-data.ps1                  # 完整部署到 /sdcard/Data/
.\deploy-data.ps1 -Minimal          # 仅关键文件（加密密钥 + 本地化 + 音乐）
.\deploy-data.ps1 -AdbArgs "-s emulator-5554"  # 指定设备
```

**3. 授权存储权限**（API 29+ 必须，即使 Manifest 已声明）
```powershell
adb shell pm grant com.mu.client android.permission.READ_EXTERNAL_STORAGE
adb shell pm grant com.mu.client android.permission.WRITE_EXTERNAL_STORAGE
```

### 游戏数据路径映射

设备端数据目录：`/sdcard/Data/`

| 游戏内路径 | 设备实际路径 | 来源 |
|-----------|-------------|------|
| `Data/Dec2.dat` | `/sdcard/Data/Dec2.dat` | APK assets 或 adb push |
| `Data/Enc1.dat` | `/sdcard/Data/Enc1.dat` | APK assets 或 adb push |
| `Data/Local/Connect.msil` | `/sdcard/Data/Local/Connect.msil` | APK assets 自动提取 |
| `data\music\Pub.mp3` | `/sdcard/Data/Music/Pub.mp3` | adb push（BGM 走 MediaPlayer 文件路径） |
| `World1/...` | `/sdcard/Data/World1/...` | adb push |

**BGM 路径转换链**：
1. 游戏代码：`data\music\Pub.mp3`（Windows 风格，带 `data\` 前缀，小写 `music`）
2. `PlayMp3`（MuBridge.cpp）：去掉 `data/` 前缀，`music/` → `Music/`（匹配磁盘大小写）
3. `bgmPlay`（MuJNI.kt）：尝试 `/sdcard/Data/Music/Pub.mp3` 文件路径 → 成功则播放

### APK 内嵌资源

`assets/Data/` 目录打包进 APK，首次访问时由 `MuAssetExtractor` 自动提取到 `/sdcard/Data/`：

| 文件 | 大小 | 用途 |
|------|------|------|
| `Data/Dec2.dat` | 54 B | SimpleModulus 解密密钥 |
| `Data/Enc1.dat` | 54 B | SimpleModulus 加密密钥 |
| `Data/Local/Connect.msil` | 178 B | EncDecKey 计算 |

### 模拟器测试

```powershell
# 列出可用 AVD
emulator -list-avds

# 启动模拟器（Pixel_4 是 API 29 x86）
emulator -avd Pixel_4 -no-boot-anim &

# 等待上线
adb wait-for-device

# 构建、安装、部署
.\gradlew assembleDebug
adb install -r app\build\outputs\apk\debug\app-debug.apk
.\deploy-data.ps1 -Minimal
adb shell pm grant com.mu.client android.permission.READ_EXTERNAL_STORAGE
adb shell pm grant com.mu.client android.permission.WRITE_EXTERNAL_STORAGE
adb shell am start -n com.mu.client/.MuActivity

# 查看日志
adb logcat -s MuGameLoop:* MuBGM:* MuNetwork:* MuAssetExtractor:*
```

### 工作流程规则

- **每次任务完成后，必须给出下一步工作计划**（列出 1-3 个推荐方向，标记优先级）。
- **按用户要求顺序执行**：用户可能选择全部或部分推荐项。

### 已知限制

- **无 GameServer**：模拟器内 10.0.2.2:55921 无服务端运行，登录会连接失败（网络包加解密链路已验证通过）
- **存储权限**：Android 10+ 分区存储下，`pm grant` 需每次重装后执行；生产环境应改用 SAF 或应用私有目录
- **部分音乐文件名大小写不匹配**：`data\music\Mutheme.mp3` → 磁盘上是 `MuTheme.mp3`，Linux 区分大小写导致该曲目无法播放（其他 70 首正常）
