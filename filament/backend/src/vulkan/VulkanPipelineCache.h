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

#ifndef TNT_FILAMENT_BACKEND_VULKANPIPELINECACHE_H
#define TNT_FILAMENT_BACKEND_VULKANPIPELINECACHE_H

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include "backend/Program.h"

#include <bluevk/BlueVK.h>

#include <utils/bitset.h>
#include <utils/compiler.h>
#include <utils/Hash.h>

#include <list>
#include <tsl/robin_map.h>
#include <type_traits>
#include <vector>
#include <unordered_map>

#include "VulkanCommands.h"

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)
VK_DEFINE_HANDLE(VmaPool)

namespace filament::backend {

struct VulkanProgram;
struct VulkanBufferObject;
struct VulkanTexture;
class VulkanResourceAllocator;

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
    static constexpr uint32_t SAMPLER_BINDING_COUNT = MAX_SAMPLER_COUNT;

    // We assume only one possible input attachment between two subpasses. See also the subpasses
    // definition in VulkanFboCache.
    static constexpr uint32_t INPUT_ATTACHMENT_COUNT = 1;

    static constexpr uint32_t SHADER_MODULE_COUNT = 2;
    static constexpr uint32_t VERTEX_ATTRIBUTE_COUNT = MAX_VERTEX_ATTRIBUTE_COUNT;

    // Three descriptor set layouts: uniforms, combined image samplers, and input attachments.
    static constexpr uint32_t DESCRIPTOR_TYPE_COUNT = 3;
    static constexpr uint32_t INITIAL_DESCRIPTOR_SET_POOL_SIZE = 512;

    // The VertexArray POD is an array of buffer targets and an array of attributes that refer to
    // those targets. It does not include any references to actual buffers, so you can think of it
    // as a vertex assembler configuration. For simplicity it contains fixed-size arrays and does
    // not store sizes; all unused entries are simply zeroed out.
    struct VertexArray {
    };

    // The ProgramBundle contains weak references to the compiled vertex and fragment shaders.
    struct ProgramBundle {
        VkShaderModule vertex;
        VkShaderModule fragment;
        VkSpecializationInfo* specializationInfos = nullptr;
    };

    using UsageFlags = utils::bitset128;
    static UsageFlags getUsageFlags(uint16_t binding, ShaderStageFlags stages, UsageFlags src = {});
    static UsageFlags disableUsageFlags(uint16_t binding, UsageFlags src);

    #pragma clang diagnostic push
    #pragma clang diagnostic warning "-Wpadded"

    // The RasterState POD contains standard graphics-related state like blending, culling, etc.
    // The following states are omitted because Filament never changes them:
    // >>> depthClampEnable, rasterizerDiscardEnable, depthBoundsTestEnable, stencilTestEnable
    // >>> minSampleShading, alphaToOneEnable, sampleShadingEnable, minDepthBounds, maxDepthBounds,
    // >>> depthBiasClamp, polygonMode, lineWidth
    struct RasterState {
        VkCullModeFlags       cullMode : 2;
        VkFrontFace           frontFace : 2;
        VkBool32              depthBiasEnable : 1;
        VkBool32              blendEnable : 1;
        VkBool32              depthWriteEnable : 1;
        VkBool32              alphaToCoverageEnable : 1;
        VkBlendFactor         srcColorBlendFactor : 5; // offset = 1 byte
        VkBlendFactor         dstColorBlendFactor : 5;
        VkBlendFactor         srcAlphaBlendFactor : 5;
        VkBlendFactor         dstAlphaBlendFactor : 5;
        VkColorComponentFlags colorWriteMask : 4;
        uint8_t               rasterizationSamples;    // offset = 4 bytes
        uint8_t               colorTargetCount;        // offset = 5 bytes
        BlendEquation         colorBlendOp : 4;        // offset = 6 bytes
        BlendEquation         alphaBlendOp : 4;
        SamplerCompareFunc    depthCompareOp;          // offset = 7 bytes
        float                 depthBiasConstantFactor; // offset = 8 bytes
        float                 depthBiasSlopeFactor;    // offset = 12 bytes
    };

