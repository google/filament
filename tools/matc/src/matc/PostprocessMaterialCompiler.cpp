/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "PostprocessMaterialCompiler.h"

#include <filamat/PostprocessMaterialBuilder.h>

using namespace filamat;

namespace matc {

bool PostprocessMaterialCompiler::run(const Config& config) {
    PostprocessMaterialBuilder builder;
    builder
        .platform(config.getPlatform())
        .targetApi(config.getTargetApi())
        .optimization(config.getOptimizationLevel())
        .printShaders(config.printShaders());

    Package package = builder.build();
    if (!package.isValid()) {
        return false;
    }
    return writePackage(package, config);
}

bool PostprocessMaterialCompiler::checkParameters(const Config& config) {
    // Check for output format.
    if (config.getOutput() == nullptr) {
        std::cerr << "Missing output filename. Aborting" << std::endl;
        return false;
    }
    return true;
}

} // namespace matc
