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

#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
#include <smolv.h>
#endif

#include <vector>
#include <cstdint>
#include <cstdlib>
#include <cstring>

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
    write16(payload, 0); // mSharedStrings
    write16(payload, 0); // mVertexStrings
    write16(payload, 0); // mFragmentStrings
    write16(payload, 0); // mComputeStrings
    write64(payload, 1); // numShaders
    payload.push_back(1); // model
    payload.push_back(0); // variant
    payload.push_back(0); // stage
    write32(payload, 23); // offset (4*2 + 8 + 1 + 1 + 1 + 4 = 23)
    
    // Shader content layout
    write32(payload, 4);  // shaderSize (vulnerable tiny size)
    write32(payload, 1);  // lineCount = 1
    write32(payload, 0);  // extLength
    write32(payload, 0);  // baseLength
    write32(payload, 0);  // numericLength
    payload.push_back(0);  // lineIndex = 0

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
    write16(payload, 0); // mSharedStrings
    write16(payload, 0); // mVertexStrings
    write16(payload, 0); // mFragmentStrings
    write16(payload, 0); // mComputeStrings
    write64(payload, 1); // numShaders
    payload.push_back(1); // model
    payload.push_back(0); // variant
    payload.push_back(0); // stage
    write32(payload, 23); // offset
    write32(payload, 100);  // shaderSize 
    write32(payload, 1);  // lineCount = 1
    write32(payload, 0);  // extLength
    write32(payload, 0);  // baseLength
    write32(payload, 0);  // numericLength
    payload.push_back(1);  // lineIndex = 1 (OOB for empty dict)

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


// 6. Heap OOB write in smolv::Decode (b/557280759)
// The decoded-size field in the SMOL-V header drives the output allocation, but the decoder
// never used it as a write bound. A blob declaring a size smaller than it actually decodes to
// therefore wrote past the end of the caller's buffer.
#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)

// Valid SPIR-V covering the decoder's three distinct write paths: plain instructions, a bunched
// run of OpMemberDecorate, and the compact vector-shuffle encoding. `groups` scales the payload.
static std::vector<uint32_t> makeTestSpirv(uint32_t groups) {
    std::vector<uint32_t> w;
    w.push_back(0x07230203); // magic
    w.push_back(0x00010000); // version 1.0
    w.push_back(0);          // generator
    w.push_back(1024);       // bound
    w.push_back(0);          // schema
    for (uint32_t i = 0; i < groups; i++) {
        // OpCapability Shader
        w.push_back((2u << 16) | 17u);
        w.push_back(1u);
        // Three OpMemberDecorate on one struct type; smol-v encodes the run as a single bunch.
        // Offset (35) has a known extra-op count, decoration 1 does not, so its length is encoded.
        w.push_back((5u << 16) | 72u); w.push_back(100u); w.push_back(0u); w.push_back(35u);
        w.push_back(0u);
        w.push_back((5u << 16) | 72u); w.push_back(100u); w.push_back(1u); w.push_back(35u);
        w.push_back(16u);
        w.push_back((6u << 16) | 72u); w.push_back(100u); w.push_back(2u); w.push_back(1u);
        w.push_back(7u); w.push_back(8u);
        // OpVectorShuffle with four components < 4, which smol-v packs into a single swizzle byte.
        w.push_back((9u << 16) | 79u);
        w.push_back(200u); w.push_back(201u); w.push_back(202u); w.push_back(203u);
        w.push_back(0u); w.push_back(1u); w.push_back(2u); w.push_back(3u);
    }
    return w;
}

// Guards against the bounds checks rejecting well-formed input.
TEST_F(FilaflatSecurityTest, SmolvDecodeRoundTrip) {
    const std::vector<uint32_t> spirv = makeTestSpirv(4);
    const size_t spirvSize = spirv.size() * 4;

    smolv::ByteArray encoded;
    ASSERT_TRUE(smolv::Encode(spirv.data(), spirvSize, encoded, 0));
    ASSERT_EQ(smolv::GetDecodedBufferSize(encoded.data(), encoded.size()), spirvSize);

    std::vector<uint8_t> decoded(spirvSize);
    ASSERT_TRUE(smolv::Decode(encoded.data(), encoded.size(), decoded.data(), decoded.size()));
    EXPECT_EQ(0, memcmp(decoded.data(), spirv.data(), spirvSize));
}