    static_assert(std::is_trivially_copyable<RasterState>::value,
            "RasterState must be a POD for fast hashing.");

    static_assert(sizeof(RasterState) == 16, "RasterState must not have implicit padding.");

    struct UniformBufferBinding {
        VkBuffer buffer;
        VkDeviceSize offset;
        VkDeviceSize size;
    };

    // Upon construction, the pipeCache initializes some internal state but does not make any Vulkan
    // calls. On destruction it will free any cached Vulkan objects that haven't already been freed.
    VulkanPipelineCache(VulkanResourceAllocator* allocator);
    ~VulkanPipelineCache();
    void setDevice(VkDevice device, VmaAllocator allocator);

    // Creates new descriptor sets if necessary and binds them using vkCmdBindDescriptorSets.
    // Returns false if descriptor set allocation fails.
    bool bindDescriptors(VkCommandBuffer cmdbuffer) noexcept;

    // Creates a new pipeline if necessary and binds it using vkCmdBindPipeline.
    // Returns false if an error occurred.
    bool bindPipeline(VulkanCommandBuffer* commands) noexcept;

    // Sets up a new scissor rectangle if it has been dirtied.
    void bindScissor(VkCommandBuffer cmdbuffer, VkRect2D scissor) noexcept;

    // Each of the following methods are fast and do not make Vulkan calls.
    void bindProgram(VulkanProgram* program) noexcept;
    void bindRasterState(const RasterState& rasterState) noexcept;
    void bindRenderPass(VkRenderPass renderPass, int subpassIndex) noexcept;
    void bindPrimitiveTopology(VkPrimitiveTopology topology) noexcept;
    void bindUniformBufferObject(uint32_t bindingIndex, VulkanBufferObject* bufferObject,
            VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) noexcept;
    void bindUniformBuffer(uint32_t bindingIndex, VkBuffer buffer,
            VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) noexcept;
    void bindSamplers(VkDescriptorImageInfo samplers[SAMPLER_BINDING_COUNT],
            VulkanTexture* textures[SAMPLER_BINDING_COUNT], UsageFlags flags) noexcept;
    void bindInputAttachment(uint32_t bindingIndex, VkDescriptorImageInfo imageInfo) noexcept;
    void bindVertexArray(VkVertexInputAttributeDescription const* attribDesc,
            VkVertexInputBindingDescription const* bufferDesc, uint8_t count);

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
    void terminate() noexcept;

    // vkCmdBindPipeline and vkCmdBindDescriptorSets establish bindings to a specific command
    // buffer; they are not global to the device. Therefore we need to be notified when a
    // new command buffer becomes active.
    void onCommandBuffer(const VulkanCommandBuffer& cmdbuffer) override;

    // Injects a dummy texture that can be used to clear out old descriptor sets.
    void setDummyTexture(VkImageView imageView) {
        mDummyTargetInfo.imageView = imageView;
    }

    // Acquires a resource to be bound to the current pipeline. The ownership of the resource
    // will be transferred to the corresponding pipeline when pipeline is bound.
    void acquireResource(VulkanResource* resource) {
        mPipelineBoundResources.acquire(resource);
    }

private:
    // PIPELINE LAYOUT CACHE KEY
    // -------------------------

    using PipelineLayoutKey = utils::bitset128;

    static_assert(PipelineLayoutKey::BIT_COUNT >= 2 * MAX_SAMPLER_COUNT);

    struct PipelineLayoutKeyHashFn {
        size_t operator()(const PipelineLayoutKey& key) const;
    };

    struct PipelineLayoutKeyEqual {
        bool operator()(const PipelineLayoutKey& k1, const PipelineLayoutKey& k2) const;
    };

    // PIPELINE CACHE KEY
    // ------------------

