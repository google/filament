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
#include <filament/FilamentAPI.h>
#include <filament/MaterialEnums.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/EntityInstance.h>

#include <math/mathfwd.h>

#include <type_traits>

namespace utils {
    class Entity;
} // namespace utils

namespace filament {

class Engine;
class IndexBuffer;
class Material;
class MaterialInstance;
class Renderer;
class VertexBuffer;

class FEngine;
class FRenderPrimitive;
class FRenderableManager;

/**
 * Factory and manager for \em renderables, which are entities that can be drawn.
 *
 * Renderables are bundles of \em primitives, each of which has its own geometry and material. All
 * primitives in a particular renderable share a set of rendering attributes, such as whether they
 * cast shadows or use vertex skinning.
 *
 * Usage example:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * auto renderable = utils::EntityManager::get().create();
 *
 * RenderableManager::Builder(1)
 *         .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
 *         .material(0, matInstance)
 *         .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertBuffer, indBuffer, 0, 3)
 *         .receiveShadows(false)
 *         .build(engine, renderable);
 *
 * scene->addEntity(renderable);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * To modify the state of an existing renderable, clients should first use RenderableManager
 * to get a temporary handle called an \em instance. The instance can then be used to get or set
 * the renderable's state. Please note that instances are ephemeral; clients should store entities,
 * not instances.
 *
 * - For details about constructing renderables, see RenderableManager::Builder.
 * - To associate a 4x4 transform with an entity, see TransformManager.
 * - To associate a human-readable label with an entity, see utils::NameComponentManager.
 */
class UTILS_PUBLIC RenderableManager : public FilamentAPI {
    struct BuilderDetails;

public:
    using Instance = utils::EntityInstance<RenderableManager>;
    using PrimitiveType = backend::PrimitiveType;

    /**
     * Checks if the given entity already has a renderable component.
     */
    bool hasComponent(utils::Entity e) const noexcept;

    /**
     * Gets a temporary handle that can be used to access the renderable state.
     *
     * @return Non-zero handle if the entity has a renderable component, 0 otherwise.
     */
    Instance getInstance(utils::Entity e) const noexcept;

    /**
     * The transformation associated with a skinning joint.
     *
     * Clients can specify bones either using this quat-vec3 pair, or by using 4x4 matrices.
     */
    struct Bone {
        math::quatf unitQuaternion = { 1, 0, 0, 0 };
        math::float3 translation = { 0, 0, 0 };
        float reserved = 0;
    };

    /**
     * Adds renderable components to entities using a builder pattern.
     */
    class Builder : public BuilderBase<BuilderDetails> {
        friend struct BuilderDetails;
    public:
        enum Result { Error = -1, Success = 0  };

        /**
         * Creates a builder for renderable components.
         *
         * @param count the number of primitives that will be supplied to the builder
         *
         * Note that builders typically do not have a long lifetime since clients should discard
         * them after calling build(). For a usage example, see RenderableManager.
         */
        explicit Builder(size_t count) noexcept;

        /*! \cond PRIVATE */
        Builder(Builder const& rhs) = delete;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder& rhs) = delete;
        Builder& operator=(Builder&& rhs) noexcept;
        /*! \endcond */

        /**
         * Specifies the geometry data for a primitive.
         *
         * Filament primitives must have an associated VertexBuffer and IndexBuffer. Typically, each
         * primitive is specified with a pair of daisy-chained calls: \c geometry(...) and \c
         * material(...).
         *
         * @param index zero-based index of the primitive, must be less than the count passed to Builder constructor
         * @param type specifies the topology of the primitive (e.g., \c RenderableManager::PrimitiveType::TRIANGLES)
         * @param vertices specifies the vertex buffer, which in turn specifies a set of attributes
         * @param indices specifies the index buffer (either u16 or u32)
         * @param offset specifies where in the index buffer to start reading (expressed as a number of bytes)
         * @param minIndex specifies the minimum index contained in the index buffer
         * @param maxIndex specifies the maximum index contained in the index buffer
         * @param count number of indices to read (for triangles, this should be a multiple of 3)
         */
        Builder& geometry(size_t index, PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices, size_t offset, size_t minIndex, size_t maxIndex, size_t count) noexcept;
        Builder& geometry(size_t index, PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices, size_t offset, size_t count) noexcept; //!< \overload
        Builder& geometry(size_t index, PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices) noexcept; //!< \overload

