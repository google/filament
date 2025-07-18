/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "ShadowMap.h"

#include <filament/Box.h>
#include <filament/Frustum.h>
#include <filament/LightManager.h>

#include "components/LightManager.h"

#include "details/DebugRegistry.h"
#include "details/Engine.h"
#include "details/Scene.h"

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Entity.h>
#include <utils/Slice.h>

#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/scalar.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <type_traits>

#include <stddef.h>
#include <stdint.h>

using namespace utils;

namespace filament {

using namespace math;
using namespace backend;

ShadowMap::ShadowMap(FEngine& engine) noexcept
        : mPerShadowMapUniforms(engine),
          mShadowType(ShadowType::DIRECTIONAL),
          mHasVisibleShadows(false),
          mFace(0) {
    Entity entities[2];
    engine.getEntityManager().create(2, entities);
    mCamera = engine.createCamera(entities[0]);
    mDebugCamera = engine.createCamera((entities[1]));
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.shadowmap.focus_shadowcasters", &engine.debug.shadowmap.focus_shadowcasters);
    debugRegistry.registerProperty("d.shadowmap.far_uses_shadowcasters", &engine.debug.shadowmap.far_uses_shadowcasters);
    debugRegistry.registerProperty("d.shadowmap.dzn", &engine.debug.shadowmap.dzn);
    debugRegistry.registerProperty("d.shadowmap.dzf", &engine.debug.shadowmap.dzf);
}

void ShadowMap::terminate(FEngine& engine) {
    Entity entities[] = { mCamera->getEntity(), mDebugCamera->getEntity() };
    for (Entity const e : entities) {
        engine.destroyCameraComponent(e);
    }
    engine.getEntityManager().destroy(sizeof(entities) / sizeof(Entity), entities);
    mPerShadowMapUniforms.terminate(engine.getDriverApi());
}

ShadowMap::~ShadowMap() = default;

void ShadowMap::initialize(size_t const lightIndex, ShadowType const shadowType,
        uint16_t const shadowIndex, uint8_t const face,
        LightManager::ShadowOptions const* options) {
    mLightIndex = lightIndex;
    mShadowIndex = shadowIndex;
    mOptions = options;
    mShadowType = shadowType;
    mFace = face;
}

mat4f ShadowMap::getDirectionalLightViewMatrix(float3 direction, float3 up,
        float3 position) noexcept {
    // 1. we use the x-axis as the "up" reference so that the math is stable when the light
    //    is pointing down, which is a common case for lights.
    // 2. we do the math in double to avoid some precision issues when the light is almost
    //    straight (i.e. parallel to the x-axis)
    mat4f const Mm = mat4f{ mat4::lookTo(direction, position, up) };
    return FCamera::rigidTransformInverse(Mm);
}

mat4f ShadowMap::getPointLightViewMatrix(TextureCubemapFace face,
        float3 position) noexcept {
    float3 direction;
    switch (TextureCubemapFace(face)) {
        case TextureCubemapFace::POSITIVE_X:    direction = {  1,  0,  0 }; break;
        case TextureCubemapFace::NEGATIVE_X:    direction = { -1,  0,  0 }; break;
        case TextureCubemapFace::POSITIVE_Y:    direction = {  0,  1,  0 }; break;
        case TextureCubemapFace::NEGATIVE_Y:    direction = {  0, -1,  0 }; break;
        case TextureCubemapFace::POSITIVE_Z:    direction = {  0,  0,  1 }; break;
        case TextureCubemapFace::NEGATIVE_Z:    direction = {  0,  0, -1 }; break;
    }
    const mat4f Mv = getDirectionalLightViewMatrix(direction, { 0, 1, 0 }, position);
    return Mv;
}

ShadowMap::ShaderParameters ShadowMap::updateDirectional(FEngine& engine,
        FScene::LightSoa const& lightData, size_t index,
        CameraInfo const& camera,
        ShadowMapInfo const& shadowMapInfo,
        SceneInfo const& sceneInfo,
        bool useDepthClamp) noexcept {

    // reset the visible shadow status
    mHasVisibleShadows = false;

    FLightManager const& lcm = engine.getLightManager();
    FLightManager::Instance const li = lightData.elementAt<FScene::LIGHT_INSTANCE>(index);
    FLightManager::ShadowParams const params = lcm.getShadowParams(li);

    const auto direction = lightData.elementAt<FScene::SHADOW_DIRECTION>(index);

    auto const [Mv, znear, zfar, lsClippedShadowVolume, vertexCount, visibleShadows] =
            computeDirectionalShadowBounds(engine, direction, params, camera, sceneInfo,
                    useDepthClamp);

    if (UTILS_UNLIKELY(!visibleShadows)) {
        return {};
    }

    // if the 2D bounds of the intersection is empty seen from the light, we have no shadow
    Aabb const bounds = compute2DBounds(Mv, lsClippedShadowVolume.data(), vertexCount);
    if (UTILS_UNLIKELY(!(bounds.min.x < bounds.max.x && bounds.min.y < bounds.max.y))) {
        return {};
    }

    /*
     * Compute the light's projection matrix
     * (directional/point lights, i.e. projection to use, including znear/zfar clip planes)
     */

    // The light's projection, ortho for directional lights, perspective otherwise
    const mat4f Mp = directionalLightFrustum(znear, zfar);
    mat4f LMp = Mp;
    mat4f LMpMv = Mp * Mv;

    /*
     * Compute warping (optional, improve quality)
     */

    mat4f W, Wp, L;
    // We can't use LISPSM in stable mode
    const bool useLispsm = params.options.lispsm && !params.options.stable;

    if (useLispsm ||
            (!params.options.stable && !engine.debug.shadowmap.disable_light_frustum_align)) {
        // Orient the shadow map in the direction of the view vector by constructing a
        const float3 lsCameraFwd = Mv.upperLeft() * camera.getForwardVector();
        L = computeLightRotation(lsCameraFwd);
        LMp = L * Mp;
        LMpMv = LMp * Mv;
    }

    if (useLispsm) {
        W = applyLISPSM(Wp, camera, params,
                LMp, Mv, LMpMv, lsClippedShadowVolume, vertexCount, direction);
    }

    /*
     * Compute focusing matrix (optional, greatly improves quality)
     */

    const mat4f WLMp = W * LMp;

    // 2) Now we find the x-y bounds of our convex-hull (view volume & shadow receivers)
    //    in light space, so we can "focus" the shadow map to the interesting area.
    //    This is the most important step to increase the quality of the shadow map.
    //
    //   In LiPSM mode, we're using the warped space here.
    float4 f = computeFocusParams(LMpMv, WLMp,
            lsClippedShadowVolume, vertexCount,
            camera, params.options.shadowFar, params.options.stable);

    if (params.options.stable) {
        const auto lsRef = lightData.elementAt<FScene::SHADOW_REF>(index);
        snapLightFrustum(f.xy, f.zw, lsRef, shadowMapInfo.shadowDimension);
    }

    const mat4f F(mat4f::row_major_init {
            f.x,  0.0f, 0.0f, f.z,
            0.0f, f.y,  0.0f, f.w,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
    });

    /*
     * Final shadow map transform
     */

    // Final shadow transform (focused warped light-space)
    const mat4f S = F * (W * LMpMv);

    // Computes St the transform to use in the shader to access the shadow map texture
    // i.e. it transforms a world-space vertex to a texture coordinate in the shadowmap
    const auto [Mt, Mn] = getTextureCoordsMapping(shadowMapInfo, getViewport());
    const mat4f St = highPrecisionMultiply(Mt, S);

    ShaderParameters shaderParameters;
    shaderParameters.scissorNormalized = getClampToEdgeCoords(shadowMapInfo);

    // note: in texelSizeWorldSpace() below, we can use Mb * Mt * F * W because
    // L * Mp * Mv is a rigid transform for directional lights, and doesn't matter.
    // if Wp[3][1] is 0, then LISPSM was cancelled.
    if (useLispsm && Wp[3][1] != 0.0f) {
        shaderParameters.texelSizeAtOneMeterWs =
                texelSizeWorldSpace(Wp, mat4f(Mt * F), shadowMapInfo.shadowDimension);
    } else {
        // We know we're using an ortho projection
        shaderParameters.texelSizeAtOneMeterWs =
                texelSizeWorldSpace(St.upperLeft(), shadowMapInfo.shadowDimension);
    }
    if (!shadowMapInfo.vsm) {
        shaderParameters.lightSpace = St;
    } else {
        shaderParameters.lightSpace = computeVsmLightSpaceMatrix(St, Mv, znear, zfar);
    }

    // We apply the constant bias in world space (as opposed to light-space) to account
    // for perspective and lispsm shadow maps. This also allows us to do this at zero-cost
    // by baking it in the shadow-map itself.
    const float constantBias = shadowMapInfo.vsm ? 0.0f : params.options.constantBias;
    const mat4f b = mat4f::translation(direction * constantBias);

    // It's important to set the light camera's model matrix separately from its projection, so
    // that the cameraPosition uniform gets set correctly.
    // mLightSpace is used in the shader to access the shadow map texture, and has the model
    // matrix baked in.

    // The model matrix below is in fact inverted to get the view matrix and passed to the
    // shader as 'viewFromWorldMatrix', and is used in the VSM case to compute the depth metric.
    // (see surface_depth_main.fs). Note that in the case of VSM, 'b' below is identity.
    mCamera->setModelMatrix(FCamera::rigidTransformInverse(highPrecisionMultiplyd(Mv, b)));
    mCamera->setCustomProjection(mat4(Mn * F * WLMp), znear, zfar);

    // for the debug camera, we need to undo the world origin
    mDebugCamera->setCustomProjection(mat4(S * b * camera.worldTransform), znear, zfar);

    mHasVisibleShadows = true;

    return shaderParameters;
}

ShadowMap::ShaderParameters ShadowMap::updatePunctual(
        mat4f const& Mv, float const outerConeAngle, float const nearPlane, float const farPlane,
        const ShadowMapInfo& shadowMapInfo, const FLightManager::ShadowParams& params) noexcept {
    const mat4f Mp = mat4f::perspective(
            outerConeAngle * f::RAD_TO_DEG * 2.0f, 1.0f, nearPlane, farPlane);

    assert_invariant(shadowMapInfo.textureDimension == mOptions->mapSize);

    // Final shadow transform
    const mat4f S = highPrecisionMultiply(Mp, Mv);
    const auto [Mt, Mn] = getTextureCoordsMapping(shadowMapInfo, getViewport());
    const mat4f St = highPrecisionMultiply(Mt, S);

    // TODO: focus projection
    //      1) focus on the casters
    //      2) additionally focus that on intersection of view & receivers
    //  Alternatively,
    //      Project receivers, casters and view onto near plane,
    //      compute intersection of that which gives the l,r,t,b planes

    // For calculating the spotlight normal bias, we need the texel size in world space at the
    // sample location. Using Thales's theorem, we find:
    //      texelSize(zInLightSpace) = zInLightSpace * texelSizeOnTheNearPlane / near
    //                               = zInLightSpace * texelSizeAtOneMeter
    //                               = zInLightSpace * (2*tan(halfConeAngle)/dimension)
    // Note: this would not work with LISPSM, which warps the texture space.
    ShaderParameters shaderParameters;
    shaderParameters.texelSizeAtOneMeterWs =
            (2.0f * std::tan(outerConeAngle) / float(shadowMapInfo.shadowDimension));
    shaderParameters.lightFromWorldZ = -transpose(Mv)[2]; // negate because camera looks in -Z

    if (!shadowMapInfo.vsm) {
        shaderParameters.lightSpace = St;
    } else {
        shaderParameters.lightSpace = computeVsmLightSpaceMatrix(St, Mv, nearPlane, farPlane);
    }

    shaderParameters.scissorNormalized = getClampToEdgeCoords(shadowMapInfo);

    const float3 direction = -transpose(Mv)[2].xyz;
    const float constantBias = shadowMapInfo.vsm ? 0.0f : params.options.constantBias;
    const mat4f b = mat4f::translation(direction * constantBias);

    // It's important to set the light camera's model matrix separately from its projection, so that
    // the cameraPosition uniform gets set correctly.
    // mLightSpace is used in the shader to access the shadow map texture, and has the model matrix
    // baked in.

    // The model matrix below is in fact inverted to get the view matrix and passed to the
    // shader as 'viewFromWorldMatrix', and is used in the VSM case to compute the depth metric.
    // (see surface_depth_main.fs). Note that in the case of VSM, 'b' below is identity.
    mCamera->setModelMatrix(FCamera::rigidTransformInverse(highPrecisionMultiplyd(Mv, b)));
    mCamera->setCustomProjection(highPrecisionMultiplyd(Mn, Mp), nearPlane, farPlane);

    return shaderParameters;
}

ShadowMap::ShaderParameters ShadowMap::updateSpot(FEngine& engine,
        const FScene::LightSoa& lightData, size_t index,
        CameraInfo const&,
        const ShadowMapInfo& shadowMapInfo,
        FScene const& scene, SceneInfo sceneInfo) noexcept {

    auto& lcm = engine.getLightManager();
    auto position   = lightData.elementAt<FScene::POSITION_RADIUS>(index).xyz;
    auto direction  = lightData.elementAt<FScene::DIRECTION>(index);
    auto radius     = lightData.elementAt<FScene::POSITION_RADIUS>(index).w;
    auto li         = lightData.elementAt<FScene::LIGHT_INSTANCE>(index);
    const FLightManager::ShadowParams& params = lcm.getShadowParams(li);
    const mat4f Mv = getDirectionalLightViewMatrix(direction, { 0, 1, 0 }, position);

    // We only keep this for reference. updateSceneInfoSpot() is quite expensive on large scenes
    // currently, and only needed to find a near/far. Instead, we just use a small near and the
    // radius as far.
    // TODO: Another potential solution would be to visit only the part of the scene that's visible
    //       by the light -- which should be much smaller.
    if constexpr (false) {
        // find decent near/far
        updateSceneInfoSpot(Mv, scene, sceneInfo);
    } else {
        sceneInfo.lsCastersNearFar = { -0.01f, -radius };
    }

    // if the scene was empty, near > far
    mHasVisibleShadows = -sceneInfo.lsCastersNearFar[0] < -sceneInfo.lsCastersNearFar[1];
    if (!mHasVisibleShadows) {
        return {};
    }

    // FIXME: we need a configuration for minimum near plane (for now hardcoded to 1cm)
    float const nearPlane = std::max(0.01f, -sceneInfo.lsCastersNearFar[0]);
    float const farPlane  = std::min(radius, -sceneInfo.lsCastersNearFar[1]);

    auto outerConeAngle = lcm.getSpotLightOuterCone(li);
    return updatePunctual(Mv, outerConeAngle, nearPlane, farPlane, shadowMapInfo, params);
}

ShadowMap::ShaderParameters ShadowMap::updatePoint(FEngine& engine,
        const FScene::LightSoa& lightData, size_t const index, CameraInfo const&,
        const ShadowMapInfo& shadowMapInfo, FScene const& scene, uint8_t face) noexcept {

    // check if this shadow map has anything to render
    mHasVisibleShadows = false;
    FScene::RenderableSoa const& UTILS_RESTRICT soa = scene.getRenderableData();
    auto const* const UTILS_RESTRICT visibleMasks = soa.data<FScene::VISIBLE_MASK>();
    size_t const c = soa.size();
    for (size_t i = 0; i < c; i++) {
        if (visibleMasks[i] & VISIBLE_DYN_SHADOW_RENDERABLE) {
            mHasVisibleShadows = true;
            break;
        }
    }
    if (!mHasVisibleShadows) {
        return {};
    }

    auto& lcm = engine.getLightManager();
    auto position   = lightData.elementAt<FScene::POSITION_RADIUS>(index).xyz;
    auto radius     = lightData.elementAt<FScene::POSITION_RADIUS>(index).w;
    auto li         = lightData.elementAt<FScene::LIGHT_INSTANCE>(index);
    const FLightManager::ShadowParams& params = lcm.getShadowParams(li);
    const mat4f Mv = getPointLightViewMatrix(TextureCubemapFace(face), position);
    return updatePunctual(Mv, 45.0f * f::DEG_TO_RAD, 0.01f, radius, shadowMapInfo, params);
}

ShadowMap::DirectionalShadowBounds ShadowMap::computeDirectionalShadowBounds(
        FEngine& engine,
        float3 const direction,
        FLightManager::ShadowParams params,
        CameraInfo const& camera,
        SceneInfo const& sceneInfo,
        bool useDepthClamp) noexcept {

    // we use aligned_storage<> here to avoid the default initialization of std::array<>
    using Storage = std::aligned_storage<sizeof(FrustumBoxIntersection)>::type;
    Storage localStorage; // NOLINT(cppcoreguidelines-pro-type-member-init)
    FrustumBoxIntersection& lsClippedShadowVolume =
            reinterpret_cast<FrustumBoxIntersection&>(localStorage);

    if (sceneInfo.lsReceiversNearFar[0] <= sceneInfo.lsReceiversNearFar[1]) {
        // no visible receivers
        return {};
    }

    if (sceneInfo.lsCastersNearFar[0] <= sceneInfo.lsCastersNearFar[1]) {
        // no casters
        return {};
    }

    // early exit if we don't have shadow receivers or casters
    Aabb const& wsShadowCastersVolume   = sceneInfo.wsShadowCastersVolume;
    Aabb const& wsShadowReceiversVolume = sceneInfo.wsShadowReceiversVolume;
    if (wsShadowCastersVolume.isEmpty() || wsShadowReceiversVolume.isEmpty()) {
        return {};
    }

#ifndef NDEBUG
    // LISPSM debugging for directional light (works because we only have one)
    const float dz = camera.zf - camera.zn;
    float& dzn = engine.debug.shadowmap.dzn;
    float& dzf = engine.debug.shadowmap.dzf;
    if (dzn < 0)    dzn = std::max(0.0f, params.options.shadowNearHint - camera.zn) / dz;
    else            params.options.shadowNearHint = dzn * dz - camera.zn;
    if (dzf > 0)    dzf =-std::max(0.0f, camera.zf - params.options.shadowFarHint) / dz;
    else            params.options.shadowFarHint = dzf * dz + camera.zf;
#endif

    /*
     * Compute the light's model matrix
     */

    // We compute the directional light's model matrix using the origin's as the light position.
    // The choice of the light's origin initially doesn't matter for a directional light.
    // This will be adjusted later because of how we compute the depth metric for VSM.
    mat4f const MvAtOrigin = getDirectionalLightViewMatrix(direction,
            normalize(camera.worldTransform[0].xyz));


    Aabb lsLightFrustumBounds = computeLightFrustumBounds(
            MvAtOrigin, wsShadowReceiversVolume, wsShadowCastersVolume,
            sceneInfo,
            params.options.stable,
            engine.debug.shadowmap.focus_shadowcasters,
            engine.debug.shadowmap.far_uses_shadowcasters);

    // handles NaNs
    if (UTILS_UNLIKELY(!((lsLightFrustumBounds.min.x < lsLightFrustumBounds.max.x) &&
                         (lsLightFrustumBounds.min.y < lsLightFrustumBounds.max.y)))) {
        return {};
    }

    // Compute the intersection of the view volume with the intersection of receivers and casters
    // in light space. This returns a set of points on the convex-hull of the intersection.
    mat4f const projection = camera.cullingProjection *
            (camera.view * FCamera::rigidTransformInverse(MvAtOrigin));
    size_t const vertexCount = intersectFrustumWithBox(lsClippedShadowVolume,
            projection, lsLightFrustumBounds);

    if (UTILS_UNLIKELY(vertexCount < 4)) {
        return {};
    }

    /*
     * Adjust the scene's zmax (i.e. Near plane) and zmin (i.e. Far plane) in light space.
     * (near/far correspond to max/min because the light looks down the -z axis).
     *
     *   - For the Far plane, the light frustum is already set to the closest of receivers and
     *     casters (i.e.: shadow casters behind the last receivers can't cast any shadows).
     *     But it can be moved closer up to the farthest point in the camera frustum & light
     *     frustum intersection.
     *
     *  - The Near plane is already set to the shadow casters max z (i.e. closest to the light)
     *
     *  If "depth clamp" is supported, we can further tighten the near plane to the
     *  shadow receiver (i.e. to the closest point of camera frustum & light frustum intersection).
     *
     *  Note: L has no influence here, since we're only interested in z values
     *        (L is a rotation around z)
     */

    float lsClippedShadowVolumeFarthest = std::numeric_limits<float>::max();
    float lsClippedShadowVolumeNearest = std::numeric_limits<float>::lowest();
    for (size_t i = 0; i < vertexCount; ++i) {
        float3 const v = lsClippedShadowVolume[i];
        lsClippedShadowVolumeFarthest = std::min(lsClippedShadowVolumeFarthest, v.z);
        lsClippedShadowVolumeNearest = std::max(lsClippedShadowVolumeNearest, v.z);
    }

    lsLightFrustumBounds.min.z =
            std::max(lsLightFrustumBounds.min.z, lsClippedShadowVolumeFarthest);

    if (useDepthClamp) {
        lsLightFrustumBounds.max.z =
                std::min(lsLightFrustumBounds.max.z, lsClippedShadowVolumeNearest);
    }

    if (engine.debug.shadowmap.far_uses_shadowcasters) {
        // far: closest of the farthest shadow casters and receivers
        lsLightFrustumBounds.min.z =
                std::max(lsLightFrustumBounds.min.z, sceneInfo.lsCastersNearFar[1]);
    }

    // near / far planes are specified relative to the direction the eye is looking at
    // i.e. the -z axis (see: ortho)
    const float znear = 0.0f;
    const float zfar = lsLightFrustumBounds.max.z - lsLightFrustumBounds.min.z;
    // if znear >= zfar, it means we don't have any shadow caster in front of a shadow receiver
    if (UTILS_UNLIKELY(!(znear < zfar))) { // handles NaNs
        return {};
    }

    // update lsLightFrustum from MvAtOrigin to Mv
    for (auto& v : lsClippedShadowVolume) {
        v.z -= lsLightFrustumBounds.max.z;
    }

    // Now that we know the znear (-lsLightFrustumBounds.max.z), adjust the light's position such
    // that znear = 0, this is only needed for VSM, but doesn't hurt PCF.
    const mat4f Mv = getDirectionalLightViewMatrix(direction, normalize(camera.worldTransform[0].xyz),
            direction * -lsLightFrustumBounds.max.z);

    return { Mv, znear, zfar, lsClippedShadowVolume, vertexCount, true };
}

mat4f ShadowMap::applyLISPSM(mat4f& Wp,
        CameraInfo const& camera, FLightManager::ShadowParams const& params,
        mat4f const& LMp,
        mat4f const& Mv,
        mat4f const& LMpMv,
        FrustumBoxIntersection const& lsShadowVolume, size_t vertexCount,
        float3 const& dir) {

    const float LoV = dot(camera.getForwardVector(), dir);
    const float sinLV = std::sqrt(std::max(0.0f, 1.0f - LoV * LoV));

    // Virtual near plane -- the default is 1 m, can be changed by the user.
    // The virtual near plane prevents too much resolution to be wasted in the area near the eye
    // where shadows might not be visible (e.g. a character standing won't see shadows at her feet).
    const float dzn = std::max(0.0f, params.options.shadowNearHint - camera.zn);
    const float dzf = std::max(0.0f, camera.zf - params.options.shadowFarHint);

    // near/far plane's distance from the eye in view space of the shadow receiver volume.

    float2 const znf = -computeNearFar(
            camera.view * FCamera::rigidTransformInverse(Mv),
            lsShadowVolume.data(), vertexCount);

    const float zn = std::max(camera.zn, znf[0]); // near plane distance from the eye
    const float zf = std::min(camera.zf, znf[1]); // far plane distance from the eye

    // Compute n and f, the near and far planes coordinates of Wp (warp space).
    // It's found by looking down the Y axis in light space (i.e. -Z axis of Wp,
    // i.e. the axis orthogonal to the light direction) and taking the min/max
    // of the shadow receivers' volume.
    // Note: znear/zfar encoded in Mp has no influence here (b/c we're interested only by the y-axis)
    const float2 nf = computeNearFarOfWarpSpace(LMp, lsShadowVolume.data(), vertexCount);
    const float n = nf[0];              // near plane coordinate of Mp (light space)
    const float f = nf[1];              // far plane coordinate of Mp (light space)
    const float d = std::abs(f - n);    // Wp's depth-range d (abs necessary because we're dealing with z-coordinates, not distances)

    // The simplification below is correct only for directional lights
    const float z0 = zn;                // for directional lights, z0 = zn
    const float z1 = z0 + d * sinLV;    // btw, note that z1 doesn't depend on zf

    mat4f W;
    // see nopt1 below for an explanation about this test
    // sinLV is positive since it comes from a square-root
    constexpr float epsilon = 0.02f; // very roughly 1 degree
    if (f > n && sinLV > epsilon && 3.0f * (dzn / (zf - zn)) < 2.0f) {
        // nopt is the optimal near plane distance of Wp (i.e. distance from P).

        // virtual near and far planes
        const float vz0 = std::max(0.0f, std::max(std::max(zn, camera.zn + dzn), z0));
        const float vz1 = std::max(0.0f, std::min(std::min(zf, camera.zf - dzf), z1));

        // in the general case, nopt is computed as:
        const float nopt0 = (1.0f / sinLV) * (z0 + std::sqrt(vz0 * vz1));

        // However, if dzn becomes too large, the max error doesn't happen in the depth range,
        // and the equation below should be used instead. If dzn reaches 2/3 of the depth range
        // zf-zn, nopt becomes infinite, and we must revert to an ortho projection.
        const float nopt1 = dzn / (2.0f - 3.0f * (dzn / (zf - zn)));

        // We simply use the max of the two expressions
        const float nopt = std::max(nopt0, nopt1);

        const float3 lsCameraPosition = mat4f::project(LMpMv, camera.getPosition());
        const float3 p = {
                // Another option here is to use lsShadowReceiversCenter.x, which skews less the
                // x-axis. Doesn't seem to make a big difference in the end.
                lsCameraPosition.x,
                n - nopt,
                // Note: various papers suggest using the shadow receiver's center z coordinate in light
                // space, i.e. to center "vertically" on the shadow receiver volume.
                // e.g. (LMpMv * wsShadowReceiversVolume.center()).z
                // However, simply using 0, guarantees to be centered on the light frustum, which itself
                // is built from the shadow receiver and/or casters bounds.
                0,
        };

        const mat4f Wv = mat4f::translation(-p);
        Wp = warpFrustum(nopt, nopt + d);
        W = Wp * Wv;
    }
    return W;
}

mat4f ShadowMap::computeLightRotation(float3 const& lsDirection) noexcept {
    mat4f L;
    // Orient the shadow map in the direction of the view vector by constructing a
    // rotation matrix in light space around the z-axis, that aligns the y-axis with the camera's
    // forward vector (V) -- this gives the wrap direction, vp, for LiSPSM.
    // If the light and view vector are parallel, this rotation becomes
    // meaningless. Just use identity.
    // (LdotV == (Mv*V).z, because L = {0,0,1} in light-space)
    if (UTILS_LIKELY(std::abs(lsDirection.z) < 0.9997f)) { // this is |dot(L, V)|
        const float3 vp{ normalize(lsDirection.xy), 0 }; // wrap direction in light-space
        L[0].xyz = cross(vp, float3{ 0, 0, 1 });
        L[1].xyz = vp;
        L[2].xyz = { 0, 0, 1 };
        L = transpose(L);
    }
    return L;
}

float4 ShadowMap::computeFocusParams(
        mat4f const& LMpMv,
        mat4f const& WLMp,
        FrustumBoxIntersection const& lsShadowVolume, size_t vertexCount,
        CameraInfo const& camera,
        float shadowFar, bool stable) noexcept {
    float2 s, o;
    if (stable) {
        // In stable mode, the light frustum size must be fixed, so we choose the
        // whole view frustum.
        // We simply take the view volume bounding sphere, but we calculate it
        // in view space, so that it's perfectly stable.

        auto getViewVolumeBoundingSphere = [&]() {
            if (shadowFar > 0) {
                float4 const wsViewVolumeBoundingSphere = { camera.getPosition(), shadowFar };
                return wsViewVolumeBoundingSphere;
            } else {
                mat4f const viewFromClip = inverse(camera.cullingProjection);
                Corners const wsFrustumVertices = computeFrustumCorners(viewFromClip);
                float4 const wsViewVolumeBoundingSphere =
                        computeBoundingSphere(wsFrustumVertices.vertices, 8);
                return wsViewVolumeBoundingSphere;
            }
        };

        float4 const wsViewVolumeBoundingSphere = getViewVolumeBoundingSphere();
        s = 1.0f / wsViewVolumeBoundingSphere.w;
        o = mat4f::project(LMpMv * camera.model, wsViewVolumeBoundingSphere.xyz).xy;
        o = -s * o;
    } else {
        Aabb const bounds = compute2DBounds(WLMp, lsShadowVolume.data(), vertexCount);
        assert_invariant(bounds.min.x < bounds.max.x);
        assert_invariant(bounds.min.y < bounds.max.y);

        s = 2.0f / float2(bounds.max.xy - bounds.min.xy);
        o = float2(bounds.max.xy + bounds.min.xy) * 0.5f;
        o = -s * o;
    }
    return { s, o };
}

// Apply these remapping in double to maintain a high precision for the depth axis
ShadowMap::TextureCoordsMapping ShadowMap::getTextureCoordsMapping(ShadowMapInfo const& info,
        backend::Viewport const& viewport) noexcept {
    // remapping from NDC to texture coordinates (i.e. [-1,1] -> [0, 1])
    // ([1, 0] for depth mapping)
    constexpr mat4f Mt{
            mat4f::row_major_init{
                    0.5f, 0.0f,  0.0f, 0.5f,
                    0.0f, 0.5f,  0.0f, 0.5f,
                    0.0f, 0.0f, -0.5f, 0.5f,
                    0.0f, 0.0f,  0.0f, 1.0f
            }};

    constexpr mat4f MtInverse = inverse(Mt);

    // apply the viewport transform
    const float2 o = float2{ viewport.left,  viewport.bottom } / float(info.atlasDimension);
    const float2 s = float2{ viewport.width, viewport.height } / float(info.atlasDimension);
    const mat4f Mv{
            mat4f::row_major_init{
                     s.x, 0.0f, 0.0f, o.x,
                    0.0f,  s.y, 0.0f, o.y,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f
            }};

    // this is equivalent to call uvToRenderTargetUV() in the shader in each texture access
    const mat4f Mf = info.textureSpaceFlipped ? mat4f{
            mat4f::row_major_init{
                    1.0f,  0.0f, 0.0f, 0.0f,
                    0.0f, -1.0f, 0.0f, 1.0f,
                    0.0f,  0.0f, 1.0f, 0.0f,
                    0.0f,  0.0f, 0.0f, 1.0f
            }} : mat4f{};

    // Compute shadow-map texture access and viewport transform
    return { Mf * (Mv * Mt), MtInverse * (Mv * Mt) };
}

mat4f ShadowMap::computeVsmLightSpaceMatrix(const mat4f& lightSpacePcf,
        const mat4f& Mv, float const znear, float const zfar) noexcept {
    // The lightSpacePcf matrix transforms coordinates from world space into (u, v, z) coordinates,
    // where (u, v) are used to access the shadow map, and z is the (non-linear) PCF comparison
    // value [0, 1].
    // For VSM, we want to leave the z coordinate in linear light-space, normalized between [0, 1],
    //  i.e. remapping [near, far] to [0, 1].
    // When sampling a VSM shadow map, the shader follows suit, and doesn't divide by w for the z
    // coordinate. See surface_shadowing.fs.
    // compute z' = -(Mv * position).z / (far - near) - (near / (far - near))
    const float scale =   1.0f / (zfar - znear);
    mat4f lightSpaceVsm{ lightSpacePcf };
    lightSpaceVsm[0].z =  -Mv[0].z * scale;
    lightSpaceVsm[1].z =  -Mv[1].z * scale;
    lightSpaceVsm[2].z =  -Mv[2].z * scale;
    lightSpaceVsm[3].z = (-Mv[3].z - znear) * scale;
    return lightSpaceVsm;
}

// This construct a frustum (similar to glFrustum or frustum), except
// it looks towards the +y axis, and assumes -1,1 for the left/right and bottom/top planes.
mat4f ShadowMap::warpFrustum(float const n, float const f) noexcept {
    assert_invariant(f > n);
    const float d = 1 / (f - n);
    const float A = (f + n) * d;
    const float B = -2 * n * f * d;
    const mat4f Wp(mat4f::row_major_init{
            n,    0.0f, 0.0f, 0.0f,
            0.0f, A,    0.0f, B,
            0.0f, 0.0f, n,    0.0f,
            0.0f, 1.0f, 0.0f, 0.0f
    });
    return Wp;
}

mat4f ShadowMap::directionalLightFrustum(float const near, float const far) noexcept {
    const float d = far - near;
    mat4f m;
    m[2][2] = -2 / d;
    m[3][2] = -(far + near)  / d;
    return m;
}

float2 ShadowMap::computeNearFar(const mat4f& view,
        float3 const* wsVertices, size_t const count) noexcept {
    float2 nearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
    for (size_t i = 0; i < count; i++) {
        // we're on the z axis in light space (looking down to -z)
        float const c = mat4f::project(view, wsVertices[i]).z;
        nearFar.x = std::max(nearFar.x, c);  // near
        nearFar.y = std::min(nearFar.y, c);  // far
    }
    return nearFar;
}

float2 ShadowMap::computeNearFarOfWarpSpace(mat4f const& lightView,
        float3 const* wsVertices, size_t const count) noexcept {
    float2 nearFar = { std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest() };
    #pragma nounroll
    for (size_t i = 0; i < count; i++) {
        // we're on the y-axis in light space (looking down to +y)
        float const c = mat4f::project(lightView, wsVertices[i]).y;
        nearFar.x = std::min(nearFar.x, c);
        nearFar.y = std::max(nearFar.y, c);
    }
    return nearFar;
}

float4 ShadowMap::computeBoundingSphere(float3 const* vertices, size_t count) noexcept {
    float4 s{};
    for (size_t i = 0; i < count; i++) {
        s.xyz += vertices[i];
    }
    s.xyz *= 1.0f / float(count);
    for (size_t i = 0; i < count; i++) {
        s.w = std::max(s.w, length2(vertices[i] - s.xyz));
    }
    s.w = std::sqrt(s.w);
    return s;
}

Aabb ShadowMap::compute2DBounds(const mat4f& lightView,
        float3 const* wsVertices, size_t const count) noexcept {
    Aabb bounds{};
    Slice<float3> const vertices{ wsVertices, count };
    for (auto const& vertice : vertices) {
        const float3 v = mat4f::project(lightView, vertice);
        bounds.min.xy = min(bounds.min.xy, v.xy);
        bounds.max.xy = max(bounds.max.xy, v.xy);
    }
    return bounds;
}

Aabb ShadowMap::compute2DBounds(const mat4f& lightView,
        Aabb const& volume) noexcept {
    Aabb bounds{};
    auto const& corners = volume.getCorners();
    for (auto const& corner : corners) {
        const float3 v = mat4f::project(lightView, corner);
        bounds.min = min(bounds.min, v);
        bounds.max = max(bounds.max, v);
    }
    return bounds;
}

ShadowMap::Corners ShadowMap::computeFrustumCorners(
        mat4f const& projectionInverse) noexcept {

    Corners out;

    // compute view frustum in world space (from its NDC)
    // matrix to convert: ndc -> camera -> world
    float const near = -1.0f;
    float const far  =  1.0f;

    Corners const csViewFrustumCorners = {
            .vertices = {
                    { -1, -1, far  },
                    {  1, -1, far  },
                    { -1,  1, far  },
                    {  1,  1, far  },
                    { -1, -1, near },
                    {  1, -1, near },
                    { -1,  1, near },
                    {  1,  1, near },
            }
    };

    for (size_t i = 0; i < csViewFrustumCorners.size(); i++) {
        out[i] = mat4f::project(projectionInverse, csViewFrustumCorners[i]);
    }

    return out;
}

Aabb ShadowMap::computeLightFrustumBounds(mat4f const& lightView,
        Aabb const& wsShadowReceiversVolume, Aabb const& wsShadowCastersVolume,
        SceneInfo const& sceneInfo,
        bool const stable, bool const focusShadowCasters, bool const farUsesShadowCasters) noexcept {
    Aabb lsLightFrustumBounds{};

    float const receiversFar = sceneInfo.lsReceiversNearFar[1];
    float const castersNear = sceneInfo.lsCastersNearFar[0];
    float const castersFar = sceneInfo.lsCastersNearFar[1];

    Aabb const lsReceiversBounds = compute2DBounds(lightView, wsShadowReceiversVolume);
    // far plane from receivers
    lsLightFrustumBounds.min = { lsReceiversBounds.min.xy, receiversFar };
    // near plane from casters
    lsLightFrustumBounds.max = { lsReceiversBounds.max.xy, castersNear };

    if (UTILS_LIKELY(!stable && focusShadowCasters)) {
        // For directional lights, we further constraint the light frustum to the
        // intersection of the shadow casters & shadow receivers in light-space.
        // ** This relies on the 1-texel shadow map border **
        Aabb const lsCastersBounds = compute2DBounds(lightView, wsShadowCastersVolume);
        // intersection of in light space of casters and receiver
        lsLightFrustumBounds.min.xy = max(lsLightFrustumBounds.min.xy, lsCastersBounds.min.xy);
        lsLightFrustumBounds.max.xy = min(lsLightFrustumBounds.max.xy, lsCastersBounds.max.xy);
    }

    if (farUsesShadowCasters) {
        // far: closest of the farthest shadow casters and receivers
        lsLightFrustumBounds.min.z = max(lsLightFrustumBounds.min.z, castersFar);
    }
    return lsLightFrustumBounds;
}

void ShadowMap::snapLightFrustum(float2& s, float2& o,
        double2 const lsRef, int2 const resolution) noexcept {

    auto proj2 = [](mat4 m, double2 const v) -> double2 {
        double2 p;
        p.x = dot(double2{ m[0].x, m[1].x }, v) + m[3].x;
        p.y = dot(double2{ m[0].y, m[1].y }, v) + m[3].y;
        return p;
    };

    auto fract = [](auto v) {
        using namespace std;
        using T = decltype(v);
        return fmod(v, T{1});
    };

    const mat4 F(mat4::row_major_init {
            s.x, 0.0, 0.0, o.x,
            0.0, s.y, 0.0, o.y,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0,
    });

    // The (resolution * 0.5) comes from Mv having a NDC in the range -1,1 (so a range of 2).
    // Another (resolution * 0.5) is there to snap on even texels, which helps with debugging

    // This offsets the texture coordinates, so it has a fixed offset w.r.t the world
    // F * Mv * ref

    double2 const lsFocusedOrigin = proj2(F, lsRef);
    double2 const d = fract(lsFocusedOrigin * (resolution * 0.25)) / (resolution * 0.25);

    // adjust offset
    o -= d;
}

size_t ShadowMap::intersectFrustumWithBox(
        FrustumBoxIntersection& UTILS_RESTRICT outVertices,
        mat4f const& UTILS_RESTRICT projection,
        Aabb const& UTILS_RESTRICT box)
{
    Frustum const frustum{ projection };
    Corners const frustumVertices{ computeFrustumCorners(inverse(projection)) };

    constexpr const float EPSILON = 1.0f / 8192.0f; // ~0.012 mm
    size_t vertexCount = 0;

    /*
     * Clip the world-space view volume (frustum) to the world-space scene volume (AABB),
     * the result is guaranteed to be a convex-hull and is returned as an array of point.
     *
     * Algorithm:
     * a) keep the view frustum vertices that are inside the scene's AABB
     * b) keep the scene's AABB that are inside the view frustum
     * c) keep intersection of AABB edges with view frustum planes
     * d) keep intersection of view frustum edges with AABB planes
     *
     */

    // world-space scene volume
    const Aabb::Corners boxCorners = box.getCorners();

    // a) Keep the frustum's vertices that are known to be inside the scene's box
    UTILS_NOUNROLL
    for (size_t i = 0; i < 8; i++) {
        float3 const p = frustumVertices[i];
        outVertices[vertexCount] = p;
        if ((p.x >= box.min.x && p.x <= box.max.x) &&
            (p.y >= box.min.y && p.y <= box.max.y) &&
            (p.z >= box.min.z && p.z <= box.max.z)) {
            vertexCount++;
        }
    }

    // at this point if we have 8 vertices, we can skip the rest
    if (vertexCount < 8) {
        const size_t frustumVerticesInsideBoxCount = vertexCount;
        float4 const* frustumPlanes = frustum.getNormalizedPlanes();

        // b) add the scene's vertices that are known to be inside the view frustum
        //
        // We need to handle the case where a corner of the box lies exactly on a plane of
        // the frustum. This actually often happens due to fitting light-space
        // We fudge the distance to the plane by a small amount.
        #pragma nounroll
        for (float3 const p : boxCorners) {
            outVertices[vertexCount] = p;
            float const l = dot(frustumPlanes[0].xyz, p) + frustumPlanes[0].w;
            float const b = dot(frustumPlanes[1].xyz, p) + frustumPlanes[1].w;
            float const r = dot(frustumPlanes[2].xyz, p) + frustumPlanes[2].w;
            float const t = dot(frustumPlanes[3].xyz, p) + frustumPlanes[3].w;
            float const f = dot(frustumPlanes[4].xyz, p) + frustumPlanes[4].w;
            float const n = dot(frustumPlanes[5].xyz, p) + frustumPlanes[5].w;
            if ((l <= EPSILON) && (b <= EPSILON) &&
                (r <= EPSILON) && (t <= EPSILON) &&
                (f <= EPSILON) && (n <= EPSILON)) {
                vertexCount++;
            }
        }

        /*
         * At this point it's possible to have zero vertices (i.e.: no vertices of the box
         * are in the frustum and no vertices of the frustum are in the box) but both objects
         * have a non-null intersection, e.g.:
         *
         *         +--------+
         *         |        |
         *    +----|--------|---+
         *    |    |        |   |
         *    |    |        |   |
         *    |    |        |   |
         *    +----|--------|---+
         *         |        |
         *         +--------+
         *
         */

        /*
         * It's not enough here to have all 8 vertices, consider this:
         *
         *                     +
         *                   / |
         *                 /   |
         *    +---------C/--B  |
         *    |       A/    |  |
         *    |       |     |  |
         *    |       A\    |  |
         *    +----------\--B  |
         *                 \   |
         *                   \ |
         *                     +
         *
         * A vertices will be selected by step (a)
         * B vertices will be selected by step (b)
         *
         * if we stop here, the segment (A,B) is inside the intersection of the box and the
         * frustum.  We do need step (c) and (d) to compute the actual intersection C.
         *
         * However, a special case is if all the vertices of the box are inside the frustum.
         */

        const size_t boxVerticesInsideFrustumCount = vertexCount - frustumVerticesInsideBoxCount;
        if (boxVerticesInsideFrustumCount < 8) {
            // c) intersect scene's volume edges with frustum planes
            vertexCount = intersectFrustum(outVertices.data(), vertexCount,
                    boxCorners.vertices, frustumVertices.vertices);

            // d) intersect frustum edges with the scene's volume planes
            vertexCount = intersectFrustum(outVertices.data(), vertexCount,
                    frustumVertices.vertices, boxCorners.vertices);
        } else {
            // by construction vertexCount should be 8 here, but it can be more because
            // step (b) above can classify a point as inside the frustum that isn't quite inside.
            // -> We can't blindly reset vertexCount to 8, we need to remove the "duplicate"
            //    vertices.
            //  It shouldn't cause any major issues later to have "nearly" duplicate vertices.
            assert_invariant(vertexCount >= 8);
        }
    }

    assert_invariant(vertexCount <= outVertices.size());

    return vertexCount;
}

UTILS_NOINLINE
size_t ShadowMap::intersectFrustum(
        float3* UTILS_RESTRICT out,
        size_t vertexCount,
        float3 const* segmentsVertices,
        float3 const* quadsVertices) noexcept {

    #pragma nounroll
    for (const Segment segment : sBoxSegments) {
        const float3 s0{ segmentsVertices[segment.v0] };
        const float3 s1{ segmentsVertices[segment.v1] };
        // each segment should only intersect with 2 quads at most
        size_t const maxVertexCount = vertexCount + 2;
        for (size_t j = 0; j < 6 && vertexCount < maxVertexCount; ++j) {
            const Quad quad = sBoxQuads[j];
            const float3 t0{ quadsVertices[quad.v0] };
            const float3 t1{ quadsVertices[quad.v1] };
            const float3 t2{ quadsVertices[quad.v2] };
            const float3 t3{ quadsVertices[quad.v3] };
            if (intersectSegmentWithPlanarQuad(out[vertexCount], s0, s1, t0, t1, t2, t3)) {
                vertexCount++;
            }
        }
    }
    return vertexCount;
}

UTILS_ALWAYS_INLINE
inline bool ShadowMap::intersectSegmentWithTriangle(float3& UTILS_RESTRICT p,
        float3 s0, float3 s1,
        float3 t0, float3 t1, float3 t2) noexcept {
    // See Real-Time Rendering -- Tomas Akenine-Moller, Eric Haines, Naty Hoffman
    constexpr const float EPSILON = 1.0f / 65536.0f;  // ~1e-5
    const auto e1 = t1 - t0;
    const auto e2 = t2 - t0;
    const auto d = s1 - s0;
    const auto q = cross(d, e2);
    const auto a = dot(e1, q);
    if (UTILS_UNLIKELY(std::abs(a) < EPSILON)) {
        // degenerate triangle
        return false;
    }
    const auto s = s0 - t0;
    const auto u = dot(s, q) * sign(a);
    const auto r = cross(s, e1);
    const auto v = dot(d, r) * sign(a);
    if (u < 0 || v < 0 || u + v > std::abs(a)) {
        // the ray doesn't intersect within the triangle
        return false;
    }
    const auto t = dot(e2, r) * sign(a);
    if (t < 0 || t > std::abs(a)) {
        // the intersection isn't on the segment
        return false;
    }

    // compute the intersection point
    //      Alternate computation: from barycentric coordinates on the triangle
    //      const auto w = 1 - (u + v) / std::abs(a);
    //      p = w * t0 + u / std::abs(a) * t1 + v / std::abs(a) * t2;

    p = s0 + d * (t / std::abs(a));
    return true;
}

bool ShadowMap::intersectSegmentWithPlanarQuad(float3& UTILS_RESTRICT p,
        float3 s0, float3 s1, float3 t0, float3 t1, float3 t2, float3 t3) noexcept {
    bool const hit = intersectSegmentWithTriangle(p, s0, s1, t0, t1, t2) ||
                     intersectSegmentWithTriangle(p, s0, s1, t0, t2, t3);
    return hit;
}

float ShadowMap::texelSizeWorldSpace(const mat3f& worldToShadowTexture,
        uint16_t shadowDimension) noexcept {
    // The Jacobian of the transformation from texture-to-world is the matrix itself for
    // orthographic projections. We just need to inverse worldToShadowTexture,
    // which is guaranteed to be orthographic.
    // The two first columns give us how a texel maps in world-space.
    const float ures = 1.0f / float(shadowDimension);
    const float vres = 1.0f / float(shadowDimension);
    const mat3f shadowTextureToWorld(inverse(worldToShadowTexture));
    const float3 Jx = shadowTextureToWorld[0];
    const float3 Jy = shadowTextureToWorld[1];
    const float s = std::max(length(Jx) * ures, length(Jy) * vres);
    return s;
}

float ShadowMap::texelSizeWorldSpace(const mat4f& Wp, const mat4f& MbMtF,
        uint16_t shadowDimension) noexcept {
    // Here we compute the Jacobian of inverse(MbMtF * Wp).
    // The expression below has been computed with Mathematica. However, it's not very hard,
    // albeit error-prone, to do it by hand because MbMtF is a linear transform.
    // So we really only need to calculate the Jacobian of inverse(Wp) at inverse(MbMtF).
    //
    // Because we're only interested in the length of the columns of the Jacobian, we can use
    // Mb * Mt * F * Wp instead of the full expression Mb * Mt * F * Wp * Wv * L * Mp * Mv,
    // because Wv * L * Mp * Mv is a rigid transform, which doesn't affect the length of
    // the Jacobian's column vectors.

    // The Jacobian is not constant, so we evaluate it in the center of the shadow-map texture.
    // It might be better to do this computation in the vertex shader.
    float3 const p = { 0.5f, 0.5f, 0.0f };

    const float ures = 1.0f / float(shadowDimension);
    const float vres = 1.0f / float(shadowDimension);
    const float dres = 1.0f / 65536.0f;

    constexpr bool JACOBIAN_ESTIMATE = false;
    if constexpr (JACOBIAN_ESTIMATE) {
        // This estimates the Jacobian -- this is a lot heavier. This is mostly for reference
        // and testing.
        const mat4f Si(inverse(MbMtF * Wp));
        const float3 p0 = mat4f::project(Si, p);
        const float3 p1 = mat4f::project(Si, p + float3{ 1, 0, 0 } * ures);
        const float3 p2 = mat4f::project(Si, p + float3{ 0, 1, 0 } * vres);
        const float3 p3 = mat4f::project(Si, p + float3{ 0, 0, 1 } * dres);
        const float3 Jx = p1 - p0;
        const float3 Jy = p2 - p0;
        const float3 UTILS_UNUSED Jz = p3 - p0;
        const float s = std::max(length(Jx), length(Jy));
        return s;
    }

    const float n = Wp[0][0];
    const float A = Wp[1][1];
    const float B = Wp[3][1];
    const float sx = MbMtF[0][0];
    const float sy = MbMtF[1][1];
    const float sz = MbMtF[2][2];
    const float ox = MbMtF[3][0];
    const float oy = MbMtF[3][1];
    const float oz = MbMtF[3][2];

    const float X = p.x - ox;
    const float Y = p.y - oy;
    const float Z = p.z - oz;

    const float dz = A * sy - Y;
    const float nsxsz = n * sx * sz;
    const float j = -(B * sy) / (nsxsz * dz * dz);
    const mat3f J(mat3f::row_major_init{
            j * dz * sz,    j * X * sz,     0.0f,
            0.0f,           j * nsxsz,      0.0f,
            0.0f,           j * Z * sx,     j * dz * sx
    });

    float3 const Jx = J[0] * ures;
    float3 const Jy = J[1] * vres;
    UTILS_UNUSED float3 const Jz = J[2] * dres;
    const float s = std::max(length(Jx), length(Jy));
    return s;
}

template<typename Casters, typename Receivers>
void ShadowMap::visitScene(const FScene& scene, uint32_t const visibleLayers,
        Casters casters, Receivers receivers) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    using State = FRenderableManager::Visibility;
    FScene::RenderableSoa const& soa = scene.getRenderableData();
    float3 const* const worldAABBCenter = soa.data<FScene::WORLD_AABB_CENTER>();
    float3 const* const worldAABBExtent = soa.data<FScene::WORLD_AABB_EXTENT>();
    uint8_t const* const layers = soa.data<FScene::LAYERS>();
    State const* const visibility = soa.data<FScene::VISIBILITY_STATE>();
    auto const* const visibleMasks = soa.data<FScene::VISIBLE_MASK>();
    size_t const c = soa.size();
    for (size_t i = 0; i < c; i++) {
        if (layers[i] & visibleLayers) {
            const Aabb aabb{ worldAABBCenter[i] - worldAABBExtent[i],
                             worldAABBCenter[i] + worldAABBExtent[i] };
            if (visibility[i].castShadows) {
                casters(aabb, visibleMasks[i]);
            }
            if (visibility[i].receiveShadows) {
                receivers(aabb, visibleMasks[i]);
            }
        }
    }
}

