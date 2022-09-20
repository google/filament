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

#include <private/filament/UibStructs.h>

#include "details/Engine.h"
#include "details/IndirectLight.h"
#include "details/Skybox.h"

#include "BufferPoolAllocator.h"

#include <utils/compiler.h>
#include <utils/EntityManager.h>
#include <utils/Range.h>
#include <utils/Systrace.h>

#include <algorithm>

using namespace filament::backend;
using namespace filament::math;
using namespace utils;

namespace filament {

// ------------------------------------------------------------------------------------------------

FScene::FScene(FEngine& engine) :
        mEngine(engine) {
}

FScene::~FScene() noexcept = default;


void FScene::prepare(const mat4& worldOriginTransform, bool shadowReceiversAreCasters) noexcept {
    // TODO: can we skip this in most cases? Since we rely on indices staying the same,
    //       we could only skip, if nothing changed in the RCM.

    SYSTRACE_CALL();

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
        if (!ri && !li) {
            continue;
        }

        // get the world transform
        auto ti = tcm.getInstance(e);
        // this is where we go from double to float for our transforms
        const mat4f worldTransform{ worldOriginTransform * tcm.getWorldTransformAccurate(ti) };
        const bool reversedWindingOrder = det(worldTransform.upperLeft()) < 0;

        // don't even draw this object if it doesn't have a transform (which shouldn't happen
        // because one is always created when creating a Renderable component).
        if (ri && ti) {
            // compute the world AABB so we can perform culling
            const Box worldAABB = rigidTransform(rcm.getAABB(ri), worldTransform);

            auto visibility = rcm.getVisibility(ri);
            visibility.reversedWindingOrder = reversedWindingOrder;
            if (shadowReceiversAreCasters && visibility.receiveShadows) {
                visibility.castShadows = true;
            }

            // FIXME: We compute and store the local scale because it's needed for glTF but
            //        we need a better way to handle this
            const mat4f& transform = tcm.getTransform(ti);
            float scale = (length(transform[0].xyz) + length(transform[1].xyz) +
                    length(transform[2].xyz)) / 3.0f;

            // we know there is enough space in the array
            sceneData.push_back_unsafe(
                    ri,                             // RENDERABLE_INSTANCE
                    worldTransform,                 // WORLD_TRANSFORM
                    visibility,                     // VISIBILITY_STATE
                    rcm.getSkinningBufferInfo(ri),  // SKINNING_BUFFER
                    rcm.getMorphingBufferInfo(ri),  // MORPHING_BUFFER
                    worldAABB.center,               // WORLD_AABB_CENTER
                    0,                              // VISIBLE_MASK
                    rcm.getChannels(ri),            // CHANNELS
                    rcm.getInstanceCount(ri),       // INSTANCE_COUNT
                    rcm.getLayerMask(ri),           // LAYERS
                    worldAABB.halfExtent,           // WORLD_AABB_EXTENT
                    {},                             // PRIMITIVES
                    0,                              // SUMMED_PRIMITIVE_COUNT
                    {},                             // UBO
                    scale                           // USER_DATA
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
    for (size_t i = lightData.size(), e = lightDataCapacity; i < e; i++) {
        new(lightData.data<POSITION_RADIUS>() + i) float4{ 0, 0, 0, 1 };
    }

    // Purely for the benefit of MSAN, we can avoid uninitialized reads by zeroing out the
    // unused scene elements between the end of the array and the rounded-up count.
    if (UTILS_HAS_SANITIZE_MEMORY) {
        for (size_t i = sceneData.size(), e = renderableDataCapacity; i < e; i++) {
            sceneData.data<LAYERS>()[i] = 0;
            sceneData.data<VISIBLE_MASK>()[i] = 0;
            sceneData.data<VISIBILITY_STATE>()[i] = {};
        }
    }
}

void FScene::prepareVisibleRenderables(Range<uint32_t> visibleRenderables) noexcept {
    RenderableSoa& sceneData = mRenderableData;
    FRenderableManager& rcm = mEngine.getRenderableManager();

    mHasContactShadows = false;
    for (uint32_t i : visibleRenderables) {
        PerRenderableData& uboData = sceneData.elementAt<UBO>(i);

        auto const visibility = sceneData.elementAt<VISIBILITY_STATE>(i);
        auto const& model = sceneData.elementAt<WORLD_TRANSFORM>(i);
        auto const ri = sceneData.elementAt<RENDERABLE_INSTANCE>(i);

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
        m *= mat3f(1.0f / std::sqrt(max(float3{length2(m[0]), length2(m[1]), length2(m[2])})));

        // The shading normal must be flipped for mirror transformations.
        // Basically we're shading the other side of the polygon and therefore need to negate the
        // normal, similar to what we already do to support double-sided lighting.
        if (visibility.reversedWindingOrder) {
            m = -m;
        }

        uboData.worldFromModelMatrix = model;

        uboData.worldFromModelNormalMatrix = m;

        uboData.flagsChannels = PerRenderableData::packFlagsChannels(
                visibility.skinning,
                visibility.morphing,
                visibility.screenSpaceContactShadows,
                sceneData.elementAt<CHANNELS>(i));

        uboData.morphTargetCount = sceneData.elementAt<MORPHING_BUFFER>(i).count;

        uboData.objectId = rcm.getEntity(ri).getId();

        // TODO: We need to find a better way to provide the scale information per object
        uboData.userData = sceneData.elementAt<USER_DATA>(i);

        mHasContactShadows = mHasContactShadows || visibility.screenSpaceContactShadows;
    }
}

void FScene::updateUBOs(
        Range<uint32_t> visibleRenderables,
        Handle<HwBufferObject> renderableUbh) noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();

    // store the UBO handle
    mRenderableViewUbh = renderableUbh;

    // don't allocate more than 16 KiB directly into the render stream
    static constexpr size_t MAX_STREAM_ALLOCATION_COUNT = 64;   // 16 KiB
    const size_t count = visibleRenderables.size();
    PerRenderableData* buffer = [&]{
        if (count >= MAX_STREAM_ALLOCATION_COUNT) {
            // use the heap allocator
            return (PerRenderableData*)mBufferPoolAllocator.get(count * sizeof(PerRenderableData));
        } else {
            // allocate space into the command stream directly
            return driver.allocatePod<PerRenderableData>(count);
        }
    }();

    // copy our data into the UBO for each visible renderable
    PerRenderableData const* const uboData = mRenderableData.data<UBO>();
    for (uint32_t i : visibleRenderables) {
        buffer[i] = uboData[i];
    }

    // update the UBO
    driver.resetBufferObject(renderableUbh);
    driver.updateBufferObjectUnsynchronized(renderableUbh, {
            buffer, count * sizeof(PerRenderableData),
            +[](void* p, size_t s, void* user) {
                if (s >= MAX_STREAM_ALLOCATION_COUNT * sizeof(PerRenderableData)) {
                    FScene* const that = static_cast<FScene*>(user);
                    that->mBufferPoolAllocator.put(p);
                }
            }, this
    }, 0);

    // update skybox
    if (mSkybox) {
        mSkybox->commit(driver);
    }
}

void FScene::terminate(FEngine& engine) {
    // DO NOT destroy this UBO, it's owned by the View
    mRenderableViewUbh.clear();
}

void FScene::prepareDynamicLights(const CameraInfo& camera, ArenaScope& rootArena,
        Handle<HwBufferObject> lightUbh) noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    FLightManager& lcm = mEngine.getLightManager();
    FScene::LightSoa& lightData = getLightData();

    /*
     * Here we copy our lights data into the GPU buffer.
     */

    size_t const size = lightData.size();
    // number of point/spot lights
    size_t positionalLightCount = size - DIRECTIONAL_LIGHTS_COUNT;
    assert_invariant(positionalLightCount);

    float4 const* const UTILS_RESTRICT spheres = lightData.data<FScene::POSITION_RADIUS>();

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
        lp[gpuIndex].direction            = directions[i];
        lp[gpuIndex].reserved1            = {};
        lp[gpuIndex].colorIES             = { lcm.getColor(li), 0.0f };
        lp[gpuIndex].spotScaleOffset      = lcm.getSpotParams(li).scaleOffset;
        lp[gpuIndex].reserved3            = {};
        lp[gpuIndex].intensity            = lcm.getIntensity(li);
        lp[gpuIndex].typeShadow           = LightsUib::packTypeShadow(
                lcm.isPointLight(li) ? 0u : 1u,
                shadowInfo[i].contactShadows,
                shadowInfo[i].index,
                shadowInfo[i].layer);
        lp[gpuIndex].channels             = LightsUib::packChannels(lcm.getLightChannels(li), shadowInfo[i].castsShadows);
    }