        /**
         * Binds a material instance to the specified primitive.
         *
         * If no material is specified for a given primitive, Filament will fall back to a basic default material.
         *
         * @param index zero-based index of the primitive, must be less than the count passed to Builder constructor
         * @param materialInstance the material to bind
         */
        Builder& material(size_t index, MaterialInstance const* materialInstance) noexcept;

        /**
         * The axis-aligned bounding box of the renderable.
         *
         * This is an object-space AABB used for frustum culling. For skinning and morphing, this
         * should encompass all possible vertex positions. It is mandatory unless culling is
         * disabled for the renderable.
         *
         * \see computeAABB()
         */
        Builder& boundingBox(const Box& axisAlignedBoundingBox) noexcept;

        /**
         * Sets bits in a visibility mask. By default, this is 0x1.
         *
         * This feature provides a simple mechanism for hiding and showing groups of renderables
         * in a Scene. See View::setVisibleLayers().
         *
         * For example, to set bit 1 and reset bits 0 and 2 while leaving all other bits unaffected,
         * do: `builder.layerMask(7, 2)`.
         *
         * To change this at run time, see RenderableManager::setLayerMask.
         *
         * @param select the set of bits to affect
         * @param values the replacement values for the affected bits
         */
        Builder& layerMask(uint8_t select, uint8_t values) noexcept;

        /**
         * Provides coarse-grained control over draw order.
         *
         * In general Filament reserves the right to re-order renderables to allow for efficient
         * rendering. However clients can control ordering at a coarse level using \em priority.
         *
         * For example, this could be used to draw a semitransparent HUD, if a client wishes to
         * avoid using a separate View for the HUD. Note that priority is completely orthogonal to
         * Builder::layerMask, which merely controls visibility.
         *
         * \see Builder::blendOrder()
         *
         * The priority is clamped to the range [0..7], defaults to 4; 7 is lowest priority
         * (rendered last).
         */
        Builder& priority(uint8_t priority) noexcept;

        /**
         * Controls frustum culling, true by default.
         *
         * \note Do not confuse frustum culling with backface culling. The latter is controlled via
         * the material.
         */
        Builder& culling(bool enable) noexcept;

        /**
         * Controls if this renderable casts shadows, false by default.
         */
        Builder& castShadows(bool enable) noexcept;

        /**
         * Controls if this renderable receives shadows, true by default.
         */
        Builder& receiveShadows(bool enable) noexcept;

        /**
         * Controls if this renderable uses screen-space contact shadows. This is more
         * expensive but can improve the quality of shadows, especially in large scenes.
         * (off by default).
         */
        Builder& screenSpaceContactShadows(bool enable) noexcept;

        /**
         * Enables GPU vertex skinning for up to 255 bones, 0 by default.
         *
         * Each vertex can be affected by up to 4 bones simultaneously. The attached
         * VertexBuffer must provide data in the \c BONE_INDICES slot (uvec4) and the
         * \c BONE_WEIGHTS slot (float4).
         *
         * See also RenderableManager::setBones(), which can be called on a per-frame basis
         * to advance the animation.
         *
         * @param boneCount 0 to disable, otherwise the number of bone transforms (up to 255)
         * @param transforms the initial set of transforms (one for each bone)
         */
        Builder& skinning(size_t boneCount, math::mat4f const* transforms) noexcept;
        Builder& skinning(size_t boneCount, Bone const* bones) noexcept; //!< \overload
        Builder& skinning(size_t boneCount) noexcept; //!< \overload