    // Equivalent to VkVertexInputAttributeDescription but half as big.
    struct VertexInputAttributeDescription {
        VertexInputAttributeDescription& operator=(const VkVertexInputAttributeDescription& that) {
            assert_invariant(that.location <= 0xffu);
            assert_invariant(that.binding <= 0xffu);
            assert_invariant(uint32_t(that.format) <= 0xffffu);
            location = that.location;
            binding = that.binding;
            format = that.format;
            offset = that.offset;
            return *this;
        }
        operator VkVertexInputAttributeDescription() const {
            return { location, binding, VkFormat(format), offset };
        }
        uint8_t     location;
        uint8_t     binding;
        uint16_t    format;
        uint32_t    offset;
    };

    // Equivalent to VkVertexInputBindingDescription but not as big.
    struct VertexInputBindingDescription {
        VertexInputBindingDescription& operator=(const VkVertexInputBindingDescription& that) {
            assert_invariant(that.binding <= 0xffffu);
            binding = that.binding;
            stride = that.stride;
            inputRate = that.inputRate;
            return *this;
        }
        operator VkVertexInputBindingDescription() const {
            return { binding, stride, (VkVertexInputRate) inputRate };
        }
        uint16_t binding;
        uint16_t inputRate;
        uint32_t stride;
    };

    // The pipeline key is a POD that represents all currently bound states that form the immutable
    // VkPipeline object. The size:offset comments below are expressed in bytes.
    struct PipelineKey {                                                          // size : offset
        VkShaderModule shaders[SHADER_MODULE_COUNT];                              //  16  : 0
        VkRenderPass renderPass;                                                  //  8   : 16
        uint16_t topology;                                                        //  2   : 24
        uint16_t subpassIndex;                                                    //  2   : 26
        VertexInputAttributeDescription vertexAttributes[VERTEX_ATTRIBUTE_COUNT]; //  128 : 28
        VertexInputBindingDescription vertexBuffers[VERTEX_ATTRIBUTE_COUNT];      //  128 : 156
        RasterState rasterState;                                                  //  16  : 284
        uint32_t padding;                                                         //  4   : 300
        PipelineLayoutKey layout;                                                 // 16   : 304
    };

    static_assert(sizeof(PipelineKey) == 320, "PipelineKey must not have implicit padding.");

    using PipelineHashFn = utils::hash::MurmurHashFn<PipelineKey>;

    struct PipelineEqual {
        bool operator()(const PipelineKey& k1, const PipelineKey& k2) const;
    };

    // DESCRIPTOR SET CACHE KEY
    // ------------------------

    // Equivalent to VkDescriptorImageInfo but with explicit padding.
    struct DescriptorImageInfo {
        DescriptorImageInfo& operator=(const VkDescriptorImageInfo& that) {
            sampler = that.sampler;
            imageView = that.imageView;
            imageLayout = that.imageLayout;
            padding = 0;
            return *this;
        }
        operator VkDescriptorImageInfo() const { return { sampler, imageView, imageLayout }; }

        // TODO: replace the 64-bit sampler handle with `uint32_t samplerParams` and remove the
        // padding field. This is possible if we have access to the VulkanSamplerCache.
        VkSampler sampler;

        VkImageView imageView;
        VkImageLayout imageLayout;
        uint32_t padding;
    };

    // We store size with 32 bits, so our "WHOLE" sentinel is different from Vk.
    static const uint32_t WHOLE_SIZE = 0xffffffffu;

    // Represents all the Vulkan state that comprises a bound descriptor set.
    struct DescriptorKey {
        VkBuffer uniformBuffers[UBUFFER_BINDING_COUNT];               //   80     0
        DescriptorImageInfo samplers[SAMPLER_BINDING_COUNT];          // 1488    80
        DescriptorImageInfo inputAttachments[INPUT_ATTACHMENT_COUNT]; //   24  1568
        uint32_t uniformBufferOffsets[UBUFFER_BINDING_COUNT];         //   40  1592
        uint32_t uniformBufferSizes[UBUFFER_BINDING_COUNT];           //   40  1632
    };
    static_assert(offsetof(DescriptorKey, samplers)              == 80);
    static_assert(offsetof(DescriptorKey, inputAttachments)      == 1568);
    static_assert(offsetof(DescriptorKey, uniformBufferOffsets)  == 1592);
    static_assert(offsetof(DescriptorKey, uniformBufferSizes)    == 1632);
    static_assert(sizeof(DescriptorKey) == 1672, "DescriptorKey must not have implicit padding.");

