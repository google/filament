/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "VulkanDescriptorSetCache.h"

#include "VulkanCommands.h"
#include "VulkanConstants.h"
#include "VulkanHandles.h"

#include "vulkan/memory/ResourcePointer.h"
#include "vulkan/utils/Definitions.h"

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#include <algorithm>
#include <array>
#include <memory>
#include <utility>
#include <vector>

namespace filament::backend {

namespace {

using Bitmask = fvkutils::UniformBufferBitmask;
static_assert(sizeof(Bitmask) * 8 == fvkutils::MAX_DESCRIPTOR_SET_BITMASK_BITS);

Bitmask foldBitsInHalf(Bitmask bitset) {
    Bitmask outBitset;
    bitset.forEachSetBit([&](size_t index) {
        constexpr size_t BITMASK_LOWER_BITS_LEN = sizeof(outBitset) * 4;
        outBitset.set(index % BITMASK_LOWER_BITS_LEN);
    });
    return outBitset;
}

using DescriptorSetLayoutArray = VulkanDescriptorSetCache::DescriptorSetLayoutArray;
using DescriptorCount = VulkanDescriptorSetCache::DescriptorCount;

// We create a pool for each layout as defined by the number of descriptors of each type. For
// example, a layout of
// 'A' =>
//   layout(binding = 0, set = 1) uniform {};
//   layout(binding = 1, set = 1) sampler1;
//   layout(binding = 2, set = 1) sampler2;
//
// would be equivalent to
// 'B' =>
//   layout(binding = 1, set = 2) uniform {};
//   layout(binding = 2, set = 2) sampler2;
//   layout(binding = 3, set = 2) sampler3;
//
// TODO: we might do better if we understand the types of unique layouts and can combine them in a
// single pool without too much waste.
class DescriptorPool {
public:
    DescriptorPool(VkDevice device,  DescriptorCount const& count, uint16_t capacity)
        : mDevice(device),
          mCount(count),
          mCapacity(capacity),
          mSize(0),
          mUnusedCount(0) {
        DescriptorCount const actual = mCount * capacity;
        VkDescriptorPoolSize sizes[4];
        uint8_t npools = 0;
        if (actual.ubo) {
            sizes[npools++] = {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = actual.ubo,
            };
        }
        if (actual.dynamicUbo) {
            sizes[npools++] = {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                .descriptorCount = actual.dynamicUbo,
            };
        }
        if (actual.sampler) {
            sizes[npools++] = {
              .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              .descriptorCount = actual.sampler,
            };
        }
        if (actual.inputAttachment) {
            sizes[npools++] = {
                .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                .descriptorCount = actual.inputAttachment,
            };
        }
        if (UTILS_VERY_UNLIKELY(npools == 0)) {
            // Vulkan requires poolSizeCount > 0 (VUID-VkDescriptorPoolCreateInfo-poolSizeCount-arraylength).
            // A broken or malicious material can produce an empty descriptor set layout; provide a
            // minimal pool size so vkCreateDescriptorPool and vkAllocateDescriptorSets succeed.
            sizes[npools++] = {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
            };
        }
        VkDescriptorPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = capacity,
            .poolSizeCount = npools,
            .pPoolSizes = sizes,
        };
        vkCreateDescriptorPool(mDevice, &info, VKALLOC, &mPool);
    }

    DescriptorPool(DescriptorPool const&) = delete;
    DescriptorPool& operator=(DescriptorPool const&) = delete;

    ~DescriptorPool() {
        vkDestroyDescriptorPool(mDevice, mPool, VKALLOC);
    }

    uint16_t const& capacity() {
        return mCapacity;
    }

    // A convenience method for checking if this pool can allocate sets for a given layout.
    inline bool canAllocate(DescriptorCount const& count) {
        return count == mCount;
    }

    VkDescriptorSet obtainSet(VkDescriptorSetLayout vklayout) {
        auto itr = findSets(vklayout);
        if (itr != mUnused.end()) {
            // If we don't have any unused, then just return an empty handle.
            if (itr->second.empty()) {
                return VK_NULL_HANDLE;
            }
            std::vector<VkDescriptorSet>& sets = itr->second;
            auto set = sets.back();
            sets.pop_back();
            mUnusedCount--;
            return set;
        }
        if (mSize + 1 > mCapacity) {
            return VK_NULL_HANDLE;
        }
        // Creating a new set
        VkDescriptorSetLayout layouts[1] = {vklayout};
        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = mPool,
            .descriptorSetCount = 1,
            .pSetLayouts = layouts,
        };
        VkDescriptorSet vkSet;
        UTILS_UNUSED VkResult result = vkAllocateDescriptorSets(mDevice, &allocInfo, &vkSet);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
                << "Failed to allocate descriptor set code=" << result << " size=" << mSize
                << " capacity=" << mCapacity << " count=" << mUnusedCount;
        mSize++;
        return vkSet;
    }

