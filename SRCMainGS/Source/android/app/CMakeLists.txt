cmake_minimum_required(VERSION 3.22)
project("mu-client")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(
    -DANDROID
    -DMAIN_UPDATE=603
    -DPROTO_EXTRA
    -DSHADER_VERSION_TEST
    -DLANG_CHN=1
)

add_compile_options(-Wno-unknown-pragmas -Wno-nonportable-include-path -Wno-format-security -Wno-c++11-narrowing)

find_library(android-lib android)
find_library(egl-lib EGL)
find_library(glesv3-lib GLESv3)
find_library(opensles-lib OpenSLES)
find_library(log-lib log)

# ImGui core sources (from Main5.2/dependencies/ImGui)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/../../Main5.2/dependencies/ImGui)

# GLM headers (from Main5.2/dependencies/include)
set(GLM_DIR ${CMAKE_SOURCE_DIR}/../../Main5.2/dependencies/include)

# Main5.2 source directory (for porting original code)
set(MAIN52_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../Main5.2/source)

# Dependency library sources (for Android NDK static builds)
set(DEP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../Main5.2/dependencies/source)

# Disable FreeType optional dependencies (not available on Android without extra work)
set(FT_DISABLE_ZLIB TRUE CACHE BOOL "")
set(FT_DISABLE_BZIP2 TRUE CACHE BOOL "")
set(FT_DISABLE_PNG TRUE CACHE BOOL "")
set(FT_DISABLE_HARFBUZZ TRUE CACHE BOOL "")
set(FT_DISABLE_BROTLI TRUE CACHE BOOL "")

# Lua 5.4.2 static library
add_subdirectory(${DEP_SOURCE_DIR}/lua-5.4.2 ${CMAKE_BINARY_DIR}/lua54)

# FreeType 2.13.3 static library
add_subdirectory(${DEP_SOURCE_DIR}/freetype-2.13.3 ${CMAKE_BINARY_DIR}/freetype)

# Crypto++ 8.2 static library
add_subdirectory(${DEP_SOURCE_DIR}/cryptopp-CRYPTOPP_8_2_0 ${CMAKE_BINARY_DIR}/cryptopp)

# libjpeg-turbo 3.1.0 static library (wrapper CMake for Android)
add_subdirectory(${DEP_SOURCE_DIR}/turbojpeg-android ${CMAKE_BINARY_DIR}/turbojpeg)

# ============================================================================
# Main5.2 portable .cpp sources — added incrementally
# Excluded: WINHANDLE, DSound, wsctlc, Winmain, StdAfx, Leaf/, GameShop/,
#           NewUI*, GMFerea, BlackWin, Win/WinEx, ProtectSysKey,
#           imgui_impl_win32, imgui_impl_opengl2
# ============================================================================

# -- Wave 1: Type system (header-only, verified in MuWave1Test) --

# -- Wave 2: BMD + Texture + ModelManager --
set(MAIN52_WAVE2
    ${MAIN52_SOURCE_DIR}/ZzzBMD.cpp
    ${MAIN52_SOURCE_DIR}/ZzzTexture.cpp
    ${MAIN52_SOURCE_DIR}/TextureScript.cpp
    ${MAIN52_SOURCE_DIR}/GlobalBitmap.cpp
    ${MAIN52_SOURCE_DIR}/CGMModelManager.cpp
    ${MAIN52_SOURCE_DIR}/CGMRenderGroupMesh.cpp
    ${MAIN52_SOURCE_DIR}/CGMScriptLua.cpp
    ${MAIN52_SOURCE_DIR}/BoneManager.cpp
)

# -- Wave 3: Terrain + Map --
set(MAIN52_WAVE3
    ${MAIN52_SOURCE_DIR}/ZzzLodTerrain.cpp
    ${MAIN52_SOURCE_DIR}/MapManager.cpp
)

# -- Wave 4: Object + Character + Camera --
set(MAIN52_WAVE4
    ${MAIN52_SOURCE_DIR}/ZzzObject.cpp
    ${MAIN52_SOURCE_DIR}/ZzzCharacter.cpp
    ${MAIN52_SOURCE_DIR}/CharacterManager.cpp
    ${MAIN52_SOURCE_DIR}/ZzzOpenglUtil.cpp
)

