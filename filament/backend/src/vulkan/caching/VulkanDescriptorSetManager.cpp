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

#include "VulkanDescriptorSetManager.h"

#include <vulkan/VulkanHandles.h>
#include <vulkan/VulkanUtility.h>
#include <vulkan/VulkanConstants.h>
#include <vulkan/VulkanImageUtility.h>
#include <vulkan/VulkanResources.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Panic.h>

#include <math.h>

#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>

namespace filament::backend {

namespace {

using BitmaskGroup = VulkanDescriptorSetLayout::Bitmask;
using DescriptorCount = VulkanDescriptorSetLayout::Count;
using DescriptorSetLayoutArray = VulkanDescriptorSetManager::DescriptorSetLayoutArray;
using BitmaskGroupHashFn = utils::hash::MurmurHashFn<BitmaskGroup>;
struct BitmaskGroupEqual {
    bool operator()(BitmaskGroup const& k1, BitmaskGroup const& k2) const {
        return k1 == k2;
    }
};

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

template<typename Bitmask>
uint32_t createBindings(VkDescriptorSetLayoutBinding* toBind, uint32_t count, VkDescriptorType type,
        Bitmask const& mask) {
    Bitmask alreadySeen;
    mask.forEachSetBit([&](size_t index) {
        VkShaderStageFlags stages = 0;
        uint32_t binding = 0;
        if (index < getFragmentStageShift<Bitmask>()) {
            binding = (uint32_t) index;
            stages |= VK_SHADER_STAGE_VERTEX_BIT;
            auto fragIndex = index + getFragmentStageShift<Bitmask>();
            if (mask.test(fragIndex)) {
                stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
                alreadySeen.set(fragIndex);
            }
        } else if (!alreadySeen.test(index)) {
            // We are in fragment stage bits
            binding = (uint32_t) (index - getFragmentStageShift<Bitmask>());
            stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        if (stages) {
            toBind[count++] = {
                .binding = binding,
                .descriptorType = type,
                .descriptorCount = 1,
                .stageFlags = stages,
            };
        }
    });
    return count;
}

inline VkDescriptorSetLayout createLayout(VkDevice device, BitmaskGroup const& bitmaskGroup) {
    // Note that the following *needs* to be static so that VkDescriptorSetLayoutCreateInfo will not
    // refer to stack memory.
    VkDescriptorSetLayoutBinding toBind[VulkanDescriptorSetLayout::MAX_BINDINGS];
    uint32_t count = 0;

    count = createBindings(toBind, count, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            bitmaskGroup.dynamicUbo);
    count = createBindings(toBind, count, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bitmaskGroup.ubo);
    count = createBindings(toBind, count, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            bitmaskGroup.sampler);
    count = createBindings(toBind, count, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            bitmaskGroup.inputAttachment);

    assert_invariant(count != 0 && "Need at least one binding for descriptor set layout.");
    VkDescriptorSetLayoutCreateInfo dlinfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = count,
            .pBindings = toBind,
    };

    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(device, &dlinfo, VKALLOC, &layout);
    return layout;
}

} // anonymous namespace

// This is an ever-expanding pool of sets where it
//    1. Keeps a list of smaller pools of different layout-dimensions.
//    2. Will add a pool if existing pool are not compatible with the requested layout o runs out.
class VulkanDescriptorSetManager::DescriptorInfinitePool {
private:
    static constexpr uint16_t EXPECTED_SET_COUNT = 10;
    static constexpr float SET_COUNT_GROWTH_FACTOR = 1.5;

public:
    DescriptorInfinitePool(VkDevice device)
        : mDevice(device) {}

