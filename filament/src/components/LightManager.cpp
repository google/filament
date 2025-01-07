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

#include "FilamentAPI-impl.h"

#include "components/LightManager.h"

#include "details/Engine.h"
#include "utils/ostream.h"

#include <filament/LightManager.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Log.h>
#include <utils/ostream.h>

#include <math/fast.h>
#include <math/scalar.h>
#include <math/vec2.h>
#include <math/vec3.h>

#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <cmath>
#include <utility>

using namespace filament::math;
using namespace utils;

namespace filament {

// ------------------------------------------------------------------------------------------------

struct LightManager::BuilderDetails {
    Type mType = Type::DIRECTIONAL;
    bool mCastShadows = false;
    bool mCastLight = true;
    uint8_t mChannels = 1u;
    float3 mPosition = {};
    float mFalloff = 1.0f;
    LinearColor mColor = LinearColor{ 1.0f };
    float mIntensity = 100000.0f;
    FLightManager::IntensityUnit mIntensityUnit = FLightManager::IntensityUnit::LUMEN_LUX;
    float3 mDirection = { 0.0f, -1.0f, 0.0f };
    float2 mSpotInnerOuter = { f::PI_4 * 0.75f, f::PI_4 };
    float mSunAngle = 0.00951f; // 0.545° in radians
    float mSunHaloSize = 10.0f;
    float mSunHaloFalloff = 80.0f;
    ShadowOptions mShadowOptions;

