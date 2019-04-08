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

#include "ShaderGenerator.h"

#include <sstream>

#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/UniformInterfaceBlock.h>
#include <filament/MaterialEnums.h>

#include <private/filament/SibGenerator.h>
#include <private/filament/UibGenerator.h>
#include <private/filament/Variant.h>

#include <utils/CString.h>

#include "filamat/MaterialBuilder.h"
#include "CodeGenerator.h"

using namespace filament;
using namespace filament::backend;

namespace filamat {

static const char* getShadingDefine(filament::Shading shading) noexcept {
    switch (shading) {
        case filament::Shading::LIT:                 return "SHADING_MODEL_LIT";
        case filament::Shading::UNLIT:               return "SHADING_MODEL_UNLIT";
        case filament::Shading::SUBSURFACE:          return "SHADING_MODEL_SUBSURFACE";
        case filament::Shading::CLOTH:               return "SHADING_MODEL_CLOTH";
        case filament::Shading::SPECULAR_GLOSSINESS: return "SHADING_MODEL_SPECULAR_GLOSSINESS";
    }
}

static void generateMaterialDefines(std::ostream& os, const CodeGenerator& cg,
        MaterialBuilder::PropertyList const properties) noexcept {
    for (size_t i = 0; i < MaterialBuilder::MATERIAL_PROPERTIES_COUNT; i++) {
        cg.generateMaterialProperty(os, static_cast<MaterialBuilder::Property>(i), properties[i]);
    }
}

static void generateVertexDomain(const CodeGenerator& cg, std::stringstream& vs,
        filament::VertexDomain domain) noexcept {
    switch (domain) {
        case VertexDomain::OBJECT:
            cg.generateDefine(vs, "VERTEX_DOMAIN_OBJECT", true);
            break;
        case VertexDomain::WORLD:
            cg.generateDefine(vs, "VERTEX_DOMAIN_WORLD", true);
            break;
        case VertexDomain::VIEW:
            cg.generateDefine(vs, "VERTEX_DOMAIN_VIEW", true);
            break;
        case VertexDomain::DEVICE:
            cg.generateDefine(vs, "VERTEX_DOMAIN_DEVICE", true);
            break;
    }
}

static size_t countLines(const std::stringstream& ss) noexcept {
    std::string s = ss.str();
    size_t lines = 0;
    for (char i : s) {
        if (i == '\n') lines++;
    }
    return lines;
}

static size_t countLines(const utils::CString& s) noexcept {
    size_t lines = 0;
    for (char i : s) {
        if (i == '\n') lines++;
    }
    return lines;
}

static void appendShader(std::stringstream& ss,
        const utils::CString& shader, size_t lineOffset) noexcept {
    if (!shader.empty()) {
        size_t lines = countLines(ss);
        ss << "#line " << lineOffset;
        if (shader[0] != '\n') ss << "\n";
        ss << shader.c_str();
        if (shader[shader.size() - 1] != '\n') {
            ss << "\n";
            lines++;
        }
        // + 2 to account for the #line directives we just added
        ss << "#line " << lines + countLines(shader) + 2 << "\n";
    }
}

ShaderGenerator::ShaderGenerator(
        MaterialBuilder::PropertyList const& properties,
        MaterialBuilder::VariableList const& variables,
        utils::CString const& materialCode, size_t lineOffset,
        utils::CString const& materialVertexCode, size_t vertexLineOffset) noexcept {

    std::copy(std::begin(properties), std::end(properties), std::begin(mProperties));
    std::copy(std::begin(variables), std::end(variables), std::begin(mVariables));

    mMaterialCode = materialCode;
    mMaterialVertexCode = materialVertexCode;
    mMaterialLineOffset = lineOffset;
    mMaterialVertexLineOffset = vertexLineOffset;

    if (mMaterialCode.empty()) {
        mMaterialCode =
                utils::CString("void material(inout MaterialInputs m) {\n    prepareMaterial(m);\n}\n");
    }
    if (mMaterialVertexCode.empty()) {
        mMaterialVertexCode =
                utils::CString("void materialVertex(inout MaterialVertexInputs m) {\n}\n");
    }
}

const std::string ShaderGenerator::createVertexProgram(filament::backend::ShaderModel shaderModel,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& material, uint8_t variantKey, filament::Interpolation interpolation,
        filament::VertexDomain vertexDomain) const noexcept {
    std::stringstream vs;

    const CodeGenerator cg(shaderModel, targetApi, targetLanguage);
    const bool lit = material.isLit;
    const filament::Variant variant(variantKey);

    cg.generateProlog(vs, ShaderType::VERTEX, material.hasExternalSamplers);

    cg.generateDefine(vs, "FLIP_UV_ATTRIBUTE", material.flipUV);
    cg.generateDefine(vs, "GEOMETRIC_SPECULAR_AA_NORMAL", material.limitOverInterpolation);

    bool litVariants = lit || material.hasShadowMultiplier;
    cg.generateDefine(vs, "HAS_DIRECTIONAL_LIGHTING", litVariants && variant.hasDirectionalLighting());
    cg.generateDefine(vs, "HAS_SHADOWING", litVariants && variant.hasShadowReceiver());
    cg.generateDefine(vs, "HAS_SHADOW_MULTIPLIER", material.hasShadowMultiplier);
    cg.generateDefine(vs, "HAS_SKINNING", variant.hasSkinning());
    cg.generateDefine(vs, getShadingDefine(material.shading), true);
    generateMaterialDefines(vs, cg, mProperties);

    AttributeBitset attributes = material.requiredAttributes;
    if (variant.hasSkinning()) {
        attributes.set(VertexAttribute::BONE_INDICES);
        attributes.set(VertexAttribute::BONE_WEIGHTS);
    }
    cg.generateShaderInputs(vs, ShaderType::VERTEX, attributes, interpolation);

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        cg.generateVariable(vs, ShaderType::VERTEX, variable, variableIndex++);
    }

