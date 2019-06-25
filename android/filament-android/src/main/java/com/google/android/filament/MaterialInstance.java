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

public class MaterialInstance {
    private Material mMaterial;
    private long mNativeObject;
    private long mNativeMaterial;

    public enum BooleanElement {
        BOOL,
        BOOL2,
        BOOL3,
        BOOL4
    }

    public enum IntElement {
        INT,
        INT2,
        INT3,
        INT4
    }

    public enum FloatElement {
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4,
        MAT3,
        MAT4
    }

    MaterialInstance(@NonNull Material material, long nativeMaterialInstance) {
        mMaterial = material;
        mNativeObject = nativeMaterialInstance;
    }

    MaterialInstance(long nativeMaterial, long nativeMaterialInstance) {
        mNativeMaterial = nativeMaterial;
        mNativeObject = nativeMaterialInstance;
    }

    @NonNull
    public Material getMaterial() {
        if (mMaterial == null) {
            mMaterial = new Material(mNativeMaterial);
        }
        return mMaterial;
    }

    public void setParameter(@NonNull String name, boolean x) {
        nSetParameterBool(getNativeObject(), name, x);
    }

    public void setParameter(@NonNull String name, float x) {
        nSetParameterFloat(getNativeObject(), name, x);
    }

    public void setParameter(@NonNull String name, int x) {
        nSetParameterInt(getNativeObject(), name, x);
    }

    public void setParameter(@NonNull String name, boolean x, boolean y) {
        nSetParameterBool2(getNativeObject(), name, x, y);
    }

    public void setParameter(@NonNull String name, float x, float y) {
        nSetParameterFloat2(getNativeObject(), name, x, y);
    }

    public void setParameter(@NonNull String name, int x, int y) {
        nSetParameterInt2(getNativeObject(), name, x, y);
    }

    public void setParameter(@NonNull String name, boolean x, boolean y, boolean z) {
        nSetParameterBool3(getNativeObject(), name, x, y, z);
    }

    public void setParameter(@NonNull String name, float x, float y, float z) {
        nSetParameterFloat3(getNativeObject(), name, x, y, z);
    }

    public void setParameter(@NonNull String name, int x, int y, int z) {
        nSetParameterInt3(getNativeObject(), name, x, y, z);
    }

    public void setParameter(@NonNull String name, boolean x, boolean y, boolean z, boolean w) {
        nSetParameterBool4(getNativeObject(), name, x, y, z, w);
    }

    public void setParameter(@NonNull String name, float x, float y, float z, float w) {
        nSetParameterFloat4(getNativeObject(), name, x, y, z, w);
    }

    public void setParameter(@NonNull String name, int x, int y, int z, int w) {
        nSetParameterInt4(getNativeObject(), name, x, y, z, w);
    }

    public void setParameter(@NonNull String name,
            @NonNull Texture texture, @NonNull TextureSampler sampler) {
        nSetParameterTexture(getNativeObject(), name, texture.getNativeObject(), sampler.mSampler);
    }

