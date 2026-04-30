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

#include "details/Engine.h"
#include "details/Skybox.h"
#include "details/View.h"

#include <private/filament/UibStructs.h>

#include <private/utils/Tracing.h>

#include <filament/Box.h>
#include <filament/LightManager.h>

#include <backend/Handle.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/Invocable.h>
#include <utils/PagedArenaBitset.h>
#include <utils/Range.h>
#include <utils/Slice.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <tuple>
#include <utility>
#include <new>


using namespace filament::backend;
using namespace filament::math;
using namespace utils;

namespace filament {

// Explicitly define the architectural boundaries we rely on
constexpr size_t BITSET_PAGE_BITS = (1U << PagedArenaBitset::PAGE_SHIFT);
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

    em.registerChangeCallback(this, [this](Slice<const Entity> entities) {
        for (auto& [view, cache] : mViewCaches) {
            std::lock_guard const lock(cache->destroyedEntitiesLock);
            for (Entity const e : entities) {
                cache->destroyedEntities.add(e.getId());
            }
        }
    });
}

FScene::~FScene() noexcept {
    mEngine.getEntityManager().unregisterChangeCallback(this);
}

void FScene::SceneCacheData::invalidate() noexcept {
    renderableData.clear();
    lightData.clear();
    renderablesHaveContactShadows = false;
    lightsHaveContactShadows = false;
}

SceneCacheData::EntitySet& SceneCacheData::getAndClearDestroyedEntities(EntitySet& out) noexcept {
    std::lock_guard const lock(destroyedEntitiesLock);
    destroyedEntities.swap(out);
    destroyedEntities.clear();
    return out;
}

bool SceneCacheData::hasDestroyedEntities() const noexcept {
    std::lock_guard const lock(destroyedEntitiesLock);
    return !destroyedEntities.empty();
}

bool SceneCacheData::hasDirtyEntities() const noexcept {
    return !removedEntities.empty() || !dirtyEntities.empty() || hasDestroyedEntities();
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

    // We always reserve at least 1 entry for the directional light
    // Capacity is always multiple of 16 (to allow SIMD operations)
    size_t const lightDataCapacity = (std::max<size_t>(1, lightCount) + 0xFu) & ~0xFu;
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
    }

    // Purely for the benefit of MSAN, we can avoid uninitialized reads by zeroing out the
    // unused scene elements between the end of the array and the rounded-up count.
    if constexpr (UTILS_HAS_SANITIZE_MEMORY) {
        for (size_t i = renderableData.size(), e = renderableData.capacity(); i < e; i++) {
            renderableData.data<FScene::LAYERS>()[i] = 0;
            renderableData.data<FScene::VISIBLE_MASK>()[i] = 0;
            renderableData.data<FScene::VISIBILITY_STATE>()[i] = {};
            renderableData.data<FScene::SKINNING_STATE>()[i] = {};
        }
    }
}

void FScene::flushNotifications() const noexcept {
    mEngine.getEntityManager().flushNotifications();
}

static FScene::RenderableSoa::Structure computeRenderableData(
        FEngine& engine,
        Entity e,
        FRenderableManager::Instance ri,
        FTransformManager::Instance const ti,
        mat4 const& worldTransform,
        bool const shadowReceiversAreCasters) {
    FRenderableManager const& rcm = engine.getRenderableManager();
    FTransformManager const& tcm = engine.getTransformManager();

    const mat4f shaderWorldTransform{
        worldTransform * tcm.getWorldTransformAccurate(ti)
    };
    const bool reversedWindingOrder = det(shaderWorldTransform.upperLeft()) < 0;

    const Box worldAABB = rigidTransform(rcm.getAABB(ri), shaderWorldTransform);

    auto visibility = rcm.getVisibility(ri);
    visibility.reversedWindingOrder = reversedWindingOrder;
    if (shadowReceiversAreCasters && visibility.receiveShadows) {
        visibility.castShadows = true;
    }

    const mat4f& transform = tcm.getTransform(ti);
    float const scale = (length(transform[0].xyz) + length(transform[1].xyz) +
        length(transform[2].xyz)) / 3.0f;

    return std::make_tuple(
            e,
            shaderWorldTransform,
            visibility,
            rcm.getSkinning(ri),
            rcm.getSkinningBufferInfo(ri),
            rcm.getMorphingBufferInfo(ri),
            rcm.getInstancesInfo(ri),
            worldAABB.center,
            static_cast<FScene::VisibleMaskType>(0),
            rcm.getLightChannels(ri),
            rcm.getLayerMask(ri),
            worldAABB.halfExtent,
            rcm.getRenderPrimitives(ri, 0),
            static_cast<uint32_t>(0),
            PerRenderableData{},
            DescriptorSetHandle{},
            scale);
}

