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

#ifndef TNT_FILAMENT_RENDERABLECOMPONENTMANAGER_H
#define TNT_FILAMENT_RENDERABLECOMPONENTMANAGER_H

#include <filament/Box.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/Renderer.h>
#include <filament/FilamentAPI.h>

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/EntityInstance.h>

#include <math/mat4.h>
#include <math/vec3.h>

#include <type_traits>

namespace filament {

namespace details {
class FEngine;
class FRenderPrimitive;
class FRenderableManager;
} // namespace details

class UTILS_PUBLIC RenderableManager : public FilamentAPI {
    struct BuilderDetails;

public:
    using Instance = utils::EntityInstance<RenderableManager>;
    using PrimitiveType = driver::PrimitiveType;

    bool hasComponent(utils::Entity e) const noexcept;

    Instance getInstance(utils::Entity e) const noexcept;

    struct Bone {
        filament::math::quatf unitQuaternion = { 1, 0, 0, 0 };
        filament::math::float3 translation = { 0, 0, 0 };
        float reserved = 0;
    };

    class Builder : public BuilderBase<BuilderDetails> {
        friend struct BuilderDetails;
    public:
        enum Result { Error = -1, Success = 0  };
        explicit Builder(size_t count) noexcept;
        Builder(Builder const& rhs) = delete;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder& rhs) = delete;
        Builder& operator=(Builder&& rhs) noexcept;

        Builder& geometry(size_t index, PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices) noexcept;
        Builder& geometry(size_t index, PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices, size_t offset, size_t count) noexcept;
        Builder& geometry(size_t index, PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices, size_t offset, size_t minIndex, size_t maxIndex, size_t count) noexcept;
        Builder& material(size_t index, MaterialInstance const* materialInstance) noexcept;
        // The axis aligned bounding box of the Renderable. Mandatory unless culling is disabled.
        Builder& boundingBox(const Box& axisAlignedBoundingBox) noexcept;
        Builder& layerMask(uint8_t select, uint8_t values) noexcept;
        // The priority is clamped to the range [0..7], defaults to 4; 7 is lowest priority
        Builder& priority(uint8_t priority) noexcept;
        Builder& culling(bool enable) noexcept; // true by default
        Builder& castShadows(bool enable) noexcept; // false by default
        Builder& receiveShadows(bool enable) noexcept; // true by default
        Builder& skinning(size_t boneCount) noexcept; // 0 by default, 255 max
        Builder& skinning(size_t boneCount, Bone const* bones) noexcept;
        Builder& skinning(size_t boneCount, filament::math::mat4f const* transforms) noexcept;

        // Sets an ordering index for blended primitives that all live at the same Z value.
        Builder& blendOrder(size_t index, uint16_t order) noexcept; // 0 by default

