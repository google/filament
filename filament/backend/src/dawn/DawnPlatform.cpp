/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "dawn/DawnPlatform.h"

#include "dawn/DawnDriver.h"
#include "webgpu/webgpu_cpp.h"

#include "vulkan/VulkanConstants.h"

namespace filament::backend {

Driver* DawnPlatform::createDriver(void* const sharedGLContext, const Platform::DriverConfig& driverConfig) noexcept {
    FVK_LOGI << "IDRIS: " <<__FILE__<<":"<<__LINE__<< " " <<__func__ << "\n";
    wgpu::InstanceDescriptor instance_descriptor;
    wgpu::Instance instance = wgpu::CreateInstance(&instance_descriptor);
    if (instance) {
        FVK_LOGI << "IDRIS: " << __func__ << " instance created\n";
    }
    return DawnDriver::create();
}

} // namespace filament
