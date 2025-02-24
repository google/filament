//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// utilities_unittest.cpp: Unit tests for ANGLE's GL utility functions

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/utilities.h"

namespace
{

// Test parsing valid single array indices
TEST(ParseResourceName, ArrayIndex)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo", gl::ParseResourceName("foo[123]", &indices));
    ASSERT_EQ(1u, indices.size());
    EXPECT_EQ(123u, indices[0]);

    EXPECT_EQ("bar", gl::ParseResourceName("bar[0]", &indices));
    ASSERT_EQ(1u, indices.size());
    EXPECT_EQ(0u, indices[0]);
}

// Parsing a negative array index should result in INVALID_INDEX.
TEST(ParseResourceName, NegativeArrayIndex)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo", gl::ParseResourceName("foo[-1]", &indices));
    ASSERT_EQ(1u, indices.size());
    EXPECT_EQ(GL_INVALID_INDEX, indices.back());
}

// Parsing no array indices should result in an empty array.
TEST(ParseResourceName, NoArrayIndex)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo", gl::ParseResourceName("foo", &indices));
    EXPECT_TRUE(indices.empty());
}

// The ParseResourceName function should work when a nullptr is passed as the indices output vector.
TEST(ParseResourceName, NULLArrayIndices)
{
    EXPECT_EQ("foo", gl::ParseResourceName("foo[10]", nullptr));
}

// Parsing multiple array indices should result in outermost array indices being last in the vector.
TEST(ParseResourceName, MultipleArrayIndices)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo", gl::ParseResourceName("foo[12][34][56]", &indices));
    ASSERT_EQ(3u, indices.size());
    // Indices are sorted with outermost array index last.
    EXPECT_EQ(56u, indices[0]);
    EXPECT_EQ(34u, indices[1]);
    EXPECT_EQ(12u, indices[2]);
}

// Trailing whitespace should not be accepted by ParseResourceName.
TEST(ParseResourceName, TrailingWhitespace)
{
    std::vector<unsigned int> indices;
    EXPECT_EQ("foo ", gl::ParseResourceName("foo ", &indices));
    EXPECT_TRUE(indices.empty());

    EXPECT_EQ("foo[10] ", gl::ParseResourceName("foo[10] ", &indices));
    EXPECT_TRUE(indices.empty());

    EXPECT_EQ("foo[10][20] ", gl::ParseResourceName("foo[10][20] ", &indices));
    EXPECT_TRUE(indices.empty());
}

// Parse a string without any index.
TEST(ParseArrayIndex, NoArrayIndex)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX, gl::ParseArrayIndex("foo", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(3u, nameLengthWithoutArrayIndex);
}

// Parse an empty string for an array index.
TEST(ParseArrayIndex, EmptyString)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX, gl::ParseArrayIndex("", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(0u, nameLengthWithoutArrayIndex);
}

// A valid array index is parsed correctly from the end of the string.
TEST(ParseArrayIndex, ArrayIndex)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(123u, gl::ParseArrayIndex("foo[123]", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(3u, nameLengthWithoutArrayIndex);
}

// An array index from the middle of the string is not parsed.
TEST(ParseArrayIndex, ArrayIndexInMiddle)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX, gl::ParseArrayIndex("foo[123].bar", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(12u, nameLengthWithoutArrayIndex);
}

// Trailing whitespace in the parsed string is taken into account.
TEST(ParseArrayIndex, TrailingWhitespace)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX, gl::ParseArrayIndex("foo[123] ", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(9u, nameLengthWithoutArrayIndex);
}

// Only the last index is parsed.
TEST(ParseArrayIndex, MultipleArrayIndices)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(34u, gl::ParseArrayIndex("foo[12][34]", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(7u, nameLengthWithoutArrayIndex);
}

// GetProgramResourceLocation spec in GLES 3.1 November 2016 page 87 mentions "decimal" integer.
// So an integer in hexadecimal format should not parse as an array index.
TEST(ParseArrayIndex, HexArrayIndex)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX, gl::ParseArrayIndex("foo[0xff]", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(9u, nameLengthWithoutArrayIndex);
}

// GetProgramResourceLocation spec in GLES 3.1 November 2016 page 87 mentions that the array
// index should not contain a leading plus sign.
TEST(ParseArrayIndex, ArrayIndexLeadingPlus)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX, gl::ParseArrayIndex("foo[+1]", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(7u, nameLengthWithoutArrayIndex);
}

// GetProgramResourceLocation spec in GLES 3.1 November 2016 page 87 says that index should not
// contain whitespace. Test leading whitespace.
TEST(ParseArrayIndex, ArrayIndexLeadingWhiteSpace)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX, gl::ParseArrayIndex("foo[ 0]", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(7u, nameLengthWithoutArrayIndex);
}

// GetProgramResourceLocation spec in GLES 3.1 November 2016 page 87 says that index should not
// contain whitespace. Test trailing whitespace.
TEST(ParseArrayIndex, ArrayIndexTrailingWhiteSpace)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX, gl::ParseArrayIndex("foo[0 ]", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(7u, nameLengthWithoutArrayIndex);
}

// GetProgramResourceLocation spec in GLES 3.1 November 2016 page 87 says that index should only
// contain an integer.
TEST(ParseArrayIndex, ArrayIndexBogus)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX, gl::ParseArrayIndex("foo[0bogus]", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(11u, nameLengthWithoutArrayIndex);
}

// Verify that using an index value out-of-range fails.
TEST(ParseArrayIndex, ArrayIndexOutOfRange)
{
    size_t nameLengthWithoutArrayIndex;
    EXPECT_EQ(GL_INVALID_INDEX,
              gl::ParseArrayIndex("foo[4294967296]", &nameLengthWithoutArrayIndex));
    EXPECT_EQ(15u, nameLengthWithoutArrayIndex);
}

// Test that ConstStrLen works.
TEST(Utilities, ConstStrLen)
{
    constexpr auto v1 = angle::ConstStrLen(nullptr);
    EXPECT_EQ(0u, v1);
    constexpr auto v2 = angle::ConstStrLen("");
    EXPECT_EQ(0u, v2);
    constexpr auto v3 = angle::ConstStrLen("a");
    EXPECT_EQ(1u, v3);
    constexpr char c[5] = "cc\0c";
    constexpr auto v4   = angle::ConstStrLen(c);
    EXPECT_EQ(2u, v4);
    constexpr char d[] = "dddd";
    constexpr auto v5  = angle::ConstStrLen(d);
    EXPECT_EQ(4u, v5);
    constexpr char *e = nullptr;
    constexpr auto v6 = angle::ConstStrLen(e);
    EXPECT_EQ(0u, v6);

    // Non-constexpr invocations
    const char cc[5] = "cc\0c";
    auto n1          = angle::ConstStrLen(cc);
    EXPECT_EQ(2u, n1);
    const char *dd = "ddd";
    auto n2        = angle::ConstStrLen(dd);
    EXPECT_EQ(3u, n2);
}

}  // anonymous namespace
