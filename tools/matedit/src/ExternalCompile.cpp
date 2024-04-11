/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "ExternalCompile.h"

#include "backend/DriverEnums.h"
#include "eiff/BlobDictionary.h"
#include "eiff/ChunkContainer.h"
#include "eiff/DictionaryMetalLibraryChunk.h"
#include "eiff/DictionarySpirvChunk.h"
#include "eiff/DictionaryTextChunk.h"
#include "eiff/LineDictionary.h"
#include "eiff/MaterialBinaryChunk.h"
#include "eiff/MaterialTextChunk.h"
#include "eiff/ShaderEntry.h"

#include <filaflat/ChunkContainer.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/MaterialChunk.h>

#include <matdbg/ShaderExtractor.h>
#include <matdbg/ShaderInfo.h>

#include <filamat/Package.h>

#include <fstream>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

class PassthroughChunk final : public filamat::Chunk {
public:
    explicit PassthroughChunk(const char* data, size_t size, filamat::ChunkType type)
        : filamat::Chunk(type), data(data), size(size) {}

    ~PassthroughChunk() = default;

private:
    void flatten(filamat::Flattener& f) override {
        f.writeRaw(data, size);
    }

    const char* data;
    size_t size;
};


using filamat::Package;
using filamat::Flattener;

namespace matedit {

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

static void dumpBinary(const uint8_t* data, size_t size, utils::Path filename) {
    std::ofstream out(filename, std::ofstream::binary);
    out.write(reinterpret_cast<const char*>(data), size);
}

static void dumpString(const std::string& data, utils::Path filename) {
    std::ofstream out(filename, std::ofstream::binary);
    out << data;
}

static bool readBinary(utils::Path filename, std::vector<uint8_t>& buffer) {
    std::ifstream in(filename, std::ifstream::binary | std::ifstream::in);
    if (!in) {
        return false;
    }
    in.seekg(0, std::ios::end);
    std::ifstream::pos_type size = in.tellg();
    in.seekg(0);
    buffer.resize(size);
    if (!in.read((char*) buffer.data(), size)) {
        return false;
    }
    return true;
}

template <typename T>
static std::vector<T> getShaderRecords(const filaflat::ChunkContainer& container,
        const filaflat::BlobDictionary& dictionary, filamat::ChunkType chunkType) {
    if (!container.hasChunk(chunkType)) {
        return {};
    }
    std::vector<T> shaderRecords;
    filaflat::MaterialChunk materialChunk(container);
    materialChunk.initialize(chunkType);
    materialChunk.visitShaders(
            [&materialChunk, &dictionary, &shaderRecords](
                    filament::backend::ShaderModel shaderModel, filament::Variant variant,
                    filament::backend::ShaderStage stage) {
                filaflat::ShaderContent content;
                UTILS_UNUSED_IN_RELEASE bool success =
                        materialChunk.getShader(content, dictionary, shaderModel, variant, stage);

                std::string source{content.data(), content.data() + content.size() - 1u};
                assert_invariant(success);

                if constexpr (std::is_same_v<T, filamat::TextEntry>) {
                    shaderRecords.push_back({shaderModel, variant, stage, std::move(source)});
                }
                if constexpr (std::is_same_v<T, filamat::BinaryEntry>) {
                    filamat::BinaryEntry e{};
                    e.shaderModel = shaderModel;
                    e.variant = variant;
                    e.stage = stage;
                    e.dictionaryIndex = 0;
                    e.data = std::vector<uint8_t>(content.begin(), content.end());
                    shaderRecords.push_back(std::move(e));
                }
            });
    return shaderRecords;
}

static std::string toString(filament::backend::ShaderModel model) {
    switch (model) {
        case filament::backend::ShaderModel::DESKTOP:
            return "desktop";
        case filament::backend::ShaderModel::MOBILE:
            return "mobile";
    }
}

static std::string toString(filament::backend::ShaderStage stage) {
    switch (stage) {
        case filament::backend::ShaderStage::VERTEX:
            return "vertex";
        case filament::backend::ShaderStage::FRAGMENT:
            return "fragment";
        case filament::backend::ShaderStage::COMPUTE:
            return "compute";
    }
}

static std::string toString(filament::Variant variant) {
    return std::to_string(variant.key);
}

static bool invokeScript(
        const std::vector<std::string>& args, utils::Path inputPath, utils::Path outputPath) {
    assert_invariant(!args.empty());

    std::vector<char*> argv;

    // The first argument is the path to the script
    argv.push_back(const_cast<char*>(args[0].c_str()));

    // Temporary input and output files
    argv.push_back(const_cast<char*>(inputPath.c_str()));
    argv.push_back(const_cast<char*>(outputPath.c_str()));

    // Optional user-supplied arguments
    for (int i = 1; i < args.size(); i++) {
        argv.push_back(const_cast<char*>(args[i].c_str()));
    }

    // execvp expects a null as the last element of the arguments array
    argv.push_back(nullptr);

    std::cout << "Invoking script: ";
    for (const char* a : argv) {
        if (a) {
            std::cout << a << " ";
        }
    }
    std::cout << std::endl;

    pid_t pid = fork();

    if (pid == -1) {
        // The fork() command failed
        std::cerr << "Unable to fork process." << std::endl;
        return false;
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child to finish

        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                std::cerr << "Script exited with status: " << WEXITSTATUS(status) << std::endl;
                return false;
            }
        }
    } else {
        // Child process
        execvp(argv[0], argv.data());

        // If execvp returns, it failed
        std::cerr << "execvp failed" << std::endl;
        exit(1);
    }

    return true;
}

