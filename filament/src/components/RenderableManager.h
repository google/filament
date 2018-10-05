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

#include "driver/DriverApiForward.h"
#include "driver/UniformBuffer.h"
#include "driver/Handle.h"

#include <filament/Box.h>
#include <filament/RenderableManager.h>

#include <utils/Entity.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Slice.h>
#include <utils/Range.h>

namespace filament {
namespace details {

class FMaterialInstance;
class FRenderPrimitive;

class FRenderableManager : public RenderableManager {
public:
    using Instance = RenderableManager::Instance;

    struct Visibility {
        uint8_t priority    : 3;
        bool castShadows    : 1;
        bool receiveShadows : 1;
        bool culling        : 1;
        bool skinning       : 1;
    };

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
    void prepare(driver::DriverApi& driver,
            RenderableManager::Instance const* instances,
            utils::Range<uint32_t> list) const noexcept;

    void gc(utils::EntityManager& em) noexcept {
        mManager.gc(em);
    }

    utils::Slice<const UniformBuffer> getUniformBuffers() const noexcept {
        return mManager.slice<UNIFORMS>();
    }

    utils::Slice<const Handle<HwUniformBuffer>> getUniformBufferHandles() const noexcept {
        return mManager.slice<UNIFORMS_HANDLE>();
    }

    void updateLocalUBO(Instance instance, const math::mat4f& model) noexcept;
    inline void setAxisAlignedBoundingBox(Instance instance, const Box& aabb) noexcept;

    inline void setLayerMask(Instance instance, uint8_t select, uint8_t values) noexcept;

    // The priority is clamped to the range [0..7]
    inline void setPriority(Instance instance, uint8_t priority) noexcept;

    inline void setCastShadows(Instance instance, bool enable) noexcept;

    inline void setLayerMask(Instance instance, uint8_t layerMask) noexcept;
    inline void setReceiveShadows(Instance instance, bool enable) noexcept;
    inline void setCulling(Instance instance, bool enable) noexcept;
    inline void setUniformHandle(Instance instance, Handle<HwUniformBuffer> const& handle) noexcept;
    inline void setPrimitives(Instance instance, utils::Slice<FRenderPrimitive> const& primitives) noexcept;
    inline void setBones(Instance instance, Bone const* transforms, size_t boneCount, size_t offset = 0) noexcept;
    inline void setBones(Instance instance, math::mat4f const* transforms, size_t boneCount, size_t offset = 0) noexcept;


    inline bool isShadowCaster(Instance instance) const noexcept;
    inline bool isShadowReceiver(Instance instance) const noexcept;
    inline bool isCullingEnabled(Instance instance) const noexcept;

    inline Box const& getAABB(Instance instance) const noexcept;
    inline Box const& getAxisAlignedBoundingBox(Instance instance) const noexcept { return getAABB(instance); }
    inline Visibility getVisibility(Instance instance) const noexcept;
    inline uint8_t getLayerMask(Instance instance) const noexcept;
    inline uint8_t getPriority(Instance instance) const noexcept;

    inline UniformBuffer const& getUniformBuffer(Instance instance) const noexcept;
    inline UniformBuffer& getUniformBuffer(Instance instance) noexcept;

    inline Handle<HwUniformBuffer> getUbh(Instance instance) const noexcept;
    inline Handle<HwUniformBuffer> getBonesUbh(Instance instance) const noexcept;


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
        filament::Handle<HwUniformBuffer> handle;
        UniformBuffer bones;
        uint8_t count;
    };

    enum {
        AABB,               // user data
        LAYERS,             // user data
        VISIBILITY,         // user data
        PRIMITIVES,         // user data
        UNIFORMS,           // filament data, UBO data where world-transform is stored
        UNIFORMS_HANDLE,    // filament data, handle to the driver's UBO
        BONES,              // filament data, UBO storing a pointer to the bones information
    };

    using Base = utils::SingleInstanceComponentManager<
            Box,
            uint8_t,
            Visibility,
            utils::Slice<FRenderPrimitive>,
            UniformBuffer,
            filament::Handle<HwUniformBuffer>,
            std::unique_ptr<Bones>
    >;

    struct Sim : public Base {
        using Base::gc;
        using Base::swap;

        struct Proxy {
            // all of this gets inlined
            UTILS_ALWAYS_INLINE
            constexpr Proxy(Base& sim, utils::EntityInstanceBase::Type i) noexcept
                    : aabb{ sim, i } { }

            union {
                // this specific usage of union is permitted. All fields are identical
                Field<AABB>             aabb;
                Field<LAYERS>           layers;
                Field<VISIBILITY>       visibility;
                Field<PRIMITIVES>       primitives;
                Field<UNIFORMS>         uniforms;
                Field<UNIFORMS_HANDLE>  uniformsHandle;
                Field<BONES>            bones;
            };
        };

        UTILS_ALWAYS_INLINE constexpr Proxy operator[](Instance i) noexcept {
            return { *this, i };
        }
        UTILS_ALWAYS_INLINE constexpr const Proxy operator[](Instance i) const noexcept {
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

void FRenderableManager::setCulling(Instance instance, bool enable) noexcept {
    if (instance) {
        Visibility& visibility = mManager[instance].visibility;
        visibility.culling = enable;
    }
}

void FRenderableManager::setUniformHandle(Instance instance,
        Handle<HwUniformBuffer> const& handle) noexcept {
    if (instance) {
        mManager[instance].uniformsHandle = handle;
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

Box const& FRenderableManager::getAABB(Instance instance) const noexcept {
    return mManager[instance].aabb;
}

UniformBuffer const& FRenderableManager::getUniformBuffer(Instance instance) const noexcept {
    return mManager[instance].uniforms;
}

UniformBuffer& FRenderableManager::getUniformBuffer(Instance instance) noexcept {
    return mManager[instance].uniforms;
}

Handle<HwUniformBuffer> FRenderableManager::getUbh(Instance instance) const noexcept {
    return mManager[instance].uniformsHandle;
}

Handle<HwUniformBuffer> FRenderableManager::getBonesUbh(Instance instance) const noexcept {
    std::unique_ptr<Bones> const& bones = mManager[instance].bones;
    return bones ? bones->handle : Handle<HwUniformBuffer>{};
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

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_RENDERABLECOMPONENTMANAGER_H
