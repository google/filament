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

#include "VulkanDescriptorSet.h"

#include <vulkan/VulkanConstants.h>
#include <vulkan/VulkanResources.h>

#include <type_traits>

namespace filament::backend {

namespace {

template<typename MaskType>
using LayoutMap = tsl::robin_map<MaskType, VkDescriptorSetLayout>;
using LayoutArray = VulkanDescriptorSetManager::LayoutArray;
using UniformBufferBitmask = VulkanDescriptorSetManager::UniformBufferBitmask;
using SamplerBitmask = VulkanDescriptorSetManager::SamplerBitmask;

constexpr uint8_t EXPECTED_IN_FLIGHT_FRAMES = 3; // Asssume triple buffering

#define PORT_CONSTANT(NameSpace, K) constexpr decltype(NameSpace::K) K = NameSpace::K;

PORT_CONSTANT(VulkanDescriptorSetManager, MAX_SUPPORTED_SHADER_STAGE)
PORT_CONSTANT(VulkanDescriptorSetManager, VERTEX_STAGE)
PORT_CONSTANT(VulkanDescriptorSetManager, FRAGMENT_STAGE)

PORT_CONSTANT(Program, UNIFORM_BINDING_COUNT)
PORT_CONSTANT(Program, SAMPLER_BINDING_COUNT)

#undef PORT_CONSTANT

uint8_t BIT_COUNT[256] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2,
    3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3,
    4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3,
    4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4,
    5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3,
    4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4,
    5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4,
    5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5,
    6, 6, 7, 6, 7, 7, 8};

inline uint8_t countBits(uint32_t num) {
    return BIT_COUNT[num & 0xFF] + BIT_COUNT[(num >> 8) & 0xFF] + BIT_COUNT[(num >> 16) & 0xFF] +
           BIT_COUNT[(num >> 24) & 0xFF];
}

struct SamplerSet {
    using Mask = UniformBufferBitmask;
    struct Key {
        Mask mask;
        static_assert(sizeof(Mask) == 4);
        uint32_t padding;// We need a padding to ensure 8-byte alignement.
        VkDescriptorSetLayout layout;
        VkSampler sampler[SAMPLER_BINDING_COUNT];
        VkImageView imageView[SAMPLER_BINDING_COUNT];
        VkImageLayout imageLayout[SAMPLER_BINDING_COUNT];
    };

    struct Equal {
        bool operator()(Key const& k1, Key const& k2) const {
            if (k1.mask != k2.mask) return false;
            if (k1.layout != k2.layout) return false;

            for (uint8_t i = 0, bitCount = countBits(k1.mask); i < bitCount; ++i) {
                if (k1.sampler[i] != k2.sampler[i] || k1.imageView[i] != k2.imageView[i] ||
                        k1.imageLayout[i] != k2.imageLayout[i]) {
                    return false;
                }
            }
            return true;
        }
    };
    using HashFn = utils::hash::MurmurHashFn<Key>;
    using Cache = std::unordered_map<Key, VulkanDescriptorSet*, HashFn, Equal>;
};

struct UBOSet {
    using Mask = SamplerBitmask;

    struct Key {
        Mask mask;
        static_assert(sizeof(Mask) == 4);
        uint32_t padding;// We need a padding to ensure 8-byte alignement.
        VkDescriptorSetLayout layout;
        VkBuffer buffers[UNIFORM_BINDING_COUNT];    //   80     0
        VkDeviceSize offsets[UNIFORM_BINDING_COUNT];//   40  1592
        VkDeviceSize sizes[UNIFORM_BINDING_COUNT];  //   40  1632
    };

