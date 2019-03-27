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

#include <private/filament/Variant.h>

namespace filamat {

class UTILS_PRIVATE CodeGenerator {
    using ShaderType = filament::backend::ShaderType;
    using TargetApi = MaterialBuilder::TargetApi;
    using TargetLanguage = MaterialBuilder::TargetLanguage;
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
    std::ostream& generateSeparator(std::ostream& out) const;

    // generate prolog for the given shader
    std::ostream& generateProlog(std::ostream& out, ShaderType type, bool hasExternalSamplers) const;

    std::ostream& generateEpilog(std::ostream& out) const;

    // generate common functions for the given shader
    std::ostream& generateCommon(std::ostream& out, ShaderType type) const;
    std::ostream& generateCommonMaterial(std::ostream& out, ShaderType type) const;

    // generate the shader's main()
    std::ostream& generateShaderMain(std::ostream& out, ShaderType type) const;
    std::ostream& generatePostProcessMain(std::ostream& out, ShaderType type,
            filament::PostProcessStage variant) const;

    // generate the shader's code for the lit shading model
    std::ostream& generateShaderLit(std::ostream& out, ShaderType type,
            filament::Variant variant, filament::Shading shading) const;

    // generate the shader's code for the unlit shading model
    std::ostream& generateShaderUnlit(std::ostream& out, ShaderType type,
            filament::Variant variant, bool hasShadowMultiplier) const;

    // generate declarations for custom interpolants
    std::ostream& generateVariable(std::ostream& out, ShaderType type,
            const utils::CString& name, size_t index) const;

    // generate declarations for non-custom "in" variables
    std::ostream& generateShaderInputs(std::ostream& out, ShaderType type,
        const filament::AttributeBitset& attributes, filament::Interpolation interpolation) const;

    // generate no-op shader for depth prepass
    std::ostream& generateDepthShaderMain(std::ostream& out, ShaderType type) const;

    // generate uniforms
    std::ostream& generateUniforms(std::ostream& out, ShaderType type, uint8_t binding,
            const filament::UniformInterfaceBlock& uib) const;

    // generate samplers
    std::ostream& generateSamplers(
        std::ostream& out, uint8_t firstBinding, const filament::SamplerInterfaceBlock& sib) const;

    // generate material properties getters
    std::ostream& generateMaterialProperty(std::ostream& out,
            MaterialBuilder::Property property, bool isSet) const;

    std::ostream& generateFunction(std::ostream& out,
            const char* returnType, const char* name, const char* body) const;

    std::ostream& generateDefine(std::ostream& out, const char* name, bool value) const;
    std::ostream& generateDefine(std::ostream& out, const char* name, float value) const;
    std::ostream& generateDefine(std::ostream& out, const char* name, uint32_t value) const;
    std::ostream& generateDefine(std::ostream& out, const char* name, const char* string) const;

    std::ostream& generateGetters(std::ostream& out, ShaderType type) const;
    std::ostream& generateParameters(std::ostream& out, ShaderType type) const;

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
    static char const* getUniformTypeName(filament::UniformInterfaceBlock::Type uniformType) noexcept;

    // return qualifier for the specified interpolation mode
    static char const* getInterpolationQualifier(filament::Interpolation interpolation) noexcept;

    static bool hasPrecision(filament::UniformInterfaceBlock::Type type) noexcept;
};

} // namespace filamat

#endif // TNT_FILAMENT_CODEGENERATOR_H