ShadowMap::SceneInfo::SceneInfo(
        FScene const& scene, uint8_t const visibleLayers) noexcept
        : visibleLayers(visibleLayers) {

    // the code below only works with affine transforms
    // Filament's API requires viewMatrix to be rigid (and thus affine).

    // We assume the light is at the origin to compute the SceneInfo. This is consumed later by
    // computeShadowCameraDirectional() which takes this into account.

    // Compute scene bounds in world space, as well as the light-space and view-space near/far planes
    wsShadowCastersVolume = {};
    wsShadowReceiversVolume = {};
    visitScene(scene, visibleLayers,
            [&](Aabb caster, Culler::result_type) {
                wsShadowCastersVolume.min = min(wsShadowCastersVolume.min, caster.min);
                wsShadowCastersVolume.max = max(wsShadowCastersVolume.max, caster.max);
            },
            [&](Aabb receiver, Culler::result_type) {
                wsShadowReceiversVolume.min = min(wsShadowReceiversVolume.min, receiver.min);
                wsShadowReceiversVolume.max = max(wsShadowReceiversVolume.max, receiver.max);
            }
    );
}

void ShadowMap::updateSceneInfoDirectional(mat4f const& Mv, FScene const& scene,
        SceneInfo& sceneInfo) {

    // the code below only works with affine transforms
    assert_invariant(transpose(Mv)[3] == float4(0, 0, 0, 1));

    sceneInfo.lsCastersNearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
    sceneInfo.lsReceiversNearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
    visitScene(scene, sceneInfo.visibleLayers,
            [&](Aabb caster, Culler::result_type) {
                auto r = Aabb::transform(Mv.upperLeft(), Mv[3].xyz, caster);
                sceneInfo.lsCastersNearFar.x = max(sceneInfo.lsCastersNearFar.x, r.max.z);
                sceneInfo.lsCastersNearFar.y = min(sceneInfo.lsCastersNearFar.y, r.min.z);
            },
            [&](Aabb receiver, Culler::result_type const vis) {
                // account only for objects that are visible by the camera
                auto mask = 1u << VISIBLE_RENDERABLE_BIT;
                if ((vis & mask) == mask) {
                    auto r = Aabb::transform(Mv.upperLeft(), Mv[3].xyz, receiver);
                    sceneInfo.lsReceiversNearFar.x = max(sceneInfo.lsReceiversNearFar.x, r.max.z);
                    sceneInfo.lsReceiversNearFar.y = min(sceneInfo.lsReceiversNearFar.y, r.min.z);
                }
            }
    );
}

