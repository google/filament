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

#include <utils/Log.h>

#include <tsl/robin_map.h>

#include <sstream>

#include <GlslangToSpv.h>

#include <smolv.h>

#include "sca/builtinResource.h"
#include "sca/GLSLTools.h"

namespace filament {
namespace matdbg {

using namespace backend;
using namespace filaflat;
using namespace filamat;
using namespace glslang;
using namespace std;
using namespace tsl;
using namespace utils;

// Tiny database of shader text that can import / export MaterialTextChunk and DictionaryTextChunk.
class ShaderIndex {
public:
    // Consumes a chunk and builds the string list.
    void addStringLines(const uint8_t* chunkContent, size_t size);

    // Consumes a chunk and builds the shader records.
    void addShaderRecords(const uint8_t* chunkContent, size_t size);

    // Produces a chunk holding the string list.
    void writeLinesChunk(ChunkType tag, ostream& stream) const;

    // Produces a chunk holding the shader records.
    void writeShadersChunk(ChunkType tag, ostream& stream) const;

    // Replaces the specified shader text with new content.
    void replaceShader(backend::ShaderModel shaderModel, uint8_t variant,
            ShaderType stage, const char* source, size_t sourceLength);

    bool isEmpty() const { return mStringLines.size() == 0 && mShaderRecords.size() == 0; }

private:
    struct ShaderRecord {
        uint8_t model;
        uint8_t variant;
        uint8_t stage;
        uint32_t offset;
        vector<uint16_t> lineIndices;
        string decodedShaderText;
        uint32_t stringLength;
    };

    void decodeShadersFromIndices();
    void encodeShadersToIndices();

    vector<ShaderRecord> mShaderRecords;
    vector<string> mStringLines;
};

// Tiny database of data blobs that can import / export MaterialSpirvChunk and DictionarySpirvChunk.
// The blobs are stored *after* they have been compressed by SMOL-V.
class BlobIndex {
public:
    // Consumes a chunk and builds the blob list.
    void addDataBlobs(const uint8_t* chunkContent, size_t size);

    // Consumes a chunk and builds the shader records.
    void addShaderRecords(const uint8_t* chunkContent, size_t size);

    // Produces a chunk holding the blob list.
    void writeBlobsChunk(ChunkType tag, ostream& stream) const;

    // Produces a chunk holding the shader records.
    void writeShadersChunk(ChunkType tag, ostream& stream) const;

    // Replaces the specified shader with new content.
    void replaceShader(backend::ShaderModel shaderModel, uint8_t variant,
            ShaderType stage, const char* source, size_t sourceLength);

    bool isEmpty() const { return mDataBlobs.size() == 0 && mShaderRecords.size() == 0; }

private:
    struct ShaderRecord {
        uint8_t model;
        uint8_t variant;
        uint8_t stage;
        uint32_t blobIndex;
    };

    using SpirvBlob = vector<unsigned int>;

