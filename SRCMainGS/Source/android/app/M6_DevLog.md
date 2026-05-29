# M6 移植开发修复日志

## 概述
M6 是 SEASON3B NewUI 移植的最后一批窗口（56个文件），含所有剩余 NewUI 窗口和收尾打磨。

## 构建配置
- **CMake 变量**: `MAIN52_WAVE13_NEWUI_M6`（56个 .cpp 文件）
- **构建时间**: 约 10 分 39 秒
- **结果**: BUILD SUCCESSFUL，0 错误，24 警告（均为已有警告）

## M6 新增文件（56个）

| 类别 | 文件 |
|------|------|
| 系统核心 | NewUISystem.cpp |
| 物品系统 | NewUIItemTooltip.cpp, NewUIItemScript.cpp, NewUIItemMng.cpp, NewUIItemEnduranceInfo.cpp, NewUIInventoryJewel.cpp, NewUIExchangeLuckyCoin.cpp |
| 消息框 | NewUICustomMessageBox.cpp, NewUICommonMessageBox.cpp |
| 宏系统 | NewUIMacroMain.cpp, NewUIMacroSub.cpp, NewUIMacroGaugeBar.cpp |
| 移动/传送 | NewUIMoveCommandWindow.cpp |
| 攻城战 | NewUICastleWindow.cpp, NewUISiegeWarfare.cpp, NewUIGuardWindow.cpp, NewUICatapultWindow.cpp |
| 事件系统 | NewUIBloodCastle.cpp, NewUIChaosCastleTime.cpp, NewUICursedTempleSystem.cpp, NewUICursedTempleEnter.cpp, NewUICursedTempleResult.cpp, NewUIDevilSquareEnter.cpp, NewUIEnterBloodCastle.cpp, NewUIKanturu2ndEnterNpc.cpp, NewUIKanturuInfoWindow.cpp |
| 幻影寺院 | NewUIDoppelGangerFrame.cpp, NewUIDoppelGangerWindow.cpp |
| 水晶灵殿 | NewUICryWolf.cpp |
| 排名系统 | NewUIRankingTop.cpp, NewUIGensRanking.cpp |
| 背包/仓库 | NewUIStorageInventory.cpp, NewUIStorageExpansion.cpp, NewUIInvenExpansion.cpp, NewUIMyInventory.cpp, NewUIMyShopInventory.cpp, NewUIPurchaseShopInventory.cpp, NewUIMixInventory.cpp, NewUIMixExpansion.cpp |
| 社交/信息 | NewUIGuildInfoWindow.cpp, NewUIPartyListWindow.cpp, NewUIFriendWindow.cpp, NewUICharacterInfoWindow.cpp |
| 任务/NPC | NewUIQuestProgressByEtc.cpp, NewUIQuestProgress.cpp, NewUIMyQuestInfoWindow.cpp, NewUINPCDialogue.cpp, NewUINPCShop.cpp |
| 技能/帮助 | NewUISkillList.cpp, NewUIMasterSkillTree.cpp, NewUIHelpWindow.cpp |
| 其他 | NewUINameWindow.cpp, NewUILuckyItemWnd.cpp, NewUILuckyCoinRegistration.cpp, NewUIEmpireGuardianTimer.cpp, NewUISlideWindow.cpp, NewUICommandWindow.cpp, NewUIQuickCommandWindow.cpp, NewUICommandFilter.cpp, NewUIReconnect.cpp, NewUICamWebzen.cpp, NewUIPurcharseVip.cpp |

## 编译错误修复记录

### 错误1: `_tzset` 未声明（NewUIItemTooltip.cpp:114）
- **根因**: `_tzset` 是 MSVC 特有的时间区域设置函数
- **修复**: 在 `MuPlatform.h` 添加 `#define _tzset tzset`（与已有 `localtime_s` 宏并列）

### 错误2: `__asm { int 3 }` 内联汇编（NewUICommonMessageBox.cpp:1946）
- **根因**: x86 断点中断指令，clang 对 ARM64 不支持
- **修复**: 替换为注释 `// __asm { int 3 } -- removed for Android clang compatibility`

