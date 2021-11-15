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

#include <iosfwd>
#include <string>

#include <utils/compiler.h>
#include <utils/Log.h>

#include <filamat/MaterialBuilder.h>

#include <backend/DriverEnums.h>
#include <private/filament/EngineEnums.h>
#include <filament/MaterialEnums.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/UniformInterfaceBlock.h>
#include <private/filament/SubpassInfo.h>

#include <utils/sstream.h>

#include <private/filament/Variant.h>

namespace filamat {

class UTILS_PRIVATE CodeGenerator {
    using ShaderType = filament::backend::ShaderType;
    using TargetApi = MaterialBuilder::TargetApi;
    using TargetLanguage = MaterialBuilder::TargetLanguage;
    using ShaderQuality = MaterialBuilder::ShaderQuality;
public:
    CodeGenerator(filament::backend::ShaderModel shaderModel,
            TargetApi targetApi, TargetLanguage targetLanguage) noexcept
            : mShaderModel(shaderModel), mTargetApi(targetApi), mTargetLanguage(targetLanguage) {
        if (targetApi == TargetApi::ALL) {
            utils::slog.e << "Must resolve target API before codegen." << utils::io::endl;
            std::terminate();
        }
    }

    filament::backend::ShaderModel getShaderModel() const noexcept { return mShaderModel; }

    // insert a separator (can be a new line)
    utils::io::sstream& generateSeparator(utils::io::sstream& out) const;

    // generate prolog for the given shader
    utils::io::sstream& generateProlog(utils::io::sstream& out, ShaderType type, bool hasExternalSamplers) const;

    utils::io::sstream& generateEpilog(utils::io::sstream& out) const;

    // generate common functions for the given shader
    utils::io::sstream& generateCommon(utils::io::sstream& out, ShaderType type) const;
    utils::io::sstream& generateCommonMaterial(utils::io::sstream& out, ShaderType type) const;

    utils::io::sstream& generateFog(utils::io::sstream& out, ShaderType type) const;

    // generate the shader's main()
    utils::io::sstream& generateShaderMain(utils::io::sstream& out, ShaderType type) const;
    utils::io::sstream& generatePostProcessMain(utils::io::sstream& out, ShaderType type) const;

    // generate the shader's code for the lit shading model
    utils::io::sstream& generateShaderLit(utils::io::sstream& out, ShaderType type,
            filament::Variant variant, filament::Shading shading, bool customSurfaceShading) const;

    // generate the shader's code for the unlit shading model
    utils::io::sstream& generateShaderUnlit(utils::io::sstream& out, ShaderType type,
            filament::Variant variant, bool hasShadowMultiplier) const;

    // generate declarations for custom interpolants
    utils::io::sstream& generateVariable(utils::io::sstream& out, ShaderType type,
            const utils::CString& name, size_t index) const;

    // generate declarations for non-custom "in" variables
    utils::io::sstream& generateShaderInputs(utils::io::sstream& out, ShaderType type,
        const filament::AttributeBitset& attributes, filament::Interpolation interpolation) const;
    utils::io::sstream& generatePostProcessInputs(utils::io::sstream& out, ShaderType type) const;

    // generate declarations for custom output variables
    utils::io::sstream& generateOutput(utils::io::sstream& out, ShaderType type,
            const utils::CString& name, size_t index,
            MaterialBuilder::VariableQualifier qualifier,
            MaterialBuilder::OutputType outputType) const;

    // generate no-op shader for depth prepass
    utils::io::sstream& generateDepthShaderMain(utils::io::sstream& out, ShaderType type) const;

    // generate uniforms
    utils::io::sstream& generateUniforms(utils::io::sstream& out, ShaderType type, uint8_t binding,
            const filament::UniformInterfaceBlock& uib) const;

    // generate samplers
    utils::io::sstream& generateSamplers(
        utils::io::sstream& out, uint8_t firstBinding, const filament::SamplerInterfaceBlock& sib) const;

    // generate subpass
    utils::io::sstream& generateSubpass(utils::io::sstream& out,
            filament::SubpassInfo subpass) const;

    // generate material properties getters
    utils::io::sstream& generateMaterialProperty(utils::io::sstream& out,
            MaterialBuilder::Property property, bool isSet) const;

    utils::io::sstream& generateQualityDefine(utils::io::sstream& out, ShaderQuality quality) const;

    utils::io::sstream& generateDefine(utils::io::sstream& out, const char* name, bool value) const;
    utils::io::sstream& generateDefine(utils::io::sstream& out, const char* name, uint32_t value) const;
    utils::io::sstream& generateDefine(utils::io::sstream& out, const char* name, const char* string) const;
    utils::io::sstream& generateIndexedDefine(utils::io::sstream& out, const char* name,
            uint32_t index, uint32_t value) const;

    utils::io::sstream& generatePostProcessGetters(utils::io::sstream& out, ShaderType type) const;
    utils::io::sstream& generateGetters(utils::io::sstream& out, ShaderType type) const;
    utils::io::sstream& generateParameters(utils::io::sstream& out, ShaderType type) const;

    static void fixupExternalSamplers(
            std::string& shader, filament::SamplerInterfaceBlock const& sib) noexcept;

private:
    filament::backend::Precision getDefaultPrecision(ShaderType type) const;
    filament::backend::Precision getDefaultUniformPrecision() const;

    const char* getUniformPrecisionQualifier(filament::backend::UniformType type,
            filament::backend::Precision precision,
            filament::backend::Precision uniformPrecision,
            filament::backend::Precision defaultPrecision) const noexcept;

    // return type name of sampler  (e.g.: "sampler2D")
    char const* getSamplerTypeName(filament::backend::SamplerType type,
            filament::backend::SamplerFormat format, bool multisample) const noexcept;

    // return name of the material property (e.g.: "ROUGHNESS")
    static char const* getConstantName(MaterialBuilder::Property property) noexcept;

    static char const* getPrecisionQualifier(filament::backend::Precision precision,
            filament::backend::Precision defaultPrecision) noexcept;

    filament::backend::ShaderModel mShaderModel;
    TargetApi mTargetApi;
    TargetLanguage mTargetLanguage;

    // return type name of uniform  (e.g.: "vec3", "vec4", "float")
    static char const* getUniformTypeName(filament::UniformInterfaceBlock::UniformInfo const& info) noexcept;

    // return type name of output  (e.g.: "vec3", "vec4", "float")
    static char const* getOutputTypeName(MaterialBuilder::OutputType type) noexcept;

    // return qualifier for the specified interpolation mode
    static char const* getInterpolationQualifier(filament::Interpolation interpolation) noexcept;

    static bool hasPrecision(filament::UniformInterfaceBlock::Type type) noexcept;
};

} // namespace filamat

#endif // TNT_FILAMENT_CODEGENERATOR_H
