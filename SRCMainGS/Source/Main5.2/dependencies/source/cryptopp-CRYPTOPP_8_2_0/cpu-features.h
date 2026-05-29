// Minimal stub for Android NDK cpu-features.h
// Crypto++ uses this for CPU feature detection on Android.
// Since we compile with CRYPTOPP_DISABLE_ASM, the detection
// results aren't used for SIMD code paths, but the header
// is still included unconditionally on __ANDROID__.

#ifndef CPU_FEATURES_H_STUB
#define CPU_FEATURES_H_STUB

#include <stdint.h>

typedef enum {
    ANDROID_CPU_FAMILY_UNKNOWN = 0,
    ANDROID_CPU_FAMILY_ARM,
    ANDROID_CPU_FAMILY_X86,
    ANDROID_CPU_FAMILY_MIPS,
    ANDROID_CPU_FAMILY_ARM64,
    ANDROID_CPU_FAMILY_X86_64,
    ANDROID_CPU_FAMILY_MIPS64,
} AndroidCpuFamily;

typedef enum {
    ANDROID_CPU_ARM_FEATURE_ARMv7       = (1 << 0),
    ANDROID_CPU_ARM_FEATURE_VFPv3       = (1 << 1),
    ANDROID_CPU_ARM_FEATURE_NEON        = (1 << 2),
    ANDROID_CPU_ARM_FEATURE_LDREX_STREX = (1 << 3),
    ANDROID_CPU_ARM_FEATURE_AES         = (1 << 4),
    ANDROID_CPU_ARM_FEATURE_PMULL       = (1 << 5),
    ANDROID_CPU_ARM_FEATURE_SHA1        = (1 << 6),
    ANDROID_CPU_ARM_FEATURE_SHA2        = (1 << 7),
    ANDROID_CPU_ARM_FEATURE_CRC32       = (1 << 8),
} AndroidCpuArmFeatures;

typedef enum {
    ANDROID_CPU_X86_FEATURE_SSSE3  = (1 << 0),
    ANDROID_CPU_X86_FEATURE_POPCNT = (1 << 1),
    ANDROID_CPU_X86_FEATURE_MOVBE  = (1 << 2),
    ANDROID_CPU_X86_FEATURE_SSE4_1 = (1 << 3),
    ANDROID_CPU_X86_FEATURE_SSE4_2 = (1 << 4),
    ANDROID_CPU_X86_FEATURE_AES_NI = (1 << 5),
    ANDROID_CPU_X86_FEATURE_AVX    = (1 << 6),
} AndroidCpuX86Features;

typedef enum {
    ANDROID_CPU_ARM64_FEATURE_FP      = (1 << 0),
    ANDROID_CPU_ARM64_FEATURE_ASIMD   = (1 << 1),
    ANDROID_CPU_ARM64_FEATURE_AES     = (1 << 2),
    ANDROID_CPU_ARM64_FEATURE_PMULL   = (1 << 3),
    ANDROID_CPU_ARM64_FEATURE_SHA1    = (1 << 4),
    ANDROID_CPU_ARM64_FEATURE_SHA2    = (1 << 5),
    ANDROID_CPU_ARM64_FEATURE_CRC32   = (1 << 6),
} AndroidCpuArm64Features;

static inline AndroidCpuFamily     android_getCpuFamily(void)    { return ANDROID_CPU_FAMILY_UNKNOWN; }
static inline uint64_t             android_getCpuFeatures(void)  { return 0; }

#endif