### 错误3: `pointer == false` 比较（NewUICommonMessageBox.cpp:1577,2837）
- **根因**: MSVC 允许 `指针 == false`，clang 视为类型错误
- **修复**: `pMsgBox == false` → `!pMsgBox`

### 错误4: `pointer == false/true` 比较（NewUICustomMessageBox.cpp，约10处）
- **根因**: 同上
- **修复**: `sed -i 's/== false/== NULL/g; s/== true/!= NULL/g'`

### 错误5: `iterator& pair = begin()` 非const左值引用绑定临时对象（NewUIMacroMain.cpp:945-978）
- **根因**: MSVC 允许非const左值引用绑定到临时对象，clang 拒绝
- **修复**: 移除5处迭代器声明中的 `&`（`MAPE_CHECKBOX::iterator pair` 等）

### 错误6: 150+ 重复符号链接错误
- **根因**: NewUIStubs.cpp 保留了 M6 已编译类的完整桩定义
- **修复**: 大幅裁剪 NewUIStubs.cpp 两个 SEASON3B 命名空间块，只保留：
  - 前向声明（所有 NewUI 类）
  - CNewUIInGameShop 完整桩（该类不在任何 NewUI*.cpp 中）
  - CNewUIMessageBoxBase/Mng/Manager/KeyInput 轻量桩
  - 全局变量桩（g_pUIJewelHarmonyinfo, g_iLengthAuthorityCode, g_iCancelSkillTarget）
  - CreateOkMessageBox / CreateOkCancelMessageBox 桩函数

### 错误7: 全局作用域 CNewUIItemTooltip 重复符号
- **根因**: 全局作用域的桩定义与 M6 的 NewUIItemTooltip.cpp 冲突
- **修复**: 移除全局作用域 CNewUIItemTooltip 类定义

### 错误8: `leaf` 命名空间未声明（NewUIStubs.cpp CreateOkCancelMessageBox签名）
- **根因**: `EVENT_CALLBACK` 参数类型引用了 `const leaf::xstreambuf&`，需要前向声明
- **修复**: 在 NewUIStubs.cpp 顶部添加 `namespace leaf { class xstreambuf; }`

### 错误9: 缺少3个全局变量定义
- **根因**: `g_pUIJewelHarmonyinfo`, `g_iLengthAuthorityCode`, `g_iCancelSkillTarget` 被 M6 代码引用但未在任何文件中定义
- **修复**: 在 NewUIStubs.cpp 中添加桩定义

## NewUIStubs.cpp 最终状态

文件从 ~2246 行精简为 ~160 行，只保留：
- **前向声明**：所有 NewUI 类的 `class CNewUIXxx;` 声明
- **CNewUIInGameShop**：完整桩构造函数 + 14个方法（不在任何 NewUI*.cpp 中）
- **全局变量桩**：SelectedCharacter, g_pUIJewelHarmonyinfo, g_iLengthAuthorityCode, g_iCancelSkillTarget
- **消息框桩函数**：CreateOkMessageBox, CreateOkCancelMessageBox
- **轻量消息框类桩**：CNewUIMessageBoxBase, CNewUIMessageBoxMng, CNewUIManager, CNewKeyInput

## 已知警告（24个，均为已有）

| 警告类型 | 文件 | 说明 |
|---------|------|------|
| `-Wnon-c-typedef-for-linkage` | WSclient.h (6处) | 匿名结构体含成员函数，非C兼容 |
| `-Wnull-conversion` | WSclient.h (2处) | NULL 隐式转换为 int 给 memset |
| `-Wnull-arithmetic` | wsclientinline.h (1处) | strcmp 结果与 NULL 比较 |
| `-Wwritable-strings` | wsclientinline.h (6处) | 字符串字面量赋值给 char* |
| `-Wreturn-type` | wsclientinline.h (2处) | 非void函数无返回值 |
| `-Wmissing-declarations` | ZzzInventory.h (3处) | const enum 声明 |