    vector<ShaderRecord> mShaderRecords;
    vector<SpirvBlob> mDataBlobs;
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

bool ShaderReplacer::replaceShaderSource(ShaderModel shaderModel, uint8_t variant,
            ShaderType stage, const char* source, size_t sourceLength) {
    if (!mOriginalPackage.parse()) {
        return false;
    }

    filaflat::ChunkContainer const& cc = mOriginalPackage;
    if (!cc.hasChunk(mMaterialTag) || !cc.hasChunk(mDictionaryTag)) {
        return false;
    }

    if (mDictionaryTag == ChunkType::DictionarySpirv) {
        return replaceSpirv(shaderModel, variant, stage, source, sourceLength);
    }

    // Clone all chunks except Dictionary* and Material*.
    stringstream sstream(string((const char*) cc.getData(), cc.getSize()));
    stringstream tstream;
    ShaderIndex shaderIndex;
    {
        uint64_t type;
        uint32_t size;
        vector<uint8_t> content;
        while (sstream) {
            sstream.read((char*) &type, sizeof(type));
            sstream.read((char*) &size, sizeof(size));
            content.resize(size);
            sstream.read((char*) content.data(), size);
            if (ChunkType(type) == mDictionaryTag) {
                shaderIndex.addStringLines(content.data(), size);
                continue;
            }
            if (ChunkType(type) == mMaterialTag) {
                shaderIndex.addShaderRecords(content.data(), size);
                continue;
            }
            tstream.write((char*) &type, sizeof(type));
            tstream.write((char*) &size, sizeof(size));
            tstream.write((char*) content.data(), size);
        }
    }

    // Append the new chunks for Dictionary* and Material*.
    if (!shaderIndex.isEmpty()) {
        shaderIndex.replaceShader(shaderModel, variant, stage, source, sourceLength);
        shaderIndex.writeLinesChunk(mDictionaryTag, tstream);
        shaderIndex.writeShadersChunk(mMaterialTag, tstream);
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

bool ShaderReplacer::replaceSpirv(ShaderModel shaderModel, uint8_t variant,
            ShaderType stage, const char* source, size_t sourceLength) {
    filaflat::ChunkContainer const& cc = mOriginalPackage;

    assert_invariant(mMaterialTag == ChunkType::MaterialSpirv);

    const EShLanguage shLang = stage == VERTEX ? EShLangVertex : EShLangFragment;

    std::string nullTerminated(source, sourceLength);
    source = nullTerminated.c_str();

    TShader tShader(shLang);
    tShader.setStrings(&source, 1);

    MaterialBuilder::TargetApi targetApi = targetApiFromBackend(mBackend);
    assert_invariant(targetApi == MaterialBuilder::TargetApi::VULKAN);

    const int langVersion = GLSLTools::glslangVersionFromShaderModel(shaderModel);
    const EShMessages msg = GLSLTools::glslangFlagsFromTargetApi(targetApi);
    const bool ok = tShader.parse(&DefaultTBuiltInResource, langVersion, false, msg);
    if (!ok) {
        slog.e << "ShaderReplacer parse:\n" << tShader.getInfoLog() << io::endl;
        return false;
    }

    TProgram program;
    program.addShader(&tShader);
    const bool linkOk = program.link(msg);
    if (!linkOk) {
        slog.e << "ShaderReplacer link:\n" << program.getInfoLog() << io::endl;
        return false;
    }

    // Unfortunately we need to use std::vector to interface with glslang.
    vector<unsigned int> spirv;

    SpvOptions options;
    options.generateDebugInfo = true;
    GlslangToSpv(*tShader.getIntermediate(), spirv, &options);

    source = (const char*) spirv.data();
    sourceLength = spirv.size() * 4;

    slog.i << "Success re-generating SPIR-V. (" << sourceLength << " bytes)" << io::endl;

    // Clone all chunks except Dictionary* and Material*.
    stringstream sstream(string((const char*) cc.getData(), cc.getSize()));
    stringstream tstream;
    BlobIndex shaderIndex;
    {
        uint64_t type;
        uint32_t size;
        vector<uint8_t> content;
        while (sstream) {
            sstream.read((char*) &type, sizeof(type));
            sstream.read((char*) &size, sizeof(size));
            content.resize(size);
            sstream.read((char*) content.data(), size);
            if (ChunkType(type) == mDictionaryTag) {
                shaderIndex.addDataBlobs(content.data(), size);
                continue;
            }
            if (ChunkType(type) == mMaterialTag) {
                shaderIndex.addShaderRecords(content.data(), size);
                continue;
            }
            tstream.write((char*) &type, sizeof(type));
            tstream.write((char*) &size, sizeof(size));
            tstream.write((char*) content.data(), size);
        }
    }

    // Append the new chunks for Dictionary* and Material*.
    if (!shaderIndex.isEmpty()) {
        shaderIndex.replaceShader(shaderModel, variant, stage, source, sourceLength);
        shaderIndex.writeBlobsChunk(mDictionaryTag, tstream);
        shaderIndex.writeShadersChunk(mMaterialTag, tstream);
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

void ShaderIndex::addStringLines(const uint8_t* chunkContent, size_t size) {
    uint32_t count = *((const uint32_t*) chunkContent);
    mStringLines.resize(count);
    const uint8_t* ptr = chunkContent + 4;
    for (uint32_t i = 0; i < count; i++) {
        mStringLines[i] = string((const char*) ptr);
        ptr += mStringLines[i].length() + 1;
    }
}

void ShaderIndex::addShaderRecords(const uint8_t* chunkContent, size_t size) {
    stringstream stream(string((const char*) chunkContent, size));
    uint64_t recordCount;
    stream.read((char*) &recordCount, sizeof(recordCount));
    mShaderRecords.resize(recordCount);
    for (auto& record : mShaderRecords) {
        stream.read((char*) &record.model, sizeof(ShaderRecord::model));
        stream.read((char*) &record.variant, sizeof(ShaderRecord::variant));
        stream.read((char*) &record.stage, sizeof(ShaderRecord::stage));
        stream.read((char*) &record.offset, sizeof(ShaderRecord::offset));

        const uint8_t* linePtr = chunkContent + record.offset;
        record.stringLength = *((uint32_t*) linePtr);
        linePtr += sizeof(uint32_t);

        const uint32_t lineCount = *((uint32_t*) linePtr);
        record.lineIndices.resize(lineCount);
        linePtr += sizeof(uint32_t);

        memcpy(record.lineIndices.data(), linePtr, lineCount * sizeof(uint16_t));
    }
}

void ShaderIndex::writeLinesChunk(ChunkType tag, ostream& stream) const {
    // First perform a prepass to compute chunk size.
    uint32_t size = sizeof(uint32_t);
    for (const auto& stringLine : mStringLines) {
        size += stringLine.length() + 1;
    }

    // Serialize the chunk.
    uint64_t type = tag;
    stream.write((char*) &type, sizeof(type));
    stream.write((char*) &size, sizeof(size));
    uint32_t count = mStringLines.size();
    stream.write((char*) &count, sizeof(count));
    for (const auto& stringLine : mStringLines) {
        stream.write(stringLine.c_str(), stringLine.length() + 1);
    }
}

void ShaderIndex::writeShadersChunk(ChunkType tag, ostream& stream) const {
    // First perform a prepass to compute chunk size.
    uint32_t size = sizeof(uint64_t);
    for (const auto& record : mShaderRecords) {
        size += sizeof(ShaderRecord::model);
        size += sizeof(ShaderRecord::variant);
        size += sizeof(ShaderRecord::stage);
        size += sizeof(ShaderRecord::offset);
    }
    for (const auto& record : mShaderRecords) {
        size += sizeof(ShaderRecord::stringLength);
        size += sizeof(uint32_t);
        size += record.lineIndices.size() * sizeof(uint16_t);
    }

    // Serialize the chunk.
    uint64_t type = tag;
    stream.write((char*) &type, sizeof(type));
    stream.write((char*) &size, sizeof(size));
    uint64_t recordCount = mShaderRecords.size();
    stream.write((char*) &recordCount, sizeof(recordCount));
    for (const auto& record : mShaderRecords) {
        stream.write((char*) &record.model, sizeof(ShaderRecord::model));
        stream.write((char*) &record.variant, sizeof(ShaderRecord::variant));
        stream.write((char*) &record.stage, sizeof(ShaderRecord::stage));
        stream.write((char*) &record.offset, sizeof(ShaderRecord::offset));
    }
    for (const auto& record : mShaderRecords) {
        uint32_t lineCount = record.lineIndices.size();
        stream.write((char*) &record.stringLength, sizeof(ShaderRecord::stringLength));
        stream.write((char*) &lineCount, sizeof(lineCount));
        stream.write((char*) record.lineIndices.data(), lineCount * sizeof(uint16_t));
    }
}

void ShaderIndex::replaceShader(backend::ShaderModel shaderModel, uint8_t variant,
            backend::ShaderType stage, const char* source, size_t sourceLength) {
    decodeShadersFromIndices();
    const uint8_t model = (uint8_t) shaderModel;
    for (auto& record : mShaderRecords) {
        if (record.model == model && record.variant == variant && record.stage == stage) {
            record.decodedShaderText = std::string(source, sourceLength);
            break;
        }
    }
    encodeShadersToIndices();
}

void ShaderIndex::decodeShadersFromIndices() {
    for (auto& record : mShaderRecords) {
        record.decodedShaderText.clear();
        for (uint16_t index : record.lineIndices) {
            if (index >= mStringLines.size()) {
                slog.e << "Internal chunk decoding error." << io::endl;
                return;
            }
            record.decodedShaderText += mStringLines[index] + "\n";
        }
    }
}

void ShaderIndex::encodeShadersToIndices() {
    robin_map<string, uint16_t> table;
    for (size_t i = 0; i < mStringLines.size(); i++) {
        table[mStringLines[i]] = uint16_t(i);
    }

    uint32_t offset = sizeof(uint64_t);
    for (const auto& record : mShaderRecords) {
        offset += sizeof(ShaderRecord::model);
        offset += sizeof(ShaderRecord::variant);
        offset += sizeof(ShaderRecord::stage);
        offset += sizeof(ShaderRecord::offset);
    }

    for (auto& record : mShaderRecords) {
        record.stringLength = record.decodedShaderText.length() + 1;
        record.lineIndices.clear();
        record.offset = offset;

        offset += sizeof(ShaderRecord::stringLength);
        offset += sizeof(uint32_t);

        const char* const start = record.decodedShaderText.c_str();
        const size_t length = record.decodedShaderText.length();
        for (size_t cur = 0; cur < length; cur++) {
            size_t pos = cur;
            size_t len = 0;
            while (start[cur] != '\n' && cur < length) {
                cur++;
                len++;
            }
            if (pos + len > length) {
                slog.e << "Internal chunk encoding error." << io::endl;
                return;
            }
            string newLine(start, pos, len);
            auto iter = table.find(newLine);
            if (iter == table.end()) {
                size_t index = mStringLines.size();
                if (index > UINT16_MAX) {
                    slog.e << "Chunk encoding error: too many unique codelines." << io::endl;
                    return;
                }
                record.lineIndices.push_back(index);
                table[newLine] = index;
                mStringLines.push_back(newLine);
                continue;
            }
            record.lineIndices.push_back(iter->second);
        }
        offset += sizeof(uint16_t) * record.lineIndices.size();
    }
}

void BlobIndex::addDataBlobs(const uint8_t* chunkContent, size_t size) {
    const uint32_t compression = *((const uint32_t*) chunkContent);
    chunkContent += 4;
    const uint32_t blobCount = *((const uint32_t*) chunkContent);
    chunkContent += 4;
    mDataBlobs.resize(blobCount);
    const uint8_t* ptr = chunkContent;
    for (uint32_t i = 0; i < blobCount; i++) {
        const uint64_t byteCount = *((const uint64_t*) ptr);
        ptr += sizeof(uint64_t);
        mDataBlobs[i].resize(byteCount);
        memcpy(mDataBlobs[i].data(), ptr, byteCount);
        ptr += byteCount;
    }
}

void BlobIndex::addShaderRecords(const uint8_t* chunkContent, size_t size) {
    stringstream stream(string((const char*) chunkContent, size));
    uint64_t recordCount;
    stream.read((char*) &recordCount, sizeof(recordCount));
    mShaderRecords.resize(recordCount);
    for (auto& record : mShaderRecords) {
        stream.read((char*) &record.model, sizeof(ShaderRecord::model));
        stream.read((char*) &record.variant, sizeof(ShaderRecord::variant));
        stream.read((char*) &record.stage, sizeof(ShaderRecord::stage));
        stream.read((char*) &record.blobIndex, sizeof(ShaderRecord::blobIndex));
    }
}

void BlobIndex::writeBlobsChunk(ChunkType tag, ostream& stream) const {
    // First perform a prepass to compute chunk size.
    uint32_t size = sizeof(uint32_t) + sizeof(uint32_t);
    for (const auto& blob : mDataBlobs) {
        size += sizeof(uint64_t);
        size += blob.size();
    }

    // Serialize the chunk.
    const uint64_t type = tag;
    stream.write((char*) &type, sizeof(type));
    stream.write((char*) &size, sizeof(size));
    const uint32_t compression = 1;
    stream.write((char*) &compression, sizeof(compression));
    const uint32_t count = mDataBlobs.size();
    stream.write((char*) &count, sizeof(count));
    for (const auto& blob : mDataBlobs) {
        const uint64_t byteCount = blob.size();
        stream.write((char*) &byteCount, sizeof(byteCount));
        stream.write((char*) blob.data(), blob.size());
    }
}

void BlobIndex::writeShadersChunk(ChunkType tag, ostream& stream) const {
    // First perform a prepass to compute chunk size.
    uint32_t size = sizeof(uint64_t);
    for (const auto& record : mShaderRecords) {
        size += sizeof(ShaderRecord::model);
        size += sizeof(ShaderRecord::variant);
        size += sizeof(ShaderRecord::stage);
        size += sizeof(ShaderRecord::blobIndex);
    }

    // Serialize the chunk.
    uint64_t type = tag;
    stream.write((char*) &type, sizeof(type));
    stream.write((char*) &size, sizeof(size));
    const uint64_t recordCount = mShaderRecords.size();
    stream.write((char*) &recordCount, sizeof(recordCount));
    for (const auto& record : mShaderRecords) {
        stream.write((char*) &record.model, sizeof(ShaderRecord::model));
        stream.write((char*) &record.variant, sizeof(ShaderRecord::variant));
        stream.write((char*) &record.stage, sizeof(ShaderRecord::stage));
        stream.write((char*) &record.blobIndex, sizeof(ShaderRecord::blobIndex));
    }
}

void BlobIndex::replaceShader(ShaderModel shaderModel, uint8_t variant,
            ShaderType stage, const char* source, size_t sourceLength) {
    smolv::ByteArray compressed;
    if (!smolv::Encode(source, sourceLength, compressed, 0)) {
        utils::slog.e << "Error with SPIRV compression" << utils::io::endl;
        return;
    }
    const uint8_t model = (uint8_t) shaderModel;
    for (auto& record : mShaderRecords) {
        if (record.model == model && record.variant == variant && record.stage == stage) {
            auto& blob = mDataBlobs[record.blobIndex];
            blob.resize(compressed.size());
            memcpy(blob.data(), compressed.data(), compressed.size());
            break;
        }
    }
}

} // namespace matdbg
} // namespace filament
