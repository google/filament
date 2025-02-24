/* Copyright (c) 2019, 2021, 2023-2024 The Khronos Group Inc.
 * Copyright (c) 2019, 2021, 2023-2024 Valve Corporation
 * Copyright (c) 2019, 2021, 2023-2024 LunarG, Inc.
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

// Includes everything needed for overloading std::hash
#include "hash_util.h"

#include <vulkan/vulkan.h>
#include "generated/vk_extension_helper.h"
#include <vulkan/utility/vk_safe_struct.hpp>
#include <vector>

// Hash and equality and/or compare functions for selected Vk types (and useful collections thereof)

// VkDescriptorSetLayoutBinding
static inline bool operator==(const vku::safe_VkDescriptorSetLayoutBinding &lhs, const vku::safe_VkDescriptorSetLayoutBinding &rhs) {
    if ((lhs.binding != rhs.binding) || (lhs.descriptorType != rhs.descriptorType) ||
        (lhs.descriptorCount != rhs.descriptorCount) || (lhs.stageFlags != rhs.stageFlags) ||
        !hash_util::SimilarForNullity(lhs.pImmutableSamplers, rhs.pImmutableSamplers)) {
        return false;
    }
    if (lhs.pImmutableSamplers) {  // either one will do as they *are* similar for nullity (i.e. either both null or both non-null)
        for (uint32_t samp = 0; samp < lhs.descriptorCount; samp++) {
            if (lhs.pImmutableSamplers[samp] != rhs.pImmutableSamplers[samp]) {
                return false;
            }
        }
    }
    return true;
}

namespace std {
template <>
struct hash<vku::safe_VkDescriptorSetLayoutBinding> {
    size_t operator()(const vku::safe_VkDescriptorSetLayoutBinding &value) const {
        hash_util::HashCombiner hc;
        hc << value.binding << value.descriptorType << value.descriptorCount << value.stageFlags;
        if (value.pImmutableSamplers) {
            for (uint32_t samp = 0; samp < value.descriptorCount; samp++) {
                hc << value.pImmutableSamplers[samp];
            }
        }
        return hc.Value();
    }
};
}  // namespace std

// VkPushConstantRange
static inline bool operator==(const VkPushConstantRange &lhs, const VkPushConstantRange &rhs) {
    return (lhs.stageFlags == rhs.stageFlags) && (lhs.offset == rhs.offset) && (lhs.size == rhs.size);
}

namespace std {
template <>
struct hash<VkPushConstantRange> {
    size_t operator()(const VkPushConstantRange &value) const {
        hash_util::HashCombiner hc;
        return (hc << value.stageFlags << value.offset << value.size).Value();
    }
};
}  // namespace std

using PushConstantRanges = std::vector<VkPushConstantRange>;

namespace std {
template <>
struct hash<PushConstantRanges> : public hash_util::IsOrderedContainer<PushConstantRanges> {};
}  // namespace std

// VkImageSubresourceRange
static inline bool operator==(const VkImageSubresourceRange &lhs, const VkImageSubresourceRange &rhs) {
    return (lhs.aspectMask == rhs.aspectMask) && (lhs.baseMipLevel == rhs.baseMipLevel) && (lhs.levelCount == rhs.levelCount) &&
           (lhs.baseArrayLayer == rhs.baseArrayLayer) && (lhs.layerCount == rhs.layerCount);
}
namespace std {
template <>
struct hash<VkImageSubresourceRange> {
    size_t operator()(const VkImageSubresourceRange &value) const {
        hash_util::HashCombiner hc;
        hc << value.aspectMask << value.baseMipLevel << value.levelCount << value.baseArrayLayer << value.layerCount;
        return hc.Value();
    }
};
}  // namespace std

namespace std {
template <>
struct hash<const ExtEnabled DeviceExtensions::*> {
  public:
    size_t operator()(const ExtEnabled DeviceExtensions::*const &p) const noexcept {
        static DeviceExtensions dummy_de;
        static std::hash<uint8_t> h;
        return h(dummy_de.*p);
    }
};
}  // namespace std

static inline bool operator==(const VkShaderModuleIdentifierEXT &a, const VkShaderModuleIdentifierEXT &b) {
    if (a.identifierSize != b.identifierSize) {
        return false;
    }
    const uint32_t copy_size = std::min(VK_MAX_SHADER_MODULE_IDENTIFIER_SIZE_EXT, a.identifierSize);
    for (uint32_t i = 0u; i < copy_size; ++i) {
        if (a.identifier[i] != b.identifier[i]) {
            return false;
        }
    }
    return true;
}
namespace std {
template <>
struct hash<VkShaderModuleIdentifierEXT> {
    size_t operator()(const VkShaderModuleIdentifierEXT &value) const {
        return hash_util::HashCombiner().Combine(value.identifier, value.identifier + value.identifierSize).Value();
    }
};
}  // namespace std
