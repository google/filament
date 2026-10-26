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

#include "Allocators.h"
#include "Culler.h"

#include "components/LightManager.h"
#include "components/RenderableManager.h"
#include "components/TransformManager.h"

#include "details/Camera.h"
#include "details/Engine.h"
#include "details/Skybox.h"
#include "details/View.h"

#include <private/filament/UibStructs.h>

#include <filament/Box.h>
#include <filament/LightManager.h>

#include <backend/Handle.h>

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/Invocable.h>
#include <utils/Mutex.h>
#include <utils/PagedArenaBitset.h>
#include <utils/Range.h>
#include <utils/Slice.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <new>
#include <tuple>
#include <utility>

using namespace filament::backend;
using namespace filament::math;
using namespace utils;

namespace filament {

// Explicitly define the architectural boundaries we rely on
constexpr size_t BITSET_PAGE_BITS = (1U << PagedArenaBitset::FILAMENT_PAGE_SHIFT);
constexpr size_t FILAMENT_INDEX_SPACE = 1U << FILAMENT_GENERATION_SHIFT;
constexpr size_t MAX_POSSIBLE_ID = 1U << (FILAMENT_GENERATION_SHIFT + FILAMENT_GENERATION_BITS);
constexpr size_t MAX_POSSIBLE_PAGES = MAX_POSSIBLE_ID / BITSET_PAGE_BITS;

//  Generation Alignment Guarantee
static_assert((FILAMENT_INDEX_SPACE % BITSET_PAGE_BITS) == 0,
    "FATAL (PERFORMANCE): Bitset Page Size must perfectly divide the "
    "Entity Index Space. This ensures that recycled entities (new generations) "
    "align perfectly with page boundaries and do not fragment the Arena.");

// PagedArenaBitset L2 Directory Capacity Guarantee
static_assert(MAX_POSSIBLE_PAGES <= std::numeric_limits<uint16_t>::max(),
    "FATAL (OVERFLOW): The maximum possible Entity ID exceeds the capacity "
    "of the PagedArenaBitset's 16-bit L2 Directory. You must either increase "
    "the Directory index type to uint32_t or increase the Page Size.");

// ------------------------------------------------------------------------------------------------

FScene::FScene(FEngine& engine) :
    mEngine(engine) {
    EntityManager& em = engine.getEntityManager();
    em.registerChangeCallback(this,
            [pending = mPendingDestroyed](Slice<const Entity> entities) {
                LockGuard const lock(pending->lock);
                for (Entity const e : entities) {
                    pending->entities.add(e.getId());
                }
            });
}

FScene::~FScene() noexcept {
    mEngine.getEntityManager().unregisterChangeCallback(this);
}

void FScene::flushPendingDestroyed() noexcept {
    mEngine.getEntityManager().flushNotifications();
    {
        LockGuard const lock(mPendingDestroyed->lock);
        if (mPendingDestroyed->entities.empty()) {
            return;
        }
        mPendingDestroyed->entities.swap(mTempPendingDestroyed);
        mPendingDestroyed->entities.clear();
    }
    mEntities.difference(mTempPendingDestroyed);
    for (FView* view : mRegisteredViews) {
        if (auto* cache = view->getSceneCache(); cache) {
            cache->destroyedEntities.merge(mTempPendingDestroyed);
        }
    }
    mTempPendingDestroyed.clear();
}

SceneCacheData::SceneCacheData() noexcept = default;
SceneCacheData::~SceneCacheData() noexcept = default;

bool SceneCacheData::hasDirtyEntities() const noexcept {
    return !removedEntities.empty() || !dirtyEntities.empty() || !destroyedEntities.empty();
}

void SceneCacheData::resize(size_t const renderableCount, size_t const lightCount) {
    // We always pad the capacity by at least 1 so we can store the summed primitive count
    // Capacity is always multiple of 16 (to allow SIMD operations)
    size_t const renderableDataCapacity = ((renderableCount + 1) + 0xFu) & ~0xFu;
    if (renderableData.capacity() < renderableDataCapacity) {
        renderableData.clear();
        renderableData.setCapacity(renderableDataCapacity);
    }
    renderableData.resize(renderableCount);

    // computeLightRanges() (starting at index DIRECTIONAL_LIGHTS_COUNT) and
    // FView::computeLightCameraDistances() (starting at index 0) process lights 4 at a time, so
    // they read up to 3 entries past the end. Capacity is always a multiple of 16 (for SIMD).
    size_t const lightDataCapacity = (std::max<size_t>(1, lightCount) + 3u + 0xFu) & ~0xFu;
    if (lightData.capacity() < lightDataCapacity) {
        lightData.clear();
        lightData.setCapacity(lightDataCapacity);
    }
    lightData.resize(lightCount);

    // some elements past the end of the array will be accessed by SIMD code, we need to make
    // sure the data is valid enough as not to produce errors such as divide-by-zero
    // (e.g. in computeLightRanges())
    for (size_t i = lightData.size(), e = lightData.capacity(); i < e; i++) {
        new(lightData.data<FScene::POSITION_RADIUS>() + i) float4{0, 0, 0, 1};
        lightData.data<FScene::POSITION_X>()[i] = 0;
        lightData.data<FScene::POSITION_Y>()[i] = 0;
        lightData.data<FScene::POSITION_Z>()[i] = 0;
        lightData.data<FScene::RADIUS>()[i]     = 1;
    }

    // Purely for the benefit of MSAN, we can avoid uninitialized reads by zeroing out the
    // unused scene elements between the end of the array and the rounded-up count.
    if constexpr (UTILS_HAS_SANITIZE_MEMORY) {
        for (size_t i = renderableData.size(), e = renderableData.capacity(); i < e; i++) {
            renderableData.data<FScene::LAYERS>()[i] = 0;
            renderableData.data<FScene::VISIBLE_MASK>()[i] = 0;
            renderableData.data<FScene::VISIBILITY_STATE>()[i] = {};
            renderableData.data<FScene::SKINNING_STATE>()[i] = {};
            renderableData.data<FScene::WORLD_AABB_CENTER_X>()[i] = 0;
            renderableData.data<FScene::WORLD_AABB_CENTER_Y>()[i] = 0;
            renderableData.data<FScene::WORLD_AABB_CENTER_Z>()[i] = 0;
            renderableData.data<FScene::WORLD_AABB_EXTENT_X>()[i] = 0;
            renderableData.data<FScene::WORLD_AABB_EXTENT_Y>()[i] = 0;
            renderableData.data<FScene::WORLD_AABB_EXTENT_Z>()[i] = 0;
        }
    }
}

UTILS_NOINLINE
static void updateRenderableCache(
        FEngine& engine,
        SceneCacheData::RenderableCache& cache,
        Entity const e,
        FRenderableManager::Instance const ri,
        FTransformManager::Instance const ti,
        mat3 const& worldRotation,
        bool const shadowReceiversAreCasters) {
    FRenderableManager const& rcm = engine.getRenderableManager();
    FTransformManager const& tcm = engine.getTransformManager();

    mat4 const model = tcm.getWorldTransformAccurate(ti);
    mat3f const shaderWorldRotation{ worldRotation * model.upperLeft() };
    double3 const rotatedTranslation = worldRotation * model[3].xyz;
    bool const reversedWindingOrder = det(shaderWorldRotation) < 0;

    Box const aabb = rcm.getAABB(ri);
    float3 const rotatedAabbCenter = shaderWorldRotation * aabb.center;
    float3 const worldAabbExtent = abs(shaderWorldRotation) * aabb.halfExtent;

    auto visibility = rcm.getVisibility(ri);
    visibility.reversedWindingOrder = reversedWindingOrder;
    if (shadowReceiversAreCasters && visibility.receiveShadows) {
        visibility.castShadows = true;
    }

    const mat4f& transform = tcm.getTransform(ti);
    float const scale = (length(transform[0].xyz) + length(transform[1].xyz) +
            length(transform[2].xyz)) / 3.0f;

    uint32_t const index = cache.acquire(e, rotatedTranslation);
    auto& soa = cache.getSoA();
    soa.elementAt<FScene::RENDERABLE_ENTITY>(index) = e;
    soa.elementAt<FScene::WORLD_TRANSFORM>(index) = mat4f{ shaderWorldRotation };
    soa.elementAt<FScene::VISIBILITY_STATE>(index) = visibility;
    soa.elementAt<FScene::SKINNING_STATE>(index) = rcm.getSkinning(ri);
    soa.elementAt<FScene::SKINNING_BUFFER>(index) = rcm.getSkinningBufferInfo(ri);
    soa.elementAt<FScene::MORPHING_BUFFER>(index) = rcm.getMorphingBufferInfo(ri);
    soa.elementAt<FScene::INSTANCES>(index) = rcm.getInstancesInfo(ri);
    soa.elementAt<FScene::WORLD_AABB_CENTER>(index) = rotatedAabbCenter;
    soa.elementAt<FScene::CHANNELS>(index) = rcm.getLightChannels(ri);
    soa.elementAt<FScene::LAYERS>(index) = rcm.getLayerMask(ri);
    soa.elementAt<FScene::WORLD_AABB_EXTENT>(index) = worldAabbExtent;
    soa.elementAt<FScene::WORLD_AABB_EXTENT_X>(index) = worldAabbExtent.x;
    soa.elementAt<FScene::WORLD_AABB_EXTENT_Y>(index) = worldAabbExtent.y;
    soa.elementAt<FScene::WORLD_AABB_EXTENT_Z>(index) = worldAabbExtent.z;
    soa.elementAt<FScene::PRIMITIVES>(index) = rcm.getRenderPrimitives(ri, 0);
    soa.elementAt<FScene::USER_DATA>(index) = scale;
}

UTILS_NOINLINE
static void updateLightCache(
        FEngine& engine,
        SceneCacheData::LightCache& cache,
        Entity const e,
        FLightManager::Instance const li,
        FTransformManager::Instance const ti,
        mat3 const& worldRotation) {
    FLightManager const& lcm = engine.getLightManager();
    FTransformManager const& tcm = engine.getTransformManager();

    mat4 const model = tcm.getWorldTransformAccurate(ti);
    mat3f const shaderWorldRotation{ worldRotation * model.upperLeft() };
    double3 const rotatedPosition =
            worldRotation * (model * double4{ lcm.getLocalPosition(li), 1.0 }).xyz;
    float const r = lcm.getRadius(li);
    float3 d = 0;
    if (!lcm.isPointLight(li) || lcm.isIESLight(li)) {
        d = lcm.getLocalDirection(li);
        d = normalize(mat3f::getTransformForNormals(shaderWorldRotation) * d);
    }

    uint32_t const index = cache.acquire(e, rotatedPosition);
    auto& soa = cache.getSoA();
    soa.elementAt<FScene::LIGHT_ENTITY>(index) = e;
    soa.elementAt<FScene::RADIUS>(index) = r;
    soa.elementAt<FScene::DIRECTION>(index) = d;
    soa.elementAt<FScene::SPOT_PARAMS>(index) =
            float2{ lcm.getCosOuterSquared(li), lcm.getSinInverse(li) };
}

bool FScene::processDeferredUpdates(mat3 const& worldRotation, bool shadowReceiversAreCasters,
        SceneCacheData& cache) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    FEngine& engine = mEngine;
    FTransformManager const& tcm = engine.getTransformManager();
    tcm.ensureWorldTransformsUpToDate();

