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

    static {
        System.loadLibrary("filamat-jni");
    }

    public enum Shading {
        UNLIT,                  // no lighting applied, emissive possible
        LIT,                    // default, standard lighting
        SUBSURFACE,             // subsurface lighting model
        CLOTH,                  // cloth lighting model
        SPECULAR_GLOSSINESS     // legacy lighting model
    }

    public enum Interpolation {
        SMOOTH,                 // default, smooth interpolation
        FLAT                    // flat interpolation
    }

    public enum UniformType {
        BOOL,
        BOOL2,
        BOOL3,
        BOOL4,
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        INT,
        INT2,
        INT3,
        INT4,
        UINT,
        UINT2,
        UINT3,
        UINT4,
        MAT3,
        MAT4
    }

    public enum SamplerType {
        SAMPLER_2D,             // 2D texture
        SAMPLER_CUBEMAP,        // Cube map texture
        SAMPLER_EXTERNAL,       // External texture
    }

    public enum SamplerFormat {
        INT,
        UINT,
        FLOAT,
        SHADOW
    }

    public enum SamplerPrecision {
        LOW,
        MEDIUM,
        HIGH,
        DEFAULT
    }

    public enum Variable {
        CUSTOM0,
        CUSTOM1,
        CUSTOM2,
        CUSTOM3
    }

    public enum VertexAttribute {
        POSITION,               // XYZ position (float3)
        TANGENTS,               // tangent, bitangent and normal, encoded as a quaternion (4 floats or half floats)
        COLOR,                  // vertex color (float4)
        UV0,                    // texture coordinates (float2)
        UV1,                    // texture coordinates (float2)
        BONE_INDICES,           // indices of 4 bones (uvec4)
        BONE_WEIGHTS            // weights of the 4 bones (normalized float4)
    }

    public enum BlendingMode {
        OPAQUE,                 // material is opaque
        TRANSPARENT,            // material is transparent and color is alpha-pre-multiplied,
                                // affects diffuse lighting only
        ADD,                    // material is additive (e.g.: hologram)
        MASKED,                 // material is masked (i.e. alpha tested)
        FADE                    // material is transparent and color is alpha-pre-multiplied,
                                // affects specular lighting
    }

    public enum VertexDomain {
        OBJECT,                 // vertices are in object space, default
        WORLD,                  // vertices are in world space
        VIEW,                   // vertices are in view space
        DEVICE                  // vertices are in normalized device space
    }

    public enum CullingMode {
        NONE,
        FRONT,
        BACK,
        FRONT_AND_BACK
    }

    public enum TransparencyMode {
        DEFAULT,                // the transparent object is drawn honoring the raster state
        TWO_PASSES_ONE_SIDE,    // the transparent object is first drawn in the depth buffer,
                                // then in the color buffer, honoring the culling mode, but
                                // ignoring the depth test function
        TWO_PASSES_TWO_SIDES    // the transparent object is drawn twice in the color buffer,
                                // first with back faces only, then with front faces; the culling
                                // mode is ignored. Can be combined with two-sided lighting
    }

    public enum Platform {
        DESKTOP,
        MOBILE,
        ALL
    }

    public enum TargetApi {
        ALL,
        OPENGL,
        VULKAN,
    }

    public enum Optimization {
        NONE,
        PREPROCESSOR,
        SIZE,
        PERFORMANCE
    }

    public static void init() {
        nMaterialBuilderInit();
    }

    public static void shutdown() {
        nMaterialBuilderShutdown();
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
    public MaterialBuilder interpolation(@NonNull Interpolation interpolation) {
        nMaterialBuilderInterpolation(mNativeObject, interpolation.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder uniformParameter(@NonNull UniformType type, String name) {
        nMaterialBuilderUniformParameter(mNativeObject, type.ordinal(), name);
        return this;
    }

    @NonNull
    public MaterialBuilder uniformParameterArray(@NonNull UniformType type, int size, String name) {
        nMaterialBuilderUniformParameterArray(mNativeObject, type.ordinal(), size, name);
        return this;
    }

    @NonNull
    public MaterialBuilder samplerParameter(@NonNull SamplerType type, SamplerFormat format,
            SamplerPrecision precision, String name) {
        nMaterialBuilderSamplerParameter(
                mNativeObject, type.ordinal(), format.ordinal(), precision.ordinal(), name);
        return this;
    }

    @NonNull
    public MaterialBuilder variable(@NonNull Variable variable, String name) {
        nMaterialBuilderVariable(mNativeObject, variable.ordinal(), name);
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
    public MaterialBuilder blending(@NonNull BlendingMode mode) {
        nMaterialBuilderBlending(mNativeObject, mode.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder postLightingBlending(@NonNull BlendingMode mode) {
        nMaterialBuilderPostLightingBlending(mNativeObject, mode.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder vertexDomain(@NonNull VertexDomain vertexDomain) {
        nMaterialBuilderVertexDomain(mNativeObject, vertexDomain.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder culling(@NonNull CullingMode mode) {
        nMaterialBuilderCulling(mNativeObject, mode.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder colorWrite(boolean enable) {
        nMaterialBuilderColorWrite(mNativeObject, enable);
        return this;
    }

    @NonNull
    public MaterialBuilder depthWrite(boolean enable) {
        nMaterialBuilderDepthWrite(mNativeObject, enable);
        return this;
    }

    @NonNull
    public MaterialBuilder depthCulling(boolean enable) {
        nMaterialBuilderDepthCulling(mNativeObject, enable);
        return this;
    }

    @NonNull
    public MaterialBuilder doubleSided(boolean doubleSided) {
        nMaterialBuilderDoubleSided(mNativeObject, doubleSided);
        return this;
    }

    @NonNull
    public MaterialBuilder maskThreshold(float threshold) {
        nMaterialBuilderMaskThreshold(mNativeObject, threshold);
        return this;
    }

    @NonNull
    public MaterialBuilder shadowMultiplier(boolean shadowMultiplier) {
        nMaterialBuilderShadowMultiplier(mNativeObject, shadowMultiplier);
        return this;
    }

    @NonNull
    public MaterialBuilder curvatureToRoughness(boolean curvatureToRoughness) {
        nMaterialBuilderCurvatureToRoughness(mNativeObject, curvatureToRoughness);
        return this;
    }

    @NonNull
    public MaterialBuilder limitOverInterpolation(boolean limitOverInterpolation) {
        nMaterialBuilderLimitOverInterpolation(mNativeObject, limitOverInterpolation);
        return this;
    }

    @NonNull
    public MaterialBuilder clearCoatIorChange(boolean clearCoatIorChange) {
        nMaterialBuilderClearCoatIorChange(mNativeObject, clearCoatIorChange);
        return this;
    }

    @NonNull
    public MaterialBuilder flipUV(boolean flipUV) {
        nMaterialBuilderFlipUV(mNativeObject, flipUV);
        return this;
    }

    @NonNull
    public MaterialBuilder transparencyMode(@NonNull TransparencyMode mode) {
        nMaterialBuilderTransparencyMode(mNativeObject, mode.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder platform(@NonNull Platform platform) {
        nMaterialBuilderPlatform(mNativeObject, platform.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder targetApi(@NonNull TargetApi api) {
        nMaterialBuilderTargetApi(mNativeObject, api.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder optimization(@NonNull Optimization optimization) {
        nMaterialBuilderOptimization(mNativeObject, optimization.ordinal());
        return this;
    }

    @NonNull
    public MaterialBuilder variantFilter(byte variantFilter) {
        nMaterialBuilderVariantFilter(mNativeObject, variantFilter);
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

    private static native void nMaterialBuilderInit();
    private static native void nMaterialBuilderShutdown();

    private static native long nCreateMaterialBuilder();
    private static native void nDestroyMaterialBuilder(long nativeBuilder);

    private static native long nBuilderBuild(long nativeBuilder);
    private static native byte[] nGetPackageBytes(long nativePackage);
    private static native boolean nGetPackageIsValid(long nativePackage);
    private static native void nDestroyPackage(long nativePackage);

    private static native void nMaterialBuilderName(long nativeBuilder, String name);
    private static native void nMaterialBuilderShading(long nativeBuilder, int shading);
    private static native void nMaterialBuilderInterpolation(long nativeBuilder, int interpolation);
    private static native void nMaterialBuilderUniformParameter(long nativeBuilder, int type,
            String name);
    private static native void nMaterialBuilderUniformParameterArray(long nativeBuilder, int type,
            int size, String name);
    private static native void nMaterialBuilderSamplerParameter(long nativeBuilder, int type,
            int format, int precision, String name);
    private static native void nMaterialBuilderVariable(long nativeBuilder, int variable,
            String name);
    private static native void nMaterialBuilderRequire(long nativeBuilder, int attribute);
    private static native void nMaterialBuilderMaterial(long nativeBuilder, String code);
    private static native void nMaterialBuilderMaterialVertex(long nativeBuilder, String code);
    private static native void nMaterialBuilderBlending(long nativeBuilder, int mode);
    private static native void nMaterialBuilderPostLightingBlending(long nativeBuilder, int mode);
    private static native void nMaterialBuilderVertexDomain(long nativeBuilder, int vertexDomain);
    private static native void nMaterialBuilderCulling(long nativeBuilder, int mode);
    private static native void nMaterialBuilderColorWrite(long nativeBuilder, boolean enable);
    private static native void nMaterialBuilderDepthWrite(long nativeBuilder, boolean enable);
    private static native void nMaterialBuilderDepthCulling(long nativeBuilder, boolean enable);
    private static native void nMaterialBuilderDoubleSided(long nativeBuilder, boolean doubleSided);
    private static native void nMaterialBuilderMaskThreshold(long nativeBuilder, float mode);

    private static native void nMaterialBuilderShadowMultiplier(long mNativeObject,
            boolean shadowMultiplier);
    private static native void nMaterialBuilderCurvatureToRoughness(long mNativeObject,
            boolean curvatureToRoughness);
    private static native void nMaterialBuilderLimitOverInterpolation(long mNativeObject,
            boolean limitOverInterpolation);
    private static native void nMaterialBuilderClearCoatIorChange(long mNativeObject,
            boolean clearCoatIorChange);
    private static native void nMaterialBuilderFlipUV(long nativeBuilder, boolean flipUV);
    private static native void nMaterialBuilderTransparencyMode(long nativeBuilder, int mode);
    private static native void nMaterialBuilderPlatform(long nativeBuilder, int platform);
    private static native void nMaterialBuilderTargetApi(long nativeBuilder, int api);
    private static native void nMaterialBuilderOptimization(long nativeBuilder, int optimization);
    private static native void nMaterialBuilderVariantFilter(long nativeBuilder,
            byte variantFilter);
}
