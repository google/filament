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

#include "vulkan/VulkanPipelineCache.h"

#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/trap.h>

#include "VulkanConstants.h"

// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"

using namespace bluevk;

namespace filament {
namespace backend {

static VulkanPipelineCache::RasterState createDefaultRasterState();

VulkanPipelineCache::VulkanPipelineCache() : mDefaultRasterState(createDefaultRasterState()) {
    markDirtyDescriptor();
    markDirtyPipeline();
    mDescriptorKey = {};

    mDummyBufferWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    mDummyBufferWriteInfo.pNext = nullptr;
    mDummyBufferWriteInfo.dstArrayElement = 0;
    mDummyBufferWriteInfo.descriptorCount = 1;
    mDummyBufferWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    mDummyBufferWriteInfo.pImageInfo = nullptr;
    mDummyBufferWriteInfo.pBufferInfo = &mDummyBufferInfo;
    mDummyBufferWriteInfo.pTexelBufferView = nullptr;

    mDummySamplerWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    mDummySamplerWriteInfo.pNext = nullptr;
    mDummySamplerWriteInfo.dstArrayElement = 0;
    mDummySamplerWriteInfo.descriptorCount = 1;
    mDummySamplerWriteInfo.pImageInfo = &mDummySamplerInfo;
    mDummySamplerWriteInfo.pBufferInfo = nullptr;
    mDummySamplerWriteInfo.pTexelBufferView = nullptr;
    mDummySamplerWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    mDummyTargetInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    mDummyTargetWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    mDummyTargetWriteInfo.pNext = nullptr;
    mDummyTargetWriteInfo.dstArrayElement = 0;
    mDummyTargetWriteInfo.descriptorCount = 1;
    mDummyTargetWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    mDummyTargetWriteInfo.pImageInfo = &mDummyTargetInfo;
    mDummyTargetWriteInfo.pBufferInfo = nullptr;
    mDummyTargetWriteInfo.pTexelBufferView = nullptr;
}

VulkanPipelineCache::~VulkanPipelineCache() {
    destroyCache();
}

bool VulkanPipelineCache::bindDescriptors(VulkanCommands& commands) noexcept {
    VkDescriptorSet descriptors[VulkanPipelineCache::DESCRIPTOR_TYPE_COUNT];
    bool bind = false, overflow = false;
    getOrCreateDescriptors(descriptors, &bind, &overflow);

    if (overflow) {
        return false;
    }
    if (bind) {
        vkCmdBindDescriptorSets(commands.get().cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                mPipelineLayout, 0, VulkanPipelineCache::DESCRIPTOR_TYPE_COUNT, descriptors,
                0, nullptr);
    }
    return true;
}

void VulkanPipelineCache::bindPipeline(VulkanCommands& commands) noexcept {
    VkPipeline pipeline;
    if (getOrCreatePipeline(&pipeline)) {
        vkCmdBindPipeline(commands.get().cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
}

void VulkanPipelineCache::getOrCreateDescriptors(
        VkDescriptorSet descriptorSets[DESCRIPTOR_TYPE_COUNT],
        bool* bind, bool* overflow) noexcept {
    // If this method has never been called before, we need to create a new layout object.
    if (!mPipelineLayout) {
        createLayoutsAndDescriptors();
    }

    DescriptorBundle*& descriptorBundle = mCmdBufferState[mCmdBufferIndex].currentDescriptorBundle;

    // Leave early if no bindings have been dirtied.
    if (!mDirtyDescriptor[mCmdBufferIndex]) {
        assert_invariant(descriptorBundle);
        for (uint32_t i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
            descriptorSets[i] = descriptorBundle->handles[i];
        }
        descriptorBundle->commandBuffers.set(mCmdBufferIndex);
        return;
    }

    // If a cached object exists, re-use it.
    auto iter = mDescriptorBundles.find(mDescriptorKey);
    if (UTILS_LIKELY(iter != mDescriptorBundles.end())) {
        descriptorBundle = &iter.value();
        for (uint32_t i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
            descriptorSets[i] = descriptorBundle->handles[i];
        }
        descriptorBundle->commandBuffers.set(mCmdBufferIndex);
        mDirtyDescriptor.unset(mCmdBufferIndex);
        *bind = true;
        return;
    }

    // If there are no available descriptor sets that can be re-used, then create brand new ones
    // (one for each type). Otherwise, grab a descriptor set from each of the arenas.
    if (mDescriptorSetArena[0].empty()) {
        if (mDescriptorBundles.size() >= mDescriptorPoolSize) {
            growDescriptorPool();
        }
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool;
        allocInfo.descriptorSetCount = DESCRIPTOR_TYPE_COUNT;
        allocInfo.pSetLayouts = mDescriptorSetLayouts;
        VkResult err = vkAllocateDescriptorSets(mDevice, &allocInfo, descriptorSets);
        assert_invariant(err == VK_SUCCESS);
        if (err != VK_SUCCESS) {
            *overflow = true;
            return;
        }
    } else {
        for (uint32_t i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
            descriptorSets[i] = mDescriptorSetArena[i].back();
            mDescriptorSetArena[i].pop_back();
        }
    }

    // Construct a cache entry in place, then stash its pointer to allow fast subsequent calls to
    // getOrCreateDescriptor when nothing has been dirtied. Note that the robin_map iterator type
    // proffers a "value" method, which returns a stable reference.
    auto& bundle = mDescriptorBundles.emplace(std::make_pair(mDescriptorKey, DescriptorBundle {}))
            .first.value();

    descriptorBundle = &bundle;
    for (uint32_t i = 0; i < DESCRIPTOR_TYPE_COUNT; ++i) {
        descriptorBundle->handles[i] = descriptorSets[i];
    }
    descriptorBundle->commandBuffers.setValue(1 << mCmdBufferIndex);

    // Clear the dirty flag for this command buffer.
    mDirtyDescriptor.unset(mCmdBufferIndex);

    // Formulate some dummy descriptor info used solely for clearing out unused bindings. This is
    // especially crucial after a texture has been destroyed. Since core Vulkan does not allow
    // specifying VK_NULL_HANDLE without the robustness2 extension, we are forced to use dummy
    // resources for this.
    mDummyBufferInfo.buffer = mDescriptorKey.uniformBuffers[0];
    mDummyBufferInfo.offset = mDescriptorKey.uniformBufferOffsets[0];
    mDummyBufferInfo.range = mDescriptorKey.uniformBufferSizes[0];
    mDummySamplerInfo.imageLayout = mDummyTargetInfo.imageLayout;
    mDummySamplerInfo.imageView = mDummyImageView;
    mDummyTargetInfo.imageView = mDummyImageView;

    if (mDummySamplerInfo.sampler == VK_NULL_HANDLE) {
        VkSamplerCreateInfo samplerInfo {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 1.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
        };
        vkCreateSampler(mDevice, &samplerInfo, VKALLOC, &mDummySamplerInfo.sampler);
    }

    // Rewrite every binding in the new descriptor sets.
    VkDescriptorBufferInfo descriptorBuffers[UBUFFER_BINDING_COUNT];
    VkDescriptorImageInfo descriptorSamplers[SAMPLER_BINDING_COUNT];
    VkDescriptorImageInfo descriptorInputAttachments[TARGET_BINDING_COUNT];
    VkWriteDescriptorSet descriptorWrites[UBUFFER_BINDING_COUNT + SAMPLER_BINDING_COUNT +
            TARGET_BINDING_COUNT];
    uint32_t nwrites = 0;
    VkWriteDescriptorSet* writes = descriptorWrites;
    nwrites = 0;
    for (uint32_t binding = 0; binding < UBUFFER_BINDING_COUNT; binding++) {
        VkWriteDescriptorSet& writeInfo = writes[nwrites++];
        if (mDescriptorKey.uniformBuffers[binding]) {
            VkDescriptorBufferInfo& bufferInfo = descriptorBuffers[binding];
            bufferInfo.buffer = mDescriptorKey.uniformBuffers[binding];
            bufferInfo.offset = mDescriptorKey.uniformBufferOffsets[binding];
            bufferInfo.range = mDescriptorKey.uniformBufferSizes[binding];
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeInfo.pImageInfo = nullptr;
            writeInfo.pBufferInfo = &bufferInfo;
            writeInfo.pTexelBufferView = nullptr;
        } else {
            writeInfo = mDummyBufferWriteInfo;
        }
        writeInfo.dstSet = descriptorBundle->handles[0];
        writeInfo.dstBinding = binding;
    }
    for (uint32_t binding = 0; binding < SAMPLER_BINDING_COUNT; binding++) {
        VkWriteDescriptorSet& writeInfo = writes[nwrites++];
        if (mDescriptorKey.samplers[binding].sampler) {
            VkDescriptorImageInfo& imageInfo = descriptorSamplers[binding];
            imageInfo = mDescriptorKey.samplers[binding];
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeInfo.pImageInfo = &imageInfo;
            writeInfo.pBufferInfo = nullptr;
            writeInfo.pTexelBufferView = nullptr;
        } else {
            writeInfo = mDummySamplerWriteInfo;
        }
        writeInfo.dstSet = descriptorBundle->handles[1];
        writeInfo.dstBinding = binding;
    }
    for (uint32_t binding = 0; binding < TARGET_BINDING_COUNT; binding++) {
        VkWriteDescriptorSet& writeInfo = writes[nwrites++];
        if (mDescriptorKey.inputAttachments[binding].imageView) {
            VkDescriptorImageInfo& imageInfo = descriptorInputAttachments[binding];
            imageInfo = mDescriptorKey.inputAttachments[binding];
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.pNext = nullptr;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorCount = 1;
            writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            writeInfo.pImageInfo = &imageInfo;
            writeInfo.pBufferInfo = nullptr;
            writeInfo.pTexelBufferView = nullptr;
        } else {
            writeInfo = mDummyTargetWriteInfo;
        }
        writeInfo.dstSet = descriptorBundle->handles[2];
        writeInfo.dstBinding = binding;
    }
    vkUpdateDescriptorSets(mDevice, nwrites, writes, 0, nullptr);
    *bind = true;
}

bool VulkanPipelineCache::getOrCreatePipeline(VkPipeline* pipeline) noexcept {
    assert_invariant(mPipelineLayout);
    PipelineVal*& currentPipeline = mCmdBufferState[mCmdBufferIndex].currentPipeline;

    // If no bindings have been dirtied, return false to indicate there's no need to re-bind.
    if (!mDirtyPipeline[mCmdBufferIndex]) {
        assert_invariant(currentPipeline);
        currentPipeline->age = 0;
        *pipeline = currentPipeline->handle;
        return false;
    }
    assert_invariant(mPipelineKey.shaders[0] && "Vertex shader is not bound.");

    // If a cached object exists, return true to indicate that the caller should call vmCmdBind.
    auto iter = mPipelines.find(mPipelineKey);
    if (UTILS_LIKELY(iter != mPipelines.end())) {
        currentPipeline = &iter.value();
        currentPipeline->age = 0;
        *pipeline = currentPipeline->handle;
        mDirtyPipeline.unset(mCmdBufferIndex);
        return true;
    }

    VkPipelineShaderStageCreateInfo shaderStages[SHADER_MODULE_COUNT];
    shaderStages[0] = VkPipelineShaderStageCreateInfo{};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].pName = "main";

    shaderStages[1] = VkPipelineShaderStageCreateInfo{};
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].pName = "main";

    VkPipelineColorBlendAttachmentState colorBlendAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT];
    VkPipelineColorBlendStateCreateInfo colorBlendState;
    colorBlendState = VkPipelineColorBlendStateCreateInfo{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = colorBlendAttachments;

    // If we reach this point, we need to create and stash a brand new pipeline object.
    shaderStages[0].module = mPipelineKey.shaders[0];
    shaderStages[1].module = mPipelineKey.shaders[1];

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

    const bool hasFragmentShader = shaderStages[1].module != VK_NULL_HANDLE;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = mPipelineLayout;
    pipelineCreateInfo.renderPass = mPipelineKey.renderPass;
    pipelineCreateInfo.subpass = mPipelineKey.subpassIndex;
    pipelineCreateInfo.stageCount = hasFragmentShader ? SHADER_MODULE_COUNT : 1;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &mPipelineKey.rasterState.rasterization;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &mPipelineKey.rasterState.multisampling;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &mPipelineKey.rasterState.depthStencil;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    // Filament assumes consistent blend state across all color attachments.
    colorBlendState.attachmentCount = mPipelineKey.rasterState.colorTargetCount;
    for (auto& target : colorBlendAttachments) {
        target = mPipelineKey.rasterState.blending;
    }

    // There are no color attachments if there is no bound fragment shader.  (e.g. shadow map gen)
    // TODO: This should be handled in a higher layer.
    if (!hasFragmentShader) {
        colorBlendState.attachmentCount = 0;
    }

    #if FILAMENT_VULKAN_VERBOSE
    utils::slog.d << "vkCreateGraphicsPipelines with shaders = ("
            << shaderStages[0].module << ", " << shaderStages[1].module << ")" << utils::io::endl;
    #endif
    VkResult err = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
            VKALLOC, pipeline);
    if (err) {
        utils::slog.e << "vkCreateGraphicsPipelines error " << err << utils::io::endl;
        utils::debug_trap();
    }

