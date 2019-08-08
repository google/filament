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

#ifndef TNT_FILAMENT_FG_RESOURCEALLOCATOR_H
#define TNT_FILAMENT_FG_RESOURCEALLOCATOR_H

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/TargetBufferInfo.h>

#include "private/backend/DriverApiForward.h"

#include <stdint.h>

namespace filament {
namespace fg {

class ResourceAllocator {
public:
    explicit ResourceAllocator(backend::DriverApi& driverApi) noexcept;

    backend::RenderTargetHandle createRenderTarget(
            backend::TargetBufferFlags targetBufferFlags,
            uint32_t width,
            uint32_t height,
            uint8_t samples,
            backend::TargetBufferInfo color,
            backend::TargetBufferInfo depth,
            backend::TargetBufferInfo stencil) noexcept;

    void destroyRenderTarget(backend::RenderTargetHandle h) noexcept;

    backend::TextureHandle createTexture(
            backend::SamplerType target,
            uint8_t levels,
            backend::TextureFormat format,
            uint8_t samples,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            backend::TextureUsage usage) noexcept;

    void destroyTexture(backend::TextureHandle h) noexcept;

private:
    backend::DriverApi& mBackend;
};

}// namespace fg
} // namespace filament

#endif //TNT_FILAMENT_FG_RESOURCEALLOCATOR_H
