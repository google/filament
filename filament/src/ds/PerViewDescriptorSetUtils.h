/*
 * Copyright (C) 2025 The Android Open Source Project
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

#pragma once

#include <private/filament/UibStructs.h>

#include <math/vec2.h>
#include <math/vec4.h>

#include <array>

namespace filament {
namespace backend {
struct Viewport;
} // namespace backend

class FEngine;
struct CameraInfo;

class PerViewDescriptorSetUtils {
public:
    static void prepareCamera(PerViewUib& uniforms,
            FEngine const& engine, const CameraInfo& camera) noexcept;

    static void prepareLodBias(PerViewUib& uniforms,
            float bias, math::float2 derivativesScale) noexcept;

    static void prepareViewport(PerViewUib& uniforms,
            backend::Viewport const& physicalViewport,
            backend::Viewport const& logicalViewport) noexcept;

    static void prepareTime(PerViewUib& uniforms,
            FEngine const& engine, math::float4 const& userTime) noexcept;

    static void prepareMaterialGlobals(PerViewUib& uniforms,
            std::array<math::float4, 4> const& materialGlobals) noexcept;
};

} //namespace filament
