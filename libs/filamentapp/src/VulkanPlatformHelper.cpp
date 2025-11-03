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

#include <filamentapp/VulkanPlatformHelper.h>

#include <backend/platforms/VulkanPlatform.h>

#include <utils/CString.h>

#include <algorithm>
#include <cstdlib>

namespace filament::filamentapp {

using namespace filament::backend;

VulkanPlatform::Customization parseGpuHint(char const* gpuHintCstr) {
    utils::CString gpuHint{ gpuHintCstr };
    if (gpuHint.empty()) {
        return {};
    }
    VulkanPlatform::Customization::GPUPreference pref;
    // Check to see if it is an integer, if so turn it into an index.
    if (std::all_of(gpuHint.begin(), gpuHint.end(), ::isdigit)) {
        char* p_end{};
        pref.index = static_cast<int8_t>(std::strtol(gpuHint.c_str(), &p_end, 10));
    } else {
        pref.deviceName = gpuHint;
    }
    return { .gpu = pref };
}

void destroyVulkanPlatform(VulkanPlatform* platform) {
    delete platform;
}

} // namespace filament::filamentapp
