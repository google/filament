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

#include <filaflat/BlobDictionary.h>
#include <filaflat/ChunkContainer.h>
#include <filaflat/MaterialChunk.h>
#include <filaflat/ShaderBuilder.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/Unflattener.h>

#include <filament/MaterialChunkType.h>

#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/UniformInterfaceBlock.h>

#include <utils/CString.h>

#include <stdlib.h>

using namespace utils;
using namespace filament::backend;
using namespace filaflat;
using namespace filamat;

namespace filament {

// ------------------------------------------------------------------------------------------------

MaterialParser::MaterialParserDetails::MaterialParserDetails(Backend backend, const void* data, size_t size)
        : mManagedBuffer(data, size),
          mChunkContainer(mManagedBuffer.data(), mManagedBuffer.size()),
          mMaterialChunk(mChunkContainer) {
    switch (backend) {
        case Backend::OPENGL:
            mMaterialTag = ChunkType::MaterialGlsl;
            mDictionaryTag = ChunkType::DictionaryText;
            break;
        case Backend::METAL:
            mMaterialTag = ChunkType::MaterialMetal;
            mDictionaryTag = ChunkType::DictionaryText;
            break;
        case Backend::VULKAN:
            mMaterialTag = ChunkType::MaterialSpirv;
            mDictionaryTag = ChunkType::DictionarySpirv;
            break;
        default:
            // this is for testing purpose -- for e.g.: with the NoopDriver
            mMaterialTag = ChunkType::MaterialGlsl;
            mDictionaryTag = ChunkType::DictionaryText;
            break;
    }
}

template<typename T>
bool MaterialParser::MaterialParserDetails::getFromSimpleChunk(filamat::ChunkType type, T* value) const noexcept {
    if (mChunkContainer.hasChunk(type)) {
        Unflattener unflattener(
                mChunkContainer.getChunkStart(type),
                mChunkContainer.getChunkEnd(type));
        return unflattener.read(value);
    }
    return false;
}

// ------------------------------------------------------------------------------------------------

MaterialParser::MaterialParser(Backend backend, const void* data, size_t size)
        : mImpl(backend, data, size) {
}

ChunkContainer& MaterialParser::getChunkContainer() noexcept {
    return mImpl.mChunkContainer;
}

ChunkContainer const& MaterialParser::getChunkContainer() const noexcept {
    return mImpl.mChunkContainer;
}

bool MaterialParser::parse() noexcept {
    ChunkContainer& cc = getChunkContainer();
    if (cc.parse()) {
        if (!cc.hasChunk(mImpl.mMaterialTag) || !cc.hasChunk(mImpl.mDictionaryTag)) {
            return false;
        }
        if (!DictionaryReader::unflatten(cc, mImpl.mDictionaryTag, mImpl.mBlobDictionary)) {
            return false;
        }
        if (!mImpl.mMaterialChunk.readIndex(mImpl.mMaterialTag)) {
            return false;
        }
    }
    return true;
}

// Accessors
bool MaterialParser::getMaterialVersion(uint32_t* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialVersion, value);
}

bool MaterialParser::getName(utils::CString* cstring) const noexcept {
   ChunkType type = ChunkType::MaterialName;
   const uint8_t* start = mImpl.mChunkContainer.getChunkStart(type);
   const uint8_t* end = mImpl.mChunkContainer.getChunkEnd(type);
   Unflattener unflattener(start, end);
   return unflattener.read(cstring);
}

bool MaterialParser::getUIB(UniformInterfaceBlock* uib) const noexcept {
    auto type = MaterialUib;
    const uint8_t* start = mImpl.mChunkContainer.getChunkStart(type);
    const uint8_t* end = mImpl.mChunkContainer.getChunkEnd(type);
    Unflattener unflattener(start, end);
    return ChunkUniformInterfaceBlock::unflatten(unflattener, uib);
}

bool MaterialParser::getSIB(SamplerInterfaceBlock* sib) const noexcept {
    auto type = MaterialSib;
    const uint8_t* start = mImpl.mChunkContainer.getChunkStart(type);
    const uint8_t* end = mImpl.mChunkContainer.getChunkEnd(type);
    Unflattener unflattener(start, end);
    return ChunkSamplerInterfaceBlock::unflatten(unflattener, sib);
}

bool MaterialParser::getShaderModels(uint32_t* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialShaderModels, value);
}

bool MaterialParser::getMaterialProperties(uint64_t* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialProperties, value);
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

bool MaterialParser::getMaskThreshold(float* value) const noexcept {
    return mImpl.getFromSimpleChunk(ChunkType::MaterialMaskThreshold, value);
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

bool MaterialParser::getShader(ShaderBuilder& shader,
        ShaderModel shaderModel, uint8_t variant, ShaderType stage) noexcept {
    return mImpl.mMaterialChunk.getShader(shader,
            mImpl.mBlobDictionary, (uint8_t)shaderModel, variant, stage);
}

// ------------------------------------------------------------------------------------------------


bool ChunkUniformInterfaceBlock::unflatten(Unflattener& unflattener,
        filament::UniformInterfaceBlock* uib) {

    UniformInterfaceBlock::Builder builder = UniformInterfaceBlock::Builder();

    CString name;
    if (!unflattener.read(&name)) {
        return false;
    }

    builder.name(std::move(name));

    // Read number of fields.
    uint64_t numFields = 0;
    if (!unflattener.read(&numFields)) {
        return false;
    }

    for (uint64_t i = 0; i < numFields; i++) {
        CString fieldName;
        uint64_t fieldSize;
        uint8_t fieldType;
        uint8_t fieldPrecision;

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

        builder.add(fieldName, fieldSize, UniformInterfaceBlock::Type(fieldType),
                UniformInterfaceBlock::Precision(fieldPrecision));
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
    builder.name(name);

    // Read number of fields.
    uint64_t numFields = 0;
    if (!unflattener.read(&numFields)) {
        return false;
    }

    for (uint64_t i = 0; i < numFields; i++) {
        CString fieldName;
        uint8_t fieldType;
        uint8_t fieldFormat;
        uint8_t fieldPrecision;
        bool fieldMultisample;

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

        builder.add(fieldName, SamplerInterfaceBlock::Type(fieldType),
                SamplerInterfaceBlock::Format(fieldFormat),
                SamplerInterfaceBlock::Precision(fieldPrecision),
                fieldMultisample);
    }

    *sib = builder.build();
    return true;
}

} // namespace filament
