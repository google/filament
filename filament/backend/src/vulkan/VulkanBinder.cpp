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

#include "vulkan/VulkanBinder.h"

#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/trap.h>

#define FILAMENT_VULKAN_VERBOSE 0

// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"

namespace filament {
namespace backend {

// All vkCreate* and vkDestroy functions take an optional allocator. For now we select the default
// allocator by passing in a null pointer, and we pinpoint the argument by using the VKALLOC macro.
static constexpr VkAllocationCallbacks* VKALLOC = nullptr;

// Maximum number of descriptor sets that can be allocated by the pool.
// TODO: Make VulkanBinder robust against a large number of descriptors.
// There are several approaches we could take to deal with too many descriptors:
// - Create another pool after hitting the limit of the first pool.
// - Allow several uniform buffer descriptors to bind simultaneously instead of just one.
static constexpr uint32_t MAX_DESCRIPTOR_SET_COUNT = 1500;

static VulkanBinder::RasterState createDefaultRasterState();

VulkanBinder::VulkanBinder() : mDefaultRasterState(createDefaultRasterState()) {
    mColorBlendState = VkPipelineColorBlendStateCreateInfo{};
    mColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    mColorBlendState.attachmentCount = 1;
    mColorBlendState.pAttachments = mColorBlendAttachments;
    mShaderStages[0] = VkPipelineShaderStageCreateInfo{};
    mShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    mShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    mShaderStages[0].pName = "main";
    mShaderStages[1] = VkPipelineShaderStageCreateInfo{};
    mShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    mShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    mShaderStages[1].pName = "main";
    resetBindings();

    mDescriptorKey = {};
}

VulkanBinder::~VulkanBinder() {
    destroyCache();
}

bool VulkanBinder::getOrCreateDescriptors(VkDescriptorSet descriptorSets[3],
        VkPipelineLayout* pipelineLayout) noexcept {
    // If this method has never been called before, we need to create a new layout object.
    if (!mPipelineLayout) {
        createLayoutsAndDescriptors();
    }

    // If no bindings have been dirtied, update the timestamp (most recent access) and return false
    // to indicate there's no need to re-bind.
    if (!mDirtyDescriptor) {
        assert(mCurrentDescriptorBundle && mCurrentDescriptorBundle->bound);
        descriptorSets[0] = mCurrentDescriptorBundle->handles[0];
        descriptorSets[1] = mCurrentDescriptorBundle->handles[1];
        descriptorSets[2] = mCurrentDescriptorBundle->handles[2];
        mCurrentDescriptorBundle->timestamp = mCurrentTime;
        return false;
    }

    // Release the previously bound descriptor and update its time stamp.
    if (mCurrentDescriptorBundle) {
        mCurrentDescriptorBundle->timestamp = mCurrentTime;
        mCurrentDescriptorBundle->bound = false;
    }

    // If a cached object exists, update the timestamp (most recent access) and return true to
    // indicate that the caller should call vmCmdBind. Note that robin_map iterators proffer a
    // value method for obtaining a stable reference.
    auto iter = mDescriptorBundles.find(mDescriptorKey);
    if (UTILS_LIKELY(iter != mDescriptorBundles.end())) {
        mCurrentDescriptorBundle = &iter.value();
        descriptorSets[0] = mCurrentDescriptorBundle->handles[0];
        descriptorSets[1] = mCurrentDescriptorBundle->handles[1];
        descriptorSets[2] = mCurrentDescriptorBundle->handles[2];
        mCurrentDescriptorBundle->timestamp = mCurrentTime;
        mCurrentDescriptorBundle->bound = true;
        mDirtyDescriptor = false;
        *pipelineLayout = mPipelineLayout;
        return true;
    }

    // Allocate one descriptor set for each type: uniforms, samplers, and input attachments.
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = 3;
    allocInfo.pSetLayouts = mDescriptorSetLayouts;
    VkResult err = vkAllocateDescriptorSets(mDevice, &allocInfo, descriptorSets);
    ASSERT_POSTCONDITION(!err, "Unable to allocate descriptor set.");
    *pipelineLayout = mPipelineLayout;

    // Here we construct a DescriptorBundle in place, then stash its pointer to allow fast
    // subsequent calls to getOrCreateDescriptor when nothing has been dirtied. Note that the
    // robin_map iterator type proffers a "value" method, which returns a stable reference.
    auto& bundle = mDescriptorBundles.emplace(std::make_pair(mDescriptorKey, DescriptorBundle {
        .handles = { descriptorSets[0], descriptorSets[1], descriptorSets[2] },
        .timestamp = mCurrentTime,
        .bound = true
    })).first.value();

    mCurrentDescriptorBundle = &bundle;
    mDirtyDescriptor = false;

    // Mutate the descriptor by setting all non-null bindings.
    uint32_t nwrites = 0;
    VkWriteDescriptorSet* writes = mDescriptorWrites;
    nwrites = 0;
    for (uint32_t binding = 0; binding < UBUFFER_BINDING_COUNT; binding++) {
        if (mDescriptorKey.uniformBuffers[binding]) {
            VkDescriptorBufferInfo& bufferInfo = mDescriptorBuffers[binding];
            bufferInfo.buffer = mDescriptorKey.uniformBuffers[binding];
            bufferInfo.offset = mDescriptorKey.uniformBufferOffsets[binding];
            bufferInfo.range = mDescriptorKey.uniformBufferSizes[binding];
            VkWriteDescriptorSet& writeInfo = writes[nwrites++];
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstSet = mCurrentDescriptorBundle->handles[0];
            writeInfo.dstBinding = binding;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeInfo.pImageInfo = nullptr;
            writeInfo.pBufferInfo = &bufferInfo;
            writeInfo.pTexelBufferView = nullptr;
        }
    }
    for (uint32_t binding = 0; binding < SAMPLER_BINDING_COUNT; binding++) {
        if (mDescriptorKey.samplers[binding].sampler) {
            VkDescriptorImageInfo& imageInfo = mDescriptorSamplers[binding];
            imageInfo = mDescriptorKey.samplers[binding];
            VkWriteDescriptorSet& writeInfo = writes[nwrites++];
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstSet = mCurrentDescriptorBundle->handles[1];
            writeInfo.dstBinding = binding;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeInfo.pImageInfo = &imageInfo;
            writeInfo.pBufferInfo = nullptr;
            writeInfo.pTexelBufferView = nullptr;
        }
    }
    for (uint32_t binding = 0; binding < TARGET_BINDING_COUNT; binding++) {
        if (mDescriptorKey.inputAttachments[binding].imageView) {
            VkDescriptorImageInfo& imageInfo = mDescriptorInputAttachments[binding];
            imageInfo = mDescriptorKey.inputAttachments[binding];
            VkWriteDescriptorSet& writeInfo = writes[nwrites++];
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstSet = mCurrentDescriptorBundle->handles[2];
            writeInfo.dstBinding = binding;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            writeInfo.pImageInfo = &imageInfo;
            writeInfo.pBufferInfo = nullptr;
            writeInfo.pTexelBufferView = nullptr;
        }
    }
    vkUpdateDescriptorSets(mDevice, nwrites, writes, 0, nullptr);
    return true;
}

bool VulkanBinder::getOrCreatePipeline(VkPipeline* pipeline) noexcept {
    ASSERT_POSTCONDITION(mPipelineLayout,
            "Must call getOrCreateDescriptor before getOrCreatePipeline.");
    // If no bindings have been dirtied, update the timestamp (most recent access) and return false
    // to indicate there's no need to re-bind.
    if (!mDirtyPipeline) {
        assert(mCurrentPipeline && mCurrentPipeline->bound);
        *pipeline = mCurrentPipeline->handle;
        mCurrentPipeline->timestamp = mCurrentTime;
        return false;
    }
    assert(mPipelineKey.shaders[0] && "Vertex shader is not bound.");

    // Release the previously bound pipeline and update its time stamp.
    if (mCurrentPipeline) {
        mCurrentPipeline->timestamp = mCurrentTime;
        mCurrentPipeline->bound = false;
    }

    // If a cached object exists, update the timestamp (most recent access) and return true to
    // indicate that the caller should call vmCmdBind. Note that robin_map iterators proffer a value
    // method for obtaining a stable reference.
    auto iter = mPipelines.find(mPipelineKey);
    if (UTILS_LIKELY(iter != mPipelines.end())) {
        mCurrentPipeline = &iter.value();
        *pipeline = mCurrentPipeline->handle;
        mCurrentPipeline->timestamp = mCurrentTime;
        mCurrentPipeline->bound = true;
        mDirtyPipeline = false;
        return true;
    }

    // If we reach this point, we need to create and stash a brand new pipeline object.
    mShaderStages[0].module = mPipelineKey.shaders[0];
    mShaderStages[1].module = mPipelineKey.shaders[1];

    // We don't store array sizes to save space, but it's quick to count all non-zero
    // entries because these arrays have a small fixed-size capacity.
    uint32_t numVertexAttribs = 0;
    uint32_t numVertexBuffers = 0;
    for (uint32_t i = 0; i < VERTEX_ATTRIBUTE_COUNT; i++) {
        if (mPipelineKey.vertexAttributes[i].format > 0) {
            numVertexAttribs++;
        }
        if (mPipelineKey.vertexBuffers[i].stride > 0) {
            numVertexBuffers++;
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = numVertexBuffers;
    vertexInputState.pVertexBindingDescriptions = mPipelineKey.vertexBuffers;
    vertexInputState.vertexAttributeDescriptionCount = numVertexAttribs;
    vertexInputState.pVertexAttributeDescriptions = mPipelineKey.vertexAttributes;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = mPipelineKey.topology;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkDynamicState dynamicStateEnables[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 2;

    const bool hasFragmentShader = mShaderStages[1].module != VK_NULL_HANDLE;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = mPipelineLayout;
    pipelineCreateInfo.renderPass = mPipelineKey.renderPass;
    pipelineCreateInfo.subpass = mPipelineKey.subpassIndex;
    pipelineCreateInfo.stageCount = hasFragmentShader ? SHADER_MODULE_COUNT : 1;
    pipelineCreateInfo.pStages = mShaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &mPipelineKey.rasterState.rasterization;
    pipelineCreateInfo.pColorBlendState = &mColorBlendState;
    pipelineCreateInfo.pMultisampleState = &mPipelineKey.rasterState.multisampling;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &mPipelineKey.rasterState.depthStencil;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    // Filament assumes consistent blend state across all color attachments.
    mColorBlendState.attachmentCount = mPipelineKey.rasterState.getColorTargetCount;
    for (auto& target : mColorBlendAttachments) {
        target = mPipelineKey.rasterState.blending;
    }

    // There are no color attachments if there is no bound fragment shader.  (e.g. shadow map gen)
    // TODO: This should be handled in a higher layer.
    if (!hasFragmentShader) {
        mColorBlendState.attachmentCount = 0;
    }

    #if FILAMENT_VULKAN_VERBOSE
    utils::slog.d << "vkCreateGraphicsPipelines with shaders = ("
            << mShaderStages[0].module << ", " << mShaderStages[1].module << ")" << utils::io::endl;
    #endif

    VkResult err = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
            VKALLOC, pipeline);
    if (err) {
        utils::slog.e << "vkCreateGraphicsPipelines error " << err << utils::io::endl;
        utils::debug_trap();
    }

    // Here we construct a PipelineVal in place, then stash its pointer to allow fast subsequent
    // calls to getOrCreatePipeline when nothing has been dirtied. Note that the robin_map
    // iterator type proffers a "value" method, which returns a stable reference.
    mCurrentPipeline = &mPipelines.emplace(std::make_pair(mPipelineKey, PipelineVal {
        *pipeline, mCurrentTime, true })).first.value();
    mDirtyPipeline = false;
    return true;
}

void VulkanBinder::bindProgramBundle(const ProgramBundle& bundle) noexcept {
    const VkShaderModule shaders[2] = { bundle.vertex, bundle.fragment };
    for (uint32_t ssi = 0; ssi < SHADER_MODULE_COUNT; ssi++) {
        if (mPipelineKey.shaders[ssi] != shaders[ssi]) {
            mDirtyPipeline = true;
            mPipelineKey.shaders[ssi] = shaders[ssi];
        }
    }
}

void VulkanBinder::bindRasterState(const RasterState& rasterState) noexcept {
    VkPipelineRasterizationStateCreateInfo& raster0 = mPipelineKey.rasterState.rasterization;
    const VkPipelineRasterizationStateCreateInfo& raster1 = rasterState.rasterization;
    VkPipelineColorBlendAttachmentState& blend0 = mPipelineKey.rasterState.blending;
    const VkPipelineColorBlendAttachmentState& blend1 = rasterState.blending;
    VkPipelineDepthStencilStateCreateInfo& ds0 = mPipelineKey.rasterState.depthStencil;
    const VkPipelineDepthStencilStateCreateInfo& ds1 = rasterState.depthStencil;
    VkPipelineMultisampleStateCreateInfo& ms0 = mPipelineKey.rasterState.multisampling;
    const VkPipelineMultisampleStateCreateInfo& ms1 = rasterState.multisampling;
    if (
            mPipelineKey.rasterState.getColorTargetCount != rasterState.getColorTargetCount ||
            raster0.polygonMode != raster1.polygonMode ||
            raster0.cullMode != raster1.cullMode ||
            raster0.frontFace != raster1.frontFace ||
            raster0.rasterizerDiscardEnable != raster1.rasterizerDiscardEnable ||
            raster0.depthBiasEnable != raster1.depthBiasEnable ||
            raster0.depthBiasConstantFactor != raster1.depthBiasConstantFactor ||
            raster0.depthBiasSlopeFactor != raster1.depthBiasSlopeFactor ||
            blend0.colorWriteMask != blend1.colorWriteMask ||
            blend0.blendEnable != blend1.blendEnable ||
            ds0.depthTestEnable != ds1.depthTestEnable ||
            ds0.depthWriteEnable != ds1.depthWriteEnable ||
            ds0.depthCompareOp != ds1.depthCompareOp ||
            ds0.stencilTestEnable != ds1.stencilTestEnable ||
            ms0.rasterizationSamples != ms1.rasterizationSamples ||
            ms0.alphaToCoverageEnable != ms1.alphaToCoverageEnable
    ) {
        mDirtyPipeline = true;
        mPipelineKey.rasterState = rasterState;
    }
}

void VulkanBinder::bindRenderPass(VkRenderPass renderPass, int subpassIndex) noexcept {
    if (mPipelineKey.renderPass != renderPass || mPipelineKey.subpassIndex != subpassIndex) {
        mDirtyPipeline = true;
        mPipelineKey.renderPass = renderPass;
        mPipelineKey.subpassIndex = subpassIndex;
    }
}

void VulkanBinder::bindPrimitiveTopology(VkPrimitiveTopology topology) noexcept {
    if (mPipelineKey.topology != topology) {
        mDirtyPipeline = true;
        mPipelineKey.topology = topology;
    }
}

void VulkanBinder::bindVertexArray(const VertexArray& varray) noexcept {
    for (size_t i = 0; i < VERTEX_ATTRIBUTE_COUNT; i++) {
        VkVertexInputAttributeDescription& attrib0 = mPipelineKey.vertexAttributes[i];
        const VkVertexInputAttributeDescription& attrib1 = varray.attributes[i];
        if (attrib1.location != attrib0.location || attrib1.binding != attrib0.binding ||
                attrib1.format != attrib0.format || attrib1.offset != attrib0.offset) {
            attrib0.format = attrib1.format;
            attrib0.binding = attrib1.binding;
            attrib0.location = attrib1.location;
            attrib0.offset = attrib1.offset;
            mDirtyPipeline = true;
        }
        VkVertexInputBindingDescription& buffer0 = mPipelineKey.vertexBuffers[i];
        const VkVertexInputBindingDescription& buffer1 = varray.buffers[i];
        if (buffer0.binding != buffer1.binding || buffer0.stride != buffer1.stride) {
            buffer0.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            buffer0.binding = buffer1.binding;
            buffer0.stride = buffer1.stride;
            mDirtyPipeline = true;
        }
    }
}

void VulkanBinder::unbindUniformBuffer(VkBuffer uniformBuffer) noexcept {
    auto& key = mDescriptorKey;
    for (uint32_t bindingIndex = 0u; bindingIndex < UBUFFER_BINDING_COUNT; ++bindingIndex) {
        if (key.uniformBuffers[bindingIndex] == uniformBuffer) {
            key.uniformBuffers[bindingIndex] = {};
            key.uniformBufferSizes[bindingIndex] = {};
            key.uniformBufferOffsets[bindingIndex] = {};
            mDirtyDescriptor = true;
        }
    }
    // This function is often called before deleting a uniform buffer. For safety, we need to evict
    // all descriptors that refer to the extinct uniform buffer, regardless of the binding offsets.
    evictDescriptors([uniformBuffer] (const DescriptorKey& key) {
        for (VkBuffer buf : key.uniformBuffers) {
            if (buf == uniformBuffer) {
                return true;
            }
        }
        return false;
    });
}

void VulkanBinder::unbindImageView(VkImageView imageView) noexcept {
    for (auto& sampler : mDescriptorKey.samplers) {
        if (sampler.imageView == imageView) {
            mDirtyDescriptor = true;
        }
    }
    for (auto& target : mDescriptorKey.inputAttachments) {
        if (target.imageView == imageView) {
            mDirtyDescriptor = true;
        }
    }
    evictDescriptors([imageView] (const DescriptorKey& key) {
        for (const auto& binding : key.samplers) {
            if (binding.imageView == imageView) {
                return true;
            }
        }
        for (const auto& binding : key.inputAttachments) {
            if (binding.imageView == imageView) {
                return true;
            }
        }
        return false;
    });
}

// Discards all descriptor sets that pass the given filter. Immediately removes the cache entries,
// but defers calling vkFreeDescriptorSets until the next eviction cycle.
void VulkanBinder::evictDescriptors(std::function<bool(const DescriptorKey&)> filter) noexcept {
    // Due to robin_map restrictions, we cannot use auto or a range-based loop.
    decltype(mDescriptorBundles)::const_iterator iter;
    for (iter = mDescriptorBundles.begin(); iter != mDescriptorBundles.end();) {
        auto& pair = *iter;
        if (filter(pair.first)) {
            auto& cacheEntry = iter->second;
            mDescriptorGraveyard.push_back({
                .handles = { cacheEntry.handles[0], cacheEntry.handles[1], cacheEntry.handles[2] },
                .timestamp = cacheEntry.timestamp,
                .bound = false
            });
            iter = mDescriptorBundles.erase(iter);
        } else {
            ++iter;
        }
    }
}

void VulkanBinder::bindUniformBuffer(uint32_t bindingIndex, VkBuffer uniformBuffer,
        VkDeviceSize offset, VkDeviceSize size) noexcept {
    ASSERT_POSTCONDITION(bindingIndex < UBUFFER_BINDING_COUNT,
            "Uniform bindings overflow: index = %d, capacity = %d.",
            bindingIndex, UBUFFER_BINDING_COUNT);
    auto& key = mDescriptorKey;
    if (key.uniformBuffers[bindingIndex] != uniformBuffer ||
        key.uniformBufferOffsets[bindingIndex] != offset ||
        key.uniformBufferSizes[bindingIndex] != size) {
        key.uniformBuffers[bindingIndex] = uniformBuffer;
        key.uniformBufferOffsets[bindingIndex] = offset;
        key.uniformBufferSizes[bindingIndex] = size;
        mDirtyDescriptor = true;
    }
}

void VulkanBinder::bindSampler(uint32_t bindingIndex, VkDescriptorImageInfo samplerInfo) noexcept {
    assert(bindingIndex < SAMPLER_BINDING_COUNT);
    if (bindingIndex >= SAMPLER_BINDING_COUNT) {
        utils::slog.w << "Sampler bindings overflow: " << bindingIndex << " / "
                << SAMPLER_BINDING_COUNT << utils::io::endl;
        return;
    }
    VkDescriptorImageInfo& imageInfo = mDescriptorKey.samplers[bindingIndex];
    if (imageInfo.sampler != samplerInfo.sampler || imageInfo.imageView != samplerInfo.imageView ||
        imageInfo.imageLayout != samplerInfo.imageLayout) {
        imageInfo = samplerInfo;
        mDirtyDescriptor = true;
    }
}

void VulkanBinder::bindInputAttachment(uint32_t bindingIndex,
        VkDescriptorImageInfo targetInfo) noexcept {
    ASSERT_POSTCONDITION(bindingIndex < TARGET_BINDING_COUNT,
            "Input attachment bindings overflow: index = %d, capacity = %d.",
            bindingIndex, TARGET_BINDING_COUNT);
    VkDescriptorImageInfo& imageInfo = mDescriptorKey.inputAttachments[bindingIndex];
    if (imageInfo.imageView != targetInfo.imageView ||
            imageInfo.imageLayout != targetInfo.imageLayout) {
        imageInfo = targetInfo;
        mDirtyDescriptor = true;
    }
}

void VulkanBinder::destroyCache() noexcept {
    // Symmetric to createLayoutsAndDescriptors.
    destroyLayoutsAndDescriptors();
    for (auto& iter : mPipelines) {
        vkDestroyPipeline(mDevice, iter.second.handle, VKALLOC);
    }
    mPipelines.clear();
    mCurrentPipeline = nullptr;
    mDirtyPipeline = true;
}

void VulkanBinder::resetBindings() noexcept {
    mDirtyPipeline = true;
    mDirtyDescriptor = true;
}

// Frees up old descriptor sets and pipelines, then nulls out their key.
//
// This method is designed to be called once per frame, and our notion of "time" is actually a
// frame counter. Frames are a better metric than wall clock because we know with certainty that
// objects last bound more than n frames ago are no longer in use (due to existing fences).
void VulkanBinder::gc() noexcept {
    // If this is one of the first few frames, return early to avoid wrapping unsigned integers.
    if (++mCurrentTime <= TIME_BEFORE_EVICTION) {
        return;
    }
    const uint32_t evictTime = mCurrentTime - TIME_BEFORE_EVICTION;

    // Due to robin_map restrictions, we cannot use auto or a range-based loop.
    for (decltype(mDescriptorBundles)::const_iterator iter = mDescriptorBundles.begin();
            iter != mDescriptorBundles.end();) {
        auto& cacheEntry = iter->second;
        if (cacheEntry.timestamp < evictTime && !cacheEntry.bound) {
            vkFreeDescriptorSets(mDevice, mDescriptorPool, 3, cacheEntry.handles);
            iter = mDescriptorBundles.erase(iter);
        } else {
            ++iter;
        }
    }
    for (decltype(mPipelines)::const_iterator iter = mPipelines.begin();
            iter != mPipelines.end();) {
        auto& cacheEntry = iter->second;
        if (cacheEntry.timestamp < evictTime && !cacheEntry.bound) {
            vkDestroyPipeline(mDevice, cacheEntry.handle, VKALLOC);
            iter = mPipelines.erase(iter);
        } else {
            ++iter;
        }
    }
    // The graveyard is composed of descriptors that contain references to extinct objects. We
    // take care only to free the ones that are old enough to be evicted, since they might be
    // referenced in a command buffer that hasn't finished executing.
    decltype(mDescriptorGraveyard) graveyard;
    graveyard.swap(mDescriptorGraveyard);
    for (auto& val : graveyard) {
        if (val.timestamp < evictTime) {
           vkFreeDescriptorSets(mDevice, mDescriptorPool, 3, val.handles);
        } else {
            mDescriptorGraveyard.emplace_back(DescriptorBundle {
                .handles = { val.handles[0], val.handles[1], val.handles[2] },
                .timestamp = val.timestamp
            });
        }
    }
}

void VulkanBinder::createLayoutsAndDescriptors() noexcept {
    VkDescriptorSetLayoutBinding binding = {};
    binding.descriptorCount = 1; // NOTE: We never use arrays-of-blocks.
    binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS; // NOTE: This is potentially non-optimal.

    // First create the descriptor set layout for UBO's.
    VkDescriptorSetLayoutBinding ubindings[UBUFFER_BINDING_COUNT];
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    for (uint32_t i = 0; i < UBUFFER_BINDING_COUNT; i++) {
        binding.binding = i;
        ubindings[i] = binding;
    }
    VkDescriptorSetLayoutCreateInfo dlinfo = {};
    dlinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dlinfo.bindingCount = UBUFFER_BINDING_COUNT;
    dlinfo.pBindings = ubindings;
    vkCreateDescriptorSetLayout(mDevice, &dlinfo, VKALLOC, &mDescriptorSetLayouts[0]);

    // Next create the descriptor set layout for samplers.
    VkDescriptorSetLayoutBinding sbindings[SAMPLER_BINDING_COUNT];
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    for (uint32_t i = 0; i < SAMPLER_BINDING_COUNT; i++) {
        binding.binding = i;
        sbindings[i] = binding;
    }
    dlinfo.bindingCount = SAMPLER_BINDING_COUNT;
    dlinfo.pBindings = sbindings;
    vkCreateDescriptorSetLayout(mDevice, &dlinfo, VKALLOC, &mDescriptorSetLayouts[1]);

    // Next create the descriptor set layout for input attachments.
    VkDescriptorSetLayoutBinding tbindings[TARGET_BINDING_COUNT];
    binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    for (uint32_t i = 0; i < TARGET_BINDING_COUNT; i++) {
        binding.binding = i;
        tbindings[i] = binding;
    }
    dlinfo.bindingCount = TARGET_BINDING_COUNT;
    dlinfo.pBindings = tbindings;
    vkCreateDescriptorSetLayout(mDevice, &dlinfo, VKALLOC, &mDescriptorSetLayouts[2]);

    // Create the one and only VkPipelineLayout that we'll ever use.
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.setLayoutCount = 3;
    pPipelineLayoutCreateInfo.pSetLayouts = mDescriptorSetLayouts;
    VkResult err = vkCreatePipelineLayout(mDevice, &pPipelineLayoutCreateInfo, VKALLOC,
            &mPipelineLayout);
    ASSERT_POSTCONDITION(!err, "Unable to create pipeline layout.");

    // Create the VkDescriptorPool.
    VkDescriptorPoolSize poolSizes[3] = {};
    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = MAX_DESCRIPTOR_SET_COUNT,
        .poolSizeCount = 3,
        .pPoolSizes = poolSizes
    };
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = poolInfo.maxSets * UBUFFER_BINDING_COUNT;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = poolInfo.maxSets * SAMPLER_BINDING_COUNT;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolSizes[2].descriptorCount = poolInfo.maxSets * TARGET_BINDING_COUNT;

    err = vkCreateDescriptorPool(mDevice, &poolInfo, VKALLOC, &mDescriptorPool);
    ASSERT_POSTCONDITION(!err, "Unable to create descriptor pool.");
}

void VulkanBinder::destroyLayoutsAndDescriptors() noexcept {
    if (mPipelineLayout == VK_NULL_HANDLE) {
        return;
    }

    // Our current descriptor set strategy can cause the # of descriptor sets to explode in certain
    // situations, so it's interesting to report the number that get stuffed into the cache.
    #ifndef NDEBUG
    utils::slog.d << "Destroying " << mDescriptorBundles.size() << " bundles of descriptor sets."
            << utils::io::endl;
    #endif

    mDescriptorBundles.clear();
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, VKALLOC);
    mPipelineLayout = VK_NULL_HANDLE;
    for (int i = 0; i < 3; i++) {
        vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayouts[i], VKALLOC);
        mDescriptorSetLayouts[i] = {};
    }
    vkDestroyDescriptorPool(mDevice, mDescriptorPool, VKALLOC);
    mDescriptorPool = VK_NULL_HANDLE;
    mCurrentDescriptorBundle = nullptr;
    mDirtyDescriptor = true;
}

bool VulkanBinder::PipelineEqual::operator()(const VulkanBinder::PipelineKey& k1,
        const VulkanBinder::PipelineKey& k2) const {
    return 0 == memcmp((const void*) &k1, (const void*) &k2, sizeof(k1));
}

bool VulkanBinder::DescEqual::operator()(const VulkanBinder::DescriptorKey& k1,
        const VulkanBinder::DescriptorKey& k2) const {
    for (uint32_t i = 0; i < UBUFFER_BINDING_COUNT; i++) {
        if (k1.uniformBuffers[i] != k2.uniformBuffers[i] ||
            k1.uniformBufferOffsets[i] != k2.uniformBufferOffsets[i] ||
            k1.uniformBufferSizes[i] != k2.uniformBufferSizes[i]) {
            return false;
        }
    }
    for (uint32_t i = 0; i < SAMPLER_BINDING_COUNT; i++) {
        if (k1.samplers[i].sampler != k2.samplers[i].sampler ||
            k1.samplers[i].imageView != k2.samplers[i].imageView ||
            k1.samplers[i].imageLayout != k2.samplers[i].imageLayout) {
            return false;
        }
    }
    for (uint32_t i = 0; i < TARGET_BINDING_COUNT; i++) {
        if (k1.inputAttachments[i].imageView != k2.inputAttachments[i].imageView ||
            k1.inputAttachments[i].imageLayout != k2.inputAttachments[i].imageLayout) {
            return false;
        }
    }
    return true;
}

static VulkanBinder::RasterState createDefaultRasterState() {
    VkPipelineRasterizationStateCreateInfo rasterization = {};
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.cullMode = VK_CULL_MODE_NONE;
    rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization.depthClampEnable = VK_FALSE;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.depthBiasEnable = VK_FALSE;
    rasterization.depthBiasConstantFactor = 0.0f;
    rasterization.depthBiasClamp = 0.0f; // 0 is a special value that disables clamping
    rasterization.depthBiasSlopeFactor = 0.0f;
    rasterization.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blending = {};
    blending.colorWriteMask = 0xf;
    blending.blendEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencil.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = depthStencil.back;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = true;

    return VulkanBinder::RasterState {
        rasterization,
        blending,
        depthStencil,
        multisampling,
        1,
    };
}

} // namespace filament
} // namespace backend

#pragma clang diagnostic pop
