/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "SsrPassDescriptorSet.h"

#include "TypedUniformBuffer.h"

#include "details/Engine.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/UibStructs.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/debug.h>

#include <math/mat4.h>

#include <memory>

namespace filament {

using namespace backend;
using namespace math;

SsrPassDescriptorSet::SsrPassDescriptorSet() noexcept = default;

void SsrPassDescriptorSet::init(FEngine& engine) noexcept {
    // create the descriptor-set from the layout
    mDescriptorSet = DescriptorSet{ engine.getPerViewDescriptorSetLayoutSsrVariant() };
}

void SsrPassDescriptorSet::terminate(DriverApi& driver) {
    mDescriptorSet.terminate(driver);
}

void SsrPassDescriptorSet::setFrameUniforms(TypedUniformBuffer<PerViewUib>& uniforms) noexcept {
    // initialize the descriptor-set
    mDescriptorSet.setBuffer(+PerViewBindingPoints::FRAME_UNIFORMS,
            uniforms.getUboHandle(), 0, uniforms.getSize());

    mUniforms = std::addressof(uniforms);
}

void SsrPassDescriptorSet::prepareHistorySSR(Handle<HwTexture> ssr,
        math::mat4f const& historyProjection,
        math::mat4f const& uvFromViewMatrix,
        ScreenSpaceReflectionsOptions const& ssrOptions) noexcept {

    mDescriptorSet.setSampler(+PerViewBindingPoints::SSR, ssr, {
        .filterMag = SamplerMagFilter::LINEAR,
        .filterMin = SamplerMinFilter::LINEAR
    });

    assert_invariant(mUniforms);
    auto& s = mUniforms->edit();
    s.ssrReprojection = historyProjection;
    s.ssrUvFromViewMatrix = uvFromViewMatrix;
    s.ssrThickness = ssrOptions.thickness;
    s.ssrBias = ssrOptions.bias;
    s.ssrDistance = ssrOptions.enabled ? ssrOptions.maxDistance : 0.0f;
    s.ssrStride = ssrOptions.stride;
}

void SsrPassDescriptorSet::prepareStructure(Handle<HwTexture> structure) noexcept {
    // sampler must be NEAREST
    mDescriptorSet.setSampler(+PerViewBindingPoints::STRUCTURE, structure, {});
}

void SsrPassDescriptorSet::commit(FEngine& engine) noexcept {
    assert_invariant(mUniforms);
    DriverApi& driver = engine.getDriverApi();
    if (mUniforms->isDirty()) {
        driver.updateBufferObject(mUniforms->getUboHandle(),
                mUniforms->toBufferDescriptor(driver), 0);
    }
    mDescriptorSet.commit(engine.getPerViewDescriptorSetLayoutSsrVariant(), driver);
}

void SsrPassDescriptorSet::bind(backend::DriverApi& driver) noexcept {
    mDescriptorSet.bind(driver, DescriptorSetBindingPoints::PER_VIEW);
}

} // namespace filament

