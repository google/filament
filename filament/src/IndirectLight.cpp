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


} // namespace filament
