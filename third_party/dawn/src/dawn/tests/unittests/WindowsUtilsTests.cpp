// Copyright 2021 The Dawn & Tint Authors
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

#include <gtest/gtest.h>

#include "dawn/common/WindowsUtils.h"

namespace dawn {
namespace {

TEST(WindowsUtilsTests, WCharToUTF8) {
    // Test the empty string
    ASSERT_EQ("", WCharToUTF8(L""));

    // Test ASCII characters
    ASSERT_EQ("abc", WCharToUTF8(L"abc"));

    // Test ASCII characters
    ASSERT_EQ("abc", WCharToUTF8(L"abc"));

    // Test two-byte utf8 character
    ASSERT_EQ("\xd1\x90", WCharToUTF8(L"\x450"));

    // Test three-byte utf8 codepoint
    ASSERT_EQ("\xe1\x81\x90", WCharToUTF8(L"\x1050"));
}

TEST(WindowsUtilsTests, UTF8ToWStr) {
    // Test the empty string
    ASSERT_EQ(L"", UTF8ToWStr(""));

    // Test ASCII characters
    ASSERT_EQ(L"abc", UTF8ToWStr("abc"));

    // Test ASCII characters
    ASSERT_EQ(L"abc", UTF8ToWStr("abc"));

    // Test two-byte utf8 character
    ASSERT_EQ(L"\x450", UTF8ToWStr("\xd1\x90"));

    // Test three-byte utf8 codepoint
    ASSERT_EQ(L"\x1050", UTF8ToWStr("\xe1\x81\x90"));
}

}  // anonymous namespace
}  // namespace dawn