    public void setParameter(@NonNull String name,
            @NonNull BooleanElement type, @NonNull boolean[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        nSetBooleanParameterArray(getNativeObject(), name, type.ordinal(), v, offset, count);
    }

    public void setParameter(@NonNull String name,
            @NonNull IntElement type, @NonNull int[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        nSetIntParameterArray(getNativeObject(), name, type.ordinal(), v, offset, count);
    }

    public void setParameter(@NonNull String name,
            @NonNull FloatElement type, @NonNull float[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        nSetFloatParameterArray(getNativeObject(), name, type.ordinal(), v, offset, count);
    }

    public void setParameter(@NonNull String name, @NonNull Colors.RgbType type,
            float r, float g, float b) {
        float[] color = Colors.toLinear(type, r, g, b);
        nSetParameterFloat3(getNativeObject(), name, color[0], color[1], color[2]);
    }

    public void setParameter(@NonNull String name, @NonNull Colors.RgbaType type,
            float r, float g, float b, float a) {
        float[] color = Colors.toLinear(type, r, g, b, a);
        nSetParameterFloat4(getNativeObject(), name, color[0], color[1], color[2], color[3]);
    }

    public void setScissor(@IntRange(from = 0) int left, @IntRange(from = 0) int bottom,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height) {
        nSetScissor(getNativeObject(), left, bottom, width, height);
    }

    public void unsetScissor() {
        nUnsetScissor(getNativeObject());
    }

    public void setPolygonOffset(float scale, float constant) {
        nSetPolygonOffset(getNativeObject(), scale, constant);
    }

    public void setMaskThreshold(float threshold) {
        nSetMaskThreshold(getNativeObject(), threshold);
    }

    public void setSpecularAntiAliasingVariance(float variance) {
        nSetSpecularAntiAliasingVariance(getNativeObject(), variance);
    }

    public void setSpecularAntiAliasingThreshold(float threshold) {
        nSetSpecularAntiAliasingThreshold(getNativeObject(), threshold);
    }

    public void setDoubleSided(boolean doubleSided) {
        nSetDoubleSided(getNativeObject(), doubleSided);
    }

    long getNativeObject() {
        if (mNativeObject == 0) {
            throw new IllegalStateException("Calling method on destroyed MaterialInstance");
        }
        return mNativeObject;
    }

    void clearNativeObject() {
        mNativeObject = 0;
    }

    private static native void nSetParameterBool(long nativeMaterialInstance,
            @NonNull String name, boolean x);
    private static native void nSetParameterFloat(long nativeMaterialInstance,
            @NonNull String name, float x);
    private static native void nSetParameterInt(long nativeMaterialInstance,
            @NonNull String name, int x);

    private static native void nSetParameterBool2(long nativeMaterialInstance,
            @NonNull String name, boolean x, boolean y);
    private static native void nSetParameterFloat2(long nativeMaterialInstance,
            @NonNull String name, float x, float y);
    private static native void nSetParameterInt2(long nativeMaterialInstance,
            @NonNull String name, int x, int y);

    private static native void nSetParameterBool3(long nativeMaterialInstance,
            @NonNull String name, boolean x, boolean y, boolean z);
    private static native void nSetParameterFloat3(long nativeMaterialInstance,
            @NonNull String name, float x, float y, float z);
    private static native void nSetParameterInt3(long nativeMaterialInstance,
            @NonNull String name, int x, int y, int z);

    private static native void nSetParameterBool4(long nativeMaterialInstance,
            @NonNull String name, boolean x, boolean y, boolean z, boolean w);
    private static native void nSetParameterFloat4(long nativeMaterialInstance,
            @NonNull String name, float x, float y, float z, float w);
    private static native void nSetParameterInt4(long nativeMaterialInstance,
            @NonNull String name, int x, int y, int z, int w);

    private static native void nSetBooleanParameterArray(long nativeMaterialInstance,
            @NonNull String name, int element, @NonNull @Size(min = 1) boolean[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count);
    private static native void nSetIntParameterArray(long nativeMaterialInstance,
            @NonNull String name, int element, @NonNull @Size(min = 1) int[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count);
    private static native void nSetFloatParameterArray(long nativeMaterialInstance,
            @NonNull String name, int element, @NonNull @Size(min = 1) float[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count);

    private static native void nSetParameterTexture(long nativeMaterialInstance,
            @NonNull String name, long nativeTexture, int sampler);

    private static native void nSetScissor(long nativeMaterialInstance,
            @IntRange(from = 0) int left, @IntRange(from = 0) int bottom,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height);

    private static native void nUnsetScissor(long nativeMaterialInstance);

    private static native void nSetPolygonOffset(long nativeMaterialInstance,
            float scale, float constant);

    private static native void nSetMaskThreshold(long nativeMaterialInstance, float threshold);

    private static native void nSetSpecularAntiAliasingVariance(long nativeMaterialInstance,
            float variance);
    private static native void nSetSpecularAntiAliasingThreshold(long nativeMaterialInstance,
            float threshold);

    private static native void nSetDoubleSided(long nativeMaterialInstance, boolean doubleSided);
}
