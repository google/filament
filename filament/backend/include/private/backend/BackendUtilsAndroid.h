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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_BACKENDUTILSANDROID_H
#define TNT_FILAMENT_BACKEND_PRIVATE_BACKENDUTILSANDROID_H

#include <backend/DriverEnums.h>

namespace filament::backend {

/**
 * Maps AHardwareBuffer format to TextureFormat.
 */
TextureFormat mapToFilamentFormat(unsigned int format, bool isSrgbTransfer) noexcept;

/**
 * Maps AHardwareBuffer usage to TextureUsage.
 */
TextureUsage mapToFilamentUsage(unsigned int usage, TextureFormat format) noexcept;

inline constexpr bool isColorFormat(TextureFormat format) noexcept {
    switch (format) {
        // Standard color formats
        case TextureFormat::R8:
        case TextureFormat::RG8:
        case TextureFormat::RGBA8:
        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGBA16F:
        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGBA32F:
        case TextureFormat::RGB10_A2:
        case TextureFormat::R11F_G11F_B10F:
        case TextureFormat::SRGB8:
        case TextureFormat::SRGB8_A8:
        case TextureFormat::RGB8:
        case TextureFormat::RGB565:
        case TextureFormat::RGB5_A1:
        case TextureFormat::RGBA4:
            return true;
        default:
            break;
    }
    return false;
}

} // namespace filament

#endif // TNT_FILAMENT_BACKEND_PRIVATE_BACKENDUTILSANDROID_H
