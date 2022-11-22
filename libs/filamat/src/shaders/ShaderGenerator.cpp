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
#include <private/filament/Variant.h>

#include <utils/CString.h>

#include "filamat/MaterialBuilder.h"
#include "CodeGenerator.h"
#include "../SibGenerator.h"
#include "../UibGenerator.h"

#include <iterator>

namespace filamat {

using namespace filament;
using namespace filament::backend;
using namespace utils;

static const char* getShadingDefine(Shading shading) noexcept {
    switch (shading) {
        case Shading::LIT:                 return "SHADING_MODEL_LIT";
        case Shading::UNLIT:               return "SHADING_MODEL_UNLIT";
        case Shading::SUBSURFACE:          return "SHADING_MODEL_SUBSURFACE";
        case Shading::CLOTH:               return "SHADING_MODEL_CLOTH";
        case Shading::SPECULAR_GLOSSINESS: return "SHADING_MODEL_SPECULAR_GLOSSINESS";
    }
}

static void generateMaterialDefines(io::sstream& os, MaterialBuilder::PropertyList const properties,
        const MaterialBuilder::PreprocessorDefineList& defines) noexcept {
    for (size_t i = 0; i < MaterialBuilder::MATERIAL_PROPERTIES_COUNT; i++) {
        CodeGenerator::generateMaterialProperty(os, static_cast<MaterialBuilder::Property>(i), properties[i]);
    }
    // synthetic defines
    bool hasTBN =
            properties[static_cast<int>(MaterialBuilder::Property::ANISOTROPY)]         ||
            properties[static_cast<int>(MaterialBuilder::Property::NORMAL)]             ||
            properties[static_cast<int>(MaterialBuilder::Property::BENT_NORMAL)]        ||
            properties[static_cast<int>(MaterialBuilder::Property::CLEAR_COAT_NORMAL)];
    CodeGenerator::generateDefine(os, "MATERIAL_NEEDS_TBN", hasTBN);

    // Additional, user-provided defines.
    for (const auto& define : defines) {
        CodeGenerator::generateDefine(os, define.name.c_str(), define.value.c_str());
    }
}

static void generateVertexDomain(io::sstream& vs, VertexDomain domain) noexcept {
    switch (domain) {
        case VertexDomain::OBJECT:
            CodeGenerator::generateDefine(vs, "VERTEX_DOMAIN_OBJECT", true);
            break;
        case VertexDomain::WORLD:
            CodeGenerator::generateDefine(vs, "VERTEX_DOMAIN_WORLD", true);
            break;
        case VertexDomain::VIEW:
            CodeGenerator::generateDefine(vs, "VERTEX_DOMAIN_VIEW", true);
            break;
        case VertexDomain::DEVICE:
            CodeGenerator::generateDefine(vs, "VERTEX_DOMAIN_DEVICE", true);
            break;
    }
}

static void generatePostProcessMaterialVariantDefines(io::sstream& shader,
        PostProcessVariant variant) noexcept {
    switch (variant) {
        case PostProcessVariant::OPAQUE:
            CodeGenerator::generateDefine(shader, "POST_PROCESS_OPAQUE", 1u);
            break;
        case PostProcessVariant::TRANSLUCENT:
            CodeGenerator::generateDefine(shader, "POST_PROCESS_OPAQUE", 0u);
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

static size_t countLines(const CString& s) noexcept {
    size_t lines = 0;
    for (char i : s) {
        if (i == '\n') lines++;
    }
    return lines;
}

static void appendShader(io::sstream& ss,
        const CString& shader, size_t lineOffset) noexcept {
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
        CString const& materialCode, size_t lineOffset,
        CString const& materialVertexCode, size_t vertexLineOffset,
        MaterialBuilder::MaterialDomain materialDomain) noexcept {

    if (materialDomain == MaterialBuilder::MaterialDomain::COMPUTE) {
        // we shouldn't have a vertex shader in a compute material
        assert_invariant(materialVertexCode.empty());
    }

    std::copy(std::begin(properties), std::end(properties), std::begin(mProperties));
    std::copy(std::begin(variables), std::end(variables), std::begin(mVariables));
    std::copy(std::begin(outputs), std::end(outputs), std::back_inserter(mOutputs));

    mMaterialFragmentCode = materialCode;
    mMaterialVertexCode = materialVertexCode;
    mIsMaterialVertexShaderEmpty = materialVertexCode.empty();
    mMaterialLineOffset = lineOffset;
    mMaterialVertexLineOffset = vertexLineOffset;
    mMaterialDomain = materialDomain;
    mDefines = defines;

    if (mMaterialFragmentCode.empty()) {
        if (mMaterialDomain == MaterialBuilder::MaterialDomain::SURFACE) {
            mMaterialFragmentCode =
                    CString("void material(inout MaterialInputs m) {\n    prepareMaterial(m);\n}\n");
        } else if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
            mMaterialFragmentCode =
                    CString("void postProcess(inout PostProcessInputs p) {\n}\n");
        } else if (mMaterialDomain == MaterialBuilder::MaterialDomain::COMPUTE) {
            mMaterialFragmentCode =
                    CString("void compute() {\n}\n");
        }
    }
    if (mMaterialVertexCode.empty()) {
        if (mMaterialDomain == MaterialBuilder::MaterialDomain::SURFACE) {
            mMaterialVertexCode =
                    CString("void materialVertex(inout MaterialVertexInputs m) {\n}\n");
        } else if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
            mMaterialVertexCode =
                    CString("void postProcessVertex(inout PostProcessVertexInputs m) {\n}\n");
        }
    }
}

void ShaderGenerator::fixupExternalSamplers(ShaderModel sm,
        std::string& shader, MaterialInfo const& material) noexcept {
    // External samplers are only supported on GL ES at the moment, we must
    // skip the fixup on desktop targets
    if (material.hasExternalSamplers && sm == ShaderModel::MOBILE) {
        CodeGenerator::fixupExternalSamplers(shader, material.sib);
    }
}

std::string ShaderGenerator::createVertexProgram(ShaderModel shaderModel,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& material, const filament::Variant variant, Interpolation interpolation,
        VertexDomain vertexDomain) const noexcept {

    assert_invariant(mMaterialDomain != MaterialBuilder::MaterialDomain::COMPUTE);

    if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return createPostProcessVertexProgram(shaderModel, targetApi,
                targetLanguage, material, variant.key);
    }

