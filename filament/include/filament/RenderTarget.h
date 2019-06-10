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

namespace details {
class FRenderTarget;
} // namespace details

class Engine;
class Texture;

/**
 * An offscreen render target that can be associated with a View.
 *
 * RenderTarget objects do not own their associated Texture attachments;
 * clients must explicitly create & destroy these.
 *
 * @see View
 */
class UTILS_PUBLIC RenderTarget : public FilamentAPI {
    struct BuilderDetails;

public:
    using CubemapFace = backend::TextureCubemapFace;

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
         * Sets the color attachment.
         *
         * All RenderTargets must have a non-null color attachment.
         *
         * @param texture The associated color attachment.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& color(Texture* texture) noexcept;

        /** 
         * Sets the depth attachment.
         *
         * @param texture The associated depth attachment, nullptr by default.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& depth(Texture* texture) noexcept;

        /** 
         * Sets the associated mipmap level.
         *
         * @param level The associated mipmap level, 0 by default.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& miplevel(uint8_t level) noexcept;

        /** 
         * Sets the associated cubemap face.
         *
         * @param face The associated cubemap face, POSITIVE_X by default.
         * @return A reference to this Builder for chaining calls.
         */
        Builder& face(CubemapFace face) noexcept;

        /**
         * Creates the RenderTarget object and returns a pointer to it.
         * 
         * @return pointer to the newly created object or nullptr if exceptions are disabled and
         *         an error occurred.
         */
        RenderTarget* build(Engine& engine);

    private:
        friend class details::FRenderTarget;
    };

    Texture* getColor() const noexcept;
    Texture* getDepth() const noexcept;
    uint8_t getMiplevel() const noexcept;
    CubemapFace getFace() const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_RENDERTARGET_H
