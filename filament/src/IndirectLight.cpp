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
    float3 irradiance[9];
    if (bands >= 1) {
        irradiance[0] = sh[0] * 0.282095f;
        if (bands >= 2) {
            irradiance[1] = sh[1] * -0.325735f;
            irradiance[2] = sh[2] *  0.325735f;
            irradiance[3] = sh[3] * -0.325735f;
            if (bands >= 3) {
                irradiance[4] = sh[4] *  0.273137f;
                irradiance[5] = sh[5] * -0.273137f;
                irradiance[6] = sh[6] *  0.078848f;
                irradiance[7] = sh[7] * -0.273137f;
                irradiance[8] = sh[8] *  0.136569f;
            }
        }
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