# -- Wave 5: Scene + Effects + AI --
set(MAIN52_WAVE5
    ${MAIN52_SOURCE_DIR}/ZzzScene.cpp
    ${MAIN52_SOURCE_DIR}/ZzzEffect.cpp
    ${MAIN52_SOURCE_DIR}/ZzzEffectBlurSpark.cpp
    ${MAIN52_SOURCE_DIR}/ZzzEffectFireLeave.cpp
    ${MAIN52_SOURCE_DIR}/ZzzEffectJoint.cpp
    ${MAIN52_SOURCE_DIR}/ZzzEffectMagicSkill.cpp
    ${MAIN52_SOURCE_DIR}/ZzzEffectNoUse.cpp
    ${MAIN52_SOURCE_DIR}/ZzzEffectParticle.cpp
    ${MAIN52_SOURCE_DIR}/ZzzEffectPoint.cpp
    ${MAIN52_SOURCE_DIR}/ZzzEffectPointer.cpp
    ${MAIN52_SOURCE_DIR}/zzzeffectsprite.cpp
    ${MAIN52_SOURCE_DIR}/ZzzAI.cpp
    ${MAIN52_SOURCE_DIR}/ZzzInterface.cpp
    ${MAIN52_SOURCE_DIR}/ZzzInventory.cpp
    ${MAIN52_SOURCE_DIR}/ZzzInfomation.cpp
    ${MAIN52_SOURCE_DIR}/ZzzOpenData.cpp
    ${MAIN52_SOURCE_DIR}/zzzpath.cpp
    ${MAIN52_SOURCE_DIR}/zzzMixInvetory.cpp
    ${MAIN52_SOURCE_DIR}/CGMItemEffect.cpp
    ${MAIN52_SOURCE_DIR}/CGMItemMng.cpp
    ${MAIN52_SOURCE_DIR}/CGMItemDropName.cpp
    ${MAIN52_SOURCE_DIR}/CGMItemWings.cpp
    ${MAIN52_SOURCE_DIR}/CGMMonsterMng.cpp
    ${MAIN52_SOURCE_DIR}/CGMInvasionManager.cpp
    ${MAIN52_SOURCE_DIR}/CGMJewelOfAction.cpp
    ${MAIN52_SOURCE_DIR}/CGMResetManager.cpp
    ${MAIN52_SOURCE_DIR}/CGMEffectHandle.cpp
    ${MAIN52_SOURCE_DIR}/SkillEffectMgr.cpp
    ${MAIN52_SOURCE_DIR}/CGMPhysicsManager.cpp
    ${MAIN52_SOURCE_DIR}/PhysicsManager.cpp
    ${MAIN52_SOURCE_DIR}/ShadowVolume.cpp
    ${MAIN52_SOURCE_DIR}/Sprite.cpp
    ${MAIN52_SOURCE_DIR}/CShaderGL.cpp
    ${MAIN52_SOURCE_DIR}/CGMFrame.cpp
    ${MAIN52_SOURCE_DIR}/CGMFontLayer.cpp
    ${MAIN52_SOURCE_DIR}/CGMHeadChat.cpp
)

