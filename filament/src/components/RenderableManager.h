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

#ifndef TNT_FILAMENT_COMPONENTS_RENDERABLEMANAGER_H
#define TNT_FILAMENT_COMPONENTS_RENDERABLEMANAGER_H

#include "downcast.h"

#include "HwRenderPrimitiveFactory.h"
#include "UniformBuffer.h"

#include "backend/DriverApiForward.h"

#include <backend/Handle.h>

#include <filament/Box.h>
#include <filament/RenderableManager.h>

#include <details/InstanceBuffer.h>

#include <private/filament/UibStructs.h>

#include <utils/Entity.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Slice.h>
#include <utils/Range.h>

namespace filament {

class FBufferObject;
class FIndexBuffer;
class FMaterialInstance;
class FMorphTargetBuffer;
class FRenderPrimitive;
class FSkinningBuffer;
class FVertexBuffer;
class FTexture;

class FRenderableManager : public RenderableManager {
public:
    using Instance = RenderableManager::Instance;

    // TODO: consider renaming, this pertains to material variants, not strictly visibility.
    struct Visibility {
        uint8_t priority                : 3;
        uint8_t channel                 : 2;
        bool castShadows                : 1;
        bool receiveShadows             : 1;
        bool culling                    : 1;
        bool skinning                   : 1;
        bool morphing                   : 1;
        bool screenSpaceContactShadows  : 1;
        bool reversedWindingOrder       : 1;
        bool fog                        : 1;
    };

    static_assert(sizeof(Visibility) == sizeof(uint16_t), "Visibility should be 16 bits");

    struct MorphTargets {
        FMorphTargetBuffer* buffer = nullptr;
        uint32_t offset = 0;
        uint32_t count = 0;
    };

    explicit FRenderableManager(FEngine& engine) noexcept;
    ~FRenderableManager();

    // free-up all resources
    void terminate() noexcept;

    void gc(utils::EntityManager& em) noexcept;

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

    inline void setAxisAlignedBoundingBox(Instance instance, const Box& aabb) noexcept;

    inline void setLayerMask(Instance instance, uint8_t select, uint8_t values) noexcept;

    // The priority is clamped to the range [0..7]
    inline void setPriority(Instance instance, uint8_t priority) noexcept;

    // The channel is clamped to the range [0..3]
    inline void setChannel(Instance instance, uint8_t channel) noexcept;

    inline void setCastShadows(Instance instance, bool enable) noexcept;

    inline void setLayerMask(Instance instance, uint8_t layerMask) noexcept;
    inline void setReceiveShadows(Instance instance, bool enable) noexcept;
    inline void setScreenSpaceContactShadows(Instance instance, bool enable) noexcept;
    inline void setCulling(Instance instance, bool enable) noexcept;
    inline void setFogEnabled(Instance instance, bool enable) noexcept;
    inline bool getFogEnabled(Instance instance) const noexcept;

    inline void setPrimitives(Instance instance, utils::Slice<FRenderPrimitive> const& primitives) noexcept;

    inline void setSkinning(Instance instance, bool enable) noexcept;
    void setBones(Instance instance, Bone const* transforms, size_t boneCount, size_t offset = 0);
    void setBones(Instance instance, math::mat4f const* transforms, size_t boneCount, size_t offset = 0);
    void setSkinningBuffer(Instance instance, FSkinningBuffer* skinningBuffer,
            size_t count, size_t offset);

    inline void setMorphing(Instance instance, bool enable) noexcept;
    void setMorphWeights(Instance instance, float const* weights, size_t count, size_t offset);
    void setMorphTargetBufferAt(Instance instance, uint8_t level, size_t primitiveIndex,
            FMorphTargetBuffer* morphTargetBuffer, size_t offset, size_t count);
    MorphTargetBuffer* getMorphTargetBufferAt(Instance instance, uint8_t level, size_t primitiveIndex) const noexcept;
    size_t getMorphTargetCount(Instance instance) const noexcept;

    void setLightChannel(Instance instance, unsigned int channel, bool enable) noexcept;
    bool getLightChannel(Instance instance, unsigned int channel) const noexcept;

