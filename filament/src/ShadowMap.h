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

#include <math/mat4.h>
#include <math/vec4.h>

namespace filament {

class FView;
class RenderPass;

class ShadowMap {
public:
    explicit ShadowMap(FEngine& engine) noexcept;

    // ShadowMap is not copyable for now
    ShadowMap(ShadowMap const& rhs) = delete;
    ShadowMap& operator=(ShadowMap const& rhs) = delete;

    ~ShadowMap();

    struct ShadowMapInfo {
        // the dimension of the encompassing texture atlas
        uint16_t atlasDimension = 0;

        // the dimension of a single shadow map texture within the atlas
        // e.g., for at atlas size of 1024 split into 4 quadrants, textureDimension would be 512
        uint16_t textureDimension = 0;

        // the dimension of the actual shadow map, taking into account the 1 texel border
        // e.g., for a texture dimension of 512, shadowDimension would be 510
        uint16_t shadowDimension = 0;

        // This spot shadowmap index.
        uint16_t spotIndex = 0;

        // whether we're using vsm
        bool vsm = false;

        // polygon offset
        backend::PolygonOffset polygonOffset{};
    };

    struct SceneInfo {
        explicit SceneInfo(uint8_t visibleLayers) noexcept : visibleLayers(visibleLayers) { }

        // The near and far planes, in clip space, to use for this shadow map
        math::float2 csNearFar = { -1.0f, 1.0f };

        // The following fields are set by computeSceneCascadeParams.

        // light's near/far expressed in light-space, calculated from the scene's content
        // assuming the light is at the origin.
        math::float2 lsNearFar{};

        // Viewing camera's near/far expressed in view-space, calculated from the scene's content
        math::float2 vsNearFar{};

        // World-space shadow-casters volume
        Aabb wsShadowCastersVolume;

        // World-space shadow-receivers volume
        Aabb wsShadowReceiversVolume;

        uint8_t visibleLayers;
    };

    static math::mat4f getDirectionalLightViewMatrix(
            math::float3 direction, math::float3 position = {}) noexcept;

    // Call once per frame if the light, scene (or visible layers) or camera changes.
    // This computes the light's camera.
    void updateDirectional(const FScene::LightSoa& lightData, size_t index,
            filament::CameraInfo const& camera,
            const ShadowMapInfo& shadowMapInfo, FScene const& scene,
            SceneInfo& sceneInfo) noexcept;

    void updateSpot(const FScene::LightSoa& lightData, size_t index,
            filament::CameraInfo const& camera,
            const ShadowMapInfo& shadowMapInfo, FScene const& scene,
            SceneInfo& sceneInfo) noexcept;

    void render(FScene const& scene, utils::Range<uint32_t> range,
            FScene::VisibleMaskType visibilityMask, filament::CameraInfo const& cameraInfo,
            RenderPass* pass) noexcept;

    // Do we have visible shadows. Valid after calling update().
    bool hasVisibleShadows() const noexcept { return mHasVisibleShadows; }

    // Computes the transform to use in the shader to access the shadow map.
    // Valid after calling update().
    math::mat4f const& getLightSpaceMatrix() const noexcept { return mLightSpace; }

    // return the size of a texel in world space (pre-warping)
    float getTexelSizAtOneMeterWs() const noexcept { return mTexelSizeAtOneMeterWs; }

    // Returns the light's projection. Valid after calling update().
    FCamera const& getCamera() const noexcept { return *mCamera; }

    // use only for debugging
    FCamera const& getDebugCamera() const noexcept { return *mDebugCamera; }

    backend::PolygonOffset getPolygonOffset() const noexcept { return mShadowMapInfo.polygonOffset; }

    // Call once per frame to populate the SceneInfo struct, then pass to update().
    // This computes values constant across all shadow maps.
    static void initSceneInfo(FScene const& scene, filament::CameraInfo const& camera,
            ShadowMap::SceneInfo& sceneInfo);

    // Update SceneInfo struct for a given light
    static void updateSceneInfo(const math::mat4f& Mv, FScene const& scene,
            ShadowMap::SceneInfo& sceneInfo);

    static void updateSceneInfo(const math::mat4f& Mv, FScene const& scene,
            ShadowMap::SceneInfo& sceneInfo, uint16_t index);

private:
    struct Segment {
        uint8_t v0, v1;
    };

    struct Quad {
        uint8_t v0, v1, v2, v3;
    };

    // 8 corners, 12 segments w/ 2 intersection max -- all of this twice (8 + 12 * 2) * 2 (768 bytes)
    using FrustumBoxIntersection = std::array<math::float3, 64>;

    static math::mat4f applyLISPSM(math::mat4f& Wp,
            filament::CameraInfo const& camera, FLightManager::ShadowParams const& params,
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

    math::mat4 getTextureCoordsMapping() const noexcept;

    static math::mat4f computeVsmLightSpaceMatrix(const math::mat4f& lightSpacePcf,
            const math::mat4f& Mv, float znear, float zfar) noexcept;

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

    FCamera* mCamera = nullptr;                 //  8
    FCamera* mDebugCamera = nullptr;            //  8
    math::mat4f mLightSpace;                    // 64
    float mTexelSizeAtOneMeterWs = 0.0f;        //  4

    // set-up in update()
    ShadowMapInfo mShadowMapInfo;               // 20
    bool mHasVisibleShadows = false;            //  1

    FEngine& mEngine;                           //  8
    const bool mClipSpaceFlipped;               //  1
    const bool mTextureSpaceFlipped;            //  1
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SHADOWMAP_H
