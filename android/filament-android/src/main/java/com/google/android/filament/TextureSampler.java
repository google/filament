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

import androidx.annotation.NonNull;

/**
 * <code>TextureSampler</code> defines how a texture is accessed.
 */
public class TextureSampler {
    public enum WrapMode {
        /**
         * The edge of the texture extends to infinity.
         */
        CLAMP_TO_EDGE,
        /**
         * The texture infinitely repeats in the wrap direction.
         */
        REPEAT,
        /**
         * The texture infinitely repeats and mirrors in the wrap direction.
         */
        MIRRORED_REPEAT
    }

    public enum MinFilter {
        /**
         * No filtering. Nearest neighbor is used.
         */
        NEAREST,
        /**
         * Box filtering. Weighted average of 4 neighbors is used.
         */
        LINEAR,
        /**
         * Mip-mapping is activated. But no filtering occurs.
         */
        NEAREST_MIPMAP_NEAREST,
        /**
         * Box filtering within a mip-map level.
         */
        LINEAR_MIPMAP_NEAREST,
        /**
         * Mip-map levels are interpolated, but no other filtering occurs.
         */
        NEAREST_MIPMAP_LINEAR,
        /**
         * Both interpolated Mip-mapping and linear filtering are used.
         */
        LINEAR_MIPMAP_LINEAR,
    }

    public enum MagFilter {
        /**
         * No filtering. Nearest neighbor is used.
         */
        NEAREST,
        /**
         * Box filtering. Weighted average of 4 neighbors is used.
         */
        LINEAR
    }

    public enum CompareMode {
        NONE,
        COMPARE_TO_TEXTURE
    }

    /**
     * Comparison functions for the depth sampler.
     */
    public enum CompareFunction {
        /**
         * Less or equal
         */
        LESS_EQUAL,
        /**
         * Greater or equal
         */
        GREATER_EQUAL,
        /**
         * Strictly less than
         */
        LESS,
        /**
         * Strictly greater than
         */
        GREATER,
        /**
         * Equal
         */
        EQUAL,
        /**
         * Not equal
         */
        NOT_EQUAL,
        /**
         * Always. Depth testing is deactivated.
         */
        ALWAYS,
        /**
         * Never. The depth test always fails.
         */
        NEVER
    }

    int mSampler = 0; // bit field used by native

    public TextureSampler(int sampler) {
        mSampler = sampler;
    }

    /**
     * Initializes the <code>TextureSampler</code> with default values.
     * <br>Minification filter: {@link MinFilter#LINEAR_MIPMAP_LINEAR}
     * <br>Magnification filter: {@link MagFilter#LINEAR}
     * <br>Wrap modes: {@link WrapMode#REPEAT}
     */
    public TextureSampler() {
        this(MinFilter.LINEAR_MIPMAP_LINEAR, MagFilter.LINEAR, WrapMode.REPEAT);
    }

    /**
     * Initializes the <code>TextureSampler</code> with default values, but specifying the
     * minification and magnification filters.
     *
     * @param minMag {@link MagFilter magnification filter},
     *               the minification filter will be set to the same value.
     */
    public TextureSampler(@NonNull MagFilter minMag) {
        this(minMag, WrapMode.CLAMP_TO_EDGE);
    }

    /**
     * Initializes the <code>TextureSampler</code> with user specified values.
     *
     * @param minMag {@link MagFilter magnification filter},
     *               the minification filter will be set to the same value.
     * @param wrap   {@link WrapMode wrapping mode} for all directions
     */
    public TextureSampler(@NonNull MagFilter minMag, @NonNull WrapMode wrap) {
        this(minFilterFromMagFilter(minMag), minMag, wrap);
    }

    /**
     * Initializes the <code>TextureSampler</code> with user specified values.
     *
     * @param min  {@link MagFilter magnification filter}
     * @param mag  {@link MinFilter minification filter}
     * @param wrap {@link WrapMode wrapping mode} for all directions
     */
    public TextureSampler(@NonNull MinFilter min, @NonNull MagFilter mag, @NonNull WrapMode wrap) {
        this(min, mag, wrap, wrap, wrap);
    }

    /**
     * Initializes the <code>TextureSampler</code> with user specified values.
     *
     * @param min {@link MagFilter magnification filter}
     * @param mag {@link MinFilter minification filter}
     * @param s   {@link WrapMode wrapping mode} for the s (horizontal) direction
     * @param t   {@link WrapMode wrapping mode} for the t (vertical) direction
     * @param r   {@link WrapMode wrapping mode} fot the r (depth) direction
     */
    public TextureSampler(@NonNull MinFilter min, @NonNull MagFilter mag,
            @NonNull WrapMode s, @NonNull WrapMode t, @NonNull WrapMode r) {
        mSampler = nCreateSampler(min.ordinal(), mag.ordinal(),
                s.ordinal(), t.ordinal(), r.ordinal());
    }

    /**
     * Initializes the <code>TextureSampler</code> with user specified comparison mode. The
     * comparison fonction is set to {@link CompareFunction#LESS_EQUAL}.
     *
     * @param mode     {@link CompareMode comparison mode}
     */
    public TextureSampler(@NonNull CompareMode mode) {
        this(mode, CompareFunction.LESS_EQUAL);
    }

