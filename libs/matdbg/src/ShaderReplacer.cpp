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

#include <utils/Log.h>

#include <tsl/robin_map.h>

#include <sstream>

#include <string.h>

namespace filament {
namespace matdbg {

using namespace backend;
using namespace filaflat;
using namespace std;
using namespace tsl;
using namespace utils;

using filamat::ChunkType;

// Utility class for managing a pair of lists: a list of shader records, and a list of string lines
// that can be encoded into an index list. Also knows how to read & write the relevant IFF chunks.
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
            backend::ShaderType stage, const char* source, size_t sourceLength);

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

bool ShaderReplacer::replaceShaderSource(backend::ShaderModel shaderModel, uint8_t variant,
            backend::ShaderType stage, const char* source, size_t sourceLength) {
    if (!mOriginalPackage.parse()) {
        return false;
    }

    ChunkContainer const& cc = mOriginalPackage;
    if (!cc.hasChunk(mMaterialTag) || !cc.hasChunk(mDictionaryTag)) {
        return false;
    }

    if (mDictionaryTag == ChunkType::DictionarySpirv) {
        slog.e << "SPIR-V editing is not yet supported." << io::endl;
        return false;
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
    const size_t size = tstream.str().size();
    uint8_t* data = new uint8_t[size];
    memcpy(data, tstream.str().data(), size);
    mEditedPackage = new filaflat::ChunkContainer(data, size);
    return true;
}

const uint8_t* ShaderReplacer::getEditedPackage() const {
    return  (const uint8_t*) mEditedPackage->getData();
}

size_t ShaderReplacer::getEditedSize() const {
    return  mEditedPackage->getSize();
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


} // namespace matdbg
} // namespace filament
