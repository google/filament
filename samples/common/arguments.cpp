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

#include <filament/Engine.h>
#include <getopt/getopt.h>

#include <iostream>
#include <string>

namespace samples {

filament::Engine::Backend parseArgumentsForBackend(std::string backend) {
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
    static constexpr const char* OPTSTR = ":a:";
    static const struct option OPTIONS[] = {
        { "api", required_argument, nullptr, 'a' }, { nullptr, 0, nullptr, 0 }
        // termination of the option list
    };

    int opt;
    int optionIndex = 0;

    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            case 'a':
                return parseArgumentsForBackend(arg);
        }
    }
    // if no args were provided return OPENGL as the default
    return filament::Engine::Backend::OPENGL;
}
} // namespace samples
