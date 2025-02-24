//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// tls.h: Simple cross-platform interface for thread local storage.

#ifndef COMMON_TLS_H_
#define COMMON_TLS_H_

#include "common/angleutils.h"
#include "common/platform.h"

#if defined(ANGLE_PLATFORM_POSIX)
#    include <errno.h>
#    include <pthread.h>
#    include <semaphore.h>
#elif defined(ANGLE_PLATFORM_WINDOWS)
#    include <windows.h>
#endif

namespace gl
{
class Context;
}

namespace angle
{

#ifdef ANGLE_PLATFORM_WINDOWS
// TLS does not exist for Windows Store and needs to be emulated
#    ifdef ANGLE_ENABLE_WINDOWS_UWP
#        ifndef TLS_OUT_OF_INDEXES
#            define TLS_OUT_OF_INDEXES static_cast<DWORD>(0xFFFFFFFF)
#        endif
#        ifndef CREATE_SUSPENDED
#            define CREATE_SUSPENDED 0x00000004
#        endif
#    endif
typedef DWORD TLSIndex;
#    define TLS_INVALID_INDEX (TLS_OUT_OF_INDEXES)
#elif defined(ANGLE_PLATFORM_POSIX)
typedef pthread_key_t TLSIndex;
#    define TLS_INVALID_INDEX (static_cast<angle::TLSIndex>(-1))
#else
#    error Unsupported platform.
#endif

#if defined(ANGLE_USE_ANDROID_TLS_SLOT)

// TLS_SLOT_OPENGL and TLS_SLOT_OPENGL_API aren't used by bionic itself, but allow the graphics code
// to access TLS directly rather than using the pthread API.
//
// Choose the TLS_SLOT_OPENGL TLS slot with the value that matches value in the header file in
// bionic(tls_defines.h).  Note that this slot cannot be used when the GLES backend of is in use.
#    if defined(__arm__) || defined(__aarch64__)
constexpr size_t kAndroidOpenGLTlsSlot = 3;
#    elif defined(__i386__) || defined(__x86_64__)
constexpr size_t kAndroidOpenGLTlsSlot = 3;
#    elif defined(__riscv)
constexpr int kAndroidOpenGLTlsSlot = -5;
#    else
#        error Unsupported platform.
#    endif

// The following ASM variant provides a much more performant store/retrieve interface
// compared to those provided by the pthread library. These have been derived from code
// in the bionic module of Android ->
// https://cs.android.com/android/platform/superproject/+/master:bionic/libc/platform/bionic/tls.h;l=30

#    if defined(__aarch64__)
#        define ANGLE_ANDROID_GET_GL_TLS()                  \
            ({                                              \
                void **__val;                               \
                __asm__("mrs %0, tpidr_el0" : "=r"(__val)); \
                __val;                                      \
            })
#    elif defined(__arm__)
#        define ANGLE_ANDROID_GET_GL_TLS()                           \
            ({                                                       \
                void **__val;                                        \
                __asm__("mrc p15, 0, %0, c13, c0, 3" : "=r"(__val)); \
                __val;                                               \
            })
#    elif defined(__mips__)
// On mips32r1, this goes via a kernel illegal instruction trap that's
// optimized for v1
#        define ANGLE_ANDROID_GET_GL_TLS()       \
            ({                                   \
                register void **__val asm("v1"); \
                __asm__(                         \
                    ".set    push\n"             \
                    ".set    mips32r2\n"         \
                    "rdhwr   %0,$29\n"           \
                    ".set    pop\n"              \
                    : "=r"(__val));              \
                __val;                           \
            })
#    elif defined(__i386__)
#        define ANGLE_ANDROID_GET_GL_TLS()                \
            ({                                            \
                void **__val;                             \
                __asm__("movl %%gs:0, %0" : "=r"(__val)); \
                __val;                                    \
            })
#    elif defined(__x86_64__)
#        define ANGLE_ANDROID_GET_GL_TLS()               \
            ({                                           \
                void **__val;                            \
                __asm__("mov %%fs:0, %0" : "=r"(__val)); \
                __val;                                   \
            })
#    elif defined(__riscv)
#        define ANGLE_ANDROID_GET_GL_TLS()          \
            ({                                      \
                void **__val;                       \
                __asm__("mv %0, tp" : "=r"(__val)); \
                __val;                              \
            })
#    else
#        error unsupported architecture
#    endif

#endif  // ANGLE_USE_ANDROID_TLS_SLOT

using PthreadKeyDestructor = void (*)(void *);
TLSIndex CreateTLSIndex(PthreadKeyDestructor destructor);
bool DestroyTLSIndex(TLSIndex index);

bool SetTLSValue(TLSIndex index, void *value);
void *GetTLSValue(TLSIndex index);

}  // namespace angle

#endif  // COMMON_TLS_H_
