/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_SHAREDSHADERS_H
#define TNT_SHAREDSHADERS_H

#include "Shader.h"
#include "Lifetimes.h"
#include "PlatformRunner.h"

namespace test {

enum class ShaderLanguage : uint8_t {
    GLSL
};

struct ShaderEnvironment {
    ShaderLanguage mLanguage;
    Backend mBackend;
};

enum class ShaderUniformType : uint8_t {
    Simple,
    SimpleWithOffset,
    Sampler,
};

struct SimpleMaterialParams {
    filament::math::float4 color;
    filament::math::float4 offset;
    filament::math::float4 scale;
};

struct SimpleWithOffsetMaterialParams {
    // The associated uniform structure in the shader will have 64 bytes of padding at the beginning
    // So users of this struct will need to add 64 bytes to its size and offset all uniform writes.
    filament::math::float4 color;
    filament::math::float4 offset;
    filament::math::float4 scale;
};

enum class VertexShaderType : uint8_t {
    Simple,
    Textured
};

enum class FragmentShaderType : uint8_t {
    White,
    SolidColored,
    Textured
};

struct ShaderRequest {
    VertexShaderType mVertexType;
    FragmentShaderType mFragmentType;
    ShaderUniformType mUniformType;
};

class SharedShaders {
public:
    Shader makeShader(filament::backend::DriverApi& api, Cleanup& cleanup, ShaderEnvironment environment, ShaderRequest request);

private:
};

} // namespace test

#endif //TNT_SHAREDSHADERS_H
