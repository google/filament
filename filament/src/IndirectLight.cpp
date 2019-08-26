/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "details/IndirectLight.h"

#include "details/Engine.h"
#include "details/Texture.h"

#include "FilamentAPI-impl.h"

#include <utils/Panic.h>

#include <backend/DriverEnums.h>
#include <filament/IndirectLight.h>
#include <utils/Log.h>

#include <math/scalar.h>

#define IBL_INTEGRATION_PREFILTERED_CUBEMAP         0
#define IBL_INTEGRATION_IMPORTANCE_SAMPLING         1
#define IBL_INTEGRATION                             IBL_INTEGRATION_PREFILTERED_CUBEMAP

using namespace filament::math;

namespace filament {

using namespace details;

// ------------------------------------------------------------------------------------------------

struct IndirectLight::BuilderDetails {
    Texture const* mReflectionsMap = nullptr;
    Texture const* mIrradianceMap = nullptr;
    float3 mIrradianceCoefs[9] = {};
    mat3f mRotation = {};
    float mIntensity = 30000.0f;
};

using BuilderType = IndirectLight;
BuilderType::Builder::Builder() noexcept = default;
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;

IndirectLight::Builder& IndirectLight::Builder::reflections(Texture const* cubemap) noexcept {
    mImpl->mReflectionsMap = cubemap;
    return *this;
}

IndirectLight::Builder& IndirectLight::Builder::irradiance(uint8_t bands, float3 const* sh) noexcept {
    // clamp to 3 bands for now
    bands = std::min(bands, uint8_t(3));
    size_t numCoefs = bands * bands;
    std::fill(std::begin(mImpl->mIrradianceCoefs), std::end(mImpl->mIrradianceCoefs), 0.0f);
    std::copy_n(sh, numCoefs, std::begin(mImpl->mIrradianceCoefs));
    return *this;
}

IndirectLight::Builder& IndirectLight::Builder::radiance(uint8_t bands, float3 const* sh) noexcept {
    // Coefficient for the polynomial form of the SH functions -- these were taken from
    // "Stupid Spherical Harmonics (SH)" by Peter-Pike Sloan
    // They simply come for expanding the computation of each SH function.
    //
    // To render spherical harmonics we can use the polynomial form, like this:
    //          c += sh[0] * A[0];
    //          c += sh[1] * A[1] * s.y;
    //          c += sh[2] * A[2] * s.z;
    //          c += sh[3] * A[3] * s.x;
    //          c += sh[4] * A[4] * s.y * s.x;
    //          c += sh[5] * A[5] * s.y * s.z;
    //          c += sh[6] * A[6] * (3 * s.z * s.z - 1);
    //          c += sh[7] * A[7] * s.z * s.x;
    //          c += sh[8] * A[8] * (s.x * s.x - s.y * s.y);
    //
    // To save math in the shader, we pre-multiply our SH coefficient by the A[i] factors.
    // Additionally, we include the lambertian diffuse BRDF 1/pi and truncated cos.

    constexpr float M_SQRT_PI = 1.7724538509f;
    constexpr float M_SQRT_3  = 1.7320508076f;
    constexpr float M_SQRT_5  = 2.2360679775f;
    constexpr float M_SQRT_15 = 3.8729833462f;
    constexpr float C[] = { M_PI, 2.0943951f, 0.785398f }; // <cos>
    constexpr float A[] = {
                  1.0f / (2.0f * M_SQRT_PI) * C[0] * M_1_PI,    // 0  0
            -M_SQRT_3  / (2.0f * M_SQRT_PI) * C[1] * M_1_PI,    // 1 -1
             M_SQRT_3  / (2.0f * M_SQRT_PI) * C[1] * M_1_PI,    // 1  0
            -M_SQRT_3  / (2.0f * M_SQRT_PI) * C[1] * M_1_PI,    // 1  1
             M_SQRT_15 / (2.0f * M_SQRT_PI) * C[2] * M_1_PI,    // 2 -2
            -M_SQRT_15 / (2.0f * M_SQRT_PI) * C[2] * M_1_PI,    // 3 -1
             M_SQRT_5  / (4.0f * M_SQRT_PI) * C[2] * M_1_PI,    // 3  0
            -M_SQRT_15 / (2.0f * M_SQRT_PI) * C[2] * M_1_PI,    // 3  1
             M_SQRT_15 / (4.0f * M_SQRT_PI) * C[2] * M_1_PI     // 3  2
    };

    // this is a way to "document" the actual value of these coefficients and at the same
    // time make sure the expression and values are always in sync.
    struct Debug {
        static constexpr bool almost(float a, float b) {
            constexpr float e = 1e-6f;
            return (a > b - e) && (a < b + e);
        }
    };
    static_assert(Debug::almost(A[0],  0.282095f), "coefficient mismatch");
    static_assert(Debug::almost(A[1], -0.325735f), "coefficient mismatch");
    static_assert(Debug::almost(A[2],  0.325735f), "coefficient mismatch");
    static_assert(Debug::almost(A[3], -0.325735f), "coefficient mismatch");
    static_assert(Debug::almost(A[4],  0.273137f), "coefficient mismatch");
    static_assert(Debug::almost(A[5], -0.273137f), "coefficient mismatch");
    static_assert(Debug::almost(A[6],  0.078848f), "coefficient mismatch");
    static_assert(Debug::almost(A[7], -0.273137f), "coefficient mismatch");
    static_assert(Debug::almost(A[8],  0.136569f), "coefficient mismatch");

    float3 irradiance[9];
    bands = std::min(bands, uint8_t(3));
    for (size_t i = 0, c = bands * bands; i<c; ++i) {
        irradiance[i] = sh[i] * A[i];
    }
    return this->irradiance(bands, irradiance);
}

IndirectLight::Builder& IndirectLight::Builder::irradiance(Texture const* cubemap) noexcept {
    mImpl->mIrradianceMap = cubemap;
    return *this;
}

IndirectLight::Builder& IndirectLight::Builder::intensity(float envIntensity) noexcept {
    mImpl->mIntensity = envIntensity;
    return *this;
}

IndirectLight::Builder& IndirectLight::Builder::rotation(mat3f const& rotation) noexcept {
    mImpl->mRotation = rotation;
    return *this;
}

IndirectLight* IndirectLight::Builder::build(Engine& engine) {
    if (mImpl->mReflectionsMap) {
        if (!ASSERT_POSTCONDITION_NON_FATAL(
                mImpl->mReflectionsMap->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP,
                "reflection map must a cubemap")) {
            return nullptr;
        }

        if (!ASSERT_POSTCONDITION_NON_FATAL(mImpl->mReflectionsMap->getLevels() ==
                upcast(mImpl->mReflectionsMap)->getMaxLevelCount(),
                "reflection map must have %u mipmap levels",
                upcast(mImpl->mReflectionsMap)->getMaxLevelCount())) {
            return nullptr;
        }
        if (IBL_INTEGRATION == IBL_INTEGRATION_IMPORTANCE_SAMPLING) {
            mImpl->mReflectionsMap->generateMipmaps(engine);
        }
    }

    if (mImpl->mIrradianceMap) {
        if (!ASSERT_POSTCONDITION_NON_FATAL(
                mImpl->mIrradianceMap->getTarget() == Texture::Sampler::SAMPLER_CUBEMAP,
                "irradiance map must a cubemap")) {
            return nullptr;
        }
    }

    return upcast(engine).createIndirectLight(*this);
}

// ------------------------------------------------------------------------------------------------

namespace details {

FIndirectLight::FIndirectLight(FEngine& engine, const Builder& builder) noexcept {
    if (builder->mReflectionsMap) {
        mReflectionsMapHandle = upcast(builder->mReflectionsMap)->getHwHandle();
        mMaxMipLevel = builder->mReflectionsMap->getLevels();
    }

    std::copy(
            std::begin(builder->mIrradianceCoefs),
            std::end(builder->mIrradianceCoefs),
            mIrradianceCoefs.begin());

    mRotation = builder->mRotation;
    mIntensity = builder->mIntensity;
    if (builder->mIrradianceMap) {
        mIrradianceMapHandle = upcast(builder->mIrradianceMap)->getHwHandle();
    } else {
        // TODO: if needed, generate the irradiance map, this is an engine config
        if (FEngine::CONFIG_IBL_USE_IRRADIANCE_MAP) {
        }
    }
}

void FIndirectLight::terminate(FEngine& engine) {
    if (FEngine::CONFIG_IBL_USE_IRRADIANCE_MAP) {
        FEngine::DriverApi& driver = engine.getDriverApi();
        driver.destroyTexture(mIrradianceMapHandle);
    }
}

math::float3 FIndirectLight::getDirectionEstimate() const noexcept {
    auto const& f = mIrradianceCoefs;
    // The linear direction is found as normalize(-sh[3], -sh[1], sh[2]), but the coefficients
    // we store are already pre-normalized, so the negative sign disappears.
    // Note: we normalize the directions only after blending, this matches code used elsewhere --
    // the length of the vector is somewhat related to the intensity in that direction.
    float3 r = float3{ f[3].r, f[1].r, f[2].r };
    float3 g = float3{ f[3].g, f[1].g, f[2].g };
    float3 b = float3{ f[3].b, f[1].b, f[2].b };
    // We're assuming there is a single white light.
    return -normalize(r * 0.2126f + g * 0.7152f + b * 0.0722f);
}

float4 FIndirectLight::getColorEstimate(float3 direction) const noexcept {
    // See: https://www.gamasutra.com/view/news/129689/Indepth_Extracting_dominant_light_from_Spherical_Harmonics.php

    // first get the direction
    const float3 s = -direction;

    // The light intensity on one channel is given by: dot(Ld, Le) / dot(Ld, Ld)

    // our pre-convolved, pre-scaled SH coefficients for the environment
    auto const& Le = mIrradianceCoefs;

    // SH coefficients of the directional light pre-scaled by 1/A[i]
    // (we pre-scale by 1/A[i] to undo Le's pre-scaling by A[i]
    const float Ld[9] = {
            1.0f,
            s.y, s.z, s.x,
            s.y * s.x, s.y * s.z, (3 * s.z * s.z - 1), s.z * s.x, (s.x * s.x - s.y * s.y)
    };

    // dot(Ld, Le) -- notice that this is equivalent to "sampling" the sphere in the light
    // direction; this is the exact same code used in the shader for SH reconstruction.
    float3 LdDotLe = Ld[0] * Le[0]
                   + Ld[1] * Le[1] + Ld[2] * Le[2] + Ld[3] * Le[3]
                   + Ld[4] * Le[4] + Ld[5] * Le[5] + Ld[6] * Le[6] + Ld[7] * Le[7] + Ld[8] * Le[8];

    // The scale factor below is explained in the gamasutra article above, however it seems
    // to cause the intensity of the light to be too low.
    //      constexpr float c = (16.0f * M_PI / 17.0f);
    //      constexpr float LdSquared = (9.0f / (4.0f * M_PI)) * c * c;
    //      LdDotLe *= c / LdSquared; // Note the final coefficient is 17/36

    // We multiply by PI because our SH coefficients contain the 1/PI lambertian BRDF.
    LdDotLe *= M_PI;

    // Make sure we don't have negative intensities
    LdDotLe = max(LdDotLe, float3{0});

    const float intensity = max(LdDotLe);
    return { LdDotLe / intensity, intensity };
}

} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

void IndirectLight::setIntensity(float intensity) noexcept {
    upcast(this)->setIntensity(intensity);
}

float IndirectLight::getIntensity() const noexcept {
    return upcast(this)->getIntensity();
}

void IndirectLight::setRotation(mat3f const& rotation) noexcept {
    upcast(this)->setRotation(rotation);
}

const math::mat3f& IndirectLight::getRotation() const noexcept {
    return upcast(this)->getRotation();
}

math::float3 IndirectLight::getDirectionEstimate() const noexcept {
    return upcast(this)->getDirectionEstimate();
}

math::float4 IndirectLight::getColorEstimate(math::float3 direction) const noexcept {
    return upcast(this)->getColorEstimate(direction);
}

} // namespace filament
