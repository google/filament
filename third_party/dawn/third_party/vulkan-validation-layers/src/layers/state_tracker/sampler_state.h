/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "state_tracker/state_object.h"
#include <vulkan/utility/vk_safe_struct.hpp>

// Note: some of the types in this header are needed by both the DescriptorSet and Pipeline
// state objects. It is helpful to have a separate header to avoid circular #include madness.
struct DescriptorSlot {
    uint32_t set;
    uint32_t binding;

    DescriptorSlot(uint32_t s, uint32_t b) : set(s), binding(b) {}
};

inline bool operator==(const DescriptorSlot &lhs, const DescriptorSlot &rhs) noexcept {
    return lhs.set == rhs.set && lhs.binding == rhs.binding;
}

struct SamplerUsedByImage {
    DescriptorSlot sampler_slot;
    uint32_t sampler_index;
};

inline bool operator==(const SamplerUsedByImage &a, const SamplerUsedByImage &b) noexcept {
    return a.sampler_slot == b.sampler_slot && a.sampler_index == b.sampler_index;
}

namespace std {
template <>
struct hash<DescriptorSlot> {
    size_t operator()(DescriptorSlot slot) const noexcept { return hash<uint32_t>()(slot.set) ^ hash<uint32_t>()(slot.binding); }
};
template <>
struct hash<SamplerUsedByImage> {
    size_t operator()(SamplerUsedByImage s) const noexcept {
        return hash<DescriptorSlot>()(s.sampler_slot) ^ hash<uint32_t>()(s.sampler_index);
    }
};
}  // namespace std

namespace vvl {

class Sampler : public StateObject {
  public:
    const vku::safe_VkSamplerCreateInfo safe_create_info;
    const VkSamplerCreateInfo &create_info;

    const VkSamplerYcbcrConversion samplerConversion;
    const VkSamplerCustomBorderColorCreateInfoEXT customCreateInfo;

    Sampler(const VkSampler handle, const VkSamplerCreateInfo *pCreateInfo)
        : StateObject(handle, kVulkanObjectTypeSampler),
          safe_create_info(pCreateInfo),
          create_info(*safe_create_info.ptr()),
          samplerConversion(GetConversion(pCreateInfo)),
          customCreateInfo(GetCustomCreateInfo(pCreateInfo)) {}

    VkSampler VkHandle() const { return handle_.Cast<VkSampler>(); }

  private:
    static inline VkSamplerYcbcrConversion GetConversion(const VkSamplerCreateInfo *pCreateInfo) {
        auto *conversion_info = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
        return conversion_info ? conversion_info->conversion : VK_NULL_HANDLE;
    }
    static inline VkSamplerCustomBorderColorCreateInfoEXT GetCustomCreateInfo(const VkSamplerCreateInfo *pCreateInfo) {
        VkSamplerCustomBorderColorCreateInfoEXT result{};
        auto cbci = vku::FindStructInPNextChain<VkSamplerCustomBorderColorCreateInfoEXT>(pCreateInfo->pNext);
        if (cbci) result = *cbci;
        return result;
    }
};

class SamplerYcbcrConversion : public StateObject {
  public:
    const VkFormatFeatureFlags2KHR format_features;
    const VkFormat format;
    const VkFilter chromaFilter;
    const uint64_t external_format;

    SamplerYcbcrConversion(VkSamplerYcbcrConversion handle, const VkSamplerYcbcrConversionCreateInfo *info,
                           VkFormatFeatureFlags2KHR features)
        : StateObject(handle, kVulkanObjectTypeSamplerYcbcrConversion),
          format_features(features),
          format(info->format),
          chromaFilter(info->chromaFilter),
          external_format(GetExternalFormat(info->pNext)) {}

    VkSamplerYcbcrConversion VkHandle() const { return handle_.Cast<VkSamplerYcbcrConversion>(); }
};

}  // namespace vvl
