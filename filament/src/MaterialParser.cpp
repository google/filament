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


#include "MaterialParser.h"

#include <filaflat/ChunkContainer.h>
#include <filaflat/MaterialChunk.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/Unflattener.h>

#include <filament/MaterialChunkType.h>

#include <private/filament/SamplerBindingsInfo.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/BufferInterfaceBlock.h>
#include <private/filament/SubpassInfo.h>
#include <private/filament/Variant.h>
#include <private/filament/ConstantInfo.h>

#include <utils/CString.h>

#include <stdlib.h>

using namespace utils;
using namespace filament::backend;
using namespace filaflat;
using namespace filamat;

namespace filament {

constexpr std::pair<ChunkType, ChunkType> shaderLanguageToTags(ShaderLanguage language) {
    switch (language) {
        case ShaderLanguage::ESSL3:
            return { ChunkType::MaterialGlsl, ChunkType::DictionaryText };
        case ShaderLanguage::ESSL1:
            return { ChunkType::MaterialEssl1, ChunkType::DictionaryText };
        case ShaderLanguage::MSL:
            return { ChunkType::MaterialMetal, ChunkType::DictionaryText };
        case ShaderLanguage::SPIRV:
            return { ChunkType::MaterialSpirv, ChunkType::DictionarySpirv };
        case ShaderLanguage::METAL_LIBRARY:
            return { ChunkType::MaterialMetalLibrary, ChunkType::DictionaryMetalLibrary };
    }
}

// ------------------------------------------------------------------------------------------------

MaterialParser::MaterialParserDetails::MaterialParserDetails(
        const utils::FixedCapacityVector<ShaderLanguage>& preferredLanguages, const void* data,
        size_t size)
    : mManagedBuffer(data, size),
      mChunkContainer(mManagedBuffer.data(), mManagedBuffer.size()),
      mPreferredLanguages(preferredLanguages),
      mMaterialChunk(mChunkContainer) {}

template<typename T>
UTILS_NOINLINE
bool MaterialParser::MaterialParserDetails::getFromSimpleChunk(
        filamat::ChunkType type, T* value) const noexcept {
    ChunkContainer const& chunkContainer = mChunkContainer;
    ChunkContainer::ChunkDesc chunkDesc;
    if (chunkContainer.hasChunk(type, &chunkDesc)) {
        Unflattener unflattener(chunkDesc.start, chunkDesc.start + chunkDesc.size);
        return unflattener.read(value);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------

MaterialParser::MaterialParser(utils::FixedCapacityVector<ShaderLanguage> preferredLanguages,
        const void* data, size_t size)
    : mImpl(preferredLanguages, data, size) {}

ChunkContainer& MaterialParser::getChunkContainer() noexcept {
    return mImpl.mChunkContainer;
}

ChunkContainer const& MaterialParser::getChunkContainer() const noexcept {
    return mImpl.mChunkContainer;
}

MaterialParser::ParseResult MaterialParser::parse() noexcept {
    ChunkContainer& cc = getChunkContainer();
    if (UTILS_UNLIKELY(!cc.parse())) {
        return ParseResult::ERROR_OTHER;
    }

    auto chooseLanguage =
            [this, &cc]() -> std::optional<std::tuple<ShaderLanguage, ChunkType, ChunkType>> {
        for (auto language : mImpl.mPreferredLanguages) {
            const auto [matTag, dictTag] = shaderLanguageToTags(language);
            if (cc.hasChunk(matTag) && cc.hasChunk(dictTag)) {
                return std::make_tuple(language, matTag, dictTag);
            }
        }
        return {};
    };
    const auto result = chooseLanguage();

    if (!result.has_value()) {
        return ParseResult::ERROR_MISSING_BACKEND;
    }

    const auto [chosenLanguage, matTag, dictTag] = result.value();
    if (UTILS_UNLIKELY(!DictionaryReader::unflatten(cc, dictTag, mImpl.mBlobDictionary))) {
        return ParseResult::ERROR_OTHER;
    }
    if (UTILS_UNLIKELY(!mImpl.mMaterialChunk.initialize(matTag))) {
        return ParseResult::ERROR_OTHER;
    }

    mImpl.mChosenLanguage = chosenLanguage;
    return ParseResult::SUCCESS;
}

backend::ShaderLanguage MaterialParser::getShaderLanguage() const noexcept {
    return mImpl.mChosenLanguage;
}

// Accessors
bool MaterialParser::getMaterialVersion(uint32_t* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialVersion, value);
}

bool MaterialParser::getFeatureLevel(uint8_t* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialFeatureLevel, value);
}

bool MaterialParser::getName(utils::CString* cstring) const noexcept {
   auto [start, end] = mImpl.mChunkContainer.getChunkRange(MaterialName);
    if (start == end) return false;
   Unflattener unflattener(start, end);
   return unflattener.read(cstring);
}

bool MaterialParser::getCacheId(uint64_t* cacheId) const noexcept {
   auto [start, end] = mImpl.mChunkContainer.getChunkRange(MaterialCacheId);
    if (start == end) return false;
   Unflattener unflattener(start, end);
   return unflattener.read(cacheId);
}

bool MaterialParser::getUIB(BufferInterfaceBlock* uib) const noexcept {
    auto [start, end] = mImpl.mChunkContainer.getChunkRange(MaterialUib);
    if (start == end) return false;
    Unflattener unflattener(start, end);
    return ChunkUniformInterfaceBlock::unflatten(unflattener, uib);
}

bool MaterialParser::getSIB(SamplerInterfaceBlock* sib) const noexcept {
    auto [start, end] = mImpl.mChunkContainer.getChunkRange(MaterialSib);
    if (start == end) return false;
    Unflattener unflattener(start, end);
    return ChunkSamplerInterfaceBlock::unflatten(unflattener, sib);
}

bool MaterialParser::getSubpasses(SubpassInfo* subpass) const noexcept {
    auto [start, end] = mImpl.mChunkContainer.getChunkRange(MaterialSubpass);
    if (start == end) return false;
    Unflattener unflattener(start, end);
    return ChunkSubpassInterfaceBlock::unflatten(unflattener, subpass);
}

bool MaterialParser::getShaderModels(uint32_t* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialShaderModels, value);
}