    struct Equal {
        bool operator()(Key const& k1, Key const& k2) const {
            if (k1.mask != k2.mask) return false;
            if (k1.layout != k2.layout) return false;

            for (uint8_t i = 0, bitCount = countBits(k1.mask); i < bitCount; ++i) {
                if (k1.buffers[i] != k2.buffers[i] || k1.offsets[i] != k2.offsets[i] ||
                        k1.sizes[i] != k2.sizes[i]) {
                    return false;
                }
            }
            return true;
        }
    };
    using HashFn = utils::hash::MurmurHashFn<Key>;
    using Cache = std::unordered_map<Key, VulkanDescriptorSet*, HashFn, Equal>;
};

constexpr uint8_t MAX_BINDINGS = UNIFORM_BINDING_COUNT * MAX_SUPPORTED_SHADER_STAGE;
static_assert(MAX_BINDINGS >= SAMPLER_BINDING_COUNT * MAX_SUPPORTED_SHADER_STAGE);

template<typename MaskType>
inline MaskType genMask(uint8_t stage, MaskType bit) {
    return static_cast<MaskType>(bit << (sizeof(MaskType) * 4 * (stage - 1)));
}

template<typename TYPE>
class DescriptorPool {
private:
    static constexpr uint32_t COUNT_MULTIPLIER =
            MAX_SUPPORTED_SHADER_STAGE * EXPECTED_IN_FLIGHT_FRAMES;
    static constexpr uint32_t CAPACITY =
            std::is_same_v<TYPE, UBOSet> ? Program::UNIFORM_BINDING_COUNT * COUNT_MULTIPLIER
                                         : Program::SAMPLER_BINDING_COUNT * COUNT_MULTIPLIER;

public:
    DescriptorPool()
        : mDevice(VK_NULL_HANDLE),
          mResourceAllocator(nullptr) {}

    DescriptorPool(VkDevice device, VulkanResourceAllocator* allocator)
        : mDevice(device),
          mResourceAllocator(allocator) {
        VkDescriptorPoolSize sizes[1] = {
            {
                .type = std::is_same_v<TYPE, UBOSet> ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
                                                     : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = CAPACITY,
            },
        };
        VkDescriptorPoolCreateInfo info{
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = EXPECTED_IN_FLIGHT_FRAMES,
            .poolSizeCount = 1,
            .pPoolSizes = sizes,
        };
        vkCreateDescriptorPool(mDevice, &info, VKALLOC, &mPool);
    }

    DescriptorPool(DescriptorPool const&) = delete;
    DescriptorPool& operator=(DescriptorPool const&) = delete;

    ~DescriptorPool() {
        vkDestroyDescriptorPool(mDevice, mPool, VKALLOC);
    }

    VulkanDescriptorSet* obtainSet(VkDescriptorSetLayout layout) {
        if (mCount == CAPACITY) {
            return nullptr;
        }

        if (auto unused = mUnused.find(layout);
                unused != mUnused.end() && !unused->second.empty()) {
            auto& unusedList = unused->second;
            auto set = unusedList.back();
            unusedList.pop_back();
            mCount++;
            return set;
        }

        // Creating a new set
        VkDescriptorSetLayout layouts[1] = {layout};
        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = mPool,
            .descriptorSetCount = 1,
            .pSetLayouts = layouts,
        };
        VkDescriptorSet vkSet;
        UTILS_UNUSED VkResult result = vkAllocateDescriptorSets(mDevice, &allocInfo, &vkSet);
        ASSERT_POSTCONDITION(result == VK_SUCCESS, "Cannot allocate descriptor set. error=%d",
                (int) result);

        mCount++;
        return createSet(layout, vkSet);
    }

private:
    inline VulkanDescriptorSet* createSet(VkDescriptorSetLayout layout, VkDescriptorSet vkSet) {
        return VulkanDescriptorSet::create(mResourceAllocator, vkSet, layout,
                [this](VulkanDescriptorSet* set) {
                    auto const layout = set->layout;
                    auto const vkSet = set->vkSet;
                    auto listIter = mUnused.find(layout);
                    assert_invariant(listIter != mUnused.end());
                    auto& unusedList = listIter->second;

                    // At this point, we know that the pointer will be stale, and we need to remove
                    // any non-ref-counted references to it.
                    // TODO: this will be inefficient
                    auto iter = std::find(unusedList.begin(), unusedList.end(), set);
                    unusedList.erase(iter);

                    // We are recycling - release the vk resource back into the pool. Note that the
                    // vk handle has not changed, but we need to change the backend handle to allow
                    // for proper refcounting of resources referenced in this set.
                    unusedList.push_back(createSet(layout, vkSet));
                    mCount--;
                });
    }

