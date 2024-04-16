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

package com.google.android.filament;

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Size;

import java.nio.Buffer;
import java.nio.BufferOverflowException;
import java.util.Collections;
import java.util.EnumSet;
import java.util.Set;

/**
 * Factory and manager for <em>renderables</em>, which are entities that can be drawn.
 *
 * <p>Renderables are bundles of <em>primitives</em>, each of which has its own geometry and material. All
 * primitives in a particular renderable share a set of rendering attributes, such as whether they
 * cast shadows or use vertex skinning. Kotlin usage example:</p>
 *
 * <pre>
 * val entity = EntityManager.get().create()
 *
 * RenderableManager.Builder(1)
 *         .boundingBox(Box(0.0f, 0.0f, 0.0f, 9000.0f, 9000.0f, 9000.0f))
 *         .geometry(0, RenderableManager.PrimitiveType.TRIANGLES, vb, ib)
 *         .material(0, material)
 *         .build(engine, entity)
 *
 * scene.addEntity(renderable)
 * </pre>
 *
 * <p>To modify the state of an existing renderable, clients should first use RenderableManager
 * to get a temporary handle called an <em>instance</em>. The instance can then be used to get or set
 * the renderable's state. Please note that instances are ephemeral; clients should store entities,
 * not instances.</p>
 *
 * <ul>
 * <li>For details about constructing renderables, see {@link RenderableManager.Builder}.</li>
 * <li>To associate a 4x4 transform with an entity, see {@link TransformManager}.</li>
 * </ul>
 */
public class RenderableManager {
    private static final String LOG_TAG = "Filament";

    private static final VertexBuffer.VertexAttribute[] sVertexAttributeValues =
            VertexBuffer.VertexAttribute.values();

    private long mNativeObject;

    RenderableManager(long nativeRenderableManager) {
        mNativeObject = nativeRenderableManager;
    }

    /**
     * Checks if the given entity already has a renderable component.
     */
    public boolean hasComponent(@Entity int entity) {
        return nHasComponent(mNativeObject, entity);
    }

    /**
     * Gets a temporary handle that can be used to access the renderable state.
     */
    @EntityInstance
    public int getInstance(@Entity int entity) {
        return nGetInstance(mNativeObject, entity);
    }

    /**
     * Destroys the renderable component in the given entity.
     */
    public void destroy(@Entity int entity) {
        nDestroy(mNativeObject, entity);
    }

    /**
     * Primitive types used in {@link RenderableManager.Builder#geometry}.
     */
    public enum PrimitiveType {
        POINTS(0),
        LINES(1),
        LINE_STRIP(3),
        TRIANGLES(4),
        TRIANGLE_STRIP(5);

        private final int mType;
        PrimitiveType(int value) { mType = value; }
        int getValue() { return mType; }
    }

