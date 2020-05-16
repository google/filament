/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_RENDERABLECOMPONENTMANAGER_H
#define TNT_FILAMENT_DETAILS_RENDERABLECOMPONENTMANAGER_H

#include "upcast.h"

#include "UniformBuffer.h"

#include "private/backend/DriverApiForward.h"

#include <backend/Handle.h>

#include <filament/Box.h>
#include <filament/RenderableManager.h>

#include <private/filament/UibGenerator.h>

#include <utils/Entity.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Slice.h>
#include <utils/Range.h>

// for gtest
class FilamentTest_Bones_Test;

namespace filament {

class FMaterialInstance;
class FRenderPrimitive;
class FIndexBuffer;
class FVertexBuffer;

class FRenderableManager : public RenderableManager {
public:
    using Instance = RenderableManager::Instance;

    // TODO: consider renaming, this pertains to material variants, not strictly visibility.
    struct Visibility {
        uint8_t priority                : 3;
        bool castShadows                : 1;
        bool receiveShadows             : 1;
        bool culling                    : 1;
        bool skinning                   : 1;
        bool morphing                   : 1;
        bool screenSpaceContactShadows  : 1;
    };

    static_assert(sizeof(Visibility) == sizeof(uint16_t), "Visibility should be 16 bits");

    explicit FRenderableManager(FEngine& engine) noexcept;
    ~FRenderableManager();

    // free-up all resources
    void terminate() noexcept;

    /*
     * Component Manager APIs
     */

    bool hasComponent(utils::Entity e) const noexcept {
        return mManager.hasComponent(e);
    }

    Instance getInstance(utils::Entity e) const noexcept {
        return mManager.getInstance(e);
    }

    void create(const RenderableManager::Builder& builder, utils::Entity entity);

    void destroy(utils::Entity e) noexcept;

    // - instances is a list of Instance (typically the list from a given scene)
    // - list is a list of index in 'instances' (typically the visible ones)
    void prepare(backend::DriverApi& driver,
            RenderableManager::Instance const* instances,
            utils::Range<uint32_t> list) const noexcept;

    void gc(utils::EntityManager& em) noexcept {
        mManager.gc(em);
    }

    inline void setAxisAlignedBoundingBox(Instance instance, const Box& aabb) noexcept;

    inline void setLayerMask(Instance instance, uint8_t select, uint8_t values) noexcept;

    // The priority is clamped to the range [0..7]
    inline void setPriority(Instance instance, uint8_t priority) noexcept;

    inline void setCastShadows(Instance instance, bool enable) noexcept;

    inline void setLayerMask(Instance instance, uint8_t layerMask) noexcept;
    inline void setReceiveShadows(Instance instance, bool enable) noexcept;
    inline void setScreenSpaceContactShadows(Instance instance, bool enable) noexcept;
    inline void setCulling(Instance instance, bool enable) noexcept;
    inline void setSkinning(Instance instance, bool enable) noexcept;
    inline void setMorphing(Instance instance, bool enable) noexcept;
    inline void setPrimitives(Instance instance, utils::Slice<FRenderPrimitive> const& primitives) noexcept;
    inline void setBones(Instance instance, Bone const* transforms, size_t boneCount, size_t offset = 0) noexcept;
    inline void setBones(Instance instance, math::mat4f const* transforms, size_t boneCount, size_t offset = 0) noexcept;
    inline void setMorphWeights(Instance instance, const math::float4& weights) noexcept;


    inline bool isShadowCaster(Instance instance) const noexcept;
    inline bool isShadowReceiver(Instance instance) const noexcept;
    inline bool isCullingEnabled(Instance instance) const noexcept;


    inline Box const& getAABB(Instance instance) const noexcept;
    inline Box const& getAxisAlignedBoundingBox(Instance instance) const noexcept { return getAABB(instance); }
    inline Visibility getVisibility(Instance instance) const noexcept;
    inline uint8_t getLayerMask(Instance instance) const noexcept;
    inline uint8_t getPriority(Instance instance) const noexcept;
    inline filament::math::float4 getMorphWeights(Instance instance) const noexcept;

    inline backend::Handle<backend::HwUniformBuffer> getBonesUbh(Instance instance) const noexcept;
    inline uint32_t getBoneCount(Instance instance) const noexcept;


    inline size_t getLevelCount(Instance instance) const noexcept { return 1; }
    inline size_t getPrimitiveCount(Instance instance, uint8_t level) const noexcept;
    void setMaterialInstanceAt(Instance instance, uint8_t level,
            size_t primitiveIndex, FMaterialInstance const* materialInstance) noexcept;
    MaterialInstance* getMaterialInstanceAt(Instance instance, uint8_t level, size_t primitiveIndex) const noexcept;
    void setGeometryAt(Instance instance, uint8_t level, size_t primitiveIndex,
            PrimitiveType type, FVertexBuffer* vertices, FIndexBuffer* indices,
            size_t offset, size_t count) noexcept;
    void setGeometryAt(Instance instance, uint8_t level, size_t primitiveIndex,
            PrimitiveType type, size_t offset, size_t count) noexcept;
    void setBlendOrderAt(Instance instance, uint8_t level, size_t primitiveIndex, uint16_t blendOrder) noexcept;
    AttributeBitset getEnabledAttributesAt(Instance instance, uint8_t level, size_t primitiveIndex) const noexcept;
    inline utils::Slice<FRenderPrimitive> const& getRenderPrimitives(Instance instance, uint8_t level) const noexcept;
    inline utils::Slice<FRenderPrimitive>& getRenderPrimitives(Instance instance, uint8_t level) noexcept;

private:
    void destroyComponent(Instance ci) noexcept;
    static void destroyComponentPrimitives(FEngine& engine,
            utils::Slice<FRenderPrimitive>& primitives) noexcept;

