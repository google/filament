//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GlobalMutex.cpp: Defines Global Mutex and utilities.

#include "libANGLE/GlobalMutex.h"

#include <atomic>
#include <type_traits>

#include "common/debug.h"
#include "common/system_utils.h"

namespace egl
{
namespace priv
{
using GlobalMutexType = std::mutex;

#if !defined(ANGLE_ENABLE_ASSERTS) && !defined(ANGLE_ENABLE_GLOBAL_MUTEX_RECURSION)
// Default version.
class GlobalMutex final : angle::NonCopyable
{
  public:
    ANGLE_INLINE void lock() { mMutex.lock(); }
    ANGLE_INLINE void unlock() { mMutex.unlock(); }

  protected:
    GlobalMutexType mMutex;
};
#endif

#if defined(ANGLE_ENABLE_ASSERTS) && !defined(ANGLE_ENABLE_GLOBAL_MUTEX_RECURSION)
// Debug version.
class GlobalMutex final : angle::NonCopyable
{
  public:
    ANGLE_INLINE void lock()
    {
        const angle::ThreadId threadId = angle::GetCurrentThreadId();
        ASSERT(getOwnerThreadId() != threadId);
        mMutex.lock();
        ASSERT(getOwnerThreadId() == angle::InvalidThreadId());
        mOwnerThreadId.store(threadId, std::memory_order_relaxed);
    }

    ANGLE_INLINE void unlock()
    {
        ASSERT(getOwnerThreadId() == angle::GetCurrentThreadId());
        mOwnerThreadId.store(angle::InvalidThreadId(), std::memory_order_relaxed);
        mMutex.unlock();
    }

  private:
    ANGLE_INLINE angle::ThreadId getOwnerThreadId() const
    {
        return mOwnerThreadId.load(std::memory_order_relaxed);
    }

    GlobalMutexType mMutex;
    std::atomic<angle::ThreadId> mOwnerThreadId{angle::InvalidThreadId()};
};
#endif  // defined(ANGLE_ENABLE_ASSERTS) && !defined(ANGLE_ENABLE_GLOBAL_MUTEX_RECURSION)

#if defined(ANGLE_ENABLE_GLOBAL_MUTEX_RECURSION)
// Recursive version.
class GlobalMutex final : angle::NonCopyable
{
  public:
    ANGLE_INLINE void lock()
    {
        const angle::ThreadId threadId = angle::GetCurrentThreadId();
        if (ANGLE_UNLIKELY(!mMutex.try_lock()))
        {
            if (ANGLE_UNLIKELY(getOwnerThreadId() == threadId))
            {
                ASSERT(mLockLevel > 0);
                ++mLockLevel;
                return;
            }
            mMutex.lock();
        }
        ASSERT(getOwnerThreadId() == angle::InvalidThreadId());
        ASSERT(mLockLevel == 0);
        mOwnerThreadId.store(threadId, std::memory_order_relaxed);
        mLockLevel = 1;
    }

    ANGLE_INLINE void unlock()
    {
        ASSERT(getOwnerThreadId() == angle::GetCurrentThreadId());
        ASSERT(mLockLevel > 0);
        if (ANGLE_LIKELY(--mLockLevel == 0))
        {
            mOwnerThreadId.store(angle::InvalidThreadId(), std::memory_order_relaxed);
            mMutex.unlock();
        }
    }

  private:
    ANGLE_INLINE angle::ThreadId getOwnerThreadId() const
    {
        return mOwnerThreadId.load(std::memory_order_relaxed);
    }

