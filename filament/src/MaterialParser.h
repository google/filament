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

#ifndef TNT_FILAMENT_MATERIAL_PARSER_H
#define TNT_FILAMENT_MATERIAL_PARSER_H

#include <filaflat/BlobDictionary.h>
#include <filaflat/ChunkContainer.h>
#include <filaflat/MaterialChunk.h>

#include <filament/MaterialEnums.h>
#include <backend/DriverEnums.h>
#include <filament/MaterialChunkType.h>

#include <utils/compiler.h>
#include <utils/CString.h>

#include <inttypes.h>

namespace filaflat {
class ChunkContainer;
class ShaderBuilder;
class Unflattener;
}

namespace filament {

class UniformInterfaceBlock;
class SamplerInterfaceBlock;

class MaterialParser {
public:
    MaterialParser(backend::Backend backend, const void* data, size_t size);

    MaterialParser(MaterialParser const& rhs) noexcept = delete;
    MaterialParser& operator=(MaterialParser const& rhs) noexcept = delete;

    bool parse() noexcept;

    // Accessors
    bool getMaterialVersion(uint32_t* value) const noexcept;
    bool getName(utils::CString*) const noexcept;
    bool getUIB(UniformInterfaceBlock* uib) const noexcept;
    bool getSIB(SamplerInterfaceBlock* sib) const noexcept;
    bool getShaderModels(uint32_t* value) const noexcept;
    bool getMaterialProperties(uint64_t* value) const noexcept;

    bool getDepthWriteSet(bool* value) const noexcept;
    bool getDepthWrite(bool* value) const noexcept;
    bool getDoubleSidedSet(bool* value) const noexcept;
    bool getDoubleSided(bool* value) const noexcept;
    bool getCullingMode(backend::CullingMode* value) const noexcept;
    bool getTransparencyMode(TransparencyMode* value) const noexcept;
    bool getColorWrite(bool* value) const noexcept;
    bool getDepthTest(bool* value) const noexcept;
    bool getInterpolation(Interpolation* value) const noexcept;
    bool getVertexDomain(VertexDomain* value) const noexcept;
    bool getMaterialDomain(MaterialDomain* domain) const noexcept;

    bool getShading(Shading*) const noexcept;
    bool getBlendingMode(BlendingMode*) const noexcept;
    bool getMaskThreshold(float*) const noexcept;
    bool hasShadowMultiplier(bool*) const noexcept;
    bool getRequiredAttributes(AttributeBitset*) const noexcept;
    bool getRefractionMode(RefractionMode* value) const noexcept;
    bool getRefractionType(RefractionType* value) const noexcept;
    bool hasCustomDepthShader(bool* value) const noexcept;
    bool hasSpecularAntiAliasing(bool* value) const noexcept;
    bool getSpecularAntiAliasingVariance(float* value) const noexcept;
    bool getSpecularAntiAliasingThreshold(float* value) const noexcept;

    bool getShader(filaflat::ShaderBuilder& shader, backend::ShaderModel shaderModel,
            uint8_t variant, backend::ShaderType stage) noexcept;

private:
    struct MaterialParserDetails {
        MaterialParserDetails(backend::Backend backend, const void* data, size_t size);

        template<typename T>
        bool getFromSimpleChunk(filamat::ChunkType type, T* value) const noexcept;

    private:
        friend class MaterialParser;

        class ManagedBuffer {
            void* mStart = nullptr;
            size_t mSize = 0;
        public:
            explicit ManagedBuffer(const void* start, size_t size)
                    : mStart(malloc(size)), mSize(size) {
                memcpy(mStart, start, size);
            }
            ~ManagedBuffer() noexcept { free(mStart); }
            ManagedBuffer(ManagedBuffer const& rhs) = delete;
            ManagedBuffer& operator=(ManagedBuffer const& rhs) = delete;
            void* data() const noexcept { return mStart; }
            void* begin() const noexcept { return mStart; }
            void* end() const noexcept { return (uint8_t*)mStart + mSize; }
            size_t size() const noexcept { return mSize; }
        };

        ManagedBuffer mManagedBuffer;
        filaflat::ChunkContainer mChunkContainer;

        // Keep MaterialChunk alive between calls to getShader to avoid reload the shader index.
        filaflat::MaterialChunk mMaterialChunk;
        filaflat::BlobDictionary mBlobDictionary;
        filamat::ChunkType mMaterialTag = filamat::ChunkType::Unknown;
        filamat::ChunkType mDictionaryTag = filamat::ChunkType::Unknown;
    };

    filaflat::ChunkContainer& getChunkContainer() noexcept;
    filaflat::ChunkContainer const& getChunkContainer() const noexcept;
    MaterialParserDetails mImpl;
};

struct ChunkUniformInterfaceBlock {
    static bool unflatten(filaflat::Unflattener& unflattener, UniformInterfaceBlock* uib);
};

struct ChunkSamplerInterfaceBlock {
    static bool unflatten(filaflat::Unflattener& unflattener, SamplerInterfaceBlock* sib);
};

} // namespace filament

#endif // TNT_FILAMENT_MATERIAL_PARSER_H
