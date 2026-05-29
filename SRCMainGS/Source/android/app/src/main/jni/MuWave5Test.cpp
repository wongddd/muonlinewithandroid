// Wave 5-6 header verification — effects, AI, and non-UI headers.
// CGMEffectHandle.h excluded (pulls in NewUIManager/NewUIMyInventory).
// ZzzScene.h excluded (pulls in GlobalText.h → NewUIBase.h).
// UI verification deferred to Phase 13.
// Removed once .cpp integration begins.
#include "stdafx_port.h"

// Wave 1-4 baseline
#include "_define.h"
#include "_enum.h"
#include "_types.h"
#include "_struct.h"
#include "Math/ZzzMathLib.h"
#include "Defined_Global.h"
#include "ZzzBMD.h"
#include "ZzzLodTerrain.h"
#include "MapManager.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "CharacterManager.h"
#include "ZzzOpenglUtil.h"

// Wave 5-6: Effect & AI headers (no UI dependency)
#include "ZzzEffect.h"
#include "ZzzAI.h"
#include "CGMItemEffect.h"
#include "SkillEffectMgr.h"

// Dummy function to avoid empty translation unit
void MuWave5Test_dummy() {}