    io::sstream vs;

    const CodeGenerator cg(shaderModel, targetApi, targetLanguage, material.featureLevel);
    const bool lit = material.isLit;

    cg.generateProlog(vs, ShaderStage::VERTEX, material);

    cg.generateQualityDefine(vs, material.quality);

    CodeGenerator::generateDefine(vs, "FLIP_UV_ATTRIBUTE", material.flipUV);

    CodeGenerator::generateDefine(vs, "LEGACY_MORPHING", material.useLegacyMorphing);

    const bool litVariants = lit || material.hasShadowMultiplier;

    assert_invariant(filament::Variant::isValid(variant));

    // note: even if the user vertex shader is empty, we can't use the "optimized" version if
    // we're in masked mode because fragment shader needs the color varyings
    const bool useOptimizedDepthVertexShader =
            // must be a depth variant
            filament::Variant::isValidDepthVariant(variant) &&
            // must have an empty vertex shader
            mIsMaterialVertexShaderEmpty &&
            // but must not be MASKED mode
            material.blendingMode != BlendingMode::MASKED &&
            // and must not have transparent shadows
            !(material.hasTransparentShadow  &&
                    (material.blendingMode == BlendingMode::TRANSPARENT ||
                     material.blendingMode == BlendingMode::FADE));

    CodeGenerator::generateDefine(vs, "USE_OPTIMIZED_DEPTH_VERTEX_SHADER",
            useOptimizedDepthVertexShader);

    // material defines
    CodeGenerator::generateDefine(vs, "MATERIAL_HAS_SHADOW_MULTIPLIER",
            material.hasShadowMultiplier);

    CodeGenerator::generateDefine(vs, "MATERIAL_HAS_INSTANCES", material.instanced);

    CodeGenerator::generateDefine(vs, "MATERIAL_HAS_VERTEX_DOMAIN_DEVICE_JITTERED",
            material.vertexDomainDeviceJittered);

    CodeGenerator::generateDefine(vs, "MATERIAL_HAS_TRANSPARENT_SHADOW",
            material.hasTransparentShadow);