    flushPendingDestroyed();

    if (UTILS_UNLIKELY(cache.needsFullUpdate)) {
        cache.directionalLights.intersect(mEntities);
        auto checkRemoved = [&](Entity const* entities, size_t const count) {
            for (size_t i = 0; i < count; ++i) {
                if (uint32_t const id = entities[i].getId(); !mEntities[id]) {
                    cache.removedEntities.add(id);
                }
            }
        };
        checkRemoved(cache.renderableCache.getSoA().data<RENDERABLE_ENTITY>(),
                cache.renderableCache.size());
        checkRemoved(cache.lightCache.getSoA().data<LIGHT_ENTITY>(),
                cache.lightCache.size());
    }

    bool const viewStateChanged = (cache.needsFullUpdate ||
            worldRotation != cache.previousWorldRotation ||
            shadowReceiversAreCasters != cache.previousShadowReceiversAreCasters);

    // Very fast path when nothing has changed at all.
    if (UTILS_LIKELY(!viewStateChanged && !cache.hasDirtyEntities())) {
        return false;
    }

    // Process deferred removals from the Scene _and_ dead entities.
    EntityManager const& em = engine.getEntityManager();
    FRenderableManager const& rcm = engine.getRenderableManager();
    FLightManager const& lcm = engine.getLightManager();

