//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextMutex.cpp: Classes for protecting Context access and EGLImage siblings.

#include "libANGLE/ContextMutex.h"

#include "common/system_utils.h"
#include "libANGLE/Context.h"

namespace egl
{

namespace
{
[[maybe_unused]] bool CheckThreadIdCurrent(const std::atomic<angle::ThreadId> &threadId,
                                           angle::ThreadId *currentThreadIdOut)
{
    *currentThreadIdOut = angle::GetCurrentThreadId();
    return (threadId.load(std::memory_order_relaxed) == *currentThreadIdOut);
}

[[maybe_unused]] bool TryUpdateThreadId(std::atomic<angle::ThreadId> *threadId,
                                        angle::ThreadId oldThreadId,
                                        angle::ThreadId newThreadId)
{
    const bool ok = (threadId->load(std::memory_order_relaxed) == oldThreadId);
    if (ok)
    {
        threadId->store(newThreadId, std::memory_order_relaxed);
    }
    return ok;
}
}  // namespace

// ScopedContextMutexAddRefLock
void ScopedContextMutexAddRefLock::lock(ContextMutex *mutex)
{
    ASSERT(mutex != nullptr);
    ASSERT(mMutex == nullptr);
    // lock() before addRef() - using mMutex as synchronization
    mutex->lock();
    // Take the "root" mutex after the lock.
    mMutex = mutex->getRoot();
    ASSERT(mMutex->isReferenced());
    mMutex->addRef();
}

// ContextMutex
ContextMutex::ContextMutex(ContextMutex *root)
    : mRoot(this), mOwnerThreadId(angle::InvalidThreadId()), mLockLevel(0), mRefCount(0), mRank(0)
{
    if (root != nullptr)
    {
        setNewRoot(root);
    }
}

ContextMutex::~ContextMutex()
{
    ASSERT(mLockLevel == 0);
    ASSERT(mRefCount == 0);
    ASSERT(mLeaves.empty());

    ContextMutex *const root = getRoot();
    if (this == root)
    {
        ASSERT(mOldRoots.empty());
    }
    else
    {
        for (ContextMutex *oldRoot : mOldRoots)
        {
            ASSERT(oldRoot->getRoot() == root);
            ASSERT(oldRoot->mLeaves.empty());
            oldRoot->release();
        }
        root->removeLeaf(this);
        root->release();
    }
}

void ContextMutex::Merge(ContextMutex *lockedMutex, ContextMutex *otherMutex)
{
    ASSERT(lockedMutex != nullptr);
    ASSERT(otherMutex != nullptr);

    // Since lockedMutex is locked, its "root" pointer is stable.
    ContextMutex *lockedRoot      = lockedMutex->getRoot();
    ContextMutex *otherLockedRoot = nullptr;

    // Mutex merging will update the structure of both mutexes, therefore both mutexes must be
    // locked before continuing. First mutex is already locked, need to lock the other mutex.
    // Because other thread may perform merge with same mutexes reversed, we can't simply lock
    // otherMutex - this may cause a deadlock. Additionally, otherMutex may have same "root" (same
    // mutex or already merged), not only merging is unnecessary, but locking otherMutex will
    // guarantee a deadlock.

    for (;;)
    {
        // First, check that "root" of otherMutex is the same as "root" of lockedMutex.
        // lockedRoot is stable by definition and it is safe to compare with "unstable root".
        ContextMutex *otherRoot = otherMutex->getRoot();
        if (otherRoot == lockedRoot)
        {
            // Do nothing if two mutexes are the same/merged.
            return;
        }
        // Second, try to lock otherMutex "root" (can't use lock()/lockImpl(), see above comment).
        if (otherRoot->tryLockImpl())
        {
            otherLockedRoot = otherRoot->getRoot();
            // otherMutex "root" can't become lockedMutex "root". For that to happen, lockedMutex
            // must be locked from some other thread first, which is impossible, since it is already
            // locked by this thread.
            ASSERT(otherLockedRoot != lockedRoot);
            // Lock is successful. Both mutexes are locked - can proceed with the merge...
            break;
        }
        // Lock was unsuccessful - unlock and retry...
        // May use "unlockImpl()" because lockedRoot is a "stable root" mutex.
        // Note: lock will be preserved in case of the recursive lock.
        lockedRoot->unlockImpl();
        // Sleep random amount to allow one of the thread acquire the lock next time...
        std::this_thread::sleep_for(std::chrono::microseconds(rand() % 91 + 10));
        // Because lockedMutex was unlocked, its "root" might have been changed. Below line will
        // reacquire the lock and update lockedRoot pointer.
        lockedMutex->lock();
        lockedRoot = lockedMutex->getRoot();
    }

    // Decide the new "root". See mRank comment for more details...

    ContextMutex *oldRoot = otherLockedRoot;
    ContextMutex *newRoot = lockedRoot;

    if (oldRoot->mRank > newRoot->mRank)
    {
        std::swap(oldRoot, newRoot);
    }
    else if (oldRoot->mRank == newRoot->mRank)
    {
        ++newRoot->mRank;
    }

    ASSERT(newRoot->isReferenced());

    // Update the structure
    for (ContextMutex *const leaf : oldRoot->mLeaves)
    {
        ASSERT(leaf->getRoot() == oldRoot);
        leaf->setNewRoot(newRoot);
    }
    oldRoot->mLeaves.clear();
    oldRoot->setNewRoot(newRoot);

    // Leave only the "merged" mutex locked. "oldRoot" already merged, need to use "unlockImpl()"
    oldRoot->unlockImpl();

    // Merge from recursive lock is unexpected. Handle such cases anyway to be safe.
    while (oldRoot->mLockLevel > 0)
    {
        newRoot->lockImpl();
        oldRoot->unlockImpl();
    }
}

void ContextMutex::setNewRoot(ContextMutex *newRoot)
{
    ContextMutex *const oldRoot = getRoot();

    ASSERT(newRoot != oldRoot);
    mRoot.store(newRoot, std::memory_order_relaxed);
    newRoot->addRef();

    newRoot->addLeaf(this);

    if (oldRoot != this)
    {
        mOldRoots.emplace_back(oldRoot);
    }
}

void ContextMutex::addLeaf(ContextMutex *leaf)
{
    ASSERT(this == getRoot());
    ASSERT(leaf->getRoot() == this);
    ASSERT(leaf->mLeaves.empty());
    ASSERT(mLeaves.count(leaf) == 0);
    mLeaves.emplace(leaf);
}

void ContextMutex::removeLeaf(ContextMutex *leaf)
{
    ASSERT(this == getRoot());
    ASSERT(leaf->getRoot() == this);
    ASSERT(leaf->mLeaves.empty());
    ASSERT(mLeaves.count(leaf) == 1);
    mLeaves.erase(leaf);
}

void ContextMutex::release(UnlockBehaviour unlockBehaviour)
{
    ASSERT(isReferenced());
    const bool needDelete = (--mRefCount == 0);
    if (unlockBehaviour == UnlockBehaviour::kUnlock)
    {
        ASSERT(this == getRoot());
        unlockImpl();
    }
    if (needDelete)
    {
        delete this;
    }
}

bool ContextMutex::try_lock()
{
    return getRoot()->tryLockImpl();
}

void ContextMutex::lock()
{
    getRoot()->lockImpl();
}

void ContextMutex::unlock()
{
    ContextMutex *const root = getRoot();
    // "root" is currently locked so "root->getRoot()" will return stable result.
    ASSERT(root == root->getRoot());
    root->unlockImpl();
}

#if defined(ANGLE_ENABLE_CONTEXT_MUTEX_RECURSION)
bool ContextMutex::tryLockImpl()
{
    const angle::ThreadId threadId = angle::GetCurrentThreadId();
    if (ANGLE_UNLIKELY(!mMutex.try_lock()))
    {
        if (ANGLE_UNLIKELY(mOwnerThreadId.load(std::memory_order_relaxed) == threadId))
        {
            ASSERT(this == getRoot());
            ASSERT(mLockLevel > 0);
            ++mLockLevel;
            return true;
        }
        return false;
    }
    ASSERT(mOwnerThreadId.load(std::memory_order_relaxed) == angle::InvalidThreadId());
    ASSERT(mLockLevel == 0);
    ContextMutex *const root = getRoot();
    if (ANGLE_UNLIKELY(this != root))
    {
        // Unlock, so only the "stable root" mutex remains locked
        mMutex.unlock();
        return root->tryLockImpl();
    }
    mOwnerThreadId.store(threadId, std::memory_order_relaxed);
    mLockLevel = 1;
    return true;
}

void ContextMutex::lockImpl()
{
    const angle::ThreadId threadId = angle::GetCurrentThreadId();
    if (ANGLE_UNLIKELY(!mMutex.try_lock()))
    {
        if (ANGLE_UNLIKELY(mOwnerThreadId.load(std::memory_order_relaxed) == threadId))
        {
            ASSERT(this == getRoot());
            ASSERT(mLockLevel > 0);
            ++mLockLevel;
            return;
        }
        mMutex.lock();
    }
    ASSERT(mOwnerThreadId.load(std::memory_order_relaxed) == angle::InvalidThreadId());
    ASSERT(mLockLevel == 0);
    ContextMutex *const root = getRoot();
    if (ANGLE_UNLIKELY(this != root))
    {
        // Unlock, so only the "stable root" mutex remains locked
        mMutex.unlock();
        root->lockImpl();
    }
    else
    {
        mOwnerThreadId.store(threadId, std::memory_order_relaxed);
        mLockLevel = 1;
    }
}

void ContextMutex::unlockImpl()
{
    ASSERT(mOwnerThreadId.load(std::memory_order_relaxed) == angle::GetCurrentThreadId());
    ASSERT(mLockLevel > 0);
    if (ANGLE_LIKELY(--mLockLevel == 0))
    {
        mOwnerThreadId.store(angle::InvalidThreadId(), std::memory_order_relaxed);
        mMutex.unlock();
    }
}
#else
bool ContextMutex::tryLockImpl()
{
    angle::ThreadId currentThreadId;
    ASSERT(!CheckThreadIdCurrent(mOwnerThreadId, &currentThreadId));
    if (mMutex.try_lock())
    {
        ContextMutex *const root = getRoot();
        if (ANGLE_UNLIKELY(this != root))
        {
            // Unlock, so only the "stable root" mutex remains locked
            mMutex.unlock();
            return root->tryLockImpl();
        }
        ASSERT(TryUpdateThreadId(&mOwnerThreadId, angle::InvalidThreadId(), currentThreadId));
        return true;
    }
    return false;
}

void ContextMutex::lockImpl()
{
    angle::ThreadId currentThreadId;
    ASSERT(!CheckThreadIdCurrent(mOwnerThreadId, &currentThreadId));
    mMutex.lock();
    ContextMutex *const root = getRoot();
    if (ANGLE_UNLIKELY(this != root))
    {
        // Unlock, so only the "stable root" mutex remains locked
        mMutex.unlock();
        root->lockImpl();
    }
    else
    {
        ASSERT(TryUpdateThreadId(&mOwnerThreadId, angle::InvalidThreadId(), currentThreadId));
    }
}

void ContextMutex::unlockImpl()
{
    ASSERT(
        TryUpdateThreadId(&mOwnerThreadId, angle::GetCurrentThreadId(), angle::InvalidThreadId()));
    mMutex.unlock();
}
#endif

}  // namespace egl
