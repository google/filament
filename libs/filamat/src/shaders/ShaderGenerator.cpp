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

#include "backend/DriverEnums.h"
#include "filamat/MaterialBuilder.h"
#include "CodeGenerator.h"
#include "SibGenerator.h"
#include "UibGenerator.h"

#include <iterator>

namespace filamat {

using namespace filament;
using namespace filament::backend;
using namespace utils;

void ShaderGenerator::generateSurfaceMaterialVariantDefines(utils::io::sstream& out,
        ShaderStage stage, MaterialBuilder::FeatureLevel featureLevel,
        MaterialInfo const& material, filament::Variant variant) noexcept {

    bool const litVariants = material.isLit || material.hasShadowMultiplier;

    CodeGenerator::generateDefine(out, "VARIANT_HAS_DIRECTIONAL_LIGHTING",
            litVariants && variant.hasDirectionalLighting());
    CodeGenerator::generateDefine(out, "VARIANT_HAS_DYNAMIC_LIGHTING",
            litVariants && variant.hasDynamicLighting());
    CodeGenerator::generateDefine(out, "VARIANT_HAS_SHADOWING",
            litVariants && filament::Variant::isShadowReceiverVariant(variant));
    CodeGenerator::generateDefine(out, "VARIANT_HAS_VSM",
            filament::Variant::isVSMVariant(variant));
    CodeGenerator::generateDefine(out, "VARIANT_HAS_STEREO",
            hasStereo(variant, featureLevel));

    switch (stage) {
        case ShaderStage::VERTEX:
        CodeGenerator::generateDefine(out, "VARIANT_HAS_SKINNING_OR_MORPHING",
                hasSkinningOrMorphing(variant, featureLevel));
            break;
        case ShaderStage::FRAGMENT:
            CodeGenerator::generateDefine(out, "VARIANT_HAS_FOG",
                    filament::Variant::isFogVariant(variant));
            CodeGenerator::generateDefine(out, "VARIANT_HAS_PICKING",
                    filament::Variant::isPickingVariant(variant));
            CodeGenerator::generateDefine(out, "VARIANT_HAS_SSR",
                    filament::Variant::isSSRVariant(variant));
            break;
        case ShaderStage::COMPUTE:
            break;
    }

    out << '\n';
    CodeGenerator::generateDefine(out, "MATERIAL_FEATURE_LEVEL", uint32_t(featureLevel));

    CodeGenerator::generateDefine(out, "MATERIAL_HAS_SHADOW_MULTIPLIER",
            material.hasShadowMultiplier);

    CodeGenerator::generateDefine(out, "MATERIAL_HAS_INSTANCES", material.instanced);

    CodeGenerator::generateDefine(out, "MATERIAL_HAS_VERTEX_DOMAIN_DEVICE_JITTERED",
            material.vertexDomainDeviceJittered);

    CodeGenerator::generateDefine(out, "MATERIAL_HAS_TRANSPARENT_SHADOW",
            material.hasTransparentShadow);

    if (stage == ShaderStage::FRAGMENT) {
        // We only support both screen-space refractions and reflections at the same time.
        // And the MATERIAL_HAS_REFRACTION/MATERIAL_HAS_REFLECTIONS defines signify if
        // refraction/reflections are supported by the material.

        CodeGenerator::generateDefine(out, "MATERIAL_HAS_REFRACTION",
                material.refractionMode != RefractionMode::NONE);
        if (material.refractionMode != RefractionMode::NONE) {
            CodeGenerator::generateDefine(out, "REFRACTION_MODE_CUBEMAP",
                    uint32_t(RefractionMode::CUBEMAP));
            CodeGenerator::generateDefine(out, "REFRACTION_MODE_SCREEN_SPACE",
                    uint32_t(RefractionMode::SCREEN_SPACE));
            switch (material.refractionMode) {
                case RefractionMode::NONE:
                    // can't be here
                    break;
                case RefractionMode::CUBEMAP:
                    CodeGenerator::generateDefine(out, "REFRACTION_MODE",
                            "REFRACTION_MODE_CUBEMAP");
                    break;
                case RefractionMode::SCREEN_SPACE:
                    CodeGenerator::generateDefine(out, "REFRACTION_MODE",
                            "REFRACTION_MODE_SCREEN_SPACE");
                    break;
            }
            CodeGenerator::generateDefine(out, "REFRACTION_TYPE_SOLID",
                    uint32_t(RefractionType::SOLID));
            CodeGenerator::generateDefine(out, "REFRACTION_TYPE_THIN",
                    uint32_t(RefractionType::THIN));
            switch (material.refractionType) {
                case RefractionType::SOLID:
                    CodeGenerator::generateDefine(out, "REFRACTION_TYPE", "REFRACTION_TYPE_SOLID");
                    break;
                case RefractionType::THIN:
                    CodeGenerator::generateDefine(out, "REFRACTION_TYPE", "REFRACTION_TYPE_THIN");
                    break;
            }
        }
        CodeGenerator::generateDefine(out, "MATERIAL_HAS_REFLECTIONS",
                material.reflectionMode == ReflectionMode::SCREEN_SPACE);

        CodeGenerator::generateDefine(out, "MATERIAL_HAS_DOUBLE_SIDED_CAPABILITY",
                material.hasDoubleSidedCapability);

        CodeGenerator::generateDefine(out, "MATERIAL_HAS_CUSTOM_SURFACE_SHADING",
                material.hasCustomSurfaceShading);

        out << '\n';
        switch (material.blendingMode) {
            case BlendingMode::OPAQUE:
                CodeGenerator::generateDefine(out, "BLEND_MODE_OPAQUE", true);
                break;
            case BlendingMode::TRANSPARENT:
                CodeGenerator::generateDefine(out, "BLEND_MODE_TRANSPARENT", true);
                break;
            case BlendingMode::ADD:
                CodeGenerator::generateDefine(out, "BLEND_MODE_ADD", true);
                break;
            case BlendingMode::MASKED:
                CodeGenerator::generateDefine(out, "BLEND_MODE_MASKED", true);
                break;
            case BlendingMode::FADE:
                // Fade is a special case of transparent
                CodeGenerator::generateDefine(out, "BLEND_MODE_TRANSPARENT", true);
                CodeGenerator::generateDefine(out, "BLEND_MODE_FADE", true);
                break;
            case BlendingMode::MULTIPLY:
                CodeGenerator::generateDefine(out, "BLEND_MODE_MULTIPLY", true);
                break;
            case BlendingMode::SCREEN:
                CodeGenerator::generateDefine(out, "BLEND_MODE_SCREEN", true);
                break;
            case BlendingMode::CUSTOM:
                CodeGenerator::generateDefine(out, "BLEND_MODE_CUSTOM", true);
                break;
        }

        switch (material.postLightingBlendingMode) {
            case BlendingMode::OPAQUE:
                CodeGenerator::generateDefine(out, "POST_LIGHTING_BLEND_MODE_OPAQUE", true);
                break;
            case BlendingMode::TRANSPARENT:
                CodeGenerator::generateDefine(out, "POST_LIGHTING_BLEND_MODE_TRANSPARENT", true);
                break;
            case BlendingMode::ADD:
                CodeGenerator::generateDefine(out, "POST_LIGHTING_BLEND_MODE_ADD", true);
                break;
            case BlendingMode::MULTIPLY:
                CodeGenerator::generateDefine(out, "POST_LIGHTING_BLEND_MODE_MULTIPLY", true);
                break;
            case BlendingMode::SCREEN:
                CodeGenerator::generateDefine(out, "POST_LIGHTING_BLEND_MODE_SCREEN", true);
                break;
            case BlendingMode::CUSTOM:
                CodeGenerator::generateDefine(out, "POST_LIGHTING_BLEND_MODE_CUSTOM", true);
                break;
            default:
                break;
        }

        out << '\n';
        CodeGenerator::generateDefine(out, "GEOMETRIC_SPECULAR_AA",
                material.specularAntiAliasing && material.isLit);

        CodeGenerator::generateDefine(out, "CLEAR_COAT_IOR_CHANGE",
                material.clearCoatIorChange);
    }
}

void ShaderGenerator::generateSurfaceMaterialVariantProperties(io::sstream& out,
        MaterialBuilder::PropertyList const properties,
        const MaterialBuilder::PreprocessorDefineList& defines) noexcept {

    UTILS_NOUNROLL
    for (size_t i = 0; i < MaterialBuilder::MATERIAL_PROPERTIES_COUNT; i++) {
        CodeGenerator::generateMaterialProperty(out,
                static_cast<MaterialBuilder::Property>(i), properties[i]);
    }

    // synthetic defines
    bool const hasTBN =
            properties[static_cast<int>(MaterialBuilder::Property::ANISOTROPY)]         ||
            properties[static_cast<int>(MaterialBuilder::Property::NORMAL)]             ||
            properties[static_cast<int>(MaterialBuilder::Property::BENT_NORMAL)]        ||
            properties[static_cast<int>(MaterialBuilder::Property::CLEAR_COAT_NORMAL)];

    CodeGenerator::generateDefine(out, "MATERIAL_NEEDS_TBN", hasTBN);

    // Additional, user-provided defines.
    for (const auto& define : defines) {
        CodeGenerator::generateDefine(out, define.name.c_str(), define.value.c_str());
    }
}

void ShaderGenerator::generateVertexDomainDefines(io::sstream& vs, VertexDomain domain) noexcept {
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

void ShaderGenerator::generatePostProcessMaterialVariantDefines(io::sstream& out,
        PostProcessVariant variant) noexcept {
    switch (variant) {
        case PostProcessVariant::OPAQUE:
            CodeGenerator::generateDefine(out, "POST_PROCESS_OPAQUE", 1u);
            break;
        case PostProcessVariant::TRANSLUCENT:
            CodeGenerator::generateDefine(out, "POST_PROCESS_OPAQUE", 0u);
            break;
    }
}

void ShaderGenerator::appendShader(io::sstream& ss,
        const CString& shader, size_t lineOffset) noexcept {

    auto countLines = [](const char* s) -> size_t {
        size_t lines = 0;
        size_t i = 0;
        while (s[i]) {
            if (s[i++] == '\n') lines++;
        }
        return lines;
    };

    if (!shader.empty()) {
        size_t lines = countLines(ss.c_str());
        ss << "#line " << lineOffset + 1 << '\n';
        ss << shader.c_str();
        if (shader[shader.size() - 1] != '\n') {
            ss << "\n";
            lines++;
        }
        // + 2 to account for the #line directives we just added
        ss << "#line " << lines + countLines(shader.c_str()) + 2 << "\n";
    }
}

void ShaderGenerator::generateUserSpecConstants(
        const CodeGenerator& cg, io::sstream& fs, MaterialBuilder::ConstantList const& constants) {
    // Constants 0 to CONFIG_MAX_RESERVED_SPEC_CONSTANTS - 1 are reserved by Filament.
    size_t index = CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
    for (const auto& constant : constants) {
        std::string const fullName = std::string("materialConstants_") + constant.name.c_str();
        switch (constant.type) {
            case ConstantType::INT:
                cg.generateSpecializationConstant(
                        fs, fullName.c_str(), index++, constant.defaultValue.i);
                break;
            case ConstantType::FLOAT:
                cg.generateSpecializationConstant(
                        fs, fullName.c_str(), index++, constant.defaultValue.f);
                break;
            case ConstantType::BOOL:
                cg.generateSpecializationConstant(
                        fs, fullName.c_str(), index++, constant.defaultValue.b);
                break;
        }
    }
}

// ------------------------------------------------------------------------------------------------

ShaderGenerator::ShaderGenerator(
        MaterialBuilder::PropertyList const& properties,
        MaterialBuilder::VariableList const& variables,
        MaterialBuilder::OutputList const& outputs,
        MaterialBuilder::PreprocessorDefineList const& defines,
        MaterialBuilder::ConstantList const& constants,
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
    mConstants = constants;

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

void ShaderGenerator::fixupExternalSamplers(ShaderModel sm, std::string& shader,
        MaterialBuilder::FeatureLevel featureLevel,
        MaterialInfo const& material) noexcept {
    // External samplers are only supported on GL ES at the moment, we must
    // skip the fixup on desktop targets
    if (material.hasExternalSamplers && sm == ShaderModel::MOBILE) {
        CodeGenerator::fixupExternalSamplers(shader, material.sib, featureLevel);
    }
}

std::string ShaderGenerator::createVertexProgram(ShaderModel shaderModel,
        MaterialBuilder::TargetApi targetApi, MaterialBuilder::TargetLanguage targetLanguage,
        MaterialBuilder::FeatureLevel featureLevel,
        MaterialInfo const& material, const filament::Variant variant, Interpolation interpolation,
        VertexDomain vertexDomain) const noexcept {

    assert_invariant(filament::Variant::isValid(variant));
    assert_invariant(mMaterialDomain != MaterialBuilder::MaterialDomain::COMPUTE);

    if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return createPostProcessVertexProgram(
                shaderModel, targetApi,
                targetLanguage, featureLevel, material, variant.key);
    }

    io::sstream vs;

    const CodeGenerator cg(shaderModel, targetApi, targetLanguage, featureLevel);

    cg.generateProlog(vs, ShaderStage::VERTEX, material, variant);

    generateUserSpecConstants(cg, vs, mConstants);

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

    generateSurfaceMaterialVariantDefines(vs, ShaderStage::VERTEX, featureLevel, material, variant);

    generateSurfaceMaterialVariantProperties(vs, mProperties, mDefines);

    AttributeBitset attributes = material.requiredAttributes;
    if (hasSkinningOrMorphing(variant, featureLevel)) {
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
    cg.generateShaderInputs(vs, ShaderStage::VERTEX, attributes, interpolation);

    CodeGenerator::generateCommonTypes(vs, ShaderStage::VERTEX);

    // custom material variables
    size_t variableIndex = 0;
    for (const auto& variable : mVariables) {
        CodeGenerator::generateVariable(vs, ShaderStage::VERTEX, variable, variableIndex++);
    }

    // materials defines
    generateVertexDomainDefines(vs, vertexDomain);

    // uniforms
    cg.generateUniforms(vs, ShaderStage::VERTEX,
            UniformBindingPoints::PER_VIEW, UibGenerator::getPerViewUib());

    cg.generateUniforms(vs, ShaderStage::VERTEX,
            UniformBindingPoints::PER_RENDERABLE, UibGenerator::getPerRenderableUib());

    const bool litVariants = material.isLit || material.hasShadowMultiplier;
    if (litVariants && filament::Variant::isShadowReceiverVariant(variant)) {
        cg.generateUniforms(vs, ShaderStage::FRAGMENT,
                UniformBindingPoints::SHADOW, UibGenerator::getShadowUib());
    }

    if (hasSkinningOrMorphing(variant, featureLevel)) {
        cg.generateUniforms(vs, ShaderStage::VERTEX,
                UniformBindingPoints::PER_RENDERABLE_BONES,
                UibGenerator::getPerRenderableBonesUib());
        cg.generateSamplers(vs, SamplerBindingPoints::PER_RENDERABLE_SKINNING,
                material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_RENDERABLE_SKINNING),
                SibGenerator::getPerRenderPrimitiveBonesSib(variant));
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
        MaterialBuilder::FeatureLevel featureLevel,
        MaterialInfo const& material, const filament::Variant variant,
        Interpolation interpolation) const noexcept {

    assert_invariant(filament::Variant::isValid(variant));
    assert_invariant(mMaterialDomain != MaterialBuilder::MaterialDomain::COMPUTE);

    if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
        return createPostProcessFragmentProgram(shaderModel, targetApi, targetLanguage,
                                                featureLevel, material, variant.key);
    }

    const CodeGenerator cg(shaderModel, targetApi, targetLanguage, featureLevel);

    io::sstream fs;
    cg.generateProlog(fs, ShaderStage::FRAGMENT, material, variant);

    generateUserSpecConstants(cg, fs, mConstants);

    generateSurfaceMaterialVariantDefines(
            fs, ShaderStage::FRAGMENT, featureLevel, material, variant);

    auto defaultSpecularAO = shaderModel == ShaderModel::MOBILE ?
                             SpecularAmbientOcclusion::NONE : SpecularAmbientOcclusion::SIMPLE;
    auto specularAO = material.specularAOSet ? material.specularAO : defaultSpecularAO;
    CodeGenerator::generateDefine(fs, "SPECULAR_AMBIENT_OCCLUSION", uint32_t(specularAO));

    bool const multiBounceAO = material.multiBounceAOSet ?
                               material.multiBounceAO : shaderModel == ShaderModel::DESKTOP;
    CodeGenerator::generateDefine(fs, "MULTI_BOUNCE_AMBIENT_OCCLUSION", multiBounceAO ? 1u : 0u);

    generateSurfaceMaterialVariantProperties(fs, mProperties, mDefines);


    cg.generateShaderInputs(fs, ShaderStage::FRAGMENT,
            material.requiredAttributes, interpolation);

    CodeGenerator::generateCommonTypes(fs, ShaderStage::FRAGMENT);

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

    if (variant.hasDynamicLighting()) {
        cg.generateUniforms(fs, ShaderStage::FRAGMENT,
                UniformBindingPoints::LIGHTS, UibGenerator::getLightsUib());
    }

    bool const litVariants = material.isLit || material.hasShadowMultiplier;
    if (litVariants && filament::Variant::isShadowReceiverVariant(variant)) {
        cg.generateUniforms(fs, ShaderStage::FRAGMENT,
                UniformBindingPoints::SHADOW, UibGenerator::getShadowUib());
    }

    if (variant.hasDynamicLighting()) {
        cg.generateUniforms(fs, ShaderStage::FRAGMENT,
                UniformBindingPoints::FROXEL_RECORDS, UibGenerator::getFroxelRecordUib());
        cg.generateUniforms(fs, ShaderStage::FRAGMENT,
                UniformBindingPoints::FROXELS, UibGenerator::getFroxelsUib());
    }

    cg.generateUniforms(fs, ShaderStage::FRAGMENT,
            UniformBindingPoints::PER_MATERIAL_INSTANCE, material.uib);

    CodeGenerator::generateSeparator(fs);

    if (featureLevel >= FeatureLevel::FEATURE_LEVEL_1) { // FIXME: generate only what we need
        cg.generateSamplers(fs, SamplerBindingPoints::PER_VIEW,
                material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_VIEW),
                SibGenerator::getPerViewSib(variant));
    }

    cg.generateSamplers(fs, SamplerBindingPoints::PER_MATERIAL_INSTANCE,
            material.samplerBindings.getBlockOffset(SamplerBindingPoints::PER_MATERIAL_INSTANCE),
            material.sib);

    fs << "float filament_lodBias;\n";

    // shading code
    CodeGenerator::generateCommon(fs, ShaderStage::FRAGMENT);
    CodeGenerator::generateGetters(fs, ShaderStage::FRAGMENT);
    CodeGenerator::generateCommonMaterial(fs, ShaderStage::FRAGMENT);
    CodeGenerator::generateParameters(fs, ShaderStage::FRAGMENT);

    if (filament::Variant::isFogVariant(variant)) {
        CodeGenerator::generateFog(fs, ShaderStage::FRAGMENT);
    }

    // shading model
    if (filament::Variant::isValidDepthVariant(variant)) {
        // In MASKED mode or with transparent shadows, we need the alpha channel computed by
        // the material (user code), so we append it here.
        if (material.userMaterialHasCustomDepth ||
            material.blendingMode == BlendingMode::MASKED || ((
                     material.blendingMode == BlendingMode::TRANSPARENT ||
                     material.blendingMode == BlendingMode::FADE)
             && material.hasTransparentShadow)) {
            appendShader(fs, mMaterialFragmentCode, mMaterialLineOffset);
        }
        // These variants are special and are treated as DEPTH variants. Filament will never
        // request that variant for the color pass.
        CodeGenerator::generateDepthShaderMain(fs, ShaderStage::FRAGMENT);
    } else {
        appendShader(fs, mMaterialFragmentCode, mMaterialLineOffset);
        if (filament::Variant::isSSRVariant(variant)) {
            CodeGenerator::generateShaderReflections(fs, ShaderStage::FRAGMENT);
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
        MaterialBuilder::FeatureLevel featureLevel,
        MaterialInfo const& material) const noexcept {
    assert_invariant(mMaterialDomain == MaterialBuilder::MaterialDomain::COMPUTE);
    assert_invariant(featureLevel >= FeatureLevel::FEATURE_LEVEL_2);
    const CodeGenerator cg(shaderModel, targetApi, targetLanguage, featureLevel);
    io::sstream s;

    cg.generateProlog(s, ShaderStage::COMPUTE, material, {});

    generateUserSpecConstants(cg, s, mConstants);

    CodeGenerator::generateCommonTypes(s, ShaderStage::COMPUTE);

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
        MaterialBuilder::FeatureLevel featureLevel,
        MaterialInfo const& material, const filament::Variant::type_t variantKey) const noexcept {
    const CodeGenerator cg(sm, targetApi, targetLanguage, featureLevel);
    io::sstream vs;
    cg.generateProlog(vs, ShaderStage::VERTEX, material, {});

    generateUserSpecConstants(cg, vs, mConstants);

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
        MaterialBuilder::FeatureLevel featureLevel,
        MaterialInfo const& material, uint8_t variant) const noexcept {
    const CodeGenerator cg(sm, targetApi, targetLanguage, featureLevel);
    io::sstream fs;
    cg.generateProlog(fs, ShaderStage::FRAGMENT, material, {});

    generateUserSpecConstants(cg, fs, mConstants);

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
                    output.qualifier, output.precision, output.type);
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

bool ShaderGenerator::hasSkinningOrMorphing(
        filament::Variant variant, MaterialBuilder::FeatureLevel featureLevel) noexcept {
    return variant.hasSkinningOrMorphing()
            // HACK(exv): Ignore skinning/morphing variant when targeting ESSL 1.0. We should
            // either properly support skinning on FL0 or build a system in matc which allows
            // the set of included variants to differ per-feature level.
            && featureLevel > MaterialBuilder::FeatureLevel::FEATURE_LEVEL_0;
}

bool ShaderGenerator::hasStereo(
        filament::Variant variant, MaterialBuilder::FeatureLevel featureLevel) noexcept {
    return variant.hasStereo()
            // HACK(exv): Ignore stereo variant when targeting ESSL 1.0. We should properly build a
            // system in matc which allows the set of included variants to differ per-feature level.
            && featureLevel > MaterialBuilder::FeatureLevel::FEATURE_LEVEL_0;
}

} // namespace filament