    /**
     * Adds renderable components to entities using a builder pattern.
     */
    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        /**
         * Creates a builder for renderable components.
         *
         * @param count the number of primitives that will be supplied to the builder
         *
         * Note that builders typically do not have a long lifetime since clients should discard
         * them after calling {@link #build}. For a usage example, see {@link RenderableManager}.
         */
        public Builder(@IntRange(from = 1) int count) {
            mNativeBuilder = nCreateBuilder(count);
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        /**
         * Specifies the geometry data for a primitive.
         *
         * Filament primitives must have an associated {@link VertexBuffer} and {@link IndexBuffer}.
         * Typically, each primitive is specified with a pair of daisy-chained calls:
         * <code>geometry()</code> and <code>material()</code>.
         *
         * @param index zero-based index of the primitive, must be less than the count passed to Builder constructor
         * @param type specifies the topology of the primitive (e.g., {@link PrimitiveType#TRIANGLES})
         * @param vertices specifies the vertex buffer, which in turn specifies a set of attributes
         * @param indices specifies the index buffer (either u16 or u32)
         * @param offset specifies where in the index buffer to start reading (expressed as a number of indices)
         * @param minIndex specifies the minimum index contained in the index buffer
         * @param maxIndex specifies the maximum index contained in the index buffer
         * @param count number of indices to read (for triangles, this should be a multiple of 3)
         */
        @NonNull
        public Builder geometry(@IntRange(from = 0) int index, @NonNull PrimitiveType type,
                @NonNull VertexBuffer vertices, @NonNull IndexBuffer indices,
                @IntRange(from = 0) int offset, @IntRange(from = 0) int minIndex,
                @IntRange(from = 0) int maxIndex, @IntRange(from = 0) int count) {
            nBuilderGeometry(mNativeBuilder, index, type.getValue(), vertices.getNativeObject(),
                    indices.getNativeObject(), offset, minIndex, maxIndex, count);
            return this;
        }

        /**
         * For details, see the {@link RenderableManager.Builder#geometry} primary overload.
         */
        @NonNull
        public Builder geometry(@IntRange(from = 0) int index, @NonNull PrimitiveType type,
                @NonNull VertexBuffer vertices, @NonNull IndexBuffer indices,
                @IntRange(from = 0) int offset, @IntRange(from = 0) int count) {
            nBuilderGeometry(mNativeBuilder, index, type.getValue(), vertices.getNativeObject(),
                    indices.getNativeObject(), offset, count);
            return this;
        }

        /**
         * For details, see the {@link RenderableManager.Builder#geometry} primary overload.
         */
        @NonNull
        public Builder geometry(@IntRange(from = 0) int index, @NonNull PrimitiveType type,
                @NonNull VertexBuffer vertices, @NonNull IndexBuffer indices) {
            nBuilderGeometry(mNativeBuilder, index, type.getValue(),
                    vertices.getNativeObject(), indices.getNativeObject());
            return this;
        }

        /**
         * Type of geometry for a Renderable
         */
        public enum GeometryType {
            /** dynamic gemoetry has no restriction */
            DYNAMIC,
            /** bounds and world space transform are immutable */
            STATIC_BOUNDS,
            /** skinning/morphing not allowed and Vertex/IndexBuffer immutables */
            STATIC
        }

        /**
         * Specify whether this renderable has static bounds. In this context his means that
         * the renderable's bounding box cannot change and that the renderable's transform is
         * assumed immutable. Changing the renderable's transform via the TransformManager
         * can lead to corrupted graphics. Note that skinning and morphing are not forbidden.
         * Disabled by default.
         * @param enable whether this renderable has static bounds. false by default.
         */
        @NonNull
        public Builder geometryType(GeometryType type) {
            nBuilderGeometryType(mNativeBuilder, type.ordinal());
            return this;
        }

        /**
         * Binds a material instance to the specified primitive.
         *
         * <p>If no material is specified for a given primitive, Filament will fall back to a basic
         * default material.</p>
         *
         * @param index zero-based index of the primitive, must be less than the count passed to Builder constructor
         * @param material the material to bind
         */
        @NonNull
        public Builder material(@IntRange(from = 0) int index, @NonNull MaterialInstance material) {
            nBuilderMaterial(mNativeBuilder, index, material.getNativeObject());
            return this;
        }

        /**
         * Sets the drawing order for blended primitives. The drawing order is either global or
         * local (default) to this Renderable. In either case, the Renderable priority takes
         * precedence.
         *
         * @param index the primitive of interest
         * @param blendOrder draw order number (0 by default). Only the lowest 15 bits are used.
         */
        @NonNull
        public Builder blendOrder(@IntRange(from = 0) int index,
                @IntRange(from = 0, to = 32767) int blendOrder) {
            nBuilderBlendOrder(mNativeBuilder, index, blendOrder);
            return this;
        }

       /**
         * Sets whether the blend order is global or local to this Renderable (by default).
         *
         * @param index the primitive of interest
         * @param enabled true for global, false for local blend ordering.
         */
        @NonNull
        public Builder globalBlendOrderEnabled(@IntRange(from = 0) int index, boolean enabled) {
            nBuilderGlobalBlendOrderEnabled(mNativeBuilder, index, enabled);
            return this;
        }

        /**
         * The axis-aligned bounding box of the renderable.
         *
         * <p>This is an object-space AABB used for frustum culling. For skinning and morphing, this
         * should encompass all possible vertex positions. It is mandatory unless culling is
         * disabled for the renderable.</p>
         */
        @NonNull
        public Builder boundingBox(@NonNull Box aabb) {
            nBuilderBoundingBox(mNativeBuilder,
                    aabb.getCenter()[0], aabb.getCenter()[1], aabb.getCenter()[2],
                    aabb.getHalfExtent()[0], aabb.getHalfExtent()[1], aabb.getHalfExtent()[2]);
            return this;
        }

        /**
         * Sets bits in a visibility mask. By default, this is 0x1.
         *
         * <p>This feature provides a simple mechanism for hiding and showing groups of renderables
         * in a Scene. See {@link View#setVisibleLayers}.</p>
         *
         * <p>For example, to set bit 1 and reset bits 0 and 2 while leaving all other bits
         * unaffected, do: <code>builder.layerMask(7, 2)</code>.</p>
         *
         * @see RenderableManager#setLayerMask
         *
         * @param select the set of bits to affect
         * @param value the replacement values for the affected bits
         */
        @NonNull
        public Builder layerMask(@IntRange(from = 0, to = 255) int select,
                @IntRange(from = 0, to = 255) int value) {
            nBuilderLayerMask(mNativeBuilder, select & 0xFF, value & 0xFF);
            return this;
        }

        /**
         * Provides coarse-grained control over draw order.
         *
         * <p>In general Filament reserves the right to re-order renderables to allow for efficient
         * rendering. However clients can control ordering at a coarse level using \em priority.
         * The priority is applied separately for opaque and translucent objects, that is, opaque
         * objects are always drawn before translucent objects regardless of the priority.</p>
         *
         * <p>For example, this could be used to draw a semitransparent HUD, if a client wishes to
         * avoid using a separate View for the HUD. Note that priority is completely orthogonal to
         * {@link Builder#layerMask}, which merely controls visibility.</p>

         * <p>The Skybox always using the lowest priority, so it's drawn last, which may improve
         * performance.</p>
         *
         * <p>The priority is clamped to the range [0..7], defaults to 4; 7 is lowest priority
         * (rendered last).</p>
         *
         * @see Builder#blendOrder
         */

        /**
         * Provides coarse-grained control over draw order.
         *
         * <p>In general Filament reserves the right to re-order renderables to allow for efficient
         * rendering. However clients can control ordering at a coarse level using priority.
         * The priority is applied separately for opaque and translucent objects, that is, opaque
         * objects are always drawn before translucent objects regardless of the priority.</p>
         *
         * <p>For example, this could be used to draw a semitransparent HUD, if a client wishes to
         * avoid using a separate View for the HUD. Note that priority is completely orthogonal to
         * {@link Builder#layerMask}, which merely controls visibility.</p>

         * <p>The Skybox always using the lowest priority, so it's drawn last, which may improve
         * performance.</p>
         *
         * @param priority clamped to the range [0..7], defaults to 4; 7 is lowest priority
         *                 (rendered last).
         *
         * @return Builder reference for chaining calls.
         *
         * @see Builder#channel
         * @see Builder#blendOrder
         * @see #setPriority
         * @see #setBlendOrderAt
         */
        @NonNull
        public Builder priority(@IntRange(from = 0, to = 7) int priority) {
            nBuilderPriority(mNativeBuilder, priority);
            return this;
        }

        /**
         * Set the channel this renderable is associated to. There can be 4 channels.
         *
         * <p>All renderables in a given channel are rendered together, regardless of anything else.
         * They are sorted as usual within a channel.</p>
         * <p>Channels work similarly to priorities, except that they enforce the strongest
         * ordering.</p>
         *
         * <p>Channels 0 and 1 may not have render primitives using a material with `refractionType`
         * set to `screenspace`.</p>
         *
         * @param channel clamped to the range [0..3], defaults to 2.
         *
         * @return Builder reference for chaining calls.
         *
         * @see Builder::blendOrder()
         * @see Builder::priority()
         * @see RenderableManager::setBlendOrderAt()
         */
        @NonNull
        public Builder channel(@IntRange(from = 0, to = 3) int channel) {
            nBuilderChannel(mNativeBuilder, channel);
            return this;
        }

        /**
         * Controls frustum culling, true by default.
         *
         * <p>Do not confuse frustum culling with backface culling. The latter is controlled via
         * the material.</p>
         */
        @NonNull
        public Builder culling(boolean enabled) {
            nBuilderCulling(mNativeBuilder, enabled);
            return this;
        }

        /**
         * Enables or disables a light channel. Light channel 0 is enabled by default.
         *
         * @param channel Light channel to enable or disable, between 0 and 7.
         * @param enable Whether to enable or disable the light channel.
         */
        @NonNull
        public Builder lightChannel(@IntRange(from = 0, to = 7) int channel, boolean enable) {
            nBuilderLightChannel(mNativeBuilder, channel, enable);
            return this;
        }

        /**
         * Specifies the number of draw instance of this renderable. The default is 1 instance and
         * the maximum number of instances allowed is 32767. 0 is invalid.
         * All instances are culled using the same bounding box, so care must be taken to make
         * sure all instances render inside the specified bounding box.
         * The material can use getInstanceIndex() in the vertex shader to get the instance index and
         * possibly adjust the position or transform.
         *
         * @param instanceCount the number of instances silently clamped between 1 and 32767.
         */
        @NonNull
        public Builder instances(@IntRange(from = 1, to = 32767) int instanceCount) {
            nBuilderInstances(mNativeBuilder, instanceCount);
            return this;
        }

        /**
         * Controls if this renderable casts shadows, false by default.
         *
         * If the View's shadow type is set to {@link View.ShadowType#VSM}, castShadows should only
         * be disabled if either is true:
         * <ul>
         *   <li>{@link RenderableManager#setReceiveShadows} is also disabled</li>
         *   <li>the object is guaranteed to not cast shadows on itself or other objects (for
         *   example, a ground plane)</li>
         * </ul>
         */
        @NonNull
        public Builder castShadows(boolean enabled) {
            nBuilderCastShadows(mNativeBuilder, enabled);
            return this;
        }

        /**
         * Controls if this renderable receives shadows, true by default.
         */
        @NonNull
        public Builder receiveShadows(boolean enabled) {
            nBuilderReceiveShadows(mNativeBuilder, enabled);
            return this;
        }

        /**
         * Controls if this renderable uses screen-space contact shadows. This is more
         * expensive but can improve the quality of shadows, especially in large scenes.
         * (off by default).
         */
        @NonNull
        public Builder screenSpaceContactShadows(boolean enabled) {
            nBuilderScreenSpaceContactShadows(mNativeBuilder, enabled);
            return this;
        }

        /**
         * Allows bones to be swapped out and shared using SkinningBuffer.
         *
         * If skinning buffer mode is enabled, clients must call #setSkinningBuffer() rather than
         * #setBonesAsQuaternions(). This allows sharing of data between renderables.
         *
         * @param enabled If true, enables buffer object mode.  False by default.
         */
        @NonNull
        public Builder enableSkinningBuffers(boolean enabled) {
            nBuilderEnableSkinningBuffers(mNativeBuilder, enabled);
            return this;
        }

        /**
         * Controls if this renderable is affected by the large-scale fog.
         * @param enabled If true, enables large-scale fog on this object. Disables it otherwise.
         *                True by default.
         * @return this <code>Builder</code> object for chaining calls
         */
         @NonNull
        public Builder fog(boolean enabled) {
            nBuilderFog(mNativeBuilder, enabled);
            return this;
        }

        /**
         * Enables GPU vertex skinning for up to 255 bones, 0 by default.
         *
         *<p>Skinning Buffer mode must be enabled.</p>
         *
         *<p>Each vertex can be affected by up to 4 bones simultaneously. The attached
         * VertexBuffer must provide data in the BONE_INDICES slot (uvec4) and the
         * BONE_WEIGHTS slot (float4).</p>
         *
         *<p>See also {@link #setSkinningBuffer}, {@link SkinningBuffer#setBonesAsMatrices}
         * or  {@link SkinningBuffer#setBonesAsQuaternions},
         * which can be called on a per-frame basis to advance the animation.</p>
         *
         * @see #setSkinningBuffer
         * @see SkinningBuffer#setBonesAsMatrices
         * @see SkinningBuffer#setBonesAsQuaternions
         *
         * @param skinningBuffer null to disable, otherwise the {@link SkinningBuffer} to use
         * @param boneCount 0 to disable, otherwise the number of bone transforms (up to 255)
         * @param offset offset in the {@link SkinningBuffer}
         * @return this <code>Builder</code> object for chaining calls
         */
        @NonNull
        public Builder skinning(SkinningBuffer skinningBuffer,
                @IntRange(from = 0, to = 255) int boneCount, int offset) {
            nBuilderSkinningBuffer(mNativeBuilder,
                    skinningBuffer != null ? skinningBuffer.getNativeObject() : 0, boneCount, offset);
            return this;
        }

        @NonNull
        public Builder skinning(@IntRange(from = 0, to = 255) int boneCount) {
            nBuilderSkinning(mNativeBuilder, boneCount);
            return this;
        }

        /**
         * Enables GPU vertex skinning for up to 255 bones, 0 by default.
         *
         * <p>Skinning Buffer mode must be disabled.</p>
         *
         * <p>Each vertex can be affected by up to 4 bones simultaneously. The attached
         * VertexBuffer must provide data in the <code>BONE_INDICES</code> slot (uvec4) and the
         * <code>BONE_WEIGHTS</code> slot (float4).</p>
         *
         * <p>See also {@link RenderableManager#setBonesAsMatrices}, which can be called on a per-frame basis
         * to advance the animation.</p>
         *
         * @see SkinningBuffer#setBonesAsMatrices
         *
         * @param boneCount Number of bones associated with this component
         * @param bones A FloatBuffer containing boneCount transforms. Each transform consists of 8 float.
         *              float 0 to 3 encode a unit quaternion w+ix+jy+kz stored as x,y,z,w.
         *              float 4 to 7 encode a translation stored as x,y,z,1
         */
        @NonNull
        public Builder skinning(@IntRange(from = 0, to = 255) int boneCount, @NonNull Buffer bones) {
            int result = nBuilderSkinningBones(mNativeBuilder, boneCount, bones, bones.remaining());
            if (result < 0) {
                throw new BufferOverflowException();
            }
            return this;
        }

        /**
         * Controls if the renderable has vertex morphing targets, zero by default. This is
         * required to enable GPU morphing.
         *
         * <p>Filament supports two morphing modes: standard (default) and legacy.</p>
         *
         * <p>For standard morphing, A {@link MorphTargetBuffer} must be created and provided via
         * {@link RenderableManager#setMorphTargetBufferAt}. Standard morphing supports up to
         * <code>CONFIG_MAX_MORPH_TARGET_COUNT</code> morph targets.</p>
         *
         * For legacy morphing, the attached {@link VertexBuffer} must provide data in the
         * appropriate {@link VertexBuffer.VertexAttribute} slots (<code>MORPH_POSITION_0</code> etc).
         * Legacy morphing only supports up to 4 morph targets and will be deprecated in the future.
         * Legacy morphing must be enabled on the material definition: either via the
         * <code>legacyMorphing</code> material attribute or by calling
         * {@link MaterialBuilder::useLegacyMorphing}.
         *
         * <p>See also {@link RenderableManager#setMorphWeights}, which can be called on a per-frame basis
         * to advance the animation.</p>
         */
        @NonNull
        public Builder morphing(@IntRange(from = 0, to = 255) int targetCount) {
            nBuilderMorphing(mNativeBuilder, targetCount);
            return this;
        }

        /**
         * Specifies the morph target buffer for a primitive.
         *
         * The morph target buffer must have an associated renderable and geometry. Two conditions
         * must be met:
         * 1. The number of morph targets in the buffer must equal the renderable's morph target
         *    count.
         * 2. The vertex count of each morph target must equal the geometry's vertex count.
         *
         * @param level the level of detail (lod), only 0 can be specified
         * @param primitiveIndex zero-based index of the primitive, must be less than the count passed to Builder constructor
         * @param morphTargetBuffer specifies the morph target buffer
         * @param offset specifies where in the morph target buffer to start reading (expressed as a number of vertices)
         * @param count number of vertices in the morph target buffer to read, must equal the geometry's count (for triangles, this should be a multiple of 3)
         */
        @NonNull
        public Builder morphing(@IntRange(from = 0) int level,
                                @IntRange(from = 0) int primitiveIndex,
                                @NonNull MorphTargetBuffer morphTargetBuffer,
                                @IntRange(from = 0) int offset,
                                @IntRange(from = 0) int count) {
            nBuilderSetMorphTargetBufferAt(mNativeBuilder, level, primitiveIndex,
                    morphTargetBuffer.getNativeObject(), offset, count);
            return this;
        }

        /**
         * Utility method to specify morph target buffer for a primitive.
         * For details, see the {@link RenderableManager.Builder#morphing}.
         */
        @NonNull
        public Builder morphing(@IntRange(from = 0) int level,
                                @IntRange(from = 0) int primitiveIndex,
                                @NonNull MorphTargetBuffer morphTargetBuffer) {
            nBuilderSetMorphTargetBufferAt(mNativeBuilder, level, primitiveIndex,
                    morphTargetBuffer.getNativeObject(), 0, morphTargetBuffer.getVertexCount());
            return this;
        }

        /**
         * Adds the Renderable component to an entity.
         *
         * <p>If this component already exists on the given entity and the construction is successful,
         * it is first destroyed as if {@link RenderableManager#destroy} was called.</p>
         *
         * @param engine reference to the <code>Engine</code> to associate this renderable with
         * @param entity entity to add the renderable component to
         */
        public void build(@NonNull Engine engine, @Entity int entity) {
            if (!nBuilderBuild(mNativeBuilder, engine.getNativeObject(), entity)) {
                throw new IllegalStateException(
                    "Couldn't create Renderable component for entity " + entity + ", see log.");
            }
        }

        private static class BuilderFinalizer {
            private final long mNativeObject;
            BuilderFinalizer(long nativeObject) { mNativeObject = nativeObject; }
            @Override
            public void finalize() {
                try {
                    super.finalize();
                } catch (Throwable t) { // Ignore
                } finally {
                    nDestroyBuilder(mNativeObject);
                }
            }
        }
    }