    // Every registered View receives every destroyed entity. Do NOT filter them by mEntities:
    // flushPendingDestroyed() has already removed them from mEntities while they may still be in
    // this View's cache. SoaCache::remove() is a no-op for entities that aren't cached.
    if (UTILS_UNLIKELY(!cache.destroyedEntities.empty() || !cache.removedEntities.empty())) {
        // Merge the entities that were removed from the scene (but could have been re-added!).
        cache.destroyedEntities.merge(cache.removedEntities);
        // Cache invalidation and scene removal.
        cache.destroyedEntities.forEachSetBit([&](uint32_t const bit) {
            Entity const e = Entity::import(int32_t(bit));
            cache.renderableCache.remove(e);
            cache.lightCache.remove(e);
            cache.directionalLights.remove(bit);
            // An Entity could have been re-added, so it could be in mEntities at this point; in
            // that case, it will also be in dirtyEntities.
            assert_invariant(!cache.removedEntities[bit] ||
                    (!mEntities[bit] || cache.dirtyEntities[bit]));
        });
        cache.destroyedEntities.clear();
        cache.removedEntities.clear();
    }

    // Check the states that require a full cache rebuild
    if (UTILS_LIKELY(viewStateChanged)) {
        cache.previousWorldRotation = worldRotation;
        cache.previousShadowReceiversAreCasters = shadowReceiversAreCasters;
        cache.dirtyEntities.copyFrom(mEntities); // Mark all as dirty for full rebuild
    }

    // Process modified entities. Entities without a TransformManager component (common for lights)
    // use the null TransformManager instance (0), whose world transform is identity.
    em.getAliveEntities(mTempAliveEntities);
    EntitySet::intersect(&mTempActiveDirty, mEntities, cache.dirtyEntities, mTempAliveEntities);
    EntitySet const& activeDirty = mTempActiveDirty;

    // remove (from the cache) renderables that have lost their RenderableManager component
    EntitySet::intersect(&mTempRenderableEntities, activeDirty, rcm.getEntityBitset());
    EntitySet const& renderableEntities = mTempRenderableEntities;
    if (UTILS_UNLIKELY(activeDirty.size() > renderableEntities.size())) {
        // An entity needs to be removed if it is dirty but no longer has the component.
        EntitySet::difference(&mTempToRemoveRenderables, activeDirty, rcm.getEntityBitset());
        mTempToRemoveRenderables.forEachSetBit([&](uint32_t const bit) {
            Entity const e = Entity::import(int32_t(bit));
            cache.renderableCache.remove(e);
        });
    }

