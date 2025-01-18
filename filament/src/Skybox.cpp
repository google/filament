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

#include "details/Skybox.h"

#include "details/Texture.h"

namespace filament {

void Skybox::setLayerMask(uint8_t const select, uint8_t const values) noexcept {
    downcast(this)->setLayerMask(select, values);
}

uint8_t Skybox::getLayerMask() const noexcept {
    return downcast(this)->getLayerMask();
}

float Skybox::getIntensity() const noexcept {
    return downcast(this)->getIntensity();
}

void Skybox::setColor(math::float4 const color) noexcept {
    downcast(this)->setColor(color);
}

Texture const* Skybox::getTexture() const noexcept {
    return downcast(this)->getTexture();
}

} // namespace filament
