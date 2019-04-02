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
#include "details/ShadowMap.h"
#include "details/Scene.h"

#include <private/filament/SibGenerator.h>

#include <backend/DriverEnums.h>

#include <limits>

using namespace filament::math;
using namespace utils;

namespace filament {
using namespace backend;

namespace details {

// do this only if depth-clamp is available
static constexpr bool USE_DEPTH_CLAMP = false;

static constexpr bool ENABLE_LISPSM = true;

ShadowMap::ShadowMap(FEngine& engine) noexcept :
        mEngine(engine),
        mClipSpaceFlipped(engine.getBackend() == Backend::VULKAN ||
                          engine.getBackend() == Backend::METAL) {
    mCamera = mEngine.createCamera(EntityManager::get().create());
    mDebugCamera = mEngine.createCamera(EntityManager::get().create());
    FDebugRegistry& debugRegistry = engine.getDebugRegistry();
    debugRegistry.registerProperty("d.shadowmap.focus_shadowcasters", &engine.debug.shadowmap.focus_shadowcasters);
    debugRegistry.registerProperty("d.shadowmap.far_uses_shadowcasters", &engine.debug.shadowmap.far_uses_shadowcasters);
    if (ENABLE_LISPSM) {
        debugRegistry.registerProperty("d.shadowmap.lispsm", &engine.debug.shadowmap.lispsm);
        debugRegistry.registerProperty("d.shadowmap.dzn", &engine.debug.shadowmap.dzn);
        debugRegistry.registerProperty("d.shadowmap.dzf", &engine.debug.shadowmap.dzf);
    }
}

ShadowMap::~ShadowMap() {
    mEngine.destroy(mCamera->getEntity());
    mEngine.destroy(mDebugCamera->getEntity());
}

void ShadowMap::prepare(DriverApi& driver, SamplerGroup& sb) noexcept {
    assert(mShadowMapDimension);

    uint32_t dim = mShadowMapDimension;
    uint32_t currentDimension = mViewport.width + 2;
    if (currentDimension == dim) {
        // nothing to do here.
        assert(mShadowMapHandle);
        return;
    }

    // destroy the current rendertarget and texture
    if (mShadowMapRenderTarget) {
        driver.destroyRenderTarget(mShadowMapRenderTarget);
    }
    if (mShadowMapHandle) {
        driver.destroyTexture(mShadowMapHandle);
    }

    // allocate new ones...
    // we set a viewport with a 1-texel border for when we index outside of the texture
    // DON'T CHANGE this unless computeLightSpaceMatrix() is updated too.
    // see: computeLightSpaceMatrix()
    mViewport = { 1, 1, dim - 2, dim - 2 };

    mShadowMapHandle = driver.createTexture(
            SamplerType::SAMPLER_2D, 1, TextureFormat::DEPTH16, 1, dim, dim, 1,
            TextureUsage::DEPTH_ATTACHMENT);

    mShadowMapRenderTarget = driver.createRenderTarget(
            TargetBufferFlags::SHADOW, dim, dim, 1, TextureFormat::DEPTH16,
            {}, { mShadowMapHandle }, {});

    SamplerParams s;
    s.filterMag = SamplerMagFilter::LINEAR;
    s.filterMin = SamplerMinFilter::LINEAR;
    s.compareFunc = SamplerCompareFunc::LE;
    s.compareMode = SamplerCompareMode::COMPARE_TO_TEXTURE;
    s.depthStencil = true;
    sb.setSampler(PerViewSib::SHADOW_MAP, { mShadowMapHandle, s });
}

void ShadowMap::terminate(DriverApi& driverApi) noexcept {
    if (mShadowMapRenderTarget) {
        driverApi.destroyRenderTarget(mShadowMapRenderTarget);
    }
    if (mShadowMapHandle) {
        driverApi.destroyTexture(mShadowMapHandle);
    }
}

void ShadowMap::update(
        const FScene::LightSoa& lightData, size_t index, FScene const* scene,
        details::CameraInfo const& camera, uint8_t visibleLayers) noexcept {
    // this is the hard part here, find a good frustum for our camera

    auto& lcm = mEngine.getLightManager();

    FLightManager::Instance li = lightData.elementAt<FScene::LIGHT_INSTANCE>(index);
    mShadowMapDimension = std::max(1u, lcm.getShadowMapSize(li));

    FLightManager::ShadowParams params = lcm.getShadowParams(li);
    mat4f projection(camera.cullingProjection);
    if (params.shadowFar > 0.0f) {
        float n = camera.zn;
        float f = params.shadowFar;
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
            .zn = camera.zn,
            .zf = camera.zf,
            .dzn = std::max(0.0f, params.shadowNearHint - camera.zn),
            .dzf = std::max(0.0f, camera.zf - params.shadowFarHint),
            .frustum = Frustum(projection * camera.view),
            .worldOrigin = camera.worldOrigin
    };

    // debugging...
    const float dz = cameraInfo.zf - cameraInfo.zn;
    float& dzn = mEngine.debug.shadowmap.dzn;
    float& dzf = mEngine.debug.shadowmap.dzf;
    if (dzn < 0)    dzn = cameraInfo.dzn / dz;
    else            cameraInfo.dzn = dzn * dz;
    if (dzf > 0)    dzf =-cameraInfo.dzf / dz;
    else            cameraInfo.dzf =-dzf * dz;


    using Type = FLightManager::Type;
    switch (lcm.getType(li)) {
        case Type::SUN:
        case Type::DIRECTIONAL:
            computeShadowCameraDirectional(
                    lightData.elementAt<FScene::DIRECTION>(index), scene, cameraInfo,
                    visibleLayers);
            break;
        case Type::FOCUSED_SPOT:
        case Type::SPOT:
            break;
        case Type::POINT:
            break;
    }
}

void ShadowMap::computeShadowCameraDirectional(
        float3 const& dir, FScene const* scene, CameraInfo const& camera,
        uint8_t visibleLayers) noexcept {

    // scene bounds in world space
    Aabb wsShadowCastersVolume, wsShadowReceiversVolume;
    scene->computeBounds(wsShadowCastersVolume, wsShadowReceiversVolume, visibleLayers);
    if (wsShadowCastersVolume.isEmpty() || wsShadowReceiversVolume.isEmpty()) {
        mHasVisibleShadows = false;
        return;
    }

    float3 wsViewFrustumCorners[8];
    computeFrustumCorners(wsViewFrustumCorners,
            camera.model * FCamera::inverseProjection(camera.projection));

    // compute the intersection of the shadow receivers volume with the view volume
    // in world space. This returns a set of points on the convex-hull of the intersection.
    size_t vertexCount = intersectFrustumWithBox(mWsClippedShadowReceiverVolume,
            camera.frustum, wsViewFrustumCorners, wsShadowReceiversVolume);

    mHasVisibleShadows = vertexCount >= 2;
    if (mHasVisibleShadows) {
        const bool USE_LISPSM = ENABLE_LISPSM && mEngine.debug.shadowmap.lispsm;

        /*
         * Compute the light's model matrix
         * (direction & position)
         *
         * The light's model matrix contains the light position and direction.
         *
         * For directional lights, we can choose any position.
         */
        const float3 lightPosition = {}; // TODO: pick something better
        const mat4f M = mat4f::lookAt(lightPosition, lightPosition + dir, float3{ 0, 1, 0 });
        const mat4f Mv = FCamera::rigidTransformInverse(M);

        // Orient the shadow map in the direction of the view vector by constructing a
        // rotation matrix around the z-axis, that aligns the y-axis with the camera's
        // forward vector (V) -- this gives the wrap direction for LiSPSM.
        //
        // If the light and view vector are parallel, this rotation becomes
        // meaningless. Just use identity.
        // (LdotV == (Mv*V).z, because L = {0,0,1} in light-space)
        mat4f L;
        const float3 wsCameraFwd(camera.getForwardVector());
        const float3 lsCameraFwd = mat4f::project(Mv, wsCameraFwd);
        if (UTILS_LIKELY(std::abs(lsCameraFwd.z) < 0.9997f)) { // this is |dot(L, V)|
            L[0].xyz = normalize(cross(lsCameraFwd, float3{ 0, 0, 1 }));
            L[1].xyz = cross(float3{ 0, 0, 1 }, L[0].xyz);
            L[2].xyz = { 0, 0, 1 };
        }
        L = transpose(L);

        // lights space matrix used for finding the near and far planes
        const mat4f LMv(L * Mv);

        /*
         * Compute the light's projection matrix
         * (directional/point lights, i.e. projection to use, including znear/zfar clip planes)
         */

        // 1) compute scene zmax (i.e. Near plane) and zmin (i.e. Far plane) in light space.
        //    (near/far correspond to max/min because the light looks down the -z axis).
        //    - The Near plane is set to the shadow casters max z (i.e. closest to the light)
        //    - The Far plane is set to the closest of the farthest shadow casters and receivers
        //      i.e.: shadow casters behind the last receivers can't cast any shadows
        //
        //    If "depth clamp" is supported, we can further tighten the near plane to the
        //    shadow receiver.

        Aabb lsLightFrustum;
        const float2 nearFar = computeNearFar(LMv, wsShadowCastersVolume);
        if (!USE_DEPTH_CLAMP) {
            // near plane from shadow caster volume
            lsLightFrustum.max.z = nearFar[0];
        }
        for (size_t i = 0; i < vertexCount; ++i) {
            // far: figure out farthest shadow receivers
            float3 v = mat4f::project(LMv, mWsClippedShadowReceiverVolume[i]);
            lsLightFrustum.min.z = std::min(lsLightFrustum.min.z, v.z);
            if (USE_DEPTH_CLAMP) {
                // further tighten to the shadow receiver volume
                lsLightFrustum.max.z = std::max(lsLightFrustum.max.z, v.z);
            }
        }
        if (mEngine.debug.shadowmap.far_uses_shadowcasters) {
            // far: closest of the farthest shadow casters and receivers
            lsLightFrustum.min.z = std::max(lsLightFrustum.min.z, nearFar[1]);
        }

        // near / far planes are specified relative to the direction the eye is looking at
        // i.e. the -z axis (see: ortho)
        const float znear = -lsLightFrustum.max.z;
        const float zfar = -lsLightFrustum.min.z;

        // if znear >= zfar, it means we don't have any shadow caster in front of a shadow receiver
        if (UTILS_UNLIKELY(znear >= zfar)) {
            mHasVisibleShadows = false;
            return;
        }

        // The light's projection, ortho for directional lights, perspective otherwise
        const mat4f Mp = directionalLightFrustum(znear, zfar);

        /*
         * Compute warping (optional, improve quality)
         */

        // lights space matrix used for finding the near and far planes
        const mat4f LMpMv(L * Mp * Mv);

        // Compute the LiSPSM warping
        mat4f W;
        if (USE_LISPSM) {
            W = applyLISPSM(camera, LMpMv, mWsClippedShadowReceiverVolume, vertexCount, dir);
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

        // disable vectorization here because vertexCount is <= 64, not worth the increased code size.
        #pragma clang loop vectorize(disable)
        for (size_t i = 0; i < vertexCount; ++i) {
            const float3 v = mat4f::project(WLMpMv, mWsClippedShadowReceiverVolume[i]);
            lsLightFrustum.min.xy = min(lsLightFrustum.min.xy, v.xy);
            lsLightFrustum.max.xy = max(lsLightFrustum.max.xy, v.xy);
        }

        // For directional lights, we further constraint the light frustum to the
        // intersection of the shadow casters & receivers in light-space.
        // However, since this relies on the 1-texel shadow map border, this wouldn't directly
        // work if we were storing several shadow maps in a single texture.
        if (mEngine.debug.shadowmap.focus_shadowcasters) {
            intersectWithShadowCasters(lsLightFrustum, WLMpMv, wsShadowCastersVolume);
        }

        if (UTILS_UNLIKELY((lsLightFrustum.min.x >= lsLightFrustum.max.x) ||
                           (lsLightFrustum.min.y >= lsLightFrustum.max.y))) {
            // this could happen if the only thing visible is a perfectly horizontal or
            // vertical thin line
            mHasVisibleShadows = false;
            return;
        }

        assert(lsLightFrustum.min.x < lsLightFrustum.max.x);
        assert(lsLightFrustum.min.y < lsLightFrustum.max.y);

        // compute focus scale and offset
        float2 s = 2.0f / float2(lsLightFrustum.max.xy - lsLightFrustum.min.xy);
        float2 o =   -s * float2(lsLightFrustum.max.xy + lsLightFrustum.min.xy) * 0.5f;

        // temporal aliasing stabilization
        // 3) snap the light frustum (width & height) to texels, to stabilize the shadow map
        snapLightFrustum(s, o, mShadowMapDimension);

        // construct the Focus transform (scale + offset)
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
        const mat4f St = getTextureCoordsMapping(S);

        mTexelSizeWs = texelSizeWorldSpace(St, float3{ 0.5f });
        mLightSpace = St;
        mSceneRange = (zfar - znear);
        mCamera->setCustomProjection(mat4(S), znear, zfar);

        // for the debug camera, we need to undo the world origin
        mDebugCamera->setCustomProjection(mat4(S * camera.worldOrigin), znear, zfar);
    }
}

mat4f ShadowMap::applyLISPSM(CameraInfo const& camera, mat4f const& LMpMv,
        FrustumBoxIntersection const& wsShadowReceiversVolume, size_t vertexCount,
        float3 const& dir) {

    const float LoV = dot(camera.getForwardVector(), dir);
    const float sinLV = std::sqrt(1.0f - LoV * LoV);

    // Virtual near plane -- the default is 1m, can be changed by the user.
    // The virtual near plane prevents too much resolution to be wasted in the area near the eye
    // where shadows might not be visible (e.g. a character standing won't see shadows at her feet).
    const float dzn = camera.dzn;
    const float dzf = camera.dzf;

    // near/far plane's distance from the eye in view space of the shadow receiver volume
    float2 znf = -computeNearFar(camera.view, wsShadowReceiversVolume.data(), vertexCount);
    const float zn = znf[0]; // near plane distance from the eye
    const float zf = znf[1]; // far plane distance from the eye

    // compute n and f, the near and far planes coordinates of Wp (warp space).
    // It's found by looking down the Y axis in light space (i.e. -Z axis of Wp,
    // i.e. the axis orthogonal to the light direction) and taking the min/max
    // of the shadow receivers volume.
    // Note: znear/zfar encoded in Mp has no influence here (b/c we're interested only by the y axis)
    const float2 nf = computeWpNearFarOfWarpSpace(LMpMv, wsShadowReceiversVolume.data(), vertexCount);
    const float n = nf[0];              // near plane coordinate of Mp (light space)
    const float f = nf[1];              // far plane coordinate of Mp (light space)
    const float d = std::abs(f - n);    // Wp's depth-range d (abs necessary because we're dealing with z-coordinates, not distances)

    // The simplification below is correct only for directional lights
    const float z0 = zn;                // for directional lights, z0 = zn
    const float z1 = z0 + d * sinLV;    // btw, note that z1 doesn't depend on zf

    mat4f W;
    // see nopt1 below for an explanation about this test
    if (3.0f * (dzn / (zf - zn)) < 2.0f) {
        // nopt is the optimal near plane distance of Wp (i.e. distance from P).

        // virtual near and far planes
        const float vz0 = std::max(zn + dzn, z0);
        const float vz1 = std::min(zf - dzf, z1);

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

        const mat4f Wp = warpFrustum(nopt, nopt + d);
        const mat4f Wv = mat4f::translation(-p);
        W = Wp * Wv;
    }
    return W;
}


mat4f ShadowMap::getTextureCoordsMapping(math::mat4f const& S) const noexcept {
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

    // apply the 1-texel border viewport transform
    const float o = 1.0f / mShadowMapDimension;
    const float s = 1.0f - 2.0f * o;
    const mat4f Mb(mat4f::row_major_init{
             s, 0, 0, o,
             0, s, 0, o,
             0, 0, 1, 0,
             0, 0, 0, 1
    });

    // Compute shadow-map texture access transform
    const mat4f MbMt = Mb * Mt;

    const mat4f St = mat4f(MbMt * S);

    return St;
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

float2 ShadowMap::computeNearFar(const mat4f& lightView,
        Aabb const& wsShadowCastersVolume) noexcept {
    const Aabb::Corners wsSceneCastersCorners = wsShadowCastersVolume.getCorners();
    return computeNearFar(lightView, wsSceneCastersCorners.data(), wsSceneCastersCorners.size());
}

float2 ShadowMap::computeNearFar(const mat4f& lightView,
        float3 const* wsVertices, size_t count) noexcept {
    float2 nearFar = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() };
    #pragma nounroll
    for (size_t i = 0; i < count; i++) {
        float3 c = mat4f::project(lightView, wsVertices[i]);
        nearFar.x = std::max(nearFar.x, c.z);  // near
        nearFar.y = std::min(nearFar.y, c.z);  // far
    }
    return nearFar;
}

void ShadowMap::intersectWithShadowCasters(
        Aabb& UTILS_RESTRICT lightFrustum,
        mat4f const& lightView,
        Aabb const& wsShadowCastersVolume) noexcept {

    // construct the Focus transform (scale + offset)
    float2 s = 2.0f / float2(lightFrustum.max.xy - lightFrustum.min.xy);
    float2 o =   -s * float2(lightFrustum.max.xy + lightFrustum.min.xy) * 0.5f;
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
            Frustum(projection), wsLightFrustumCorners, wsShadowCastersVolume);

    // compute shadow-caster bounds in light space
    Aabb box;

    // disable vectorization here because vertexCount is <= 64, not worth the increased code size.
    #pragma clang loop vectorize(disable)
    for (size_t i = 0; i < vertexCount; ++i) {
        const float3 v(mat4f::project(lightView, wsClippedShadowCasterVolumeVertices[i]));
        box.min.xy = min(box.min.xy, v.xy);
        box.max.xy = max(box.max.xy, v.xy);
    }

    // intersect shadow-caster and current light frustum bounds
    lightFrustum.min.xy = max(box.min.xy, lightFrustum.min.xy);
    lightFrustum.max.xy = min(box.max.xy, lightFrustum.max.xy);
}

void ShadowMap::computeFrustumCorners(
        float3* UTILS_RESTRICT out,
        const mat4f& UTILS_RESTRICT projectionViewInverse) noexcept {

    // compute view frustum in world space (from its NDC)
    // matrix to convert: ndc -> camera -> world
    constexpr float3 csViewFrustumCorners[8] = {
            { -1, -1,  1 },
            {  1, -1,  1 },
            { -1,  1,  1 },
            {  1,  1,  1 },
            { -1, -1, -1 },
            {  1, -1, -1 },
            { -1,  1, -1 },
            {  1,  1, -1 },
    };
    for (float3 c : csViewFrustumCorners) {
        *out++ = mat4f::project(projectionViewInverse, c);
    }
}

float2 ShadowMap::computeWpNearFarOfWarpSpace(
        mat4f const& lightView,
        float3 const* wsViewFrustumCorners, size_t count) noexcept {
    float ymin = std::numeric_limits<float>::max();
    float ymax = std::numeric_limits<float>::lowest();
    #pragma nounroll
    for (size_t i = 0; i < count; i++) {
        float c = mat4f::project(lightView, wsViewFrustumCorners[i]).y;
        ymin = std::min(ymin, c);
        ymax = std::max(ymax, c);
    }
    // we're on the y axis in light space (looking down to +y)
    return { ymin, ymax };
}

size_t ShadowMap::intersectFrustumWithBox(
        FrustumBoxIntersection& UTILS_RESTRICT outVertices,
        const Frustum& UTILS_RESTRICT frustum, const float3* UTILS_RESTRICT wsFrustumCorners,
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

    // at this point if we have 8 vertices, we can skip the rest
    if (vertexCount < 8) {
        float4 const* wsFrustumPlanes = frustum.getNormalizedPlanes();

        // b) add the scene's vertices that are known to be inside the view frustum
        //
        // We need to handle the case where a corner of the box lies exactly on a plane of
        // the frustum. This actually happens often due to fitting light-space
        // We fudge the distance to the plane by a small amount.
        constexpr const float EPSILON = 1.0f / 8192.0f; // ~0.012 mm
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

        // at this point if we have 8 vertices, we can skip the segments intersection tests
        if (vertexCount < 8) {
            // c) intersect scene's volume edges with frustum planes
            vertexCount = intersectFrustums(outVertices.data(), vertexCount,
                    wsSceneReceiversCorners.vertices, wsFrustumCorners);

            // d) intersect frustum edges with the scene's volume planes
            vertexCount = intersectFrustums(outVertices.data(), vertexCount,
                    wsFrustumCorners, wsSceneReceiversCorners.vertices);
        }
    }

    assert(vertexCount <= outVertices.size());

    return vertexCount;
}

void ShadowMap::snapLightFrustum(float2& s, float2& o,
        uint32_t shadowMapDimension) noexcept {
    // TODO: we can also quantize the scaling value
    const float r = shadowMapDimension * 0.5f;
    o = ceil(o * r) / r;
}


constexpr const ShadowMap::Segment ShadowMap::sBoxSegments[12];
constexpr const ShadowMap::Quad ShadowMap::sBoxQuads[6];

UTILS_NOINLINE
size_t ShadowMap::intersectFrustums(
        float3* UTILS_RESTRICT out,
        size_t vertexCount,
        float3 const* segmentsVertices,
        float3 const* quadsVertices) noexcept {

#pragma nounroll
    for (const Segment segment : sBoxSegments) {
        const float3 s0 = segmentsVertices[segment.v0];
        const float3 s1 = segmentsVertices[segment.v1];
        // each segment should only intersect with 2 quads at most
        size_t maxVertexCount = vertexCount + 2;
        for (size_t j = 0; j < 6 && vertexCount < maxVertexCount; ++j) {
            const Quad quad = sBoxQuads[j];
            const float3 t0 = quadsVertices[quad.v0];
            const float3 t1 = quadsVertices[quad.v1];
            const float3 t2 = quadsVertices[quad.v2];
            const float3 t3 = quadsVertices[quad.v3];
            if (intersectSegmentWithPlanarQuad(out[vertexCount], s0, s1, t0, t1, t2, t3)) {
                vertexCount++;
            }
        }
    }
    return vertexCount;
}

UTILS_ALWAYS_INLINE
bool ShadowMap::intersectSegmentWithPlanarQuad(float3& UTILS_RESTRICT p,
        float3 s0, float3 s1,
        float3 t0, float3 t1, float3 t2, float3 t3) noexcept {
    constexpr float EPSILON = std::numeric_limits<float>::epsilon();
    const float3 u = t1 - t0;
    const float3 v = t2 - t0;
    const float3 pn = cross(u, v);
    if (!intersectSegmentWithPlane(p, s0, s1, pn, t0)) {
        return false;
    }

    // check if the segment intersects the first triangle
    const float uu = dot(u, u);
    const float uv = dot(u, v);
    const float vv = dot(v, v);
    const float ua = uv * uv - uu * vv;
    if (UTILS_UNLIKELY(std::fabs(ua) < EPSILON)) {
        // degenerate triangle
        return false;
    }

    const float3 k = p - t0;
    const float ku = dot(k, u);
    const float kv = dot(k, v);
    float D = 1.0f / ua;
    float s = (uv * kv - vv * ku) * D;
    float t = (uv * ku - uu * kv) * D;
    if ((s >= 0.0f && s <= 1.0f) && (t >= 0.0f && (s + t) <= 1.0f)) {
        return true;
    }


    // if not, check the second triangle
    const float3 w = t3 - t0;
    const float ww = dot(w, w);
    const float wv = dot(w, v);
    const float wa = wv * wv - ww * vv;
    if (UTILS_UNLIKELY(std::fabs(wa) < EPSILON)) {
        // degenerate triangle
        return false;
    }

    const float kw = dot(k, w);
    D = 1.0f / wa;
    s = (wv * kv - vv * kw) * D;
    t = (wv * kw - ww * kv) * D;
    return (s >= 0.0f && s <= 1.0f) && (t >= 0.0f && (s + t) <= 1.0f);
}

UTILS_ALWAYS_INLINE
bool ShadowMap::intersectSegmentWithPlane(float3& UTILS_RESTRICT p,
        float3 s0, float3 s1,
        float3 pn, float3 p0) noexcept {
    constexpr const float EPSILON = 1.0f / 8192.0f; // ~0.012 mm
    const float3 d = s1 - s0;
    const float n = dot(pn, d);
    if (std::fabs(n) >= EPSILON) {
        const float t = -(dot(pn, s0 - p0)) / n;
        if (t >= 0.0f && t <= 1.0f) {
            p = s0 + t * d;
            return true;
        }
    }
    return false;
}

float ShadowMap::texelSizeWorldSpace(const mat4f& lightSpaceMatrix) const noexcept {
    // this version works only for orthographic projections
    const mat3f shadowmapToWorldMatrix(inverse(lightSpaceMatrix.upperLeft()));
    const float3 texelSizeWs = shadowmapToWorldMatrix * float3{ 1, 1, 0 };
    const float s = length(texelSizeWs) / mShadowMapDimension;
    return s;
}

float ShadowMap::texelSizeWorldSpace(const mat4f& lightSpaceMatrix, float3 const& str) const noexcept {
    // for non-orthographic projection, the projection of a texel in world-space is not constant
    // therefore we need to specify which texel we want to back-project.
    const mat4f shadowmapToWorldMatrix(inverse(lightSpaceMatrix));
    const float3 p0 = mat4f::project(shadowmapToWorldMatrix, str);
    const float3 p1 = mat4f::project(shadowmapToWorldMatrix, str + float3{ 1, 1, 0 } / mShadowMapDimension);
    const float s = length(p1 - p0);
    return s;
}

} // namespace details
} // namespace filament
