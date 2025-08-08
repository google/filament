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

#ifndef TNT_SHAREDSHADERSCONSTANTS_H
#define TNT_SHAREDSHADERSCONSTANTS_H

#include "math/mathfwd.h"

enum class ShaderUniformType : uint8_t {
    None,
    Simple,
    SimpleWithPadding,
    Sampler2D,
    ISampler2D,
    USampler2D,
};

struct SimpleMaterialParams {
    filament::math::float4 color;
    // 1.0 will be added to this value before use.
    // The XY values are used to scale position inputs and the Z value is used to set the output
    // position's Z value.
    filament::math::float4 scaleMinusOne;
    // Offset will be applied after scale
    filament::math::float4 offset;
};

struct SimpleWithPaddingMaterialParams {
    // The associated uniform structure in the shader will have 64 bytes of padding at the beginning
    // So users of this struct will need to add 64 bytes to its size and offset all uniform writes.
    filament::math::float4 color;
    // 1.0 will be added to this value before use.
    // The XY values are used to scale position inputs and the Z value is used to set the output
    // position's Z value.
    filament::math::float4 scaleMinusOne;
    // Offset will be applied after scale
    filament::math::float4 offset;
};

enum class VertexShaderType : uint8_t {
    Noop,
    Simple,
    Textured
};

enum class FragmentShaderType : uint8_t {
    White,
    SolidColored,
    Textured
};

#endif //TNT_SHAREDSHADERSCONSTANTS_H
