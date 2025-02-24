//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextMutex.h: Classes for protecting Context access and EGLImage siblings.

#ifndef LIBANGLE_CONTEXT_MUTEX_H_
#define LIBANGLE_CONTEXT_MUTEX_H_

#include <atomic>

#include "common/debug.h"

namespace gl
{
class Context;
}

namespace egl
{
#if defined(ANGLE_ENABLE_CONTEXT_MUTEX)
constexpr bool kIsContextMutexEnabled = true;
#else
constexpr bool kIsContextMutexEnabled = false;
#endif

// Use standard mutex for now
using ContextMutexType = std::mutex;

class ContextMutex final : angle::NonCopyable
{
  public:
    explicit ContextMutex(ContextMutex *root = nullptr);
    // For "leaf" mutex its "root" must be locked during destructor call.
    ~ContextMutex();

    // Merges mutexes so they work as one.
    // At the end, only single "root" mutex will be locked.
    // Does nothing if two mutexes are the same or already merged (have same "root" mutex).
    static void Merge(ContextMutex *lockedMutex, ContextMutex *otherMutex);

    // Returns current "root" mutex.
    // Warning! Result is only stable if mutex is locked, while may change any time if unlocked.
    // May be used to compare against already locked "root" mutex.
    ANGLE_INLINE ContextMutex *getRoot() { return mRoot.load(std::memory_order_relaxed); }
    ANGLE_INLINE const ContextMutex *getRoot() const
    {
        return mRoot.load(std::memory_order_relaxed);
    }

    // Below group of methods are not thread safe and must be protected by "this" mutex instance.
    ANGLE_INLINE void addRef() { ++mRefCount; }
    ANGLE_INLINE void release() { release(UnlockBehaviour::kDoNotUnlock); }
    // Must be only called on "root" mutex.
    ANGLE_INLINE void releaseAndUnlock() { release(UnlockBehaviour::kUnlock); }
    ANGLE_INLINE bool isReferenced() const { return mRefCount > 0; }

    bool try_lock();
    void lock();
    void unlock();

  private:
    enum class UnlockBehaviour
    {
        kDoNotUnlock,
        kUnlock
    };

    bool tryLockImpl();
    void lockImpl();
    void unlockImpl();

    // All methods below must be protected by "this" mutex ("stable root" in "this" instance).

    void setNewRoot(ContextMutex *newRoot);
    void addLeaf(ContextMutex *leaf);
    void removeLeaf(ContextMutex *leaf);

    void release(UnlockBehaviour unlockBehaviour);

  private:
    // mRoot and mLeaves tree structure details:
    // - used to implement primary functionality of this class;
    // - initially, all mutexes are "root"s;
    // - "root" mutex has "mRoot == this";
    // - "root" mutex stores unreferenced pointers to all its leaves (used in merging);
    // - "leaf" mutex holds reference (addRef) to the current "root" mutex in the mRoot;
    // - "leaf" mutex has empty mLeaves;
    // - "leaf" mutex can't become a "root" mutex;
    // - before locking the mMutex, "this" is an "unstable root" or a "leaf";
    // - the implementation always locks mRoot's mMutex ("unstable root");
    // - if after locking the mMutex "mRoot != this", then "this" is/become a "leaf";
    // - otherwise, "this" is a locked "stable root" - lock is successful.

    // mOldRoots is used to solve a particular problem (below example does not use mRank):
    // - have "leaf" mutex_2 with a reference to mutex_1 "root";
    // - the mutex_1 has no other references (only in the mutex_2);
    // - have other mutex_3 "root";
    // - mutex_1 pointer is cached on the stack during locking of mutex_2 (thread A);
    // - merge mutex_3 and mutex_2 (thread B):
    //     * now "leaf" mutex_2 stores reference to mutex_3 "root";
    //     * old "root" mutex_1 becomes a "leaf" of mutex_3;
    //     * old "root" mutex_1 has no references and gets destroyed.
    // - invalid pointer to destroyed mutex_1 stored on the stack and in the mLeaves of mutex_3;
    // - to fix this problem, references to old "root"s are kept in the mOldRoots vector.

