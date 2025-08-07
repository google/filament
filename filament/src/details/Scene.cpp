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
#include "BufferPoolAllocator.h"

#include "backend/Handle.h"
#include "components/LightManager.h"
#include "components/RenderableManager.h"
#include "components/TransformManager.h"

#include "details/Engine.h"
#include "details/InstanceBuffer.h"
#include "details/Skybox.h"

#include <private/filament/UibStructs.h>
#include <private/utils/Tracing.h>

#include <filament/Box.h>
#include <filament/TransformManager.h>
#include <filament/RenderableManager.h>
#include <filament/LightManager.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <utils/Allocator.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/EntityManager.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Invocable.h>
#include <utils/JobSystem.h>
#include <utils/Range.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <functional>
#include <memory>
#include <utility>
#include <new>

#include <cstddef>

using namespace filament::backend;
using namespace filament::math;
using namespace utils;

namespace filament {

// ------------------------------------------------------------------------------------------------

FScene::FScene(FEngine& engine) :
        mEngine(engine), mSharedState(std::make_shared<SharedState>()) {
}

FScene::~FScene() noexcept = default;


void FScene::prepare(JobSystem& js,
        RootArenaScope& rootArenaScope,
        mat4 const& worldTransform,
        bool shadowReceiversAreCasters) noexcept {
    // TODO: can we skip this in most cases? Since we rely on indices staying the same,
    //       we could only skip, if nothing changed in the RCM.

    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);

    FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT);

    // This will reset the allocator upon exiting
    ArenaScope localArenaScope(rootArenaScope.getArena());

    FEngine& engine = mEngine;
    EntityManager const& em = engine.getEntityManager();
    FRenderableManager const& rcm = engine.getRenderableManager();
    FTransformManager const& tcm = engine.getTransformManager();
    FLightManager const& lcm = engine.getLightManager();
    // go through the list of entities, and gather the data of those that are renderables
    auto& sceneData = mRenderableData;
    auto& lightData = mLightData;
    auto const& entities = mEntities;

    using RenderableContainerData = std::pair<RenderableManager::Instance, TransformManager::Instance>;
    using RenderableInstanceContainer = FixedCapacityVector<RenderableContainerData,
            STLAllocator< RenderableContainerData, LinearAllocatorArena >, false>;

    using LightContainerData = std::pair<LightManager::Instance, TransformManager::Instance>;
    using LightInstanceContainer = FixedCapacityVector<LightContainerData,
            STLAllocator< LightContainerData, LinearAllocatorArena >, false>;

    RenderableInstanceContainer renderableInstances{
            RenderableInstanceContainer::with_capacity(entities.size(), localArenaScope.getArena()) };

    LightInstanceContainer lightInstances{
            LightInstanceContainer::with_capacity(entities.size(), localArenaScope.getArena()) };

    FILAMENT_TRACING_NAME_BEGIN(FILAMENT_TRACING_CATEGORY_FILAMENT, "InstanceLoop");

    // find the max intensity directional light index in our local array
    float maxIntensity = 0.0f;
    std::pair<LightManager::Instance, TransformManager::Instance> directionalLightInstances{};

    /*
     * First compute the exact number of renderables and lights in the scene.
     * Also find the main directional light.
     */

    for (Entity const e: entities) {
        if (UTILS_LIKELY(em.isAlive(e))) {
            auto ti = tcm.getInstance(e);
            auto li = lcm.getInstance(e);
            auto ri = rcm.getInstance(e);
            if (li) {
                // we handle the directional light here because it'd prevent multithreading below
                if (UTILS_UNLIKELY(lcm.isDirectionalLight(li))) {
                    // we don't store the directional lights, because we only have a single one
                    if (lcm.getIntensity(li) >= maxIntensity) {
                        maxIntensity = lcm.getIntensity(li);
                        directionalLightInstances = { li, ti };
                    }
                } else {
                    lightInstances.emplace_back(li, ti);
                }
            }
            if (ri) {
                renderableInstances.emplace_back(ri, ti);
            }
        }
    }

    FILAMENT_TRACING_NAME_END(FILAMENT_TRACING_CATEGORY_FILAMENT);

    /*
     * Evaluate the capacity needed for the renderable and light SoAs
     */

    // we need the capacity to be multiple of 16 for SIMD loops
    // we need 1 extra entry at the end for the summed primitive count
    size_t renderableDataCapacity = entities.size();
    renderableDataCapacity = (renderableDataCapacity + 0xFu) & ~0xFu;
    renderableDataCapacity = renderableDataCapacity + 1;

    // The light data list will always contain at least one entry for the
    // dominating directional light, even if there are no entities.
    // We need the capacity to be multiple of 16 for SIMD loops
    size_t lightDataCapacity = std::max<size_t>(DIRECTIONAL_LIGHTS_COUNT, entities.size());
    lightDataCapacity = (lightDataCapacity + 0xFu) & ~0xFu;

    /*
     * Now resize the SoAs if needed
     */

    // TODO: the resize below could happen in a job

    if (!sceneData.capacity() || sceneData.size() != renderableInstances.size()) {
        sceneData.clear();
        if (sceneData.capacity() < renderableDataCapacity) {
            sceneData.setCapacity(renderableDataCapacity);
        }
        assert_invariant(renderableInstances.size() <= sceneData.capacity());
        sceneData.resize(renderableInstances.size());
    }

    if (lightData.size() != lightInstances.size() + DIRECTIONAL_LIGHTS_COUNT) {
        lightData.clear();
        if (lightData.capacity() < lightDataCapacity) {
            lightData.setCapacity(lightDataCapacity);
        }
        assert_invariant(lightInstances.size() + DIRECTIONAL_LIGHTS_COUNT <= lightData.capacity());
        lightData.resize(lightInstances.size() + DIRECTIONAL_LIGHTS_COUNT);
    }

    /*
     * Fill the SoA with the JobSystem
     */

    auto renderableWork = [first = renderableInstances.data(), &rcm, &tcm, &worldTransform,
                 &sceneData, shadowReceiversAreCasters](auto* p, auto c) {
        FILAMENT_TRACING_NAME(FILAMENT_TRACING_CATEGORY_FILAMENT, "renderableWork");

        for (size_t i = 0; i < c; i++) {
            auto [ri, ti] = p[i];

            // this is where we go from double to float for our transforms
            const mat4f shaderWorldTransform{
                    worldTransform * tcm.getWorldTransformAccurate(ti) };
            const bool reversedWindingOrder = det(shaderWorldTransform.upperLeft()) < 0;

            // compute the world AABB so we can perform culling
            const Box worldAABB = rigidTransform(rcm.getAABB(ri), shaderWorldTransform);

            auto visibility = rcm.getVisibility(ri);
            visibility.reversedWindingOrder = reversedWindingOrder;
            if (shadowReceiversAreCasters && visibility.receiveShadows) {
                visibility.castShadows = true;
            }

            // FIXME: We compute and store the local scale because it's needed for glTF but
            //        we need a better way to handle this
            const mat4f& transform = tcm.getTransform(ti);
            float const scale = (length(transform[0].xyz) + length(transform[1].xyz) +
                                 length(transform[2].xyz)) / 3.0f;

            size_t const index = std::distance(first, p) + i;
            assert_invariant(index < sceneData.size());

            sceneData.elementAt<RENDERABLE_INSTANCE>(index) = ri;
            sceneData.elementAt<WORLD_TRANSFORM>(index)     = shaderWorldTransform;
            sceneData.elementAt<VISIBILITY_STATE>(index)    = visibility;
            sceneData.elementAt<SKINNING_BUFFER>(index)     = rcm.getSkinningBufferInfo(ri);
            sceneData.elementAt<MORPHING_BUFFER>(index)     = rcm.getMorphingBufferInfo(ri);
            sceneData.elementAt<INSTANCES>(index)           = rcm.getInstancesInfo(ri);
            sceneData.elementAt<WORLD_AABB_CENTER>(index)   = worldAABB.center;
            sceneData.elementAt<VISIBLE_MASK>(index)        = 0;
            sceneData.elementAt<CHANNELS>(index)            = rcm.getChannels(ri);
            sceneData.elementAt<LAYERS>(index)              = rcm.getLayerMask(ri);
            sceneData.elementAt<WORLD_AABB_EXTENT>(index)   = worldAABB.halfExtent;
            //sceneData.elementAt<PRIMITIVES>(index)          = {}; // already initialized, Slice<>
            sceneData.elementAt<SUMMED_PRIMITIVE_COUNT>(index) = 0;
            //sceneData.elementAt<UBO>(index)                 = {}; // not needed here
            sceneData.elementAt<USER_DATA>(index)           = scale;
        }
    };

    auto lightWork = [first = lightInstances.data(), &lcm, &tcm, &worldTransform,
            &lightData](auto* p, auto c) {
        FILAMENT_TRACING_NAME(FILAMENT_TRACING_CATEGORY_FILAMENT, "lightWork");
        for (size_t i = 0; i < c; i++) {
            auto [li, ti] = p[i];
            // this is where we go from double to float for our transforms
            mat4f const shaderWorldTransform{
                    worldTransform * tcm.getWorldTransformAccurate(ti) };
            float4 const position = shaderWorldTransform * float4{ lcm.getLocalPosition(li), 1 };
            float3 d = 0;
            if (!lcm.isPointLight(li) || lcm.isIESLight(li)) {
                d = lcm.getLocalDirection(li);
                // using mat3f::getTransformForNormals handles non-uniform scaling
                d = normalize(mat3f::getTransformForNormals(shaderWorldTransform.upperLeft()) * d);
            }
            size_t const index = DIRECTIONAL_LIGHTS_COUNT + std::distance(first, p) + i;
            assert_invariant(index < lightData.size());
            lightData.elementAt<POSITION_RADIUS>(index) = float4{ position.xyz, lcm.getRadius(li) };
            lightData.elementAt<DIRECTION>(index) = d;
            lightData.elementAt<LIGHT_INSTANCE>(index) = li;
        }
    };


    FILAMENT_TRACING_NAME_BEGIN(FILAMENT_TRACING_CATEGORY_FILAMENT, "Renderable and Light jobs");

    JobSystem::Job* rootJob = js.createJob();

    auto* renderableJob = parallel_for(js, rootJob,
            renderableInstances.data(), renderableInstances.size(),
            std::cref(renderableWork), jobs::CountSplitter<64>());

    auto* lightJob = parallel_for(js, rootJob,
            lightInstances.data(), lightInstances.size(),
            std::cref(lightWork), jobs::CountSplitter<32, 5>());

    js.run(renderableJob);
    js.run(lightJob);

    // Everything below can be done in parallel.

    /*
     * Handle the directional light separately
     */

    if (auto [li, ti] = directionalLightInstances ; li) {
        // in the code below, we only transform directions, so the translation of the
        // world transform is irrelevant, and we don't need to use getWorldTransformAccurate()

        mat3 const worldDirectionTransform =
                mat3::getTransformForNormals(tcm.getWorldTransformAccurate(ti).upperLeft());
        FLightManager::ShadowParams const params = lcm.getShadowParams(li);
        float3 const localDirection = worldDirectionTransform * lcm.getLocalDirection(li);
        double3 const shadowLocalDirection = params.options.transform * localDirection;

        // using mat3::getTransformForNormals handles non-uniform scaling
        // note: in the common case of the rigid-body transform, getTransformForNormals() returns
        // identity.
        mat3 const worlTransformNormals = mat3::getTransformForNormals(worldTransform.upperLeft());
        double3 const d = worlTransformNormals * localDirection;
        double3 const s = worlTransformNormals * shadowLocalDirection;

        // We compute the reference point for snapping shadowmaps without applying the
        // rotation of `worldOriginTransform` on both sides, so that we don't have any instability
        // due to the limited precision of the "light space" matrix (even at double precision).

        // getMv() Returns the world-to-lightspace transformation. See ShadowMap.cpp.
        auto getMv = [](double3 direction) -> mat3 {
            // We use the x-axis as the "up" reference so that the math is stable when the light
            // is pointing down, which is a common case for lights. See ShadowMap.cpp.
            return transpose(mat3::lookTo(direction, double3{ 1, 0, 0 }));
        };
        double3 const worldOrigin = transpose(worldTransform.upperLeft()) * worldTransform[3].xyz;
        mat3 const Mv = getMv(shadowLocalDirection);
        double2 const lsReferencePoint = (Mv * worldOrigin).xy;

        constexpr float inf = std::numeric_limits<float>::infinity();
        lightData.elementAt<POSITION_RADIUS>(0) = float4{ 0, 0, 0, inf };
        lightData.elementAt<DIRECTION>(0) = normalize(d);
        lightData.elementAt<SHADOW_DIRECTION>(0) = normalize(s);
        lightData.elementAt<SHADOW_REF>(0) = lsReferencePoint;
        lightData.elementAt<LIGHT_INSTANCE>(0) = li;
    } else {
        lightData.elementAt<LIGHT_INSTANCE>(0) = 0;
    }

    // some elements past the end of the array will be accessed by SIMD code, we need to make
    // sure the data is valid enough as not to produce errors such as divide-by-zero
    // (e.g. in computeLightRanges())
    for (size_t i = lightData.size(), e = lightData.capacity(); i < e; i++) {
        new(lightData.data<POSITION_RADIUS>() + i) float4{ 0, 0, 0, 1 };
    }

    // Purely for the benefit of MSAN, we can avoid uninitialized reads by zeroing out the
    // unused scene elements between the end of the array and the rounded-up count.
    if constexpr (UTILS_HAS_SANITIZE_MEMORY) {
        for (size_t i = sceneData.size(), e = sceneData.capacity(); i < e; i++) {
            sceneData.data<LAYERS>()[i] = 0;
            sceneData.data<VISIBLE_MASK>()[i] = 0;
            sceneData.data<VISIBILITY_STATE>()[i] = {};
        }
    }

    js.runAndWait(rootJob);

    FILAMENT_TRACING_NAME_END(FILAMENT_TRACING_CATEGORY_FILAMENT);
}

