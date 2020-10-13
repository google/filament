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

// ------------------------------------------------------------------------------------------------

FScene::FScene(FEngine& engine) :
        mEngine(engine) {
}

FScene::~FScene() noexcept = default;


void FScene::prepare(const mat4f& worldOriginTransform) {
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
    renderableDataCapacity = (renderableDataCapacity + 0xFu) & ~0xFu;
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
    lightDataCapacity = (lightDataCapacity + 0xFu) & ~0xFu;

    lightData.clear();
    if (lightData.capacity() < lightDataCapacity) {
        lightData.setCapacity(lightDataCapacity);
    }
    // the first entries are reserved for the directional lights (currently only one)
    lightData.resize(DIRECTIONAL_LIGHTS_COUNT);


    // find the max intensity directional light index in our local array
    float maxIntensity = 0.0f;

    for (Entity e : entities) {
        if (!em.isAlive(e)) {
            continue;
        }

        // getInstance() always returns null if the entity is the Null entity
        // so we don't need to check for that, but we need to check it's alive
        auto ri = rcm.getInstance(e);
        auto li = lcm.getInstance(e);
        if (!ri & !li) {
            continue;
        }

        // get the world transform
        auto ti = tcm.getInstance(e);
        const mat4f worldTransform = worldOriginTransform * tcm.getWorldTransform(ti);
        const bool reversedWindingOrder = det(worldTransform.upperLeft()) < 0;

        // don't even draw this object if it doesn't have a transform (which shouldn't happen
        // because one is always created when creating a Renderable component).
        if (ri && ti) {
            // compute the world AABB so we can perform culling
            const Box worldAABB = rigidTransform(rcm.getAABB(ri), worldTransform);

            // we know there is enough space in the array
            sceneData.push_back_unsafe(
                    ri,                       // RENDERABLE_INSTANCE
                    worldTransform,           // WORLD_TRANSFORM
                    reversedWindingOrder,     // REVERSED_WINDING_ORDER
                    rcm.getVisibility(ri),    // VISIBILITY_STATE
                    rcm.getBonesUbh(ri),      // BONES_UBH
                    worldAABB.center,         // WORLD_AABB_CENTER
                    0,                        // VISIBLE_MASK
                    rcm.getMorphWeights(ri),  // MORPH_WEIGHTS
                    rcm.getLayerMask(ri),     // LAYERS
                    worldAABB.halfExtent,     // WORLD_AABB_EXTENT
                    {},                       // PRIMITIVES
                    0                         // SUMMED_PRIMITIVE_COUNT
            );
        }

        if (li) {
            // find the dominant directional light
            if (UTILS_UNLIKELY(lcm.isDirectionalLight(li))) {
                // we don't store the directional lights, because we only have a single one
                if (lcm.getIntensity(li) >= maxIntensity) {
                    maxIntensity = lcm.getIntensity(li);
                    float3 d = lcm.getLocalDirection(li);
                    // using mat3f::getTransformForNormals handles non-uniform scaling
                    d = normalize(mat3f::getTransformForNormals(worldTransform.upperLeft()) * d);
                    lightData.elementAt<FScene::POSITION_RADIUS>(0) =
                            float4{ 0, 0, 0, std::numeric_limits<float>::infinity() };
                    lightData.elementAt<FScene::DIRECTION>(0)       = d;
                    lightData.elementAt<FScene::LIGHT_INSTANCE>(0)  = li;
                }
            } else {
                const float4 p = worldTransform * float4{ lcm.getLocalPosition(li), 1 };
                float3 d = 0;
                if (!lcm.isPointLight(li) || lcm.isIESLight(li)) {
                    d = lcm.getLocalDirection(li);
                    // using mat3f::getTransformForNormals handles non-uniform scaling
                    d = normalize(mat3f::getTransformForNormals(worldTransform.upperLeft()) * d);
                }
                lightData.push_back_unsafe(
                        float4{ p.xyz, lcm.getRadius(li) }, d, li, {}, {}, {});
            }
        }
    }

    // some elements past the end of the array will be accessed by SIMD code, we need to make
    // sure the data is valid enough as not to produce errors such as divide-by-zero
    // (e.g. in computeLightRanges())
    for (size_t i = lightData.size(), e = (lightData.size() + 3u) & ~3u; i < e; i++) {
        new(lightData.data<POSITION_RADIUS>() + i) float4{ 0, 0, 0, 1 };
    }
}

