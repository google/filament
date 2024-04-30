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

#ifndef TNT_FILAMENT_CODEGENERATOR_H
#define TNT_FILAMENT_CODEGENERATOR_H


#include "MaterialInfo.h"

#include <filamat/MaterialBuilder.h>

#include <filament/MaterialEnums.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/BufferInterfaceBlock.h>
#include <private/filament/SubpassInfo.h>
#include <private/filament/Variant.h>

#include <backend/DriverEnums.h>

#include <utils/compiler.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Log.h>
#include <utils/sstream.h>

#include <exception>
#include <iosfwd>
#include <string>
#include <variant>

namespace filamat {

class UTILS_PRIVATE CodeGenerator {
    using ShaderModel = filament::backend::ShaderModel;
    using ShaderStage = filament::backend::ShaderStage;
    using FeatureLevel = filament::backend::FeatureLevel;
    using TargetApi = MaterialBuilder::TargetApi;
    using TargetLanguage = MaterialBuilder::TargetLanguage;
    using ShaderQuality = MaterialBuilder::ShaderQuality;

public:
    CodeGenerator(ShaderModel shaderModel,
            TargetApi targetApi,
            TargetLanguage targetLanguage,
            FeatureLevel featureLevel) noexcept
            : mShaderModel(shaderModel),
              mTargetApi(targetApi),
              mTargetLanguage(targetLanguage),
              mFeatureLevel(featureLevel) {
        if (targetApi == TargetApi::ALL) {
            utils::slog.e << "Must resolve target API before codegen." << utils::io::endl;
            std::terminate();
        }
    }

    filament::backend::ShaderModel getShaderModel() const noexcept { return mShaderModel; }

    // insert a separator (can be a new line)
    static utils::io::sstream& generateSeparator(utils::io::sstream& out);

    // generate prolog for the given shader
    utils::io::sstream& generateProlog(utils::io::sstream& out, ShaderStage stage,
            MaterialInfo const& material, filament::Variant v) const;

    static utils::io::sstream& generateEpilog(utils::io::sstream& out);

    static utils::io::sstream& generateCommonTypes(utils::io::sstream& out, ShaderStage stage);

    // generate common functions for the given shader
    static utils::io::sstream& generateCommon(utils::io::sstream& out, ShaderStage stage);
    static utils::io::sstream& generatePostProcessCommon(utils::io::sstream& out, ShaderStage type);
    static utils::io::sstream& generateCommonMaterial(utils::io::sstream& out, ShaderStage type);

    static utils::io::sstream& generateFog(utils::io::sstream& out, ShaderStage type);

    // generate the shader's main()
    static utils::io::sstream& generateShaderMain(utils::io::sstream& out, ShaderStage stage);
    static utils::io::sstream& generatePostProcessMain(utils::io::sstream& out, ShaderStage type);

    // generate the shader's code for the lit shading model
    static utils::io::sstream& generateShaderLit(utils::io::sstream& out, ShaderStage type,
            filament::Variant variant, filament::Shading shading, bool customSurfaceShading);

    // generate the shader's code for the unlit shading model
    static utils::io::sstream& generateShaderUnlit(utils::io::sstream& out, ShaderStage type,
            filament::Variant variant, bool hasShadowMultiplier);

    // generate the shader's code for the screen-space reflections
    static utils::io::sstream& generateShaderReflections(utils::io::sstream& out, ShaderStage type);

    // generate declarations for custom interpolants
    static utils::io::sstream& generateVariable(utils::io::sstream& out, ShaderStage stage,
            const utils::CString& name, size_t index);

    // generate declarations for non-custom "in" variables
    utils::io::sstream& generateShaderInputs(utils::io::sstream& out, ShaderStage type,
        const filament::AttributeBitset& attributes, filament::Interpolation interpolation) const;
    static utils::io::sstream& generatePostProcessInputs(utils::io::sstream& out, ShaderStage type);

    // generate declarations for custom output variables
    utils::io::sstream& generateOutput(utils::io::sstream& out, ShaderStage type,
            const utils::CString& name, size_t index,
            MaterialBuilder::VariableQualifier qualifier,
            MaterialBuilder::Precision precision,
            MaterialBuilder::OutputType outputType) const;

    // generate no-op shader for depth prepass
    static utils::io::sstream& generateDepthShaderMain(utils::io::sstream& out, ShaderStage type);

    // generate samplers
    utils::io::sstream& generateSamplers(utils::io::sstream& out,
            filament::SamplerBindingPoints bindingPoint, uint8_t firstBinding,
            const filament::SamplerInterfaceBlock& sib) const;

    // generate subpass
    static utils::io::sstream& generateSubpass(utils::io::sstream& out,
            filament::SubpassInfo subpass);

