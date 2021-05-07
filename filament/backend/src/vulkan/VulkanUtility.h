/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_VULKANUTILITY_H
#define TNT_FILAMENT_DRIVER_VULKANUTILITY_H

#include <backend/DriverEnums.h>

#include <bluevk/BlueVK.h>

namespace filament {
namespace backend {

struct VulkanLayoutTransition {
    VkImage image;
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    VkImageSubresourceRange subresources;
    VkPipelineStageFlags srcStage;
    VkAccessFlags srcAccessMask;
    VkPipelineStageFlags dstStage;
    VkAccessFlags dstAccessMask;
};

void createSemaphore(VkDevice device, VkSemaphore* semaphore);
VkFormat getVkFormat(ElementType type, bool normalized, bool integer);
VkFormat getVkFormat(TextureFormat format);
VkFormat getVkFormat(PixelDataFormat format, PixelDataType type);
VkFormat getVkFormatLinear(VkFormat format);
uint32_t getBytesPerPixel(TextureFormat format);
VkCompareOp getCompareOp(SamplerCompareFunc func);
VkBlendFactor getBlendFactor(BlendFunction mode);
VkCullModeFlags getCullMode(CullingMode mode);
VkFrontFace getFrontFace(bool inverseFrontFaces);
PixelDataType getComponentType(VkFormat format);
VkComponentMapping getSwizzleMap(TextureSwizzle swizzle[4]);
void transitionImageLayout(VkCommandBuffer cmdbuffer, VulkanLayoutTransition transition);

} // namespace filament
} // namespace backend

bool operator<(const VkImageSubresourceRange& a, const VkImageSubresourceRange& b);

#endif // TNT_FILAMENT_DRIVER_VULKANUTILITY_H
