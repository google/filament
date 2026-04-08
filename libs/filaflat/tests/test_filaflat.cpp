/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <gtest/gtest.h>

#include <filaflat/ChunkContainer.h>
#include <filaflat/DictionaryReader.h>
#include <filaflat/MaterialChunk.h>
#include <filaflat/Unflattener.h>
#include <filament/MaterialChunkType.h>

#include <vector>
#include <cstdint>

using namespace filaflat;

class FilaflatSecurityTest : public ::testing::Test {
protected:
    void write64(std::vector<uint8_t>& vec, uint64_t val) {
        for (int i = 0; i < 8; i++) vec.push_back((val >> (8 * i)) & 0xFF);
    }
    void write32(std::vector<uint8_t>& vec, uint32_t val) {
        for (int i = 0; i < 4; i++) vec.push_back((val >> (8 * i)) & 0xFF);
    }
    void write16(std::vector<uint8_t>& vec, uint16_t val) {
        for (int i = 0; i < 2; i++) vec.push_back((val >> (8 * i)) & 0xFF);
    }
};

#ifndef _WIN32
#include <sys/mman.h>
#include <unistd.h>
#endif

// 1. OOB Read during dictionary text flat buffer parsing 
// By definition, strlen() will read far out of bounds since we provide no null terminator.
TEST_F(FilaflatSecurityTest, DictionaryTextOOBRead) {
    std::vector<uint8_t> payload;
    write32(payload, 1); // stringCount = 1
    // Maliciously omitting the null terminator here
    payload.push_back('v'); payload.push_back('u'); payload.push_back('l'); payload.push_back('n');

    std::vector<uint8_t> fileData;
    write64(fileData, (uint64_t)filamat::ChunkType::DictionaryText);
    write32(fileData, payload.size());
    fileData.insert(fileData.end(), payload.begin(), payload.end());

#ifndef _WIN32
    // To reliably trigger a crash (segfault) without ASAN when strlen tries to read OOB,
    // we allocate exactly up to a protected page boundary.
    size_t pageSize = getpagesize();
    uint8_t* memory = (uint8_t*)mmap(NULL, pageSize * 2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    ASSERT_NE(memory, MAP_FAILED);
    // Protect the second page so any read into it causes an immediate SIGSEGV.
    mprotect(memory + pageSize, pageSize, PROT_NONE);

    // Place our fileData at the very end of the first page.
    uint8_t* exactData = memory + pageSize - fileData.size();
    memcpy(exactData, fileData.data(), fileData.size());

    ChunkContainer container(exactData, fileData.size());
    ASSERT_TRUE(container.parse());

    BlobDictionary dictionary;
    // THIS LINE EXPLOITS THE VULNERABILITY (Will violently Segmentation Fault due to page access violation)
    DictionaryReader::unflatten(container, filamat::ChunkType::DictionaryText, dictionary);
    
    munmap(memory, pageSize * 2);
#else
    ChunkContainer container(fileData.data(), fileData.size());
    ASSERT_TRUE(container.parse());
    BlobDictionary dictionary;
    DictionaryReader::unflatten(container, filamat::ChunkType::DictionaryText, dictionary);
#endif
}

// 2. Heap Buffer Overflow writing dictionary arrays to undersized output buffers
TEST_F(FilaflatSecurityTest, MaterialChunkHeapOverflow) {
    std::vector<uint8_t> payload;
    write64(payload, 1); // numShaders
    payload.push_back(1); // model
    payload.push_back(0); // variant
    payload.push_back(0); // stage
    write32(payload, 15); // offset (8 + 1 + 1 + 1 + 4 = 15)
    
    // Shader content layout
    write32(payload, 4);  // shaderSize (vulnerable tiny size)
    write32(payload, 1);  // lineCount = 1
    write16(payload, 0);  // lineIndex = 0

    std::vector<uint8_t> fileData;
    write64(fileData, (uint64_t)filamat::ChunkType::MaterialGlsl);
    write32(fileData, payload.size());
    fileData.insert(fileData.end(), payload.begin(), payload.end());

    ChunkContainer container(fileData.data(), fileData.size());
    ASSERT_TRUE(container.parse());

    MaterialChunk chunk(container);
    ASSERT_TRUE(chunk.initialize(filamat::ChunkType::MaterialGlsl));

    BlobDictionary dict;
    dict.reserve(1); // FixedCapacityVector must be explicitly reserved before push_back
    ShaderContent content;
    // Pre-populate dictionary with an enormous string
    // This is vastly larger than shaderSize=4 above
    content.reserve(1024);
    content.resize(1024);
    content[0] = 'H'; content[1023] = '\0';
    dict.push_back(content);

    ShaderContent output;
    // THIS LINE EXPLOITS THE VULNERABILITY (Heap buffer overflow, memcpy overwrite)
    // The patched `getShader` routine securely rejects it by evaluating `content.size() > shaderSize` returning false.
    bool valid = chunk.getShader(output, dict, MaterialChunk::ShaderModel(1), filament::Variant{0}, MaterialChunk::ShaderStage(0));
    EXPECT_FALSE(valid) << "VULNERABILITY: Heap overflow validation bypassed!";
}

// 3. Out of Bounds Read via index mapping evasion (Text mode)
TEST_F(FilaflatSecurityTest, MaterialChunkOOBReadText) {
    std::vector<uint8_t> payload;
    write64(payload, 1); // numShaders
    payload.push_back(1); // model
    payload.push_back(0); // variant
    payload.push_back(0); // stage
    write32(payload, 15); // offset (8 + 1 + 1 + 1 + 4 = 15)
    write32(payload, 100);  // shaderSize 
    write32(payload, 1);  // lineCount = 1
    write16(payload, 9999);  // lineIndex = 9999 (VULNERABLE ACCESS)

    std::vector<uint8_t> fileData;
    write64(fileData, (uint64_t)filamat::ChunkType::MaterialGlsl);
    write32(fileData, payload.size());
    fileData.insert(fileData.end(), payload.begin(), payload.end());

    ChunkContainer container(fileData.data(), fileData.size());
    ASSERT_TRUE(container.parse());

    MaterialChunk chunk(container);
    ASSERT_TRUE(chunk.initialize(filamat::ChunkType::MaterialGlsl));

    BlobDictionary dict; // Empty dictionary
    ShaderContent output;
    // THIS LINE EXPLOITS THE VULNERABILITY (OOB Memory Access dict[9999])
    chunk.getShader(output, dict, MaterialChunk::ShaderModel(1), filament::Variant{0}, MaterialChunk::ShaderStage(0));
}

// 4. Out of Bounds Read via index mapping evasion (Binary mode)
TEST_F(FilaflatSecurityTest, MaterialChunkOOBReadBinary) {
    std::vector<uint8_t> payload;
    write64(payload, 1); // numShaders
    payload.push_back(1); // model
    payload.push_back(0); // variant
    payload.push_back(0); // stage
    
    // For binary mode, the offset field serves as the dictionary index
    write32(payload, 9999); // offset = 9999 (VULNERABLE ACCESS)

    std::vector<uint8_t> fileData;
    // Utilizing MaterialSpirv triggers getBinaryShader
    write64(fileData, (uint64_t)filamat::ChunkType::MaterialSpirv);
    write32(fileData, payload.size());
    fileData.insert(fileData.end(), payload.begin(), payload.end());

    ChunkContainer container(fileData.data(), fileData.size());
    ASSERT_TRUE(container.parse());

    MaterialChunk chunk(container);
    ASSERT_TRUE(chunk.initialize(filamat::ChunkType::MaterialSpirv));

    BlobDictionary dict; // Empty dictionary
    ShaderContent output;
    // THIS LINE EXPLOITS THE VULNERABILITY (OOB Memory Access dict[9999])
    chunk.getShader(output, dict, MaterialChunk::ShaderModel(1), filament::Variant{0}, MaterialChunk::ShaderStage(0));
}

// 5. Integer overflow / Pointer wrap-around evasion 
TEST_F(FilaflatSecurityTest, UnflattenerIntegerWrapBypass) {
    std::vector<uint8_t> payload;
    // An artificially huge size likely to wrap around mCursor + nbytes 
    write64(payload, 0xFFFFFFFFFFFFFFF0); 

    Unflattener unflattener(payload.data(), payload.data() + payload.size());
    const char* blob;
    size_t size;
    
    // Attempt the Out-Of-Bounds wrap read
    bool bypassed = unflattener.read(&blob, &size);

    // THIS LINE EXPLOITS THE VULNERABILITY (Will securely trigger Test Failure)
    // A secure implementation should evaluate the impossible wrapper size and explicitly return false.
    // The vulnerability forces it to return true, defying the integer boundaries and bypassing checks.
    EXPECT_FALSE(bypassed) << "VULNERABILITY: Integer wrap successfully bypassed Unflattener boundaries!";
}


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    const int rv = RUN_ALL_TESTS();
    if (testing::UnitTest::GetInstance()->test_to_run_count() == 0) {
        //If you run a test filter that contains 0 tests that was likely not intentional. Fail in that scenario.
        return 1;
    }
    return rv;
}