    inline bool isShadowCaster(Instance instance) const noexcept;
    inline bool isShadowReceiver(Instance instance) const noexcept;
    inline bool isCullingEnabled(Instance instance) const noexcept;


    inline Box const& getAABB(Instance instance) const noexcept;
    inline Box const& getAxisAlignedBoundingBox(Instance instance) const noexcept { return getAABB(instance); }
    inline Visibility getVisibility(Instance instance) const noexcept;
    inline uint8_t getLayerMask(Instance instance) const noexcept;
    inline uint8_t getPriority(Instance instance) const noexcept;
    inline uint8_t getChannels(Instance instance) const noexcept;

    struct SkinningBindingInfo {
        backend::Handle<backend::HwBufferObject> handle;
        uint32_t offset;
    };

    inline SkinningBindingInfo getSkinningBufferInfo(Instance instance) const noexcept;
    inline uint32_t getBoneCount(Instance instance) const noexcept;

    struct MorphingBindingInfo {
        backend::Handle<backend::HwBufferObject> handle;
        uint32_t count;
        MorphTargets const* targets; // Pointer to Slice<MorphTargets> at a renderable.
    };
    inline MorphingBindingInfo getMorphingBufferInfo(Instance instance) const noexcept;

    struct InstancesInfo {
        union {
            FInstanceBuffer* buffer;
            uint64_t padding;          // ensures the pointer is 64 bits on all archs
        };
        backend::Handle<backend::HwBufferObject> handle;
        uint16_t count;
        char padding0[2];
    };
    static_assert(sizeof(InstancesInfo) == 16);
    inline InstancesInfo getInstancesInfo(Instance instance) const noexcept;

    utils::Entity getEntity(Instance instance) const noexcept {
        return mManager.getEntity(instance);
    }

    inline size_t getLevelCount(Instance) const noexcept { return 1u; }
    size_t getPrimitiveCount(Instance instance, uint8_t level) const noexcept;
    void setMaterialInstanceAt(Instance instance, uint8_t level,
            size_t primitiveIndex, FMaterialInstance const* materialInstance);
    MaterialInstance* getMaterialInstanceAt(Instance instance, uint8_t level, size_t primitiveIndex) const noexcept;
    void setGeometryAt(Instance instance, uint8_t level, size_t primitiveIndex,
            PrimitiveType type, FVertexBuffer* vertices, FIndexBuffer* indices,
            size_t offset, size_t count) noexcept;
    void setBlendOrderAt(Instance instance, uint8_t level, size_t primitiveIndex, uint16_t blendOrder) noexcept;
    void setGlobalBlendOrderEnabledAt(Instance instance, uint8_t level, size_t primitiveIndex, bool enabled) noexcept;
    AttributeBitset getEnabledAttributesAt(Instance instance, uint8_t level, size_t primitiveIndex) const noexcept;
    inline utils::Slice<FRenderPrimitive> const& getRenderPrimitives(Instance instance, uint8_t level) const noexcept;
    inline utils::Slice<FRenderPrimitive>& getRenderPrimitives(Instance instance, uint8_t level) noexcept;
    inline utils::Slice<MorphTargets> const& getMorphTargets(Instance instance, uint8_t level) const noexcept;
    inline utils::Slice<MorphTargets>& getMorphTargets(Instance instance, uint8_t level) noexcept;

private:
    void destroyComponent(Instance ci) noexcept;
    static void destroyComponentPrimitives(
            HwRenderPrimitiveFactory& factory, backend::DriverApi& driver,
            utils::Slice<FRenderPrimitive>& primitives) noexcept;
    static void destroyComponentMorphTargets(FEngine& engine,
            utils::Slice<MorphTargets>& morphTargets) noexcept;

    struct Bones {
        backend::Handle<backend::HwBufferObject> handle;
        uint16_t count = 0;
        uint16_t offset = 0;
        bool skinningBufferMode = false;
    };
    static_assert(sizeof(Bones) == 12);

