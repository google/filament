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

#ifndef TNT_FILAMENT_MATERIALPARSER_H
#define TNT_FILAMENT_MATERIALPARSER_H

#include <filaflat/ChunkContainer.h>
#include <filaflat/MaterialChunk.h>

#include <filament/MaterialEnums.h>
#include <filament/MaterialChunkType.h>

#include "../../libs/filamat/src/SamplerBindingMap.h"
#include <private/filament/Variant.h>

#include <backend/DriverEnums.h>
#include <backend/Program.h>

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>
#include <utils/compiler.h>

#include <inttypes.h>
#include <utility>

namespace filaflat {
class ChunkContainer;
class Unflattener;
}

namespace filament {

class BufferInterfaceBlock;
class SamplerInterfaceBlock;
struct SubpassInfo;
struct MaterialConstant;
struct MaterialPushConstant;

class MaterialParser {
public:
    MaterialParser(utils::FixedCapacityVector<backend::ShaderLanguage> preferredLanguages,
            const void* data, size_t size);

    MaterialParser(MaterialParser const& rhs) noexcept = delete;
    MaterialParser& operator=(MaterialParser const& rhs) noexcept = delete;

    enum class ParseResult {
        SUCCESS,
        ERROR_MISSING_BACKEND,
        ERROR_OTHER
    };

    ParseResult parse() noexcept;
    backend::ShaderLanguage getShaderLanguage() const noexcept;

    // Accessors
    bool getMaterialVersion(uint32_t* value) const noexcept;
    bool getFeatureLevel(uint8_t* value) const noexcept;
    bool getName(utils::CString*) const noexcept;
    bool getCacheId(uint64_t* cacheId) const noexcept;
    bool getUIB(BufferInterfaceBlock* uib) const noexcept;
    bool getSIB(SamplerInterfaceBlock* sib) const noexcept;
    bool getSubpasses(SubpassInfo* subpass) const noexcept;
    bool getShaderModels(uint32_t* value) const noexcept;
    bool getMaterialProperties(uint64_t* value) const noexcept;
    bool getUniformBlockBindings(utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>>* value) const noexcept;
    bool getSamplerBlockBindings(SamplerGroupBindingInfoList* pSamplerGroupInfoList,
            SamplerBindingToNameMap* pSamplerBindingToNameMap) const noexcept;
    bool getConstants(utils::FixedCapacityVector<MaterialConstant>* value) const noexcept;
    bool getPushConstants(utils::CString* structVarName,
            utils::FixedCapacityVector<MaterialPushConstant>* value) const noexcept;

    using BindingUniformInfoContainer = utils::FixedCapacityVector<
            std::pair<filament::UniformBindingPoints, backend::Program::UniformInfo>>;

    bool getBindingUniformInfo(BindingUniformInfoContainer* container) const noexcept;

    using AttributeInfoContainer = utils::FixedCapacityVector<
            std::pair<utils::CString, uint8_t>>;

    bool getAttributeInfo(AttributeInfoContainer* container) const noexcept;

    bool getDepthWriteSet(bool* value) const noexcept;
    bool getDepthWrite(bool* value) const noexcept;
    bool getDoubleSidedSet(bool* value) const noexcept;
    bool getDoubleSided(bool* value) const noexcept;
    bool getCullingMode(backend::CullingMode* value) const noexcept;
    bool getTransparencyMode(TransparencyMode* value) const noexcept;
    bool getColorWrite(bool* value) const noexcept;
    bool getDepthTest(bool* value) const noexcept;
    bool getInstanced(bool* value) const noexcept;
    bool getInterpolation(Interpolation* value) const noexcept;
    bool getVertexDomain(VertexDomain* value) const noexcept;
    bool getMaterialDomain(MaterialDomain* domain) const noexcept;
    bool getMaterialVariantFilterMask(UserVariantFilterMask* userVariantFilterMask) const noexcept;

