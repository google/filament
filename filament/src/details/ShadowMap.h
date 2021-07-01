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

#ifndef TNT_FILAMENT_DETAILS_SHADOWMAP_H
#define TNT_FILAMENT_DETAILS_SHADOWMAP_H

#include "components/LightManager.h"

#include "details/Camera.h"
#include "details/Scene.h"

#include "private/backend/DriverApiForward.h"
#include "private/backend/SamplerGroup.h"

#include <filament/Viewport.h>

#include <math/mat4.h>
#include <math/vec4.h>

namespace filament {

class FView;
class RenderPass;

class ShadowMap {
public:
    explicit ShadowMap(FEngine& engine) noexcept;
    ~ShadowMap();

    struct ShadowMapInfo {
        // the smallest increment in depth precision
        // e.g., for 16 bit depth textures, is this 1 / (2^16)
        float zResolution = 0.0f;

        // the dimension of the encompassing texture atlas
        size_t atlasDimension = 0;

        // the dimension of a single shadow map texture within the atlas
        // e.g., for at atlas size of 1024 split into 4 quadrants, textureDimension would be 512
        size_t textureDimension = 0;

        // the dimension of the actual shadow map, taking into account the 1 texel border
        // e.g., for a texture dimension of 512, shadowDimension would be 510
        size_t shadowDimension = 0;

        // whether we're using vsm
        bool vsm = false;
    };

    struct SceneInfo {
        // The near and far planes, in clip space, to use for this shadow map
        math::float2 csNearFar = { -1.0f, 1.0f };

        // The following fields are set by computeSceneCascadeParams.

        // light's near/far expressed in light-space, calculated from the scene's content
        // assuming the light is at the origin.
        math::float2 lsNearFar;

        // Viewing camera's near/far expressed in view-space, calculated from the scene's content
        math::float2 vsNearFar;

        // World-space shadow-casters volume
        Aabb wsShadowCastersVolume;

        // World-space shadow-receivers volume
        Aabb wsShadowReceiversVolume;
    };

    static math::mat4f getLightViewMatrix(
            math::float3 position, math::float3 direction) noexcept;

    // Call once per frame to populate the CascadeParameters struct, then pass to update().
    // This computes values constant across all cascades.
    static void computeSceneInfo(math::float3 dir,
            FScene const& scene, filament::CameraInfo const& camera, uint8_t visibleLayers,
            SceneInfo& sceneInfo);

    // Call once per frame if the light, scene (or visible layers) or camera changes.
    // This computes the light's camera.
    void update(const FScene::LightSoa& lightData, size_t index,
            filament::CameraInfo const& camera,
            const ShadowMapInfo& shadowMapInfo, const SceneInfo& cascadeParams) noexcept;

    void render(backend::DriverApi& driver, utils::Range<uint32_t> const& range,
            RenderPass* pass, FView& view) noexcept;

    // Do we have visible shadows. Valid after calling update().
    bool hasVisibleShadows() const noexcept { return mHasVisibleShadows; }

    // Computes the transform to use in the shader to access the shadow map.
    // Valid after calling update().
    math::mat4f const& getLightSpaceMatrix() const noexcept { return mLightSpace; }

    // return the size of a texel in world space (pre-warping)
    float getTexelSizeWorldSpace() const noexcept { return mTexelSizeWs; }

    // Returns the light's projection. Valid after calling update().
    FCamera const& getCamera() const noexcept { return *mCamera; }

    // use only for debugging
    FCamera const& getDebugCamera() const noexcept { return *mDebugCamera; }

    backend::PolygonOffset getPolygonOffset() const noexcept { return mPolygonOffset; }

private:
    struct CameraInfo {
        math::mat4f projection;
        math::mat4f model;
        math::mat4f view;
        math::mat4f worldOrigin;
        float zn = 0;
        float zf = 0;
        Frustum frustum;
        float getNear() const noexcept { return zn; }
        float getFar() const noexcept { return zf; }
        math::float3 const& getPosition() const noexcept { return model[3].xyz; }
        math::float3 getForwardVector() const noexcept {
            return -normalize(model[2].xyz);   // the camera looks towards -z
        }
    };

    struct Segment {
        uint8_t v0, v1;
    };

    struct Quad {
        uint8_t v0, v1, v2, v3;
    };

    // 8 corners, 12 segments w/ 2 intersection max -- all of this twice (8 + 12 * 2) * 2 (768 bytes)
    using FrustumBoxIntersection = std::array<math::float3, 64>;

    void computeShadowCameraDirectional(
            math::float3 const& direction,
            CameraInfo const& camera, FLightManager::ShadowParams const& params,
            SceneInfo cascadeParams) noexcept;
    void computeShadowCameraSpot(math::float3 const& position, math::float3 const& dir,
            float outerConeAngle, float radius, CameraInfo const& camera,
            FLightManager::ShadowParams const& params) noexcept;

