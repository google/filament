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

#include <filament/MaterialEnums.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/SibGenerator.h>
#include <private/filament/Variant.h>

#include <utils/CString.h>

#include "filamat/MaterialBuilder.h"
#include "CodeGenerator.h"
#include "../UibGenerator.h"

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

static void generateMaterialDefines(utils::io::sstream& os, const CodeGenerator& cg,
        MaterialBuilder::PropertyList const properties,
        const MaterialBuilder::PreprocessorDefineList& defines) noexcept {
    for (size_t i = 0; i < MaterialBuilder::MATERIAL_PROPERTIES_COUNT; i++) {
        cg.generateMaterialProperty(os, static_cast<MaterialBuilder::Property>(i), properties[i]);
    }
    // synthetic defines
    bool hasTBN =
            properties[static_cast<int>(MaterialBuilder::Property::ANISOTROPY)]         ||
            properties[static_cast<int>(MaterialBuilder::Property::NORMAL)]             ||
            properties[static_cast<int>(MaterialBuilder::Property::BENT_NORMAL)]        ||
            properties[static_cast<int>(MaterialBuilder::Property::CLEAR_COAT_NORMAL)];
    cg.generateDefine(os, "MATERIAL_NEEDS_TBN", hasTBN);

    // Additional, user-provided defines.
    for (const auto& define : defines) {
        cg.generateDefine(os, define.name.c_str(), define.value.c_str());
    }
}

