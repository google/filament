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

#ifndef TNT_FILAMENT_RENDERABLEMANAGER_H
#define TNT_FILAMENT_RENDERABLEMANAGER_H

#include <filament/Box.h>
#include <filament/FilamentAPI.h>
#include <filament/MaterialEnums.h>
#include <filament/MorphTargetBuffer.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/EntityInstance.h>
#include <utils/FixedCapacityVector.h>

#include <math/mathfwd.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <type_traits>

#include <float.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {
    class Entity;
} // namespace utils

namespace filament {

class BufferObject;
class Engine;
class IndexBuffer;
class MaterialInstance;
class Renderer;
class SkinningBuffer;
class VertexBuffer;
class Texture;
class InstanceBuffer;

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
     * @return the number of Components
     */
    size_t getComponentCount() const noexcept;

    /**
     * @return true if the this manager has no components
     */
    bool empty() const noexcept;

    /**
     * Retrieve the `Entity` of the component from its `Instance`.
     * @param i Instance of the component obtained from getInstance()
     * @return
     */
    utils::Entity getEntity(Instance i) const noexcept;

    /**
     * Retrieve the Entities of all the components of this manager.
     * @return A list, in no particular order, of all the entities managed by this manager.
     */
    utils::Entity const* UTILS_NONNULL getEntities() const noexcept;

    /**
     * The transformation associated with a skinning joint.
     *
     * Clients can specify bones either using this quat-vec3 pair, or by using 4x4 matrices.
     */
    struct Bone {
        math::quatf unitQuaternion = { 1.f, 0.f, 0.f, 0.f };
        math::float3 translation = { 0.f, 0.f, 0.f };
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
         * Default render channel
         * @see Builder::channel()
         */
        static constexpr uint8_t DEFAULT_CHANNEL = 2u;

        /**
         * Type of geometry for a Renderable
         */
        enum class GeometryType : uint8_t {
            DYNAMIC,        //!< dynamic gemoetry has no restriction
            STATIC_BOUNDS,  //!< bounds and world space transform are immutable
            STATIC          //!< skinning/morphing not allowed and Vertex/IndexBuffer immutables
        };

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
         * @param offset specifies where in the index buffer to start reading (expressed as a number of indices)
         * @param minIndex specifies the minimum index contained in the index buffer
         * @param maxIndex specifies the maximum index contained in the index buffer
         * @param count number of indices to read (for triangles, this should be a multiple of 3)
         */
        Builder& geometry(size_t index, PrimitiveType type,
                VertexBuffer* UTILS_NONNULL vertices,
                IndexBuffer* UTILS_NONNULL indices,
                size_t offset, size_t minIndex, size_t maxIndex, size_t count) noexcept;

        Builder& geometry(size_t index, PrimitiveType type,
                VertexBuffer* UTILS_NONNULL vertices,
                IndexBuffer* UTILS_NONNULL indices,
                size_t offset, size_t count) noexcept; //!< \overload

        Builder& geometry(size_t index, PrimitiveType type,
                VertexBuffer* UTILS_NONNULL vertices,
                IndexBuffer* UTILS_NONNULL indices) noexcept; //!< \overload


        /**
         * Specify the type of geometry for this renderable. DYNAMIC geometry has no restriction,
         * STATIC_BOUNDS geometry means that both the bounds and the world-space transform of the
         * the renderable are immutable.
         * STATIC geometry has the same restrictions as STATIC_BOUNDS, but in addition disallows
         * skinning, morphing and changing the VertexBuffer or IndexBuffer in any way.
         * @param enable whether this renderable has static bounds. false by default.
         */
        Builder& geometryType(GeometryType type) noexcept;

        /**
         * Binds a material instance to the specified primitive.
         *
         * If no material is specified for a given primitive, Filament will fall back to a basic
         * default material.
         *
         * The MaterialInstance's material must have a feature level equal or lower to the engine's
         * selected feature level.
         *
         * @param index zero-based index of the primitive, must be less than the count passed to
         * Builder constructor
         * @param materialInstance the material to bind
         *
         * @see Engine::setActiveFeatureLevel
         */
        Builder& material(size_t index,
                MaterialInstance const* UTILS_NONNULL materialInstance) noexcept;

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
         * The priority is applied separately for opaque and translucent objects, that is, opaque
         * objects are always drawn before translucent objects regardless of the priority.
         *
         * For example, this could be used to draw a semitransparent HUD on top of everything,
         * without using a separate View. Note that priority is completely orthogonal to
         * Builder::layerMask, which merely controls visibility.
         *
         * The Skybox always using the lowest priority, so it's drawn last, which may improve
         * performance.
         *
         * @param priority clamped to the range [0..7], defaults to 4; 7 is lowest priority
         *                 (rendered last).
         *
         * @return Builder reference for chaining calls.
         *
         * @see Builder::blendOrder()
         * @see Builder::channel()
         * @see RenderableManager::setPriority()
         * @see RenderableManager::setBlendOrderAt()
         */
        Builder& priority(uint8_t priority) noexcept;