    // materials defines
    generateVertexDomain(cg, vs, vertexDomain);

    // uniforms
    cg.generateUniforms(vs, ShaderType::VERTEX,
            BindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(vs, ShaderType::VERTEX,
            BindingPoints::PER_RENDERABLE, UibGenerator::getPerRenderableUib());
    if (variant.hasSkinning()) {
        cg.generateUniforms(vs, ShaderType::VERTEX,
                BindingPoints::PER_RENDERABLE_BONES,
                UibGenerator::getPerRenderableBonesUib());
    }
    cg.generateUniforms(vs, ShaderType::VERTEX,
            BindingPoints::PER_MATERIAL_INSTANCE, material.uib);
    cg.generateSeparator(vs);
    // TODO: should we generate per-view SIB in the vertex shader?
    cg.generateSamplers(vs,
            material.samplerBindings.getBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    // shader code
    cg.generateCommon(vs, ShaderType::VERTEX);
    cg.generateGetters(vs, ShaderType::VERTEX);
    cg.generateCommonMaterial(vs, ShaderType::VERTEX);

    if (variant.isDepthPass() &&
        (material.blendingMode != BlendingMode::MASKED) &&
        !hasCustomDepthShader()) {
        // these variants are special and are treated as DEPTH variants. Filament will never
        // request that variant for the color pass.
        cg.generateDepthShaderMain(vs, ShaderType::VERTEX);
    } else {
        // main entry point
        appendShader(vs, mMaterialVertexCode, mMaterialVertexLineOffset);
        cg.generateShaderMain(vs, ShaderType::VERTEX);
    }

    cg.generateEpilog(vs);

    return vs.str();
}

bool ShaderGenerator::hasCustomDepthShader() const noexcept {
    for (const auto& variable : mVariables) {
        if (!variable.empty()) {
            return true;
        }
    }
    return false;
}

const std::string ShaderGenerator::createFragmentProgram(filament::backend::ShaderModel shaderModel,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& material, uint8_t variantKey,
        filament::Interpolation interpolation) const noexcept {

    const CodeGenerator cg(shaderModel, targetApi, targetLanguage);
    const bool lit = material.isLit;
    const filament::Variant variant(variantKey);

    std::stringstream fs;
    cg.generateProlog(fs, ShaderType::FRAGMENT, material.hasExternalSamplers);

    cg.generateDefine(fs, "IBL_USE_RGBM", filament::CONFIG_IBL_RGBM);
    cg.generateDefine(fs, "IBL_MAX_MIP_LEVEL", std::log2f(filament::CONFIG_IBL_SIZE));

    // this should probably be a code generation option
    cg.generateDefine(fs, "USE_MULTIPLE_SCATTERING_COMPENSATION", true);

    cg.generateDefine(fs, "GEOMETRIC_SPECULAR_AA_ROUGHNESS", material.curvatureToRoughness);
    cg.generateDefine(fs, "GEOMETRIC_SPECULAR_AA_NORMAL", material.limitOverInterpolation);

    cg.generateDefine(fs, "CLEAR_COAT_IOR_CHANGE", material.clearCoatIorChange);

    // lighting variants
    bool litVariants = lit || material.hasShadowMultiplier;
    cg.generateDefine(fs, "HAS_DIRECTIONAL_LIGHTING", litVariants && variant.hasDirectionalLighting());
    cg.generateDefine(fs, "HAS_DYNAMIC_LIGHTING", litVariants && variant.hasDynamicLighting());
    cg.generateDefine(fs, "HAS_SHADOWING", litVariants && variant.hasShadowReceiver());
    cg.generateDefine(fs, "HAS_SHADOW_MULTIPLIER", material.hasShadowMultiplier);

    // material defines
    cg.generateDefine(fs, "MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY", material.hasDoubleSidedCapability);
    switch (material.blendingMode) {
        case BlendingMode::OPAQUE:
            cg.generateDefine(fs, "BLEND_MODE_OPAQUE", true);
            break;
        case BlendingMode::TRANSPARENT:
            cg.generateDefine(fs, "BLEND_MODE_TRANSPARENT", true);
            break;
        case BlendingMode::FADE:
            // Fade is a special case of transparent
            cg.generateDefine(fs, "BLEND_MODE_TRANSPARENT", true);
            cg.generateDefine(fs, "BLEND_MODE_FADE", true);
            break;
        case BlendingMode::ADD:
            cg.generateDefine(fs, "BLEND_MODE_ADD", true);
            break;
        case BlendingMode::MASKED:
            cg.generateDefine(fs, "BLEND_MODE_MASKED", true);
            break;
    }
    switch (material.postLightingBlendingMode) {
        case BlendingMode::OPAQUE:
            cg.generateDefine(fs, "POST_LIGHTING_BLEND_MODE_OPAQUE", true);
            break;
        case BlendingMode::TRANSPARENT:
            cg.generateDefine(fs, "POST_LIGHTING_BLEND_MODE_TRANSPARENT", true);
            break;
        case BlendingMode::ADD:
            cg.generateDefine(fs, "POST_LIGHTING_BLEND_MODE_ADD", true);
            break;
        default:
            break;
    }
    cg.generateDefine(fs, getShadingDefine(material.shading), true);
    generateMaterialDefines(fs, cg, mProperties);

    cg.generateShaderInputs(fs, ShaderType::FRAGMENT, material.requiredAttributes, interpolation);

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        cg.generateVariable(fs, ShaderType::FRAGMENT, variable, variableIndex++);
    }

    // uniforms and samplers
    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::LIGHTS, UibGenerator::getLightsUib());
    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::PER_MATERIAL_INSTANCE, material.uib);
    cg.generateSeparator(fs);
    cg.generateSamplers(fs,
            material.samplerBindings.getBlockOffset(BindingPoints::PER_VIEW),
            SibGenerator::getPerViewSib());
    cg.generateSamplers(fs,
            material.samplerBindings.getBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    // shading code
    cg.generateCommon(fs, ShaderType::FRAGMENT);
    cg.generateGetters(fs, ShaderType::FRAGMENT);
    cg.generateCommonMaterial(fs, ShaderType::FRAGMENT);
    cg.generateParameters(fs, ShaderType::FRAGMENT);

    // shading model
    if (variant.isDepthPass()) {
        if (material.blendingMode == BlendingMode::MASKED) {
            appendShader(fs, mMaterialCode, mMaterialLineOffset);
        }
        // these variants are special and are treated as DEPTH variants. Filament will never
        // request that variant for the color pass.
        cg.generateDepthShaderMain(fs, ShaderType::FRAGMENT);
    } else {
        appendShader(fs, mMaterialCode, mMaterialLineOffset);
        if (material.isLit) {
            cg.generateShaderLit(fs, ShaderType::FRAGMENT, variant, material.shading);
        } else {
            cg.generateShaderUnlit(fs, ShaderType::FRAGMENT, variant, material.hasShadowMultiplier);
        }
        // entry point
        cg.generateShaderMain(fs, ShaderType::FRAGMENT);
    }

    cg.generateEpilog(fs);

    return fs.str();
}

