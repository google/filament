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

#include "src/tint/utils/bytes/swap.h"

#include "gtest/gtest.h"

namespace tint::bytes {
namespace {

TEST(BytesSwapTest, Uint) {
    EXPECT_EQ(Swap<uint8_t>(0x41), static_cast<uint8_t>(0x41));
    EXPECT_EQ(Swap<uint16_t>(0x4152), static_cast<uint16_t>(0x5241));
    EXPECT_EQ(Swap<uint32_t>(0x41526374), static_cast<uint32_t>(0x74635241));
    EXPECT_EQ(Swap<uint64_t>(0x415263748596A7B8), static_cast<uint64_t>(0xB8A7968574635241));
}

TEST(BytesSwapTest, Sint) {
    EXPECT_EQ(Swap<int8_t>(0x41), static_cast<int8_t>(0x41));
    EXPECT_EQ(Swap<int8_t>(-0x41), static_cast<int8_t>(-0x41));
    EXPECT_EQ(Swap<int16_t>(0x4152), static_cast<int16_t>(0x5241));
    EXPECT_EQ(Swap<int16_t>(-0x4152), static_cast<int16_t>(0xAEBE));
    EXPECT_EQ(Swap<int32_t>(0x41526374), static_cast<int32_t>(0x74635241));
    EXPECT_EQ(Swap<int32_t>(-0x41526374), static_cast<int32_t>(0x8C9CADBE));
    EXPECT_EQ(Swap<int64_t>(0x415263748596A7B8), static_cast<int64_t>(0xB8A7968574635241));
    EXPECT_EQ(Swap<int64_t>(-0x415263748596A7B8), static_cast<int64_t>(0x4858697A8B9CADBE));
}

}  // namespace
}  // namespace tint::bytes