        /**
         * Set the channel this renderable is associated to. There can be 4 channels.
         * All renderables in a given channel are rendered together, regardless of anything else.
         * They are sorted as usual within a channel.
         * Channels work similarly to priorities, except that they enforce the strongest ordering.
         *
         * Channels 0 and 1 may not have render primitives using a material with `refractionType`
         * set to `screenspace`.
         *
         * @param channel clamped to the range [0..3], defaults to 2.
         *
         * @return Builder reference for chaining calls.
         *
         * @see Builder::blendOrder()
         * @see Builder::priority()
         * @see RenderableManager::setBlendOrderAt()
         */
        Builder& channel(uint8_t channel) noexcept;

        /**
         * Controls frustum culling, true by default.
         *
         * \note Do not confuse frustum culling with backface culling. The latter is controlled via
         * the material.
         */
        Builder& culling(bool enable) noexcept;

        /**
         * Enables or disables a light channel. Light channel 0 is enabled by default.
         *
         * @param channel Light channel to enable or disable, between 0 and 7.
         * @param enable Whether to enable or disable the light channel.
         */
        Builder& lightChannel(unsigned int channel, bool enable = true) noexcept;

        /**
         * Controls if this renderable casts shadows, false by default.
         *
         * If the View's shadow type is set to ShadowType::VSM, castShadows should only be disabled
         * if either is true:
         *   - receiveShadows is also disabled
         *   - the object is guaranteed to not cast shadows on itself or other objects (for example,
         *     a ground plane)
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
         * Allows bones to be swapped out and shared using SkinningBuffer.
         *
         * If skinning buffer mode is enabled, clients must call setSkinningBuffer() rather than
         * setBones(). This allows sharing of data between renderables.
         *
         * @param enabled If true, enables buffer object mode.  False by default.
         */
        Builder& enableSkinningBuffers(bool enabled = true) noexcept;

        /**
         * Controls if this renderable is affected by the large-scale fog.
         * @param enabled If true, enables large-scale fog on this object. Disables it otherwise.
         *                True by default.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& fog(bool enabled = true) noexcept;

        /**
         * Enables GPU vertex skinning for up to 255 bones, 0 by default.
         *
         * Skinning Buffer mode must be enabled.
         *
         * Each vertex can be affected by up to 4 bones simultaneously. The attached
         * VertexBuffer must provide data in the \c BONE_INDICES slot (uvec4) and the
         * \c BONE_WEIGHTS slot (float4).
         *
         * See also RenderableManager::setSkinningBuffer() or SkinningBuffer::setBones(),
         * which can be called on a per-frame basis to advance the animation.
         *
         * @param skinningBuffer nullptr to disable, otherwise the SkinningBuffer to use
         * @param count 0 to disable, otherwise the number of bone transforms (up to 255)
         * @param offset offset in the SkinningBuffer
         */
        Builder& skinning(SkinningBuffer* UTILS_NONNULL skinningBuffer,
                size_t count, size_t offset) noexcept;