void ShadowMap::updateSceneInfoSpot(mat4f const& Mv, FScene const& scene,
        SceneInfo& sceneInfo) {

    // the code below only works with affine transforms
    assert_invariant(transpose(Mv)[3] == float4(0, 0, 0, 1));

    sceneInfo.lsCastersNearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
    // account only for objects that are visible by both the camera and the light
    visitScene(scene, sceneInfo.visibleLayers,
            [&](Aabb caster, Culler::result_type const vis) {
                auto mask = VISIBLE_DYN_SHADOW_RENDERABLE;
                if ((vis & mask) == mask) {
                    auto r = Aabb::transform(Mv.upperLeft(), Mv[3].xyz, caster);
                    sceneInfo.lsCastersNearFar.x = std::max(sceneInfo.lsCastersNearFar.x, r.max.z);  // near
                    sceneInfo.lsCastersNearFar.y = std::min(sceneInfo.lsCastersNearFar.y, r.min.z);  // far
                }
            },
            [&](Aabb receiver, Culler::result_type) {
            }
    );
}
void ShadowMap::setAllocation(uint8_t const layer, backend::Viewport viewport) noexcept {
    mLayer = layer;
    mOffset = { viewport.left, viewport.bottom };
}

backend::Viewport ShadowMap::getViewport() const noexcept {
    // We set a viewport with a 1-texel border for when we index outside the texture.
    // This happens only for directional lights when "focus shadow casters" is used,
    // or when shadowFar is smaller than the camera far.
    // For spot- and point-lights we also use a 1-texel border, so that bilinear filtering
    // can work properly if the shadowmap is in an atlas (and we can't rely on h/w clamp).
    const uint32_t dim = mOptions->mapSize;
    const uint16_t border = 1u;
    return { mOffset.x + border, mOffset.y + border, dim - 2u * border, dim - 2u * border };
}