    void recycle(VkDescriptorSetLayout vklayout, VkDescriptorSet vkSet) {
        // We are recycling - release the set back into the pool. Note that the
        // vk handle has not changed, but we need to change the backend handle to allow
        // for proper refcounting of resources referenced in this set.
        auto itr = findSets(vklayout);
        if (itr != mUnused.end()) {
            itr->second.push_back(vkSet);
        } else {
            mUnused.push_back(std::make_pair(vklayout, std::vector<VkDescriptorSet> {vkSet}));
        }
        mUnusedCount++;
    }

private:
    using UnusedSets = std::pair<VkDescriptorSetLayout, std::vector<VkDescriptorSet>>;
    using UnusedSetMap = std::vector<UnusedSets>;

    inline UnusedSetMap::iterator findSets(VkDescriptorSetLayout vklayout) {
        return std::find_if(mUnused.begin(), mUnused.end(), [vklayout](auto const& value) {
            return value.first == vklayout;
        });
    }

    VkDevice mDevice;
    VkDescriptorPool mPool;
    DescriptorCount const mCount;
    uint16_t const mCapacity;

    // Tracks the number of allocated descriptor sets.
    uint16_t mSize;
    // Tracks  the number of in-use descriptor sets.
    uint16_t mUnusedCount;

    // This maps a layout to a list of descriptor sets allocated for that layout.
    UnusedSetMap mUnused;
};

template<typename Key>
struct Equal {
    bool operator()(Key const& k1, Key const& k2) const {
        return 0 == memcmp((const void*) &k1, (const void*) &k2, sizeof(Key));
    }
};

} // anonymous namespace

// This is an ever-expanding pool of sets where it
//    1. Keeps a list of smaller pools of different layout-dimensions.
//    2. Will add a pool if existing pool are not compatible with the requested layout o runs out.
class VulkanDescriptorSetCache::DescriptorInfinitePool {
private:
    static constexpr uint16_t EXPECTED_SET_COUNT = 10;
    static constexpr float SET_COUNT_GROWTH_FACTOR = 1.5;

public:
    DescriptorInfinitePool(VkDevice device)
        : mDevice(device) {}

    VkDescriptorSet obtainSet(DescriptorCount const& count, VkDescriptorSetLayout vklayout) {
        DescriptorPool* sameTypePool = nullptr;
        for (auto& pool: mPools) {
            if (!pool->canAllocate(count)) {
                continue;
            }
            if (auto set = pool->obtainSet(vklayout); set != VK_NULL_HANDLE) {
                return set;
            }
            if (!sameTypePool || sameTypePool->capacity() < pool->capacity()) {
                sameTypePool = pool.get();
            }
        }

        uint16_t capacity = EXPECTED_SET_COUNT;
        if (sameTypePool) {
            // Exponentially increase the size of the pool  to ensure we don't hit this too often.
            capacity = std::ceil(sameTypePool->capacity() * SET_COUNT_GROWTH_FACTOR);
        }

        // We need to increase the set of pools by one.
        mPools.push_back(std::make_unique<DescriptorPool>(mDevice, count, capacity));
        auto& pool = mPools.back();
        auto ret = pool->obtainSet(vklayout);
        assert_invariant(ret != VK_NULL_HANDLE && "failed to obtain a set?");
        return ret;
    }

    void recycle(DescriptorCount const& count, VkDescriptorSetLayout vklayout,
            VkDescriptorSet vkSet) {
        for (auto& pool: mPools) {
            if (!pool->canAllocate(count)) {
                continue;
            }
            pool->recycle(vklayout, vkSet);
            break;
        }
    }

private:
    VkDevice mDevice;
    std::vector<std::unique_ptr<DescriptorPool>> mPools;
};


VulkanDescriptorSetCache::VulkanDescriptorSetCache(VkDevice device,
        fvkmemory::ResourceManager* resourceManager,
        fvkutils::CommandBufferAgeCounter const& ageCounter)
    : mDevice(device),
      mResourceManager(resourceManager),
      mAgeCounter(ageCounter),
      mDescriptorPool(std::make_unique<DescriptorInfinitePool>(device)) {}

