/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "MotionVectorsDescriptorSet.h"

#include "PerViewDescriptorSetUtils.h"

#include "details/Camera.h"
#include "details/Engine.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/UibStructs.h>

#include <utils/compiler.h>
#include <utils/debug.h>

#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec4.h>

#include <array>

namespace filament {

using namespace backend;
using namespace math;

MotionVectorsDescriptorSet::MotionVectorsDescriptorSet() noexcept = default;

MotionVectorsDescriptorSet::~MotionVectorsDescriptorSet() noexcept {
    assert_invariant(!mDescriptorSet.getHandle());
}

void MotionVectorsDescriptorSet::init(FEngine& engine) noexcept {
#if defined(FILAMENT_SUPPORTS_MOTION_VECTORS)
    mUniforms.init(engine.getDriverApi());

    // the depth variants, including the motion vectors ones, use this layout
    mDescriptorSetLayout = &engine.getPerViewDescriptorSetLayoutDepthVariant();

    // create the descriptor-set from the layout
    mDescriptorSet = DescriptorSet{
        "MotionVectorsDescriptorSet", *mDescriptorSetLayout };

    // initialize the descriptor-set
    mDescriptorSet.setBuffer(*mDescriptorSetLayout, +PerViewBindingPoints::FRAME_UNIFORMS,
            mUniforms.getUboHandle(), 0, sizeof(PerViewUib));
#else
    (void)engine;
#endif
}

void MotionVectorsDescriptorSet::terminate(DriverApi& driver) {
#if defined(FILAMENT_SUPPORTS_MOTION_VECTORS)
    mDescriptorSet.terminate(driver);
    mUniforms.terminate(driver);
#else
    (void)driver;
#endif
}

void MotionVectorsDescriptorSet::bind(DriverApi& driver) const noexcept {
    assert_invariant(mDescriptorSetLayout);
    if (mUniforms.isDirty()) {
        mUniforms.clean();
        driver.updateBufferObject(mUniforms.getUboHandle(),
                mUniforms.toBufferDescriptor(driver), 0);
        mDescriptorSet.commit(*mDescriptorSetLayout, driver);
    }
    mDescriptorSet.bind(driver, DescriptorSetBindingPoints::PER_VIEW);
}

void MotionVectorsDescriptorSet::prepareViewport(
        backend::Viewport const& physicalViewport,
        backend::Viewport const& logicalViewport) noexcept {
    PerViewDescriptorSetUtils::prepareViewport(mUniforms.edit(), physicalViewport, logicalViewport);
}

void MotionVectorsDescriptorSet::prepareCamera(FEngine const& engine,
        CameraInfo const& camera) noexcept {
    PerViewDescriptorSetUtils::prepareCamera(mUniforms.edit(), engine, camera);
}

void MotionVectorsDescriptorSet::prepareLodBias(float const bias,
        float2 const derivativesScale) noexcept {
    PerViewDescriptorSetUtils::prepareLodBias(mUniforms.edit(), bias, derivativesScale);
}

void MotionVectorsDescriptorSet::prepareTime(FEngine const& engine,
        float4 const& userTime) noexcept {
    PerViewDescriptorSetUtils::prepareTime(mUniforms.edit(), engine, userTime);
}

void MotionVectorsDescriptorSet::prepareMaterialGlobals(
        std::array<float4, 4> const& materialGlobals) noexcept {
    PerViewDescriptorSetUtils::prepareMaterialGlobals(mUniforms.edit(), materialGlobals);
}

void MotionVectorsDescriptorSet::prepareMotionVectors(
        mat4f const& prevClipFromWorldMatrix) noexcept {
    mUniforms.edit().prevClipFromWorldMatrix = prevClipFromWorldMatrix;
}

} // namespace filament
