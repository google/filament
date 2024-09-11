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

    VkDescriptorSet obtainSet(VulkanDescriptorSetLayout* layout) {
        if (UnusedSetMap::iterator itr = mUnused.find(layout->bitmask); itr != mUnused.end()) {
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
        VkDescriptorSetLayout layouts[1] = {layout->vklayout};
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

    void recycle(BitmaskGroup const& layoutMask, VkDescriptorSet vkSet) {
        // We are recycling - release the set back into the pool. Note that the
        // vk handle has not changed, but we need to change the backend handle to allow
        // for proper refcounting of resources referenced in this set.
        if (auto itr = mUnused.find(layoutMask); itr != mUnused.end()) {
            itr->second.push_back(vkSet);
        } else {
            mUnused[layoutMask].push_back(vkSet);
        }
        mUnusedCount++;
    }

private:
    VkDevice mDevice;
    VkDescriptorPool mPool;
    DescriptorCount const mCount;
    uint16_t const mCapacity;

    // Tracks the number of allocated descriptor sets.
    uint16_t mSize;
    // Tracks  the number of in-use descriptor sets.
    uint16_t mUnusedCount;

    // This maps a layout ot a list of descriptor sets allocated for that layout.
    using UnusedSetMap = std::unordered_map<BitmaskGroup, std::vector<VkDescriptorSet>,
            BitmaskGroupHashFn, BitmaskGroupEqual>;
    UnusedSetMap mUnused;
};

// This is an ever-expanding pool of sets where it
//    1. Keeps a list of smaller pools of different layout-dimensions.
//    2. Will add a pool if existing pool are not compatible with the requested layout o runs out.
class DescriptorInfinitePool {
private:
    static constexpr uint16_t EXPECTED_SET_COUNT = 10;
    static constexpr float SET_COUNT_GROWTH_FACTOR = 1.5;

public:
    DescriptorInfinitePool(VkDevice device)
        : mDevice(device) {}

    VkDescriptorSet obtainSet(VulkanDescriptorSetLayout* layout) {
        DescriptorPool* sameTypePool = nullptr;
        for (auto& pool: mPools) {
            if (!pool->canAllocate(layout->count)) {
                continue;
            }
            if (auto set = pool->obtainSet(layout); set != VK_NULL_HANDLE) {
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
        auto ret = pool->obtainSet(layout);
        assert_invariant(ret != VK_NULL_HANDLE && "failed to obtain a set?");
        return ret;
    }

    void recycle(DescriptorCount const& count, BitmaskGroup const& layoutMask,
            VkDescriptorSet vkSet) {
        for (auto& pool: mPools) {
            if (!pool->canAllocate(count)) {
                continue;
            }
            pool->recycle(layoutMask, vkSet);
            break;
        }
    }

private:
    VkDevice mDevice;
    std::vector<std::unique_ptr<DescriptorPool>> mPools;
};

template<typename Key>
struct Equal {
    bool operator()(Key const& k1, Key const& k2) const {
        return 0 == memcmp((const void*) &k1, (const void*) &k2, sizeof(Key));
    }
};

} // anonymous namespace

class VulkanDescriptorSetManager::Impl {
private:
    using DescriptorSetArray = std::array<VkDescriptorSet, UNIQUE_DESCRIPTOR_SET_COUNT>;

    struct DescriptorSetHistory {
    private:
        using TextureBundle = std::pair<VulkanTexture*, VkImageSubresourceRange>;
    public:
        DescriptorSetHistory() : mResources(nullptr) {}

        DescriptorSetHistory(BitmaskGroup const& mask, VulkanResourceAllocator* allocator,
                VulkanDescriptorSet* set)
            : dynamicUboMask(mask.dynamicUbo),
              mResources(allocator),
              mSet(set),
              mBound(false) {
            // initial state is unbound.
            mResources.acquire(mSet);
            unbind();
        }

        ~DescriptorSetHistory() {
            assert_invariant(mSet);
            mResources.clear();
        }

        void setOffsets(backend::DescriptorSetOffsetArray&& offsets) noexcept {
            mOffsets = std::move(offsets);
            mBound = false;
        }

        void write(uint8_t binding) noexcept {
            mBound = false;
        }

        // Ownership will be transfered to the commandbuffer.
        void bind(VulkanCommandBuffer* commands, VkPipelineLayout pipelineLayout, uint8_t index) noexcept {
            VkCommandBuffer const cmdbuffer = commands->buffer();
            vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                    index, 1, &mSet->vkSet, (uint32_t) dynamicUboMask.count(), mOffsets.data());

            commands->acquire(mSet);
            mResources.clear();
            mBound = true;
        }

        void unbind() noexcept {
            mResources.acquire(mSet);
            mBound = false;
        }

        bool bound() const noexcept { return mBound; }

        UniformBufferBitmask const dynamicUboMask;

    private:
        FixedSizeVulkanResourceManager<1> mResources;
        VulkanDescriptorSet* mSet = nullptr;

        backend::DescriptorSetOffsetArray mOffsets;
        bool mBound = false;
    };

    struct BoundInfo {
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        DescriptorSetMask setMask;
        DescriptorSetArray boundSets = {};

        bool operator==(BoundInfo const& info) const {
            if (pipelineLayout != info.pipelineLayout || setMask != info.setMask) {
                return false;
            }
            bool equal = true;
            setMask.forEachSetBit([&](size_t i) {
                if (boundSets[i] != info.boundSets[i]) {
                    equal = false;
                }
            });
            return equal;
        }
    };

public:
    Impl(VkDevice device, VulkanResourceAllocator* resourceAllocator)
        : mDevice(device),
          mResourceAllocator(resourceAllocator),
          mDescriptorPool(device) {}

    // bind() is not really binding the set but just stashing until we have all the info
    // (pipelinelayout).
    void bind(uint8_t setIndex, VulkanDescriptorSet* set,
            backend::DescriptorSetOffsetArray&& offsets) {
        auto& history = mHistory[set->vkSet];
        history.setOffsets(std::move(offsets));

        auto const lastSet = mStashedSets[setIndex];
        if (lastSet != VK_NULL_HANDLE) {
            assert_invariant(mHistory.find(lastSet) != mHistory.end());
            mHistory[lastSet].unbind();
        }
        mStashedSets[setIndex] = set->vkSet;
    }

    void commit(VulkanCommandBuffer* commands, VkPipelineLayout pipelineLayout,
            DescriptorSetMask const& setMask) {
        std::array<DescriptorSetHistory*, UNIQUE_DESCRIPTOR_SET_COUNT> updateSets = {nullptr};
        auto const& historyEnd = mHistory.end();

        // setMask indicates the set of descriptor sets the driver wants to bind, curMask is the
        // actual set of sets that *needs* to be bound.
        DescriptorSetMask curMask = setMask;

        setMask.forEachSetBit([&](size_t index) {
            auto const vkset = mStashedSets[index];
            if (auto itr = mHistory.find(vkset); itr != historyEnd && !itr->second.bound()) {
                updateSets[index] = &itr->second;
            } else {
                curMask.unset(index);
            }
        });

        BoundInfo nextInfo = {
            pipelineLayout,
            setMask,
            mStashedSets,
        };
        if (curMask.none() && mLastBoundInfo == nextInfo) {
            return;
        }

        curMask.forEachSetBit([&updateSets, commands, pipelineLayout](size_t index) {
            updateSets[index]->bind(commands, pipelineLayout, index);
        });
        mLastBoundInfo = nextInfo;
    }

    void updateBuffer(VulkanDescriptorSet* set, uint8_t binding, VulkanBufferObject* bufferObject,
            VkDeviceSize offset, VkDeviceSize size) noexcept {
        VkDescriptorBufferInfo const info = {
            .buffer = bufferObject->buffer.getGpuBuffer(),
            .offset = offset,
            .range = size,
        };

        VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        auto& history = mHistory[set->vkSet];

        if (history.dynamicUboMask.test(binding)) {
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
        history.write(binding);
    }

    void updateSampler(VulkanDescriptorSet* set, uint8_t binding, VulkanTexture* texture,
            VkSampler sampler) noexcept {
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
        info.imageLayout = imgutil::getVkLayout(texture->getPrimaryImageLayout());
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
        mHistory[set->vkSet].write(binding);
    }

    void updateInputAttachment(VulkanDescriptorSet* set, VulkanAttachment attachment) noexcept {
        // TOOD: fill-in this region
    }

    void setPlaceHolders(VkSampler sampler, VulkanTexture* texture,
            VulkanBufferObject* bufferObject) noexcept {
        mPlaceHolderBufferInfo = {
                .buffer = bufferObject->buffer.getGpuBuffer(),
                .offset = 0,
                .range = 1,
        };
        mPlaceHolderImageInfo = {
                .sampler = sampler,
                .imageView = texture->getPrimaryImageView(),
                .imageLayout = imgutil::getVkLayout(texture->getPrimaryImageLayout()),
        };
    }

    void createSet(Handle<HwDescriptorSet> handle, VulkanDescriptorSetLayout* layout) {
        auto const vkSet = mDescriptorPool.obtainSet(layout);
        auto const& count = layout->count;
        auto const& layoutMask = layout->bitmask;
        auto set = mResourceAllocator->construct<VulkanDescriptorSet>(handle, mResourceAllocator,
                vkSet, [vkSet, count, layoutMask, this]() {
                    mHistory.erase(vkSet);
                    mDescriptorPool.recycle(count, layoutMask, vkSet);
                });
        mHistory.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(vkSet),
                std::forward_as_tuple(layout->bitmask, mResourceAllocator, set));
    }

    void destroySet(Handle<HwDescriptorSet> handle) {
        VulkanDescriptorSet* set = mResourceAllocator->handle_cast<VulkanDescriptorSet*>(handle);
        mHistory.erase(set->vkSet);
        for (uint8_t i = 0; i < mStashedSets.size(); ++i) {
            if (mStashedSets[i] == set->vkSet) {
                mStashedSets[i] = VK_NULL_HANDLE;
            }
        }
    }

private:

    VkDevice mDevice;
    VulkanResourceAllocator* mResourceAllocator;
    DescriptorInfinitePool mDescriptorPool;
    std::pair<VulkanAttachment, VkDescriptorImageInfo> mInputAttachment;
    std::unordered_map<VkDescriptorSet, DescriptorSetHistory> mHistory;
    DescriptorSetArray mStashedSets = { VK_NULL_HANDLE };

    BoundInfo mLastBoundInfo;

    VkDescriptorBufferInfo mPlaceHolderBufferInfo;
    VkDescriptorImageInfo mPlaceHolderImageInfo;
};

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VkDevice device,
        VulkanResourceAllocator* resourceAllocator)
    : mImpl(new Impl(device, resourceAllocator)) {}

