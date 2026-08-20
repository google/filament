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

#ifndef TNT_SAMPLES_ARGUMENTS_H
#define TNT_SAMPLES_ARGUMENTS_H

#include "SampleConfig.h"

#include <filamentapp/AssetLoader.h>
#include <filamentapp/DisplayManager.h>
#include <filamentapp/FilamentApp2.h>

#include <filament/Engine.h>

#include <utils/CString.h>
#include <utils/getopt.h>

#include <functional>
#include <memory>
#include <vector>

namespace samples {

FilamentApp2::Builder getBuilder(const SampleConfig& config,
        filament::app::DisplayManager* dm = nullptr, filament::app::AssetLoader* loader = nullptr);

std::unique_ptr<filament::app::DisplayManager> getDisplayManager(const SampleConfig& config);

using CustomArgumentHandler = std::function<bool(int opt, const utils::CString& arg)>;

struct CommandLineSpecification {
    utils::CString sampleDescription;
    utils::CString positionalArgsDescription;
    int requiredPositionalArgCount = 0;
    std::vector<char> requiredFlags;
    utils::CString customOptionsHelp;
    CustomArgumentHandler customHandler = nullptr;
    const char* customOptStr = "";
    const utils::getopt::option* customOptions = nullptr;
};

void printUsage(const char* execName, const CommandLineSpecification& spec = {});

int handleCommandLineArguments(int argc, char* argv[], SampleConfig* config,
        const CommandLineSpecification& spec = {});
} // namespace samples
#endif //TNT_SAMPLES_ARGUMENTS_H
