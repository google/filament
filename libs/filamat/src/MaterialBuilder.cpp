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

#include "filamat/MaterialBuilder.h"

#include <filamat/Enums.h>

#include "Includes.h"
#include "MaterialVariants.h"
#include "shaders/SibGenerator.h"
#include "shaders/UibGenerator.h"

#include "GLSLPostProcessor.h"
#include "sca/GLSLTools.h"

#include "shaders/MaterialInfo.h"
#include "shaders/ShaderGenerator.h"

#include "eiff/BlobDictionary.h"
#include "eiff/LineDictionary.h"
#include "eiff/MaterialInterfaceBlockChunk.h"
#include "eiff/MaterialTextChunk.h"
#include "eiff/MaterialBinaryChunk.h"
#include "eiff/ChunkContainer.h"
#include "eiff/SimpleFieldChunk.h"
#include "eiff/DictionaryTextChunk.h"
#include "eiff/DictionarySpirvChunk.h"
#include "eiff/DictionaryMetalLibraryChunk.h"

#include <private/filament/BufferInterfaceBlock.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/UibStructs.h>
#include <private/filament/ConstantInfo.h>

#include <backend/Program.h>

#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Mutex.h>
#include <utils/Panic.h>
#include <utils/Hash.h>

#include <atomic>
#include <utility>
#include <vector>


