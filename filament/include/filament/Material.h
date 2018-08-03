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
#include <filament/EngineEnums.h>
#include <filament/FilamentAPI.h>
#include <filament/MaterialEnums.h>
#include <filament/MaterialInstance.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>

#include <filament/driver/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/CString.h>

#include <math/vec4.h>

#include <stdint.h>

namespace filaflat {
class MaterialParser;
}

namespace filament {
namespace details {
class  FEngine;
class  FMaterial;
struct ShaderGenerator;
} // namespace details

class Engine;

class UTILS_PUBLIC Material : public FilamentAPI {
    struct BuilderDetails;

public:
    using Variable = filament::Variable;
    using BlendingMode = filament::BlendingMode;
    using Shading = filament::Shading;
    using Interpolation = filament::Interpolation;
    using VertexDomain = filament::VertexDomain;
    using TransparencyMode = filament::TransparencyMode;

    using ParameterType = filament::driver::UniformType;
    using Precision = filament::driver::Precision;
    using SamplerType = filament::driver::SamplerType;
    using SamplerFormat = filament::driver::SamplerFormat;
    using CullingMode = filament::driver::CullingMode;

    using ShaderModel = filament::driver::ShaderModel;

    struct ParameterInfo {
        const char* name;
        bool isSampler;
        union {
            ParameterType type;
            SamplerType samplerType;
        };
        uint32_t count;
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

        // This does not copies the content of the RAM, only copy references.
        // The RAM must stay valid until build() is called.
        Builder& package(const void* payload, size_t size);

        /**
         * Creates the Material object and returns a pointer to it.
         *
         * @param engine Reference to the filament::Engine to associate this Material with.
         *
         * @return pointer to the newly created object or nullptr if exceptions are disabled and
         *         an error occured.
         *
         * @exception utils::PostConditionPanic if a runtime error occurred, such as running out of
         *            memory or other resources.
         * @exception utils::PreConditionPanic if a parameter to a builder function was invalid.
         */
        Material* build(Engine& engine);
    private:
        friend class details::FMaterial;
    };

    MaterialInstance* createInstance() const noexcept;

    const char* getName() const noexcept;
    Shading getShading()  const noexcept;
    Interpolation getInterpolation() const noexcept;
    BlendingMode getBlendingMode() const noexcept;
    VertexDomain getVertexDomain() const noexcept;
    CullingMode getCullingMode() const noexcept;
    TransparencyMode getTransparencyMode() const noexcept;
    bool isColorWriteEnabled() const noexcept;
    bool isDepthWriteEnabled() const noexcept;
    bool isDepthCullingEnabled() const noexcept;
    bool isDoubleSided() const noexcept;
    float getMaskThreshold() const noexcept;
    bool hasShadowMultiplier() const noexcept;
    AttributeBitset getRequiredAttributes() const noexcept;

    size_t getParameterCount() const noexcept;
    size_t getParameters(ParameterInfo* parameters, size_t count) const noexcept;
    bool hasParameter(const char* name) const noexcept;

    template <typename T>
    void setDefaultParameter(const char* name, T value) noexcept {
        getDefaultInstance()->setParameter(name, value);
    }

    void setDefaultParameter(const char* name, Texture const* texture, TextureSampler const& sampler) noexcept {
        getDefaultInstance()->setParameter(name, texture, sampler);
    }

    void setDefaultParameter(const char* name, RgbType type, math::float3 color) noexcept {
        getDefaultInstance()->setParameter(name, type, color);
    }

    void setDefaultParameter(const char* name, RgbaType type, math::float4 color) noexcept {
        getDefaultInstance()->setParameter(name, type, color);
    }

    MaterialInstance* getDefaultInstance() noexcept;
    MaterialInstance const* getDefaultInstance() const noexcept;
};

} // namespace filament

#endif // TNT_FILAMENT_MATERIAL_H
