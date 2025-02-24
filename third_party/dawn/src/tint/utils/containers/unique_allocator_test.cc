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

#include "src/tint/utils/containers/unique_allocator.h"

#include <string>

#include "gtest/gtest.h"

namespace tint {
namespace {

TEST(UniqueAllocator, Int) {
    UniqueAllocator<int> a;
    EXPECT_NE(a.Get(0), a.Get(1));
    EXPECT_NE(a.Get(1), a.Get(2));
    EXPECT_EQ(a.Get(0), a.Get(0));
    EXPECT_EQ(a.Get(1), a.Get(1));
    EXPECT_EQ(a.Get(2), a.Get(2));
}

TEST(UniqueAllocator, Float) {
    UniqueAllocator<float> a;
    EXPECT_NE(a.Get(0.1f), a.Get(1.1f));
    EXPECT_NE(a.Get(1.1f), a.Get(2.1f));
    EXPECT_EQ(a.Get(0.1f), a.Get(0.1f));
    EXPECT_EQ(a.Get(1.1f), a.Get(1.1f));
    EXPECT_EQ(a.Get(2.1f), a.Get(2.1f));
}

TEST(UniqueAllocator, String) {
    UniqueAllocator<std::string> a;
    EXPECT_NE(a.Get("x"), a.Get("y"));
    EXPECT_NE(a.Get("z"), a.Get("w"));
    EXPECT_EQ(a.Get("x"), a.Get("x"));
    EXPECT_EQ(a.Get("y"), a.Get("y"));
    EXPECT_EQ(a.Get("z"), a.Get("z"));
}

}  // namespace
}  // namespace tint