static void generateVertexDomain(const CodeGenerator& cg, utils::io::sstream& vs,
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

static void generatePostProcessMaterialVariantDefines(const CodeGenerator& cg,
        utils::io::sstream& shader, PostProcessVariant variant) noexcept {
    switch (variant) {
        case PostProcessVariant::OPAQUE:
            cg.generateDefine(shader, "POST_PROCESS_OPAQUE", 1u);
            break;
        case PostProcessVariant::TRANSLUCENT:
            cg.generateDefine(shader, "POST_PROCESS_OPAQUE", 0u);
            break;
    }
}

static size_t countLines(const char* s) noexcept {
    size_t lines = 0;
    size_t i = 0;
    while (s[i] != 0) {
        if (s[i++] == '\n') lines++;
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

static void appendShader(utils::io::sstream& ss,
        const utils::CString& shader, size_t lineOffset) noexcept {
    if (!shader.empty()) {
        size_t lines = countLines(ss.c_str());
        ss << "#line " << lineOffset + 1 << '\n';
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
        MaterialBuilder::OutputList const& outputs,
        MaterialBuilder::PreprocessorDefineList const& defines,
        utils::CString const& materialCode, size_t lineOffset,
        utils::CString const& materialVertexCode, size_t vertexLineOffset,
        MaterialBuilder::MaterialDomain materialDomain) noexcept {
    std::copy(std::begin(properties), std::end(properties), std::begin(mProperties));
    std::copy(std::begin(variables), std::end(variables), std::begin(mVariables));
    std::copy(std::begin(outputs), std::end(outputs), std::back_inserter(mOutputs));

    mMaterialCode = materialCode;
    mMaterialVertexCode = materialVertexCode;
    mMaterialLineOffset = lineOffset;
    mMaterialVertexLineOffset = vertexLineOffset;
    mMaterialDomain = materialDomain;
    mDefines = defines;

    if (mMaterialCode.empty()) {
        if (mMaterialDomain == MaterialBuilder::MaterialDomain::SURFACE) {
            mMaterialCode =
                    utils::CString("void material(inout MaterialInputs m) {\n    prepareMaterial(m);\n}\n");
        } else if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
            mMaterialCode =
                    utils::CString("void postProcess(inout PostProcessInputs p) {\n}\n");
        }
    }
    if (mMaterialVertexCode.empty()) {
        if (mMaterialDomain == MaterialBuilder::MaterialDomain::SURFACE) {
            mMaterialVertexCode =
                    utils::CString("void materialVertex(inout MaterialVertexInputs m) {\n}\n");
        } else if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
            mMaterialVertexCode =
                    utils::CString("void postProcessVertex(inout PostProcessVertexInputs m) {\n}\n");
        }
    }
}

std::string ShaderGenerator::createVertexProgram(filament::backend::ShaderModel shaderModel,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& material, uint8_t variantKey, filament::Interpolation interpolation,
        filament::VertexDomain vertexDomain) const noexcept {
    if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return createPostProcessVertexProgram(shaderModel, targetApi,
                targetLanguage, material, variantKey, material.samplerBindings);
    }

    utils::io::sstream vs;

    const CodeGenerator cg(shaderModel, targetApi, targetLanguage);
    const bool lit = material.isLit;
    const filament::Variant variant(variantKey);

    cg.generateProlog(vs, ShaderType::VERTEX, material.hasExternalSamplers);

    cg.generateQualityDefine(vs, material.quality);

    cg.generateDefine(vs, "MAX_SHADOW_CASTING_SPOTS", uint32_t(CONFIG_MAX_SHADOW_CASTING_SPOTS));

    cg.generateDefine(vs, "FLIP_UV_ATTRIBUTE", material.flipUV);

    bool litVariants = lit || material.hasShadowMultiplier;
    cg.generateDefine(vs, "HAS_DIRECTIONAL_LIGHTING", litVariants && variant.hasDirectionalLighting());
    cg.generateDefine(vs, "HAS_DYNAMIC_LIGHTING", litVariants && variant.hasDynamicLighting());
    cg.generateDefine(vs, "HAS_SHADOWING", litVariants && variant.hasShadowReceiver());
    cg.generateDefine(vs, "HAS_SHADOW_MULTIPLIER", material.hasShadowMultiplier);
    cg.generateDefine(vs, "HAS_SKINNING_OR_MORPHING", variant.hasSkinningOrMorphing());
    cg.generateDefine(vs, "HAS_VSM", variant.hasVsm());
    cg.generateDefine(vs, getShadingDefine(material.shading), true);
    generateMaterialDefines(vs, cg, mProperties, mDefines);

    AttributeBitset attributes = material.requiredAttributes;
    if (variant.hasSkinningOrMorphing()) {
        attributes.set(VertexAttribute::BONE_INDICES);
        attributes.set(VertexAttribute::BONE_WEIGHTS);
        attributes.set(VertexAttribute::MORPH_POSITION_0);
        attributes.set(VertexAttribute::MORPH_POSITION_1);
        attributes.set(VertexAttribute::MORPH_POSITION_2);
        attributes.set(VertexAttribute::MORPH_POSITION_3);
        attributes.set(VertexAttribute::MORPH_TANGENTS_0);
        attributes.set(VertexAttribute::MORPH_TANGENTS_1);
        attributes.set(VertexAttribute::MORPH_TANGENTS_2);
        attributes.set(VertexAttribute::MORPH_TANGENTS_3);
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
    if (litVariants && variant.hasShadowReceiver()) {
        cg.generateUniforms(vs, ShaderType::VERTEX,
                BindingPoints::SHADOW, UibGenerator::getShadowUib());
    }
    if (variant.hasSkinningOrMorphing()) {
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

    return vs.c_str();
}

bool ShaderGenerator::hasCustomDepthShader() const noexcept {
    for (const auto& variable : mVariables) {
        if (!variable.empty()) {
            return true;
        }
    }
    return false;
}

static bool isMobileTarget(filament::backend::ShaderModel model) {
    switch (model) {
        case ShaderModel::UNKNOWN:
            return false;
        case ShaderModel::GL_ES_30:
            return true;
        case ShaderModel::GL_CORE_41:
            return false;
    }
}

std::string ShaderGenerator::createFragmentProgram(filament::backend::ShaderModel shaderModel,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& material, uint8_t variantKey,
        filament::Interpolation interpolation) const noexcept {
    if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return createPostProcessFragmentProgram(shaderModel, targetApi, targetLanguage, material,
                variantKey, material.samplerBindings);
    }

    const CodeGenerator cg(shaderModel, targetApi, targetLanguage);
    const bool lit = material.isLit;
    const filament::Variant variant(variantKey);

    utils::io::sstream fs;
    cg.generateProlog(fs, ShaderType::FRAGMENT, material.hasExternalSamplers);

    cg.generateQualityDefine(fs, material.quality);

    cg.generateDefine(fs, "GEOMETRIC_SPECULAR_AA", material.specularAntiAliasing && lit);

    cg.generateDefine(fs, "CLEAR_COAT_IOR_CHANGE", material.clearCoatIorChange);

    cg.generateDefine(fs, "MAX_SHADOW_CASTING_SPOTS", uint32_t(CONFIG_MAX_SHADOW_CASTING_SPOTS));

    auto defaultSpecularAO = isMobileTarget(shaderModel) ?
            SpecularAmbientOcclusion::NONE : SpecularAmbientOcclusion::SIMPLE;
    auto specularAO = material.specularAOSet ? material.specularAO : defaultSpecularAO;
    cg.generateDefine(fs, "SPECULAR_AMBIENT_OCCLUSION", uint32_t(specularAO));

    cg.generateDefine(fs, "HAS_REFRACTION", material.refractionMode != RefractionMode::NONE);
    if (material.refractionMode != RefractionMode::NONE) {
        cg.generateDefine(fs, "REFRACTION_MODE_CUBEMAP", uint32_t(RefractionMode::CUBEMAP));
        cg.generateDefine(fs, "REFRACTION_MODE_SCREEN_SPACE", uint32_t(RefractionMode::SCREEN_SPACE));
        switch (material.refractionMode) {
            case RefractionMode::NONE:
                // can't be here
                break;
            case RefractionMode::CUBEMAP:
                cg.generateDefine(fs, "REFRACTION_MODE", "REFRACTION_MODE_CUBEMAP");
                break;
            case RefractionMode::SCREEN_SPACE:
                cg.generateDefine(fs, "REFRACTION_MODE", "REFRACTION_MODE_SCREEN_SPACE");
                break;
        }
        cg.generateDefine(fs, "REFRACTION_TYPE_SOLID", uint32_t(RefractionType::SOLID));
        cg.generateDefine(fs, "REFRACTION_TYPE_THIN", uint32_t(RefractionType::THIN));
        switch (material.refractionType) {
            case RefractionType::SOLID:
                cg.generateDefine(fs, "REFRACTION_TYPE", "REFRACTION_TYPE_SOLID");
                break;
            case RefractionType::THIN:
                cg.generateDefine(fs, "REFRACTION_TYPE", "REFRACTION_TYPE_THIN");
                break;
        }
    }

    bool multiBounceAO = material.multiBounceAOSet ?
            material.multiBounceAO : !isMobileTarget(shaderModel);
    cg.generateDefine(fs, "MULTI_BOUNCE_AMBIENT_OCCLUSION", multiBounceAO ? 1u : 0u);

    // lighting variants
    bool litVariants = lit || material.hasShadowMultiplier;
    cg.generateDefine(fs, "HAS_DIRECTIONAL_LIGHTING", litVariants && variant.hasDirectionalLighting());
    cg.generateDefine(fs, "HAS_DYNAMIC_LIGHTING", litVariants && variant.hasDynamicLighting());
    cg.generateDefine(fs, "HAS_SHADOWING", litVariants && variant.hasShadowReceiver());
    cg.generateDefine(fs, "HAS_SHADOW_MULTIPLIER", material.hasShadowMultiplier);
    cg.generateDefine(fs, "HAS_FOG", variant.hasFog());
    cg.generateDefine(fs, "HAS_VSM", variant.hasVsm());

    // material defines
    cg.generateDefine(fs, "MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY", material.hasDoubleSidedCapability);
    switch (material.blendingMode) {
        case BlendingMode::OPAQUE:
            cg.generateDefine(fs, "BLEND_MODE_OPAQUE", true);
            break;
        case BlendingMode::TRANSPARENT:
            cg.generateDefine(fs, "BLEND_MODE_TRANSPARENT", true);
            break;
        case BlendingMode::ADD:
            cg.generateDefine(fs, "BLEND_MODE_ADD", true);
            break;
        case BlendingMode::MASKED:
            cg.generateDefine(fs, "BLEND_MODE_MASKED", true);
            break;
        case BlendingMode::FADE:
            // Fade is a special case of transparent
            cg.generateDefine(fs, "BLEND_MODE_TRANSPARENT", true);
            cg.generateDefine(fs, "BLEND_MODE_FADE", true);
            break;
        case BlendingMode::MULTIPLY:
            cg.generateDefine(fs, "BLEND_MODE_MULTIPLY", true);
            break;
        case BlendingMode::SCREEN:
            cg.generateDefine(fs, "BLEND_MODE_SCREEN", true);
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
        case BlendingMode::MULTIPLY:
            cg.generateDefine(fs, "POST_LIGHTING_BLEND_MODE_MULTIPLY", true);
            break;
        case BlendingMode::SCREEN:
            cg.generateDefine(fs, "POST_LIGHTING_BLEND_MODE_SCREEN", true);
            break;
        default:
            break;
    }
    cg.generateDefine(fs, getShadingDefine(material.shading), true);
    generateMaterialDefines(fs, cg, mProperties, mDefines);

    cg.generateDefine(fs, "MATERIAL_HAS_CUSTOM_SURFACE_SHADING", material.hasCustomSurfaceShading);

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
            BindingPoints::PER_RENDERABLE, UibGenerator::getPerRenderableUib());
    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::LIGHTS, UibGenerator::getLightsUib());
    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::FROXEL_RECORDS, UibGenerator::getFroxelRecordUib());
    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::PER_MATERIAL_INSTANCE, material.uib);
    cg.generateSeparator(fs);
    cg.generateSamplers(fs,
            material.samplerBindings.getBlockOffset(BindingPoints::PER_VIEW),
            SibGenerator::getPerViewSib(variantKey));
    cg.generateSamplers(fs,
            material.samplerBindings.getBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    // shading code
    cg.generateCommon(fs, ShaderType::FRAGMENT);
    cg.generateGetters(fs, ShaderType::FRAGMENT);
    cg.generateCommonMaterial(fs, ShaderType::FRAGMENT);
    cg.generateParameters(fs, ShaderType::FRAGMENT);
    cg.generateFog(fs, ShaderType::FRAGMENT);

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
            cg.generateShaderLit(fs, ShaderType::FRAGMENT, variant, material.shading,
                    material.hasCustomSurfaceShading);
        } else {
            cg.generateShaderUnlit(fs, ShaderType::FRAGMENT, variant, material.hasShadowMultiplier);
        }
        // entry point
        cg.generateShaderMain(fs, ShaderType::FRAGMENT);
    }

    cg.generateEpilog(fs);

    return fs.c_str();
}

