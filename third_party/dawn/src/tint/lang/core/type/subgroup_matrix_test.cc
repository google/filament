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

#include "src/tint/lang/core/type/helper_test.h"

#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/subgroup_matrix.h"

namespace tint::core::type {
namespace {

using SubgroupMatrixTest = TestHelper;

TEST_F(SubgroupMatrixTest, Creation) {
    auto* f32 = create<F32>();

    auto* l1 = create<SubgroupMatrix>(SubgroupMatrixKind::kLeft, f32, 3u, 4u);

    EXPECT_EQ(l1->Type(), f32);
    EXPECT_EQ(l1->Kind(), SubgroupMatrixKind::kLeft);
    EXPECT_EQ(l1->Columns(), 3u);
    EXPECT_EQ(l1->Rows(), 4u);
}

TEST_F(SubgroupMatrixTest, Creation_TypeManager) {
    core::type::Manager mgr;

    {
        auto* l = mgr.subgroup_matrix(SubgroupMatrixKind::kRight, mgr.f32(), 2, 4);
        ASSERT_NE(l, nullptr);
        EXPECT_EQ(SubgroupMatrixKind::kRight, l->Kind());
        EXPECT_EQ(mgr.f32(), l->Type());
        EXPECT_EQ(2u, l->Columns());
        EXPECT_EQ(4u, l->Rows());
    }

    {
        auto* l = mgr.subgroup_matrix_right(mgr.f32(), 2, 4);
        EXPECT_EQ(SubgroupMatrixKind::kRight, l->Kind());
    }
    {
        auto* l = mgr.subgroup_matrix_left(mgr.f32(), 2, 4);
        EXPECT_EQ(SubgroupMatrixKind::kLeft, l->Kind());
    }
    {
        auto* l = mgr.subgroup_matrix_result(mgr.f32(), 2, 4);
        EXPECT_EQ(SubgroupMatrixKind::kResult, l->Kind());
    }
}

TEST_F(SubgroupMatrixTest, Hash) {
    auto* a = create<SubgroupMatrix>(SubgroupMatrixKind::kRight, create<I32>(), 3u, 4u);
    auto* b = create<SubgroupMatrix>(SubgroupMatrixKind::kRight, create<I32>(), 3u, 4u);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(SubgroupMatrixTest, Equals) {
    auto* f32 = create<F32>();
    auto* i8 = create<I8>();

    auto* l1 = create<SubgroupMatrix>(SubgroupMatrixKind::kLeft, f32, 3u, 4u);
    auto* l2 = create<SubgroupMatrix>(SubgroupMatrixKind::kLeft, f32, 3u, 4u);
    auto* l3 = create<SubgroupMatrix>(SubgroupMatrixKind::kLeft, i8, 3u, 4u);
    auto* l4 = create<SubgroupMatrix>(SubgroupMatrixKind::kLeft, f32, 4u, 3u);

    auto* r1 = create<SubgroupMatrix>(SubgroupMatrixKind::kRight, f32, 3u, 4u);
    auto* r2 = create<SubgroupMatrix>(SubgroupMatrixKind::kRight, f32, 3u, 4u);
    auto* res1 = create<SubgroupMatrix>(SubgroupMatrixKind::kResult, f32, 3u, 4u);
    auto* res2 = create<SubgroupMatrix>(SubgroupMatrixKind::kResult, f32, 3u, 4u);

    EXPECT_EQ(l1, l2);
    EXPECT_NE(l1, l3);
    EXPECT_NE(l1, l4);
    EXPECT_NE(l1, r1);
    EXPECT_NE(l1, res1);

    EXPECT_EQ(r1, r2);
    EXPECT_NE(r1, res1);

    EXPECT_EQ(res1, res2);
}

TEST_F(SubgroupMatrixTest, FriendlyName_Left) {
    I8 i8;
    SubgroupMatrix m{SubgroupMatrixKind::kLeft, &i8, 2, 4};
    EXPECT_EQ(m.FriendlyName(), "subgroup_matrix_left<i8, 2, 4>");
}

TEST_F(SubgroupMatrixTest, FriendlyName_Right) {
    F32 f32;
    SubgroupMatrix m{SubgroupMatrixKind::kRight, &f32, 8, 8};
    EXPECT_EQ(m.FriendlyName(), "subgroup_matrix_right<f32, 8, 8>");
}

TEST_F(SubgroupMatrixTest, FriendlyName_Result) {
    U32 u32;
    SubgroupMatrix m{SubgroupMatrixKind::kResult, &u32, 32, 32};
    EXPECT_EQ(m.FriendlyName(), "subgroup_matrix_result<u32, 32, 32>");
}

TEST_F(SubgroupMatrixTest, Clone) {
    auto* a = create<SubgroupMatrix>(SubgroupMatrixKind::kResult, create<I32>(), 3u, 4u);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* s = a->Clone(ctx);
    EXPECT_EQ(SubgroupMatrixKind::kResult, s->Kind());
    EXPECT_TRUE(s->Type()->Is<I32>());
    EXPECT_EQ(s->Columns(), 3u);
    EXPECT_EQ(s->Rows(), 4u);
}

}  // namespace
}  // namespace tint::core::type