# -- Wave 6: Data + Logic (no rendering dependency) --
set(MAIN52_WAVE6
    ${MAIN52_SOURCE_DIR}/_GlobalFunctions.cpp
    ${MAIN52_SOURCE_DIR}/Util.cpp
    ${MAIN52_SOURCE_DIR}/UsefulDef.cpp
    ${MAIN52_SOURCE_DIR}/MultiLanguage.cpp
    ${MAIN52_SOURCE_DIR}/Protocol.cpp
    ${MAIN52_SOURCE_DIR}/ProtocolSend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/CGMProtectStub.cpp
    ${MAIN52_SOURCE_DIR}/MuCrypto/MuCrypto.cpp
    ${MAIN52_SOURCE_DIR}/pugixml.cpp
    # ConnectVersionHex.cpp is Win32-only (CreateProcess, registry, etc.)
    # ${MAIN52_SOURCE_DIR}/ConnectVersionHex.cpp
    ${MAIN52_SOURCE_DIR}/KeyGenerater.cpp
    ${MAIN52_SOURCE_DIR}/steady_clock.cpp
    ${MAIN52_SOURCE_DIR}/ItemInfo.cpp
    ${MAIN52_SOURCE_DIR}/ItemAddOptioninfo.cpp
    ${MAIN52_SOURCE_DIR}/SkillInfo.cpp
    ${MAIN52_SOURCE_DIR}/MoveCommandData.cpp
    ${MAIN52_SOURCE_DIR}/MovereqInfo.cpp
    ${MAIN52_SOURCE_DIR}/FilterInfo.cpp
    ${MAIN52_SOURCE_DIR}/FilterNameInfo.cpp
    ${MAIN52_SOURCE_DIR}/w_Buff.cpp
    ${MAIN52_SOURCE_DIR}/w_BuffScriptLoader.cpp
    ${MAIN52_SOURCE_DIR}/w_BuffStateSystem.cpp
    ${MAIN52_SOURCE_DIR}/w_BuffStateValueControl.cpp
    ${MAIN52_SOURCE_DIR}/w_BuffTimeControl.cpp
    ${MAIN52_SOURCE_DIR}/w_CharacterInfo.cpp
    ${MAIN52_SOURCE_DIR}/w_ObjectInfo.cpp
    ${MAIN52_SOURCE_DIR}/w_MapProcess.cpp
    ${MAIN52_SOURCE_DIR}/w_BasePet.cpp
    ${MAIN52_SOURCE_DIR}/w_PetActionCollecter.cpp
    ${MAIN52_SOURCE_DIR}/w_PetActionCollecter_Add.cpp
    ${MAIN52_SOURCE_DIR}/w_PetActionDemon.cpp
    ${MAIN52_SOURCE_DIR}/w_PetActionMuun.cpp
    ${MAIN52_SOURCE_DIR}/w_PetActionRound.cpp
    ${MAIN52_SOURCE_DIR}/w_PetActionStand.cpp
    ${MAIN52_SOURCE_DIR}/w_PetActionUnicorn.cpp
    ${MAIN52_SOURCE_DIR}/w_PetProcess.cpp
    ${MAIN52_SOURCE_DIR}/w_CursedTemple.cpp
    ${MAIN52_SOURCE_DIR}/GambleSystem.cpp
    ${MAIN52_SOURCE_DIR}/GameCensorship.cpp
    ${MAIN52_SOURCE_DIR}/ChangeRingManager.cpp
    ${MAIN52_SOURCE_DIR}/GuildCache.cpp
    ${MAIN52_SOURCE_DIR}/GuildManager.cpp
    ${MAIN52_SOURCE_DIR}/PartyManager.cpp
    ${MAIN52_SOURCE_DIR}/DuelMgr.cpp
    ${MAIN52_SOURCE_DIR}/MatchEvent.cpp
    ${MAIN52_SOURCE_DIR}/Event.cpp
    ${MAIN52_SOURCE_DIR}/SkillManager.cpp
    ${MAIN52_SOURCE_DIR}/MonkSystem.cpp
    ${MAIN52_SOURCE_DIR}/SummonSystem.cpp
    ${MAIN52_SOURCE_DIR}/GIPetManager.cpp
    ${MAIN52_SOURCE_DIR}/ItemManager.cpp
    ${MAIN52_SOURCE_DIR}/GOBoid.cpp
    ${MAIN52_SOURCE_DIR}/SMD.cpp
    ${MAIN52_SOURCE_DIR}/SMD2BMD.cpp
    ${MAIN52_SOURCE_DIR}/SideHair.cpp
    ${MAIN52_SOURCE_DIR}/ScriptItem.cpp
    ${MAIN52_SOURCE_DIR}/Builder.cpp
    ${MAIN52_SOURCE_DIR}/CAIController.cpp
    ${MAIN52_SOURCE_DIR}/CameraManager.cpp
    ${MAIN52_SOURCE_DIR}/CameraMove.cpp
    ${MAIN52_SOURCE_DIR}/CComGem.cpp
    ${MAIN52_SOURCE_DIR}/CDirection.cpp
    ${MAIN52_SOURCE_DIR}/CGMCameraWorld.cpp
    ${MAIN52_SOURCE_DIR}/CGMCharacter.cpp
    ${MAIN52_SOURCE_DIR}/CMVP1stDirection.cpp
    ${MAIN52_SOURCE_DIR}/CKANTURUDirection.cpp
    ${MAIN52_SOURCE_DIR}/CSChaosCastle.cpp
    ${MAIN52_SOURCE_DIR}/CSEventMatch.cpp
    ${MAIN52_SOURCE_DIR}/CSItemOption.cpp
    ${MAIN52_SOURCE_DIR}/CSMapServer.cpp
    ${MAIN52_SOURCE_DIR}/CSParts.cpp
    ${MAIN52_SOURCE_DIR}/CSPetSystem.cpp
    ${MAIN52_SOURCE_DIR}/CSQuest.cpp
    ${MAIN52_SOURCE_DIR}/CSWaterTerrain.cpp
    ${MAIN52_SOURCE_DIR}/NewBloodCastleSystem.cpp
    ${MAIN52_SOURCE_DIR}/NewChaosCastleSystem.cpp
    ${MAIN52_SOURCE_DIR}/QuestInfo.cpp
    ${MAIN52_SOURCE_DIR}/QuestMng.cpp
    ${MAIN52_SOURCE_DIR}/MixMgr.cpp
    ${MAIN52_SOURCE_DIR}/PathFinding.cpp
    ${MAIN52_SOURCE_DIR}/PortalMgr.cpp
    ${MAIN52_SOURCE_DIR}/ServerGroup.cpp
    ${MAIN52_SOURCE_DIR}/ServerInfo.cpp
    ${MAIN52_SOURCE_DIR}/ServerListManager.cpp
    ${MAIN52_SOURCE_DIR}/supportingfeature.cpp
    ${MAIN52_SOURCE_DIR}/PersonalShopTitleImp.cpp
    ${MAIN52_SOURCE_DIR}/jpexs.cpp
    ${MAIN52_SOURCE_DIR}/Resolutions.cpp
    ${MAIN52_SOURCE_DIR}/ScaleForm.cpp
    ${MAIN52_SOURCE_DIR}/Local.cpp
    ${MAIN52_SOURCE_DIR}/TextClien.cpp
    ${MAIN52_SOURCE_DIR}/SocketSystem.cpp
    ${MAIN52_SOURCE_DIR}/Math/ZzzMathLib.cpp
    ${MAIN52_SOURCE_DIR}/Time/CTimCheck.cpp
    ${MAIN52_SOURCE_DIR}/Time/Timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/WINHANDLEStub.cpp
    # JpegStubs.cpp replaced by real libjpeg-turbo library
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/NewUIStubs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/MuFreeTypeRender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jni/ErrorReportStub.cpp
    ${MAIN52_SOURCE_DIR}/Utilities/Log/muConsoleDebug.cpp
    ${MAIN52_SOURCE_DIR}/Utilities/Memory/MemoryLock.cpp
)

