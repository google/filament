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

#include <fstream>

#include "VulkanPipelineStateSerializer.h"
#include <utils/Log.h>
#include <utils/Panic.h>

#include "VulkanConstants.h"
#include "VulkanHandles.h"

#if defined(__clang__)
// Vulkan functions often immediately dereference pointers, so it's fine to pass in a pointer
// to a stack-allocated variable.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-stack-address"
#endif

using namespace bluevk;

namespace filament::backend {

VulkanPipelineStateSerializer::VulkanPipelineStateSerializer(const utils::CString& name) { 
    mBuffer << "{" << std::endl;
    mBuffer << "\"program_name\":" << "\"" << name.c_str() << "\"" << "," << std::endl;
}
VulkanPipelineStateSerializer::~VulkanPipelineStateSerializer() {
    //Shader Modules
    mBuffer << "\"shader_modules\":[" << std::endl;
    for (uint32_t i = 0; i < mShaderStages.size(); ++i) {
        mBuffer << "{" << std::endl;
        mBuffer << "\"stage\":" << mShaderStages[i].stage << "," << std::endl;
        mBuffer << "\"key\":" << mShaderStages[i].module  << "," << std::endl;
        mBuffer << "\"name\":" << "\"" << mShaderStages[i].mName << "\"" << std::endl;
        mBuffer << "}";
        if (i < mShaderStages.size() - 1) {
            mBuffer << ",";
        }
        mBuffer <<  std::endl;
    }
    mBuffer << "]," << std::endl;

    //Vertex Attribute
    mBuffer << "\"vertex_attributes\":[" << std::endl;
    for (uint32_t i = 0; i < mVtxAttribs.size(); ++i) {
        mBuffer << "{" << std::endl;
        mBuffer << "\"location\":" << mVtxAttribs[i].location << "," << std::endl;
        mBuffer << "\"binding\":" <<  mVtxAttribs[i].binding << "," << std::endl;
        mBuffer << "\"format\":" << mVtxAttribs[i].format << "," << std::endl;
        mBuffer << "\"offset\":" << mVtxAttribs[i].offset << std::endl;
        mBuffer << "}";
        if (i < mVtxAttribs.size() - 1) {
            mBuffer << ",";
        }
        mBuffer << std::endl;
    }
    mBuffer << "]," << std::endl;

    // Vertex Bindings
    mBuffer << "\"vertex_bindings\":[" << std::endl;
    for (uint32_t i = 0; i < mVtxBindings.size(); ++i) {
        mBuffer << "{" << std::endl;
        mBuffer << "\"binding\":" << mVtxBindings[i].binding << "," << std::endl;
        mBuffer << "\"stride\":" << mVtxBindings[i].stride << "," << std::endl;
        mBuffer << "\"input_rate\":" << mVtxBindings[i].inputRate << std::endl;
        mBuffer << "}";
        if (i < mVtxBindings.size() - 1) {
            mBuffer << ",";
        }
        mBuffer << std::endl;
    }
    mBuffer << "]" << std::endl;

    mBuffer << "}" << std::endl;

    std::ofstream file(mFileName);
    if (file.is_open()) {
        file << mBuffer.str();
    }
    file.close();
}

void VulkanPipelineStateSerializer::setPipelineLayoutKey(uint32_t pipelineLayoutKey) {
    mBuffer << "\"layout\":" << pipelineLayoutKey << "," << std::endl;
}

void VulkanPipelineStateSerializer::setRenderPassKey(uint32_t renderPassKey) {
    mBuffer << "\"render_pass\":" << renderPassKey << "," << std::endl;
}

void VulkanPipelineStateSerializer::setID(uint32_t key) {
    std::stringstream temp;
    temp << "pipeline_" << key << ".json";
    temp >> mFileName;
}

VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
        const VkPipelineInputAssemblyStateCreateInfo& inputAsm) {
    mBuffer << "\"topology\":" << inputAsm.topology << "," << std::endl;
    return *this;
}

VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
        const VkPipelineViewportStateCreateInfo& viewport) {
    mBuffer << "\"viewport_count\":" << viewport.viewportCount << "," << std::endl;
    mBuffer << "\"scissor_count\":" << viewport.scissorCount << "," << std::endl;
    return *this;
}

VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
        const VkPipelineRasterizationStateCreateInfo& rasterState) {
    mBuffer << "\"raster_states\":{" << std::endl;
    mBuffer << "\"depth_clamp_enable\":" << rasterState.depthClampEnable << "," << std::endl;
    mBuffer << "\"polygon_mode\":" << rasterState.polygonMode << "," << std::endl;
    mBuffer << "\"cull_mode\":" << rasterState.cullMode << "," << std::endl;
    mBuffer << "\"front_face\":" << rasterState.frontFace << "," << std::endl;
    mBuffer << "\"depth_bias_enable\":" << rasterState.depthBiasEnable << "," << std::endl;
    mBuffer << "\"depth_bias_constant_factor\":" << rasterState.depthBiasConstantFactor << ","
            << std::endl;
    mBuffer << "\"depth_bias_clamp\":" << rasterState.depthBiasClamp << "," << std::endl;
    mBuffer << "\"depth_bias_slope_factor\":" << rasterState.depthBiasSlopeFactor << "," << std::endl;
    mBuffer << "\"line_width\":" << rasterState.lineWidth << std::endl;
    mBuffer << "}," << std::endl;
    return *this;
}

VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
        const VkPipelineDepthStencilStateCreateInfo& depthStencil) {
    mBuffer << "\"depth_stencil_states\":{" << std::endl;
    mBuffer << "\"depth_test_enable\":" << depthStencil.depthTestEnable << "," << std::endl;
    mBuffer << "\"depth_write_enable\":" << depthStencil.depthWriteEnable << "," << std::endl;
    mBuffer << "\"depth_compare_op\":" << depthStencil.depthCompareOp << "," << std::endl;
    mBuffer << "\"depth_bounds_test_enable\":" << depthStencil.depthBoundsTestEnable << ","
            << std::endl;
    mBuffer << "\"min_depth_bounds\":" << depthStencil.minDepthBounds << "," << std::endl;
    mBuffer << "\"max_depth_bounds\":" << depthStencil.maxDepthBounds << "," << std::endl;
    mBuffer << "\"front\":{" << std::endl;
    mBuffer << "\"fail_op\":" << depthStencil.front.failOp << "," << std::endl;
    mBuffer << "\"pass_op\":" << depthStencil.front.passOp << "," << std::endl;
    mBuffer << "\"depth_fail_op\":" << depthStencil.front.depthFailOp << "," << std::endl;
    mBuffer << "\"compare_op\":" << depthStencil.front.compareOp << "," << std::endl;
    mBuffer << "\"compare_mask\":" << depthStencil.front.compareMask << "," << std::endl;
    mBuffer << "\"write_mask\":" << depthStencil.front.writeMask << "," << std::endl;
    mBuffer << "\"reference\":" << depthStencil.front.reference << std::endl;
    mBuffer << "}," << std::endl;
    mBuffer << "\"back\":{" << std::endl;
    mBuffer << "\"fail_op\":" << depthStencil.back.failOp << "," << std::endl;
    mBuffer << "\"pass_op\":" << depthStencil.back.passOp << "," << std::endl;
    mBuffer << "\"depth_fail_op\":" << depthStencil.back.depthFailOp << "," << std::endl;
    mBuffer << "\"compare_op\":" << depthStencil.back.compareOp << "," << std::endl;
    mBuffer << "\"compare\_mask\":" << depthStencil.back.compareMask << "," << std::endl;
    mBuffer << "\"write_mask\":" << depthStencil.back.writeMask << "," << std::endl;
    mBuffer << "\"reference\":" << depthStencil.back.reference << std::endl;
    mBuffer << "}" << std::endl;
    mBuffer << "}," << std::endl;
    return *this;
}

VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
        const VkPipelineMultisampleStateCreateInfo& multiSample) {
    mBuffer << "\"multisample_states\":{" << std::endl;
    mBuffer << "\"rasterization_samples\":" << multiSample.rasterizationSamples << "," << std::endl;
    mBuffer << "\"sample_shading_enable\":" << multiSample.sampleShadingEnable << "," << std::endl;
    mBuffer << "\"min_sample_shading\":" << multiSample.minSampleShading << "," << std::endl;
    mBuffer << "\"alpha_to_coverage_enable\":" << multiSample.alphaToCoverageEnable << ","
            << std::endl;
    mBuffer << "\"alpha_to_one_enable\":" << multiSample.alphaToOneEnable << std::endl;
    mBuffer << "}," << std::endl;
    return *this;
}

VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
        const VkPipelineDynamicStateCreateInfo& dyStates) {
    mBuffer << "\"dynamic_states\":[" << std::endl;
    for (uint32_t i = 0; i < dyStates.dynamicStateCount; ++i) {
        mBuffer << dyStates.pDynamicStates[i];
        if (i < dyStates.dynamicStateCount - 1) {
            mBuffer << ",";
        }
        mBuffer << std::endl;
    }
    mBuffer << "]," << std::endl;
    return *this;
}

VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
        const VkPipelineColorBlendStateCreateInfo& blendStateInfo) {
    mBuffer << "\"color_blend_state_attachments\":[" << std::endl;
    for (uint32_t i = 0; i < blendStateInfo.attachmentCount; ++i) {
        mBuffer << "{" << std::endl;
        mBuffer << "\"blend_enable\":" << blendStateInfo.pAttachments[i].blendEnable << ","
                << std::endl;
        mBuffer << "\"src_color_blend_factor\":" << blendStateInfo.pAttachments[i].srcColorBlendFactor
                << ","
                << std::endl;
        mBuffer << "\"dst_color_blend_factor\":" << blendStateInfo.pAttachments[i].dstColorBlendFactor
                << ","
                << std::endl;
        mBuffer << "\"color_blend_op\":" << blendStateInfo.pAttachments[i].colorBlendOp << ","
                << std::endl;
        mBuffer << "\"src_alpha_blend_factor\":" << blendStateInfo.pAttachments[i].srcAlphaBlendFactor
                << ","
                << std::endl;
        mBuffer << "\"dst_alpha_blend_factor\":" << blendStateInfo.pAttachments[i].dstAlphaBlendFactor
                << ","
                << std::endl;
        mBuffer << "\"alpha_blend_op\":" << blendStateInfo.pAttachments[i].alphaBlendOp << ","
                << std::endl;
        mBuffer << "\"color_write_mask\":" << blendStateInfo.pAttachments[i].colorWriteMask
                << std::endl;
        mBuffer << "}";
        if (i < blendStateInfo.attachmentCount - 1) {
            mBuffer << ",";
        }
        mBuffer << std::endl;
    }
    mBuffer << "]," << std::endl;
    return *this;
}

VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
        const ShaderStageInfo& shaderStages) {
    mShaderStages.push_back(shaderStages);
    return *this;
}

VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
    const VkVertexInputAttributeDescription& attrib) {
    mVtxAttribs.push_back(attrib);
    return *this;
}
VulkanPipelineStateSerializer& VulkanPipelineStateSerializer::operator<<(
    const VkVertexInputBindingDescription& binding) {
    mVtxBindings.push_back(binding);
    return *this;
}
}
