// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_COMMON_PLATFORM_H_
#define SRC_DAWN_COMMON_PLATFORM_H_

// Use #if DAWN_PLATFORM_IS(X) for platform specific code.
// Do not use #ifdef or the naked macro DAWN_PLATFORM_IS_X.
// This can help avoid common mistakes like not including "Platform.h" and falling into unwanted
// code block as usage of undefined macro "function" will be blocked by the compiler.
#define DAWN_PLATFORM_IS(X) (1 == DAWN_PLATFORM_IS_##X)

// Define platform macros for OSes:
//
//  - WINDOWS
//    - WIN32
//    - WINUWP
//  - POSIX
//    - LINUX
//      - ANDROID
//      - CHROMEOS
//    - APPLE
//      - IOS
//      - TVOS
//      - MACOS
//    - FUCHSIA
//    - EMSCRIPTEN
#if defined(_WIN32) || defined(_WIN64)
#include <winapifamily.h>
#define DAWN_PLATFORM_IS_WINDOWS 1
#if WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
#define DAWN_PLATFORM_IS_WIN32 1
#elif WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
#define DAWN_PLATFORM_IS_WINUWP 1
#else
#error "Unsupported Windows platform."
#endif

#elif defined(__linux__)
#define DAWN_PLATFORM_IS_LINUX 1
#define DAWN_PLATFORM_IS_POSIX 1
#if defined(__ANDROID__)
#define DAWN_PLATFORM_IS_ANDROID 1
#elif defined(DAWN_OS_CHROMEOS)
#define DAWN_PLATFORM_IS_CHROMEOS 1
#else
#define DAWN_PLATFORM_IS_LINUX_DESKTOP 1
#endif

#elif defined(__APPLE__)
#define DAWN_PLATFORM_IS_APPLE 1
#define DAWN_PLATFORM_IS_POSIX 1
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define DAWN_PLATFORM_IS_IOS 1
#if TARGET_OS_TV
#define DAWN_PLATFORM_IS_TVOS 1
#endif
#elif TARGET_OS_MAC
#define DAWN_PLATFORM_IS_MACOS 1
#else
#error "Unsupported Apple platform."
#endif

#elif defined(__Fuchsia__)
#define DAWN_PLATFORM_IS_FUCHSIA 1
#define DAWN_PLATFORM_IS_POSIX 1

#elif defined(__EMSCRIPTEN__)
#define DAWN_PLATFORM_IS_EMSCRIPTEN 1
#define DAWN_PLATFORM_IS_POSIX 1
#include <emscripten/emscripten.h>

#else
#error "Unsupported platform."
#endif

// Define platform macros for CPU architectures:
//
//  - X86
//    - I386
//    - X86_64
//  - ARM
//    - ARM32
//    - ARM64
//  - LOONGARCH
//    - LOONGARCH32
//    - LOONGARCH64
//  - RISCV
//    - RISCV32
//    - RISCV64
//  - MIPS
//    - MIPS32
//    - MIPS64
//  - S390
//  - S390X
//  - PPC
//  - PPC64
#if defined(__i386__) || defined(_M_IX86)
#define DAWN_PLATFORM_IS_X86 1
#define DAWN_PLATFORM_IS_I386 1
#elif defined(__x86_64__) || defined(_M_X64)
#define DAWN_PLATFORM_IS_X86 1
#define DAWN_PLATFORM_IS_X86_64 1

#elif defined(__arm__) || defined(_M_ARM)
#define DAWN_PLATFORM_IS_ARM 1
#define DAWN_PLATFORM_IS_ARM32 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#define DAWN_PLATFORM_IS_ARM 1
#define DAWN_PLATFORM_IS_ARM64 1

#elif defined(__loongarch__)
#define DAWN_PLATFORM_IS_LOONGARCH 1
#if __loongarch_grlen == 64
#define DAWN_PLATFORM_IS_LOONGARCH64 1
#else
#define DAWN_PLATFORM_IS_LOONGARCH32 1
#endif

