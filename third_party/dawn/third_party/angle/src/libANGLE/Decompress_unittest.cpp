//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Decompress_unittest.cpp: Unit tests for the |(Compress/Decompress)*Blob| functions.

#include <gtest/gtest.h>

#include "libANGLE/angletypes.h"

namespace angle
{
namespace
{
class DecompressTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        constexpr size_t kTestDataSize = 100'000;

        mTestData.resize(kTestDataSize);
        for (size_t i = 0; i < kTestDataSize; ++i)
        {
            mTestData[i] = static_cast<uint8_t>(i);
        }

        ASSERT_TRUE(CompressBlob(mTestData.size(), mTestData.data(), &mCompressedData));
    }

    void setCompressedDataLastDWord(uint32_t value)
    {
        ASSERT(IsLittleEndian());
        ASSERT(mCompressedData.size() > sizeof(value));
        memcpy(mCompressedData.data() + mCompressedData.size() - sizeof(value), &value,
               sizeof(value));
    }

    bool decompress(size_t compressedSize, size_t maxUncompressedDataSize)
    {
        return DecompressBlob(mCompressedData.data(), compressedSize, maxUncompressedDataSize,
                              &mUncompressedData);
    }

    bool checkUncompressedData()
    {
        return (mTestData.size() == mUncompressedData.size()) &&
               (memcmp(mTestData.data(), mUncompressedData.data(), mTestData.size()) == 0);
    }

    std::vector<uint8_t> mTestData;
    MemoryBuffer mCompressedData;
    MemoryBuffer mUncompressedData;
};

// Tests that decompressing full data has no errors.
TEST_F(DecompressTest, FullData)
{
    EXPECT_TRUE(decompress(mCompressedData.size(), mTestData.size()));
    EXPECT_TRUE(checkUncompressedData());
}

// Tests expected failure if |maxUncompressedDataSize| is less than actual uncompressed size.
TEST_F(DecompressTest, InsufficientMaxUncompressedDataSize)
{
    EXPECT_FALSE(decompress(mCompressedData.size(), mTestData.size() - 1));
}

// Tests expected failure if try to decompress partial compressed data.
TEST_F(DecompressTest, UnexpectedPartialData)
{
    // Use this to avoid |maxUncompressedDataSize| affecting the test.
    constexpr size_t kMaxUncompressedDataSize = std::numeric_limits<size_t>::max();

    EXPECT_FALSE(decompress(mCompressedData.size() - 1, kMaxUncompressedDataSize));
}

// Tests expected failure if try to decompress corrupted data.
TEST_F(DecompressTest, CorruptedData)
{
    // Corrupt the compressed data.
    const size_t corruptIndex = mCompressedData.size() / 2;
    mCompressedData[corruptIndex] ^= 255;

    EXPECT_FALSE(decompress(mCompressedData.size(), mTestData.size()));
}

// Tests expected failures if try to decompress data with the corrupted last dword.
TEST_F(DecompressTest, CorruptedLastDWord)
{
    // Last dword stores decompressed data size and not the actual compressed data. This dword must
    // match the decompressed size. Decompress should fail if it is not the case.

    // Use this to avoid |maxUncompressedDataSize| affecting the test.
    constexpr size_t kMaxUncompressedDataSize = std::numeric_limits<size_t>::max();

    // Try to decompress with decreased size in the last dword.
    setCompressedDataLastDWord(static_cast<uint32_t>(mTestData.size() - 1));
    EXPECT_FALSE(decompress(mCompressedData.size(), kMaxUncompressedDataSize));

    // Try to decompress with increased size in the last dword.
    setCompressedDataLastDWord(static_cast<uint32_t>(mTestData.size() + 1));
    EXPECT_FALSE(decompress(mCompressedData.size(), kMaxUncompressedDataSize));

    // Try to decompress with the last dword set to 0.
    setCompressedDataLastDWord(0);
    EXPECT_FALSE(decompress(mCompressedData.size(), kMaxUncompressedDataSize));

    // Decompress with the last dword set to the correct size should succeed.
    setCompressedDataLastDWord(static_cast<uint32_t>(mTestData.size()));
    EXPECT_TRUE(decompress(mCompressedData.size(), kMaxUncompressedDataSize));
    EXPECT_TRUE(checkUncompressedData());
}

}  // anonymous namespace
}  // namespace angle
