/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "details/Scene.h"

#include "components/LightManager.h"
#include "components/RenderableManager.h"

#include <private/filament/UibGenerator.h>

#include "details/Culler.h"
#include "details/Engine.h"
#include "details/IndirectLight.h"
#include "details/Skybox.h"

#include <utils/compiler.h>
#include <utils/EntityManager.h>
#include <utils/Range.h>
#include <utils/Zip2Iterator.h>

#include <algorithm>

using namespace filament::math;
using namespace utils;

namespace filament {
namespace details {

// ------------------------------------------------------------------------------------------------

FScene::FScene(FEngine& engine) :
        mEngine(engine),
        mIndirectLight(engine.getDefaultIndirectLight()) {
}

FScene::~FScene() noexcept = default;


void FScene::prepare(const filament::math::mat4f& worldOriginTransform) {
    // TODO: can we skip this in most cases? Since we rely on indices staying the same,
    //       we could only skip, if nothing changed in the RCM.

    FEngine& engine = mEngine;
    EntityManager& em = engine.getEntityManager();
    FRenderableManager& rcm = engine.getRenderableManager();
    FTransformManager& tcm = engine.getTransformManager();
    FLightManager& lcm = engine.getLightManager();
    // go through the list of entities, and gather the data of those that are renderables
    auto& sceneData = mRenderableData;
    auto& lightData = mLightData;
    auto const& entities = mEntities;


    // NOTE: we can't know in advance how many entities are renderable or lights because the corresponding
    // component can be added after the entity is added to the scene.

    size_t renderableDataCapacity = entities.size();
    // we need the capacity to be multiple of 16 for SIMD loops
    renderableDataCapacity = (renderableDataCapacity + 0xF) & ~0xF;
    // we need 1 extra entry at the end for the summed primitive count
    renderableDataCapacity = renderableDataCapacity + 1;

    sceneData.clear();
    if (sceneData.capacity() < renderableDataCapacity) {
        sceneData.setCapacity(renderableDataCapacity);
    }

    // The light data list will always contain at least one entry for the
    // dominating directional light, even if there are no entities.
    size_t lightDataCapacity = std::max<size_t>(1, entities.size());
    // we need the capacity to be multiple of 16 for SIMD loops
    lightDataCapacity = (lightDataCapacity + 0xF) & ~0xF;

    lightData.clear();
    if (lightData.capacity() < lightDataCapacity) {
        lightData.setCapacity(lightDataCapacity);
    }
    // the first entries are reserved for the directional lights (currently only one)
    lightData.resize(DIRECTIONAL_LIGHTS_COUNT);


    // find the max intensity directional light index in our local array
    float maxIntensity = 0;

    for (Entity e : entities) {
        if (!em.isAlive(e))
            continue;

        // getInstance() always returns null if the entity is the Null entity
        // so we don't need to check for that, but we need to check it's alive
        auto ri = rcm.getInstance(e);
        auto li = lcm.getInstance(e);
        if (!ri & !li)
            continue;

        // get the world transform
        auto ti = tcm.getInstance(e);
        const mat4f worldTransform = worldOriginTransform * tcm.getWorldTransform(ti);

        // don't even draw this object if it doesn't have a transform (which shouldn't happen
        // because one is always created when creating a Renderable component).
        if (ri && ti) {
            // compute the world AABB so we can perform culling
            const Box worldAABB = rigidTransform(rcm.getAABB(ri), worldTransform);

            // we know there is enough space in the array
            sceneData.push_back_unsafe(
                    ri,
                    worldTransform,
                    rcm.getVisibility(ri),
                    rcm.getBonesUbh(ri),
                    worldAABB.center,
                    0,
                    rcm.getLayerMask(ri),
                    worldAABB.halfExtent,
                    {}, {});
        }

        if (li) {
            // find the dominant directional light
            if (UTILS_UNLIKELY(lcm.isDirectionalLight(li))) {
                // we don't store the directional lights, because we only have a single one
                if (lcm.getIntensity(li) >= maxIntensity) {
                    float3 d = lcm.getLocalDirection(li);
                    // using the inverse-transpose handles non-uniform scaling
                    d = normalize(transpose(inverse(worldTransform.upperLeft())) * d);
                    lightData.elementAt<FScene::POSITION_RADIUS>(0) = float4{ 0, 0, 0, std::numeric_limits<float>::infinity() };
                    lightData.elementAt<FScene::DIRECTION>(0)       = d;
                    lightData.elementAt<FScene::LIGHT_INSTANCE>(0)  = li;
                }
            } else {
                const float4 p = worldTransform * float4{ lcm.getLocalPosition(li), 1 };
                float3 d = 0;
                if (!lcm.isPointLight(li) || lcm.isIESLight(li)) {
                    d = lcm.getLocalDirection(li);
                    // using the inverse-transpose handles non-uniform scaling
                    d = normalize(transpose(inverse(worldTransform.upperLeft())) * d);
                }
                lightData.push_back_unsafe(
                        float4{ p.xyz, lcm.getRadius(li) }, d, li, {}, {});
            }
        }
    }

    // some elements past the end of the array will be accessed by SIMD code, we need to make
    // sure the data is valid enough as not to produce errors such as divide-by-zero
    // (e.g. in computeLightRanges())
    for (size_t i = lightData.size(), e = (lightData.size() + 3) & ~3; i < e; i++) {
        new(lightData.data<POSITION_RADIUS>() + i) float4{ 0, 0, 0, 1 };
    }
}

void FScene::updateUBOs(utils::Range<uint32_t> visibleRenderables, Handle<HwUniformBuffer> renderableUbh) noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    const size_t size = visibleRenderables.size() * sizeof(PerRenderableUib);

    // allocate space into the command stream directly
    void* const buffer = driver.allocate(size);

    auto& sceneData = mRenderableData;
    for (uint32_t i : visibleRenderables) {
        mat4f const& model = sceneData.elementAt<WORLD_TRANSFORM>(i);
        const size_t offset = i * sizeof(PerRenderableUib);

        UniformBuffer::setUniform(buffer,
                offset + offsetof(PerRenderableUib, worldFromModelMatrix),
                model);

        // Using the inverse-transpose handles non-uniform scaling, but DOESN'T guarantee that
        // the transformed normals will have unit-length, therefore they need to be normalized
        // in the shader (that's already the case anyways, since normalization is needed after
        // interpolation).
        //
        // We pre-scale normals by the inverse of the largest scale factor to avoid
        // large post-transform magnitudes in the shader, especially in the fragment shader, where
        // we use medium precision.
        //
        // Note: if the model matrix is known to be a rigid-transform, we could just use it directly.

        mat3f m = transpose(inverse(model.upperLeft()));
        m *= mat3f(1.0f / std::sqrt(max(float3{length2(m[0]), length2(m[1]), length2(m[2])})));

        UniformBuffer::setUniform(buffer,
                offset + offsetof(PerRenderableUib, worldFromModelNormalMatrix), m);
    }

    // TODO: handle static objects separately
    mRenderableViewUbh = renderableUbh;
    driver.updateUniformBuffer(renderableUbh, { buffer, size });
}

void FScene::terminate(FEngine& engine) {
    // DO NOT destroy this UBO, it's owned by the View
    mRenderableViewUbh.clear();
}

void FScene::prepareDynamicLights(const CameraInfo& camera, ArenaScope& rootArena, Handle<HwUniformBuffer> lightUbh) noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    FLightManager& lcm = mEngine.getLightManager();
    FScene::LightSoa& lightData = getLightData();

    /*
     * Here we copy our lights data into the GPU buffer, some lights might be left out if there
     * are more than the GPU buffer allows (i.e. 256).
     *
     * We always sort lights by distance to the camera plane so that:
     * - we can build light trees
     * - lights farther from the camera are dropped when in excess
     *   (note this doesn't work well, e.g. for search-lights)
     */

    ArenaScope arena(rootArena.getAllocator());
    float* const UTILS_RESTRICT distances = arena.allocate<float>(lightData.size(), CACHELINE_SIZE);

