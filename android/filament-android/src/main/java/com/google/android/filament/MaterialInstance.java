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

import com.google.android.filament.proguard.UsedByNative;

@UsedByNative("AssetLoader.cpp")
public class MaterialInstance {
    private static final Material.CullingMode[] sCullingModeValues = Material.CullingMode.values();
    private Material mMaterial;
    private String mName;
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

    /**
     * Operations that control how the stencil buffer is updated.
     */
    public enum StencilOperation {
        /**
         * Keeps the current value.
         */
        KEEP,
        /**
         * Sets the value to 0.
         */
        ZERO,
        /**
         * Sets the value to the stencil reference value.
         */
        REPLACE,
        /**
         * Increments the current value. Clamps to the maximum representable unsigned value.
         */
        INCR_CLAMP,
        /**
         * Increments the current value. Wraps value to zero when incrementing the maximum
         * representable unsigned value.
         */
        INCR_WRAP,
        /**
         * Decrements the current value. Clamps to 0.
         */
        DECR_CLAMP,
        /**
         * Decrements the current value. Wraps value to the maximum representable unsigned value
         * when decrementing a value of zero.
         */
        DECR_WRAP,
        /**
         * Bitwise inverts the current value.
         */
        INVERT,
    }

    public enum StencilFace {
        FRONT,
        BACK,
        FRONT_AND_BACK
    }
    // Converts the StencilFace enum ordinal to Filament's equivalent bit field.
    static final int[] sStencilFaceMapping = {0x1, 0x2, 0x3};

    public MaterialInstance(Engine engine, long nativeMaterialInstance) {
        mNativeObject = nativeMaterialInstance;
        mNativeMaterial = nGetMaterial(mNativeObject);
    }

    MaterialInstance(@NonNull Material material, long nativeMaterialInstance) {
        mMaterial = material;
        mNativeMaterial = material.getNativeObject();
        mNativeObject = nativeMaterialInstance;
    }

    MaterialInstance(long nativeMaterialInstance) {
        mNativeObject = nativeMaterialInstance;
        mNativeMaterial = nGetMaterial(mNativeObject);
    }

    /**
     * Creates a new {@link #MaterialInstance} using another {@link #MaterialInstance} as a template for initialization.
     * The new {@link #MaterialInstance} is an instance of the same {@link Material} of the template instance and
     * must be destroyed just like any other {@link #MaterialInstance}.
     *
     * @param other A {@link #MaterialInstance} to use as a template for initializing a new instance
     * @param name  A name for the new {@link #MaterialInstance} or nullptr to use the template's name
     * @return      A new {@link #MaterialInstance}
     */
    @NonNull
    public static MaterialInstance duplicate(@NonNull MaterialInstance other, String name) {
        long nativeInstance = nDuplicate(other.mNativeObject, name);
        if (nativeInstance == 0) throw new IllegalStateException("Couldn't duplicate MaterialInstance");
        return new MaterialInstance(other.getMaterial(), nativeInstance);
    }

    /** @return the {@link Material} associated with this instance */
    @NonNull
    public Material getMaterial() {
        if (mMaterial == null) {
            mMaterial = new Material(mNativeMaterial);
        }
        return mMaterial;
    }

