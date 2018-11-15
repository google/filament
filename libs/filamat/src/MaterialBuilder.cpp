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

#include <private/filament/Variant.h>

#include "shaders/MaterialInfo.h"
#include "shaders/ShaderGenerator.h"

#include "eiff/BlobDictionary.h"
#include "eiff/LineDictionary.h"
#include "eiff/MaterialInterfaceBlockChunk.h"
#include "eiff/MaterialGlslChunk.h"
#include "eiff/MaterialSpirvChunk.h"
#include "eiff/ChunkContainer.h"
#include "eiff/SimpleFieldChunk.h"
#include "eiff/DictionaryGlslChunk.h"
#include "eiff/DictionarySpirvChunk.h"

using namespace utils;

namespace filamat {

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

    // If the code gen target API was specifically set to Vulkan, generate for Vulkan, otherwise
    // generate for OpenGL (case OPENGL or ALL)
    TargetApi glCodeGenTargetApi = mCodeGenTargetApi != TargetApi::VULKAN ?
            TargetApi::OPENGL : TargetApi::VULKAN;

    // Build a list of codegen permutations, which is useful across all types of material builders.
    // The shader model loop starts at 1 to skip ShaderModel::UNKNOWN.
    for (uint8_t i = 1; i < filament::driver::SHADER_MODEL_COUNT; i++) {
        if (!mShaderModels.test(i)) {
            continue; // skip this shader model since it was not requested.
        }
        switch (mTargetApi) {
            case TargetApi::ALL:
                mCodeGenPermutations.push_back({i, TargetApi::OPENGL, glCodeGenTargetApi});
                mCodeGenPermutations.push_back({i, TargetApi::VULKAN, TargetApi::VULKAN});
                break;
            case TargetApi::OPENGL:
                mCodeGenPermutations.push_back({i, TargetApi::OPENGL, glCodeGenTargetApi});
                break;
            case TargetApi::VULKAN:
                mCodeGenPermutations.push_back({i, TargetApi::VULKAN, TargetApi::VULKAN});
                break;
        }
    }
}

