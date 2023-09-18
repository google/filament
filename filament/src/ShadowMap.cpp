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

#include "RenderPass.h"

#include "components/LightManager.h"

#include "details/Engine.h"
#include "details/Scene.h"

#include <backend/DriverEnums.h>

#include <utils/debug.h>
#include <utils/Systrace.h>

#include <limits>

using namespace utils;

namespace filament {

using namespace math;
using namespace backend;

// do this only if depth-clamp is available
static constexpr bool USE_DEPTH_CLAMP = false;

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

void ShadowMap::initialize(size_t lightIndex, ShadowType shadowType,
        uint16_t shadowIndex, uint8_t face,
        LightManager::ShadowOptions const* options) {
    mLightIndex = lightIndex;
    mShadowIndex = shadowIndex;
    mOptions = options;
    mShadowType = shadowType;
    mFace = face;
}

mat4f ShadowMap::getDirectionalLightViewMatrix(float3 direction, float3 position) noexcept {
    auto z_axis = direction;
    auto norm_up = float3{ 0, 1, 0 };
    if (UTILS_UNLIKELY(std::abs(dot(z_axis, norm_up)) > 0.999f)) {
        // Fix up vector if we're degenerate (looking straight up, basically)
        norm_up = { norm_up.z, norm_up.x, norm_up.y };
    }
    auto x_axis = normalize(cross(z_axis, norm_up));
    auto y_axis = cross(x_axis, z_axis);
    const mat4f Mm{
            float4{   x_axis, 0 },
            float4{   y_axis, 0 },
            float4{  -z_axis, 0 },
            float4{ position, 1 }};
    return FCamera::rigidTransformInverse(Mm);
}

math::mat4f ShadowMap::getPointLightViewMatrix(backend::TextureCubemapFace face,
        math::float3 position) noexcept {
    float3 direction;
    switch (TextureCubemapFace(face)) {
        case TextureCubemapFace::POSITIVE_X:    direction = {  1,  0,  0 }; break;
        case TextureCubemapFace::NEGATIVE_X:    direction = { -1,  0,  0 }; break;
        case TextureCubemapFace::POSITIVE_Y:    direction = {  0,  1,  0 }; break;
        case TextureCubemapFace::NEGATIVE_Y:    direction = {  0, -1,  0 }; break;
        case TextureCubemapFace::POSITIVE_Z:    direction = {  0,  0,  1 }; break;
        case TextureCubemapFace::NEGATIVE_Z:    direction = {  0,  0, -1 }; break;
    }
    const mat4f Mv = getDirectionalLightViewMatrix(direction, position);
    return Mv;
}

ShadowMap::ShaderParameters ShadowMap::updateDirectional(FEngine& engine,
        FScene::LightSoa const& lightData, size_t index,
        filament::CameraInfo const& camera,
        ShadowMapInfo const& shadowMapInfo,
        SceneInfo const& sceneInfo) noexcept {

    // Note: we keep the polygon offset even with VSM as it seems to help.
    auto& lcm = engine.getLightManager();
    FLightManager::Instance const li = lightData.elementAt<FScene::LIGHT_INSTANCE>(index);
    FLightManager::ShadowParams params = lcm.getShadowParams(li);

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

    const auto direction = params.options.transform * lightData.elementAt<FScene::DIRECTION>(index);

    /*
     * Compute the light's model matrix
     */

    // We compute the directional light's model matrix using the origin's as the light position.
    // The choice of the light's origin initially doesn't matter for a directional light.
    // This will be adjusted later because of how we compute the depth metric for VSM.
    const mat4f MvAtOrigin = ShadowMap::getDirectionalLightViewMatrix(direction);

    const Aabb wsShadowCastersVolume = sceneInfo.wsShadowCastersVolume;
    const Aabb wsShadowReceiversVolume = sceneInfo.wsShadowReceiversVolume;
    if (wsShadowCastersVolume.isEmpty() || wsShadowReceiversVolume.isEmpty()) {
        mHasVisibleShadows = false;
        return {};
    }

    // view frustum vertices in world-space
    float3 wsViewFrustumVertices[8];
    const mat4f worldToClipMatrix = camera.cullingProjection * camera.view;
    const Frustum wsFrustum(worldToClipMatrix);
    computeFrustumCorners(wsViewFrustumVertices, inverse(worldToClipMatrix), sceneInfo.csNearFar);

    // we use aligned_storage<> here to avoid the default initialization of std::array<>
    std::aligned_storage<sizeof(FrustumBoxIntersection)>::type localStorage; // NOLINT(cppcoreguidelines-pro-type-member-init)
    FrustumBoxIntersection& wsClippedShadowReceiverVolume{ reinterpret_cast<FrustumBoxIntersection&>(localStorage) };

    // compute the intersection of the shadow receivers' volume with the view volume
    // in world space. This returns a set of points on the convex-hull of the intersection.
    size_t const vertexCount = intersectFrustumWithBox(wsClippedShadowReceiverVolume,
            wsFrustum, wsViewFrustumVertices, wsShadowReceiversVolume);

    if (UTILS_UNLIKELY(vertexCount < 4)) {
        mHasVisibleShadows = false;
        return {};
    }

    /*
     *  compute scene zmax (i.e. Near plane) and zmin (i.e. Far plane) in light space.
     *  (near/far correspond to max/min because the light looks down the -z axis).
     *  - The Near plane is set to the shadow casters max z (i.e. closest to the light)
     *  - The Far plane is set to the closest of the farthest shadow casters and receivers
     *    i.e.: shadow casters behind the last receivers can't cast any shadows
     *
     *  If "depth clamp" is supported, we can further tighten the near plane to the
     *  shadow receiver.
     *
     *  Note: L has no influence here, since we're only interested in z values
     *        (L is a rotation around z)
     */

    Aabb lsLightFrustumBounds;
    if constexpr (!USE_DEPTH_CLAMP) {
        // near plane from shadow caster volume
        lsLightFrustumBounds.max.z = sceneInfo.lsNearFar[0];
    }
    for (size_t i = 0; i < vertexCount; ++i) {
        // far: figure out the farthest shadow receivers
        float3 const v = mat4f::project(MvAtOrigin, wsClippedShadowReceiverVolume[i]);
        lsLightFrustumBounds.min.z = std::min(lsLightFrustumBounds.min.z, v.z);
        if constexpr (USE_DEPTH_CLAMP) {
            // tighten the shadow receiver volume further
            lsLightFrustumBounds.max.z = std::max(lsLightFrustumBounds.max.z, v.z);
        }
    }
    if (engine.debug.shadowmap.far_uses_shadowcasters) {
        // far: closest of the farthest shadow casters and receivers
        lsLightFrustumBounds.min.z = std::max(lsLightFrustumBounds.min.z, sceneInfo.lsNearFar[1]);
    }

    // Now that we know the znear (-lsLightFrustumBounds.max.z), adjust the light's position such
    // that znear = 0, this is only needed for VSM, but doesn't hurt PCF.
    const mat4f Mv = getDirectionalLightViewMatrix(direction, direction * -lsLightFrustumBounds.max.z);

    // near / far planes are specified relative to the direction the eye is looking at
    // i.e. the -z axis (see: ortho)
    const float znear = 0.0f;
    const float zfar = lsLightFrustumBounds.max.z - lsLightFrustumBounds.min.z;

    // if znear >= zfar, it means we don't have any shadow caster in front of a shadow receiver
    if (UTILS_UNLIKELY(znear >= zfar)) {
        mHasVisibleShadows = false;
        return {};
    }

    float4 viewVolumeBoundingSphere = {};
    if (UTILS_UNLIKELY(params.options.stable)) {
        // In stable mode, the light frustum size must be fixed, so we can choose either the
        // whole view frustum, or the whole scene bounding volume. We simply pick whichever
        // is smaller.

        // in stable mode we simply take the shadow receivers volume
        const float4 shadowReceiverVolumeBoundingSphere = computeBoundingSphere(
                wsShadowReceiversVolume.getCorners().data(), 8);

        // in stable mode we simply take the view volume bounding sphere, but we calculate it
        // in view space, so that it's perfectly stable.
        float3 vertices[8];
        computeFrustumCorners(vertices, inverse(camera.cullingProjection), sceneInfo.csNearFar);
        viewVolumeBoundingSphere = computeBoundingSphere(vertices, 8);

        if (shadowReceiverVolumeBoundingSphere.w < viewVolumeBoundingSphere.w) {

            // When using the shadowReceiver volume, we don't have to use its enclosing sphere
            // because (we assume) the scene volume doesn't change. Seen from the light it only
            // changes when the light moves or rotates, and it is acceptable in that case to have
            // non "stable" shadows (the shadow will never be stable when the light moves).
            //
            // On the other hand, when using the view volume, we must use a sphere because otherwise
            // its projection's bounds in light space change with the camera, leading to unstable
            // shadows with camera movement.

            viewVolumeBoundingSphere.w = 0;
            std::copy_n(wsShadowReceiversVolume.getCorners().data(), 8,
                    wsClippedShadowReceiverVolume.data());
        }
    }

    mHasVisibleShadows = true;

    // We can't use LISPSM in stable mode
    const bool useLispsm = params.options.lispsm && !params.options.stable;

    /*
     * Compute the light's projection matrix
     * (directional/point lights, i.e. projection to use, including znear/zfar clip planes)
     */

    // The light's projection, ortho for directional lights, perspective otherwise
    const mat4f Mp = directionalLightFrustum(znear, zfar);

    const mat4f MpMv(Mp * Mv);

    /*
     * Compute warping (optional, improve quality)
     */

    mat4f LMpMv = MpMv;

    // Compute the LiSPSM warping
    mat4f W, Wp;
    mat4f L; // Rotation matrix in light space
    if (useLispsm) {
        // Orient the shadow map in the direction of the view vector by constructing a
        // rotation matrix in light space around the z-axis, that aligns the y-axis with the camera's
        // forward vector (V) -- this gives the wrap direction, vp, for LiSPSM.
        const float3 wsCameraFwd = camera.getForwardVector();
        const float3 lsCameraFwd = Mv.upperLeft() * wsCameraFwd;
        // If the light and view vector are parallel, this rotation becomes
        // meaningless. Just use identity.
        // (LdotV == (Mv*V).z, because L = {0,0,1} in light-space)
        if (UTILS_LIKELY(std::abs(lsCameraFwd.z) < 0.9997f)) { // this is |dot(L, V)|
            const float3 vp{ normalize(lsCameraFwd.xy), 0 }; // wrap direction in light-space
            L[0].xyz = cross(vp, float3{ 0, 0, 1 });
            L[1].xyz = vp;
            L[2].xyz = { 0, 0, 1 };
            L = transpose(L);
        }

        LMpMv = L * MpMv;

        W = applyLISPSM(Wp, camera, params, LMpMv,
                wsClippedShadowReceiverVolume, vertexCount, direction);
    }

    /*
     * Compute focusing matrix (optional, greatly improves quality)
     */

    // construct the warped light-space
    const mat4f WLMpMv = W * LMpMv;

    // 2) Now we find the x-y bounds of our convex-hull (view volume & shadow receivers)
    //    in light space, so we can "focus" the shadow map to the interesting area.
    //    This is the most important step to increase the quality of the shadow map.
    //
    //   In LiPSM mode, we're using the warped space here.

    float2 s, o;
    if (params.options.stable) {
        if (viewVolumeBoundingSphere.w > 0) {
            s = 1.0f / viewVolumeBoundingSphere.w;
            o = mat4f::project(LMpMv * camera.model, viewVolumeBoundingSphere.xyz).xy;
        } else {
            Aabb const bounds = compute2DBounds(LMpMv,
                    wsClippedShadowReceiverVolume.data(), vertexCount);
            if (UTILS_UNLIKELY((bounds.min.x >= bounds.max.x) || (bounds.min.y >= bounds.max.y))) {
                // this could happen if the only thing visible is a perfectly horizontal or
                // vertical thin line
                mHasVisibleShadows = false;
                return {};
            }
            assert_invariant(bounds.min.x < bounds.max.x);
            assert_invariant(bounds.min.y < bounds.max.y);

            s = 2.0f / float2(bounds.max.xy - bounds.min.xy);
            o = float2(bounds.max.xy + bounds.min.xy) * 0.5f;

            // Quantize the scale in world-space units. This value can be very small because
            // if it wasn't for floating-point imprecision, the scale would be a constant.
            double2 const quantizer = 0.0625;
            s = 1.0 / (ceil(1.0 / (s * quantizer)) * quantizer);
        }
    } else {
        Aabb const bounds = compute2DBounds(WLMpMv,
                wsClippedShadowReceiverVolume.data(), vertexCount);
        lsLightFrustumBounds.min.xy = bounds.min.xy;
        lsLightFrustumBounds.max.xy = bounds.max.xy;
        // For directional lights, we further constraint the light frustum to the
        // intersection of the shadow casters & shadow receivers in light-space.
        // ** This relies on the 1-texel shadow map border **

        if (engine.debug.shadowmap.focus_shadowcasters) {
            intersectWithShadowCasters(&lsLightFrustumBounds, WLMpMv, wsShadowCastersVolume);
        }
        if (UTILS_UNLIKELY((lsLightFrustumBounds.min.x >= lsLightFrustumBounds.max.x) ||
                           (lsLightFrustumBounds.min.y >= lsLightFrustumBounds.max.y))) {
            // this could happen if the only thing visible is a perfectly horizontal or
            // vertical thin line
            mHasVisibleShadows = false;
            return {};
        }
        assert_invariant(lsLightFrustumBounds.min.x < lsLightFrustumBounds.max.x);
        assert_invariant(lsLightFrustumBounds.min.y < lsLightFrustumBounds.max.y);

        s = 2.0f / float2(lsLightFrustumBounds.max.xy - lsLightFrustumBounds.min.xy);
        o = float2(lsLightFrustumBounds.max.xy + lsLightFrustumBounds.min.xy) * 0.5f;

        // TODO: we could quantize `s` here to give some stability when lispsm is disabled,
        //       however, the quantization paramater should probably be user settable.
    }

    // adjust offset for scale
    o = -s * o;

    if (params.options.stable) {
        snapLightFrustum(s, o, LMpMv,
                sceneInfo.wsShadowCastersVolume.center(), shadowMapInfo.shadowDimension);
    }

    const mat4f F(mat4f::row_major_init {
             s.x,  0.0f, 0.0f, o.x,
             0.0f, s.y,  0.0f, o.y,
             0.0f, 0.0f, 1.0f, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f,
    });

    /*
     * Final shadow map transform
     */

    // Final shadow transform
    const mat4f S = F * WLMpMv;

    // Computes St the transform to use in the shader to access the shadow map texture
    // i.e. it transforms a world-space vertex to a texture coordinate in the shadowmap
    const auto [Mt, Mn] = ShadowMap::getTextureCoordsMapping(shadowMapInfo, getViewport());
    const mat4f St = math::highPrecisionMultiply(Mt, S);

    ShadowMap::ShaderParameters shaderParameters;

    // note: in texelSizeWorldSpace() below, we can use Mb * Mt * F * W because
    // L * Mp * Mv is a rigid transform for directional lights, and doesn't matter.
    // if Wp[3][1] is 0, then LISPSM was cancelled.
    if (useLispsm && Wp[3][1] != 0.0f) {
        shaderParameters.texelSizeAtOneMeterWs = texelSizeWorldSpace(Wp, mat4f(Mt * F),
                shadowMapInfo.shadowDimension);
    } else {
        // We know we're using an ortho projection
        shaderParameters.texelSizeAtOneMeterWs = texelSizeWorldSpace(St.upperLeft(),
                shadowMapInfo.shadowDimension);
    }
    if (!shadowMapInfo.vsm) {
        shaderParameters.lightSpace = St;
    } else {
        shaderParameters.lightSpace = computeVsmLightSpaceMatrix(St, Mv, znear, zfar);
    }

    shaderParameters.scissorNormalized = getClampToEdgeCoords(shadowMapInfo);

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
    // (see depth_main.fs). Note that in the case of VSM, 'b' below is identity.
    mCamera->setModelMatrix(FCamera::rigidTransformInverse(math::highPrecisionMultiplyd(Mv, b)));
    mCamera->setCustomProjection(mat4(Mn * F * W * L * Mp), znear, zfar);

    // for the debug camera, we need to undo the world origin
    mDebugCamera->setCustomProjection(mat4(S * b * camera.worldOrigin), znear, zfar);

    return shaderParameters;
}

ShadowMap::ShaderParameters ShadowMap::updatePunctual(
        mat4f const& Mv, float outerConeAngle, float nearPlane, float farPlane,
        const ShadowMapInfo& shadowMapInfo, const FLightManager::ShadowParams& params) noexcept {
    const mat4f Mp = mat4f::perspective(outerConeAngle * f::RAD_TO_DEG * 2.0f, 1.0f, nearPlane, farPlane);

    assert_invariant(shadowMapInfo.textureDimension == mOptions->mapSize);

    // Final shadow transform
    const mat4f S = math::highPrecisionMultiply(Mp, Mv);
    const auto [Mt, Mn] = ShadowMap::getTextureCoordsMapping(shadowMapInfo, getViewport());
    const mat4f St = math::highPrecisionMultiply(Mt, S);

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
    // (see depth_main.fs). Note that in the case of VSM, 'b' below is identity.
    mCamera->setModelMatrix(FCamera::rigidTransformInverse(math::highPrecisionMultiplyd(Mv, b)));
    mCamera->setCustomProjection(math::highPrecisionMultiplyd(Mn, Mp), nearPlane, farPlane);

    return shaderParameters;
}

ShadowMap::ShaderParameters ShadowMap::updateSpot(FEngine& engine,
        const FScene::LightSoa& lightData, size_t index,
        filament::CameraInfo const&,
        const ShadowMapInfo& shadowMapInfo,
        FScene const& scene, SceneInfo sceneInfo) noexcept {

    auto& lcm = engine.getLightManager();
    auto position   = lightData.elementAt<FScene::POSITION_RADIUS>(index).xyz;
    auto direction  = lightData.elementAt<FScene::DIRECTION>(index);
    auto radius     = lightData.elementAt<FScene::POSITION_RADIUS>(index).w;
    auto li         = lightData.elementAt<FScene::LIGHT_INSTANCE>(index);
    const FLightManager::ShadowParams& params = lcm.getShadowParams(li);
    const mat4f Mv = getDirectionalLightViewMatrix(direction, position);

    // We only keep this for reference. updateSceneInfoSpot() is quite expensive on large scenes
    // currently, and only needed to find a near/far. Instead, we just use a small near and the
    // radius as far.
    // TODO: Another potential solution would be to visit only the part of the scene that's visible
    //       by the light -- which should be much smaller.
    if constexpr (false) {
        // find decent near/far
        ShadowMap::updateSceneInfoSpot(Mv, scene, sceneInfo);
    } else {
        sceneInfo.lsNearFar = { -0.01f, -radius };
    }

    // if the scene was empty, near > far
    mHasVisibleShadows = -sceneInfo.lsNearFar[0] < -sceneInfo.lsNearFar[1];
    if (!mHasVisibleShadows) {
        return {};
    }

    // FIXME: we need a configuration for minimum near plane (for now hardcoded to 1cm)
    float const nearPlane = std::max(0.01f, -sceneInfo.lsNearFar[0]);
    float const farPlane  = std::min(radius, -sceneInfo.lsNearFar[1]);

    auto outerConeAngle = lcm.getSpotLightOuterCone(li);
    return updatePunctual(Mv, outerConeAngle, nearPlane, farPlane, shadowMapInfo, params);
}

ShadowMap::ShaderParameters ShadowMap::updatePoint(FEngine& engine,
        const FScene::LightSoa& lightData, size_t index, filament::CameraInfo const&,
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

mat4f ShadowMap::applyLISPSM(mat4f& Wp,
        filament::CameraInfo const& camera, FLightManager::ShadowParams const& params,
        mat4f const& LMpMv,
        FrustumBoxIntersection const& wsShadowReceiversVolume, size_t vertexCount,
        float3 const& dir) {

    const float LoV = dot(camera.getForwardVector(), dir);
    const float sinLV = std::sqrt(std::max(0.0f, 1.0f - LoV * LoV));

    // Virtual near plane -- the default is 1 m, can be changed by the user.
    // The virtual near plane prevents too much resolution to be wasted in the area near the eye
    // where shadows might not be visible (e.g. a character standing won't see shadows at her feet).
    const float dzn = std::max(0.0f, params.options.shadowNearHint - camera.zn);
    const float dzf = std::max(0.0f, camera.zf - params.options.shadowFarHint);

    // near/far plane's distance from the eye in view space of the shadow receiver volume.
    float2 znf = -computeNearFar(camera.view, wsShadowReceiversVolume.data(), vertexCount);
    const float zn = std::max(camera.zn, znf[0]); // near plane distance from the eye
    const float zf = std::min(camera.zf, znf[1]); // far plane distance from the eye

    // compute n and f, the near and far planes coordinates of Wp (warp space).
    // It's found by looking down the Y axis in light space (i.e. -Z axis of Wp,
    // i.e. the axis orthogonal to the light direction) and taking the min/max
    // of the shadow receivers' volume.
    // Note: znear/zfar encoded in Mp has no influence here (b/c we're interested only by the y-axis)
    const float2 nf = computeNearFarOfWarpSpace(LMpMv, wsShadowReceiversVolume.data(), vertexCount);
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
    if (sinLV > epsilon && 3.0f * (dzn / (zf - zn)) < 2.0f) {
        // nopt is the optimal near plane distance of Wp (i.e. distance from P).

        // virtual near and far planes
        const float vz0 = std::max(0.0f, std::max(std::max(zn, camera.zn + dzn), z0));
        const float vz1 = std::max(0.0f, std::min(std::min(zf, camera.zf - dzf), z1));

        // in the general case, nopt is computed as:
        const float nopt0 = (1.0f / sinLV) * (z0 + std::sqrt(vz0 * vz1));

        // however, if dzn becomes too large, the max error doesn't happen in the depth range,
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
                // note: various papers suggest using the shadow receiver's center z coordinate in light
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
    return { Mf * (Mv * Mt), inverse(Mt) * (Mv * Mt) };
}

mat4f ShadowMap::computeVsmLightSpaceMatrix(const mat4f& lightSpacePcf,
        const mat4f& Mv, float znear, float zfar) noexcept {
    // The lightSpacePcf matrix transforms coordinates from world space into (u, v, z) coordinates,
    // where (u, v) are used to access the shadow map, and z is the (non-linear) PCF comparison
    // value [0, 1].
    // For VSM, we want to leave the z coordinate in linear light-space, normalized between [0, 1],
    //  i.e. remapping [near, far] to [0, 1].
    // When sampling a VSM shadow map, the shader follows suit, and doesn't divide by w for the z
    // coordinate. See shadowing.fs.
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
mat4f ShadowMap::warpFrustum(float n, float f) noexcept {
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

mat4f ShadowMap::directionalLightFrustum(float near, float far) noexcept {
    const float d = far - near;
    mat4f m;
    m[2][2] = -2 / d;
    m[3][2] = -(far + near)  / d;
    return m;
}

float2 ShadowMap::computeNearFar(const mat4f& view,
        float3 const* wsVertices, size_t count) noexcept {
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
        float3 const* wsVertices, size_t count) noexcept {
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
    s.xyz *= 1.0f / count;
    for (size_t i = 0; i < count; i++) {
        s.w = std::max(s.w, length2(vertices[i] - s.xyz));
    }
    s.w = std::sqrt(s.w);
    return s;
}

Aabb ShadowMap::compute2DBounds(const mat4f& lightView,
        float3 const* wsVertices, size_t count) noexcept {
    Aabb bounds{};
    // disable vectorization here because vertexCount is <= 64, not worth the increased code size.
    #pragma clang loop vectorize(disable)
    for (size_t i = 0; i < count; ++i) {
        const float3 v = mat4f::project(lightView, wsVertices[i]);
        bounds.min.xy = min(bounds.min.xy, v.xy);
        bounds.max.xy = max(bounds.max.xy, v.xy);
    }
    return bounds;
}

void ShadowMap::intersectWithShadowCasters(Aabb* UTILS_RESTRICT lightFrustum,
        mat4f const& lightView, Aabb const& wsShadowCastersVolume) noexcept {

    // construct the Focus transform (scale + offset)
    const float2 s = 2.0f / float2(lightFrustum->max.xy - lightFrustum->min.xy);
    const float2 o =   -s * float2(lightFrustum->max.xy + lightFrustum->min.xy) * 0.5f;
    const mat4f F(mat4f::row_major_init {
            s.x,  0.0f, 0.0f,    o.x,
            0.0f,  s.y, 0.0f,    o.y,
            0.0f, 0.0f, 1.0f,   0.0f,
            0.0f, 0.0f, 0.0f,   1.0f,
    });
    float3 wsLightFrustumCorners[8];
    const mat4f projection = F * lightView;
    const Frustum wsLightFrustum(projection);
    computeFrustumCorners(wsLightFrustumCorners, inverse(projection));

    // Intersect shadow-caster AABB with current light frustum in world-space:
    //
    // This (obviously) guarantees that the resulting volume is inside the light frustum;
    // when using LiSPSM (or projection lights, i.e. when lightView is a projection), we must
    // first intersect wsShadowCastersVolume with the light's frustum, otherwise we end-up
    // transforming vertices that are "outside" the frustum, and that's forbidden.
    FrustumBoxIntersection wsClippedShadowCasterVolumeVertices;
    size_t const vertexCount = intersectFrustumWithBox(wsClippedShadowCasterVolumeVertices,
            wsLightFrustum, wsLightFrustumCorners, wsShadowCastersVolume);

    // compute shadow-caster bounds in light space
    Aabb const box = compute2DBounds(lightView, wsClippedShadowCasterVolumeVertices.data(), vertexCount);

    // intersect shadow-caster and current light frustum bounds
    lightFrustum->min.xy = max(box.min.xy, lightFrustum->min.xy);
    lightFrustum->max.xy = min(box.max.xy, lightFrustum->max.xy);
}

void ShadowMap::computeFrustumCorners(float3* UTILS_RESTRICT out,
        const mat4f& UTILS_RESTRICT projectionViewInverse, float2 csNearFar) noexcept {

    // compute view frustum in world space (from its NDC)
    // matrix to convert: ndc -> camera -> world
    float near = csNearFar.x;
    float far = csNearFar.y;
    float3 const csViewFrustumCorners[8] = {
            { -1, -1,  far },
            {  1, -1,  far },
            { -1,  1,  far },
            {  1,  1,  far },
            { -1, -1,  near },
            {  1, -1,  near },
            { -1,  1,  near },
            {  1,  1,  near },
    };
    for (float3 const c : csViewFrustumCorners) {
        *out++ = mat4f::project(projectionViewInverse, c);
    }
}

void ShadowMap::snapLightFrustum(float2& s, float2& o,
        mat4f const& Mv, double3 wsSnapCoords, int2 resolution) noexcept {

    auto proj = [](mat4 m, double3 v) -> double3 {
        // for directional light p.w == 1, exactly
        auto p = m * v;
        assert_invariant(p.w == 1.0);
        return p.xyz;
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

    // focused light-space
    mat4 const FMv{ F * Mv };

    // This offsets the texture coordinates, so it has a fixed offset w.r.t the world
    double2 const lsOrigin = proj(FMv, wsSnapCoords).xy;
    double2 const d = (fract(lsOrigin * resolution * 0.5) * 2.0) / resolution;

    // adjust offset
    o -= d;
}

size_t ShadowMap::intersectFrustumWithBox(
        FrustumBoxIntersection& UTILS_RESTRICT outVertices,
        Frustum const& UTILS_RESTRICT wsFrustum,
        float3 const* UTILS_RESTRICT wsFrustumCorners,
        Aabb const& UTILS_RESTRICT wsBox)
{
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
    const Aabb::Corners wsSceneReceiversCorners = wsBox.getCorners();

    // a) Keep the frustum's vertices that are known to be inside the scene's box
    UTILS_NOUNROLL
    for (size_t i = 0; i < 8; i++) {
        float3 const p = wsFrustumCorners[i];
        outVertices[vertexCount] = p;
        if ((p.x >= wsBox.min.x && p.x <= wsBox.max.x) &&
            (p.y >= wsBox.min.y && p.y <= wsBox.max.y) &&
            (p.z >= wsBox.min.z && p.z <= wsBox.max.z)) {
            vertexCount++;
        }
    }

    // at this point if we have 8 vertices, we can skip the rest
    if (vertexCount < 8) {
        const size_t frustumVerticesInsideBoxCount = vertexCount;
        float4 const* wsFrustumPlanes = wsFrustum.getNormalizedPlanes();

        // b) add the scene's vertices that are known to be inside the view frustum
        //
        // We need to handle the case where a corner of the box lies exactly on a plane of
        // the frustum. This actually often happens due to fitting light-space
        // We fudge the distance to the plane by a small amount.
        #pragma nounroll
        for (float3 const p : wsSceneReceiversCorners) {
            outVertices[vertexCount] = p;
            float const l = dot(wsFrustumPlanes[0].xyz, p) + wsFrustumPlanes[0].w;
            float const b = dot(wsFrustumPlanes[1].xyz, p) + wsFrustumPlanes[1].w;
            float const r = dot(wsFrustumPlanes[2].xyz, p) + wsFrustumPlanes[2].w;
            float const t = dot(wsFrustumPlanes[3].xyz, p) + wsFrustumPlanes[3].w;
            float const f = dot(wsFrustumPlanes[4].xyz, p) + wsFrustumPlanes[4].w;
            float const n = dot(wsFrustumPlanes[5].xyz, p) + wsFrustumPlanes[5].w;
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
                    wsSceneReceiversCorners.vertices, wsFrustumCorners);

            // d) intersect frustum edges with the scene's volume planes
            vertexCount = intersectFrustum(outVertices.data(), vertexCount,
                    wsFrustumCorners, wsSceneReceiversCorners.vertices);
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
void ShadowMap::visitScene(const FScene& scene, uint32_t visibleLayers,
        Casters casters, Receivers receivers) noexcept {
    SYSTRACE_CALL();

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
        FScene const& scene, uint8_t visibleLayers, mat4f const& viewMatrix) noexcept
        : vsNearFar(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()),
          visibleLayers(visibleLayers) {

    // the code below only works with affine transforms
    assert_invariant(transpose(viewMatrix)[3] == float4(0, 0, 0, 1));

    // We assume the light is at the origin to compute the SceneInfo. This is consumed later by
    // computeShadowCameraDirectional() which takes this into account.

    // Compute scene bounds in world space, as well as the light-space and view-space near/far planes
    wsShadowCastersVolume = {};
    wsShadowReceiversVolume = {};
    ShadowMap::visitScene(scene, visibleLayers,
            [&](Aabb caster, Culler::result_type) {
                wsShadowCastersVolume.min = min(wsShadowCastersVolume.min, caster.min);
                wsShadowCastersVolume.max = max(wsShadowCastersVolume.max, caster.max);
            },
            [&](Aabb receiver, Culler::result_type) {
                wsShadowReceiversVolume.min = min(wsShadowReceiversVolume.min, receiver.min);
                wsShadowReceiversVolume.max = max(wsShadowReceiversVolume.max, receiver.max);
                auto r = Aabb::transform(viewMatrix.upperLeft(), viewMatrix[3].xyz, receiver);
                vsNearFar.x = std::max(vsNearFar.x, r.max.z);
                vsNearFar.y = std::min(vsNearFar.y, r.min.z);
            }
    );
}

void ShadowMap::updateSceneInfoDirectional(mat4f const& Mv, FScene const& scene,
        SceneInfo& sceneInfo) {

    // the code below only works with affine transforms
    assert_invariant(transpose(Mv)[3] == float4(0, 0, 0, 1));

    sceneInfo.lsNearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
    visitScene(scene, sceneInfo.visibleLayers,
            [&](Aabb caster, Culler::result_type) {
                auto r = Aabb::transform(Mv.upperLeft(), Mv[3].xyz, caster);
                sceneInfo.lsNearFar.x = std::max(sceneInfo.lsNearFar.x, r.max.z);  // near
                sceneInfo.lsNearFar.y = std::min(sceneInfo.lsNearFar.y, r.min.z);  // far
            },
            [&](Aabb receiver, Culler::result_type) {
            }
    );
}

void ShadowMap::updateSceneInfoSpot(mat4f const& Mv, FScene const& scene,
        SceneInfo& sceneInfo) {

    // the code below only works with affine transforms
    assert_invariant(transpose(Mv)[3] == float4(0, 0, 0, 1));

    sceneInfo.lsNearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
    visitScene(scene, sceneInfo.visibleLayers,
            [&](Aabb caster, Culler::result_type mask) {
                if (mask & VISIBLE_DYN_SHADOW_RENDERABLE) {
                    auto r = Aabb::transform(Mv.upperLeft(), Mv[3].xyz, caster);
                    sceneInfo.lsNearFar.x = std::max(sceneInfo.lsNearFar.x, r.max.z);  // near
                    sceneInfo.lsNearFar.y = std::min(sceneInfo.lsNearFar.y, r.min.z);  // far
                }
            },
            [&](Aabb receiver, Culler::result_type) {
            }
    );
}

backend::Viewport ShadowMap::getViewport() const noexcept {
    // We set a viewport with a 1-texel border for when we index outside the texture.
    // This happens only for directional lights when "focus shadow casters" is used,
    // or when shadowFar is smaller than the camera far.
    // For spot- and point-lights we also use a 1-texel border, so that bilinear filtering
    // can work properly if the shadowmap is in an atlas (and we can't rely on h/w clamp).
    const uint32_t dim = mOptions->mapSize;
    const uint16_t border = 1u;
    return { border, border, dim - 2u * border, dim - 2u * border };
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
            return { border, border, dim - 2u * border, dim - 2u * border };
        case ShadowType::SPOT:
        case ShadowType::POINT:
            return { 0, 0, dim, dim };
    }
}

math::float4 ShadowMap::getClampToEdgeCoords(ShadowMapInfo const& shadowMapInfo) const noexcept {
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
    float const l = border;
    float const b = border;
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
        FEngine& engine, const CameraInfo& cameraInfo) noexcept {
    PerShadowMapUniforms::prepareCamera(transaction, engine, cameraInfo);
    PerShadowMapUniforms::prepareLodBias(transaction, 0.0f);
}

void ShadowMap::prepareViewport(Transaction const& transaction,
        backend::Viewport const& viewport) noexcept {
    PerShadowMapUniforms::prepareViewport(transaction, viewport);
}

void ShadowMap::prepareTime(Transaction const& transaction,
        FEngine& engine, math::float4 const& userTime) noexcept {
    PerShadowMapUniforms::prepareTime(transaction, engine, userTime);
}

void ShadowMap::prepareShadowMapping(Transaction const& transaction,
        bool highPrecision) noexcept {
    PerShadowMapUniforms::prepareShadowMapping(transaction, highPrecision);
}

PerShadowMapUniforms::Transaction ShadowMap::open(DriverApi& driver) noexcept {
    return PerShadowMapUniforms::open(driver);
}

void ShadowMap::commit(Transaction& transaction,
        backend::DriverApi& driver) const noexcept {
    mPerShadowMapUniforms.commit(transaction, driver);
}

void ShadowMap::bind(backend::DriverApi& driver) const noexcept {
    mPerShadowMapUniforms.bind(driver);
}

} // namespace filament
