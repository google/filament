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
    mDescriptorSet = DescriptorSet{
            "SsrPassDescriptorSet", engine.getPerViewDescriptorSetLayoutSsrVariant() };

    // create a dummy Shadow UBO (see comment in setFrameUniforms() below)
    mShadowUbh = engine.getDriverApi().createBufferObject(sizeof(ShadowUib),
            BufferObjectBinding::UNIFORM, BufferUsage::STATIC);
}

void SsrPassDescriptorSet::terminate(DriverApi& driver) {
    mDescriptorSet.terminate(driver);
    driver.destroyBufferObject(mShadowUbh);
}

void SsrPassDescriptorSet::setFrameUniforms(FEngine const& engine,
        TypedUniformBuffer<PerViewUib>& uniforms) noexcept {
    // initialize the descriptor-set
    mDescriptorSet.setBuffer(engine.getPerViewDescriptorSetLayoutSsrVariant(),
            +PerViewBindingPoints::FRAME_UNIFORMS,
            uniforms.getUboHandle(), 0, uniforms.getSize());

    // This is actually not used for the SSR variants, but the descriptor-set layout needs
    // to have this UBO because the fragment shader used is the "generic" one. Both Metal
    // and GL would be okay without this, but Vulkan's validation layer would complain.
    mDescriptorSet.setBuffer(engine.getPerViewDescriptorSetLayoutSsrVariant(),
            +PerViewBindingPoints::SHADOWS, mShadowUbh, 0, sizeof(ShadowUib));
}

void SsrPassDescriptorSet::prepareHistorySSR(FEngine const& engine, Handle<HwTexture> ssr) noexcept {
    mDescriptorSet.setSampler(engine.getPerViewDescriptorSetLayoutSsrVariant(),
            +PerViewBindingPoints::SSR_HISTORY, ssr, {
                .filterMag = SamplerMagFilter::LINEAR,
                .filterMin = SamplerMinFilter::LINEAR
            });
}

void SsrPassDescriptorSet::prepareStructure(FEngine const& engine,
        Handle<HwTexture> structure) noexcept {
    // sampler must be NEAREST
    mDescriptorSet.setSampler(engine.getPerViewDescriptorSetLayoutSsrVariant(),
            +PerViewBindingPoints::STRUCTURE, structure, {});
}

void SsrPassDescriptorSet::commit(FEngine& engine) noexcept {
    DriverApi& driver = engine.getDriverApi();
    mDescriptorSet.commit(engine.getPerViewDescriptorSetLayoutSsrVariant(), driver);
}

void SsrPassDescriptorSet::bind(DriverApi& driver) noexcept {
    mDescriptorSet.bind(driver, DescriptorSetBindingPoints::PER_VIEW);
}

} // namespace filament

