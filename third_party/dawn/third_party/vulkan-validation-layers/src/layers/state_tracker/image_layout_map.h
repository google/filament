/* Copyright (c) 2019-2024 The Khronos Group Inc.
 * Copyright (c) 2019-2024 Valve Corporation
 * Copyright (c) 2019-2024 LunarG, Inc.
 * Copyright (C) 2019-2024 Google Inc.
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
 *
 * John Zulauf <jzulauf@lunarg.com>
 *
 */
#pragma once

#include <functional>

#include "containers/range_vector.h"
#include "containers/subresource_adapter.h"
#include "utils/vk_layer_utils.h"
#include "vulkan/vulkan.h"
#include "error_message/logging.h"

namespace vvl {
class Image;
class ImageView;
class CommandBuffer;
}  // namespace vvl

namespace image_layout_map {
const static VkImageLayout kInvalidLayout = VK_IMAGE_LAYOUT_MAX_ENUM;

// Common types for this namespace
using IndexType = subresource_adapter::IndexType;
using IndexRange = sparse_container::range<IndexType>;
using Encoder = subresource_adapter::RangeEncoder;
using RangeGenerator = subresource_adapter::RangeGenerator;

struct InitialLayoutState {
    VkImageView image_view;          // For relaxed matching rule evaluation, else VK_NULL_HANDLE
    VkImageAspectFlags aspect_mask;  // For relaxed matching rules... else 0
    LoggingLabel label;
    InitialLayoutState(const vvl::CommandBuffer& cb_state_, const vvl::ImageView* view_state_);
    InitialLayoutState() : image_view(VK_NULL_HANDLE), aspect_mask(0), label() {}
};

// Contains all info around an image, its subresources and layout map
class ImageLayoutRegistry {
  public:
    typedef std::function<bool(const VkImageSubresource&, VkImageLayout, VkImageLayout)> Callback;

    struct SubresourceLayout {
        VkImageSubresource subresource;
        VkImageLayout current_layout;
        VkImageLayout initial_layout;

        bool operator==(const SubresourceLayout& rhs) const;
        bool operator!=(const SubresourceLayout& rhs) const { return !(*this == rhs); }
        SubresourceLayout(const VkImageSubresource& subresource_, VkImageLayout current_layout_, VkImageLayout initial_layout_)
            : subresource(subresource_), current_layout(current_layout_), initial_layout(initial_layout_) {}
        SubresourceLayout() = default;
    };

    struct LayoutEntry {
        VkImageLayout initial_layout;
        VkImageLayout current_layout;
        InitialLayoutState* state;

        LayoutEntry(VkImageLayout initial_ = kInvalidLayout, VkImageLayout current_ = kInvalidLayout,
                    InitialLayoutState* s = nullptr)
            : initial_layout(initial_), current_layout(current_), state(s) {}

        bool operator!=(const LayoutEntry& rhs) const {
            return initial_layout != rhs.initial_layout || current_layout != rhs.current_layout || state != rhs.state;
        }
        bool CurrentWillChange(VkImageLayout new_layout) const {
            return new_layout != kInvalidLayout && current_layout != new_layout;
        }
        bool Update(const LayoutEntry& src) {
            bool updated_current = false;
            // current_layout can be updated repeatedly.
            if (CurrentWillChange(src.current_layout)) {
                current_layout = src.current_layout;
                updated_current = true;
            }
            // initial_layout and state cannot be updated once they have a valid value.
            if (initial_layout == kInvalidLayout) {
                initial_layout = src.initial_layout;
            }
            if (state == nullptr) {
                state = src.state;
            }
            return updated_current;
        }
        // updater for splice()
        struct Updater {
            bool update(LayoutEntry& dst, const LayoutEntry& src) const { return dst.Update(src); }
            std::optional<LayoutEntry> insert(const LayoutEntry& src) const {
                return std::optional<LayoutEntry>(vvl::in_place, src);
            }
        };
    };
    using InitialLayoutStates = small_vector<InitialLayoutState, 2, uint32_t>;
    using LayoutMap = subresource_adapter::BothRangeMap<LayoutEntry, 16>;
    using RangeType = LayoutMap::key_type;

    bool SetSubresourceRangeLayout(const vvl::CommandBuffer& cb_state, const VkImageSubresourceRange& range, VkImageLayout layout,
                                   VkImageLayout expected_layout = kInvalidLayout);
    void SetSubresourceRangeInitialLayout(const vvl::CommandBuffer& cb_state, const VkImageSubresourceRange& range,
                                          VkImageLayout layout);
    void SetSubresourceRangeInitialLayout(const vvl::CommandBuffer& cb_state, VkImageLayout layout,
                                          const vvl::ImageView& view_state);
    bool UpdateFrom(const ImageLayoutRegistry& from);
    uintptr_t CompatibilityKey() const;
    const LayoutMap& GetLayoutMap() const { return layout_map_; }
    ImageLayoutRegistry(const vvl::Image& image_state);
    ~ImageLayoutRegistry() {}
    uint32_t GetImageId() const;

    // This looks a bit ponderous but kAspectCount is a compile time constant
    VkImageSubresource Decode(IndexType index) const {
        const auto subres = encoder_.Decode(index);
        return encoder_.MakeVkSubresource(subres);
    }

    RangeGenerator RangeGen(const VkImageSubresourceRange& subres_range) const {
        if (encoder_.InRange(subres_range)) {
            return (RangeGenerator(encoder_, subres_range));
        }
        // Return empty range generator
        return RangeGenerator();
    }

    bool AnyInRange(const VkImageSubresourceRange& normalized_range,
                    std::function<bool(const RangeType& range, const LayoutEntry& state)>&& func) const {
        return AnyInRange(RangeGen(normalized_range), std::move(func));
    }

    bool AnyInRange(const RangeGenerator& gen, std::function<bool(const RangeType& range, const LayoutEntry& state)>&& func) const {
        return AnyInRange(RangeGenerator(gen), std::move(func));
    }

    bool AnyInRange(RangeGenerator&& gen, std::function<bool(const RangeType& range, const LayoutEntry& state)>&& func) const {
        for (; gen->non_empty(); ++gen) {
            for (auto pos = layout_map_.lower_bound(*gen); (pos != layout_map_.end()) && (gen->intersects(pos->first)); ++pos) {
                if (func(pos->first, pos->second)) {
                    return true;
                }
            }
        }
        return false;
    }

  protected:
    bool InRange(const VkImageSubresource& subres) const { return encoder_.InRange(subres); }
    bool InRange(const VkImageSubresourceRange& range) const { return encoder_.InRange(range); }

  private:
    const vvl::Image& image_state_;
    const Encoder& encoder_;
    LayoutMap layout_map_;
    InitialLayoutStates initial_layout_states_;
};
}  // namespace image_layout_map

class GlobalImageLayoutRangeMap : public subresource_adapter::BothRangeMap<VkImageLayout, 16> {
  public:
    using RangeGenerator = image_layout_map::RangeGenerator;
    using RangeType = key_type;

    GlobalImageLayoutRangeMap(index_type index) : BothRangeMap<VkImageLayout, 16>(index) {}
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    bool AnyInRange(RangeGenerator& gen, std::function<bool(const key_type& range, const mapped_type& state)>&& func) const;

  private:
    mutable std::shared_mutex lock_;
};
