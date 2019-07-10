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

import android.support.annotation.IntRange;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import java.nio.Buffer;
import java.nio.BufferOverflowException;
import java.util.Collections;
import java.util.EnumSet;
import java.util.Set;

public class RenderableManager {
    private static final String LOG_TAG = "Filament";
    private long mNativeObject;

    RenderableManager(long nativeRenderableManager) {
        mNativeObject = nativeRenderableManager;
    }

    public boolean hasComponent(@Entity int entity) {
        return nHasComponent(mNativeObject, entity);
    }

    @EntityInstance
    public int getInstance(@Entity int entity) {
        return nGetInstance(mNativeObject, entity);
    }

    public void destroy(@Entity int entity) {
        nDestroy(mNativeObject, entity);
    }

    public enum PrimitiveType {
        POINTS(0),
        LINES(1),
        TRIANGLES(4);

        private final int mType;
        PrimitiveType(int value) { mType = value; }
        int getValue() { return mType; }
    }

    public static class Builder {
        @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"}) // Keep to finalize native resources
        private final BuilderFinalizer mFinalizer;
        private final long mNativeBuilder;

        public Builder(@IntRange(from = 1) int count) {
            mNativeBuilder = nCreateBuilder(count);
            mFinalizer = new BuilderFinalizer(mNativeBuilder);
        }

        @NonNull
        public Builder geometry(@IntRange(from = 0) int index, @NonNull PrimitiveType type,
                @NonNull VertexBuffer vertices, @NonNull IndexBuffer indices) {
            nBuilderGeometry(mNativeBuilder, index, type.getValue(),
                    vertices.getNativeObject(), indices.getNativeObject());
            return this;
        }

        @NonNull
        public Builder geometry(@IntRange(from = 0) int index, @NonNull PrimitiveType type,
                @NonNull VertexBuffer vertices, @NonNull IndexBuffer indices,
                @IntRange(from = 0) int offset, @IntRange(from = 0) int count) {
            nBuilderGeometry(mNativeBuilder, index, type.getValue(), vertices.getNativeObject(),
                    indices.getNativeObject(), offset, count);
            return this;
        }

        @NonNull
        public Builder geometry(@IntRange(from = 0) int index, @NonNull PrimitiveType type,
                @NonNull VertexBuffer vertices, @NonNull IndexBuffer indices,
                @IntRange(from = 0) int offset, @IntRange(from = 0) int minIndex,
                @IntRange(from = 0) int maxIndex, @IntRange(from = 0) int count) {
            nBuilderGeometry(mNativeBuilder, index, type.getValue(), vertices.getNativeObject(),
                    indices.getNativeObject(), offset, minIndex, maxIndex, count);
            return this;
        }

        @NonNull
        public Builder material(@IntRange(from = 0) int index, @NonNull MaterialInstance material) {
            nBuilderMaterial(mNativeBuilder, index, material.getNativeObject());
            return this;
        }

        // Sets an ordering index for blended primitives that all live at the same Z value.
        @NonNull
        public Builder blendOrder(@IntRange(from = 0) int index,
                @IntRange(from = 0, to = 32767) int blendOrder) {
            nBuilderBlendOrder(mNativeBuilder, index, blendOrder);
            return this;
        }

        @NonNull
        public Builder boundingBox(@NonNull Box aabb) {
            nBuilderBoundingBox(mNativeBuilder,
                    aabb.getCenter()[0], aabb.getCenter()[1], aabb.getCenter()[2],
                    aabb.getHalfExtent()[0], aabb.getHalfExtent()[1], aabb.getHalfExtent()[2]);
            return this;
        }

        @NonNull
        public Builder layerMask(@IntRange(from = 0, to = 255) int select,
                @IntRange(from = 0, to = 255) int value) {
            nBuilderLayerMask(mNativeBuilder, select & 0xFF, value & 0xFF);
            return this;
        }

        @NonNull
        public Builder priority(@IntRange(from = 0, to = 7) int priority) {
            nBuilderPriority(mNativeBuilder, priority);
            return this;
        }

        @NonNull
        public Builder culling(boolean enabled) {
            nBuilderCulling(mNativeBuilder, enabled);
            return this;
        }

        @NonNull
        public Builder castShadows(boolean enabled) {
            nBuilderCastShadows(mNativeBuilder, enabled);
            return this;
        }

        @NonNull
        public Builder receiveShadows(boolean enabled) {
            nBuilderReceiveShadows(mNativeBuilder, enabled);
            return this;
        }

        @NonNull
        public Builder skinning(@IntRange(from = 0, to = 255) int boneCount) {
            nBuilderSkinning(mNativeBuilder, boneCount);
            return this;
        }

