/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <matdbg/ShaderReplacer.h>

#include <backend/DriverEnums.h>

#include <filamat/MaterialBuilder.h>

#include <filaflat/DictionaryReader.h>
#include <filaflat/MaterialChunk.h>

#include <utils/Log.h>

#include <sstream>

#include <GlslangToSpv.h>

#include "sca/builtinResource.h"
#include "sca/GLSLTools.h"

#include "eiff/ChunkContainer.h"
#include "eiff/DictionarySpirvChunk.h"
#include "eiff/DictionaryTextChunk.h"
#include "eiff/MaterialBinaryChunk.h"
#include "eiff/MaterialTextChunk.h"
#include "eiff/LineDictionary.h"

#include "spirv-tools/libspirv.h"

namespace filament::matdbg {

using namespace backend;
using namespace filaflat;
using namespace filamat;
using namespace glslang;
using namespace utils;

using std::ostream;
using std::stringstream;
using std::streampos;
using std::vector;

// Tiny database of shader text that can import / export MaterialTextChunk and DictionaryTextChunk.
class ShaderIndex {
public:
    ShaderIndex(ChunkType dictTag, ChunkType matTag, const filaflat::ChunkContainer& cc);

    void writeChunks(ostream& stream);

    // Replaces the specified shader text with new content.
    void replaceShader(backend::ShaderModel model, Variant variant,
            ShaderStage stage, const char* source, size_t sourceLength);

    bool isEmpty() const { return mShaderRecords.size() == 0; }

private:
    const ChunkType mDictTag;
    const ChunkType mMatTag;
    vector<TextEntry> mShaderRecords;
};

// Tiny database of data blobs that can import / export MaterialBinaryChunk and DictionarySpirvChunk.
// The blobs are stored *after* they have been compressed by SMOL-V.
class BlobIndex {
public:
    BlobIndex(ChunkType dictTag, ChunkType matTag, const filaflat::ChunkContainer& cc);

    void writeChunks(ostream& stream);

    // Replaces the specified shader with new content.
    void replaceShader(backend::ShaderModel shaderModel, Variant variant,
            ShaderStage stage, const char* source, size_t sourceLength);

    bool isEmpty() const { return mDataBlobs.size() == 0 && mShaderRecords.size() == 0; }

private:
    const ChunkType mDictTag;
    const ChunkType mMatTag;
    vector<BinaryEntry> mShaderRecords;
    filaflat::BlobDictionary mDataBlobs;
};

ShaderReplacer::ShaderReplacer(Backend backend, const void* data, size_t size) :
        mBackend(backend), mOriginalPackage(data, size) {
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
            break;
    }
}

ShaderReplacer::~ShaderReplacer() {
    delete mEditedPackage;
}

bool ShaderReplacer::replaceShaderSource(ShaderModel shaderModel, Variant variant,
            ShaderStage stage, const char* sourceString, size_t stringLength) {
    if (!mOriginalPackage.parse()) {
        return false;
    }

    filaflat::ChunkContainer const& cc = mOriginalPackage;
    if (!cc.hasChunk(mMaterialTag) || !cc.hasChunk(mDictionaryTag)) {
        return false;
    }

    if (mDictionaryTag == ChunkType::DictionarySpirv) {
        return replaceSpirv(shaderModel, variant, stage, sourceString, stringLength);
    }

    // Clone all chunks except Dictionary* and Material*.
    stringstream sstream(std::string((const char*) cc.getData(), cc.getSize()));
    stringstream tstream;
    {
        uint64_t type;
        uint32_t size;
        vector<uint8_t> content;
        while (sstream) {
            sstream.read((char*) &type, sizeof(type));
            sstream.read((char*) &size, sizeof(size));
            content.resize(size);
            sstream.read((char*) content.data(), size);
            if (ChunkType(type) == mDictionaryTag|| ChunkType(type) == mMaterialTag) {
                continue;
            }
            tstream.write((char*) &type, sizeof(type));
            tstream.write((char*) &size, sizeof(size));
            tstream.write((char*) content.data(), size);
        }
    }

    // Append the new chunks for Dictionary* and Material*.
    if (ShaderIndex shaderIndex(mDictionaryTag, mMaterialTag, cc); !shaderIndex.isEmpty()) {
        shaderIndex.replaceShader(shaderModel, variant, stage, sourceString, stringLength);
        shaderIndex.writeChunks(tstream);
    }

    // Copy the new package from the stringstream into a ChunkContainer.
    // The memory gets freed by DebugServer, which has ownership over the material package.
    const size_t size = tstream.str().size();
    uint8_t* data = new uint8_t[size];
    memcpy(data, tstream.str().data(), size);

    assert_invariant(mEditedPackage == nullptr);
    mEditedPackage = new filaflat::ChunkContainer(data, size);

    return true;
}