# -- Game scene files (GM*.cpp) - portable scene logic --
set(MAIN52_WAVE7
    ${MAIN52_SOURCE_DIR}/GMCharacterScene.cpp
    ${MAIN52_SOURCE_DIR}/GMLogInScene.cpp
    # -- P2-6: Remaining PC module ports --
    ${MAIN52_SOURCE_DIR}/GMFerea.cpp        # Scene map handler (Ferea map)
    ${MAIN52_SOURCE_DIR}/LoadingScene.cpp
    ${MAIN52_SOURCE_DIR}/MovieScene.cpp
    ${MAIN52_SOURCE_DIR}/GM3rdChangeUp.cpp
    ${MAIN52_SOURCE_DIR}/GMAida.cpp
    ${MAIN52_SOURCE_DIR}/GMBattleCastle.cpp
    ${MAIN52_SOURCE_DIR}/GMCryingWolf2nd.cpp
    ${MAIN52_SOURCE_DIR}/GMCrywolf1st.cpp
    ${MAIN52_SOURCE_DIR}/GMDoppelGanger1.cpp
    ${MAIN52_SOURCE_DIR}/GMDoppelGanger2.cpp
    ${MAIN52_SOURCE_DIR}/GMDoppelGanger3.cpp
    ${MAIN52_SOURCE_DIR}/GMDoppelGanger4.cpp
    ${MAIN52_SOURCE_DIR}/GMDuelArena.cpp
    ${MAIN52_SOURCE_DIR}/GMEmpireGuardian1.cpp
    ${MAIN52_SOURCE_DIR}/GMEmpireGuardian2.cpp
    ${MAIN52_SOURCE_DIR}/GMEmpireGuardian3.cpp
    ${MAIN52_SOURCE_DIR}/GMEmpireGuardian4.cpp
    ${MAIN52_SOURCE_DIR}/GMHellas.cpp
    ${MAIN52_SOURCE_DIR}/GMHuntingGround.cpp
    ${MAIN52_SOURCE_DIR}/GMKarutan1.cpp
    ${MAIN52_SOURCE_DIR}/GMNewTown.cpp
    ${MAIN52_SOURCE_DIR}/GMSantaTown.cpp
    ${MAIN52_SOURCE_DIR}/GMSwampOfQuiet.cpp
    ${MAIN52_SOURCE_DIR}/GMUnitedMarketPlace.cpp
    ${MAIN52_SOURCE_DIR}/GM_kanturu_1st.cpp
    ${MAIN52_SOURCE_DIR}/GM_Kanturu_2nd.cpp
    ${MAIN52_SOURCE_DIR}/GM_Kanturu_3rd.cpp
    ${MAIN52_SOURCE_DIR}/GM_PK_Field.cpp
    ${MAIN52_SOURCE_DIR}/GM_Raklion.cpp
    ${MAIN52_SOURCE_DIR}/GMAcheron.cpp
    ${MAIN52_SOURCE_DIR}/GMDeventer.cpp
    ${MAIN52_SOURCE_DIR}/GMZoneNars.cpp
    ${MAIN52_SOURCE_DIR}/GMUrukMountain.cpp
    ${MAIN52_SOURCE_DIR}/GMNixiesLake.cpp
    ${MAIN52_SOURCE_DIR}/TestEventMap.cpp
    ${MAIN52_SOURCE_DIR}/GaugeBar.cpp
    ${MAIN52_SOURCE_DIR}/CBInterface.cpp
    ${MAIN52_SOURCE_DIR}/CBDrawInterface.cpp
    # CBTMessageBox.cpp is Win32-only (CBT hooks, GetDesktopWindow, etc.)
    # ${MAIN52_SOURCE_DIR}/CBTMessageBox.cpp
    ${MAIN52_SOURCE_DIR}/CB_DangKyInGame.cpp
    ${MAIN52_SOURCE_DIR}/CB_GetMixRate.cpp
    ${MAIN52_SOURCE_DIR}/CUIListBox.cpp
    ${MAIN52_SOURCE_DIR}/CustomEventTime.cpp
    ${MAIN52_SOURCE_DIR}/DialogInfo.cpp
    ${MAIN52_SOURCE_DIR}/ExplanationInfo.cpp
    ${MAIN52_SOURCE_DIR}/ItemTooltip.cpp
    ${MAIN52_SOURCE_DIR}/MsgWin.cpp
    ${MAIN52_SOURCE_DIR}/OptionWin.cpp
    ${MAIN52_SOURCE_DIR}/ServerMsgWin.cpp
    ${MAIN52_SOURCE_DIR}/SlideInfo.cpp
    ${MAIN52_SOURCE_DIR}/Slider.cpp
    ${MAIN52_SOURCE_DIR}/Observer.cpp
    ${MAIN52_SOURCE_DIR}/PList.cpp
    ${MAIN52_SOURCE_DIR}/CreditInfo.cpp
    ${MAIN52_SOURCE_DIR}/CreditWin.cpp
    ${MAIN52_SOURCE_DIR}/Button.cpp
    ${MAIN52_SOURCE_DIR}/CharInfoBalloon.cpp
    ${MAIN52_SOURCE_DIR}/CharInfoBalloonMng.cpp
    ${MAIN52_SOURCE_DIR}/CharMakeWin.cpp
    ${MAIN52_SOURCE_DIR}/CharSelMainWin.cpp
    ${MAIN52_SOURCE_DIR}/LoginMainWin.cpp
    ${MAIN52_SOURCE_DIR}/LoginWin.cpp
    ${MAIN52_SOURCE_DIR}/ServerSelWin.cpp
    ${MAIN52_SOURCE_DIR}/SysMenuWin.cpp
    ${MAIN52_SOURCE_DIR}/UIControlRender.cpp
    ${MAIN52_SOURCE_DIR}/UIControls.cpp
    ${MAIN52_SOURCE_DIR}/UIDefaultBase.cpp
    ${MAIN52_SOURCE_DIR}/UIGateKeeper.cpp
    ${MAIN52_SOURCE_DIR}/UIGuardsMan.cpp
    ${MAIN52_SOURCE_DIR}/UIGuildInfo.cpp
    ${MAIN52_SOURCE_DIR}/UIGuildMaster.cpp
    ${MAIN52_SOURCE_DIR}/UIJewelHarmony.cpp
    ${MAIN52_SOURCE_DIR}/UIManager.cpp
    ${MAIN52_SOURCE_DIR}/UIMapName.cpp
    ${MAIN52_SOURCE_DIR}/UIMng.cpp
    ${MAIN52_SOURCE_DIR}/UIPopup.cpp
    ${MAIN52_SOURCE_DIR}/UISenatus.cpp
    ${MAIN52_SOURCE_DIR}/UIWindows.cpp
    ${MAIN52_SOURCE_DIR}/npcBreeder.cpp
    ${MAIN52_SOURCE_DIR}/npcCatapult.cpp
    ${MAIN52_SOURCE_DIR}/npcGateSwitch.cpp
)