TEST_F(FilaflatSecurityTest, SmolvDecodeTruncatedSizeNoOverflow) {
    const std::vector<uint32_t> spirv = makeTestSpirv(16);
    const size_t spirvSize = spirv.size() * 4;

    smolv::ByteArray encoded;
    ASSERT_TRUE(smolv::Encode(spirv.data(), spirvSize, encoded, 0));

    // Shrink the declared decoded size to just the SPIR-V header. Every instruction that follows
    // is then written past the end of a buffer sized from this field.
    const uint32_t truncatedSize = 20;
    ASSERT_LT(truncatedSize, spirvSize);
    memcpy(encoded.data() + 20, &truncatedSize, 4); // words[5] == decoded size

    // Mirrors DictionaryReader: the allocation and the size handed to Decode both come from here,
    // so Decode's own `spirvOutputBufferSize < neededBufferSize` check can never catch this.
    const size_t outSize = smolv::GetDecodedBufferSize(encoded.data(), encoded.size());
    ASSERT_EQ(outSize, truncatedSize);

    // Decode's write bound is the size argument, not the allocation, so over-allocating and
    // checking the tail detects the overflow in any build. Testing the return value alone does
    // not: the vulnerable decoder also returns false, from a check that runs after the writes.
    constexpr size_t canarySize = 4096;
    constexpr uint8_t canary = 0xCD;
    std::vector<uint8_t> decoded(outSize + canarySize, canary);

    EXPECT_FALSE(smolv::Decode(encoded.data(), encoded.size(), decoded.data(), outSize));
    for (size_t i = outSize; i < decoded.size(); i++) {
        ASSERT_EQ(canary, decoded[i])
                << "VULNERABILITY: smolv::Decode wrote " << (i - outSize)
                << " bytes past the end of the output buffer!";
    }
}

#endif // FILAMENT_DRIVER_SUPPORTS_VULKAN


// 7. Unbounded allocation from attacker-controlled dictionary counts (b/562964630)
// `reserve()` runs before a single blob is read, so a declared count never has to be backed by
// actual data. The loop's bounds checks are sound but they all run *after* the allocation.

// Records the largest single allocation while in scope. An allocation bug needs this: the
// vulnerable code returns false just like the fixed code does, and on Linux a multi-gigabyte
// request usually succeeds lazily through overcommit instead of throwing, so neither the return
// value nor a crash is a dependable signal.
namespace {

size_t gLargestAllocation = 0;
bool gRecording = false;

class AllocationRecorder {
public:
    AllocationRecorder() {
        gLargestAllocation = 0;
        gRecording = true;
    }
    ~AllocationRecorder() { gRecording = false; }
    size_t largest() const { return gLargestAllocation; }
};

} // namespace

void* operator new(size_t size) {
    if (gRecording && size > gLargestAllocation) {
        gLargestAllocation = size;
    }
    void* const p = malloc(size ? size : 1);
    if (!p) {
        // Filament also builds with -fno-exceptions, so do not throw std::bad_alloc here.
        abort();
    }
    return p;
}

void operator delete(void* p) noexcept { free(p); }
void operator delete(void* p, size_t) noexcept { free(p); }


// Chunk layout is [type u64][size u32][payload]. Blobs are padded to an 8-byte offset from the
// start of the file by filamat's Flattener, but Unflattener::skipAlignmentPadding() pads to an
// absolute 8-byte address, so the two only agree when the buffer itself is 8-byte aligned.
static std::vector<uint8_t> finishChunk(filamat::ChunkType type, std::vector<uint8_t>&& fileData) {
    const uint32_t payloadSize = uint32_t(fileData.size() - 12);
    for (int i = 0; i < 8; i++) fileData[i] = uint8_t((uint64_t(type) >> (8 * i)) & 0xFF);
    for (int i = 0; i < 4; i++) fileData[8 + i] = uint8_t((payloadSize >> (8 * i)) & 0xFF);
    return std::move(fileData);
}

