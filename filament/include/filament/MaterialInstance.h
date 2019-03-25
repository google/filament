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

#ifndef TNT_FILAMENT_MATERIALINSTANCE_H
#define TNT_FILAMENT_MATERIALINSTANCE_H

#include <filament/FilamentAPI.h>
#include <filament/Color.h>
#include <filament/TextureSampler.h>

#include <utils/compiler.h>

namespace filament {

class Material;
class Texture;
class UniformBuffer;
class UniformInterfaceBlock;

class UTILS_PUBLIC MaterialInstance : public FilamentAPI {
public:
    /**
     * @return the Material associated with this instance
     */
    Material const* getMaterial() const noexcept;

    /**
     * Set a uniform by name
     *
     * @param name      Name of the parameter as defined by Material. Cannot be nullptr.
     * @param value     Value of the parameter to set.
     * @throws utils::PreConditionPanic if name doesn't exist or no-op if exceptions are disabled.
     */
    template<typename T>
    void setParameter(const char* name, T value) noexcept;

    /**
     * Set a uniform array by name
     *
     * @param name      Name of the parameter array as defined by Material. Cannot be nullptr.
     * @param values    Array of values to set to the named parameter array.
     * @param count     Size of the array to set.
     * @throws utils::PreConditionPanic if name doesn't exist or no-op if exceptions are disabled.
     */
    template<typename T>
    void setParameter(const char* name, const T* values, size_t count) noexcept;

    /**
     * Set a texture as the named parameter
     *
     * @param name      Name of the parameter as defined by Material. Cannot be nullptr.
     * @param texture   Non nullptr Texture object pointer.
     * @param sampler   Sampler parameters.
     * @throws utils::PreConditionPanic if name doesn't exist or no-op if exceptions are disabled.
     */
    void setParameter(const char* name,
            Texture const* texture, TextureSampler const& sampler) noexcept;

    /**
     * Set an RGB color as the named parameter.
     * A conversion might occur depending on the specified type
     *
     * @param name      Name of the parameter as defined by Material. Cannot be nullptr.
     * @param type      Whether the color value is encoded as Linear or sRGB.
     * @param color     Array of read, green, blue channels values.
     * @throws utils::PreConditionPanic if name doesn't exist or no-op if exceptions are disabled.
     */
    void setParameter(const char* name, RgbType type, math::float3 color) noexcept;

    /**
     * Set an RGBA color as the named parameter.
     * A conversion might occur depending on the specified type
     *
     * @param name      Name of the parameter as defined by Material. Cannot be nullptr.
     * @param type      Whether the color value is encoded as Linear or sRGB/A.
     * @param color     Array of read, green, blue and alpha channels values.
     * @throws utils::PreConditionPanic if name doesn't exist or no-op if exceptions are disabled.
     */
    void setParameter(const char* name, RgbaType type, math::float4 color) noexcept;

    /**
     * Set up a custom scissor rectangle; by default this encompasses the View.
     * 
     * @param left      left coordinate of the scissor box
     * @param bottom    bottom coordinate of the scissor box
     * @param width     width of the scissor box
     * @param height    height of the scissor box
     */
    void setScissor(uint32_t left, uint32_t bottom, uint32_t width, uint32_t height) noexcept;

    /**
     * Returns the scissor rectangle to its default setting, which encompasses the View.
     */
    void unsetScissor() noexcept;

    /**
     * Sets a polygon offset that will be applied to all renderables drawn with this material
     * instance.
     *
     *  The value of the offset is scale * dz + r * constant, where dz is the change in depth
     *  relative to the screen area of the triangle, and r is the smallest value that is guaranteed
     *  to produce a resolvable offset for a given implementation. This offset is added before the
     *  depth test.
     *
     *  @warning using a polygon offset other than zero has a significant negative performance
     *  impact, as most implementations have to disable early depth culling. DO NOT USE unless
     *  absolutely necessary.
     *
     * @param scale scale factor used to create a variable depth offset for each triangle
     * @param constant scale factore used to create a constant depth offset for each triangle
     */
    void setPolygonOffset(float scale, float constant) noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_MATERIALINSTANCE_H