# -- NewUI M1: Foundation classes --
# NewUISystem.cpp deferred — it depends on NewUISystem.h which includes 80+
# NewUI headers for window classes not yet ported.
# NewUICommon.cpp IsPurchaseShop() wrapped in #ifndef ANDROID — uses stub fallback.
set(MAIN52_WAVE8_NEWUI_M1
    ${MAIN52_SOURCE_DIR}/NewUIManager.cpp
    ${MAIN52_SOURCE_DIR}/NewUIGroup.cpp
    ${MAIN52_SOURCE_DIR}/NewUI3DRenderMng.cpp
    ${MAIN52_SOURCE_DIR}/NewUIRenderNumber.cpp
    ${MAIN52_SOURCE_DIR}/NewUICommon.cpp
    ${MAIN52_SOURCE_DIR}/NewUIButton.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMessageBox.cpp
    ${MAIN52_SOURCE_DIR}/NewUIScrollBar.cpp
    ${MAIN52_SOURCE_DIR}/NewUIListGroupButton.cpp
    ${MAIN52_SOURCE_DIR}/ExternalObject/Leaf/xstreambuf.cpp
)

# -- NewUI M2: Login & HUD windows --
set(MAIN52_WAVE9_NEWUI_M2
    ${MAIN52_SOURCE_DIR}/NewUIMacroGaugeBar.cpp
    ${MAIN52_SOURCE_DIR}/NewUIBuffWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIHotKey.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMiniMap.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMainFrameWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMasterSkillTree.cpp
    ${MAIN52_SOURCE_DIR}/NewUICharacterInfoWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIDuelWatchMainFrameWindow.cpp
)