    VkDevice mDevice;
    VulkanResourceAllocator* mResourceAllocator;
    VkDescriptorPool mPool;

    uint32_t mCount = 0;
    std::unordered_map<VkDescriptorSetLayout, std::vector<VulkanDescriptorSet*>> mUnused;
};

template<typename TYPE>
class DescriptorInfinitePool {
public:
    DescriptorInfinitePool(VkDevice device, VulkanResourceAllocator* allocator)
        : mDevice(device),
          mResourceAllocator(allocator) {}

    ~DescriptorInfinitePool() {
        for (auto pool: mPools) {
            delete pool;
        }
    }

    VulkanDescriptorSet* obtainSet(VkDescriptorSetLayout layout) {
        for (auto pool: mPools) {
            auto set = pool->obtainSet(layout);
            if (set) {
                return set;
            }
        }
        // We need to increase the number of pools
        mPools.push_back(new DescriptorPool<TYPE>{mDevice, mResourceAllocator});
        auto pool = mPools.back();
        return pool->obtainSet(layout);
    }

private:
    VkDevice mDevice;
    VulkanResourceAllocator* mResourceAllocator;
    std::vector<DescriptorPool<TYPE>*> mPools;
};

// This holds a cache of descriptor sets of a TYPE (UBO or Sampler). The Key is defined as the
// layout and the content (for example specific samplers).
template<typename TYPE, typename Cache, typename Key>
class DescriptorSetCache {
public:
    DescriptorSetCache(VkDevice device, VulkanResourceAllocator* allocator)
        : mPool(device, allocator),
          mResourceManager(allocator) {}

    std::pair<VulkanDescriptorSet*, bool> obtainSet(Key const& key) {
        mAge++;
        if (auto result = mCache.find(key); result != mCache.end()) {
            auto set = result->second;
            auto const oldAge = mReverseHistory[set];
            mHistory.erase(oldAge);
            mHistory[mAge] = set;
            mReverseHistory[set] = mAge;
            return {set, true};
        }

        VulkanDescriptorSet* set = mPool.obtainSet(key.layout);
        mCache[key] = set;
        mReverseCache[set] = key;
        mHistory[mAge] = set;
        mReverseHistory[set] = mAge;
        mResourceManager.acquire(set);
        return {set, false};
    }

    // We need to periodically purge the descriptor sets so that we're not holding on to resources
    // unnecessarily. The strategy for purging needs to be examined more.
    void gc() noexcept {
        constexpr uint32_t ALLOWED_ENTRIES = EXPECTED_IN_FLIGHT_FRAMES * 3;
        int32_t const toCutCount = mHistory.size() - ALLOWED_ENTRIES;

        if (toCutCount <= 0) {
            return;
        }

        std::vector<uint64_t> remove;
        for (auto entry = mHistory.begin(); entry != mHistory.end(); entry++) {
            auto const& set = entry->second;
            Key const& key = mReverseCache[set];
            mCache.erase(key);
            mReverseCache.erase(set);
            mReverseHistory.erase(set);

            remove.push_back(entry->first);
            mResourceManager.release(set);
        }
        for (auto removeAge: remove) {
            mHistory.erase(removeAge);
        }
    }

private:
    DescriptorInfinitePool<TYPE> mPool;

    // TODO: combine some of these data structures
    Cache mCache;
    std::unordered_map<VulkanDescriptorSet*, Key> mReverseCache;

    // Use the ordering for purging if needed;
    std::map<uint64_t, VulkanDescriptorSet*> mHistory;
    std::unordered_map<VulkanDescriptorSet*, uint64_t> mReverseHistory;
    uint64_t mAge;

