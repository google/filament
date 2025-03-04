// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/text/base64.h"

#include <optional>
#include <vector>

#include "gtest/gtest.h"

#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/text/string.h"

namespace tint::utils {
namespace {

struct DecodeBase64Case {
    char in;
    std::optional<uint8_t> out;
};

using DecodeBase64Test = testing::TestWithParam<DecodeBase64Case>;

TEST_P(DecodeBase64Test, Char) {
    EXPECT_EQ(DecodeBase64(GetParam().in), GetParam().out);
}

INSTANTIATE_TEST_SUITE_P(Valid,
                         DecodeBase64Test,
                         testing::Values(DecodeBase64Case{'A', 0},
                                         DecodeBase64Case{'B', 1},
                                         DecodeBase64Case{'C', 2},
                                         DecodeBase64Case{'D', 3},
                                         DecodeBase64Case{'E', 4},
                                         DecodeBase64Case{'F', 5},
                                         DecodeBase64Case{'G', 6},
                                         DecodeBase64Case{'H', 7},
                                         DecodeBase64Case{'I', 8},
                                         DecodeBase64Case{'J', 9},
                                         DecodeBase64Case{'K', 10},
                                         DecodeBase64Case{'L', 11},
                                         DecodeBase64Case{'M', 12},
                                         DecodeBase64Case{'N', 13},
                                         DecodeBase64Case{'O', 14},
                                         DecodeBase64Case{'P', 15},
                                         DecodeBase64Case{'Q', 16},
                                         DecodeBase64Case{'R', 17},
                                         DecodeBase64Case{'S', 18},
                                         DecodeBase64Case{'T', 19},
                                         DecodeBase64Case{'U', 20},
                                         DecodeBase64Case{'V', 21},
                                         DecodeBase64Case{'W', 22},
                                         DecodeBase64Case{'X', 23},
                                         DecodeBase64Case{'Y', 24},
                                         DecodeBase64Case{'Z', 25},
                                         DecodeBase64Case{'a', 26},
                                         DecodeBase64Case{'b', 27},
                                         DecodeBase64Case{'c', 28},
                                         DecodeBase64Case{'d', 29},
                                         DecodeBase64Case{'e', 30},
                                         DecodeBase64Case{'f', 31},
                                         DecodeBase64Case{'g', 32},
                                         DecodeBase64Case{'h', 33},
                                         DecodeBase64Case{'i', 34},
                                         DecodeBase64Case{'j', 35},
                                         DecodeBase64Case{'k', 36},
                                         DecodeBase64Case{'l', 37},
                                         DecodeBase64Case{'m', 38},
                                         DecodeBase64Case{'n', 39},
                                         DecodeBase64Case{'o', 40},
                                         DecodeBase64Case{'p', 41},
                                         DecodeBase64Case{'q', 42},
                                         DecodeBase64Case{'r', 43},
                                         DecodeBase64Case{'s', 44},
                                         DecodeBase64Case{'t', 45},
                                         DecodeBase64Case{'u', 46},
                                         DecodeBase64Case{'v', 47},
                                         DecodeBase64Case{'w', 48},
                                         DecodeBase64Case{'x', 49},
                                         DecodeBase64Case{'y', 50},
                                         DecodeBase64Case{'z', 51},
                                         DecodeBase64Case{'0', 52},
                                         DecodeBase64Case{'1', 53},
                                         DecodeBase64Case{'2', 54},
                                         DecodeBase64Case{'3', 55},
                                         DecodeBase64Case{'4', 56},
                                         DecodeBase64Case{'5', 57},
                                         DecodeBase64Case{'6', 58},
                                         DecodeBase64Case{'7', 59},
                                         DecodeBase64Case{'8', 60},
                                         DecodeBase64Case{'9', 61},
                                         DecodeBase64Case{'+', 62},
                                         DecodeBase64Case{'/', 63}));

INSTANTIATE_TEST_SUITE_P(Invalid,
                         DecodeBase64Test,
                         testing::Values(DecodeBase64Case{'@', std::nullopt},
                                         DecodeBase64Case{'#', std::nullopt},
                                         DecodeBase64Case{'^', std::nullopt},
                                         DecodeBase64Case{'&', std::nullopt},
                                         DecodeBase64Case{'!', std::nullopt},
                                         DecodeBase64Case{'*', std::nullopt},
                                         DecodeBase64Case{'(', std::nullopt},
                                         DecodeBase64Case{')', std::nullopt},
                                         DecodeBase64Case{'-', std::nullopt},
                                         DecodeBase64Case{'.', std::nullopt},
                                         DecodeBase64Case{'\0', std::nullopt},
                                         DecodeBase64Case{'\n', std::nullopt}));

INSTANTIATE_TEST_SUITE_P(Padding,
                         DecodeBase64Test,
                         testing::Values(DecodeBase64Case{'=', std::nullopt}));

struct DecodeBase64FromCommentsCase {
    std::string_view wgsl;
    Vector<int, 0> expected;
};

static std::ostream& operator<<(std::ostream& o, const DecodeBase64FromCommentsCase& c) {
    return o << "'" << ReplaceAll(c.wgsl, "\n", "␤") << "'";
}

using DecodeBase64FromCommentsTest = ::testing::TestWithParam<DecodeBase64FromCommentsCase>;

TEST_P(DecodeBase64FromCommentsTest, None) {
    auto got_bytes = DecodeBase64FromComments(GetParam().wgsl);
    auto got = Transform(got_bytes, [](std::byte byte) { return static_cast<int>(byte); });
    EXPECT_EQ(got, GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(,
                         DecodeBase64FromCommentsTest,
                         testing::ValuesIn(std::vector<DecodeBase64FromCommentsCase>{
                             {"", Empty},
                             {"//", Empty},
                             {"abc", Empty},
                             {"abc//", Empty},
                             {
                                 R"(a
b
c)",
                                 Empty,
                             },
                             {"// abc", {26, 27, 28}},
                             {"a // bc", {27, 28}},
                             {"ab // c", {28}},
                             {"// a.b.c", {26, 27, 28}},
                             {
                                 R"(a
b
c)",
                                 Empty,
                             },
                             {
                                 R"(a
// b
c)",
                                 {27},
                             },
                             {
                                 R"(// a
// b
// c)",
                                 {26, 27, 28},
                             },
                             {
                                 R"(/* a
b
c
*/)",
                                 {26, 27, 28},
                             },
                             {
                                 R"(/* a
b
*/
c)",
                                 {26, 27},
                             },
                             {
                                 R"(a/*
b
*/
c)",
                                 {27},
                             },
                             {
                                 "x/*a*/b/*c*/y",
                                 {26, 28},
                             },
                             {
                                 "x/*a/*b*/c*/z",
                                 {26, 27, 28},
                             },
                         }));

}  // namespace
}  // namespace tint::utils
