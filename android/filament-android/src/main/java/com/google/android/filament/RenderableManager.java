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
        TRIANGLES(4);

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
         * them after calling build(). For a usage example, see {@link RenderableManager}.
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
         * @param type specifies the topology of the primitive (e.g., PrimitiveType.TRIANGLES)
         * @param vertices specifies the vertex buffer, which in turn specifies a set of attributes
         * @param indices specifies the index buffer (either u16 or u32)
         * @param offset specifies where in the index buffer to start reading (expressed as a number of bytes)
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
         * For details, see the {@link RenderableManager.Builder#geometry primary overload}.
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
         * For details, see the {@link RenderableManager.Builder#geometry primary overload}.
         */
        @NonNull
        public Builder geometry(@IntRange(from = 0) int index, @NonNull PrimitiveType type,
                @NonNull VertexBuffer vertices, @NonNull IndexBuffer indices) {
            nBuilderGeometry(mNativeBuilder, index, type.getValue(),
                    vertices.getNativeObject(), indices.getNativeObject());
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
         * Sets an ordering index for blended primitives that all live at the same Z value.
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
         * rendering. However clients can control ordering at a coarse level using <em>priority</em>.</p>
         *
         * <p>For example, this could be used to draw a semitransparent HUD, if a client wishes to
         * avoid using a separate View for the HUD. Note that priority is completely orthogonal to
         * {@link Builder#layerMask}, which merely controls visibility.</p>
         *
         * <p>The priority is clamped to the range [0..7], defaults to 4; 7 is lowest priority
         * (rendered last).</p>
         *
         * @see Builder#blendOrder
         */
        @NonNull
        public Builder priority(@IntRange(from = 0, to = 7) int priority) {
            nBuilderPriority(mNativeBuilder, priority);
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
         * Controls if this renderable casts shadows, false by default.
         *
         * If the View's shadow type is set to {@link View.ShadowType#VSM}, castShadows should only
         * be disabled if either is true:
         * <ul>
         *   <li>{@link RenderableManager#setReceiveShadows} is also disabled</li>
         *   <li>the object is guaranteed to not cast shadows on themselves or other objects (for
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

        @NonNull
        public Builder skinning(@IntRange(from = 0, to = 255) int boneCount) {
            nBuilderSkinning(mNativeBuilder, boneCount);
            return this;
        }

        /**
         * Enables GPU vertex skinning for up to 255 bones, 0 by default.
         *
         * <p>Each vertex can be affected by up to 4 bones simultaneously. The attached
         * VertexBuffer must provide data in the <code>BONE_INDICES</code> slot (uvec4) and the
         * <code>BONE_WEIGHTS</code> slot (float4).</p>
         *
         * <p>See also {@link RenderableManager#setBonesAsMatrices}, which can be called on a per-frame basis
         * to advance the animation.</p>
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
         * Controls if the renderable has vertex morphing targets, false by default.
         *
         * <p>This is required to enable GPU morphing for up to 4 attributes. The attached VertexBuffer
         * must provide data in the appropriate VertexAttribute slots (<code>MORPH_POSITION_0</code> etc).</p>
         *
         * <p>See also {@link RenderableManager#setMorphWeights}, which can be called on a per-frame basis
         * to advance the animation.</p>
         */
        @NonNull
        public Builder morphing(boolean enabled) {
            nBuilderMorphing(mNativeBuilder, enabled);
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
     * <p>This is specified using a 4-tuple, one float per morph target. If the renderable has fewer
     * than 4 morph targets, then clients should fill the unused components with zeroes.</p>
     *
     * <p>The renderable must be built with morphing enabled.</p>
     *
     * @see Builder#morphing
     */
    public void setMorphWeights(@EntityInstance int i, @NonNull @Size(min = 4) float[] weights) {
        nSetMorphWeights(mNativeObject, i, weights);
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
     * Changes whether or not frustum culling is on.
     *
     * @see Builder#culling
     */
    public void setCulling(@EntityInstance int i, boolean enabled) {
        nSetCulling(mNativeObject, i, enabled);
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
     * Changes the geometry for the given primitive.
     *
     * @see Builder#geometry Builder.geometry
     */
    public void setGeometryAt(@EntityInstance int i, @IntRange(from = 0) int primitiveIndex,
            @NonNull PrimitiveType type, @IntRange(from = 0) int offset, @IntRange(from = 0) int count) {
        nSetGeometryAt(mNativeObject, i, primitiveIndex, type.getValue(), offset, count);
    }

    /**
     * Changes the ordering index for blended primitives that all live at the same Z value.
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
     * Retrieves the set of enabled attribute slots in the given primitive's VertexBuffer.
     */
    public Set<VertexBuffer.VertexAttribute> getEnabledAttributesAt(@EntityInstance int i, @IntRange(from = 0) int primitiveIndex) {
        int bitSet = nGetEnabledAttributesAt(mNativeObject, i, primitiveIndex);
        Set<VertexBuffer.VertexAttribute> requiredAttributes = EnumSet.noneOf(VertexBuffer.VertexAttribute.class);
        VertexBuffer.VertexAttribute[] values = VertexBuffer.VertexAttribute.values();
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
    private static native void nBuilderMaterial(long nativeBuilder, int index, long nativeMaterialInstance);
    private static native void nBuilderBlendOrder(long nativeBuilder, int index, int blendOrder);
    private static native void nBuilderBoundingBox(long nativeBuilder, float cx, float cy, float cz, float ex, float ey, float ez);
    private static native void nBuilderLayerMask(long nativeBuilder, int select, int value);
    private static native void nBuilderPriority(long nativeBuilder, int priority);
    private static native void nBuilderCulling(long nativeBuilder, boolean enabled);
    private static native void nBuilderCastShadows(long nativeBuilder, boolean enabled);
    private static native void nBuilderReceiveShadows(long nativeBuilder, boolean enabled);
    private static native void nBuilderScreenSpaceContactShadows(long nativeBuilder, boolean enabled);
    private static native void nBuilderSkinning(long nativeBuilder, int boneCount);
    private static native int nBuilderSkinningBones(long nativeBuilder, int boneCount, Buffer bones, int remaining);
    private static native void nBuilderMorphing(long nativeBuilder, boolean enabled);

    private static native int nSetBonesAsMatrices(long nativeObject, int i, Buffer matrices, int remaining, int boneCount, int offset);
    private static native int nSetBonesAsQuaternions(long nativeObject, int i, Buffer quaternions, int remaining, int boneCount, int offset);
    private static native void nSetMorphWeights(long nativeObject, int instance, float[] weights);
    private static native void nSetAxisAlignedBoundingBox(long nativeRenderableManager, int i, float cx, float cy, float cz, float ex, float ey, float ez);
    private static native void nSetLayerMask(long nativeRenderableManager, int i, int select, int value);
    private static native void nSetPriority(long nativeRenderableManager, int i, int priority);
    private static native void nSetCulling(long nativeRenderableManager, int i, boolean enabled);
    private static native void nSetCastShadows(long nativeRenderableManager, int i, boolean enabled);
    private static native void nSetReceiveShadows(long nativeRenderableManager, int i, boolean enabled);
    private static native void nSetScreenSpaceContactShadows(long nativeRenderableManager, int i, boolean enabled);
    private static native boolean nIsShadowCaster(long nativeRenderableManager, int i);
    private static native boolean nIsShadowReceiver(long nativeRenderableManager, int i);
    private static native void nGetAxisAlignedBoundingBox(long nativeRenderableManager, int i, float[] center, float[] halfExtent);
    private static native int nGetPrimitiveCount(long nativeRenderableManager, int i);
    private static native void nSetMaterialInstanceAt(long nativeRenderableManager, int i, int primitiveIndex, long nativeMaterialInstance);
    private static native long nGetMaterialInstanceAt(long nativeRenderableManager, int i, int primitiveIndex);
    private static native long nGetMaterialAt(long nativeRenderableManager, int i, int primitiveIndex);
    private static native void nSetGeometryAt(long nativeRenderableManager, int i, int primitiveIndex, int primitiveType, long nativeVertexBuffer, long nativeIndexBuffer, int offset, int count);
    private static native void nSetGeometryAt(long nativeRenderableManager, int i, int primitiveIndex, int primitiveType, int offset, int count);
    private static native void nSetBlendOrderAt(long nativeRenderableManager, int i, int primitiveIndex, int blendOrder);
    private static native int nGetEnabledAttributesAt(long nativeRenderableManager, int i, int primitiveIndex);
}
