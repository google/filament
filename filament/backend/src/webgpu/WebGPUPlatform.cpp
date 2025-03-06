/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "webgpu/WebGPUPlatform.h"
#include "webgpu/WebGPUDriver.h"

#include "utils/Log.h"

namespace filament::backend {

Driver* WebGPUPlatform::createDriver(void* const sharedContext , const Platform::DriverConfig& driverConfig) noexcept {
    wgpu::InstanceDescriptor instance_descriptor {};
    wgpu::Instance instance = wgpu::CreateInstance(&instance_descriptor);
    if (instance) {
        utils::slog.i << " WebGPU instance created\n";
    } else {
        utils::slog.e << " WebGPU instance failed to create\n";
        return nullptr;
    }
    return WebGPUDriver::create();
}

} // namespace filament