    CodeGenerator::generateDefine(vs, "VARIANT_HAS_DIRECTIONAL_LIGHTING",
            litVariants && variant.hasDirectionalLighting());
    CodeGenerator::generateDefine(vs, "VARIANT_HAS_DYNAMIC_LIGHTING",
            litVariants && variant.hasDynamicLighting());
    CodeGenerator::generateDefine(vs, "VARIANT_HAS_SHADOWING",
            litVariants && filament::Variant::isShadowReceiverVariant(variant));
    CodeGenerator::generateDefine(vs, "VARIANT_HAS_SKINNING_OR_MORPHING",
            variant.hasSkinningOrMorphing());
    CodeGenerator::generateDefine(vs, "VARIANT_HAS_VSM",
            filament::Variant::isVSMVariant(variant));

    CodeGenerator::generateDefine(vs, getShadingDefine(material.shading), true);
    generateMaterialDefines(vs, mProperties, mDefines);

    AttributeBitset attributes = material.requiredAttributes;
    if (variant.hasSkinningOrMorphing()) {
        attributes.set(VertexAttribute::BONE_INDICES);
        attributes.set(VertexAttribute::BONE_WEIGHTS);
        if (material.useLegacyMorphing) {
            attributes.set(VertexAttribute::MORPH_POSITION_0);
            attributes.set(VertexAttribute::MORPH_POSITION_1);
            attributes.set(VertexAttribute::MORPH_POSITION_2);
            attributes.set(VertexAttribute::MORPH_POSITION_3);
            attributes.set(VertexAttribute::MORPH_TANGENTS_0);
            attributes.set(VertexAttribute::MORPH_TANGENTS_1);
            attributes.set(VertexAttribute::MORPH_TANGENTS_2);
            attributes.set(VertexAttribute::MORPH_TANGENTS_3);
        }
    }
    CodeGenerator::generateShaderInputs(vs, ShaderStage::VERTEX, attributes, interpolation);

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        CodeGenerator::generateVariable(vs, ShaderStage::VERTEX, variable, variableIndex++);
    }

    // materials defines
    generateVertexDomain(vs, vertexDomain);

    // uniforms
    cg.generateUniforms(vs, ShaderStage::VERTEX,
            UniformBindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(vs, ShaderStage::VERTEX,
            UniformBindingPoints::PER_RENDERABLE, UibGenerator::getPerRenderableUib());
    if (litVariants && filament::Variant::isShadowReceiverVariant(variant)) {
        cg.generateUniforms(vs, ShaderStage::FRAGMENT,
                UniformBindingPoints::SHADOW, UibGenerator::getShadowUib());
    }
    if (variant.hasSkinningOrMorphing()) {
        cg.generateUniforms(vs, ShaderStage::VERTEX,
                UniformBindingPoints::PER_RENDERABLE_BONES,
                UibGenerator::getPerRenderableBonesUib());
        cg.generateUniforms(vs, ShaderStage::VERTEX,
                UniformBindingPoints::PER_RENDERABLE_MORPHING,
                UibGenerator::getPerRenderableMorphingUib());
        cg.generateSamplers(vs, SamplerBindingPoints::PER_RENDERABLE_MORPHING,
                material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_RENDERABLE_MORPHING),
                SibGenerator::getPerRenderPrimitiveMorphingSib(variant));
    }
    cg.generateUniforms(vs, ShaderStage::VERTEX,
            UniformBindingPoints::PER_MATERIAL_INSTANCE, material.uib);
    CodeGenerator::generateSeparator(vs);
    // TODO: should we generate per-view SIB in the vertex shader?
    cg.generateSamplers(vs, SamplerBindingPoints::PER_MATERIAL_INSTANCE,
            material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    // shader code
    CodeGenerator::generateCommon(vs, ShaderStage::VERTEX);
    CodeGenerator::generateGetters(vs, ShaderStage::VERTEX);
    CodeGenerator::generateCommonMaterial(vs, ShaderStage::VERTEX);

    // main entry point
    appendShader(vs, mMaterialVertexCode, mMaterialVertexLineOffset);
    CodeGenerator::generateShaderMain(vs, ShaderStage::VERTEX);

    CodeGenerator::generateEpilog(vs);

    return vs.c_str();
}

