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
    static SamplerInterfaceBlock const& getPerViewSib() noexcept;
    static SamplerInterfaceBlock const* getSib(uint8_t bindingPoint) noexcept;
};

struct PerViewSib {
    // indices of each samplers in this SamplerInterfaceBlock (see: getPerViewSib())
    static constexpr size_t SHADOW_MAP     = 0;
    static constexpr size_t RECORDS        = 1;
    static constexpr size_t FROXELS        = 2;
    static constexpr size_t IBL_DFG_LUT    = 3;
    static constexpr size_t IBL_SPECULAR   = 4;
    static constexpr size_t SSAO           = 5;
    static constexpr size_t SSR            = 6;

    static constexpr size_t SAMPLER_COUNT = 7;
};

}
#endif // TNT_FILABRIDGE_SIBGENERATOR_H