    // mRank is used to fix a problem of indefinite grows of mOldRoots:
    // - merge mutex_2 and mutex_1 -> mutex_2 is "root" of mutex_1 (mOldRoots == 0);
    // - destroy mutex_2;
    // - merge mutex_3 and mutex_1 -> mutex_3 is "root" of mutex_1 (mOldRoots == 1);
    // - destroy mutex_3;
    // - merge mutex_4 and mutex_1 -> mutex_4 is "root" of mutex_1 (mOldRoots == 2);
    // - destroy mutex_4;
    // - continuing this pattern can lead to indefinite grows of mOldRoots, while pick number of
    //   mutexes is only 2.
    // Fix details using mRank:
    // - initially "mRank == 0" and only relevant for "root" mutexes;
    // - merging mutexes with equal mRank of their "root"s, will use first (lockedMutex) "root"
    //   mutex as a new "root" and increase its mRank by 1;
    // - otherwise, "root" mutex with a highest rank will be used without changing the mRank;
    // - this way, "stronger" (with a higher mRank) "root" mutex will "protect" its "leaves" from
    //   "mRoot" replacement and therefore - mOldRoots grows.
    // Lets look at the problematic pattern with the mRank:
    // - merge mutex_2 and mutex_1 -> mutex_2 is "root" (mRank == 1) of mutex_1 (mOldRoots == 0);
    // - destroy mutex_2;
    // - merge mutex_3 and mutex_1 -> mutex_2 is "root" (mRank == 1) of mutex_3 (mOldRoots == 0);
    // - destroy mutex_3;
    // - merge mutex_4 and mutex_1 -> mutex_2 is "root" (mRank == 1) of mutex_4 (mOldRoots == 0);
    // - destroy mutex_4;
    // - no mOldRoots grows at all;
    // - minumum number of mutexes to reach mOldRoots size of N => 2^(N+1).

    std::atomic<ContextMutex *> mRoot;
    ContextMutexType mMutex;
    // Used when ASSERT() and/or recursion are/is enabled.
    std::atomic<angle::ThreadId> mOwnerThreadId;
    // Used only when recursion is enabled.
    uint32_t mLockLevel;
    size_t mRefCount;

    std::set<ContextMutex *> mLeaves;
    std::vector<ContextMutex *> mOldRoots;
    uint32_t mRank;
};

// Prevents destruction while locked, uses mMutex to protect addRef()/releaseAndUnlock() calls.
class [[nodiscard]] ScopedContextMutexAddRefLock final : angle::NonCopyable
{
  public:
    ANGLE_INLINE ScopedContextMutexAddRefLock() = default;
    ANGLE_INLINE explicit ScopedContextMutexAddRefLock(ContextMutex &mutex) { lock(&mutex); }
    ANGLE_INLINE ScopedContextMutexAddRefLock(ContextMutex *mutex)
    {
        if (mutex != nullptr)
        {
            lock(mutex);
        }
    }
    ANGLE_INLINE ~ScopedContextMutexAddRefLock()
    {
        if (mMutex != nullptr)
        {
            mMutex->releaseAndUnlock();
        }
    }

  private:
    void lock(ContextMutex *mutex);

  private:
    ContextMutex *mMutex = nullptr;
};

class [[nodiscard]] ScopedContextMutexLock final
{
  public:
    ANGLE_INLINE ScopedContextMutexLock() = default;
    ANGLE_INLINE explicit ScopedContextMutexLock(ContextMutex &mutex) : mMutex(&mutex)
    {
        mutex.lock();
    }
    ANGLE_INLINE ScopedContextMutexLock(ContextMutex *mutex) : mMutex(mutex)
    {
        if (ANGLE_LIKELY(mutex != nullptr))
        {
            mutex->lock();
        }
    }
    ANGLE_INLINE ~ScopedContextMutexLock()
    {
        if (ANGLE_LIKELY(mMutex != nullptr))
        {
            mMutex->unlock();
        }
    }

    ANGLE_INLINE ScopedContextMutexLock(ScopedContextMutexLock &&other) : mMutex(other.mMutex)
    {
        other.mMutex = nullptr;
    }
    ANGLE_INLINE ScopedContextMutexLock &operator=(ScopedContextMutexLock &&other)
    {
        std::swap(mMutex, other.mMutex);
        return *this;
    }

  private:
    ContextMutex *mMutex = nullptr;
};

}  // namespace egl

#endif  // LIBANGLE_CONTEXT_MUTEX_H_