        /**
         * Enables GPU vertex skinning for up to 255 bones, 0 by default.
         *
         * Skinning Buffer mode must be disabled.
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
        Builder& skinning(size_t boneCount, math::mat4f const* UTILS_NONNULL transforms) noexcept;
        Builder& skinning(size_t boneCount, Bone const* UTILS_NONNULL bones) noexcept; //!< \overload
        Builder& skinning(size_t boneCount) noexcept; //!< \overload

        /**
         * Define bone indices and weights "pairs" for vertex skinning as a float2.
         * The unsigned int(pair.x) defines index of the bone and pair.y is the bone weight.
         * The pairs substitute \c BONE_INDICES and the \c BONE_WEIGHTS defined in the VertexBuffer.
         * Both ways of indices and weights definition must not be combined in one primitive.
         * Number of pairs per vertex bonesPerVertex is not limited to 4 bones.
         * Vertex buffer used for \c primitiveIndex must be set for advance skinning.
         * All bone weights of one vertex should sum to one. Otherwise they will be normalized.
         * Data must be rectangular and number of bone pairs must be same for all vertices of this
         * primitive.
         * The data is arranged sequentially, all bone pairs for the first vertex, then for the
         * second vertex, and so on.
         *
         * @param primitiveIndex zero-based index of the primitive, must be less than the primitive
         *                       count passed to Builder constructor
         * @param indicesAndWeights pairs of bone index and bone weight for all vertices
         *                          sequentially
         * @param count number of all pairs, must be a multiple of vertexCount of the primitive
         *                          count = vertexCount * bonesPerVertex
         * @param bonesPerVertex number of bone pairs, same for all vertices of the primitive
         *
         * @return Builder reference for chaining calls.
         *
         * @see VertexBuffer:Builder:advancedSkinning
         */
        Builder& boneIndicesAndWeights(size_t primitiveIndex,
                math::float2 const* UTILS_NONNULL indicesAndWeights,
                size_t count, size_t bonesPerVertex) noexcept;

        /**
         * Define bone indices and weights "pairs" for vertex skinning as a float2.
         * The unsigned int(pair.x) defines index of the bone and pair.y is the bone weight.
         * The pairs substitute \c BONE_INDICES and the \c BONE_WEIGHTS defined in the VertexBuffer.
         * Both ways of indices and weights definition must not be combined in one primitive.
         * Number of pairs is not limited to 4 bones per vertex.
         * Vertex buffer used for \c primitiveIndex must be set for advance skinning.
         * All bone weights of one vertex should sum to one. Otherwise they will be normalized.
         * Data doesn't have to be rectangular and number of pairs per vertices of primitive can be
         * variable.
         * The vector of the vertices contains the vectors of the pairs
         *
         * @param primitiveIndex zero-based index of the primitive, must be less than the primitive
         *                       count passed to Builder constructor
         * @param indicesAndWeightsVectors pairs of bone index and bone weight for all vertices of
         *                                 the primitive sequentially
         *
         * @return Builder reference for chaining calls.
         *
         * @see VertexBuffer:Builder:advancedSkinning
         */
        Builder& boneIndicesAndWeights(size_t primitiveIndex,
                utils::FixedCapacityVector<
                    utils::FixedCapacityVector<math::float2>> indicesAndWeightsVector) noexcept;

        /**
         * Controls if the renderable has legacy vertex morphing targets, zero by default. This is
         * required to enable GPU morphing.
         *
         * For legacy morphing, the attached VertexBuffer must provide data in the
         * appropriate VertexAttribute slots (\c MORPH_POSITION_0 etc). Legacy morphing only
         * supports up to 4 morph targets and will be deprecated in the future. Legacy morphing must
         * be enabled on the material definition: either via the legacyMorphing material attribute
         * or by calling filamat::MaterialBuilder::useLegacyMorphing().
         *
         * See also RenderableManager::setMorphWeights(), which can be called on a per-frame basis
         * to advance the animation.
         */
        Builder& morphing(size_t targetCount) noexcept;

        /**
         * Controls if the renderable has vertex morphing targets, zero by default. This is
         * required to enable GPU morphing.
         *
         * Filament supports two morphing modes: standard (default) and legacy.
         *
         * For standard morphing, A MorphTargetBuffer must be provided.
         * Standard morphing supports up to \c CONFIG_MAX_MORPH_TARGET_COUNT morph targets.
         *
         * See also RenderableManager::setMorphWeights(), which can be called on a per-frame basis
         * to advance the animation.
         */
        Builder& morphing(MorphTargetBuffer* UTILS_NONNULL morphTargetBuffer) noexcept;

        /**
         * Specifies the the range of the MorphTargetBuffer to use with this primitive.
         *
         * @param level the level of detail (lod), only 0 can be specified
         * @param primitiveIndex zero-based index of the primitive, must be less than the count passed to Builder constructor
         * @param offset specifies where in the morph target buffer to start reading (expressed as a number of vertices)
         */
        RenderableManager::Builder& morphing(uint8_t level,
                size_t primitiveIndex, size_t offset) noexcept;


        /**
         * Sets the drawing order for blended primitives. The drawing order is either global or
         * local (default) to this Renderable. In either case, the Renderable priority takes
         * precedence.
         *
         * @param primitiveIndex the primitive of interest
         * @param order draw order number (0 by default). Only the lowest 15 bits are used.
         *
         * @return Builder reference for chaining calls.
         *
         * @see globalBlendOrderEnabled
         */
        Builder& blendOrder(size_t primitiveIndex, uint16_t order) noexcept;

