/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "details/Texture.h"

#include "details/Engine.h"
#include "details/Stream.h"

namespace filament {

size_t Texture::getWidth(size_t level) const noexcept {
    return downcast(this)->getWidth(level);
}

size_t Texture::getHeight(size_t level) const noexcept {
    return downcast(this)->getHeight(level);
}

size_t Texture::getDepth(size_t level) const noexcept {
    return downcast(this)->getDepth(level);
}

size_t Texture::getLevels() const noexcept {
    return downcast(this)->getLevelCount();
}

Texture::Sampler Texture::getTarget() const noexcept {
    return downcast(this)->getTarget();
}

Texture::InternalFormat Texture::getFormat() const noexcept {
    return downcast(this)->getFormat();
}

void Texture::setImage(Engine& engine, size_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& buffer) const {
    downcast(this)->setImage(downcast(engine),
            level, xoffset, yoffset, zoffset, width, height, depth, std::move(buffer));
}

void Texture::setImage(Engine& engine, size_t level,
        PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const {
    downcast(this)->setImage(downcast(engine), level, std::move(buffer), faceOffsets);
}

void Texture::setExternalImage(Engine& engine, void* image) noexcept {
    downcast(this)->setExternalImage(downcast(engine), image);
}

void Texture::setExternalImage(Engine& engine, void* image, size_t plane) noexcept {
    downcast(this)->setExternalImage(downcast(engine), image, plane);
}

void Texture::setExternalStream(Engine& engine, Stream* stream) noexcept {
    downcast(this)->setExternalStream(downcast(engine), downcast(stream));
}

void Texture::generateMipmaps(Engine& engine) const noexcept {
    downcast(this)->generateMipmaps(downcast(engine));
}

bool Texture::isTextureFormatSupported(Engine& engine, InternalFormat format) noexcept {
    return FTexture::isTextureFormatSupported(downcast(engine), format);
}

bool Texture::isProtectedTexturesSupported(Engine& engine) noexcept {
    return FTexture::isProtectedTexturesSupported(downcast(engine));
}

bool Texture::isTextureSwizzleSupported(Engine& engine) noexcept {
    return FTexture::isTextureSwizzleSupported(downcast(engine));
}

size_t Texture::computeTextureDataSize(Format format, Type type, size_t stride,
        size_t height, size_t alignment) noexcept {
    return FTexture::computeTextureDataSize(format, type, stride, height, alignment);
}

void Texture::generatePrefilterMipmap(Engine& engine, PixelBufferDescriptor&& buffer,
        const FaceOffsets& faceOffsets, PrefilterOptions const* options) {
    downcast(this)->generatePrefilterMipmap(downcast(engine), std::move(buffer), faceOffsets, options);
}

} // namespace filament