std::string ShaderGenerator::createFragmentProgram(ShaderModel shaderModel,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& material, const filament::Variant variant,
        Interpolation interpolation) const noexcept {

    assert_invariant(mMaterialDomain != MaterialBuilder::MaterialDomain::COMPUTE);

    if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return createPostProcessFragmentProgram(shaderModel, targetApi, targetLanguage, material,
                variant.key);
    }

    const CodeGenerator cg(shaderModel, targetApi, targetLanguage, material.featureLevel);
    const bool lit = material.isLit;

    io::sstream fs;
    cg.generateProlog(fs, ShaderStage::FRAGMENT, material);

    cg.generateQualityDefine(fs, material.quality);

    CodeGenerator::generateDefine(fs, "GEOMETRIC_SPECULAR_AA", material.specularAntiAliasing && lit);

    CodeGenerator::generateDefine(fs, "CLEAR_COAT_IOR_CHANGE", material.clearCoatIorChange);

    auto defaultSpecularAO = shaderModel == ShaderModel::MOBILE ?
            SpecularAmbientOcclusion::NONE : SpecularAmbientOcclusion::SIMPLE;
    auto specularAO = material.specularAOSet ? material.specularAO : defaultSpecularAO;
    CodeGenerator::generateDefine(fs, "SPECULAR_AMBIENT_OCCLUSION", uint32_t(specularAO));

    // We only support both screen-space refractions and reflections at the same time.
    // And the MATERIAL_HAS_REFRACTION/MATERIAL_HAS_REFLECTIONS defines signify if refraction/reflections are supported by
    // the material.

    CodeGenerator::generateDefine(fs, "MATERIAL_HAS_REFRACTION", material.refractionMode != RefractionMode::NONE);
    if (material.refractionMode != RefractionMode::NONE) {
        CodeGenerator::generateDefine(fs, "REFRACTION_MODE_CUBEMAP", uint32_t(RefractionMode::CUBEMAP));
        CodeGenerator::generateDefine(fs, "REFRACTION_MODE_SCREEN_SPACE", uint32_t(RefractionMode::SCREEN_SPACE));
        switch (material.refractionMode) {
            case RefractionMode::NONE:
                // can't be here
                break;
            case RefractionMode::CUBEMAP:
                CodeGenerator::generateDefine(fs, "REFRACTION_MODE", "REFRACTION_MODE_CUBEMAP");
                break;
            case RefractionMode::SCREEN_SPACE:
                CodeGenerator::generateDefine(fs, "REFRACTION_MODE", "REFRACTION_MODE_SCREEN_SPACE");
                break;
        }
        CodeGenerator::generateDefine(fs, "REFRACTION_TYPE_SOLID", uint32_t(RefractionType::SOLID));
        CodeGenerator::generateDefine(fs, "REFRACTION_TYPE_THIN", uint32_t(RefractionType::THIN));
        switch (material.refractionType) {
            case RefractionType::SOLID:
                CodeGenerator::generateDefine(fs, "REFRACTION_TYPE", "REFRACTION_TYPE_SOLID");
                break;
            case RefractionType::THIN:
                CodeGenerator::generateDefine(fs, "REFRACTION_TYPE", "REFRACTION_TYPE_THIN");
                break;
        }
    }

    CodeGenerator::generateDefine(fs, "MATERIAL_HAS_REFLECTIONS",
            material.reflectionMode == ReflectionMode::SCREEN_SPACE);

    bool multiBounceAO = material.multiBounceAOSet ?
            material.multiBounceAO : shaderModel == ShaderModel::DESKTOP;
    CodeGenerator::generateDefine(fs, "MULTI_BOUNCE_AMBIENT_OCCLUSION", multiBounceAO ? 1u : 0u);

    assert_invariant(filament::Variant::isValid(variant));

    // lighting variants
    bool litVariants = lit || material.hasShadowMultiplier;

    // material defines
    CodeGenerator::generateDefine(fs, "MATERIAL_HAS_SHADOW_MULTIPLIER",
            material.hasShadowMultiplier);

    CodeGenerator::generateDefine(fs, "MATERIAL_HAS_INSTANCES", material.instanced);

    CodeGenerator::generateDefine(fs, "MATERIAL_HAS_VERTEX_DOMAIN_DEVICE_JITTERED",
            material.vertexDomainDeviceJittered);

    CodeGenerator::generateDefine(fs, "MATERIAL_HAS_TRANSPARENT_SHADOW",
            material.hasTransparentShadow);

    CodeGenerator::generateDefine(fs, "VARIANT_HAS_DIRECTIONAL_LIGHTING",
            litVariants && variant.hasDirectionalLighting());
    CodeGenerator::generateDefine(fs, "VARIANT_HAS_DYNAMIC_LIGHTING",
            litVariants && variant.hasDynamicLighting());
    CodeGenerator::generateDefine(fs, "VARIANT_HAS_SHADOWING",
            litVariants && filament::Variant::isShadowReceiverVariant(variant));
    CodeGenerator::generateDefine(fs, "VARIANT_HAS_FOG",
            filament::Variant::isFogVariant(variant));
    CodeGenerator::generateDefine(fs, "VARIANT_HAS_PICKING",
            filament::Variant::isPickingVariant(variant));
    CodeGenerator::generateDefine(fs, "VARIANT_HAS_VSM",
            filament::Variant::isVSMVariant(variant));
    CodeGenerator::generateDefine(fs, "VARIANT_HAS_SSR",
            filament::Variant::isSSRVariant(variant));

    CodeGenerator::generateDefine(fs, "MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY",
            material.hasDoubleSidedCapability);
    switch (material.blendingMode) {
        case BlendingMode::OPAQUE:
            CodeGenerator::generateDefine(fs, "BLEND_MODE_OPAQUE", true);
            break;
        case BlendingMode::TRANSPARENT:
            CodeGenerator::generateDefine(fs, "BLEND_MODE_TRANSPARENT", true);
            break;
        case BlendingMode::ADD:
            CodeGenerator::generateDefine(fs, "BLEND_MODE_ADD", true);
            break;
        case BlendingMode::MASKED:
            CodeGenerator::generateDefine(fs, "BLEND_MODE_MASKED", true);
            break;
        case BlendingMode::FADE:
            // Fade is a special case of transparent
            CodeGenerator::generateDefine(fs, "BLEND_MODE_TRANSPARENT", true);
            CodeGenerator::generateDefine(fs, "BLEND_MODE_FADE", true);
            break;
        case BlendingMode::MULTIPLY:
            CodeGenerator::generateDefine(fs, "BLEND_MODE_MULTIPLY", true);
            break;
        case BlendingMode::SCREEN:
            CodeGenerator::generateDefine(fs, "BLEND_MODE_SCREEN", true);
            break;
    }
    switch (material.postLightingBlendingMode) {
        case BlendingMode::OPAQUE:
            CodeGenerator::generateDefine(fs, "POST_LIGHTING_BLEND_MODE_OPAQUE", true);
            break;
        case BlendingMode::TRANSPARENT:
            CodeGenerator::generateDefine(fs, "POST_LIGHTING_BLEND_MODE_TRANSPARENT", true);
            break;
        case BlendingMode::ADD:
            CodeGenerator::generateDefine(fs, "POST_LIGHTING_BLEND_MODE_ADD", true);
            break;
        case BlendingMode::MULTIPLY:
            CodeGenerator::generateDefine(fs, "POST_LIGHTING_BLEND_MODE_MULTIPLY", true);
            break;
        case BlendingMode::SCREEN:
            CodeGenerator::generateDefine(fs, "POST_LIGHTING_BLEND_MODE_SCREEN", true);
            break;
        default:
            break;
    }
    CodeGenerator::generateDefine(fs, getShadingDefine(material.shading), true);
    generateMaterialDefines(fs, mProperties, mDefines);

    CodeGenerator::generateDefine(fs, "MATERIAL_HAS_CUSTOM_SURFACE_SHADING", material.hasCustomSurfaceShading);

    CodeGenerator::generateShaderInputs(fs, ShaderStage::FRAGMENT, material.requiredAttributes, interpolation);

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        CodeGenerator::generateVariable(fs, ShaderStage::FRAGMENT, variable, variableIndex++);
    }

    // uniforms and samplers
    cg.generateUniforms(fs, ShaderStage::FRAGMENT,
            UniformBindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(fs, ShaderStage::FRAGMENT,
            UniformBindingPoints::PER_RENDERABLE, UibGenerator::getPerRenderableUib());
    cg.generateUniforms(fs, ShaderStage::FRAGMENT,
            UniformBindingPoints::LIGHTS, UibGenerator::getLightsUib());
    if (litVariants && filament::Variant::isShadowReceiverVariant(variant)) {
        cg.generateUniforms(fs, ShaderStage::FRAGMENT,
                UniformBindingPoints::SHADOW, UibGenerator::getShadowUib());
    }
    cg.generateUniforms(fs, ShaderStage::FRAGMENT,
            UniformBindingPoints::FROXEL_RECORDS, UibGenerator::getFroxelRecordUib());
    cg.generateUniforms(fs, ShaderStage::FRAGMENT,
            UniformBindingPoints::PER_MATERIAL_INSTANCE, material.uib);
    CodeGenerator::generateSeparator(fs);
    cg.generateSamplers(fs, SamplerBindingPoints::PER_VIEW,
            material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_VIEW),
            SibGenerator::getPerViewSib(variant));
    cg.generateSamplers(fs, SamplerBindingPoints::PER_MATERIAL_INSTANCE,
            material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    fs << "float filament_lodBias;\n";

    // shading code
    CodeGenerator::generateCommon(fs, ShaderStage::FRAGMENT);
    CodeGenerator::generateGetters(fs, ShaderStage::FRAGMENT);
    CodeGenerator::generateCommonMaterial(fs, ShaderStage::FRAGMENT);
    CodeGenerator::generateParameters(fs, ShaderStage::FRAGMENT);
    CodeGenerator::generateFog(fs, ShaderStage::FRAGMENT);

    // shading model
    if (filament::Variant::isValidDepthVariant(variant)) {
        // In MASKED mode or with transparent shadows, we need the alpha channel computed by
        // the material (user code), so we append it here.
        if (material.blendingMode == BlendingMode::MASKED || material.hasTransparentShadow) {
            appendShader(fs, mMaterialFragmentCode, mMaterialLineOffset);
        }
        // these variants are special and are treated as DEPTH variants. Filament will never
        // request that variant for the color pass.
        CodeGenerator::generateDepthShaderMain(fs, ShaderStage::FRAGMENT);
    } else {
        appendShader(fs, mMaterialFragmentCode, mMaterialLineOffset);
        if (filament::Variant::isSSRVariant(variant)) {
            CodeGenerator::generateShaderReflections(fs, ShaderStage::FRAGMENT, variant);
        } else if (material.isLit) {
            CodeGenerator::generateShaderLit(fs, ShaderStage::FRAGMENT, variant,
                    material.shading,material.hasCustomSurfaceShading);
        } else {
            CodeGenerator::generateShaderUnlit(fs, ShaderStage::FRAGMENT, variant,
                    material.hasShadowMultiplier);
        }
        // entry point
        CodeGenerator::generateShaderMain(fs, ShaderStage::FRAGMENT);
    }

    CodeGenerator::generateEpilog(fs);

    return fs.c_str();
}

