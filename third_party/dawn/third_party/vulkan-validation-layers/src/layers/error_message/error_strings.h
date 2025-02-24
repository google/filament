/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include <sstream>
#include <string>

[[maybe_unused]] static std::string string_Attachment(uint32_t attachment) {
    if (attachment == VK_ATTACHMENT_UNUSED) {
        return "VK_ATTACHMENT_UNUSED";
    } else {
        return std::to_string(attachment);
    }
}

[[maybe_unused]] static std::string string_VkExtent2D(VkExtent2D extent) {
    std::stringstream ss;
    ss << "width = " << extent.width << ", height = " << extent.height;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkExtent3D(VkExtent3D extent) {
    std::stringstream ss;
    ss << "width = " << extent.width << ", height = " << extent.height << ", depth = " << extent.depth;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkOffset2D(VkOffset2D offset) {
    std::stringstream ss;
    ss << "x = " << offset.x << ", y = " << offset.y;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkOffset3D(VkOffset3D offset) {
    std::stringstream ss;
    ss << "x = " << offset.x << ", y = " << offset.y << ", z = " << offset.z;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkRect2D(VkRect2D rect) {
    std::stringstream ss;
    ss << "offset.x = " << rect.offset.x << ", offset.y = " << rect.offset.y << ", extent.width = " << rect.extent.width
       << ", extent.height = " << rect.extent.height;
    return ss.str();
}

[[maybe_unused]] static std::string string_LevelCount(const VkImageCreateInfo &ci, VkImageSubresourceRange const &range) {
    std::stringstream ss;
    if (range.levelCount == VK_REMAINING_MIP_LEVELS) {
        const uint32_t level_count = ci.mipLevels - range.baseMipLevel;
        ss << "VK_REMAINING_MIP_LEVELS [mipLevels (" << ci.mipLevels << ") - baseMipLevel (" << range.baseMipLevel
           << ") = " << level_count << "]";
    } else {
        ss << range.levelCount;
    }
    return ss.str();
}

[[maybe_unused]] static std::string string_LayerCount(const VkImageCreateInfo &ci, VkImageSubresourceRange const &range) {
    std::stringstream ss;
    if (range.layerCount == VK_REMAINING_ARRAY_LAYERS) {
        const uint32_t layer_count = ci.arrayLayers - range.baseArrayLayer;
        ss << "VK_REMAINING_ARRAY_LAYERS [arrayLayers (" << ci.arrayLayers << ") - baseArrayLayer (" << range.baseArrayLayer
           << ") = " << layer_count << "]";
    } else {
        ss << range.layerCount;
    }
    return ss.str();
}

[[maybe_unused]] static std::string string_VkPushConstantRange(VkPushConstantRange range) {
    std::stringstream ss;
    ss << "range [" << range.offset << ", " << (range.offset + range.size) << ") for "
       << string_VkShaderStageFlags(range.stageFlags);
    return ss.str();
}

[[maybe_unused]] static std::string string_VkImageSubresource(VkImageSubresource subresource) {
    std::stringstream ss;
    ss << "aspectMask = " << string_VkImageAspectFlags(subresource.aspectMask) << ", mipLevel = " << subresource.mipLevel
       << ", arrayLayer = " << subresource.arrayLayer;
    return ss.str();
}

[[maybe_unused]] static std::string string_VkComponentMapping(VkComponentMapping components) {
    std::stringstream ss;
    ss << "r swizzle = " << string_VkComponentSwizzle(components.r) << "\n";
    ss << "g swizzle = " << string_VkComponentSwizzle(components.g) << "\n";
    ss << "b swizzle = " << string_VkComponentSwizzle(components.b) << "\n";
    ss << "a swizzle = " << string_VkComponentSwizzle(components.a) << "\n";
    return ss.str();
}

[[maybe_unused]] static std::string string_VkBool32(VkBool32 value) { return value ? "VK_TRUE" : "VK_FALSE"; }