    // remove (from the cache) lights that have lost their LightManager component
    EntitySet::intersect(&mTempLightEntities, activeDirty, lcm.getEntityBitset());
    EntitySet const& lightEntities = mTempLightEntities;
    if (UTILS_UNLIKELY(activeDirty.size() > lightEntities.size())) {
        // An entity needs to be removed if it is dirty but no longer has the component.
        EntitySet::difference(&mTempToRemoveLights, activeDirty, lcm.getEntityBitset());
        mTempToRemoveLights.forEachSetBit([&](uint32_t const bit) {
            Entity const e = Entity::import(int32_t(bit));
            cache.lightCache.remove(e);
            cache.directionalLights.remove(bit);
        });
    }

    renderableEntities.extractTo(mTempExtractedRenderables);
    for (uint32_t const id : mTempExtractedRenderables) {
        Entity const e = Entity::import(int32_t(id));
        auto const ri = rcm.getInstance(e);
        // Renderables with an instance count of 0 are not drawn, so we skip them entirely.
        if (UTILS_LIKELY(rcm.getInstancesInfo(ri).count)) {
            auto const ti = tcm.getInstance(e);
            updateRenderableCache(engine, cache.renderableCache, e, ri, ti,
                    worldRotation, shadowReceiversAreCasters);
        } else {
            cache.renderableCache.remove(e);
        }
    }

    lightEntities.extractTo(mTempExtractedLights);
    for (uint32_t const id : mTempExtractedLights) {
        Entity const e = Entity::import(int32_t(id));
        auto const li = lcm.getInstance(e);
        auto const ti = tcm.getInstance(e);
        if (lcm.isDirectionalLight(li)) {
            cache.lightCache.remove(e);
            cache.directionalLights.add(id);
        } else {
            cache.directionalLights.remove(id);
            updateLightCache(engine, cache.lightCache, e, li, ti, worldRotation);
        }
    }

    // Collect directional lights
    auto& directionalInstances = mTempDirectionalInstances;
    directionalInstances.clear();
    cache.directionalLights.forEachSetBit([&](uint32_t const bit) {
        Entity const e = Entity::import(int32_t(bit));
        if (auto const li = lcm.getInstance(e)) {
            directionalInstances.emplace_back(li, tcm.getInstance(e));
        }
    });

    if (directionalInstances.size() > 1) {
        size_t const keep = std::min<size_t>(directionalInstances.size(),
                DIRECTIONAL_LIGHTS_COUNT + CONFIG_MAX_EXTRA_DIRECTIONAL_LIGHTS);
        std::partial_sort(directionalInstances.begin(),
                directionalInstances.begin() + keep, directionalInstances.end(),
                [&lcm](auto const& lhs, auto const& rhs) {
                    float const l = lcm.getIntensity(lhs.first);
                    float const r = lcm.getIntensity(rhs.first);
                    if (l != r) {
                        return l > r;
                    }
                    // tie-break: lowest entity id first
                    return lcm.getEntity(lhs.first).getId() < lcm.getEntity(rhs.first).getId();
                });
    }

    // Extra directional lights (evaluated without shadows)
    cache.extraDirectionalLightCount = 0;
    if (UTILS_UNLIKELY(engine.getConfig().enableMultipleDirectionalLights &&
                directionalInstances.size() > DIRECTIONAL_LIGHTS_COUNT)) {
        mat3 const worldTransformNormals = mat3::getTransformForNormals(worldRotation);
        size_t const extraCount = std::min(directionalInstances.size() - DIRECTIONAL_LIGHTS_COUNT,
                CONFIG_MAX_EXTRA_DIRECTIONAL_LIGHTS);
        for (size_t i = 0; i < extraCount; i++) {
            auto const [li, ti] = directionalInstances[DIRECTIONAL_LIGHTS_COUNT + i];
            mat3 const worldDirectionTransform =
                    mat3::getTransformForNormals(tcm.getWorldTransformAccurate(ti).upperLeft());
            double3 const d = worldTransformNormals *
                    (worldDirectionTransform * lcm.getLocalDirection(li));
            cache.extraDirectionalLightDirections[i] = float3(normalize(d));
            cache.extraDirectionalLightInstances[i] = li;
        }
        cache.extraDirectionalLightCount = uint8_t(extraCount);
    }

    // Dominant directional light
    FLightManager::Instance const directionalLight = directionalInstances.empty() ?
            FLightManager::Instance{} : directionalInstances[0].first;
    cache.dominantDirectionalLight =
            directionalLight ? lcm.getEntity(directionalLight) : Entity{};