void FScene::updateUBOs(utils::Range<uint32_t> visibleRenderables, backend::Handle<backend::HwUniformBuffer> renderableUbh) noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    const size_t size = visibleRenderables.size() * sizeof(PerRenderableUib);

    // allocate space into the command stream directly
    void* const buffer = driver.allocate(size);

    bool hasContactShadows = false;
    auto& sceneData = mRenderableData;
    for (uint32_t i : visibleRenderables) {
        mat4f const& model = sceneData.elementAt<WORLD_TRANSFORM>(i);
        const size_t offset = i * sizeof(PerRenderableUib);

        UniformBuffer::setUniform(buffer,
                offset + offsetof(PerRenderableUib, worldFromModelMatrix), model);

        // Using mat3f::getTransformForNormals handles non-uniform scaling, but DOESN'T guarantee that
        // the transformed normals will have unit-length, therefore they need to be normalized
        // in the shader (that's already the case anyways, since normalization is needed after
        // interpolation).
        //
        // We pre-scale normals by the inverse of the largest scale factor to avoid
        // large post-transform magnitudes in the shader, especially in the fragment shader, where
        // we use medium precision.
        //
        // Note: if the model matrix is known to be a rigid-transform, we could just use it directly.

        mat3f m = mat3f::getTransformForNormals(model.upperLeft());
        m *= mat3f(1.0f / std::sqrt(max(float3{length2(m[0]), length2(m[1]), length2(m[2])})));

        // The shading normal must be flipped for mirror transformations.
        // Basically we're shading the other side of the polygon and therefore need to negate the
        // normal, similar to what we already do to support double-sided lighting.
        if (sceneData.elementAt<REVERSED_WINDING_ORDER>(i)) {
            m = -m;
        }

        UniformBuffer::setUniform(buffer,
                offset + offsetof(PerRenderableUib, worldFromModelNormalMatrix), m);

        // Note that we cast bool to uint32_t. Booleans are byte-sized in C++, but we need to
        // initialize all 32 bits in the UBO field.

        FRenderableManager::Visibility visibility = sceneData.elementAt<VISIBILITY_STATE>(i);
        hasContactShadows = hasContactShadows || visibility.screenSpaceContactShadows;
        UniformBuffer::setUniform(buffer,
                offset + offsetof(PerRenderableUib, skinningEnabled),
                uint32_t(visibility.skinning));

        UniformBuffer::setUniform(buffer,
                offset + offsetof(PerRenderableUib, morphingEnabled),
                uint32_t(visibility.morphing));

        UniformBuffer::setUniform(buffer,
                offset + offsetof(PerRenderableUib, screenSpaceContactShadows),
                uint32_t(visibility.screenSpaceContactShadows));

        UniformBuffer::setUniform(buffer,
                offset + offsetof(PerRenderableUib, morphWeights),
                sceneData.elementAt<MORPH_WEIGHTS>(i));
    }

    // TODO: handle static objects separately
    mHasContactShadows = hasContactShadows;
    mRenderableViewUbh = renderableUbh;
    driver.loadUniformBuffer(renderableUbh, { buffer, size });

    if (mSkybox) {
        mSkybox->commit(driver);
    }
}

void FScene::terminate(FEngine& engine) {
    // DO NOT destroy this UBO, it's owned by the View
    mRenderableViewUbh.clear();
}

