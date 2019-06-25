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
import android.support.annotation.Size;
import com.google.android.filament.proguard.UsedByNative;

import java.nio.Buffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;

public class Material {
    private long mNativeObject;
    private final MaterialInstance mDefaultInstance;

    private Set<VertexBuffer.VertexAttribute> mRequiredAttributes;

    public enum Shading {
        UNLIT,
        LIT,
        SUBSURFACE,
        CLOTH,
        SPECULAR_GLOSSINESS
    }

    public enum Interpolation {
        SMOOTH,
        FLAT
    }

    public enum BlendingMode {
        OPAQUE,
        TRANSPARENT,
        ADD,
        MODULATE,
        MASKED,
        FADE
    }

    public enum VertexDomain {
        OBJECT,
        WORLD,
        VIEW,
        DEVICE
    }

    public enum CullingMode {
        NONE,
        FRONT,
        BACK,
        FRONT_AND_BACK
    }

    @UsedByNative("Material.cpp")
    public static class Parameter {
        public enum Type {
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
            MAT4,
            SAMPLER_2D,
            SAMPLER_CUBEMAP,
            SAMPLER_EXTERNAL
        }

        public enum Precision {
            LOW,
            MEDIUM,
            HIGH,
            DEFAULT
        }

        @SuppressWarnings("unused")
        @UsedByNative("Material.cpp")
        private static final int SAMPLER_OFFSET = Type.MAT4.ordinal() + 1;

        @NonNull
        public final String name;
        @NonNull
        public final Type type;
        @NonNull
        public final Precision precision;
        @IntRange(from = 1)
        public final int count;

        private Parameter(@NonNull String name, @NonNull Type type, @NonNull Precision precision,
                @IntRange(from = 1) int count) {
            this.name = name;
            this.type = type;
            this.precision = precision;
            this.count = count;
        }

        @SuppressWarnings("unused")
        @UsedByNative("Material.cpp")
        private static void add(@NonNull List<Parameter> parameters, @NonNull String name,
                @IntRange(from = 0) int type, @IntRange(from = 0) int precision,
                @IntRange(from = 1) int count) {
            parameters.add(
                    new Parameter(name, Type.values()[type], Precision.values()[precision], count));
        }
    }

    Material(long nativeMaterial) {
        mNativeObject = nativeMaterial;
        long nativeDefaultInstance = nGetDefaultInstance(nativeMaterial);
        mDefaultInstance = new MaterialInstance(this, nativeDefaultInstance);
    }

    public static class Builder {
        private Buffer mBuffer;
        private int mSize;

        @NonNull
        public Builder payload(@NonNull Buffer buffer, @IntRange(from = 0) int size) {
            mBuffer = buffer;
            mSize = size;
            return this;
        }

        @NonNull
        public Material build(@NonNull Engine engine) {
            long nativeMaterial = nBuilderBuild(engine.getNativeObject(), mBuffer, mSize);
            if (nativeMaterial == 0) throw new IllegalStateException("Couldn't create Material");
            return new Material(nativeMaterial);
        }
    }

    @NonNull
    public MaterialInstance createInstance() {
        long nativeInstance = nCreateInstance(getNativeObject());
        if (nativeInstance == 0) throw new IllegalStateException("Couldn't create MaterialInstance");
        return new MaterialInstance(this, nativeInstance);
    }

    @NonNull
    public MaterialInstance getDefaultInstance() {
        return mDefaultInstance;
    }

    public String getName() {
        return nGetName(getNativeObject());
    }

    public Shading getShading() {
        return Shading.values()[nGetShading(getNativeObject())];
    }

    public Interpolation getInterpolation() {
        return Interpolation.values()[nGetInterpolation(getNativeObject())];
    }

    public BlendingMode getBlendingMode() {
        return BlendingMode.values()[nGetBlendingMode(getNativeObject())];
    }

    public VertexDomain getVertexDomain() {
        return VertexDomain.values()[nGetVertexDomain(getNativeObject())];
    }

    public CullingMode getCullingMode() {
        return CullingMode.values()[nGetCullingMode(getNativeObject())];
    }

    public boolean isColorWriteEnabled() {
        return nIsColorWriteEnabled(getNativeObject());
    }

    public boolean isDepthWriteEnabled() {
        return nIsDepthWriteEnabled(getNativeObject());
    }

    public boolean isDepthCullingEnabled() {
        return nIsDepthCullingEnabled(getNativeObject());
    }

    public boolean isDoubleSided() {
        return nIsDoubleSided(getNativeObject());
    }

    public float getMaskThreshold() {
        return nGetMaskThreshold(getNativeObject());
    }

    public float getSpecularAntiAliasingVariance() {
        return nGetSpecularAntiAliasingVariance(getNativeObject());
    }

    public float getSpecularAntiAliasingThreshold() {
        return nGetSpecularAntiAliasingThreshold(getNativeObject());
    }

    public Set<VertexBuffer.VertexAttribute> getRequiredAttributes() {
        if (mRequiredAttributes == null) {
            int bitSet = nGetRequiredAttributes(getNativeObject());
            mRequiredAttributes = EnumSet.noneOf(VertexBuffer.VertexAttribute.class);
            VertexBuffer.VertexAttribute[] values = VertexBuffer.VertexAttribute.values();
            for (int i = 0; i < values.length; i++) {
                if ((bitSet & (1 << i)) != 0) {
                    mRequiredAttributes.add(values[i]);
                }
            }
            mRequiredAttributes = Collections.unmodifiableSet(mRequiredAttributes);
        }
        return mRequiredAttributes;
    }