        /**
         * Sets whether the blend order is global or local to this Renderable (by default).
         *
         * @param primitiveIndex the primitive of interest
         * @param enabled true for global, false for local blend ordering.
         *
         * @return Builder reference for chaining calls.
         *
         * @see blendOrder
         */
        Builder& globalBlendOrderEnabled(size_t primitiveIndex, bool enabled) noexcept;

        /**
         * Specifies the number of draw instances of this renderable. The default is 1 instance and
         * the maximum number of instances allowed is 32767. 0 is invalid.
         *
         * All instances are culled using the same bounding box, so care must be taken to make
         * sure all instances render inside the specified bounding box.
         *
         * The material must set its `instanced` parameter to `true` in order to use
         * getInstanceIndex() in the vertex or fragment shader to get the instance index and
         * possibly adjust the position or transform.
         *
         * @param instanceCount the number of instances silently clamped between 1 and 32767.
         */
        Builder& instances(size_t instanceCount) noexcept;

        /**
         * Specifies the number of draw instances of this renderable and an \c InstanceBuffer
         * containing their local transforms. The default is 1 instance and the maximum number of
         * instances allowed when supplying transforms is given by
         * \c Engine::getMaxAutomaticInstances (64 on most platforms). 0 is invalid. The
         * \c InstanceBuffer must not be destroyed before this renderable.
         *
         * All instances are culled using the same bounding box, so care must be taken to make
         * sure all instances render inside the specified bounding box.
         *
         * The material must set its `instanced` parameter to `true` in order to use
         * \c getInstanceIndex() in the vertex or fragment shader to get the instance index.
         *
         * Only the \c VERTEX_DOMAIN_OBJECT vertex domain is supported.
         *
         * The local transforms of each instance can be updated with
         * \c InstanceBuffer::setLocalTransforms.
         *
         * \see InstanceBuffer
         * \see instances(size_t, * math::mat4f const*)
         * @param instanceCount the number of instances, silently clamped between 1 and
         *                      the result of Engine::getMaxAutomaticInstances().
         * @param instanceBuffer an InstanceBuffer containing at least instanceCount transforms
         */
        Builder& instances(size_t instanceCount,
                InstanceBuffer* UTILS_NONNULL instanceBuffer) noexcept;

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
    };

    /**
     * Destroys the renderable component in the given entity.
     */
    void destroy(utils::Entity e) noexcept;

    /**
     * Changes the bounding box used for frustum culling.
     * The renderable must not have staticGeometry enabled.
     *
     * \see Builder::boundingBox()
     * \see RenderableManager::getAxisAlignedBoundingBox()
     */
    void setAxisAlignedBoundingBox(Instance instance, const Box& aabb);

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
     * Changes the channel a renderable is associated to.
     *
     * \see Builder::channel().
     */
    void setChannel(Instance instance, uint8_t channel) noexcept;

    /**
     * Changes whether or not frustum culling is on.
     *
     * \see Builder::culling()
     */
    void setCulling(Instance instance, bool enable) noexcept;

    /**
     * Changes whether or not the large-scale fog is applied to this renderable
     * @see Builder::fog()
     */
    void setFogEnabled(Instance instance, bool enable) noexcept;

    /**
     * Returns whether large-scale fog is enabled for this renderable.
     * @return True if fog is enabled for this renderable.
     * @see Builder::fog()
     */
    bool getFogEnabled(Instance instance) const noexcept;

    /**
     * Enables or disables a light channel.
     * Light channel 0 is enabled by default.
     *
     * \see Builder::lightChannel()
     */
    void setLightChannel(Instance instance, unsigned int channel, bool enable) noexcept;

    /**
     * Returns whether a light channel is enabled on a specified renderable.
     * @param instance Instance of the component obtained from getInstance().
     * @param channel  Light channel to query
     * @return         true if the light channel is enabled, false otherwise
     */
    bool getLightChannel(Instance instance, unsigned int channel) const noexcept;

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
    void setBones(Instance instance, Bone const* UTILS_NONNULL transforms,
            size_t boneCount = 1, size_t offset = 0);

    void setBones(Instance instance, math::mat4f const* UTILS_NONNULL transforms,
            size_t boneCount = 1, size_t offset = 0); //!< \overload

