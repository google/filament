/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_TEXTURE_BINDINGS_TEST_H
#define TNT_FILAMENT_TEXTURE_BINDINGS_TEST_H

#include <filament/FilamentAPI.h>

#include <backend/DriverEnums.h>
#include <backend/PixelBufferDescriptor.h>
#include <backend/Platform.h>

#include <utils/compiler.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class Engine;
class Stream;

class UTILS_PUBLIC TextureBindingsTest : public FilamentAPI {
    struct BuilderDetails;

public:
    static constexpr size_t BASE_LEVEL = 0;

    using PixelBufferDescriptor = backend::PixelBufferDescriptor;
    using Sampler = backend::SamplerType;
    using InternalFormat = backend::TextureFormat;
    using CubemapFace = backend::TextureCubemapFace;
    using Format = backend::PixelDataFormat;
    using Type = backend::PixelDataType;
    using CompressedType = backend::CompressedPixelDataType;
    using Usage = backend::TextureUsage;
    using Swizzle = backend::TextureSwizzle;
    using ExternalImageHandleRef = backend::Platform::ExternalImageHandleRef;

    static bool isTextureFormatSupported(Engine& engine, InternalFormat format) noexcept;
    static size_t computeTextureDataSize(Format format, Type type,
            size_t stride, size_t height, size_t alignment) noexcept;

    class Builder : public BuilderBase<BuilderDetails>, public BuilderNameMixin<Builder> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        Builder& width(uint32_t width) noexcept;
        Builder& height(uint32_t height) noexcept;
        Builder& sampler(Sampler target) noexcept;
        Builder& format(InternalFormat format) noexcept;
        Builder& usage(Usage usage) noexcept;
        Builder& swizzle(Swizzle r, Swizzle g, Swizzle b, Swizzle a) noexcept;

        UTILS_APIGEN_ALTERNATE_NAME(importTexture)
        Builder& import(intptr_t id) noexcept;

        UTILS_NOAPIGEN
        Builder& async(void* UTILS_NULLABLE handler) noexcept;

        TextureBindingsTest* UTILS_NONNULL build(Engine& engine);
    };

    size_t getWidth(size_t level = BASE_LEVEL) const noexcept;
    Sampler getTarget() const noexcept;
    InternalFormat getFormat() const noexcept;

    void setImage(Engine& engine, size_t level, PixelBufferDescriptor&& buffer) const;

    void setExternalImage(Engine& engine, ExternalImageHandleRef image);
    void setExternalImage(Engine& engine, void* UTILS_NONNULL image, size_t plane);
    void setExternalStream(Engine& engine, Stream* UTILS_NULLABLE stream);

    void generateMipmaps(Engine& engine) const;
    bool isCreationComplete() const noexcept;

    UTILS_DEPRECATED
    UTILS_NOAPIGEN
    void setExternalImage(Engine& engine, void* UTILS_NONNULL image);

    UTILS_NOAPIGEN
    void setImageAsync(Engine& engine, size_t level, PixelBufferDescriptor&& buffer) const;
};

} // namespace filament

#endif // TNT_FILAMENT_TEXTURE_BINDINGS_TEST_H