    // pre-compute the lights' distance to the camera plane, for sorting below
    // - we don't skip the directional light, because we don't care, it's ignored during sorting
    float4 const* const UTILS_RESTRICT spheres = lightData.data<FScene::POSITION_RADIUS>();
    computeLightCameraPlaneDistances(distances, camera, spheres, lightData.size());

    // skip directional light
    Zip2Iterator<FScene::LightSoa::iterator, float*> b = { lightData.begin(), distances };
    std::sort(b + DIRECTIONAL_LIGHTS_COUNT, b + lightData.size(),
            [](auto const& lhs, auto const& rhs) { return lhs.second < rhs.second; });

    // drop excess lights
    lightData.resize(std::min(lightData.size(), CONFIG_MAX_LIGHT_COUNT + DIRECTIONAL_LIGHTS_COUNT));

    // number of point/spot lights
    size_t positionalLightCount = lightData.size() - DIRECTIONAL_LIGHTS_COUNT;

    // compute the light ranges (needed when building light trees)
    float2* const zrange = lightData.data<FScene::SCREEN_SPACE_Z_RANGE>();
    computeLightRanges(zrange, camera, spheres + DIRECTIONAL_LIGHTS_COUNT, positionalLightCount);

    LightsUib* const lp = driver.allocatePod<LightsUib>(positionalLightCount);

    auto const* UTILS_RESTRICT directions   = lightData.data<FScene::DIRECTION>();
    auto const* UTILS_RESTRICT instances    = lightData.data<FScene::LIGHT_INSTANCE>();
    for (size_t i = DIRECTIONAL_LIGHTS_COUNT, c = lightData.size(); i < c; ++i) {
        const size_t gpuIndex = i - DIRECTIONAL_LIGHTS_COUNT;
        auto li = instances[i];
        lp[gpuIndex].positionFalloff      = { spheres[i].xyz, lcm.getSquaredFalloffInv(li) };
        lp[gpuIndex].colorIntensity       = { lcm.getColor(li), lcm.getIntensity(li) };
        lp[gpuIndex].directionIES         = { directions[i], 0 };
        lp[gpuIndex].spotScaleOffset.xy   = { lcm.getSpotParams(li).scaleOffset };
    }

    driver.updateUniformBuffer(lightUbh, { lp, positionalLightCount * sizeof(LightsUib) });
}

// These methods need to exist so clang honors the __restrict__ keyword, which in turn
// produces much better vectorization. The ALWAYS_INLINE keyword makes sure we actually don't
// pay the price of the call!
UTILS_ALWAYS_INLINE
void FScene::computeLightCameraPlaneDistances(
        float* UTILS_RESTRICT const distances,
        CameraInfo const& UTILS_RESTRICT camera,
        float4 const* UTILS_RESTRICT const spheres, size_t count) noexcept {

    // without this, the vectorization is less efficient
    // we're guaranteed to have a multiple of 4 lights (at least)
    count = uint32_t(count + 3u) & ~3u;

    for (size_t i = 0 ; i < count; i++) {
        const float4 sphere = spheres[i];
        const float4 center = camera.view * sphere.xyz; // camera points towards the -z axis
        distances[i] = -center.z > 0.0f ? -center.z : 0.0f; // std::max() prevents vectorization (???)
    }
}

// These methods need to exist so clang honors the __restrict__ keyword, which in turn
// produces much better vectorization. The ALWAYS_INLINE keyword makes sure we actually don't
// pay the price of the call!
UTILS_ALWAYS_INLINE
void FScene::computeLightRanges(
        float2* UTILS_RESTRICT const zrange,
        CameraInfo const& UTILS_RESTRICT camera,
        float4 const* UTILS_RESTRICT const spheres, size_t count) noexcept {

    // without this clang seems to assume the src and dst might overlap even if they're
    // restricted.
    // we're guaranteed to have a multiple of 4 lights (at least)
    count = uint32_t(count + 3u) & ~3u;

    for (size_t i = 0 ; i < count; i++) {
        // this loop gets vectorized x4
        const float4 sphere = spheres[i];
        const float4 center = camera.view * sphere.xyz; // camera points towards the -z axis
        float4 n = center + float4{ 0, 0, sphere.w, 0 };
        float4 f = center - float4{ 0, 0, sphere.w, 0 };
        // project to clip space
        n = camera.projection * n;
        f = camera.projection * f;
        // convert to NDC
        const float min = (n.w > camera.zn) ? (n.z / n.w) : -1.0f;
        const float max = (f.w < camera.zf) ? (f.z / f.w) :  1.0f;
        // convert to screen space
        zrange[i].x = (min + 1.0f) * 0.5f;
        zrange[i].y = (max + 1.0f) * 0.5f;
    }
}