    // Stash a stable pointer to the stored cache entry to allow fast subsequent calls to
    // getOrCreatePipeline when nothing has been dirtied.
    const PipelineVal cacheEntry = { *pipeline, 0u };
    currentPipeline = &mPipelines.emplace(std::make_pair(mPipelineKey, cacheEntry)).first.value();
    mDirtyPipeline.unset(mCmdBufferIndex);

    return true;
}

void VulkanPipelineCache::bindProgramBundle(const ProgramBundle& bundle) noexcept {
    const VkShaderModule shaders[2] = { bundle.vertex, bundle.fragment };
    for (uint32_t ssi = 0; ssi < SHADER_MODULE_COUNT; ssi++) {
        if (mPipelineKey.shaders[ssi] != shaders[ssi]) {
            markDirtyPipeline();
            mPipelineKey.shaders[ssi] = shaders[ssi];
        }
    }
}

void VulkanPipelineCache::bindRasterState(const RasterState& rasterState) noexcept {
    VkPipelineRasterizationStateCreateInfo& raster0 = mPipelineKey.rasterState.rasterization;
    const VkPipelineRasterizationStateCreateInfo& raster1 = rasterState.rasterization;
    VkPipelineColorBlendAttachmentState& blend0 = mPipelineKey.rasterState.blending;
    const VkPipelineColorBlendAttachmentState& blend1 = rasterState.blending;
    VkPipelineDepthStencilStateCreateInfo& ds0 = mPipelineKey.rasterState.depthStencil;
    const VkPipelineDepthStencilStateCreateInfo& ds1 = rasterState.depthStencil;
    VkPipelineMultisampleStateCreateInfo& ms0 = mPipelineKey.rasterState.multisampling;
    const VkPipelineMultisampleStateCreateInfo& ms1 = rasterState.multisampling;
    if (
            mPipelineKey.rasterState.colorTargetCount != rasterState.colorTargetCount ||
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
        markDirtyPipeline();
        mPipelineKey.rasterState = rasterState;
    }
}

void VulkanPipelineCache::bindRenderPass(VkRenderPass renderPass, int subpassIndex) noexcept {
    if (mPipelineKey.renderPass != renderPass || mPipelineKey.subpassIndex != subpassIndex) {
        markDirtyPipeline();
        mPipelineKey.renderPass = renderPass;
        mPipelineKey.subpassIndex = subpassIndex;
    }
}

void VulkanPipelineCache::bindPrimitiveTopology(VkPrimitiveTopology topology) noexcept {
    if (mPipelineKey.topology != topology) {
        markDirtyPipeline();
        mPipelineKey.topology = topology;
    }
}

void VulkanPipelineCache::bindVertexArray(const VertexArray& varray) noexcept {
    for (size_t i = 0; i < VERTEX_ATTRIBUTE_COUNT; i++) {
        VkVertexInputAttributeDescription& attrib0 = mPipelineKey.vertexAttributes[i];
        const VkVertexInputAttributeDescription& attrib1 = varray.attributes[i];
        if (attrib1.location != attrib0.location || attrib1.binding != attrib0.binding ||
                attrib1.format != attrib0.format || attrib1.offset != attrib0.offset) {
            attrib0.format = attrib1.format;
            attrib0.binding = attrib1.binding;
            attrib0.location = attrib1.location;
            attrib0.offset = attrib1.offset;
            markDirtyPipeline();
        }
        VkVertexInputBindingDescription& buffer0 = mPipelineKey.vertexBuffers[i];
        const VkVertexInputBindingDescription& buffer1 = varray.buffers[i];
        if (buffer0.binding != buffer1.binding || buffer0.stride != buffer1.stride) {
            buffer0.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            buffer0.binding = buffer1.binding;
            buffer0.stride = buffer1.stride;
            markDirtyPipeline();
        }
    }
}

void VulkanPipelineCache::unbindUniformBuffer(VkBuffer uniformBuffer) noexcept {
    auto& key = mDescriptorKey;
    for (uint32_t bindingIndex = 0u; bindingIndex < UBUFFER_BINDING_COUNT; ++bindingIndex) {
        if (key.uniformBuffers[bindingIndex] == uniformBuffer) {
            key.uniformBuffers[bindingIndex] = {};
            key.uniformBufferSizes[bindingIndex] = {};
            key.uniformBufferOffsets[bindingIndex] = {};
            markDirtyDescriptor();
        }
    }
}

void VulkanPipelineCache::unbindImageView(VkImageView imageView) noexcept {
    for (auto& sampler : mDescriptorKey.samplers) {
        if (sampler.imageView == imageView) {
            sampler = {};
            markDirtyDescriptor();
        }
    }
    for (auto& target : mDescriptorKey.inputAttachments) {
        if (target.imageView == imageView) {
            target = {};
            markDirtyDescriptor();
        }
    }
}

void VulkanPipelineCache::bindUniformBuffer(uint32_t bindingIndex, VkBuffer uniformBuffer,
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
        markDirtyDescriptor();
    }
}