# -- NewUI M3: Inventory & overlay windows --
set(MAIN52_WAVE10_NEWUI_M3
    ${MAIN52_SOURCE_DIR}/NewUIItemMng.cpp
    ${MAIN52_SOURCE_DIR}/NewUIInventoryCore.cpp
    ${MAIN52_SOURCE_DIR}/NewUIInventoryCtrl.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMyInventory.cpp
    ${MAIN52_SOURCE_DIR}/NewUIPurchaseShopInventory.cpp
    ${MAIN52_SOURCE_DIR}/NewUIInventoryJewel.cpp
    ${MAIN52_SOURCE_DIR}/NewUITrade.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMixInventory.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMyShopInventory.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMixExpansion.cpp
    ${MAIN52_SOURCE_DIR}/NewUIInvenExpansion.cpp
    ${MAIN52_SOURCE_DIR}/NewUINPCShop.cpp
    ${MAIN52_SOURCE_DIR}/NewUIOtherInventory.cpp
)

# -- NewUI M4: Dialog windows --
set(MAIN52_WAVE11_NEWUI_M4
    ${MAIN52_SOURCE_DIR}/NewUIOptionWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIHelpWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIFriendWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUINPCDialogue.cpp
    ${MAIN52_SOURCE_DIR}/NewUIGuildInfoWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIGuildMakeWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIPartyInfoWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIPartyListWindow.cpp
)