bool ShaderReplacer::replaceSpirv(ShaderModel shaderModel, Variant variant,
            ShaderStage stage, const char* source, size_t sourceLength) {
    assert_invariant(mMaterialTag == ChunkType::MaterialSpirv);

    auto getShaderStage = [](ShaderStage type) {
        switch (type) {
            case ShaderStage::VERTEX:   return EShLanguage::EShLangVertex;
            case ShaderStage::FRAGMENT: return EShLanguage::EShLangFragment;
            case ShaderStage::COMPUTE:  return EShLanguage::EShLangCompute;
        }
    };

    MaterialBuilder::TargetApi const targetApi = targetApiFromBackend(mBackend);
    assert_invariant(targetApi == MaterialBuilder::TargetApi::VULKAN);

    // Unfortunately we need to use std::vector to interface with glslang.
    vector<unsigned int> spirv;

    const std::string_view src{ source, sourceLength };
    if (!src.compare(0, 8, "; SPIR-V")) {
        // we're receiving disassembled spirv
        spv_binary binary;
        spv_diagnostic diagnostic = nullptr;
        spv_context context = spvContextCreate(SPV_ENV_VULKAN_1_1);
        spv_result_t const error = spvTextToBinaryWithOptions(context, source, sourceLength,
                SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS, &binary, &diagnostic);
        spvContextDestroy(context);
        if (error) {
            slog.e << "[matdbg] ShaderReplacer spirv-as failed (spv_result_t: " << error << ")" << io::endl;
            spvDiagnosticPrint(diagnostic);
            spvDiagnosticDestroy(diagnostic);
            return false;
        }
        spirv.insert(spirv.end(), binary->code, binary->code + binary->wordCount);
        spvBinaryDestroy(binary);

    } else {
        // we're receiving glsl

        std::string const nullTerminated(source, sourceLength);
        source = nullTerminated.c_str();

        const EShLanguage shLang = getShaderStage(stage);
        TShader tShader(shLang);
        tShader.setStrings(&source, 1);

        const int version = GLSLTools::getGlslDefaultVersion(shaderModel);
        const EShMessages msg = GLSLTools::glslangFlagsFromTargetApi(targetApi,
                MaterialBuilder::TargetLanguage::SPIRV);

        GLSLTools::prepareShaderParser(targetApi,
                MaterialBuilder::TargetLanguage::SPIRV, tShader, shLang, version);

        const bool ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
        if (!ok) {
            slog.e << "[matdbg] ShaderReplacer parse:\n" << tShader.getInfoLog() << io::endl;
            return false;
        }

        TProgram program;
        program.addShader(&tShader);
        const bool linkOk = program.link(msg);
        if (!linkOk) {
            slog.e << "[matdbg] ShaderReplacer link:\n" << program.getInfoLog() << io::endl;
            return false;
        }

        SpvOptions options;
        options.generateDebugInfo = true;
        GlslangToSpv(*tShader.getIntermediate(), spirv, &options);
    }

    source = (const char*) spirv.data();
    sourceLength = spirv.size() * 4;

    slog.i << "[matdbg] Success re-generating SPIR-V. (" << sourceLength << " bytes)" << io::endl;

    // Clone all chunks except Dictionary* and Material*.
    filaflat::ChunkContainer const& cc = mOriginalPackage;
    stringstream sstream(std::string((const char*) cc.getData(), cc.getSize()));
    stringstream tstream;
    {
        uint64_t type;
        uint32_t size;
        vector<uint8_t> content;
        while (sstream) {
            sstream.read((char*) &type, sizeof(type));
            sstream.read((char*) &size, sizeof(size));
            content.resize(size);
            sstream.read((char*) content.data(), size);
            if (ChunkType(type) == mDictionaryTag || ChunkType(type) == mMaterialTag) {
                continue;
            }
            tstream.write((char*) &type, sizeof(type));
            tstream.write((char*) &size, sizeof(size));
            tstream.write((char*) content.data(), size);
        }
    }

    // Append the new chunks for Dictionary* and Material*.
    if (BlobIndex shaderIndex(mDictionaryTag, mMaterialTag, cc); !shaderIndex.isEmpty()) {
        shaderIndex.replaceShader(shaderModel, variant, stage, source, sourceLength);
        shaderIndex.writeChunks(tstream);
    }

    // Copy the new package from the stringstream into a ChunkContainer.
    // The memory gets freed by DebugServer, which has ownership over the material package.
    const size_t size = tstream.str().size();
    uint8_t* data = new uint8_t[size];
    memcpy(data, tstream.str().data(), size);

    assert_invariant(mEditedPackage == nullptr);
    mEditedPackage = new filaflat::ChunkContainer(data, size);

    return true;
}

const uint8_t* ShaderReplacer::getEditedPackage() const {
    return  (const uint8_t*) mEditedPackage->getData();
}

size_t ShaderReplacer::getEditedSize() const {
    return mEditedPackage->getSize();
}