    explicit BuilderDetails(Type type) noexcept : mType(type) { }
    // this is only needed for the explicit instantiation below
    BuilderDetails() = default;
};

using BuilderType = LightManager;
BuilderType::Builder::Builder(Type type) noexcept: BuilderBase<BuilderDetails>(type) {}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(Builder const& rhs) noexcept = default;
BuilderType::Builder::Builder(Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder const& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(Builder&& rhs) noexcept = default;

LightManager::Builder& LightManager::Builder::castShadows(bool enable) noexcept {
    mImpl->mCastShadows = enable;
    return *this;
}

LightManager::Builder& LightManager::Builder::shadowOptions(const ShadowOptions& options) noexcept {
    mImpl->mShadowOptions = options;
    return *this;
}

LightManager::Builder& LightManager::Builder::castLight(bool enable) noexcept {
    mImpl->mCastLight = enable;
    return *this;
}

LightManager::Builder& LightManager::Builder::position(const float3& position) noexcept {
    mImpl->mPosition = position;
    return *this;
}

LightManager::Builder& LightManager::Builder::direction(const float3& direction) noexcept {
    mImpl->mDirection = direction;
    return *this;
}

LightManager::Builder& LightManager::Builder::color(const LinearColor& color) noexcept {
    mImpl->mColor = color;
    return *this;
}

LightManager::Builder& LightManager::Builder::intensity(float intensity) noexcept {
    mImpl->mIntensity = intensity;
    mImpl->mIntensityUnit = FLightManager::IntensityUnit::LUMEN_LUX;
    return *this;
}

LightManager::Builder& LightManager::Builder::intensityCandela(float intensity) noexcept {
    mImpl->mIntensity = intensity;
    mImpl->mIntensityUnit = FLightManager::IntensityUnit::CANDELA;
    return *this;
}

LightManager::Builder& LightManager::Builder::intensity(float watts, float efficiency) noexcept {
    mImpl->mIntensity = efficiency * 683.0f * watts;
    mImpl->mIntensityUnit = FLightManager::IntensityUnit::LUMEN_LUX;
    return *this;
}

LightManager::Builder& LightManager::Builder::falloff(float radius) noexcept {
    mImpl->mFalloff = radius;
    return *this;
}

LightManager::Builder& LightManager::Builder::spotLightCone(float inner, float outer) noexcept {
    mImpl->mSpotInnerOuter = { inner, outer };
    return *this;
}

LightManager::Builder& LightManager::Builder::sunAngularRadius(float sunAngle) noexcept {
    mImpl->mSunAngle = sunAngle;
    return *this;
}

LightManager::Builder& LightManager::Builder::sunHaloSize(float haloSize) noexcept {
    mImpl->mSunHaloSize = haloSize;
    return *this;
}

LightManager::Builder& LightManager::Builder::sunHaloFalloff(float haloFalloff) noexcept {
    mImpl->mSunHaloFalloff = haloFalloff;
    return *this;
}

LightManager::Builder& LightManager::Builder::lightChannel(unsigned int channel, bool enable) noexcept {
    if (channel < 8) {
        const uint8_t mask = 1u << channel;
        mImpl->mChannels &= ~mask;
        mImpl->mChannels |= enable ? mask : 0u;
    }
    return *this;
}

LightManager::Builder::Result LightManager::Builder::build(Engine& engine, Entity entity) {
    downcast(engine).createLight(*this, entity);
    return Success;
}

// ------------------------------------------------------------------------------------------------

FLightManager::FLightManager(FEngine& engine) noexcept : mEngine(engine) {
    // DON'T use engine here in the ctor, because it's not fully constructed yet.
}

FLightManager::~FLightManager() {
    // all components should have been destroyed when we get here
    // (terminate should have been called from Engine's shutdown())
    assert_invariant(mManager.getComponentCount() == 0);
}

void FLightManager::init(FEngine&) noexcept {
}

void FLightManager::create(const Builder& builder, Entity entity) {
    auto& manager = mManager;

    if (UTILS_UNLIKELY(manager.hasComponent(entity))) {
        destroy(entity);
    }
    Instance const i = manager.addComponent(entity);
    assert_invariant(i);

    if (i) {
        // This needs to happen before we call the set() methods below
        // Type must be set first (some calls depend on it below)
        LightType& lightType = manager[i].lightType;
        lightType.type = builder->mType;
        lightType.shadowCaster = builder->mCastShadows;
        lightType.lightCaster = builder->mCastLight;

        mManager[i].channels = builder->mChannels;

        // set default values by calling the setters
        setShadowOptions(i, builder->mShadowOptions);
        setLocalPosition(i, builder->mPosition);
        setLocalDirection(i, builder->mDirection);
        setColor(i, builder->mColor);

        // this must be set before intensity
        setSpotLightCone(i, builder->mSpotInnerOuter.x, builder->mSpotInnerOuter.y);
        setIntensity(i, builder->mIntensity, builder->mIntensityUnit);

        setFalloff(i, builder->mCastLight ? builder->mFalloff : 0);
        setSunAngularRadius(i, builder->mSunAngle);
        setSunHaloSize(i, builder->mSunHaloSize);
        setSunHaloFalloff(i, builder->mSunHaloFalloff);
    }
}

void FLightManager::prepare(backend::DriverApi&) const noexcept {
}

void FLightManager::destroy(Entity e) noexcept {
    Instance const i = getInstance(e);
    if (i) {
        auto& manager = mManager;
        manager.removeComponent(e);
    }
}

void FLightManager::terminate() noexcept {
    auto& manager = mManager;
    if (!manager.empty()) {
#ifndef NDEBUG
        slog.d << "cleaning up " << manager.getComponentCount()
               << " leaked Light components" << io::endl;
#endif
        while (!manager.empty()) {
            Instance const ci = manager.end() - 1;
            manager.removeComponent(manager.getEntity(ci));
        }
    }
}
void FLightManager::gc(EntityManager& em) noexcept {
    mManager.gc(em, [this](Entity e) {
        destroy(e);
    });
}

void FLightManager::setShadowOptions(Instance const i, ShadowOptions const& options) noexcept {
    ShadowParams& params = mManager[i].shadowParams;
    params.options = options;
    params.options.mapSize = clamp(options.mapSize, 8u, 2048u);
    params.options.shadowCascades = clamp<uint8_t>(options.shadowCascades, 1, CONFIG_MAX_SHADOW_CASCADES);
    params.options.constantBias = clamp(options.constantBias, 0.0f, 2.0f);
    params.options.normalBias = clamp(options.normalBias, 0.0f, 3.0f);
    params.options.shadowFar = std::max(options.shadowFar, 0.0f);
    params.options.shadowNearHint = std::max(options.shadowNearHint, 0.0f);
    params.options.shadowFarHint = std::max(options.shadowFarHint, 0.0f);
    params.options.vsm.blurWidth = std::max(0.0f, options.vsm.blurWidth);
}

void FLightManager::setLightChannel(Instance i, unsigned int channel, bool enable) noexcept {
    if (i) {
        if (channel < 8) {
            auto& manager = mManager;
            const uint8_t mask = 1u << channel;
            manager[i].channels &= ~mask;
            manager[i].channels |= enable ? mask : 0u;
        }
    }
}

bool FLightManager::getLightChannel(Instance i, unsigned int channel) const noexcept {
    if (i) {
        if (channel < 8) {
            auto& manager = mManager;
            const uint8_t mask = 1u << channel;
            return bool(manager[i].channels & mask);
        }
    }
    return false;
}

void FLightManager::setLocalPosition(Instance i, const float3& position) noexcept {
    if (i) {
        auto& manager = mManager;
        manager[i].position = position;
    }
}

void FLightManager::setLocalDirection(Instance i, float3 direction) noexcept {
    if (i) {
        auto& manager = mManager;
        manager[i].direction = direction;
    }
}

void FLightManager::setColor(Instance i, const LinearColor& color) noexcept {
    if (i) {
        auto& manager = mManager;
        manager[i].color = color;
    }
}

void FLightManager::setIntensity(Instance i, float intensity, IntensityUnit unit) noexcept {
    auto& manager = mManager;
    if (i) {
        Type const type = getLightType(i).type;
        float luminousPower = intensity;
        float luminousIntensity = 0.0f;
        switch (type) {
            case Type::SUN:
            case Type::DIRECTIONAL:
                // luminousPower is in lux, nothing to do.
                luminousIntensity = luminousPower;
                break;

            case Type::POINT:
                if (unit == IntensityUnit::LUMEN_LUX) {
                    // li = lp / (4 * pi)
                    luminousIntensity = luminousPower * f::ONE_OVER_PI * 0.25f;
                } else {
                    assert_invariant(unit == IntensityUnit::CANDELA);
                    // intensity specified directly in candela, no conversion needed
                    luminousIntensity = luminousPower;
                }
                break;

            case Type::FOCUSED_SPOT: {
                SpotParams& spotParams = manager[i].spotParams;
                float const cosOuter = std::sqrt(spotParams.cosOuterSquared);
                if (unit == IntensityUnit::LUMEN_LUX) {
                    // li = lp / (2 * pi * (1 - cos(cone_outer / 2)))
                    luminousIntensity = luminousPower / (f::TAU * (1.0f - cosOuter));
                } else {
                    assert_invariant(unit == IntensityUnit::CANDELA);
                    // intensity specified directly in candela, no conversion needed
                    luminousIntensity = luminousPower;
                    // lp = li * (2 * pi * (1 - cos(cone_outer / 2)))
                    luminousPower = luminousIntensity * (f::TAU * (1.0f - cosOuter));
                }
                spotParams.luminousPower = luminousPower;
                break;
            }
            case Type::SPOT:
                if (unit == IntensityUnit::LUMEN_LUX) {
                    // li = lp / pi
                    luminousIntensity = luminousPower * f::ONE_OVER_PI;
                } else {
                    assert_invariant(unit == IntensityUnit::CANDELA);
                    // intensity specified directly in Candela, no conversion needed
                    luminousIntensity = luminousPower;
                }
                break;
        }
        manager[i].intensity = luminousIntensity;
    }
}

void FLightManager::setFalloff(Instance i, float falloff) noexcept {
    auto& manager = mManager;
    if (i && !isDirectionalLight(i)) {
        float const sqFalloff = falloff * falloff;
        SpotParams& spotParams = manager[i].spotParams;
        manager[i].squaredFallOffInv = sqFalloff > 0.0f ? (1 / sqFalloff) : 0;
        spotParams.radius = falloff;
    }
}

void FLightManager::setSpotLightCone(Instance i, float inner, float outer) noexcept {
    auto& manager = mManager;
    if (i && isSpotLight(i)) {
        // clamp the inner/outer angles to [0.5 degrees, 90 degrees]
        float innerClamped = std::clamp(std::abs(inner), 0.5f * f::DEG_TO_RAD, f::PI_2);
        float const outerClamped = std::clamp(std::abs(outer), 0.5f * f::DEG_TO_RAD, f::PI_2);

        // inner must always be smaller than outer
        innerClamped = std::min(innerClamped, outerClamped);

        float const cosOuter = fast::cos(outerClamped);
        float const cosInner = fast::cos(innerClamped);
        float const cosOuterSquared = cosOuter * cosOuter;
        float const scale = 1.0f / std::max(1.0f / 1024.0f, cosInner - cosOuter);
        float const offset = -cosOuter * scale;

        SpotParams& spotParams = manager[i].spotParams;
        spotParams.outerClamped = outerClamped;
        spotParams.cosOuterSquared = cosOuterSquared;
        spotParams.sinInverse = 1.0f / std::sin(outerClamped);
        spotParams.scaleOffset = { scale, offset };

        // we need to recompute the luminous intensity
        Type const type = getLightType(i).type;
        if (type == Type::FOCUSED_SPOT) {
            // li = lp / (2 * pi * (1 - cos(cone_outer / 2)))
            float const luminousPower = spotParams.luminousPower;
            float const luminousIntensity = luminousPower / (f::TAU * (1.0f - cosOuter));
            manager[i].intensity = luminousIntensity;
        }
    }
}

void FLightManager::setSunAngularRadius(Instance i, float angularRadius) noexcept {
    if (i && isSunLight(i)) {
        angularRadius = clamp(angularRadius, 0.25f, 20.0f);
        mManager[i].sunAngularRadius = angularRadius * f::DEG_TO_RAD;
    }
}

void FLightManager::setSunHaloSize(Instance i, float haloSize) noexcept {
    if (i && isSunLight(i)) {
        mManager[i].sunHaloSize = haloSize;
    }
}

void FLightManager::setSunHaloFalloff(Instance i, float haloFalloff) noexcept {
    if (i && isSunLight(i)) {
        mManager[i].sunHaloFalloff = haloFalloff;
    }
}

void FLightManager::setShadowCaster(Instance i, bool shadowCaster) noexcept {
    if (i) {
        LightType& lightType = mManager[i].lightType;
        lightType.shadowCaster = shadowCaster;
    }
}

float FLightManager::getSpotLightInnerCone(Instance i) const noexcept {
    const auto& spotParams = getSpotParams(i);
    float const cosOuter = std::cos(spotParams.outerClamped);
    float const scale = spotParams.scaleOffset.x;
    float const inner = std::acos((1.0f / scale) + cosOuter);
    return inner;
}

// ------------------------------------------------------------------------------------------------
// ShadowCascades utility methods
// ------------------------------------------------------------------------------------------------

void LightManager::ShadowCascades::computeUniformSplits(float splitPositions[3], uint8_t cascades) {
    size_t s = 0;
    cascades = min(cascades, (uint8_t) 4u);
    for (size_t c = 1; c < cascades; c++) {
        splitPositions[s++] = float(c) / float(cascades);
    }
}

void LightManager::ShadowCascades::computeLogSplits(float splitPositions[3], uint8_t cascades,
        float near, float far) {
    size_t s = 0;
    cascades = min(cascades, (uint8_t) 4u);
    for (size_t c = 1; c < cascades; c++) {
        splitPositions[s++] =
            (near * std::pow(far / near, float(c) / float(cascades)) - near) / (far - near);
    }
}

void LightManager::ShadowCascades::computePracticalSplits(float splitPositions[3], uint8_t cascades,
        float near, float far, float lambda) {
    float uniformSplits[3];
    float logSplits[3];
    cascades = min(cascades, (uint8_t) 4u);
    computeUniformSplits(uniformSplits, cascades);
    computeLogSplits(logSplits, cascades, near, far);
    size_t s = 0;
    for (size_t c = 1; c < cascades; c++) {
        splitPositions[s] = lambda * logSplits[s] + (1.0f - lambda) * uniformSplits[s];
        s++;
    }
}

} // namespace filament
