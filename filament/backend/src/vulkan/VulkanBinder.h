/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_VULKANBINDER_H
#define TNT_FILAMENT_DRIVER_VULKANBINDER_H

#include <backend/DriverEnums.h>

#include <private/backend/Program.h>

#include <bluevk/BlueVK.h>

#include <utils/compiler.h>
#include <utils/Hash.h>

#include <tsl/robin_map.h>
#include <vector>

namespace filament {
namespace backend {

// VulkanBinder manages a cache of descriptor sets and pipelines.
//
// The VulkanBinder interface has two parts: the "bindFoo" methods (bindRasterState,
// bindUniformBuffer, etc), and the "getOrCreateFoo" methods (getOrCreateDescriptors,
// getOrCreatePipeline).
//
// Abbreviated example usage:
//
//     void Driver::bindUniformBuffer(uint32_t index, UniformBlock block) {
//         VkBuffer buffer = block->getGpuBuffer();
//         mBinder.bindUniformBuffer(index, buffer);
//     }
//
//     void Driver::draw(Geometry geo) {
//        mBinder.bindPrimitiveTopology(geo.topology);
//        mBinder.bindVertexArray(geo.varray);
//        VkDescriptorSet descriptors[2];
//        if (mBinder.getOrCreateDescriptors(descriptors, ...)) {
//            vkCmdBindDescriptorSets(... descriptors ...);
//        }
//        VkPipeline pipeline;
//        if (mBinder.getOrCreatePipeline(&pipeline)) {
//            vkCmdBindPipeline(cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
//        }
//        vkCmdBindVertexBuffers(cmdbuffer, geo.vbo, ...);
//        vkCmdBindIndexBuffer(cmdbuffer, geo.ibo, ...);
//        vkCmdDrawIndexed(cmdbuffer, ...);
//     }
//
// The class declaration and implementation have no dependencies on any other Filament files,
// modulo some constants and low-level utility functions.
//
// In the name of simplicity, VulkanBinder has the following limitations:
// - Push constants are not supported. (if adding support, see VkPipelineLayoutCreateInfo)
// - Only two descriptor sets are bound at a time: one for uniform buffers and one for samplers.
// - Descriptor sets are never mutated using vkUpdateDescriptorSets, except upon creation.
// - Assumes that viewport and scissor should be dynamic. (not baked into VkPipeline)
// - Assumes that uniform buffers should be visible across all shader stages.
//
class VulkanBinder {
public:
    static constexpr uint32_t UBUFFER_BINDING_COUNT = Program::UNIFORM_BINDING_COUNT;
    static constexpr uint32_t SAMPLER_BINDING_COUNT = backend::MAX_SAMPLER_COUNT;
    static constexpr uint32_t SHADER_MODULE_COUNT = 2;
    static constexpr uint32_t VERTEX_ATTRIBUTE_COUNT = backend::MAX_VERTEX_ATTRIBUTE_COUNT;

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
    };
    static_assert(std::is_pod<RasterState>::value, "RasterState must be a POD for fast hashing.");

    // Upon construction, the binder initializes some internal state but does not make any Vulkan
    // calls. On destruction it will free any cached Vulkan objects that haven't already been freed
    // via resetBindings(). We don't pass the VkDevice to the constructor to allow the client to own
    // a concrete instance of Binder rather than going through a pointer.
    VulkanBinder();
    ~VulkanBinder();
    void setDevice(VkDevice device) { mDevice = device; }

    // Clients should initialize their copy of the raster state using this method. They can then
    // mutate their copy and pass it back through bindRasterState().
    const RasterState& getDefaultRasterState() const { return mDefaultRasterState; }

    // Returns true if vkCmdBindDescriptorSets is required.
    bool getOrCreateDescriptors(VkDescriptorSet descriptors[2], VkPipelineLayout* pipelineLayout)
            noexcept;

    // Returns true if any pipeline bindings have changed. (i.e., vkCmdBindPipeline is required)
    bool getOrCreatePipeline(VkPipeline* pipeline) noexcept;

    // Each bind method is fast and does not make Vulkan calls.
    void bindProgramBundle(const ProgramBundle& bundle) noexcept;
    void bindRasterState(const RasterState& rasterState) noexcept;
    void bindRenderPass(VkRenderPass renderPass) noexcept;
    void bindPrimitiveTopology(VkPrimitiveTopology topology) noexcept;
    void bindUniformBuffer(uint32_t bindingIndex, VkBuffer uniformBuffer,
            VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) noexcept;
    void bindSampler(uint32_t bindingIndex, VkDescriptorImageInfo imageInfo) noexcept;
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

    // Destroys all managed Vulkan objects. This should be called before changing the VkDevice, or
    // when the cache gets too big.
    void destroyCache() noexcept;

    // Force the subsequent call to getOrCreate to unconditionally return true, thus signaling
    // to the client that we need to re-bind the current descriptor set and pipeline. This should
    // be called after every swap if the VulkanBinder is shared amongst command buffers.
    void resetBindings() noexcept;

