// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/void.h"

namespace tint::core::type {
namespace {

using ArrayTest = TestHelper;

TEST_F(ArrayTest, CreateSizedArray) {
    Manager ty;
    auto* a = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* b = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* c = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(3u), 4u, 8u, 32u, 16u);
    auto* d = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 5u, 8u, 32u, 16u);
    auto* e = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 9u, 32u, 16u);
    auto* f = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 8u, 33u, 16u);
    auto* g = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 8u, 33u, 17u);

    EXPECT_EQ(a->ElemType(), ty.u32());
    EXPECT_EQ(a->Count(), ty.Get<ConstantArrayCount>(2u));
    EXPECT_EQ(a->Align(), 4u);
    EXPECT_EQ(a->Size(), 8u);
    EXPECT_EQ(a->Stride(), 32u);
    EXPECT_EQ(a->ImplicitStride(), 16u);
    EXPECT_FALSE(a->IsStrideImplicit());
    EXPECT_FALSE(a->Count()->Is<RuntimeArrayCount>());

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
    EXPECT_NE(a, f);
    EXPECT_NE(a, g);
}

TEST_F(ArrayTest, CreateRuntimeArray) {
    Manager ty;
    auto* a = ty.Get<Array>(ty.u32(), ty.Get<RuntimeArrayCount>(), 4u, 8u, 32u, 32u);
    auto* b = ty.Get<Array>(ty.u32(), ty.Get<RuntimeArrayCount>(), 4u, 8u, 32u, 32u);
    auto* c = ty.Get<Array>(ty.u32(), ty.Get<RuntimeArrayCount>(), 5u, 8u, 32u, 32u);
    auto* d = ty.Get<Array>(ty.u32(), ty.Get<RuntimeArrayCount>(), 4u, 9u, 32u, 32u);
    auto* e = ty.Get<Array>(ty.u32(), ty.Get<RuntimeArrayCount>(), 4u, 8u, 33u, 32u);
    auto* f = ty.Get<Array>(ty.u32(), ty.Get<RuntimeArrayCount>(), 4u, 8u, 33u, 17u);

    EXPECT_EQ(a->ElemType(), ty.u32());
    EXPECT_EQ(a->Count(), ty.Get<RuntimeArrayCount>());
    EXPECT_EQ(a->Align(), 4u);
    EXPECT_EQ(a->Size(), 8u);
    EXPECT_EQ(a->Stride(), 32u);
    EXPECT_EQ(a->ImplicitStride(), 32u);
    EXPECT_TRUE(a->IsStrideImplicit());
    EXPECT_TRUE(a->Count()->Is<RuntimeArrayCount>());

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
    EXPECT_NE(a, f);
}

TEST_F(ArrayTest, Hash) {
    Manager ty;
    auto* a = ty.array(ty.u32(), 2u);
    auto* b = ty.array(ty.u32(), 2u);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(ArrayTest, Equals) {
    Manager ty;
    auto* a = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* b = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* c = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(3u), 4u, 8u, 32u, 16u);
    auto* d = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 5u, 8u, 32u, 16u);
    auto* e = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 9u, 32u, 16u);
    auto* f = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 8u, 33u, 16u);
    auto* g = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 8u, 33u, 17u);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(*f));
    EXPECT_FALSE(a->Equals(*g));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(ArrayTest, FriendlyNameRuntimeSized) {
    Manager ty;
    auto* arr = ty.runtime_array(ty.i32());
    EXPECT_EQ(arr->FriendlyName(), "array<i32>");
}

TEST_F(ArrayTest, FriendlyNameStaticSized) {
    Manager ty;
    auto* arr = ty.array(ty.i32(), 5u);
    EXPECT_EQ(arr->FriendlyName(), "array<i32, 5>");
}

TEST_F(ArrayTest, FriendlyNameRuntimeSizedNonImplicitStride) {
    Manager ty;
    auto* arr = ty.runtime_array(ty.i32(), 8u);
    EXPECT_EQ(arr->FriendlyName(), "@stride(8) array<i32>");
}

TEST_F(ArrayTest, FriendlyNameStaticSizedNonImplicitStride) {
    Manager ty;
    auto* arr = ty.array(ty.i32(), 5u, 8u);
    EXPECT_EQ(arr->FriendlyName(), "@stride(8) array<i32, 5>");
}

TEST_F(ArrayTest, IsConstructable) {
    Manager ty;
    auto* fixed_sized = ty.array(ty.u32(), 2u);
    auto* runtime_sized = ty.runtime_array(ty.u32());

    EXPECT_TRUE(fixed_sized->IsConstructible());
    EXPECT_FALSE(runtime_sized->IsConstructible());
}

TEST_F(ArrayTest, HasCreationFixedFootprint) {
    Manager ty;
    auto* fixed_sized = ty.array(ty.u32(), 2u);
    auto* runtime_sized = ty.runtime_array(ty.u32());

    EXPECT_TRUE(fixed_sized->HasCreationFixedFootprint());
    EXPECT_FALSE(runtime_sized->HasCreationFixedFootprint());
}

TEST_F(ArrayTest, HasFixedFootprint) {
    Manager ty;
    auto* fixed_sized = ty.array(ty.u32(), 2u);
    auto* runtime_sized = ty.runtime_array(ty.u32());

    EXPECT_TRUE(fixed_sized->HasFixedFootprint());
    EXPECT_FALSE(runtime_sized->HasFixedFootprint());
}

TEST_F(ArrayTest, CloneSizedArray) {
    Manager ty;
    auto* ary = ty.Get<Array>(ty.u32(), ty.Get<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* val = ary->Clone(ctx);

    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->ElemType()->Is<U32>());
    EXPECT_TRUE(val->Count()->Is<ConstantArrayCount>());
    EXPECT_EQ(val->Count()->As<ConstantArrayCount>()->value, 2u);
    EXPECT_EQ(val->Align(), 4u);
    EXPECT_EQ(val->Size(), 8u);
    EXPECT_EQ(val->Stride(), 32u);
    EXPECT_EQ(val->ImplicitStride(), 16u);
    EXPECT_FALSE(val->IsStrideImplicit());
}

TEST_F(ArrayTest, CloneRuntimeArray) {
    Manager ty;
    auto* ary = ty.Get<Array>(ty.u32(), ty.Get<RuntimeArrayCount>(), 4u, 8u, 32u, 32u);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* val = ary->Clone(ctx);
    ASSERT_NE(val, nullptr);
    EXPECT_TRUE(val->ElemType()->Is<U32>());
    EXPECT_TRUE(val->Count()->Is<RuntimeArrayCount>());
    EXPECT_EQ(val->Align(), 4u);
    EXPECT_EQ(val->Size(), 8u);
    EXPECT_EQ(val->Stride(), 32u);
    EXPECT_EQ(val->ImplicitStride(), 32u);
    EXPECT_TRUE(val->IsStrideImplicit());
}

}  // namespace
}  // namespace tint::core::type
