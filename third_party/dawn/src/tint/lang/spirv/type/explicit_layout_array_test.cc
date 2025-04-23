// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/type/explicit_layout_array.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/u32.h"

namespace tint::spirv::type {
namespace {

using ExplicitLayoutArrayTest = core::type::TestHelper;

TEST_F(ExplicitLayoutArrayTest, Hash) {
    auto* a = create<ExplicitLayoutArray>(create<core::type::U32>(),
                                          create<core::type::ConstantArrayCount>(2u), 4u, 8u, 32u);
    auto* b = create<ExplicitLayoutArray>(create<core::type::U32>(),
                                          create<core::type::ConstantArrayCount>(2u), 4u, 8u, 32u);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(ExplicitLayoutArrayTest, Equals) {
    auto* count = create<core::type::ConstantArrayCount>(4u);
    auto* a = create<ExplicitLayoutArray>(create<core::type::I32>(), count, 4u, 16u, 4u);
    auto* b = create<ExplicitLayoutArray>(create<core::type::I32>(), count, 4u, 16u, 4u);
    auto* c = create<ExplicitLayoutArray>(create<core::type::U32>(), count, 4u, 16u, 4u);

    // Make sure it does not match the equivalent regular array.
    auto* d = create<core::type::Array>(create<core::type::I32>(), count, 4u, 16u, 4u, 4u);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

TEST_F(ExplicitLayoutArrayTest, FriendlyName) {
    auto* count = create<core::type::ConstantArrayCount>(4u);
    auto* a = create<ExplicitLayoutArray>(create<core::type::U32>(), count, 4u, 16u, 4u);
    EXPECT_EQ(a->FriendlyName(), "spirv.explicit_layout_array<u32, 4>");
}

TEST_F(ExplicitLayoutArrayTest, CloneArray) {
    auto* ary = create<ExplicitLayoutArray>(
        create<core::type::U32>(), create<core::type::ConstantArrayCount>(2u), 4u, 8u, 32u);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* val = ary->Clone(ctx);

    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->ElemType()->Is<core::type::U32>());
    EXPECT_NE(val->ElemType(), ary->ElemType());
    EXPECT_TRUE(val->Count()->Is<core::type::ConstantArrayCount>());
    EXPECT_EQ(val->Count()->As<core::type::ConstantArrayCount>()->value, 2u);
    EXPECT_NE(val->Count(), ary->Count());
    EXPECT_EQ(val->Align(), 4u);
    EXPECT_EQ(val->Size(), 8u);
    EXPECT_EQ(val->Stride(), 32u);
}

}  // namespace
}  // namespace tint::spirv::type