VulkanDescriptorSetCache::~VulkanDescriptorSetCache() = default;

void VulkanDescriptorSetCache::terminate() noexcept{
    mDescriptorPool.reset();
    mStashedSets = {};
    mLastBoundInfo = {};
}

// bind() is not really binding the set but just stashing until we have all the info
// (pipelinelayout).
void VulkanDescriptorSetCache::bind(uint8_t setIndex,
        fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        backend::DescriptorSetOffsetArray&& offsets) {
    set->setOffsets(std::move(offsets));
    mStashedSets[setIndex] = set;
}

void VulkanDescriptorSetCache::unbind(uint8_t setIndex) {
    mStashedSets[setIndex] = {};
}

void VulkanDescriptorSetCache::commit(VulkanCommandBuffer* commands,
        VkPipelineLayout pipelineLayout, fvkutils::DescriptorSetMask const& setMask) {
    auto& last = mLastBoundInfo;

    // Bindings do not carry over to another command buffer. Rather than relying on every caller
    // that switches command buffers (a flush, or the switch between the regular and the protected
    // command buffer) to call resetCachedState(), detect the switch here. Ages are unique across
    // both command buffer pools.
    if (UTILS_UNLIKELY(last.age != commands->age())) {
        last = {};
        last.age = commands->age();
    }

    bool const sameLayout = last.pipelineLayout == pipelineLayout;

    // The set this draw executes with at `index`: the one stashed by bind() since the last commit,
    // or else the one left bound by an earlier commit.
    //
    // The frontend does not re-bind a set that it expects to still be bound, even across a
    // pipeline layout change (e.g. the per-view set in the shadow, structure, SSR, and
    // post-process passes, see RenderPass::Executor::execute()). This relies on the Vulkan
    // pipeline layout compatibility rules, which keep set N bound as long as the layouts are
    // compatible for set N. Carrying these sets forward regardless of the layout matters for the
    // stale-UBO check below: a UBO can be swapped out between two draws without the frontend
    // re-binding its set, and the draw would otherwise run with a descriptor pointing at the old
    // VkBuffer.
    auto const getSet =
            [&](size_t index) -> fvkmemory::resource_ptr<VulkanDescriptorSet> const& {
        return mStashedSets[index] ? mStashedSets[index] : last.boundSets[index];
    };

    // First, re-point the stale UBO descriptors of every set this draw uses. This switches each
    // such set to a new VkDescriptorSet, which the comparison below then picks up. Doing this for
    // all indices before deciding what to bind matters if the same set is used at two indices:
    // refreshing it for one index must also cause it to be re-bound at the other.
    setMask.forEachSetBit([&](size_t index) {
        auto const& set = getSet(index);
        if (!set) {
            return;
        }
        if (UTILS_UNLIKELY(set->getStaleUbos().any())) {
            // The VkBuffer of some UBOs has been swapped out (due to the memcpy fast path).
            // Cloning the set rewrites their descriptors with the current VkBuffer.
            cloneSet(set, {} /* samplerMask */);
        }
    });

    // Then work out which sets need to be bound. setMask indicates the sets the draw uses, and
    // bindMask the ones that actually need a vkCmdBindDescriptorSets. This must go in ascending
    // order for `rebindHigher` to work.
    fvkutils::DescriptorSetMask bindMask;
    bool rebindHigher = false;
    setMask.forEachSetBit([&](size_t index) {
        auto const& set = getSet(index);
        if (!set) {
            return;
        }
        // Compare the VkDescriptorSet rather than the wrapper, since cloneSet() (for external
        // samplers or stale UBOs) changes the VkDescriptorSet of a wrapper in place.
        bool const sameVkSet = set->getVkSet() == last.boundVkSets[index];
        bool needsBind;
        if (mStashedSets[index]) {
            // Sets with dynamic UBOs are always re-bound, since bind() may have changed their
            // offsets.
            bool const alreadyBound = sameLayout && last.boundWithLayout[index] && sameVkSet;
            needsBind = !alreadyBound || set->uniqueDynamicUboCount > 0;
        } else {
            // Still bound from an earlier commit, with unchanged offsets. It only needs to be
            // re-bound if its VkDescriptorSet has changed (i.e. stale UBOs were refreshed).
            needsBind = !sameVkSet;
            // Binding set N with a pipeline layout other than the one the higher sets were bound
            // with may disturb them, so re-bind those too. Lower sets are not disturbed, since
            // the frontend relies on the layouts being compatible for set N.
            rebindHigher |= needsBind && !sameLayout;
        }
        if (needsBind || rebindHigher) {
            bindMask.set(index);
        }
    });

    VkCommandBuffer const cmdbuffer = commands->buffer();
    bindMask.forEachSetBit([&](size_t index) {
        // Copy, since getSet() may return a reference to last.boundSets[index], which is
        // overwritten below.
        fvkmemory::resource_ptr<VulkanDescriptorSet> set = getSet(index);
        VkDescriptorSet const vkset = set->getVkSet();
        commands->acquire(set);
        vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, index,
                1, &vkset, set->uniqueDynamicUboCount, set->getOffsets()->data());
        set->referencedBy(*commands, mAgeCounter);
        last.boundSets[index] = set;
        last.boundVkSets[index] = vkset;
    });

    last.boundWithLayout = sameLayout ? (last.boundWithLayout | bindMask) : bindMask;
    last.pipelineLayout = pipelineLayout;
    mStashedSets = {};
}

