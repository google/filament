// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/i8.h"

#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/u8.h"

namespace tint::core::type {
namespace {

using I8Test = TestHelper;

TEST_F(I8Test, Creation) {
    auto* a = create<I8>();
    auto* b = create<I8>();
    EXPECT_EQ(a, b);
}

TEST_F(I8Test, SizeAndAlign) {
    auto* a = create<I8>();
    EXPECT_EQ(a->Size(), 1u);
    EXPECT_EQ(a->Align(), 1u);
}

TEST_F(I8Test, Hash) {
    auto* a = create<I8>();
    auto* b = create<I8>();
    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(I8Test, Equals) {
    auto* a = create<I8>();
    auto* b = create<I8>();
    auto* c = create<U8>();
    auto* d = create<I32>();
    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
}

TEST_F(I8Test, FriendlyName) {
    I8 i;
    EXPECT_EQ(i.FriendlyName(), "i8");
}

TEST_F(I8Test, Clone) {
    auto* a = create<I8>();

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* b = a->Clone(ctx);
    ASSERT_TRUE(b->Is<I8>());
}

}  // namespace
}  // namespace tint::core::type
