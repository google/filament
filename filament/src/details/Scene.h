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
#include <utils/EntityInstanceMap.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Invocable.h>
#include <utils/Mutex.h>
#include <utils/PagedArenaBitset.h>
#include <utils/Range.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Slice.h>
#include <utils/StructureOfArrays.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <tsl/robin_map.h>

#include <array>
#include <type_traits>
#include <utility>
#include <vector>

#include <stddef.h>
#include <stdint.h>

namespace utils {
class JobSystem;
} // namespace utils

namespace filament {

class FEngine;
class FIndirectLight;
class FInstanceBuffer;
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

struct PrevTransform {
    // user world transform, in its exact stored representation (see getWorldTranslationLo())
    math::mat4f world;
    math::float3 lo;

    // Bitwise equality of the stored representation, i.e. "did the value written change?".
    // Unlike a float comparison this tells -0 from +0 and treats NaN as equal to itself, so a
    // no-op write or an A->B->A sequence isn't a motion.
    bool bitwiseEquals(PrevTransform const& other) const noexcept;
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
 *
 * Extra, when not void, is per-row side data that follows the row across swap-and-pop removals,
 * like the translation. It lives outside the SoA, so it is never copied into the per-frame
 * renderableData nor moved by its partitioning; see extraAt().
 *
 * The Entity -> row map is keyed by the 17-bit entity index and validates the generation on
 * lookup, so at most one generation of a given index can be cached at a time: a stale generation
 * must be remove()d before its successor is acquire()d (insertNew() asserts this).
 */
template<typename SoAType, typename Extra, size_t EntityColumn, size_t... CachedColumns>
class SoaCache {
    struct NoExtra {};
    static constexpr bool kHasExtra = !std::is_void_v<Extra>;
    using ExtraType = std::conditional_t<kHasExtra, Extra, NoExtra>;
    using ExtraStorage = std::conditional_t<kHasExtra, std::vector<ExtraType>, NoExtra>;

    SoAType mSoA;
    std::vector<math::double3> mTranslations;
    ExtraStorage mExtras;   // per-row side data, only when kHasExtra (see Extra)
    utils::EntityInstanceMap<> mIndexMap;

public:
    // Number of cached entities (and rows in the SoA).
    size_t size() const noexcept { return mSoA.size(); }

    // Underlying SoA storage. Only EntityColumn and CachedColumns are maintained across
    // removals and insertions (see class comment).
    SoAType& getSoA() noexcept { return mSoA; }
    const SoAType& getSoA() const noexcept { return mSoA; }

    // Double-precision world translations, indexed identically to the SoA rows.
    utils::Slice<const math::double3> getTranslations() const noexcept {
        return { mTranslations.data(), mTranslations.size() };
    }

    // Per-row side data, indexed like the SoA. acquire() leaves it value-initialized for a new
    // row; the caller is expected to set it.
    ExtraType& extraAt(uint32_t const index) noexcept {
        static_assert(kHasExtra, "this SoaCache has no Extra");
        return mExtras[index];
    }
    ExtraType const& extraAt(uint32_t const index) const noexcept {
        static_assert(kHasExtra, "this SoaCache has no Extra");
        return mExtras[index];
    }

    // Looks up `e` or inserts a new row for it, updating its cached `translation` in both cases.
    // Returns `{ index, inserted }`, where `index` is the row index in the SoA and `inserted` is
    // true iff a new row was appended (in which case EntityColumn is set to `e`, the other SoA
    // columns are default-constructed, and extraAt(index) is value-initialized for the caller
    // to populate).
    std::pair<uint32_t, bool> acquire(utils::Entity const e, math::double3 const& translation) {
        if (auto const existing = mIndexMap.getInstance(e); UTILS_LIKELY(existing != 0)) {
            uint32_t const index = existing - 1u;
            mTranslations[index] = translation;
            return { index, false };
        }
        return insertNew(e, translation);
    }

