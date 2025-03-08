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

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/Invocable.h>

#include <math/mathfwd.h>

#include <type_traits>
#include <utility>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

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

    using ParameterType = backend::UniformType;
    using Precision = backend::Precision;
    using SamplerType = backend::SamplerType;
    using SamplerFormat = backend::SamplerFormat;
    using CullingMode = backend::CullingMode;
    using ShaderModel = backend::ShaderModel;
    using SubpassType = backend::SubpassType;

    /**
     * Holds information about a material parameter.
     */
    struct ParameterInfo {
        //! Name of the parameter.
        const char* UTILS_NONNULL name;
        //! Whether the parameter is a sampler (texture).
        bool isSampler;
        //! Whether the parameter is a subpass type.
        bool isSubpass;
        union {
            //! Type of the parameter if the parameter is not a sampler.
            ParameterType type;
            //! Type of the parameter if the parameter is a sampler.
            SamplerType samplerType;
            //! Type of the parameter if the parameter is a subpass.
            SubpassType subpassType;
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

        enum class ShadowSamplingQuality : uint8_t {
            HARD,   // 2x2 PCF
            LOW     // 3x3 gaussian filter
        };

        /**
         * Specifies the material data. The material data is a binary blob produced by
         * libfilamat or by matc.
         *
         * @param payload Pointer to the material data, must stay valid until build() is called.
         * @param size Size of the material data pointed to by "payload" in bytes.
         */
        Builder& package(const void* UTILS_NONNULL payload, size_t size);

        template<typename T>
        using is_supported_constant_parameter_t = std::enable_if_t<
                std::is_same_v<int32_t, T> ||
                std::is_same_v<float, T> ||
                std::is_same_v<bool, T>>;

        /**
         * Specialize a constant parameter specified in the material definition with a concrete
         * value for this material. Once build() is called, this constant cannot be changed.
         * Will throw an exception if the name does not match a constant specified in the
         * material definition or if the type provided does not match.
         *
         * @tparam T The type of constant parameter, either int32_t, float, or bool.
         * @param name The name of the constant parameter specified in the material definition, such
         *             as "myConstant".
         * @param nameLength Length in `char` of the name parameter.
         * @param value The value to use for the constant parameter, must match the type specified
         *              in the material definition.
         */
        template<typename T, typename = is_supported_constant_parameter_t<T>>
        Builder& constant(const char* UTILS_NONNULL name, size_t nameLength, T value);

        /** inline helper to provide the constant name as a null-terminated C string */
        template<typename T, typename = is_supported_constant_parameter_t<T>>
        inline Builder& constant(const char* UTILS_NONNULL name, T value) {
            return constant(name, strlen(name), value);
        }

        Builder& externalImageInfo(const char* UTILS_NONNULL name, uint64_t externalFormat,
                backend::SamplerParams sampler, backend::SamplerYcbcrConversion chroma) noexcept;

        /**
         * Sets the quality of the indirect lights computations. This is only taken into account
         * if this material is lit and in the surface domain. This setting will affect the
         * IndirectLight computation if one is specified on the Scene and Spherical Harmonics
         * are used for the irradiance.
         *
         * @param shBandCount Number of spherical harmonic bands. Must be 1, 2 or 3 (default).
         * @return Reference to this Builder for chaining calls.
         * @see IndirectLight
         */
        Builder& sphericalHarmonicsBandCount(size_t shBandCount) noexcept;

        /**
         * Set the quality of shadow sampling. This is only taken into account
         * if this material is lit and in the surface domain.
         * @param quality
         * @return
         */
        Builder& shadowSamplingQuality(ShadowSamplingQuality quality) noexcept;

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
        Material* UTILS_NULLABLE build(Engine& engine) const;
    private:
        friend class FMaterial;
    };

    using CompilerPriorityQueue = backend:: CompilerPriorityQueue;

    /**
     * Asynchronously ensures that a subset of this Material's variants are compiled. After issuing
     * several Material::compile() calls in a row, it is recommended to call Engine::flush()
     * such that the backend can start the compilation work as soon as possible.
     * The provided callback is guaranteed to be called on the main thread after all specified
     * variants of the material are compiled. This can take hundreds of milliseconds.
     *
     * If all the material's variants are already compiled, the callback will be scheduled as
     * soon as possible, but this might take a few dozen millisecond, corresponding to how
     * many previous frames are enqueued in the backend. This also varies by backend. Therefore,
     * it is recommended to only call this method once per material shortly after creation.
     *
     * If the same variant is scheduled for compilation multiple times, the first scheduling
     * takes precedence; later scheduling are ignored.
     *
     * caveat: A consequence is that if a variant is scheduled on the low priority queue and later
     * scheduled again on the high priority queue, the later scheduling is ignored.
     * Therefore, the second callback could be called before the variant is compiled.
     * However, the first callback, if specified, will trigger as expected.
     *
     * The callback is guaranteed to be called. If the engine is destroyed while some material
     * variants are still compiling or in the queue, these will be discarded and the corresponding
     * callback will be called. In that case however the Material pointer passed to the callback
     * is guaranteed to be invalid (either because it's been destroyed by the user already, or,
     * because it's been cleaned-up by the Engine).
     *
     * UserVariantFilterMask::ALL should be used with caution. Only variants that an application
     * needs should be included in the variants argument. For example, the STE variant is only used
     * for stereoscopic rendering. If an application is not planning to render in stereo, this bit
     * should be turned off to avoid unnecessary material compilations.
     *
     * @param priority      Which priority queue to use, LOW or HIGH.
     * @param variants      Variants to include to the compile command.
     * @param handler       Handler to dispatch the callback or nullptr for the default handler
     * @param callback      callback called on the main thread when the compilation is done on
     *                      by backend.
     */
    void compile(CompilerPriorityQueue priority,
            UserVariantFilterMask variants,
            backend::CallbackHandler* UTILS_NULLABLE handler = nullptr,
            utils::Invocable<void(Material* UTILS_NONNULL)>&& callback = {}) noexcept;

    inline void compile(CompilerPriorityQueue priority,
            UserVariantFilterBit variants,
            backend::CallbackHandler* UTILS_NULLABLE handler = nullptr,
            utils::Invocable<void(Material* UTILS_NONNULL)>&& callback = {}) noexcept {
        compile(priority, UserVariantFilterMask(variants), handler,
                std::forward<utils::Invocable<void(Material* UTILS_NONNULL)>>(callback));
    }

    inline void compile(CompilerPriorityQueue priority,
            backend::CallbackHandler* UTILS_NULLABLE handler = nullptr,
            utils::Invocable<void(Material* UTILS_NONNULL)>&& callback = {}) noexcept {
        compile(priority, UserVariantFilterBit::ALL, handler,
                std::forward<utils::Invocable<void(Material* UTILS_NONNULL)>>(callback));
    }

    /**
     * Creates a new instance of this material. Material instances should be freed using
     * Engine::destroy(const MaterialInstance*).
     *
     * @param name Optional name to associate with the given material instance. If this is null,
     * then the instance inherits the material's name.
     *
     * @return A pointer to the new instance.
     */
    MaterialInstance* UTILS_NONNULL createInstance(const char* UTILS_NULLABLE name = nullptr) const noexcept;

    //! Returns the name of this material as a null-terminated string.
    const char* UTILS_NONNULL getName() const noexcept;

    //! Returns the shading model of this material.
    Shading getShading() const noexcept;

    //! Returns the interpolation mode of this material. This affects how variables are interpolated.
    Interpolation getInterpolation() const noexcept;

    //! Returns the blending mode of this material.
    BlendingMode getBlendingMode() const noexcept;

    //! Returns the vertex domain of this material.
    VertexDomain getVertexDomain() const noexcept;

    //! Returns the material's supported variants
    UserVariantFilterMask getSupportedVariants() const noexcept;

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

    //! Indicates whether this material uses alpha to coverage.
    bool isAlphaToCoverageEnabled() const noexcept;

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

    //! Return the refraction type used by this material.
    RefractionType getRefractionType() const noexcept;

    //! Returns the reflection mode used by this material.
    ReflectionMode getReflectionMode() const noexcept;

    //! Returns the minimum required feature level for this material.
    backend::FeatureLevel getFeatureLevel() const noexcept;

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
    size_t getParameters(ParameterInfo* UTILS_NONNULL parameters, size_t count) const noexcept;

    //! Indicates whether a parameter of the given name exists on this material.
    bool hasParameter(const char* UTILS_NONNULL name) const noexcept;

    //! Indicates whether an existing parameter is a sampler or not.
    bool isSampler(const char* UTILS_NONNULL name) const noexcept;

    /**
     * Sets the value of the given parameter on this material's default instance.
     *
     * @param name The name of the material parameter
     * @param value The value of the material parameter
     *
     * @see getDefaultInstance()
     */
    template <typename T>
    void setDefaultParameter(const char* UTILS_NONNULL name, T value) noexcept {
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
    void setDefaultParameter(const char* UTILS_NONNULL name,
            Texture const* UTILS_NULLABLE texture, TextureSampler const& sampler) noexcept {
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
    void setDefaultParameter(const char* UTILS_NONNULL name, RgbType type, math::float3 color) noexcept {
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
    void setDefaultParameter(const char* UTILS_NONNULL name, RgbaType type, math::float4 color) noexcept {
        getDefaultInstance()->setParameter(name, type, color);
    }

    //! Returns this material's default instance.
    MaterialInstance* UTILS_NONNULL getDefaultInstance() noexcept;

    //! Returns this material's default instance.
    MaterialInstance const* UTILS_NONNULL getDefaultInstance() const noexcept;

protected:
    // prevent heap allocation
    ~Material() = default;
};

} // namespace filament

#endif // TNT_FILAMENT_MATERIAL_H
