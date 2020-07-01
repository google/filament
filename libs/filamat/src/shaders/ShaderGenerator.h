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

#include <algorithm>

#include <filament/MaterialEnums.h>

#include <filamat/MaterialBuilder.h>

#include "MaterialInfo.h"

#include <utils/CString.h>
#include <utils/sstream.h>

namespace filamat {

class CodeGenerator;

class ShaderGenerator {
public:
    ShaderGenerator(
            MaterialBuilder::PropertyList const& properties,
            MaterialBuilder::VariableList const& variables,
            MaterialBuilder::PreprocessorDefineList const& defines,
            utils::CString const& materialCode,
            size_t lineOffset,
            utils::CString const& materialVertexCode,
            size_t vertexLineOffset,
            MaterialBuilder::MaterialDomain materialDomain) noexcept;

    std::string createVertexProgram(filament::backend::ShaderModel sm,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            MaterialInfo const& material, uint8_t variantKey,
            filament::Interpolation interpolation,
            filament::VertexDomain vertexDomain) const noexcept;
    std::string createFragmentProgram(filament::backend::ShaderModel sm,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            MaterialInfo const& material, uint8_t variantKey,
            filament::Interpolation interpolation) const noexcept;

    bool hasCustomDepthShader() const noexcept;

    /**
     * When a GLSL shader is optimized we run it through an intermediate SPIR-V
     * representation. Unfortunately external samplers cannot be used with SPIR-V
     * at this time, so we must transform them into regular 2D samplers. This
     * fixup step can be used to turn the samplers back into external samplers after
     * the optimizations have been applied.
     */
    void fixupExternalSamplers(filament::backend::ShaderModel sm, std::string& shader,
            MaterialInfo const& material) const noexcept;

private:

    std::string createPostProcessVertexProgram(filament::backend::ShaderModel sm,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            MaterialInfo const& material, uint8_t variant,
            const filament::SamplerBindingMap& samplerBindingMap) const noexcept;

    std::string createPostProcessFragmentProgram(filament::backend::ShaderModel sm,
            MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
            MaterialInfo const& material, uint8_t variant,
            const filament::SamplerBindingMap& samplerBindingMap) const noexcept;

    MaterialBuilder::PropertyList mProperties;
    MaterialBuilder::VariableList mVariables;
    MaterialBuilder::MaterialDomain mMaterialDomain;
    MaterialBuilder::PreprocessorDefineList mDefines;
    utils::CString mMaterialCode;
    utils::CString mMaterialVertexCode;
    size_t mMaterialLineOffset;
    size_t mMaterialVertexLineOffset;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SHADERGENERATOR_H
