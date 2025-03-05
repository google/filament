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

#include "private/backend/BackendUtilsAndroid.h"

#include <android/hardware_buffer.h>

namespace filament::backend {

TextureFormat mapToFilamentFormat(unsigned int format, bool isSrgbTransfer) noexcept {
    if (isSrgbTransfer) {
        switch (format) {
            case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
                return TextureFormat::SRGB8;
            default:
                return TextureFormat::SRGB8_A8;
        }
    }
    switch (format) {
        case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
        case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
            return TextureFormat::RGBA8;
        case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
        case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
            return TextureFormat::RGB8;
        case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
            return TextureFormat::RGB565;
        case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
            return TextureFormat::RGBA16F;
        case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
            return TextureFormat::RGB10_A2;
        case AHARDWAREBUFFER_FORMAT_D16_UNORM:
            return TextureFormat::DEPTH16;
        case AHARDWAREBUFFER_FORMAT_D24_UNORM:
            return TextureFormat::DEPTH24;
        case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
            return TextureFormat::DEPTH24_STENCIL8;
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
            return TextureFormat::DEPTH32F;
        case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
            return TextureFormat::DEPTH32F_STENCIL8;
        case AHARDWAREBUFFER_FORMAT_S8_UINT:
            return TextureFormat::STENCIL8;
        case AHARDWAREBUFFER_FORMAT_R8_UNORM:
            return TextureFormat::R8;
        default:
            return TextureFormat::UNUSED;
    }
}

TextureUsage mapToFilamentUsage(unsigned int usage, TextureFormat format) noexcept {
    TextureUsage usageFlags = TextureUsage::NONE;

    if (usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) {
        usageFlags |= TextureUsage::SAMPLEABLE;
    }

    if (usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER) {
        if (isDepthFormat(format)) {
            usageFlags |= TextureUsage::DEPTH_ATTACHMENT;
        }
        if (isStencilFormat(format)) {
            usageFlags |= TextureUsage::STENCIL_ATTACHMENT;
        }
        if (isColorFormat(format)) {
            usageFlags |= TextureUsage::COLOR_ATTACHMENT;
        }
    }

    if (usage & AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER) {
        usageFlags |= TextureUsage::UPLOADABLE;
    }

    if (usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT) {
        usageFlags |= TextureUsage::PROTECTED;
    }

    if (usageFlags == TextureUsage::NONE) {
        usageFlags = TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE;
    }

    return usageFlags;
}

} // namespace backend::filament
