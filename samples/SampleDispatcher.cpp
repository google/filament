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

#include "include/samples/SampleDispatcher.h"

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <samples/SampleConfig.h>

#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility>

using namespace filament::app;

#define FILAMENT_SAMPLES_LIST(V)                                                                   \
    V(hellotriangle)                                                                               \
    V(hellomorphing)                                                                               \
    V(hellopbr)                                                                                    \
    V(helloskinning)                                                                               \
    V(hellouvmorphing)                                                                             \
    V(procedural_effect)                                                                           \
    V(procedural_texture_quad)                                                                     \
    V(rendertarget)                                                                                \
    V(sample_full_pbr)                                                                             \
    V(sample_normal_map)                                                                           \
    V(shadowtest)                                                                                  \
    V(skinningtest)                                                                                \
    V(strobecolor)                                                                                 \
    V(suzanne)                                                                                     \
    V(texturedquad)                                                                                \
    V(vbotest)

// Currently not working samples
//    V(sample_cloth)
//    V(lightbulb)
//

#define SAMPLES_PARAM_DECL(sampleName)                                                             \
    extern samples::SampleParameters create_ ## sampleName ## _AppParameters();

FILAMENT_SAMPLES_LIST(SAMPLES_PARAM_DECL)
#undef SAMPLES_PARAM_DECL

#define SAMPLES_APP_DECL(sampleName)                                                               \
    extern std::unique_ptr<FilamentApp2> create_ ## sampleName ## _App(SampleConfig config,        \
            DisplayManager* dm, filament::app::AssetLoader* loader);

FILAMENT_SAMPLES_LIST(SAMPLES_APP_DECL)
#undef SAMPLES_APP_DECL

namespace samples {

utils::FixedCapacityVector<utils::CString> getSampleNames() {
    static const utils::FixedCapacityVector<utils::CString> names = {
#define V(name) utils::CString(#name),
        FILAMENT_SAMPLES_LIST(V)
#undef V
    };
    return names;
}

SampleParameters getSampleParameters(const utils::CString& name) {
    using ParamFunc = SampleParameters (*)();
    static const std::unordered_map<std::string_view, ParamFunc> paramFuncs = {
#define V(sampleName) { #sampleName, &::create_ ## sampleName ## _AppParameters },
        FILAMENT_SAMPLES_LIST(V)
#undef V
    };

    if (auto itr = paramFuncs.find(name.c_str_safe()); itr != paramFuncs.end()) {
        return itr->second();
    }
    return {};
}

std::unique_ptr<FilamentApp2> dispatchSample(const utils::CString& name, SampleConfig config,
        DisplayManager* dm, filament::app::AssetLoader* loader) {

    using FactoryFunc = std::unique_ptr<FilamentApp2> (*)(SampleConfig, DisplayManager*,
            filament::app::AssetLoader*);
    static const std::unordered_map<std::string_view, FactoryFunc> factories = {
#define V(sampleName) { #sampleName, &::create_ ## sampleName ## _App },
        FILAMENT_SAMPLES_LIST(V)
#undef V
    };

    if (auto itr = factories.find(name.c_str_safe()); itr != factories.end()) {
        return itr->second(std::move(config), dm, loader);
    }
    return nullptr;
}

} // namespace samples
