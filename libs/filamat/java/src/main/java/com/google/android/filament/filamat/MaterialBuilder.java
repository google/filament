/*
 * Copyright (C) 2019 The Android Open Source Project
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

package com.google.android.filament.filamat;

import android.support.annotation.NonNull;
import java.nio.ByteBuffer;

public class MaterialBuilder {

    @SuppressWarnings({"FieldCanBeLocal", "UnusedDeclaration"})
    // Keep to finalize native resources
    private final BuilderFinalizer mFinalizer;
    private final long mNativeObject;

    public enum VertexAttribute {
        POSITION,               // XYZ position (float3)
        TANGENTS,               // tangent, bitangent and normal, encoded as a quaternion (4 floats or half floats)
        COLOR,                  // vertex color (float4)
        UV0,                    // texture coordinates (float2)
        UV1,                    // texture coordinates (float2)
        BONE_INDICES,           // indices of 4 bones (uvec4)
        BONE_WEIGHTS            // weights of the 4 bones (normalized float4)
    }

    public enum Shading {
        UNLIT,                  // no lighting applied, emissive possible
        LIT,                    // default, standard lighting
        SUBSURFACE,             // subsurface lighting model
        CLOTH,                  // cloth lighting model
    }

    public enum Platform {
        DESKTOP,
        MOBILE,
        ALL
    }

    public MaterialBuilder() {
        mNativeObject = nCreateMaterialBuilder();
        mFinalizer = new BuilderFinalizer(mNativeObject);
    }

    @NonNull
    public MaterialBuilder name(@NonNull String name) {
        nMaterialBuilderName(mNativeObject, name);
        return this;
    }

    @NonNull
    public MaterialBuilder shading(@NonNull Shading shading) {
        nMaterialBuilderShading(mNativeObject, shading.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder require(@NonNull VertexAttribute attribute) {
        nMaterialBuilderRequire(mNativeObject, attribute.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder material(@NonNull String code) {
        nMaterialBuilderMaterial(mNativeObject, code);
        return this;
    }

    @NonNull
    public MaterialBuilder materialVertex(@NonNull String code) {
        nMaterialBuilderMaterialVertex(mNativeObject, code);
        return this;
    }

    @NonNull
    public MaterialBuilder colorWrite(boolean enable) {
        nMaterialBuilderColorWrite(mNativeObject, enable);
        return this;
    }

    @NonNull
    public MaterialBuilder platform(@NonNull Platform platform) {
        nMaterialBuilderPlatform(mNativeObject, platform.ordinal());
        return this;
    }

    @NonNull
    public MaterialPackage build() {
        long nativePackage = nBuilderBuild(mNativeObject);
        byte[] data = nGetPackageBytes(nativePackage);
        MaterialPackage result =
                new MaterialPackage(ByteBuffer.wrap(data), nGetPackageIsValid(nativePackage));
        nDestroyPackage(nativePackage);
        return result;
    }

    private static class BuilderFinalizer {
        private final long mNativeObject;

        BuilderFinalizer(long nativeObject) {
            mNativeObject = nativeObject;
        }

        @Override
        public void finalize() {
            try {
                super.finalize();
            } catch (Throwable t) { // Ignore
            } finally {
                nDestroyMaterialBuilder(mNativeObject);
            }
        }
    }

    private static native long nCreateMaterialBuilder();
    private static native void nDestroyMaterialBuilder(long nativeBuilder);

    private static native long nBuilderBuild(long nativeBuilder);
    private static native byte[] nGetPackageBytes(long nativePackage);
    private static native boolean nGetPackageIsValid(long nativePackage);
    private static native void nDestroyPackage(long nativePackage);

    private static native void nMaterialBuilderName(long nativeBuilder, String name);
    private static native void nMaterialBuilderShading(long nativeBuilder, int shading);
    private static native void nMaterialBuilderRequire(long nativeBuilder, int attribute);
    private static native void nMaterialBuilderMaterial(long nativeBuilder, String code);
    private static native void nMaterialBuilderMaterialVertex(long nativeBuilder, String code);
    private static native void nMaterialBuilderColorWrite(long nativeBuilder, boolean enable);
    private static native void nMaterialBuilderPlatform(long nativeBuilder, int platform);
}