    using DescHashFn = utils::hash::MurmurHashFn<DescriptorKey>;

    struct DescEqual {
        bool operator()(const DescriptorKey& k1, const DescriptorKey& k2) const;
    };

    #pragma clang diagnostic pop

    // CACHE ENTRY STRUCTS
    // -------------------

    // The timestamp associated with a given cache entry represents time as a count of flush
    // events since the cache was constructed. If any cache entry was most recently used over
    // FVK_MAX_PIPELINE_AGE flushes in the past, then we can be sure that it is no longer
    // being used by the GPU, and is therefore safe to destroy or reclaim.
    using Timestamp = uint64_t;
    Timestamp mCurrentTime = 0;

    // The descriptor set cache entry is a group of descriptor sets that are bound simultaneously.
    struct DescriptorCacheEntry {
        std::array<VkDescriptorSet, DESCRIPTOR_TYPE_COUNT> handles;
        Timestamp lastUsed;
        PipelineLayoutKey pipelineLayout;
        uint32_t id;
    };
    uint32_t mDescriptorCacheEntryCount = 0;


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
    using DescriptorMap
            = tsl::robin_map<DescriptorKey, DescriptorCacheEntry, DescHashFn, DescEqual>;
    using DescriptorResourceMap
            = std::unordered_map<uint32_t, std::unique_ptr<VulkanAcquireOnlyResourceManager>>;

    PipelineLayoutMap mPipelineLayouts;
    PipelineMap mPipelines;
    DescriptorMap mDescriptorSets;
    DescriptorResourceMap mDescriptorResources;

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

    // Current requirements for the pipeline layout, pipeline, and descriptor sets.
    PipelineKey mPipelineRequirements = {};
    DescriptorKey mDescriptorRequirements = {};

    // Current bindings for the pipeline and descriptor sets.
    PipelineKey mBoundPipeline = {};
    DescriptorKey mBoundDescriptor = {};

    // Current state for scissoring.
    VkRect2D mCurrentScissor = {};

    // The descriptor set pool starts out with a decent number of descriptor sets.  The cache can
    // grow the pool by re-creating it with a larger size.  See growDescriptorPool().
    VkDescriptorPool mDescriptorPool;

    // This describes the number of descriptor sets in mDescriptorPool. Note that this needs to be
    // multiplied by DESCRIPTOR_TYPE_COUNT to get the actual number of descriptor sets. Also note
    // that the number of low-level "descriptors" (not descriptor *sets*) is actually much more than
    // this size. It can be computed only by factoring in UBUFFER_BINDING_COUNT etc.
    uint32_t mDescriptorPoolSize = INITIAL_DESCRIPTOR_SET_POOL_SIZE;

    // To get the actual number of descriptor sets that have been allocated from the pool,
    // take the sum of mDescriptorArenasCount (these are inactive descriptor sets) and the
    // number of entries in the mDescriptorPool map (active descriptor sets). Multiply the result by
    // DESCRIPTOR_TYPE_COUNT.
    uint32_t mDescriptorArenasCount = 0;

    // After a growth event (i.e. when the VkDescriptorPool is replaced with a bigger version), all
    // currently used descriptors are moved into the "extinct" sets so that they can be safely
    // destroyed a few frames later.
    std::list<VkDescriptorPool> mExtinctDescriptorPools;
    std::list<DescriptorCacheEntry> mExtinctDescriptorBundles;

    VkDescriptorBufferInfo mDummyBufferInfo = {};
    VkWriteDescriptorSet mDummyBufferWriteInfo = {};
    VkDescriptorImageInfo mDummyTargetInfo = {};
    VkWriteDescriptorSet mDummyTargetWriteInfo = {};

    VkBuffer mDummyBuffer;
    VmaAllocation mDummyMemory;

    VulkanResourceAllocator* mResourceAllocator;
    VulkanAcquireOnlyResourceManager mPipelineBoundResources;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANPIPELINECACHE_H