void ShaderGenerator::fixupExternalSamplers(filament::backend::ShaderModel sm,
        std::string& shader, MaterialInfo const& material) const noexcept {
    // External samplers are only supported on GL ES at the moment, we must
    // skip the fixup on desktop targets
    if (material.hasExternalSamplers && sm == ShaderModel::GL_ES_30) {
        CodeGenerator::fixupExternalSamplers(shader, material.sib);
    }
}

std::string ShaderGenerator::createPostProcessVertexProgram(
        filament::backend::ShaderModel sm, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage, MaterialInfo const& material,
        uint8_t variant, const filament::SamplerBindingMap& samplerBindingMap) const noexcept {
    const CodeGenerator cg(sm, targetApi, targetLanguage);
    utils::io::sstream vs;
    cg.generateProlog(vs, ShaderType::VERTEX, false);

    cg.generateQualityDefine(vs, material.quality);

    cg.generateDefine(vs, "LOCATION_POSITION", uint32_t(VertexAttribute::POSITION));

    // The UVs are at the location immediately following the custom variables.
    cg.generateDefine(vs, "LOCATION_UVS", uint32_t(MaterialBuilder::MATERIAL_VARIABLES_COUNT));

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        cg.generateVariable(vs, ShaderType::VERTEX, variable, variableIndex++);
    }

    cg.generatePostProcessInputs(vs, ShaderType::VERTEX);
    generatePostProcessMaterialVariantDefines(cg, vs, PostProcessVariant(variant));

    cg.generateUniforms(vs, ShaderType::VERTEX,
            BindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(vs, ShaderType::VERTEX,
            BindingPoints::PER_MATERIAL_INSTANCE, material.uib);

    cg.generateSamplers(vs,
            material.samplerBindings.getBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    cg.generateCommon(vs, ShaderType::VERTEX);
    cg.generatePostProcessGetters(vs, ShaderType::VERTEX);

    appendShader(vs, mMaterialVertexCode, mMaterialVertexLineOffset);

    cg.generatePostProcessMain(vs, ShaderType::VERTEX);

    cg.generateEpilog(vs);
    return vs.c_str();
}

std::string ShaderGenerator::createPostProcessFragmentProgram(
        filament::backend::ShaderModel sm, MaterialBuilder::TargetApi targetApi,
        MaterialBuilder::TargetLanguage targetLanguage, MaterialInfo const& material,
        uint8_t variant, const filament::SamplerBindingMap& samplerBindingMap) const noexcept {
    const CodeGenerator cg(sm, targetApi, targetLanguage);
    utils::io::sstream fs;
    cg.generateProlog(fs, ShaderType::FRAGMENT, false);

    cg.generateQualityDefine(fs, material.quality);

    // The UVs are at the location immediately following the custom variables.
    cg.generateDefine(fs, "LOCATION_UVS", uint32_t(MaterialBuilder::MATERIAL_VARIABLES_COUNT));

    generatePostProcessMaterialVariantDefines(cg, fs, PostProcessVariant(variant));

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        cg.generateVariable(fs, ShaderType::FRAGMENT, variable, variableIndex++);
    }

    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(fs, ShaderType::FRAGMENT,
            BindingPoints::PER_MATERIAL_INSTANCE, material.uib);

    cg.generateSamplers(fs,
            material.samplerBindings.getBlockOffset(BindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    // subpass
    cg.generateSubpass(fs, material.subpass);

    cg.generateCommon(fs, ShaderType::FRAGMENT);
    cg.generatePostProcessGetters(fs, ShaderType::FRAGMENT);

    // Generate post-process outputs.
    for (const auto& output : mOutputs) {
        if (output.target == MaterialBuilder::OutputTarget::COLOR) {
            cg.generateOutput(fs, ShaderType::FRAGMENT, output.name, output.location,
                    output.qualifier, output.type);
        }
        if (output.target == MaterialBuilder::OutputTarget::DEPTH) {
            cg.generateDefine(fs, "FRAG_OUTPUT_DEPTH", 1u);
        }
    }

    cg.generatePostProcessInputs(fs, ShaderType::FRAGMENT);

    appendShader(fs, mMaterialCode, mMaterialLineOffset);

    cg.generatePostProcessMain(fs, ShaderType::FRAGMENT);
    cg.generateEpilog(fs);
    return fs.c_str();
}


} // namespace filament
