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

//! \file

#ifndef TNT_FILAMENT_RENDERTARGET_H
#define TNT_FILAMENT_RENDERTARGET_H

#include <filament/FilamentAPI.h>

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <utils/compiler.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class FRenderTarget;

class Engine;
class Texture;

/**
 * An offscreen render target that can be associated with a View and contains
 * weak references to a set of attached Texture objects.
 *
 * RenderTarget is intended to be used with the View's post-processing disabled for the most part.
 * especially when a DEPTH attachment is also used (see Builder::texture()).
 *
 * Custom RenderTarget are ultimately intended to render into textures that might be used during
 * the main render pass.
 *
 * Clients are responsible for the lifetime of all associated Texture attachments.
 *
 * @see View
 */
class UTILS_PUBLIC RenderTarget : public FilamentAPI {
    struct BuilderDetails;

public:
    using CubemapFace = backend::TextureCubemapFace;

    /** Minimum number of color attachment supported */
    static constexpr uint8_t MIN_SUPPORTED_COLOR_ATTACHMENTS_COUNT =
            backend::MRT::MIN_SUPPORTED_RENDER_TARGET_COUNT;

    /** Maximum number of color attachment supported */
    static constexpr uint8_t MAX_SUPPORTED_COLOR_ATTACHMENTS_COUNT =
            backend::MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT;

    /**
     * Attachment identifiers
     */
    enum class AttachmentPoint : uint8_t {
        COLOR0 = 0,          //!< identifies the 1st color attachment
        COLOR1 = 1,          //!< identifies the 2nd color attachment
        COLOR2 = 2,          //!< identifies the 3rd color attachment
        COLOR3 = 3,          //!< identifies the 4th color attachment
        COLOR4 = 4,          //!< identifies the 5th color attachment
        COLOR5 = 5,          //!< identifies the 6th color attachment
        COLOR6 = 6,          //!< identifies the 7th color attachment
        COLOR7 = 7,          //!< identifies the 8th color attachment
        DEPTH  = MAX_SUPPORTED_COLOR_ATTACHMENTS_COUNT,   //!< identifies the depth attachment
        COLOR  = COLOR0,     //!< identifies the 1st color attachment
    };

    //! Use Builder to construct a RenderTarget object instance
    class Builder : public BuilderBase<BuilderDetails>, public BuilderNameMixin<Builder> {
        friend struct BuilderDetails;
    public:
        Builder() noexcept;
        Builder(Builder const& rhs) noexcept;
        Builder(Builder&& rhs) noexcept;
        ~Builder() noexcept;
        Builder& operator=(Builder const& rhs) noexcept;
        Builder& operator=(Builder&& rhs) noexcept;

        /**
         * Sets a texture to a given attachment point.
         *
         * When using a DEPTH attachment, it is important to always disable post-processing
         * in the View. Failing to do so will cause the DEPTH attachment to be ignored in most
         * cases.
         *
         * When the intention is to keep the content of the DEPTH attachment after rendering,
         * Usage::SAMPLEABLE must be set on the DEPTH attachment, otherwise the content of the
         * DEPTH buffer may be discarded.
         *
         * @param attachment The attachment point of the texture.
         * @param texture The associated texture object.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& texture(AttachmentPoint attachment, Texture* UTILS_NULLABLE texture) noexcept;

        /**
         * Sets the mipmap level for a given attachment point.
         *
         * @param attachment The attachment point of the texture.
         * @param level The associated mipmap level, 0 by default.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& mipLevel(AttachmentPoint attachment, uint8_t level) noexcept;

        /**
         * Sets the face for cubemap textures at the given attachment point.
         *
         * @param attachment The attachment point.
         * @param face The associated cubemap face.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& face(AttachmentPoint attachment, CubemapFace face) noexcept;

        /**
         * Sets an index of a single layer for 2d array, cubemap array, and 3d textures at the given
         * attachment point.
         *
         * For cubemap array textures, layer is translated into an array index and face according to
         *  - index: layer / 6
         *  - face: layer % 6
         *
         * @param attachment The attachment point.
         * @param layer The associated cubemap layer.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& layer(AttachmentPoint attachment, uint32_t layer) noexcept;

        /**
         * Sets the starting index of the 2d array textures for multiview at the given attachment
         * point.
         *
         * This requires COLOR and DEPTH attachments (if set) to be of 2D array textures.
         *
         * @param attachment The attachment point.
         * @param layerCount The number of layers used for multiview, starting from baseLayer.
         * @param baseLayer The starting index of the 2d array texture.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& multiview(AttachmentPoint attachment, uint8_t layerCount, uint8_t baseLayer = 0) noexcept;

        /**
         * Sets the number of samples used for MSAA (Multisample Anti-Aliasing).
         *
         * @param samples The number of samples used for multisampling.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& samples(uint8_t samples) noexcept;

        /**
         * Creates the RenderTarget object and returns a pointer to it.
         *
         * @return pointer to the newly created object.
         */
        RenderTarget* UTILS_NONNULL build(Engine& engine);

    private:
        friend class FRenderTarget;
    };

    /**
     * Gets the texture set on the given attachment point
     * @param attachment Attachment point
     * @return A Texture object or nullptr if no texture is set for this attachment point
     */
    Texture* UTILS_NULLABLE getTexture(AttachmentPoint attachment) const noexcept;

    /**
     * Returns the mipmap level set on the given attachment point
     * @param attachment Attachment point
     * @return the mipmap level set on the given attachment point
     */
    uint8_t getMipLevel(AttachmentPoint attachment) const noexcept;

    /**
     * Returns the face of a cubemap set on the given attachment point
     * @param attachment Attachment point
     * @return A cubemap face identifier. This is only relevant if the attachment's texture is
     * a cubemap.
     */
    CubemapFace getFace(AttachmentPoint attachment) const noexcept;

    /**
     * Returns the texture-layer set on the given attachment point
     * @param attachment Attachment point
     * @return A texture layer. This is only relevant if the attachment's texture is a 3D texture.
     */
    uint32_t getLayer(AttachmentPoint attachment) const noexcept;

    /**
     * Returns the number of color attachments usable by this instance of Engine. This method is
     * guaranteed to return at least MIN_SUPPORTED_COLOR_ATTACHMENTS_COUNT and at most
     * MAX_SUPPORTED_COLOR_ATTACHMENTS_COUNT.
     * @return Number of color attachments usable in a render target.
     */
    uint8_t getSupportedColorAttachmentsCount() const noexcept;

protected:
    // prevent heap allocation
    ~RenderTarget() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_RENDERTARGET_H
