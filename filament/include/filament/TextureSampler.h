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

#ifndef TNT_FILAMENT_TEXTURESAMPLER_H
#define TNT_FILAMENT_TEXTURESAMPLER_H

#include <filament/driver/DriverEnums.h>

#include <utils/compiler.h>

#include <math.h>

namespace filament {

class UTILS_PUBLIC TextureSampler {
public:
    using WrapMode = driver::SamplerWrapMode;
    using MinFilter = driver::SamplerMinFilter;
    using MagFilter = driver::SamplerMagFilter;
    using CompareMode = driver::SamplerCompareMode;
    using CompareFunc = driver::SamplerCompareFunc;

    TextureSampler() noexcept = default;

    explicit TextureSampler(MagFilter minMag, WrapMode str = WrapMode::CLAMP_TO_EDGE) noexcept  {
        mSamplerParams.filterMin = MinFilter(minMag);
        mSamplerParams.filterMag = minMag;
        mSamplerParams.wrapS = str;
        mSamplerParams.wrapT = str;
        mSamplerParams.wrapR = str;
    }

    TextureSampler(MinFilter min, MagFilter mag, WrapMode str = WrapMode::CLAMP_TO_EDGE) noexcept  {
        mSamplerParams.filterMin = min;
        mSamplerParams.filterMag = mag;
        mSamplerParams.wrapS = str;
        mSamplerParams.wrapT = str;
        mSamplerParams.wrapR = str;
    }

    TextureSampler(MinFilter min, MagFilter mag, WrapMode s, WrapMode t, WrapMode r) noexcept  {
        mSamplerParams.filterMin = min;
        mSamplerParams.filterMag = mag;
        mSamplerParams.wrapS = s;
        mSamplerParams.wrapT = t;
        mSamplerParams.wrapR = r;
    }

    explicit TextureSampler(CompareMode mode, CompareFunc func = CompareFunc::LE) noexcept  {
        mSamplerParams.compareMode = mode;
        mSamplerParams.compareFunc = func;
    }

    TextureSampler(const TextureSampler& rhs) noexcept = default;
    TextureSampler& operator=(const TextureSampler& rhs) noexcept = default;

    void setMinFilter(MinFilter v) noexcept {
        mSamplerParams.filterMin = v;
    }
    void setMagFilter(MagFilter v) noexcept {
        mSamplerParams.filterMag = v;
    }
    void setWrapModeS(WrapMode v) noexcept {
        mSamplerParams.wrapS = v;
    }
    void setWrapModeT(WrapMode v) noexcept {
        mSamplerParams.wrapT = v;
    }
    void setWrapModeR(WrapMode v) noexcept {
        mSamplerParams.wrapR = v;
    }

    // Amount of anisotropy, should be a power-of-two.
    void setAnisotropy(float anisotropy) noexcept {
        const int log2 = ilogbf(fabsf(anisotropy));
        mSamplerParams.anisotropyLog2 = uint8_t(log2 < 7 ? log2 : 7);
    }

    void setCompareMode(CompareMode mode, CompareFunc func = CompareFunc::LE) noexcept {
        mSamplerParams.compareMode = mode;
        mSamplerParams.compareFunc = func;
    }

    driver::SamplerParams getSamplerParams() const noexcept  { return mSamplerParams; }

    MinFilter getMinFilter() const noexcept { return mSamplerParams.filterMin; }
    MagFilter getMagFilter() const noexcept { return mSamplerParams.filterMag; }
    WrapMode getWrapModeS() const noexcept  { return mSamplerParams.wrapS; }
    WrapMode getWrapModeT() const noexcept  { return mSamplerParams.wrapT; }
    WrapMode getWrapModeR() const noexcept  { return mSamplerParams.wrapR; }
    float getAnisotropy() const noexcept { return float(1 << mSamplerParams.anisotropyLog2); }
    CompareMode getCompareMode() const noexcept { return mSamplerParams.compareMode; }
    CompareFunc getCompareFunc() const noexcept { return mSamplerParams.compareFunc; }


private:
    driver::SamplerParams mSamplerParams;
};

} // namespace filament

#endif // TNT_FILAMENT_TEXTURESAMPLER_H
