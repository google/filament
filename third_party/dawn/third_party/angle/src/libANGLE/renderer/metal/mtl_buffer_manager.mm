//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_buffer_manager.mm:
//    Implements the class methods for BufferManager.
//

#include "libANGLE/renderer/metal/mtl_buffer_manager.h"

#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"

namespace rx
{

namespace mtl
{

namespace
{

int storageModeToCacheIndex(MTLStorageMode storageMode)
{
    static_assert(MTLStorageModeShared == 0);
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    static_assert(MTLStorageModeManaged == 1);
#endif
    return static_cast<int>(storageMode);
}

}  // namespace

BufferManager::BufferManager() {}

void BufferManager::freeUnusedBuffers(ContextMtl *contextMtl)
{
    // Scan for the first buffer still in use.
    BufferList::iterator firstInUseIter =
        std::find_if(mInUseBuffers.begin(), mInUseBuffers.end(),
                     [&contextMtl](auto ref) { return ref->isBeingUsedByGPU(contextMtl); });

    // Move unused buffers to the free lists
    for (BufferList::iterator it = mInUseBuffers.begin(); it != firstInUseIter; ++it)
    {
        addBufferRefToFreeLists(*it);
    }
    mInUseBuffers.erase(mInUseBuffers.begin(), firstInUseIter);
}

void BufferManager::addBufferRefToFreeLists(mtl::BufferRef &bufferRef)
{
    int cacheIndex = storageModeToCacheIndex(bufferRef->storageMode());
    ASSERT(cacheIndex < kNumCachedStorageModes);
    mFreeBuffers[cacheIndex].insert(BufferMap::value_type(bufferRef->size(), bufferRef));
}

void BufferManager::returnBuffer(ContextMtl *contextMtl, BufferRef &bufferRef)
{
    int cacheIndex = storageModeToCacheIndex(bufferRef->storageMode());
    if (cacheIndex >= kNumCachedStorageModes)
    {
        return;  // Storage mode that we do not have a cache for.
    }
    bufferRef->setNumContextSwitchesAtLastUse(mContextSwitches);
    bufferRef->setNumCommandBufferCommitsAtLastUse(mCommandBufferCommits);
    if (bufferRef->isBeingUsedByGPU(contextMtl))
    {
        mInUseBuffers.push_back(bufferRef);
    }
    else
    {
        addBufferRefToFreeLists(bufferRef);
    }
}

void BufferManager::incrementNumContextSwitches()
{
    ++mContextSwitches;
    // Ignore wraparound for the moment
    if (mContextSwitches - mContextSwitchesAtLastGC >= kContextSwitchesBetweenGC)
    {
        collectGarbage(GCReason::ContextSwitches);
    }
}

void BufferManager::incrementNumCommandBufferCommits()
{
    ++mCommandBufferCommits;
    // Ignore wraparound for the moment
    if (mCommandBufferCommits - mCommandBufferCommitsAtLastGC >= kCommandBufferCommitsBetweenGC)
    {
        collectGarbage(GCReason::CommandBufferCommits);
    }
}

void BufferManager::collectGarbage(BufferManager::GCReason reason)
{
    ANGLE_UNUSED_VARIABLE(reason);
#ifdef ANGLE_MTL_TRACK_BUFFER_MEM
    {
        fprintf(stderr, "** Before BufferManager GC (reason=%s): totalMem: %zu, ",
                (reason == GCReason::ContextSwitches
                     ? "context switches"
                     : (reason == GCReason::CommandBufferCommits ? "command buffer commits"
                                                                 : "total memory")),
                mTotalMem);
        size_t numBuffers = 0;
        for (auto iter = mAllocatedSizes.begin(); iter != mAllocatedSizes.end(); ++iter)
        {
            fprintf(stderr, "%zu: %zu, ", iter->first, iter->second);
            numBuffers += iter->second;
        }
        fprintf(stderr, " total: %zu\n", numBuffers);
    }
#endif

    for (int i = 0; i < kNumCachedStorageModes; ++i)
    {
        BufferMap &map = mFreeBuffers[i];
        auto iter      = map.begin();
        while (iter != map.end())
        {
            // Clean out cached buffers for the following reasons:
            // 1) GC was triggered because of total memory use.
            // 2) The buffer has gone unused for multiple context switches.
            // 3) The buffer hasn't been used for many command buffer commits.
            if (reason == GCReason::TotalMem ||
                (mContextSwitches - iter->second->getNumContextSwitchesAtLastUse() >=
                 kContextSwitchesBetweenGC) ||
                (mCommandBufferCommits - iter->second->getNumCommandBufferCommitsAtLastUse() >=
                 kCommandBufferCommitsBetweenGC))
            {
                iter = map.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
    mContextSwitchesAtLastGC      = mContextSwitches;
    mCommandBufferCommitsAtLastGC = mCommandBufferCommits;

    {
        mTotalMem = 0;
#ifdef ANGLE_MTL_TRACK_BUFFER_MEM
        mAllocatedSizes.clear();
#endif
        for (auto iter = mInUseBuffers.begin(); iter != mInUseBuffers.end(); ++iter)
        {
            size_t sz = (*iter)->size();
            mTotalMem += sz;
#ifdef ANGLE_MTL_TRACK_BUFFER_MEM
            ++mAllocatedSizes[sz];
#endif
        }

        for (int i = 0; i < kNumCachedStorageModes; ++i)
        {
            BufferMap &map = mFreeBuffers[i];
            for (auto iter = map.begin(); iter != map.end(); ++iter)
            {
                size_t sz = iter->first;
#ifdef ANGLE_MTL_TRACK_BUFFER_MEM
                ++mAllocatedSizes[sz];
#endif
                mTotalMem += sz;
            }
        }

#ifdef ANGLE_MTL_TRACK_BUFFER_MEM
        fprintf(stderr, "** After BufferManager GC: totalMem: %zu, ", mTotalMem);
        size_t numBuffers = 0;
        for (auto iter = mAllocatedSizes.begin(); iter != mAllocatedSizes.end(); ++iter)
        {
            fprintf(stderr, "%zu: %zu, ", iter->first, iter->second);
            numBuffers += iter->second;
        }
        fprintf(stderr, " total: %zu\n", numBuffers);
#endif

        mTotalMemAtLastGC = mTotalMem;
    }
}

angle::Result BufferManager::getBuffer(ContextMtl *contextMtl,
                                       MTLStorageMode storageMode,
                                       size_t size,
                                       BufferRef &bufferRef)
{
    freeUnusedBuffers(contextMtl);
    const int cacheIndex = storageModeToCacheIndex(storageMode);
    if (cacheIndex < kNumCachedStorageModes)
    {
        // Buffer has a storage mode that have a cache for.
        BufferMap &freeBuffers = mFreeBuffers[cacheIndex];
        auto iter              = freeBuffers.find(size);
        if (iter != freeBuffers.end())
        {
            bufferRef = iter->second;
            freeBuffers.erase(iter);
            bufferRef->setNumContextSwitchesAtLastUse(mContextSwitches);
            bufferRef->setNumCommandBufferCommitsAtLastUse(mCommandBufferCommits);
            return angle::Result::Continue;
        }
    }

    // Create a new one
    mtl::BufferRef newBufferRef;

    ANGLE_TRY(mtl::Buffer::MakeBufferWithStorageMode(contextMtl, storageMode, size, nullptr,
                                                     &newBufferRef));

    mTotalMem += size;

#ifdef ANGLE_MTL_TRACK_BUFFER_MEM
    {
        mAllocatedSizes[size]++;
        fprintf(stderr, "totalMem: %zu, ", mTotalMem);
        size_t numBuffers = 0;
        for (auto iter = mAllocatedSizes.begin(); iter != mAllocatedSizes.end(); ++iter)
        {
            fprintf(stderr, "%zu: %zu, ", iter->first, iter->second);
            numBuffers += iter->second;
        }
        fprintf(stderr, " total: %zu\n", numBuffers);
    }
#endif

    bool lotOfMemAllocedRecently = (mTotalMem > mTotalMemAtLastGC + kMemAllocedBetweenGC);
    if (mTotalMem > kMinMemBasedGC &&
        (mTotalMem > 2 * mTotalMemAtLastGC || lotOfMemAllocedRecently))
    {
        if (mInUseBuffers.size() > 0 && lotOfMemAllocedRecently)
        {
            // A considerable amount of memory has been allocated since the last GC. Wait for all
            // pending GPU work to finish in order to unreference in-flight buffers more quickly.
            contextMtl->flushCommandBuffer(mtl::WaitUntilFinished);
        }
        else
        {
            // Flush the current command buffer so that the in-flight buffers are scheduled and will
            // eventually become unreferenced. They won't be collected during this GC cycle but
            // hopefully will in the next one.
            contextMtl->flushCommandBuffer(mtl::NoWait);
        }
        freeUnusedBuffers(contextMtl);
        collectGarbage(GCReason::TotalMem);
    }

    bufferRef = newBufferRef;

    return angle::Result::Continue;
}

angle::Result BufferManager::queueBlitCopyDataToBuffer(ContextMtl *contextMtl,
                                                       const void *srcPtr,
                                                       size_t sizeToCopy,
                                                       size_t offset,
                                                       mtl::BufferRef &dstMetalBuffer)
{
    const uint8_t *src = reinterpret_cast<const uint8_t *>(srcPtr);

    for (size_t srcOffset = 0; srcOffset < sizeToCopy; srcOffset += kMaxStagingBufferSize)
    {
        size_t subSizeToCopy = std::min(kMaxStagingBufferSize, sizeToCopy - srcOffset);

        mtl::BufferRef bufferRef;
        // TODO(anglebug.com/40644888): Here we pass DynamicDraw to get managed buffer for the
        // operation. This should be checked to see if this makes sense.
        auto storageMode = Buffer::getStorageModeForUsage(contextMtl, Buffer::Usage::DynamicDraw);
        ANGLE_TRY(getBuffer(contextMtl, storageMode, subSizeToCopy, bufferRef));

        // copy data to buffer
        uint8_t *ptr = bufferRef->mapWithOpt(contextMtl, false, true);
        std::copy(src + srcOffset, src + srcOffset + subSizeToCopy, ptr);
        bufferRef->unmapAndFlushSubset(contextMtl, 0, subSizeToCopy);

        // queue blit
        mtl::BlitCommandEncoder *blitEncoder = contextMtl->getBlitCommandEncoder();
        blitEncoder->copyBuffer(bufferRef, 0, dstMetalBuffer, offset + srcOffset, subSizeToCopy);

        returnBuffer(contextMtl, bufferRef);
    }
    return angle::Result::Continue;
}

}  // namespace mtl
}  // namespace rx