    // Note that instead of owning the resources (i.e. descriptor set) in the pools, we keep them
    // here since all the allocated sets have to pass through this class, and this class has better
    // knowledge to make decisions about gc.
    VulkanResourceManager mResourceManager;
};

using UBOSetCache = DescriptorSetCache<UBOSet, UBOSet::Cache, UBOSet::Key>;
using SamplerSetCache = DescriptorSetCache<SamplerSet, SamplerSet::Cache, SamplerSet::Key>;

template<typename MaskType>
class LayoutCache {
public:
    explicit LayoutCache(VkDevice device)
        : mDevice(device) {}

    ~LayoutCache() {
        for (auto [mask, layout]: mLayouts) {
            vkDestroyDescriptorSetLayout(mDevice, layout, VKALLOC);
        }
        mLayouts.clear();
    }

    VkDescriptorSetLayout getLayout(MaskType stageMask) noexcept {
        if (stageMask == 0) {
            return VK_NULL_HANDLE;
        }
        if (auto iter = mLayouts.find(stageMask); iter != mLayouts.end()) {
            return iter->second;
        }
        VkDescriptorType descriptorType;
        if constexpr (std::is_same_v<MaskType, UBOSet::Mask>) {
            descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        } else if constexpr (std::is_same_v<MaskType, SamplerSet::Mask>) {
            descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }

        VkDescriptorSetLayoutBinding toBind[MAX_BINDINGS];
        uint32_t count = 0;
        for (uint8_t i = 0, maxBindings = sizeof(stageMask) * 4; i < maxBindings; i++) {
            VkShaderStageFlags stages = 0;
            if ((stageMask & (VERTEX_STAGE << i)) != 0) {
                stages |= VK_SHADER_STAGE_VERTEX_BIT;
            }
            if ((stageMask & (FRAGMENT_STAGE << i)) == 0) {
                stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            if (stages == 0) {
                continue;
            }

            auto& bindInfo = toBind[count++];
            bindInfo = {
                .binding = i,
                .descriptorType = descriptorType,
                .descriptorCount = 1,
                .stageFlags = stages,
            };
        }
        VkDescriptorSetLayoutCreateInfo dlinfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = count,
            .pBindings = toBind,
        };

        VkDescriptorSetLayout handle;
        vkCreateDescriptorSetLayout(mDevice, &dlinfo, VKALLOC, &handle);
        mLayouts[stageMask] = handle;

        return handle;
    }

private:
    VkDevice mDevice;
    LayoutMap<MaskType> mLayouts;
};

using UBOSetLayoutCache = LayoutCache<UBOSet::Mask>;
using SamplerSetLayoutCache = LayoutCache<SamplerSet::Mask>;

} // anonymous namespace

VulkanDescriptorSet::VulkanDescriptorSet(VulkanResourceAllocator* allocator, VkDescriptorSet rawSet,
        VkDescriptorSetLayout layout, OnRecycle&& onRecycleFn)
    : VulkanResource(VulkanResourceType::DESCRIPTOR_SET),
      resources(allocator),
      vkSet(rawSet),
      layout(layout),
      mOnRecycleFn(std::move(onRecycleFn)) {}

VulkanDescriptorSet::~VulkanDescriptorSet() {
    if (mOnRecycleFn) {
        mOnRecycleFn(this);
    }
}

VulkanDescriptorSet* VulkanDescriptorSet::create(VulkanResourceAllocator* allocator,
        VkDescriptorSet rawSet, VkDescriptorSetLayout layout, OnRecycle&& onRecycleFn) {
    auto handle = allocator->allocHandle<VulkanDescriptorSet>();
    auto set = allocator->construct<VulkanDescriptorSet>(handle, allocator, rawSet, layout,
            std::move(onRecycleFn));
    return set;
}

