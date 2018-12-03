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

#ifndef TNT_FILAMAT_POSTPROCESS_PACKAGE_BUILDER_H
#define TNT_FILAMAT_POSTPROCESS_PACKAGE_BUILDER_H

#include <filamat/Package.h>
#include <filamat/MaterialBuilder.h>

#include <filament/driver/DriverEnums.h>

#include <utils/bitset.h>

namespace filamat {

class UTILS_PUBLIC PostprocessMaterialBuilder : public MaterialBuilderBase {
public:

    Package build();

    // specifies desktop vs mobile; works in concert with TargetApi to determine the shader models
    // (used to generate code) and final output representations (spirv and/or text).
    PostprocessMaterialBuilder& platform(Platform platform) noexcept {
        mPlatform = platform;
        return *this;
    }

    // specifies vulkan vs opengl; works in concert with Platform to determine the shader models
    // (used to generate code) and final output representations (spirv and/or text).
    PostprocessMaterialBuilder& targetApi(TargetApi targetApi) noexcept {
        mTargetApi = targetApi;
        return *this;
    }

    PostprocessMaterialBuilder& optimization(Optimization optimization) noexcept {
        mOptimization = optimization;
        return *this;
    }

    PostprocessMaterialBuilder& printShaders(bool printShaders) noexcept {
        mPrintShaders = printShaders;
        return *this;
    }
};

} // namespace
#endif // TNT_FILAMAT_POSTPROCESS_PACKAGE_BUILDER_H
