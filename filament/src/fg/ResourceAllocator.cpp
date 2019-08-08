/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "ResourceAllocator.h"

#include "private/backend/DriverApi.h"

namespace filament {

using namespace backend;

namespace fg {

ResourceAllocator::ResourceAllocator(DriverApi& driverApi) noexcept
        : mBackend(driverApi) {
}

RenderTargetHandle ResourceAllocator::createRenderTarget(
        TargetBufferFlags targetBufferFlags, uint32_t width, uint32_t height,
        uint8_t samples, TargetBufferInfo color, TargetBufferInfo depth,
        TargetBufferInfo stencil) noexcept {
    return mBackend.createRenderTarget(targetBufferFlags,
            width, height, samples, color, depth, stencil);
}

void ResourceAllocator::destroyRenderTarget(RenderTargetHandle h) noexcept {
    return mBackend.destroyRenderTarget(h);
}

TextureHandle ResourceAllocator::createTexture(SamplerType target, uint8_t levels,
        TextureFormat format, uint8_t samples, uint32_t width, uint32_t height,
        uint32_t depth, TextureUsage usage) noexcept {
    return mBackend.createTexture(target, levels, format, samples, width, height, depth, usage);
}

void ResourceAllocator::destroyTexture(TextureHandle h) noexcept {
    return mBackend.destroyTexture(h);
}

} // namespace fg
} // namespace filament