    // Check contact shadows
    // TODO: only recompute this when lights (or the dominant directional light) change, rather than
    // scanning all cached lights on every scene cache update.
    cache.lightsHaveContactShadows = false;
    if (directionalLight) {
        if (lcm.getShadowOptions(directionalLight).screenSpaceContactShadows) {
            cache.lightsHaveContactShadows = true;
        }
    }
    if (!cache.lightsHaveContactShadows) {
        auto const* pLightEntities = cache.lightCache.getSoA().data<LIGHT_ENTITY>();
        for (size_t i = 0, c = cache.lightCache.getSoA().size(); i < c; i++) {
            Entity const entity = pLightEntities[i];
            if (!entity.isNull()) {
                if (lcm.getShadowOptions(lcm.getInstance(entity)).screenSpaceContactShadows) {
                    cache.lightsHaveContactShadows = true;
                    break;
                }
            }
        }
    }

    cache.dirtyEntities.clear();
    cache.needsFullUpdate = false;
    return true;
}

void FScene::prepare(JobSystem&,
        LinearAllocatorArena&,
        mat4 const& worldTransform,
        bool const shadowReceiversAreCasters,
        SceneCacheData& cache) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    processDeferredUpdates(worldTransform.upperLeft(), shadowReceiversAreCasters, cache);

    auto& sceneData = cache.renderableData;
    auto& lightData = cache.lightData;

    size_t const renderableCount = cache.renderableCache.size();
    size_t const positionalLightCount = cache.lightCache.size();
    double3 const worldTranslation = worldTransform[3].xyz;

    // Copy Master Cache (valid across frames) to Scene Cache (valid for this frame)
    // resize the cache to our needs (this can clear it)
    cache.resize(renderableCount, DIRECTIONAL_LIGHTS_COUNT + positionalLightCount);

    // Copy the renderable cache, skipping per-frame-only temporaries (VISIBLE_MASK,
    // SUMMED_PRIMITIVE_COUNT, UBO, DESCRIPTOR_SET_HANDLE) and translation-shifted AABB centers
    // (WORLD_AABB_CENTER, WORLD_AABB_CENTER_X/Y/Z) that are populated in the loop below.
    // Only columns listed in SceneCacheData::RenderableCache's CachedColumns may be read from the
    // master cache (see SoaCache).
    static_assert(RenderableSoa::getArrayCount() == 23,
            "Update sceneData.copyRange column list when RenderableSoa columns change");
    sceneData.copyRange<
            RENDERABLE_ENTITY,
            WORLD_TRANSFORM,
            VISIBILITY_STATE,
            SKINNING_STATE,
            SKINNING_BUFFER,
            MORPHING_BUFFER,
            INSTANCES,
            CHANNELS,
            LAYERS,
            WORLD_AABB_EXTENT,
            WORLD_AABB_EXTENT_X,
            WORLD_AABB_EXTENT_Y,
            WORLD_AABB_EXTENT_Z,
            PRIMITIVES,
            USER_DATA
    >(0, cache.renderableCache.getSoA(), 0, renderableCount);

    double3 const* UTILS_RESTRICT const rTranslations =
            cache.renderableCache.getTranslations().data();
    float3 const* UTILS_RESTRICT const rAabbOffsets =
            cache.renderableCache.getSoA().data<WORLD_AABB_CENTER>();
    mat4f* UTILS_RESTRICT const outTransforms = sceneData.data<WORLD_TRANSFORM>();
    float3* UTILS_RESTRICT const outCenters = sceneData.data<WORLD_AABB_CENTER>();
    float* UTILS_RESTRICT const outCenterX = sceneData.data<WORLD_AABB_CENTER_X>();
    float* UTILS_RESTRICT const outCenterY = sceneData.data<WORLD_AABB_CENTER_Y>();
    float* UTILS_RESTRICT const outCenterZ = sceneData.data<WORLD_AABB_CENTER_Z>();
    for (size_t i = 0; i < renderableCount; ++i) {
        float3 const t = float3(rTranslations[i] + worldTranslation);
        outTransforms[i][3].xyz = t;
        float3 const c = rAabbOffsets[i] + t;
        outCenters[i] = c;
        outCenterX[i] = c.x;
        outCenterY[i] = c.y;
        outCenterZ[i] = c.z;
    }

    // Populate directional light at index 0
    FLightManager const& lcm = mEngine.getLightManager();
    FTransformManager const& tcm = mEngine.getTransformManager();

    FLightManager::Instance const directionalLight =
            lcm.getInstance(cache.dominantDirectionalLight);
    FTransformManager::Instance const directionalTransform =
            tcm.getInstance(cache.dominantDirectionalLight);

    if (directionalLight) {
        mat3 const worldDirectionTransform =
                mat3::getTransformForNormals(tcm.getWorldTransformAccurate(directionalTransform).upperLeft());
        FLightManager::ShadowParams const params = lcm.getShadowParams(directionalLight);
        float3 const localDirection = worldDirectionTransform * lcm.getLocalDirection(directionalLight);
        double3 const shadowLocalDirection = params.options.transform * localDirection;
        mat3 const worldTransformNormals = mat3::getTransformForNormals(worldTransform.upperLeft());
        double3 const d = worldTransformNormals * localDirection;
        double3 const s = worldTransformNormals * shadowLocalDirection;
        auto getMv = [](double3 direction) -> mat3 {
            return transpose(mat3::lookTo(direction, double3{1, 0, 0}));
        };
        double3 const worldOrigin = transpose(worldTransform.upperLeft()) * worldTransform[3].xyz;
        mat3 const Mv = getMv(shadowLocalDirection);
        double2 const lsReferencePoint = (Mv * worldOrigin).xy;
        constexpr float inf = std::numeric_limits<float>::infinity();

        lightData.template elementAt<LIGHT_ENTITY>(0) = lcm.getEntity(directionalLight);
        lightData.template elementAt<POSITION_RADIUS>(0) = float4{0, 0, 0, inf};
        lightData.template elementAt<POSITION_X>(0) = 0;
        lightData.template elementAt<POSITION_Y>(0) = 0;
        lightData.template elementAt<POSITION_Z>(0) = 0;
        lightData.template elementAt<RADIUS>(0)     = inf;
        lightData.template elementAt<DIRECTION>(0) = normalize(d);
        lightData.template elementAt<SHADOW_DIRECTION>(0) = normalize(s);
        lightData.template elementAt<SHADOW_REF>(0) = lsReferencePoint;
        lightData.template elementAt<SPOT_PARAMS>(0) = float2{0};
        lightData.template elementAt<VISIBILITY>(0) = static_cast<Culler::result_type>(0);
        lightData.template elementAt<SCREEN_SPACE_Z_RANGE>(0) = float2{0};
        lightData.template elementAt<SHADOW_INFO>(0) = ShadowInfo{};
    } else {
        lightData.template elementAt<LIGHT_ENTITY>(0) = Entity{};
        lightData.template elementAt<POSITION_RADIUS>(0) = float4{0, 0, 0, 1};
        lightData.template elementAt<POSITION_X>(0) = 0;
        lightData.template elementAt<POSITION_Y>(0) = 0;
        lightData.template elementAt<POSITION_Z>(0) = 0;
        lightData.template elementAt<RADIUS>(0)     = 1;
        lightData.template elementAt<DIRECTION>(0) = float3{0};
        lightData.template elementAt<SHADOW_DIRECTION>(0) = float3{0};
        lightData.template elementAt<SHADOW_REF>(0) = double2{0};
        lightData.template elementAt<SPOT_PARAMS>(0) = float2{0};
        lightData.template elementAt<VISIBILITY>(0) = static_cast<Culler::result_type>(0);
        lightData.template elementAt<SCREEN_SPACE_Z_RANGE>(0) = float2{0};
        lightData.template elementAt<SHADOW_INFO>(0) = ShadowInfo{};
    }

    // Copy positional lights to index 1 .. 1 + positionalLightCount, skipping directional-only,
    // per-frame-only (SHADOW_DIRECTION, SHADOW_REF, VISIBILITY, SCREEN_SPACE_Z_RANGE, SHADOW_INFO),
    // and translation-shifted position columns (POSITION_RADIUS, POSITION_X/Y/Z).
    // Only columns listed in SceneCacheData::LightCache's CachedColumns may be read from the
    // master cache (see SoaCache). SHADOW_INFO is reset to defaults here, each frame, before
    // prepareShadowing() selectively populates it.
    static_assert(LightSoa::getArrayCount() == 13,
            "Update lightData.copyRange column list when LightSoa columns change");
    if (positionalLightCount > 0) {
        lightData.copyRange<
                LIGHT_ENTITY,
                RADIUS,
                DIRECTION,
                SPOT_PARAMS
        >(DIRECTIONAL_LIGHTS_COUNT, cache.lightCache.getSoA(), 0, positionalLightCount);

        double3 const* UTILS_RESTRICT const lTranslations =
                cache.lightCache.getTranslations().data();
        float const* UTILS_RESTRICT const lRadii = cache.lightCache.getSoA().data<RADIUS>();
        float4* UTILS_RESTRICT const outPosRadius =
                lightData.data<POSITION_RADIUS>() + DIRECTIONAL_LIGHTS_COUNT;
        float* UTILS_RESTRICT const outPosX =
                lightData.data<POSITION_X>() + DIRECTIONAL_LIGHTS_COUNT;
        float* UTILS_RESTRICT const outPosY =
                lightData.data<POSITION_Y>() + DIRECTIONAL_LIGHTS_COUNT;
        float* UTILS_RESTRICT const outPosZ =
                lightData.data<POSITION_Z>() + DIRECTIONAL_LIGHTS_COUNT;
        ShadowInfo* UTILS_RESTRICT const outShadowInfo =
                lightData.data<SHADOW_INFO>() + DIRECTIONAL_LIGHTS_COUNT;
        for (size_t i = 0; i < positionalLightCount; ++i) {
            float3 const p = float3(lTranslations[i] + worldTranslation);
            outPosRadius[i] = float4{ p, lRadii[i] };
            outPosX[i] = p.x;
            outPosY[i] = p.y;
            outPosZ[i] = p.z;
            outShadowInfo[i] = ShadowInfo{};
        }
    }
}