bool MaterialParser::getMaterialProperties(uint64_t* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialProperties, value);
}

bool MaterialParser::getUniformBlockBindings(
        utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>>* value) const noexcept {
    auto [start, end] = mImpl.mChunkContainer.getChunkRange(MaterialUniformBindings);
    if (start == end) return false;
    Unflattener unflattener(start, end);
    return ChunkUniformBlockBindings::unflatten(unflattener, value);
}

bool MaterialParser::getBindingUniformInfo(BindingUniformInfoContainer* container) const noexcept {
    auto [start, end] = mImpl.mChunkContainer.getChunkRange(MaterialBindingUniformInfo);
    if (start == end) return false;
    Unflattener unflattener(start, end);
    return ChunkBindingUniformInfo::unflatten(unflattener, container);
}

bool MaterialParser::getAttributeInfo(AttributeInfoContainer* container) const noexcept {
    auto [start, end] = mImpl.mChunkContainer.getChunkRange(MaterialAttributeInfo);
    if (start == end) return false;
    Unflattener unflattener(start, end);
    return ChunkAttributeInfo::unflatten(unflattener, container);
}

bool MaterialParser::getSamplerBlockBindings(
        SamplerGroupBindingInfoList* pSamplerGroupInfoList,
        SamplerBindingToNameMap* pSamplerBindingToNameMap) const noexcept {
    auto [start, end] = mImpl.mChunkContainer.getChunkRange(MaterialSamplerBindings);
    if (start == end) return false;
    Unflattener unflattener(start, end);
    return ChunkSamplerBlockBindings::unflatten(unflattener,
            pSamplerGroupInfoList, pSamplerBindingToNameMap);
}

bool MaterialParser::getConstants(utils::FixedCapacityVector<MaterialConstant>* value) const noexcept {
    auto [start, end] = mImpl.mChunkContainer.getChunkRange(filamat::MaterialConstants);
    if (start == end) return false;
    Unflattener unflattener(start, end);
    return ChunkMaterialConstants::unflatten(unflattener, value);
}

bool MaterialParser::getDepthWriteSet(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialDepthWriteSet, value);
}

bool MaterialParser::getDepthWrite(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialDepthWrite, value);
}

bool MaterialParser::getDoubleSidedSet(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialDoubleSidedSet, value);
}

bool MaterialParser::getDoubleSided(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialDoubleSided, value);
}

bool MaterialParser::getColorWrite(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialColorWrite, value);
}

bool MaterialParser::getDepthTest(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialDepthTest, value);
}

bool MaterialParser::getInstanced(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialInstanced, value);
}

