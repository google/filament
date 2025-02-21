/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKAN_UTILS_CONVERSION_H
#define TNT_FILAMENT_BACKEND_VULKAN_UTILS_CONVERSION_H

#include <backend/DriverEnums.h>

#include <private/backend/BackendUtils.h>  // for getFormatSize()

#include <bluevk/BlueVK.h>

// Methods for converting Filament types to Vulkan types

namespace filament::backend::fvkutils {

VkFormat getVkFormat(ElementType type, bool normalized, bool integer);
VkFormat getVkFormat(TextureFormat format);

// Converts PixelBufferDescriptor format + type pair into a VkFormat.
// NOTE: This function only returns formats that support VK_FORMAT_FEATURE_BLIT_SRC_BIT as per
// "Required Format Support" in the Vulkan specification. These are the only formats that can be
// used for format conversion. If the requested format does not support this feature, then
// VK_FORMAT_UNDEFINED is returned.
VkFormat getVkFormat(PixelDataFormat format, PixelDataType type);

// Converts an SRGB Vulkan format identifier into its corresponding canonical UNORM format. If the
// given format is not an SRGB format, it is returned unmodified. This function is useful when
// determining if blit-based conversion is needed, since SRGB is orthogonal to the actual bit layout
// of color components.
VkFormat getVkFormatLinear(VkFormat format);

// See also FTexture::computeTextureDataSize, which takes a public-facing Texture format rather
// than a driver-level Texture format, and can account for a specified byte alignment.
uint32_t getBytesPerPixel(TextureFormat format);

VkCompareOp getCompareOp(SamplerCompareFunc func);
VkBlendFactor getBlendFactor(BlendFunction mode);
VkCullModeFlags getCullMode(CullingMode mode);
VkFrontFace getFrontFace(bool inverseFrontFaces);
PixelDataType getComponentType(VkFormat format);
uint32_t getComponentCount(VkFormat format);
VkComponentMapping getSwizzleMap(TextureSwizzle const swizzle[4]);
VkShaderStageFlags getShaderStageFlags(ShaderStageFlags stageFlags);

inline VkImageViewType getViewType(SamplerType target) {
    switch (target) {
        case SamplerType::SAMPLER_CUBEMAP:
            return VK_IMAGE_VIEW_TYPE_CUBE;
        case SamplerType::SAMPLER_2D_ARRAY:
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        case SamplerType::SAMPLER_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        default:
            return VK_IMAGE_VIEW_TYPE_2D;
    }
}

inline VkPrimitiveTopology getPrimitiveTopology(PrimitiveType pt) noexcept {
    switch (pt) {
        case PrimitiveType::POINTS:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveType::LINES:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveType::LINE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveType::TRIANGLES:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveType::TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    }
}

}// namespace filament::backend::fvkutils

#endif// TNT_FILAMENT_BACKEND_VULKAN_UTILS_CONVERSION_H