backend::Viewport ShadowMap::getScissor() const noexcept {
    // We set a viewport with a 1-texel border for when we index outside the texture.
    // This happens only for directional lights when "focus shadow casters" is used,
    // or when shadowFar is smaller than the camera far.
    // For spot- and point-lights we also use a 1-texel border, so that bilinear filtering
    // can work properly if the shadowmap is in an atlas (and we can't rely on h/w clamp), so we
    // don't scissor the border, so it gets filled with correct neighboring texels.
    const uint32_t dim = mOptions->mapSize;
    const uint16_t border = 1u;
    switch (mShadowType) {
        case ShadowType::DIRECTIONAL:
            return { mOffset.x + border, mOffset.y + border, dim - 2u * border, dim - 2u * border };
        case ShadowType::SPOT:
        case ShadowType::POINT:
            return { mOffset.x, mOffset.y, dim, dim };
    }
}

float4 ShadowMap::getClampToEdgeCoords(ShadowMapInfo const& shadowMapInfo) const noexcept {
    float border; // shadowmap border in texels
    switch (mShadowType) {
        case ShadowType::DIRECTIONAL:
            // For directional lights, we need to allow the sampling to reach the border, it
            // happens when "focus shadow casters" is used for instance.
            border = 0.5f;
            break;
        case ShadowType::SPOT:
        case ShadowType::POINT:
            // For spot and point light, this is equal to the viewport. i.e. the valid
            // texels are inside the viewport (w/ 1-texel border), the border will be used
            // for bilinear filtering.
            border = 1.0f;
            break;
    }

    float const texel = 1.0f / float(shadowMapInfo.atlasDimension);
    float const dim = float(mOptions->mapSize);
    float const l = float(mOffset.x) + border;
    float const b = float(mOffset.y) + border;
    float const w = dim - 2.0f * border;
    float const h = dim - 2.0f * border;
    float4 const v = float4{ l, b, l + w, b + h } * texel;
    if (shadowMapInfo.textureSpaceFlipped) {
        // this is equivalent to calling uvToRenderTargetUV() in the shader *after* clamping
        // texture coordinates to this normalized viewport.
        return { v.x, 1.0f - v.w, v.z, 1.0f - v.y };
    }
    return v;
}

