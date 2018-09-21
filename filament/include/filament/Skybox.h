/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_SKYBOX_H
#define TNT_FILAMENT_SKYBOX_H

#include <filament/FilamentAPI.h>

#include <utils/compiler.h>

#include <stdint.h>

namespace filament {

namespace details {
class FSkybox;
} // namespace details

class Engine;
class Texture;

/**
 * Skybox
 *
 * When added to a Scene, the Skybox fills all untouched pixels.
 *
 * Creation and destruction
 * ========================
 *
 * A Skybox object is created using the Skybox::Builder and destroyed by calling
 * Engine::destroy(const Skybox*).
 *
 * ~~~~~~~~~~~{.cpp}
 *  filament::Engine* engine = filament::Engine::create();
 *
 *  filament::IndirectLight* skybox = filament::Skybox::Builder()
 *              .environment(cubemap)
 *              .build(*engine);
 *
 *  engine->destroy(skybox);
 * ~~~~~~~~~~~
 *
 *
 * @note
 * Currently only Texture based sky boxes are supported.
 *
 * @see Scene, IndirectLight
 */
class UTILS_PUBLIC Skybox : public FilamentAPI {
    struct BuilderDetails;

public:
    //! Use Builder to construct an Skybox object instance
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
         * Set the environment map (i.e. the skybox content).
         *
         * The Skybox is rendered as though it were an infinitely large cube with the camera
         * inside it. This means that the cubemap which is mapped onto the cube's exterior
         * will appear mirrored. This follows the OpenGL conventions.
         *
         * The cmgen tool generates reflection maps by default which are therefore ideal to use
         * as skyboxes.
         *
         * @param cubemap This Texture must be a cube map.
         *
         * @return This Builder, for chaining calls.
         *
         * @see Texture
         */
        Builder& environment(Texture* cubemap) noexcept;

        /**
         * Indicates whether the sun should be rendered. The sun can only be
         * rendered if there is at least one light of type SUN in the scene.
         * The default value is false.
         *
         * @param show True if the sun should be rendered, false otherwise
         *
         * @return This Builder, for chaining calls.
         */
        Builder& showSun(bool show) noexcept;

        /**
         * Creates the Skybox object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this Skybox with.
         *
         * @return pointer to the newly created object, or nullptr if the light couldn't be created.
         */
        Skybox* build(Engine& engine);

    private:
        friend class details::FSkybox;
    };

    void setLayerMask(uint8_t select, uint8_t values) noexcept;

    uint8_t getLayerMask() const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_SKYBOX_H
