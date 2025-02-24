//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// mtl_buffer_manager.h:
//    BufferManager manages buffers across all contexts for a single
//    device.
//
#ifndef LIBANGLE_RENDERER_METAL_MTL_BUFFER_MANAGER_H_
#define LIBANGLE_RENDERER_METAL_MTL_BUFFER_MANAGER_H_

#include "common/FixedVector.h"
#include "libANGLE/renderer/metal/mtl_resources.h"

#include <map>
#include <vector>

namespace rx
{
class ContextMtl;

namespace mtl
{

// GL buffers are backed by Metal buffers. Which metal
// buffer is backing a particular GL buffer is fluid.
// The case being optimized is a loop of something like
//
//    for 1..4
//      glBufferSubData
//      glDrawXXX
//
// You can't update a buffer in the middle of a render pass
// in metal so instead we'd end up using multiple buffers.
//
// Simple case, the call to `glBufferSubData` updates the
// entire buffer. In this case we'd end up with each call
// to `glBufferSubData` getting a new buffer from this
// BufferManager and copying the new data to it. We'd
// end up submitting this renderpass
//
//    draw with buf1
//    draw with buf2
//    draw with buf3
//    draw with buf4
//
// The GL buffer now references buf4. And buf1, buf2, buf3 and
// buf0 (the buffer that was previously referenced by the GL buffer)
// are all added to the inuse-list
//

// This macro enables showing the running totals of the various
// buckets of unused buffers.
// #define ANGLE_MTL_TRACK_BUFFER_MEM

class BufferManager
{
  public:
    BufferManager();

    static constexpr size_t kMaxStagingBufferSize = 1024 * 1024;
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    static constexpr int kNumCachedStorageModes = 2;
#else
    static constexpr int kNumCachedStorageModes = 1;
#endif
    static constexpr size_t kContextSwitchesBetweenGC      = 120;
    static constexpr size_t kCommandBufferCommitsBetweenGC = 5000;
    static constexpr size_t kMinMemBasedGC                 = 1024 * 1024;
    static constexpr size_t kMemAllocedBetweenGC           = 64 * 1024 * 1024;
    angle::Result queueBlitCopyDataToBuffer(ContextMtl *contextMtl,
                                            const void *srcPtr,
                                            size_t sizeToCopy,
                                            size_t offset,
                                            mtl::BufferRef &dstMetalBuffer);

    angle::Result getBuffer(ContextMtl *contextMtl,
                            MTLStorageMode storageMode,
                            size_t size,
                            mtl::BufferRef &bufferRef);
    void returnBuffer(ContextMtl *contextMtl, mtl::BufferRef &bufferRef);

    void incrementNumContextSwitches();
    void incrementNumCommandBufferCommits();

  private:
    typedef std::vector<mtl::BufferRef> BufferList;
    typedef std::multimap<size_t, mtl::BufferRef> BufferMap;
    enum class GCReason
    {
        ContextSwitches,
        CommandBufferCommits,
        TotalMem
    };

    void freeUnusedBuffers(ContextMtl *contextMtl);
    void addBufferRefToFreeLists(mtl::BufferRef &bufferRef);
    void collectGarbage(GCReason reason);

    BufferList mInUseBuffers;

    BufferMap mFreeBuffers[kNumCachedStorageModes];

    // For garbage collecting expired buffer shadow copies
    size_t mContextSwitches              = 0;
    size_t mContextSwitchesAtLastGC      = 0;
    size_t mCommandBufferCommits         = 0;
    size_t mCommandBufferCommitsAtLastGC = 0;
    size_t mTotalMem                     = 0;
    size_t mTotalMemAtLastGC             = 0;

#ifdef ANGLE_MTL_TRACK_BUFFER_MEM
    std::map<size_t, size_t> mAllocatedSizes;
#endif
};

}  // namespace mtl
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_MTL_BUFFER_MANAGER_H_ */