# -- NewUI M5: Social & utility windows --
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

# -- NewUI M6: Remaining windows & polish --
set(MAIN52_WAVE13_NEWUI_M6
    ${MAIN52_SOURCE_DIR}/NewUISystem.cpp
    ${MAIN52_SOURCE_DIR}/NewUIItemTooltip.cpp
    ${MAIN52_SOURCE_DIR}/NewUICustomMessageBox.cpp
    ${MAIN52_SOURCE_DIR}/NewUICommonMessageBox.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMacroMain.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMacroSub.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMoveCommandWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUICastleWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIGuardWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIGatemanWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIGateSwitchWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIPetInfoWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIHeroPositionInfo.cpp
    ${MAIN52_SOURCE_DIR}/NewUIRankingTop.cpp
    ${MAIN52_SOURCE_DIR}/NewUIGensRanking.cpp
    ${MAIN52_SOURCE_DIR}/NewUITextBox.cpp
    ${MAIN52_SOURCE_DIR}/NewUIEventTimeView.cpp
    ${MAIN52_SOURCE_DIR}/NewUISetItemExplanation.cpp
    ${MAIN52_SOURCE_DIR}/NewUIItemExplanationWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIItemEnduranceInfo.cpp
    ${MAIN52_SOURCE_DIR}/NewUIUnitedMarketPlaceWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIStorageExpansion.cpp
    ${MAIN52_SOURCE_DIR}/NewUIStorageInventory.cpp
    ${MAIN52_SOURCE_DIR}/NewUIExchangeLuckyCoin.cpp
    ${MAIN52_SOURCE_DIR}/NewUIRegistrationLuckyCoin.cpp
    ${MAIN52_SOURCE_DIR}/NewUILuckyItemWnd.cpp
    ${MAIN52_SOURCE_DIR}/NewUIPurcharseVip.cpp
    ${MAIN52_SOURCE_DIR}/NewUINPCQuest.cpp
    ${MAIN52_SOURCE_DIR}/NewUIQuestProgress.cpp
    ${MAIN52_SOURCE_DIR}/NewUIQuestProgressByEtc.cpp
    ${MAIN52_SOURCE_DIR}/NewUIMyQuestInfoWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUISiegeWarObserver.cpp
    ${MAIN52_SOURCE_DIR}/NewUISiegeWarSoldier.cpp
    ${MAIN52_SOURCE_DIR}/NewUISiegeWarCommander.cpp
    ${MAIN52_SOURCE_DIR}/NewUISiegeWarBase.cpp
    ${MAIN52_SOURCE_DIR}/NewUISeigeWarfare.cpp
    ${MAIN52_SOURCE_DIR}/NewUICatapultWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIBloodCastleEnter.cpp
    ${MAIN52_SOURCE_DIR}/NewUIBloodCastleTime.cpp
    ${MAIN52_SOURCE_DIR}/NewUIChaosCastleTime.cpp
    ${MAIN52_SOURCE_DIR}/NewUICursedTempleEnter.cpp
    ${MAIN52_SOURCE_DIR}/NewUICursedTempleResult.cpp
    ${MAIN52_SOURCE_DIR}/NewUICursedTempleSystem.cpp
    ${MAIN52_SOURCE_DIR}/NewUIDevilSquareEnter.cpp
    ${MAIN52_SOURCE_DIR}/NewUICryWolf.cpp
    ${MAIN52_SOURCE_DIR}/NewUIKanturuEvent.cpp
    ${MAIN52_SOURCE_DIR}/NewUIEmpireGuardianNPC.cpp
    ${MAIN52_SOURCE_DIR}/NewUIEmpireGuardianTimer.cpp
    ${MAIN52_SOURCE_DIR}/NewUIBattleSoccerScore.cpp
    ${MAIN52_SOURCE_DIR}/NewUIDoppelGangerFrame.cpp
    ${MAIN52_SOURCE_DIR}/NewUIDoppelGangerWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIDuelWatchUserListWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIDuelWatchWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIDuelWindow.cpp
    ${MAIN52_SOURCE_DIR}/NewUIGoldBowmanLena.cpp
    ${MAIN52_SOURCE_DIR}/NewUIGoldBowmanWindow.cpp
)

