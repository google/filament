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

#include "arguments.h"

#ifndef __ANDROID__
#include <filamentapp/HtmlDisplayManager.h>
#include <filamentapp/SDLDisplayManager.h>
#endif

#include <filament/Engine.h>

#include <utils/getopt.h>
#include <utils/Path.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace {

utils::CString getBackendAPIArgumentsUsage() {
    return "   --api, -a\n"
           "       Specify the backend API: opengl, vulkan, metal, or webgpu\n\n";
}

filament::Engine::Backend parseArgumentsForBackend(const utils::CString& backend) {
    if (backend == "metal") {
        return filament::Engine::Backend::METAL;
    } else if (backend == "opengl") {
        return filament::Engine::Backend::OPENGL;
    } else if (backend == "vulkan") {
        return filament::Engine::Backend::VULKAN;
    } else if (backend == "webgpu") {
        return filament::Engine::Backend::WEBGPU;
    } else {
        std::cerr << "Unrecognized target API. Must be 'opengl'|'vulkan'|'metal'|'webgpu'."
                  << std::endl;
        exit(1);
    }
}

filament::Engine::Backend resolveBackend(filament::Engine::Backend backend) {
    if (backend == filament::Engine::Backend::DEFAULT) {
        // This mirrors the logic for choosing a backend given compile-time flags and client having
        // provided DEFAULT as the backend (see PlatformFactory.cpp)
#if defined(FILAMENT_IOS) || defined(__APPLE__)
        backend = filament::Engine::Backend::METAL;
#elif defined(__EMSCRIPTEN__) || defined(__ANDROID__)
        backend = filament::Engine::Backend::OPENGL;
#elif defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
        backend = filament::Engine::Backend::VULKAN;
#elif defined(FILAMENT_DRIVER_SUPPORTS_WEBGPU)
        backend = filament::Engine::Backend::WEBGPU;
#else
        // For windows and unknown platforms, let's pick GL
        backend = filament::Engine::Backend::OPENGL;
#endif
    }
    return backend;
}

} // namespace

