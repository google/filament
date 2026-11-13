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

#ifndef TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSETCACHE_H
#define TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSETCACHE_H

#include "VulkanHandles.h"

#include "vulkan/memory/ResourcePointer.h"
#include "vulkan/utils/CommandBufferAgeCounter.h"
#include "vulkan/utils/Definitions.h"  // For DescriptorSetMask

#include <backend/DriverEnums.h>
#include <backend/Program.h>
#include <backend/TargetBufferInfo.h>

#include <bluevk/BlueVK.h>

#include <utils/bitset.h>

#include <tsl/robin_map.h>

#include <array>
#include <cstdint>
#include <memory>

namespace filament::backend {

// Abstraction over the descriptor set pool.
class VulkanDescriptorSetCache {
public:
    static constexpr uint8_t UNIQUE_DESCRIPTOR_SET_COUNT =
            VulkanDescriptorSetLayout::UNIQUE_DESCRIPTOR_SET_COUNT;

    using DescriptorSetLayoutArray = VulkanDescriptorSetLayout::DescriptorSetLayoutArray;
    using DescriptorSetArray =
            std::array<fvkmemory::resource_ptr<VulkanDescriptorSet>, UNIQUE_DESCRIPTOR_SET_COUNT>;
    using DescriptorCount = VulkanDescriptorSetLayout::Count;

    // `ageCounter` is the counter shared by the command buffer pools (see
    // VulkanCommands::getAgeCounter()), and must outlive this cache.
    VulkanDescriptorSetCache(VkDevice device, fvkmemory::ResourceManager* resourceManager,
            fvkutils::CommandBufferAgeCounter const& ageCounter);
    ~VulkanDescriptorSetCache();

    void terminate() noexcept;

    void updateBuffer(fvkmemory::resource_ptr<VulkanDescriptorSet> set, uint8_t binding,
            fvkmemory::resource_ptr<VulkanBufferObject> bufferObject, VkDeviceSize offset,
            VkDeviceSize size) noexcept;

    void updateSampler(fvkmemory::resource_ptr<VulkanDescriptorSet> set, uint8_t binding,
            fvkmemory::resource_ptr<VulkanTexture> texture, VkSampler sampler) noexcept;

    void updateInputAttachment(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
            VulkanAttachment const& attachment) noexcept;

    void bind(uint8_t setIndex, fvkmemory::resource_ptr<VulkanDescriptorSet> set,
            backend::DescriptorSetOffsetArray&& offsets);

    void unbind(uint8_t setIndex);

    void commit(VulkanCommandBuffer* commands, VkPipelineLayout pipelineLayout,
            fvkutils::DescriptorSetMask const& setMask);

    fvkmemory::resource_ptr<VulkanDescriptorSet> createSet(Handle<HwDescriptorSet> handle,
            fvkmemory::resource_ptr<VulkanDescriptorSetLayout> layout);

    // Create and set as current a new VkDescriptorSet using the `set` currently bound layout and
    // copy all the written bindings except for the samplers bindings in the `samplerMask`.
    // Uniform buffers whose VkBuffer has been swapped out since they were written (see
    // VulkanDescriptorSet::getStaleUbos()) are not copied, but rewritten with the current VkBuffer.
    void cloneSet(fvkmemory::resource_ptr<VulkanDescriptorSet> set,
            fvkutils::SamplerBitmask samplerMask) noexcept;

    // This method is meant to be used with external samplers
    VkDescriptorSet getVkSet(DescriptorCount const& count, VkDescriptorSetLayout vklayout);

    // This method is meant to be used with external samplers
    void manualRecycle(VulkanDescriptorSetLayout::Count const& count, VkDescriptorSetLayout vklayout,
            VkDescriptorSet vkSet);

    // Returns the sets stashed by bind() since the last commit(). These are not necessarily the
    // sets that are bound in the command buffer; see mLastBoundInfo for those.
    DescriptorSetArray const& getStashedSets() const { return mStashedSets; }

    void gc();

    void resetCachedState() noexcept { mLastBoundInfo = {}; }

private:
    void copySet(VkDescriptorSet srcSet, VkDescriptorSet destSet,
            fvkutils::SamplerBitmask copyBindings) const;

    class DescriptorInfinitePool;

    VkDevice mDevice;
    fvkmemory::ResourceManager* mResourceManager;
    // Tells which command buffers are still in flight. It is used to record which command buffers
    // bind a set, and to only recycle a retired VkDescriptorSet once all of them have completed.
    fvkutils::CommandBufferAgeCounter const& mAgeCounter;
    std::unique_ptr<DescriptorInfinitePool> mDescriptorPool;
    std::pair<VulkanAttachment, VkDescriptorImageInfo> mInputAttachment;
    DescriptorSetArray mStashedSets = {};

    // What vkCmdBindDescriptorSets has bound in the command buffer of age `age`. Descriptor set
    // bindings persist across draws (and pipeline layout changes, see commit()) within a command
    // buffer, but not across command buffers, so commit() discards this when the age changes.
    struct {
        // Age 0 is never handed out, so the initial value never matches a command buffer.
        uint64_t age = 0;
        // The pipeline layout of the most recent commit.
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        // The indices that have been bound with `pipelineLayout` since it last changed. A set bound
        // with an earlier layout may have been disturbed (in the Vulkan sense) since, so a newly
        // stashed set is only skipped if its index is in this mask.
        fvkutils::DescriptorSetMask boundWithLayout;
        // The set that was last bound at each index. It is kept to check for stale UBOs when the
        // frontend relies on a binding from an earlier draw.
        DescriptorSetArray boundSets = {};
        // The VkDescriptorSet that was last bound at each index. This is compared rather than the
        // wrapper in boundSets, because cloning a set (cloneSet()) changes its VkDescriptorSet in
        // place. A VkDescriptorSet is only recycled once no command buffer can use it, so within
        // one command buffer equal handles mean the same descriptors.
        std::array<VkDescriptorSet, UNIQUE_DESCRIPTOR_SET_COUNT> boundVkSets = {};
    } mLastBoundInfo;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_CACHING_VULKANDESCRIPTORSETCACHE_H
