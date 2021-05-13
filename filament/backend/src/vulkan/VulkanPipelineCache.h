/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_VULKANPIPELINECACHE_H
#define TNT_FILAMENT_DRIVER_VULKANPIPELINECACHE_H

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <private/backend/Program.h>

#include <bluevk/BlueVK.h>

#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/Hash.h>

#include <tsl/robin_map.h>
#include <vector>

#include "VulkanCommands.h"

namespace filament {
namespace backend {

// VulkanPipelineCache manages a cache of descriptor sets and pipelines.
//
// Please note the following limitations:
//
// - Push constants are not supported. (if adding support, see VkPipelineLayoutCreateInfo)
// - Only DESCRIPTOR_TYPE_COUNT descriptor sets are bound at a time.
// - Assumes that viewport and scissor should be dynamic. (not baked into VkPipeline)
// - Assumes that uniform buffers should be visible across all shader stages.
//
class VulkanPipelineCache : public CommandBufferObserver {
public:
    VulkanPipelineCache(VulkanPipelineCache const&) = delete;
    VulkanPipelineCache& operator=(VulkanPipelineCache const&) = delete;

    static constexpr uint32_t UBUFFER_BINDING_COUNT = Program::UNIFORM_BINDING_COUNT;
    static constexpr uint32_t SAMPLER_BINDING_COUNT = backend::MAX_SAMPLER_COUNT;
    static constexpr uint32_t TARGET_BINDING_COUNT = MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT;
    static constexpr uint32_t SHADER_MODULE_COUNT = 2;
    static constexpr uint32_t VERTEX_ATTRIBUTE_COUNT = backend::MAX_VERTEX_ATTRIBUTE_COUNT;

    // Three descriptor set layouts: uniforms, combined image samplers, and input attachments.
    static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 3;

    // The VertexArray POD is an array of buffer targets and an array of attributes that refer to
    // those targets. It does not include any references to actual buffers, so you can think of it
    // as a vertex assembler configuration. For simplicity it contains fixed-size arrays and does
    // not store sizes; all unused entries are simply zeroed out.
    struct VertexArray {
        VkVertexInputAttributeDescription attributes[VERTEX_ATTRIBUTE_COUNT];
        VkVertexInputBindingDescription buffers[VERTEX_ATTRIBUTE_COUNT];
    };

    // The ProgramBundle contains weak references to the compiled vertex and fragment shaders.
    struct ProgramBundle {
        VkShaderModule vertex;
        VkShaderModule fragment;
    };

    // The RasterState POD contains standard graphics-related state like blending, culling, etc.
    // Note that several fields are unused (sType etc) so we could shrink this by avoiding the Vk
    // structures. However it's super convenient just to use standard Vulkan structs here.
    struct alignas(8) RasterState {
        VkPipelineRasterizationStateCreateInfo rasterization;
        VkPipelineColorBlendAttachmentState blending;
        VkPipelineDepthStencilStateCreateInfo depthStencil;
        VkPipelineMultisampleStateCreateInfo multisampling;
        uint32_t colorTargetCount;
    };
    static_assert(std::is_pod<RasterState>::value, "RasterState must be a POD for fast hashing.");

    // Upon construction, the pipeCache initializes some internal state but does not make any Vulkan
    // calls. On destruction it will free any cached Vulkan objects that haven't already been freed.
    VulkanPipelineCache();
    ~VulkanPipelineCache();
    void setDevice(VkDevice device) { mDevice = device; }

    // Clients should initialize their copy of the raster state using this method. They can then
    // mutate their copy and pass it back through bindRasterState().
    const RasterState& getDefaultRasterState() const { return mDefaultRasterState; }

    // Creates new descriptor sets if necessary and binds them using vkCmdBindDescriptorSets.
    // Returns false if descriptor set allocation fails.
    bool bindDescriptors(VulkanCommands& commands) noexcept;

    // Creates a new pipeline if necessary and binds it using vkCmdBindPipeline.
    void bindPipeline(VulkanCommands& commands) noexcept;