        /**
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

        @NonNull
        public Builder morphing(boolean enabled) {
            nBuilderMorphing(mNativeBuilder, enabled);
            return this;
        }

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
     * Sets the transforms associated to each bone of a Renderable
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
     * Sets the transforms associated to each bone of a Renderable
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

    public void setMorphWeights(@EntityInstance int i, float[] weights) {
        nSetMorphWeights(mNativeObject, i, weights);
    }

    public void setAxisAlignedBoundingBox(@EntityInstance int i, @NonNull Box aabb) {
        nSetAxisAlignedBoundingBox(mNativeObject, i,
                aabb.getCenter()[0], aabb.getCenter()[1], aabb.getCenter()[2],
                aabb.getHalfExtent()[0], aabb.getHalfExtent()[1], aabb.getHalfExtent()[2]);
    }

    public void setLayerMask(@EntityInstance int i, @IntRange(from = 0, to = 255) int select,
            @IntRange(from = 0, to = 255) int value) {
        nSetLayerMask(mNativeObject, i, select, value);
    }

    public void setPriority(@EntityInstance int i, @IntRange(from = 0, to = 7) int priority) {
        nSetPriority(mNativeObject, i, priority);
    }

    public void setCastShadows(@EntityInstance int i, boolean enabled) {
        nSetCastShadows(mNativeObject, i, enabled);
    }

    public void setReceiveShadows(@EntityInstance int i, boolean enabled) {
        nSetReceiveShadows(mNativeObject, i, enabled);
    }

    public boolean isShadowCaster(@EntityInstance int i) {
        return nIsShadowCaster(mNativeObject, i);
    }

    public boolean isShadowReceiver(@EntityInstance int i) {
        return nIsShadowReceiver(mNativeObject, i);
    }

    @NonNull
    public Box getAxisAlignedBoundingBox(@EntityInstance int i, @Nullable Box out) {
        if (out == null) out = new Box();
        nGetAxisAlignedBoundingBox(mNativeObject, i, out.getCenter(), out.getHalfExtent());
        return out;
    }

    @IntRange(from = 0)
    public int getPrimitiveCount(@EntityInstance int i) {
        return nGetPrimitiveCount(mNativeObject, i);
    }

    // set/change the material of a given render primitive
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

    // creates a MaterialInstance Java wrapper object for a particular material instance
    public @NonNull MaterialInstance getMaterialInstanceAt(@EntityInstance int i,
            @IntRange(from = 0) int primitiveIndex) {
        long nativeMatInstance = nGetMaterialInstanceAt(mNativeObject, i, primitiveIndex);
        long nativeMaterial = nGetMaterialAt(mNativeObject, i, primitiveIndex);
        return new MaterialInstance(nativeMaterial, nativeMatInstance);
    }

    // set/change the geometry (vertex/index buffers) of a given primitive
    public void setGeometryAt(@EntityInstance int i, @IntRange(from = 0) int primitiveIndex,
            @NonNull PrimitiveType type, @NonNull VertexBuffer vertices,
            @NonNull IndexBuffer indices, @IntRange(from = 0) int offset,
            @IntRange(from = 0) int count) {
        nSetGeometryAt(mNativeObject, i, primitiveIndex, type.getValue(), vertices.getNativeObject(), indices.getNativeObject(), offset, count);
    }

    // set/change the geometry (vertex/index buffers) of a given primitive
    public void setGeometryAt(@EntityInstance int i, @IntRange(from = 0) int primitiveIndex,
            @NonNull PrimitiveType type, @NonNull VertexBuffer vertices,
            @NonNull IndexBuffer indices) {
        nSetGeometryAt(mNativeObject, i, primitiveIndex, type.getValue(), vertices.getNativeObject(), indices.getNativeObject(),
                0, indices.getIndexCount());
    }

    // set/change the offset/count in the currently set index buffer of a given primitive
    public void setGeometryAt(@EntityInstance int i, @IntRange(from = 0) int primitiveIndex,
            @NonNull PrimitiveType type, @IntRange(from = 0) int offset, @IntRange(from = 0) int count) {
        nSetGeometryAt(mNativeObject, i, primitiveIndex, type.getValue(), offset, count);
    }

    public void setBlendOrderAt(@EntityInstance int i, @IntRange(from = 0) int primitiveIndex,
            @IntRange(from = 0, to = 65535) int blendOrder) {
        nSetBlendOrderAt(mNativeObject, i, primitiveIndex, blendOrder);
    }

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
    private static native void nBuilderSkinning(long nativeBuilder, int boneCount);
    private static native int nBuilderSkinningBones(long nativeBuilder, int boneCount, Buffer bones, int remaining);
    private static native void nBuilderMorphing(long nativeBuilder, boolean enabled);

    private static native int nSetBonesAsMatrices(long nativeObject, int i, Buffer matrices, int remaining, int boneCount, int offset);
    private static native int nSetBonesAsQuaternions(long nativeObject, int i, Buffer quaternions, int remaining, int boneCount, int offset);
    private static native void nSetMorphWeights(long nativeObject, int instance, float[] weights);
    private static native void nSetAxisAlignedBoundingBox(long nativeRenderableManager, int i, float cx, float cy, float cz, float ex, float ey, float ez);
    private static native void nSetLayerMask(long nativeRenderableManager, int i, int select, int value);
    private static native void nSetPriority(long nativeRenderableManager, int i, int priority);
    private static native void nSetCastShadows(long nativeRenderableManager, int i, boolean enabled);
    private static native void nSetReceiveShadows(long nativeRenderableManager, int i, boolean enabled);
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
