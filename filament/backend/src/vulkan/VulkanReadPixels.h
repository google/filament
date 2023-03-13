/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANREADPIXELS_H
#define TNT_FILAMENT_BACKEND_VULKANREADPIXELS_H

#include "VulkanTaskHandler.h"
#include "private/backend/Driver.h"

#include <bluevk/BlueVK.h>
#include <math/vec4.h>

#include <functional>
#include <vector>

namespace filament::backend {

struct VulkanContext;
struct VulkanRenderTarget;

class VulkanReadPixels {
public:
    using OnReadCompleteFunction = std::function<void(PixelBufferDescriptor&&)>;
    using SelecteMemoryFunction = std::function<uint32_t(uint32_t, VkFlags)>;
    using IsValidFenceFunction = std::function<bool(VkFence)>;
    using FenceList = std::vector<VkFence>;

    void initialize(VkDevice device) noexcept;

    void shutdown() noexcept;

    void run(VulkanRenderTarget const* srcTarget, uint32_t x, uint32_t y, uint32_t width,
            uint32_t height, uint32_t graphicsQueueFamilyIndex, PixelBufferDescriptor&& pbd,
            const FenceList& fences, VulkanTaskHandler& taskHandler,
            SelecteMemoryFunction const& selectMemoryFunc,
            IsValidFenceFunction const& isValidFenceFunc,
            OnReadCompleteFunction const& readCompleteFunc);

private:
    VkDevice mDevice = VK_NULL_HANDLE;
    VkCommandPool mCommandPool = VK_NULL_HANDLE;
};

}  // namespace filament::backend

#endif  // TNT_FILAMENT_BACKEND_VULKANREADPIXELS_H
