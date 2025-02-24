//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// platform.h: Operating system specific includes and defines.

#ifndef COMMON_PLATFORM_H_
#define COMMON_PLATFORM_H_

#if defined(_WIN32)
#    define ANGLE_PLATFORM_WINDOWS 1
#elif defined(__Fuchsia__)
#    define ANGLE_PLATFORM_FUCHSIA 1
#    define ANGLE_PLATFORM_POSIX 1
#elif defined(__APPLE__)
#    define ANGLE_PLATFORM_APPLE 1
#    define ANGLE_PLATFORM_POSIX 1
#elif defined(ANDROID) && !defined(ANGLE_ANDROID_DMA_BUF)
#    define ANGLE_PLATFORM_ANDROID 1
#    define ANGLE_PLATFORM_POSIX 1
#elif defined(__ggp__)
#    define ANGLE_PLATFORM_GGP 1
#    define ANGLE_PLATFORM_POSIX 1
#elif defined(__linux__) || defined(EMSCRIPTEN)
#    define ANGLE_PLATFORM_LINUX 1
#    define ANGLE_PLATFORM_POSIX 1
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) ||              \
    defined(__DragonFly__) || defined(__sun) || defined(__GLIBC__) || defined(__GNU__) || \
    defined(__QNX__) || defined(__Fuchsia__) || defined(__HAIKU__)
#    define ANGLE_PLATFORM_POSIX 1
#else
#    error Unsupported platform.
#endif

#ifdef ANGLE_PLATFORM_WINDOWS
#    ifndef STRICT
#        define STRICT 1
#    endif
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN 1
#    endif
#    ifndef NOMINMAX
#        define NOMINMAX 1
#    endif

#    include <intrin.h>

#    if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#        define ANGLE_ENABLE_WINDOWS_UWP 1
#    endif

#    if defined(ANGLE_ENABLE_D3D9)
#        include <d3d9.h>
#        include <d3dcompiler.h>
#    endif

// Include D3D11 headers when OpenGL is enabled on Windows for interop extensions.
#    if defined(ANGLE_ENABLE_D3D11) || defined(ANGLE_ENABLE_OPENGL)
#        include <d3d10_1.h>
#        include <d3d11.h>
#        include <d3d11_3.h>
#        include <d3d11on12.h>
#        include <d3d12.h>
#        include <d3dcompiler.h>
#        include <dxgi.h>
#        include <dxgi1_2.h>
#        include <dxgi1_4.h>
#    endif

#    if defined(ANGLE_ENABLE_D3D9) || defined(ANGLE_ENABLE_D3D11)
#        include <wrl.h>
#    endif

#    if defined(ANGLE_ENABLE_WINDOWS_UWP)
#        include <dxgi1_3.h>
#        if defined(_DEBUG)
#            include <DXProgrammableCapture.h>
#            include <dxgidebug.h>
#        endif
#    endif

// GCC < 10.4 or 11.0 - 11.3 miscodegen extern thread_local variable accesses.
// This affects MinGW targets only.
// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=104862
#    if defined(__GNUC__)
#        if __GNUC__ < 10 || __GNUC__ == 10 && __GNUC_MINOR__ < 4 || \
            __GNUC__ == 11 && __GNUC_MINOR__ < 3
#            define ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES 1
#        endif
#    endif

// Include <windows.h> to ensure files that refer to near/far can be compiled.
#    include <windows.h>

// Macros 'near', 'far', 'NEAR' and 'FAR' are defined by 'shared/minwindef.h' in the Windows SDK.
// Macros 'near' and 'far' are empty. They are not used by other Windows headers and are undefined
// here to avoid identifier conflicts. Macros 'NEAR' and 'FAR' contain 'near' and 'far'. They are
// used by other Windows headers and are cleared here to avoid compilation errors.
#    undef near
#    undef far
#    undef NEAR
#    undef FAR
#    define NEAR
#    define FAR
#endif

// Mips and arm devices need to include stddef for size_t.
#if defined(__mips__) || defined(__arm__) || defined(__aarch64__) || defined(__riscv)
#    include <stddef.h>
#endif

// Macro for hinting that an expression is likely to be true/false.
#if !defined(ANGLE_LIKELY) || !defined(ANGLE_UNLIKELY)
#    if defined(__GNUC__) || defined(__clang__)
#        define ANGLE_LIKELY(x) __builtin_expect(!!(x), 1)
#        define ANGLE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#    else
#        define ANGLE_LIKELY(x) (x)
#        define ANGLE_UNLIKELY(x) (x)
#    endif  // defined(__GNUC__) || defined(__clang__)
#endif      // !defined(ANGLE_LIKELY) || !defined(ANGLE_UNLIKELY)

#ifdef ANGLE_PLATFORM_APPLE
#    include <AvailabilityMacros.h>
#    include <TargetConditionals.h>
#    if TARGET_OS_OSX
#        if __MAC_OS_X_VERSION_MAX_ALLOWED < 120000
#            error macOS 12 SDK or newer is required.
#        endif
#        define ANGLE_PLATFORM_MACOS 1
#    elif TARGET_OS_IPHONE
#        define ANGLE_PLATFORM_IOS_FAMILY 1
#        if TARGET_OS_SIMULATOR
#            define ANGLE_PLATFORM_IOS_FAMILY_SIMULATOR 1
#        endif
#        if TARGET_OS_VISION  // Must be checked before iOS
#            define ANGLE_PLATFORM_VISIONOS 1
#        elif TARGET_OS_IOS
#            if __IPHONE_OS_VERSION_MAX_ALLOWED < 170000
#                error iOS 17 SDK or newer is required.
#            endif
#            define ANGLE_PLATFORM_IOS 1
#            if TARGET_OS_MACCATALYST
#                define ANGLE_PLATFORM_MACCATALYST 1
#            endif
#        elif TARGET_OS_WATCH
#            define ANGLE_PLATFORM_WATCHOS 1
#        elif TARGET_OS_TV
#            if __TV_OS_VERSION_MAX_ALLOWED < 170000
#                error tvOS 17 SDK or newer is required.
#            endif
#            define ANGLE_PLATFORM_APPLETV 1
#        endif
#    endif
#endif

// Define ANGLE_WITH_ASAN macro.
#if defined(__has_feature)
#    if __has_feature(address_sanitizer)
#        define ANGLE_WITH_ASAN 1
#    endif
#endif

// Define ANGLE_WITH_MSAN macro.
#if defined(__has_feature)
#    if __has_feature(memory_sanitizer)
#        define ANGLE_WITH_MSAN 1
#    endif
#endif

// Define ANGLE_WITH_TSAN macro.
#if defined(__has_feature)
#    if __has_feature(thread_sanitizer)
#        define ANGLE_WITH_TSAN 1
#    endif
#endif

// Define ANGLE_WITH_UBSAN macro.
#if defined(__has_feature)
#    if __has_feature(undefined_behavior_sanitizer)
#        define ANGLE_WITH_UBSAN 1
#    endif
#endif

#if defined(ANGLE_WITH_ASAN) || defined(ANGLE_WITH_TSAN) || defined(ANGLE_WITH_UBSAN)
#    define ANGLE_WITH_SANITIZER 1
#endif  // defined(ANGLE_WITH_ASAN) || defined(ANGLE_WITH_TSAN) || defined(ANGLE_WITH_UBSAN)

#include <stdint.h>
#if INTPTR_MAX == INT64_MAX
#    define ANGLE_IS_64_BIT_CPU 1
#else
#    define ANGLE_IS_32_BIT_CPU 1
#endif

#endif  // COMMON_PLATFORM_H_