void ShaderGenerator::fixupExternalSamplers(filament::backend::ShaderModel sm,
        std::string& shader, MaterialInfo const& material) const noexcept {
    // External samplers are only supported on GL ES at the moment, we must
    // skip the fixup on desktop targets
    if (material.hasExternalSamplers && sm == ShaderModel::GL_ES_30) {
        CodeGenerator::fixupExternalSamplers(shader, material.sib);
    }
}

const std::string ShaderPostProcessGenerator::createPostProcessVertexProgram(
        filament::backend::ShaderModel sm, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage, filament::PostProcessStage variant,
        uint8_t firstSampler) noexcept {
    const CodeGenerator cg(sm, targetApi, targetLanguage);
    std::stringstream vs;
    cg.generateProlog(vs, ShaderType::VERTEX, false);
    cg.generateDefine(vs, "LOCATION_POSITION", uint32_t(VertexAttribute::POSITION));
    generatePostProcessStageDefines(vs, cg, variant);

    cg.generateUniforms(vs, ShaderType::VERTEX,
            BindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(vs, ShaderType::VERTEX,
            BindingPoints::POST_PROCESS, UibGenerator::getPostProcessingUib());
    cg.generateSamplers(vs,
            firstSampler, SibGenerator::getPostProcessSib());

    cg.generateCommon(vs, ShaderType::VERTEX);
    cg.generatePostProcessMain(vs, ShaderType::VERTEX, variant);
    cg.generateEpilog(vs);
    return vs.str();
}

const std::string ShaderPostProcessGenerator::createPostProcessFragmentProgram(
        filament::backend::ShaderModel sm, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage, filament::PostProcessStage variant,
        uint8_t firstSampler) noexcept {
    const CodeGenerator cg(sm, targetApi, targetLanguage);
    std::stringstream fs;
    cg.generateProlog(fs, ShaderType::FRAGMENT, false);
    generatePostProcessStageDefines(fs, cg, variant);

    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::POST_PROCESS, UibGenerator::getPostProcessingUib());
    cg.generateSamplers(fs,
            firstSampler, SibGenerator::getPostProcessSib());

    cg.generateCommon(fs, ShaderType::FRAGMENT);
    cg.generatePostProcessMain(fs, ShaderType::FRAGMENT, variant);
    cg.generateEpilog(fs);
    return fs.str();
}

