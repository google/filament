// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImmutableString_test.cpp:
//   Tests for ImmutableString and ImmutableStringBuilder.

#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/PoolAlloc.h"
#include "gtest/gtest.h"

using namespace sh;

class ImmutableStringBuilderTest : public testing::Test
{
  public:
    ImmutableStringBuilderTest() {}

  protected:
    void SetUp() override
    {
        allocator.push();
        SetGlobalPoolAllocator(&allocator);
    }

    void TearDown() override
    {
        SetGlobalPoolAllocator(nullptr);
        allocator.pop();
    }

    angle::PoolAllocator allocator;
};

// Test writing a 32-bit signed int as hexadecimal using ImmutableStringBuilder.
TEST_F(ImmutableStringBuilderTest, AppendHexInt32)
{
    int32_t i = -1;
    ImmutableStringBuilder strBuilder(2 * sizeof(int32_t));
    strBuilder.appendHex(i);
    ImmutableString str = strBuilder;
    EXPECT_EQ(std::string("ffffffff"), str.data());
}

// Test writing a 32-bit unsigned int as hexadecimal using ImmutableStringBuilder.
TEST_F(ImmutableStringBuilderTest, AppendHexUint32)
{
    uint32_t i = 0x1234beefu;
    ImmutableStringBuilder strBuilder(2 * sizeof(uint32_t));
    strBuilder.appendHex(i);
    ImmutableString str = strBuilder;
    EXPECT_EQ(std::string("1234beef"), str.data());
}

// Test writing a 64-bit signed int as hexadecimal using ImmutableStringBuilder.
TEST_F(ImmutableStringBuilderTest, AppendHexInt64)
{
    int64_t i = -1;
    ImmutableStringBuilder strBuilder(2 * sizeof(int64_t));
    strBuilder.appendHex(i);
    ImmutableString str = strBuilder;
    EXPECT_EQ(std::string("ffffffffffffffff"), str.data());
}

// Test writing a 64-bit unsigned int as hexadecimal using ImmutableStringBuilder.
TEST_F(ImmutableStringBuilderTest, AppendHexUint64)
{
    uint64_t i = 0xfeedcafe9876beefull;
    ImmutableStringBuilder strBuilder(2 * sizeof(uint64_t));
    strBuilder.appendHex(i);
    ImmutableString str = strBuilder;
    EXPECT_EQ(std::string("feedcafe9876beef"), str.data());
}

// Test writing a decimal using ImmutableStringBuilder of exact size.
TEST_F(ImmutableStringBuilderTest, AppendDecimal)
{
    ImmutableStringBuilder b1(1);
    b1 << 1;
    ImmutableString s1 = b1;
    EXPECT_EQ(std::string("1"), s1.data());

    ImmutableStringBuilder b20(2);
    b20 << 20;
    ImmutableString s20 = b20;
    EXPECT_EQ(std::string("20"), s20.data());

    ImmutableStringBuilder b30000(5);
    b30000 << 30000;
    ImmutableString s30000 = b30000;
    EXPECT_EQ(std::string("30000"), s30000.data());
}

// Test BuildConcatenatedImmutableString.
TEST_F(ImmutableStringBuilderTest, BuildConcatenatedImmutableString)
{
    EXPECT_EQ(BuildConcatenatedImmutableString('a', 0, 'b'), "a0b");
    EXPECT_EQ(BuildConcatenatedImmutableString('a', 10, 'b'), "a10b");
    EXPECT_EQ(BuildConcatenatedImmutableString('a', -153, 'b'), "a-153b");
    EXPECT_EQ(BuildConcatenatedImmutableString('a', 78788u, 'b'), "a78788b");
    EXPECT_EQ(BuildConcatenatedImmutableString('a', INT64_MAX, 'b'), "a9223372036854775807b");
    EXPECT_EQ(BuildConcatenatedImmutableString('a', INT64_MIN, 'b'), "a-9223372036854775808b");
    EXPECT_EQ(BuildConcatenatedImmutableString('a', 'c', 'b'), "acb");
    EXPECT_EQ(BuildConcatenatedImmutableString("ab", 'c', "de"), "abcde");
}
