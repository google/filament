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

#include "upcast.h"
#include "components/LightManager.h"
#include "components/RenderableManager.h"
#include "components/TransformManager.h"

#include "details/Culler.h"

#include "Allocators.h"

#include <filament/Box.h>
#include <filament/Scene.h>

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/Slice.h>
#include <utils/StructureOfArrays.h>
#include <utils/Range.h>
#include <utils/debug.h>

#include <cstddef>
#include <tsl/robin_set.h>

namespace filament {

struct CameraInfo;
class FEngine;
class FIndirectLight;
class FRenderer;
class FSkybox;


class FScene : public Scene {
public:

    /*
     * User Public API
     */

    void setSkybox(FSkybox* skybox) noexcept;
    FSkybox const* getSkybox() const noexcept { return mSkybox; }
    FSkybox* getSkybox() noexcept { return mSkybox; }

    void setIndirectLight(FIndirectLight const* ibl) noexcept { mIndirectLight = ibl; }
    FIndirectLight const* getIndirectLight() const noexcept { return mIndirectLight; }

    void addEntity(utils::Entity entity);
    void addEntities(const utils::Entity* entities, size_t count);
    void remove(utils::Entity entity);
    void removeEntities(const utils::Entity* entities, size_t count);

    size_t getRenderableCount() const noexcept;
    size_t getLightCount() const noexcept;
    bool hasEntity(utils::Entity entity) const noexcept;

public:
    /*
     * Filaments-scope Public API
     */

    // the directional light is always stored first in the LightSoA, so we need to account
    // for that in a few places.
    static constexpr size_t DIRECTIONAL_LIGHTS_COUNT = 1;

    explicit FScene(FEngine& engine);
    ~FScene() noexcept;
    void terminate(FEngine& engine);

    void prepare(const math::mat4f& worldOriginTransform, bool shadowReceiversAreCasters) noexcept;
    void prepareDynamicLights(const CameraInfo& camera, ArenaScope& arena,
            backend::Handle<backend::HwBufferObject> lightUbh) noexcept;


    filament::backend::Handle<backend::HwBufferObject> getRenderableUBO() const noexcept {
        return mRenderableViewUbh;
    }

    /*
     * Storage for per-frame renderable data
     */

    using VisibleMaskType = Culler::result_type;

    enum {
        RENDERABLE_INSTANCE,    //  4 | instance of the Renderable component
        WORLD_TRANSFORM,        // 16 | instance of the Transform component
        REVERSED_WINDING_ORDER, //  1 | det(WORLD_TRANSFORM)<0
        VISIBILITY_STATE,       //  1 | visibility data of the component
        BONES_UBH,              //  4 | bones uniform buffer handle
        WORLD_AABB_CENTER,      // 12 | world-space bounding box center of the renderable
        VISIBLE_MASK,           //  1 | each bit represents a visibility in a pass
        MORPH_WEIGHTS,          //  4 | floats for morphing

        // These are not needed anymore after culling
        LAYERS,                 //  1 | layers
        WORLD_AABB_EXTENT,      // 12 | world-space bounding box half-extent of the renderable

        // These are temporaries and should be stored out of line
        PRIMITIVES,             //  8 | level-of-detail'ed primitives
        SUMMED_PRIMITIVE_COUNT, //  4 | summed visible primitive counts

        // FIXME: We need a better way to handle this
        USER_DATA,              //  4 | user data currently used to store the scale
    };

    using RenderableSoa = utils::StructureOfArrays<
            utils::EntityInstance<RenderableManager>,   // RENDERABLE_INSTANCE
            math::mat4f,                                // WORLD_TRANSFORM
            bool,                                       // REVERSED_WINDING_ORDER
            FRenderableManager::Visibility,             // VISIBILITY_STATE
            backend::Handle<backend::HwBufferObject>,   // BONES_UBH
            math::float3,                               // WORLD_AABB_CENTER
            VisibleMaskType,                            // VISIBLE_MASK
            math::float4,                               // MORPH_WEIGHTS
            uint8_t,                                    // LAYERS
            math::float3,                               // WORLD_AABB_EXTENT
            utils::Slice<FRenderPrimitive>,             // PRIMITIVES
            uint32_t,                                   // SUMMED_PRIMITIVE_COUNT
            // FIXME: We need a better way to handle this
            float                                       // USER_DATA
    >;

    RenderableSoa const& getRenderableData() const noexcept { return mRenderableData; }
    RenderableSoa& getRenderableData() noexcept { return mRenderableData; }

    static inline uint32_t getPrimitiveCount(RenderableSoa const& soa,
            uint32_t first, uint32_t last) noexcept {
        // the caller must guarantee that last is dereferenceable
        return soa.elementAt<SUMMED_PRIMITIVE_COUNT>(last) -
                soa.elementAt<SUMMED_PRIMITIVE_COUNT>(first);
    }

    static inline uint32_t getPrimitiveCount(RenderableSoa const& soa, uint32_t last) noexcept {
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
        uint8_t layer = 0;              // which layer of the shadow texture array to sample from

        //  -- LSB -------------
        //  castsShadows     : 1
        //  contactShadows   : 1
        //  index            : 4
        //  layer            : 4
        //  -- MSB -------------
        uint32_t pack() const {
            assert_invariant(index < 16);
            assert_invariant(layer < 16);
            return uint8_t(castsShadows)   << 0u    |
                   uint8_t(contactShadows) << 1u    |
                   index                   << 2u    |
                   layer                   << 6u;
        }
    };

    enum {
        POSITION_RADIUS,
        DIRECTION,
        LIGHT_INSTANCE,
        VISIBILITY,
        SCREEN_SPACE_Z_RANGE,
        SHADOW_INFO
    };

    using LightSoa = utils::StructureOfArrays<
            math::float4,
            math::float3,
            FLightManager::Instance,
            Culler::result_type,
            math::float2,
            ShadowInfo
    >;

    LightSoa const& getLightData() const noexcept { return mLightData; }
    LightSoa& getLightData() noexcept { return mLightData; }

    void updateUBOs(utils::Range<uint32_t> visibleRenderables, backend::Handle<backend::HwBufferObject> renderableUbh) noexcept;

    bool hasContactShadows() const noexcept;

private:
    static inline void computeLightRanges(math::float2* zrange,
            CameraInfo const& camera, const math::float4* spheres, size_t count) noexcept;

    FEngine& mEngine;
    FSkybox* mSkybox = nullptr;
    FIndirectLight const* mIndirectLight = nullptr;

    /*
     * list of Entities in the scene. We use a robin_set<> so we can do efficient removes
     * (a vector<> could work, but removes would be O(n)). robin_set<> iterates almost as
     * nicely as vector<>, which is a good compromise.
     */
    tsl::robin_set<utils::Entity> mEntities;


    /*
     * The data below is valid only during a view pass. i.e. if a scene is used in multiple
     * views, the data below is updated for each view.
     * In essence, this data should be owned by View, but it's so scene-specific, that for now
     * we store it here.
     */
    RenderableSoa mRenderableData;
    LightSoa mLightData;
    backend::Handle<backend::HwBufferObject> mRenderableViewUbh; // This is actually owned by the view.
    bool mHasContactShadows = false;
};

FILAMENT_UPCAST(Scene)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SCENE_H
