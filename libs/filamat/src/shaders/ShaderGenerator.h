/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_SHADERGENERATOR_H
#define TNT_FILAMENT_DETAILS_SHADERGENERATOR_H


#include "MaterialInfo.h"

#include <filament/MaterialEnums.h>

#include <filamat/MaterialBuilder.h>

#include <private/filament/Variant.h>

#include <utils/CString.h>
#include <utils/sstream.h>

#include <algorithm>

namespace filamat {

class CodeGenerator;

class ShaderGenerator {
public:
    ShaderGenerator(
            MaterialBuilder::PropertyList const& properties,
            MaterialBuilder::VariableList const& variables,
            MaterialBuilder::OutputList const& outputs,
            MaterialBuilder::PreprocessorDefineList const& defines,
            MaterialBuilder::ConstantList const& constants,
            utils::CString const& materialCode,
            size_t lineOffset,
            utils::CString const& materialVertexCode,
            size_t vertexLineOffset,
            MaterialBuilder::MaterialDomain materialDomain) noexcept;

    std::string createVertexProgram(filament::backend::ShaderModel shaderModel,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            MaterialBuilder::FeatureLevel featureLevel,
            MaterialInfo const& material, filament::Variant variant,
            filament::Interpolation interpolation,
            filament::VertexDomain vertexDomain) const noexcept;

    std::string createFragmentProgram(filament::backend::ShaderModel shaderModel,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            MaterialBuilder::FeatureLevel featureLevel,
            MaterialInfo const& material, filament::Variant variant,
            filament::Interpolation interpolation) const noexcept;

    std::string createComputeProgram(filament::backend::ShaderModel shaderModel,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            MaterialBuilder::FeatureLevel featureLevel,
            MaterialInfo const& material) const noexcept;

    /**
     * When a GLSL shader is optimized we run it through an intermediate SPIR-V
     * representation. Unfortunately external samplers cannot be used with SPIR-V
     * at this time, so we must transform them into regular 2D samplers. This
     * fixup step can be used to turn the samplers back into external samplers after
     * the optimizations have been applied.
     */
    static void fixupExternalSamplers(filament::backend::ShaderModel sm, std::string& shader,
            MaterialBuilder::FeatureLevel featureLevel,
            MaterialInfo const& material) noexcept;

private:
    static void generateVertexDomainDefines(utils::io::sstream& out,
            filament::VertexDomain domain) noexcept;

    static void generateSurfaceMaterialVariantProperties(utils::io::sstream& out,
            MaterialBuilder::PropertyList const properties,
            const MaterialBuilder::PreprocessorDefineList& defines) noexcept;

    static void generateSurfaceMaterialVariantDefines(utils::io::sstream& out,
            filament::backend::ShaderStage stage,
            MaterialBuilder::FeatureLevel featureLevel,
            MaterialInfo const& material, filament::Variant variant) noexcept;

    static void generatePostProcessMaterialVariantDefines(utils::io::sstream& out,
            filament::PostProcessVariant variant) noexcept;

    static void generateUserSpecConstants(
            const CodeGenerator& cg, utils::io::sstream& fs,
            MaterialBuilder::ConstantList const& constants);

    std::string createPostProcessVertexProgram(filament::backend::ShaderModel sm,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            MaterialBuilder::FeatureLevel featureLevel,
            MaterialInfo const& material, filament::Variant::type_t variantKey) const noexcept;

    std::string createPostProcessFragmentProgram(filament::backend::ShaderModel sm,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            MaterialBuilder::FeatureLevel featureLevel,
            MaterialInfo const& material, uint8_t variant) const noexcept;

    static void appendShader(utils::io::sstream& ss,
            const utils::CString& shader, size_t lineOffset) noexcept;

    static bool hasSkinningOrMorphing(
            filament::Variant variant,
            MaterialBuilder::FeatureLevel featureLevel) noexcept;

    static bool hasStereo(
            filament::Variant variant,
            MaterialBuilder::FeatureLevel featureLevel) noexcept;

    MaterialBuilder::PropertyList mProperties;
    MaterialBuilder::VariableList mVariables;
    MaterialBuilder::OutputList mOutputs;
    MaterialBuilder::MaterialDomain mMaterialDomain;
    MaterialBuilder::PreprocessorDefineList mDefines;
    MaterialBuilder::ConstantList mConstants;
    utils::CString mMaterialFragmentCode;   // fragment or compute code
    utils::CString mMaterialVertexCode;
    size_t mMaterialLineOffset;
    size_t mMaterialVertexLineOffset;
    bool mIsMaterialVertexShaderEmpty;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SHADERGENERATOR_H