MaterialBuilder::MaterialBuilder() : mMaterialName("Unnamed") {
    std::fill_n(mProperties, filament::MATERIAL_PROPERTIES_COUNT, false);
    mShaderModels.reset();
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

MaterialBuilder& MaterialBuilder::set(Property p) noexcept {
    // Note: switch/case here is useful in case we're given an invalid property
    switch (p) {
        case Property::BASE_COLOR:
        case Property::ROUGHNESS:
        case Property::METALLIC:
        case Property::REFLECTANCE:
        case Property::AMBIENT_OCCLUSION:
        case Property::CLEAR_COAT:
        case Property::CLEAR_COAT_ROUGHNESS:
        case Property::CLEAR_COAT_NORMAL:
        case Property::ANISOTROPY:
        case Property::ANISOTROPY_DIRECTION:
        case Property::THICKNESS:
        case Property::SUBSURFACE_POWER:
        case Property::SUBSURFACE_COLOR:
        case Property::SHEEN_COLOR:
        case Property::EMISSIVE:
        case Property::NORMAL:
            assert(size_t(p) < filament::MATERIAL_PROPERTIES_COUNT);
            mProperties[size_t(p)] = true;
            break;
    }
    return *this;
}

MaterialBuilder& MaterialBuilder::variable(Variable v, const char* name) noexcept {
    switch (v) {
        case Variable::CUSTOM0:
        case Variable::CUSTOM1:
        case Variable::CUSTOM2:
        case Variable::CUSTOM3:
            assert(size_t(v) < filament::MATERIAL_VARIABLES_COUNT);
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
    mDoubleSidedSet = true;
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
    mCodeGenTargetApi = targetApi;
    return *this;
}

MaterialBuilder& MaterialBuilder::codeGenTargetApi(TargetApi targetApi) noexcept {
    mCodeGenTargetApi = targetApi;
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
        ibb.add("maskThreshold", 1, UniformType::FLOAT);
    }

    mRequiredAttributes.set(filament::VertexAttribute::POSITION);
    if (mShading != filament::Shading::UNLIT || mShadowMultiplier) {
        mRequiredAttributes.set(filament::VertexAttribute::TANGENTS);
    }

    info.sib = sbb.name("MaterialParams").build();
    info.uib = ibb.name("MaterialParams").build();

    info.isLit = isLit();
    info.isDoubleSided = mDoubleSided;
    info.hasExternalSamplers = hasExternalSampler();
    info.requiredAttributes = mRequiredAttributes;
    info.blendingMode = mBlendingMode;
    info.shading = mShading;
    info.hasShadowMultiplier = mShadowMultiplier;
    info.samplerBindings.populate(&info.sib);
}

static void showErrorMessage(const char* materialName, uint8_t variant,
        MaterialBuilder::TargetApi targetApi, filament::driver::ShaderType shaderType,
        const std::string& shaderCode) {
    using ShaderType = filament::driver::ShaderType;
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
    MaterialInfo info;
    prepareToBuild(info);

    // Create chunk tree.
    ChunkContainer container;

    SimpleFieldChunk<uint32_t> matVersion(ChunkType::MaterialVersion, 1);
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

    MaterialSamplerBindingsChunk matSb = MaterialSamplerBindingsChunk(info.samplerBindings);
    if (mTargetApi == TargetApi::VULKAN || mTargetApi == TargetApi::ALL) {
        container.addChild(&matSb);
    }

    SimpleFieldChunk<bool> matDepthWriteSet(ChunkType::MaterialDepthWriteSet, mDepthWriteSet);
    container.addChild(&matDepthWriteSet);

    SimpleFieldChunk<bool> matDoubleSidedSet(ChunkType::MaterialDoubleSidedSet, mDoubleSidedSet);
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

    // In order to generate SPIR-V, we must run the GLSL through the post-processor.
    if (mCodeGenTargetApi != TargetApi::OPENGL && mPostprocessorCallback == nullptr) {
        utils::slog.e << "SPIR-V requested for " << mMaterialName.c_str()
                << " but there is no post-processor." << utils::io::endl;
    }

    SimpleFieldChunk<uint32_t> matShaderModels(ChunkType::MaterialShaderModels,
            mShaderModels.getValue());
    container.addChild(&matShaderModels);

    // Generate all shaders.
    std::vector<GlslEntry> glslEntries;
    std::vector<SpirvEntry> spirvEntries;
    LineDictionary glslDictionary;
    BlobDictionary spirvDictionary;
    std::vector<uint32_t> spirv;

    ShaderGenerator sg(mProperties, mVariables,
            mMaterialCode, mMaterialLineOffset, mMaterialVertexCode, mMaterialVertexLineOffset);

    bool emptyVertexCode = mMaterialVertexCode.empty();
    bool customDepth = sg.hasCustomDepthShader() ||
            mBlendingMode == BlendingMode::MASKED || !emptyVertexCode;
    SimpleFieldChunk<bool> hasCustomDepth(ChunkType::MaterialHasCustomDepthShader, customDepth);
    container.addChild(&hasCustomDepth);

    bool errorOccured = false;
    for (const auto& params : mCodeGenPermutations) {
        const ShaderModel shaderModel = ShaderModel(params.shaderModel);
        const TargetApi targetApi = params.targetApi;
        const TargetApi codeGenTargetApi = params.codeGenTargetApi;
        std::vector<uint32_t>* pSpirv = (targetApi == TargetApi::VULKAN) ? &spirv : nullptr;

        GlslEntry glslEntry;
        SpirvEntry spirvEntry;

        glslEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);
        spirvEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);

        // apply custom variants filters
        uint8_t variantMask = ~mVariantFilter;

        for (uint8_t k = 0; k < filament::VARIANT_COUNT; k++) {

            if (filament::Variant::isReserved(k)) {
                continue;
            }

            glslEntry.variant = k;
            spirvEntry.variant = k;

            // Remove variants for unlit materials
            uint8_t v = filament::Variant::filterVariant(k & variantMask, isLit() || mShadowMultiplier);

            if (filament::Variant::filterVariantVertex(v) == k) {
                // Vertex Shader
                std::string vs = sg.createVertexProgram(
                        shaderModel, targetApi, codeGenTargetApi, info, k,
                        mInterpolation, mVertexDomain);
                if (mPostprocessorCallback != nullptr) {
                    bool ok = mPostprocessorCallback(vs, filament::driver::ShaderType::VERTEX,
                            shaderModel, &vs, pSpirv);
                    if (!ok) {
                        showErrorMessage(mMaterialName.c_str_safe(), k, targetApi,
                                filament::driver::ShaderType::VERTEX, vs);
                        errorOccured = true;
                        break;
                    }
                }
                if (targetApi == TargetApi::OPENGL) {
                    glslEntry.stage = filament::driver::ShaderType::VERTEX;
                    glslEntry.shaderSize = vs.size();
                    glslEntry.shader = (char*)malloc(glslEntry.shaderSize + 1);
                    strcpy(glslEntry.shader, vs.c_str());
                    glslDictionary.addText(glslEntry.shader);
                    glslEntries.push_back(glslEntry);
                }
                if (targetApi == TargetApi::VULKAN) {
                    assert(spirv.size() > 0);
                    spirvEntry.stage = filament::driver::ShaderType::VERTEX;
                    spirvEntry.dictionaryIndex = spirvDictionary.addBlob(spirv);
                    spirv.clear();
                    spirvEntries.push_back(spirvEntry);
                }
            }

            if (filament::Variant::filterVariantFragment(v) == k) {
                // Fragment Shader
                std::string fs = sg.createFragmentProgram(
                        shaderModel, targetApi, codeGenTargetApi, info, k, mInterpolation);
                if (mPostprocessorCallback != nullptr) {
                    bool ok = mPostprocessorCallback(fs, filament::driver::ShaderType::FRAGMENT,
                            shaderModel, &fs, pSpirv);
                    if (!ok) {
                        showErrorMessage(mMaterialName.c_str_safe(), k, targetApi,
                                filament::driver::ShaderType::FRAGMENT, fs);
                        errorOccured = true;
                        break;
                    }
                }
                if (targetApi == TargetApi::OPENGL) {
                    glslEntry.stage = filament::driver::ShaderType::FRAGMENT;
                    glslEntry.shaderSize = fs.size();
                    glslEntry.shader = (char*)malloc(glslEntry.shaderSize + 1);
                    strcpy(glslEntry.shader, fs.c_str());
                    glslDictionary.addText(glslEntry.shader);
                    glslEntries.push_back(glslEntry);
                }
                if (targetApi == TargetApi::VULKAN) {
                    assert(spirv.size() > 0);
                    spirvEntry.stage = filament::driver::ShaderType::FRAGMENT;
                    spirvEntry.dictionaryIndex = spirvDictionary.addBlob(spirv);
                    spirv.clear();
                    spirvEntries.push_back(spirvEntry);
                }
            }
        }
    }

    // Emit GLSL chunks (TextDictionaryReader and MaterialGlslChunk).
    filamat::DictionaryGlslChunk dicGlslChunk(glslDictionary);
    MaterialGlslChunk glslChunk(glslEntries, glslDictionary);
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

    // Flatten all chunks in the container into a Package.
    size_t packageSize = container.getSize();
    Package package(packageSize);
    Flattener f(package);
    container.flatten(f);
    package.setValid(!errorOccured);

    // Free all shaders that were created earlier.
    for (GlslEntry entry : glslEntries) {
        free(entry.shader);
    }
    return package;
}

MaterialBuilder& MaterialBuilder::postProcessor(PostProcessCallBack callback) {
    mPostprocessorCallback = callback;
    return *this;
}

const std::string MaterialBuilder::peek(filament::driver::ShaderType type,
        filament::driver::ShaderModel& model) noexcept {

    ShaderGenerator sg(mProperties, mVariables,
            mMaterialCode, mMaterialLineOffset, mMaterialVertexCode, mMaterialVertexLineOffset);

    MaterialInfo info;
    prepareToBuild(info);

    for (const auto& params : mCodeGenPermutations) {
        model = ShaderModel(params.shaderModel);
        const TargetApi targetApi = params.targetApi;
        const TargetApi codeGenTargetApi = params.codeGenTargetApi;
        if (type == filament::driver::ShaderType::VERTEX) {
            return sg.createVertexProgram(model, targetApi, codeGenTargetApi,
                    info, 0, mInterpolation, mVertexDomain);
        } else {
            return sg.createFragmentProgram(model, targetApi, codeGenTargetApi,
                    info, 0, mInterpolation);
        }
    }

    return std::string("");
}

} // namespace filamat