void FScene::addEntity(Entity entity) {
    mEntities.insert(entity);
}

void FScene::addEntities(const Entity* entities, size_t count) {
    mEntities.insert(entities, entities + count);
}

void FScene::remove(Entity entity) {
    mEntities.erase(entity);
}

size_t FScene::getRenderableCount() const noexcept {
    FEngine& engine = mEngine;
    EntityManager& em = engine.getEntityManager();
    FRenderableManager& rcm = engine.getRenderableManager();
    size_t count = 0;
    auto const& entities = mEntities;
    for (Entity e : entities) {
        count += em.isAlive(e) && rcm.getInstance(e) ? 1 : 0;
    }
    return count;
}

size_t FScene::getLightCount() const noexcept {
    FEngine& engine = mEngine;
    EntityManager& em = engine.getEntityManager();
    FLightManager& lcm = engine.getLightManager();
    size_t count = 0;
    auto const& entities = mEntities;
    for (Entity e : entities) {
        count += em.isAlive(e) && lcm.getInstance(e) ? 1 : 0;
    }
    return count;
}

void FScene::setSkybox(FSkybox const* skybox) noexcept {
    std::swap(mSkybox, skybox);
    if (skybox) {
        remove(skybox->getEntity());
    }
    if (mSkybox) {
        addEntity(mSkybox->getEntity());
    }
}

void FScene::computeBounds(
        Aabb& UTILS_RESTRICT castersBox,
        Aabb& UTILS_RESTRICT receiversBox,
        uint32_t visibleLayers) const noexcept {
    using State = FRenderableManager::Visibility;

    // Compute the scene bounding volume
    RenderableSoa const& UTILS_RESTRICT soa = mRenderableData;
    float3 const* const UTILS_RESTRICT worldAABBCenter = soa.data<WORLD_AABB_CENTER>();
    float3 const* const UTILS_RESTRICT worldAABBExtent = soa.data<WORLD_AABB_EXTENT>();
    uint8_t const* const UTILS_RESTRICT layers = soa.data<LAYERS>();
    State const* const UTILS_RESTRICT visibility = soa.data<VISIBILITY_STATE>();
    size_t c = soa.size();
    for (size_t i = 0; i < c; i++) {
        if (layers[i] & visibleLayers) {
            const Aabb aabb{ worldAABBCenter[i] - worldAABBExtent[i],
                             worldAABBCenter[i] + worldAABBExtent[i] };
            if (visibility[i].castShadows) {
                castersBox.min = min(castersBox.min, aabb.min);
                castersBox.max = max(castersBox.max, aabb.max);
            }
            if (visibility[i].receiveShadows) {
                receiversBox.min = min(receiversBox.min, aabb.min);
                receiversBox.max = max(receiversBox.max, aabb.max);
            }
        }
    }
}

} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

void Scene::setSkybox(Skybox const* skybox) noexcept {
    upcast(this)->setSkybox(upcast(skybox));
}

void Scene::setIndirectLight(IndirectLight const* ibl) noexcept {
    upcast(this)->setIndirectLight(upcast(ibl));
}

void Scene::addEntity(Entity entity) {
    upcast(this)->addEntity(entity);
}

void Scene::addEntities(const Entity* entities, size_t count) {
    upcast(this)->addEntities(entities, count);
}

void Scene::remove(Entity entity) {
    upcast(this)->remove(entity);
}

size_t Scene::getRenderableCount() const noexcept {
    return upcast(this)->getRenderableCount();
}

size_t Scene::getLightCount() const noexcept {
    return upcast(this)->getLightCount();
}

} // namespace filament