static FScene::LightSoa::Structure computeLightData(
        FEngine& engine,
        Entity e,
        FLightManager::Instance li,
        FTransformManager::Instance ti,
        mat4 const& worldTransform) {
    FLightManager const& lcm = engine.getLightManager();
    FTransformManager const& tcm = engine.getTransformManager();

    mat4f const shaderWorldTransform{
        worldTransform * tcm.getWorldTransformAccurate(ti)
    };
    float4 const position = shaderWorldTransform * float4{lcm.getLocalPosition(li), 1};
    float3 d = 0;
    if (!lcm.isPointLight(li) || lcm.isIESLight(li)) {
        d = lcm.getLocalDirection(li);
        d = normalize(mat3f::getTransformForNormals(shaderWorldTransform.upperLeft()) * d);
    }

    return std::make_tuple(
            e,
            float4{position.xyz, lcm.getRadius(li)},
            d,
            float3{0},
            double2{0},
            float2{lcm.getCosOuterSquared(li), lcm.getSinInverse(li)},
            static_cast<Culler::result_type>(0),
            float2{0},
            FScene::ShadowInfo{});
}

bool FScene::processDeferredUpdates(mat4 const& worldTransform, bool shadowReceiversAreCasters,
        SceneCacheData& cache) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    bool const viewStateChanged = (worldTransform != cache.previousWorldTransform ||
            shadowReceiversAreCasters != cache.previousShadowReceiversAreCasters);

    // Very fast path when nothing has changed at all.
    if (UTILS_LIKELY(!viewStateChanged && !cache.hasDirtyEntities())) {
        return false;
    }

    // Process deferred removals from the Scene _and_ dead entities.
    FEngine& engine = mEngine;
    UTILS_UNUSED EntityManager const& em = engine.getEntityManager();
    FRenderableManager const& rcm = engine.getRenderableManager();
    FLightManager const& lcm = engine.getLightManager();
    FTransformManager const& tcm = engine.getTransformManager();

    // Safely retrieve the thread-safe list and clear it
    EntitySet& destroyedEntities = cache.getAndClearDestroyedEntities(mTempDestroyedEntities);
    // filter the destroyed Entities by the ones in our Scene
    destroyedEntities.intersect(mEntities);
    // remove dead Entities from the Scene (and from the cache below)
    mEntities.difference(destroyedEntities);
    // and merge the entities that were removed from the scene (but could have been re-added!)
    destroyedEntities.merge(cache.removedEntities);
    // cache invalidation and scene removal
    destroyedEntities.forEachSetBit([&](uint32_t const bit) {
        // invalidate the cache for all these Entities
        Entity const e = Entity::import(int32_t(bit));
        cache.renderableCache.remove(e);
        cache.lightCache.remove(e);
        cache.directionalLights.remove(bit);
        // An Entity could have been re-added, so it could be in mEntities at this point, in that case, it will
        // also be in mDirtyEntities
        assert_invariant(!cache.removedEntities[bit] || (!mEntities[bit] || cache.dirtyEntities[bit]));
    });

    // Check the states that require a full cache rebuild
    if (UTILS_LIKELY(viewStateChanged)) {
        cache.previousWorldTransform = worldTransform;
        cache.previousShadowReceiversAreCasters = shadowReceiversAreCasters;
        cache.dirtyEntities = mEntities.clone(); // Mark all as dirty for full rebuild
    }

    // Process modified entities
    EntitySet const activeDirty = EntitySet::intersect(mEntities, cache.dirtyEntities, em.getAliveEntities());

    // We assume the overwhelmingly common case is that all components have a transform
    std::optional<EntitySet> temp;
    if (UTILS_UNLIKELY(!activeDirty.empty() && activeDirty.isSubsetOf(tcm.getEntityBitset()))) {
        // uncommon case, we redo the intersection but this time we store the result
        temp = EntitySet::intersect(activeDirty, tcm.getEntityBitset());
    }
    EntitySet const& hasTransform = temp ? *temp : activeDirty;

    // remove (from the cache) entities that have lost their TransformManager component
    if (UTILS_UNLIKELY(activeDirty.size() > hasTransform.size())) {
        EntitySet const lostTransform = EntitySet::difference(activeDirty, tcm.getEntityBitset());
        lostTransform.forEachSetBit([&](uint32_t const bit) {
            Entity const e = Entity::import(int32_t(bit));
            cache.renderableCache.remove(e);
            cache.lightCache.remove(e);
            cache.directionalLights.remove(bit);
        });
    }

    // remove (from the cache) renderables that have lost their RenderableManager component
    EntitySet const renderableEntities = EntitySet::intersect(hasTransform, rcm.getEntityBitset());
    if (UTILS_UNLIKELY(hasTransform.size() > renderableEntities.size())) {
        // An entity needs to be removed if it HAS a transform (was valid) but NO LONGER has the component.
        EntitySet::difference(&mTempToRemoveRenderables, hasTransform, rcm.getEntityBitset());
        mTempToRemoveRenderables.forEachSetBit([&](uint32_t const bit) {
            Entity const e = Entity::import(int32_t(bit));
            cache.renderableCache.remove(e);
        });
    }

    // remove (from the cache) lights that have lost their LightManager component
    EntitySet const lightEntities = EntitySet::intersect(hasTransform, lcm.getEntityBitset());
    if (UTILS_UNLIKELY(hasTransform.size() > lightEntities.size())) {
        // An entity needs to be removed if it HAS a transform (was valid) but NO LONGER has the component.
        EntitySet::difference(&mTempToLights, hasTransform, lcm.getEntityBitset());
        mTempToLights.forEachSetBit([&](uint32_t const bit) {
            Entity const e = Entity::import(int32_t(bit));
            cache.lightCache.remove(e);
            cache.directionalLights.remove(bit);
        });
    }

    renderableEntities.extractTo(mTempExtractedRenderables);
    for (uint32_t const id : mTempExtractedRenderables) {
        Entity const e = Entity::import(int32_t(id));
        auto const ri = rcm.getInstance(e);
        auto const ti = tcm.getInstance(e);
        cache.renderableCache.update(e, computeRenderableData(engine, e, ri, ti, worldTransform, shadowReceiversAreCasters));
    }

    lightEntities.extractTo(mTempExtractedLights);
    for (uint32_t const id : mTempExtractedLights) {
        Entity const e = Entity::import(int32_t(id));
        auto const li = lcm.getInstance(e);
        auto const ti = tcm.getInstance(e);
        cache.lightCache.update(e, computeLightData(engine, e, li, ti, worldTransform));
        if (lcm.isDirectionalLight(li)) {
            cache.directionalLights.add(id);
        } else {
            cache.directionalLights.remove(id);
        }
    }

    // Search for directional light with max intensity (typically very short loop)
    FLightManager::Instance directionalLight = 0;
    FTransformManager::Instance directionalTransform = 0;
    float maxIntensity = -1.0f;
    cache.directionalLights.forEachSetBit([&](uint32_t const bit) {
        Entity const e = Entity::import(int32_t(bit));
        if (auto const li = lcm.getInstance(e)) {
            float const intensity = lcm.getIntensity(li);
            if (intensity > maxIntensity) {
                maxIntensity = intensity;
                directionalLight = li;
                directionalTransform = tcm.getInstance(e);
                cache.directionalLightIndex = cache.lightCache.getIndex(e);
            }
        }
    });

    // Handle directional light data at index cache.directionalLightIndex
    auto& lightData = cache.lightCache.getSoA();
    if (lightData.size() == 0) {
        lightData.resize(DIRECTIONAL_LIGHTS_COUNT);
    }
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

        lightData.template elementAt<POSITION_RADIUS>(cache.directionalLightIndex) = float4{0, 0, 0, inf};
        lightData.template elementAt<DIRECTION>(cache.directionalLightIndex) = normalize(d);
        lightData.template elementAt<SHADOW_DIRECTION>(cache.directionalLightIndex) = normalize(s);
        lightData.template elementAt<SHADOW_REF>(cache.directionalLightIndex) = lsReferencePoint;
    }
    cache.lightsHaveContactShadows = false;
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

    cache.dirtyEntities.clear();
    cache.removedEntities.clear();
    return true;
}

