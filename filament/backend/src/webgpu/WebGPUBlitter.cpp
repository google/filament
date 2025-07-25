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

#include "WebGPUBlitter.h"

#include <utils/Panic.h>

namespace filament::backend {
WebGPUBlitter::WebGPUBlitter(const wgpu::Device& device)
    : mDevice{ device } {}

void WebGPUBlitter::blit(const wgpu::CommandEncoder&, uint32_t layer, uint32_t level,
        const wgpu::Texture& stagingTexture, const wgpu::Texture& outTexture) {
    // TODO Implement BLIT
    PANIC_POSTCONDITION("Blit is not yet implemented");
}

} // namespace filament::backend
