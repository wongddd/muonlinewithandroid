#pragma once
// Shim for <intrin.h> (MSVC intrinsic functions) on Android/Clang
// NDK's own intrin.h can cause circular includes; this shim breaks the cycle.
// Real x86 intrinsics are handled by Clang's builtins on x86 targets.
