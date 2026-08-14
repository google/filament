#ifndef SAMPLE_CONFIG_H
#define SAMPLE_CONFIG_H

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
    utils::CString fileName;
    std::unordered_map<utils::CString, utils::CString> customArgs;
};
#endif
