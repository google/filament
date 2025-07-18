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

#include "DescriptorSet.h"

#include "TypedUniformBuffer.h"

#include <private/filament/UibStructs.h>

#include <math/vec2.h>
#include <math/vec4.h>

#include <array>

namespace filament {
namespace backend {
struct Viewport;
} // namespace backend

class FEngine;
class DescriptorSetLayout;
struct CameraInfo;

class StructureDescriptorSet {
public:
    explicit StructureDescriptorSet(FEngine& engine) noexcept;

    void terminate(backend::DriverApi& driver);

    void commit(backend::DriverApi& driver) noexcept;

    void bind(backend::DriverApi& driver) const noexcept;


    // All UBO values that can affect user code must be set here

    void prepareCamera(FEngine const& engine, const CameraInfo& camera) noexcept;

    void prepareLodBias(float bias, math::float2 derivativesScale) noexcept;

    void prepareViewport(const backend::Viewport& physicalViewport,
            const backend::Viewport& logicalViewport) noexcept;

    void prepareTime(FEngine const& engine, math::float4 const& userTime) noexcept;

    void prepareMaterialGlobals(std::array<math::float4, 4> const& materialGlobals) noexcept;

private:
    DescriptorSetLayout const& mDescriptorSetLayout;
    DescriptorSet mDescriptorSet;
    TypedUniformBuffer<PerViewUib> mUniforms;
};

} // namespace filament