    // Each of the following methods are fast and do not make Vulkan calls.
    void bindProgramBundle(const ProgramBundle& bundle) noexcept;
    void bindRasterState(const RasterState& rasterState) noexcept;
    void bindRenderPass(VkRenderPass renderPass, int subpassIndex) noexcept;
    void bindPrimitiveTopology(VkPrimitiveTopology topology) noexcept;
    void bindUniformBuffer(uint32_t bindingIndex, VkBuffer uniformBuffer,
            VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) noexcept;
    void bindSamplers(VkDescriptorImageInfo samplers[SAMPLER_BINDING_COUNT]) noexcept;
    void bindInputAttachment(uint32_t bindingIndex, VkDescriptorImageInfo imageInfo) noexcept;
    void bindVertexArray(const VertexArray& varray) noexcept;

    // Checks if the given uniform is bound to any slot, and if so binds "null" to that slot.
    // Also invalidates all cached descriptors that refer to the given buffer.
    // This is only necessary when the client knows that the UBO is about to be destroyed.
    void unbindUniformBuffer(VkBuffer uniformBuffer) noexcept;

    // Checks if an image view is bound to any sampler, and if so resets that particular slot.
    // Also invalidates all cached descriptors that refer to the given image view.
    // This is only necessary when the client knows that a texture is about to be destroyed.
    void unbindImageView(VkImageView imageView) noexcept;

    // NOTE: In theory we should proffer "unbindSampler" but in practice we never destroy samplers.

    // Destroys all managed Vulkan objects. This should be called before changing the VkDevice.
    void destroyCache() noexcept;

    // vkCmdBindPipeline and vkCmdBindDescriptorSets establish bindings to a specific command
    // buffer; they are not global to the device. Therefore we need to be notified when a
    // new command buffer becomes active.
    void onCommandBuffer(const VulkanCommandBuffer& cmdbuffer) override;

    // Injects a dummy texture that can be used to clear out old descriptor sets.
    void setDummyTexture(VkImageView imageView) { mDummyImageView = imageView; }

private:
    static constexpr uint32_t ALL_COMMAND_BUFFERS = (1 << VK_MAX_COMMAND_BUFFERS) - 1;

    // The pipeline key is a POD that represents all currently bound states that form the immutable
    // VkPipeline object. We apply a hash function to its contents only if has been mutated since
    // the previous call to getOrCreatePipeline.
    #pragma pack(push, 1)
    struct UTILS_PACKED PipelineKey {
        VkShaderModule shaders[SHADER_MODULE_COUNT]; // 8*2 bytes
        RasterState rasterState; // 248 bytes
        VkRenderPass renderPass; // 8 bytes
        VkPrimitiveTopology topology : 16; // 2 bytes
        uint16_t subpassIndex; // 2 bytes
        VkVertexInputAttributeDescription vertexAttributes[VERTEX_ATTRIBUTE_COUNT]; // 16*5 bytes
        VkVertexInputBindingDescription vertexBuffers[VERTEX_ATTRIBUTE_COUNT]; // 12*5 bytes
    };
    #pragma pack(pop)

    static_assert(std::is_pod<PipelineKey>::value, "PipelineKey must be a POD for fast hashing.");

    using PipelineHashFn = utils::hash::MurmurHashFn<PipelineKey>;

    struct PipelineEqual {
        bool operator()(const PipelineKey& k1, const PipelineKey& k2) const;
    };

    // The descriptor key is a POD that represents all currently bound states that go into the
    // descriptor set. We apply a hash function to its contents only if has been mutated since
    // the previous call to getOrCreateDescriptors.
    #pragma pack(push, 1)
    struct UTILS_PACKED DescriptorKey {
        VkBuffer uniformBuffers[UBUFFER_BINDING_COUNT];
        VkDescriptorImageInfo samplers[SAMPLER_BINDING_COUNT];
        VkDescriptorImageInfo inputAttachments[TARGET_BINDING_COUNT];
        VkDeviceSize uniformBufferOffsets[UBUFFER_BINDING_COUNT];
        VkDeviceSize uniformBufferSizes[UBUFFER_BINDING_COUNT];
    };
    #pragma pack(pop)

    static_assert(std::is_pod<DescriptorKey>::value, "DescriptorKey must be a POD.");

    using DescHashFn = utils::hash::MurmurHashFn<DescriptorKey>;

    struct DescEqual {
        bool operator()(const DescriptorKey& k1, const DescriptorKey& k2) const;
    };

