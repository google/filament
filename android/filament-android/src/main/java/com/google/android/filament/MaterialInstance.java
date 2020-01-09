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
import androidx.annotation.Size;

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

    /** @return the {@link Material} associated with this instance */
    @NonNull
    public Material getMaterial() {
        if (mMaterial == null) {
            mMaterial = new Material(mNativeMaterial);
        }
        return mMaterial;
    }

    /**
     * Sets the value of a bool parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the material parameter
     */
    public void setParameter(@NonNull String name, boolean x) {
        nSetParameterBool(getNativeObject(), name, x);
    }

    /**
     * Sets the value of a float parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the material parameter
     */
    public void setParameter(@NonNull String name, float x) {
        nSetParameterFloat(getNativeObject(), name, x);
    }

    /**
     * Sets the value of an int parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the material parameter
     */
    public void setParameter(@NonNull String name, int x) {
        nSetParameterInt(getNativeObject(), name, x);
    }

    /**
     * Sets the value of a bool2 parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     */
    public void setParameter(@NonNull String name, boolean x, boolean y) {
        nSetParameterBool2(getNativeObject(), name, x, y);
    }

    /**
     * Sets the value of a float2 parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     */
    public void setParameter(@NonNull String name, float x, float y) {
        nSetParameterFloat2(getNativeObject(), name, x, y);
    }

    /**
     * Sets the value of an int2 parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     */
    public void setParameter(@NonNull String name, int x, int y) {
        nSetParameterInt2(getNativeObject(), name, x, y);
    }

    /**
     * Sets the value of a bool3 parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     */
    public void setParameter(@NonNull String name, boolean x, boolean y, boolean z) {
        nSetParameterBool3(getNativeObject(), name, x, y, z);
    }

    /**
     * Sets the value of a float3 parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     */
    public void setParameter(@NonNull String name, float x, float y, float z) {
        nSetParameterFloat3(getNativeObject(), name, x, y, z);
    }

    /**
     * Sets the value of a int3 parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     */
    public void setParameter(@NonNull String name, int x, int y, int z) {
        nSetParameterInt3(getNativeObject(), name, x, y, z);
    }

    /**
     * Sets the value of a bool4 parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     * @param w    the value of the fourth component
     */
    public void setParameter(@NonNull String name, boolean x, boolean y, boolean z, boolean w) {
        nSetParameterBool4(getNativeObject(), name, x, y, z, w);
    }

    /**
     * Sets the value of a float4 parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     * @param w    the value of the fourth component
     */
    public void setParameter(@NonNull String name, float x, float y, float z, float w) {
        nSetParameterFloat4(getNativeObject(), name, x, y, z, w);
    }

    /**
     * Sets the value of a int4 parameter.
     *
     * @param name the name of the material parameter
     * @param x    the value of the first component
     * @param y    the value of the second component
     * @param z    the value of the third component
     * @param w    the value of the fourth component
     */
    public void setParameter(@NonNull String name, int x, int y, int z, int w) {
        nSetParameterInt4(getNativeObject(), name, x, y, z, w);
    }

    /**
     * Sets a texture and sampler parameter on this material's default instance.
     *
     * @param name The name of the material texture parameter
     * @param texture The texture to set as parameter
     * @param sampler The sampler to be used with this texture
     */
    public void setParameter(@NonNull String name,
            @NonNull Texture texture, @NonNull TextureSampler sampler) {
        nSetParameterTexture(getNativeObject(), name, texture.getNativeObject(), sampler.mSampler);
    }

    /**
     * Set a bool parameter array by name.
     *
     * @param name   name of the parameter array as defined by this Material
     * @param type   the number of components for each individual parameter
     * @param v      array of values to set to the named parameter array
     * @param offset the number of elements to skip
     * @param count  the number of elements in the parameter array to set
     *
     * <p>For example, to set a parameter array of 4 bool4s:
     * <pre>{@code
     *     boolean[] a = new boolean[4 * 4];
     *     instance.setParameter("param", MaterialInstance.BooleanElement.BOOL4, a, 0, 4);
     * }</pre>
     * To only set the last 3 elements, specify an offset of 1 and a count of 3:
     * <pre>{@code
     *     boolean[] a = new boolean[4 * 3];
     *     instance.setParameter("param", MaterialInstance.BooleanElement.BOOL4, a, 1, 3);
     * }</pre>
     * </p>
     */
    public void setParameter(@NonNull String name,
            @NonNull BooleanElement type, @NonNull boolean[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        nSetBooleanParameterArray(getNativeObject(), name, type.ordinal(), v, offset, count);
    }

    /**
     * Set an int parameter array by name.
     *
     * @param name   name of the parameter array as defined by this Material
     * @param type   the number of components for each individual parameter
     * @param v      array of values to set to the named parameter array
     * @param offset the number of elements to skip
     * @param count  the number of elements in the parameter array to set
     *
     * <p>For example, to set a parameter array of 4 int4s:
     * <pre>{@code
     *     int[] a = new int[4 * 4];
     *     instance.setParameter("param", MaterialInstance.IntElement.INT4, a, 0, 4);
     * }</pre>
     * To only set the last 3 elements, specify an offset of 1 and a count of 3:
     * <pre>{@code
     *     int[] a = new int[4 * 3];
     *     instance.setParameter("param", MaterialInstance.IntElement.INT4, a, 1, 3);
     * }</pre>
     * </p>
     */
    public void setParameter(@NonNull String name,
            @NonNull IntElement type, @NonNull int[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        nSetIntParameterArray(getNativeObject(), name, type.ordinal(), v, offset, count);
    }

    /**
     * Set a float parameter array by name.
     *
     * @param name   name of the parameter array as defined by this Material
     * @param type   the number of components for each individual parameter
     * @param v      array of values to set to the named parameter array
     * @param offset the number of elements to skip
     * @param count  the number of elements in the parameter array to set
     *
     * <p>For example, to set a parameter array of 4 float4s:
     * <pre>{@code
     *     float[] a = new float[4 * 4];
     *     material.setDefaultParameter("param", MaterialInstance.FloatElement.FLOAT4, a, 0, 4);
     * }</pre>
     * To only set the last 3 elements, specify an offset of 1 and a count of 3:
     * <pre>{@code
     *     float[] a = new float[4 * 3];
     *     material.setDefaultParameter("param", MaterialInstance.FloatElement.FLOAT4, a, 1, 3);
     * }</pre>
     * </p>
     */
    public void setParameter(@NonNull String name,
            @NonNull FloatElement type, @NonNull float[] v,
            @IntRange(from = 0) int offset, @IntRange(from = 1) int count) {
        nSetFloatParameterArray(getNativeObject(), name, type.ordinal(), v, offset, count);
    }

    /**
     * Sets the color of the given parameter on this material's default instance.
     *
     * @param name the name of the material color parameter
     * @param type whether the color is specified in the linear or sRGB space
     * @param r    red component
     * @param g    green component
     * @param b    blue component
     */
    public void setParameter(@NonNull String name, @NonNull Colors.RgbType type,
            float r, float g, float b) {
        float[] color = Colors.toLinear(type, r, g, b);
        nSetParameterFloat3(getNativeObject(), name, color[0], color[1], color[2]);
    }

    /**
     * Sets the color of the given parameter on this material's default instance.
     *
     * @param name the name of the material color parameter
     * @param type whether the color is specified in the linear or sRGB space
     * @param r    red component
     * @param g    green component
     * @param b    blue component
     * @param a    alpha component
     */
    public void setParameter(@NonNull String name, @NonNull Colors.RgbaType type,
            float r, float g, float b, float a) {
        float[] color = Colors.toLinear(type, r, g, b, a);
        nSetParameterFloat4(getNativeObject(), name, color[0], color[1], color[2], color[3]);
    }

    /**
     * Set up a custom scissor rectangle; by default this encompasses the View.
     *
     * @param left      left coordinate of the scissor box
     * @param bottom    bottom coordinate of the scissor box
     * @param width     width of the scissor box
     * @param height    height of the scissor box
     */
    public void setScissor(@IntRange(from = 0) int left, @IntRange(from = 0) int bottom,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height) {
        nSetScissor(getNativeObject(), left, bottom, width, height);
    }

    /** Returns the scissor rectangle to its default setting, which encompasses the View. */
    public void unsetScissor() {
        nUnsetScissor(getNativeObject());
    }

    /**
     * Sets a polygon offset that will be applied to all renderables drawn with this material
     * instance.
     *
     *  The value of the offset is scale * dz + r * constant, where dz is the change in depth
     *  relative to the screen area of the triangle, and r is the smallest value that is guaranteed
     *  to produce a resolvable offset for a given implementation. This offset is added before the
     *  depth test.
     *
     *  Warning: using a polygon offset other than zero has a significant negative performance
     *  impact, as most implementations have to disable early depth culling. DO NOT USE unless
     *  absolutely necessary.
     *
     * @param scale scale factor used to create a variable depth offset for each triangle
     * @param constant scale factor used to create a constant depth offset for each triangle
     */
    public void setPolygonOffset(float scale, float constant) {
        nSetPolygonOffset(getNativeObject(), scale, constant);
    }

    /**
     * Overrides the minimum alpha value a fragment must have to not be discarded when the blend
     * mode is MASKED. Defaults to 0.4 if it has not been set in the parent Material. The specified
     * value should be between 0 and 1 and will be clamped if necessary.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/blendingandtransparency:maskthreshold">
     * Blending and transparency: maskThreshold</a>
     */
    public void setMaskThreshold(float threshold) {
        nSetMaskThreshold(getNativeObject(), threshold);
    }

    /**
     * Sets the screen space variance of the filter kernel used when applying specular
     * anti-aliasing. The default value is set to 0.15. The specified value should be between
     * 0 and 1 and will be clamped if necessary.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/anti-aliasing:specularantialiasingvariance">
     * Anti-aliasing: specularAntiAliasingVariance</a>
     */
    public void setSpecularAntiAliasingVariance(float variance) {
        nSetSpecularAntiAliasingVariance(getNativeObject(), variance);
    }

    /**
     * Sets the clamping threshold used to suppress estimation errors when applying specular
     * anti-aliasing. The default value is set to 0.2. The specified value should be between 0
     * and 1 and will be clamped if necessary.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/anti-aliasing:specularantialiasingthreshold">
     * Anti-aliasing: specularAntiAliasingThreshold</a>
     */
    public void setSpecularAntiAliasingThreshold(float threshold) {
        nSetSpecularAntiAliasingThreshold(getNativeObject(), threshold);
    }

    /**
     * Enables or disables double-sided lighting if the parent Material has double-sided capability,
     * otherwise prints a warning. If double-sided lighting is enabled, backface culling is
     * automatically disabled.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:doublesided">
     * Rasterization: doubleSided</a>
     */
    public void setDoubleSided(boolean doubleSided) {
        nSetDoubleSided(getNativeObject(), doubleSided);
    }

    /**
     * Overrides the default triangle culling state that was set on the material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:culling">
     * Rasterization: culling</a>
     */
    public void setCullingMode(Material.CullingMode mode) {
        nSetCullingMode(getNativeObject(), mode.ordinal());
    }

    public long getNativeObject() {
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

    private static native void nSetCullingMode(long nativeMaterialInstance, long mode);
}