    driver.updateBufferObject(lightUbh, { lp, positionalLightCount * sizeof(LightsUib) }, 0);
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

UTILS_NOINLINE
void FScene::addEntity(Entity entity) {
    mEntities.insert(entity);
}

UTILS_NOINLINE
void FScene::addEntities(const Entity* entities, size_t count) {
    mEntities.insert(entities, entities + count);
}

UTILS_NOINLINE
void FScene::remove(Entity entity) {
    mEntities.erase(entity);
}

UTILS_NOINLINE
void FScene::removeEntities(const Entity* entities, size_t count) {
    for (size_t i = 0; i < count; ++i, ++entities) {
        remove(*entities);
    }
}

UTILS_NOINLINE
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

UTILS_NOINLINE
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

UTILS_NOINLINE
bool FScene::hasEntity(Entity entity) const noexcept {
    return mEntities.find(entity) != mEntities.end();
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

bool FScene::hasContactShadows() const noexcept {
    // at least some renderables in the scene must have contact-shadows enabled
    // TODO: we should refine this with only the visible ones
    if (!mHasContactShadows) {
        return false;
    }

    // find out if at least one light has contact-shadow enabled
    // TODO: we could cache the result of this Loop in the LightManager
    auto& lcm = mEngine.getLightManager();
    const auto *pFirst = mLightData.begin<LIGHT_INSTANCE>();
    const auto *pLast = mLightData.end<LIGHT_INSTANCE>();
    while (pFirst != pLast) {
        if (pFirst->isValid()) {
            auto const& shadowOptions = lcm.getShadowOptions(*pFirst);
            if (shadowOptions.screenSpaceContactShadows) {
                return true;
            }
        }
        ++pFirst;
    }
    return false;
}

UTILS_NOINLINE
void FScene::forEach(Invocable<void(Entity)>&& functor) const noexcept {
    std::for_each(mEntities.begin(), mEntities.end(), std::move(functor));
}

} // namespace filament