void FScene::prepareVisibleRenderables(Range<uint32_t> visibleRenderables) noexcept {
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    RenderableSoa& sceneData = mRenderableData;
    FRenderableManager const& rcm = mEngine.getRenderableManager();

    mHasContactShadows = false;
    for (uint32_t const i : visibleRenderables) {
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
                visibility.skinning,
                visibility.morphing,
                visibility.screenSpaceContactShadows,
                sceneData.elementAt<INSTANCES>(i).buffer != nullptr,
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
    FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT);
    FEngine::DriverApi& driver = mEngine.getDriverApi();

    // don't allocate more than 16 KiB directly into the render stream
    static constexpr size_t MAX_STREAM_ALLOCATION_COUNT = 64;   // 16 KiB
    const size_t count = visibleRenderables.size();
    PerRenderableData* buffer = [&]{
        if (count >= MAX_STREAM_ALLOCATION_COUNT) {
            // use the heap allocator
            auto& bufferPoolAllocator = mSharedState->mBufferPoolAllocator;
            return static_cast<PerRenderableData*>(bufferPoolAllocator.get(count * sizeof(PerRenderableData)));
        } else {
            // allocate space into the command stream directly
            return driver.allocatePod<PerRenderableData>(count);
        }
    }();

    PerRenderableData const* const uboData = mRenderableData.data<UBO>();
    mat4f const* const worldTransformData = mRenderableData.data<WORLD_TRANSFORM>();

    // prepare each InstanceBuffer.
    FRenderableManager::InstancesInfo const* instancesData = mRenderableData.data<INSTANCES>();
    for (uint32_t const i : visibleRenderables) {
        auto& instancesInfo = instancesData[i];
        if (UTILS_UNLIKELY(instancesInfo.buffer)) {
            instancesInfo.buffer->prepare(
                    mEngine, worldTransformData[i], uboData[i]);
        }
    }

    // copy our data into the UBO for each visible renderable
    for (uint32_t const i : visibleRenderables) {
        buffer[i] = uboData[i];
    }

    // We capture state shared between Scene and the update buffer callback, because the Scene could
    // be destroyed before the callback executes.
    std::weak_ptr<SharedState>* const weakShared =
            new (std::nothrow) std::weak_ptr<SharedState>(mSharedState);

    // update the UBO
    driver.resetBufferObject(renderableUbh);
    driver.updateBufferObjectUnsynchronized(renderableUbh, {
            buffer, count * sizeof(PerRenderableData),
            +[](void* p, size_t const s, void* user) {
                std::weak_ptr<SharedState>* const weakShared =
                        static_cast<std::weak_ptr<SharedState>*>(user);
                if (s >= MAX_STREAM_ALLOCATION_COUNT * sizeof(PerRenderableData)) {
                    if (auto state = weakShared->lock()) {
                        state->mBufferPoolAllocator.put(p);
                    }
                }
                delete weakShared;
            }, weakShared
    }, 0);
}