        /**
         * Controls if the renderable has vertex morphing targets, false by default.
         *
         * This is required to enable GPU morphing for up to 4 attributes. The attached VertexBuffer
         * must provide data in the appropriate VertexAttribute slots (\c MORPH_POSITION_0 etc).
         *
         * See also RenderableManager::setMorphWeights(), which can be called on a per-frame basis
         * to advance the animation.
         */
        Builder& morphing(bool enable) noexcept;

        /**
         * Sets an ordering index for blended primitives that all live at the same Z value.
         *
         * @param primitiveIndex the primitive of interest
         * @param order draw order number (0 by default). Only the lowest 15 bits are used.
         */
        Builder& blendOrder(size_t primitiveIndex, uint16_t order) noexcept;

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
        friend class FEngine;
        friend class FRenderPrimitive;
        friend class FRenderableManager;
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

    /**
     * Destroys the renderable component in the given entity.
     */
    void destroy(utils::Entity e) noexcept;

    /**
     * Changes the bounding box used for frustum culling.
     *
     * \see Builder::boundingBox()
     * \see RenderableManager::getAxisAlignedBoundingBox()
     */
    void setAxisAlignedBoundingBox(Instance instance, const Box& aabb) noexcept;

    /**
     * Changes the visibility bits.
     *
     * \see Builder::layerMask()
     * \see View::setVisibleLayers().
     * \see RenderableManager::getLayerMask()
     */
    void setLayerMask(Instance instance, uint8_t select, uint8_t values) noexcept;

    /**
     * Changes the coarse-level draw ordering.
     *
     * \see Builder::priority().
     */
    void setPriority(Instance instance, uint8_t priority) noexcept;

    /**
     * Changes whether or not frustum culling is on.
     *
     * \see Builder::culling()
     */
    void setCulling(Instance instance, bool enable) noexcept;

    /**
     * Changes whether or not the renderable casts shadows.
     *
     * \see Builder::castShadows()
     */
    void setCastShadows(Instance instance, bool enable) noexcept;

    /**
     * Changes whether or not the renderable can receive shadows.
     *
     * \see Builder::receiveShadows()
     */
    void setReceiveShadows(Instance instance, bool enable) noexcept;

    /**
     * Changes whether or not the renderable can use screen-space contact shadows.
     *
     * \see Builder::screenSpaceContactShadows()
     */
    void setScreenSpaceContactShadows(Instance instance, bool enable) noexcept;

    /**
     * Checks if the renderable can cast shadows.
     *
     * \see Builder::castShadows().
     */
    bool isShadowCaster(Instance instance) const noexcept;

    /**
     * Checks if the renderable can receive shadows.
     *
     * \see Builder::receiveShadows().
     */
    bool isShadowReceiver(Instance instance) const noexcept;

    /**
     * Updates the bone transforms in the range [offset, offset + boneCount).
     * The bones must be pre-allocated using Builder::skinning().
     */
    void setBones(Instance instance, Bone const* transforms, size_t boneCount = 1, size_t offset = 0) noexcept;
    void setBones(Instance instance, math::mat4f const* transforms, size_t boneCount = 1, size_t offset = 0) noexcept; //!< \overload

    /**
     * Updates the vertex morphing weights on a renderable, all zeroes by default.
     *
     * This is specified using a 4-tuple, one float per morph target. If the renderable has fewer
     * than 4 morph targets, then clients should fill the unused components with zeroes.
     *
     * The renderable must be built with morphing enabled, see Builder::morphing().
     */
    void setMorphWeights(Instance instance, math::float4 const& weights) noexcept;

    /**
     * Gets the bounding box used for frustum culling.
     *
     * \see Builder::boundingBox()
     * \see RenderableManager::setAxisAlignedBoundingBox()
     */
    const Box& getAxisAlignedBoundingBox(Instance instance) const noexcept;

