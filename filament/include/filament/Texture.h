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

//! \file

#ifndef TNT_FILAMENT_TEXTURE_H
#define TNT_FILAMENT_TEXTURE_H

#include <filament/FilamentAPI.h>
#include <backend/DriverEnums.h>
#include <backend/PixelBufferDescriptor.h>

#include <utils/compiler.h>

#include <stddef.h>

namespace filament {

namespace details {
class FTexture;
} // namespace details

class Engine;
class Stream;

/**
 * Texture
 *
 * The Texture class supports:
 *  - 2D textures
 *  - 3D textures
 *  - Cube maps
 *  - mip mapping
 *
 *
 * Creation and destruction
 * ========================
 *
 * A Texture object is created using the Texture::Builder and destroyed by calling
 * Engine::destroy(const Texture*).
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *  filament::Engine* engine = filament::Engine::create();
 *
 *  filament::IndirectLight* texture = filament::Texture::Builder()
 *              .width(64)
 *              .height(64)
 *              .build(*engine);
 *
 *  engine->destroy(texture);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 */
class UTILS_PUBLIC Texture : public FilamentAPI {
    struct BuilderDetails;

public:
    static constexpr const size_t BASE_LEVEL = 0;

    using PixelBufferDescriptor = backend::PixelBufferDescriptor;    //!< Geometry of a pixel buffer
    using Sampler = backend::SamplerType;                            //!< Type of sampler
    using InternalFormat = backend::TextureFormat;                   //!< Internal texel format
    using CubemapFace = backend::TextureCubemapFace;                 //!< Cube map faces
    using Format = backend::PixelDataFormat;                         //!< Pixel color format
    using Type = backend::PixelDataType;                             //!< Pixel data format
    using CompressedType = backend::CompressedPixelDataType;         //!< Compressed pixel data format
    using FaceOffsets = backend::FaceOffsets;                        //!< Cube map faces offsets
    using Usage = backend::TextureUsage;                             //!< Usage affects texel layout

    static bool isTextureFormatSupported(Engine& engine, InternalFormat format) noexcept;

    static size_t computeTextureDataSize(Texture::Format format, Texture::Type type,
            size_t stride, size_t height, size_t alignment) noexcept;

    //! Use Builder to construct a Texture object instance
    class Builder : public BuilderBase<BuilderDetails> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Specifies the width in texels of the texture. Doesn't need to be a power-of-two.
         * @param width Width of the texture in texels (default: 1).
         * @return This Builder, for chaining calls.
         */
        Builder& width(uint32_t width) noexcept;

        /**
         * Specifies the height in texels of the texture. Doesn't need to be a power-of-two.
         * @param height Height of the texture in texels (default: 1).
         * @return This Builder, for chaining calls.
         */
        Builder& height(uint32_t height) noexcept;

        /**
         * Specifies the depth in texels of the texture. Doesn't need to be a power-of-two.
         * This creates a 3D textures.
         * @param depth Depth of the texture in texels (default: 1).
         * @return This Builder, for chaining calls.
         */
        Builder& depth(uint32_t depth) noexcept;

        /**
         * Specifies the numbers of mip map levels.
         * This creates a mip-map pyramid. The maximum number of levels a texture can have is
         * such that max(width, height, level) / 2^MAX_LEVELS = 1
         * @param levels Number of mipmap levels for this texture.
         * @return This Builder, for chaining calls.
         */
        Builder& levels(uint8_t levels) noexcept;

        /**
         * Specifies whether this texture is a cubemap
         * @param target either backend::SamplerType::SAMPLER_2D or
         *                      backend::SamplerType::SAMPLER_CUBEMAP
         * @return This Builder, for chaining calls.
         * @see Sampler
         */
        Builder& sampler(Sampler target) noexcept;

        /**
         * Specifies the *internal* format of this texture.
         *
         * The internal format specifies how texels are stored (which may be different from how
         * they're specified in setImage()). InternalFormat specifies both the color components
         * and the data type used.
         *
         * @param format Format of the texture's texel.
         * @return This Builder, for chaining calls.
         * @see InternalFormat, setImage
         */
        Builder& format(InternalFormat format) noexcept;

