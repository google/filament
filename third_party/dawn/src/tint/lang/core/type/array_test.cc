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

#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/wgsl/sem/array_count.h"

namespace tint::core::type {
namespace {

using ArrayTest = TestHelper;

TEST_F(ArrayTest, CreateSizedArray) {
    auto* a = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* b = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* c = create<Array>(create<U32>(), create<ConstantArrayCount>(3u), 4u, 8u, 32u, 16u);
    auto* d = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 5u, 8u, 32u, 16u);
    auto* e = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 9u, 32u, 16u);
    auto* f = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 33u, 16u);
    auto* g = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 33u, 17u);

    EXPECT_EQ(a->ElemType(), create<U32>());
    EXPECT_EQ(a->Count(), create<ConstantArrayCount>(2u));
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
    auto* a = create<Array>(create<U32>(), create<RuntimeArrayCount>(), 4u, 8u, 32u, 32u);
    auto* b = create<Array>(create<U32>(), create<RuntimeArrayCount>(), 4u, 8u, 32u, 32u);
    auto* c = create<Array>(create<U32>(), create<RuntimeArrayCount>(), 5u, 8u, 32u, 32u);
    auto* d = create<Array>(create<U32>(), create<RuntimeArrayCount>(), 4u, 9u, 32u, 32u);
    auto* e = create<Array>(create<U32>(), create<RuntimeArrayCount>(), 4u, 8u, 33u, 32u);
    auto* f = create<Array>(create<U32>(), create<RuntimeArrayCount>(), 4u, 8u, 33u, 17u);

    EXPECT_EQ(a->ElemType(), create<U32>());
    EXPECT_EQ(a->Count(), create<RuntimeArrayCount>());
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
    auto* a = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* b = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(ArrayTest, Equals) {
    auto* a = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* b = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* c = create<Array>(create<U32>(), create<ConstantArrayCount>(3u), 4u, 8u, 32u, 16u);
    auto* d = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 5u, 8u, 32u, 16u);
    auto* e = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 9u, 32u, 16u);
    auto* f = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 33u, 16u);
    auto* g = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 33u, 17u);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(*f));
    EXPECT_FALSE(a->Equals(*g));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(ArrayTest, FriendlyNameRuntimeSized) {
    auto* arr = create<Array>(create<I32>(), create<RuntimeArrayCount>(), 0u, 4u, 4u, 4u);
    EXPECT_EQ(arr->FriendlyName(), "array<i32>");
}

TEST_F(ArrayTest, FriendlyNameStaticSized) {
    auto* arr = create<Array>(create<I32>(), create<ConstantArrayCount>(5u), 4u, 20u, 4u, 4u);
    EXPECT_EQ(arr->FriendlyName(), "array<i32, 5>");
}

TEST_F(ArrayTest, FriendlyNameRuntimeSizedNonImplicitStride) {
    auto* arr = create<Array>(create<I32>(), create<RuntimeArrayCount>(), 0u, 4u, 8u, 4u);
    EXPECT_EQ(arr->FriendlyName(), "@stride(8) array<i32>");
}

TEST_F(ArrayTest, FriendlyNameStaticSizedNonImplicitStride) {
    auto* arr = create<Array>(create<I32>(), create<ConstantArrayCount>(5u), 4u, 20u, 8u, 4u);
    EXPECT_EQ(arr->FriendlyName(), "@stride(8) array<i32, 5>");
}

TEST_F(ArrayTest, IsConstructable) {
    auto* fixed_sized =
        create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* named_override_sized = create<Array>(
        create<U32>(), create<sem::NamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);
    auto* unnamed_override_sized = create<Array>(
        create<U32>(), create<sem::UnnamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);
    auto* runtime_sized =
        create<Array>(create<U32>(), create<RuntimeArrayCount>(), 4u, 8u, 32u, 16u);

    EXPECT_TRUE(fixed_sized->IsConstructible());
    EXPECT_FALSE(named_override_sized->IsConstructible());
    EXPECT_FALSE(unnamed_override_sized->IsConstructible());
    EXPECT_FALSE(runtime_sized->IsConstructible());
}

TEST_F(ArrayTest, HasCreationFixedFootprint) {
    auto* fixed_sized =
        create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* named_override_sized = create<Array>(
        create<U32>(), create<sem::NamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);
    auto* unnamed_override_sized = create<Array>(
        create<U32>(), create<sem::UnnamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);
    auto* runtime_sized =
        create<Array>(create<U32>(), create<RuntimeArrayCount>(), 4u, 8u, 32u, 16u);

    EXPECT_TRUE(fixed_sized->HasCreationFixedFootprint());
    EXPECT_FALSE(named_override_sized->HasCreationFixedFootprint());
    EXPECT_FALSE(unnamed_override_sized->HasCreationFixedFootprint());
    EXPECT_FALSE(runtime_sized->HasCreationFixedFootprint());
}

TEST_F(ArrayTest, HasFixedFootprint) {
    auto* fixed_sized =
        create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);
    auto* named_override_sized = create<Array>(
        create<U32>(), create<sem::NamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);
    auto* unnamed_override_sized = create<Array>(
        create<U32>(), create<sem::UnnamedOverrideArrayCount>(nullptr), 4u, 8u, 32u, 16u);
    auto* runtime_sized =
        create<Array>(create<U32>(), create<RuntimeArrayCount>(), 4u, 8u, 32u, 16u);

    EXPECT_TRUE(fixed_sized->HasFixedFootprint());
    EXPECT_TRUE(named_override_sized->HasFixedFootprint());
    EXPECT_TRUE(unnamed_override_sized->HasFixedFootprint());
    EXPECT_FALSE(runtime_sized->HasFixedFootprint());
}

TEST_F(ArrayTest, CloneSizedArray) {
    auto* ary = create<Array>(create<U32>(), create<ConstantArrayCount>(2u), 4u, 8u, 32u, 16u);

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
    auto* ary = create<Array>(create<U32>(), create<RuntimeArrayCount>(), 4u, 8u, 32u, 32u);

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