void FScene::prepareDynamicLights(const CameraInfo& camera, ArenaScope& rootArena, backend::Handle<backend::HwUniformBuffer> lightUbh) noexcept {
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
    size_t const size = lightData.size();

    // always allocate at least 4 entries, because the vectorized loops below rely on that
    float* const UTILS_RESTRICT distances = arena.allocate<float>((size + 3u) & 3u, CACHELINE_SIZE);

    // pre-compute the lights' distance to the camera plane, for sorting below
    // - we don't skip the directional light, because we don't care, it's ignored during sorting
    float4 const* const UTILS_RESTRICT spheres = lightData.data<FScene::POSITION_RADIUS>();
    computeLightCameraPlaneDistances(distances, camera, spheres, size);

    // skip directional light
    Zip2Iterator<FScene::LightSoa::iterator, float*> b = { lightData.begin(), distances };
    std::sort(b + DIRECTIONAL_LIGHTS_COUNT, b + size,
            [](auto const& lhs, auto const& rhs) { return lhs.second < rhs.second; });

    // drop excess lights
    lightData.resize(std::min(size, CONFIG_MAX_LIGHT_COUNT + DIRECTIONAL_LIGHTS_COUNT));

    // number of point/spot lights
    size_t positionalLightCount = size - DIRECTIONAL_LIGHTS_COUNT;

    // compute the light ranges (needed when building light trees)
    float2* const zrange = lightData.data<FScene::SCREEN_SPACE_Z_RANGE>();
    computeLightRanges(zrange, camera, spheres + DIRECTIONAL_LIGHTS_COUNT, positionalLightCount);

    LightsUib* const lp = driver.allocatePod<LightsUib>(positionalLightCount);

    auto const* UTILS_RESTRICT directions       = lightData.data<FScene::DIRECTION>();
    auto const* UTILS_RESTRICT instances        = lightData.data<FScene::LIGHT_INSTANCE>();
    auto const* UTILS_RESTRICT shadowInfo       = lightData.data<FScene::SHADOW_INFO>();
    for (size_t i = DIRECTIONAL_LIGHTS_COUNT, c = size; i < c; ++i) {
        const size_t gpuIndex = i - DIRECTIONAL_LIGHTS_COUNT;
        auto li = instances[i];
        lp[gpuIndex].positionFalloff      = { spheres[i].xyz, lcm.getSquaredFalloffInv(li) };
        lp[gpuIndex].colorIntensity       = { lcm.getColor(li), lcm.getIntensity(li) };
        lp[gpuIndex].directionIES         = { directions[i], 0.0f };
        lp[gpuIndex].spotScaleOffset      = lcm.getSpotParams(li).scaleOffset;
        lp[gpuIndex].shadow               = { shadowInfo[i].pack() };
        lp[gpuIndex].type                 = lcm.isPointLight(li) ? 0u : 1u;
    }

    driver.loadUniformBuffer(lightUbh, { lp, positionalLightCount * sizeof(LightsUib) });
}

// These methods need to exist so clang honors the __restrict__ keyword, which in turn
// produces much better vectorization. The ALWAYS_INLINE keyword makes sure we actually don't
// pay the price of the call!
UTILS_ALWAYS_INLINE
inline void FScene::computeLightCameraPlaneDistances(
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
inline void FScene::computeLightRanges(
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

void FScene::removeEntities(const Entity* entities, size_t count) {
    for (size_t i = 0; i < count; ++i, ++entities) {
        remove(*entities);
    }
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

bool FScene::hasEntity(Entity entity) const noexcept {
    return mEntities.find(entity) != mEntities.end();
}

void FScene::setSkybox(FSkybox* skybox) noexcept {
    std::swap(mSkybox, skybox);
    if (skybox) {
        remove(skybox->getEntity());
    }
    if (mSkybox) {
        addEntity(mSkybox->getEntity());
    }
}

bool FScene::hasContactShadows() const noexcept {
    // find out if at least one light has contact-shadow enabled
    // TODO: cache the the result of this Loop in the LightManager
    bool hasContactShadows = false;
    auto& lcm = mEngine.getLightManager();
    const auto *pFirst = mLightData.begin<LIGHT_INSTANCE>();
    const auto *pLast = mLightData.end<LIGHT_INSTANCE>();
    while (pFirst != pLast && !hasContactShadows) {
        if (pFirst->isValid()) {
            auto const& shadowOptions = lcm.getShadowOptions(*pFirst);
            hasContactShadows = shadowOptions.screenSpaceContactShadows;
        }
        ++pFirst;
    }

    // at least some renderables in the scene must have contact-shadows enabled
    // TODO: we should refine this with only the visible ones
    return hasContactShadows && mHasContactShadows;
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

void Scene::setSkybox(Skybox* skybox) noexcept {
    upcast(this)->setSkybox(upcast(skybox));
}

Skybox* Scene::getSkybox() noexcept {
    return upcast(this)->getSkybox();
}

Skybox const* Scene::getSkybox() const noexcept {
    return upcast(this)->getSkybox();
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

void Scene::removeEntities(const Entity* entities, size_t count) {
    upcast(this)->removeEntities(entities, count);
}

size_t Scene::getRenderableCount() const noexcept {
    return upcast(this)->getRenderableCount();
}

size_t Scene::getLightCount() const noexcept {
    return upcast(this)->getLightCount();
}

bool Scene::hasEntity(Entity entity) const noexcept {
    return upcast(this)->hasEntity(entity);
}

} // namespace filament