    // Evicts `e` from the cache via swap-and-pop, migrating the last row's EntityColumn,
    // CachedColumns, translation, and Extra into the vacated slot. No-op if `e` is not cached.
    // Invalidates the row index previously held by the last row.
    UTILS_NOINLINE
    void remove(utils::Entity const e) {
        if (mSoA.size() == 0) {
            return;
        }
        if (uint32_t* const slot = mIndexMap.findSlot(e); slot != nullptr) {
            uint32_t const index = utils::EntityInstanceMap<>::getSlotInstance(slot) - 1u;
            size_t const lastIndex = mSoA.size() - 1;
            if (index != lastIndex) {
                utils::Entity const lastEntity = mSoA.template elementAt<EntityColumn>(lastIndex);
                mSoA.template elementAt<EntityColumn>(index) = lastEntity;
                ((mSoA.template elementAt<CachedColumns>(index) =
                        mSoA.template elementAt<CachedColumns>(lastIndex)), ...);
                mTranslations[index] = mTranslations[lastIndex];
                if constexpr (kHasExtra) {
                    mExtras[index] = std::move(mExtras[lastIndex]);
                }
                mIndexMap.update(lastEntity, index + 1u);
            }
            mSoA.pop_back();
            mTranslations.pop_back();
            if constexpr (kHasExtra) {
                mExtras.pop_back();
            }
            utils::EntityInstanceMap<>::clearSlot(slot);
        }
    }

    // Returns the current row index of `e` in the SoA, or 0xFFFFFFFF if `e` is not cached.
    // Note: row indices may change across remove() calls due to swap-and-pop.
    uint32_t getIndex(utils::Entity const e) const noexcept {
        return static_cast<uint32_t>(mIndexMap.getInstance(e)) - 1u;
    }

    // Clears all cached rows.
    void clear() noexcept {
        mSoA.clear();
        mTranslations.clear();
        if constexpr (kHasExtra) {
            mExtras.clear();
        }
        mIndexMap.clear();
    }

private:
    UTILS_NOINLINE
    std::pair<uint32_t, bool> insertNew(utils::Entity e, math::double3 const& translation);
};

class FScene : public Scene {
public:
    using RenderableSoa = filament::RenderableSoa;
    using LightSoa = filament::LightSoa;
    using VisibleMaskType = filament::VisibleMaskType;
    using ShadowInfo = filament::ShadowInfo;
    using PrevTransform = filament::PrevTransform;

    enum {
        RENDERABLE_ENTITY,      //   4 | Entity of the Renderable component
        WORLD_TRANSFORM,        //  64 | instance of the Transform component
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
#if defined(FILAMENT_SUPPORTS_MOTION_VECTORS)
        // The previous world transforms (motion vectors) ride along as the rows' Extra.
        using RenderableCacheExtra = PrevTransform;
#else
        using RenderableCacheExtra = void;
#endif
        using RenderableCache = SoaCache<RenderableSoa, RenderableCacheExtra, RENDERABLE_ENTITY,
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
        using LightCache = SoaCache<LightSoa, void, LIGHT_ENTITY,
                RADIUS,
                DIRECTION,
                SPOT_PARAMS>;

        struct InstanceBufferHistory {
            uint64_t revision = 0;      // FInstanceBuffer revision of `locals`
            uint32_t lastSeen = 0;      // last renderIndex at which a scene renderable used it
            // set by trackInstanceBuffer() when `locals` must be snapshot at commit, null otherwise
            FInstanceBuffer const* pendingSnapshot = nullptr;
            // local transforms at the previous render, empty until the first snapshot
            utils::FixedCapacityVector<math::mat4f> locals;
        };

        // Motion vectors state for this (View, Scene) pair, main thread only.
        struct MotionVectorsState {
            // key: FInstanceBuffer uid, which is never reused. There is one lookup per scene
            // renderable that has an InstanceBuffer, at every render (see trackInstanceBuffer()).
            tsl::robin_map<uint64_t, InstanceBufferHistory> instanceBuffers;
            // Renderable entities whose transform changed at this render, with their new transform.
            // Filled by processDeferredUpdates() while hasHistory is set, and drained by
            // commitMovedRenderables() from FView::prepareMotionVectors() within the same
            // FView::prepare() (hasHistory implies that motion vectors are active).
            std::vector<std::pair<utils::Entity, PrevTransform>> moved;
            uint32_t renderIndex = 0;            // View-local render counter
            bool hasHistory = false;             // set at commit while motion vectors are active
            bool hasMovingRenderables = false;   // set by prepare()

            // Marks `ib` as used at this render, and schedules its snapshot at commit if it's new
            // or its revision changed. Returns its local transforms at the previous render if its
            // revision changed since, null otherwise.
            math::mat4f const* trackInstanceBuffer(FInstanceBuffer const& ib) noexcept;

            void reset() noexcept;
        };

        SceneCacheData() noexcept;
        ~SceneCacheData() noexcept;