    /** @return the name associated with this instance */
    @NonNull
    public String getName() {
        if (mName == null) {
            mName = nGetName(getNativeObject());
        }
        return mName;
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
     * <p>
     * Note: Depth textures can't be sampled with a linear filter unless the comparison mode is set
     *       to COMPARE_TO_TEXTURE.
     * </p>
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
     * @param offset the number of elements in <code>v</code> to skip
     * @param count  the number of elements in the parameter array to set
     *
     * <p>For example, to set a parameter array of 4 bool4s:
     * <pre>{@code
     *     boolean[] a = new boolean[4 * 4];
     *     instance.setParameter("param", MaterialInstance.BooleanElement.BOOL4, a, 0, 4);
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
     * @param offset the number of elements in <code>v</code> to skip
     * @param count  the number of elements in the parameter array to set
     *
     * <p>For example, to set a parameter array of 4 int4s:
     * <pre>{@code
     *     int[] a = new int[4 * 4];
     *     instance.setParameter("param", MaterialInstance.IntElement.INT4, a, 0, 4);
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
     * @param offset the number of elements in <code>v</code> to skip
     * @param count  the number of elements in the parameter array to set
     *
     * <p>For example, to set a parameter array of 4 float4s:
     * <pre>{@code
     *     float[] a = new float[4 * 4];
     *     material.setDefaultParameter("param", MaterialInstance.FloatElement.FLOAT4, a, 0, 4);
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
     * Set-up a custom scissor rectangle; by default it is disabled.
     *
     * <p>
     * The scissor rectangle gets clipped by the View's viewport, in other words, the scissor
     * cannot affect fragments outside of the View's Viewport.
     * </p>
     *
     * <p>
     * Currently the scissor is not compatible with dynamic resolution and should always be
     * disabled when dynamic resolution is used.
     * </p>
     *
     * @param left      left coordinate of the scissor box relative to the viewport
     * @param bottom    bottom coordinate of the scissor box relative to the viewport
     * @param width     width of the scissor box
     * @param height    height of the scissor box
     *
     * @see #unsetScissor
     * @see View#setViewport
     * @see View#setDynamicResolutionOptions
     */
    public void setScissor(@IntRange(from = 0) int left, @IntRange(from = 0) int bottom,
            @IntRange(from = 0) int width, @IntRange(from = 0) int height) {
        nSetScissor(getNativeObject(), left, bottom, width, height);
    }

    /**
     * Returns the scissor rectangle to its default disabled setting.
     * <p>
     * Currently the scissor is not compatible with dynamic resolution and should always be
     * disabled when dynamic resolution is used.
     * </p>
     * @see View#setDynamicResolutionOptions
     */
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
     * Gets the minimum alpha value a fragment must have to not be discarded when the blend
     * mode is MASKED
     */
    public float getMaskThreshold() {
        return nGetMaskThreshold(getNativeObject());
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
     * Gets the screen space variance of the filter kernel used when applying specular
     * anti-aliasing.
     */
    public float getSpecularAntiAliasingVariance() {
        return nGetSpecularAntiAliasingVariance(getNativeObject());
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
     * Gets the clamping threshold used to suppress estimation errors when applying specular
     * anti-aliasing.
     */
    public float getSpecularAntiAliasingThreshold() {
        return nGetSpecularAntiAliasingThreshold(getNativeObject());
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
     * Returns whether double-sided lighting is enabled when the parent Material has double-sided
     * capability.
     */
    public boolean isDoubleSided() {
        return nIsDoubleSided(getNativeObject());
    }

    /**
     * Overrides the default triangle culling state that was set on the material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:culling">
     * Rasterization: culling</a>
     */
    public void setCullingMode(@NonNull Material.CullingMode mode) {
        nSetCullingMode(getNativeObject(), mode.ordinal());
    }

    /**
     * Overrides the default triangle culling state that was set on the material separately for the
     * color and shadow passes
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:culling">
     * Rasterization: culling</a>
     */
    public void setCullingMode(@NonNull Material.CullingMode colorPassCullingMode,
                               @NonNull Material.CullingMode shadowPassCullingMode) {
        nSetCullingModeSeparate(getNativeObject(),
            colorPassCullingMode.ordinal(), shadowPassCullingMode.ordinal());
    }

    /**
     * Returns the face culling mode.
     */
    @NonNull
    public Material.CullingMode getCullingMode() {
        return sCullingModeValues[nGetCullingMode(getNativeObject())];
    }

    /**
     * Returns the face culling mode for the shadow passes.
     */
    @NonNull
    public Material.CullingMode getShadowCullingMode() {
        return sCullingModeValues[nGetShadowCullingMode(getNativeObject())];
    }

    /**
     * Overrides the default color-buffer write state that was set on the material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:colorWrite">
     * Rasterization: colorWrite</a>
     */
    public void setColorWrite(boolean enable) {
        nSetColorWrite(getNativeObject(), enable);
    }

    /**
     * Returns whether color write is enabled.
     */
    public boolean isColorWriteEnabled() {
        return nIsColorWriteEnabled(getNativeObject());
    }

    /**
     * Overrides the default depth-buffer write state that was set on the material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:depthWrite">
     * Rasterization: depthWrite</a>
     */
    public void setDepthWrite(boolean enable) {
        nSetDepthWrite(getNativeObject(), enable);
    }

    /**
     * Returns whether depth write is enabled.
     */
    public boolean isDepthWriteEnabled() {
        return nIsDepthWriteEnabled(getNativeObject());
    }

    /**
     * Enables or Disable stencil writes
     */
    public void setStencilWrite(boolean enable) {
        nSetStencilWrite(getNativeObject(), enable);
    }

    /**
     * Returns whether stencil write is enabled.
     */
    public boolean isStencilWriteEnabled() {
        return nIsStencilWriteEnabled(getNativeObject());
    }

    /**
     * Overrides the default depth testing state that was set on the material.
     *
     * @see
     * <a href="https://google.github.io/filament/Materials.html#materialdefinitions/materialblock/rasterization:depthCulling">
     * Rasterization: depthCulling</a>
     */
    public void setDepthCulling(boolean enable) {
        nSetDepthCulling(getNativeObject(), enable);
    }

    /**
     * Sets the depth comparison function (default is {@link TextureSampler.CompareFunction#GE}).
     *
     * @param func the depth comparison function
     */
    public void setDepthFunc(TextureSampler.CompareFunction func) {
        nSetDepthFunc(getNativeObject(), func.ordinal());
    }

    /**
     * Returns whether depth culling is enabled.
     */
    public boolean isDepthCullingEnabled() {
        return nIsDepthCullingEnabled(getNativeObject());
    }

    /**
     * Returns the depth comparison function.
     */
    public TextureSampler.CompareFunction getDepthFunc() {
        return TextureSampler.EnumCache.sCompareFunctionValues[nGetDepthFunc(getNativeObject())];
    }

    /**
     * Sets the stencil comparison function (default is {@link TextureSampler.CompareFunction#ALWAYS}).
     *
     * <p>
     * It's possible to set separate stencil comparison functions; one for front-facing polygons,
     * and one for back-facing polygons. The face parameter determines the comparison function(s)
     * updated by this call.
     * </p>
     *
     * @param func the stencil comparison function
     * @param face the faces to update the comparison function for
     */
    public void setStencilCompareFunction(TextureSampler.CompareFunction func, StencilFace face) {
        nSetStencilCompareFunction(getNativeObject(), func.ordinal(),
                sStencilFaceMapping[face.ordinal()]);
    }

    /**
     * Sets the stencil comparison function for both front and back-facing polygons.
     * @see #setStencilCompareFunction(TextureSampler.CompareFunction, StencilFace)
     */
    public void setStencilCompareFunction(TextureSampler.CompareFunction func) {
        setStencilCompareFunction(func, StencilFace.FRONT_AND_BACK);
    }

    /**
     * Sets the stencil fail operation (default is {@link StencilOperation#KEEP}).
     *
     * <p>
     * The stencil fail operation is performed to update values in the stencil buffer when the
     * stencil test fails.
     * </p>
     *
     * <p>
     * It's possible to set separate stencil fail operations; one for front-facing polygons, and one
     * for back-facing polygons. The face parameter determines the stencil fail operation(s) updated
     * by this call.
     * </p>
     *
     * @param op the stencil fail operation
     * @param face the faces to update the stencil fail operation for
     */
    public void setStencilOpStencilFail(StencilOperation op, StencilFace face) {
        nSetStencilOpStencilFail(getNativeObject(), op.ordinal(),
                sStencilFaceMapping[face.ordinal()]);
    }

    /**
     * Sets the stencil fail operation for both front and back-facing polygons.
     * @see #setStencilOpStencilFail(StencilOperation, StencilFace)
     */
    public void setStencilOpStencilFail(StencilOperation op) {
        setStencilOpStencilFail(op, StencilFace.FRONT_AND_BACK);
    }

    /**
     * Sets the depth fail operation (default is {@link StencilOperation#KEEP}).
     *
     * <p>
     * The depth fail operation is performed to update values in the stencil buffer when the depth
     * test fails.
     * </p>
     *
     * <p>
     * It's possible to set separate depth fail operations; one for front-facing polygons, and one
     * for back-facing polygons. The face parameter determines the depth fail operation(s) updated
     * by this call.
     * </p>
     *
     * @param op the depth fail operation
     * @param face the faces to update the depth fail operation for
     */
    public void setStencilOpDepthFail(StencilOperation op, StencilFace face) {
        nSetStencilOpDepthFail(getNativeObject(), op.ordinal(),
                sStencilFaceMapping[face.ordinal()]);
    }

    /**
     * Sets the depth fail operation for both front and back-facing polygons.
     * @see #setStencilOpDepthFail(StencilOperation, StencilFace)
     */
    public void setStencilOpDepthFail(StencilOperation op) {
        setStencilOpDepthFail(op, StencilFace.FRONT_AND_BACK);
    }

    /**
     * Sets the depth-stencil pass operation (default is {@link StencilOperation#KEEP}).
     *
     * <p>
     * The depth-stencil pass operation is performed to update values in the stencil buffer when
     * both the stencil test and depth test pass.
     * </p>
     *
     * <p>
     * It's possible to set separate depth-stencil pass operations; one for front-facing polygons,
     * and one for back-facing polygons. The face parameter determines the depth-stencil pass
     * operation(s) updated by this call.
     * </p>
     *
     * @param op the depth-stencil pass operation
     * @param face the faces to update the depth-stencil operation for
     */
    public void setStencilOpDepthStencilPass(StencilOperation op, StencilFace face) {
        nSetStencilOpDepthStencilPass(getNativeObject(), op.ordinal(),
                sStencilFaceMapping[face.ordinal()]);
    }

    /**
     * Sets the depth-stencil pass operation for both front and back-facing polygons.
     * @see #setStencilOpDepthStencilPass(StencilOperation, StencilFace)
     */
    public void setStencilOpDepthStencilPass(StencilOperation op) {
        setStencilOpDepthStencilPass(op, StencilFace.FRONT_AND_BACK);
    }

    /**
     * Sets the stencil reference value (default is 0).
     *
     * <p>
     * The stencil reference value is the left-hand side for stencil comparison tests. It's also
     * used as the replacement stencil value when {@link StencilOperation} is
     * {@link StencilOperation#REPLACE}.
     * </p>
     *
     * <p>
     * It's possible to set separate stencil reference values; one for front-facing polygons, and
     * one for back-facing polygons. The face parameter determines the reference value(s) updated by
     * this call.
     * </p>
     *
     * @param value the stencil reference value (only the least significant 8 bits are used)
     * @param face the faces to update the reference value for
     */
    public void setStencilReferenceValue(@IntRange(from = 0, to = 255) int value, StencilFace face) {
        nSetStencilReferenceValue(getNativeObject(), value, sStencilFaceMapping[face.ordinal()]);
    }

    /**
     * Sets the stencil reference value for both front and back-facing polygons.
     * @see #setStencilReferenceValue(int, StencilFace)
     */
    public void setStencilReferenceValue(@IntRange(from = 0, to = 255) int value) {
        setStencilReferenceValue(value, StencilFace.FRONT_AND_BACK);
    }

    /**
     * Sets the stencil read mask (default is 0xFF).
     *
     * <p>
     * The stencil read mask masks the bits of the values participating in the stencil comparison
     * test- both the value read from the stencil buffer and the reference value.
     * </p>
     *
     * <p>
     * It's possible to set separate stencil read masks; one for front-facing polygons, and one for
     * back-facing polygons. The face parameter determines the stencil read mask(s) updated by this
     * call.
     * </p>
     *
     * @param readMask the read mask (only the least significant 8 bits are used)
     * @param face the faces to update the read mask for
     */
    public void setStencilReadMask(@IntRange(from = 0, to = 255) int readMask, StencilFace face) {
        nSetStencilReadMask(getNativeObject(), readMask, sStencilFaceMapping[face.ordinal()]);
    }

    /**
     * Sets the stencil read mask for both front and back-facing polygons.
     * @see #setStencilReadMask(int, StencilFace)
     */
    public void setStencilReadMask(@IntRange(from = 0, to = 255) int readMask) {
        setStencilReadMask(readMask, StencilFace.FRONT_AND_BACK);
    }

    /**
     * Sets the stencil write mask (default is 0xFF).
     *
     * <p>
     * The stencil write mask masks the bits in the stencil buffer updated by stencil operations.
     * </p>
     *
     * <p>
     * It's possible to set separate stencil write masks; one for front-facing polygons, and one for
     * back-facing polygons. The face parameter determines the stencil write mask(s) updated by this
     * call.
     * </p>
     *
     * @param writeMask the write mask (only the least significant 8 bits are used)
     * @param face the faces to update the read mask for
     */
    public void setStencilWriteMask(@IntRange(from = 0, to = 255) int writeMask, StencilFace face) {
        nSetStencilWriteMask(getNativeObject(), writeMask, sStencilFaceMapping[face.ordinal()]);
    }

    /**
     * Sets the stencil write mask for both front and back-facing polygons.
     * @see #setStencilWriteMask(int, StencilFace)
     */
    public void setStencilWriteMask(int writeMask) {
        setStencilWriteMask(writeMask, StencilFace.FRONT_AND_BACK);
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
            @NonNull String name, long nativeTexture, long sampler);

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
    private static native void nSetCullingModeSeparate(long nativeMaterialInstance,
            long colorPassCullingMode, long shadowPassCullingMode);
    private static native void nSetColorWrite(long nativeMaterialInstance, boolean enable);
    private static native void nSetDepthWrite(long nativeMaterialInstance, boolean enable);
    private static native void nSetStencilWrite(long nativeMaterialInstance, boolean enable);
    private static native void nSetDepthCulling(long nativeMaterialInstance, boolean enable);
    private static native void nSetDepthFunc(long nativeMaterialInstance, long function);

    private static native void nSetStencilCompareFunction(long nativeMaterialInstance,
            long function, long face);
    private static native void nSetStencilOpStencilFail(long nativeMaterialInstance, long op,
            long face);
    private static native void nSetStencilOpDepthFail(long nativeMaterialInstance, long op,
            long face);
    private static native void nSetStencilOpDepthStencilPass(long nativeMaterialInstance, long op,
            long face);
    private static native void nSetStencilReferenceValue(long nativeMaterialInstance, int value,
            long face);
    private static native void nSetStencilReadMask(long nativeMaterialInstance, int readMask,
            long face);
    private static native void nSetStencilWriteMask(long nativeMaterialInstance, int writeMask,
            long face);

    private static native String nGetName(long nativeMaterialInstance);
    private static native long nGetMaterial(long nativeMaterialInstance);

    private static native long nDuplicate(long otherNativeMaterialInstance, String name);


    private static native float nGetMaskThreshold(long nativeMaterialInstance);
    private static native float nGetSpecularAntiAliasingVariance(long nativeMaterialInstance);
    private static native float nGetSpecularAntiAliasingThreshold(long nativeMaterialInstance);
    private static native boolean nIsDoubleSided(long nativeMaterialInstance);
    private static native int nGetCullingMode(long nativeMaterialInstance);
    private static native int nGetShadowCullingMode(long nativeMaterialInstance);
    private static native boolean nIsColorWriteEnabled(long nativeMaterialInstance);
    private static native boolean nIsDepthWriteEnabled(long nativeMaterialInstance);
    private static native boolean nIsStencilWriteEnabled(long nativeMaterialInstance);
    private static native boolean nIsDepthCullingEnabled(long nativeMaterialInstance);
    private static native int nGetDepthFunc(long nativeMaterialInstance);
}