void FScene::prepare(JobSystem&,
        RootArenaScope&,
        mat4 const& worldTransform,
        bool const shadowReceiversAreCasters,
        SceneCacheData& cache) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    flushNotifications();

    bool const changed = processDeferredUpdates(worldTransform, shadowReceiversAreCasters, cache);
    if (!changed) {
        return;
    }

    auto& sceneData = cache.renderableData;
    auto& lightData = cache.lightData;

    // Copy Master Cache (valid across frames) to Scene Cache (valid for this frame)
    // resize the cache to our needs (this can clear it)
    cache.resize(cache.renderableCache.size(), cache.lightCache.size());

    // copy the renderable cache
    sceneData.copyRange(0, cache.renderableCache.getSoA(), 0, cache.renderableCache.size());

    // copy the light cache
    lightData.copyRange(0, cache.lightCache.getSoA(), 0, cache.lightCache.size());
    if (cache.directionalLightIndex != 0 && cache.directionalLightIndex < cache.lightCache.size()) {
        // always more the directional light at index 0
        lightData.swap(0, cache.directionalLightIndex);
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

        // TODO: We need to find a better way to provide the scale information per object
        uboData.userData = sceneData.elementAt<USER_DATA>(i);

        cache.renderablesHaveContactShadows = cache.renderablesHaveContactShadows || visibility.screenSpaceContactShadows;
    }
}

