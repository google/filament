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
#include "SharedShadersConstants.h"
#include "Lifetimes.h"
#include "PlatformRunner.h"

namespace test {

enum class ShaderLanguage : uint8_t {
    GLSL,
    MSL,
    WGSL
};

struct ShaderRequest {
    VertexShaderType mVertexType;
    FragmentShaderType mFragmentType;
    ShaderUniformType mUniformType;
};

class SharedShaders {
public:
    static Shader makeShader(filament::backend::DriverApi& api, Cleanup& cleanup,
            ShaderRequest request);
    static std::string getVertexShaderText(VertexShaderType vertex, ShaderUniformType uniform);
    static std::string getFragmentShaderText(FragmentShaderType fragment,
            ShaderUniformType uniform);
};

} // namespace test

#endif //TNT_SHAREDSHADERS_H
