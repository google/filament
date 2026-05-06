// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/native/ShaderModule.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"

namespace dawn {
namespace {

class CountUTF16CodeUnitsFromUTF8StringTest : public ValidationTest {};

TEST_F(CountUTF16CodeUnitsFromUTF8StringTest, ValidUnicodeString) {
    struct TestCase {
        const char* u8String;
        uint64_t lengthInUTF16;
    };

    // Referenced from src/tint/utils/text/unicode_test.cc
    constexpr std::array<TestCase, 14> kTestCases = {{
        {"", 0},
        {"abc", 3},
        {"\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c", 4},
        {"def\xf0\x9f\x91\x8b\xf0\x9f\x8c\x8e", 7},
        {"\xed\x9f\xbf", 1},      // CodePoint == 0xD7FF
        {"\xed\x9f\xbe", 1},      // CodePoint == 0xD7FF - 1
        {"\xee\x80\x80", 1},      // CodePoint == 0xE000
        {"\xee\x80\x81", 1},      // CodePoint == 0xE000 + 1
        {"\xef\xbf\xbf", 1},      // CodePoint == 0xFFFF
        {"\xef\xbf\xbe", 1},      // CodePoint == 0xFFFF - 1
        {"\xf0\x90\x80\x80", 2},  // CodePoint == 0x10000
        {"\xf0\x90\x80\x81", 2},  // CodePoint == 0x10000 + 1

        // While surrogates are technically invalid, CountUTF16CodeUnitsFromUTF8String supports and
        // counts them as a single UTF-16 code unit.
        {"\xed\xa0\x80", 1},  // CodePoint == 0xD7FF + 1
        {"\xed\xbf\xbf", 1},  // CodePoint == 0xE000 - 1
    }};

    for (const TestCase& testCase : kTestCases) {
        ASSERT_EQ(testCase.lengthInUTF16,
                  native::CountUTF16CodeUnitsFromUTF8String(std::string_view(testCase.u8String)));
    }
}

}  // anonymous namespace
}  // namespace dawn