void VulkanDescriptorSetManager::terminate() noexcept {
    assert_invariant(mImpl);
    delete mImpl;
    mImpl = nullptr;
}

void VulkanDescriptorSetManager::updateBuffer(VulkanDescriptorSet* set, uint8_t binding,
        VulkanBufferObject* bufferObject, VkDeviceSize offset, VkDeviceSize size) noexcept {
    mImpl->updateBuffer(set, binding, bufferObject, offset, size);
}

void VulkanDescriptorSetManager::updateSampler(VulkanDescriptorSet* set, uint8_t binding,
        VulkanTexture* texture, VkSampler sampler) noexcept {
    mImpl->updateSampler(set, binding, texture, sampler);
}

void VulkanDescriptorSetManager::updateInputAttachment(VulkanDescriptorSet* set,
        VulkanAttachment attachment) noexcept {
    mImpl->updateInputAttachment(set, attachment);
}

void VulkanDescriptorSetManager::setPlaceHolders(VkSampler sampler, VulkanTexture* texture,
        VulkanBufferObject* bufferObject) noexcept {
    mImpl->setPlaceHolders(sampler, texture, bufferObject);
}

void VulkanDescriptorSetManager::bind(uint8_t setIndex, VulkanDescriptorSet* set,
        backend::DescriptorSetOffsetArray&& offsets) {
    return mImpl->bind(setIndex, set, std::move(offsets));
}

void VulkanDescriptorSetManager::commit(VulkanCommandBuffer* commands,
        VkPipelineLayout pipelineLayout, DescriptorSetMask const& setMask) {
    mImpl->commit(commands, pipelineLayout, setMask);
}

void VulkanDescriptorSetManager::createSet(Handle<HwDescriptorSet> handle,
        VulkanDescriptorSetLayout* layout) {
    mImpl->createSet(handle, layout);
}

void VulkanDescriptorSetManager::destroySet(Handle<HwDescriptorSet> handle) {
    mImpl->destroySet(handle);
}


}// namespace filament::backend
