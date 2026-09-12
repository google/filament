/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_SAMPLES_SAMPLECONFIG_H
#define TNT_SAMPLES_SAMPLECONFIG_H

#include <filament/Engine.h>

#include <camutils/Manipulator.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <samples/Parameter.h>

#include <cstdint>
#include <unordered_map>
#include <variant>

struct SampleConfig {
    utils::CString title;
    uint32_t width = 1024;
    uint32_t height = 640;
    utils::CString iblDirectory;
    utils::CString dirt;
    bool splitView = false;
    mutable filament::Engine::Backend backend = filament::Engine::Backend::DEFAULT;
    mutable filament::backend::FeatureLevel featureLevel =
            filament::backend::FeatureLevel::FEATURE_LEVEL_3;
    filament::camutils::Mode cameraMode = filament::camutils::Mode::ORBIT;
    bool resizeable = true;
    bool headless = false;
    int stereoscopicEyeCount = 2;
    utils::CString vulkanGPUHint;
    using WebGPUBackend = filament::Engine::Backend;
    WebGPUBackend forcedWebGPUBackend = WebGPUBackend::DEFAULT;
    enum class DisplayManager { SDL, WEB };
    DisplayManager displayManager = DisplayManager::SDL;
    filament::backend::AsynchronousMode asynchronousMode =
            filament::backend::AsynchronousMode::NONE;
    utils::CString screenshotPath;
    int warmupFrames = 10;
    float fixedTimeStep = 0.0f;
    utils::FixedCapacityVector<utils::CString> positionalArgs;
    std::unordered_map<utils::CString, samples::Parameter> parameters;

    bool getBool(const utils::CString& name, bool fallback = false) const {
        auto it = parameters.find(name);
        if (it != parameters.end() && std::holds_alternative<bool>(it->second.value)) {
            return std::get<bool>(it->second.value);
        }
        return fallback;
    }

    int getInt(const utils::CString& name, int fallback = 0) const {
        auto it = parameters.find(name);
        if (it != parameters.end() && std::holds_alternative<int>(it->second.value)) {
            return std::get<int>(it->second.value);
        }
        return fallback;
    }

    float getFloat(const utils::CString& name, float fallback = 0.0f) const {
        auto it = parameters.find(name);
        if (it != parameters.end() && std::holds_alternative<float>(it->second.value)) {
            return std::get<float>(it->second.value);
        }
        return fallback;
    }

    utils::CString getString(const utils::CString& name,
            const utils::CString& fallback = "") const {
        auto it = parameters.find(name);
        if (it != parameters.end() && std::holds_alternative<utils::CString>(it->second.value)) {
            return std::get<utils::CString>(it->second.value);
        }
        return fallback;
    }
};

namespace samples {
using SampleConfig = ::SampleConfig;
}

#endif // TNT_SAMPLES_SAMPLECONFIG_H
