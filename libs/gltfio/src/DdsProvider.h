/*
* Copyright (C) 2022 The Android Open Source Project
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

#ifndef DDS_PROVIDER_H
#define DDS_PROVIDER_H

#include <array>

#include <filament/Texture.h>

#include <gli/gli.hpp>

using namespace filament;

namespace filament::gltfio {

backend::SamplerType convertSampler(gli::target gliTarget);

constexpr auto SIZE_SWIZZLE = gli::swizzles::length();
#define SWIZZLE(i) \
    (static_cast<backend::TextureSwizzle>(i + static_cast<uint8_t>(backend::TextureSwizzle::CHANNEL_0)))
glm::vec<SIZE_SWIZZLE, backend::TextureSwizzle> convertSwizzles(const gli::swizzles& gliSwizzles);

backend::PixelDataFormat convertPixelDataFormat(gli::format gliFormat);
backend::PixelDataType convertPixelDataType(gli::format gliFormat);
backend::CompressedPixelDataType convertCompressedPixelDataType(gli::format gliFormat);

backend::TextureFormat convertTextureFormat(gli::format gliFormat);

std::string pixelDataFormatString(backend::PixelDataFormat pixelDataFormat);
std::string pixelDataTypeString(backend::PixelDataType pixelDataType);
std::string compressedPixelDataTypeString(backend::CompressedPixelDataType compressedPixelDataType);
std::string textureFormatString(backend::TextureFormat textureFormat);
std::string samplerTypeString(backend::SamplerType samplerType);
std::string textureSwizzleString(backend::TextureSwizzle textureSwizzle);

std::string gliFormatString(gli::format gliFormat);
std::string gliSwizzleString(gli::swizzle gliSwizzle);
std::string gliTargetString(gli::target gliTarget);
}

#endif //DDS_PROVIDER_H