namespace samples {

void printUsage(const char* name, const CommandLineSpecification& spec) {
    utils::CString exec_name(utils::Path(name).getName().c_str());
    utils::CString apiUsage = getBackendAPIArgumentsUsage();
    std::cout << exec_name.c_str() << "\n"
              << "Usage:\n"
              << "    " << exec_name.c_str_safe() << " [options]";
    if (!spec.positionalArgsDescription.empty()) {
        std::cout << " " << spec.positionalArgsDescription.c_str_safe();
    }
    std::cout << "\n\n";
    if (!spec.sampleDescription.empty()) {
        std::cout << spec.sampleDescription.c_str_safe() << "\n\n";
    }
    std::cout << "Options:\n"
              << "   --help, -h\n"
              << "       Prints this message\n\n"
              << apiUsage.c_str();
    if (!spec.customOptionsHelp.empty()) {
        std::cout << spec.customOptionsHelp.c_str_safe() << "\n";
    }
}

std::unique_ptr<filament::app::DisplayManager> getDisplayManager(const SampleConfig& config) {
#ifndef __ANDROID__
    if (config.displayManager == SampleConfig::DisplayManager::WEB) {
        return std::make_unique<filament::app::HtmlDisplayManager>();
    }
    return std::make_unique<filament::app::SDLDisplayManager>(config.backend);
#else
    return nullptr;
#endif
}

int handleCommandLineArguments(int argc, char* argv[], SampleConfig* config,
        const CommandLineSpecification& spec) {

    static constexpr const char* DEFAULT_OPTSTR = "ha:f:i:usc:rt:y:b:evg:dw:x";
    utils::CString optstr(DEFAULT_OPTSTR);
    if (spec.customOptStr && spec.customOptStr[0] != '\0') {
        utils::CString custom(spec.customOptStr);
        for (size_t i = 0; i < custom.length(); ++i) {
            char c = custom[i];
            if (c == ':') continue;
            const char* p = strchr(optstr.c_str(), c);
            if (p) {
                size_t pos = p - optstr.c_str();
                utils::CString newOpt(optstr.c_str(), pos);
                if (pos + 1 < optstr.length() && optstr[pos + 1] == ':') {
                    newOpt += utils::CString(optstr.c_str() + pos + 2, optstr.length() - pos - 2);
                } else {
                    newOpt += utils::CString(optstr.c_str() + pos + 1, optstr.length() - pos - 1);
                }
                optstr = newOpt;
            }
        }
        optstr += spec.customOptStr;
    }

    static const utils::getopt::option OPTIONS[] = {
        { "help", utils::getopt::no_argument, nullptr, 'h' },
        { "api", utils::getopt::required_argument, nullptr, 'a' },
        { "feature-level", utils::getopt::required_argument, nullptr, 'f' },
        { "batch", utils::getopt::required_argument, nullptr, 'b' },
        { "headless", utils::getopt::no_argument, nullptr, 'e' },
        { "ibl", utils::getopt::required_argument, nullptr, 'i' },
        { "ubershader", utils::getopt::no_argument, nullptr, 'u' },
        { "actual-size", utils::getopt::no_argument, nullptr, 's' },
        { "camera", utils::getopt::required_argument, nullptr, 'c' },
        { "eyes", utils::getopt::required_argument, nullptr, 'y' },
        { "recompute-aabb", utils::getopt::no_argument, nullptr, 'r' },
        { "settings", utils::getopt::required_argument, nullptr, 't' },
        { "split-view", utils::getopt::no_argument, nullptr, 'v' },
        { "vulkan-gpu-hint", utils::getopt::required_argument, nullptr, 'g' },
        { "screenshot-as-ppm", utils::getopt::no_argument, nullptr, 'd' },
        { "webgpu-backend", utils::getopt::required_argument, nullptr, 'w' },
        { "remote", utils::getopt::no_argument, nullptr, 'x' },
        { nullptr, 0, nullptr, 0 },
    };

    std::vector<utils::getopt::option> options;
    for (size_t i = 0; OPTIONS[i].name != nullptr; ++i) {
        options.push_back(OPTIONS[i]);
    }
    if (spec.customOptions) {
        for (size_t i = 0; spec.customOptions[i].name != nullptr; ++i) {
            options.push_back(spec.customOptions[i]);
        }
    }
    options.push_back({ nullptr, 0, nullptr, 0 });

    int opt;
    int option_index = 0;
    std::vector<int> seenOptions;
    while ((opt = utils::getopt::getopt_long(argc, argv, optstr.c_str(), options.data(),
                    &option_index)) >= 0) {
        seenOptions.push_back(opt);
        utils::CString const arg(utils::getopt::optarg ? utils::getopt::optarg : "");

        if (spec.customHandler && spec.customHandler(opt, arg)) {
            continue;
        }

        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0], spec);
                exit(0);
            case 'a':
                config->backend = parseArgumentsForBackend(arg);
                break;
            case 'f':
                if (arg == "1") {
                    config->featureLevel = filament::backend::FeatureLevel::FEATURE_LEVEL_1;
                } else if (arg == "2") {
                    config->featureLevel = filament::backend::FeatureLevel::FEATURE_LEVEL_2;
                } else if (arg == "3") {
                    config->featureLevel = filament::backend::FeatureLevel::FEATURE_LEVEL_3;
                } else {
                    std::cerr << "Unrecognized feature level. Must be 1, 2 or 3.\n";
                }
                break;
            case 'c':
                if (arg == "flight") {
                    config->cameraMode = filament::camutils::Mode::FREE_FLIGHT;
                } else if (arg == "orbit") {
                    config->cameraMode = filament::camutils::Mode::ORBIT;
                } else {
                    std::cerr << "Unrecognized camera mode. Must be 'flight'|'orbit'.\n";
                }
                break;
            case 'y': {
                int eyeCount = 0;
                try {
                    eyeCount = std::stoi(arg.c_str());
                } catch (std::invalid_argument& e) {
                }
                if (eyeCount >= 1 && eyeCount <= 4) {
                    config->stereoscopicEyeCount = eyeCount;
                } else {
                    std::cerr << "Eye count must be between 1 and 4.\n";
                }
                break;
            }
            case 'e':
                config->headless = true;
                break;
            case 'i':
                config->iblDirectory = arg;
                break;
            case 'u':
                config->customArgs["ubershader"] = utils::CString("true");
                break;
            case 's':
                config->customArgs["actualSize"] = utils::CString("true");
                break;
            case 'r':
                config->customArgs["recomputeAabb"] = utils::CString("true");
                break;
            case 't':
                config->customArgs["settings"] = utils::CString(arg.c_str());
                break;
            case 'b': {
                config->customArgs["batch"] = utils::CString(arg.c_str());
                break;
            }
            case 'v': {
                config->splitView = true;
                break;
            }
            case 'g': {
                config->vulkanGPUHint = arg;
                break;
            }
            case 'd': {
                config->customArgs["screenshotAsPPM"] = utils::CString("true");
                break;
            }
            case 'w': {
                config->forcedWebGPUBackend = parseArgumentsForBackend(arg);
                break;
            }
            case 'x': {
                config->displayManager = SampleConfig::DisplayManager::WEB;
                config->headless = true;
                break;
            }
        }
    }

    bool missingRequiredFlag = false;
    for (char reqFlag: spec.requiredFlags) {
        if (std::find(seenOptions.begin(), seenOptions.end(), int(reqFlag)) == seenOptions.end()) {
            missingRequiredFlag = true;
            break;
        }
    }
    int const numPositionalArgs = argc - utils::getopt::optind;
    if (missingRequiredFlag || numPositionalArgs < spec.requiredPositionalArgCount) {
        printUsage(argv[0], spec);
        exit(1);
    }

    // Make sure that default resolves to a particular backend.
    config->backend = resolveBackend(config->backend);

    return utils::getopt::optind;
}


} // namespace samples