void VulkanDescriptorSetCache::updateBuffer(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        uint8_t binding, fvkmemory::resource_ptr<VulkanBufferObject> bufferObject,
        VkDeviceSize offset, VkDeviceSize size) noexcept {
    auto const& bitmask = set->getLayout()->bitmask;
    size_t const vbit = binding + fvkutils::getVertexStageShift<fvkutils::UniformBufferBitmask>();
    size_t const fbit = binding + fvkutils::getFragmentStageShift<fvkutils::UniformBufferBitmask>();
    bool const isDynamic = set->dynamicUboMask.test(vbit) || set->dynamicUboMask.test(fbit);
    bool const isStatic = bitmask.ubo.test(vbit) || bitmask.ubo.test(fbit);
    if (UTILS_VERY_UNLIKELY(!isDynamic && !isStatic)) {
        return;
    }

    VkDescriptorBufferInfo const info = {
        .buffer = bufferObject->getVkBuffer(),
        .offset = offset,
        .range = size,
    };
    VkDescriptorType const type = isDynamic
            ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
            : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VkWriteDescriptorSet descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set->getVkSet(),
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &info,
    };
    vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
    set->acquire(binding, bufferObject, offset, size);
}

void VulkanDescriptorSetCache::updateSampler(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        uint8_t binding, fvkmemory::resource_ptr<VulkanTexture> texture,
        VkSampler sampler) noexcept {
    auto const& samplerMask = set->getLayout()->bitmask.sampler;
    size_t const vbit = binding + fvkutils::getVertexStageShift<fvkutils::SamplerBitmask>();
    size_t const fbit = binding + fvkutils::getFragmentStageShift<fvkutils::SamplerBitmask>();
    if (UTILS_VERY_UNLIKELY(!samplerMask.test(vbit) && !samplerMask.test(fbit))) {
        return;
    }

    VkDescriptorSet const vkset = set->getVkSet();
    VkImageSubresourceRange const range = texture->getPrimaryViewRange();
    VkDescriptorImageInfo info = {
        .sampler = sampler,
        .imageView = texture->getView(range),
        .imageLayout = fvkutils::getVkLayout(texture->getSamplerLayout()),
    };
    VkWriteDescriptorSet descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = vkset,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &info,
    };
    vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
    set->acquire(binding, texture);
}

void VulkanDescriptorSetCache::updateInputAttachment(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        VulkanAttachment const& attachment) noexcept {
    // TOOD: fill this in.
}


fvkmemory::resource_ptr<VulkanDescriptorSet> VulkanDescriptorSetCache::createSet(
        Handle<HwDescriptorSet> handle, fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout) {
    auto const vkSet = mDescriptorPool->obtainSet(layout->count, layout->getVkLayout());
    auto const& count = layout->count;
    auto const vklayout = layout->getVkLayout();
    auto set = fvkmemory::resource_ptr<VulkanDescriptorSet>::make(
            mResourceManager, handle, layout,
            [vkSet, count, vklayout, this](
                    VulkanDescriptorSet*) { this->manualRecycle(count, vklayout, vkSet); },
            vkSet);
    return set;
}

