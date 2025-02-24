/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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

#include <spirv-tools/libspirv.hpp>
#include <glslang/Public/ShaderLang.h>

#include "render.h"
#include "shader_templates.h"

class VkRenderFramework;

// What is the incoming source to be turned into VkShaderModuleCreateInfo::pCode
typedef enum {
    SPV_SOURCE_GLSL,
    SPV_SOURCE_ASM,
    // TRY == Won't try in contructor as need to be called as function that can return the VkResult
    SPV_SOURCE_GLSL_TRY,
    SPV_SOURCE_ASM_TRY,
} SpvSourceType;

bool GLSLtoSPV(const VkPhysicalDeviceLimits &device_limits, const VkShaderStageFlagBits shader_type, const char *p_shader,
               std::vector<uint32_t> &spv, const spv_target_env spv_env = SPV_ENV_VULKAN_1_0);
bool ASMtoSPV(const spv_target_env target_env, const uint32_t options, const char *p_asm, std::vector<uint32_t> &spv);

// VkShaderObj is really just the Shader Module, but we named before VK_EXT_shader_object
// TODO - move all of VkShaderObj to vkt::ShaderModule
class VkShaderObj : public vkt::ShaderModule {
  public:
    // optional arguments listed order of most likely to be changed manually by a test
    VkShaderObj(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage,
                const spv_target_env env = SPV_ENV_VULKAN_1_0, SpvSourceType source_type = SPV_SOURCE_GLSL,
                const VkSpecializationInfo *spec_info = nullptr, char const *entry_point = "main", const void *pNext = nullptr);
    VkPipelineShaderStageCreateInfo const &GetStageCreateInfo() const;

    bool InitFromGLSL(const void *pNext = nullptr);
    VkResult InitFromGLSLTry(const vkt::Device *custom_device = nullptr);
    bool InitFromASM();
    VkResult InitFromASMTry();

    // These functions return a pointer to a newly created _and initialized_ VkShaderObj if initialization was successful.
    // Otherwise, {} is returned.
    static std::unique_ptr<VkShaderObj> CreateFromGLSL(VkRenderFramework *framework, const char *source,
                                                       VkShaderStageFlagBits stage, const spv_target_env = SPV_ENV_VULKAN_1_0,
                                                       const VkSpecializationInfo *spec_info = nullptr,
                                                       const char *entry_point = "main");
    static std::unique_ptr<VkShaderObj> CreateFromASM(VkRenderFramework *framework, const char *source, VkShaderStageFlagBits stage,
                                                      const spv_target_env spv_env = SPV_ENV_VULKAN_1_0,
                                                      const VkSpecializationInfo *spec_info = nullptr,
                                                      const char *entry_point = "main");

  protected:
    VkPipelineShaderStageCreateInfo m_stage_info;
    vkt::Device &m_device;
    const char *m_source;
    spv_target_env m_spv_env;
};