    VkDescriptorSet obtainSet(VulkanDescriptorSetLayout* layout) {
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

class VulkanDescriptorSetManager::DescriptorSetLayoutManager {
public:
    DescriptorSetLayoutManager(VkDevice device)
        : mDevice(device) {}

    VkDescriptorSetLayout getVkLayout(VulkanDescriptorSetLayout* layout) {
        auto const& bitmasks = layout->bitmask;
        if (auto itr = mVkLayouts.find(bitmasks); itr != mVkLayouts.end()) {
            return itr->second;
        }
        auto vklayout = createLayout(mDevice, layout->bitmask);
        mVkLayouts[layout->bitmask] = vklayout;
        return vklayout;
    }

    ~DescriptorSetLayoutManager() {
        for (auto& itr: mVkLayouts) {
            vkDestroyDescriptorSetLayout(mDevice, itr.second, VKALLOC);
        }
    }

private:
    VkDevice mDevice;
    tsl::robin_map<BitmaskGroup, VkDescriptorSetLayout, BitmaskGroupHashFn, BitmaskGroupEqual>
            mVkLayouts;
};

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VkDevice device,
        VulkanResourceAllocator* resourceAllocator)
    : mDevice(device),
      mResourceAllocator(resourceAllocator),
      mLayoutManager(std::make_unique<DescriptorSetLayoutManager>(device)),
      mDescriptorPool(std::make_unique<DescriptorInfinitePool>(device)) {}

VulkanDescriptorSetManager::~VulkanDescriptorSetManager() = default;

void VulkanDescriptorSetManager::terminate() noexcept{
    mLayoutManager.reset();
    mDescriptorPool.reset();
}

// bind() is not really binding the set but just stashing until we have all the info
// (pipelinelayout).
void VulkanDescriptorSetManager::bind(uint8_t setIndex, VulkanDescriptorSet* set,
        backend::DescriptorSetOffsetArray&& offsets) {
    set->setOffsets(std::move(offsets));
    mStashedSets[setIndex] = set;
}

void VulkanDescriptorSetManager::commit(VulkanCommandBuffer* commands,
        VkPipelineLayout pipelineLayout, DescriptorSetMask const& setMask) {
    // setMask indicates the set of descriptor sets the driver wants to bind, curMask is the
    // actual set of sets that *needs* to be bound.
    DescriptorSetMask curMask = setMask;

    auto& updateSets = mStashedSets;
    auto& lastBoundSets = mLastBoundInfo.boundSets;

    setMask.forEachSetBit([&](size_t index) {
        if (!updateSets[index] || updateSets[index] == lastBoundSets[index]) {
            curMask.unset(index);
        }
    });

    if (curMask.none() &&
            (mLastBoundInfo.pipelineLayout == pipelineLayout && mLastBoundInfo.setMask == setMask &&
                    mLastBoundInfo.boundSets == updateSets)) {
        return;
    }

    curMask.forEachSetBit([&updateSets, commands, pipelineLayout](size_t index) {
        // This code actually binds the descriptor sets.
        auto set = updateSets[index];
        VkCommandBuffer const cmdbuffer = commands->buffer();
        vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, index,
                1, &set->vkSet, set->uniqueDynamicUboCount, set->getOffsets()->data());
        commands->acquire(set);
    });

    mStashedSets = {};

    mLastBoundInfo = {
        pipelineLayout,
        setMask,
        updateSets,
    };
}

void VulkanDescriptorSetManager::updateBuffer(VulkanDescriptorSet* set, uint8_t binding,
        VulkanBufferObject* bufferObject, VkDeviceSize offset, VkDeviceSize size) noexcept {
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
        .pNext = nullptr,
        .dstSet = set->vkSet,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = type,
        .pBufferInfo = &info,
    };
    vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
    set->acquire(bufferObject);
}

void VulkanDescriptorSetManager::updateSampler(VulkanDescriptorSet* set, uint8_t binding,
        VulkanTexture* texture, VkSampler sampler) noexcept {
    VkDescriptorImageInfo info{
        .sampler = sampler,
    };
    VkImageSubresourceRange const range = texture->getPrimaryViewRange();
    VkImageViewType const expectedType = texture->getViewType();
    if (any(texture->usage & TextureUsage::DEPTH_ATTACHMENT) &&
            expectedType == VK_IMAGE_VIEW_TYPE_2D) {
        // If the sampler is part of a mipmapped depth texture, where one of the level *can* be
        // an attachment, then the sampler for this texture has the same view properties as a
        // view for an attachment. Therefore, we can use getAttachmentView to get a
        // corresponding VkImageView.
        info.imageView = texture->getAttachmentView(range);
    } else {
        info.imageView = texture->getViewForType(range, expectedType);
    }
    info.imageLayout = imgutil::getVkLayout(texture->getDefaultLayout());
    VkWriteDescriptorSet const descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = set->vkSet,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &info,
    };
    vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
    set->acquire(texture);
}

void VulkanDescriptorSetManager::updateInputAttachment(VulkanDescriptorSet* set,
        VulkanAttachment attachment) noexcept {
    // TOOD: fill-in this region
}

void VulkanDescriptorSetManager::createSet(Handle<HwDescriptorSet> handle,
        VulkanDescriptorSetLayout* layout) {
    auto const vkSet = mDescriptorPool->obtainSet(layout);
    auto const& count = layout->count;
    auto const vklayout = layout->getVkLayout();
    mResourceAllocator->construct<VulkanDescriptorSet>(handle, mResourceAllocator, vkSet,
            layout->bitmask.dynamicUbo, layout->count.dynamicUbo,
            [vkSet, count, vklayout, this](VulkanDescriptorSet* set) {
                eraseSetFromHistory(set);
                mDescriptorPool->recycle(count, vklayout, vkSet);
            });
}

void VulkanDescriptorSetManager::destroySet(Handle<HwDescriptorSet> handle) {
}

void VulkanDescriptorSetManager::initVkLayout(VulkanDescriptorSetLayout* layout) {
    layout->setVkLayout(mLayoutManager->getVkLayout(layout));
}

void VulkanDescriptorSetManager::eraseSetFromHistory(VulkanDescriptorSet* set) {
    for (uint8_t i = 0; i < mStashedSets.size(); ++i) {
        if (mStashedSets[i] == set) {
            mStashedSets[i] = nullptr;
        }
    }
}


} // namespace filament::backend