std::string ShaderGenerator::createComputeProgram(filament::backend::ShaderModel shaderModel,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& material, filament::Variant variant) const noexcept {
    assert_invariant(mMaterialDomain == MaterialBuilder::MaterialDomain::COMPUTE);
    assert_invariant(material.featureLevel >= FeatureLevel::FEATURE_LEVEL_2);
    const CodeGenerator cg(shaderModel, targetApi, targetLanguage, material.featureLevel);
    io::sstream s;

    cg.generateProlog(s, ShaderStage::COMPUTE, material);

    cg.generateQualityDefine(s, material.quality);

    cg.generateUniforms(s, ShaderStage::COMPUTE,
            UniformBindingPoints::PER_VIEW, UibGenerator::getPerViewUib());

    cg.generateUniforms(s, ShaderStage::COMPUTE,
            UniformBindingPoints::PER_MATERIAL_INSTANCE, material.uib);

    cg.generateSamplers(s, SamplerBindingPoints::PER_MATERIAL_INSTANCE,
            material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    // generate SSBO
    cg.generateBuffers(s, material.buffers);

    // TODO: generate images

    CodeGenerator::generateCommon(s, ShaderStage::COMPUTE);

    CodeGenerator::generateGetters(s, ShaderStage::COMPUTE);

    appendShader(s, mMaterialFragmentCode, mMaterialLineOffset);

    CodeGenerator::generateShaderMain(s, ShaderStage::COMPUTE);

    CodeGenerator::generateEpilog(s);
    return s.c_str();
}

std::string ShaderGenerator::createPostProcessVertexProgram(ShaderModel sm,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& material, const filament::Variant::type_t variantKey) const noexcept {
    const CodeGenerator cg(sm, targetApi, targetLanguage, material.featureLevel);
    io::sstream vs;
    cg.generateProlog(vs, ShaderStage::VERTEX, material);

    cg.generateQualityDefine(vs, material.quality);

    CodeGenerator::generateDefine(vs, "LOCATION_POSITION", uint32_t(VertexAttribute::POSITION));

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        CodeGenerator::generateVariable(vs, ShaderStage::VERTEX, variable, variableIndex++);
    }

    CodeGenerator::generatePostProcessInputs(vs, ShaderStage::VERTEX);
    generatePostProcessMaterialVariantDefines(vs, PostProcessVariant(variantKey));

    cg.generateUniforms(vs, ShaderStage::VERTEX,
            UniformBindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(vs, ShaderStage::VERTEX,
            UniformBindingPoints::PER_MATERIAL_INSTANCE, material.uib);

    cg.generateSamplers(vs, SamplerBindingPoints::PER_MATERIAL_INSTANCE,
            material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    CodeGenerator::generatePostProcessCommon(vs, ShaderStage::VERTEX);
    CodeGenerator::generatePostProcessGetters(vs, ShaderStage::VERTEX);

    appendShader(vs, mMaterialVertexCode, mMaterialVertexLineOffset);

    CodeGenerator::generatePostProcessMain(vs, ShaderStage::VERTEX);

    CodeGenerator::generateEpilog(vs);
    return vs.c_str();
}

std::string ShaderGenerator::createPostProcessFragmentProgram(ShaderModel sm,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialInfo const& material, uint8_t variant) const noexcept {
    const CodeGenerator cg(sm, targetApi, targetLanguage, material.featureLevel);
    io::sstream fs;
    cg.generateProlog(fs, ShaderStage::FRAGMENT, material);

    cg.generateQualityDefine(fs, material.quality);

    generatePostProcessMaterialVariantDefines(fs, PostProcessVariant(variant));

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        CodeGenerator::generateVariable(fs, ShaderStage::FRAGMENT, variable, variableIndex++);
    }

    cg.generateUniforms(fs, ShaderStage::FRAGMENT,
            UniformBindingPoints::PER_VIEW, UibGenerator::getPerViewUib());
    cg.generateUniforms(fs, ShaderStage::FRAGMENT,
            UniformBindingPoints::PER_MATERIAL_INSTANCE, material.uib);

    cg.generateSamplers(fs, SamplerBindingPoints::PER_MATERIAL_INSTANCE,
            material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    // subpass
    CodeGenerator::generateSubpass(fs, material.subpass);

    CodeGenerator::generatePostProcessCommon(fs, ShaderStage::FRAGMENT);
    CodeGenerator::generatePostProcessGetters(fs, ShaderStage::FRAGMENT);

    // Generate post-process outputs.
    for (const auto& output : mOutputs) {
        if (output.target == MaterialBuilder::OutputTarget::COLOR) {
            cg.generateOutput(fs, ShaderStage::FRAGMENT, output.name, output.location,
                    output.qualifier, output.type);
        }
        if (output.target == MaterialBuilder::OutputTarget::DEPTH) {
            CodeGenerator::generateDefine(fs, "FRAG_OUTPUT_DEPTH", 1u);
        }
    }

    CodeGenerator::generatePostProcessInputs(fs, ShaderStage::FRAGMENT);

    appendShader(fs, mMaterialFragmentCode, mMaterialLineOffset);

    CodeGenerator::generatePostProcessMain(fs, ShaderStage::FRAGMENT);
    CodeGenerator::generateEpilog(fs);
    return fs.c_str();
}

} // namespace filament
