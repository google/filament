//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SimpleMutex.cpp:
//   Implementation of SimpleMutex.h.

#include "common/SimpleMutex.h"

#if ANGLE_USE_FUTEX

#    include <limits.h>
#    include <stdint.h>

#    if defined(ANGLE_PLATFORM_LINUX) || defined(ANGLE_PLATFORM_ANDROID)
#        include <linux/futex.h>
#        include <sys/syscall.h>
#        include <unistd.h>
#    endif  // defined(ANGLE_PLATFORM_LINUX) || defined(ANGLE_PLATFORM_ANDROID)

#    if defined(ANGLE_PLATFORM_WINDOWS)
#        include <errno.h>
#        include <windows.h>
#    endif  // defined(ANGLE_PLATFORM_WINDOWS)

namespace angle
{
namespace priv
{
#    if defined(ANGLE_PLATFORM_LINUX) || defined(ANGLE_PLATFORM_ANDROID)
namespace
{
ANGLE_INLINE void SysFutex(void *addr, int op, int val, int val3)
{
    syscall(SYS_futex, addr, op, val, nullptr, nullptr, val3);
}
}  // anonymous namespace

void MutexOnFutex::futexWait()
{
    SysFutex(&mState, FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG, kBlocked, FUTEX_BITSET_MATCH_ANY);
}
void MutexOnFutex::futexWake()
{
    SysFutex(&mState, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, kLocked, 0);
}
#    endif  // defined(ANGLE_PLATFORM_LINUX) || defined(ANGLE_PLATFORM_ANDROID)

#    if defined(ANGLE_PLATFORM_WINDOWS)
void MutexOnFutex::futexWait()
{
    int value = kBlocked;
    WaitOnAddress(&mState, &value, sizeof(value), INFINITE);
}

void MutexOnFutex::futexWake()
{
    WakeByAddressSingle(&mState);
}
#    endif  // defined(ANGLE_PLATFORM_WINDOWS)
}  // namespace priv
}  // namespace angle

#endif  // ANGLE_USE_FUTEX
