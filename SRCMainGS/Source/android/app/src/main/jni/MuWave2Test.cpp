// Wave 2 header verification — verify BMD/Texture/ModelManager headers compile
// with the full Wave 1 type system baseline.
// Removed once Wave 2 .cpp files are integrated.
#include "stdafx_port.h"

// Wave 1 baseline (types needed by BMD headers)
#include "_define.h"
#include "_enum.h"
#include "_types.h"
#include "_struct.h"
#include "Math/ZzzMathLib.h"
#include "Defined_Global.h"

// Wave 2 headers
#include "ZzzBMD.h"
#include "ZzzTexture.h"
#include "CGMModelManager.h"

// Dummy function to avoid empty translation unit
void MuWave2Test_dummy() {}