    // Evicts old unused Vulkan objects. Call this once per frame.
    void gc() noexcept;

private:
    // The pipeline key is a POD that represents all currently bound states that form the immutable
    // VkPipeline object. We apply a hash function to its contents only if has been mutated since
    // the previous call to getOrCreatePipeline.
    #pragma pack(push, 1)
    struct UTILS_PACKED PipelineKey {
        VkShaderModule shaders[SHADER_MODULE_COUNT]; // 8*2 bytes
        RasterState rasterState; // 248 bytes
        VkRenderPass renderPass; // 8 bytes
        VkPrimitiveTopology topology; // 4 bytes
        VkVertexInputAttributeDescription vertexAttributes[VERTEX_ATTRIBUTE_COUNT]; // 16*5 bytes
        VkVertexInputBindingDescription vertexBuffers[VERTEX_ATTRIBUTE_COUNT]; // 12*5 bytes
    };
    #pragma pack(pop)

    static_assert(std::is_pod<PipelineKey>::value, "PipelineKey must be a POD for fast hashing.");

    using PipelineHashFn = utils::hash::MurmurHashFn<PipelineKey>;

    struct PipelineEqual {
        bool operator()(const PipelineKey& k1, const PipelineKey& k2) const;
    };

    struct PipelineVal {
        VkPipeline handle;
        uint32_t timestamp;
        bool bound;
    };

    // The descriptor key is a POD that represents all currently bound states that go into the
    // descriptor set. We apply a hash function to its contents only if has been mutated since
    // the previous call to getOrCreateDescriptors.
    #pragma pack(push, 1)
    struct UTILS_PACKED DescriptorKey {
        VkBuffer uniformBuffers[UBUFFER_BINDING_COUNT];
        VkDescriptorImageInfo samplers[SAMPLER_BINDING_COUNT];
        VkDeviceSize uniformBufferOffsets[UBUFFER_BINDING_COUNT];
        VkDeviceSize uniformBufferSizes[UBUFFER_BINDING_COUNT];
    };
    #pragma pack(pop)

    static_assert(std::is_pod<DescriptorKey>::value, "DescriptorKey must be a POD.");

    using DescHashFn = utils::hash::MurmurHashFn<DescriptorKey>;

    struct DescEqual {
        bool operator()(const DescriptorKey& k1, const DescriptorKey& k2) const;
    };

    struct DescriptorVal {
        VkDescriptorSet handles[2];
        uint32_t timestamp;
        bool bound;
    };

    void createLayoutsAndDescriptors() noexcept;
    void destroyLayoutsAndDescriptors() noexcept;
    void evictDescriptors(std::function<bool(const DescriptorKey&)> filter) noexcept;

    VkDevice mDevice = nullptr;
    const RasterState mDefaultRasterState;

    // These structs are used only in a transient way but are stored for convenience.
    VkPipelineShaderStageCreateInfo mShaderStages[SHADER_MODULE_COUNT];
    VkPipelineColorBlendStateCreateInfo mColorBlendState;
    VkDescriptorBufferInfo mDescriptorBuffers[UBUFFER_BINDING_COUNT];
    VkDescriptorImageInfo mDescriptorSamplers[SAMPLER_BINDING_COUNT];
    VkWriteDescriptorSet mDescriptorWrites[UBUFFER_BINDING_COUNT + SAMPLER_BINDING_COUNT];

    // Current bindings are divided into two "keys" which are composed of a mix of actual values
    // (e.g., blending is OFF) and weak references to Vulkan objects (e.g., shader programs and
    // uniform buffers).
    PipelineKey mPipelineKey;
    DescriptorKey mDescriptorKey;

    // Weak references to the currently bound pipeline and descriptor set.
    PipelineVal* mCurrentPipeline = nullptr;
    DescriptorVal* mCurrentDescriptor = nullptr;

    // If one of these dirty flags is set, then one or more its constituent bindings have changed, so
    // a new pipeline or descriptor set needs to be retrieved from the cache or created.
    bool mDirtyPipeline = true;
    bool mDirtyDescriptor = true;

    // Cached Vulkan objects. These objects are owned by the Binder.
    VkDescriptorSetLayout mDescriptorSetLayouts[2] = {};
    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    tsl::robin_map<PipelineKey, PipelineVal, PipelineHashFn, PipelineEqual> mPipelines;
    tsl::robin_map<DescriptorKey, DescriptorVal, DescHashFn, DescEqual> mDescriptorSets;
    VkDescriptorPool mDescriptorPool;
    std::vector<DescriptorVal> mDescriptorGraveyard;

    // Store the current "time" (really just a frame count) and LRU eviction parameters.
    uint32_t mCurrentTime = 0;
    static constexpr uint32_t TIME_BEFORE_EVICTION = 2;
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANBINDER_H
