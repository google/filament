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
#include <private/filament/Variant.h>

#include "GLSLPostProcessor.h"

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

#include "sca/GLSLTools.h"

using namespace utils;

namespace filamat {

std::atomic<int> MaterialBuilderBase::materialBuilderClients(0);

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

    // Build a list of codegen permutations, which is useful across all types of material builders.
    // The shader model loop starts at 1 to skip ShaderModel::UNKNOWN.
    for (uint8_t i = 1; i < filament::backend::SHADER_MODEL_COUNT; i++) {
        if (!mShaderModels.test(i)) {
            continue; // skip this shader model since it was not requested.
        }
        switch (mTargetApi) {
            case TargetApi::ALL:
                mCodeGenPermutations.push_back({i, TargetApi::OPENGL, glTargetLanguage});
                mCodeGenPermutations.push_back({i, TargetApi::VULKAN, TargetLanguage::SPIRV});
                mCodeGenPermutations.push_back({i, TargetApi::METAL, TargetLanguage::SPIRV});
                break;
            case TargetApi::OPENGL:
                mCodeGenPermutations.push_back({i, TargetApi::OPENGL, glTargetLanguage});
                break;
            case TargetApi::VULKAN:
                mCodeGenPermutations.push_back({i, TargetApi::VULKAN, TargetLanguage::SPIRV});
            case TargetApi::METAL:
                mCodeGenPermutations.push_back({i, TargetApi::METAL, TargetLanguage::SPIRV});
                break;
        }
    }
}

MaterialBuilder::MaterialBuilder() : mMaterialName("Unnamed") {
    std::fill_n(mProperties, MATERIAL_PROPERTIES_COUNT, false);
    mShaderModels.reset();
}

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

MaterialBuilder& MaterialBuilder::material(const char* code, size_t line) noexcept {
    mMaterialCode = CString(code);
    mMaterialLineOffset = line;
    return *this;
}

