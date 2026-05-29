# M5 社交与工具窗口移植日志

## 实际移植文件

原计划文件名为虚构的，实际代码库中的文件名称不同。本次 M5 实际移植了 10 个文件：

| 文件 | 大小 | 说明 |
|------|------|------|
| NewUISlideWindow.cpp | 1.7 KB | 滑动窗口 |
| NewUIWindowMenu.cpp | 6.2 KB | 窗口菜单 |
| NewUIMenuUser.cpp | 6.9 KB | 用户菜单 |
| NewUINameWindow.cpp | 7.0 KB | 名称窗口 / 地图名显示 |
| NewUIQuickCommandWindow.cpp | 7.6 KB | 快速命令窗口 |
| NewUIReconnect.cpp | 10.6 KB | 断线重连 |
| NewUICommandFilter.cpp | 10.5 KB | 命令过滤 |
| NewUICommandWindow.cpp | 21.9 KB | 命令窗口 |
| NewUIChatInputBox.cpp | 29.5 KB | 聊天输入框 |
| NewUIChatLogWindow.cpp | 51.3 KB | 聊天记录窗口 |

注：`NewUIMasterSkillTree.cpp`（技能树）已在 M2/WAVE9 中移植，M5 跳过。

## 时间线

- 开始: 2026-05-24
- 完成: 2026-05-24

## 修改文件清单

### 1. CMakeLists.txt — 添加 WAVE12_NEWUI_M5
```cmake
set(MAIN52_WAVE12_NEWUI_M5
    ${MAIN52_SOURCE_DIR}/NewUISlideWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIWindowMenu.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMenuUser.cpp
    ${MAIN52_SOURCE_DIR}/NewUINameWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIQuickCommandWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIReconnect.cpp
    ${MAIN52_SOURCE_DIR}/NewUICommandFilter.cpp
    ${MAIN52_SOURCE_DIR}/NewUICommandWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIChatInputBox.cpp
    ${MAIN52_SOURCE_DIR}/NewUIChatLogWindow.cpp
)
```
合并到 `MAIN52_SOURCES`。

### 2. MuPlatform.h — 添加 strncat_s 桩函数
原因: `NewUICommandFilter.cpp:442` 使用了 MSVC 特有函数 `strncat_s`。

```cpp
inline int strncat_s(char *dst, size_t dst_size, const char* src, size_t count) {
    if (count == ((size_t)-1)) count = dst_size - strlen(dst) - 1;
    size_t max_copy = dst_size - strlen(dst) - 1;
    if (count > max_copy) count = max_copy;
    strncat(dst, src, count);
    return 0;
}
```

### 3. NewUIStubs.cpp — 移除 7 个已移植类的桩定义
移除以下类的桩 class 定义及其 out-of-line 方法（仅保留 CNewUISystem 中的 getter 前向声明）：
- **CNewUIChatLogWindow** (原行 381-403, 1439-1443): 移除 class + 12 个方法体
- **CNewUIReconnect** (原行 421-436, 1470-1476): 移除 class + 9 个方法体
- **CNewUICommandWindow** (原行 443-453): 移除 class + 2 个方法体
- **CNewUIQuickCommandWindow** (原行 455-465): 移除 class + 2 个方法体
- **CNewUICommandFilter** (原行 196-205, 1346-1347): 移除 class + 2 个方法体
- **CNewUINameWindow** (原行 622-632): 移除 class + 2 个方法体
- **CNewUIChatInputBox** (原行 640-651, 1436-1437): 移除 class + 3 个方法体

### 4. NewUIStubs.cpp — 添加缺失的桩方法
- `CNewUISystem::GetUI_NewHelpWindow()` — 添加 getter 声明和 `return nullptr;` 实现
- `CNewUIMoveCommandWindow::GetMapIndexFromMovereq(const char*)` — 返回 nullptr
- `CNewUIMoveCommandWindow::GetMoveCommandKey()` — 返回 0
- `CNewUIMoveCommandWindow::IsTheMapInDifferentServer(int, int) const` — 返回 false

### 5. NewUIStubs.cpp — g_fScreenRate_x/y 命名空间桥接
原因: M5 文件（NewUINameWindow, NewUICommandFilter, NewUIChatInputBox, NewUIChatLogWindow）在 `namespace SEASON3B { }` 内引用 `g_fScreenRate_x`/`g_fScreenRate_y`，但这两个变量的实际定义在 `UIControls.cpp` 的全局作用域。

解决方案:
- 在文件顶部（全局作用域）添加 `extern float g_fScreenRate_x;` 和 `extern float g_fScreenRate_y;`
- 在第二个 `namespace SEASON3B { }` 内创建引用:
  ```cpp
  float& g_fScreenRate_x = ::g_fScreenRate_x;
  float& g_fScreenRate_y = ::g_fScreenRate_y;
  ```

## 错误修复记录

| 序号 | 错误类型 | 描述 | 解决方案 |
|------|---------|------|----------|
| 1 | 编译错误 | `strncat_s` 未声明 | 在 MuPlatform.h 添加 `strncat_s` inline 实现 |
| 2 | 链接错误 | 28 个重复符号 | 删除 NewUIStubs.cpp 中 7 个 M5 类的桩定义 |
| 3 | 链接错误 | `GetUI_NewHelpWindow()` 未定义 | 添加 CNewUISystem getter 桩 |
| 4 | 链接错误 | `CNewUIMoveCommandWindow` 3 个方法未定义 | 添加 GetMapIndexFromMovereq/GetMoveCommandKey/IsTheMapInDifferentServer 桩 |
| 5 | 链接错误 | `SEASON3B::g_fScreenRate_x` 未定义 | 在 SEASON3B 命名空间内创建指向全局变量的引用 |

## 构建结果

```
[2/2] Linking CXX shared library libmu-client.so
BUILD SUCCESSFUL
```

所有 10 个 M5 文件编译通过，链接成功。
