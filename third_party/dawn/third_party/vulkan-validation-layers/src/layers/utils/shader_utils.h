/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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
 *
 * The Shader Validation file is in charge of taking the Shader Module data and validating it
 */

#pragma once

#include <vulkan/vulkan_core.h>
#include "utils/vk_layer_utils.h"
#include "containers/custom_containers.h"

#include <spirv-tools/libspirv.hpp>

struct DeviceFeatures;
struct DeviceExtensions;
class APIVersion;

enum class ShaderObjectStage : uint32_t {
    VERTEX = 0u,
    TESSELLATION_CONTROL,
    TESSELLATION_EVALUATION,
    GEOMETRY,
    FRAGMENT,
    COMPUTE,
    TASK,
    MESH,

    LAST = 8u,
};

constexpr uint32_t kShaderObjectStageCount = 8u;

inline ShaderObjectStage VkShaderStageToShaderObjectStage(VkShaderStageFlagBits stage) {
    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return ShaderObjectStage::VERTEX;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return ShaderObjectStage::TESSELLATION_CONTROL;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return ShaderObjectStage::TESSELLATION_EVALUATION;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return ShaderObjectStage::GEOMETRY;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return ShaderObjectStage::FRAGMENT;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            return ShaderObjectStage::COMPUTE;
        case VK_SHADER_STAGE_TASK_BIT_EXT:
            return ShaderObjectStage::TASK;
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            return ShaderObjectStage::MESH;
        default:
            break;
    }
    return ShaderObjectStage::LAST;
}

class ValidationCache {
  public:
    static VkValidationCacheEXT Create(VkValidationCacheCreateInfoEXT const *pCreateInfo, uint32_t spirv_val_option_hash) {
        auto cache = new ValidationCache(spirv_val_option_hash);
        cache->Load(pCreateInfo);
        return VkValidationCacheEXT(cache);
    }

    void Load(VkValidationCacheCreateInfoEXT const *pCreateInfo);
    void Write(size_t *pDataSize, void *pData);
    void Merge(ValidationCache const *other);

    bool Contains(uint32_t hash) {
        auto guard = ReadLock();
        return good_shader_hashes_.count(hash) != 0;
    }

    void Insert(uint32_t hash) {
        auto guard = WriteLock();
        good_shader_hashes_.insert(hash);
    }

  private:
    ValidationCache(uint32_t spirv_val_option_hash) : spirv_val_option_hash_(spirv_val_option_hash) {}
    ReadLockGuard ReadLock() const { return ReadLockGuard(lock_); }
    WriteLockGuard WriteLock() { return WriteLockGuard(lock_); }

    void GetUUID(uint8_t *uuid);

    // Can hit cases where error appear/disappear if spirv-val settings are adjusted
    // see https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8031
    uint32_t spirv_val_option_hash_;

    // hashes of shaders that have passed validation before, and can be skipped.
    // we don't store negative results, as we would have to also store what was
    // wrong with them; also, we expect they will get fixed, so we're less
    // likely to see them again.
    vvl::unordered_set<uint32_t> good_shader_hashes_;
    mutable std::shared_mutex lock_;
};

spv_target_env PickSpirvEnv(const APIVersion &api_version, bool spirv_1_4);

void AdjustValidatorOptions(const DeviceExtensions &device_extensions, const DeviceFeatures &enabled_features,
                            spvtools::ValidatorOptions &out_options, uint32_t *out_hash);

void DumpSpirvToFile(std::string file_name, const char *spirv_data, size_t spirv_size);