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

// This assumes we have at most 32-bound samplers, 10 UBOs and, 1 input attachment.
// TODO: Obsolete after [GDSR].
constexpr uint8_t MAX_SAMPLER_BINDING = 32;
constexpr uint8_t MAX_UBO_BINDING = 10;
constexpr uint8_t MAX_INPUT_ATTACHMENT_BINDING = 1;
constexpr uint8_t MAX_BINDINGS =
        MAX_SAMPLER_BINDING + MAX_UBO_BINDING + MAX_INPUT_ATTACHMENT_BINDING;

using Bitmask = VulkanDescriptorSetLayout::Bitmask;
using DescriptorCount = VulkanDescriptorSetLayout::Count;
using UBOMap = std::array<std::pair<VkDescriptorBufferInfo, VulkanBufferObject*>, MAX_UBO_BINDING>;
using SamplerMap =
        std::array<std::pair<VkDescriptorImageInfo, VulkanTexture*>, MAX_SAMPLER_BINDING>;
using BitmaskHashFn = utils::hash::MurmurHashFn<Bitmask>;
struct BitmaskEqual {
    bool operator()(Bitmask const& k1, Bitmask const& k2) const {
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
    DescriptorPool(VkDevice device, VulkanResourceAllocator* allocator,
            DescriptorCount const& count, uint16_t capacity)
        : mDevice(device),
          mAllocator(allocator),
          mCount(count),
          mCapacity(capacity),
          mSize(0),
          mUnusedCount(0),
          mDisableRecycling(false) {
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
        // Note that these have to manually destroyed because they were not explicitly ref-counted.
        for (auto const& [mask, sets]: mUnused) {
            for (auto set: sets) {
                mAllocator->destruct<VulkanDescriptorSet>(set);
            }
        }
        vkDestroyDescriptorPool(mDevice, mPool, VKALLOC);
    }

    void disableRecycling() noexcept {
        mDisableRecycling = true;
    }

    uint16_t const& capacity() {
        return mCapacity;
    }

    // A convenience method for checking if this pool can allocate sets for a given layout.
    inline bool canAllocate(VulkanDescriptorSetLayout* layout) {
        return layout->count == mCount;
    }

    Handle<VulkanDescriptorSet> obtainSet(VulkanDescriptorSetLayout* layout) {
        if (UnusedSetMap::iterator itr = mUnused.find(layout->bitmask); itr != mUnused.end()) {
            // If we don't have any unused, then just return an empty handle.
            if (itr->second.empty()) {
                return {};
            }
            std::vector<Handle<VulkanDescriptorSet>>& sets = itr->second;
            auto set = sets.back();
            sets.pop_back();
            mUnusedCount--;
            return set;
        }
        if (mSize + 1 > mCapacity) {
            return {};
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
        return createSet(layout->bitmask, vkSet);
    }

private:
    Handle<VulkanDescriptorSet> createSet(Bitmask const& layoutMask, VkDescriptorSet vkSet) {
        return mAllocator->initHandle<VulkanDescriptorSet>(mAllocator, vkSet,
                [this, layoutMask, vkSet]() {
                    if (mDisableRecycling) {
                        return;
                    }
                    // We are recycling - release the set back into the pool. Note that the
                    // vk handle has not changed, but we need to change the backend handle to allow
                    // for proper refcounting of resources referenced in this set.
                    auto setHandle = createSet(layoutMask, vkSet);
                    if (auto itr = mUnused.find(layoutMask); itr != mUnused.end()) {
                        itr->second.push_back(setHandle);
                    } else {
                        mUnused[layoutMask].push_back(setHandle);
                    }
                    mUnusedCount++;
                });
    }

    VkDevice mDevice;
    VulkanResourceAllocator* mAllocator;
    VkDescriptorPool mPool;
    DescriptorCount const mCount;
    uint16_t const mCapacity;

    // Tracks the number of allocated descriptor sets.
    uint16_t mSize;
    // Tracks  the number of in-use descriptor sets.
    uint16_t mUnusedCount;

    // This maps a layout ot a list of descriptor sets allocated for that layout.
    using UnusedSetMap = std::unordered_map<Bitmask, std::vector<Handle<VulkanDescriptorSet>>,
            BitmaskHashFn, BitmaskEqual>;
    UnusedSetMap mUnused;

    bool mDisableRecycling;
};

// This is an ever-expanding pool of sets where it
//    1. Keeps a list of smaller pools of different layout-dimensions.
//    2. Will add a pool if existing pool are not compatible with the requested layout o runs out.
class DescriptorInfinitePool {
private:
    static constexpr uint16_t EXPECTED_SET_COUNT = 10;
    static constexpr float SET_COUNT_GROWTH_FACTOR = 1.5;

public:
    DescriptorInfinitePool(VkDevice device, VulkanResourceAllocator* allocator)
        : mDevice(device),
          mAllocator(allocator) {}

    Handle<VulkanDescriptorSet> obtainSet(VulkanDescriptorSetLayout* layout) {
        DescriptorPool* sameTypePool = nullptr;
        for (auto& pool: mPools) {
            if (!pool->canAllocate(layout)) {
                continue;
            }
            if (auto set = pool->obtainSet(layout); set) {
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
        mPools.push_back(std::make_unique<DescriptorPool>(mDevice, mAllocator,
                DescriptorCount::fromLayoutBitmask(layout->bitmask), capacity));
        auto& pool = mPools.back();
        auto ret = pool->obtainSet(layout);
        assert_invariant(ret && "failed to obtain a set?");
        return ret;
    }

    void disableRecycling() noexcept {
        for (auto& pool: mPools) {
            pool->disableRecycling();
        }
    }

private:
    VkDevice mDevice;
    VulkanResourceAllocator* mAllocator;
    std::vector<std::unique_ptr<DescriptorPool>> mPools;
};

class LayoutCache {
private:
    using Key = Bitmask;

    // Make sure the key is 8-bytes aligned.
    static_assert(sizeof(Key) % 8 == 0);

    using LayoutMap = std::unordered_map<Key, Handle<VulkanDescriptorSetLayout>, BitmaskHashFn,
            BitmaskEqual>;

public:
    explicit LayoutCache(VkDevice device, VulkanResourceAllocator* allocator)
        : mDevice(device),
          mAllocator(allocator) {}

    ~LayoutCache() {
        for (auto [key, layout]: mLayouts) {
            mAllocator->destruct<VulkanDescriptorSetLayout>(layout);
        }
        mLayouts.clear();
    }

    void destroyLayout(Handle<VulkanDescriptorSetLayout> handle) {
        for (auto [key, layout]: mLayouts) {
            if (layout == handle) {
                mLayouts.erase(key);
                break;
            }
        }
        mAllocator->destruct<VulkanDescriptorSetLayout>(handle);
    }

    Handle<VulkanDescriptorSetLayout> getLayout(descset::DescriptorSetLayout const& layout) {
        Key key = Bitmask::fromBackendLayout(layout);
        if (auto iter = mLayouts.find(key); iter != mLayouts.end()) {
            return iter->second;
        }

        VkDescriptorSetLayoutBinding toBind[MAX_BINDINGS];
        uint32_t count = 0;

        for (auto const& binding: layout.bindings) {
            VkShaderStageFlags stages = 0;
            VkDescriptorType type;

            if (binding.stageFlags & descset::ShaderStageFlags2::VERTEX) {
                stages |= VK_SHADER_STAGE_VERTEX_BIT;
            }
            if (binding.stageFlags & descset::ShaderStageFlags2::FRAGMENT) {
                stages |= VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            assert_invariant(stages != 0);

            switch (binding.type) {
                case descset::DescriptorType::UNIFORM_BUFFER: {
                    type = binding.flags == descset::DescriptorFlags::DYNAMIC_OFFSET
                                   ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                                   : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    break;
                }
                case descset::DescriptorType::SAMPLER: {
                    type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    break;
                }
                case descset::DescriptorType::INPUT_ATTACHMENT: {
                    type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                    break;
                }
            }
            toBind[count++] = {
                    .binding = binding.binding,
                    .descriptorType = type,
                    .descriptorCount = 1,
                    .stageFlags = stages,
            };
        }

        if (count == 0) {
            return {};
        }

        VkDescriptorSetLayoutCreateInfo dlinfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .bindingCount = count,
                .pBindings = toBind,
        };
        return (mLayouts[key] =
                        mAllocator->initHandle<VulkanDescriptorSetLayout>(mDevice, dlinfo, key));
    }

private:
    VkDevice mDevice;
    VulkanResourceAllocator* mAllocator;
    LayoutMap mLayouts;
};

template<typename Key>
struct Equal {
    bool operator()(Key const& k1, Key const& k2) const {
        return 0 == memcmp((const void*) &k1, (const void*) &k2, sizeof(Key));
    }
};

// TODO: Obsolete after [GDSR].
//       No need to cache afterwards.
struct UBOKey {
    uint8_t count;
    uint8_t padding[5];
    uint8_t bindings[MAX_UBO_BINDING];
    // Note that the number of bytes for above is 1 + 5 + 10 = 16, which is divisible by 8.
    static_assert((sizeof(count) + sizeof(padding) + sizeof(bindings)) % 8 == 0);

    VkBuffer buffers[MAX_UBO_BINDING];
    VkDeviceSize offsets[MAX_UBO_BINDING];
    VkDeviceSize sizes[MAX_UBO_BINDING];

    static inline UBOKey key(UBOMap const& uboMap, VulkanDescriptorSetLayout* layout) {
        UBOKey ret{
                .count = (uint8_t) layout->count.ubo,
        };
        uint8_t count = 0;
        for (uint8_t binding: layout->bindings.ubo) {
            auto const& [info, obj] = uboMap[binding];
            ret.bindings[count] = binding;
            if (obj) {
                ret.buffers[count] = info.buffer;
                ret.offsets[count] = info.offset;
                ret.sizes[count] = info.range;
            }// else we keep them as VK_NULL_HANDLE and 0s.
            count++;
        }
        return ret;
    }

    using HashFn = utils::hash::MurmurHashFn<UBOKey>;
    using Equal = Equal<UBOKey>;
};

// TODO: Obsolete after [GDSR].
//       No need to cache afterwards.
struct SamplerKey {
    uint8_t count;
    uint8_t padding[7];
    uint8_t bindings[MAX_SAMPLER_BINDING];
    static_assert(sizeof(bindings) % 8 == 0);
    VkSampler sampler[MAX_SAMPLER_BINDING];
    VkImageView imageView[MAX_SAMPLER_BINDING];
    VkImageLayout imageLayout[MAX_SAMPLER_BINDING];

    static inline SamplerKey key(SamplerMap const& samplerMap, VulkanDescriptorSetLayout* layout) {
        SamplerKey ret{
                .count = (uint8_t) layout->count.sampler,
        };
        uint8_t count = 0;
        for (uint8_t binding: layout->bindings.sampler) {
            auto const& [info, obj] = samplerMap[binding];
            ret.bindings[count] = binding;
            if (obj) {
                ret.sampler[count] = info.sampler;
                ret.imageView[count] = info.imageView;
                ret.imageLayout[count] = info.imageLayout;
            } // else keep them as VK_NULL_HANDLEs.
            count++;
        }
        return ret;
    }

    using HashFn = utils::hash::MurmurHashFn<SamplerKey>;
    using Equal = Equal<SamplerKey>;
};

// TODO: Obsolete after [GDSR].
//       No need to cache afterwards.
struct InputAttachmentKey {
    // This count should be fixed.
    uint8_t count;
    uint8_t padding[3];
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageView view = VK_NULL_HANDLE;

    static inline InputAttachmentKey key(VkDescriptorImageInfo const& info,
            VulkanDescriptorSetLayout* layout) {
        return {
                .count = (uint8_t) layout->count.inputAttachment,
                .imageLayout = info.imageLayout,
                .view = info.imageView,
        };
    }

    using HashFn = utils::hash::MurmurHashFn<InputAttachmentKey>;
    using Equal = Equal<InputAttachmentKey>;
};

// TODO: Obsolete after [GDSR].
//       No need to cache afterwards.
template<typename Key>
class LRUDescriptorSetCache {
private:
    static constexpr size_t MINIMUM_DESCRIPTOR_SETS = 100;
    static constexpr float LRU_REDUCTION_FACTOR = .8f;
    static constexpr uint64_t N_FRAMES_AGO = 20;

public:
    using SetPtr = VulkanDescriptorSet*;
    LRUDescriptorSetCache(VulkanResourceAllocator* allocator)
        : mFrame(0),
          mId(0),
          mResources(allocator) {}

    void gc() {
        mFrame++;

        // Never gc for the first N frames.
        if (mFrame < N_FRAMES_AGO) {
            return;
        }

        uint64_t const nFramesAgo = (mFrame - N_FRAMES_AGO) << 32;
        size_t const size = mCache.size();
        if (size < MINIMUM_DESCRIPTOR_SETS) {
            return;
        }

        auto const& popped = mLRU.pop((size_t) (size * LRU_REDUCTION_FACTOR), nFramesAgo);
        for (auto p: popped) {
            mCache.erase(p);
            mResources.release(p);
        }
    }

    inline SetPtr get(Key const& key) {
        if (auto itr = mCache.find(key); itr != mCache.end()) {
            auto const& ret = itr->second;
            mLRU.update(ret, (mFrame << 32) | mId++);
            return ret;
        }
        return nullptr;
    }

    void put(Key const& key, SetPtr set) {
        mLRU.update(set, (mFrame << 32) | mId++);
        mCache.put(key, set);
        mResources.acquire(set);
    }

    void erase(SetPtr set) {
        mCache.erase(set);
        mLRU.erase(set);
        mResources.release(set);
    }

    inline size_t size() const {
        return mCache.size();
    }

private:
    struct BiMap {
        using ForwardMap
                = std::unordered_map<Key, SetPtr, typename Key::HashFn, typename Key::Equal>;

        typename ForwardMap::const_iterator find(Key const& key) const {
            return forward.find(key);
        }
        typename ForwardMap::const_iterator end() const {
            return forward.end();
        }

        inline size_t size() const {
            return forward.size();
        }

        void erase(Key const& key) {
            if (auto itr = forward.find(key); itr != forward.end()) {
                auto const& ptr = itr->second;
                forward.erase(key);
                backward.erase(ptr);
            }
        }

        void erase(SetPtr ptr) {
            if (auto itr = backward.find(ptr); itr != backward.end()) {
                auto const& key = itr->second;
                forward.erase(key);
                backward.erase(ptr);
            }
        }

        void put(Key const& key, SetPtr ptr) {
            forward[key] = ptr;
            backward[ptr] = key;
        }

        SetPtr const& get(Key const& key) {
            return forward[key];
        }

    private:
        ForwardMap forward;
        std::unordered_map<SetPtr, Key> backward;
    };

    struct PriorityQueue {
        void update(SetPtr const& ptr, uint64_t priority) {
            if (auto itr = backward.find(ptr); itr != backward.end()) {
                auto const& priority = itr->second;
                forward.erase(priority);
                forward[priority] = ptr;
                backward[ptr] = priority;
            } else {
                backward[ptr] = priority;
                forward[priority] = ptr;
            }
        }

        void erase(SetPtr ptr) {
            if (auto itr = backward.find(ptr); itr != backward.end()) {
                auto const& priority = itr->second;
                forward.erase(priority);
                backward.erase(ptr);
            }
        }

        void erase(uint64_t priority) {
            if (auto itr = forward.find(priority); itr != forward.end()) {
                auto const& ptr = itr->second;
                backward.erase(ptr);
                forward.erase(itr);
            }
        }

        // Pop the lowest `popCount` elements that are equal or less than `priority`
        utils::FixedCapacityVector<SetPtr> pop(size_t popCount, uint64_t priority) {
            utils::FixedCapacityVector<SetPtr> evictions
                    = utils::FixedCapacityVector<SetPtr>::with_capacity(popCount);
            for (auto itr = forward.begin(); itr != forward.end() && popCount > 0;
                    itr++, popCount--) {
                auto const& [ipriority, ival] = *itr;
                if (ipriority > priority) {
                    break;
                }
                evictions.push_back(ival);
            }
            for (auto p: evictions) {
                erase(p);
            }
            return evictions;
        }

    private:
        std::map<uint64_t, SetPtr> forward;
        std::unordered_map<SetPtr, uint64_t> backward;
    };

    uint64_t mFrame;
    uint64_t mId;

    BiMap mCache;
    PriorityQueue mLRU;
    VulkanResourceManager mResources;
};

// TODO: Obsolete after [GDSR].
//       No need to cache afterwards.
// The purpose of this class is to ensure that each descriptor set is only written to once, and can
// be re-bound if necessary. Therefore, we'll cache a set based on its content and return a cached
// set if we find a content match.
// It also uses a LRU heuristic for caching. The implementation of the heuristic is in the above
// class LRUDescriptorSetCache.
class DescriptorSetCache {
public:
    DescriptorSetCache(VkDevice device, VulkanResourceAllocator* allocator)
        : mAllocator(allocator),
          mDescriptorPool(std::make_unique<DescriptorInfinitePool>(device, allocator)),
          mUBOCache(std::make_unique<LRUDescriptorSetCache<UBOKey>>(allocator)),
          mSamplerCache(std::make_unique<LRUDescriptorSetCache<SamplerKey>>(allocator)),
          mInputAttachmentCache(
                  std::make_unique<LRUDescriptorSetCache<InputAttachmentKey>>(allocator)) {}

    template<typename Key>
    inline std::pair<VulkanDescriptorSet*, bool> get(Key const& key,
            VulkanDescriptorSetLayout* layout) {
        if constexpr (std::is_same_v<Key, UBOKey>) {
            return get(key, *mUBOCache, layout);
        } else if constexpr (std::is_same_v<Key, SamplerKey>) {
            return get(key, *mSamplerCache, layout);
        } else if constexpr (std::is_same_v<Key, InputAttachmentKey>) {
            return get(key, *mInputAttachmentCache, layout);
        }
        PANIC_POSTCONDITION("Unexpected key type");
    }

    ~DescriptorSetCache() {
        // This will prevent the descriptor sets recycling when we destroy descriptor set caches.
        mDescriptorPool->disableRecycling();

        mInputAttachmentCache.reset();
        mSamplerCache.reset();
        mUBOCache.reset();
        mDescriptorPool.reset();
    }

    // gc() should be called at the end of everyframe
    void gc() {
        mUBOCache->gc();
        mSamplerCache->gc();
        mInputAttachmentCache->gc();
    }

private:
    template<typename Key>
    inline std::pair<VulkanDescriptorSet*, bool> get(Key const& key,
            LRUDescriptorSetCache<Key>& cache, VulkanDescriptorSetLayout* layout) {
        if (auto set = cache.get(key); set) {
            return {set, true};
        }
        auto set = mAllocator->handle_cast<VulkanDescriptorSet*>(
            mDescriptorPool->obtainSet(layout));
        cache.put(key, set);
        return {set, false};
    }

    VulkanResourceAllocator* mAllocator;

    // We need to heap-allocate so that the destruction can be strictly ordered.
    std::unique_ptr<DescriptorInfinitePool> mDescriptorPool;
    std::unique_ptr<LRUDescriptorSetCache<UBOKey>> mUBOCache;
    std::unique_ptr<LRUDescriptorSetCache<SamplerKey>> mSamplerCache;
    std::unique_ptr<LRUDescriptorSetCache<InputAttachmentKey>> mInputAttachmentCache;
};

} // anonymous namespace

class VulkanDescriptorSetManager::Impl {
private:
    using GetPipelineLayoutFunction = VulkanDescriptorSetManager::GetPipelineLayoutFunction;
    using DescriptorSetVkHandles = utils::FixedCapacityVector<VkDescriptorSet>;

    static inline DescriptorSetVkHandles initDescSetHandles() {
        return DescriptorSetVkHandles::with_capacity(
                VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT);
    }

    struct BoundState {
        BoundState()
            : cmdbuf(VK_NULL_HANDLE),
              pipelineLayout(VK_NULL_HANDLE),
              vkSets(initDescSetHandles()) {}

        inline bool operator==(BoundState const& b) const {
            if (cmdbuf != b.cmdbuf || pipelineLayout != b.pipelineLayout) {
                return false;
            }
            for (size_t i = 0; i < vkSets.size(); ++i) {
                if (vkSets[i] != b.vkSets[i]) {
                    return false;
                }
            }
            return true;
        }

        inline bool operator!=(BoundState const& b) const {
            return !(*this == b);
        }

        inline bool valid() noexcept {
            return cmdbuf != VK_NULL_HANDLE;
        }

        VkCommandBuffer cmdbuf;
        VkPipelineLayout pipelineLayout;
        DescriptorSetVkHandles vkSets;
        VulkanDescriptorSetLayoutList layouts;
    };

    static constexpr uint8_t UBO_SET_ID = 0;
    static constexpr uint8_t SAMPLER_SET_ID = 1;
    static constexpr uint8_t INPUT_ATTACHMENT_SET_ID = 2;

public:
    Impl(VkDevice device, VulkanResourceAllocator* allocator)
        : mDevice(device),
          mAllocator(allocator),
          mLayoutCache(device, allocator),
          mDescriptorSetCache(device, allocator),
          mHaveDynamicUbos(false),
          mResources(allocator) {}

    VkPipelineLayout bind(VulkanCommandBuffer* commands, VulkanProgram* program,
            GetPipelineLayoutFunction& getPipelineLayoutFn) {
        FVK_SYSTRACE_CONTEXT();
        FVK_SYSTRACE_START("bind");

        VulkanDescriptorSetLayoutList layouts;
        if (auto itr = mLayoutStash.find(program); itr != mLayoutStash.end()) {
            layouts = itr->second;
        } else {
            auto const& layoutDescriptions = program->getLayoutDescriptionList();
            uint8_t count = 0;
            for (auto const& description: layoutDescriptions) {
                layouts[count++] = createLayout(description);
            }
            mLayoutStash[program] = layouts;
        }

        VulkanDescriptorSetLayoutList outLayouts = layouts;
        DescriptorSetVkHandles vkDescSets = initDescSetHandles();
        VkWriteDescriptorSet descriptorWrites[MAX_BINDINGS];
        uint32_t nwrites = 0;

        // Use placeholders when necessary
        for (uint8_t i = 0; i < VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT; ++i) {
            if (!layouts[i]) {
                if (i == INPUT_ATTACHMENT_SET_ID ||
                        (i == SAMPLER_SET_ID && !layouts[INPUT_ATTACHMENT_SET_ID])) {
                    continue;
                }
                outLayouts[i] = getPlaceHolderLayout(i);
            } else {
                outLayouts[i] = layouts[i];
                auto p = mAllocator->handle_cast<VulkanDescriptorSetLayout*>(layouts[i]);
                if (!((i == UBO_SET_ID && p->bitmask.ubo)
                        || (i == SAMPLER_SET_ID && p->bitmask.sampler)
                        || (i == INPUT_ATTACHMENT_SET_ID && p->bitmask.inputAttachment
                                && mInputAttachment.first.texture))) {
                    outLayouts[i] = getPlaceHolderLayout(i);
                }
            }
        }

        for (uint8_t i = 0; i < VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT; ++i) {
            if (!outLayouts[i]) {
                continue;
            }
            VulkanDescriptorSetLayout* layout
                    = mAllocator->handle_cast<VulkanDescriptorSetLayout*>(outLayouts[i]);
            bool const usePlaceholder = layouts[i] != outLayouts[i];

            auto const& [set, cached] = getSet(i, layout);
            VkDescriptorSet const vkSet = set->vkSet;
            commands->acquire(set);
            vkDescSets.push_back(vkSet);

            // Note that we still need to bind the set, but 'cached' means that we found a set with
            // the exact same content already written, and we would just bind that one instead.
            // We also don't need to write to the placeholder set.
            if (cached || usePlaceholder) {
                continue;
            }

            switch (i) {
                case UBO_SET_ID: {
                    for (uint8_t binding: layout->bindings.ubo) {
                        auto const& [info, ubo] = mUboMap[binding];
                        descriptorWrites[nwrites++] = {
                                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet = vkSet,
                                .dstBinding = binding,
                                .descriptorCount = 1,
                                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                .pBufferInfo = ubo ? &info : &mPlaceHolderBufferInfo,
                        };
                        if (ubo) {
                            set->resources.acquire(ubo);
                        }
                    }
                    break;
                }
                case SAMPLER_SET_ID: {
                    for (uint8_t binding: layout->bindings.sampler) {
                        auto const& [info, texture] = mSamplerMap[binding];
                        descriptorWrites[nwrites++] = {
                                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = nullptr,
                                .dstSet = vkSet,
                                .dstBinding = binding,
                                .descriptorCount = 1,
                                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                .pImageInfo = texture ? &info : &mPlaceHolderImageInfo,
                        };
                        if (texture) {
                            set->resources.acquire(texture);
                        }
                    }
                    break;
                }
                case INPUT_ATTACHMENT_SET_ID: {
                    descriptorWrites[nwrites++] = {
                            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                            .pNext = nullptr,
                            .dstSet = vkSet,
                            .dstBinding = 0,
                            .descriptorCount = 1,
                            .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
                            .pImageInfo = &mInputAttachment.second,
                    };
                    set->resources.acquire(mInputAttachment.first.texture);
                    break;
                }
                default:
                    PANIC_POSTCONDITION("Invalid set id=%d", i);
            }
        }

        if (nwrites) {
            vkUpdateDescriptorSets(mDevice, nwrites, descriptorWrites, 0, nullptr);
        }

        VkPipelineLayout const pipelineLayout = getPipelineLayoutFn(outLayouts, program);
        VkCommandBuffer const cmdbuffer = commands->buffer();

        BoundState state{};
        state.cmdbuf = cmdbuffer;
        state.pipelineLayout = pipelineLayout;
        state.vkSets = vkDescSets;
        state.layouts = layouts;

        if (state != mBoundState) {
            vkCmdBindDescriptorSets(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                    vkDescSets.size(), vkDescSets.data(), 0, nullptr);
            mBoundState = state;
        }

        // Once bound, the resources are now ref'd in the descriptor set and the some resources can
        // be released and the descriptor set is ref'd by the command buffer.
        for (uint8_t i = 0; i < mSamplerMap.size(); ++i) {
            auto const& [info, texture] = mSamplerMap[i];
            if (texture) {
                mResources.release(texture);
            }
            mSamplerMap[i] = {{}, nullptr};
        }
        mInputAttachment = {};
        mHaveDynamicUbos = false;

        FVK_SYSTRACE_END();
        return pipelineLayout;
    }

    void dynamicBind(VulkanCommandBuffer* commands, Handle<VulkanDescriptorSetLayout> uboLayout) {
        if (!mHaveDynamicUbos) {
            return;
        }
        FVK_SYSTRACE_CONTEXT();
        FVK_SYSTRACE_START("dynamic-bind");

        assert_invariant(mBoundState.valid());
        assert_invariant(commands->buffer() == mBoundState.cmdbuf);

        auto layout = mAllocator->handle_cast<VulkanDescriptorSetLayout*>(
                mBoundState.layouts[UBO_SET_ID]);

        // Note that this is costly, instead just use dynamic UBOs with dynamic offsets.
        auto const& [set, cached] = getSet(UBO_SET_ID, layout);
        VkDescriptorSet const vkSet = set->vkSet;

        if (!cached) {
            VkWriteDescriptorSet descriptorWrites[MAX_UBO_BINDING];
            uint8_t nwrites = 0;

            for (uint8_t binding: layout->bindings.ubo) {
                auto const& [info, ubo] = mUboMap[binding];
                descriptorWrites[nwrites++] = {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = vkSet,
                        .dstBinding = binding,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pBufferInfo = ubo ? &info : &mPlaceHolderBufferInfo,
                };
                if (ubo) {
                    set->resources.acquire(ubo);
                }
            }
            if (nwrites > 0) {
                vkUpdateDescriptorSets(mDevice, nwrites, descriptorWrites, 0, nullptr);
            }
        }
        commands->acquire(set);

        if (mBoundState.vkSets[UBO_SET_ID] != vkSet) {
            vkCmdBindDescriptorSets(mBoundState.cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    mBoundState.pipelineLayout, 0, 1, &vkSet, 0, nullptr);
            mBoundState.vkSets[UBO_SET_ID] = vkSet;
        }
        mHaveDynamicUbos = false;
        FVK_SYSTRACE_END();
    }

    void clearProgram(VulkanProgram* program) noexcept {
        mLayoutStash.erase(program);
    }

    Handle<VulkanDescriptorSetLayout> createLayout(
            descset::DescriptorSetLayout const& description) {
        return mLayoutCache.getLayout(description);
    }

    void destroyLayout(Handle<VulkanDescriptorSetLayout> layout) {
        mLayoutCache.destroyLayout(layout);
    }

    // Note that before [GDSR] arrives, the "update" methods stash state within this class and is
    // not actually working with respect to a descriptor set.
    void updateBuffer(Handle<VulkanDescriptorSet>, uint8_t binding,
            VulkanBufferObject* bufferObject, VkDeviceSize offset, VkDeviceSize size) noexcept {
        VkDescriptorBufferInfo const info{
                .buffer = bufferObject->buffer.getGpuBuffer(),
                .offset = offset,
                .range = size,
        };
        mUboMap[binding] = {info, bufferObject};
        mResources.acquire(bufferObject);

        if (!mHaveDynamicUbos && mBoundState.valid()) {
            mHaveDynamicUbos = true;
        }
    }

    void updateSampler(Handle<VulkanDescriptorSet>, uint8_t binding, VulkanTexture* texture,
            VkSampler sampler) noexcept {
        VkDescriptorImageInfo info{
                .sampler = sampler,
        };
        VkImageSubresourceRange const range = texture->getPrimaryViewRange();
        VkImageViewType const expectedType = texture->getViewType();
        if (any(texture->usage & TextureUsage::DEPTH_ATTACHMENT)
                && expectedType == VK_IMAGE_VIEW_TYPE_2D) {
            // If the sampler is part of a mipmapped depth texture, where one of the level *can* be
            // an attachment, then the sampler for this texture has the same view properties as a
            // view for an attachment. Therefore, we can use getAttachmentView to get a
            // corresponding VkImageView.
            info.imageView = texture->getAttachmentView(range);
        } else {
            info.imageView = texture->getViewForType(range, expectedType);
        }
        info.imageLayout = imgutil::getVkLayout(texture->getPrimaryImageLayout());
        mSamplerMap[binding] = {info, texture};
        mResources.acquire(texture);
    }

    void updateInputAttachment(Handle<VulkanDescriptorSet>, VulkanAttachment attachment) noexcept {
        VkDescriptorImageInfo info = {
                .imageView = attachment.getImageView(),
                .imageLayout = imgutil::getVkLayout(attachment.getLayout()),
        };
        mInputAttachment = {attachment, info};
        mResources.acquire(attachment.texture);
    }

    void clearBuffer(uint32_t binding) {
        auto const& [info, ubo] = mUboMap[binding];
        if (ubo) {
            mResources.release(ubo);
        }
        mUboMap[binding] = {{}, nullptr};
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

    void clearState() noexcept {
        mHaveDynamicUbos = false;
        if (mInputAttachment.first.texture) {
            mResources.release(mInputAttachment.first.texture);
        }
        mInputAttachment = {};
        mBoundState = {};
    }

    inline void gc() {
        mDescriptorSetCache.gc();
    }

private:
    inline std::pair<VulkanDescriptorSet*, bool> getSet(uint8_t const setIndex,
            VulkanDescriptorSetLayout* layout) {
        switch (setIndex) {
            case UBO_SET_ID: {
                auto key = UBOKey::key(mUboMap, layout);
                return mDescriptorSetCache.get(key, layout);
            }
            case SAMPLER_SET_ID: {
                auto key = SamplerKey::key(mSamplerMap, layout);
                return mDescriptorSetCache.get(key, layout);
            }
            case INPUT_ATTACHMENT_SET_ID: {
                auto key = InputAttachmentKey::key(mInputAttachment.second, layout);
                return mDescriptorSetCache.get(key, layout);
            }
            default:
                PANIC_POSTCONDITION("Invalid set-id=%d", setIndex);
        }
    }

    inline Handle<VulkanDescriptorSetLayout> getPlaceHolderLayout(uint8_t setID) {
        if (mPlaceholderLayout[setID]) {
            return mPlaceholderLayout[setID];
        }
        descset::DescriptorSetLayout inputLayout {
                .bindings = {{}},
        };
        switch (setID) {
            case UBO_SET_ID:
                inputLayout.bindings[0] = {
                        .type = descset::DescriptorType::UNIFORM_BUFFER,
                        .stageFlags = descset::ShaderStageFlags2::VERTEX,
                        .binding = 0,
                        .flags = descset::DescriptorFlags::NONE,
                        .count = 0,
                };
                break;
            case SAMPLER_SET_ID:
                inputLayout.bindings[0] = {
                        .type = descset::DescriptorType::SAMPLER,
                        .stageFlags = descset::ShaderStageFlags2::FRAGMENT,
                        .binding = 0,
                        .flags = descset::DescriptorFlags::NONE,
                        .count = 0,
                };
                break;
            case INPUT_ATTACHMENT_SET_ID:
                inputLayout.bindings[0] = {
                        .type = descset::DescriptorType::INPUT_ATTACHMENT,
                        .stageFlags = descset::ShaderStageFlags2::FRAGMENT,
                        .binding = 0,
                        .flags = descset::DescriptorFlags::NONE,
                        .count = 0,
                };
                break;
            default:
                PANIC_POSTCONDITION("Unexpected set id=%d", setID);
        }
        mPlaceholderLayout[setID] = mLayoutCache.getLayout(inputLayout);
        return mPlaceholderLayout[setID];
    }

    VkDevice mDevice;
    VulkanResourceAllocator* mAllocator;
    LayoutCache mLayoutCache;
    DescriptorSetCache mDescriptorSetCache;
    bool mHaveDynamicUbos;
    UBOMap mUboMap;
    SamplerMap mSamplerMap;
    std::pair<VulkanAttachment, VkDescriptorImageInfo> mInputAttachment;
    VulkanResourceManager mResources;
    VkDescriptorBufferInfo mPlaceHolderBufferInfo;
    VkDescriptorImageInfo mPlaceHolderImageInfo;
    std::unordered_map<VulkanProgram*, VulkanDescriptorSetLayoutList> mLayoutStash;
    BoundState mBoundState;
    VulkanDescriptorSetLayoutList mPlaceholderLayout = {};
};

VulkanDescriptorSetManager::VulkanDescriptorSetManager(VkDevice device,
        VulkanResourceAllocator* resourceAllocator)
    : mImpl(new Impl(device, resourceAllocator)) {}

void VulkanDescriptorSetManager::terminate() noexcept {
    assert_invariant(mImpl);
    delete mImpl;
    mImpl = nullptr;
}

void VulkanDescriptorSetManager::gc() {
    mImpl->gc();
}

VkPipelineLayout VulkanDescriptorSetManager::bind(VulkanCommandBuffer* commands,
        VulkanProgram* program,
        VulkanDescriptorSetManager::GetPipelineLayoutFunction& getPipelineLayoutFn) {
    return mImpl->bind(commands, program, getPipelineLayoutFn);
}

void VulkanDescriptorSetManager::dynamicBind(VulkanCommandBuffer* commands,
        Handle<VulkanDescriptorSetLayout> uboLayout) {
    mImpl->dynamicBind(commands, uboLayout);
}

void VulkanDescriptorSetManager::clearProgram(VulkanProgram* program) noexcept {
    mImpl->clearProgram(program);
}

Handle<VulkanDescriptorSetLayout> VulkanDescriptorSetManager::createLayout(
        descset::DescriptorSetLayout const& layout) {
    return mImpl->createLayout(layout);
}

void VulkanDescriptorSetManager::destroyLayout(Handle<VulkanDescriptorSetLayout> layout) {
    mImpl->destroyLayout(layout);
}

void VulkanDescriptorSetManager::updateBuffer(Handle<VulkanDescriptorSet> set,
        uint8_t binding, VulkanBufferObject* bufferObject, VkDeviceSize offset,
        VkDeviceSize size) noexcept {
    mImpl->updateBuffer(set, binding, bufferObject, offset, size);
}

void VulkanDescriptorSetManager::updateSampler(Handle<VulkanDescriptorSet> set,
        uint8_t binding, VulkanTexture* texture, VkSampler sampler) noexcept {
    mImpl->updateSampler(set, binding, texture, sampler);
}

void VulkanDescriptorSetManager::updateInputAttachment(Handle<VulkanDescriptorSet> set, VulkanAttachment attachment) noexcept {
    mImpl->updateInputAttachment(set, attachment);
}

void VulkanDescriptorSetManager::clearBuffer(uint32_t bindingIndex) {
    mImpl->clearBuffer(bindingIndex);
}

void VulkanDescriptorSetManager::setPlaceHolders(VkSampler sampler, VulkanTexture* texture,
        VulkanBufferObject* bufferObject) noexcept {
    mImpl->setPlaceHolders(sampler, texture, bufferObject);
}

void VulkanDescriptorSetManager::clearState() noexcept { mImpl->clearState(); }


}// namespace filament::backend
