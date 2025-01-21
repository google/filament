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

#include "details/Texture.h"

namespace filament {

using namespace math;

void IndirectLight::setIntensity(float const intensity) noexcept {
    downcast(this)->setIntensity(intensity);
}

float IndirectLight::getIntensity() const noexcept {
    return downcast(this)->getIntensity();
}

void IndirectLight::setRotation(mat3f const& rotation) noexcept {
    downcast(this)->setRotation(rotation);
}

const mat3f& IndirectLight::getRotation() const noexcept {
    return downcast(this)->getRotation();
}

Texture const* IndirectLight::getReflectionsTexture() const noexcept {
    return downcast(this)->getReflectionsTexture();
}

Texture const* IndirectLight::getIrradianceTexture() const noexcept {
    return downcast(this)->getIrradianceTexture();
}

float3 IndirectLight::getDirectionEstimate() const noexcept {
    return downcast(this)->getDirectionEstimate();
}

float4 IndirectLight::getColorEstimate(float3 const direction) const noexcept {
    return downcast(this)->getColorEstimate(direction);
}

float3 IndirectLight::getDirectionEstimate(const float3* sh) noexcept {
    return FIndirectLight::getDirectionEstimate(sh);
}

float4 IndirectLight::getColorEstimate(const float3* sh, float3 const direction) noexcept {
    return FIndirectLight::getColorEstimate(sh, direction);
}

} // namespace filament
