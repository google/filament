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

void createSemaphore(VkDevice device, VkSemaphore* semaphore);
VkFormat getVkFormat(ElementType type, bool normalized);
VkFormat getVkFormat(TextureFormat format);
uint32_t getBytesPerPixel(TextureFormat format);
VkCompareOp getCompareOp(SamplerCompareFunc func);
VkBlendFactor getBlendFactor(BlendFunction mode);
VkCullModeFlags getCullMode(CullingMode mode);
VkFrontFace getFrontFace(bool inverseFrontFaces);

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANUTILITY_H