    // Represents a group of descriptor sets that are bound simultaneously.
    struct DescriptorBundle {
        VkDescriptorSet handles[DESCRIPTOR_TYPE_COUNT];
        utils::bitset32 commandBuffers;
    };

    struct PipelineVal {
        VkPipeline handle;

        // The "age" of a pipeline cache entry is the number of command buffer flush events that
        // have occurred since it was last used in a command buffer. This is used for LRU caching,
        // which is a crucial feature because VkPipeline construction is very slow.
        uint32_t age;
    };

    using PipelineMap = tsl::robin_map<PipelineKey, PipelineVal, PipelineHashFn, PipelineEqual>;
    using DescriptorMap = tsl::robin_map<DescriptorKey, DescriptorBundle, DescHashFn, DescEqual>;

    struct CmdBufferState {
        // Weak references to the currently bound pipeline and descriptor sets.
        PipelineVal* currentPipeline = nullptr;
        DescriptorBundle* currentDescriptorBundle = nullptr;
    };

    // If bind is set to true, vkCmdBindDescriptorSets is required.
    // If overflow is set to true, a descriptor set allocation error has occurred.
    void getOrCreateDescriptors(VkDescriptorSet descriptors[DESCRIPTOR_TYPE_COUNT],
            bool* bind, bool* overflow) noexcept;

    // Returns true if any pipeline bindings have changed. (i.e., vkCmdBindPipeline is required)
    bool getOrCreatePipeline(VkPipeline* pipeline) noexcept;

    void createLayoutsAndDescriptors() noexcept;
    void destroyLayoutsAndDescriptors() noexcept;
    void markDirtyPipeline() noexcept { mDirtyPipeline.setValue(ALL_COMMAND_BUFFERS); }
    void markDirtyDescriptor() noexcept { mDirtyDescriptor.setValue(ALL_COMMAND_BUFFERS); }
    VkDescriptorPool createDescriptorPool(uint32_t size) const;
    void growDescriptorPool() noexcept;

    VkDevice mDevice = nullptr;
    const RasterState mDefaultRasterState;

    // Current bindings are divided into two "keys" which are composed of a mix of actual values
    // (e.g., blending is OFF) and weak references to Vulkan objects (e.g., shader programs and
    // uniform buffers).
    PipelineKey mPipelineKey;
    DescriptorKey mDescriptorKey;

    // Each command buffer has associated state, including the bindings set up by vkCmdBindPipeline
    // and vkCmdBindDescriptorSets.
    CmdBufferState mCmdBufferState[VK_MAX_COMMAND_BUFFERS];

    // One dirty bit per command buffer, stored in a bitset to permit fast "set all dirty bits". If
    // a dirty flag is set for the current command buffer, then a new pipeline or descriptor set
    // needs to be retrieved from the cache or created.
    utils::bitset32 mDirtyPipeline;
    utils::bitset32 mDirtyDescriptor;

    VkDescriptorSetLayout mDescriptorSetLayouts[DESCRIPTOR_TYPE_COUNT] = {};

    std::vector<VkDescriptorSet> mDescriptorSetArena[DESCRIPTOR_TYPE_COUNT];

    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    PipelineMap mPipelines;
    DescriptorMap mDescriptorBundles;
    uint32_t mCmdBufferIndex = 0;

    VkDescriptorPool mDescriptorPool;
    uint32_t mDescriptorPoolSize = 500;

    // After a growth event (i.e. when the VkDescriptorPool is replaced with a bigger version), all
    // currently used descriptors are moved into the "extinct" sets so that they can be safely
    // destroyed a few frames later.
    std::vector<VkDescriptorPool> mExtinctDescriptorPools = {};
    std::vector<DescriptorBundle> mExtinctDescriptorBundles;

    VkImageView mDummyImageView = VK_NULL_HANDLE;
    VkDescriptorBufferInfo mDummyBufferInfo = {};
    VkWriteDescriptorSet mDummyBufferWriteInfo = {};
    VkDescriptorImageInfo mDummySamplerInfo = {};
    VkWriteDescriptorSet mDummySamplerWriteInfo = {};
    VkDescriptorImageInfo mDummyTargetInfo = {};
    VkWriteDescriptorSet mDummyTargetWriteInfo = {};
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANPIPELINECACHE_H
