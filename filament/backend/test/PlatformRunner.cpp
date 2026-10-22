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

#include "PlatformRunner.h"

namespace utils {

template<>
CString to_string<test::Backend>(test::Backend backend) noexcept {
    switch (backend) {
        case test::Backend::OPENGL: {
            return "OpenGL";
        }
        case test::Backend::VULKAN: {
            return "Vulkan";
        }
        case test::Backend::METAL: {
            return "Metal";
        }
        case test::Backend::WEBGPU: {
            return "WebGPU";
        }
        case test::Backend::NOOP:
        default: {
            return "No-op";
        }
    }
}

template<>
CString to_string(test::OperatingSystem os) noexcept {
    switch (os) {
        case test::OperatingSystem::LINUX: {
            return "Linux";
        }
        case test::OperatingSystem::APPLE: {
            return "Apple";
        }
        case test::OperatingSystem::OTHER:
        default: {
            return "Other";
        }
    }
}

} // namespace utils

