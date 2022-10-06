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

#include "backend/DriverApiForward.h"
#include "private/backend/SamplerGroup.h"
#include "math/mat4.h"

#include <math/mat4.h>
#include <math/vec4.h>

namespace filament {

class RenderPass;

// The value of the 'VISIBLE_MASK' after culling. Each bit represents visibility in a frustum
// (either camera or light).
//
//                                    1
// bits                               5 ... 7 6 5 4 3 2 1 0
// +------------------------------------------------------+
// VISIBLE_RENDERABLE                                     X
// VISIBLE_DIR_SHADOW_RENDERABLE                        X
// VISIBLE_DYN_SHADOW_RENDERABLE                      X

// A "shadow renderable" is a renderable rendered to the shadow map during a shadow pass:
// PCF shadows: only shadow casters
// VSM shadows: both shadow casters and shadow receivers

static constexpr size_t VISIBLE_RENDERABLE_BIT              = 0u;
static constexpr size_t VISIBLE_DIR_SHADOW_RENDERABLE_BIT   = 1u;
static constexpr size_t VISIBLE_DYN_SHADOW_RENDERABLE_BIT   = 2u;

static constexpr Culler::result_type VISIBLE_DIR_SHADOW_RENDERABLE = 1u << VISIBLE_DIR_SHADOW_RENDERABLE_BIT;
static constexpr Culler::result_type VISIBLE_DYN_SHADOW_RENDERABLE = 1u << VISIBLE_DYN_SHADOW_RENDERABLE_BIT;

class ShadowMap {
public:

    enum class ShadowType : uint8_t {
        DIRECTIONAL,
        SPOT,
        POINT
    };

    explicit ShadowMap(FEngine& engine) noexcept;

    // ShadowMap is not copyable for now
    ShadowMap(ShadowMap const& rhs) = delete;
    ShadowMap& operator=(ShadowMap const& rhs) = delete;

    ~ShadowMap();

    void terminate(FEngine& engine);

    struct ShadowMapInfo {
        // the dimension of the encompassing texture atlas
        uint16_t atlasDimension = 0;

        // the dimension of a single shadow map texture within the atlas
        // e.g., for at atlas size of 1024 split into 4 quadrants, textureDimension would be 512
        uint16_t textureDimension = 0;

        // the dimension of the actual shadow map, taking into account the 1 texel border
        // e.g., for a texture dimension of 512, shadowDimension would be 510
        uint16_t shadowDimension = 0;

        // e.g. Vulkan clip-space
        bool clipSpaceFlipped = false;

        // e.g. metal textures
        bool textureSpaceFlipped = false;

        // whether we're using vsm
        bool vsm = false;
    };

    struct SceneInfo {
        // scratch data: The near and far planes, in clip space, to use for this shadow map
        math::float2 csNearFar = { -1.0f, 1.0f };

        // scratch data: light's near/far expressed in light-space, calculated from the scene's
        // content assuming the light is at the origin.
        math::float2 lsNearFar{};

        // scratch data: Viewing camera's near/far expressed in view-space, calculated from the
        // scene's content.
        math::float2 vsNearFar{};

        // World-space shadow-casters volume
        Aabb wsShadowCastersVolume;

        // World-space shadow-receivers volume
        Aabb wsShadowReceiversVolume;

        uint8_t visibleLayers;
    };

    static math::mat4f getDirectionalLightViewMatrix(
            math::float3 direction, math::float3 position = {}) noexcept;

    static math::mat4f getPointLightViewMatrix(backend::TextureCubemapFace face,
            math::float3 position) noexcept;

    void initialize(size_t lightIndex, ShadowType shadowType, uint16_t shadowIndex,
            LightManager::ShadowOptions const* options);

    struct ShaderParameters {
        math::mat4f lightSpace{};
        math::float4 lightFromWorldZ{};
        float texelSizeAtOneMeterWs{};
    };