    static math::mat4f applyLISPSM(math::mat4f& Wp,
            CameraInfo const& camera, FLightManager::ShadowParams const& params,
            const math::mat4f& LMpMv,
            FrustumBoxIntersection const& wsShadowReceiverVolume, size_t vertexCount,
            const math::float3& dir);

    static inline void snapLightFrustum(math::float2& s, math::float2& o,
            math::mat4f const& Mv, math::float3 worldOrigin, math::float2 shadowMapResolution) noexcept;

    static inline void computeFrustumCorners(math::float3* out,
            const math::mat4f& projectionViewInverse, math::float2 csNearFar = { -1.0f, 1.0f }) noexcept;

    static inline math::float2 computeNearFar(math::mat4f const& view,
            Aabb const& wsShadowCastersVolume) noexcept;

    static inline math::float2 computeNearFar(math::mat4f const& view,
            math::float3 const* wsVertices, size_t count) noexcept;

    static inline math::float4 computeBoundingSphere(
            math::float3 const* vertices, size_t count) noexcept;

    template<typename Casters, typename Receivers>
    static void visitScene(FScene const& scene, uint32_t visibleLayers,
            Casters casters, Receivers receivers) noexcept;

    static inline Aabb compute2DBounds(const math::mat4f& lightView,
            math::float3 const* wsVertices, size_t count) noexcept;

    static inline Aabb compute2DBounds(const math::mat4f& lightView,
            math::float4 const& sphere) noexcept;

    static inline void intersectWithShadowCasters(Aabb& lightFrustum, const math::mat4f& lightView,
            Aabb const& wsShadowCastersVolume) noexcept;

    static inline math::float2 computeNearFarOfWarpSpace(math::mat4f const& lightView,
            math::float3 const* wsVertices, size_t count) noexcept;

    static inline bool intersectSegmentWithPlanarQuad(math::float3& p,
            math::float3 s0, math::float3 s1,
            math::float3 t0, math::float3 t1,
            math::float3 t2, math::float3 t3) noexcept;

    static inline bool intersectSegmentWithTriangle(math::float3& UTILS_RESTRICT p,
            math::float3 s0, math::float3 s1,
            math::float3 t0, math::float3 t1, math::float3 t2) noexcept;

    static size_t intersectFrustum(math::float3* out, size_t vertexCount,
            math::float3 const* segmentsVertices, math::float3 const* quadsVertices) noexcept;

    static size_t intersectFrustumWithBox(
            FrustumBoxIntersection& outVertices,
            const math::float3* wsFrustumCorners,
            Aabb const& wsBox);

    static math::mat4f warpFrustum(float n, float f) noexcept;

    static math::mat4f directionalLightFrustum(float n, float f) noexcept;

    math::mat4f getTextureCoordsMapping() const noexcept;

    static math::mat4f computeVsmLightSpaceMatrix(const math::mat4f& lightSpace,
            const math::mat4f& Mv, float zfar) noexcept;

    float texelSizeWorldSpace(const math::mat3f& worldToShadowTexture) const noexcept;
    float texelSizeWorldSpace(const math::mat4f& W, const math::mat4f& MbMtF) const noexcept;

    static constexpr const Segment sBoxSegments[12] = {
            { 0, 1 }, { 1, 3 }, { 3, 2 }, { 2, 0 },
            { 4, 5 }, { 5, 7 }, { 7, 6 }, { 6, 4 },
            { 0, 4 }, { 1, 5 }, { 3, 7 }, { 2, 6 },
    };
    static constexpr const Quad sBoxQuads[6] = {
            { 2, 0, 1, 3 },  // far
            { 6, 4, 5, 7 },  // near
            { 2, 0, 4, 6 },  // left
            { 3, 1, 5, 7 },  // right
            { 0, 4, 5, 1 },  // bottom
            { 2, 6, 7, 3 },  // top
    };

    FCamera* mCamera = nullptr;
    FCamera* mDebugCamera = nullptr;
    math::mat4f mLightSpace;
    float mTexelSizeWs = 0.0f;

    // set-up in update()
    ShadowMapInfo mShadowMapInfo;
    bool mHasVisibleShadows = false;
    backend::PolygonOffset mPolygonOffset{};

    // use a member here (instead of stack) because we don't want to pay the
    // initialization of the float3 each time
    FrustumBoxIntersection mWsClippedShadowReceiverVolume;

    FEngine& mEngine;
    const bool mClipSpaceFlipped;
    const bool mTextureSpaceFlipped;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SHADOWMAP_H