#elif defined(__riscv)
#define DAWN_PLATFORM_IS_RISCV 1
#if __riscv_xlen == 32
#define DAWN_PLATFORM_IS_RISCV32 1
#else
#define DAWN_PLATFORM_IS_RISCV64 1
#endif

#elif defined(__mips__)
#define DAWN_PLATFORM_IS_MIPS 1
#if _MIPS_SIM == _ABIO32
#define DAWN_PLATFORM_IS_MIPS32 1
#else
#define DAWN_PLATFORM_IS_MIPS64 1
#endif

#elif defined(__s390__)
#define DAWN_PLATFORM_IS_S390 1
#elif defined(__s390x__)
#define DAWN_PLATFORM_IS_S390X 1

#elif defined(__PPC__)
#define DAWN_PLATFORM_IS_PPC 1
#elif defined(__PPC64__)
#define DAWN_PLATFORM_IS_PPC64 1

#elif defined(__wasm32__)
#define DAWN_PLATFORM_IS_WASM32 1
#elif defined(__wasm64__)
#define DAWN_PLATFORM_IS_WASM64 1

#else
#error "Unsupported platform."
#endif

// Define platform macros for pointer width:
//
//  - 64_BIT
//  - 32_BIT
#if defined(DAWN_PLATFORM_IS_X86_64) || defined(DAWN_PLATFORM_IS_ARM64) ||   \
    defined(DAWN_PLATFORM_IS_RISCV64) || defined(DAWN_PLATFORM_IS_MIPS64) || \
    defined(DAWN_PLATFORM_IS_S390X) || defined(DAWN_PLATFORM_IS_PPC64) ||    \
    defined(DAWN_PLATFORM_IS_LOONGARCH64) || defined(DAWN_PLATFORM_IS_WASM64)
#define DAWN_PLATFORM_IS_64_BIT 1
static_assert(sizeof(sizeof(char)) == 8, "Expect sizeof(size_t) == 8");
#elif defined(DAWN_PLATFORM_IS_I386) || defined(DAWN_PLATFORM_IS_ARM32) ||   \
    defined(DAWN_PLATFORM_IS_RISCV32) || defined(DAWN_PLATFORM_IS_MIPS32) || \
    defined(DAWN_PLATFORM_IS_S390) || defined(DAWN_PLATFORM_IS_PPC32) ||     \
    defined(DAWN_PLATFORM_IS_LOONGARCH32) || defined(DAWN_PLATFORM_IS_WASM32)
#define DAWN_PLATFORM_IS_32_BIT 1
static_assert(sizeof(sizeof(char)) == 4, "Expect sizeof(size_t) == 4");
#else
#error "Unsupported platform"
#endif

// This section define other platform macros to 0 to avoid undefined macro usage error.
#if !defined(DAWN_PLATFORM_IS_WINDOWS)
#define DAWN_PLATFORM_IS_WINDOWS 0
#endif
#if !defined(DAWN_PLATFORM_IS_WIN32)
#define DAWN_PLATFORM_IS_WIN32 0
#endif
#if !defined(DAWN_PLATFORM_IS_WINUWP)
#define DAWN_PLATFORM_IS_WINUWP 0
#endif

#if !defined(DAWN_PLATFORM_IS_POSIX)
#define DAWN_PLATFORM_IS_POSIX 0
#endif

#if !defined(DAWN_PLATFORM_IS_LINUX)
#define DAWN_PLATFORM_IS_LINUX 0
#endif
#if !defined(DAWN_PLATFORM_IS_ANDROID)
#define DAWN_PLATFORM_IS_ANDROID 0
#endif
#if !defined(DAWN_PLATFORM_IS_CHROMEOS)
#define DAWN_PLATFORM_IS_CHROMEOS 0
#endif
#if !defined(DAWN_PLATFORM_IS_LINUX_DESKTOP)
#define DAWN_PLATFORM_IS_LINUX_DESKTOP 0
#endif