        /**
         * Adds the Renderable component to an entity.
         *
         * @param engine Reference to the filament::Engine to associate this Renderable with.
         * @param entity Entity to add the Renderable component to.
         * @return Success if the component was created successfully, Error otherwise.
         *
         * If exceptions are disabled and an error occurs, this function is a no-op.
         *        Success can be checked by looking at the return value.
         *
         * If this component already exists on the given entity and the construction is successful,
         * it is first destroyed as if destroy(utils::Entity e) was called. In case of error,
         * the existing component is unmodified.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        Result build(Engine& engine, utils::Entity entity);

    private:
        friend class details::FEngine;
        friend class details::FRenderPrimitive;
        friend class details::FRenderableManager;
        struct Entry {
            VertexBuffer* vertices = nullptr;
            IndexBuffer* indices = nullptr;
            size_t offset = 0;
            size_t minIndex = 0;
            size_t maxIndex = 0;
            size_t count = 0;
            MaterialInstance const* materialInstance = nullptr;
            PrimitiveType type = PrimitiveType::TRIANGLES;
            uint16_t blendOrder = 0;
        };
    };

    // destroys this component from the given entity
    void destroy(utils::Entity e) noexcept;

    void setAxisAlignedBoundingBox(Instance instance, const Box& aabb) noexcept;
    void setLayerMask(Instance instance, uint8_t select, uint8_t values) noexcept;
    void setPriority(Instance instance, uint8_t priority) noexcept;
    void setCastShadows(Instance instance, bool enable) noexcept;
    void setReceiveShadows(Instance instance, bool enable) noexcept;
    bool isShadowCaster(Instance instance) const noexcept;
    bool isShadowReceiver(Instance instance) const noexcept;

    // Updates the bone transforms in the range [offset, offset + boneCount).
    // The bones must be pre-allocated using Builder::skinning().
    void setBones(Instance instance, Bone const* transforms, size_t boneCount = 1, size_t offset = 0) noexcept;
    void setBones(Instance instance, filament::math::mat4f const* transforms, size_t boneCount = 1, size_t offset = 0) noexcept;


    // getters...
    const Box& getAxisAlignedBoundingBox(Instance instance) const noexcept;

    // number of render primitives in this renderable
    size_t getPrimitiveCount(Instance instance) const noexcept;

    // set/change the material of a given render primitive
    void setMaterialInstanceAt(Instance instance,
            size_t primitiveIndex, MaterialInstance const* materialInstance) noexcept;
    MaterialInstance* getMaterialInstanceAt(Instance instance, size_t primitiveIndex) const noexcept;

    // set/change the geometry (vertex/index buffers) of a given primitive
    void setGeometryAt(Instance instance, size_t primitiveIndex,
            PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices,
            size_t offset, size_t count) noexcept;

    // set/change the offset/count in the currently set index buffer of a given primitive
    void setGeometryAt(Instance instance, size_t primitiveIndex,
            PrimitiveType type, size_t offset, size_t count) noexcept;

    // set the blend order of the given primitive, only the first 15 bits are used
    void setBlendOrderAt(Instance instance, size_t primitiveIndex, uint16_t order) noexcept;

    AttributeBitset getEnabledAttributesAt(Instance instance, size_t primitiveIndex) const noexcept;

    template<typename T>
    struct is_supported_vector_type {
        using type = typename std::enable_if<
                std::is_same<filament::math::float4, T>::value ||
                std::is_same<filament::math::half4,  T>::value ||
                std::is_same<filament::math::float3, T>::value ||
                std::is_same<filament::math::half3,  T>::value
        >::type;
    };

    template<typename T>
    struct is_supported_index_type {
        using type = typename std::enable_if<
                std::is_same<uint16_t, T>::value ||
                std::is_same<uint32_t, T>::value
        >::type;
    };

    // Helper to compute the AABB from a set of vertices
    // note: when passing double4{f|h}, .w is assumed to be 1.0
    template<typename VECTOR, typename INDEX,
            typename = typename is_supported_vector_type<VECTOR>::type,
            typename = typename is_supported_index_type<INDEX>::type>
    static Box computeAABB(VECTOR const* vertices, INDEX const* indices, size_t count,
            size_t stride = sizeof(VECTOR)) noexcept;
};

template<typename VECTOR, typename INDEX, typename, typename>
Box RenderableManager::computeAABB(VECTOR const* vertices, INDEX const* indices, size_t count,
        size_t stride) noexcept {
    filament::math::float3 bmin(std::numeric_limits<float>::max());
    filament::math::float3 bmax(std::numeric_limits<float>::lowest());
    for (size_t i = 0; i < count; ++i) {
        VECTOR const* p = reinterpret_cast<VECTOR const*>(
                (char const*)vertices + indices[i] * stride);
        const filament::math::float3 v(p->x, p->y, p->z);
        bmin = min(bmin, v);
        bmax = max(bmax, v);
    }
    return Box().set(bmin, bmax);
}

} // namespace filament

#endif // TNT_FILAMENT_RENDERABLECOMPONENTMANAGER_H