void FScene::prepareVisibleRenderables(Range<uint32_t> visibleRenderables, SceneCacheData& cache) const noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    RenderableSoa& sceneData = cache.renderableData;

    cache.renderablesHaveContactShadows = false;
    for (uint32_t const i : visibleRenderables) {
        PerRenderableData& uboData = sceneData.elementAt<UBO>(i);

        auto const visibility = sceneData.elementAt<VISIBILITY_STATE>(i);
        auto const skinning = sceneData.elementAt<SKINNING_STATE>(i);
        auto const& model = sceneData.elementAt<WORLD_TRANSFORM>(i);
        auto const entity = sceneData.elementAt<RENDERABLE_ENTITY>(i);

        // Using mat3f::getTransformForNormals handles non-uniform scaling, but DOESN'T guarantee that
        // the transformed normals will have unit-length, therefore they need to be normalized
        // in the shader (that's already the case anyway, since normalization is needed after
        // interpolation).
        //
        // We pre-scale normals by the inverse of the largest scale factor to avoid
        // large post-transform magnitudes in the shader, especially in the fragment shader, where
        // we use medium precision.
        //
        // Note: if the model matrix is known to be a rigid-transform, we could just use it directly.

        mat3f m = mat3f::getTransformForNormals(model.upperLeft());
        m = prescaleForNormals(m);

        // The shading normal must be flipped for mirror transformations.
        // Basically we're shading the other side of the polygon and therefore need to negate the
        // normal, similar to what we already do to support double-sided lighting.
        if (visibility.reversedWindingOrder) {
            m = -m;
        }

        uboData.worldFromModelMatrix = model;

        uboData.worldFromModelNormalMatrix = m;

        uboData.flagsChannels = PerRenderableData::packFlagsChannels(
                skinning.skinning,
                static_cast<uint8_t>(skinning.morphType),
                visibility.screenSpaceContactShadows,
                sceneData.elementAt<INSTANCES>(i).buffer != nullptr,
                sceneData.elementAt<CHANNELS>(i));

        uboData.morphTargetCount = sceneData.elementAt<MORPHING_BUFFER>(i).count;

        uboData.objectId = int32_t(entity.getId());

        // TODO: We need a better way to provide the scale information per object
        uboData.userData = sceneData.elementAt<USER_DATA>(i);

        cache.renderablesHaveContactShadows = cache.renderablesHaveContactShadows || visibility.screenSpaceContactShadows;
    }
}

