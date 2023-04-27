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
#include "details/InstanceBuffer.h"
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
        mEngine(engine), mSharedState(std::make_shared<SharedState>()) {
}

FScene::~FScene() noexcept = default;


void FScene::prepare(utils::JobSystem& js,
        LinearAllocatorArena& allocator,
        const mat4& worldOriginTransform,
        bool shadowReceiversAreCasters) noexcept {
    // TODO: can we skip this in most cases? Since we rely on indices staying the same,
    //       we could only skip, if nothing changed in the RCM.

    SYSTRACE_CALL();

    SYSTRACE_CONTEXT();

    // This will reset the allocator upon exiting
    ArenaScope const arena(allocator);

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
            utils::STLAllocator< RenderableContainerData, LinearAllocatorArena >, false>;

    using LightContainerData = std::pair<LightManager::Instance, TransformManager::Instance>;
    using LightInstanceContainer = FixedCapacityVector<LightContainerData,
            utils::STLAllocator< LightContainerData, LinearAllocatorArena >, false>;

    RenderableInstanceContainer renderableInstances{
            RenderableInstanceContainer::with_capacity(entities.size(), allocator) };

    LightInstanceContainer lightInstances{
            LightInstanceContainer::with_capacity(entities.size(), allocator) };

    SYSTRACE_NAME_BEGIN("InstanceLoop");

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

    SYSTRACE_NAME_END();

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
    // we need the capacity to be multiple of 16 for SIMD loops
    size_t lightDataCapacity = std::max<size_t>(DIRECTIONAL_LIGHTS_COUNT, entities.size());
    lightDataCapacity = (lightDataCapacity + 0xFu) & ~0xFu;

    /*
     * Now resize the SoAs if needed
     */

    // TODO: the resize below could happen in a job

    if (sceneData.size() != renderableInstances.size()) {
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

    auto renderableWork = [first = renderableInstances.data(), &rcm, &tcm, &worldOriginTransform,
                 &sceneData, shadowReceiversAreCasters](auto* p, auto c) {
        SYSTRACE_NAME("renderableWork");

        for (size_t i = 0; i < c; i++) {
            auto [ri, ti] = p[i];

            // this is where we go from double to float for our transforms
            const mat4f worldTransform{
                    worldOriginTransform * tcm.getWorldTransformAccurate(ti) };
            const bool reversedWindingOrder = det(worldTransform.upperLeft()) < 0;

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
            float const scale = (length(transform[0].xyz) + length(transform[1].xyz) +
                                 length(transform[2].xyz)) / 3.0f;

            size_t const index = std::distance(first, p) + i;
            assert_invariant(index < sceneData.size());

            sceneData.elementAt<RENDERABLE_INSTANCE>(index) = ri;
            sceneData.elementAt<WORLD_TRANSFORM>(index)     = worldTransform;
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

    auto lightWork = [first = lightInstances.data(), &lcm, &tcm, &worldOriginTransform,
            &lightData](auto* p, auto c) {
        SYSTRACE_NAME("lightWork");
        for (size_t i = 0; i < c; i++) {
            auto [li, ti] = p[i];
            // this is where we go from double to float for our transforms
            const mat4f worldTransform{ worldOriginTransform * tcm.getWorldTransformAccurate(ti) };
            const float4 position = worldTransform * float4{ lcm.getLocalPosition(li), 1 };
            float3 d = 0;
            if (!lcm.isPointLight(li) || lcm.isIESLight(li)) {
                d = lcm.getLocalDirection(li);
                // using mat3f::getTransformForNormals handles non-uniform scaling
                d = normalize(mat3f::getTransformForNormals(worldTransform.upperLeft()) * d);
            }
            size_t const index = DIRECTIONAL_LIGHTS_COUNT + std::distance(first, p) + i;
            assert_invariant(index < lightData.size());
            lightData.elementAt<POSITION_RADIUS>(index) = float4{ position.xyz, lcm.getRadius(li) };
            lightData.elementAt<DIRECTION>(index) = d;
            lightData.elementAt<LIGHT_INSTANCE>(index) = li;
        }
    };


    SYSTRACE_NAME_BEGIN("Renderable and Light jobs");

    JobSystem::Job* rootJob = js.createJob();

    auto* renderableJob = jobs::parallel_for(js, rootJob,
            renderableInstances.data(), renderableInstances.size(),
            std::cref(renderableWork), jobs::CountSplitter<128, 5>());

    auto* lightJob = jobs::parallel_for(js, rootJob,
            lightInstances.data(), lightInstances.size(),
            std::cref(lightWork), jobs::CountSplitter<32, 5>());

    js.run(renderableJob);
    js.run(lightJob);

    // Everything below can be done in parallel.

    /*
     * Handle the directional light separately
     */

    if (auto [li, ti] = directionalLightInstances ; li) {
        const mat4f worldTransform{
                worldOriginTransform * tcm.getWorldTransformAccurate(ti) };
        // using mat3f::getTransformForNormals handles non-uniform scaling
        float3 d = lcm.getLocalDirection(li);
        d = normalize(mat3f::getTransformForNormals(worldTransform.upperLeft()) * d);
        constexpr float inf = std::numeric_limits<float>::infinity();
        lightData.elementAt<POSITION_RADIUS>(0) = float4{ 0, 0, 0, inf };
        lightData.elementAt<DIRECTION>(0) = d;
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
    if (UTILS_HAS_SANITIZE_MEMORY) {
        for (size_t i = sceneData.size(), e = sceneData.capacity(); i < e; i++) {
            sceneData.data<LAYERS>()[i] = 0;
            sceneData.data<VISIBLE_MASK>()[i] = 0;
            sceneData.data<VISIBILITY_STATE>()[i] = {};
        }
    }

    js.runAndWait(rootJob);

    SYSTRACE_NAME_END();
}

void FScene::prepareVisibleRenderables(Range<uint32_t> visibleRenderables) noexcept {
    SYSTRACE_CALL();
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
    SYSTRACE_CALL();
    FEngine::DriverApi& driver = mEngine.getDriverApi();

    // store the UBO handle
    mRenderableViewUbh = renderableUbh;

    // don't allocate more than 16 KiB directly into the render stream
    static constexpr size_t MAX_STREAM_ALLOCATION_COUNT = 64;   // 16 KiB
    const size_t count = visibleRenderables.size();
    PerRenderableData* buffer = [&]{
        if (count >= MAX_STREAM_ALLOCATION_COUNT) {
            // use the heap allocator
            auto& bufferPoolAllocator = mSharedState->mBufferPoolAllocator;
            return (PerRenderableData*)bufferPoolAllocator.get(count * sizeof(PerRenderableData));
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
                    mEngine, worldTransformData[i], uboData[i], instancesInfo.handle);
        }
    }

    // copy our data into the UBO for each visible renderable
    for (uint32_t const i : visibleRenderables) {
        buffer[i] = uboData[i];
    }

    // We capture state shared between Scene and the update buffer callback, because the Scene could
    // be destroyed before the callback executes.
    std::weak_ptr<SharedState>* const weakShared = new std::weak_ptr<SharedState>(mSharedState);

    // update the UBO
    driver.resetBufferObject(renderableUbh);
    driver.updateBufferObjectUnsynchronized(renderableUbh, {
            buffer, count * sizeof(PerRenderableData),
            +[](void* p, size_t s, void* user) {
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

    // update skybox
    if (mSkybox) {
        mSkybox->commit(driver);
    }
}

void FScene::terminate(FEngine&) {
    // DO NOT destroy this UBO, it's owned by the View
    mRenderableViewUbh.clear();
}

void FScene::prepareDynamicLights(const CameraInfo& camera, ArenaScope&,
        Handle<HwBufferObject> lightUbh) noexcept {
    FEngine::DriverApi& driver = mEngine.getDriverApi();
    FLightManager const& lcm = mEngine.getLightManager();
    FScene::LightSoa& lightData = getLightData();

    /*
     * Here we copy our lights data into the GPU buffer.
     */

    size_t const size = lightData.size();
    // number of point-light/spotlights
    size_t const positionalLightCount = size - DIRECTIONAL_LIGHTS_COUNT;
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