        /**
         * Specifies if the texture will be used as a render target attachment.
         *
         * If the texture is potentially rendered into, it may require a different memory layout,
         * which needs to be known during construction.
         *
         * @param usage Defaults to Texture::Usage::DEFAULT; c.f. Texture::Usage::COLOR_ATTACHMENT.
         * @return This Builder, for chaining calls.
         */
        Builder& usage(Usage usage) noexcept;

        /**
         * Specifies that the alpha channel contains a color multiplier (e.g. for HDR)
         * The default value is false.
         *
         * @param enabled True if the shader should interpret alpha as a color multiplier
         *
         * @return This Builder, for chaining calls.
         */
        Builder& rgbm(bool enabled) noexcept;

        /**
         * Creates the Texture object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this Texture with.
         *
         * @return pointer to the newly created object or nullptr if exceptions are disabled and
         *         an error occurred.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        Texture* build(Engine& engine);

    private:
        friend class details::FTexture;
    };

    /**
     * Returns the width of a 2D or 3D texture level
     * @param level texture level.
     * @return Width in texel of the specified \p level, clamped to 1.
     * @attention If this texture is using backend::SamplerType::SAMPLER_EXTERNAL, the dimension
     * of the texture are unknown and this method always returns whatever was set on the Builder.
     */
    size_t getWidth(size_t level = BASE_LEVEL) const noexcept;

    /**
     * Returns the height of a 2D or 3D texture level
     * @param level texture level.
     * @return Height in texel of the specified \p level, clamped to 1.
     * @attention If this texture is using backend::SamplerType::SAMPLER_EXTERNAL, the dimension
     * of the texture are unknown and this method always returns whatever was set on the Builder.
     */
    size_t getHeight(size_t level = BASE_LEVEL) const noexcept;

    /**
     * Returns the depth of a 3D texture level
     * @param level texture level.
     * @return Depth in texel of the specified \p level, clamped to 1.
     * @attention If this texture is using backend::SamplerType::SAMPLER_EXTERNAL, the dimension
     * of the texture are unknown and this method always returns whatever was set on the Builder.
     */
    size_t getDepth(size_t level = BASE_LEVEL) const noexcept;

    /**
     * Returns the maximum number of levels this texture can have.
     * @return maximum number of levels this texture can have.
     * @attention If this texture is using backend::SamplerType::SAMPLER_EXTERNAL, the dimension
     * of the texture are unknown and this method always returns whatever was set on the Builder.
     */
    size_t getLevels() const noexcept;

    /**
     * Return this texture Sampler as set by Builder::sampler().
     * @return this texture Sampler as set by Builder::sampler()
     */
    Sampler getTarget() const noexcept;

    /**
     * Return this texture InternalFormat as set by Builder::format().
     * @return this texture InternalFormat as set by Builder::format().
     */
    InternalFormat getFormat() const noexcept;

    /**
     * Return if this texture has RGBM data as set by Builder::rgbm().
     * @return if this texture has RGBM data as set by Builder::rgbm().
     */
    bool isRgbm() const noexcept;

    /**
     * Specify the image of a 2D texture for a level.
     *
     * @param engine    Engine this texture is associated to.
     * @param level     Level to set the image for.
     * @param buffer    Client-side buffer containing the image to set.
     *
     * @attention \p engine must be the instance passed to Builder::build()
     * @attention \p level must be less than getLevels().
     * @attention \p buffer's backend::PixelDataFormat must match that of getFormat().
     * @attention This Texture instance must use backend::SamplerType::SAMPLER_2D or
     *            backend::SamplerType::SAMPLER_EXTERNAL. IF the later is specified
     *            and external textures are supported by the driver implementation,
     *            this method will have no effect, otherwise it will behave as if the
     *            texture was specified with driver::SamplerType::SAMPLER_2D.
     *
     * @note
     * This is equivalent to calling:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * setImage(engine, level, 0, 0, getWidth(level), getHeight(level), buffer);
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * @see Builder::sampler()
     */
    void setImage(Engine& engine, size_t level, PixelBufferDescriptor&& buffer) const noexcept;