void FScene::terminate(FEngine&) {
    while (!mRegisteredViews.empty()) {
        FView* const view = mRegisteredViews.back();
        unregisterView(view);
        view->detachScene(this);
    }
}

void FScene::prepareDynamicLights(const CameraInfo& camera,
        Handle<HwBufferObject> lightUbh, LightSoa& lightData) const noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    FLightManager const& lcm = mEngine.getLightManager();

    /*
     * Here we copy our lights data into the GPU buffer.
     */

    size_t const size = lightData.size();
    // number of point-light/spotlights
    size_t const positionalLightCount = size - DIRECTIONAL_LIGHTS_COUNT;
    assert_invariant(positionalLightCount);

    float4 const* const UTILS_RESTRICT spheres = lightData.data<POSITION_RADIUS>();

    // compute the light ranges (needed when building light trees)
    float2* const zrange = lightData.data<SCREEN_SPACE_Z_RANGE>();
    computeLightRanges(zrange, camera, spheres + DIRECTIONAL_LIGHTS_COUNT, positionalLightCount);

    LightsUib* const lp = driver.allocatePod<LightsUib>(positionalLightCount);

    auto const* UTILS_RESTRICT directions       = lightData.data<DIRECTION>();
    auto const* UTILS_RESTRICT entities         = lightData.data<LIGHT_ENTITY>();
    auto const* UTILS_RESTRICT shadowInfo       = lightData.data<SHADOW_INFO>();
    for (size_t i = DIRECTIONAL_LIGHTS_COUNT, c = size; i < c; ++i) {
        const size_t gpuIndex = i - DIRECTIONAL_LIGHTS_COUNT;
        auto const li = lcm.getInstance(entities[i]);
        lp[gpuIndex].positionFalloff      = { spheres[i].xyz, lcm.getSquaredFalloffInv(li) };
        lp[gpuIndex].direction            = directions[i];
        lp[gpuIndex].reserved1            = {};
        lp[gpuIndex].colorIES             = { lcm.getColor(li), 0.0f };
        lp[gpuIndex].spotScaleOffset      = lcm.getSpotParams(li).scaleOffset;
        lp[gpuIndex].reserved3            = {};
        lp[gpuIndex].intensity            = lcm.getIntensity(li);
        lp[gpuIndex].typeShadow           = LightsUib::packTypeShadow(
                lcm.isPointLight(li) ? 0u : 1u,
                shadowInfo[i].contactShadows,
                shadowInfo[i].index);
        lp[gpuIndex].channels             = LightsUib::packChannels(
                lcm.getLightChannels(li),
                shadowInfo[i].castsShadows);
    }

    driver.updateBufferObject(lightUbh, {lp, positionalLightCount * sizeof(LightsUib)}, 0);
}

