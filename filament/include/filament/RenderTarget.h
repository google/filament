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

#include <stddef.h>

namespace filament {

class FRenderTarget;

class Engine;
class Texture;

/**
 * An offscreen render target that can be associated with a View and contains
 * weak references to a set of attached Texture objects.
 *
 * Clients are responsible for the lifetime of all associated Texture attachments.
 *
 * @see View
 */
class UTILS_PUBLIC RenderTarget : public FilamentAPI {
    struct BuilderDetails;

public:
    using CubemapFace = backend::TextureCubemapFace;

    /**
     * Attachment identifiers
     */
    enum AttachmentPoint {
        COLOR = 0,      //!< identifies the color attachment
        DEPTH = 1,      //!< identifies the depth attachment
    };

    static constexpr size_t ATTACHMENT_COUNT = 2;

    //! Use Builder to construct a RenderTarget object instance
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
         * Sets a texture to a given attachment point.
         *
         * All RenderTargets must have a non-null COLOR attachment.
         *
         * @param attachment The attachment point of the texture.
         * @param texture The associated texture object.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& texture(AttachmentPoint attachment, Texture* texture) noexcept;

        /**
         * Sets the mipmap level for a given attachment point.
         *
         * @param attachment The attachment point of the texture.
         * @param level The associated mipmap level, 0 by default.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& mipLevel(AttachmentPoint attachment, uint8_t level) noexcept;

        /**
         * Sets the cubemap face for a given attachment point.
         *
         * @param attachment The attachment point.
         * @param face The associated cubemap face.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& face(AttachmentPoint attachment, CubemapFace face) noexcept;

        /**
         * Sets the layer for a given attachment point (for 3D textures).
         *
         * @param attachment The attachment point.
         * @param layer The associated cubemap layer.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& layer(AttachmentPoint attachment, uint32_t layer) noexcept;

        /**
         * Creates the RenderTarget object and returns a pointer to it.
         *
         * @return pointer to the newly created object or nullptr if exceptions are disabled and
         *         an error occurred.
         */
        RenderTarget* build(Engine& engine);

    private:
        friend class FRenderTarget;
    };

    /**
     * Gets the texture set on the given attachment point
     * @param attachment Attachment point
     * @return A Texture object or nullptr if no texture is set for this attachment point
     */
    Texture* getTexture(AttachmentPoint attachment) const noexcept;

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
};

} // namespace filament

#endif // TNT_FILAMENT_RENDERTARGET_H