    // generate uniforms
    utils::io::sstream& generateUniforms(utils::io::sstream& out, ShaderStage stage,
            filament::UniformBindingPoints binding, const filament::BufferInterfaceBlock& uib) const;

    // generate buffers
    utils::io::sstream& generateBuffers(utils::io::sstream& out,
            MaterialInfo::BufferContainer const& buffers) const;

    // generate an interface block
    utils::io::sstream& generateBufferInterfaceBlock(utils::io::sstream& out, ShaderStage stage,
            uint32_t binding, const filament::BufferInterfaceBlock& uib) const;

    // generate material properties getters
    static utils::io::sstream& generateMaterialProperty(utils::io::sstream& out,
            MaterialBuilder::Property property, bool isSet);

    utils::io::sstream& generateQualityDefine(utils::io::sstream& out, ShaderQuality quality) const;

    static utils::io::sstream& generateDefine(utils::io::sstream& out, const char* name, bool value);
    static utils::io::sstream& generateDefine(utils::io::sstream& out, const char* name, uint32_t value);
    static utils::io::sstream& generateDefine(utils::io::sstream& out, const char* name, const char* string);
    static utils::io::sstream& generateIndexedDefine(utils::io::sstream& out, const char* name,
            uint32_t index, uint32_t value);

    utils::io::sstream& generateSpecializationConstant(utils::io::sstream& out,
            const char* name, uint32_t id, std::variant<int, float, bool> value) const;

    utils::io::sstream& generateVertexPushConstants(utils::io::sstream& out,
            size_t const layoutLocation) const;
    utils::io::sstream& generateFragmentPushConstants(utils::io::sstream& out,
            size_t const layoutLocation) const;

    static utils::io::sstream& generatePostProcessGetters(utils::io::sstream& out, ShaderStage type);
    static utils::io::sstream& generateGetters(utils::io::sstream& out, ShaderStage stage);
    static utils::io::sstream& generateParameters(utils::io::sstream& out, ShaderStage type);

    static void fixupExternalSamplers(
            std::string& shader, filament::SamplerInterfaceBlock const& sib,
            FeatureLevel featureLevel) noexcept;

    // These constants must match the equivalent in MetalState.h.
    // These values represent the starting index for uniform, ssbo, and sampler group [[buffer(n)]]
    // bindings. See the chart at the top of MetalState.h.
    static constexpr uint32_t METAL_UNIFORM_BUFFER_BINDING_START = 17u;
    static constexpr uint32_t METAL_SAMPLER_GROUP_BINDING_START = 27u;
    static constexpr uint32_t METAL_SSBO_BINDING_START = 0;

private:
    filament::backend::Precision getDefaultPrecision(ShaderStage stage) const;
    filament::backend::Precision getDefaultUniformPrecision() const;

    utils::io::sstream& generateInterfaceFields(utils::io::sstream& out,
            utils::FixedCapacityVector<filament::BufferInterfaceBlock::FieldInfo> const& infos,
            filament::backend::Precision defaultPrecision) const;

    utils::io::sstream& generateUboAsPlainUniforms(utils::io::sstream& out, ShaderStage stage,
            const filament::BufferInterfaceBlock& uib) const;

    static const char* getUniformPrecisionQualifier(filament::backend::UniformType type,
            filament::backend::Precision precision,
            filament::backend::Precision uniformPrecision,
            filament::backend::Precision defaultPrecision) noexcept;

    // return type name of sampler  (e.g.: "sampler2D")
    char const* getSamplerTypeName(filament::backend::SamplerType type,
            filament::backend::SamplerFormat format, bool multisample) const noexcept;

    // return name of the material property (e.g.: "ROUGHNESS")
    static char const* getConstantName(MaterialBuilder::Property property) noexcept;

    static char const* getPrecisionQualifier(filament::backend::Precision precision) noexcept;

    // return type (e.g.: "vec3", "vec4", "float")
    static char const* getTypeName(UniformType type) noexcept;

    // return type name of uniform Field (e.g.: "vec3", "vec4", "float")
    static char const* getUniformTypeName(filament::BufferInterfaceBlock::FieldInfo const& info) noexcept;

    // return type name of output  (e.g.: "vec3", "vec4", "float")
    static char const* getOutputTypeName(MaterialBuilder::OutputType type) noexcept;

    // return qualifier for the specified interpolation mode
    static char const* getInterpolationQualifier(filament::Interpolation interpolation) noexcept;

    static bool hasPrecision(filament::BufferInterfaceBlock::Type type) noexcept;

    ShaderModel mShaderModel;
    TargetApi mTargetApi;
    TargetLanguage mTargetLanguage;
    FeatureLevel mFeatureLevel;
};

} // namespace filamat

#endif // TNT_FILAMENT_CODEGENERATOR_H
