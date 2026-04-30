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

#ifndef TNT_FILAMENT_DETAILS_SCENE_H
#define TNT_FILAMENT_DETAILS_SCENE_H

#include <filament/Scene.h>

#include "downcast.h"
#include "Allocators.h"
#include "Culler.h"

#include "components/LightManager.h"
#include "components/RenderableManager.h"

#include "details/InstanceBuffer.h"

#include <backend/Handle.h>

#include <utils/Entity.h>
#include <utils/PagedArenaBitset.h>
#include <utils/EntityInstance.h>
#include <utils/Invocable.h>
#include <utils/JobSystem.h>
#include <utils/Mutex.h>
#include <utils/Slice.h>
#include <utils/StructureOfArrays.h>
#include <utils/Range.h>

#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include <stddef.h>

namespace filament {

class FEngine;
class FIndirectLight;
class FRenderer;
class FSkybox;
class FView;
class RenderableManager;
struct CameraInfo;
struct SceneCacheData;

using VisibleMaskType = Culler::result_type;

struct ShadowInfo {
    bool castsShadows = false;
    bool contactShadows = false;
    uint8_t index = 0;
};

using RenderableSoa = utils::StructureOfArrays<
        utils::Entity,                              // RENDERABLE_ENTITY
        math::mat4f,                                // WORLD_TRANSFORM
        FRenderableManager::Visibility,             // VISIBILITY_STATE
        FRenderableManager::Skinning,               // SKINNING_STATE
        FRenderableManager::SkinningBindingInfo,    // SKINNING_BUFFER
        FRenderableManager::MorphingBindingInfo,    // MORPHING_BUFFER
        FRenderableManager::InstancesInfo,          // INSTANCES
        math::float3,                               // WORLD_AABB_CENTER
        VisibleMaskType,                            // VISIBLE_MASK
        uint8_t,                                    // CHANNELS
        uint8_t,                                    // LAYERS
        math::float3,                               // WORLD_AABB_EXTENT
        utils::Slice<const FRenderPrimitive>,       // PRIMITIVES
        uint32_t,                                   // SUMMED_PRIMITIVE_COUNT
        PerRenderableData,                          // UBO
        backend::DescriptorSetHandle,               // DESCRIPTOR_SET_HANDLE
        float                                       // USER_DATA
>;

using LightSoa = utils::StructureOfArrays<
        utils::Entity,                              // LIGHT_ENTITY
        math::float4,                               // POSITION_RADIUS
        math::float3,                               // DIRECTION
        math::float3,                               // SHADOW_DIRECTION
        math::double2,                              // SHADOW_REF
        math::float2,                               // SPOT_PARAMS
        Culler::result_type,                        // VISIBILITY
        math::float2,                               // SCREEN_SPACE_Z_RANGE
        ShadowInfo                                  // SHADOW_INFO
>;

template<typename SoAType, size_t EntityColumn>
class SoaCache {
    SoAType mSoA;
    tsl::robin_map<uint32_t, uint32_t> mIndexMap;

public:
    size_t size() const noexcept { return mSoA.size(); }
    SoAType& getSoA() { return mSoA; }
    const SoAType& getSoA() const { return mSoA; }
    void update(utils::Entity const e, typename SoAType::Structure const& tuple) {
        auto const it = mIndexMap.find(e.getId());
        if (it != mIndexMap.end()) {
            uint32_t const index = it->second;
            mSoA[index] = tuple;
        } else {
            uint32_t const index = mSoA.size();
            typename SoAType::Structure copy = tuple;
            mSoA.push_back(std::move(copy));
            mIndexMap[e.getId()] = index;
        }
    }
    void remove(utils::Entity const e) {
        auto const it = mIndexMap.find(e.getId());
        if (it != mIndexMap.end()) {
            uint32_t const index = it->second;
            size_t const lastIndex = mSoA.size() - 1;
            if (index != lastIndex) {
                utils::Entity const lastEntity = mSoA.template elementAt<EntityColumn>(lastIndex);
                mSoA.swap(index, lastIndex);
                mIndexMap[lastEntity.getId()] = index;
            }
            mSoA.pop_back();
            mIndexMap.erase(it);
        }
    }
    uint32_t getIndex(utils::Entity const e) const {
        auto const it = mIndexMap.find(e.getId());
        if (it != mIndexMap.end()) {
            return it->second;
        }
        return 0xFFFFFFFF;
    }
};

struct SceneCacheData {
    using EntitySet = utils::PagedArenaBitset;

    // Scene Cache. This can be reordered/modified outside of FScene
    RenderableSoa renderableData;
    LightSoa lightData;
    bool renderablesHaveContactShadows = false;
    bool lightsHaveContactShadows = false;

    // Internal Master Cache, cannot be modified outside FScene
    SoaCache<RenderableSoa, 0> renderableCache;
    SoaCache<LightSoa, 0> lightCache;
    EntitySet directionalLights;
    uint32_t directionalLightIndex = 0;

    // entities for which a component has changed (could have been added/removed/modified)
    EntitySet dirtyEntities;

    // entities that have been removed from the scene
    EntitySet removedEntities;

    // entities that have died
    mutable utils::Mutex destroyedEntitiesLock;
    EntitySet destroyedEntities;

    math::mat4 previousWorldTransform;
    bool previousShadowReceiversAreCasters = false;

