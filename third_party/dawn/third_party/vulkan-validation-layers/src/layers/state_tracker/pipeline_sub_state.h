/* Copyright (c) 2015-2017, 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2025 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "state_tracker/pipeline_layout_state.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>

// Graphics pipeline sub-state as defined by VK_KHR_graphics_pipeline_library

namespace vvl {
class Device;
class RenderPass;
class Pipeline;
class PipelineLayout;
struct ShaderModule;
}  // namespace vvl

namespace spirv {
struct EntryPoint;
struct StatelessData;
}  // namespace spirv

template <typename CreateInfoType>
static inline VkGraphicsPipelineLibraryFlagsEXT GetGraphicsLibType(const CreateInfoType &create_info) {
    const auto lib_ci = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(create_info.pNext);
    if (lib_ci) {
        return lib_ci->flags;
    }
    return static_cast<VkGraphicsPipelineLibraryFlagsEXT>(0);
}

// Common amoung all pipeline sub state
struct PipelineSubState {
    PipelineSubState(const vvl::Pipeline &p) : parent(p) {}
    const vvl::Pipeline &parent;

    VkPipelineLayoutCreateFlags PipelineLayoutCreateFlags() const;
};

struct VertexAttrState {
    VertexAttrState(uint32_t index_, const VkVertexInputAttributeDescription *desc_) : index(index_) {
        desc.location = desc_->location;
        desc.binding = desc_->binding;
        desc.format = desc_->format;
        desc.offset = desc_->offset;
    }

    VertexAttrState(uint32_t index_, const VkVertexInputAttributeDescription2EXT *desc_) : index(index_), desc(desc_) {}

    uint32_t index;  // Original index into the caller's pVertexAttributeDescriptions
    vku::safe_VkVertexInputAttributeDescription2EXT desc;
};

struct VertexBindingState {
    VertexBindingState(uint32_t index_, const VkVertexInputBindingDescription *desc_) : index(index_) {
        desc.binding = desc_->binding;
        desc.stride = desc_->stride;
        desc.inputRate = desc_->inputRate;
        desc.divisor = 1;
    }

    VertexBindingState(uint32_t index_, const VkVertexInputBindingDescription2EXT *desc_) : index(index_), desc(desc_) {}

    // Original index into the caller's pVertexBindingDescriptions
    uint32_t index;
    vku::safe_VkVertexInputBindingDescription2EXT desc;
    // Attributes for this binding, key is the location
    vvl::unordered_map<uint32_t, VertexAttrState> locations;
};

struct VertexInputState : public PipelineSubState {
    VertexInputState(const vvl::Pipeline &p, const vku::safe_VkGraphicsPipelineCreateInfo &create_info);

    vku::safe_VkPipelineVertexInputStateCreateInfo *input_state = nullptr;
    vku::safe_VkPipelineInputAssemblyStateCreateInfo *input_assembly_state = nullptr;

    // key is binding number
    vvl::unordered_map<uint32_t, VertexBindingState> bindings;

    std::shared_ptr<VertexInputState> FromCreateInfo(const vvl::Device &state,
                                                     const vku::safe_VkGraphicsPipelineCreateInfo &create_info);
};

struct PreRasterState : public PipelineSubState {
    PreRasterState(const vvl::Pipeline &p, const vvl::Device &dev_data, const vku::safe_VkGraphicsPipelineCreateInfo &create_info,
                   std::shared_ptr<const vvl::RenderPass> rp, spirv::StatelessData *stateless_data);

    static inline VkShaderStageFlags ValidShaderStages() {
        return VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
               VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
    }

    std::shared_ptr<const vvl::PipelineLayout> pipeline_layout;
    vku::safe_VkPipelineViewportStateCreateInfo *viewport_state = nullptr;
    vku::safe_VkPipelineRasterizationStateCreateInfo *raster_state = nullptr;
    const vku::safe_VkPipelineTessellationStateCreateInfo *tessellation_state = nullptr;

    std::shared_ptr<const vvl::RenderPass> rp_state;
    uint32_t subpass = 0;

    VkShaderStageFlagBits last_stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;

    std::shared_ptr<const vvl::ShaderModule> tessc_shader, tesse_shader;
    const vku::safe_VkPipelineShaderStageCreateInfo *tessc_shader_ci = nullptr, *tesse_shader_ci = nullptr;

    std::shared_ptr<const vvl::ShaderModule> vertex_shader, geometry_shader, task_shader, mesh_shader;
    const vku::safe_VkPipelineShaderStageCreateInfo *vertex_shader_ci = nullptr, *geometry_shader_ci = nullptr,
                                                   *task_shader_ci = nullptr, *mesh_shader_ci = nullptr;
};

std::unique_ptr<const vku::safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const vku::safe_VkPipelineColorBlendStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineColorBlendStateCreateInfo> ToSafeColorBlendState(
    const VkPipelineColorBlendStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const vku::safe_VkPipelineMultisampleStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ToSafeMultisampleState(
    const VkPipelineMultisampleStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const vku::safe_VkPipelineDepthStencilStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineDepthStencilStateCreateInfo> ToSafeDepthStencilState(
    const VkPipelineDepthStencilStateCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(
    const vku::safe_VkPipelineShaderStageCreateInfo &cbs);
std::unique_ptr<const vku::safe_VkPipelineShaderStageCreateInfo> ToShaderStageCI(const VkPipelineShaderStageCreateInfo &cbs);

struct FragmentShaderState : public PipelineSubState {
    FragmentShaderState(const vvl::Pipeline &pipeline_state, const vvl::Device &dev_data, std::shared_ptr<const vvl::RenderPass> rp,
                        uint32_t subpass, VkPipelineLayout layout);

    template <typename CreateInfo>
    FragmentShaderState(const vvl::Pipeline &pipeline_state, const vvl::Device &dev_data, const CreateInfo &create_info,
                        std::shared_ptr<const vvl::RenderPass> rp, spirv::StatelessData *stateless_data)
        : FragmentShaderState(pipeline_state, dev_data, rp, create_info.subpass, create_info.layout) {
        if (create_info.pMultisampleState) {
            ms_state = ToSafeMultisampleState(*create_info.pMultisampleState);
        }
        if (create_info.pDepthStencilState) {
            ds_state = ToSafeDepthStencilState(*create_info.pDepthStencilState);
        }
        FragmentShaderState::SetFragmentShaderInfo(pipeline_state, *this, dev_data, create_info, stateless_data);
    }

    static inline VkShaderStageFlags ValidShaderStages() { return VK_SHADER_STAGE_FRAGMENT_BIT; }

    std::shared_ptr<const vvl::RenderPass> rp_state;
    uint32_t subpass = 0;

    std::shared_ptr<const vvl::PipelineLayout> pipeline_layout;
    std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ms_state;
    std::unique_ptr<const vku::safe_VkPipelineDepthStencilStateCreateInfo> ds_state;

    std::shared_ptr<const vvl::ShaderModule> fragment_shader;
    std::unique_ptr<const vku::safe_VkPipelineShaderStageCreateInfo> fragment_shader_ci;
    // many times we need to quickly get the entry point to access the SPIR-V static data
    std::shared_ptr<const spirv::EntryPoint> fragment_entry_point;

  private:
    static void SetFragmentShaderInfo(const vvl::Pipeline &pipeline_state, FragmentShaderState &fs_state,
                                      const vvl::Device &state_data, const VkGraphicsPipelineCreateInfo &create_info,
                                      spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages]);
    static void SetFragmentShaderInfo(const vvl::Pipeline &pipeline_state, FragmentShaderState &fs_state,
                                      const vvl::Device &state_data, const vku::safe_VkGraphicsPipelineCreateInfo &create_info,
                                      spirv::StatelessData stateless_data[kCommonMaxGraphicsShaderStages]);
};

template <typename CreateInfo>
static bool IsSampleLocationEnabled(const CreateInfo &create_info) {
    bool result = false;
    if (create_info.pMultisampleState) {
        const auto *sample_location_state =
            vku::FindStructInPNextChain<VkPipelineSampleLocationsStateCreateInfoEXT>(create_info.pMultisampleState->pNext);
        if (sample_location_state != nullptr) {
            result = (sample_location_state->sampleLocationsEnable != 0);
        }
    }
    return result;
}

struct FragmentOutputState : public PipelineSubState {
    using AttachmentStateVector = std::vector<VkPipelineColorBlendAttachmentState>;

    FragmentOutputState(const vvl::Pipeline &p, std::shared_ptr<const vvl::RenderPass> rp, uint32_t sp);
    // For a graphics library, a "non-safe" create info must be passed in in order for pColorBlendState and pMultisampleState to not
    // get stripped out. If this is a "normal" pipeline, then we want to keep the logic from vku::safe_VkGraphicsPipelineCreateInfo
    // that strips out pointers that should be ignored.
    template <typename CreateInfo>
    FragmentOutputState(const vvl::Pipeline &p, const CreateInfo &create_info, std::shared_ptr<const vvl::RenderPass> rp)
        : FragmentOutputState(p, rp, create_info.subpass) {
        if (create_info.pColorBlendState) {
            const auto &cbci = *create_info.pColorBlendState;
            color_blend_state = ToSafeColorBlendState(cbci);
            // In case of being dynamic state
            if (cbci.pAttachments) {
                if (cbci.attachmentCount) {
                    attachment_states.reserve(cbci.attachmentCount);
                    std::copy(cbci.pAttachments, cbci.pAttachments + cbci.attachmentCount, std::back_inserter(attachment_states));
                }
                blend_constants_enabled = IsBlendConstantsEnabled(attachment_states);
            }
        }

        if (create_info.pMultisampleState) {
            ms_state = ToSafeMultisampleState(*create_info.pMultisampleState);
            sample_location_enabled = IsSampleLocationEnabled(create_info);
        }

        const auto flags2 = vku::FindStructInPNextChain<VkPipelineCreateFlags2CreateInfoKHR>(create_info.pNext);
        if (flags2) {
            legacy_dithering_enabled = (flags2->flags & VK_PIPELINE_CREATE_2_ENABLE_LEGACY_DITHERING_BIT_EXT) != 0;
        }

        // TODO
        // auto format_ci = vku::FindStructInPNextChain<VkPipelineRenderingFormatCreateInfoKHR>(gpci->pNext);
    }

    static bool IsBlendConstantsEnabled(const AttachmentStateVector &attachment_states);

    std::shared_ptr<const vvl::RenderPass> rp_state;
    uint32_t subpass = 0;

    std::unique_ptr<const vku::safe_VkPipelineColorBlendStateCreateInfo> color_blend_state;
    std::unique_ptr<const vku::safe_VkPipelineMultisampleStateCreateInfo> ms_state;

    AttachmentStateVector attachment_states;

    bool legacy_dithering_enabled = false;
    bool blend_constants_enabled = false;  // Blend constants enabled for any attachments
    bool sample_location_enabled = false;
};