void VulkanPipelineCache::bindSamplers(VkDescriptorImageInfo samplers[SAMPLER_BINDING_COUNT]) noexcept {
    for (uint32_t bindingIndex = 0; bindingIndex < SAMPLER_BINDING_COUNT; bindingIndex++) {
        const VkDescriptorImageInfo& requested = samplers[bindingIndex];
        VkDescriptorImageInfo& existing = mDescriptorKey.samplers[bindingIndex];
        if (existing.sampler != requested.sampler ||
            existing.imageView != requested.imageView ||
            existing.imageLayout != requested.imageLayout) {
            existing = requested;
            markDirtyDescriptor();
        }
    }
}

void VulkanPipelineCache::bindInputAttachment(uint32_t bindingIndex,
        VkDescriptorImageInfo targetInfo) noexcept {
    ASSERT_POSTCONDITION(bindingIndex < TARGET_BINDING_COUNT,
            "Input attachment bindings overflow: index = %d, capacity = %d.",
            bindingIndex, TARGET_BINDING_COUNT);
    VkDescriptorImageInfo& imageInfo = mDescriptorKey.inputAttachments[bindingIndex];
    if (imageInfo.imageView != targetInfo.imageView ||
            imageInfo.imageLayout != targetInfo.imageLayout) {
        imageInfo = targetInfo;
        markDirtyDescriptor();
    }
}

