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

#include "Allocators.h"
#include "Culler.h"
#include "downcast.h"

#include "components/LightManager.h"
#include "components/RenderableManager.h"

#include <private/filament/EngineEnums.h>

#include <filament/Scene.h>
#include <filament/TransformManager.h>

#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/Invocable.h>
#include <utils/Mutex.h>
#include <utils/PagedArenaBitset.h>
#include <utils/Range.h>
#include <utils/Slice.h>
#include <utils/StructureOfArrays.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <array>
#include <memory>
#include <utility>
#include <vector>

#include <stddef.h>

namespace utils {
class JobSystem;
} // namespace utils

namespace filament {

class FEngine;
class FIndirectLight;
class FRenderer;
class FSkybox;
class FView;
class RenderableManager;
struct CameraInfo;

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
        float,                                      // WORLD_AABB_CENTER_X
        float,                                      // WORLD_AABB_CENTER_Y
        float,                                      // WORLD_AABB_CENTER_Z
        float,                                      // WORLD_AABB_EXTENT_X
        float,                                      // WORLD_AABB_EXTENT_Y
        float,                                      // WORLD_AABB_EXTENT_Z
        utils::Slice<const FRenderPrimitive>,       // PRIMITIVES
        uint32_t,                                   // SUMMED_PRIMITIVE_COUNT
        PerRenderableData,                          // UBO
        backend::DescriptorSetHandle,               // DESCRIPTOR_SET_HANDLE
        float                                       // USER_DATA
>;

using LightSoa = utils::StructureOfArrays<
        utils::Entity,                              // LIGHT_ENTITY
        math::float4,                               // POSITION_RADIUS
        float,                                      // POSITION_X
        float,                                      // POSITION_Y
        float,                                      // POSITION_Z
        float,                                      // RADIUS
        math::float3,                               // DIRECTION
        math::float3,                               // SHADOW_DIRECTION
        math::double2,                              // SHADOW_REF
        math::float2,                               // SPOT_PARAMS
        Culler::result_type,                        // VISIBILITY
        math::float2,                               // SCREEN_SPACE_Z_RANGE
        ShadowInfo                                  // SHADOW_INFO
>;

/*
 * SoaCache is a persistent (cross-frame) SoA keyed by Entity, with swap-and-pop removal.
 *
 * Invariant: only EntityColumn and CachedColumns are maintained by this cache. On removal,
 * only those columns (and the translation) are migrated from the last row into the vacated
 * slot; on acquire() of a new row, all other columns are default-constructed (i.e. left
 * uninitialized for trivial types). Therefore every column that a producer writes into this
 * cache, and every column that a consumer reads back from it, MUST be listed in CachedColumns.
 * Any other column holds stale or indeterminate data and must be treated as write-only
 * scratch at best.
 */
template<typename SoAType, size_t EntityColumn, size_t... CachedColumns>
class SoaCache {
    SoAType mSoA;
    std::vector<math::double3> mTranslations;
    tsl::robin_map<utils::Entity, uint32_t, utils::Entity::Hasher> mIndexMap;

public:
    size_t size() const noexcept { return mSoA.size(); }
    SoAType& getSoA() noexcept { return mSoA; }
    const SoAType& getSoA() const noexcept { return mSoA; }
    utils::Slice<const math::double3> getTranslations() const noexcept {
        return { mTranslations.data(), mTranslations.size() };
    }
    UTILS_NOINLINE
    uint32_t acquire(utils::Entity const e, math::double3 const& translation) {
        auto const it = mIndexMap.find(e);
        if (UTILS_LIKELY(it != mIndexMap.end())) {
            uint32_t const index = it->second;
            mTranslations[index] = translation;
            return index;
        }
        uint32_t const index = static_cast<uint32_t>(mSoA.size());
        mSoA.push_back();
        mTranslations.push_back(translation);
        mIndexMap[e] = index;
        return index;
    }
    UTILS_NOINLINE
    void remove(utils::Entity const e) {
        if (mSoA.size() == 0) {
            return;
        }
        auto const it = mIndexMap.find(e);
        if (it != mIndexMap.end()) {
            uint32_t const index = it->second;
            size_t const lastIndex = mSoA.size() - 1;
            if (index != lastIndex) {
                utils::Entity const lastEntity = mSoA.template elementAt<EntityColumn>(lastIndex);
                mSoA.template elementAt<EntityColumn>(index) = lastEntity;
                ((mSoA.template elementAt<CachedColumns>(index) =
                        mSoA.template elementAt<CachedColumns>(lastIndex)), ...);
                mTranslations[index] = mTranslations[lastIndex];
                mIndexMap.find(lastEntity).value() = index;
            }
            mSoA.pop_back();
            mTranslations.pop_back();
            mIndexMap.erase(it);
        }
    }
    uint32_t getIndex(utils::Entity const e) const {
        auto const it = mIndexMap.find(e);
        if (it != mIndexMap.end()) {
            return it->second;
        }
        return 0xFFFFFFFF;
    }
};

class FScene : public Scene {
public:
    using RenderableSoa = filament::RenderableSoa;
    using LightSoa = filament::LightSoa;
    using VisibleMaskType = filament::VisibleMaskType;
    using ShadowInfo = filament::ShadowInfo;

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

