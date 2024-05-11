/*
 * Copyright (C) 2020 The Android Open Source Project
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

import java.nio.Buffer;

/**
 * Helper used to populate <code>TANGENTS</code> buffers.
 */
public class SurfaceOrientation {
    private long mNativeObject;

    private SurfaceOrientation(long nativeSurfaceOrientation) {
        mNativeObject = nativeSurfaceOrientation;
    }

    /**
     * Constructs an immutable surface orientation helper.
     *
     * At a minimum, clients must supply a vertex count.
     * They can supply data in any of the following combinations:
     *
     * <ol>
     * <li>normals only (not recommended)</li>
     * <li>normals + tangents (sign of W determines bitangent orientation)</li>
     * <li>normals + uvs + positions + indices</li>
     * <li>positions + indices</li>
     * </ol>
     *
     * Additionally, the client-side data has the following type constraints:
     *
     * <ol>
     * <li>Normals must be float3</li>
     * <li>Tangents must be float4</li>
     * <li>UVs must be float2</li>
     * <li>Positions must be float3</li>
     * <li>Triangles must be uint3 or ushort3</li>
     * </ol>
     */
    public static class Builder {
        private int mVertexCount;
        private int mTriangleCount;

        private Buffer mNormals;
        private int mNormalsStride;

        private Buffer mTangents;
        private int mTangentsStride;

        private Buffer mTexCoords;
        private int mTexCoordsStride;

        private Buffer mPositions;
        private int mPositionsStride;

        private Buffer mTrianglesUint16;
        private Buffer mTrianglesUint32;

        @NonNull
        public Builder vertexCount(@IntRange(from = 1) int vertexCount) {
            mVertexCount = vertexCount;
            return this;
        }

        @NonNull
        public Builder normals(@NonNull Buffer buffer) {
            mNormals = buffer;
            mNormalsStride = 0;
            return this;
        }

        @NonNull
        public Builder tangents(@NonNull Buffer buffer) {
            mTangents = buffer;
            mTangentsStride = 0;
            return this;
        }

        @NonNull
        public Builder uvs(@NonNull Buffer buffer) {
            mTexCoords = buffer;
            mTexCoordsStride = 0;
            return this;
        }

        @NonNull
        public Builder positions(@NonNull Buffer buffer) {
            mPositions = buffer;
            mPositionsStride = 0;
            return this;
        }

        @NonNull
        public Builder triangleCount(int triangleCount) {
            mTriangleCount = triangleCount;
            return this;
        }

        @NonNull
        public Builder triangles_uint16(@NonNull Buffer buffer) {
            mTrianglesUint16 = buffer;
            return this;
        }

        @NonNull
        public Builder triangles_uint32(@NonNull Buffer buffer) {
            mTrianglesUint32 = buffer;
            return this;
        }

        /**
         * Consumes the input data, produces quaternions, and destroys the native builder.
         */
        @NonNull
        public SurfaceOrientation build() {

            // The C++ Builder API specifies that the pointers are consumed during build(), not
            // during the individual daisy-chain methods. Therefore we need to retain the Java
            // buffers until this point in the code.

            long builder = nCreateBuilder();
            nBuilderVertexCount(builder, mVertexCount);
            nBuilderTriangleCount(builder, mTriangleCount);

            if (mNormals != null) {
                nBuilderNormals(builder, mNormals, mNormals.remaining(), mNormalsStride);
            }

            if (mTangents != null) {
                nBuilderTangents(builder, mTangents, mTangents.remaining(), mTangentsStride);
            }

            if (mTexCoords != null) {
                nBuilderUVs(builder, mTexCoords, mTexCoords.remaining(), mTexCoordsStride);
            }

            if (mPositions != null) {
                nBuilderPositions(builder, mPositions, mPositions.remaining(), mPositionsStride);
            }

            if (mTrianglesUint16 != null) {
                nBuilderTriangles16(builder, mTrianglesUint16, mTrianglesUint16.remaining());
            }

            if (mTrianglesUint32 != null) {
                nBuilderTriangles32(builder, mTrianglesUint32, mTrianglesUint32.remaining());
            }

            long nativeSurfaceOrientation = nBuilderBuild(builder);
            nDestroyBuilder(builder);
            if (nativeSurfaceOrientation == 0) {
                throw new IllegalStateException("Could not create SurfaceOrientation");
            }
            return new SurfaceOrientation(nativeSurfaceOrientation);
        }
    }

    public long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed SurfaceOrientation");
        }
        return mNativeObject;
    }

    @IntRange(from = 0)
    public int getVertexCount() {
        return nGetVertexCount(mNativeObject);
    }

    @NonNull
    public void getQuatsAsFloat(@NonNull Buffer buffer) {
        nGetQuatsAsFloat(mNativeObject, buffer, buffer.remaining());
    }

    @NonNull
    public void getQuatsAsHalf(@NonNull Buffer buffer) {
        nGetQuatsAsHalf(mNativeObject, buffer, buffer.remaining());
    }

    @NonNull
    public void getQuatsAsShort(@NonNull Buffer buffer) {
        nGetQuatsAsShort(mNativeObject, buffer, buffer.remaining());
    }

    public void destroy() {
        nDestroy(mNativeObject);
        mNativeObject = 0;
    }

    private static native long nCreateBuilder();
    private static native void nDestroyBuilder(long nativeBuilder);

    private static native void nBuilderVertexCount(long nativeBuilder, int vertexCount);
    private static native void nBuilderNormals(long nativeBuilder, Buffer buffer, int remaining, int stride);
    private static native void nBuilderTangents(long nativeBuilder, Buffer buffer, int remaining, int stride);
    private static native void nBuilderUVs(long nativeBuilder, Buffer buffer, int remaining, int stride);
    private static native void nBuilderPositions(long nativeBuilder, Buffer buffer, int remaining, int stride);
    private static native void nBuilderTriangleCount(long nativeBuilder, int triangleCount);
    private static native void nBuilderTriangles16(long nativeBuilder, Buffer buffer, int remaining);
    private static native void nBuilderTriangles32(long nativeBuilder, Buffer buffer, int remaining);
    private static native long nBuilderBuild(long nativeBuilder);

    private static native int nGetVertexCount(long nativeSurfaceOrientation);
    private static native void nGetQuatsAsFloat(long nativeSurfaceOrientation, Buffer buffer, int remaining);
    private static native void nGetQuatsAsHalf(long nativeSurfaceOrientation, Buffer buffer, int remaining);
    private static native void nGetQuatsAsShort(long nativeSurfaceOrientation, Buffer buffer, int remaining);
    private static native void nDestroy(long nativeSurfaceOrientation);
}
