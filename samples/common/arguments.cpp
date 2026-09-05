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

#include <filamentapp/HtmlDisplayManager.h>
#include <filamentapp/SDLDisplayManager.h>

#include <filament/Engine.h>

#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/getopt.h>
#include <utils/Path.h>

#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

namespace {

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

samples::SampleParameters getCommonParameters() {
    return {
        samples::Parameter::makeEnum("api", 'a', "Specify the backend API", "default",
                { "opengl", "vulkan", "metal", "webgpu" }),
        samples::Parameter::makeInt("feature-level", 'f', "Specify feature level", 3, 1, 3, 1),
        samples::Parameter::makeBool("headless", 'e', "Run in headless mode", false),
        samples::Parameter::makeString("ibl", 'i', "Path to directory containing IBL", ""),
        samples::Parameter::makeEnum("camera", 'c', "Specify camera mode", "orbit",
                { "orbit", "flight" }),
        samples::Parameter::makeInt("eyes", 'y', "Stereoscopic eye count", 2, 1, 4, 1),
        samples::Parameter::makeBool("split-view", 'v', "Enable split-view", false),
        samples::Parameter::makeString("vulkan-gpu-hint", 'g',
                "Vulkan physical device selection hint", ""),
        samples::Parameter::makeEnum("webgpu-backend", 'w', "Forced WebGPU backend", "default",
                { "opengl", "vulkan", "metal", "webgpu" }),
        samples::Parameter::makeBool("remote", 'x', "Run web server and enable remote control",
                false),
        samples::Parameter::makeString("screenshot", '\0', "Output screenshot image path", ""),
        samples::Parameter::makeInt("frames", '\0', "Number of frames before capture / exit", 10,
                1),
        samples::Parameter::makeFloat("fixed-timestep", '\0',
                "Fixed animation timestep in seconds (<=0 for wallclock)", 0.0f, 0.0f),
        samples::Parameter::makeString("window-size", '\0', "Window size in WIDTHxHEIGHT format",
                ""),
    };
}

void printParameterHelp(const samples::Parameter& param) {
    std::cout << "   --" << param.name.c_str();
    if (param.shorthand != '\0') {
        std::cout << ", -" << param.shorthand;
    }
    switch (param.type) {
        case samples::ParameterType::INT:
            std::cout << " <integer>";
            break;
        case samples::ParameterType::FLOAT:
            std::cout << " <float>";
            break;
        case samples::ParameterType::STRING:
            std::cout << " <string>";
            break;
        case samples::ParameterType::ENUM:
            std::cout << " <choice>";
            break;
        case samples::ParameterType::BOOL:
            break;
    }
    if (param.required) {
        std::cout << " (required)";
    }
    std::cout << "\n       " << param.description.c_str();

    if (param.type == samples::ParameterType::ENUM && !param.choices.empty()) {
        std::cout << " [";
        for (size_t i = 0; i < param.choices.size(); ++i) {
            if (i > 0) std::cout << "|";
            std::cout << param.choices[i].c_str();
        }
        std::cout << "]";
    } else if (param.type == samples::ParameterType::INT && param.intRange) {
        if (param.intRange->min != std::numeric_limits<int>::min() &&
                param.intRange->max != std::numeric_limits<int>::max()) {
            std::cout << " [" << param.intRange->min << ".." << param.intRange->max << "]";
        }
    } else if (param.type == samples::ParameterType::FLOAT && param.floatRange) {
        if (param.floatRange->min != -std::numeric_limits<float>::infinity() &&
                param.floatRange->max != std::numeric_limits<float>::infinity()) {
            std::cout << " [" << param.floatRange->min << ".." << param.floatRange->max << "]";
        }
    }
    std::cout << "\n\n";
}

void validateSpecification(const samples::CommandLineSpecification& spec) {
    auto commonParams = getCommonParameters();

    for (size_t i = 0; i < spec.parameters.size(); ++i) {
        const auto& param = spec.parameters[i];
        assert_invariant(samples::Parameter::isValidName(param.name.c_str()));
        assert_invariant(param.name != "help");

        if (param.shorthand != '\0') {
            assert_invariant(param.shorthand != 'h');

            // Check against other sample parameters
            for (size_t j = i + 1; j < spec.parameters.size(); ++j) {
                const auto& other = spec.parameters[j];
                assert_invariant(param.name != other.name);
                if (other.shorthand != '\0') {
                    assert_invariant(param.shorthand != other.shorthand);
                }
            }

            // Check against common parameters
            for (const auto& cp : commonParams) {
                if (cp.shorthand != '\0' && param.name != cp.name) {
                    assert_invariant(param.shorthand != cp.shorthand);
                }
            }
        }
    }
}

} // namespace