class ScopedTempFile {
public:
    ScopedTempFile(utils::Path&& path) noexcept : mPath(path) {}
    ~ScopedTempFile() noexcept { mPath.unlinkFile(); }

    const utils::Path& getPath() const noexcept { return mPath; }

    ScopedTempFile(const ScopedTempFile& rhs) = delete;
    ScopedTempFile(ScopedTempFile&& rhs) = delete;
    ScopedTempFile& operator=(const ScopedTempFile& rhs) = delete;
    ScopedTempFile& operator=(ScopedTempFile&& rhs) = delete;

private:
    utils::Path mPath;
};

bool compileMetalShaders(const std::vector<filamat::TextEntry>& mslEntries,
        std::vector<filamat::BinaryEntry>& metalBinaryEntries,
        const std::vector<std::string>& userArgs) {
    const utils::Path tempDir = utils::Path::getTemporaryDirectory();

    for (const auto& mslEntry : mslEntries) {
        const std::string fileName = toString(mslEntry.shaderModel) + "_" +
                toString(mslEntry.stage) + "_" + toString(mslEntry.variant);
        const std::string inputFileName = fileName + ".metal";
        const std::string outputFileName = fileName + ".metallib";

        // TODO: use mkstemps to create a temporary file name

        ScopedTempFile inputFile = tempDir + inputFileName;
        ScopedTempFile outputFile = tempDir + outputFileName;

        dumpString(mslEntry.shader, inputFile.getPath());
        if (!invokeScript(userArgs, inputFile.getPath(), outputFile.getPath())) {
            return false;
        }

        std::vector<uint8_t> buffer;
        if (!readBinary(outputFile.getPath(), buffer)) {
            std::cerr << "Could not read output file " << outputFile.getPath() << std::endl;
            return false;
        }

        if (buffer.empty()) {
            std::cerr << "Output file " << outputFile.getPath() << " is empty" << std::endl;
            return false;
        }

        filamat::BinaryEntry metalBinaryEntry {};
        metalBinaryEntry.shaderModel = mslEntry.shaderModel;
        metalBinaryEntry.variant = mslEntry.variant;
        metalBinaryEntry.stage = mslEntry.stage;
        metalBinaryEntry.data = std::move(buffer);
        metalBinaryEntries.push_back(metalBinaryEntry);
    }

    return true;
}

