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

import android.support.annotation.NonNull;

public class TextureSampler {
    public enum WrapMode {
        CLAMP_TO_EDGE,
        REPEAT,
        MIRRORED_REPEAT
    }

    public enum MinFilter {
        NEAREST,
        LINEAR,
        NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR,
    }

    public enum MagFilter {
        NEAREST,
        LINEAR
    }

    public enum CompareMode {
        NONE,
        COMPARE_TO_TEXTURE
    }

    public enum CompareFunction {
        LESS_EQUAL,
        GREATER_EQUAL,
        LESS,
        GREATER,
        EQUAL,
        NOT_EQUAL,
        ALWAYS,
        NEVER
    }

    int mSampler = 0; // bit field used by native

    /**
     * Min filter: LINEAR_MIPMAP_LINEAR
     * Mag filter: LINEAR
     * Wrap mode: REPEAT
     */
    public TextureSampler() {
        this(MinFilter.LINEAR_MIPMAP_LINEAR, MagFilter.LINEAR, WrapMode.REPEAT);
    }

    public TextureSampler(@NonNull MagFilter minMag) {
        this(minMag, WrapMode.CLAMP_TO_EDGE);
    }

    public TextureSampler(@NonNull MagFilter minMag, @NonNull WrapMode wrap) {
        this(minFilterFromMagFilter(minMag), minMag, wrap);
    }

    public TextureSampler(@NonNull MinFilter min, @NonNull MagFilter mag, @NonNull WrapMode wrap) {
        this(min, mag, wrap, wrap, wrap);
    }

    public TextureSampler(@NonNull MinFilter min, @NonNull MagFilter mag,
            @NonNull WrapMode s, @NonNull WrapMode t, @NonNull WrapMode r) {
        mSampler = nCreateSampler(min.ordinal(), mag.ordinal(),
                s.ordinal(), t.ordinal(), r.ordinal());
    }

    public TextureSampler(@NonNull CompareMode mode) {
        this(mode, CompareFunction.LESS_EQUAL);
    }

    public TextureSampler(@NonNull CompareMode mode, @NonNull CompareFunction function) {
        mSampler = nCreateCompareSampler(mode.ordinal(), function.ordinal());
    }

    public MinFilter getMinFilter() {
        return MinFilter.values()[nGetMinFilter(mSampler)];
    }

    public void setMinFilter(MinFilter filter) {
        mSampler = nSetMinFilter(mSampler, filter.ordinal());
    }

    public MagFilter getMagFilter() {
        return MagFilter.values()[nGetMagFilter(mSampler)];
    }

    public void setMagFilter(MagFilter filter) {
        mSampler = nSetMagFilter(mSampler, filter.ordinal());
    }

    public WrapMode getWrapModeS() {
        return WrapMode.values()[nGetWrapModeS(mSampler)];
    }

    public void setWrapModeS(WrapMode mode) {
        mSampler = nSetWrapModeS(mSampler, mode.ordinal());
    }

    public WrapMode getWrapModeT() {
        return WrapMode.values()[nGetWrapModeT(mSampler)];
    }

    public void setWrapModeT(WrapMode mode) {
        mSampler = nSetWrapModeT(mSampler, mode.ordinal());
    }

    public WrapMode getWrapModeR() {
        return WrapMode.values()[nGetWrapModeR(mSampler)];
    }

    public void setWrapModeR(WrapMode mode) {
        mSampler = nSetWrapModeR(mSampler, mode.ordinal());
    }

    public float getAnisotropy() {
        return nGetAnisotropy(mSampler);
    }

    public void setAnisotropy(float anisotropy) {
        mSampler = nSetAnisotropy(mSampler, anisotropy);
    }

    public CompareMode getCompareMode() {
        return CompareMode.values()[nGetCompareMode(mSampler)];
    }

    public void setCompareMode(CompareMode mode) {
        mSampler = nSetCompareMode(mSampler, mode.ordinal());
    }

    public CompareFunction getCompareFunction() {
        return CompareFunction.values()[nGetCompareFunction(mSampler)];
    }

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
