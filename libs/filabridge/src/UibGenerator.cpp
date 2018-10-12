/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "private/filament/UibGenerator.h"

#include "filament/EngineEnums.h"
#include "filament/driver/DriverEnums.h"

namespace filament {
    using namespace driver;

UniformInterfaceBlock& UibGenerator::getPerViewUib() noexcept  {
    // IMPORTANT NOTE: Respect std140 layout, don't update without updating Engine::PerViewUib
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("FrameUniforms")
            // transforms
            .add("viewFromWorldMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromViewMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("clipFromViewMatrix",      1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("viewFromClipMatrix",      1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("clipFromWorldMatrix",     1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("lightFromWorldMatrix",    1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            // view
            .add("resolution",              1, UniformInterfaceBlock::Type::FLOAT4, Precision::HIGH)
            // camera
            .add("cameraPosition",          1, UniformInterfaceBlock::Type::FLOAT3, Precision::HIGH)
            // time
            .add("time",                    1, UniformInterfaceBlock::Type::FLOAT, Precision::HIGH)
            // directional light
            .add("lightColorIntensity",     1, UniformInterfaceBlock::Type::FLOAT4)
            .add("sun",                     1, UniformInterfaceBlock::Type::FLOAT4)
            .add("lightDirection",          1, UniformInterfaceBlock::Type::FLOAT3)
            .add("fParamsX",                1, UniformInterfaceBlock::Type::UINT)
            // shadow
            .add("shadowBias",              1, UniformInterfaceBlock::Type::FLOAT3)
            .add("oneOverFroxelDimensionY", 1, UniformInterfaceBlock::Type::FLOAT)
            // froxels
            .add("zParams",                 1, UniformInterfaceBlock::Type::FLOAT4)
            .add("fParams",                 1, UniformInterfaceBlock::Type::UINT2)
            .add("origin",                  1, UniformInterfaceBlock::Type::FLOAT2)
            // froxels (again, for alignment purposes)
            .add("oneOverFroxelDimension",  1, UniformInterfaceBlock::Type::FLOAT)
            // ibl
            .add("iblLuminance",            1, UniformInterfaceBlock::Type::FLOAT)
            // camera
            .add("exposure",                1, UniformInterfaceBlock::Type::FLOAT)
            .add("ev100",                   1, UniformInterfaceBlock::Type::FLOAT)
            // ibl
            .add("iblSH",                   9, UniformInterfaceBlock::Type::FLOAT3)
            .build();
    return uib;
}

UniformInterfaceBlock& UibGenerator::getPerRenderableUib() noexcept {
    static UniformInterfaceBlock uib =  UniformInterfaceBlock::Builder()
            .name("ObjectUniforms")
            .add("worldFromModelMatrix",       1, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .add("worldFromModelNormalMatrix", 1, UniformInterfaceBlock::Type::MAT3, Precision::HIGH)
            .build();
    return uib;
}

UniformInterfaceBlock& UibGenerator::getLightsUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("LightsUniforms")
            .add("lights", CONFIG_MAX_LIGHT_COUNT, UniformInterfaceBlock::Type::MAT4, Precision::HIGH)
            .build();
    return uib;
}

UniformInterfaceBlock& UibGenerator::getPostProcessingUib() noexcept {
    static UniformInterfaceBlock uib =  UniformInterfaceBlock::Builder()
            .name("PostProcessUniforms")
            .add("uvScale", 1, UniformInterfaceBlock::Type::FLOAT2)
            .add("time",    1, UniformInterfaceBlock::Type::FLOAT)
            .add("yOffset", 1, UniformInterfaceBlock::Type::FLOAT)
            .build();
    return uib;
}

UniformInterfaceBlock& UibGenerator::getPerRenderableBonesUib() noexcept {
    static UniformInterfaceBlock uib = UniformInterfaceBlock::Builder()
            .name("BonesUniforms")
            .add("bones", CONFIG_MAX_BONE_COUNT * 6, UniformInterfaceBlock::Type::FLOAT4, Precision::MEDIUM)
            .build();
    return uib;
}

} // namespace filament