    // Call once per frame if the light, scene (or visible layers) or camera changes.
    // This computes the light's camera.
    ShaderParameters updateDirectional(FEngine& engine,
            const FScene::LightSoa& lightData, size_t index,
            filament::CameraInfo const& camera,
            ShadowMapInfo const& shadowMapInfo, FScene const& scene,
            SceneInfo& sceneInfo) noexcept;

    ShaderParameters updateSpot(FEngine& engine,
            const FScene::LightSoa& lightData, size_t index,
            filament::CameraInfo const& camera,
            const ShadowMapInfo& shadowMapInfo, FScene const& scene,
            SceneInfo sceneInfo) noexcept;

    ShaderParameters updatePoint(FEngine& engine,
            const FScene::LightSoa& lightData, size_t index,
            filament::CameraInfo const& camera, const ShadowMapInfo& shadowMapInfo,
            FScene const& scene,
            SceneInfo sceneInfo, uint8_t face) noexcept;

    // Do we have visible shadows. Valid after calling update().
    bool hasVisibleShadows() const noexcept { return mHasVisibleShadows; }

    // Returns the light's projection. Valid after calling update().
    FCamera const& getCamera() const noexcept { return *mCamera; }

    // use only for debugging
    FCamera const& getDebugCamera() const noexcept { return *mDebugCamera; }

    // Call once per frame to populate the SceneInfo struct, then pass to update().
    // This computes values constant across all shadow maps.
    static void initSceneInfo(ShadowMap::SceneInfo& sceneInfo, uint8_t visibleLayers,
            FScene const& scene, math::mat4f const& viewMatrix);

    // Update SceneInfo struct for a given light
    static void updateSceneInfoDirectional(const math::mat4f& Mv, FScene const& scene,
            SceneInfo& sceneInfo);

    static void updateSceneInfoSpot(const math::mat4f& Mv, FScene const& scene,
            SceneInfo& sceneInfo);

    LightManager::ShadowOptions const* getShadowOptions() const noexcept { return mOptions; }
    size_t getLightIndex() const { return mLightIndex; }
    uint16_t getShadowIndex() const { return mShadowIndex; }
    void setLayer(uint8_t layer) noexcept { mLayer = layer; }
    uint8_t getLayer() const noexcept { return mLayer; }
    bool isDirectionalShadow() const noexcept { return mShadowType == ShadowType::DIRECTIONAL; }
    bool isSpotShadow() const noexcept { return mShadowType == ShadowType::SPOT; }
    bool isPointShadow() const noexcept { return mShadowType == ShadowType::POINT; }
    ShadowType getShadowType() const noexcept { return mShadowType; }

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
            Frustum const& wsFrustum,
            math::float3 const* wsFrustumCorners,
            Aabb const& wsBox);

    static math::mat4f warpFrustum(float n, float f) noexcept;

    static math::mat4f directionalLightFrustum(float n, float f) noexcept;

    static math::mat4 getTextureCoordsMapping(ShadowMapInfo const& info) noexcept;

    static math::mat4f computeVsmLightSpaceMatrix(const math::mat4f& lightSpacePcf,
            const math::mat4f& Mv, float znear, float zfar) noexcept;

    float texelSizeWorldSpace(const math::mat3f& worldToShadowTexture,
            uint16_t shadowDimension) const noexcept;

    float texelSizeWorldSpace(const math::mat4f& W, const math::mat4f& MbMtF,
            uint16_t shadowDimension) const noexcept;

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

    // The data below technically belongs to ShadowMapManager, but it simplifies allocations
    // to store it here. This data is always associated with this shadow map anyway.
    LightManager::ShadowOptions const* mOptions = nullptr;                  // 8
    uint32_t mLightIndex = 0;   // which light are we shadowing             // 4
    uint16_t mShadowIndex = 0;  // our index in the shadowMap vector        // 2
    uint8_t mLayer = 0;         // our layer in the shadowMap texture       // 1
    ShadowType mShadowType : 2;
    bool mHasVisibleShadows : 2;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SHADOWMAP_H
