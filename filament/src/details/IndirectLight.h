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

#ifndef TNT_FILAMENT_DETAILS_INDIRECT_LIGHT_H
#define TNT_FILAMENT_DETAILS_INDIRECT_LIGHT_H

#include "upcast.h"

#include <backend/Handle.h>

#include <filament/IndirectLight.h>
#include <filament/Texture.h>

#include <utils/compiler.h>

#include <array>

namespace filament {
namespace details {

class FEngine;

class FIndirectLight : public IndirectLight {
public:
    static constexpr float DEFAULT_INTENSITY = 30000.0f;    // lux

    FIndirectLight(FEngine& engine, const Builder& builder) noexcept;

    void terminate(FEngine& engine);

    backend::Handle<backend::HwTexture> getReflectionMap() const noexcept { return mReflectionsMapHandle; }
    backend::Handle<backend::HwTexture> getIrradianceMap() const noexcept { return mIrradianceMapHandle; }
    math::float3 const* getSH() const noexcept{ return mIrradianceCoefs.data(); }
    float getIntensity() const noexcept { return mIntensity; }
    void setIntensity(float intensity) noexcept { mIntensity = intensity; }
    void setRotation(math::mat3f const& rotation) noexcept { mRotation = rotation; }
    const math::mat3f& getRotation() const noexcept { return mRotation; }
    size_t getMaxMipLevel() const noexcept { return mMaxMipLevel; }
    math::float3 getDirectionEstimate() const noexcept;
    math::float4 getColorEstimate(math::float3 direction) const noexcept;

private:
    backend::Handle<backend::HwTexture> mReflectionsMapHandle;
    backend::Handle<backend::HwTexture> mIrradianceMapHandle;
    std::array<math::float3, 9> mIrradianceCoefs;
    float mIntensity = DEFAULT_INTENSITY;
    math::mat3f mRotation;
    uint8_t mMaxMipLevel = 0;
};

FILAMENT_UPCAST(IndirectLight)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_INDIRECT_LIGHT_H
