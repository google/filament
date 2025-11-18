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

#ifndef TNT_FILAMENT_BACKEND_VULKANPIPELINESTATESERIALIZER_H
#define TNT_FILAMENT_BACKEND_VULKANPIPELINESTATESERIALIZER_H

#include "VulkanCommands.h"

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <bluevk/BlueVK.h>
#include <iostream>
#include <type_traits>
#include <sstream>
#include <vector>

namespace filament::backend {

// VulkanPipelineStateSerializer stores to a file the pipeline states.
class VulkanPipelineStateSerializer {
public:
    VulkanPipelineStateSerializer(VulkanPipelineStateSerializer const&) = delete;
    VulkanPipelineStateSerializer& operator=(VulkanPipelineStateSerializer const&) = delete;

    VulkanPipelineStateSerializer(const utils::CString& name);
    ~VulkanPipelineStateSerializer();

    void setID(VkPipeline cache);
    void setPipelineLayoutKey(uint32_t pipelineLayoutKey);
    VulkanPipelineStateSerializer& operator<<(
            const VkPipelineInputAssemblyStateCreateInfo& inputAsm);
    VulkanPipelineStateSerializer& operator<<(const VkPipelineViewportStateCreateInfo& viewport);
    VulkanPipelineStateSerializer& operator<<(const VkPipelineDynamicStateCreateInfo& dyStates);
    VulkanPipelineStateSerializer& operator<<(
            const VkPipelineRasterizationStateCreateInfo& rasterState);
    VulkanPipelineStateSerializer& operator<<(
            const VkPipelineDepthStencilStateCreateInfo& depthStencil);
    VulkanPipelineStateSerializer& operator<<(const VkPipelineMultisampleStateCreateInfo& multiSample);
    VulkanPipelineStateSerializer& operator<<(
            const VkPipelineColorBlendStateCreateInfo& blendStateInfo);

    struct ShaderStageInfo {
        VkShaderStageFlagBits stage;
        uint64_t module;
        std::string mName;
    };
    VulkanPipelineStateSerializer& operator<<(const ShaderStageInfo& shaderStages);
    VulkanPipelineStateSerializer& operator<<(
            const VkVertexInputAttributeDescription& attrib);
    VulkanPipelineStateSerializer& operator<<(const VkVertexInputBindingDescription& binding);

private:
    std::string mFileName;
    std::stringstream mBuffer;

    //temp data
    utils::CString mProgramName;
    std::vector<ShaderStageInfo> mShaderStages;
    std::vector<VkVertexInputAttributeDescription> mVtxAttribs;
    std::vector<VkVertexInputBindingDescription> mVtxBindings;
};
} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_VULKANPIPELINESTATESERIALIZER_H
