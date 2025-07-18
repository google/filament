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

#include "ShadowMapDescriptorSet.h"

#include "PerViewDescriptorSetUtils.h"

#include "details/Camera.h"
#include "details/Engine.h"

#include <private/filament/EngineEnums.h>
#include <private/filament/DescriptorSets.h>
#include <private/filament/UibStructs.h>

#include <backend/DriverEnums.h>

#include <utils/debug.h>

#include <math/mat4.h>

#include <stdint.h>

namespace filament {

using namespace backend;
using namespace math;

ShadowMapDescriptorSet::ShadowMapDescriptorSet(FEngine& engine) noexcept {
    DriverApi& driver = engine.getDriverApi();

    mUniformBufferHandle = driver.createBufferObject(sizeof(PerViewUib),
            BufferObjectBinding::UNIFORM, BufferUsage::DYNAMIC);

    // create the descriptor-set from the layout
    mDescriptorSet = DescriptorSet{
            "ShadowMapDescriptorSet", engine.getPerViewDescriptorSetLayoutDepthVariant() };

    // initialize the descriptor-set
    mDescriptorSet.setBuffer(engine.getPerViewDescriptorSetLayoutDepthVariant(),
            +PerViewBindingPoints::FRAME_UNIFORMS, mUniformBufferHandle, 0, sizeof(PerViewUib));
}

void ShadowMapDescriptorSet::terminate(DriverApi& driver) {
    mDescriptorSet.terminate(driver);
    driver.destroyBufferObject(mUniformBufferHandle);
}

PerViewUib& ShadowMapDescriptorSet::edit(Transaction const& transaction) noexcept {
    assert_invariant(transaction.uniforms);
    return *transaction.uniforms;
}

void ShadowMapDescriptorSet::prepareCamera(Transaction const& transaction,
        FEngine const& engine, const CameraInfo& camera) noexcept {
    PerViewDescriptorSetUtils::prepareCamera(edit(transaction), engine, camera);
    // TODO: stereo values didn't used to be set
}

void ShadowMapDescriptorSet::prepareLodBias(Transaction const& transaction, float const bias) noexcept {
    PerViewDescriptorSetUtils::prepareLodBias(edit(transaction), bias, 0);
    // TODO: check why derivativesScale was missing
}

void ShadowMapDescriptorSet::prepareViewport(Transaction const& transaction,
        backend::Viewport const& viewport) noexcept {
    PerViewDescriptorSetUtils::prepareViewport(edit(transaction), viewport, viewport);
    // TODO: offset calculation is now different
}

void ShadowMapDescriptorSet::prepareTime(Transaction const& transaction,
        FEngine const& engine, float4 const& userTime) noexcept {
    PerViewDescriptorSetUtils::prepareTime(edit(transaction), engine, userTime);
}

void ShadowMapDescriptorSet::prepareShadowMapping(Transaction const& transaction,
        bool const highPrecision) noexcept {
    auto& s = edit(transaction);
    constexpr float low  = 5.54f; // ~ std::log(std::numeric_limits<math::half>::max()) * 0.5f;
    constexpr float high = 42.0f; // ~ std::log(std::numeric_limits<float>::max()) * 0.5f;
    s.vsmExponent = highPrecision ? high : low;
}

ShadowMapDescriptorSet::Transaction ShadowMapDescriptorSet::open(DriverApi& driver) noexcept {
    Transaction transaction;
    // TODO: use out-of-line buffer if too large
    transaction.uniforms = (PerViewUib *)driver.allocate(sizeof(PerViewUib), 16);
    assert_invariant(transaction.uniforms);
    return transaction;
}

void ShadowMapDescriptorSet::commit(Transaction& transaction,
        FEngine& engine, DriverApi& driver) noexcept {
    driver.updateBufferObject(mUniformBufferHandle, {
            transaction.uniforms, sizeof(PerViewUib) }, 0);
    mDescriptorSet.commit(engine.getPerViewDescriptorSetLayoutDepthVariant(), driver);
    transaction.uniforms = nullptr;
}

void ShadowMapDescriptorSet::bind(DriverApi& driver) noexcept {
    mDescriptorSet.bind(driver, DescriptorSetBindingPoints::PER_VIEW);
}

} // namespace filament