    /**
     * Get the visibility bits.
     *
     * \see Builder::layerMask()
     * \see View::setVisibleLayers().
     * \see RenderableManager::getLayerMask()
     */
    uint8_t getLayerMask(Instance instance) const noexcept;

    /**
     * Gets the immutable number of primitives in the given renderable.
     */
    size_t getPrimitiveCount(Instance instance) const noexcept;

    /**
     * Changes the material instance binding for the given primitive.
     *
     * \see Builder::material()
     */
    void setMaterialInstanceAt(Instance instance,
            size_t primitiveIndex, MaterialInstance const* materialInstance) noexcept;

    /**
     * Retrieves the material instance that is bound to the given primitive.
     */
    MaterialInstance* getMaterialInstanceAt(Instance instance, size_t primitiveIndex) const noexcept;

    /**
     * Changes the geometry for the given primitive.
     *
     * \see Builder::geometry()
     */
    void setGeometryAt(Instance instance, size_t primitiveIndex,
            PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices,
            size_t offset, size_t count) noexcept;

    /**
     * Changes the active range of indices or topology for the given primitive.
     *
     * \see Builder::geometry()
     */
    void setGeometryAt(Instance instance, size_t primitiveIndex,
            PrimitiveType type, size_t offset, size_t count) noexcept;

    /**
     * Changes the ordering index for blended primitives that all live at the same Z value.
     *
     * \see Builder::blendOrder()
     *
     * @param instance the renderable of interest
     * @param primitiveIndex the primitive of interest
     * @param order draw order number (0 by default). Only the lowest 15 bits are used.
     */
    void setBlendOrderAt(Instance instance, size_t primitiveIndex, uint16_t order) noexcept;

    /**
     * Retrieves the set of enabled attribute slots in the given primitive's VertexBuffer.
     */
    AttributeBitset getEnabledAttributesAt(Instance instance, size_t primitiveIndex) const noexcept;

    /*! \cond PRIVATE */
    template<typename T>
    struct is_supported_vector_type {
        using type = typename std::enable_if<
                std::is_same<math::float4, T>::value ||
                std::is_same<math::half4,  T>::value ||
                std::is_same<math::float3, T>::value ||
                std::is_same<math::half3,  T>::value
        >::type;
    };

    template<typename T>
    struct is_supported_index_type {
        using type = typename std::enable_if<
                std::is_same<uint16_t, T>::value ||
                std::is_same<uint32_t, T>::value
        >::type;
    };
    /*! \endcond */

    /**
     * Utility method that computes the axis-aligned bounding box from a set of vertices.
     *
     * - The index type must be \c uint16_t or \c uint32_t.
     * - The vertex type must be \c float4, \c half4, \c float3, or \c half3.
     * - For 4-component vertices, the w component is ignored (implicitly replaced with 1.0).
     */
    template<typename VECTOR, typename INDEX,
            typename = typename is_supported_vector_type<VECTOR>::type,
            typename = typename is_supported_index_type<INDEX>::type>
    static Box computeAABB(VECTOR const* vertices, INDEX const* indices, size_t count,
            size_t stride = sizeof(VECTOR)) noexcept;
};

template<typename VECTOR, typename INDEX, typename, typename>
Box RenderableManager::computeAABB(VECTOR const* vertices, INDEX const* indices, size_t count,
        size_t stride) noexcept {
    math::float3 bmin(std::numeric_limits<float>::max());
    math::float3 bmax(std::numeric_limits<float>::lowest());
    for (size_t i = 0; i < count; ++i) {
        VECTOR const* p = reinterpret_cast<VECTOR const*>(
                (char const*)vertices + indices[i] * stride);
        const math::float3 v(p->x, p->y, p->z);
        bmin = min(bmin, v);
        bmax = max(bmax, v);
    }
    return Box().set(bmin, bmax);
}

} // namespace filament

#endif // TNT_FILAMENT_RENDERABLECOMPONENTMANAGER_H