# ============================================================================
# Combine all Main5.2 sources
# ============================================================================
set(MAIN52_SOURCES
    ${MAIN52_WAVE2}
    ${MAIN52_WAVE3}
    ${MAIN52_WAVE4}
    ${MAIN52_WAVE5}
    ${MAIN52_WAVE6}
    ${MAIN52_WAVE7}
    ${MAIN52_WAVE8_NEWUI_M1}
    ${MAIN52_WAVE9_NEWUI_M2}
    ${MAIN52_WAVE10_NEWUI_M3}
    ${MAIN52_WAVE11_NEWUI_M4}
    ${MAIN52_WAVE12_NEWUI_M5}
    ${MAIN52_WAVE13_NEWUI_M6}
)

add_library(mu-client SHARED
    # Phase 1-9: JNI bridge, game loop, EGL, input, audio, file I/O, rendering, network
    src/main/jni/MuBridge.cpp
    src/main/jni/MuGameLoop.cpp
    src/main/jni/MuGLContext.cpp
    src/main/jni/MuInput.cpp
    src/main/jni/MuAudio.cpp
    src/main/jni/MuFile.cpp
    src/main/jni/MuAssetExtractor.cpp
    src/main/jni/ShaderLoader.cpp
    src/main/jni/RenderBatcher.cpp
    src/main/jni/RenderState.cpp
    src/main/jni/TerrainBatcher.cpp
    src/main/jni/SpriteRenderer.cpp
    src/main/jni/MuNetwork.cpp
    src/main/jni/SimpleModulus.cpp
    src/main/jni/TimerManager.cpp
    src/main/jni/ProtocolDispatch.cpp
    # Phase 12A: Global state + compatibility layer
    src/main/jni/MuGlobalState.cpp
    src/main/jni/MuStringTable.cpp
    src/main/jni/MuConfig.cpp
    # Phase 13: Singleton initialization (uses original stdafx.h include chain)
    src/main/jni/MuGameInit.cpp
    # Phase 12C: CWsctlc stubs (original protocol send/receive bridge)
    src/main/jni/wsctlc_stubs.cpp
    # Phase 12D: Original WSclient.cpp — TranslateProtocol + all packet handlers
    ${MAIN52_SOURCE_DIR}/WSclient.cpp
    # Phase 12F: Original Main5.2 source files
    ${MAIN52_SOURCES}
    # ImGui core
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    # ImGui backends (Android)
    src/main/jni/imgui_impl_android.cpp
    src/main/jni/imgui_impl_gles3.cpp
)

target_include_directories(mu-client PRIVATE
    ${CMAKE_SOURCE_DIR}/src/main/jni
    ${IMGUI_DIR}
    ${GLM_DIR}
    ${GLM_DIR}/glm
    ${CMAKE_BINARY_DIR}/turbojpeg      # Generated Android jconfig.h (before Windows one)
    ${GLM_DIR}/turbojpeg
    ${GLM_DIR}/Sol
    ${GLM_DIR}/asio
    ${MAIN52_SOURCE_DIR}
    # Library includes provided by linked targets below:
    #   lua54    -> Lua 5.4.2 headers
    #   cryptopp -> Crypto++ 8.2 headers
    #   freetype -> FreeType 2.13.3 headers
)

target_link_libraries(mu-client
    ${android-lib}
    ${egl-lib}
    ${glesv3-lib}
    ${opensles-lib}
    ${log-lib}
    lua54
    freetype
    cryptopp
    turbojpeg
)

# Force-include MuPlatform.h so all Main5.2 source files get Windows type shims
target_compile_options(mu-client PRIVATE -include MuPlatform.h)

target_link_options(mu-client PRIVATE -Wl,--error-limit=0)