int externalCompile(utils::Path input, utils::Path output, std::vector<std::string> args) {
    std::ifstream in(input.c_str(), std::ifstream::in | std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Could not open the source material " << input << std::endl;
        return 1;
    }

    const long fileSize = static_cast<long>(getFileSize(input.c_str()));

    std::vector<char> buffer(static_cast<unsigned long>(fileSize));
    if (!in.read(buffer.data(), fileSize)) {
        std::cerr << "Could not read the source material." << std::endl;
        return 1;
    }

    filaflat::ChunkContainer container(buffer.data(), buffer.size());
    if (!container.parse()) {
        return 1;
    }

    // Get all shaders from the input material.
    filaflat::BlobDictionary stringBlobs;
    filaflat::BlobDictionary spirvBinaryBlobs;
    filaflat::DictionaryReader reader;
    if (container.hasChunk(filamat::ChunkType::DictionaryText)) {
        reader.unflatten(container, filamat::ChunkType::DictionaryText, stringBlobs);
    }
    if (container.hasChunk(filamat::ChunkType::DictionarySpirv)) {
        reader.unflatten(container, filamat::ChunkType::DictionarySpirv, spirvBinaryBlobs);
    }
    auto mslEntries = getShaderRecords<filamat::TextEntry>(
            container, stringBlobs, filamat::ChunkType::MaterialMetal);
    auto glslEntries = getShaderRecords<filamat::TextEntry>(
            container, stringBlobs, filamat::ChunkType::MaterialGlsl);
    auto essl1Entries = getShaderRecords<filamat::TextEntry>(
            container, stringBlobs, filamat::ChunkType::MaterialEssl1);
    auto spirvEntries = getShaderRecords<filamat::BinaryEntry>(
            container, spirvBinaryBlobs, filamat::ChunkType::MaterialSpirv);

    // Ask the user script to compile the MSL shaders into .metallib files.
    filamat::BlobDictionary metalBinaryDictionary;
    std::vector<filamat::BinaryEntry> metalBinaryEntries;
    if (!compileMetalShaders(mslEntries, metalBinaryEntries, args) ) {
        return 1;
    }

    // Since we're modifying text shaders, we'll need to regenerate the text dictionary.
    // We'll also need to re-emit text based shaders that rely on the dictionary.
    // Here we ONLY add GLSL and ESSL 1 types, as we're removing MSL completely.
    filamat::LineDictionary textDictionary;
    for (const auto& s : glslEntries) {
        textDictionary.addText(s.shader);
    }
    for (const auto& s : essl1Entries) {
        textDictionary.addText(s.shader);
    }

    // We'll also need to regenerate the SPIRV dictionary and SPIRV shaders.
    // This is required, as the SPIRV blobs have alignment requirements. Since we're modifying other
    // chunks, their alignment might have changed.
    filamat::BlobDictionary spirvDictionary;
    for (auto& s : spirvEntries) {
        std::vector<uint8_t> spirv = std::move(s.data);
        s.dictionaryIndex = spirvDictionary.addBlob(spirv);
    }

    // Generate the Metal library dictionary.
    for (auto& e : metalBinaryEntries) {
        std::vector<uint8_t> data = std::move(e.data);
        e.dictionaryIndex = metalBinaryDictionary.addBlob(data);
    }

    // Pass through chunks that don't need to change.
    filamat::ChunkContainer outputChunks;
    for (int i = 0; i < container.getChunkCount(); i++) {
        filaflat::ChunkContainer::Chunk c = container.getChunk(i);
        if (c.type == filamat::ChunkType::MaterialMetal) {
            // This chunk is being removed, skip it.
            continue;
        }
        if (c.type == filamat::ChunkType::MaterialGlsl ||
                c.type == filamat::ChunkType::MaterialEssl1 ||
                c.type == filamat::ChunkType::MaterialSpirv ||
                c.type == filamat::ChunkType::DictionarySpirv ||
                c.type == filamat::ChunkType::DictionaryText) {
            // These shader / dictionary chunks will be re-added below.
            continue;
        }
        outputChunks.push<PassthroughChunk>(
                reinterpret_cast<const char*>(c.desc.start), c.desc.size, c.type);
    }

    // Add the re-generated text dictionary chunk and text-based shaders.
    if (!textDictionary.isEmpty()) {
        const auto& dictionaryChunk = outputChunks.push<filamat::DictionaryTextChunk>(
                std::move(textDictionary), filamat::ChunkType::DictionaryText);

        // Re-emit GLSL chunk (MaterialTextChunk).
        if (!glslEntries.empty()) {
            outputChunks.push<filamat::MaterialTextChunk>(std::move(glslEntries),
                    dictionaryChunk.getDictionary(), filamat::ChunkType::MaterialGlsl);
        }

        // Re-emit ESSL1 chunk (MaterialTextChunk).
        if (!essl1Entries.empty()) {
            outputChunks.push<filamat::MaterialTextChunk>(std::move(essl1Entries),
                    dictionaryChunk.getDictionary(), filamat::ChunkType::MaterialEssl1);
        }
    }

    // Add the SPIRV chunks.
    if (!spirvEntries.empty()) {
        const bool stripInfo = true;
        outputChunks.push<filamat::DictionarySpirvChunk>(std::move(spirvDictionary), stripInfo);
        outputChunks.push<filamat::MaterialBinaryChunk>(
                std::move(spirvEntries), filamat::ChunkType::MaterialSpirv);
    }

    // Add the new Metal binary chunks.
    outputChunks.push<filamat::DictionaryMetalLibraryChunk>(std::move(metalBinaryDictionary));
    outputChunks.push<filamat::MaterialBinaryChunk>(
            std::move(metalBinaryEntries), filamat::ChunkType::MaterialMetalLibrary);

    // Flatten into a Package and write to disk.
    Package package(outputChunks.getSize());
    Flattener f{ package.getData() };
    outputChunks.flatten(f);

    assert_invariant(package.isValid());

    dumpBinary(package.getData(), package.getSize(), output);

    return 0;
}

}  // namespace matedit