    /**
     * Updates a sub-image of a 2D texture for a level.
     *
     * @param engine    Engine this texture is associated to.
     * @param level     Level to set the image for.
     * @param xoffset   Left offset of the sub-region to update.
     * @param yoffset   Bottom offset of the sub-region to update.
     * @param width     Width of the sub-region to update.
     * @param height    Height of the sub-region to update.
     * @param buffer    Client-side buffer containing the image to set.
     *
     * @attention \p engine must be the instance passed to Builder::build()
     * @attention \p level must be less than getLevels().
     * @attention \p buffer's backend::PixelDataFormat must match that of getFormat().
     * @attention This Texture instance must use backend::SamplerType::SAMPLER_2D or
     *            backend::SamplerType::SAMPLER_EXTERNAL. IF the later is specified
     *            and external textures are supported by the driver implementation,
     *            this method will have no effect, otherwise it will behave as if the
     *            texture was specified with backend::SamplerType::SAMPLER_2D.
     *
     * @see Builder::sampler()
     */
    void setImage(Engine& engine, size_t level,
            uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            PixelBufferDescriptor&& buffer) const noexcept;

    /**
     * Specify all six images of a cube map level.
     *
     * This method follows exactly the OpenGL conventions.
     *
     * @param engine        Engine this texture is associated to.
     * @param level         Level to set the image for.
     * @param buffer        Client-side buffer containing the images to set.
     * @param faceOffsets   Offsets in bytes into \p buffer for all six images. The offsets
     *                      are specified in the following order: +x, -x, +y, -y, +z, -z
     *
     * @attention \p engine must be the instance passed to Builder::build()
     * @attention \p level must be less than getLevels().
     * @attention \p buffer's backend::PixelDataFormat must match that of getFormat().
     * @attention This Texture instance must use backend::SamplerType::SAMPLER_CUBEMAP or it has no effect
     *
     * @see backend::TextureCubemapFace, Builder::sampler()
     */
    void setImage(Engine& engine, size_t level,
            PixelBufferDescriptor&& buffer, const FaceOffsets& faceOffsets) const noexcept;


    /**
     * Specify the external image to associate with this Texture. Typically the external
     * image is OS specific, and can be a video or camera frame.
     * There are many restrictions when using an external image as a texture, such as:
     *   - only the level of detail (lod) 0 can be specified
     *   - only nearest or linear filtering is supported
     *   - the size and format of the texture is defined by the external image
     *
     * @param engine        Engine this texture is associated to.
     * @param image         An opaque handle to a platform specific image. The only supported
     *                      type is eglImageOES.
     *
     * @attention \p engine must be the instance passed to Builder::build()
     * @attention This Texture instance must use backend::SamplerType::SAMPLER_EXTERNAL or it has no effect
     *
     * @see Builder::sampler()
     *
     */
    void setExternalImage(Engine& engine, void* image) noexcept;


    /**
     * Specify the external stream to associate with this Texture. Typically the external
     * stream is OS specific, and can be a video or camera stream.
     * There are many restrictions when using an external stream as a texture, such as:
     *   - only the level of detail (lod) 0 can be specified
     *   - only nearest or linear filtering is supported
     *   - the size and format of the texture is defined by the external stream
     *
     * @param engine        Engine this texture is associated to.
     * @param stream        A Stream object
     *
     * @attention \p engine must be the instance passed to Builder::build()
     * @attention This Texture instance must use backend::SamplerType::SAMPLER_EXTERNAL or it has no effect
     *
     * @see Builder::sampler(), Stream
     *
     */
    void setExternalStream(Engine& engine, Stream* stream) noexcept;

    /**
     * Generates all the mipmap levels automatically. This requires the texture to have a
     * color-renderable format.
     *
     * @param engine        Engine this texture is associated to.
     *
     * @attention \p engine must be the instance passed to Builder::build()
     * @attention This Texture instance must NOT use backend::SamplerType::SAMPLER_CUBEMAP or it has no effect
     */
    void generateMipmaps(Engine& engine) const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_TEXTURE_H