    bool getShading(Shading*) const noexcept;
    bool getBlendingMode(BlendingMode*) const noexcept;
    bool getCustomBlendFunction(std::array<backend::BlendFunction, 4>*) const noexcept;
    bool getMaskThreshold(float*) const noexcept;
    bool getAlphaToCoverageSet(bool*) const noexcept;
    bool getAlphaToCoverage(bool*) const noexcept;
    bool hasShadowMultiplier(bool*) const noexcept;
    bool getRequiredAttributes(AttributeBitset*) const noexcept;
    bool getRefractionMode(RefractionMode* value) const noexcept;
    bool getRefractionType(RefractionType* value) const noexcept;
    bool getReflectionMode(ReflectionMode* value) const noexcept;
    bool hasCustomDepthShader(bool* value) const noexcept;
    bool hasSpecularAntiAliasing(bool* value) const noexcept;
    bool getSpecularAntiAliasingVariance(float* value) const noexcept;
    bool getSpecularAntiAliasingThreshold(float* value) const noexcept;
    bool getStereoscopicType(backend::StereoscopicType*) const noexcept;

    bool getShader(filaflat::ShaderContent& shader, backend::ShaderModel shaderModel,
            Variant variant, backend::ShaderStage stage) noexcept;

    bool hasShader(backend::ShaderModel model,
            Variant variant, backend::ShaderStage stage) const noexcept {
        return getMaterialChunk().hasShader(model, variant, stage);
    }

    filaflat::MaterialChunk const& getMaterialChunk() const noexcept {
        return mImpl.mMaterialChunk;
    }

private:
    struct MaterialParserDetails {
        MaterialParserDetails(
                const utils::FixedCapacityVector<backend::ShaderLanguage>& preferredLanguages,
                const void* data, size_t size);

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
        utils::FixedCapacityVector<backend::ShaderLanguage> mPreferredLanguages;
        backend::ShaderLanguage mChosenLanguage;

        // Keep MaterialChunk alive between calls to getShader to avoid reload the shader index.
        filaflat::MaterialChunk mMaterialChunk;
        filaflat::BlobDictionary mBlobDictionary;
    };

    filaflat::ChunkContainer& getChunkContainer() noexcept;
    filaflat::ChunkContainer const& getChunkContainer() const noexcept;
    MaterialParserDetails mImpl;
};

struct ChunkUniformInterfaceBlock {
    static bool unflatten(filaflat::Unflattener& unflattener, BufferInterfaceBlock* uib);
};

struct ChunkSamplerInterfaceBlock {
    static bool unflatten(filaflat::Unflattener& unflattener, SamplerInterfaceBlock* sib);
};

struct ChunkSubpassInterfaceBlock {
    static bool unflatten(filaflat::Unflattener& unflattener, SubpassInfo* sib);
};

struct ChunkUniformBlockBindings {
    static bool unflatten(filaflat::Unflattener& unflattener,
            utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>>* uniformBlockBindings);
};

struct ChunkBindingUniformInfo {
    static bool unflatten(filaflat::Unflattener& unflattener,
            MaterialParser::BindingUniformInfoContainer* bindingUniformInfo);
};

struct ChunkAttributeInfo {
    static bool unflatten(filaflat::Unflattener& unflattener,
            MaterialParser::AttributeInfoContainer* attributeInfoContainer);
};

struct ChunkSamplerBlockBindings {
    static bool unflatten(filaflat::Unflattener& unflattener,
            SamplerGroupBindingInfoList* pSamplerGroupBindingInfoList,
            SamplerBindingToNameMap* pSamplerBindingToNameMap);
};

struct ChunkMaterialConstants {
    static bool unflatten(filaflat::Unflattener& unflattener,
            utils::FixedCapacityVector<MaterialConstant>* materialConstants);
};

struct ChunkMaterialPushConstants {
    static bool unflatten(filaflat::Unflattener& unflattener, utils::CString* structVarName,
            utils::FixedCapacityVector<MaterialPushConstant>* materialPushConstants);
};

} // namespace filament

#endif // TNT_FILAMENT_MATERIALPARSER_H