TEST_F(FilaflatSecurityTest, DictionarySpirvBlobCountExceedsChunk) {
    // 0x400000 blobs (64 MB) allocates cheaply, and 0xFFFFFFFF is the reported case: 64 GB from a
    // 20-byte file. Neither reliably aborts -- Linux overcommit hands back both without touching
    // a page -- so the capacity check below, not a crash, is what detects the bug.
    for (uint32_t blobCount : {0x400000u, 0xFFFFFFFFu}) {
        std::vector<uint8_t> fileData(12, 0); // type + size, patched below
        write32(fileData, 1);         // compressionScheme
        write32(fileData, blobCount); // no blob data follows at all
        fileData = finishChunk(filamat::ChunkType::DictionarySpirv, std::move(fileData));

        ChunkContainer container(fileData.data(), fileData.size());
        ASSERT_TRUE(container.parse());

        BlobDictionary dictionary;
        EXPECT_FALSE(DictionaryReader::unflatten(
                container, filamat::ChunkType::DictionarySpirv, dictionary));

        // Returning false is not enough on its own: the vulnerable code also returns false, from
        // the read that fails after reserve() already committed the allocation. Each blob costs at
        // least the 8 bytes of its length header, so the dictionary can never hold more blobs than
        // the chunk has bytes.
        EXPECT_LE(dictionary.capacity(), fileData.size())
                << "VULNERABILITY: reserved " << dictionary.capacity()
                << " blobs from a " << fileData.size() << "-byte file!";
    }
}

TEST_F(FilaflatSecurityTest, DictionaryMetalLibraryBlobCountExceedsChunk) {
    for (uint32_t blobCount : {0x400000u, 0xFFFFFFFFu}) {
        std::vector<uint8_t> fileData(12, 0);
        write32(fileData, blobCount);
        fileData = finishChunk(filamat::ChunkType::DictionaryMetalLibrary, std::move(fileData));

        ChunkContainer container(fileData.data(), fileData.size());
        ASSERT_TRUE(container.parse());

        BlobDictionary dictionary;
        EXPECT_FALSE(DictionaryReader::unflatten(
                container, filamat::ChunkType::DictionaryMetalLibrary, dictionary));
        EXPECT_LE(dictionary.capacity(), fileData.size())
                << "VULNERABILITY: reserved " << dictionary.capacity()
                << " blobs from a " << fileData.size() << "-byte file!";
    }
}

// Guards the new bound against rejecting well-formed input.
TEST_F(FilaflatSecurityTest, DictionaryMetalLibraryValidBlobsStillParse) {
    const char* blobs[] = { "metallib", "xyz" };

    std::vector<uint8_t> fileData(12, 0);
    write32(fileData, 2); // blobCount
    for (const char* blob : blobs) {
        while (fileData.size() % 8 != 0) fileData.push_back(0); // alignment padding
        const size_t len = strlen(blob);
        write64(fileData, len);
        fileData.insert(fileData.end(), blob, blob + len);
    }
    fileData = finishChunk(filamat::ChunkType::DictionaryMetalLibrary, std::move(fileData));

    ASSERT_EQ(0u, uintptr_t(fileData.data()) % 8) << "reader pads on absolute addresses";

    ChunkContainer container(fileData.data(), fileData.size());
    ASSERT_TRUE(container.parse());

    BlobDictionary dictionary;
    ASSERT_TRUE(DictionaryReader::unflatten(
            container, filamat::ChunkType::DictionaryMetalLibrary, dictionary));
    ASSERT_EQ(2u, dictionary.size());
    for (size_t i = 0; i < 2; i++) {
        const size_t len = strlen(blobs[i]);
        ASSERT_EQ(len, dictionary[i].size());
        EXPECT_EQ(0, memcmp(dictionary[i].data(), blobs[i], len));
    }
}

#if defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)

