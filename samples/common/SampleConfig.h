#ifndef SAMPLE_CONFIG_H
#define SAMPLE_CONFIG_H

#include "Parameter.h"

#include <filament/Engine.h>

#include <camutils/Manipulator.h>

#include <utils/CString.h>

#include <unordered_map>

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
#endif