    struct Bones {
        filament::backend::Handle<backend::HwUniformBuffer> handle;
        UniformBuffer bones;
        size_t count;
    };

    friend class ::FilamentTest_Bones_Test;

    static void makeBone(PerRenderableUibBone* out, math::mat4f const& transforms) noexcept;

    enum {
        AABB,               // user data
        LAYERS,             // user data
        MORPH_WEIGHTS,      // user data
        VISIBILITY,         // user data
        PRIMITIVES,         // user data
        BONES,              // filament data, UBO storing a pointer to the bones information
    };

    using Base = utils::SingleInstanceComponentManager<
            Box,                             // AABB
            uint8_t,                         // LAYERS
            filament::math::float4,          // MORPH_WEIGHTS
            Visibility,                      // VISIBILITY
            utils::Slice<FRenderPrimitive>,  // PRIMITIVES
            std::unique_ptr<Bones>           // BONES
    >;

    struct Sim : public Base {
        using Base::gc;
        using Base::swap;

        struct Proxy {
            // all of this gets inlined
            UTILS_ALWAYS_INLINE
            Proxy(Base& sim, utils::EntityInstanceBase::Type i) noexcept
                    : aabb{ sim, i } { }

            union {
                // this specific usage of union is permitted. All fields are identical
                Field<AABB>         aabb;
                Field<LAYERS>       layers;
                Field<MORPH_WEIGHTS> morphWeights;
                Field<VISIBILITY>   visibility;
                Field<PRIMITIVES>   primitives;
                Field<BONES>        bones;
            };
        };

        UTILS_ALWAYS_INLINE Proxy operator[](Instance i) noexcept {
            return { *this, i };
        }
        UTILS_ALWAYS_INLINE const Proxy operator[](Instance i) const noexcept {
            return { const_cast<Sim&>(*this), i };
        }
    };

    Sim mManager;
    FEngine& mEngine;
};

FILAMENT_UPCAST(RenderableManager)

void FRenderableManager::setAxisAlignedBoundingBox(Instance instance, const Box& aabb) noexcept {
    if (instance) {
        mManager[instance].aabb = aabb;
    }
}

void FRenderableManager::setLayerMask(Instance instance,
        uint8_t select, uint8_t values) noexcept {
    if (instance) {
        uint8_t& layers = mManager[instance].layers;
        layers = (layers & ~select) | (values & select);
    }
}

void FRenderableManager::setLayerMask(Instance instance, uint8_t layerMask) noexcept {
    if (instance) {
        mManager[instance].layers = layerMask;
    }
}

void FRenderableManager::setPriority(Instance instance, uint8_t priority) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.priority = priority;
    }
}

void FRenderableManager::setCastShadows(Instance instance, bool enable) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.castShadows = enable;
    }
}

void FRenderableManager::setReceiveShadows(Instance instance, bool enable) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.receiveShadows = enable;
    }
}

void FRenderableManager::setScreenSpaceContactShadows(Instance instance, bool enable) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.screenSpaceContactShadows = enable;
    }
}

void FRenderableManager::setCulling(Instance instance, bool enable) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.culling = enable;
    }
}

void FRenderableManager::setSkinning(Instance instance, bool enable) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.skinning = enable;
    }
}

void FRenderableManager::setMorphing(Instance instance, bool enable) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.morphing = enable;
    }
}

void FRenderableManager::setPrimitives(Instance instance,
        utils::Slice<FRenderPrimitive> const& primitives) noexcept {
    if (instance) {
        mManager[instance].primitives = primitives;
    }
}

FRenderableManager::Visibility
FRenderableManager::getVisibility(Instance instance) const noexcept {
    return mManager[instance].visibility;
}

bool FRenderableManager::isShadowCaster(Instance instance) const noexcept {
    return getVisibility(instance).castShadows;
}

bool FRenderableManager::isShadowReceiver(Instance instance) const noexcept {
    return getVisibility(instance).receiveShadows;
}

bool FRenderableManager::isCullingEnabled(Instance instance) const noexcept {
    return getVisibility(instance).culling;
}

uint8_t FRenderableManager::getLayerMask(Instance instance) const noexcept {
    return mManager[instance].layers;
}

uint8_t FRenderableManager::getPriority(Instance instance) const noexcept {
    return getVisibility(instance).priority;
}

filament::math::float4 FRenderableManager::getMorphWeights(Instance instance) const noexcept {
    return mManager[instance].morphWeights;
}

Box const& FRenderableManager::getAABB(Instance instance) const noexcept {
    return mManager[instance].aabb;
}

backend::Handle<backend::HwUniformBuffer> FRenderableManager::getBonesUbh(Instance instance) const noexcept {
    std::unique_ptr<Bones> const& bones = mManager[instance].bones;
    return bones ? bones->handle : backend::Handle<backend::HwUniformBuffer>{};
}

inline uint32_t FRenderableManager::getBoneCount(Instance instance) const noexcept {
    std::unique_ptr<Bones> const& bones = mManager[instance].bones;
    return bones ? bones->count : 0;
}

utils::Slice<FRenderPrimitive> const& FRenderableManager::getRenderPrimitives(
        Instance instance, uint8_t level) const noexcept {
    return mManager[instance].primitives;
}

utils::Slice<FRenderPrimitive>& FRenderableManager::getRenderPrimitives(
        Instance instance, uint8_t level) noexcept {
    return mManager[instance].primitives;
}

size_t FRenderableManager::getPrimitiveCount(Instance instance, uint8_t level) const noexcept {
    return getRenderPrimitives(instance, level).size();
}

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_RENDERABLECOMPONENTMANAGER_H
