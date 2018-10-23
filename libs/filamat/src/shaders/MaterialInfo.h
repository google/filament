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

#ifndef TNT_FILAMAT_MATERIALINFO_H
#define TNT_FILAMAT_MATERIALINFO_H

#include <filament/driver/DriverEnums.h>
#include <filament/EngineEnums.h>
#include <filament/MaterialEnums.h>
#include <private/filament/UniformInterfaceBlock.h>
#include <filament/SamplerBindingMap.h>
#include <private/filament/SamplerInterfaceBlock.h>

#include <utils/compiler.h>

namespace filamat {

using UniformType = filament::driver::UniformType;
using SamplerType = filament::driver::SamplerType;
using CullingMode = filament::driver::CullingMode;

struct UTILS_PUBLIC MaterialInfo {
    bool isLit;
    bool isDoubleSided;
    bool hasExternalSamplers;
    bool hasShadowMultiplier;
    filament::AttributeBitset requiredAttributes;
    filament::BlendingMode blendingMode;
    filament::Shading shading;
    filament::UniformInterfaceBlock uib;
    filament::SamplerInterfaceBlock sib;
    filament::SamplerBindingMap samplerBindings;
};

}
#endif
