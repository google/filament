/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_DRIVER_METALSTAGEPOOL_H
#define TNT_FILAMENT_DRIVER_METALSTAGEPOOL_H

#include <Metal/Metal.h>

#include <map>
#include <mutex>
#include <unordered_set>

namespace filament {
namespace backend {
namespace metal {

struct MetalContext;

// Immutable POD representing a shared CPU-GPU buffer.
struct MetalBufferPoolEntry {
    id<MTLBuffer> buffer;
    size_t capacity;
    mutable uint64_t lastAccessed;
    mutable uint32_t referenceCount;
};

// Manages a pool of Metal buffers, periodically releasing ones that have been unused for awhile.
class MetalBufferPool {
public:
    explicit MetalBufferPool(MetalContext& context) noexcept : mContext(context) {}

    // Finds or creates a buffer whose capacity is at least the given number of bytes.
    MetalBufferPoolEntry const* acquireBuffer(size_t numBytes);

    // Increments the reference count of the buffer.
    void retainBuffer(MetalBufferPoolEntry const *stage) noexcept;

    // Decrements the reference count of the buffer, returning the given buffer back to the pool if
    // the count is 0.
    void releaseBuffer(MetalBufferPoolEntry const *stage) noexcept;

    // Evicts old unused buffers and bumps the current frame number.
    void gc() noexcept;

    // Destroys all unused buffers.
    void reset() noexcept;

private:
    MetalContext& mContext;

    // Synchronizes access to mFreeStages, mUsedStages, and mutable data inside MetalBufferPoolEntrys.
    // acquireBuffer and releaseBuffer may be called on separate threads (the engine thread and a
    // Metal callback thread, for example).
    std::mutex mMutex;

    // Use an ordered multimap for quick (capacity => stage) lookups using lower_bound().
    std::multimap<size_t, MetalBufferPoolEntry const*> mFreeStages;

    // Simple unordered set for stashing a list of in-use stages that can be reclaimed later.
    // In theory this need not exist, but is useful for validation and ensuring no leaks.
    std::unordered_set<MetalBufferPoolEntry const*> mUsedStages;

    // Store the current "time" (really just a frame count) and LRU eviction parameters.
    uint64_t mCurrentFrame = 0;
    static constexpr uint32_t TIME_BEFORE_EVICTION = 10;
};

} // namespace metal
} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_METALSTAGEPOOL_H