MaterialBuilder& MaterialBuilder::materialVertex(const char* code, size_t line) noexcept {
    mMaterialVertexCode = CString(code);
    mMaterialVertexLineOffset = line;
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

MaterialBuilder& MaterialBuilder::curvatureToRoughness(bool curvatureToRoughness) noexcept {
    mCurvatureToRoughness = curvatureToRoughness;
    return *this;
}

MaterialBuilder& MaterialBuilder::limitOverInterpolation(bool limitOverInterpolation) noexcept {
    mLimitOverInterpolation = limitOverInterpolation;
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

MaterialBuilder& MaterialBuilder::transparencyMode(TransparencyMode mode) noexcept {
    mTransparencyMode = mode;
    return *this;
}

MaterialBuilder& MaterialBuilder::platform(Platform platform) noexcept {
    mPlatform = platform;
    return *this;
}

MaterialBuilder& MaterialBuilder::targetApi(TargetApi targetApi) noexcept {
    mTargetApi = targetApi;
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
    info.curvatureToRoughness = mCurvatureToRoughness;
    info.limitOverInterpolation = mLimitOverInterpolation;
    info.clearCoatIorChange = mClearCoatIorChange;
    info.flipUV = mFlipUV;
    info.requiredAttributes = mRequiredAttributes;
    info.blendingMode = mBlendingMode;
    info.postLightingBlendingMode = mPostLightingBlendingMode;
    info.shading = mShading;
    info.hasShadowMultiplier = mShadowMultiplier;
}

bool MaterialBuilder::runStaticCodeAnalysis() noexcept {
    using namespace filament::backend;

    GLSLTools glslTools;

    // Populate mProperties with the properties set in the shader.
    if (!glslTools.findProperties(*this, mProperties, mTargetApi)) {
        return false;
    }

    // At this point the shader is syntactically correct. Perform semantic analysis now.
    ShaderModel model;

    std::string shaderCode = peek(ShaderType::VERTEX, model, mProperties);
    bool result = glslTools.analyzeVertexShader(shaderCode, model, mTargetApi);
    if (!result) return false;

    shaderCode = peek(ShaderType::FRAGMENT, model, mProperties);
    result = glslTools.analyzeFragmentShader(shaderCode, model, mTargetApi);
    return result;
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

Package MaterialBuilder::build() noexcept {
    if (materialBuilderClients == 0) {
        utils::slog.e << "Error: MaterialBuilder::init() must be called before build()."
            << utils::io::endl;
        // Return an empty package to signal a failure to build the material.
        Package package(0);
        package.setValid(false);
        return package;
    }

    if (!runStaticCodeAnalysis()) {
        // Return an empty package to signal a failure to build the material.
        Package package(0);
        package.setValid(false);
        return package;
    }

    bool errorOccured = false;

    MaterialInfo info;
    prepareToBuild(info);

    // Create a postprocessor to optimize / compile to Spir-V if necessary.
    GLSLPostProcessor postProcessor(mOptimization, mPrintShaders);

    // Create chunk tree.
    ChunkContainer container;

    SimpleFieldChunk<uint32_t> matVersion(ChunkType::MaterialVersion, filament::MATERIAL_VERSION);
    container.addChild(&matVersion);

    SimpleFieldChunk<const char*> matName(ChunkType::MaterialName, mMaterialName.c_str_safe());
    container.addChild(&matName);

    SimpleFieldChunk<uint8_t> matShading(ChunkType::MaterialShading, static_cast<uint8_t>(mShading));
    container.addChild(&matShading);

    SimpleFieldChunk<uint8_t> matBlendingMode(ChunkType::MaterialBlendingMode,
            static_cast<uint8_t>(mBlendingMode));
    container.addChild(&matBlendingMode);

    SimpleFieldChunk<float> matMaskThreshold(ChunkType::MaterialMaskThreshold, mMaskThreshold);
    if (mBlendingMode == BlendingMode::MASKED) {
        container.addChild(&matMaskThreshold);
    }

    SimpleFieldChunk<bool> matShadowMultiplier(ChunkType::MaterialShadowMultiplier, mShadowMultiplier);
    if (mShading == Shading::UNLIT) {
        container.addChild(&matShadowMultiplier);
    }

    SimpleFieldChunk<bool> matCurvatureToRoughness(
            ChunkType::MaterialCurvatureToRoughness, mCurvatureToRoughness);
    container.addChild(&matCurvatureToRoughness);

    SimpleFieldChunk<bool> matLimitOverInterpolation(
            ChunkType::MaterialLimitOverInterpolation, mLimitOverInterpolation);
    container.addChild(&matLimitOverInterpolation);

    SimpleFieldChunk<bool> matClearCoatIorChange(
            ChunkType::MaterialClearCoatIorChange, mClearCoatIorChange);
    container.addChild(&matClearCoatIorChange);

    SimpleFieldChunk<uint8_t> matTransparency(ChunkType::MaterialTransparencyMode,
            static_cast<uint8_t>(mTransparencyMode));
    container.addChild(&matTransparency);

    SimpleFieldChunk<uint32_t> matRequiredAttributes(ChunkType::MaterialRequiredAttributes,
            mRequiredAttributes.getValue());
    container.addChild(&matRequiredAttributes);

    // UIB
    MaterialUniformInterfaceBlockChunk matUib = MaterialUniformInterfaceBlockChunk(info.uib);
    container.addChild(&matUib);

    // SIB
    MaterialSamplerInterfaceBlockChunk matSib = MaterialSamplerInterfaceBlockChunk(info.sib);
    container.addChild(&matSib);

    SimpleFieldChunk<bool> matDepthWriteSet(ChunkType::MaterialDepthWriteSet, mDepthWriteSet);
    container.addChild(&matDepthWriteSet);

    SimpleFieldChunk<bool> matDoubleSidedSet(ChunkType::MaterialDoubleSidedSet, mDoubleSidedCapability);
    container.addChild(&matDoubleSidedSet);

    SimpleFieldChunk<bool> matDoubleSided(ChunkType::MaterialDoubleSided, mDoubleSided);
    container.addChild(&matDoubleSided);

    SimpleFieldChunk<bool> matColorWrite(ChunkType::MaterialColorWrite, mColorWrite);
    container.addChild(&matColorWrite);

    SimpleFieldChunk<bool> matDepthWrite(ChunkType::MaterialDepthWrite, mDepthWrite);
    container.addChild(&matDepthWrite);

    SimpleFieldChunk<bool> matDepthTest(ChunkType::MaterialDepthTest, mDepthTest);
    container.addChild(&matDepthTest);

    SimpleFieldChunk<uint8_t> matCullingMode(ChunkType::MaterialCullingMode,
            static_cast<uint8_t>(mCullingMode));
    container.addChild(&matCullingMode);

    SimpleFieldChunk<uint8_t> matVertexDomain(ChunkType::MaterialVertexDomain,
            static_cast<uint8_t>(mVertexDomain));
    container.addChild(&matVertexDomain);

    SimpleFieldChunk<uint8_t> matInterpolation(ChunkType::MaterialInterpolation,
            static_cast<uint8_t>(mInterpolation));
    container.addChild(&matInterpolation);

    SimpleFieldChunk<uint32_t> matShaderModels(ChunkType::MaterialShaderModels,
            mShaderModels.getValue());
    container.addChild(&matShaderModels);

    // Generate all shaders.
    std::vector<TextEntry> glslEntries;
    std::vector<SpirvEntry> spirvEntries;
    std::vector<TextEntry> metalEntries;
    LineDictionary glslDictionary;
    BlobDictionary spirvDictionary;
    LineDictionary metalDictionary;
    std::vector<uint32_t> spirv;
    std::string msl;

    ShaderGenerator sg(mProperties, mVariables,
            mMaterialCode, mMaterialLineOffset, mMaterialVertexCode, mMaterialVertexLineOffset);

    bool emptyVertexCode = mMaterialVertexCode.empty();
    bool customDepth = sg.hasCustomDepthShader() ||
            mBlendingMode == BlendingMode::MASKED || !emptyVertexCode;
    SimpleFieldChunk<bool> hasCustomDepth(ChunkType::MaterialHasCustomDepthShader, customDepth);
    container.addChild(&hasCustomDepth);

    filament::SamplerBindingMap map;
    map.populate(&info.sib, mMaterialName.c_str());
    info.samplerBindings = std::move(map);

    for (const auto& params : mCodeGenPermutations) {
        const ShaderModel shaderModel = ShaderModel(params.shaderModel);
        const TargetApi targetApi = params.targetApi;
        const TargetLanguage targetLanguage = params.targetLanguage;

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

        // apply custom variants filters
        uint8_t variantMask = ~mVariantFilter;

        for (uint8_t k = 0; k < filament::VARIANT_COUNT; k++) {

            if (filament::Variant::isReserved(k)) {
                continue;
            }

            glslEntry.variant = k;
            spirvEntry.variant = k;
            metalEntry.variant = k;

            // Remove variants for unlit materials
            uint8_t v = filament::Variant::filterVariant(
                    k & variantMask, isLit() || mShadowMultiplier);

            if (filament::Variant::filterVariantVertex(v) == k) {
                // Vertex Shader
                std::string vs = sg.createVertexProgram(
                        shaderModel, targetApi, targetLanguage, info, k,
                        mInterpolation, mVertexDomain);
                bool ok = postProcessor.process(vs, filament::backend::ShaderType::VERTEX,
                        shaderModel, &vs, pSpirv, pMsl);
                if (!ok) {
                    showErrorMessage(mMaterialName.c_str_safe(), k, targetApi,
                            filament::backend::ShaderType::VERTEX, vs);
                    errorOccured = true;
                    break;
                }

                if (targetApi == TargetApi::OPENGL) {
                    if (targetLanguage == TargetLanguage::SPIRV) {
                        sg.fixupExternalSamplers(shaderModel, vs, info);
                    }

                    glslEntry.stage = filament::backend::ShaderType::VERTEX;
                    glslEntry.shaderSize = vs.size();
                    glslEntry.shader = (char*) malloc(glslEntry.shaderSize + 1);
                    strcpy(glslEntry.shader, vs.c_str());
                    glslDictionary.addText(glslEntry.shader);
                    glslEntries.push_back(glslEntry);
                }

                if (targetApi == TargetApi::VULKAN) {
                    assert(!spirv.empty());
                    spirvEntry.stage = filament::backend::ShaderType::VERTEX;
                    spirvEntry.dictionaryIndex = spirvDictionary.addBlob(spirv);
                    spirv.clear();
                    spirvEntries.push_back(spirvEntry);
                }
                if (targetApi == TargetApi::METAL) {
                    assert(spirv.size() > 0);
                    assert(msl.length() > 0);
                    metalEntry.stage = filament::backend::ShaderType::VERTEX;
                    metalEntry.shaderSize = msl.length();
                    metalEntry.shader = (char*)malloc(metalEntry.shaderSize + 1);
                    strcpy(metalEntry.shader, msl.c_str());
                    spirv.clear();
                    msl.clear();
                    metalDictionary.addText(metalEntry.shader);
                    metalEntries.push_back(metalEntry);
                }
            }

            if (filament::Variant::filterVariantFragment(v) == k) {
                // Fragment Shader
                std::string fs = sg.createFragmentProgram(
                        shaderModel, targetApi, targetLanguage, info, k, mInterpolation);

                bool ok = postProcessor.process(fs, filament::backend::ShaderType::FRAGMENT,
                        shaderModel, &fs, pSpirv, pMsl);
                if (!ok) {
                    showErrorMessage(mMaterialName.c_str_safe(), k, targetApi,
                            filament::backend::ShaderType::FRAGMENT, fs);
                    errorOccured = true;
                    break;
                }

                if (targetApi == TargetApi::OPENGL) {
                    if (targetLanguage == TargetLanguage::SPIRV) {
                        sg.fixupExternalSamplers(shaderModel, fs, info);
                    }

                    glslEntry.stage = filament::backend::ShaderType::FRAGMENT;
                    glslEntry.shaderSize = fs.size();
                    glslEntry.shader = (char*) malloc(glslEntry.shaderSize + 1);
                    strcpy(glslEntry.shader, fs.c_str());
                    glslDictionary.addText(glslEntry.shader);
                    glslEntries.push_back(glslEntry);
                }

                if (targetApi == TargetApi::VULKAN) {
                    assert(!spirv.empty());
                    spirvEntry.stage = filament::backend::ShaderType::FRAGMENT;
                    spirvEntry.dictionaryIndex = spirvDictionary.addBlob(spirv);
                    spirv.clear();
                    spirvEntries.push_back(spirvEntry);
                }
                if (targetApi == TargetApi::METAL) {
                    assert(spirv.size() > 0);
                    assert(msl.length() > 0);
                    metalEntry.stage = filament::backend::ShaderType::FRAGMENT;
                    metalEntry.shaderSize = msl.length();
                    metalEntry.shader = (char*)malloc(metalEntry.shaderSize + 1);
                    strcpy(metalEntry.shader, msl.c_str());
                    spirv.clear();
                    msl.clear();
                    metalDictionary.addText(metalEntry.shader);
                    metalEntries.push_back(metalEntry);
                }
            }
        }
    }

    // Emit GLSL chunks (TextDictionaryReader and MaterialTextChunk).
    filamat::DictionaryTextChunk dicGlslChunk(glslDictionary, ChunkType::DictionaryGlsl);
    MaterialTextChunk glslChunk(glslEntries, glslDictionary, ChunkType::MaterialGlsl);
    if (!glslEntries.empty()) {
        container.addChild(&dicGlslChunk);
        container.addChild(&glslChunk);
    }

    // Emit SPIRV chunks (SpirvDictionaryReader and MaterialSpirvChunk).
    filamat::DictionarySpirvChunk dicSpirvChunk(spirvDictionary);
    MaterialSpirvChunk spirvChunk(spirvEntries);
    if (!spirvEntries.empty()) {
        container.addChild(&dicSpirvChunk);
        container.addChild(&spirvChunk);
    }

    // Emit Metal chunks (MetalDictionaryReader and MaterialMetalChunk).
    filamat::DictionaryTextChunk dicMetalChunk(metalDictionary, ChunkType::DictionaryMetal);
    MaterialTextChunk metalChunk(metalEntries, metalDictionary, ChunkType::MaterialMetal);
    if (!metalEntries.empty()) {
        container.addChild(&dicMetalChunk);
        container.addChild(&metalChunk);
    }

    // Flatten all chunks in the container into a Package.
    size_t packageSize = container.getSize();
    Package package(packageSize);
    Flattener f(package);
    container.flatten(f);
    package.setValid(!errorOccured);

    // Free all shaders that were created earlier.
    for (TextEntry entry : glslEntries) {
        free(entry.shader);
    }
    for (TextEntry entry : metalEntries) {
        free(entry.shader);
    }
    return package;
}

const std::string MaterialBuilder::peek(filament::backend::ShaderType type,
        filament::backend::ShaderModel& model, const PropertyList& properties) noexcept {

    ShaderGenerator sg(properties, mVariables,
            mMaterialCode, mMaterialLineOffset, mMaterialVertexCode, mMaterialVertexLineOffset);

    MaterialInfo info;
    prepareToBuild(info);

    filament::SamplerBindingMap map;
    map.populate(&info.sib, mMaterialName.c_str());
    info.samplerBindings = std::move(map);

    for (const auto& params : mCodeGenPermutations) {
        model = ShaderModel(params.shaderModel);
        if (type == filament::backend::ShaderType::VERTEX) {
            return sg.createVertexProgram(model, params.targetApi, params.targetLanguage,
                    info, 0, mInterpolation, mVertexDomain);
        } else {
            return sg.createFragmentProgram(model, params.targetApi, params.targetLanguage,
                    info, 0, mInterpolation);
        }
    }

    return std::string("");
}

} // namespace filamat
