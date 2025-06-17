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

#include <filament/Engine.h>

#include <utils/Path.h>

#include <getopt/getopt.h>

#include <iostream>
#include <string>

namespace samples {

std::string getBackendAPIArgumentsUsage() {
    return "   --api, -a\n"
           "       Specify the backend API: opengl, vulkan, metal, or webgpu\n\n";
}

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage("EXEC_NAME\n"
                      "Usage:\n"
                      "    EXEC_NAME [options]\n"
                      "Options:\n"
                      "   --help, -h\n"
                      "       Prints this message\n\n"
                      "API_USAGE");
    const std::string from("EXEC_NAME");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    const std::string apiUsage("API_USAGE");
    for (size_t pos = usage.find(apiUsage); pos != std::string::npos;
            pos = usage.find(apiUsage, pos)) {
        usage.replace(pos, apiUsage.length(), getBackendAPIArgumentsUsage());
    }
    std::cout << usage;
}

filament::Engine::Backend parseArgumentsForBackend(const std::string& backend) {
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

filament::Engine::Backend parseArgumentsForBackend(int argc, char* argv[]) {
    // The first colon in OPTSTR turns on silent error reporting. This is important, as the
    // arguments may also contain gtest parameters we don't know about.
    static constexpr const char* OPTSTR = "ha:";
    static const struct option OPTIONS[] = {
        { "help", no_argument,       nullptr, 'h' },
        { "api",  required_argument, nullptr, 'a' },
        { nullptr, 0,                nullptr, 0 }
    };
    int opt;
    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'a':
                return parseArgumentsForBackend(arg);
        }
    }
    // if no args were provided return DEFAULT
    return filament::Engine::Backend::DEFAULT;
}

} // namespace samples
