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
    mDescriptorSet = DescriptorSet{ engine.getPerViewDescriptorSetLayoutDepthVariant() };

    // initialize the descriptor-set
    mDescriptorSet.setBuffer(+PerViewBindingPoints::FRAME_UNIFORMS,
            mUniformBufferHandle, 0, sizeof(PerViewUib));
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
        FEngine& engine, const CameraInfo& camera) noexcept {
    mat4f const& viewFromWorld = camera.view;
    mat4f const& worldFromView = camera.model;
    mat4f const& clipFromView  = camera.projection;

    const mat4f viewFromClip{ inverse((mat4)camera.projection) };
    const mat4f clipFromWorld{ highPrecisionMultiply(clipFromView, viewFromWorld) };
    const mat4f worldFromClip{ highPrecisionMultiply(worldFromView, viewFromClip) };

    auto& s = edit(transaction);
    s.viewFromWorldMatrix = viewFromWorld;    // view
    s.worldFromViewMatrix = worldFromView;    // model
    s.clipFromViewMatrix  = clipFromView;     // projection
    s.viewFromClipMatrix  = viewFromClip;     // 1/projection
    s.clipFromWorldMatrix[0] = clipFromWorld; // projection * view
    s.worldFromClipMatrix = worldFromClip;    // 1/(projection * view)
    s.userWorldFromWorldMatrix = mat4f(inverse(camera.worldTransform));
    s.clipTransform = camera.clipTransform;
    s.cameraFar = camera.zf;
    s.oneOverFarMinusNear = 1.0f / (camera.zf - camera.zn);
    s.nearOverFarMinusNear = camera.zn / (camera.zf - camera.zn);

    // with a clip-space of [-w, w] ==> z' = -z
    // with a clip-space of [0,  w] ==> z' = (w - z)/2
    s.clipControl = engine.getDriverApi().getClipSpaceParams();
}

void ShadowMapDescriptorSet::prepareLodBias(Transaction const& transaction, float bias) noexcept {
    auto& s = edit(transaction);
    s.lodBias = bias;
}

void ShadowMapDescriptorSet::prepareViewport(Transaction const& transaction,
        backend::Viewport const& viewport) noexcept {
    float2 const dimensions{ viewport.width, viewport.height };
    auto& s = edit(transaction);
    s.resolution = { dimensions, 1.0f / dimensions };
    s.logicalViewportScale = 1.0f;
    s.logicalViewportOffset = 0.0f;
}

void ShadowMapDescriptorSet::prepareTime(Transaction const& transaction,
        FEngine& engine, math::float4 const& userTime) noexcept {
    auto& s = edit(transaction);
    const uint64_t oneSecondRemainder = engine.getEngineTime().count() % 1'000'000'000;
    const float fraction = float(double(oneSecondRemainder) / 1'000'000'000.0);
    s.time = fraction;
    s.userTime = userTime;
}

void ShadowMapDescriptorSet::prepareShadowMapping(Transaction const& transaction,
        bool highPrecision) noexcept {
    auto& s = edit(transaction);
    constexpr float low  = 5.54f; // ~ std::log(std::numeric_limits<math::half>::max()) * 0.5f;
    constexpr float high = 42.0f; // ~ std::log(std::numeric_limits<float>::max()) * 0.5f;
    s.vsmExponent = highPrecision ? high : low;
}

ShadowMapDescriptorSet::Transaction ShadowMapDescriptorSet::open(backend::DriverApi& driver) noexcept {
    Transaction transaction;
    // TODO: use out-of-line buffer if too large
    transaction.uniforms = (PerViewUib *)driver.allocate(sizeof(PerViewUib), 16);
    assert_invariant(transaction.uniforms);
    return transaction;
}

void ShadowMapDescriptorSet::commit(Transaction& transaction,
        FEngine& engine, backend::DriverApi& driver) noexcept {
    driver.updateBufferObject(mUniformBufferHandle, {
            transaction.uniforms, sizeof(PerViewUib) }, 0);
    mDescriptorSet.commit(engine.getPerViewDescriptorSetLayoutDepthVariant(), driver);
    transaction.uniforms = nullptr;
}

void ShadowMapDescriptorSet::bind(backend::DriverApi& driver) noexcept {
    mDescriptorSet.bind(driver, DescriptorSetBindingPoints::PER_VIEW);
}

} // namespace filament

