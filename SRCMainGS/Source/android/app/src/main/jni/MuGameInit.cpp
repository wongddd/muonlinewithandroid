// ============================================================================
// MuGameInit.cpp — Full singleton/global initialization
//
// This file uses the ORIGINAL "stdafx.h" (Main5.2 precompiled header) so it
// has access to all game types. All Windows dependencies are satisfied by
// stubs in the JNI directory (windows.h, gl/gl.h, WinSock2.h, etc.).
//
// This initialization mirrors WinMain.cpp lines 1250-1327 exactly.
// Called from MuGameLoop.cpp:MuGameInit().
// ============================================================================

#include "stdafx.h"
#include "NewUISystem.h"
#include "UIWindows.h"
#include "UIManager.h"
#include "UIMapName.h"
#include "Time/Timer.h"
#include "CAIController.h"
#include "w_BuffStateSystem.h"
#include "w_MapProcess.h"
#include "w_PetProcess.h"
#include "UIMng.h"
#include "TextClien.h"
#include "ZzzInfomation.h"

#include <android/log.h>

#define LOG_TAG "MuGameInit"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Globals defined in MuGlobalState.cpp (WinMain.cpp replacements)
extern CChatRoomSocketList* g_pChatRoomSocketList;
extern CUIManager* g_pUIManager;
extern CUIMapName* g_pUIMapName;
extern CTimer* g_pTimer;

// Globals defined in compiled Main5.2 sources
extern BuffStateSystemPtr g_BuffSystem;
extern MapProcessPtr g_MapProcess;
extern PetProcessPtr g_petProcess;

void MuGameInit_FullInit() {
    LOGI("ENTER MuGameInit_FullInit");

    LOGI("MuGameInit_FullInit: Starting full singleton initialization...");

    LOGI("  Step 1: Gate/Skill arrays");
    GateAttribute = new GATE_ATTRIBUTE[MAX_GATES];
    SkillAttribute = new SKILL_ATTRIBUTE[MAX_SKILLS];
    memset(GateAttribute, 0, sizeof(GATE_ATTRIBUTE) * MAX_GATES);
    memset(SkillAttribute, 0, sizeof(SKILL_ATTRIBUTE) * MAX_SKILLS);
    LOGI("  Step 1 done");

    LOGI("  Step 2: CharacterMachine");
    CharacterMachine = new CHARACTER_MACHINE;
    memset(CharacterMachine, 0, sizeof(CHARACTER_MACHINE));
    CharacterAttribute = &CharacterMachine->Character;
    CharacterMachine->Init();
    LOGI("  Step 2 done");

    LOGI("  Step 3: ChatRoomSocketList");
    g_pChatRoomSocketList = new CChatRoomSocketList;

    LOGI("  Step 4: UI managers");
    g_pUIManager = new CUIManager;
    g_pUIMapName = new CUIMapName;

    LOGI("  Step 5: Timer");
    g_pTimer = new CTimer();

    LOGI("  Step 6: AI Controller");
    static CAIController pAIController(Hero);

    LOGI("  Step 7: Buff system");
    g_BuffSystem = BuffStateSystem::Make();

    LOGI("  Step 8: Map process");
    g_MapProcess = MapProcess::Make();

    LOGI("  Step 9: Pet process (SKIP on ANDROID - hangs on Huawei)");
    // g_petProcess = PetProcess::Make();  // Hangs on Huawei Mate 60 Pro+

    LOGI("  Step 10: CUIMng Create");
    CUIMng::Instance().Create();
    LOGI("  Step 10 done");

    LOGI("  Step 11: TextClien load");
    gTextClien.Load();

    LOGI("MuGameInit_FullInit: Complete (11 subsystems initialized)");

    // 12. NewUI system (SEASON3B) — creates all NewUI windows
    // In WinMain.cpp this is called before CUIMng::CreateLoginScene()
    g_pNewUISystem->Create();
    LOGI("MuGameInit_FullInit: NewUI system created");
}
