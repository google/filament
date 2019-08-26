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

#include <vector>

#include <utils/Panic.h>
#include <utils/Log.h>

#include <private/filament/UniformInterfaceBlock.h>
#include <private/filament/SamplerInterfaceBlock.h>

#include <private/filament/SibGenerator.h>

#include "MaterialVariants.h"

#include "shaders/MaterialInfo.h"
#include "shaders/ShaderGenerator.h"

#include "eiff/BlobDictionary.h"
#include "eiff/LineDictionary.h"
#include "eiff/MaterialInterfaceBlockChunk.h"
#include "eiff/MaterialTextChunk.h"
#include "eiff/MaterialSpirvChunk.h"
#include "eiff/ChunkContainer.h"
#include "eiff/SimpleFieldChunk.h"
#include "eiff/DictionaryTextChunk.h"
#include "eiff/DictionarySpirvChunk.h"

#include "Includes.h"

#ifndef FILAMAT_LITE
#include "GLSLPostProcessor.h"
#include "sca/GLSLTools.h"
#else
#include "sca/GLSLToolsLite.h"
#endif

using namespace utils;

namespace filamat {

std::atomic<int> MaterialBuilderBase::materialBuilderClients(0);

inline void assertSingleTargetApi(MaterialBuilderBase::TargetApi api) {
    // Assert that a single bit is set.
    UTILS_UNUSED uint8_t bits = (uint8_t) api;
    assert(bits && !(bits & bits - 1u));
}

void MaterialBuilderBase::prepare() {
    mCodeGenPermutations.clear();
    mShaderModels.reset();

    if (mPlatform == Platform::MOBILE) {
        mShaderModels.set(static_cast<size_t>(ShaderModel::GL_ES_30));
    } else if (mPlatform == Platform::DESKTOP) {
        mShaderModels.set(static_cast<size_t>(ShaderModel::GL_CORE_41));
    } else if (mPlatform == Platform::ALL) {
        mShaderModels.set(static_cast<size_t>(ShaderModel::GL_ES_30));
        mShaderModels.set(static_cast<size_t>(ShaderModel::GL_CORE_41));
    }

    // OpenGL is a special case. If we're doing any optimization, then we need to go to Spir-V.
    TargetLanguage glTargetLanguage = mOptimization > MaterialBuilder::Optimization::PREPROCESSOR ?
            TargetLanguage::SPIRV : TargetLanguage::GLSL;

    // Select OpenGL as the default TargetApi if none was specified.
    if (mTargetApi == (TargetApi) 0) {
        mTargetApi = TargetApi::OPENGL;
    }

    // Build a list of codegen permutations, which is useful across all types of material builders.
    // The shader model loop starts at 1 to skip ShaderModel::UNKNOWN.
    for (uint8_t i = 1; i < filament::backend::SHADER_MODEL_COUNT; i++) {
        if (!mShaderModels.test(i)) {
            continue; // skip this shader model since it was not requested.
        }
        if (mTargetApi & TargetApi::OPENGL) {
            mCodeGenPermutations.push_back({i, TargetApi::OPENGL, glTargetLanguage});
        }
        if (mTargetApi & TargetApi::VULKAN) {
            mCodeGenPermutations.push_back({i, TargetApi::VULKAN, TargetLanguage::SPIRV});
        }
        if (mTargetApi & TargetApi::METAL) {
            mCodeGenPermutations.push_back({i, TargetApi::METAL, TargetLanguage::SPIRV});
        }
    }
}

MaterialBuilder::MaterialBuilder() : mMaterialName("Unnamed") {
    std::fill_n(mProperties, MATERIAL_PROPERTIES_COUNT, false);
    mShaderModels.reset();
}

void MaterialBuilderBase::init() {
    materialBuilderClients++;
#ifndef FILAMAT_LITE
    GLSLTools::init();
#endif
}

void MaterialBuilderBase::shutdown() {
    materialBuilderClients--;
#ifndef FILAMAT_LITE
    GLSLTools::shutdown();
#endif
}

MaterialBuilder& MaterialBuilder::name(const char* name) noexcept {
    mMaterialName = CString(name);
    return *this;
}

MaterialBuilder& MaterialBuilder::material(const char* code, size_t line) noexcept {
    mMaterialCode.setUnresolved(CString(code));
    mMaterialCode.setLineOffset(line);
    return *this;
}

MaterialBuilder& MaterialBuilder::includeCallback(IncludeCallback callback) noexcept {
    mIncludeCallback = callback;
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

MaterialBuilder& MaterialBuilder::parameter(UniformType type, const char* name) noexcept {
    ASSERT_POSTCONDITION(mParameterCount < MAX_PARAMETERS_COUNT, "Too many parameters");
    mParameters[mParameterCount++] = { name, type, 1 };
    return *this;
}

MaterialBuilder& MaterialBuilder::parameter(UniformType type, size_t size, const char* name) noexcept {
    ASSERT_POSTCONDITION(mParameterCount < MAX_PARAMETERS_COUNT, "Too many parameters");
    mParameters[mParameterCount++] = { name, type, size };
    return *this;
}

MaterialBuilder& MaterialBuilder::parameter(
        SamplerType samplerType, SamplerFormat format, SamplerPrecision precision, const char* name) noexcept {
    ASSERT_POSTCONDITION(mParameterCount < MAX_PARAMETERS_COUNT, "Too many parameters");
    mParameters[mParameterCount++] = { name, samplerType, format, precision };
    return *this;
}

MaterialBuilder& MaterialBuilder::parameter(
        SamplerType samplerType, SamplerFormat format, const char* name) noexcept {
    return parameter(samplerType, format, SamplerPrecision::DEFAULT, name);
}

MaterialBuilder& MaterialBuilder::parameter(
        SamplerType samplerType, SamplerPrecision precision, const char* name) noexcept {
    return parameter(samplerType, SamplerFormat::FLOAT, precision, name);
}

MaterialBuilder& MaterialBuilder::parameter(
        SamplerType samplerType, const char* name) noexcept {
    return parameter(samplerType, SamplerFormat::FLOAT, SamplerPrecision::DEFAULT, name);
}

MaterialBuilder& MaterialBuilder::require(filament::VertexAttribute attribute) noexcept {
    mRequiredAttributes.set(attribute);
    return *this;
}

MaterialBuilder& MaterialBuilder::materialDomain(MaterialDomain materialDomain) noexcept {
    mMaterialDomain = materialDomain;
    return *this;
}

MaterialBuilder& MaterialBuilder::blending(BlendingMode blending) noexcept {
    mBlendingMode = blending;
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

MaterialBuilder& MaterialBuilder::doubleSided(bool doubleSided) noexcept {
    mDoubleSided = doubleSided;
    mDoubleSidedCapability = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::maskThreshold(float threshold) noexcept {
    mMaskThreshold = threshold;
    return *this;
}

MaterialBuilder& MaterialBuilder::shadowMultiplier(bool shadowMultiplier) noexcept {
    mShadowMultiplier = shadowMultiplier;
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

MaterialBuilder& MaterialBuilder::multiBounceAmbientOcclusion(bool multiBounceAO) noexcept {
    mMultiBounceAO = multiBounceAO;
    mMultiBounceAOSet = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::specularAmbientOcclusion(bool specularAO) noexcept {
    mSpecularAO = specularAO;
    mSpecularAOSet = true;
    return *this;
}

MaterialBuilder& MaterialBuilder::transparencyMode(TransparencyMode mode) noexcept {
    mTransparencyMode = mode;
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

MaterialBuilder& MaterialBuilder::variantFilter(uint8_t variantFilter) noexcept {
    mVariantFilter = variantFilter;
    return *this;
}

bool MaterialBuilder::hasExternalSampler() const noexcept {
    for (size_t i = 0, c = mParameterCount; i < c; i++) {
        auto const& param = mParameters[i];
        if (param.isSampler && param.samplerType == SamplerType::SAMPLER_EXTERNAL) {
            return  true;
        }
    }
    return false;
}

void MaterialBuilder::prepareToBuild(MaterialInfo& info) noexcept {
    MaterialBuilderBase::prepare();

    // Build the per-material sampler block and uniform block.
    filament::SamplerInterfaceBlock::Builder sbb;
    filament::UniformInterfaceBlock::Builder ibb;
    for (size_t i = 0, c = mParameterCount; i < c; i++) {
        auto const& param = mParameters[i];
        if (param.isSampler) {
            sbb.add(param.name, param.samplerType, param.samplerFormat, param.samplerPrecision);
        } else {
            ibb.add(param.name, param.size, param.uniformType);
        }
    }

    if (mSpecularAntiAliasing) {
        ibb.add("_specularAntiAliasingVariance", 1, UniformType::FLOAT);
        ibb.add("_specularAntiAliasingThreshold", 1, UniformType::FLOAT);
    }

    if (mBlendingMode == BlendingMode::MASKED) {
        ibb.add("_maskThreshold", 1, UniformType::FLOAT);
    }

    if (mDoubleSidedCapability) {
        ibb.add("_doubleSided", 1, UniformType::BOOL);
    }

    mRequiredAttributes.set(filament::VertexAttribute::POSITION);
    if (mShading != filament::Shading::UNLIT || mShadowMultiplier) {
        mRequiredAttributes.set(filament::VertexAttribute::TANGENTS);
    }

    info.sib = sbb.name("MaterialParams").build();
    info.uib = ibb.name("MaterialParams").build();

    info.isLit = isLit();
    info.hasDoubleSidedCapability = mDoubleSidedCapability;
    info.hasExternalSamplers = hasExternalSampler();
    info.specularAntiAliasing = mSpecularAntiAliasing;
    info.clearCoatIorChange = mClearCoatIorChange;
    info.flipUV = mFlipUV;
    info.requiredAttributes = mRequiredAttributes;
    info.blendingMode = mBlendingMode;
    info.postLightingBlendingMode = mPostLightingBlendingMode;
    info.shading = mShading;
    info.hasShadowMultiplier = mShadowMultiplier;
    info.multiBounceAO = mMultiBounceAO;
    info.multiBounceAOSet = mMultiBounceAOSet;
    info.specularAO = mSpecularAO;
    info.specularAOSet = mSpecularAOSet;
}

bool MaterialBuilder::findProperties() noexcept {
    if (mMaterialDomain != MaterialDomain::SURFACE) {
        return true;
    }

#ifndef FILAMAT_LITE
    using namespace filament::backend;
    GLSLTools glslTools;

    // Some fields in MaterialInputs only exist if the property is set (e.g: normal, subsurface
    // for cloth shading model). Give our shader all properties. This will enable us to parse and
    // static code analyse the AST.
    MaterialBuilder::PropertyList allProperties;
    std::fill_n(allProperties, MATERIAL_PROPERTIES_COUNT, true);

    // Use the first permutation to generate the shader code.
    assert(!mCodeGenPermutations.empty());
    CodeGenParams params = mCodeGenPermutations[0];
    std::string shaderCodeAllProperties = peek(ShaderType::FRAGMENT, params, allProperties);

    // Populate mProperties with the properties set in the shader.
    if (!glslTools.findProperties(shaderCodeAllProperties, mProperties, params.targetApi,
                ShaderModel(params.shaderModel))) {
        return false;
    }

    return true;
#else
    GLSLToolsLite glslTools;
    return glslTools.findProperties(mMaterialCode.getResolved(), mProperties);
#endif
}

bool MaterialBuilder::runSemanticAnalysis() noexcept {
#ifndef FILAMAT_LITE
    using namespace filament::backend;
    GLSLTools glslTools;

    // Use the first permutation to generate the shader code.
    assert(!mCodeGenPermutations.empty());
    CodeGenParams params = mCodeGenPermutations[0];

    TargetApi targetApi = params.targetApi;
    assertSingleTargetApi(targetApi);
    ShaderModel model = static_cast<ShaderModel>(params.shaderModel);

    std::string shaderCode = peek(ShaderType::VERTEX, params, mProperties);
    bool result = glslTools.analyzeVertexShader(shaderCode, model, mMaterialDomain, targetApi);
    if (!result) return false;

    shaderCode = peek(ShaderType::FRAGMENT, params, mProperties);
    result = glslTools.analyzeFragmentShader(shaderCode, model, mMaterialDomain, targetApi);
    return result;
#else
    return true;
#endif
}

bool MaterialBuilder::checkLiteRequirements() noexcept {
#ifdef FILAMAT_LITE
    if (mTargetApi != TargetApi::OPENGL) {
        utils::slog.e
                << "Filamat lite only supports building materials for the OpenGL backend."
                << utils::io::endl;
        return false;
    }

    if (mOptimization != Optimization::NONE) {
        utils::slog.e
                << "Filamat lite does not support material optimization." << utils::io::endl
                << "Ensure optimization is set to NONE." << utils::io::endl;
        return false;
    }
#endif
    return true;
}

bool MaterialBuilder::ShaderCode::resolveIncludes(IncludeCallback callback) noexcept {
    if (!mCode.empty()) {
        if (!::filamat::resolveIncludes(utils::CString(""), mCode, callback)) {
            return false;
        }
    }

    mIncludesResolved = true;
    return true;
}

static void showErrorMessage(const char* materialName, uint8_t variant,
        MaterialBuilder::TargetApi targetApi, filament::backend::ShaderType shaderType,
        const std::string& shaderCode) {
    using ShaderType = filament::backend::ShaderType;
    using TargetApi = MaterialBuilder::TargetApi;
    utils::slog.e
            << "Error in \"" << materialName << "\""
            << ", Variant 0x" << io::hex << (int) variant
            << (targetApi == TargetApi::VULKAN ? ", Vulkan.\n" : ", OpenGL.\n")
            << "=========================\n"
            << "Generated "
            << (shaderType == ShaderType::VERTEX ? "Vertex Shader\n" : "Fragment Shader\n")
            << "=========================\n"
            << shaderCode;
}

bool MaterialBuilder::generateShaders(const std::vector<Variant>& variants, ChunkContainer& container,
        const MaterialInfo& info) const noexcept {
    // Create a postprocessor to optimize / compile to Spir-V if necessary.
#ifndef FILAMAT_LITE
    uint32_t flags = 0;
    flags |= mPrintShaders ? GLSLPostProcessor::PRINT_SHADERS : 0;
    flags |= mGenerateDebugInfo ? GLSLPostProcessor::GENERATE_DEBUG_INFO : 0;
    GLSLPostProcessor postProcessor(mOptimization, flags);
#endif

    // Generate all shaders.
    std::vector<TextEntry> glslEntries;
    std::vector<SpirvEntry> spirvEntries;
    std::vector<TextEntry> metalEntries;
    LineDictionary glslDictionary;
#ifndef FILAMAT_LITE
    BlobDictionary spirvDictionary;
    LineDictionary metalDictionary;
#endif
    std::vector<uint32_t> spirv;
    std::string msl;

    ShaderGenerator sg(mProperties, mVariables, mMaterialCode.getResolved(),
            mMaterialCode.getLineOffset(), mMaterialVertexCode.getResolved(),
            mMaterialVertexCode.getLineOffset(), mMaterialDomain);

    bool emptyVertexCode = mMaterialVertexCode.getResolved().empty();
    bool customDepth = sg.hasCustomDepthShader() ||
            mBlendingMode == BlendingMode::MASKED || !emptyVertexCode;
    container.addSimpleChild<bool>(ChunkType::MaterialHasCustomDepthShader, customDepth);

    for (const auto& params : mCodeGenPermutations) {
        const ShaderModel shaderModel = ShaderModel(params.shaderModel);
        const TargetApi targetApi = params.targetApi;
        const TargetLanguage targetLanguage = params.targetLanguage;

        assertSingleTargetApi(targetApi);

        // Metal Shading Language is cross-compiled from Vulkan.
        const bool targetApiNeedsSpirv =
                (targetApi == TargetApi::VULKAN || targetApi == TargetApi::METAL);
        const bool targetApiNeedsMsl = targetApi == TargetApi::METAL;
        std::vector<uint32_t>* pSpirv = targetApiNeedsSpirv ? &spirv : nullptr;
        std::string* pMsl = targetApiNeedsMsl ? &msl : nullptr;

        TextEntry glslEntry{0};
        SpirvEntry spirvEntry{0};
        TextEntry metalEntry{0};

        glslEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);
        spirvEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);
        metalEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);

        for (const auto& v : variants) {
            glslEntry.variant = v.variant;
            spirvEntry.variant = v.variant;
            metalEntry.variant = v.variant;

            // Generate raw shader code.
            std::string shader;
            if (v.stage == filament::backend::ShaderType::VERTEX) {
                shader = sg.createVertexProgram(
                        shaderModel, targetApi, targetLanguage, info, v.variant,
                        mInterpolation, mVertexDomain);
            } else if (v.stage == filament::backend::ShaderType::FRAGMENT) {
                shader = sg.createFragmentProgram(
                        shaderModel, targetApi, targetLanguage, info, v.variant, mInterpolation);
            }

#ifndef FILAMAT_LITE
            bool ok = postProcessor.process(shader, v.stage, shaderModel, &shader, pSpirv, pMsl);
#else
            bool ok = true;
#endif
            if (!ok) {
                showErrorMessage(mMaterialName.c_str_safe(), v.variant, targetApi, v.stage, shader);
                return false;
            }

            if (targetApi == TargetApi::OPENGL) {
                if (targetLanguage == TargetLanguage::SPIRV) {
                    sg.fixupExternalSamplers(shaderModel, shader, info);
                }

                glslEntry.stage = v.stage;
                glslEntry.shader = shader;
                glslDictionary.addText(glslEntry.shader);
                glslEntries.push_back(glslEntry);
            }

#ifndef FILAMAT_LITE
            if (targetApi == TargetApi::VULKAN) {
                assert(!spirv.empty());
                spirvEntry.stage = v.stage;
                spirvEntry.dictionaryIndex = spirvDictionary.addBlob(spirv);
                spirv.clear();
                spirvEntries.push_back(spirvEntry);
            }
            if (targetApi == TargetApi::METAL) {
                assert(!spirv.empty());
                assert(msl.length() > 0);
                metalEntry.stage = v.stage;
                metalEntry.shader = msl;
                spirv.clear();
                msl.clear();
                metalDictionary.addText(metalEntry.shader);
                metalEntries.push_back(metalEntry);
            }
#endif
        }
    }

    // Emit GLSL chunks (TextDictionaryReader and MaterialTextChunk).
    if (!glslEntries.empty()) {
        const auto& dictionaryChunk = container.addChild<filamat::DictionaryTextChunk>(
                std::move(glslDictionary), ChunkType::DictionaryGlsl);
        container.addChild<MaterialTextChunk>(std::move(glslEntries),
                dictionaryChunk.getDictionary(), ChunkType::MaterialGlsl);
    }

    // Emit SPIRV chunks (SpirvDictionaryReader and MaterialSpirvChunk).
#ifndef FILAMAT_LITE
    if (!spirvEntries.empty()) {
        const bool stripInfo = !mGenerateDebugInfo;
        container.addChild<filamat::DictionarySpirvChunk>(std::move(spirvDictionary), stripInfo);
        container.addChild<MaterialSpirvChunk>(std::move(spirvEntries));
    }

    // Emit Metal chunks (MetalDictionaryReader and MaterialMetalChunk).
    if (!metalEntries.empty()) {
        const auto& dictionaryChunk = container.addChild<filamat::DictionaryTextChunk>(
                std::move(metalDictionary), ChunkType::DictionaryMetal);
        container.addChild<MaterialTextChunk>(std::move(metalEntries),
                dictionaryChunk.getDictionary(), ChunkType::MaterialMetal);
    }
#endif

    return true;
}

Package MaterialBuilder::build() noexcept {
    if (materialBuilderClients == 0) {
        utils::slog.e << "Error: MaterialBuilder::init() must be called before build()."
            << utils::io::endl;
        // Return an empty package to signal a failure to build the material.
        return Package::invalidPackage();
    }

    // Resolve all the #include directives within user code.
    if (!mMaterialCode.resolveIncludes(mIncludeCallback) ||
        !mMaterialVertexCode.resolveIncludes(mIncludeCallback)) {
        return Package::invalidPackage();
    }

    // prepareToBuild must be called first, to populate mCodeGenPermutations.
    MaterialInfo info;
    prepareToBuild(info);

    // Run checks, in order.
    // The call to findProperties populates mProperties and must come before runSemanticAnalysis.
    if (!checkLiteRequirements() ||
        !findProperties() ||
        !runSemanticAnalysis()) {
        // Return an empty package to signal a failure to build the material.
        return Package::invalidPackage();
    }

    filament::SamplerBindingMap map;
    map.populate(&info.sib, mMaterialName.c_str());
    info.samplerBindings = std::move(map);

    // Create chunk tree.
    ChunkContainer container;
    writeCommonChunks(container, info);
    if (mMaterialDomain == MaterialDomain::SURFACE) {
        writeSurfaceChunks(container);
    }

    // Generate all shaders and write the shader chunks.
    const auto variants = mMaterialDomain == MaterialDomain::SURFACE ?
        determineSurfaceVariants(mVariantFilter, isLit(), mShadowMultiplier) :
        determinePostProcessVariants();
    bool success = generateShaders(variants, container, info);

    // Flatten all chunks in the container into a Package.
    Package package(container.getSize());
    Flattener f(package);
    container.flatten(f);
    package.setValid(success);

    return package;
}

const std::string MaterialBuilder::peek(filament::backend::ShaderType type,
        const CodeGenParams& params, const PropertyList& properties) noexcept {
    ShaderGenerator sg(properties, mVariables, mMaterialCode.getResolved(),
            mMaterialCode.getLineOffset(), mMaterialVertexCode.getResolved(),
            mMaterialVertexCode.getLineOffset(), mMaterialDomain);

    MaterialInfo info;
    prepareToBuild(info);

    filament::SamplerBindingMap map;
    map.populate(&info.sib, mMaterialName.c_str());
    info.samplerBindings = std::move(map);

    if (type == filament::backend::ShaderType::VERTEX) {
        return sg.createVertexProgram(ShaderModel(params.shaderModel),
                params.targetApi, params.targetLanguage, info, 0, mInterpolation, mVertexDomain);
    } else {
        return sg.createFragmentProgram(ShaderModel(params.shaderModel), params.targetApi,
                params.targetLanguage, info, 0, mInterpolation);
    }

    return std::string("");
}

void MaterialBuilder::writeCommonChunks(ChunkContainer& container, MaterialInfo& info) const noexcept {
    container.addSimpleChild<uint32_t>(ChunkType::MaterialVersion, filament::MATERIAL_VERSION);
    container.addSimpleChild<const char*>(ChunkType::MaterialName, mMaterialName.c_str_safe());
    container.addSimpleChild<uint32_t>(ChunkType::MaterialShaderModels, mShaderModels.getValue());
    container.addSimpleChild<uint8_t>(ChunkType::MaterialDomain, static_cast<uint8_t>(mMaterialDomain));

    // UIB
    container.addChild<MaterialUniformInterfaceBlockChunk>(info.uib);

    // SIB
    container.addChild<MaterialSamplerInterfaceBlockChunk>(info.sib);

    container.addSimpleChild<bool>(ChunkType::MaterialDoubleSidedSet, mDoubleSidedCapability);
    container.addSimpleChild<bool>(ChunkType::MaterialDoubleSided, mDoubleSided);

    container.addSimpleChild<uint8_t>(ChunkType::MaterialBlendingMode, static_cast<uint8_t>(mBlendingMode));
    container.addSimpleChild<uint8_t>(ChunkType::MaterialTransparencyMode, static_cast<uint8_t>(mTransparencyMode));
    container.addSimpleChild<bool>(ChunkType::MaterialDepthWriteSet, mDepthWriteSet);
    container.addSimpleChild<bool>(ChunkType::MaterialColorWrite, mColorWrite);
    container.addSimpleChild<bool>(ChunkType::MaterialDepthWrite, mDepthWrite);
    container.addSimpleChild<bool>(ChunkType::MaterialDepthTest, mDepthTest);
    container.addSimpleChild<uint8_t>(ChunkType::MaterialCullingMode, static_cast<uint8_t>(mCullingMode));
}

void MaterialBuilder::writeSurfaceChunks(ChunkContainer& container) const noexcept {
    if (mBlendingMode == BlendingMode::MASKED) {
        container.addSimpleChild<float>(ChunkType::MaterialMaskThreshold, mMaskThreshold);
    }

    container.addSimpleChild<uint8_t>(ChunkType::MaterialShading, static_cast<uint8_t>(mShading));

    if (mShading == Shading::UNLIT) {
        container.addSimpleChild<bool>(ChunkType::MaterialShadowMultiplier, mShadowMultiplier);
    }

    container.addSimpleChild<bool>(ChunkType::MaterialClearCoatIorChange, mClearCoatIorChange);
    container.addSimpleChild<uint32_t>(ChunkType::MaterialRequiredAttributes, mRequiredAttributes.getValue());
    container.addSimpleChild<bool>(ChunkType::MaterialSpecularAntiAliasing, mSpecularAntiAliasing);
    container.addSimpleChild<float>(ChunkType::MaterialSpecularAntiAliasingVariance, mSpecularAntiAliasingVariance);
    container.addSimpleChild<float>(ChunkType::MaterialSpecularAntiAliasingThreshold, mSpecularAntiAliasingThreshold);
    container.addSimpleChild<uint8_t>(ChunkType::MaterialVertexDomain, static_cast<uint8_t>(mVertexDomain));
    container.addSimpleChild<uint8_t>(ChunkType::MaterialInterpolation, static_cast<uint8_t>(mInterpolation));
}

} // namespace filamat