class VulkanDescriptorSetManager::Impl {
public:
    Impl(VkDevice device, VulkanResourceAllocator* allocator)
        : mDevice(device),
          mUBOLayoutCache(device),
          mUBOCache(device, allocator),
          mSamplerLayoutCache(device),
          mSamplerCache(device, allocator),
          mResources(allocator) {}

    void setUniformBufferObject(uint32_t bindingIndex, VulkanBufferObject* bufferObject,
            VkDeviceSize offset, VkDeviceSize size) noexcept {
        mUbos.push_back({bindingIndex, bufferObject, offset, size});

        // Between "set" and "commit", we need to ref the buffer object to avoid it being collected.
        mResources.acquire(bufferObject);
    }

    void setSamplers(SamplerArray&& samplers) {
        mSamplers = std::move(samplers);
        for (auto const& sampler: mSamplers) {
            mResources.acquire(sampler.texture);
        }
    }

    void gc() noexcept {
        mUBOCache.gc();
        mSamplerCache.gc();
    }

    // This will write/update all of the descriptor set (and create a set if a one of the same
    // layout is not available).
    void bind(VulkanCommandBuffer* commands, GetPipelineLayoutFunction& getPipelineLayoutFn) {
        LayoutArray layouts = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        std::array<VkDescriptorSet, 2> descSets = {VK_NULL_HANDLE, VK_NULL_HANDLE};
        uint8_t descSetCount = 0;

        VkWriteDescriptorSet descriptorWrites[UNIFORM_BINDING_COUNT + SAMPLER_BINDING_COUNT];
        VkDescriptorBufferInfo uboWrite[UNIFORM_BINDING_COUNT];
        VkDescriptorImageInfo samplerWrite[SAMPLER_BINDING_COUNT];
        uint32_t nwrites = 0;

        UBOSet::Key uboKey = {};
        auto& uboMask = uboKey.mask;
        uint8_t uboCount = 0;
        for (auto const& ubo: mUbos) {
            auto const& binding = std::get<0>(ubo);

            uboKey.buffers[uboCount] = std::get<1>(ubo)->buffer.getGpuBuffer();
            uboKey.offsets[uboCount] = std::get<2>(ubo);
            uboKey.sizes[uboCount] = std::get<3>(ubo);
            uboCount++;

            // Currently we let ubo be visible in all stages.
            uboMask |= genMask<UBOSet::Mask>(VERTEX_STAGE, 1 << binding);
            uboMask |= genMask<UBOSet::Mask>(FRAGMENT_STAGE, 1 << binding);
        }

        if (uboMask) {
            layouts[UBO_SET_INDEX] = uboKey.layout = mUBOLayoutCache.getLayout(uboMask);

            auto [descriptorSet, cached] = mUBOCache.obtainSet(uboKey);
            descSets[descSetCount++] = descriptorSet->vkSet;

            // We need to write to the descriptor set since it wasn't cached.
            if (!cached) {
                // If it wasn't cached before, we need to ref the resources that it touches.
                for (auto const& ubo: mUbos) {
                    auto const buffer = std::get<1>(ubo);
                    descriptorSet->resources.acquire(buffer);
                }

                for (uint8_t i = 0; i < uboCount; ++i) {
                    auto const& binding = std::get<0>(mUbos[i]);
                    VkWriteDescriptorSet& descriptorWrite = descriptorWrites[nwrites++];
                    auto& writeInfo = uboWrite[i];
                    writeInfo = {
                        .buffer = uboKey.buffers[i],
                        .offset = uboKey.offsets[i],
                        .range = uboKey.sizes[i],
                    };
                    descriptorWrite = {
                        .dstSet = descriptorSet->vkSet,
                        .dstBinding = binding,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pBufferInfo = &writeInfo,
                    };
                }
            }
            commands->acquire(descriptorSet);
        }

        SamplerSet::Key samplerKey = {};
        auto& samplerMask = samplerKey.mask;
        uint8_t samplerCount = 0;
        for (auto const sampler: mSamplers) {
            samplerMask |= genMask<SamplerSet::Mask>(sampler.stage, 1 << sampler.binding);

            samplerKey.sampler[samplerCount] = sampler.info.sampler;
            samplerKey.imageView[samplerCount] = sampler.info.imageView;
            samplerKey.imageLayout[samplerCount] = sampler.info.imageLayout;
            samplerCount++;
        }

        if (samplerMask) {
            layouts[SAMPLER_SET_INDEX] = samplerKey.layout =
                    mSamplerLayoutCache.getLayout(samplerMask);

            auto [descriptorSet, cached] = mSamplerCache.obtainSet(samplerKey);
            descSets[descSetCount++] = descriptorSet->vkSet;

            if (!cached) {
                for (auto const& sampler: mSamplers) {
                    descriptorSet->resources.acquire(sampler.texture);
                }

                for (uint8_t i = 0; i < samplerCount; ++i) {
                    auto const& binding = mSamplers[i].binding;
                    VkWriteDescriptorSet& descriptorWrite = descriptorWrites[nwrites++];
                    auto& writeInfo = samplerWrite[i];
                    writeInfo = mSamplers[i].info;
                    descriptorWrite = {
                        .dstSet = descriptorSet->vkSet,
                        .dstBinding = binding,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo = &writeInfo,
                    };
                }
            }
            commands->acquire(descriptorSet);
        }
        if (nwrites) {
            vkUpdateDescriptorSets(mDevice, nwrites, descriptorWrites, 0, nullptr);
        }

        VkPipelineLayout const pipelineLayout = getPipelineLayoutFn(layouts);
        VkCommandBuffer const cmdbuffer = commands->buffer();

        BoundState state {
            .cmdbuf = cmdbuffer,
            .pipelineLayout = pipelineLayout,
        };

        if (state == mPreviousBoundState) {
            return;
        }

        vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                descSetCount, descSets.data(), 0, nullptr);