namespace filamat {

using namespace utils;
using namespace filament;

// Note: the VertexAttribute enum value must match the index in the array
const MaterialBuilder::AttributeDatabase MaterialBuilder::sAttributeDatabase = {{
        { "position",      AttributeType::FLOAT4, VertexAttribute::POSITION     },
        { "tangents",      AttributeType::FLOAT4, VertexAttribute::TANGENTS     },
        { "color",         AttributeType::FLOAT4, VertexAttribute::COLOR        },
        { "uv0",           AttributeType::FLOAT2, VertexAttribute::UV0          },
        { "uv1",           AttributeType::FLOAT2, VertexAttribute::UV1          },
        { "bone_indices",  AttributeType::UINT4,  VertexAttribute::BONE_INDICES },
        { "bone_weights",  AttributeType::FLOAT4, VertexAttribute::BONE_WEIGHTS },
        { },
        { "custom0",       AttributeType::FLOAT4, VertexAttribute::CUSTOM0      },
        { "custom1",       AttributeType::FLOAT4, VertexAttribute::CUSTOM1      },
        { "custom2",       AttributeType::FLOAT4, VertexAttribute::CUSTOM2      },
        { "custom3",       AttributeType::FLOAT4, VertexAttribute::CUSTOM3      },
        { "custom4",       AttributeType::FLOAT4, VertexAttribute::CUSTOM4      },
        { "custom5",       AttributeType::FLOAT4, VertexAttribute::CUSTOM5      },
        { "custom6",       AttributeType::FLOAT4, VertexAttribute::CUSTOM6      },
        { "custom7",       AttributeType::FLOAT4, VertexAttribute::CUSTOM7      },
}};

std::atomic<int> MaterialBuilderBase::materialBuilderClients(0);

inline void assertSingleTargetApi(MaterialBuilderBase::TargetApi api) {
    // Assert that a single bit is set.
    UTILS_UNUSED uint8_t bits = (uint8_t)api;
    assert(bits && !(bits & bits - 1u));
}

void MaterialBuilderBase::prepare(bool vulkanSemantics,
        filament::backend::FeatureLevel featureLevel) {
    mCodeGenPermutations.clear();
    mShaderModels.reset();

    if (mPlatform == Platform::MOBILE) {
        mShaderModels.set(static_cast<size_t>(ShaderModel::MOBILE));
    } else if (mPlatform == Platform::DESKTOP) {
        mShaderModels.set(static_cast<size_t>(ShaderModel::DESKTOP));
    } else if (mPlatform == Platform::ALL) {
        mShaderModels.set(static_cast<size_t>(ShaderModel::MOBILE));
        mShaderModels.set(static_cast<size_t>(ShaderModel::DESKTOP));
    }

    // OpenGL is a special case. If we're doing any optimization, then we need to go to Spir-V.
    TargetLanguage glTargetLanguage = mOptimization > MaterialBuilder::Optimization::PREPROCESSOR ?
                                      TargetLanguage::SPIRV : TargetLanguage::GLSL;
    if (vulkanSemantics) {
        // Currently GLSLPostProcessor.cpp is incapable of compiling SPIRV to GLSL without
        // running the optimizer. For now we just activate the optimizer in that case.
        mOptimization = MaterialBuilder::Optimization::PERFORMANCE;
        glTargetLanguage = TargetLanguage::SPIRV;
    }

    // Select OpenGL as the default TargetApi if none was specified.
    if (none(mTargetApi)) {
        mTargetApi = TargetApi::OPENGL;
    }

    // Generally build for a minimum of feature level 1. If feature level 0 is specified, an extra
    // permutation is specifically included for the OpenGL/mobile target.
    MaterialBuilder::FeatureLevel effectiveFeatureLevel =
            std::max(featureLevel, filament::backend::FeatureLevel::FEATURE_LEVEL_1);

    // Build a list of codegen permutations, which is useful across all types of material builders.
    static_assert(backend::SHADER_MODEL_COUNT == 2);
    for (const auto shaderModel: { ShaderModel::MOBILE, ShaderModel::DESKTOP }) {
        const auto i = static_cast<uint8_t>(shaderModel);
        if (!mShaderModels.test(i)) {
            continue; // skip this shader model since it was not requested.
        }

        if (any(mTargetApi & TargetApi::OPENGL)) {
            mCodeGenPermutations.push_back({
                shaderModel,
                TargetApi::OPENGL,
                glTargetLanguage,
                effectiveFeatureLevel,
            });
            if (mIncludeEssl1
                    && featureLevel == filament::backend::FeatureLevel::FEATURE_LEVEL_0
                    && shaderModel == ShaderModel::MOBILE) {
                mCodeGenPermutations.push_back({
                    shaderModel,
                    TargetApi::OPENGL,
                    glTargetLanguage,
                    filament::backend::FeatureLevel::FEATURE_LEVEL_0
                });
            }
        }
        if (any(mTargetApi & TargetApi::VULKAN)) {
            mCodeGenPermutations.push_back({
                shaderModel,
                TargetApi::VULKAN,
                TargetLanguage::SPIRV,
                effectiveFeatureLevel,
            });
        }
        if (any(mTargetApi & TargetApi::METAL)) {
            mCodeGenPermutations.push_back({
                shaderModel,
                TargetApi::METAL,
                TargetLanguage::SPIRV,
                effectiveFeatureLevel,
            });
        }
    }
}

MaterialBuilder::MaterialBuilder() : mMaterialName("Unnamed") {
    std::fill_n(mProperties, MATERIAL_PROPERTIES_COUNT, false);
    mShaderModels.reset();
}

MaterialBuilder::~MaterialBuilder() = default;

void MaterialBuilderBase::init() {
    materialBuilderClients++;
    GLSLTools::init();
}

void MaterialBuilderBase::shutdown() {
    materialBuilderClients--;
    GLSLTools::shutdown();
}

MaterialBuilder& MaterialBuilder::name(const char* name) noexcept {
    mMaterialName = CString(name);
    return *this;
}

MaterialBuilder& MaterialBuilder::fileName(const char* fileName) noexcept {
    mFileName = CString(fileName);
    return *this;
}

MaterialBuilder& MaterialBuilder::material(const char* code, size_t line) noexcept {
    mMaterialFragmentCode.setUnresolved(CString(code));
    mMaterialFragmentCode.setLineOffset(line);
    return *this;
}

MaterialBuilder& MaterialBuilder::includeCallback(IncludeCallback callback) noexcept {
    mIncludeCallback = std::move(callback);
    return *this;
}

MaterialBuilder& MaterialBuilder::materialVertex(const char* code, size_t line) noexcept {
    mMaterialVertexCode.setUnresolved(CString(code));
    mMaterialVertexCode.setLineOffset(line);
    return *this;
}

MaterialBuilder& MaterialBuilder::shading(Shading shading) noexcept {
    mShading = shading;
    return *this;
}

MaterialBuilder& MaterialBuilder::interpolation(Interpolation interpolation) noexcept {
    mInterpolation = interpolation;
    return *this;
}

MaterialBuilder& MaterialBuilder::variable(Variable v, const char* name) noexcept {
    switch (v) {
        case Variable::CUSTOM0:
        case Variable::CUSTOM1:
        case Variable::CUSTOM2:
        case Variable::CUSTOM3:
            assert(size_t(v) < MATERIAL_VARIABLES_COUNT);
            mVariables[size_t(v)] = CString(name);
            break;
    }
    return *this;
}

MaterialBuilder& MaterialBuilder::parameter(const char* name, size_t size, UniformType type,
        ParameterPrecision precision) noexcept {
    ASSERT_POSTCONDITION(mParameterCount < MAX_PARAMETERS_COUNT, "Too many parameters");
    mParameters[mParameterCount++] = { name, type, size, precision };
    return *this;
}

MaterialBuilder& MaterialBuilder::parameter(const char* name, UniformType type,
        ParameterPrecision precision) noexcept {
    return parameter(name, 1, type, precision);
}


MaterialBuilder& MaterialBuilder::parameter(const char* name, SamplerType samplerType,
        SamplerFormat format, ParameterPrecision precision, bool multisample) noexcept {

    ASSERT_PRECONDITION(
            !multisample || (format != SamplerFormat::SHADOW && (
                                    samplerType == SamplerType::SAMPLER_2D
                                            || samplerType == SamplerType::SAMPLER_2D_ARRAY)),
            "multisample samplers only possible with SAMPLER_2D or SAMPLER_2D_ARRAY,"
            " as long as type is not SHADOW");

    ASSERT_POSTCONDITION(mParameterCount < MAX_PARAMETERS_COUNT, "Too many parameters");
    mParameters[mParameterCount++] = { name, samplerType, format, precision, multisample };
    return *this;
}

template<typename T, typename>
MaterialBuilder& MaterialBuilder::constant(const char* name, ConstantType type, T defaultValue) {
    auto result = std::find_if(mConstants.begin(), mConstants.end(), [name](const Constant& c) {
        return c.name == utils::CString(name);
    });
    ASSERT_POSTCONDITION(result == mConstants.end(),
            "There is already a constant parameter present with the name %s.", name);
    Constant constant {
            .name = CString(name),
            .type = type,
    };
    auto toString = [](ConstantType t) {
        switch (t) {
            case ConstantType::INT: return "INT";
            case ConstantType::FLOAT: return "FLOAT";
            case ConstantType::BOOL: return "BOOL";
        }
    };

    const char* const errMessage =
            "Constant %s was declared with type %s but given %s default value.";
    if constexpr (std::is_same_v<T, int32_t>) {
        ASSERT_POSTCONDITION(
                type == ConstantType::INT, errMessage, name, toString(type), "an int");
        constant.defaultValue.i = defaultValue;
    } else if constexpr (std::is_same_v<T, float>) {
        ASSERT_POSTCONDITION(
                type == ConstantType::FLOAT, errMessage, name, toString(type), "a float");
        constant.defaultValue.f = defaultValue;
    } else if constexpr (std::is_same_v<T, bool>) {
        ASSERT_POSTCONDITION(
                type == ConstantType::BOOL, errMessage, name, toString(type), "a bool");
        constant.defaultValue.b = defaultValue;
    } else {
        assert_invariant(false);
    }

    mConstants.push_back(constant);
    return *this;
}
template MaterialBuilder& MaterialBuilder::constant<int32_t>(
        const char* name, ConstantType type, int32_t defaultValue);
template MaterialBuilder& MaterialBuilder::constant<float>(
        const char* name, ConstantType type, float defaultValue);
template MaterialBuilder& MaterialBuilder::constant<bool>(
        const char* name, ConstantType type, bool defaultValue);

MaterialBuilder& MaterialBuilder::buffer(BufferInterfaceBlock bib) noexcept {
    ASSERT_POSTCONDITION(mBuffers.size() < MAX_BUFFERS_COUNT, "Too many buffers");
    mBuffers.emplace_back(std::make_unique<filament::BufferInterfaceBlock>(std::move(bib)));
    return *this;
}

MaterialBuilder& MaterialBuilder::subpass(SubpassType subpassType, SamplerFormat format,
        ParameterPrecision precision, const char* name) noexcept {
    ASSERT_PRECONDITION(format == SamplerFormat::FLOAT,
            "Subpass parameters must have FLOAT format.");

    ASSERT_POSTCONDITION(mSubpassCount < MAX_SUBPASS_COUNT, "Too many subpasses");
    mSubpasses[mSubpassCount++] = { name, subpassType, format, precision };
    return *this;
}

MaterialBuilder& MaterialBuilder::subpass(SubpassType subpassType, SamplerFormat format,
        const char* name) noexcept {
    return subpass(subpassType, format, ParameterPrecision::DEFAULT, name);
}

MaterialBuilder& MaterialBuilder::subpass(SubpassType subpassType, ParameterPrecision precision,
        const char* name) noexcept {
    return subpass(subpassType, SamplerFormat::FLOAT, precision, name);
}

MaterialBuilder& MaterialBuilder::subpass(SubpassType subpassType, const char* name) noexcept {
    return subpass(subpassType, SamplerFormat::FLOAT, ParameterPrecision::DEFAULT, name);
}

MaterialBuilder& MaterialBuilder::require(VertexAttribute attribute) noexcept {
    mRequiredAttributes.set(attribute);
    return *this;
}

MaterialBuilder& MaterialBuilder::groupSize(filament::math::uint3 groupSize) noexcept {
    mGroupSize = groupSize;
    return *this;
}

MaterialBuilder& MaterialBuilder::materialDomain(
        MaterialBuilder::MaterialDomain materialDomain) noexcept {
    mMaterialDomain = materialDomain;
    if (mMaterialDomain == MaterialDomain::COMPUTE) {
        // compute implies feature level 2
        if (mFeatureLevel < FeatureLevel::FEATURE_LEVEL_2) {
            mFeatureLevel = FeatureLevel::FEATURE_LEVEL_2;
        }
    }
    return *this;
}

MaterialBuilder& MaterialBuilder::refractionMode(RefractionMode refraction) noexcept {
    mRefractionMode = refraction;
    return *this;
}

MaterialBuilder& MaterialBuilder::refractionType(RefractionType refractionType) noexcept {
    mRefractionType = refractionType;
    return *this;
}

MaterialBuilder& MaterialBuilder::quality(ShaderQuality quality) noexcept {
    mShaderQuality = quality;
    return *this;
}

MaterialBuilder& MaterialBuilder::featureLevel(FeatureLevel featureLevel) noexcept {
    mFeatureLevel = featureLevel;
    return *this;
}

MaterialBuilder& MaterialBuilder::blending(BlendingMode blending) noexcept {
    mBlendingMode = blending;
    return *this;
}

MaterialBuilder& MaterialBuilder::customBlendFunctions(
        BlendFunction srcRGB, BlendFunction srcA,
        BlendFunction dstRGB, BlendFunction dstA) noexcept {
    mCustomBlendFunctions[0] = srcRGB;
    mCustomBlendFunctions[1] = srcA;
    mCustomBlendFunctions[2] = dstRGB;
    mCustomBlendFunctions[3] = dstA;
    return *this;
}

MaterialBuilder& MaterialBuilder::postLightingBlending(BlendingMode blending) noexcept {
    mPostLightingBlendingMode = blending;
    return *this;
}

MaterialBuilder& MaterialBuilder::vertexDomain(VertexDomain domain) noexcept {
    mVertexDomain = domain;
    return *this;
}

MaterialBuilder& MaterialBuilder::culling(CullingMode culling) noexcept {
    mCullingMode = culling;
    return *this;
}

MaterialBuilder& MaterialBuilder::colorWrite(bool enable) noexcept {
    mColorWrite = enable;
    return *this;
}

MaterialBuilder& MaterialBuilder::depthWrite(bool enable) noexcept {
    mDepthWrite = enable;
    mDepthWriteSet = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::depthCulling(bool enable) noexcept {
    mDepthTest = enable;
    return *this;
}

MaterialBuilder& MaterialBuilder::instanced(bool enable) noexcept {
    mInstanced = enable;
    return *this;
}

MaterialBuilder& MaterialBuilder::doubleSided(bool doubleSided) noexcept {
    mDoubleSided = doubleSided;
    mDoubleSidedCapability = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::maskThreshold(float threshold) noexcept {
    mMaskThreshold = threshold;
    return *this;
}

MaterialBuilder& MaterialBuilder::alphaToCoverage(bool enable) noexcept {
    mAlphaToCoverage = enable;
    mAlphaToCoverageSet = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::shadowMultiplier(bool shadowMultiplier) noexcept {
    mShadowMultiplier = shadowMultiplier;
    return *this;
}

MaterialBuilder& MaterialBuilder::transparentShadow(bool transparentShadow) noexcept {
    mTransparentShadow = transparentShadow;
    return *this;
}

MaterialBuilder& MaterialBuilder::specularAntiAliasing(bool specularAntiAliasing) noexcept {
    mSpecularAntiAliasing = specularAntiAliasing;
    return *this;
}

MaterialBuilder& MaterialBuilder::specularAntiAliasingVariance(float screenSpaceVariance) noexcept {
    mSpecularAntiAliasingVariance = screenSpaceVariance;
    return *this;
}

MaterialBuilder& MaterialBuilder::specularAntiAliasingThreshold(float threshold) noexcept {
    mSpecularAntiAliasingThreshold = threshold;
    return *this;
}

MaterialBuilder& MaterialBuilder::clearCoatIorChange(bool clearCoatIorChange) noexcept {
    mClearCoatIorChange = clearCoatIorChange;
    return *this;
}

MaterialBuilder& MaterialBuilder::flipUV(bool flipUV) noexcept {
    mFlipUV = flipUV;
    return *this;
}

MaterialBuilder& MaterialBuilder::customSurfaceShading(bool customSurfaceShading) noexcept {
    mCustomSurfaceShading = customSurfaceShading;
    return *this;
}

MaterialBuilder& MaterialBuilder::multiBounceAmbientOcclusion(bool multiBounceAO) noexcept {
    mMultiBounceAO = multiBounceAO;
    mMultiBounceAOSet = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::specularAmbientOcclusion(SpecularAmbientOcclusion specularAO) noexcept {
    mSpecularAO = specularAO;
    mSpecularAOSet = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::transparencyMode(TransparencyMode mode) noexcept {
    mTransparencyMode = mode;
    return *this;
}

MaterialBuilder& MaterialBuilder::stereoscopicType(StereoscopicType stereoscopicType) noexcept {
    mStereoscopicType = stereoscopicType;
    return *this;
}

MaterialBuilder& MaterialBuilder::stereoscopicEyeCount(uint8_t eyeCount) noexcept {
    mStereoscopicEyeCount = eyeCount;
    return *this;
}

MaterialBuilder& MaterialBuilder::reflectionMode(ReflectionMode mode) noexcept {
    mReflectionMode = mode;
    return *this;
}

MaterialBuilder& MaterialBuilder::platform(Platform platform) noexcept {
    mPlatform = platform;
    return *this;
}

MaterialBuilder& MaterialBuilder::targetApi(TargetApi targetApi) noexcept {
    mTargetApi |= targetApi;
    return *this;
}

MaterialBuilder& MaterialBuilder::optimization(Optimization optimization) noexcept {
    mOptimization = optimization;
    return *this;
}

MaterialBuilder& MaterialBuilder::printShaders(bool printShaders) noexcept {
    mPrintShaders = printShaders;
    return *this;
}

MaterialBuilder& MaterialBuilder::generateDebugInfo(bool generateDebugInfo) noexcept {
    mGenerateDebugInfo = generateDebugInfo;
    return *this;
}

MaterialBuilder& MaterialBuilder::variantFilter(UserVariantFilterMask variantFilter) noexcept {
    mVariantFilter = variantFilter;
    return *this;
}

MaterialBuilder& MaterialBuilder::shaderDefine(const char* name, const char* value) noexcept {
    mDefines.emplace_back(name, value);
    return *this;
}

bool MaterialBuilder::hasSamplerType(SamplerType samplerType) const noexcept {
    for (size_t i = 0, c = mParameterCount; i < c; i++) {
        auto const& param = mParameters[i];
        if (param.isSampler() && param.samplerType == samplerType) {
            return  true;
        }
    }
    return false;
}

void MaterialBuilder::prepareToBuild(MaterialInfo& info) noexcept {
    MaterialBuilderBase::prepare(mEnableFramebufferFetch, mFeatureLevel);

    // Build the per-material sampler block and uniform block.
    SamplerInterfaceBlock::Builder sbb;
    BufferInterfaceBlock::Builder ibb;
    for (size_t i = 0, c = mParameterCount; i < c; i++) {
        auto const& param = mParameters[i];
        assert_invariant(!param.isSubpass());
        if (param.isSampler()) {
            sbb.add({ param.name.data(), param.name.size() },
                    param.samplerType, param.format, param.precision, param.multisample);
        } else if (param.isUniform()) {
            ibb.add({{{ param.name.data(), param.name.size() },
                      uint32_t(param.size == 1u ? 0u : param.size), param.uniformType,
                      param.precision, FeatureLevel::FEATURE_LEVEL_0 }});
        }
    }

    for (size_t i = 0, c = mSubpassCount; i < c; i++) {
        auto const& param = mSubpasses[i];
        assert_invariant(param.isSubpass());
        // For now, we only support a single subpass for attachment 0.
        // Subpasses belong to the "MaterialParams" block.
        const uint8_t attachmentIndex = 0;
        const uint8_t binding = 0;
        info.subpass = { CString("MaterialParams"), param.name, param.subpassType,
                         param.format, param.precision, attachmentIndex, binding };
    }

    for (auto const& buffer : mBuffers) {
        info.buffers.emplace_back(buffer.get());
    }

    if (mSpecularAntiAliasing) {
        ibb.add({
                { "_specularAntiAliasingVariance",  0, UniformType::FLOAT },
                { "_specularAntiAliasingThreshold", 0, UniformType::FLOAT },
        });
    }

    if (mBlendingMode == BlendingMode::MASKED) {
        ibb.add({{ "_maskThreshold", 0, UniformType::FLOAT, Precision::DEFAULT, FeatureLevel::FEATURE_LEVEL_0 }});
    }

    if (mDoubleSidedCapability) {
        ibb.add({{ "_doubleSided", 0, UniformType::BOOL, Precision::DEFAULT, FeatureLevel::FEATURE_LEVEL_0 }});
    }

    mRequiredAttributes.set(VertexAttribute::POSITION);
    if (mShading != Shading::UNLIT || mShadowMultiplier) {
        mRequiredAttributes.set(VertexAttribute::TANGENTS);
    }

    info.sib = sbb.name("MaterialParams").build();
    info.uib = ibb.name("MaterialParams").build();

    info.isLit = isLit();
    info.hasDoubleSidedCapability = mDoubleSidedCapability;
    info.hasExternalSamplers = hasSamplerType(SamplerType::SAMPLER_EXTERNAL);
    info.has3dSamplers = hasSamplerType(SamplerType::SAMPLER_3D);
    info.specularAntiAliasing = mSpecularAntiAliasing;
    info.clearCoatIorChange = mClearCoatIorChange;
    info.flipUV = mFlipUV;
    info.requiredAttributes = mRequiredAttributes;
    info.blendingMode = mBlendingMode;
    info.postLightingBlendingMode = mPostLightingBlendingMode;
    info.shading = mShading;
    info.hasShadowMultiplier = mShadowMultiplier;
    info.hasTransparentShadow = mTransparentShadow;
    info.multiBounceAO = mMultiBounceAO;
    info.multiBounceAOSet = mMultiBounceAOSet;
    info.specularAO = mSpecularAO;
    info.specularAOSet = mSpecularAOSet;
    info.refractionMode = mRefractionMode;
    info.refractionType = mRefractionType;
    info.reflectionMode = mReflectionMode;
    info.quality = mShaderQuality;
    info.hasCustomSurfaceShading = mCustomSurfaceShading;
    info.useLegacyMorphing = mUseLegacyMorphing;
    info.instanced = mInstanced;
    info.vertexDomainDeviceJittered = mVertexDomainDeviceJittered;
    info.featureLevel = mFeatureLevel;
    info.groupSize = mGroupSize;
    info.stereoscopicType = mStereoscopicType;
    info.stereoscopicEyeCount = mStereoscopicEyeCount;

    // This is determined via static analysis of the glsl after prepareToBuild().
    info.userMaterialHasCustomDepth = false;
}

bool MaterialBuilder::findProperties(backend::ShaderStage type,
        MaterialBuilder::PropertyList& allProperties,
        CodeGenParams const& semanticCodeGenParams) noexcept {
    GLSLTools glslTools;
    std::string shaderCodeAllProperties = peek(type, semanticCodeGenParams, allProperties);
    // Populate mProperties with the properties set in the shader.
    if (!glslTools.findProperties(type, shaderCodeAllProperties, mProperties,
            semanticCodeGenParams.targetApi,
            semanticCodeGenParams.targetLanguage,
            semanticCodeGenParams.shaderModel)) {
        if (mPrintShaders) {
            slog.e << shaderCodeAllProperties << io::endl;
        }
        return false;
    }
    return true;
}

bool MaterialBuilder::findAllProperties(CodeGenParams const& semanticCodeGenParams) noexcept {
    if (mMaterialDomain != MaterialDomain::SURFACE) {
        return true;
    }

    using namespace backend;

    // Some fields in MaterialInputs only exist if the property is set (e.g: normal, subsurface
    // for cloth shading model). Give our shader all properties. This will enable us to parse and
    // static code analyse the AST.
    MaterialBuilder::PropertyList allProperties;
    std::fill_n(allProperties, MATERIAL_PROPERTIES_COUNT, true);
    if (!findProperties(ShaderStage::FRAGMENT, allProperties, semanticCodeGenParams)) {
        return false;
    }
    if (!findProperties(ShaderStage::VERTEX, allProperties, semanticCodeGenParams)) {
        return false;
    }
    return true;
}

bool MaterialBuilder::runSemanticAnalysis(MaterialInfo* inOutInfo,
        CodeGenParams const& semanticCodeGenParams) noexcept {
    using namespace backend;

    TargetApi targetApi = semanticCodeGenParams.targetApi;
    TargetLanguage const targetLanguage = semanticCodeGenParams.targetLanguage;
    assertSingleTargetApi(targetApi);

    if (mEnableFramebufferFetch) {
        // framebuffer fetch is only available with vulkan semantics
        targetApi = TargetApi::VULKAN;
    }

    bool success = false;
    std::string shaderCode;
    ShaderModel const model = semanticCodeGenParams.shaderModel;
    if (mMaterialDomain == filament::MaterialDomain::COMPUTE) {
        shaderCode = peek(ShaderStage::COMPUTE, semanticCodeGenParams, mProperties);
        success = GLSLTools::analyzeComputeShader(shaderCode, model,
                targetApi, targetLanguage);
    } else {
        shaderCode = peek(ShaderStage::VERTEX, semanticCodeGenParams, mProperties);
        success = GLSLTools::analyzeVertexShader(shaderCode, model, mMaterialDomain,
                targetApi, targetLanguage);
        if (success) {
            shaderCode = peek(ShaderStage::FRAGMENT, semanticCodeGenParams, mProperties);
            auto result = GLSLTools::analyzeFragmentShader(shaderCode, model, mMaterialDomain,
                    targetApi, targetLanguage, mCustomSurfaceShading);
            success = result.has_value();
            if (success) {
                inOutInfo->userMaterialHasCustomDepth = result->userMaterialHasCustomDepth;
            }
        }
    }
    if (!success && mPrintShaders) {
        slog.e << shaderCode << io::endl;
    }
    return success;
}

bool MaterialBuilder::ShaderCode::resolveIncludes(IncludeCallback callback,
        const CString& fileName) noexcept {
    if (!mCode.empty()) {
        ResolveOptions options {
                .insertLineDirectives = true,
                .insertLineDirectiveCheck = true
        };
        IncludeResult source {
                .includeName = fileName,
                .text = mCode,
                .lineNumberOffset = getLineOffset(),
                .name = CString("")
        };
        if (!::filamat::resolveIncludes(source, std::move(callback), options)) {
            return false;
        }
        mCode = source.text;
    }

    mIncludesResolved = true;
    return true;
}

static void showErrorMessage(const char* materialName, filament::Variant variant,
        MaterialBuilder::TargetApi targetApi, backend::ShaderStage shaderType,
        MaterialBuilder::FeatureLevel featureLevel,
        const std::string& shaderCode) {
    using ShaderStage = backend::ShaderStage;
    using TargetApi = MaterialBuilder::TargetApi;

    const char* targetApiString;
    switch (targetApi) {
        case TargetApi::OPENGL:
            targetApiString = (featureLevel == MaterialBuilder::FeatureLevel::FEATURE_LEVEL_0)
                              ? "GLES 2.0.\n" : "OpenGL.\n";
            break;
        case TargetApi::VULKAN:
            targetApiString = "Vulkan.\n";
            break;
        case TargetApi::METAL:
            targetApiString = "Metal.\n";
            break;
        case TargetApi::ALL:
            assert(0); // Unreachable.
            break;
    }

    const char* shaderStageString;
    switch (shaderType) {
        case ShaderStage::VERTEX:
            shaderStageString = "Vertex Shader\n";
            break;
        case ShaderStage::FRAGMENT:
            shaderStageString = "Fragment Shader\n";
            break;
        case ShaderStage::COMPUTE:
            shaderStageString = "Compute Shader\n";
            break;
    }

    slog.e
            << "Error in \"" << materialName << "\""
            << ", Variant 0x" << io::hex << +variant.key
            << ", " << targetApiString
            << "=========================\n"
            << "Generated " << shaderStageString
            << "=========================\n"
            << shaderCode;
}

bool MaterialBuilder::generateShaders(JobSystem& jobSystem, const std::vector<Variant>& variants,
        ChunkContainer& container, const MaterialInfo& info) const noexcept {
    // Create a postprocessor to optimize / compile to Spir-V if necessary.

    uint32_t flags = 0;
    flags |= mPrintShaders ? GLSLPostProcessor::PRINT_SHADERS : 0;
    flags |= mGenerateDebugInfo ? GLSLPostProcessor::GENERATE_DEBUG_INFO : 0;
    GLSLPostProcessor postProcessor(mOptimization, flags);

    // Start: must be protected by lock
    Mutex entriesLock;
    std::vector<TextEntry> glslEntries;
    std::vector<TextEntry> essl1Entries;
    std::vector<BinaryEntry> spirvEntries;
    std::vector<TextEntry> metalEntries;
    LineDictionary textDictionary;
    BlobDictionary spirvDictionary;
    // End: must be protected by lock

    ShaderGenerator sg(mProperties, mVariables, mOutputs, mDefines, mConstants,
            mMaterialFragmentCode.getResolved(), mMaterialFragmentCode.getLineOffset(),
            mMaterialVertexCode.getResolved(), mMaterialVertexCode.getLineOffset(),
            mMaterialDomain);

    container.emplace<bool>(ChunkType::MaterialHasCustomDepthShader, needsStandardDepthProgram());

    std::atomic_bool cancelJobs(false);
    bool firstJob = true;

    for (const auto& params : mCodeGenPermutations) {
        if (cancelJobs.load()) {
            return false;
        }

        const ShaderModel shaderModel = ShaderModel(params.shaderModel);
        const TargetApi targetApi = params.targetApi;
        const TargetLanguage targetLanguage = params.targetLanguage;
        const FeatureLevel featureLevel = params.featureLevel;

        assertSingleTargetApi(targetApi);

        // Metal Shading Language is cross-compiled from Vulkan.
        const bool targetApiNeedsSpirv =
                (targetApi == TargetApi::VULKAN || targetApi == TargetApi::METAL);
        const bool targetApiNeedsMsl = targetApi == TargetApi::METAL;
        const bool targetApiNeedsGlsl = targetApi == TargetApi::OPENGL;

        // Set when a job fails
        JobSystem::Job* parent = jobSystem.createJob();

        for (const auto& v : variants) {
            JobSystem::Job* job = jobs::createJob(jobSystem, parent, [&]() {
                if (cancelJobs.load()) {
                    return;
                }

                // TODO: avoid allocations when not required
                std::vector<uint32_t> spirv;
                std::string msl;

                std::vector<uint32_t>* pSpirv = targetApiNeedsSpirv ? &spirv : nullptr;
                std::string* pMsl = targetApiNeedsMsl ? &msl : nullptr;

                TextEntry glslEntry{};
                BinaryEntry spirvEntry{};
                TextEntry metalEntry{};

                glslEntry.shaderModel  = params.shaderModel;
                spirvEntry.shaderModel = params.shaderModel;
                metalEntry.shaderModel = params.shaderModel;

                glslEntry.variant  = v.variant;
                spirvEntry.variant = v.variant;
                metalEntry.variant = v.variant;

                // Generate raw shader code.
                // The quotes in Google-style line directives cause problems with certain drivers. These
                // directives are optimized away when using the full filamat, so down below we
                // explicitly remove them when using filamat lite.
                std::string shader;
                if (v.stage == backend::ShaderStage::VERTEX) {
                    shader = sg.createVertexProgram(
                            shaderModel, targetApi, targetLanguage, featureLevel, info, v.variant,
                            mInterpolation, mVertexDomain);
                } else if (v.stage == backend::ShaderStage::FRAGMENT) {
                    shader = sg.createFragmentProgram(
                            shaderModel, targetApi, targetLanguage, featureLevel, info, v.variant,
                            mInterpolation);
                } else if (v.stage == backend::ShaderStage::COMPUTE) {
                    shader = sg.createComputeProgram(
                            shaderModel, targetApi, targetLanguage, featureLevel, info);
                }

                std::string* pGlsl = nullptr;
                if (targetApiNeedsGlsl) {
                    pGlsl = &shader;
                }

                GLSLPostProcessor::Config config{
                        .variant = v.variant,
                        .targetApi = targetApi,
                        .targetLanguage = targetLanguage,
                        .shaderType = v.stage,
                        .shaderModel = shaderModel,
                        .featureLevel = featureLevel,
                        .domain = mMaterialDomain,
                        .materialInfo = &info,
                        .hasFramebufferFetch = mEnableFramebufferFetch,
                        .usesClipDistance = v.variant.hasStereo() && info.stereoscopicType == StereoscopicType::INSTANCED,
                        .glsl = {},
                };

                if (mEnableFramebufferFetch) {
                    config.glsl.subpassInputToColorLocation.emplace_back(0, 0);
                }

                bool const ok = postProcessor.process(shader, config, pGlsl, pSpirv, pMsl);
                if (!ok) {
                    showErrorMessage(mMaterialName.c_str_safe(), v.variant, targetApi, v.stage,
                                     featureLevel, shader);
                    cancelJobs = true;
                    if (mPrintShaders) {
                        slog.e << shader << io::endl;
                    }
                    return;
                }

                if (targetApi == TargetApi::OPENGL) {
                    if (targetLanguage == TargetLanguage::SPIRV) {
                        ShaderGenerator::fixupExternalSamplers(shaderModel, shader, featureLevel,
                                info);
                    }
                }

                // NOTE: Everything below touches shared structures protected by a lock
                // NOTE: do not execute expensive work from here on!
                std::unique_lock<Mutex> const lock(entriesLock);

                // below we rely on casting ShaderStage to uint8_t
                static_assert(sizeof(filament::backend::ShaderStage) == 1);


                switch (targetApi) {
                    case TargetApi::ALL:
                        // should never happen
                        break;
                    case TargetApi::OPENGL:
                        glslEntry.stage = v.stage;
                        glslEntry.shader = shader;
                        if (featureLevel == FeatureLevel::FEATURE_LEVEL_0) {
                            essl1Entries.push_back(glslEntry);
                        } else {
                            glslEntries.push_back(glslEntry);
                        }
                        break;
                    case TargetApi::VULKAN:
                        assert(!spirv.empty());
                        spirvEntry.stage = v.stage;
                        spirvEntry.data = std::move(spirv);
                        spirvEntries.push_back(spirvEntry);
                        break;
                    case TargetApi::METAL:
                        assert(!spirv.empty());
                        assert(msl.length() > 0);
                        metalEntry.stage = v.stage;
                        metalEntry.shader = msl;
                        metalEntries.push_back(metalEntry);
                        break;
                }
            });

            // NOTE: We run the first job separately to work the lack of thread safety
            //       guarantees in glslang. This library performs unguarded global
            //       operations on first use.
            if (firstJob) {
                jobSystem.runAndWait(job);
                firstJob = false;
            } else {
                jobSystem.run(job);
            }
        }

        jobSystem.runAndWait(parent);
    }

    if (cancelJobs.load()) {
        return false;
    }

    // Sort the variants.
    auto compare = [](const auto& a, const auto& b) {
        static_assert(sizeof(decltype(a.variant.key)) == 1);
        static_assert(sizeof(decltype(b.variant.key)) == 1);
        const uint32_t akey = (uint32_t(a.shaderModel) << 16) | (uint32_t(a.variant.key) << 8) | uint32_t(a.stage);
        const uint32_t bkey = (uint32_t(b.shaderModel) << 16) | (uint32_t(b.variant.key) << 8) | uint32_t(b.stage);
        return akey < bkey;
    };
    std::sort(glslEntries.begin(), glslEntries.end(), compare);
    std::sort(essl1Entries.begin(), essl1Entries.end(), compare);
    std::sort(spirvEntries.begin(), spirvEntries.end(), compare);
    std::sort(metalEntries.begin(), metalEntries.end(), compare);

    // Generate the dictionaries.
    for (const auto& s : glslEntries) {
        textDictionary.addText(s.shader);
    }
    for (const auto& s : essl1Entries) {
        textDictionary.addText(s.shader);
    }
    for (auto& s : spirvEntries) {
        std::vector<uint32_t> spirv = std::move(s.data);
        s.dictionaryIndex = spirvDictionary.addBlob(spirv);
    }
    for (const auto& s : metalEntries) {
        textDictionary.addText(s.shader);
    }

    // Emit dictionary chunk (TextDictionaryReader and DictionaryTextChunk)
    const auto& dictionaryChunk = container.push<filamat::DictionaryTextChunk>(
            std::move(textDictionary), ChunkType::DictionaryText);

    // Emit GLSL chunk (MaterialTextChunk).
    if (!glslEntries.empty()) {
        container.push<MaterialTextChunk>(std::move(glslEntries),
                dictionaryChunk.getDictionary(), ChunkType::MaterialGlsl);
    }

    // Emit ESSL1 chunk (MaterialTextChunk).
    if (!essl1Entries.empty()) {
        container.push<MaterialTextChunk>(std::move(essl1Entries),
                dictionaryChunk.getDictionary(), ChunkType::MaterialEssl1);
    }

    // Emit SPIRV chunks (SpirvDictionaryReader and MaterialBinaryChunk).
    if (!spirvEntries.empty()) {
        const bool stripInfo = !mGenerateDebugInfo;
        container.push<filamat::DictionarySpirvChunk>(std::move(spirvDictionary), stripInfo);
        container.push<MaterialBinaryChunk>(std::move(spirvEntries), ChunkType::MaterialSpirv);
    }

    // Emit Metal chunk (MaterialTextChunk).
    if (!metalEntries.empty()) {
        container.push<MaterialTextChunk>(std::move(metalEntries),
                dictionaryChunk.getDictionary(), ChunkType::MaterialMetal);
    }

    return true;
}

MaterialBuilder& MaterialBuilder::output(VariableQualifier qualifier, OutputTarget target,
        Precision precision, OutputType type, const char* name, int location) noexcept {

    ASSERT_PRECONDITION(target != OutputTarget::DEPTH || type == OutputType::FLOAT,
            "Depth outputs must be of type FLOAT.");
    ASSERT_PRECONDITION(target != OutputTarget::DEPTH || qualifier == VariableQualifier::OUT,
            "Depth outputs must use OUT qualifier.");

    ASSERT_PRECONDITION(location >= -1,
            "Output location must be >= 0 (or use -1 for default location).");

    // A location value of -1 signals using the default location. We'll simply take the previous
    // output's location and add 1.
    if (location == -1) {
        location = mOutputs.empty() ? 0 : mOutputs.back().location + 1;
    }

    // Unconditionally add this output, then we'll check if we've maxed on on any particular target.
    mOutputs.emplace_back(name, qualifier, target, precision, type, location);

    uint8_t colorOutputCount = 0;
    uint8_t depthOutputCount = 0;
    for (const auto& output : mOutputs) {
        if (output.target == OutputTarget::COLOR) {
            colorOutputCount++;
        }
        if (output.target == OutputTarget::DEPTH) {
            depthOutputCount++;
        }
    }

    ASSERT_PRECONDITION(colorOutputCount <= MAX_COLOR_OUTPUT,
            "A maximum of %d COLOR outputs is allowed.", MAX_COLOR_OUTPUT);
    ASSERT_PRECONDITION(depthOutputCount <= MAX_DEPTH_OUTPUT,
            "A maximum of %d DEPTH output is allowed.", MAX_DEPTH_OUTPUT);

    assert(mOutputs.size() <= MAX_COLOR_OUTPUT + MAX_DEPTH_OUTPUT);

    return *this;
}

MaterialBuilder& MaterialBuilder::enableFramebufferFetch() noexcept {
    // This API is temporary, it is used to enable EXT_framebuffer_fetch for GLSL shaders,
    // this is used sparingly by filament's post-processing stage.
    mEnableFramebufferFetch = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::vertexDomainDeviceJittered(bool enabled) noexcept {
    mVertexDomainDeviceJittered = enabled;
    return *this;
}

MaterialBuilder& MaterialBuilder::useLegacyMorphing() noexcept {
    mUseLegacyMorphing = true;
    return *this;
}

Package MaterialBuilder::build(JobSystem& jobSystem) noexcept {
    bool success;
    if (materialBuilderClients == 0) {
        slog.e << "Error: MaterialBuilder::init() must be called before build()." << io::endl;
        // Return an empty package to signal a failure to build the material.
error:
        return Package::invalidPackage();
    }

    // Force post process materials to be unlit. This prevents imposing a lot of extraneous
    // data, code, and expectations for materials which do not need them.
    if (mMaterialDomain == MaterialDomain::POST_PROCESS) {
        mShading = Shading::UNLIT;
    }

    // Add a default color output.
    if (mMaterialDomain == MaterialDomain::POST_PROCESS && mOutputs.empty()) {
        output(VariableQualifier::OUT,
                OutputTarget::COLOR, Precision::DEFAULT, OutputType::FLOAT4, "color");
    }

    // TODO: maybe check MaterialDomain::COMPUTE has outputs

    // Resolve all the #include directives within user code.
    if (!mMaterialFragmentCode.resolveIncludes(mIncludeCallback, mFileName) ||
        !mMaterialVertexCode.resolveIncludes(mIncludeCallback, mFileName)) {
        goto error;
    }

    if (mCustomSurfaceShading && mShading != Shading::LIT) {
        slog.e << "Error: customSurfaceShading can only be used with lit materials." << io::endl;
        goto error;
    }

    // prepareToBuild must be called first, to populate mCodeGenPermutations.
    MaterialInfo info{};
    prepareToBuild(info);

    // check level features
    if (!checkMaterialLevelFeatures(info)) {
        goto error;
    }

    // Run checks, in order.
    // The call to findProperties populates mProperties and must come before runSemanticAnalysis.
    // Return an empty package to signal a failure to build the material.

    // For finding properties and running semantic analysis, we always use the same code gen
    // permutation. This is the first permutation generated with default arguments passed to matc.
    CodeGenParams const semanticCodeGenParams = {
            .shaderModel = ShaderModel::MOBILE,
            .targetApi = TargetApi::OPENGL,
            .targetLanguage = (info.featureLevel == FeatureLevel::FEATURE_LEVEL_0) ?
                              TargetLanguage::GLSL : TargetLanguage::SPIRV,
            .featureLevel = info.featureLevel,
    };

    if (!findAllProperties(semanticCodeGenParams)) {
        goto error;
    }

    if (!runSemanticAnalysis(&info, semanticCodeGenParams)) {
        goto error;
    }

    info.samplerBindings.init(mMaterialDomain, info.sib);

    // adjust variant-filter for feature level *before* we start writing into the container
    if (mFeatureLevel == filament::backend::FeatureLevel::FEATURE_LEVEL_0) {
        // at feature level 0, many variants are not supported
        mVariantFilter |= uint32_t(UserVariantFilterBit::DIRECTIONAL_LIGHTING);
        mVariantFilter |= uint32_t(UserVariantFilterBit::DYNAMIC_LIGHTING);
        mVariantFilter |= uint32_t(UserVariantFilterBit::SHADOW_RECEIVER);
        mVariantFilter |= uint32_t(UserVariantFilterBit::VSM);
        mVariantFilter |= uint32_t(UserVariantFilterBit::SSR);
    }

    // Create chunk tree.
    ChunkContainer container;
    writeCommonChunks(container, info);
    if (mMaterialDomain == MaterialDomain::SURFACE) {
        writeSurfaceChunks(container);
    }

    info.useLegacyMorphing = mUseLegacyMorphing;

    // Generate all shaders and write the shader chunks.

    std::vector<Variant> variants;
    switch (mMaterialDomain) {
        case MaterialDomain::SURFACE:
            variants = determineSurfaceVariants(mVariantFilter, isLit(), mShadowMultiplier);
            break;
        case MaterialDomain::POST_PROCESS:
            variants = determinePostProcessVariants();
            break;
        case MaterialDomain::COMPUTE:
            variants = determineComputeVariants();
            break;
    }

    success = generateShaders(jobSystem, variants, container, info);
    if (!success) {
        // Return an empty package to signal a failure to build the material.
        goto error;
    }

    // Flatten all chunks in the container into a Package.
    Package package(container.getSize());
    Flattener f{ package.getData() };
    container.flatten(f);

    return package;
}

using namespace backend;
static const char* to_string(ShaderStageFlags stageFlags) noexcept {
    switch (stageFlags) {
        case ShaderStageFlags::NONE:                    return "{ }";
        case ShaderStageFlags::VERTEX:                  return "{ vertex }";
        case ShaderStageFlags::FRAGMENT:                return "{ fragment }";
        case ShaderStageFlags::COMPUTE:                 return "{ compute }";
        case ShaderStageFlags::ALL_SHADER_STAGE_FLAGS:  return "{ vertex | fragment | COMPUTE }";
    }
    return nullptr;
}

bool MaterialBuilder::checkMaterialLevelFeatures(MaterialInfo const& info) const noexcept {

    auto logSamplerOverflow = [](SamplerInterfaceBlock const& sib) {
        auto const& samplers = sib.getSamplerInfoList();
        auto const* stage = to_string(sib.getStageFlags());
        for (auto const& sampler: samplers) {
            slog.e << "\"" << sampler.name.c_str() << "\" "
                   << Enums::toString(sampler.type).c_str() << " " << stage << '\n';
        }
        flush(slog.e);
    };

    auto userSamplerCount = info.sib.getSize();
    for (auto const& sampler: info.sib.getSamplerInfoList()) {
        if (sampler.type == SamplerInterfaceBlock::Type::SAMPLER_EXTERNAL) {
            userSamplerCount += 1;
        }
    }

    switch (info.featureLevel) {
        case FeatureLevel::FEATURE_LEVEL_0:
            // TODO: check FEATURE_LEVEL_0 features (e.g. unlit only, no texture arrays, etc...)
            if (info.isLit) {
                slog.e << "Error: material \"" << mMaterialName.c_str()
                       << "\" has feature level " << +info.featureLevel
                       << " and is not 'unlit'." << io::endl;
                return false;
            }
            return true;
        case FeatureLevel::FEATURE_LEVEL_1:
        case FeatureLevel::FEATURE_LEVEL_2: {
            if (mNoSamplerValidation) {
                break;
            }

            auto const maxTextureCount = backend::FEATURE_LEVEL_CAPS[1].MAX_FRAGMENT_SAMPLER_COUNT;

            // count how many samplers filament uses based on the material properties
            // note: currently SSAO is not used with unlit, but we want to keep that possibility.
            uint32_t textureUsedByFilamentCount = 4;    // shadowMap, structure, ssao, fog texture
            if (info.isLit) {
                textureUsedByFilamentCount += 3;        // froxels, dfg, specular
            }
            if (info.reflectionMode == ReflectionMode::SCREEN_SPACE ||
                info.refractionMode == RefractionMode::SCREEN_SPACE) {
                textureUsedByFilamentCount += 1;        // ssr
            }
            if (mVariantFilter & (uint32_t)UserVariantFilterBit::FOG) {
                textureUsedByFilamentCount -= 1;        // fog texture
            }

            // TODO: we need constants somewhere for these values
            if (userSamplerCount > maxTextureCount - textureUsedByFilamentCount) {
                slog.e << "Error: material \"" << mMaterialName.c_str()
                       << "\" has feature level " << +info.featureLevel
                       << " and is using more than " << maxTextureCount - textureUsedByFilamentCount
                       << " samplers." << io::endl;
                logSamplerOverflow(info.sib);
                return false;
            }
            auto const& samplerList = info.sib.getSamplerInfoList();
            using SamplerInfo = SamplerInterfaceBlock::SamplerInfo;
            if (std::any_of(samplerList.begin(), samplerList.end(),
                    [](const SamplerInfo& sampler) {
                        return sampler.type == SamplerType::SAMPLER_CUBEMAP_ARRAY;
                    })) {
                slog.e << "Error: material \"" << mMaterialName.c_str()
                       << "\" has feature level " << +info.featureLevel
                       << " and uses a samplerCubemapArray." << io::endl;
                logSamplerOverflow(info.sib);
                return false;
            }
            break;
        }
        case FeatureLevel::FEATURE_LEVEL_3: {
            // TODO: we need constants somewhere for these values
            // TODO: 16 is artificially low for now, until we have a better idea of what we want
            if (userSamplerCount > 16) {
                slog.e << "Error: material \"" << mMaterialName.c_str()
                       << "\" has feature level " << +info.featureLevel
                       << " and is using more than 16 samplers" << io::endl;
                logSamplerOverflow(info.sib);
                return false;
            }
            break;
        }
    }
    return true;
}

bool MaterialBuilder::hasCustomVaryings() const noexcept {
    for (const auto& variable : mVariables) {
        if (!variable.empty()) {
            return true;
        }
    }
    return false;
}

bool MaterialBuilder::needsStandardDepthProgram() const noexcept {
    const bool hasEmptyVertexCode = mMaterialVertexCode.getResolved().empty();
    return !hasEmptyVertexCode ||
           hasCustomVaryings() ||
           mBlendingMode == BlendingMode::MASKED ||
           (mTransparentShadow &&
            (mBlendingMode == BlendingMode::TRANSPARENT ||
             mBlendingMode == BlendingMode::FADE));
}

std::string MaterialBuilder::peek(backend::ShaderStage stage,
        const CodeGenParams& params, const PropertyList& properties) noexcept {

    ShaderGenerator sg(properties, mVariables, mOutputs, mDefines, mConstants,
            mMaterialFragmentCode.getResolved(), mMaterialFragmentCode.getLineOffset(),
            mMaterialVertexCode.getResolved(), mMaterialVertexCode.getLineOffset(),
            mMaterialDomain);

    MaterialInfo info;
    prepareToBuild(info);
    info.samplerBindings.init(mMaterialDomain, info.sib);

    switch (stage) {
        case backend::ShaderStage::VERTEX:
            return sg.createVertexProgram(
                    params.shaderModel, params.targetApi, params.targetLanguage,
                    params.featureLevel, info, {}, mInterpolation, mVertexDomain);
        case backend::ShaderStage::FRAGMENT:
            return sg.createFragmentProgram(
                    params.shaderModel, params.targetApi, params.targetLanguage,
                    params.featureLevel, info, {}, mInterpolation);
        case backend::ShaderStage::COMPUTE:
            return sg.createComputeProgram(
                    params.shaderModel, params.targetApi, params.targetLanguage,
                    params.featureLevel, info);
    }
}

static Program::UniformInfo extractUniforms(BufferInterfaceBlock const& uib) noexcept {
    auto list = uib.getFieldInfoList();
    Program::UniformInfo uniforms = Program::UniformInfo::with_capacity(list.size());

    char const firstLetter = std::tolower( uib.getName().at(0) );
    std::string_view const nameAfterFirstLetter{
        uib.getName().data() + 1, uib.getName().size() - 1 };

    for (auto const& item : list) {
        // construct the fully qualified name
        std::string qualified;
        qualified.reserve(uib.getName().size() + item.name.size() + 1u);
        qualified.append({ &firstLetter, 1u });
        qualified.append(nameAfterFirstLetter);
        qualified.append(".");
        qualified.append({ item.name.data(), item.name.size() });

        uniforms.push_back({
            { qualified.data(), qualified.size() },
            item.offset,
            uint8_t(item.size < 1u ? 1u : item.size),
            item.type
        });
    }
    return uniforms;
}

void MaterialBuilder::writeCommonChunks(ChunkContainer& container, MaterialInfo& info) const noexcept {
    container.emplace<uint32_t>(ChunkType::MaterialVersion, MATERIAL_VERSION);
    container.emplace<uint8_t>(ChunkType::MaterialFeatureLevel, (uint8_t)info.featureLevel);
    container.emplace<const char*>(ChunkType::MaterialName, mMaterialName.c_str_safe());
    container.emplace<uint32_t>(ChunkType::MaterialShaderModels, mShaderModels.getValue());
    container.emplace<uint8_t>(ChunkType::MaterialDomain, static_cast<uint8_t>(mMaterialDomain));

    // if that ever needed to change, this would require a material version bump
    static_assert(sizeof(uint32_t) >= sizeof(UserVariantFilterMask));

    container.emplace<uint32_t>(ChunkType::MaterialVariantFilterMask, mVariantFilter);

    using namespace filament;

    if (info.featureLevel == FeatureLevel::FEATURE_LEVEL_0) {
        FixedCapacityVector<std::pair<UniformBindingPoints, Program::UniformInfo>> list({
                { UniformBindingPoints::PER_VIEW,
                        extractUniforms(UibGenerator::getPerViewUib()) },
                { UniformBindingPoints::PER_RENDERABLE,
                        extractUniforms(UibGenerator::getPerRenderableUib()) },
                { UniformBindingPoints::PER_MATERIAL_INSTANCE,
                        extractUniforms(info.uib) },
        });

        // FIXME: don't hardcode this
        auto& uniforms = list[1].second;
        uniforms.clear();
        uniforms.reserve(6);
        uniforms.push_back({
                "objectUniforms.data[0].worldFromModelMatrix",
                offsetof(PerRenderableUib, data[0].worldFromModelMatrix), 1,
                UniformType::MAT4 });
        uniforms.push_back({
                "objectUniforms.data[0].worldFromModelNormalMatrix",
                offsetof(PerRenderableUib, data[0].worldFromModelNormalMatrix), 1,
                UniformType::MAT3 });
        uniforms.push_back({
                "objectUniforms.data[0].morphTargetCount",
                offsetof(PerRenderableUib, data[0].morphTargetCount), 1,
                UniformType::INT });
        uniforms.push_back({
                "objectUniforms.data[0].flagsChannels",
                offsetof(PerRenderableUib, data[0].flagsChannels), 1,
                UniformType::INT });
        uniforms.push_back({
                "objectUniforms.data[0].objectId",
                offsetof(PerRenderableUib, data[0].objectId), 1,
                UniformType::INT });
        uniforms.push_back({
                "objectUniforms.data[0].userData",
                offsetof(PerRenderableUib, data[0].userData), 1,
                UniformType::FLOAT });

        container.push<MaterialBindingUniformInfoChunk>(std::move(list));

        using Container = utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>>;
        auto attributes = Container::with_capacity(sAttributeDatabase.size());
        for (auto const& attribute: sAttributeDatabase) {
            std::string name("mesh_");
            name.append(attribute.name);
            attributes.emplace_back(utils::CString{ name.data(), name.size() }, attribute.location);
        }
        container.push<MaterialAttributesInfoChunk>(std::move(attributes));
    }

    // TODO: currently, the feature level used is determined by the material because we
    //       don't have "feature level" variants. In other words, a feature level 0 material
    //       won't work with a feature level 1 engine. However, we do embed the feature level 1
    //       meta-data, as it should.

    if (info.featureLevel <= FeatureLevel::FEATURE_LEVEL_1) {
        // note: this chunk is only needed for OpenGL backends, which don't all support layout(binding=)
        FixedCapacityVector<std::pair<std::string_view, UniformBindingPoints>> list = {
                { PerViewUib::_name,               UniformBindingPoints::PER_VIEW },
                { PerRenderableUib::_name,         UniformBindingPoints::PER_RENDERABLE },
                { LightsUib::_name,                UniformBindingPoints::LIGHTS },
                { ShadowUib::_name,                UniformBindingPoints::SHADOW },
                { FroxelRecordUib::_name,          UniformBindingPoints::FROXEL_RECORDS },
                { FroxelsUib::_name,               UniformBindingPoints::FROXELS },
                { PerRenderableBoneUib::_name,     UniformBindingPoints::PER_RENDERABLE_BONES },
                { PerRenderableMorphingUib::_name, UniformBindingPoints::PER_RENDERABLE_MORPHING },
                { info.uib.getName(),              UniformBindingPoints::PER_MATERIAL_INSTANCE }
        };
        container.push<MaterialUniformBlockBindingsChunk>(std::move(list));
    }

    // note: this chunk is needed for Vulkan and GL backends. Metal shouldn't need it (but
    // still does as of now).
    container.push<MaterialSamplerBlockBindingChunk>(info.samplerBindings);

    // User Material UIB
    container.push<MaterialUniformInterfaceBlockChunk>(info.uib);

    // User Material SIB
    container.push<MaterialSamplerInterfaceBlockChunk>(info.sib);

    // User constant parameters
    utils::FixedCapacityVector<MaterialConstant> constantsEntry(mConstants.size());
    std::transform(mConstants.begin(), mConstants.end(), constantsEntry.begin(),
            [](Constant const& c) { return MaterialConstant(c.name.c_str(), c.type); });
    container.push<MaterialConstantParametersChunk>(std::move(constantsEntry));

    // TODO: should we write the SSBO info? this would only be needed if we wanted to provide
    //       an interface to set [get?] values in the buffer. But we can do that easily
    //       with a c-struct (what about kotlin/java?). tbd.

    if (mMaterialDomain != MaterialDomain::COMPUTE) {
        // User Subpass
        container.push<MaterialSubpassInterfaceBlockChunk>(info.subpass);

        container.emplace<bool>(ChunkType::MaterialDoubleSidedSet, mDoubleSidedCapability);
        container.emplace<bool>(ChunkType::MaterialDoubleSided, mDoubleSided);
        container.emplace<uint8_t>(ChunkType::MaterialBlendingMode,
                static_cast<uint8_t>(mBlendingMode));

        if (mBlendingMode == BlendingMode::CUSTOM) {
            uint32_t const blendFunctions =
                    (uint32_t(mCustomBlendFunctions[0]) << 24) |
                    (uint32_t(mCustomBlendFunctions[1]) << 16) |
                    (uint32_t(mCustomBlendFunctions[2]) <<  8) |
                    (uint32_t(mCustomBlendFunctions[3]) <<  0);
            container.emplace< uint32_t >(ChunkType::MaterialBlendFunction, blendFunctions);
        }

        container.emplace<uint8_t>(ChunkType::MaterialTransparencyMode,
                static_cast<uint8_t>(mTransparencyMode));
        container.emplace<uint8_t>(ChunkType::MaterialReflectionMode,
                static_cast<uint8_t>(mReflectionMode));
        container.emplace<bool>(ChunkType::MaterialColorWrite, mColorWrite);
        container.emplace<bool>(ChunkType::MaterialDepthWriteSet, mDepthWriteSet);
        container.emplace<bool>(ChunkType::MaterialDepthWrite, mDepthWrite);
        container.emplace<bool>(ChunkType::MaterialDepthTest, mDepthTest);
        container.emplace<bool>(ChunkType::MaterialInstanced, mInstanced);
        container.emplace<bool>(ChunkType::MaterialAlphaToCoverageSet, mAlphaToCoverageSet);
        container.emplace<bool>(ChunkType::MaterialAlphaToCoverage, mAlphaToCoverage);
        container.emplace<uint8_t>(ChunkType::MaterialCullingMode,
                static_cast<uint8_t>(mCullingMode));

        uint64_t properties = 0;
        UTILS_NOUNROLL
        for (size_t i = 0; i < MATERIAL_PROPERTIES_COUNT; i++) {
            if (mProperties[i]) {
                properties |= uint64_t(1u) << i;
            }
        }
        container.emplace<uint64_t>(ChunkType::MaterialProperties, properties);
    }

    // create a unique material id
    auto const& vert = mMaterialVertexCode.getResolved();
    auto const& frag = mMaterialFragmentCode.getResolved();
    std::hash<std::string_view> const hasher;
    size_t const materialId = utils::hash::combine(
            MATERIAL_VERSION,
            utils::hash::combine(
                    hasher({ vert.data(), vert.size() }),
                    hasher({ frag.data(), frag.size() })));

    container.emplace<uint64_t>(ChunkType::MaterialCacheId, materialId);
}

void MaterialBuilder::writeSurfaceChunks(ChunkContainer& container) const noexcept {
    if (mBlendingMode == BlendingMode::MASKED) {
        container.emplace<float>(ChunkType::MaterialMaskThreshold, mMaskThreshold);
    }

    container.emplace<uint8_t>(ChunkType::MaterialShading, static_cast<uint8_t>(mShading));

    if (mShading == Shading::UNLIT) {
        container.emplace<bool>(ChunkType::MaterialShadowMultiplier, mShadowMultiplier);
    }

    container.emplace<uint8_t>(ChunkType::MaterialRefraction, static_cast<uint8_t>(mRefractionMode));
    container.emplace<uint8_t>(ChunkType::MaterialRefractionType,
            static_cast<uint8_t>(mRefractionType));
    container.emplace<bool>(ChunkType::MaterialClearCoatIorChange, mClearCoatIorChange);
    container.emplace<uint32_t>(ChunkType::MaterialRequiredAttributes,
            mRequiredAttributes.getValue());
    container.emplace<bool>(ChunkType::MaterialSpecularAntiAliasing, mSpecularAntiAliasing);
    container.emplace<float>(ChunkType::MaterialSpecularAntiAliasingVariance,
            mSpecularAntiAliasingVariance);
    container.emplace<float>(ChunkType::MaterialSpecularAntiAliasingThreshold,
            mSpecularAntiAliasingThreshold);
    container.emplace<uint8_t>(ChunkType::MaterialVertexDomain, static_cast<uint8_t>(mVertexDomain));
    container.emplace<uint8_t>(ChunkType::MaterialInterpolation,
            static_cast<uint8_t>(mInterpolation));
}

MaterialBuilder& MaterialBuilder::noSamplerValidation(bool enabled) noexcept {
    mNoSamplerValidation = enabled;
    return *this;
}

MaterialBuilder& MaterialBuilder::includeEssl1(bool enabled) noexcept {
    mIncludeEssl1 = enabled;
    return *this;
}

} // namespace filamat
