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

#include "PerViewDescriptorSetUtils.h"

#include "details/Camera.h"
#include "details/Engine.h"

#include <private/filament/UibStructs.h>

#include <filament/Engine.h>
#include <filament/Viewport.h>

#include <backend/DriverEnums.h>

#include <math/mat4.h>
#include <math/vec4.h>

#include <array>

#include <stdint.h>

namespace filament {

using namespace backend;
using namespace math;

void PerViewDescriptorSetUtils::prepareCamera(PerViewUib& s,
        FEngine const& engine, const CameraInfo& camera) noexcept {
    mat4f const& viewFromWorld = camera.view;
    mat4f const& worldFromView = camera.model;
    mat4f const& clipFromView  = camera.projection;

    const mat4f viewFromClip{ inverse((mat4)camera.projection) };
    const mat4f worldFromClip{ highPrecisionMultiply(worldFromView, viewFromClip) };

    s.viewFromWorldMatrix = viewFromWorld;    // view
    s.worldFromViewMatrix = worldFromView;    // model
    s.clipFromViewMatrix  = clipFromView;     // projection
    s.viewFromClipMatrix  = viewFromClip;     // 1/projection
    s.worldFromClipMatrix = worldFromClip;    // 1/(projection * view)
    s.userWorldFromWorldMatrix = mat4f(inverse(camera.worldTransform));
    s.clipTransform = camera.clipTransform;
    s.cameraFar = camera.zf;
    s.oneOverFarMinusNear = 1.0f / (camera.zf - camera.zn);
    s.nearOverFarMinusNear = camera.zn / (camera.zf - camera.zn);

    mat4f const& headFromWorld = camera.view;
    Engine::Config const& config = engine.getConfig();
    for (int i = 0; i < config.stereoscopicEyeCount; i++) {
        mat4f const& eyeFromHead = camera.eyeFromView[i];   // identity for monoscopic rendering
        mat4f const& clipFromEye = camera.eyeProjection[i];
        // clipFromEye * eyeFromHead * headFromWorld
        s.clipFromWorldMatrix[i] = highPrecisionMultiply(
                clipFromEye, highPrecisionMultiply(eyeFromHead, headFromWorld));
    }

    // with a clip-space of [-w, w] ==> z' = -z
    // with a clip-space of [0,  w] ==> z' = (w - z)/2
    s.clipControl = const_cast<FEngine&>(engine).getDriverApi().getClipSpaceParams();
}

void PerViewDescriptorSetUtils::prepareLodBias(PerViewUib& s, float bias,
        float2 derivativesScale) noexcept {
    s.lodBias = bias;
    s.derivativesScale = derivativesScale;
}

void PerViewDescriptorSetUtils::prepareViewport(PerViewUib& s,
        backend::Viewport const& physicalViewport,
        backend::Viewport const& logicalViewport) noexcept {
    float4 const physical{ physicalViewport.left, physicalViewport.bottom,
                           physicalViewport.width, physicalViewport.height };
    float4 const logical{ logicalViewport.left, logicalViewport.bottom,
                          logicalViewport.width, logicalViewport.height };
    s.resolution = { physical.zw, 1.0f / physical.zw };
    s.logicalViewportScale = physical.zw / logical.zw;
    s.logicalViewportOffset = -logical.xy / logical.zw;
}

void PerViewDescriptorSetUtils::prepareTime(PerViewUib& s,
        FEngine const& engine, float4 const& userTime) noexcept {
    const uint64_t oneSecondRemainder = engine.getEngineTime().count() % 1000000000;
    const float fraction = float(double(oneSecondRemainder) / 1000000000.0);
    s.time = fraction;
    s.userTime = userTime;
}

void PerViewDescriptorSetUtils::prepareMaterialGlobals(PerViewUib& s,
        std::array<float4, 4> const& materialGlobals) noexcept {
    s.custom[0] = materialGlobals[0];
    s.custom[1] = materialGlobals[1];
    s.custom[2] = materialGlobals[2];
    s.custom[3] = materialGlobals[3];
}

} // namespace filament