    void invalidate() noexcept;
    EntitySet& getAndClearDestroyedEntities(EntitySet& out) noexcept;
    bool hasDestroyedEntities() const noexcept;
    bool hasDirtyEntities() const noexcept;
    void resize(size_t renderableCount, size_t lightCount);
};

class FScene : public Scene {
public:
    using RenderableSoa = filament::RenderableSoa;
    using LightSoa = filament::LightSoa;
    using VisibleMaskType = filament::VisibleMaskType;
    using ShadowInfo = filament::ShadowInfo;
    using SceneCacheData = filament::SceneCacheData;

    enum {
        RENDERABLE_ENTITY,      //   4 | Entity of the Renderable component
        WORLD_TRANSFORM,        //  16 | instance of the Transform component
        VISIBILITY_STATE,       //   2 | visibility data of the component
        SKINNING_STATE,         //   1 | skinning data of the component
        SKINNING_BUFFER,        //   8 | bones uniform buffer handle, offset, indices and weights
        MORPHING_BUFFER,        //  16 | weights uniform buffer handle, count, morph targets
        INSTANCES,              //  16 | instancing info for this Renderable
        WORLD_AABB_CENTER,      //  12 | world-space bounding box center of the renderable
        VISIBLE_MASK,           //   2 | each bit represents a visibility in a pass
        CHANNELS,               //   1 | currently light channels only

        // These are not needed anymore after culling
        LAYERS,                 //   1 | layers
        WORLD_AABB_EXTENT,      //  12 | world-space bounding box half-extent of the renderable

        // These are temporaries and could be stored out of line
        PRIMITIVES,             //   8 | level-of-detail'ed primitives
        SUMMED_PRIMITIVE_COUNT, //   4 | summed visible primitive counts
        UBO,                    // 128 |
        DESCRIPTOR_SET_HANDLE,

        // FIXME: We need a better way to handle this
        USER_DATA,              //   4 | user data currently used to store the scale
    };

    enum {
        LIGHT_ENTITY,
        POSITION_RADIUS,
        DIRECTION,
        SHADOW_DIRECTION,
        SHADOW_REF,
        SPOT_PARAMS,
        VISIBILITY,
        SCREEN_SPACE_Z_RANGE,
        SHADOW_INFO
    };

    /*
     * Filament-scope Public API
     */

    FSkybox* getSkybox() const noexcept { return mSkybox; }

    FIndirectLight* getIndirectLight() const noexcept { return mIndirectLight; }

    // the directional light is always stored first in the LightSoA, so we need to account
    // for that in a few places.
    static constexpr size_t DIRECTIONAL_LIGHTS_COUNT = 1;

    explicit FScene(FEngine& engine);
    ~FScene() noexcept;

    void terminate(FEngine& engine);

    SceneCacheData* registerView(FView const* view);
    void unregisterView(FView const* view);

    bool processDeferredUpdates(math::mat4 const& worldTransform, bool shadowReceiversAreCasters, SceneCacheData& cache) noexcept;

    void prepare(utils::JobSystem& js, RootArenaScope& rootArenaScope,
            math::mat4 const& worldTransform, bool shadowReceiversAreCasters,
            SceneCacheData& cache) noexcept;

    void prepareVisibleRenderables(utils::Range<uint32_t> visibleRenderables,
            SceneCacheData& cache) const noexcept;

    void prepareDynamicLights(const CameraInfo& camera,
            backend::Handle<backend::HwBufferObject> lightUbh,
            LightSoa& lightData) const noexcept;

    bool hasContactShadows(SceneCacheData const& cache) const noexcept;

    static uint32_t getPrimitiveCount(RenderableSoa const& soa,
        uint32_t const first, uint32_t const last) noexcept {
        // the caller must guarantee that last is dereferenceable
        return soa.elementAt<SUMMED_PRIMITIVE_COUNT>(last) -
                soa.elementAt<SUMMED_PRIMITIVE_COUNT>(first);
    }

    static uint32_t getPrimitiveCount(RenderableSoa const& soa, uint32_t const last) noexcept {
        // the caller must guarantee that last is dereferenceable
        return soa.elementAt<SUMMED_PRIMITIVE_COUNT>(last);
    }

private:
    using EntitySet = SceneCacheData::EntitySet;

    friend class Scene;
    void setSkybox(FSkybox* skybox) noexcept;
    void setIndirectLight(FIndirectLight* ibl) noexcept { mIndirectLight = ibl; }
    void addEntity(utils::Entity entity);
    void addEntities(const utils::Entity* entities, size_t count);
    void remove(utils::Entity entity);
    void removeEntities(const utils::Entity* entities, size_t count);
    void removeAllEntities() noexcept;
    size_t getEntityCount() const noexcept { return mEntities.size(); }
    size_t getRenderableCount() const noexcept;
    size_t getLightCount() const noexcept;
    bool hasEntity(utils::Entity entity) const noexcept;
    void forEach(utils::Invocable<void(utils::Entity)>&& functor) const noexcept;
    void flushNotifications() const noexcept;

    static inline void computeLightRanges(math::float2* zrange,
            CameraInfo const& camera, const math::float4* spheres, size_t count) noexcept;

    FEngine& mEngine;
    FSkybox* mSkybox = nullptr;
    FIndirectLight* mIndirectLight = nullptr;
    std::unordered_map<FView const*, std::unique_ptr<SceneCacheData>> mViewCaches;

    // Master list of Entities in the scene
    EntitySet mEntities;

    // temporary storage, used to avoid allocations in steady state
    std::vector<uint32_t> mTempExtractedRenderables;
    std::vector<uint32_t> mTempExtractedLights;
    EntitySet mTempDestroyedEntities;
    EntitySet mTempToRemoveRenderables;
    EntitySet mTempToLights;
};

FILAMENT_DOWNCAST(Scene)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SCENE_H
