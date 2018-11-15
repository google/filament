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

#include "filaflat/MaterialParser.h"

#include <filaflat/ChunkContainer.h>
#include <filaflat/ShaderBuilder.h>
#include <filaflat/Unflattener.h>

#include "BlobDictionary.h"
#include "ChunkInterfaceBlock.h"
#include "MaterialChunk.h"
#include "TextDictionaryReader.h"
#include "SpirvDictionaryReader.h"

#include <filament/SamplerBindingMap.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/UniformInterfaceBlock.h>

#include <utils/CString.h>

#include <stdlib.h>

using namespace utils;
using namespace filament;
using namespace filamat;

namespace filaflat {

// Make a copy of content and own the allocated memory.
class ManagedBuffer  {
    void* mStart = nullptr;
    size_t mSize = 0;
public:
    explicit ManagedBuffer(const void* start, size_t size)
            : mStart(malloc(size)), mSize(size) {
        memcpy(mStart, start, size);
    }

    void* begin() const noexcept { return mStart; }
    void* end() const noexcept { return (uint8_t*)mStart + mSize; }
    size_t size() const noexcept { return mSize; }

    ~ManagedBuffer() noexcept {
        free(mStart);
    }
};

struct MaterialParserDetails {
    MaterialParserDetails(filament::driver::Backend backend, const void* data, size_t size)
            : mUnflattenable(data, size),
              mChunkContainer(mUnflattenable.begin(), mUnflattenable.size()),
              mBackend(backend) {
    }
    ManagedBuffer mUnflattenable;
    ChunkContainer mChunkContainer;

    // Keep MaterialChunk alive between calls to getShader to avoid reload the shader index.
    filament::driver::Backend mBackend;
    MaterialChunk mMaterialChunk;
    BlobDictionary mBlobDictionary;

    template<typename T>
    bool getFromSimpleChunk(filamat::ChunkType type, T* value) const noexcept;

    bool getVkShader(filament::driver::ShaderModel shaderModel, uint8_t variant,
            filament::driver::ShaderType st, ShaderBuilder& shader) noexcept;

    bool getGlShader(filament::driver::ShaderModel shaderModel, uint8_t variant,
            filament::driver::ShaderType st, ShaderBuilder& shader) noexcept;
};

template<typename T>
bool MaterialParserDetails::getFromSimpleChunk(filamat::ChunkType type, T* value) const noexcept {
    if (!mChunkContainer.hasChunk(type)) {
        return false;
    }
    const uint8_t* start = mChunkContainer.getChunkStart(type);
    const uint8_t* end = mChunkContainer.getChunkEnd(type);
    Unflattener unflattener(start, end);
    return unflattener.read(value);
}

MaterialParser::MaterialParser(filament::driver::Backend backend, const void* data, size_t size)
        : mImpl(new MaterialParserDetails(backend, data, size)) {
}

MaterialParser::~MaterialParser() {
    delete mImpl;
}

ChunkContainer& MaterialParser::getChunkContainer() noexcept {
    return mImpl->mChunkContainer;
}

ChunkContainer const& MaterialParser::getChunkContainer() const noexcept {
    return mImpl->mChunkContainer;
}

bool MaterialParser::parse() noexcept {
    ChunkContainer& cc = getChunkContainer();
    return cc.parse();
}

bool MaterialParser::isShadingMaterial() const noexcept {
    ChunkContainer const& cc = getChunkContainer();
    return cc.hasChunk(MaterialName) &&
           cc.hasChunk(MaterialVersion) &&
           cc.hasChunk(MaterialUib) &&
           cc.hasChunk(MaterialSib) &&
           (cc.hasChunk(MaterialGlsl) || cc.hasChunk(MaterialSpirv)) &&
           cc.hasChunk(MaterialShaderModels);
}

bool MaterialParser::isPostProcessMaterial() const noexcept {
    ChunkContainer const& cc = getChunkContainer();
    return cc.hasChunk(PostProcessVersion) &&
           ((cc.hasChunk(MaterialSpirv) && cc.hasChunk(DictionarySpirv)) ||
            (cc.hasChunk(MaterialGlsl) && cc.hasChunk(DictionaryGlsl)));
}

// Accessors
bool MaterialParser::getName(utils::CString* cstring) const noexcept {
   ChunkType type = ChunkType::MaterialName;

   const uint8_t* start = mImpl->mChunkContainer.getChunkStart(type);
   const uint8_t* end = mImpl->mChunkContainer.getChunkEnd(type);
   Unflattener unflattener(start, end);

   CString s;
   if (unflattener.read(&s)) {
       *cstring =  CString(s.c_str());
       return true;
   } else {
       *cstring = CString("");
       return false;
   }
}

bool MaterialParser::getUIB(filament::UniformInterfaceBlock* uib) const noexcept {
    auto type = MaterialUib;

    const uint8_t* start = mImpl->mChunkContainer.getChunkStart(type);
    const uint8_t* end = mImpl->mChunkContainer.getChunkEnd(type);
    Unflattener unflattener(start, end);

    return ChunkUniformInterfaceBlock().unflatten(unflattener, uib);
}

bool MaterialParser::getSIB(filament::SamplerInterfaceBlock* sib) const noexcept {
    auto type = MaterialSib;

    const uint8_t* start = mImpl->mChunkContainer.getChunkStart(type);
    const uint8_t* end = mImpl->mChunkContainer.getChunkEnd(type);
    Unflattener unflattener(start, end);

    return ChunkSamplerInterfaceBlock().unflatten(unflattener, sib);
}

bool MaterialParser::getSamplerBindingMap(filament::SamplerBindingMap* bindings) const noexcept {
    auto type = MaterialSamplerBindings;

    if (!mImpl->mChunkContainer.hasChunk(type)) {
        return false;
    }

    const uint8_t* start = mImpl->mChunkContainer.getChunkStart(type);
    const uint8_t* end = mImpl->mChunkContainer.getChunkEnd(type);
    Unflattener unflattener(start, end);

    return ChunkSamplerBindingsBlock().unflatten(unflattener, bindings);
}

bool MaterialParser::getShaderModels(uint32_t* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialShaderModels, value);
}

