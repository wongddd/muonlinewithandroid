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
    LOGI("MuGameInit_FullInit: Starting full singleton initialization...");

    // 1. Allocate attribute arrays (must exist before CharacterMachine::Init)
    GateAttribute = new GATE_ATTRIBUTE[MAX_GATES];
    SkillAttribute = new SKILL_ATTRIBUTE[MAX_SKILLS];
    memset(GateAttribute, 0, sizeof(GATE_ATTRIBUTE) * MAX_GATES);
    memset(SkillAttribute, 0, sizeof(SKILL_ATTRIBUTE) * MAX_SKILLS);

    // 2. CharacterMachine — core character state
    CharacterMachine = new CHARACTER_MACHINE;
    memset(CharacterMachine, 0, sizeof(CHARACTER_MACHINE));
    CharacterAttribute = &CharacterMachine->Character;
    CharacterMachine->Init();

    // 3. Chat room socket list
    g_pChatRoomSocketList = new CChatRoomSocketList;

    // 4. UI managers
    g_pUIManager = new CUIManager;
    g_pUIMapName = new CUIMapName;

    // 5. Timer (frame timing)
    g_pTimer = new CTimer();

    // 6. AI controller (static, linked to Hero)
    static CAIController pAIController(Hero);

    // 7. Buff system (skill effects, buffs, debuffs)
    g_BuffSystem = BuffStateSystem::Make();

    // 8. Map process (terrain management, world loading)
    g_MapProcess = MapProcess::Make();

    // 9. Pet process (pet AI, rendering)
    g_petProcess = PetProcess::Make();

    // 10. UI manager singleton (CNewUIManager via CUIMng wrapper)
    CUIMng::Instance().Create();

    // 11. Text client (localized text, menu strings)
    gTextClien.Load();

    LOGI("MuGameInit_FullInit: Complete (11 subsystems initialized)");

    // 12. NewUI system (SEASON3B) — creates all NewUI windows
    // In WinMain.cpp this is called before CUIMng::CreateLoginScene()
    g_pNewUISystem->Create();
    LOGI("MuGameInit_FullInit: NewUI system created");
}