// ------------------------------------------------------------------------------------------------

void ShadowMap::prepareCamera(Transaction const& transaction,
        FEngine const& engine, const CameraInfo& cameraInfo) noexcept {
    ShadowMapDescriptorSet::prepareCamera(transaction, engine, cameraInfo);
    ShadowMapDescriptorSet::prepareLodBias(transaction, 0.0f);
}

void ShadowMap::prepareViewport(Transaction const& transaction,
        backend::Viewport const& viewport) noexcept {
    ShadowMapDescriptorSet::prepareViewport(transaction, viewport);
}

void ShadowMap::prepareTime(Transaction const& transaction,
        FEngine const& engine, float4 const& userTime) noexcept {
    ShadowMapDescriptorSet::prepareTime(transaction, engine, userTime);
}

void ShadowMap::prepareShadowMapping(Transaction const& transaction,
        bool const highPrecision) noexcept {
    ShadowMapDescriptorSet::prepareShadowMapping(transaction, highPrecision);
}

ShadowMapDescriptorSet::Transaction ShadowMap::open(DriverApi& driver) noexcept {
    return ShadowMapDescriptorSet::open(driver);
}

void ShadowMap::commit(Transaction& transaction,
        FEngine& engine, DriverApi& driver) const noexcept {
    mPerShadowMapUniforms.commit(transaction, engine, driver);
}

void ShadowMap::bind(DriverApi& driver) const noexcept {
    mPerShadowMapUniforms.bind(driver);
}

} // namespace filament