namespace samples {

void printUsage(const char* name, const CommandLineSpecification& spec) {
    validateSpecification(spec);

    utils::CString exec_name(utils::Path(name).getName().c_str());
    std::cout << exec_name.c_str() << "\n"
              << "Usage:\n"
              << "    " << exec_name.c_str_safe() << " [options]";
    for (size_t i = 0; i < spec.positionalArgsDescription.size(); ++i) {
        const auto& desc = spec.positionalArgsDescription[i];
        if (i < static_cast<size_t>(spec.requiredPositionalArgCount)) {
            std::cout << " <" << desc.c_str_safe() << ">";
        } else {
            std::cout << " [" << desc.c_str_safe() << "]";
        }
    }
    std::cout << "\n\n";
    if (!spec.sampleDescription.empty()) {
        std::cout << spec.sampleDescription.c_str_safe() << "\n\n";
    }
    std::cout << "Options:\n"
              << "   --help, -h\n"
              << "       Prints this message\n\n";

    if (!spec.parameters.empty()) {
        std::cout << "Sample Options:\n";
        for (const auto& param: spec.parameters) {
            printParameterHelp(param);
        }
    }

    std::cout << "Common Options:\n";
    auto commonParams = getCommonParameters();
    for (const auto& param: commonParams) {
        bool overridden = false;
        for (const auto& sp: spec.parameters) {
            if (sp.name == param.name) {
                overridden = true;
                break;
            }
        }
        if (!overridden) {
            printParameterHelp(param);
        }
    }
}

FilamentApp2::Builder getBuilder(const SampleConfig& config, filament::app::DisplayManager* dm,
        filament::app::AssetLoader* loader, filament::app::AssetWriter* writer) {
    auto builder = FilamentApp2::Builder()
                           .title(config.title)
                           .size(config.width, config.height)
                           .iblDirectory(config.iblDirectory)
                           .dirt(config.dirt)
                           .splitView(config.splitView)
                           .backend(config.backend)
                           .featureLevel(config.featureLevel)
                           .cameraMode(config.cameraMode)
                           .resizeable(config.resizeable)
                           .headless(config.headless)
                           .stereoscopicEyeCount(config.stereoscopicEyeCount)
                           .vulkanGPUHint(config.vulkanGPUHint)
                           .forcedWebGPUBackend(config.forcedWebGPUBackend)
                           .asynchronousMode(config.asynchronousMode)
                           .screenshotPath(config.screenshotPath)
                           .warmupFrames(config.warmupFrames)
                           .fixedTimeStep(config.fixedTimeStep);

    if (dm) {
        builder.displayManager(dm);
    }
    if (loader) {
        builder.assetLoader(loader);
    }
    if (writer) {
        builder.assetWriter(writer);
    }
    return builder;
}

std::unique_ptr<filament::app::DisplayManager> getDisplayManager(const SampleConfig& config) {
    if (config.displayManager == SampleConfig::DisplayManager::WEB) {
        return std::make_unique<filament::app::HtmlDisplayManager>();
    }
    return std::make_unique<filament::app::SDLDisplayManager>(config.backend);
}

int handleCommandLineArguments(int argc, char* argv[], SampleConfig* config,
        const CommandLineSpecification& spec) {
    validateSpecification(spec);

    auto commonParams = getCommonParameters();
    auto activeCommonParams = SampleParameters::with_capacity(commonParams.size());
    for (const auto& cp: commonParams) {
        bool overridden = false;
        for (const auto& sp: spec.parameters) {
            if (sp.name == cp.name) {
                overridden = true;
                break;
            }
        }
        if (!overridden) {
            activeCommonParams.push_back(cp);
        }
    }

    std::vector<utils::getopt::option> longOptions;
    utils::CString optstr("h");

    longOptions.push_back({ "help", utils::getopt::no_argument, nullptr, 'h' });

    int nextOptionVal = 1000;

    struct OptionMapping {
        int val;
        bool isCommon;
        size_t index;
    };
    std::vector<OptionMapping> optionMappings;

    for (size_t i = 0; i < activeCommonParams.size(); ++i) {
        const auto& cp = activeCommonParams[i];
        int val = (cp.shorthand != '\0') ? cp.shorthand : nextOptionVal++;
        int hasArg = (cp.type == ParameterType::BOOL) ? utils::getopt::no_argument
                                                      : utils::getopt::required_argument;
        longOptions.push_back({ cp.name.c_str(), hasArg, nullptr, val });
        optionMappings.push_back({ val, true, i });

        if (cp.shorthand != '\0') {
            optstr += cp.shorthand;
            if (hasArg == utils::getopt::required_argument) {
                optstr += ":";
            }
        }
    }

    for (size_t i = 0; i < spec.parameters.size(); ++i) {
        const auto& sp = spec.parameters[i];
        int val = (sp.shorthand != '\0') ? sp.shorthand : nextOptionVal++;
        int hasArg = (sp.type == ParameterType::BOOL) ? utils::getopt::no_argument
                                                      : utils::getopt::required_argument;
        longOptions.push_back({ sp.name.c_str(), hasArg, nullptr, val });
        optionMappings.push_back({ val, false, i });

        if (sp.shorthand != '\0') {
            optstr += sp.shorthand;
            if (hasArg == utils::getopt::required_argument) {
                optstr += ":";
            }
        }

        config->parameters[sp.name] = sp;
    }

    longOptions.push_back({ nullptr, 0, nullptr, 0 });

    int opt;
    int option_index = 0;
    std::vector<bool> seenSampleParams(spec.parameters.size(), false);

    while ((opt = utils::getopt::getopt_long(argc, argv, optstr.c_str(), longOptions.data(),
                    &option_index)) >= 0) {
        utils::CString const arg(utils::getopt::optarg ? utils::getopt::optarg : "");

        if (opt == 'h') {
            printUsage(argv[0], spec);
            exit(0);
        }

        const OptionMapping* mapping = nullptr;
        for (const auto& m: optionMappings) {
            if (m.val == opt) {
                mapping = &m;
                break;
            }
        }

        if (!mapping) {
            continue;
        }

        if (mapping->isCommon) {
            const auto& cp = activeCommonParams[mapping->index];
            if (cp.name == "api") {
                config->backend = parseArgumentsForBackend(arg);
            } else if (cp.name == "feature-level") {
                int fl = 3;
                try {
                    fl = std::stoi(arg.c_str());
                } catch (...) {
                }
                if (fl == 1) {
                    config->featureLevel = filament::backend::FeatureLevel::FEATURE_LEVEL_1;
                } else if (fl == 2) {
                    config->featureLevel = filament::backend::FeatureLevel::FEATURE_LEVEL_2;
                } else if (fl == 3) {
                    config->featureLevel = filament::backend::FeatureLevel::FEATURE_LEVEL_3;
                } else {
                    std::cerr << "Unrecognized feature level. Must be 1, 2 or 3.\n";
                    exit(1);
                }
            } else if (cp.name == "camera") {
                if (arg == "flight") {
                    config->cameraMode = filament::camutils::Mode::FREE_FLIGHT;
                } else if (arg == "orbit") {
                    config->cameraMode = filament::camutils::Mode::ORBIT;
                } else {
                    std::cerr << "Unrecognized camera mode. Must be 'flight'|'orbit'.\n";
                    exit(1);
                }
            } else if (cp.name == "eyes") {
                int eyeCount = 0;
                try {
                    eyeCount = std::stoi(arg.c_str());
                } catch (...) {
                }
                if (eyeCount >= 1 && eyeCount <= 4) {
                    config->stereoscopicEyeCount = eyeCount;
                } else {
                    std::cerr << "Eye count must be between 1 and 4.\n";
                    exit(1);
                }
            } else if (cp.name == "headless") {
                config->headless = true;
            } else if (cp.name == "ibl") {
                config->iblDirectory = arg;
            } else if (cp.name == "split-view") {
                config->splitView = true;
            } else if (cp.name == "vulkan-gpu-hint") {
                config->vulkanGPUHint = arg;
            } else if (cp.name == "webgpu-backend") {
                config->forcedWebGPUBackend = parseArgumentsForBackend(arg);
            } else if (cp.name == "remote") {
                config->displayManager = SampleConfig::DisplayManager::WEB;
                config->headless = true;
            } else if (cp.name == "screenshot") {
                config->screenshotPath = arg;
                if (!arg.empty()) {
                    config->headless = true;
                }
            } else if (cp.name == "frames") {
                try {
                    config->warmupFrames = std::stoi(arg.c_str());
                } catch (...) {
                    std::cerr << "Failed to parse argument 'frames'" << std::endl;
                }
            } else if (cp.name == "fixed-timestep") {
                try {
                    config->fixedTimeStep = std::stof(arg.c_str());
                } catch (...) {
                    std::cerr << "Failed to parse argument 'fixed-timestep'" << std::endl;
                }
            } else if (cp.name == "window-size") {
                int w = 0, h = 0;
                if (sscanf(arg.c_str(), "%dx%d", &w, &h) == 2 && w > 0 && h > 0) {
                    config->width = uint32_t(w);
                    config->height = uint32_t(h);
                } else {
                    std::cerr << "Failed to parse argument 'window-size'. Should be of" <<
                            "format [int]x[int]" << std::endl;
                }
            }
        } else {
            seenSampleParams[mapping->index] = true;
            const auto& sp = spec.parameters[mapping->index];
            Parameter parsedParam = sp;
            switch (sp.type) {
                case ParameterType::BOOL:
                    parsedParam.value = true;
                    break;
                case ParameterType::INT: {
                    int val = 0;
                    try {
                        val = std::stoi(arg.c_str());
                    } catch (...) {
                        std::cerr << "Invalid integer value '" << arg.c_str() << "' for option --"
                                  << sp.name.c_str() << "\n";
                        exit(1);
                    }
                    if (sp.intRange && (val < sp.intRange->min || val > sp.intRange->max)) {
                        std::cerr << "Value " << val << " for option --" << sp.name.c_str()
                                  << " is out of range [" << sp.intRange->min << ".."
                                  << sp.intRange->max << "]\n";
                        exit(1);
                    }
                    parsedParam.value = val;
                    break;
                }
                case ParameterType::FLOAT: {
                    float val = 0.0f;
                    try {
                        val = std::stof(arg.c_str());
                    } catch (...) {
                        std::cerr << "Invalid float value '" << arg.c_str() << "' for option --"
                                  << sp.name.c_str() << "\n";
                        exit(1);
                    }
                    if (sp.floatRange && (val < sp.floatRange->min || val > sp.floatRange->max)) {
                        std::cerr << "Value " << val << " for option --" << sp.name.c_str()
                                  << " is out of range [" << sp.floatRange->min << ".."
                                  << sp.floatRange->max << "]\n";
                        exit(1);
                    }
                    parsedParam.value = val;
                    break;
                }
                case ParameterType::STRING:
                    parsedParam.value = arg;
                    break;
                case ParameterType::ENUM: {
                    bool matched = false;
                    for (const auto& choice: sp.choices) {
                        if (choice == arg) {
                            matched = true;
                            break;
                        }
                    }
                    if (!matched) {
                        std::cerr << "Invalid choice '" << arg.c_str() << "' for option --"
                                  << sp.name.c_str() << ". Allowed choices: [";
                        for (size_t c = 0; c < sp.choices.size(); ++c) {
                            if (c > 0) std::cerr << "|";
                            std::cerr << sp.choices[c].c_str();
                        }
                        std::cerr << "]\n";
                        exit(1);
                    }
                    parsedParam.value = arg;
                    break;
                }
            }
            config->parameters[sp.name] = parsedParam;
        }
    }

    bool missingRequiredParam = false;
    for (size_t i = 0; i < spec.parameters.size(); ++i) {
        if (spec.parameters[i].required && !seenSampleParams[i]) {
            std::cerr << "Error: Missing required option: --" << spec.parameters[i].name.c_str();
            if (spec.parameters[i].shorthand != '\0') {
                std::cerr << " (-" << spec.parameters[i].shorthand << ")";
            }
            std::cerr << "\n";
            missingRequiredParam = true;
        }
    }

    int const numPositionalArgs = argc - utils::getopt::optind;
    if (missingRequiredParam || numPositionalArgs < spec.requiredPositionalArgCount) {
        printUsage(argv[0], spec);
        exit(1);
    }

    if (config != nullptr) {
        config->positionalArgs =
                utils::FixedCapacityVector<utils::CString>::with_capacity(numPositionalArgs);
        for (int i = utils::getopt::optind; i < argc; ++i) {
            config->positionalArgs.push_back(utils::CString(argv[i]));
        }

        // Make sure that default resolves to a particular backend.
        config->backend = resolveBackend(config->backend);
    }

    return utils::getopt::optind;
}

} // namespace samples