void VulkanPipelineCache::destroyCache() noexcept {
    // Symmetric to createLayoutsAndDescriptors.
    destroyLayoutsAndDescriptors();
    for (auto& iter : mPipelines) {
        vkDestroyPipeline(mDevice, iter.second.handle, VKALLOC);
    }
    mPipelines.clear();
    for (int i = 0; i < VK_MAX_COMMAND_BUFFERS; i++) {
        mCmdBufferState[i].currentPipeline = nullptr;
    }
    markDirtyPipeline();
    if (mDummySamplerInfo.sampler) {
        vkDestroySampler(mDevice, mDummySamplerInfo.sampler, VKALLOC);
        mDummySamplerInfo.sampler = VK_NULL_HANDLE;
    }
}

void VulkanPipelineCache::onCommandBuffer(const VulkanCommandBuffer& cmdbuffer) {
    // This method is called each time a command buffer is flushed and a new command buffer is
    // ready to be written into. Stash the index of this command buffer for state-tracking purposes.
    mCmdBufferIndex = cmdbuffer.index;

    mDirtyPipeline.set(mCmdBufferIndex);
    mDirtyDescriptor.set(mCmdBufferIndex);

    // NOTE: Due to robin_map restrictions, we cannot use auto or range-based loops.

    // Check if any bundles in the cache are no longer in use by any command buffer. Descriptors
    // from unused bundles are moved back to their respective arenas.
    using ConstDescIterator = decltype(mDescriptorBundles)::const_iterator;
    for (ConstDescIterator iter = mDescriptorBundles.begin(); iter != mDescriptorBundles.end();) {
        const DescriptorBundle& cacheEntry = iter.value();
        if (cacheEntry.commandBuffers.getValue() == 0) {
            mDescriptorSetArena[0].push_back(cacheEntry.handles[0]);
            mDescriptorSetArena[1].push_back(cacheEntry.handles[1]);
            mDescriptorSetArena[2].push_back(cacheEntry.handles[2]);
            iter = mDescriptorBundles.erase(iter);
        } else {
            ++iter;
        }
    }

    // Increment the "age" of all cached pipelines. If the age of any pipeline is 0, then it is
    // being used by the command buffer that was just flushed.
    using PipeIterator = decltype(mPipelines)::iterator;
    for (PipeIterator iter = mPipelines.begin(); iter != mPipelines.end(); ++iter) {
        ++iter.value().age;
    }

    // Evict any pipelines that have not been used in a while.
    // Any pipeline older than VK_MAX_COMMAND_BUFFERS can be safely destroyed.
    static_assert(VK_MAX_PIPELINE_AGE >= VK_MAX_COMMAND_BUFFERS);
    using ConstPipeIterator = decltype(mPipelines)::const_iterator;
    for (ConstPipeIterator iter = mPipelines.begin(); iter != mPipelines.end();) {
        if (iter.value().age > VK_MAX_PIPELINE_AGE) {
            vkDestroyPipeline(mDevice, iter->second.handle, VKALLOC);
            iter = mPipelines.erase(iter);
        } else {
            ++iter;
        }
    }

    // We know that the new command buffer is not being processed by the GPU, so we can clear
    // its "in use" bit from all descriptors in the cache.
    using DescIterator = decltype(mDescriptorBundles)::iterator;
    for (DescIterator iter = mDescriptorBundles.begin(); iter != mDescriptorBundles.end(); ++iter) {
        iter.value().commandBuffers.unset(mCmdBufferIndex);
    }

    // Descriptor sets that arose from an old pool (i.e. before the most recent growth event)
    // also need to have their "in use" bit cleared for this command buffer.
    bool canPurgeExtinctPools = true;
    for (auto& bundle : mExtinctDescriptorBundles) {
        bundle.commandBuffers.unset(mCmdBufferIndex);
        if (bundle.commandBuffers.getValue() != 0) {
            canPurgeExtinctPools = false;
        }
    }

    // If there are no descriptors from any extinct pool that are still in use, we can safely
    // destroy the extinct pools, which implicitly frees their associated descriptor sets.
    if (canPurgeExtinctPools) {
        for (VkDescriptorPool pool : mExtinctDescriptorPools) {
            vkDestroyDescriptorPool(mDevice, pool, VKALLOC);
        }
        mExtinctDescriptorPools.clear();
        mExtinctDescriptorBundles.clear();
    }
}

