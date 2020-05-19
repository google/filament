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
#include <math/mathfwd.h>

namespace filament {

class FSkybox;

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
         * Skybox intensity when no IndirectLight is set
         *
         * This call is ignored when an IndirectLight is set, otherwise it is used in its place.
         *
         *
         * @param envIntensity  Scale factor applied to the skybox texel values such that
         *                      the result is in cd/m^2 (lux) units (default = 30000)
         *
         * @return This Builder, for chaining calls.
         *
         * @see IndirectLight::Builder::intensity
         */
        Builder& intensity(float envIntensity) noexcept;

        /**
         * Sets the skybox to a constant color. Default is opaque black.
         *
         * Ignored if an environment is set.
         *
         * @param color
         *
         * @return This Builder, for chaining calls.
         */
        Builder& color(math::float4 color) noexcept;

        /**
         * Creates the Skybox object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this Skybox with.
         *
         * @return pointer to the newly created object, or nullptr if the light couldn't be created.
         */
        Skybox* build(Engine& engine);

    private:
        friend class FSkybox;
    };

    void setColor(math::float4 color) noexcept;

    /**
     * Sets bits in a visibility mask. By default, this is 0x1.
     *
     * This provides a simple mechanism for hiding or showing this Skybox in a Scene.
     *
     * @see View::setVisibleLayers().
     *
     * For example, to set bit 1 and reset bits 0 and 2 while leaving all other bits unaffected,
     * call: `setLayerMask(7, 2)`.
     *
     * @param select the set of bits to affect
     * @param values the replacement values for the affected bits
     */
    void setLayerMask(uint8_t select, uint8_t values) noexcept;

    /**
     * @return the visibility mask bits
     */
    uint8_t getLayerMask() const noexcept;

    /**
     * Returns the skybox's intensity in cd/m^2.
     */
    float getIntensity() const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_SKYBOX_H