        // Planar bounding box coordinates for high-throughput vectorized culling
        WORLD_AABB_CENTER_X,    //   4 | center.x
        WORLD_AABB_CENTER_Y,    //   4 | center.y
        WORLD_AABB_CENTER_Z,    //   4 | center.z
        WORLD_AABB_EXTENT_X,    //   4 | halfExtent.x
        WORLD_AABB_EXTENT_Y,    //   4 | halfExtent.y
        WORLD_AABB_EXTENT_Z,    //   4 | halfExtent.z

        // These are temporaries and could be stored out of line
        PRIMITIVES,             //   8 | level-of-detail'ed primitives
        SUMMED_PRIMITIVE_COUNT, //   4 | summed visible primitive counts
        UBO,                    // 256 |
        DESCRIPTOR_SET_HANDLE,

        // FIXME: We need a better way to handle this
        USER_DATA,              //   4 | user data currently used to store the scale
    };

    enum {
        LIGHT_ENTITY,
        POSITION_RADIUS,
        POSITION_X,
        POSITION_Y,
        POSITION_Z,
        RADIUS,
        DIRECTION,
        SHADOW_DIRECTION,
        SHADOW_REF,
        SPOT_PARAMS,
        VISIBILITY,
        SCREEN_SPACE_Z_RANGE,
        SHADOW_INFO
    };

    struct SceneCacheData {
        using EntitySet = utils::PagedArenaBitset;
        using RenderableCache = SoaCache<RenderableSoa, RENDERABLE_ENTITY,
                WORLD_TRANSFORM,
                VISIBILITY_STATE,
                SKINNING_STATE,
                SKINNING_BUFFER,
                MORPHING_BUFFER,
                INSTANCES,
                WORLD_AABB_CENTER,
                CHANNELS,
                LAYERS,
                WORLD_AABB_EXTENT,
                WORLD_AABB_EXTENT_X,
                WORLD_AABB_EXTENT_Y,
                WORLD_AABB_EXTENT_Z,
                PRIMITIVES,
                USER_DATA>;
        using LightCache = SoaCache<LightSoa, LIGHT_ENTITY,
                RADIUS,
                DIRECTION,
                SPOT_PARAMS>;

        SceneCacheData() noexcept;
        ~SceneCacheData() noexcept;

        // Scene Cache. This can be reordered/modified outside of FScene
        RenderableSoa renderableData;
        LightSoa lightData;

        // Directional lights in addition to the dominant one (which is stored at index 0 of
        // lightData). These are evaluated without shadows, so we only need their direction
        // and LightManager instance.
        std::array<math::float3, CONFIG_MAX_EXTRA_DIRECTIONAL_LIGHTS>
                extraDirectionalLightDirections{};
        std::array<FLightManager::Instance, CONFIG_MAX_EXTRA_DIRECTIONAL_LIGHTS>
                extraDirectionalLightInstances{};
        uint8_t extraDirectionalLightCount = 0;