void FScene::terminate(FEngine&) {
    while (!mViewCaches.empty()) {
        mViewCaches.begin()->first->invalidateCache(this);
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

FScene::SceneCacheData* FScene::registerView(FView const* view) {
    auto& engine = mEngine;
    FRenderableManager& rcm = engine.getRenderableManager();
    FLightManager& lcm = engine.getLightManager();
    FTransformManager& tcm = engine.getTransformManager();

    std::unique_ptr<SceneCacheData> cachePtr = std::make_unique<SceneCacheData>();
    SceneCacheData* const cache = cachePtr.get();
    rcm.registerBitset(&cache->dirtyEntities);
    lcm.registerBitset(&cache->dirtyEntities);
    tcm.registerBitset(&cache->dirtyEntities);

    auto const pos = mViewCaches.emplace(view, std::move(cachePtr)).first;
    return pos->second.get();
}

void FScene::unregisterView(FView const* view) {
    auto& engine = mEngine;
    FRenderableManager& rcm = engine.getRenderableManager();
    FLightManager& lcm = engine.getLightManager();
    FTransformManager& tcm = engine.getTransformManager();

    if (auto const node = mViewCaches.extract(view); node) {
        auto const cache = std::move(node.mapped());
        rcm.unregisterBitset(&cache->dirtyEntities);
        lcm.unregisterBitset(&cache->dirtyEntities);
        tcm.unregisterBitset(&cache->dirtyEntities);
    }
}

UTILS_NOINLINE
void FScene::addEntity(Entity const entity) {
    if (!mEntities.fetchAdd(entity.getId())) {
        for (auto& [view, cache] : mViewCaches) {
            cache->dirtyEntities.add(entity.getId());
        }
    }
}

UTILS_NOINLINE
void FScene::addEntities(const Entity* entities, size_t const count) {
    for (size_t i = 0; i < count; ++i, ++entities) {
        addEntity(*entities);
    }
}

UTILS_NOINLINE
void FScene::remove(Entity const entity) {
    if (mEntities.fetchRemove(entity.getId())) {
        for (auto& [view, cache] : mViewCaches) {
            cache->removedEntities.add(entity.getId());
        }
    }
}

UTILS_NOINLINE
void FScene::removeEntities(const Entity* entities, size_t const count) {
    for (size_t i = 0; i < count; ++i, ++entities) {
        remove(*entities);
    }
}

UTILS_NOINLINE
void FScene::removeAllEntities() noexcept {
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