    /**
     * Initializes the <code>TextureSampler</code> with user specified comparison mode and function.
     *
     * @param mode     {@link CompareMode comparison mode}
     * @param function {@link CompareFunction comparison function}
     */
    public TextureSampler(@NonNull CompareMode mode, @NonNull CompareFunction function) {
        mSampler = nCreateCompareSampler(mode.ordinal(), function.ordinal());
    }

    /**
     * @return the minification filter
     */
    public MinFilter getMinFilter() {
        return MinFilter.values()[nGetMinFilter(mSampler)];
    }

    /**
     * Sets the minification filter.
     *
     * @param filter minification filter
     */
    public void setMinFilter(MinFilter filter) {
        mSampler = nSetMinFilter(mSampler, filter.ordinal());
    }

    /**
     * @return the magnification filter
     */
    public MagFilter getMagFilter() {
        return MagFilter.values()[nGetMagFilter(mSampler)];
    }

    /**
     * Sets the magnification filter.
     *
     * @param filter magnification filter
     */
    public void setMagFilter(MagFilter filter) {
        mSampler = nSetMagFilter(mSampler, filter.ordinal());
    }

    /**
     * @return the wrapping mode in the s (horizontal) direction
     */
    public WrapMode getWrapModeS() {
        return WrapMode.values()[nGetWrapModeS(mSampler)];
    }

    /**
     * Sets the wrapping mode in the s (horizontal) direction.
     * @param mode wrapping mode
     */
    public void setWrapModeS(WrapMode mode) {
        mSampler = nSetWrapModeS(mSampler, mode.ordinal());
    }

    /**
     * @return the wrapping mode in the t (vertical) direction
     */
    public WrapMode getWrapModeT() {
        return WrapMode.values()[nGetWrapModeT(mSampler)];
    }

    /**
     * Sets the wrapping mode in the t (vertical) direction.
     * @param mode wrapping mode
     */
    public void setWrapModeT(WrapMode mode) {
        mSampler = nSetWrapModeT(mSampler, mode.ordinal());
    }

    /**
     * @return the wrapping mode in the r (depth) direction
     */
    public WrapMode getWrapModeR() {
        return WrapMode.values()[nGetWrapModeR(mSampler)];
    }

    /**
     * Sets the wrapping mode in the t (depth) direction.
     * @param mode wrapping mode
     */
    public void setWrapModeR(WrapMode mode) {
        mSampler = nSetWrapModeR(mSampler, mode.ordinal());
    }

    /**
     * @return the anisotropy value
     * @see #setAnisotropy
     */
    public float getAnisotropy() {
        return nGetAnisotropy(mSampler);
    }

    /**
     * This controls anisotropic filtering.
     *
     * @param anisotropy Amount of anisotropy, should be a power-of-two. The default is 0.
     *                   The maximum permissible value is 7.
     */
    public void setAnisotropy(float anisotropy) {
        mSampler = nSetAnisotropy(mSampler, anisotropy);
    }

    /**
     * @return the comparison mode
     */
    public CompareMode getCompareMode() {
        return CompareMode.values()[nGetCompareMode(mSampler)];
    }

    /**
     * Sets the comparison mode.
     *
     * @param mode comparison mode
     */
    public void setCompareMode(CompareMode mode) {
        mSampler = nSetCompareMode(mSampler, mode.ordinal());
    }

    /**
     * @return the comparison function
     */
    public CompareFunction getCompareFunction() {
        return CompareFunction.values()[nGetCompareFunction(mSampler)];
    }

    /**
     * Sets the comparison function.
     * @param function the comparison function
     */
    public void setCompareFunction(CompareFunction function) {
        mSampler = nSetCompareFunction(mSampler, function.ordinal());
    }

    private static MinFilter minFilterFromMagFilter(@NonNull MagFilter minMag) {
        switch (minMag) {
            case NEAREST:
                return MinFilter.NEAREST;
            case LINEAR:
            default:
                return MinFilter.LINEAR;
        }
    }

    private static native int nCreateSampler(int min, int max, int s, int t, int r);
    private static native int nCreateCompareSampler(int mode, int function);

    private static native int nGetMinFilter(int sampler);
    private static native int nSetMinFilter(int sampler, int filter);
    private static native int nGetMagFilter(int sampler);
    private static native int nSetMagFilter(int sampler, int filter);

    private static native int nGetWrapModeS(int sampler);
    private static native int nSetWrapModeS(int sampler, int mode);
    private static native int nGetWrapModeT(int sampler);
    private static native int nSetWrapModeT(int sampler, int mode);
    private static native int nGetWrapModeR(int sampler);
    private static native int nSetWrapModeR(int sampler, int mode);

    private static native int nGetCompareMode(int sampler);
    private static native int nSetCompareMode(int sampler, int mode);
    private static native int nGetCompareFunction(int sampler);
    private static native int nSetCompareFunction(int sampler, int function);

    private static native float nGetAnisotropy(int sampler);
    private static native int nSetAnisotropy(int sampler, float anisotropy);
}