    GlobalMutexType mMutex;
    std::atomic<angle::ThreadId> mOwnerThreadId{angle::InvalidThreadId()};
    uint32_t mLockLevel = 0;
};
#endif  // defined(ANGLE_ENABLE_GLOBAL_MUTEX_RECURSION)
namespace
{
#if defined(ANGLE_ENABLE_GLOBAL_MUTEX_LOAD_TIME_ALLOCATE)
#    if !ANGLE_HAS_ATTRIBUTE_CONSTRUCTOR || !ANGLE_HAS_ATTRIBUTE_DESTRUCTOR
#        error \
            "'angle_enable_global_mutex_load_time_allocate' " \
               "requires constructor/destructor compiler atributes."
#    endif
GlobalMutex *g_MutexPtr        = nullptr;
GlobalMutex *g_EGLSyncMutexPtr = nullptr;

void ANGLE_CONSTRUCTOR AllocateGlobalMutex()
{
    ASSERT(g_MutexPtr == nullptr);
    g_MutexPtr = new GlobalMutex();
    ASSERT(g_EGLSyncMutexPtr == nullptr);
    g_EGLSyncMutexPtr = new GlobalMutex();
}

void ANGLE_DESTRUCTOR DeallocateGlobalMutex()
{
    SafeDelete(g_MutexPtr);
    SafeDelete(g_EGLSyncMutexPtr);
}
#else
ANGLE_REQUIRE_CONSTANT_INIT std::atomic<GlobalMutex *> g_Mutex(nullptr);
ANGLE_REQUIRE_CONSTANT_INIT std::atomic<GlobalMutex *> g_EGLSyncMutex(nullptr);
static_assert(std::is_trivially_destructible<decltype(g_Mutex)>::value,
              "global mutex is not trivially destructible");
static_assert(std::is_trivially_destructible<decltype(g_EGLSyncMutex)>::value,
              "global EGL Sync mutex is not trivially destructible");

GlobalMutex *AllocateGlobalMutexImpl(std::atomic<GlobalMutex *> *globalMutex)
{
    GlobalMutex *currentMutex = nullptr;
    std::unique_ptr<GlobalMutex> newMutex(new GlobalMutex());
    do
    {
        if (globalMutex->compare_exchange_weak(currentMutex, newMutex.get()))
        {
            return newMutex.release();
        }
    } while (currentMutex == nullptr);
    return currentMutex;
}

GlobalMutex *GetGlobalMutex()
{
    GlobalMutex *mutex = g_Mutex.load();
    return mutex != nullptr ? mutex : AllocateGlobalMutexImpl(&g_Mutex);
}

GlobalMutex *GetGlobalEGLSyncObjectMutex()
{
    GlobalMutex *mutex = g_EGLSyncMutex.load();
    return mutex != nullptr ? mutex : AllocateGlobalMutexImpl(&g_EGLSyncMutex);
}
#endif
}  // anonymous namespace

// ScopedGlobalMutexLock implementation.
#if defined(ANGLE_ENABLE_GLOBAL_MUTEX_LOAD_TIME_ALLOCATE)
template <GlobalMutexChoice mutexChoice>
ScopedGlobalMutexLock<mutexChoice>::ScopedGlobalMutexLock()
{
    switch (mutexChoice)
    {
        case GlobalMutexChoice::EGL:
            g_MutexPtr->lock();
            break;
        case GlobalMutexChoice::Sync:
            g_EGLSyncMutexPtr->lock();
            break;
        default:
            UNREACHABLE();
            break;
    }
}

template <GlobalMutexChoice mutexChoice>
ScopedGlobalMutexLock<mutexChoice>::~ScopedGlobalMutexLock()
{
    switch (mutexChoice)
    {
        case GlobalMutexChoice::EGL:
            g_MutexPtr->unlock();
            break;
        case GlobalMutexChoice::Sync:
            g_EGLSyncMutexPtr->unlock();
            break;
        default:
            UNREACHABLE();
            break;
    }
}
#else
template <GlobalMutexChoice mutexChoice>
ScopedGlobalMutexLock<mutexChoice>::ScopedGlobalMutexLock()
{
    switch (mutexChoice)
    {
        case GlobalMutexChoice::EGL:
            mMutex = GetGlobalMutex();
            break;
        case GlobalMutexChoice::Sync:
            mMutex = GetGlobalEGLSyncObjectMutex();
            break;
        default:
            UNREACHABLE();
            break;
    }

    mMutex->lock();
}

template <GlobalMutexChoice mutexChoice>
ScopedGlobalMutexLock<mutexChoice>::~ScopedGlobalMutexLock()
{
    mMutex->unlock();
}
#endif

template class ScopedGlobalMutexLock<GlobalMutexChoice::EGL>;
template class ScopedGlobalMutexLock<GlobalMutexChoice::Sync>;
}  // namespace priv

// ScopedOptionalGlobalMutexLock implementation.
ScopedOptionalGlobalMutexLock::ScopedOptionalGlobalMutexLock(bool enabled)
{
    if (enabled)
    {
#if defined(ANGLE_ENABLE_GLOBAL_MUTEX_LOAD_TIME_ALLOCATE)
        mMutex = priv::g_MutexPtr;
#else
        mMutex = priv::GetGlobalMutex();
#endif
        mMutex->lock();
    }
    else
    {
        mMutex = nullptr;
    }
}

ScopedOptionalGlobalMutexLock::~ScopedOptionalGlobalMutexLock()
{
    if (mMutex != nullptr)
    {
        mMutex->unlock();
    }
}

// Global functions.
#if defined(ANGLE_PLATFORM_WINDOWS) && !defined(ANGLE_STATIC)
#    if defined(ANGLE_ENABLE_GLOBAL_MUTEX_LOAD_TIME_ALLOCATE)
#        error "'angle_enable_global_mutex_load_time_allocate' is not supported in Windows DLL."
#    endif

void AllocateGlobalMutex()
{
    (void)priv::AllocateGlobalMutexImpl(&priv::g_Mutex);
    (void)priv::AllocateGlobalMutexImpl(&priv::g_EGLSyncMutex);
}

void DeallocateGlobalMutexImpl(std::atomic<priv::GlobalMutex *> *globalMutex)
{
    priv::GlobalMutex *mutex = globalMutex->exchange(nullptr);
    if (mutex != nullptr)
    {
        {
            // Wait for the mutex to become released by other threads before deleting.
            std::lock_guard<priv::GlobalMutex> lock(*mutex);
        }
        delete mutex;
    }
}

void DeallocateGlobalMutex()
{
    DeallocateGlobalMutexImpl(&priv::g_Mutex);
    DeallocateGlobalMutexImpl(&priv::g_EGLSyncMutex);
}
#endif

}  // namespace egl