    int getRequiredAttributesAsInt() {
        return nGetRequiredAttributes(getNativeObject());
    }

    public int getParameterCount() {
        return nGetParameterCount(getNativeObject());
    }

    public List<Parameter> getParameters() {
        int count = getParameterCount();
        List<Parameter> parameters = new ArrayList<>(count);
        if (count > 0) nGetParameters(getNativeObject(), parameters, count);
        return parameters;
    }

    public boolean hasParameter(@NonNull String name) {
        return nHasParameter(getNativeObject(), name);
    }

    public void setDefaultParameter(@NonNull String name, boolean x) {
        mDefaultInstance.setParameter(name, x);
    }

    public void setDefaultParameter(@NonNull String name, float x) {
        mDefaultInstance.setParameter(name, x);
    }

    public void setDefaultParameter(@NonNull String name, int x) {
        mDefaultInstance.setParameter(name, x);
    }

    public void setDefaultParameter(@NonNull String name, boolean x, boolean y) {
        mDefaultInstance.setParameter(name, x, y);
    }

    public void setDefaultParameter(@NonNull String name, float x, float y) {
        mDefaultInstance.setParameter(name, x, y);
    }

    public void setDefaultParameter(@NonNull String name, int x, int y) {
        mDefaultInstance.setParameter(name, x, y);
    }

    public void setDefaultParameter(@NonNull String name, boolean x, boolean y, boolean z) {
        mDefaultInstance.setParameter(name, x, y, z);
    }

    public void setDefaultParameter(@NonNull String name, float x, float y, float z) {
        mDefaultInstance.setParameter(name, x, y, z);
    }

    public void setDefaultParameter(@NonNull String name, int x, int y, int z) {
        mDefaultInstance.setParameter(name, x, y, z);
    }

    public void setDefaultParameter(@NonNull String name, boolean x, boolean y, boolean z, boolean w) {
        mDefaultInstance.setParameter(name, x, y, z, w);
    }

    public void setDefaultParameter(@NonNull String name, float x, float y, float z, float w) {
        mDefaultInstance.setParameter(name, x, y, z, w);
    }

    public void setDefaultParameter(@NonNull String name, int x, int y, int z, int w) {
        mDefaultInstance.setParameter(name, x, y, z, w);
    }

    public void setDefaultParameter(@NonNull String name,
            @NonNull MaterialInstance.BooleanElement type, @NonNull @Size(min = 1) boolean[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        mDefaultInstance.setParameter(name, type, v, offset, count);
    }

    public void setDefaultParameter(@NonNull String name,
            @NonNull MaterialInstance.IntElement type, @NonNull @Size(min = 1) int[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        mDefaultInstance.setParameter(name, type, v, offset, count);
    }

    public void setDefaultParameter(@NonNull String name,
            @NonNull MaterialInstance.FloatElement type, @NonNull @Size(min = 1) float[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        mDefaultInstance.setParameter(name, type, v, offset, count);
    }

    public void setDefaultParameter(@NonNull String name, @NonNull Colors.RgbType type,
            float r, float g, float b) {
        mDefaultInstance.setParameter(name, type, r, g, b);
    }

    public void setDefaultParameter(@NonNull String name, @NonNull Colors.RgbaType type,
            float r, float g, float b, float a) {
        mDefaultInstance.setParameter(name, type, r, g, b, a);
    }

    public void setDefaultParameter(@NonNull String name,
            @NonNull Texture texture, @NonNull TextureSampler sampler) {
        mDefaultInstance.setParameter(name, texture, sampler);
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed Material");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native long nBuilderBuild(long nativeEngine, @NonNull Buffer buffer, int size);
    private static native long nCreateInstance(long nativeMaterial);
    private static native long nGetDefaultInstance(long nativeMaterial);

    private static native String nGetName(long nativeMaterial);
    private static native int nGetShading(long nativeMaterial);
    private static native int nGetInterpolation(long nativeMaterial);
    private static native int nGetBlendingMode(long nativeMaterial);
    private static native int nGetVertexDomain(long nativeMaterial);
    private static native int nGetCullingMode(long nativeMaterial);
    private static native boolean nIsColorWriteEnabled(long nativeMaterial);
    private static native boolean nIsDepthWriteEnabled(long nativeMaterial);
    private static native boolean nIsDepthCullingEnabled(long nativeMaterial);
    private static native boolean nIsDoubleSided(long nativeMaterial);
    private static native float nGetMaskThreshold(long nativeMaterial);
    private static native float nGetSpecularAntiAliasingVariance(long nativeMaterial);
    private static native float nGetSpecularAntiAliasingThreshold(long nativeMaterial);

    private static native int nGetParameterCount(long nativeMaterial);
    private static native void nGetParameters(long nativeMaterial,
            @NonNull List<Parameter> parameters, @IntRange(from = 1) int count);
    private static native int nGetRequiredAttributes(long nativeMaterial);

    private static native boolean nHasParameter(long nativeMaterial, @NonNull String name);
}
