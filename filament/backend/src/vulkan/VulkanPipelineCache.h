/*
 * Copyright (C) 2022 The Android Open Source Project
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
#include <type_traits>
#include <vector>

#include "VulkanCommands.h"

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)
VK_DEFINE_HANDLE(VmaPool)

namespace filament {
namespace backend {

struct VulkanProgram;

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

    static constexpr uint32_t UBUFFER_BINDING_COUNT = Program::BINDING_COUNT;
    static constexpr uint32_t SAMPLER_BINDING_COUNT = backend::MAX_SAMPLER_COUNT;
    static constexpr uint32_t TARGET_BINDING_COUNT = MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT;
    static constexpr uint32_t SHADER_MODULE_COUNT = 2;
    static constexpr uint32_t VERTEX_ATTRIBUTE_COUNT = backend::MAX_VERTEX_ATTRIBUTE_COUNT;

    // Three descriptor set layouts: uniforms, combined image samplers, and input attachments.
    static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 3;
    static constexpr uint32_t INITIAL_DESCRIPTOR_SET_POOL_SIZE = 512;

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
    struct RasterState {
        struct {
            VkBool32                                   depthClampEnable;
            VkBool32                                   rasterizerDiscardEnable;
            VkPolygonMode                              polygonMode;
            VkCullModeFlags                            cullMode;
            VkFrontFace                                frontFace;
            VkBool32                                   depthBiasEnable;
            float                                      depthBiasConstantFactor;
            float                                      depthBiasClamp;
            float                                      depthBiasSlopeFactor;
            float                                      lineWidth;
        } rasterization;                              // 40 bytes
        VkPipelineColorBlendAttachmentState blending; // 32 bytes
        struct {
            VkBool32                                  depthTestEnable;
            VkBool32                                  depthWriteEnable;
            VkCompareOp                               depthCompareOp;
            VkBool32                                  depthBoundsTestEnable;
            VkBool32                                  stencilTestEnable;
            float                                     minDepthBounds;
            float                                     maxDepthBounds;
        } depthStencil;                               // 28 bytes
        struct {
            VkSampleCountFlagBits                    rasterizationSamples;
            VkBool32                                 sampleShadingEnable;
            float                                    minSampleShading;
            VkBool32                                 alphaToCoverageEnable;
            VkBool32                                 alphaToOneEnable;
        } multisampling;                             // 20 bytes
        uint32_t colorTargetCount;
    };

    static_assert(std::is_trivially_copyable<RasterState>::value,
            "RasterState must be a POD for fast hashing.");

    static_assert(sizeof(RasterState) == 124, "RasterState must not have any padding.");

    struct UniformBufferBinding {
        VkBuffer buffer;
        VkDeviceSize offset;
        VkDeviceSize size;
    };

    // Upon construction, the pipeCache initializes some internal state but does not make any Vulkan
    // calls. On destruction it will free any cached Vulkan objects that haven't already been freed.
    VulkanPipelineCache();
    ~VulkanPipelineCache();
    void setDevice(VkDevice device, VmaAllocator allocator);

    // Clients should initialize their copy of the raster state using this method. They can then
    // mutate their copy and pass it back through bindRasterState().
    const RasterState& getDefaultRasterState() const { return mDefaultRasterState; }

    // Creates new descriptor sets if necessary and binds them using vkCmdBindDescriptorSets.
    // Returns false if descriptor set allocation fails.
    bool bindDescriptors(VkCommandBuffer cmdbuffer) noexcept;

    // Creates a new pipeline if necessary and binds it using vkCmdBindPipeline.
    // Returns false if an error occurred.
    bool bindPipeline(VkCommandBuffer cmdbuffer) noexcept;

    // Sets up a new scissor rectangle if it has been dirtied.
    void bindScissor(VkCommandBuffer cmdbuffer, VkRect2D scissor) noexcept;

    // Each of the following methods are fast and do not make Vulkan calls.
    void bindProgram(const VulkanProgram& program) noexcept;
    void bindRasterState(const RasterState& rasterState) noexcept;
    void bindRenderPass(VkRenderPass renderPass, int subpassIndex) noexcept;
    void bindPrimitiveTopology(VkPrimitiveTopology topology) noexcept;
    void bindUniformBuffer(uint32_t bindingIndex, VkBuffer uniformBuffer,
            VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) noexcept;
    void bindSamplers(VkDescriptorImageInfo samplers[SAMPLER_BINDING_COUNT]) noexcept;
    void bindInputAttachment(uint32_t bindingIndex, VkDescriptorImageInfo imageInfo) noexcept;
    void bindVertexArray(const VertexArray& varray) noexcept;

    // Gets the current UBO at the given slot, useful for push / pop.
    UniformBufferBinding getUniformBufferBinding(uint32_t bindingIndex) const noexcept;

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
    void setDummyTexture(VkImageView imageView) {
        mDummySamplerInfo.imageView = imageView;
        mDummyTargetInfo.imageView = imageView;
    }

private:

    // PIPELINE LAYOUT CACHE KEY
    // -------------------------

    // The cache key for pipeline layouts represents 32 samplers, each with 2 bits (one for each
    // shader stage).
    using PipelineLayoutKey = utils::bitset64;

    struct PipelineLayoutKeyHashFn {
        size_t operator()(const PipelineLayoutKey& key) const;
    };

    struct PipelineLayoutKeyEqual {
        bool operator()(const PipelineLayoutKey& k1, const PipelineLayoutKey& k2) const;
    };

    // PIPELINE CACHE KEY
    // ------------------

    // The pipeline key is a POD that represents all currently bound states that form the immutable
    // VkPipeline object.
    struct PipelineKey {
        VkShaderModule shaders[SHADER_MODULE_COUNT]; // 16 bytes
        RasterState rasterState;                     // 124 bytes
        VkPrimitiveTopology topology;                // 4 bytes
        VkRenderPass renderPass;                     // 8 bytes
        uint16_t subpassIndex;                       // 2 bytes
        uint16_t padding0;                           // 2 bytes
        VkVertexInputAttributeDescription vertexAttributes[VERTEX_ATTRIBUTE_COUNT]; // 256 bytes
        VkVertexInputBindingDescription vertexBuffers[VERTEX_ATTRIBUTE_COUNT];      // 192 bytes
        uint32_t padding1;                                                          // 4 bytes
    };

    static_assert(sizeof(VkVertexInputBindingDescription) == 12);

    static_assert(offsetof(PipelineKey, rasterState)      == 16);
    static_assert(offsetof(PipelineKey, topology)         == 140);
    static_assert(offsetof(PipelineKey, renderPass)       == 144);
    static_assert(offsetof(PipelineKey, subpassIndex)     == 152);
    static_assert(offsetof(PipelineKey, vertexAttributes) == 156);
    static_assert(offsetof(PipelineKey, vertexBuffers)    == 412);
    static_assert(sizeof(PipelineKey) == 608, "PipelineKey must not have any padding.");

    static_assert(std::is_trivially_copyable<PipelineKey>::value,
            "PipelineKey must be a POD for fast hashing.");

    using PipelineHashFn = utils::hash::MurmurHashFn<PipelineKey>;

    struct PipelineEqual {
        bool operator()(const PipelineKey& k1, const PipelineKey& k2) const;
    };

    // DESCRIPTOR SET CACHE KEY
    // ------------------------

    #pragma pack(push, 1)

    // Equivalent to VkDescriptorImageInfo but with explicit padding.
    struct UTILS_PACKED DescriptorImageInfo {
        DescriptorImageInfo& operator=(VkDescriptorImageInfo& that) {
            sampler = that.sampler;
            imageView = that.imageView;
            imageLayout = that.imageLayout;
            padding = 0;
            return *this;
        }
        operator VkDescriptorImageInfo() const { return { sampler, imageView, imageLayout }; }
        VkSampler sampler;
        VkImageView imageView;
        VkImageLayout imageLayout;
        uint32_t padding;
    };

    // Represents all the Vulkan state that comprises a bound descriptor set.
    struct UTILS_PACKED DescriptorKey {
        VkBuffer uniformBuffers[UBUFFER_BINDING_COUNT];
        DescriptorImageInfo samplers[SAMPLER_BINDING_COUNT];
        DescriptorImageInfo inputAttachments[TARGET_BINDING_COUNT];
        VkDeviceSize uniformBufferOffsets[UBUFFER_BINDING_COUNT];
        VkDeviceSize uniformBufferSizes[UBUFFER_BINDING_COUNT];
    };

    #pragma pack(pop)

    static_assert(std::is_trivially_copyable<DescriptorKey>::value, "DescriptorKey must be a POD.");

    using DescHashFn = utils::hash::MurmurHashFn<DescriptorKey>;

    struct DescEqual {
        bool operator()(const DescriptorKey& k1, const DescriptorKey& k2) const;
    };

    // CACHE ENTRY STRUCTS
    // -------------------

    // The timestamp associated with a given cache entry represents time as a count of flush
    // events since the cache was constructed. If any cache entry was most recently used over
    // VK_MAX_PIPELINE_AGE flushes in the past, then we can be sure that it is no longer
    // being used by the GPU, and is therefore safe to destroy or reclaim.
    using Timestamp = uint64_t;
    Timestamp mCurrentTime = 0;

    // The descriptor set cache entry is a group of descriptor sets that are bound simultaneously.
    struct DescriptorCacheEntry {
        std::array<VkDescriptorSet, DESCRIPTOR_TYPE_COUNT> handles;
        Timestamp lastUsed;
        PipelineLayoutKey pipelineLayout;
    };

    struct PipelineCacheEntry {
        VkPipeline handle;
        Timestamp lastUsed;
    };

    struct PipelineLayoutCacheEntry {
        VkPipelineLayout handle;
        Timestamp lastUsed;

        std::array<VkDescriptorSetLayout, DESCRIPTOR_TYPE_COUNT> descriptorSetLayouts;

        // Each pipeline layout has 3 arenas of unused descriptors (one for each binding type).
        //
        // The difference between the "arenas" and the "pool" are as follows.
        //
        // - The "pool" is a single, centralized factory for all descriptors (VkDescriptorPool).
        //
        // - Each "arena" is a set of unused (but alive) descriptors that can only be used with a
        //   specific pipeline layout and a specific binding type. We manually manage each arena.
        //   The arenas are created in an empty state, and they are gradually populated as new
        //   descriptors are reclaimed over time.  This is quite different from the pool, which is
        //   given a fixed size when it is constructed.
        //
        std::array<std::vector<VkDescriptorSet>, DESCRIPTOR_TYPE_COUNT> descriptorSetArenas;
    };

    // CACHE CONTAINERS
    // ----------------

    using PipelineLayoutMap = tsl::robin_map<PipelineLayoutKey , PipelineLayoutCacheEntry,
            PipelineLayoutKeyHashFn, PipelineLayoutKeyEqual>;
    using PipelineMap = tsl::robin_map<PipelineKey, PipelineCacheEntry,
            PipelineHashFn, PipelineEqual>;
    using DescriptorMap = tsl::robin_map<DescriptorKey, DescriptorCacheEntry,
            DescHashFn, DescEqual>;

    PipelineLayoutMap mPipelineLayouts;
    PipelineMap mPipelines;
    DescriptorMap mDescriptorSets;

    // These helpers all return unstable pointers that should not be stored.
    DescriptorCacheEntry* createDescriptorSets() noexcept;
    PipelineCacheEntry* createPipeline() noexcept;
    PipelineLayoutCacheEntry* getOrCreatePipelineLayout() noexcept;

    // Misc helper methods.
    void destroyLayoutsAndDescriptors() noexcept;
    VkDescriptorPool createDescriptorPool(uint32_t size) const;
    void growDescriptorPool() noexcept;

    // Immutable state.
    VkDevice mDevice = VK_NULL_HANDLE;
    VmaAllocator mAllocator = VK_NULL_HANDLE;
    const RasterState mDefaultRasterState;

    // Current requirements for the pipeline layout, pipeline, and descriptor sets.
    PipelineLayoutKey mLayoutRequirements = {};
    PipelineKey mPipelineRequirements = {};
    DescriptorKey mDescriptorRequirements = {};

    // Current bindings for the pipeline layout, pipeline, and descriptor sets.
    PipelineLayoutKey mBoundLayout = {};
    PipelineKey mBoundPipeline = {};
    DescriptorKey mBoundDescriptor = {};

    // Current state for scissoring.
    VkRect2D mCurrentScissor = {};

    // The descriptor set pool starts out with a decent number of descriptor sets.  The cache can
    // grow the pool by re-creating it with a larger size.  See growDescriptorPool().
    VkDescriptorPool mDescriptorPool;
    uint32_t mDescriptorPoolSize = INITIAL_DESCRIPTOR_SET_POOL_SIZE;

    // After a growth event (i.e. when the VkDescriptorPool is replaced with a bigger version), all
    // currently used descriptors are moved into the "extinct" sets so that they can be safely
    // destroyed a few frames later.
    std::vector<VkDescriptorPool> mExtinctDescriptorPools;
    std::vector<DescriptorCacheEntry> mExtinctDescriptorBundles;

    VkDescriptorBufferInfo mDummyBufferInfo = {};
    VkWriteDescriptorSet mDummyBufferWriteInfo = {};
    VkDescriptorImageInfo mDummySamplerInfo = {};
    VkWriteDescriptorSet mDummySamplerWriteInfo = {};
    VkDescriptorImageInfo mDummyTargetInfo = {};
    VkWriteDescriptorSet mDummyTargetWriteInfo = {};

    VkBuffer mDummyBuffer;
    VmaAllocation mDummyMemory;
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANPIPELINECACHE_H