    /**
     * Associates a {@link SkinningBuffer} to a renderable instance
     * @param i Instance of the Renderable
     * @param skinningBuffer {@link SkinningBuffer} to use
     * @param count Numbers of bones to set
     * @param offset Offset in the {@link SkinningBuffer}
     */
    public void setSkinningBuffer(@EntityInstance int i, @NonNull SkinningBuffer skinningBuffer,
                           int count, int offset) {
        nSetSkinningBuffer(mNativeObject, i, skinningBuffer.getNativeObject(), count, offset);
    }

    /**
     * Sets the transforms associated with each bone of a Renderable.
     *
     * @param i Instance of the Renderable
     * @param matrices A FloatBuffer containing boneCount 4x4 packed matrices (i.e. 16 floats each matrix and no gap between matrices)
     * @param boneCount Number of bones to set
     * @param offset Index of the first bone to set
     */
    public void setBonesAsMatrices(@EntityInstance int i,
            @NonNull Buffer matrices, @IntRange(from = 0, to = 255) int boneCount,
            @IntRange(from = 0) int offset) {
        int result = nSetBonesAsMatrices(mNativeObject, i, matrices, matrices.remaining(), boneCount, offset);
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    /**
     * Sets the transforms associated with each bone of a Renderable.
     *
     * @param i Instance of the Renderable
     * @param quaternions A FloatBuffer containing boneCount transforms. Each transform consists of 8 float.
     *                    float 0 to 3 encode a unit quaternion w+ix+jy+kz stored as x,y,z,w.
     *                    float 4 to 7 encode a translation stored as x,y,z,1
     * @param boneCount Number of bones to set
     * @param offset Index of the first bone to set
     */
    public void setBonesAsQuaternions(@EntityInstance int i,
            @NonNull Buffer quaternions, @IntRange(from = 0, to = 255) int boneCount,
            @IntRange(from = 0) int offset) {
        int result = nSetBonesAsQuaternions(mNativeObject, i, quaternions, quaternions.remaining(), boneCount, offset);
        if (result < 0) {
            throw new BufferOverflowException();
        }
    }

    /**
     * Updates the vertex morphing weights on a renderable, all zeroes by default.
     *
     * <p>The renderable must be built with morphing enabled. In legacy morphing mode, only the
     * first 4 weights are considered.</p>
     *
     * @see Builder#morphing
     */
    public void setMorphWeights(@EntityInstance int i, @NonNull float[] weights, @IntRange(from = 0) int offset) {
        nSetMorphWeights(mNativeObject, i, weights, offset);
    }

    /**
     * Changes the morph target buffer for the given primitive.
     *
     * <p>The renderable must be built with morphing enabled.</p>
     *
     * @see Builder#morphing
     */
    public void setMorphTargetBufferAt(@EntityInstance int i,
                                       @IntRange(from = 0) int level,
                                       @IntRange(from = 0) int primitiveIndex,
                                       @NonNull MorphTargetBuffer morphTargetBuffer,
                                       @IntRange(from = 0) int offset,
                                       @IntRange(from = 0) int count) {
        nSetMorphTargetBufferAt(mNativeObject, i, level, primitiveIndex,
                morphTargetBuffer.getNativeObject(), offset, count);
    }

    /**
     * Utility method to change morph target buffer for the given primitive.
     * For details, see the {@link RenderableManager#setMorphTargetBufferAt}.
     */
    public void setMorphTargetBufferAt(@EntityInstance int i,
                                       @IntRange(from = 0) int level,
                                       @IntRange(from = 0) int primitiveIndex,
                                       @NonNull MorphTargetBuffer morphTargetBuffer) {
        nSetMorphTargetBufferAt(mNativeObject, i, level, primitiveIndex,
                morphTargetBuffer.getNativeObject(), 0, morphTargetBuffer.getVertexCount());
    }

    /**
     * Gets the morph target count on a renderable.
     */
    @IntRange(from = 0)
    public int getMorphTargetCount(@EntityInstance int i) {
        return nGetMorphTargetCount(mNativeObject, i);
    }

    /**
     * Changes the bounding box used for frustum culling.
     *
     * @see Builder#boundingBox
     * @see RenderableManager#getAxisAlignedBoundingBox
     */
    public void setAxisAlignedBoundingBox(@EntityInstance int i, @NonNull Box aabb) {
        nSetAxisAlignedBoundingBox(mNativeObject, i,
                aabb.getCenter()[0], aabb.getCenter()[1], aabb.getCenter()[2],
                aabb.getHalfExtent()[0], aabb.getHalfExtent()[1], aabb.getHalfExtent()[2]);
    }

    /**
     * Changes the visibility bits.
     *
     * @see Builder#layerMask
     * @see View#setVisibleLayers
     */
    public void setLayerMask(@EntityInstance int i, @IntRange(from = 0, to = 255) int select,
            @IntRange(from = 0, to = 255) int value) {
        nSetLayerMask(mNativeObject, i, select, value);
    }

    /**
     * Changes the coarse-level draw ordering.
     *
     * @see Builder#priority
     */
    public void setPriority(@EntityInstance int i, @IntRange(from = 0, to = 7) int priority) {
        nSetPriority(mNativeObject, i, priority);
    }

    /**
     * Changes the channel of a renderable
     *
     * @see Builder#channel
     */
    public void setChannel(@EntityInstance int i, @IntRange(from = 0, to = 3) int channel) {
        nSetChannel(mNativeObject, i, channel);
    }

    /**
     * Changes whether or not frustum culling is on.
     *
     * @see Builder#culling
     */
    public void setCulling(@EntityInstance int i, boolean enabled) {
        nSetCulling(mNativeObject, i, enabled);
    }

    /**
     * Changes whether or not the large-scale fog is applied to this renderable
     * @see Builder#fog
     */
    public void setFogEnabled(@EntityInstance int i, boolean enabled) {
        nSetFogEnabled(mNativeObject, i, enabled);
    }

    /**
     * Returns whether large-scale fog is enabled for this renderable.
     * @return True if fog is enabled for this renderable.
     * @see Builder#fog
     */
    public boolean getFogEnabled(@EntityInstance int i) {
        return nGetFogEnabled(mNativeObject, i);
    }

    /**
     * Enables or disables a light channel.
     * Light channel 0 is enabled by default.
     *
     * @param i        Instance of the component obtained from getInstance().
     * @param channel  Light channel to set
     * @param enable   true to enable, false to disable
     *
     * @see Builder#lightChannel
     */
    public void setLightChannel(@EntityInstance int i, @IntRange(from = 0, to = 7) int channel, boolean enable) {
        nSetLightChannel(mNativeObject, i, channel, enable);
    }

    /**
     * Returns whether a light channel is enabled on a specified renderable.
     * @param i        Instance of the component obtained from getInstance().
     * @param channel  Light channel to query
     * @return         true if the light channel is enabled, false otherwise
     */
    public boolean getLightChannel(@EntityInstance int i, @IntRange(from = 0, to = 7) int channel) {
        return nGetLightChannel(mNativeObject, i, channel);
    }

    /**
     * Changes whether or not the renderable casts shadows.
     *
     * @see Builder#castShadows
     */
    public void setCastShadows(@EntityInstance int i, boolean enabled) {
        nSetCastShadows(mNativeObject, i, enabled);
    }

    /**
     * Changes whether or not the renderable can receive shadows.
     *
     * @see Builder#receiveShadows
     */
    public void setReceiveShadows(@EntityInstance int i, boolean enabled) {
        nSetReceiveShadows(mNativeObject, i, enabled);
    }

    /**
     * Changes whether or not the renderable can use screen-space contact shadows.

     *
     * @see Builder#screenSpaceContactShadows
     */
    public void setScreenSpaceContactShadows(@EntityInstance int i, boolean enabled) {
        nSetScreenSpaceContactShadows(mNativeObject, i, enabled);
    }

    /**
     * Checks if the renderable can cast shadows.
     *
     * @see Builder#castShadows
     */
    public boolean isShadowCaster(@EntityInstance int i) {
        return nIsShadowCaster(mNativeObject, i);
    }

    /**
     * Checks if the renderable can receive shadows.
     *
     * @see Builder#receiveShadows
     */
    public boolean isShadowReceiver(@EntityInstance int i) {
        return nIsShadowReceiver(mNativeObject, i);
    }

    /**
     * Gets the bounding box used for frustum culling.
     *
     * @see Builder#boundingBox
     * @see RenderableManager#setAxisAlignedBoundingBox
     */
    @NonNull
    public Box getAxisAlignedBoundingBox(@EntityInstance int i, @Nullable Box out) {
        if (out == null) out = new Box();
        nGetAxisAlignedBoundingBox(mNativeObject, i, out.getCenter(), out.getHalfExtent());
        return out;
    }

    /**
     * Gets the immutable number of primitives in the given renderable.
     */
    @IntRange(from = 0)
    public int getPrimitiveCount(@EntityInstance int i) {
        return nGetPrimitiveCount(mNativeObject, i);
    }

    /**
     * Changes the material instance binding for the given primitive.
     *
     * @see Builder#material
     */
    public void setMaterialInstanceAt(@EntityInstance int i, @IntRange(from = 0) int primitiveIndex,
            @NonNull MaterialInstance materialInstance) {
        int required = materialInstance.getMaterial().getRequiredAttributesAsInt();
        int declared = nGetEnabledAttributesAt(mNativeObject, i, primitiveIndex);
        if ((declared & required) != required) {
            Platform.get().warn("setMaterialInstanceAt() on primitive "
                    + primitiveIndex + " of Renderable at " + i
                    + ": declared attributes " + getEnabledAttributesAt(i, primitiveIndex)
                    + " do no satisfy required attributes " + materialInstance.getMaterial().getRequiredAttributes());
        }
        nSetMaterialInstanceAt(mNativeObject, i, primitiveIndex, materialInstance.getNativeObject());
    }

    /**
     * Creates a MaterialInstance Java wrapper object for a particular material instance.
     */
    public @NonNull MaterialInstance getMaterialInstanceAt(@EntityInstance int i,
            @IntRange(from = 0) int primitiveIndex) {
        long nativeMatInstance = nGetMaterialInstanceAt(mNativeObject, i, primitiveIndex);
        return new MaterialInstance(nativeMatInstance);
    }

    /**
     * Changes the geometry for the given primitive.
     *
     * @see Builder#geometry Builder.geometry
     */
    public void setGeometryAt(@EntityInstance int i, @IntRange(from = 0) int primitiveIndex,
            @NonNull PrimitiveType type, @NonNull VertexBuffer vertices,
            @NonNull IndexBuffer indices, @IntRange(from = 0) int offset,
            @IntRange(from = 0) int count) {
        nSetGeometryAt(mNativeObject, i, primitiveIndex, type.getValue(), vertices.getNativeObject(), indices.getNativeObject(), offset, count);
    }

    /**
     * Changes the geometry for the given primitive.
     *
     * @see Builder#geometry Builder.geometry
     */
    public void setGeometryAt(@EntityInstance int i, @IntRange(from = 0) int primitiveIndex,
            @NonNull PrimitiveType type, @NonNull VertexBuffer vertices,
            @NonNull IndexBuffer indices) {
        nSetGeometryAt(mNativeObject, i, primitiveIndex, type.getValue(), vertices.getNativeObject(), indices.getNativeObject(),
                0, indices.getIndexCount());
    }

     /**
     * Changes the drawing order for blended primitives. The drawing order is either global or
     * local (default) to this Renderable. In either case, the Renderable priority takes precedence.
     *
     * @see Builder#blendOrder
     *
     * @param instance the renderable of interest
     * @param primitiveIndex the primitive of interest
     * @param blendOrder draw order number (0 by default). Only the lowest 15 bits are used.
     */
    public void setBlendOrderAt(@EntityInstance int instance, @IntRange(from = 0) int primitiveIndex,
            @IntRange(from = 0, to = 65535) int blendOrder) {
        nSetBlendOrderAt(mNativeObject, instance, primitiveIndex, blendOrder);
    }

    /**
     * Changes whether the blend order is global or local to this Renderable (by default).
     *
     * @see Builder#globalBlendOrderEnabled
     *
     * @param instance the renderable of interest
     * @param primitiveIndex the primitive of interest
     * @param enabled true for global, false for local blend ordering.
     */
    public void setGlobalBlendOrderEnabledAt(@EntityInstance int instance, @IntRange(from = 0) int primitiveIndex,
            boolean enabled) {
        nSetGlobalBlendOrderEnabledAt(mNativeObject, instance, primitiveIndex, enabled);
    }

    /**
     * Retrieves the set of enabled attribute slots in the given primitive's VertexBuffer.
     */
    public Set<VertexBuffer.VertexAttribute> getEnabledAttributesAt(
            @EntityInstance int i, @IntRange(from = 0) int primitiveIndex) {
        int bitSet = nGetEnabledAttributesAt(mNativeObject, i, primitiveIndex);
        Set<VertexBuffer.VertexAttribute> requiredAttributes =
                EnumSet.noneOf(VertexBuffer.VertexAttribute.class);
        VertexBuffer.VertexAttribute[] values = sVertexAttributeValues;

        for (int j = 0; j < values.length; j++) {
            if ((bitSet & (1 << j)) != 0) {
                requiredAttributes.add(values[j]);
            }
        }

        requiredAttributes = Collections.unmodifiableSet(requiredAttributes);
        return requiredAttributes;
    }

    public long getNativeObject() {
        return mNativeObject;
    }

    private static native boolean nHasComponent(long nativeRenderableManager, int entity);
    private static native int nGetInstance(long nativeRenderableManager, int entity);
    private static native void nDestroy(long nativeRenderableManager, int entity);

    private static native long nCreateBuilder(int count);
    private static native void nDestroyBuilder(long nativeBuilder);
    private static native boolean nBuilderBuild(long nativeBuilder, long nativeEngine, int entity);

    private static native void nBuilderGeometry(long nativeBuilder, int index, int value, long nativeVertexBuffer, long nativeIndexBuffer);
    private static native void nBuilderGeometry(long nativeBuilder, int index, int value, long nativeVertexBuffer, long nativeIndexBuffer, int offset, int count);
    private static native void nBuilderGeometry(long nativeBuilder, int index, int value, long nativeVertexBuffer, long nativeIndexBuffer, int offset, int minIndex, int maxIndex, int count);
    private static native void nBuilderGeometryType(long nativeBuilder, int type);
    private static native void nBuilderMaterial(long nativeBuilder, int index, long nativeMaterialInstance);
    private static native void nBuilderBlendOrder(long nativeBuilder, int index, int blendOrder);
    private static native void nBuilderGlobalBlendOrderEnabled(long nativeBuilder, int index, boolean enabled);
    private static native void nBuilderBoundingBox(long nativeBuilder, float cx, float cy, float cz, float ex, float ey, float ez);
    private static native void nBuilderLayerMask(long nativeBuilder, int select, int value);
    private static native void nBuilderPriority(long nativeBuilder, int priority);
    private static native void nBuilderChannel(long nativeBuilder, int channel);
    private static native void nBuilderCulling(long nativeBuilder, boolean enabled);
    private static native void nBuilderCastShadows(long nativeBuilder, boolean enabled);
    private static native void nBuilderReceiveShadows(long nativeBuilder, boolean enabled);
    private static native void nBuilderScreenSpaceContactShadows(long nativeBuilder, boolean enabled);
    private static native void nBuilderSkinning(long nativeBuilder, int boneCount);
    private static native int nBuilderSkinningBones(long nativeBuilder, int boneCount, Buffer bones, int remaining);
    private static native void nBuilderSkinningBuffer(long nativeBuilder, long nativeSkinningBuffer, int boneCount, int offset);
    private static native void nBuilderMorphing(long nativeBuilder, int targetCount);
    private static native void nBuilderSetMorphTargetBufferAt(long nativeBuilder, int level, int primitiveIndex, long nativeMorphTargetBuffer, int offset, int count);
    private static native void nBuilderEnableSkinningBuffers(long nativeBuilder, boolean enabled);
    private static native void nBuilderFog(long nativeBuilder, boolean enabled);
    private static native void nBuilderLightChannel(long nativeRenderableManager, int channel, boolean enable);
    private static native void nBuilderInstances(long nativeRenderableManager, int instances);

    private static native void nSetSkinningBuffer(long nativeObject, int i, long nativeSkinningBuffer, int count, int offset);
    private static native int nSetBonesAsMatrices(long nativeObject, int i, Buffer matrices, int remaining, int boneCount, int offset);
    private static native int nSetBonesAsQuaternions(long nativeObject, int i, Buffer quaternions, int remaining, int boneCount, int offset);
    private static native void nSetMorphWeights(long nativeObject, int instance, float[] weights, int offset);
    private static native void nSetMorphTargetBufferAt(long nativeObject, int i, int level, int primitiveIndex, long nativeMorphTargetBuffer, int offset, int count);
    private static native int nGetMorphTargetCount(long nativeObject, int i);
    private static native void nSetAxisAlignedBoundingBox(long nativeRenderableManager, int i, float cx, float cy, float cz, float ex, float ey, float ez);
    private static native void nSetLayerMask(long nativeRenderableManager, int i, int select, int value);
    private static native void nSetPriority(long nativeRenderableManager, int i, int priority);
    private static native void nSetChannel(long nativeRenderableManager, int i, int channel);
    private static native void nSetCulling(long nativeRenderableManager, int i, boolean enabled);
    private static native void nSetFogEnabled(long nativeRenderableManager, int i, boolean enabled);
    private static native boolean nGetFogEnabled(long nativeRenderableManager, int i);
    private static native void nSetLightChannel(long nativeRenderableManager, int i, int channel, boolean enable);
    private static native boolean nGetLightChannel(long nativeRenderableManager, int i, int channel);
    private static native void nSetCastShadows(long nativeRenderableManager, int i, boolean enabled);
    private static native void nSetReceiveShadows(long nativeRenderableManager, int i, boolean enabled);
    private static native void nSetScreenSpaceContactShadows(long nativeRenderableManager, int i, boolean enabled);
    private static native boolean nIsShadowCaster(long nativeRenderableManager, int i);
    private static native boolean nIsShadowReceiver(long nativeRenderableManager, int i);
    private static native void nGetAxisAlignedBoundingBox(long nativeRenderableManager, int i, float[] center, float[] halfExtent);
    private static native int nGetPrimitiveCount(long nativeRenderableManager, int i);
    private static native void nSetMaterialInstanceAt(long nativeRenderableManager, int i, int primitiveIndex, long nativeMaterialInstance);
    private static native long nGetMaterialInstanceAt(long nativeRenderableManager, int i, int primitiveIndex);
    private static native void nSetGeometryAt(long nativeRenderableManager, int i, int primitiveIndex, int primitiveType, long nativeVertexBuffer, long nativeIndexBuffer, int offset, int count);
    private static native void nSetBlendOrderAt(long nativeRenderableManager, int i, int primitiveIndex, int blendOrder);
    private static native void nSetGlobalBlendOrderEnabledAt(long nativeRenderableManager, int i, int primitiveIndex, boolean enabled);
    private static native int nGetEnabledAttributesAt(long nativeRenderableManager, int i, int primitiveIndex);
}