    /**
     * Associates a region of a SkinningBuffer to a renderable instance
     *
     * Note: due to hardware limitations offset + 256 must be smaller or equal to
     *       skinningBuffer->getBoneCount()
     *
     * @param instance          Instance of the component obtained from getInstance().
     * @param skinningBuffer    skinning buffer to associate to the instance
     * @param count             Size of the region in bones, must be smaller or equal to 256.
     * @param offset            Start offset of the region in bones
     */
    void setSkinningBuffer(Instance instance, SkinningBuffer* UTILS_NONNULL skinningBuffer,
            size_t count, size_t offset);

    /**
     * Updates the vertex morphing weights on a renderable, all zeroes by default.
     *
     * The renderable must be built with morphing enabled, see Builder::morphing(). In legacy
     * morphing mode, only the first 4 weights are considered.
     *
     * @param instance Instance of the component obtained from getInstance().
     * @param weights Pointer to morph target weights to be update.
     * @param count Number of morph target weights.
     * @param offset Index of the first morph target weight to set at instance.
     */
    void setMorphWeights(Instance instance,
            float const* UTILS_NONNULL weights, size_t count, size_t offset = 0);

    /**
     * Associates a MorphTargetBuffer to the given primitive.
     */
    void setMorphTargetBufferOffsetAt(Instance instance, uint8_t level, size_t primitiveIndex,
            size_t offset);

    /**
     * Get a MorphTargetBuffer to the given renderable or null if it doesn't exist.
     */
    MorphTargetBuffer* UTILS_NULLABLE getMorphTargetBuffer(Instance instance) const noexcept;

    /**
     * Gets the number of morphing in the given entity.
     */
    size_t getMorphTargetCount(Instance instance) const noexcept;

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
     * The MaterialInstance's material must have a feature level equal or lower to the engine's
     * selected feature level.
     *
     * @exception utils::PreConditionPanic if the engine doesn't support the material's
     *                                     feature level.
     *
     * @see Builder::material()
     * @see Engine::setActiveFeatureLevel
     */
    void setMaterialInstanceAt(Instance instance,
            size_t primitiveIndex, MaterialInstance const* UTILS_NONNULL materialInstance);

    /**
     * Clear the MaterialInstance for the given primitive.
     * @param instance Renderable's instance
     * @param primitiveIndex Primitive index
     */
    void clearMaterialInstanceAt(Instance instance, size_t primitiveIndex);

    /**
     * Retrieves the material instance that is bound to the given primitive.
     */
    MaterialInstance* UTILS_NULLABLE getMaterialInstanceAt(
            Instance instance, size_t primitiveIndex) const noexcept;

    /**
     * Changes the geometry for the given primitive.
     *
     * \see Builder::geometry()
     */
    void setGeometryAt(Instance instance, size_t primitiveIndex, PrimitiveType type,
            VertexBuffer* UTILS_NONNULL vertices,
            IndexBuffer* UTILS_NONNULL indices,
            size_t offset, size_t count) noexcept;

    /**
     * Changes the drawing order for blended primitives. The drawing order is either global or
     * local (default) to this Renderable. In either case, the Renderable priority takes precedence.
     *
     * @param instance the renderable of interest
     * @param primitiveIndex the primitive of interest
     * @param order draw order number (0 by default). Only the lowest 15 bits are used.
     *
     * @see Builder::blendOrder(), setGlobalBlendOrderEnabledAt()
     */
    void setBlendOrderAt(Instance instance, size_t primitiveIndex, uint16_t order) noexcept;

    /**
     * Changes whether the blend order is global or local to this Renderable (by default).
     *
     * @param instance the renderable of interest
     * @param primitiveIndex the primitive of interest
     * @param enabled true for global, false for local blend ordering.
     *
     * @see Builder::globalBlendOrderEnabled(), setBlendOrderAt()
     */
    void setGlobalBlendOrderEnabledAt(Instance instance, size_t primitiveIndex, bool enabled) noexcept;

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
    static Box computeAABB(
            VECTOR const* UTILS_NONNULL vertices,
            INDEX const* UTILS_NONNULL indices, size_t count,
            size_t stride = sizeof(VECTOR)) noexcept;

protected:
    // prevent heap allocation
    ~RenderableManager() = default;
};

template<typename VECTOR, typename INDEX, typename, typename>
Box RenderableManager::computeAABB(
        VECTOR const* UTILS_NONNULL vertices,
        INDEX const* UTILS_NONNULL indices,
        size_t count, size_t stride) noexcept {
    math::float3 bmin(FLT_MAX);
    math::float3 bmax(-FLT_MAX);
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

#endif // TNT_FILAMENT_RENDERABLEMANAGER_H
