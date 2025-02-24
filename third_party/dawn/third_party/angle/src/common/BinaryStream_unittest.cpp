//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BinaryStream_unittest.cpp: Unit tests of the binary stream classes.

#include <gtest/gtest.h>

#include "common/BinaryStream.h"

namespace angle
{

// Test that errors are properly generated for overflows.
TEST(BinaryInputStream, Overflow)
{
    const uint8_t goodValue = 2;
    const uint8_t badValue  = 255;

    const size_t dataSize = 1024;
    const size_t slopSize = 1024;

    std::vector<uint8_t> data(dataSize + slopSize);
    std::fill(data.begin(), data.begin() + dataSize, goodValue);
    std::fill(data.begin() + dataSize, data.end(), badValue);

    std::vector<uint8_t> outputData(dataSize);

    auto checkDataIsSafe = [=](uint8_t item) { return item == goodValue; };

    {
        // One large read
        gl::BinaryInputStream stream(data.data(), dataSize);
        stream.readBytes(outputData.data(), dataSize);
        ASSERT_FALSE(stream.error());
        ASSERT_TRUE(std::all_of(outputData.begin(), outputData.end(), checkDataIsSafe));
        ASSERT_TRUE(stream.endOfStream());
    }

    {
        // Two half-sized reads
        gl::BinaryInputStream stream(data.data(), dataSize);
        stream.readBytes(outputData.data(), dataSize / 2);
        ASSERT_FALSE(stream.error());
        stream.readBytes(outputData.data() + dataSize / 2, dataSize / 2);
        ASSERT_FALSE(stream.error());
        ASSERT_TRUE(std::all_of(outputData.begin(), outputData.end(), checkDataIsSafe));
        ASSERT_TRUE(stream.endOfStream());
    }

    {
        // One large read that is too big
        gl::BinaryInputStream stream(data.data(), dataSize);
        stream.readBytes(outputData.data(), dataSize + 1);
        ASSERT_TRUE(stream.error());
    }

    {
        // Two reads, one that overflows the offset
        gl::BinaryInputStream stream(data.data(), dataSize);
        stream.readBytes(outputData.data(), dataSize - 1);
        ASSERT_FALSE(stream.error());
        stream.readBytes(outputData.data(), std::numeric_limits<size_t>::max() - dataSize - 2);
    }
}

// Test that readVector and writeVector match.
TEST(BinaryStream, IntVector)
{
    std::vector<unsigned int> writeData = {1, 2, 3, 4, 5};
    std::vector<unsigned int> readData;

    gl::BinaryOutputStream out;
    out.writeVector(writeData);

    gl::BinaryInputStream in(out.data(), out.length());
    in.readVector(&readData);

    ASSERT_EQ(writeData.size(), readData.size());

    for (size_t i = 0; i < writeData.size(); ++i)
    {
        ASSERT_EQ(writeData[i], readData[i]);
    }
}
}  // namespace angle