        bool renderablesHaveContactShadows = false;
        bool lightsHaveContactShadows = false;

        // Internal Master Cache, cannot be modified outside FScene.
        //
        // Invariant: SceneCacheData is per-View, whereas FScene::mEntities is shared across all
        // Views attached to the Scene. Prior to processDeferredUpdates() running for a given View,
        // its master cache is NOT guaranteed to be a subset of mEntities (it may still hold rows
        // for entities already removed from mEntities via remove() or flushPendingDestroyed()).
        // Only AFTER processDeferredUpdates() completes for that View is its cache
        // guaranteed to be a subset of (mEntities & alive). Consequently, per-View cache eviction
        // must depend solely on this View's own removedEntities and destroyedEntities sets, and
        // must never be filtered by FScene::mEntities.
        RenderableCache renderableCache;
        LightCache lightCache;
        EntitySet directionalLights;
        utils::Entity dominantDirectionalLight;

        // entities for which a component has changed (could have been added/removed/modified)
        EntitySet dirtyEntities;

        // entities that have been removed from the scene
        EntitySet removedEntities;

        // entities that have died (main-thread only)
        EntitySet destroyedEntities;

        math::mat3 previousWorldRotation;
        bool previousShadowReceiversAreCasters = false;
        // Set on creation and on every registration; cleared at the end of
        // processDeferredUpdates().
        bool needsFullUpdate = true;

        bool hasDirtyEntities() const noexcept;
        void resize(size_t renderableCount, size_t lightCount);
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

    bool processDeferredUpdates(math::mat3 const& worldRotation, bool shadowReceiversAreCasters, SceneCacheData& cache) noexcept;

    void prepare(utils::JobSystem& js, LinearAllocatorArena& arena,
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
    void addEntities(utils::Slice<const utils::Entity> entities);
    void remove(utils::Entity entity);
    void removeEntities(utils::Slice<const utils::Entity> entities);
    void removeAllEntities() noexcept;
    size_t getEntityCount() const noexcept { return mEntities.size(); }
    size_t getRenderableCount() const noexcept;
    size_t getLightCount() const noexcept;
    bool hasEntity(utils::Entity entity) const noexcept;
    void forEach(utils::Invocable<void(utils::Entity)>&& functor) const noexcept;
    void flushNotifications() const noexcept;
    void flushPendingDestroyed() noexcept;

    static inline void computeLightRanges(math::float2* zrange,
            CameraInfo const& camera, const math::float4* spheres, size_t count) noexcept;

    struct PendingDestroyed {
        utils::Mutex lock;
        EntitySet entities UTILS_GUARDED_BY(lock);
    };

    FEngine& mEngine;
    FSkybox* mSkybox = nullptr;
    FIndirectLight* mIndirectLight = nullptr;
    std::shared_ptr<PendingDestroyed> mPendingDestroyed =
            std::make_shared<PendingDestroyed>();
    friend class FView;
    void registerView(FView* view);
    void unregisterView(FView* view);
    std::vector<FView*> mRegisteredViews;

    // Master list of Entities in the scene
    EntitySet mEntities;

    // temporary storage, used to avoid allocations in steady state
    std::vector<uint32_t> mTempExtractedRenderables;
    std::vector<uint32_t> mTempExtractedLights;
    std::vector<std::pair<FLightManager::Instance, TransformManager::Instance>>
            mTempDirectionalInstances;
    EntitySet mTempPendingDestroyed;
    EntitySet mTempAliveEntities;
    EntitySet mTempActiveDirty;
    EntitySet mTempRenderableEntities;
    EntitySet mTempLightEntities;
    EntitySet mTempToRemoveRenderables;
    EntitySet mTempToRemoveLights;
};

using SceneCacheData = FScene::SceneCacheData;

FILAMENT_DOWNCAST(Scene)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SCENE_H
