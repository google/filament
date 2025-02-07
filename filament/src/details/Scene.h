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

#include "downcast.h"

#include "Allocators.h"
#include "Culler.h"

#include "ds/DescriptorSet.h"

#include "components/LightManager.h"
#include "components/RenderableManager.h"
#include "components/TransformManager.h"

#include "BufferPoolAllocator.h"

#include <filament/Box.h>
#include <filament/Scene.h>

#include <math/mathfwd.h>

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/Slice.h>
#include <utils/StructureOfArrays.h>
#include <utils/Range.h>
#include <utils/debug.h>

#include <stddef.h>

#include <tsl/robin_set.h>

#include <memory>

namespace filament {

struct CameraInfo;
class FEngine;
class FIndirectLight;
class FRenderer;
class FSkybox;

class FScene : public Scene {
public:
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

    void prepare(utils::JobSystem& js, RootArenaScope& rootArenaScope,
            math::mat4 const& worldTransform, bool shadowReceiversAreCasters) noexcept;

    void prepareVisibleRenderables(utils::Range<uint32_t> visibleRenderables) noexcept;

    void prepareDynamicLights(const CameraInfo& camera,
            backend::Handle<backend::HwBufferObject> lightUbh) noexcept;

    /*
     * Storage for per-frame renderable data
     */

    using VisibleMaskType = Culler::result_type;

    enum {
        RENDERABLE_INSTANCE,    //   4 | instance of the Renderable component
        WORLD_TRANSFORM,        //  16 | instance of the Transform component
        VISIBILITY_STATE,       //   2 | visibility data of the component
        SKINNING_BUFFER,        //   8 | bones uniform buffer handle, offset, indices and weights
        MORPHING_BUFFER,        //  16 | weights uniform buffer handle, count, morph targets
        INSTANCES,              //  16 | instancing info for this Renderable
        WORLD_AABB_CENTER,      //  12 | world-space bounding box center of the renderable
        VISIBLE_MASK,           //   2 | each bit represents a visibility in a pass
        CHANNELS,               //   1 | currently light channels only

        // These are not needed anymore after culling
        LAYERS,                 //   1 | layers
        WORLD_AABB_EXTENT,      //  12 | world-space bounding box half-extent of the renderable

        // These are temporaries and should be stored out of line
        PRIMITIVES,             //   8 | level-of-detail'ed primitives
        SUMMED_PRIMITIVE_COUNT, //   4 | summed visible primitive counts
        UBO,                    // 128 |
        DESCRIPTOR_SET_HANDLE,

        // FIXME: We need a better way to handle this
        USER_DATA,              //   4 | user data currently used to store the scale
    };

    using RenderableSoa = utils::StructureOfArrays<
            utils::EntityInstance<RenderableManager>,   // RENDERABLE_INSTANCE
            math::mat4f,                                // WORLD_TRANSFORM
            FRenderableManager::Visibility,             // VISIBILITY_STATE
            FRenderableManager::SkinningBindingInfo,    // SKINNING_BUFFER
            FRenderableManager::MorphingBindingInfo,    // MORPHING_BUFFER
            FRenderableManager::InstancesInfo,          // INSTANCES
            math::float3,                               // WORLD_AABB_CENTER
            VisibleMaskType,                            // VISIBLE_MASK
            uint8_t,                                    // CHANNELS
            uint8_t,                                    // LAYERS
            math::float3,                               // WORLD_AABB_EXTENT
            utils::Slice<FRenderPrimitive>,             // PRIMITIVES
            uint32_t,                                   // SUMMED_PRIMITIVE_COUNT
            PerRenderableData,                          // UBO
            backend::DescriptorSetHandle,               // DESCRIPTOR_SET_HANDLE
            // FIXME: We need a better way to handle this
            float                                       // USER_DATA
    >;

    RenderableSoa const& getRenderableData() const noexcept { return mRenderableData; }
    RenderableSoa& getRenderableData() noexcept { return mRenderableData; }

    static inline uint32_t getPrimitiveCount(RenderableSoa const& soa,
            uint32_t const first, uint32_t const last) noexcept {
        // the caller must guarantee that last is dereferenceable
        return soa.elementAt<SUMMED_PRIMITIVE_COUNT>(last) -
                soa.elementAt<SUMMED_PRIMITIVE_COUNT>(first);
    }

    static inline uint32_t getPrimitiveCount(RenderableSoa const& soa, uint32_t const last) noexcept {
        // the caller must guarantee that last is dereferenceable
        return soa.elementAt<SUMMED_PRIMITIVE_COUNT>(last);
    }

    /*
     * Storage for per-frame light data
     */

    struct ShadowInfo {
        // These are per-light values.
        // They're packed into 32 bits and stored in the Lights uniform buffer.
        // They're unpacked in the fragment shader and used to calculate punctual shadows.
        bool castsShadows = false;      // whether this light casts shadows
        bool contactShadows = false;    // whether this light casts contact shadows
        uint8_t index = 0;              // an index into the arrays in the Shadows uniform buffer
    };

    enum {
        POSITION_RADIUS,
        DIRECTION,
        SHADOW_DIRECTION,
        SHADOW_REF,
        LIGHT_INSTANCE,
        VISIBILITY,
        SCREEN_SPACE_Z_RANGE,
        SHADOW_INFO
    };

    using LightSoa = utils::StructureOfArrays<
            math::float4,
            math::float3,
            math::float3,
            math::double2,
            FLightManager::Instance,
            Culler::result_type,
            math::float2,
            ShadowInfo
    >;

    LightSoa const& getLightData() const noexcept { return mLightData; }
    LightSoa& getLightData() noexcept { return mLightData; }

    void updateUBOs(utils::Range<uint32_t> visibleRenderables,
            backend::Handle<backend::HwBufferObject> renderableUbh) noexcept;

    bool hasContactShadows() const noexcept;

private:
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

    static inline void computeLightRanges(math::float2* zrange,
            CameraInfo const& camera, const math::float4* spheres, size_t count) noexcept;

    FEngine& mEngine;
    FSkybox* mSkybox = nullptr;
    FIndirectLight* mIndirectLight = nullptr;

    /*
     * list of Entities in the scene. We use a robin_set<> so we can do efficient removes
     * (a vector<> could work, but removes would be O(n)). robin_set<> iterates almost as
     * nicely as vector<>, which is a good compromise.
     */
    tsl::robin_set<utils::Entity, utils::Entity::Hasher> mEntities;


    /*
     * The data below is valid only during a view pass. i.e. if a scene is used in multiple
     * views, the data below is updated for each view.
     * In essence, this data should be owned by View, but it's so scene-specific, that for now
     * we store it here.
     */
    RenderableSoa mRenderableData;
    LightSoa mLightData;
    bool mHasContactShadows = false;

    // State shared between Scene and driver callbacks.
    struct SharedState {
        BufferPoolAllocator<3> mBufferPoolAllocator = {};
    };
    std::shared_ptr<SharedState> mSharedState;
};

FILAMENT_DOWNCAST(Scene)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SCENE_H
