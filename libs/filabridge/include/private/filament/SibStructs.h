/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_FILABRIDGE_SIBSTRUCTS_H
#define TNT_FILABRIDGE_SIBSTRUCTS_H

#include <stdint.h>
#include <stddef.h>

namespace filament {

struct PerViewSib {
    // indices of each sampler in this SamplerInterfaceBlock (see: getPerViewSib())
    static constexpr size_t SHADOW_MAP     = 0;     // user defined (1024x1024) DEPTH, array
    static constexpr size_t IBL_DFG_LUT    = 1;     // user defined (128x128), RGB16F
    static constexpr size_t IBL_SPECULAR   = 2;     // user defined, user defined, CUBEMAP
    static constexpr size_t SSAO           = 3;     // variable, RGB8 {AO, [depth]}
    static constexpr size_t SSR            = 4;     // variable, RGB_11_11_10, mipmapped
    static constexpr size_t STRUCTURE      = 5;     // variable, DEPTH
    static constexpr size_t FOG            = 6;     // variable, user defined, CUBEMAP

    static constexpr size_t SAMPLER_COUNT  = 7;
};

struct PerRenderPrimitiveMorphingSib {
    static constexpr size_t POSITIONS      = 0;
    static constexpr size_t TANGENTS       = 1;

    static constexpr size_t SAMPLER_COUNT  = 2;
};

struct PerRenderPrimitiveSkinningSib {
    static constexpr size_t BONE_INDICES_AND_WEIGHTS = 0;   //bone indices and weights

    static constexpr size_t SAMPLER_COUNT  = 1;
};

} // namespace filament

#endif //TNT_FILABRIDGE_SIBSTRUCTS_H