bool MaterialParser::getCullingMode(CullingMode* value) const noexcept {
    static_assert(sizeof(CullingMode) == sizeof(uint8_t),
            "CullingMode expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialCullingMode, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getTransparencyMode(TransparencyMode* value) const noexcept {
    static_assert(sizeof(TransparencyMode) == sizeof(uint8_t),
            "TransparencyMode expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialTransparencyMode,
            reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getInterpolation(Interpolation* value) const noexcept {
    static_assert(sizeof(Interpolation) == sizeof(uint8_t),
            "Interpolation expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialInterpolation, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getVertexDomain(VertexDomain* value) const noexcept {
    static_assert(sizeof(VertexDomain) == sizeof(uint8_t),
            "VertexDomain expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialVertexDomain, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getMaterialVariantFilterMask(UserVariantFilterMask* value) const noexcept {
    static_assert(sizeof(UserVariantFilterMask) == sizeof(uint32_t),
            "UserVariantFilterMask expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialVariantFilterMask,
            reinterpret_cast<UserVariantFilterMask*>(value));
}

bool MaterialParser::getMaterialDomain(MaterialDomain* value) const noexcept {
    static_assert(sizeof(MaterialDomain) == sizeof(uint8_t),
            "MaterialDomain expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialDomain, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getBlendingMode(BlendingMode* value) const noexcept {
    static_assert(sizeof(BlendingMode) == sizeof(uint8_t),
            "BlendingMode expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialBlendingMode, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getCustomBlendFunction(std::array<BlendFunction, 4>* value) const noexcept {
    uint32_t blendFunctions = 0;
    bool const result =  mImpl.getFromSimpleChunk(ChunkType::MaterialBlendFunction, &blendFunctions);
    (*value)[0] = BlendFunction((blendFunctions >> 24) & 0xFF);
    (*value)[1] = BlendFunction((blendFunctions >> 16) & 0xFF);
    (*value)[2] = BlendFunction((blendFunctions >>  8) & 0xFF);
    (*value)[3] = BlendFunction((blendFunctions >>  0) & 0xFF);
    return result;
}

bool MaterialParser::getMaskThreshold(float* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialMaskThreshold, value);
}

bool MaterialParser::getAlphaToCoverageSet(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialAlphaToCoverageSet, value);
}

bool MaterialParser::getAlphaToCoverage(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialAlphaToCoverage, value);
}

bool MaterialParser::hasShadowMultiplier(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialShadowMultiplier, value);
}

bool MaterialParser::getShading(Shading* value) const noexcept {
    static_assert(sizeof(Shading) == sizeof(uint8_t),
            "Shading expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialShading, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::hasCustomDepthShader(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialHasCustomDepthShader, value);
}

bool MaterialParser::hasSpecularAntiAliasing(bool* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialSpecularAntiAliasing, value);
}

bool MaterialParser::getSpecularAntiAliasingVariance(float* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialSpecularAntiAliasingVariance, value);
}

bool MaterialParser::getSpecularAntiAliasingThreshold(float* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialSpecularAntiAliasingThreshold, value);
}

bool MaterialParser::getRequiredAttributes(AttributeBitset* value) const noexcept {
    uint32_t rawAttributes = 0;
    if (!mImpl.getFromSimpleChunk(ChunkType::MaterialRequiredAttributes, &rawAttributes)) {
        return false;
    }
    *value = AttributeBitset();
    value->setValue(rawAttributes);
    return true;
}

bool MaterialParser::getRefractionMode(RefractionMode* value) const noexcept {
    static_assert(sizeof(RefractionMode) == sizeof(uint8_t),
            "Refraction expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialRefraction, (uint8_t*)value);
}

bool MaterialParser::getRefractionType(RefractionType* value) const noexcept {
    static_assert(sizeof(RefractionType) == sizeof(uint8_t),
            "RefractionType expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialRefractionType, (uint8_t*)value);
}

bool MaterialParser::getReflectionMode(ReflectionMode* value) const noexcept {
    static_assert(sizeof(ReflectionMode) == sizeof(uint8_t),
            "ReflectionMode expected size is wrong");
    return mImpl.getFromSimpleChunk(ChunkType::MaterialReflectionMode, (uint8_t*)value);
}

bool MaterialParser::getShader(ShaderContent& shader,
        ShaderModel shaderModel, Variant variant, ShaderStage stage) noexcept {
    return mImpl.mMaterialChunk.getShader(shader,
            mImpl.mBlobDictionary, shaderModel, variant, stage);
}

// ------------------------------------------------------------------------------------------------


bool ChunkUniformInterfaceBlock::unflatten(Unflattener& unflattener,
        filament::BufferInterfaceBlock* uib) {

    BufferInterfaceBlock::Builder builder = BufferInterfaceBlock::Builder();

    CString name;
    if (!unflattener.read(&name)) {
        return false;
    }

    builder.name({ name.data(), name.size() });

    // Read number of fields.
    uint64_t numFields = 0;
    if (!unflattener.read(&numFields)) {
        return false;
    }

    for (uint64_t i = 0; i < numFields; i++) {
        CString fieldName;
        uint64_t fieldSize = 0;
        uint8_t fieldType = 0;
        uint8_t fieldPrecision = 0;

        if (!unflattener.read(&fieldName)) {
            return false;
        }

        if (!unflattener.read(&fieldSize)) {
            return false;
        }

        if (!unflattener.read(&fieldType)) {
            return false;
        }

        if (!unflattener.read(&fieldPrecision)) {
            return false;
        }

        // a size of 1 means not an array
        builder.add({{{ fieldName.data(), fieldName.size() },
                      uint32_t(fieldSize == 1 ? 0 : fieldSize),
                      BufferInterfaceBlock::Type(fieldType),
                      BufferInterfaceBlock::Precision(fieldPrecision) }});
    }

    *uib = builder.build();
    return true;
}

bool ChunkSamplerInterfaceBlock::unflatten(Unflattener& unflattener,
        filament::SamplerInterfaceBlock* sib) {

    SamplerInterfaceBlock::Builder builder = SamplerInterfaceBlock::Builder();

    CString name;
    if (!unflattener.read(&name)) {
        return false;
    }
    builder.name({ name.data(), name.size() });

    // Read number of fields.
    uint64_t numFields = 0;
    if (!unflattener.read(&numFields)) {
        return false;
    }

    for (uint64_t i = 0; i < numFields; i++) {
        CString fieldName;
        uint8_t fieldType = 0;
        uint8_t fieldFormat = 0;
        uint8_t fieldPrecision = 0;
        bool fieldMultisample = false;

        if (!unflattener.read(&fieldName)) {
            return false;
        }

        if (!unflattener.read(&fieldType)) {
            return false;
        }

        if (!unflattener.read(&fieldFormat)) {
            return false;
        }

        if (!unflattener.read(&fieldPrecision)) {
            return false;
        }

        if (!unflattener.read(&fieldMultisample)) {
            return false;
        }

        builder.add({ fieldName.data(), fieldName.size() }, SamplerInterfaceBlock::Type(fieldType),
                SamplerInterfaceBlock::Format(fieldFormat),
                SamplerInterfaceBlock::Precision(fieldPrecision),
                fieldMultisample);
    }

    *sib = builder.build();
    return true;
}

bool ChunkSubpassInterfaceBlock::unflatten(Unflattener& unflattener,
        filament::SubpassInfo* subpass) {

    CString block;
    if (!unflattener.read(&block)) {
        return false;
    }
    subpass->block = block;

    // Read number of subpasses.
    uint64_t numSubpasses = 0;
    if (!unflattener.read(&numSubpasses)) {
        return false;
    }

    for (uint64_t i = 0; i < numSubpasses; i++) {
        CString subpassName;
        uint8_t subpassType = 0;
        uint8_t subpassFormat = 0;
        uint8_t subpassPrecision = 0;

        if (!unflattener.read(&subpass->name)) {
            return false;
        }

        if (!unflattener.read(&subpassType)) {
            return false;
        }

        if (!unflattener.read(&subpassFormat)) {
            return false;
        }

        if (!unflattener.read(&subpassPrecision)) {
            return false;
        }

        if (!unflattener.read(&subpass->attachmentIndex)) {
            return false;
        }

        if (!unflattener.read(&subpass->binding)) {
            return false;
        }

        subpass->type = SubpassType (subpassType);
        subpass->format = Format (subpassFormat);
        subpass->precision = Precision (subpassPrecision);

        subpass->isValid = true;
    }

    return true;
}

bool ChunkUniformBlockBindings::unflatten(filaflat::Unflattener& unflattener,
        utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>>* uniformBlockBindings) {
    uint8_t count;
    if (!unflattener.read(&count)) {
        return false;
    }
    uniformBlockBindings->reserve(count);

    for (uint8_t i = 0; i < count; i++) {
        CString name;
        uint8_t binding;
        if (!unflattener.read(&name)) {
            return false;
        }
        if (!unflattener.read(&binding)) {
            return false;
        }
        uniformBlockBindings->emplace_back(std::move(name), binding);
    }
    return true;
}

bool ChunkBindingUniformInfo::unflatten(filaflat::Unflattener& unflattener,
        MaterialParser::BindingUniformInfoContainer* bindingUniformInfo) {
    uint8_t bindingPointCount;
    if (!unflattener.read(&bindingPointCount)) {
        return false;
    }
    bindingUniformInfo->reserve(bindingPointCount);
    for (size_t i = 0; i < bindingPointCount; i++) {
        uint8_t index;
        if (!unflattener.read(&index)) {
            return false;
        }
        uint8_t uniformCount;
        if (!unflattener.read(&uniformCount)) {
            return false;
        }

        Program::UniformInfo uniforms = Program::UniformInfo::with_capacity(uniformCount);
        for (size_t j = 0; j < uniformCount; j++) {
            utils::CString name;
            if (!unflattener.read(&name)) {
                return false;
            }
            uint16_t offset;
            if (!unflattener.read(&offset)) {
                return false;
            }
            uint8_t size;
            if (!unflattener.read(&size)) {
                return false;
            }
            uint8_t type;
            if (!unflattener.read(&type)) {
                return false;
            }
            uniforms.push_back({ name, offset, size, UniformType(type) });
        }
        bindingUniformInfo->emplace_back(UniformBindingPoints(index), std::move(uniforms));
    }
    return true;
}

bool ChunkAttributeInfo::unflatten(filaflat::Unflattener& unflattener,
        MaterialParser::AttributeInfoContainer* attributeInfoContainer) {

    uint8_t attributeCount;
    if (!unflattener.read(&attributeCount)) {
        return false;
    }

    attributeInfoContainer->reserve(attributeCount);

    for (size_t j = 0; j < attributeCount; j++) {
        utils::CString name;
        if (!unflattener.read(&name)) {
            return false;
        }
        uint8_t location;
        if (!unflattener.read(&location)) {
            return false;
        }
        attributeInfoContainer->emplace_back(name, location);
    }

    return true;
}

bool ChunkSamplerBlockBindings::unflatten(Unflattener& unflattener,
        SamplerGroupBindingInfoList* pSamplerGroupBindingInfoList,
        SamplerBindingToNameMap* pSamplerBindingToNameMap) {
    assert_invariant(pSamplerGroupBindingInfoList && pSamplerBindingToNameMap);
    SamplerGroupBindingInfoList& samplerGroupBindingInfoList = *pSamplerGroupBindingInfoList;
    SamplerBindingToNameMap& samplerBindingToNameMap = *pSamplerBindingToNameMap;

    uint8_t count;
    if (!unflattener.read(&count)) {
        return false;
    }
    assert_invariant(count == utils::Enum::count<SamplerBindingPoints>());

    UTILS_NOUNROLL
    for (size_t i = 0; i < count; i++) {
        if (!unflattener.read(&samplerGroupBindingInfoList[i].bindingOffset)) {
            return false;
        }
        if (!unflattener.read((uint8_t *)&samplerGroupBindingInfoList[i].shaderStageFlags)) {
            return false;
        }
        if (!unflattener.read(&samplerGroupBindingInfoList[i].count)) {
            return false;
        }
    }

    if (!unflattener.read(&count)) {
        return false;
    }

    samplerBindingToNameMap.reserve(count);
    samplerBindingToNameMap.resize(count);
    for (size_t i = 0; i < count; i++) {
        uint8_t binding;
        if (!unflattener.read(&binding)) {
            return false;
        }
        assert_invariant(binding < backend::MAX_SAMPLER_COUNT);
        if (!unflattener.read(&samplerBindingToNameMap[binding])) {
            return false;
        }
    }

    return true;
}

bool ChunkMaterialConstants::unflatten(filaflat::Unflattener& unflattener,
        utils::FixedCapacityVector<MaterialConstant>* materialConstants) {
    assert_invariant(materialConstants);

    // Read number of constants.
    uint64_t numConstants = 0;
    if (!unflattener.read(&numConstants)) {
        return false;
    }

    materialConstants->reserve(numConstants);
    materialConstants->resize(numConstants);

    for (uint64_t i = 0; i < numConstants; i++) {
        CString constantName;
        uint8_t constantType = 0;

        if (!unflattener.read(&constantName)) {
            return false;
        }

        if (!unflattener.read(&constantType)) {
            return false;
        }

        (*materialConstants)[i].name = constantName;
        (*materialConstants)[i].type = static_cast<backend::ConstantType>(constantType);
    }

    return true;
}

} // namespace filament
