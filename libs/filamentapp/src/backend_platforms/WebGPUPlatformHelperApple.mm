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

#include <backend/platforms/WebGPUPlatformApple.h>
#include <utils/Log.h>

#include "../PlatformHelper.h"

namespace filament::app {

using namespace filament::backend;

class FilamentAppWebGPUPlatform : public WebGPUPlatformApple {
public:
    FilamentAppWebGPUPlatform(Config::WebGPUBackend backend)
            : mBackend(backend) {}

    virtual WebGPUPlatform::Configuration getConfiguration() const noexcept override {
        WebGPUPlatform::Configuration config = {};
        switch (mBackend) {
            case Config::WebGPUBackend::VULKAN:
                config.forceBackendType = wgpu::BackendType::Vulkan;
                break;
            case Config::WebGPUBackend::METAL:
                config.forceBackendType = wgpu::BackendType::Metal;
                break;
            case Config::WebGPUBackend::DEFAULT:
                break;
            default:
                utils::slog.e << "FilamentApp: Unsupported webgpu backend was selected(="
                              << (int) mBackend << "). Selection is ignored." << utils::io::endl;
                break;
        }
        return config;
    }

private:
    Config::WebGPUBackend const mBackend;
};

filament::backend::Platform* createWebGPUPlatform(Config::WebGPUBackend forcedBackend) {
    return new FilamentAppWebGPUPlatform(forcedBackend);
}

} // namespace filament::app
