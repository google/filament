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

#include <utils/getopt.h>

#include <iostream>
#include <string>

namespace test {

TestArguments parseArguments(int argc, char* argv[]) {
    TestArguments arguments = {};
    arguments.backend = Backend::OPENGL;

    // The first colon in OPTSTR turns on silent error reporting. This is important, as the
    // arguments may also contain gtest parameters we don't know about.
    static constexpr const char* OPTSTR = ":a:kc";
    static const utils::getopt::option OPTIONS[] = {
            { "api", utils::getopt::required_argument, nullptr, 'a' },
            { "headless_only", utils::getopt::no_argument, nullptr, 'k' },
            { "ci", utils::getopt::no_argument, nullptr, 'c' },
            { nullptr, 0, nullptr, 0 }  // termination of the utils::getopt::option list
    };

    int opt;
    int optionIndex = 0;

    while ((opt = utils::getopt::getopt_long(argc, argv, OPTSTR, OPTIONS, &optionIndex)) >= 0) {
        std::string arg(utils::getopt::optarg ? utils::getopt::optarg : "");
        switch (opt) {
            case 'a':
                if (arg == "opengl") {
                    arguments.backend = Backend::OPENGL;
                } else if (arg == "vulkan") {
                    arguments.backend = Backend::VULKAN;
                } else if (arg == "metal") {
                    arguments.backend = Backend::METAL;
                } else if (arg == "webgpu") {
                    arguments.backend = Backend::WEBGPU;
                } else {
                    std::cerr << "Unrecognized target API. Must be 'opengl'|'vulkan'|'metal'|'webgpu'."
                              << std::endl
                              << "Defaulting to OpenGL."
                              << std::endl;
                }
                break;
            case 'k':
                arguments.headlessOnly = true;
                break;
            case 'c':
                arguments.isContinuousIntegration = true;
                break;
        }
    }

    return arguments;
}

} // namespace test
