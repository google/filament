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
    return upcast(this)->getWidth(level);
}

size_t Texture::getHeight(size_t level) const noexcept {
    return upcast(this)->getHeight(level);
}

size_t Texture::getDepth(size_t level) const noexcept {
    return upcast(this)->getDepth(level);
}

size_t Texture::getLevels() const noexcept {
    return upcast(this)->getLevelCount();
}

Texture::Sampler Texture::getTarget() const noexcept {
    return upcast(this)->getTarget();
}

Texture::InternalFormat Texture::getFormat() const noexcept {
    return upcast(this)->getFormat();
}

void Texture::setImage(Engine& engine, size_t level,
        Texture::PixelBufferDescriptor&& buffer) const {
    upcast(this)->setImage(upcast(engine), level, std::move(buffer));
}

void Texture::setImage(Engine& engine,
        size_t level, uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
        PixelBufferDescriptor&& buffer) const {
    upcast(this)->setImage(upcast(engine),
            level, xoffset, yoffset, width, height, std::move(buffer));
}

void Texture::setImage(Engine& engine, size_t level,
        uint32_t xoffset, uint32_t yoffset, uint32_t zoffset,
        uint32_t width, uint32_t height, uint32_t depth,
        PixelBufferDescriptor&& buffer) const {
    upcast(this)->setImage(upcast(engine),
            level, xoffset, yoffset, zoffset, width, height, depth, std::move(buffer));
}

void Texture::setImage(Engine& engine, size_t level,
        Texture::PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const {
    upcast(this)->setImage(upcast(engine), level, std::move(buffer), faceOffsets);
}

void Texture::setExternalImage(Engine& engine, void* image) noexcept {
    upcast(this)->setExternalImage(upcast(engine), image);
}

void Texture::setExternalImage(Engine& engine, void* image, size_t plane) noexcept {
    upcast(this)->setExternalImage(upcast(engine), image, plane);
}

void Texture::setExternalStream(Engine& engine, Stream* stream) noexcept {
    upcast(this)->setExternalStream(upcast(engine), upcast(stream));
}

void Texture::generateMipmaps(Engine& engine) const noexcept {
    upcast(this)->generateMipmaps(upcast(engine));
}

bool Texture::isTextureFormatSupported(Engine& engine, InternalFormat format) noexcept {
    return FTexture::isTextureFormatSupported(upcast(engine), format);
}

bool Texture::isTextureSwizzleSupported(Engine& engine) noexcept {
    return FTexture::isTextureSwizzleSupported(upcast(engine));
}

size_t Texture::computeTextureDataSize(Texture::Format format, Texture::Type type, size_t stride,
        size_t height, size_t alignment) noexcept {
    return FTexture::computeTextureDataSize(format, type, stride, height, alignment);
}

void Texture::generatePrefilterMipmap(Engine& engine, Texture::PixelBufferDescriptor&& buffer,
        const Texture::FaceOffsets& faceOffsets, PrefilterOptions const* options) {
    upcast(this)->generatePrefilterMipmap(upcast(engine), std::move(buffer), faceOffsets, options);
}

} // namespace filament