bool MaterialParser::getDepthWriteSet(bool* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialDepthWriteSet, value);
}

bool MaterialParser::getDepthWrite(bool* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialDepthWrite, value);
}

bool MaterialParser::getDoubleSidedSet(bool* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialDoubleSidedSet, value);
}

bool MaterialParser::getDoubleSided(bool* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialDoubleSided, value);
}

bool MaterialParser::getColorWrite(bool* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialColorWrite, value);
}

bool MaterialParser::getDepthTest(bool* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialDepthTest, value);
}

bool MaterialParser::getCullingMode(driver::CullingMode* value) const noexcept {
    static_assert(sizeof(driver::CullingMode) == sizeof(uint8_t),
            "CullingMode expected size is wrong");
    return mImpl->getFromSimpleChunk(ChunkType::MaterialCullingMode, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getTransparencyMode(TransparencyMode* value) const noexcept {
    static_assert(sizeof(TransparencyMode) == sizeof(uint8_t),
            "TransparencyMode expected size is wrong");
    return mImpl->getFromSimpleChunk(ChunkType::MaterialTransparencyMode,
            reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getInterpolation(Interpolation* value) const noexcept {
    static_assert(sizeof(Interpolation) == sizeof(uint8_t),
            "Interpolation expected size is wrong");
    return mImpl->getFromSimpleChunk(ChunkType::MaterialInterpolation, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getVertexDomain(VertexDomain* value) const noexcept {
    static_assert(sizeof(VertexDomain) == sizeof(uint8_t),
            "VertexDomain expected size is wrong");
    return mImpl->getFromSimpleChunk(ChunkType::MaterialVertexDomain, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getBlendingMode(BlendingMode* value) const noexcept {
    static_assert(sizeof(BlendingMode) == sizeof(uint8_t),
            "BlendingMode expected size is wrong");
    return mImpl->getFromSimpleChunk(ChunkType::MaterialBlendingMode, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::getMaskThreshold(float* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialMaskThreshold, value);
}

bool MaterialParser::hasShadowMultiplier(bool* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialShadowMultiplier, value);
}

bool MaterialParser::getShading(Shading* value) const noexcept {
    static_assert(sizeof(Shading) == sizeof(uint8_t),
            "Shading expected size is wrong");
    return mImpl->getFromSimpleChunk(ChunkType::MaterialShading, reinterpret_cast<uint8_t*>(value));
}

bool MaterialParser::hasCustomDepthShader(bool* value) const noexcept {
    return mImpl->getFromSimpleChunk(ChunkType::MaterialHasCustomDepthShader, value);
}

bool MaterialParser::getRequiredAttributes(AttributeBitset* value) const noexcept {
    uint32_t rawAttributes = 0;
    if (!mImpl->getFromSimpleChunk(ChunkType::MaterialRequiredAttributes, &rawAttributes)) {
        return false;
    }

    *value = AttributeBitset();
    value->setValue(rawAttributes);

    return true;
}

bool MaterialParser::getShader(
        filament::driver::ShaderModel shaderModel, uint8_t variant, filament::driver::ShaderType st,
        ShaderBuilder& shader) noexcept {
    return (mImpl->mBackend == filament::driver::Backend::VULKAN) ?
           mImpl->getVkShader(shaderModel, variant, st, shader) :
           mImpl->getGlShader(shaderModel, variant, st, shader);
}

bool MaterialParserDetails::getVkShader(filament::driver::ShaderModel shaderModel, uint8_t variant,
        filament::driver::ShaderType st, ShaderBuilder& shader) noexcept {

    ChunkContainer const& container = mChunkContainer;
    if (!container.hasChunk(ChunkType::MaterialSpirv) ||
        !container.hasChunk(ChunkType::DictionarySpirv)) {
        return false;
    }

    if (UTILS_UNLIKELY(mBlobDictionary.isEmpty())) {
        if (!SpirvDictionaryReader::unflatten(container, mBlobDictionary)) {
            return false;
        }
    }

    Unflattener unflattener(container, ChunkType::MaterialSpirv);
    return mMaterialChunk.getSpirvShader(unflattener, mBlobDictionary, shader, shaderModel, variant, st);
}

bool MaterialParserDetails::getGlShader(filament::driver::ShaderModel shaderModel, uint8_t variant,
        filament::driver::ShaderType st, ShaderBuilder& shader) noexcept {

    ChunkContainer const& container = mChunkContainer;
    if (!container.hasChunk(ChunkType::MaterialGlsl) ||
        !container.hasChunk(ChunkType::DictionaryGlsl)) {
        return false;
    }

    // Read the dictionary only if it has not been read yet.
    if (UTILS_UNLIKELY(mBlobDictionary.isEmpty())) {
        if (!TextDictionaryReader::unflatten(container, mBlobDictionary)) {
            return false;
        }
    }

    Unflattener unflattener(container, ChunkType::MaterialGlsl);
    return mMaterialChunk.getTextShader(unflattener, mBlobDictionary, shader, shaderModel, variant, st);
}

} // namespace filaflat