    struct MorphWeights {
        backend::Handle<backend::HwBufferObject> handle;
        uint32_t count = 0;
    };
    static_assert(sizeof(MorphWeights) == 8);

    enum {
        AABB,                   // user data
        LAYERS,                 // user data
        MORPH_WEIGHTS,          // filament data, UBO storing a pointer to the morph weights information
        CHANNELS,               // user data
        INSTANCES,              // user data
        VISIBILITY,             // user data
        PRIMITIVES,             // user data
        BONES,                  // filament data, UBO storing a pointer to the bones information
        MORPH_TARGETS
    };

    using Base = utils::SingleInstanceComponentManager<
            Box,                             // AABB
            uint8_t,                         // LAYERS
            MorphWeights,                    // MORPH_WEIGHTS
            uint8_t,                         // CHANNELS
            InstancesInfo,                   // INSTANCES
            Visibility,                      // VISIBILITY
            utils::Slice<FRenderPrimitive>,  // PRIMITIVES
            Bones,                           // BONES
            utils::Slice<MorphTargets>       // MORPH_TARGETS
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
                Field<AABB>                 aabb;
                Field<LAYERS>               layers;
                Field<MORPH_WEIGHTS>        morphWeights;
                Field<CHANNELS>             channels;
                Field<INSTANCES>            instances;
                Field<VISIBILITY>           visibility;
                Field<PRIMITIVES>           primitives;
                Field<BONES>                bones;
                Field<MORPH_TARGETS>        morphTargets;
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
    HwRenderPrimitiveFactory mHwRenderPrimitiveFactory;
};

FILAMENT_DOWNCAST(RenderableManager)

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
        visibility.priority = std::min(priority, uint8_t(0x7));
    }
}

void FRenderableManager::setChannel(Instance instance, uint8_t channel) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.channel = std::min(channel, uint8_t(0x3));
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

void FRenderableManager::setFogEnabled(Instance instance, bool enable) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.fog = enable;
    }
}

bool FRenderableManager::getFogEnabled(RenderableManager::Instance instance) const noexcept {
    return getVisibility(instance).fog;
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

uint8_t FRenderableManager::getChannels(Instance instance) const noexcept {
    return mManager[instance].channels;
}

Box const& FRenderableManager::getAABB(Instance instance) const noexcept {
    return mManager[instance].aabb;
}

FRenderableManager::SkinningBindingInfo
FRenderableManager::getSkinningBufferInfo(Instance instance) const noexcept {
    Bones const& bones = mManager[instance].bones;
    return { bones.handle, bones.offset };
}

inline uint32_t FRenderableManager::getBoneCount(Instance instance) const noexcept {
    Bones const& bones = mManager[instance].bones;
    return bones.count;
}

FRenderableManager::MorphingBindingInfo
FRenderableManager::getMorphingBufferInfo(Instance instance) const noexcept {
    MorphWeights const& morphWeights = mManager[instance].morphWeights;
    utils::Slice<MorphTargets> const& morphTargets = getMorphTargets(instance, 0);
    return { morphWeights.handle, morphWeights.count, morphTargets.data() };
}

FRenderableManager::InstancesInfo
FRenderableManager::getInstancesInfo(Instance instance) const noexcept {
    return mManager[instance].instances;
}

utils::Slice<FRenderPrimitive> const& FRenderableManager::getRenderPrimitives(
        Instance instance, uint8_t level) const noexcept {
    return mManager[instance].primitives;
}

utils::Slice<FRenderPrimitive>& FRenderableManager::getRenderPrimitives(
        Instance instance, uint8_t level) noexcept {
    return mManager[instance].primitives;
}

utils::Slice<FRenderableManager::MorphTargets> const& FRenderableManager::getMorphTargets(
        Instance instance, uint8_t level) const noexcept {
    return mManager[instance].morphTargets;
}

utils::Slice<FRenderableManager::MorphTargets>& FRenderableManager::getMorphTargets(
        Instance instance, uint8_t level) noexcept {
    return mManager[instance].morphTargets;
}

} // namespace filament

#endif // TNT_FILAMENT_COMPONENTS_RENDERABLEMANAGER_H