        mPreviousBoundState = state;

        // Once bound, the resources are now ref'd in the descriptor set and the references in this
        // class can be released and the descriptor set is ref'd by the command buffer.
        mResources.clear();
    }

private:
    struct BoundState {
        VkCommandBuffer cmdbuf;
        VkPipelineLayout pipelineLayout;

        bool operator==(BoundState const& b) const {
            return cmdbuf == b.cmdbuf && pipelineLayout == b.pipelineLayout;
        }
    };

    VkDevice mDevice;

    UBOSetLayoutCache mUBOLayoutCache;
    UBOSetCache mUBOCache;

    SamplerSetLayoutCache mSamplerLayoutCache;
    SamplerSetCache mSamplerCache;

    std::vector<std::tuple<uint32_t, VulkanBufferObject*, VkDeviceSize, VkDeviceSize>> mUbos;
    SamplerArray mSamplers;
    VulkanAcquireOnlyResourceManager mResources;

    BoundState mPreviousBoundState;
};

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VkDevice device,
        VulkanResourceAllocator* resourceAllocator)
    : mImpl(new Impl(device, resourceAllocator)) {}

void VulkanDescriptorSetManager::terminate() noexcept {
    assert_invariant(mImpl);
    delete mImpl;
    mImpl = nullptr;
}

void VulkanDescriptorSetManager::gc() noexcept { mImpl->gc(); }

void VulkanDescriptorSetManager::bind(VulkanCommandBuffer* commands,
        GetPipelineLayoutFunction& getPipelineLayoutFn) {
    mImpl->bind(commands, getPipelineLayoutFn);
}

void VulkanDescriptorSetManager::setUniformBufferObject(uint32_t bindingIndex,
        VulkanBufferObject* bufferObject, VkDeviceSize offset, VkDeviceSize size) noexcept {
    mImpl->setUniformBufferObject(bindingIndex, bufferObject, offset, size);
}

void VulkanDescriptorSetManager::setSamplers(SamplerArray&& samplers) {
    mImpl->setSamplers(std::move(samplers));
}

} // namespace filament::backend
