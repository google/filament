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
#include "VulkanHandles.h"
#include "VulkanConstants.h"

#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#include <algorithm>
#include <memory>
#include <vector>

namespace filament::backend {

namespace {

using DescriptorCount = VulkanDescriptorSetLayout::Count;
using DescriptorSetLayoutArray = VulkanDescriptorSetCache::DescriptorSetLayoutArray;

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

    VkDescriptorSet obtainSet(fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout) {
        auto const vklayout = layout->getVkLayout();
        DescriptorPool* sameTypePool = nullptr;
        for (auto& pool: mPools) {
            if (!pool->canAllocate(layout->count)) {
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
        mPools.push_back(std::make_unique<DescriptorPool>(mDevice,
                DescriptorCount::fromLayoutBitmask(layout->bitmask), capacity));
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
        fvkmemory::ResourceManager* resourceManager)
    : mDevice(device),
      mResourceManager(resourceManager),
      mDescriptorPool(std::make_unique<DescriptorInfinitePool>(device)) {}

VulkanDescriptorSetCache::~VulkanDescriptorSetCache() = default;

void VulkanDescriptorSetCache::terminate() noexcept{
    mDescriptorPool.reset();
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
    // setMask indicates the set of descriptor sets the driver wants to bind, curMask is the
    // actual set of sets that *needs* to be bound.
    fvkutils::DescriptorSetMask curMask = setMask;

    auto& updateSets = mStashedSets;
    bool const pipelineLayoutIsSame = mLastBoundInfo.pipelineLayout == pipelineLayout;

    if (pipelineLayoutIsSame) {
        auto& lastBoundSets = mLastBoundInfo.boundSets;
        setMask.forEachSetBit([&](size_t index) {
            if (!updateSets[index] || updateSets[index] == lastBoundSets[index]) {
                curMask.unset(index);
            }
        });
        if (curMask.none() &&
                mLastBoundInfo.setMask == setMask && mLastBoundInfo.boundSets == updateSets) {
            return;
        }
    } else {
        setMask.forEachSetBit([&](size_t index) {
            if (!updateSets[index]) {
                curMask.unset(index);
            }
        });
    }

    curMask.forEachSetBit([&updateSets, commands, pipelineLayout](size_t index) {
        // This code actually binds the descriptor sets.
        auto set = updateSets[index];
        VkCommandBuffer const cmdbuffer = commands->buffer();
        vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, index,
                1, &set->getVkSet(), set->uniqueDynamicUboCount, set->getOffsets()->data());
        commands->acquire(set);
    });

    mStashedSets = {};

    mLastBoundInfo = {
        pipelineLayout,
        setMask,
        updateSets,
    };
}

void VulkanDescriptorSetCache::updateBuffer(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        uint8_t binding, fvkmemory::resource_ptr<VulkanBufferObject> bufferObject,
        VkDeviceSize offset, VkDeviceSize size) noexcept {
    VkDescriptorBufferInfo const info = {
        .buffer = bufferObject->buffer.getGpuBuffer(),
        .offset = offset,
        .range = size,
    };
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    if (set->dynamicUboMask.test(binding)) {
        type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    }
    VkWriteDescriptorSet const descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set->getVkSet(),
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &info,
    };
    vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
    set->acquire(bufferObject);
}

void VulkanDescriptorSetCache::updateSampler(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        uint8_t binding, fvkmemory::resource_ptr<VulkanTexture> texture,
        VkSampler sampler) noexcept {
    VkImageSubresourceRange range = texture->getPrimaryViewRange();
    VkImageViewType const expectedType = texture->getViewType();
    if (any(texture->usage & TextureUsage::DEPTH_ATTACHMENT) &&
            expectedType == VK_IMAGE_VIEW_TYPE_2D) {
        // If the sampler is part of a mipmapped depth texture, where one of the level *can* be
        // an attachment, then the range for this view has exactly one level and one layer.
        range.levelCount = 1;
        range.layerCount = 1;
    }
    VkDescriptorImageInfo info{
        .sampler = sampler,
        .imageView = texture->getView(range),
        .imageLayout = fvkutils::getVkLayout(texture->getDefaultLayout()),
    };

    VkWriteDescriptorSet const descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = set->getVkSet(),
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &info,
    };
    vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
    set->acquire(texture);
}

void VulkanDescriptorSetCache::updateInputAttachment(
        fvkmemory::resource_ptr<VulkanDescriptorSet> set,
        VulkanAttachment const& attachment) noexcept {
    // TOOD: fill this in.
}

fvkmemory::resource_ptr<VulkanDescriptorSet> VulkanDescriptorSetCache::createSet(
        Handle<HwDescriptorSet> handle, fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout) {
    auto const vkSet = getVkSet(layout);
    auto const& count = layout->count;
    auto const vklayout = layout->getVkLayout();
    auto set = fvkmemory::resource_ptr<VulkanDescriptorSet>::make(mResourceManager, handle,
            layout->bitmask.dynamicUbo, layout->count.dynamicUbo,
            [vkSet, count, vklayout, this](VulkanDescriptorSet*) {
                // Note that mDescriptorPool could be gone due to terminate (when the backend shuts
                // down).
                if (mDescriptorPool) {
                    mDescriptorPool->recycle(count, vklayout, vkSet);
                }
            });
    set->setVkSet(vkSet);
    return set;
}

VkDescriptorSet VulkanDescriptorSetCache::getVkSet(
        fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout) {
    return mDescriptorPool->obtainSet(layout);
}

void VulkanDescriptorSetCache::manualRecyle(VulkanDescriptorSetLayout::Count const& count,
        VkDescriptorSetLayout vklayout, VkDescriptorSet vkSet) {
    mDescriptorPool->recycle(count, vklayout, vkSet);
}

void VulkanDescriptorSetCache::gc() { mStashedSets = {}; }

} // namespace filament::backend