        // Advances the master cache's previous transforms to this render's, and clears the
        // motionMoved latch, for the entities in motionVectors.moved. Must run once those rows'
        // previous transforms have been used, i.e. after FView::prepareMotionVectors() read them.
        void commitMovedRenderables() noexcept;

        // Scene Cache. This can be reordered/modified outside of FScene
        RenderableSoa renderableData;
        LightSoa lightData;
        MotionVectorsState motionVectors;

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
        // Invariant: Whenever an entity is removed or evicted from FScene (via remove(),
        // removeAllEntities(), gc(), or pruneDestroyedEntities()), it is immediately evicted from
        // every registered FView's renderableCache, lightCache, and directionalLights before
        // FScene's EBR watermark advances, ensuring no stale Entity handle can linger past index
        // reclamation. When a FView attaches to an FScene (needsFullUpdate = true), any rows left
        // over from a previous Scene are pruned by full 32-bit Entity membership in mSceneManager.
        RenderableCache renderableCache;
        LightCache lightCache;
        EntitySet directionalLights;
        utils::Entity dominantDirectionalLight;

        // 17-bit indices of entities for which a component has changed (added/removed/modified)
        EntitySet dirtyEntities;

        math::mat3 previousWorldRotation;
        bool previousShadowReceiversAreCasters = false;
        bool hasEvictedEntities = false;
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

    void gc() noexcept;

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

    class SceneComponentManager : public utils::SingleInstanceComponentManager<> {
        using Base = utils::SingleInstanceComponentManager<>;
    public:
        explicit SceneComponentManager(utils::EntityManager& em) noexcept
                : Base(em, "SceneComponentManager") {}

        using Base::gc;
        using Base::catchupCurrentGarbage;

        void clearAll() noexcept {
            if (empty()) {
                return;
            }
            mInstanceMap.clear();
            mPendingDestruction.clear();
            {
                utils::LockGuard const lock(mEbrEntitiesLock);
                mEntities.clear();
            }
            mData.resize(1);
            suspend();
        }
    };

    friend class Scene;
    void setSkybox(FSkybox* skybox) noexcept;
    void setIndirectLight(FIndirectLight* ibl) noexcept { mIndirectLight = ibl; }
    void addEntity(utils::Entity entity);
    void addEntities(utils::Slice<const utils::Entity> entities);
    void remove(utils::Entity entity);
    void removeEntities(utils::Slice<const utils::Entity> entities);
    void removeAllEntities() noexcept;
    size_t getEntityCount() const noexcept { return mSceneManager.getComponentCount(); }
    size_t getRenderableCount() const noexcept;
    size_t getLightCount() const noexcept;
    bool hasEntity(utils::Entity entity) const noexcept;
    void forEach(utils::Invocable<void(utils::Entity)>&& functor) const noexcept;
    void pruneDestroyedEntities() noexcept;
    void destroyComponents(utils::Entity const* entities, size_t count) noexcept;

    static inline void computeLightRanges(math::float2* zrange,
            CameraInfo const& camera, const math::float4* spheres, size_t count) noexcept;

    FEngine& mEngine;
    FSkybox* mSkybox = nullptr;
    FIndirectLight* mIndirectLight = nullptr;
    SceneComponentManager mSceneManager;

    friend class FView;
    void registerView(FView* view);
    void unregisterView(FView* view);
    std::vector<FView*> mRegisteredViews;

    // temporary storage, used to avoid allocations in steady state
    std::vector<uint32_t> mTempExtractedRenderables;
    std::vector<uint32_t> mTempExtractedLights;
    std::vector<std::pair<FLightManager::Instance, TransformManager::Instance>>
            mTempDirectionalInstances;
    EntitySet mTempActiveDirty;
    EntitySet mTempRenderableEntities;
    EntitySet mTempLightEntities;
    EntitySet mTempToRemoveRenderables;
    EntitySet mTempToRemoveLights;
};

using SceneCacheData = FScene::SceneCacheData;

/*
 * SoaCache::insertNew() is defined in Scene.cpp and explicitly instantiated there for the two
 * caches below. Any new SoaCache specialization that calls acquire() needs an explicit
 * instantiation there as well, or it will fail to link.
 */
extern template std::pair<uint32_t, bool> SceneCacheData::RenderableCache::insertNew(
        utils::Entity, math::double3 const&);
extern template std::pair<uint32_t, bool> SceneCacheData::LightCache::insertNew(
        utils::Entity, math::double3 const&);

FILAMENT_DOWNCAST(Scene)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SCENE_H
