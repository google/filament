//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlobalMutex.h: Defines Global Mutex and utilities.

#ifndef LIBANGLE_GLOBAL_MUTEX_H_
#define LIBANGLE_GLOBAL_MUTEX_H_

#include "common/angleutils.h"

namespace egl
{
namespace priv
{
class GlobalMutex;

enum class GlobalMutexChoice
{
    // All EGL entry points except EGL Sync objects
    EGL,
    // Entry points relating to EGL Sync objects
    Sync,
};

template <GlobalMutexChoice mutexChoice>
class [[nodiscard]] ScopedGlobalMutexLock final : angle::NonCopyable
{
  public:
    ScopedGlobalMutexLock();
    ~ScopedGlobalMutexLock();

#if !defined(ANGLE_ENABLE_GLOBAL_MUTEX_LOAD_TIME_ALLOCATE)
  private:
    GlobalMutex *mMutex;
#endif
};
}  // namespace priv

using ScopedGlobalEGLMutexLock = priv::ScopedGlobalMutexLock<priv::GlobalMutexChoice::EGL>;
using ScopedGlobalEGLSyncObjectMutexLock =
    priv::ScopedGlobalMutexLock<priv::GlobalMutexChoice::Sync>;

// For Context protection where lock is optional. Works slower than ScopedGlobalMutexLock.
class [[nodiscard]] ScopedOptionalGlobalMutexLock final : angle::NonCopyable
{
  public:
    explicit ScopedOptionalGlobalMutexLock(bool enabled);
    ~ScopedOptionalGlobalMutexLock();

  private:
    priv::GlobalMutex *mMutex;
};

#if defined(ANGLE_PLATFORM_WINDOWS) && !defined(ANGLE_STATIC)
void AllocateGlobalMutex();
void DeallocateGlobalMutex();
#endif

}  // namespace egl

#endif  // LIBANGLE_GLOBAL_MUTEX_H_