void VulkanPipelineCache::createLayoutsAndDescriptors() noexcept {
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

    mDescriptorPool = createDescriptorPool(mDescriptorPoolSize);
}

VkDescriptorPool VulkanPipelineCache::createDescriptorPool(uint32_t size) const {
    VkDescriptorPoolSize poolSizes[DESCRIPTOR_TYPE_COUNT] = {};
    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = size * DESCRIPTOR_TYPE_COUNT,
        .poolSizeCount = DESCRIPTOR_TYPE_COUNT,
        .pPoolSizes = poolSizes
    };
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = poolInfo.maxSets * UBUFFER_BINDING_COUNT;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = poolInfo.maxSets * SAMPLER_BINDING_COUNT;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    poolSizes[2].descriptorCount = poolInfo.maxSets * TARGET_BINDING_COUNT;

    VkDescriptorPool pool;
    const UTILS_UNUSED VkResult result = vkCreateDescriptorPool(mDevice, &poolInfo, VKALLOC, &pool);
    assert_invariant(result == VK_SUCCESS);
    return pool;
}

void VulkanPipelineCache::destroyLayoutsAndDescriptors() noexcept {
    if (mPipelineLayout == VK_NULL_HANDLE) {
        return;
    }

    // Our current descriptor set strategy can cause the # of descriptor sets to explode in certain
    // situations, so it's interesting to report the number that get stuffed into the cache.
    #ifndef NDEBUG
    utils::slog.d << "Destroying " << mDescriptorBundles.size() << " bundles of descriptor sets."
            << utils::io::endl;
    #endif

    // There is no need to free descriptor sets individually since destroying the VkDescriptorPool
    // implicitly frees them.
    for (auto& arena : mDescriptorSetArena) {
        arena.clear();
    }

    mDescriptorBundles.clear();
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, VKALLOC);
    mPipelineLayout = VK_NULL_HANDLE;
    for (int i = 0; i < 3; i++) {
        vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayouts[i], VKALLOC);
        mDescriptorSetLayouts[i] = {};
    }
    vkDestroyDescriptorPool(mDevice, mDescriptorPool, VKALLOC);
    mDescriptorPool = VK_NULL_HANDLE;
    for (int i = 0; i < VK_MAX_COMMAND_BUFFERS; i++) {
        mCmdBufferState[i].currentDescriptorBundle = nullptr;
    }

    for (VkDescriptorPool pool : mExtinctDescriptorPools) {
        vkDestroyDescriptorPool(mDevice, pool, VKALLOC);
    }
    mExtinctDescriptorPools.clear();
    mExtinctDescriptorBundles.clear();

    markDirtyDescriptor();
}