// Builds a DictionarySpirv chunk holding one real smol-v blob.
static std::vector<uint8_t> makeSpirvDictionaryChunk(const smolv::ByteArray& encoded) {
    std::vector<uint8_t> fileData(12, 0);
    for (int i = 0; i < 4; i++) fileData.push_back(i == 0 ? 1 : 0); // compressionScheme = 1
    for (int i = 0; i < 4; i++) fileData.push_back(i == 0 ? 1 : 0); // blobCount = 1
    while (fileData.size() % 8 != 0) fileData.push_back(0);
    for (int i = 0; i < 8; i++) fileData.push_back(uint8_t((uint64_t(encoded.size()) >> (8 * i)) & 0xFF));
    fileData.insert(fileData.end(), encoded.begin(), encoded.end());
    return finishChunk(filamat::ChunkType::DictionarySpirv, std::move(fileData));
}

TEST_F(FilaflatSecurityTest, DictionarySpirvValidBlobStillParses) {
    const std::vector<uint32_t> spirv = makeTestSpirv(4);
    const size_t spirvSize = spirv.size() * 4;

    smolv::ByteArray encoded;
    ASSERT_TRUE(smolv::Encode(spirv.data(), spirvSize, encoded, 0));

    std::vector<uint8_t> fileData = makeSpirvDictionaryChunk(encoded);
    ASSERT_EQ(0u, uintptr_t(fileData.data()) % 8) << "reader pads on absolute addresses";

    ChunkContainer container(fileData.data(), fileData.size());
    ASSERT_TRUE(container.parse());

    BlobDictionary dictionary;
    ASSERT_TRUE(DictionaryReader::unflatten(
            container, filamat::ChunkType::DictionarySpirv, dictionary));
    ASSERT_EQ(1u, dictionary.size());
    ASSERT_EQ(spirvSize, dictionary[0].size());
    EXPECT_EQ(0, memcmp(dictionary[0].data(), spirv.data(), spirvSize));

    // The decoded/encoded ratio the bound has to accommodate. smol-v's worst sustained expansion
    // is 8x (a bunched OpMemberDecorate member costs 2 input bytes and emits 16 output bytes), so
    // real content should sit far below it.
    EXPECT_LT(spirvSize, 8 * encoded.size());
}

// The decoded size is taken verbatim from the blob's header word, so a tiny blob can request an
// arbitrarily large buffer. smolv::GetDecodedBufferSize() cannot bound this itself because it does
// not know the input size; the caller does.
TEST_F(FilaflatSecurityTest, DictionarySpirvDecodedSizeExceedsRatio) {
    const std::vector<uint32_t> spirv = makeTestSpirv(1);
    smolv::ByteArray encoded;
    ASSERT_TRUE(smolv::Encode(spirv.data(), spirv.size() * 4, encoded, 0));

    // Claim 4 GB of decoded output from a blob of a few dozen bytes.
    const uint32_t hugeSize = 0xFFFFFFFFu;
    memcpy(encoded.data() + 20, &hugeSize, 4); // words[5] == decoded size
    ASSERT_EQ(hugeSize, smolv::GetDecodedBufferSize(encoded.data(), encoded.size()));

    std::vector<uint8_t> fileData = makeSpirvDictionaryChunk(encoded);
    ChunkContainer container(fileData.data(), fileData.size());
    ASSERT_TRUE(container.parse());

    BlobDictionary dictionary;
    size_t largest;
    {
        // The return value cannot tell the fix from the vulnerability here: the vulnerable code
        // allocates the 4 GB buffer and *then* returns false when Decode rejects the blob.
        AllocationRecorder recorder;
        EXPECT_FALSE(DictionaryReader::unflatten(
                container, filamat::ChunkType::DictionarySpirv, dictionary));
        largest = recorder.largest();
    }

    EXPECT_LE(largest, 8 * encoded.size())
            << "VULNERABILITY: allocated " << largest << " bytes for a "
            << encoded.size() << "-byte blob!";
}


#endif // FILAMENT_DRIVER_SUPPORTS_VULKAN


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    const int rv = RUN_ALL_TESTS();
    if (testing::UnitTest::GetInstance()->test_to_run_count() == 0) {
        //If you run a test filter that contains 0 tests that was likely not intentional. Fail in that scenario.
        return 1;
    }
    return rv;
}
