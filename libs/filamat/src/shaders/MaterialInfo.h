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

#include <backend/DriverEnums.h>
#include <filament/MaterialEnums.h>
#include <private/filament/UniformInterfaceBlock.h>
#include <private/filament/SamplerBindingMap.h>
#include <private/filament/SamplerInterfaceBlock.h>

#include <utils/compiler.h>

namespace filamat {

using UniformType = filament::backend::UniformType;
using SamplerType = filament::backend::SamplerType;
using CullingMode = filament::backend::CullingMode;

struct UTILS_PUBLIC MaterialInfo {
    bool isLit;
    bool hasDoubleSidedCapability;
    bool hasExternalSamplers;
    bool hasShadowMultiplier;
    bool curvatureToRoughness;
    bool limitOverInterpolation;
    bool clearCoatIorChange;
    bool flipUV;
    filament::AttributeBitset requiredAttributes;
    filament::BlendingMode blendingMode;
    filament::BlendingMode postLightingBlendingMode;
    filament::Shading shading;
    filament::UniformInterfaceBlock uib;
    filament::SamplerInterfaceBlock sib;
    filament::SamplerBindingMap samplerBindings;
};

}
#endif // TNT_FILAMAT_MATERIALINFO_H