bool FScene::hasContactShadows(SceneCacheData const& cache) const noexcept {
    return cache.renderablesHaveContactShadows && cache.lightsHaveContactShadows;
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

    for (size_t i = 0; i < count; i++) {
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
        const float max = (f.w < camera.zf) ? (f.z / f.w) : 1.0f;
        // convert to screen space
        zrange[i].x = (min + 1.0f) * 0.5f;
        zrange[i].y = (max + 1.0f) * 0.5f;
    }
}

void FScene::registerView(FView* view) {
    auto it = std::find(mRegisteredViews.begin(), mRegisteredViews.end(), view);
    assert_invariant(it == mRegisteredViews.end());
    if (it == mRegisteredViews.end()) {
        mRegisteredViews.push_back(view);
        auto& engine = mEngine;
        FRenderableManager& rcm = engine.getRenderableManager();
        FLightManager& lcm = engine.getLightManager();
        FTransformManager& tcm = engine.getTransformManager();
        if (auto* cache = view->getSceneCache(); cache) {
            cache->needsFullUpdate = true;
            cache->dirtyEntities.clear();
            cache->removedEntities.clear();
            rcm.registerBitset(&cache->dirtyEntities, &mEntities);
            lcm.registerBitset(&cache->dirtyEntities, &mEntities);
            tcm.registerBitset(&cache->dirtyEntities, &mEntities);
        }
    }
}

void FScene::unregisterView(FView* view) {
    auto it = std::find(mRegisteredViews.begin(), mRegisteredViews.end(), view);
    assert_invariant(it != mRegisteredViews.end());
    if (it != mRegisteredViews.end()) {
        flushPendingDestroyed();
        auto& engine = mEngine;
        FRenderableManager& rcm = engine.getRenderableManager();
        FLightManager& lcm = engine.getLightManager();
        FTransformManager& tcm = engine.getTransformManager();
        if (auto* cache = view->getSceneCache(); cache) {
            rcm.unregisterBitset(&cache->dirtyEntities);
            lcm.unregisterBitset(&cache->dirtyEntities);
            tcm.unregisterBitset(&cache->dirtyEntities);
        }
        mRegisteredViews.erase(it);
    }
}

UTILS_NOINLINE
void FScene::addEntity(Entity const entity) {
    // A destroyed (or null) Entity can never be rendered. It must not enter mEntities either:
    // its EntityManager destruction notification was delivered before this call, so nothing
    // would ever prune it.
    if (UTILS_UNLIKELY(!mEngine.getEntityManager().isAlive(entity))) {
        return;
    }
    if (!mEntities.fetchAdd(entity.getId())) {
        for (FView* view : mRegisteredViews) {
            if (auto* cache = view->getSceneCache(); cache) {
                cache->dirtyEntities.add(entity.getId());
            }
        }
    }
}

UTILS_NOINLINE
void FScene::addEntities(Slice<const Entity> entities) {
    for (Entity const entity : entities) {
        addEntity(entity);
    }
}

UTILS_NOINLINE
void FScene::remove(Entity const entity) {
    if (mEntities.fetchRemove(entity.getId())) {
        for (FView* view : mRegisteredViews) {
            if (auto* cache = view->getSceneCache(); cache) {
                cache->removedEntities.add(entity.getId());
            }
        }
    }
}

UTILS_NOINLINE
void FScene::removeEntities(Slice<const Entity> entities) {
    for (Entity const entity : entities) {
        remove(entity);
    }
}

UTILS_NOINLINE
void FScene::removeAllEntities() noexcept {
    for (FView* view : mRegisteredViews) {
        if (auto* cache = view->getSceneCache(); cache) {
            cache->removedEntities.merge(mEntities);
        }
    }
    mEntities.clear();
}

UTILS_NOINLINE
size_t FScene::getRenderableCount() const noexcept {
    FEngine& engine = mEngine;
    EntityManager const& em = engine.getEntityManager();
    FRenderableManager const& rcm = engine.getRenderableManager();
    return EntitySet::intersectSize(em.getAliveEntities(), rcm.getEntityBitset(), mEntities);
}

UTILS_NOINLINE
size_t FScene::getLightCount() const noexcept {
    FEngine& engine = mEngine;
    EntityManager const& em = engine.getEntityManager();
    FLightManager const& lcm = engine.getLightManager();
    return EntitySet::intersectSize(em.getAliveEntities(), lcm.getEntityBitset(), mEntities);
}

UTILS_NOINLINE
bool FScene::hasEntity(Entity const entity) const noexcept {
    return mEntities[entity.getId()];
}

UTILS_NOINLINE
void FScene::setSkybox(FSkybox* skybox) noexcept {
    std::swap(mSkybox, skybox);
    if (skybox) {
        remove(skybox->getEntity());
    }
    if (mSkybox) {
        addEntity(mSkybox->getEntity());
    }
}


UTILS_NOINLINE
void FScene::forEach(Invocable<void(Entity)>&& functor) const noexcept {
    mEntities.forEachSetBit([&](uint32_t const bit) {
        functor(Entity::import(int32_t(bit)));
    });
}
} // namespace filament
