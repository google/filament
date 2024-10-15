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

#include "VulkanCommands.h"
#include "VulkanMemory.h"
#include "VulkanUtility.h"

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

namespace filament::backend {

struct VulkanProgram;
struct VulkanBufferObject;
struct VulkanTexture;

// VulkanPipelineCache manages a cache of descriptor sets and pipelines.
//
// Please note the following limitations:
//
// - Push constants are not supported. (if adding support, see VkPipelineLayoutCreateInfo)
// - Only DESCRIPTOR_TYPE_COUNT descriptor sets are bound at a time.
// - Assumes that viewport and scissor should be dynamic. (not baked into VkPipeline)
// - Assumes that uniform buffers should be visible across all shader stages.
//
class VulkanPipelineCache {
public:
    VulkanPipelineCache(VulkanPipelineCache const&) = delete;
    VulkanPipelineCache& operator=(VulkanPipelineCache const&) = delete;

    static constexpr uint32_t SHADER_MODULE_COUNT = 2;
    static constexpr uint32_t VERTEX_ATTRIBUTE_COUNT = MAX_VERTEX_ATTRIBUTE_COUNT;

    // The ProgramBundle contains weak references to the compiled vertex and fragment shaders.
    struct ProgramBundle {
        VkShaderModule vertex;
        VkShaderModule fragment;
        VkSpecializationInfo* specializationInfos = nullptr;
    };

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
        uint8_t               rasterizationSamples : 4;// offset = 4 bytes
        uint8_t               depthClamp : 4;
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
    VulkanPipelineCache(VkDevice device, VmaAllocator allocator);
    ~VulkanPipelineCache();

    void bindLayout(VkPipelineLayout layout) noexcept;

    // Creates a new pipeline if necessary and binds it using vkCmdBindPipeline.
    void bindPipeline(VulkanCommandBuffer* commands);

    // Each of the following methods are fast and do not make Vulkan calls.
    void bindProgram(fvkmemory::resource_ptr<VulkanProgram> program) noexcept;
    void bindRasterState(const RasterState& rasterState) noexcept;
    void bindRenderPass(VkRenderPass renderPass, int subpassIndex) noexcept;
    void bindPrimitiveTopology(VkPrimitiveTopology topology) noexcept;

    void bindVertexArray(VkVertexInputAttributeDescription const* attribDesc,
            VkVertexInputBindingDescription const* bufferDesc, uint8_t count);

    // Destroys all managed Vulkan objects. This should be called before changing the VkDevice.
    void terminate() noexcept;

    static VkPrimitiveTopology getPrimitiveTopology(PrimitiveType pt) noexcept {
        switch (pt) {
            case PrimitiveType::POINTS:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case PrimitiveType::LINES:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case PrimitiveType::LINE_STRIP:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case PrimitiveType::TRIANGLES:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case PrimitiveType::TRIANGLE_STRIP:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        }
    }

    void gc() noexcept;

private:
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
        VkPipelineLayout layout;                                                  //  8   : 304
    };

    static_assert(sizeof(PipelineKey) == 312, "PipelineKey must not have implicit padding.");

    using PipelineHashFn = utils::hash::MurmurHashFn<PipelineKey>;

    struct PipelineEqual {
        bool operator()(const PipelineKey& k1, const PipelineKey& k2) const;
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

    struct PipelineCacheEntry {
        VkPipeline handle;
        Timestamp lastUsed;
    };

    struct PipelineLayoutCacheEntry {
        VkPipelineLayout handle;
        Timestamp lastUsed;
    };

    // CACHE CONTAINERS
    // ----------------

    using PipelineMap = tsl::robin_map<PipelineKey, PipelineCacheEntry,
            PipelineHashFn, PipelineEqual>;

private:

    PipelineCacheEntry* getOrCreatePipeline() noexcept;

    PipelineMap mPipelines;

    // These helpers all return unstable pointers that should not be stored.
    PipelineCacheEntry* createPipeline() noexcept;
    PipelineLayoutCacheEntry* getOrCreatePipelineLayout() noexcept;

    // Immutable state.
    VkDevice mDevice = VK_NULL_HANDLE;
    VmaAllocator mAllocator = VK_NULL_HANDLE;

    // Current requirements for the pipeline layout, pipeline, and descriptor sets.
    PipelineKey mPipelineRequirements = {};

    // Current bindings for the pipeline and descriptor sets.
    PipelineKey mBoundPipeline = {};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANPIPELINECACHE_H