void VulkanDescriptorSetCache::cloneSet(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        fvkutils::SamplerBitmask samplerMask) noexcept {
    using Entry = VulkanDescriptorSet::BoundResource;
    constexpr size_t vshift = fvkutils::getVertexStageShift<Bitmask>();
    constexpr size_t fshift = fvkutils::getFragmentStageShift<Bitmask>();

    auto const& layout = set->getLayout();
    // Build a new descriptor set from layout
    VkDescriptorSetLayout const genLayout = set->boundLayout;
    VkDescriptorSet const newSet = getVkSet(layout->count, genLayout);

    // The uniform buffers whose VkBuffer has been swapped out (due to the memcpy fast path) since
    // they were written. Their descriptors must not be copied, since the old VkBuffer may already
    // have been recycled or destroyed. They are rewritten with the current VkBuffer below instead.
    // getStaleUbos() returns indices into mResources, so convert them to layout space, where each
    // binding has a bit per stage, and mask out both stages.
    fvkutils::UniformBufferBitmask const staleUbos = set->getStaleUbos();
    Bitmask staleBindings;
    staleUbos.forEachSetBit([&](size_t index) {
        uint8_t const binding = set->mResources[index].binding;
        staleBindings.set(binding + vshift);
        staleBindings.set(binding + fshift);
    });

    // Each bitmask denotes a binding index, and separated into two stages - vertex and buffer
    // We fold the two stages into just the lower half of the bits to denote a combined set of
    // bindings.
    // Don't copy the stale ubos, nor the samplers in the mask.
    Bitmask const ubo = (layout->bitmask.ubo | layout->bitmask.dynamicUbo) & ~staleBindings;
    Bitmask const samplers = layout->bitmask.sampler & ~samplerMask;
    // Only copy the bindings that have actually been written. The layout may declare bindings
    // that are never written, and their source descriptors are undefined.
    Bitmask const copyBindings = foldBitsInHalf(ubo | samplers) & set->mWrittenBindings;

    VkDescriptorSet const srcSet = set->getVkSet();
    copySet(srcSet, newSet, copyBindings);
    set->addNewSet(newSet,
            [this, layoutCount = layout->count, genLayout, newSet](
                    VulkanDescriptorSet*) { this->manualRecycle(layoutCount, genLayout, newSet); },
            mAgeCounter);

    // updateBuffer() writes into the new (now current) VkDescriptorSet.
    staleUbos.forEachSetBit([&](size_t index) {
        // Copy the entry, since updateBuffer() overwrites it with the current generation.
        Entry const entry = set->mResources[index];
        auto bo = fvkmemory::resource_ptr<VulkanBufferObject>::cast(
                static_cast<VulkanBufferObject*>(entry.resource.get()));
        // Rewrite the descriptor with the range it was originally given, so that subranges and
        // dynamic offsets keep working.
        updateBuffer(set, entry.binding, bo, entry.offset, entry.size);
    });
}

VkDescriptorSet VulkanDescriptorSetCache::getVkSet(DescriptorCount const& count,
        VkDescriptorSetLayout vklayout) {
    return mDescriptorPool->obtainSet(count, vklayout);
}

void VulkanDescriptorSetCache::manualRecycle(VulkanDescriptorSetLayout::Count const& count,
        VkDescriptorSetLayout vklayout, VkDescriptorSet vkSet) {
    // Note that mDescriptorPool could be gone due to terminate (when the backend shuts
    // down).
    if (mDescriptorPool) {
        mDescriptorPool->recycle(count, vklayout, vkSet);
    }
}

void VulkanDescriptorSetCache::gc() { mStashedSets = {}; }

void VulkanDescriptorSetCache::copySet(VkDescriptorSet srcSet, VkDescriptorSet dstSet,
        fvkutils::SamplerBitmask bindings) const {
    // `bindings` is in folded (per-binding) space, so it has at most MAX_BINDINGS set bits. This
    // runs on every clone (external samplers and stale-UBO refreshes), so avoid a heap allocation.
    std::array<VkCopyDescriptorSet, VulkanDescriptorSetLayout::MAX_BINDINGS> copies;
    uint32_t count = 0;
    bindings.forEachSetBit([&](size_t index) {
        assert_invariant(count < copies.size());
        copies[count++] = {
            .sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET,
            .srcSet = srcSet,
            .srcBinding = (uint32_t) index,
            .dstSet = dstSet,
            .dstBinding = (uint32_t) index,
            .descriptorCount = 1,
        };
    });
    if (count == 0) {
        return;
    }
    vkUpdateDescriptorSets(mDevice, 0, nullptr, count, copies.data());
}


} // namespace filament::backend
