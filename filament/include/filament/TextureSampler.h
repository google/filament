/*
 * Copyright (C) 2015 The Android Open Source Project
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

//! \file

#ifndef TNT_FILAMENT_TEXTURESAMPLER_H
#define TNT_FILAMENT_TEXTURESAMPLER_H

#include <backend/DriverEnums.h>

#include <utils/compiler.h>

#include <math.h>

namespace filament {

/**
 * TextureSampler defines how a texture is accessed.
 */
class UTILS_PUBLIC TextureSampler {
public:
    using WrapMode = backend::SamplerWrapMode;
    using MinFilter = backend::SamplerMinFilter;
    using MagFilter = backend::SamplerMagFilter;
    using CompareMode = backend::SamplerCompareMode;
    using CompareFunc = backend::SamplerCompareFunc;

    /**
     * Creates a default sampler.
     * The default parameters are:
     * - filterMag      : NEAREST
     * - filterMin      : NEAREST
     * - wrapS          : CLAMP_TO_EDGE
     * - wrapT          : CLAMP_TO_EDGE
     * - wrapR          : CLAMP_TO_EDGE
     * - compareMode    : NONE
     * - compareFunc    : Less or equal
     * - no anisotropic filtering
     */
    TextureSampler() noexcept = default;

    TextureSampler(const TextureSampler& rhs) noexcept = default;
    TextureSampler& operator=(const TextureSampler& rhs) noexcept = default;

    /**
     * Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.
     * @param minMag filtering for both minification and magnification
     * @param str wrapping mode for all texture coordinate axes
     */
    explicit TextureSampler(MagFilter minMag, WrapMode str = WrapMode::CLAMP_TO_EDGE) noexcept  {
        mSamplerParams.filterMin = MinFilter(minMag);
        mSamplerParams.filterMag = minMag;
        mSamplerParams.wrapS = str;
        mSamplerParams.wrapT = str;
        mSamplerParams.wrapR = str;
    }

    /**
     * Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.
     * @param min filtering for minification
     * @param mag filtering for magnification
     * @param str wrapping mode for all texture coordinate axes
     */
    TextureSampler(MinFilter min, MagFilter mag, WrapMode str = WrapMode::CLAMP_TO_EDGE) noexcept  {
        mSamplerParams.filterMin = min;
        mSamplerParams.filterMag = mag;
        mSamplerParams.wrapS = str;
        mSamplerParams.wrapT = str;
        mSamplerParams.wrapR = str;
    }

    /**
     * Creates a TextureSampler with the default parameters but setting the filtering and wrap modes.
     * @param min filtering for minification
     * @param mag filtering for magnification
     * @param s wrap mode for the s (horizontal)texture coordinate
     * @param t wrap mode for the t (vertical) texture coordinate
     * @param r wrap mode for the r (depth) texture coordinate
     */
    TextureSampler(MinFilter min, MagFilter mag, WrapMode s, WrapMode t, WrapMode r) noexcept  {
        mSamplerParams.filterMin = min;
        mSamplerParams.filterMag = mag;
        mSamplerParams.wrapS = s;
        mSamplerParams.wrapT = t;
        mSamplerParams.wrapR = r;
    }

    /**
     * Creates a TextureSampler with the default parameters but setting the compare mode and function
     * @param mode Compare mode
     * @param func Compare function
     */
    explicit TextureSampler(CompareMode mode, CompareFunc func = CompareFunc::LE) noexcept  {
        mSamplerParams.compareMode = mode;
        mSamplerParams.compareFunc = func;
    }

    /**
     * Sets the minification filter
     * @param v Minification filter
     */
    void setMinFilter(MinFilter v) noexcept {
        mSamplerParams.filterMin = v;
    }

    /**
     * Sets the magnification filter
     * @param v Magnification filter
     */
    void setMagFilter(MagFilter v) noexcept {
        mSamplerParams.filterMag = v;
    }

    /**
     * Sets the wrap mode for the s (horizontal) texture coordinate
     * @param v wrap mode
     */
    void setWrapModeS(WrapMode v) noexcept {
        mSamplerParams.wrapS = v;
    }

    /**
     * Sets the wrap mode for the t (vertical) texture coordinate
     * @param v wrap mode
     */
    void setWrapModeT(WrapMode v) noexcept {
        mSamplerParams.wrapT = v;
    }

    /**
     * Sets the wrap mode for the r (depth, for 3D textures) texture coordinate
     * @param v wrap mode
     */
    void setWrapModeR(WrapMode v) noexcept {
        mSamplerParams.wrapR = v;
    }

    /**
     * This controls anisotropic filtering.
     * @param anisotropy Amount of anisotropy, should be a power-of-two. The default is 0.
     *                   The maximum permissible value is 7.
     */
    void setAnisotropy(float anisotropy) noexcept {
        const int log2 = ilogbf(anisotropy > 0 ? anisotropy : -anisotropy);
        mSamplerParams.anisotropyLog2 = uint8_t(log2 < 7 ? log2 : 7);
    }

    /**
     * Sets the compare mode and function.
     * @param mode Compare mode
     * @param func Compare function
     */
    void setCompareMode(CompareMode mode, CompareFunc func = CompareFunc::LE) noexcept {
        mSamplerParams.compareMode = mode;
        mSamplerParams.compareFunc = func;
    }

    //! returns the minification filter value
    MinFilter getMinFilter() const noexcept { return mSamplerParams.filterMin; }

    //! returns the magnification filter value
    MagFilter getMagFilter() const noexcept { return mSamplerParams.filterMag; }

    //! returns the s-coordinate wrap mode (horizontal)
    WrapMode getWrapModeS() const noexcept  { return mSamplerParams.wrapS; }

    //! returns the t-coordinate wrap mode (vertical)
    WrapMode getWrapModeT() const noexcept  { return mSamplerParams.wrapT; }

    //! returns the r-coordinate wrap mode (depth)
    WrapMode getWrapModeR() const noexcept  { return mSamplerParams.wrapR; }

    //! returns the anisotropy value
    float getAnisotropy() const noexcept { return float(1u << mSamplerParams.anisotropyLog2); }

    //! returns the compare mode
    CompareMode getCompareMode() const noexcept { return mSamplerParams.compareMode; }

    //! returns the compare function
    CompareFunc getCompareFunc() const noexcept { return mSamplerParams.compareFunc; }


    // no user-serviceable parts below...
    backend::SamplerParams getSamplerParams() const noexcept  { return mSamplerParams; }

private:
    backend::SamplerParams mSamplerParams{};
};

} // namespace filament

#endif // TNT_FILAMENT_TEXTURESAMPLER_H