void VulkanPipelineCache::growDescriptorPool() noexcept {
    // We need to destroy the old VkDescriptorPool, but we can't do so immediately because many
    // of its descriptors are still in use. So, stash it in an "extinct" list.
    mExtinctDescriptorPools.push_back(mDescriptorPool);

    // Create the new VkDescriptorPool, twice as big as the old one.
    mDescriptorPoolSize *= 2;
    mDescriptorPool = createDescriptorPool(mDescriptorPoolSize);

    // Clear out all unused descriptor sets in the arena so they don't get reclaimed. There is no
    // need to free them individually since the old VkDescriptorPool will be destroyed.
    for (auto& arena : mDescriptorSetArena) {
        arena.clear();
    }

    // Move all in-use descriptors from the primary cache into an "extinct" list, so that they will
    // later be destroyed rather than reclaimed.
    using DescIterator = decltype(mDescriptorBundles)::iterator;
    for (DescIterator iter = mDescriptorBundles.begin(); iter != mDescriptorBundles.end(); ++iter) {
        mExtinctDescriptorBundles.push_back(iter.value());
    }
    mDescriptorBundles.clear();
}

bool VulkanPipelineCache::PipelineEqual::operator()(const VulkanPipelineCache::PipelineKey& k1,
        const VulkanPipelineCache::PipelineKey& k2) const {
    return 0 == memcmp((const void*) &k1, (const void*) &k2, sizeof(k1));
}

bool VulkanPipelineCache::DescEqual::operator()(const VulkanPipelineCache::DescriptorKey& k1,
        const VulkanPipelineCache::DescriptorKey& k2) const {
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

static VulkanPipelineCache::RasterState createDefaultRasterState() {
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

    return VulkanPipelineCache::RasterState {
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
