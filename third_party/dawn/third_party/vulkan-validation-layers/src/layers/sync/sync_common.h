/*
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include "containers/subresource_adapter.h"
#include "containers/range_vector.h"
#include "generated/sync_validation_types.h"
#include <set>

namespace vvl {
class Buffer;
class BufferView;
struct VertexBufferBinding;
struct IndexBufferBinding;
}  // namespace vvl

namespace syncval_state {
class CommandBuffer;
class ImageState;
class ImageViewState;
class Swapchain;
}  // namespace syncval_state

class HazardResult;
class SyncValidator;

using ImageRangeGen = subresource_adapter::ImageRangeGenerator;

// The resource tag index is relative to the command buffer or queue in which it's found
using QueueId = uint32_t;
constexpr static QueueId kQueueIdInvalid = QueueId(vvl::kU32Max);
constexpr static QueueId kQueueAny = kQueueIdInvalid - 1;

using ResourceUsageTag = size_t;
constexpr static ResourceUsageTag kMaxIndex = std::numeric_limits<ResourceUsageTag>::max();
constexpr static ResourceUsageTag kInvalidTag = kMaxIndex;

using ResourceUsageRange = sparse_container::range<ResourceUsageTag>;
using ResourceAddress = VkDeviceSize;
using ResourceAccessRange = sparse_container::range<ResourceAddress>;

// Usage tag extended with resource handle information
struct ResourceUsageTagEx {
    ResourceUsageTag tag = kInvalidTag;
    uint32_t handle_index = vvl::kNoIndex32;
};

template <typename T>
ResourceAccessRange MakeRange(const T &has_offset_and_size) {
    return ResourceAccessRange(has_offset_and_size.offset, (has_offset_and_size.offset + has_offset_and_size.effective_size));
}
ResourceAccessRange MakeRange(VkDeviceSize start, VkDeviceSize size);
ResourceAccessRange MakeRange(const vvl::Buffer &buffer, VkDeviceSize offset, VkDeviceSize size);
ResourceAccessRange MakeRange(const vvl::BufferView &buf_view_state);
ResourceAccessRange MakeRange(VkDeviceSize offset, uint32_t first_index, uint32_t count, uint32_t stride);

extern const ResourceAccessRange kFullRange;

constexpr VkImageAspectFlags kColorAspects =
    VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
constexpr VkImageAspectFlags kDepthStencilAspects = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

// Useful Utilites for manipulating StageAccess parameters, suitable as base class to save typing
struct SyncStageAccess {
    static inline const SyncAccessInfo &AccessInfo(SyncAccessIndex access_index) {
        return syncAccessInfoByAccessIndex()[access_index];
    }
    static inline SyncAccessFlags FlagBit(SyncAccessIndex stage_access) {
        return syncAccessInfoByAccessIndex()[stage_access].access_bit;
    }

    static bool IsRead(SyncAccessIndex access_index) { return syncAccessReadMask[access_index]; }
    static bool IsRead(const SyncAccessInfo &info) { return IsRead(info.access_index); }
    static bool IsWrite(SyncAccessIndex access_index) { return syncAccessWriteMask[access_index]; }
    static bool IsWrite(const SyncAccessInfo &info) { return IsWrite(info.access_index); }

    static VkPipelineStageFlags2 PipelineStageBit(SyncAccessIndex access_index) {
        return syncAccessInfoByAccessIndex()[access_index].stage_mask;
    }
    static SyncAccessFlags AccessScopeByStage(VkPipelineStageFlags2 stages);
    static SyncAccessFlags AccessScopeByAccess(VkAccessFlags2 access);
    static SyncAccessFlags AccessScope(VkPipelineStageFlags2 stages, VkAccessFlags2 access);
    static SyncAccessFlags AccessScope(const SyncAccessFlags &stage_scope, VkAccessFlags2 accesses) {
        return stage_scope & AccessScopeByAccess(accesses);
    }
};

// Notes:
//  * Design goal is performance optimized set creation during specific SyncVal operations
//  * Key must be integral.
//  * We aren't interested as of this implementation in caching lookups, only inserts
//  * using a raw C-style array instead of std::array intentionally for size/performance reasons
//
// The following were shown to not improve hit rate for current usage (tag set gathering).  For general use YMMV.
//  * More complicated index construction (at >> LogSize ^ at)
//  * Multi-way LRU eviction caching (equivalent hit rate to 1-way direct replacement of same total cache slots) but with
//    higher complexity.
template <typename IntegralKey, size_t LogSize = 4U, IntegralKey kInvalidKey = IntegralKey(0)>
class CachedInsertSet : public std::set<IntegralKey> {
  public:
    using Base = std::set<IntegralKey>;
    using key_type = typename Base::key_type;
    using Index = unsigned;
    static constexpr Index kSize = 1 << LogSize;
    static constexpr key_type kMask = static_cast<key_type>(kSize) - 1;

    void CachedInsert(const key_type key) {
        // 1-way direct replacement
        const Index index = static_cast<Index>(key & kMask);  // Simplest

        if (entries_[index] != key) {
            entries_[index] = key;
            Base::insert(key);
        }
    }

    CachedInsertSet() { std::fill(entries_, entries_ + kSize, kInvalidKey); }

  private:
    key_type entries_[kSize];
};

// A wrapper for a single range with the same semantics as other non-trivial range generators
template <typename KeyType>
class SingleRangeGenerator {
  public:
    using RangeType = KeyType;
    SingleRangeGenerator(const KeyType &range) : current_(range) {}
    const KeyType &operator*() const { return current_; }
    const KeyType *operator->() const { return &current_; }
    SingleRangeGenerator &operator++() {
        current_ = KeyType();  // just one real range
        return *this;
    }

    bool operator==(const SingleRangeGenerator &other) const { return current_ == other.current_; }

  private:
    SingleRangeGenerator() = default;
    const KeyType range_;
    KeyType current_;
};