void FScene::terminate(FEngine&) {
}

void FScene::prepareDynamicLights(const CameraInfo& camera,
        Handle<HwBufferObject> lightUbh) noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    FLightManager const& lcm = mEngine.getLightManager();
    LightSoa& lightData = getLightData();

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
    auto const* UTILS_RESTRICT instances        = lightData.data<LIGHT_INSTANCE>();
    auto const* UTILS_RESTRICT shadowInfo       = lightData.data<SHADOW_INFO>();
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
                shadowInfo[i].index);
        lp[gpuIndex].channels             = LightsUib::packChannels(
                lcm.getLightChannels(li),
                shadowInfo[i].castsShadows);
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
void FScene::addEntity(Entity const entity) {
    mEntities.insert(entity);
}

UTILS_NOINLINE
void FScene::addEntities(const Entity* entities, size_t const count) {
    mEntities.insert(entities, entities + count);
}

UTILS_NOINLINE
void FScene::remove(Entity const entity) {
    mEntities.erase(entity);
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
    size_t count = 0;
    auto const& entities = mEntities;
    for (Entity const e : entities) {
        count += em.isAlive(e) && rcm.getInstance(e) ? 1 : 0;
    }
    return count;
}

UTILS_NOINLINE
size_t FScene::getLightCount() const noexcept {
    FEngine& engine = mEngine;
    EntityManager const& em = engine.getEntityManager();
    FLightManager const& lcm = engine.getLightManager();
    size_t count = 0;
    auto const& entities = mEntities;
    for (Entity const e : entities) {
        count += em.isAlive(e) && lcm.getInstance(e) ? 1 : 0;
    }
    return count;
}

UTILS_NOINLINE
bool FScene::hasEntity(Entity const entity) const noexcept {
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