void ShaderPostProcessGenerator::generatePostProcessStageDefines(std::stringstream& vs,
        CodeGenerator const& cg, PostProcessStage variant) noexcept {
    cg.generateDefine(vs, "POST_PROCESS_TONE_MAPPING_OPAQUE",
            uint32_t(PostProcessStage::TONE_MAPPING_OPAQUE));
    cg.generateDefine(vs, "POST_PROCESS_TONE_MAPPING_TRANSLUCENT",
            uint32_t(PostProcessStage::TONE_MAPPING_TRANSLUCENT));
    cg.generateDefine(vs, "POST_PROCESS_ANTI_ALIASING_OPAQUE",
            uint32_t(PostProcessStage::ANTI_ALIASING_OPAQUE));
    cg.generateDefine(vs, "POST_PROCESS_ANTI_ALIASING_TRANSLUCENT",
            uint32_t(PostProcessStage::ANTI_ALIASING_TRANSLUCENT));
    switch (variant) {
        case PostProcessStage::TONE_MAPPING_OPAQUE:
            cg.generateDefine(vs, "POST_PROCESS_STAGE", "POST_PROCESS_TONE_MAPPING_OPAQUE");
            cg.generateDefine(vs, "POST_PROCESS_TONE_MAPPING",  1u);
            cg.generateDefine(vs, "POST_PROCESS_ANTI_ALIASING", 0u);
            cg.generateDefine(vs, "POST_PROCESS_OPAQUE",        1u);
            break;
        case PostProcessStage::TONE_MAPPING_TRANSLUCENT:
            cg.generateDefine(vs, "POST_PROCESS_STAGE", "POST_PROCESS_TONE_MAPPING_TRANSLUCENT");
            cg.generateDefine(vs, "POST_PROCESS_TONE_MAPPING",  1u);
            cg.generateDefine(vs, "POST_PROCESS_ANTI_ALIASING", 0u);
            cg.generateDefine(vs, "POST_PROCESS_OPAQUE",        0u);
            break;
        case PostProcessStage::ANTI_ALIASING_OPAQUE:
            cg.generateDefine(vs, "POST_PROCESS_STAGE", "POST_PROCESS_ANTI_ALIASING_OPAQUE");
            cg.generateDefine(vs, "POST_PROCESS_TONE_MAPPING",  0u);
            cg.generateDefine(vs, "POST_PROCESS_ANTI_ALIASING", 1u);
            cg.generateDefine(vs, "POST_PROCESS_OPAQUE",        1u);
            break;
        case PostProcessStage::ANTI_ALIASING_TRANSLUCENT:
            cg.generateDefine(vs, "POST_PROCESS_STAGE", "POST_PROCESS_ANTI_ALIASING_TRANSLUCENT");
            cg.generateDefine(vs, "POST_PROCESS_TONE_MAPPING",  0u);
            cg.generateDefine(vs, "POST_PROCESS_ANTI_ALIASING", 1u);
            cg.generateDefine(vs, "POST_PROCESS_OPAQUE",        0u);
            break;
    }
}


} // namespace filament