#if !defined(DAWN_PLATFORM_IS_APPLE)
#define DAWN_PLATFORM_IS_APPLE 0
#endif
#if !defined(DAWN_PLATFORM_IS_IOS)
#define DAWN_PLATFORM_IS_IOS 0
#endif
#if !defined(DAWN_PLATFORM_IS_MACOS)
#define DAWN_PLATFORM_IS_MACOS 0
#endif

#if !defined(DAWN_PLATFORM_IS_FUCHSIA)
#define DAWN_PLATFORM_IS_FUCHSIA 0
#endif
#if !defined(DAWN_PLATFORM_IS_EMSCRIPTEN)
#define DAWN_PLATFORM_IS_EMSCRIPTEN 0
#endif

#if !defined(DAWN_PLATFORM_IS_X86)
#define DAWN_PLATFORM_IS_X86 0
#endif
#if !defined(DAWN_PLATFORM_IS_I386)
#define DAWN_PLATFORM_IS_I386 0
#endif
#if !defined(DAWN_PLATFORM_IS_X86_64)
#define DAWN_PLATFORM_IS_X86_64 0
#endif

#if !defined(DAWN_PLATFORM_IS_ARM)
#define DAWN_PLATFORM_IS_ARM 0
#endif
#if !defined(DAWN_PLATFORM_IS_ARM32)
#define DAWN_PLATFORM_IS_ARM32 0
#endif
#if !defined(DAWN_PLATFORM_IS_ARM64)
#define DAWN_PLATFORM_IS_ARM64 0
#endif

#if !defined(DAWN_PLATFORM_IS_LOONGARCH)
#define DAWN_PLATFORM_IS_LOONGARCH 0
#endif
#if !defined(DAWN_PLATFORM_IS_LOONGARCH32)
#define DAWN_PLATFORM_IS_LOONGARCH32 0
#endif
#if !defined(DAWN_PLATFORM_IS_LOONGARCH64)
#define DAWN_PLATFORM_IS_LOONGARCH64 0
#endif

#if !defined(DAWN_PLATFORM_IS_RISCV)
#define DAWN_PLATFORM_IS_RISCV 0
#endif
#if !defined(DAWN_PLATFORM_IS_RISCV32)
#define DAWN_PLATFORM_IS_RISCV32 0
#endif
#if !defined(DAWN_PLATFORM_IS_RISCV64)
#define DAWN_PLATFORM_IS_RISCV64 0
#endif

#if !defined(DAWN_PLATFORM_IS_MIPS)
#define DAWN_PLATFORM_IS_MIPS 0
#endif
#if !defined(DAWN_PLATFORM_IS_MIPS32)
#define DAWN_PLATFORM_IS_MIPS32 0
#endif
#if !defined(DAWN_PLATFORM_IS_MIPS64)
#define DAWN_PLATFORM_IS_MIPS64 0
#endif

#if !defined(DAWN_PLATFORM_IS_S390)
#define DAWN_PLATFORM_IS_S390 0
#endif
#if !defined(DAWN_PLATFORM_IS_S390X)
#define DAWN_PLATFORM_IS_S390X 0
#endif

#if !defined(DAWN_PLATFORM_IS_PPC)
#define DAWN_PLATFORM_IS_PPC 0
#endif
#if !defined(DAWN_PLATFORM_IS_PPC64)
#define DAWN_PLATFORM_IS_PPC64 0
#endif

#if !defined(DAWN_PLATFORM_IS_WASM32)
#define DAWN_PLATFORM_IS_WASM32 0
#endif
#if !defined(DAWN_PLATFORM_IS_WASM64)
#define DAWN_PLATFORM_IS_WASM64 0
#endif

#if !defined(DAWN_PLATFORM_IS_64_BIT)
#define DAWN_PLATFORM_IS_64_BIT 0
#endif
#if !defined(DAWN_PLATFORM_IS_32_BIT)
#define DAWN_PLATFORM_IS_32_BIT 0
#endif

#endif  // SRC_DAWN_COMMON_PLATFORM_H_
