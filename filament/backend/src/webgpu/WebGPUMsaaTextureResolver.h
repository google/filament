/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUMSAATEXTURERESOLVER_H
#define TNT_FILAMENT_BACKEND_WEBGPUMSAATEXTURERESOLVER_H

#include <cstdint>

namespace wgpu {
class CommandEncoder;
class Texture;
enum class TextureAspect : uint32_t;
enum class TextureFormat : uint32_t;
enum class TextureViewDimension : uint32_t;
} // namespace wgpu

namespace filament::backend {

class WebGPUTexture;

class WebGPUMsaaTextureResolver final {
public:
    struct ResolveRequest final {
        struct TextureInfo final {
            wgpu::Texture const& texture;
            wgpu::TextureViewDimension viewDimension;
            uint8_t mipLevel;
            uint8_t layer;
            wgpu::TextureAspect aspect;
        };
        wgpu::CommandEncoder const& commandEncoder;
        wgpu::TextureFormat viewFormat;
        TextureInfo source;
        TextureInfo destination;
    };

    void resolve(ResolveRequest const&);
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPUMSAATEXTURERESOLVER_H
