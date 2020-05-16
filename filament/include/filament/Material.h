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

#ifndef TNT_FILAMENT_MATERIAL_H
#define TNT_FILAMENT_MATERIAL_H

#include <filament/Color.h>
#include <filament/FilamentAPI.h>
#include <filament/MaterialEnums.h>
#include <filament/MaterialInstance.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>

#include <math/mathfwd.h>

#include <stdint.h>

namespace utils {
    class CString;
} // namespace utils

namespace filament {

class Texture;
class TextureSampler;

class FEngine;
class FMaterial;

class Engine;

class UTILS_PUBLIC Material : public FilamentAPI {
    struct BuilderDetails;

public:
    using BlendingMode = filament::BlendingMode;
    using Shading = filament::Shading;
    using Interpolation = filament::Interpolation;
    using VertexDomain = filament::VertexDomain;
    using TransparencyMode = filament::TransparencyMode;

    using ParameterType = filament::backend::UniformType;
    using Precision = filament::backend::Precision;
    using SamplerType = filament::backend::SamplerType;
    using SamplerFormat = filament::backend::SamplerFormat;
    using CullingMode = filament::backend::CullingMode;
    using ShaderModel = filament::backend::ShaderModel;

    /**
     * Holds information about a material parameter.
     */
    struct ParameterInfo {
        //! Name of the parameter.
        const char* name;
        //! Whether the parameter is a sampler (texture).
        bool isSampler;
        union {
            //! Type of the parameter if the parameter is not a sampler.
            ParameterType type;
            //! Type of the parameter if the parameter is a sampler.
            SamplerType samplerType;
        };
        //! Size of the parameter when the parameter is an array.
        uint32_t count;
        //! Requested precision of the parameter.
        Precision precision;
    };

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
         * Specifies the material data. The material data is a binary blob produced by
         * libfilamat or by matc.
         *
         * @param payload Pointer to the material data, must stay valid until build() is called.
         * @param size Size of the material data pointed to by "payload" in bytes.
         */
        Builder& package(const void* payload, size_t size);

        /**
         * Creates the Material object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this Material with.
         *
         * @return pointer to the newly created object or nullptr if exceptions are disabled and
         *         an error occurred.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        Material* build(Engine& engine);
    private:
        friend class FMaterial;
    };

    /**
     * Creates a new instance of this material. Material instances should be freed using
     * Engine::destroy(const MaterialInstance*).
     *
     * @param name Optional name to associate with the given material instance. If this is null,
     * then the instance inherits the material's name.
     *
     * @return A pointer to the new instance.
     */
    MaterialInstance* createInstance(const char* name = nullptr) const noexcept;

    //! Returns the name of this material as a null-terminated string.
    const char* getName() const noexcept;

    //! Returns the shading model of this material.
    Shading getShading()  const noexcept;

    //! Returns the interpolation mode of this material. This affects how variables are interpolated.
    Interpolation getInterpolation() const noexcept;

    //! Returns the blending mode of this material.
    BlendingMode getBlendingMode() const noexcept;

    //! Returns the vertex domain of this material.
    VertexDomain getVertexDomain() const noexcept;

    //! Returns the material domain of this material.
    //! The material domain determines how the material is used.
    MaterialDomain getMaterialDomain() const noexcept;

    //! Returns the default culling mode of this material.
    CullingMode getCullingMode() const noexcept;

    //! Returns the transparency mode of this material.
    //! This value only makes sense when the blending mode is transparent or fade.
    TransparencyMode getTransparencyMode() const noexcept;

    //! Indicates whether instances of this material will, by default, write to the color buffer.
    bool isColorWriteEnabled() const noexcept;

    //! Indicates whether instances of this material will, by default, write to the depth buffer.
    bool isDepthWriteEnabled() const noexcept;

    //! Indicates whether instances of this material will, by default, use depth testing.
    bool isDepthCullingEnabled() const noexcept;

    //! Indicates whether this material is double-sided.
    bool isDoubleSided() const noexcept;

    //! Returns the alpha mask threshold used when the blending mode is set to masked.
    float getMaskThreshold() const noexcept;

    //! Indicates whether this material uses the shadowing factor as a color multiplier.
    //! This values only makes sense when the shading mode is unlit.
    bool hasShadowMultiplier() const noexcept;

    //! Indicates whether this material has specular anti-aliasing enabled
    bool hasSpecularAntiAliasing() const noexcept;

    //! Returns the screen-space variance for specular-antialiasing, this value is between 0 and 1.
    float getSpecularAntiAliasingVariance() const noexcept;

    //! Returns the clamping threshold for specular-antialiasing, this value is between 0 and 1.
    float getSpecularAntiAliasingThreshold() const noexcept;

    //! Returns the list of vertex attributes required by this material.
    AttributeBitset getRequiredAttributes() const noexcept;

    //! Returns the refraction mode used by this material.
    RefractionMode getRefractionMode() const noexcept;

    // Return the refraction type used by this material.
    RefractionType getRefractionType() const noexcept;

    /**
     * Returns the number of parameters declared by this material.
     * The returned value can be 0.
     */
    size_t getParameterCount() const noexcept;

    /**
     * Gets information about this material's parameters.
     *
     * @param parameters A pointer to a list of ParameterInfo.
     *                   The list must be at least "count" large
     * @param count The number of parameters to retrieve. Must be >= 0 and can be > count.
     *
     * @return The number of parameters written to the parameters pointer.
     */
    size_t getParameters(ParameterInfo* parameters, size_t count) const noexcept;

    //! Indicates whether a parameter of the given name exists on this material.
    bool hasParameter(const char* name) const noexcept;

    /**
     * Sets the value of the given parameter on this material's default instance.
     *
     * @param name The name of the material parameter
     * @param value The value of the material parameter
     *
     * @see getDefaultInstance()
     */
    template <typename T>
    void setDefaultParameter(const char* name, T value) noexcept {
        getDefaultInstance()->setParameter(name, value);
    }

    /**
     * Sets a texture and sampler parameters on this material's default instance.
     *
     * @param name The name of the material texture parameter
     * @param texture The texture to set as parameter
     * @param sampler The sampler to be used with this texture
     *
     * @see getDefaultInstance()
     */
    void setDefaultParameter(const char* name, Texture const* texture,
            TextureSampler const& sampler) noexcept {
        getDefaultInstance()->setParameter(name, texture, sampler);
    }

    /**
     * Sets the color of the given parameter on this material's default instance.
     *
     * @param name The name of the material color parameter
     * @param type Whether the color is specified in the linear or sRGB space
     * @param color The color as a floating point red, green, blue tuple
     *
     * @see getDefaultInstance()
     */
    void setDefaultParameter(const char* name, RgbType type, math::float3 color) noexcept {
        getDefaultInstance()->setParameter(name, type, color);
    }

    /**
     * Sets the color of the given parameter on this material's default instance.
     *
     * @param name The name of the material color parameter
     * @param type Whether the color is specified in the linear or sRGB space
     * @param color The color as a floating point red, green, blue, alpha tuple
     *
     * @see getDefaultInstance()
     */
    void setDefaultParameter(const char* name, RgbaType type, math::float4 color) noexcept {
        getDefaultInstance()->setParameter(name, type, color);
    }

    //! Returns this material's default instance.
    MaterialInstance* getDefaultInstance() noexcept;

    //! Returns this material's default instance.
    MaterialInstance const* getDefaultInstance() const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_MATERIAL_H
