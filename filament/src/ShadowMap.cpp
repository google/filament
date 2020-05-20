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

#include "details/ShadowMap.h"

#include "components/LightManager.h"

#include "details/Engine.h"
#include "details/Scene.h"
#include "details/View.h"

#include "RenderPass.h"

#include <private/filament/SibGenerator.h>

#include <backend/DriverEnums.h>

#include <limits>

using namespace filament::math;
using namespace utils;

namespace filament {
using namespace backend;

// do this only if depth-clamp is available
static constexpr bool USE_DEPTH_CLAMP = false;

static constexpr bool ENABLE_LISPSM = true;

ShadowMap::ShadowMap(FEngine& engine) noexcept :
        mEngine(engine),
        mClipSpaceFlipped(engine.getBackend() == Backend::VULKAN),
        mTextureSpaceFlipped(engine.getBackend() == Backend::METAL) {
    Entity entities[2];
    engine.getEntityManager().create(2, entities);
    mCamera = mEngine.createCamera(entities[0]);
    mDebugCamera = mEngine.createCamera((entities[1]));
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.shadowmap.focus_shadowcasters", &engine.debug.shadowmap.focus_shadowcasters);
    debugRegistry.registerProperty("d.shadowmap.far_uses_shadowcasters", &engine.debug.shadowmap.far_uses_shadowcasters);
    debugRegistry.registerProperty("d.shadowmap.checkerboard", &engine.debug.shadowmap.checkerboard);
    if (ENABLE_LISPSM) {
        debugRegistry.registerProperty("d.shadowmap.lispsm", &engine.debug.shadowmap.lispsm);
        debugRegistry.registerProperty("d.shadowmap.dzn", &engine.debug.shadowmap.dzn);
        debugRegistry.registerProperty("d.shadowmap.dzf", &engine.debug.shadowmap.dzf);
    }
}

ShadowMap::~ShadowMap() {
    FEngine& engine = mEngine;
    Entity entities[] = { mCamera->getEntity(), mDebugCamera->getEntity() };
    for (Entity e : entities) {
        engine.destroyCameraComponent(e);
    }
    engine.getEntityManager().destroy(sizeof(entities) / sizeof(Entity), entities);
}

void ShadowMap::render(DriverApi& driver, Handle<HwRenderTarget> rt,
        filament::Viewport const& viewport, FView::Range const& range, RenderPass& pass, FView& view) noexcept {
    FEngine& engine = mEngine;

    FScene& scene = *view.getScene();

    // FIXME: in the future this will come from the framegraph
    RenderPassParams params = {};
    params.flags.clear = TargetBufferFlags::DEPTH;
    params.flags.discardStart = TargetBufferFlags::DEPTH;
    params.flags.discardEnd = TargetBufferFlags::COLOR0 | TargetBufferFlags::STENCIL;
    params.clearDepth = 1.0;
    params.viewport = viewport;

    FCamera const& camera = getCamera();
    filament::CameraInfo cameraInfo(camera);

    pass.setCamera(cameraInfo);
    pass.setGeometry(scene.getRenderableData(), range, scene.getRenderableUBO());

    view.updatePrimitivesLod(engine, cameraInfo, scene.getRenderableData(), range);
    view.prepareCamera(cameraInfo);
    view.prepareViewport(viewport);
    view.commitUniforms(driver);

    pass.overridePolygonOffset(&mPolygonOffset);
    pass.newCommandBuffer();
    pass.appendCommands(RenderPass::SHADOW);
    pass.sortCommands();
    pass.execute("Shadow map Pass", rt, params);
}

void ShadowMap::update(const FScene::LightSoa& lightData, size_t index, FScene const* scene,
        filament::CameraInfo const& camera, uint8_t visibleLayers, ShadowMapLayout layout,
        CascadeParameters cascadeParams) noexcept {
    // this is the hard part here, find a good frustum for our camera

    auto& lcm = mEngine.getLightManager();

    FLightManager::Instance li = lightData.elementAt<FScene::LIGHT_INSTANCE>(index);
    mShadowMapLayout = layout;

    FLightManager::ShadowParams params = lcm.getShadowParams(li);
    mPolygonOffset = {
            .slope = params.options.polygonOffsetSlope,
            .constant = params.options.polygonOffsetConstant
    };
    mat4f projection(camera.cullingProjection);
    if (params.options.shadowFar > 0.0f) {
        float n = camera.zn;
        float f = params.options.shadowFar;
        if (std::abs(projection[2].w) <= std::numeric_limits<float>::epsilon()) {
            // perspective projection
            projection[2].z =     (f + n) / (n - f);
            projection[3].z = (2 * f * n) / (n - f);
        } else {
            // ortho projection
            projection[2].z =    2.0f / (n - f);
            projection[3].z = (f + n) / (n - f);
        }
    }

    CameraInfo cameraInfo = {
            .projection = projection,
            .model = camera.model,
            .view = camera.view,
            .worldOrigin = camera.worldOrigin,
            .zn = camera.zn,
            .zf = camera.zf,
            .frustum = Frustum(projection * camera.view)
    };

    // debugging...
    const float dz = cameraInfo.zf - cameraInfo.zn;
    float& dzn = mEngine.debug.shadowmap.dzn;
    float& dzf = mEngine.debug.shadowmap.dzf;
    if (dzn < 0)    dzn = std::max(0.0f, params.options.shadowNearHint - camera.zn) / dz;
    else            params.options.shadowNearHint = dzn * dz - camera.zn;
    if (dzf > 0)    dzf =-std::max(0.0f, camera.zf - params.options.shadowFarHint) / dz;
    else            params.options.shadowFarHint = dzf * dz + camera.zf;
    using Type = FLightManager::Type;
    switch (lcm.getType(li)) {
        case Type::SUN:
        case Type::DIRECTIONAL:
            computeShadowCameraDirectional(
                    lightData.elementAt<FScene::DIRECTION>(index), scene, cameraInfo, params,
                    visibleLayers, cascadeParams);
            break;
        case Type::FOCUSED_SPOT:
        case Type::SPOT:
            computeShadowCameraSpot(lightData.elementAt<FScene::POSITION_RADIUS>(index).xyz,
                    lightData.elementAt<FScene::DIRECTION>(index), lcm.getSpotLightOuterCone(li),
                    lightData.elementAt<FScene::POSITION_RADIUS>(index).w, cameraInfo, params);
            break;
        case Type::POINT:
            break;
    }
}

void ShadowMap::computeShadowCameraDirectional(
        float3 const& dir, FScene const* scene, CameraInfo const& camera,
        FLightManager::ShadowParams const& params,
        uint8_t visibleLayers, CascadeParameters cascadeParams) noexcept {

    /*
     * Compute the light's model matrix
     * (direction & position)
     *
     * The light's model matrix contains the light position and direction.
     *
     * For directional lights, we could choose any position; we pick the camera position
     * so we have a fixed reference -- that's "not too far" from the scene.
     */
    const float3 lightPosition = camera.getPosition();
    const mat4f M = mat4f::lookAt(lightPosition, lightPosition + dir, float3{ 0, 1, 0 });
    const mat4f Mv = FCamera::rigidTransformInverse(M);

    // Compute scene bounds in world space, as well as the light-space near/far planes
    // TODO: this is recalculated for each cascade, but doesn't need to be.
    float2 nearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
    Aabb wsShadowCastersVolume, wsShadowReceiversVolume;
    visitScene(*scene, visibleLayers,
            [&wsShadowCastersVolume, &Mv, &nearFar](Aabb caster) {
                wsShadowCastersVolume.min = min(wsShadowCastersVolume.min, caster.min);
                wsShadowCastersVolume.max = max(wsShadowCastersVolume.max, caster.max);
                float2 nf = computeNearFar(Mv, caster);
                nearFar.x = std::max(nearFar.x, nf.x);  // near
                nearFar.y = std::min(nearFar.y, nf.y);  // far
            },
            [&wsShadowReceiversVolume](Aabb receiver) {
                wsShadowReceiversVolume.min = min(wsShadowReceiversVolume.min, receiver.min);
                wsShadowReceiversVolume.max = max(wsShadowReceiversVolume.max, receiver.max);
            }
    );

    if (wsShadowCastersVolume.isEmpty() || wsShadowReceiversVolume.isEmpty()) {
        mHasVisibleShadows = false;
        return;
    }

    // view frustum vertices in world-space
    float3 wsViewFrustumVertices[8];
    computeFrustumCorners(wsViewFrustumVertices,
            camera.model * FCamera::inverseProjection(camera.projection),
            cascadeParams.csNear, cascadeParams.csFar);

    // compute the intersection of the shadow receivers volume with the view volume
    // in world space. This returns a set of points on the convex-hull of the intersection.
    size_t vertexCount = intersectFrustumWithBox(mWsClippedShadowReceiverVolume,
            wsViewFrustumVertices, wsShadowReceiversVolume);

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
    if (!USE_DEPTH_CLAMP) {
        // near plane from shadow caster volume
        lsLightFrustumBounds.max.z = nearFar[0];
    }
    for (size_t i = 0; i < vertexCount; ++i) {
        // far: figure out farthest shadow receivers
        float3 v = mat4f::project(Mv, mWsClippedShadowReceiverVolume[i]);
        lsLightFrustumBounds.min.z = std::min(lsLightFrustumBounds.min.z, v.z);
        if (USE_DEPTH_CLAMP) {
            // further tighten to the shadow receiver volume
            lsLightFrustumBounds.max.z = std::max(lsLightFrustumBounds.max.z, v.z);
        }
    }
    if (mEngine.debug.shadowmap.far_uses_shadowcasters) {
        // far: closest of the farthest shadow casters and receivers
        lsLightFrustumBounds.min.z = std::max(lsLightFrustumBounds.min.z, nearFar[1]);
    }

    // near / far planes are specified relative to the direction the eye is looking at
    // i.e. the -z axis (see: ortho)
    const float znear = -lsLightFrustumBounds.max.z;
    const float zfar = -lsLightFrustumBounds.min.z;

    // if znear >= zfar, it means we don't have any shadow caster in front of a shadow receiver
    if (UTILS_UNLIKELY(znear >= zfar)) {
        mHasVisibleShadows = false;
        return;
    }

    float4 viewVolumeBoundingSphere = {};
    if (params.options.stable) {
        // In stable mode, the light frustum size must be fixed, so we can choose either the
        // whole view frustum, or the whole scene bounding volume. We simply pick whichever is
        // is smaller.

        // in stable mode we simply take the shadow receivers volume
        const float4 shadowReceiverVolumeBoundingSphere = computeBoundingSphere(
                wsShadowReceiversVolume.getCorners().data(), 8);

        // in stable mode we simply take the view volume, bounding sphere
        viewVolumeBoundingSphere = computeBoundingSphere(wsViewFrustumVertices, 8);

        if (shadowReceiverVolumeBoundingSphere.w < viewVolumeBoundingSphere.w) {
            viewVolumeBoundingSphere.w = 0;
            std::copy_n(wsShadowReceiversVolume.getCorners().data(), 8,
                    mWsClippedShadowReceiverVolume.data());
        }
    }

    mHasVisibleShadows = vertexCount >= 2;
    if (mHasVisibleShadows) {
        // We can't use LISPSM in stable mode
        const bool USE_LISPSM = ENABLE_LISPSM && mEngine.debug.shadowmap.lispsm && !params.options.stable;

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
        if (USE_LISPSM) {
            // Orient the shadow map in the direction of the view vector by constructing a
            // rotation matrix in light space around the z-axis, that aligns the y-axis with the camera's
            // forward vector (V) -- this gives the wrap direction, vp, for LiSPSM.
            const float3 wsCameraFwd = camera.getForwardVector();
            const float3 lsCameraFwd = Mv.upperLeft() * wsCameraFwd;
            // If the light and view vector are parallel, this rotation becomes
            // meaningless. Just use identity.
            // (LdotV == (Mv*V).z, because L = {0,0,1} in light-space)
            mat4f L; // Rotation matrix in light space
            if (UTILS_LIKELY(std::abs(lsCameraFwd.z) < 0.9997f)) { // this is |dot(L, V)|
                const float3 vp{ normalize(lsCameraFwd.xy), 0 }; // wrap direction in light-space
                L[0].xyz = cross(vp, float3{ 0, 0, 1 });
                L[1].xyz = vp;
                L[2].xyz = { 0, 0, 1 };
                L = transpose(L);
            }

            LMpMv = L * MpMv;

            W = applyLISPSM(Wp, camera, params, LMpMv,
                    mWsClippedShadowReceiverVolume, vertexCount, dir);
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

        Aabb bounds;
        if (params.options.stable && viewVolumeBoundingSphere.w > 0) {
            bounds = compute2DBounds(Mv, viewVolumeBoundingSphere);
        } else {
            bounds = compute2DBounds(WLMpMv, mWsClippedShadowReceiverVolume.data(), vertexCount);
        }
        lsLightFrustumBounds.min.xy = bounds.min.xy;
        lsLightFrustumBounds.max.xy = bounds.max.xy;

        if (params.options.stable) {
            // in stable mode we can't do anything that can change the scaling of the texture
        } else {
            // For directional lights, we further constraint the light frustum to the
            // intersection of the shadow casters & shadow receivers in light-space.
            // ** This relies on the 1-texel shadow map border **
            if (mEngine.debug.shadowmap.focus_shadowcasters) {
                intersectWithShadowCasters(lsLightFrustumBounds, WLMpMv, wsShadowCastersVolume);
            }
        }

        if (UTILS_UNLIKELY((lsLightFrustumBounds.min.x >= lsLightFrustumBounds.max.x) ||
                           (lsLightFrustumBounds.min.y >= lsLightFrustumBounds.max.y))) {
            // this could happen if the only thing visible is a perfectly horizontal or
            // vertical thin line
            mHasVisibleShadows = false;
            return;
        }

        assert(lsLightFrustumBounds.min.x < lsLightFrustumBounds.max.x);
        assert(lsLightFrustumBounds.min.y < lsLightFrustumBounds.max.y);

        // compute focus scale and offset
        float2 s = 2.0f / float2(lsLightFrustumBounds.max.xy - lsLightFrustumBounds.min.xy);
        float2 o =   -s * float2(lsLightFrustumBounds.max.xy + lsLightFrustumBounds.min.xy) * 0.5f;

        if (params.options.stable) {
            // Use the world origin as reference point, fixed w.r.t. the camera
            snapLightFrustum(s, o, Mv, camera.worldOrigin[3].xyz, 1.0f / mShadowMapLayout.shadowDimension);
        }

        const mat4f F(mat4f::row_major_init {
                 s.x,   0,  0, o.x,
                   0, s.y,  0, o.y,
                   0,   0,  1,   0,
                   0,   0,  0,   1,
        });


        /*
         * Final shadow map transform
         */

        // Final shadow transform
        const mat4f S = F * WLMpMv;

        // Computes St the transform to use in the shader to access the shadow map texture
        // i.e. it transform a world-space vertex to a texture coordinate in the shadow-map
        const mat4f MbMt = getTextureCoordsMapping();
        const mat4f St = MbMt * S;

        // note: in texelSizeWorldSpace() below, we could use Mb * Mt * F * W because
        // L * Mp * Mv is a rigid transform (for directional lights)
        if (USE_LISPSM) {
            mTexelSizeWs = texelSizeWorldSpace(Wp, MbMt * F);
        } else {
            // We know we're using an ortho projection
            mTexelSizeWs = texelSizeWorldSpace(St.upperLeft());
        }
        mLightSpace = St;

        // We apply the constant bias in world space (as opposed to light-space) to account
        // for perspective and lispsm shadow maps. This also allows us to do this at zero-cost
        // by baking it in the shadow-map itself.

        const mat4f Sb = S * mat4f::translation(dir * params.options.constantBias);
        mCamera->setCustomProjection(mat4(Sb), znear, zfar);

        // for the debug camera, we need to undo the world origin
        mDebugCamera->setCustomProjection(mat4(Sb * camera.worldOrigin), znear, zfar);
    }
}

void ShadowMap::computeShadowCameraSpot(math::float3 const& position, math::float3 const& dir,
        float outerConeAngle, float radius, CameraInfo const& camera, FLightManager::ShadowParams const& params) noexcept {

    // TODO: correctly compute if this spot light has any visible shadows.
    mHasVisibleShadows = true;

    /*
     * Compute the light models matrix.
     */

    // Choose a reasonable value for the near plane.
    const float nearPlane = 0.1f;

    const float3 lightPosition = position;
    const mat4f M = mat4f::lookAt(lightPosition, lightPosition + dir, float3{0, 1, 0});
    const mat4f Mv = FCamera::rigidTransformInverse(M);

    float outerConeAngleDegrees = outerConeAngle / (2.0f * F_PI) * 360.0f;
    const mat4f Mp = mat4f::perspective(outerConeAngleDegrees * 2, 1.0f, nearPlane, radius,
            mat4f::Fov::HORIZONTAL);

    const mat4f MpMv(Mp * Mv);

    // Final shadow transform
    const mat4f S = MpMv;

    const mat4f MbMt = getTextureCoordsMapping();
    const mat4f St = MbMt * S;
    mTexelSizeWs = texelSizeWorldSpace(Mp, MbMt);
    mLightSpace = St;

    const mat4f Sb = S * mat4f::translation(dir * params.options.constantBias);
    mCamera->setCustomProjection(mat4(Sb), nearPlane, radius);

    // for the debug camera, we need to undo the world origin
    mDebugCamera->setCustomProjection(mat4(Sb * camera.worldOrigin), nearPlane, radius);
}

mat4f ShadowMap::applyLISPSM(math::mat4f& Wp,
        CameraInfo const& camera, FLightManager::ShadowParams const& params,
        mat4f const& LMpMv,
        FrustumBoxIntersection const& wsShadowReceiversVolume, size_t vertexCount,
        float3 const& dir) {

    const float LoV = dot(camera.getForwardVector(), dir);
    const float sinLV = std::sqrt(std::max(0.0f, 1.0f - LoV * LoV));

    // Virtual near plane -- the default is 1m, can be changed by the user.
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
    // of the shadow receivers volume.
    // Note: znear/zfar encoded in Mp has no influence here (b/c we're interested only by the y axis)
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
    if (sinLV > 0 && 3.0f * (dzn / (zf - zn)) < 2.0f) {
        // nopt is the optimal near plane distance of Wp (i.e. distance from P).

        // virtual near and far planes
        const float vz0 = std::max(0.0f, std::max(zn + dzn, z0));
        const float vz1 = std::max(0.0f, std::min(zf - dzf, z1));

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
                // x axis. Doesn't seem to make a big difference in the end.
                lsCameraPosition.x,
                n - nopt,
                // note: various papers suggest to use the shadow receiver's center z coordinate in light
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


mat4f ShadowMap::getTextureCoordsMapping() const noexcept {
    // remapping from NDC to texture coordinates (i.e. [-1,1] -> [0, 1])
    const mat4f Mt(mClipSpaceFlipped ? mat4f::row_major_init{
            0.5f,   0,    0,  0.5f,
              0, -0.5f,   0,  0.5f,
              0,    0,  0.5f, 0.5f,
              0,    0,    0,    1
    } : mat4f::row_major_init{
            0.5f,   0,    0,  0.5f,
              0,  0.5f,   0,  0.5f,
              0,    0,  0.5f, 0.5f,
              0,    0,    0,    1
    });

    // the shadow map texture might be larger than the shadow map dimension, so we add a scaling
    // factor
    const float v = (float) mShadowMapLayout.textureDimension / mShadowMapLayout.atlasDimension;
    const mat4f Mv(mat4f::row_major_init{
            v, 0, 0, 0,
            0, v, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
    });

    // apply the 1-texel border viewport transform
    const float o = 1.0f / mShadowMapLayout.atlasDimension;
    const float s = 1.0f - 2.0f * (1.0f / mShadowMapLayout.textureDimension);
    const mat4f Mb(mat4f::row_major_init{
             s, 0, 0, o,
             0, s, 0, o,
             0, 0, 1, 0,
             0, 0, 0, 1
    });

    const mat4f Mf = mTextureSpaceFlipped ? mat4f(mat4f::row_major_init{
            1,  0,  0,  0,
            0, -1,  0,  1,
            0,  0,  1,  0,
            0,  0,  0,  1
    }) : mat4f();

    // Compute shadow-map texture access transform
    return Mf * Mb * Mv * Mt;
}

// This construct a frustum (similar to glFrustum or frustum), except
// it looks towards the +y axis, and assumes -1,1 for the left/right and bottom/top planes.
mat4f ShadowMap::warpFrustum(float n, float f) noexcept {
    const float d = 1 / (f - n);
    const float A = (f + n) * d;
    const float B = -2 * n * f * d;
    const mat4f Wp(mat4f::row_major_init{
            n, 0, 0, 0,
            0, A, 0, B,
            0, 0, n, 0,
            0, 1, 0, 0
    });
    return Wp;
}

math::mat4f ShadowMap::directionalLightFrustum(float near, float far) noexcept {
    const float d = far - near;
    mat4f m;
    m[2][2] = -2 / d;
    m[3][2] = -(far + near)  / d;
    return m;
}

float2 ShadowMap::computeNearFar(const mat4f& view,
        Aabb const& wsShadowCastersVolume) noexcept {
    const Aabb::Corners wsSceneCastersCorners = wsShadowCastersVolume.getCorners();
    return computeNearFar(view, wsSceneCastersCorners.data(), wsSceneCastersCorners.size());
}

float2 ShadowMap::computeNearFar(const mat4f& view,
        float3 const* wsVertices, size_t count) noexcept {
    float2 nearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
    for (size_t i = 0; i < count; i++) {
        // we're on the z axis in light space (looking down to -z)
        float c = mat4f::project(view, wsVertices[i]).z;
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
        // we're on the y axis in light space (looking down to +y)
        float c = mat4f::project(lightView, wsVertices[i]).y;
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

Aabb ShadowMap::compute2DBounds(const mat4f& lightView, float4 const& sphere) noexcept {
    // this assumes a rigid body transform
    float4 s;
    s.xyz = (lightView * float4{sphere.xyz, 1.0f}).xyz;
    s.w = sphere.w;
    return Aabb{s.xyz - s.w, s.xyz + s.w};
}

void ShadowMap::intersectWithShadowCasters(Aabb& UTILS_RESTRICT lightFrustum,
        mat4f const& lightView, Aabb const& wsShadowCastersVolume) noexcept {

    // construct the Focus transform (scale + offset)
    const float2 s = 2.0f / float2(lightFrustum.max.xy - lightFrustum.min.xy);
    const float2 o =   -s * float2(lightFrustum.max.xy + lightFrustum.min.xy) * 0.5f;
    const mat4f F(mat4f::row_major_init {
            s.x,   0,  0, o.x,
              0, s.y,  0, o.y,
              0,   0,  1,   0,
              0,   0,  0,   1,
    });
    float3 wsLightFrustumCorners[8];
    const mat4f projection = F * lightView;
    computeFrustumCorners(wsLightFrustumCorners, inverse(projection));

    // Intersect shadow-caster AABB with current light frustum in world-space:
    //
    // This (obviously) guarantees that the resulting volume is inside the light frustum;
    // when using LiSPSM (or projection lights, i.e. when lightView is a projection), we must
    // first intersect wsShadowCastersVolume with the light's frustum, otherwise we end-up
    // transforming vertices that are "outside" the frustum, and that's forbidden.
    FrustumBoxIntersection wsClippedShadowCasterVolumeVertices;
    size_t vertexCount = intersectFrustumWithBox(wsClippedShadowCasterVolumeVertices,
            wsLightFrustumCorners, wsShadowCastersVolume);

    // compute shadow-caster bounds in light space
    Aabb box = compute2DBounds(lightView, wsClippedShadowCasterVolumeVertices.data(), vertexCount);

    // intersect shadow-caster and current light frustum bounds
    lightFrustum.min.xy = max(box.min.xy, lightFrustum.min.xy);
    lightFrustum.max.xy = min(box.max.xy, lightFrustum.max.xy);
}

void ShadowMap::computeFrustumCorners(float3* UTILS_RESTRICT out,
        const mat4f& UTILS_RESTRICT projectionViewInverse, float csNear, float csFar) noexcept {

    // compute view frustum in world space (from its NDC)
    // matrix to convert: ndc -> camera -> world
    float3 csViewFrustumCorners[8] = {
            { -1, -1,  csFar },
            {  1, -1,  csFar },
            { -1,  1,  csFar },
            {  1,  1,  csFar },
            { -1, -1,  csNear },
            {  1, -1,  csNear },
            { -1,  1,  csNear },
            {  1,  1,  csNear },
    };
    for (float3 c : csViewFrustumCorners) {
        *out++ = mat4f::project(projectionViewInverse, c);
    }
}

void ShadowMap::snapLightFrustum(float2& s, float2& o,
        mat4f const& Mv, float3 worldOrigin, float2 shadowMapResolution) noexcept {

    auto fmod = [](float2 x, float2 y) -> float2 {
        auto mod = [](float x, float y) -> float { return std::fmod(x, y); };
        return float2{ mod(x[0], y[0]), mod(x[1], y[1]) };
    };

    // This snaps the shadow map bounds to texels.
    // The 2.0 comes from Mv having a NDC in the range -1,1 (so a range of 2).
    const float2 r = 2.0f * shadowMapResolution;
    o -= fmod(o, r);

    // This offsets the texture coordinates so it has a fixed offset w.r.t the world
    const float2 lsOrigin = mat4f::project(Mv, worldOrigin).xy * s;
    o -= fmod(lsOrigin, r);
}

size_t ShadowMap::intersectFrustumWithBox(
        FrustumBoxIntersection& UTILS_RESTRICT outVertices,
        const float3* UTILS_RESTRICT wsFrustumCorners,
        Aabb const& UTILS_RESTRICT wsBox)
{
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
    #pragma nounroll
    for (size_t i = 0; i < 8; i++) {
        float3 p = wsFrustumCorners[i];
        outVertices[vertexCount] = p;
        if ((p.x >= wsBox.min.x && p.x <= wsBox.max.x) &&
            (p.y >= wsBox.min.y && p.y <= wsBox.max.y) &&
            (p.z >= wsBox.min.z && p.z <= wsBox.max.z)) {
            vertexCount++;
        }
    }
    const bool someFrustumVerticesAreInTheBox = vertexCount > 0;
    constexpr const float EPSILON = 1.0f / 8192.0f; // ~0.012 mm

    // at this point if we have 8 vertices, we can skip the rest
    if (vertexCount < 8) {
        Frustum frustum(wsFrustumCorners);
        float4 const* wsFrustumPlanes = frustum.getNormalizedPlanes();

        // b) add the scene's vertices that are known to be inside the view frustum
        //
        // We need to handle the case where a corner of the box lies exactly on a plane of
        // the frustum. This actually happens often due to fitting light-space
        // We fudge the distance to the plane by a small amount.
        #pragma nounroll
        for (float3 p : wsSceneReceiversCorners) {
            outVertices[vertexCount] = p;
            float l = dot(wsFrustumPlanes[0].xyz, p) + wsFrustumPlanes[0].w;
            float b = dot(wsFrustumPlanes[1].xyz, p) + wsFrustumPlanes[1].w;
            float r = dot(wsFrustumPlanes[2].xyz, p) + wsFrustumPlanes[2].w;
            float t = dot(wsFrustumPlanes[3].xyz, p) + wsFrustumPlanes[3].w;
            float f = dot(wsFrustumPlanes[4].xyz, p) + wsFrustumPlanes[4].w;
            float n = dot(wsFrustumPlanes[5].xyz, p) + wsFrustumPlanes[5].w;
            if ((l <= EPSILON) && (b <= EPSILON) &&
                (r <= EPSILON) && (t <= EPSILON) &&
                (f <= EPSILON) && (n <= EPSILON)) {
                ++vertexCount;
            }
        }

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

        if (someFrustumVerticesAreInTheBox || vertexCount < 8) {
            // c) intersect scene's volume edges with frustum planes
            vertexCount = intersectFrustum(outVertices.data(), vertexCount,
                    wsSceneReceiversCorners.vertices, wsFrustumCorners);

            // d) intersect frustum edges with the scene's volume planes
            vertexCount = intersectFrustum(outVertices.data(), vertexCount,
                    wsFrustumCorners, wsSceneReceiversCorners.vertices);
        }
    }

    assert(vertexCount <= outVertices.size());

    return vertexCount;
}

constexpr const ShadowMap::Segment ShadowMap::sBoxSegments[12];
constexpr const ShadowMap::Quad ShadowMap::sBoxQuads[6];

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
        size_t maxVertexCount = vertexCount + 2;
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
    bool hit = intersectSegmentWithTriangle(p, s0, s1, t0, t1, t2) ||
               intersectSegmentWithTriangle(p, s0, s1, t0, t2, t3);
    return hit;
}

float ShadowMap::texelSizeWorldSpace(const mat3f& worldToShadowTexture) const noexcept {
    // The Jacobian of the transformation from texture-to-world is the matrix itself for
    // orthographic projections. We just need to inverse worldToShadowTexture,
    // which is guaranteed to be orthographic.
    // The two first columns give us the how a texel maps in world-space.
    const float ures = 1.0f / mShadowMapLayout.shadowDimension;
    const float vres = 1.0f / mShadowMapLayout.shadowDimension;
    const mat3f shadowTextureToWorld(inverse(worldToShadowTexture));
    const float3 Jx = shadowTextureToWorld[0];
    const float3 Jy = shadowTextureToWorld[1];
    const float s = std::max(length(Jx) * ures, length(Jy) * vres);
    return s;
}

float ShadowMap::texelSizeWorldSpace(const mat4f& Wp, const mat4f& MbMtF) const noexcept {
    // Here we compute the Jacobian of inverse(MbMtF * Wp).
    // The expression below has been computed with Mathematica. However, it's not very hard,
    // albeit error prone, to do it by hand because MbMtF is a linear transform.
    // So we really only need to calculate the Jacobian of inverse(Wp) at inverse(MbMtF).
    //
    // Because we're only interested in the length of the columns of the Jacobian, we can use
    // Mb * Mt * F * Wp instead of the full expression Mb * Mt * F * Wp * Wv * L * Mp * Mv,
    // because Wv * L * Mp * Mv is a rigid transform, which doesn't affect the length of
    // the Jacobian's column vectors.

    // The Jacobian is not constant, so we evaluate it in the center of the shadow-map texture.
    // It might be better to do this computation in the vertex shader.
    float3 p = {0.5, 0.5, 0.0};

    const float ures = 1.0f / mShadowMapLayout.shadowDimension;
    const float vres = 1.0f / mShadowMapLayout.shadowDimension;
    const float dres = mShadowMapLayout.zResolution;

    constexpr bool JACOBIAN_ESTIMATE = false;
    if (JACOBIAN_ESTIMATE) {
        // this estimates the Jacobian -- this is a lot heavier. This is mostly for reference
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
            j * dz * sz,    j * X * sz,     0,
            0,              j * nsxsz,      0,
            0,              j * Z * sx,     j * dz * sx
    });

    float3 Jx = J[0] * ures;
    float3 Jy = J[1] * vres;
    UTILS_UNUSED float3 Jz = J[2] * dres;
    const float s = std::max(length(Jx), length(Jy));
    return s;
}


template<typename Casters, typename Receivers>
void ShadowMap::visitScene(const FScene& scene, uint32_t visibleLayers,
        Casters casters, Receivers receivers) noexcept {
    using State = FRenderableManager::Visibility;
    FScene::RenderableSoa const& UTILS_RESTRICT soa = scene.getRenderableData();
    float3 const* const UTILS_RESTRICT worldAABBCenter = soa.data<FScene::WORLD_AABB_CENTER>();
    float3 const* const UTILS_RESTRICT worldAABBExtent = soa.data<FScene::WORLD_AABB_EXTENT>();
    uint8_t const* const UTILS_RESTRICT layers = soa.data<FScene::LAYERS>();
    State const* const UTILS_RESTRICT visibility = soa.data<FScene::VISIBILITY_STATE>();
    size_t c = soa.size();
    for (size_t i = 0; i < c; i++) {
        if (layers[i] & visibleLayers) {
            const Aabb aabb{ worldAABBCenter[i] - worldAABBExtent[i],
                             worldAABBCenter[i] + worldAABBExtent[i] };
            if (visibility[i].castShadows) {
                casters(aabb);
            }
            if (visibility[i].receiveShadows) {
                receivers(aabb);
            }
        }
    }
}

} // namespace filament