ShaderIndex::ShaderIndex(ChunkType dictTag, ChunkType matTag, const filaflat::ChunkContainer& cc) :
        mDictTag(dictTag), mMatTag(matTag) {

    assert_invariant(matTag != filamat::ChunkType::MaterialSpirv);

    filaflat::BlobDictionary stringBlobs;
    DictionaryReader reader;
    reader.unflatten(cc, dictTag, stringBlobs);

    filaflat::MaterialChunk matChunk(cc);
    matChunk.initialize(matTag);

    matChunk.visitShaders([this, &matChunk, &stringBlobs]
            (ShaderModel shaderModel, Variant variant, ShaderStage stage) {
                ShaderContent content;
                UTILS_UNUSED_IN_RELEASE bool success = matChunk.getShader(content,
                        stringBlobs, shaderModel, variant, stage);

                std::string source{ content.data(), content.data() + content.size() - 1u };
                assert_invariant(success);

                mShaderRecords.push_back({ shaderModel, variant, stage, std::move(source) });
            });
}

void ShaderIndex::writeChunks(ostream& stream) {
    filamat::LineDictionary lines;
    for (const auto& record : mShaderRecords) {
        lines.addText(record.shader);
    }

    filamat::ChunkContainer cc;
    const auto& dchunk = cc.push<DictionaryTextChunk>(std::move(lines), mDictTag);
    cc.push<MaterialTextChunk>(std::move(mShaderRecords), dchunk.getDictionary(), mMatTag);

    const size_t bufSize = cc.getSize();
    auto buffer = std::make_unique<uint8_t[]>(bufSize);
    Flattener writer(buffer.get());
    UTILS_UNUSED_IN_RELEASE const size_t written = cc.flatten(writer);
    assert_invariant(written == bufSize);
    stream.write((char*)buffer.get(), bufSize);
}

void ShaderIndex::replaceShader(backend::ShaderModel model, Variant variant,
            backend::ShaderStage stage, const char* source, size_t sourceLength) {
    for (auto& record : mShaderRecords) {
        if (record.shaderModel == model && record.variant == variant &&
                record.stage == stage) {
            record.shader = std::string(source, sourceLength);
            return;
        }
    }
    slog.e << "[matdbg] Failed to replace shader." << io::endl;
}

BlobIndex::BlobIndex(ChunkType dictTag, ChunkType matTag, const filaflat::ChunkContainer& cc) :
        mDictTag(dictTag), mMatTag(matTag) {
    // Decompress SMOL-V.
    DictionaryReader reader;
    reader.unflatten(cc, mDictTag, mDataBlobs);

    filaflat::MaterialChunk matChunk(cc);
    matChunk.initialize(matTag);

    const auto& offsets = matChunk.getOffsets();
    mShaderRecords.reserve(offsets.size());
    for (auto [key, offset] : offsets) {
        BinaryEntry info;
        filaflat::MaterialChunk::decodeKey(key, &info.shaderModel, &info.variant, &info.stage);
        info.dictionaryIndex = offset;
        mShaderRecords.emplace_back(info);
    }
}

void BlobIndex::writeChunks(ostream& stream) {
    // Convert the filaflat dictionary into a filamat dictionary.
    filamat::BlobDictionary blobs;
    for (auto& record : mShaderRecords) {
        const auto& src = mDataBlobs[record.dictionaryIndex];
        assert(src.size() % 4 == 0);
        const uint32_t* ptr = (const uint32_t*) src.data();
        record.dictionaryIndex = blobs.addBlob(vector<uint8_t>(ptr, ptr + src.size()));
    }

    // Adjust start cursor of flatteners to match alignment of output stream.
    const size_t pad = stream.tellp() % 8;
    const auto initialize = [pad](Flattener& f) {
        for (size_t i = 0; i < pad; i++) {
            f.writeUint8(0);
        }
    };

    // Apply SMOL-V compression and write out the results.
    filamat::ChunkContainer cc;
    cc.push<MaterialBinaryChunk>(std::move(mShaderRecords), ChunkType::MaterialSpirv);
    cc.push<DictionarySpirvChunk>(std::move(blobs), false);

    Flattener prepass = Flattener::getDryRunner();
    initialize(prepass);

    const size_t bufSize = cc.flatten(prepass);
    auto buffer = std::make_unique<uint8_t[]>(bufSize);
    assert_invariant(intptr_t(buffer.get()) % 8 == 0);

    Flattener writer(buffer.get());
    initialize(writer);
    UTILS_UNUSED_IN_RELEASE const size_t written = cc.flatten(writer);

    assert_invariant(written == bufSize);
    stream.write((char*)buffer.get() + pad, bufSize - pad);
}

void BlobIndex::replaceShader(ShaderModel model, Variant variant,
            ShaderStage stage, const char* source, size_t sourceLength) {
    for (auto& record : mShaderRecords) {
        if (record.shaderModel == model && record.variant == variant && record.stage == stage) {
            // TODO: because a single blob entry might be used by more than one variant, matdbg
            // users may unwittingly edit more than 1 variant when multiple variants have the exact
            // same content before the edit. In practice this is rarely problematic, but we should
            // perhaps fix this one day.

            auto& blob = mDataBlobs[record.dictionaryIndex];
            blob.reserve(sourceLength);
            blob.resize(sourceLength);
            memcpy(blob.data(), source, sourceLength);

            return;
        }
    }
    slog.e << "[matdbg] Unable to replace shader." << io::endl;
}

} // namespace filament::matdbg
