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

#ifndef TNT_FILABRIDGE_SIBGENERATOR_H
#define TNT_FILABRIDGE_SIBGENERATOR_H

#include <private/filament/EngineEnums.h>
#include <backend/DriverEnums.h>

#include <stdint.h>
#include <stddef.h>

namespace filament {

class SamplerInterfaceBlock;

class SibGenerator {
public:
    static SamplerInterfaceBlock const& getPerViewSib(uint8_t variantKey) noexcept;
    static SamplerInterfaceBlock const* getSib(uint8_t bindingPoint, uint8_t variantKey) noexcept;
    // When adding a sampler block here, make sure to also update
    //      FMaterial::getSurfaceProgramSlow and FMaterial::getPostProcessProgramSlow if needed
};

struct PerViewSib {
    // indices of each samplers in this SamplerInterfaceBlock (see: getPerViewSib())
    static constexpr size_t SHADOW_MAP     = 0;     // user defined (1024x1024) DEPTH, array
    static constexpr size_t FROXELS        = 1;     // 64x2048, RG16 {index, count, reserved}
    static constexpr size_t IBL_DFG_LUT    = 2;     // user defined (128x128), RGB16F
    static constexpr size_t IBL_SPECULAR   = 3;     // user defined, user defined, CUBEMAP
    static constexpr size_t SSAO           = 4;     // variable, RGB8 {AO, [depth]}
    static constexpr size_t SSR            = 5;     // variable, RGB_11_11_10, mipmapped
    static constexpr size_t STRUCTURE      = 6;     // variable, DEPTH

    static constexpr size_t SAMPLER_COUNT  = 7;
};

}
#endif // TNT_FILABRIDGE_SIBGENERATOR_H
