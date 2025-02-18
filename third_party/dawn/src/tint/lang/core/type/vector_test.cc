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

#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/texture.h"

namespace tint::core::type {
namespace {

using VectorTest = TestHelper;

TEST_F(VectorTest, Creation) {
    auto* a = create<Vector>(create<I32>(), 2u);
    auto* b = create<Vector>(create<I32>(), 2u);
    auto* c = create<Vector>(create<F32>(), 2u);
    auto* d = create<Vector>(create<F32>(), 3u);

    EXPECT_EQ(a->Type(), create<I32>());
    EXPECT_EQ(a->Width(), 2u);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

TEST_F(VectorTest, Creation_Packed) {
    auto* v = create<Vector>(create<F32>(), 3u);
    auto* p1 = create<Vector>(create<F32>(), 3u, true);
    auto* p2 = create<Vector>(create<F32>(), 3u, true);

    EXPECT_FALSE(v->Packed());

    EXPECT_EQ(p1->Type(), create<F32>());
    EXPECT_EQ(p1->Width(), 3u);
    EXPECT_TRUE(p1->Packed());

    EXPECT_NE(v, p1);
    EXPECT_EQ(p1, p2);
}

TEST_F(VectorTest, Hash) {
    auto* a = create<Vector>(create<I32>(), 2u);
    auto* b = create<Vector>(create<I32>(), 2u);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(VectorTest, Equals) {
    auto* a = create<Vector>(create<I32>(), 2u);
    auto* b = create<Vector>(create<I32>(), 2u);
    auto* c = create<Vector>(create<F32>(), 2u);
    auto* d = create<Vector>(create<F32>(), 3u);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(VectorTest, FriendlyName) {
    auto* f32 = create<F32>();
    auto* v = create<Vector>(f32, 3u);
    EXPECT_EQ(v->FriendlyName(), "vec3<f32>");
}

TEST_F(VectorTest, FriendlyName_Packed) {
    auto* f32 = create<F32>();
    auto* v = create<Vector>(f32, 3u, true);
    EXPECT_EQ(v->FriendlyName(), "__packed_vec3<f32>");
}

TEST_F(VectorTest, Clone) {
    auto* a = create<Vector>(create<I32>(), 2u);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* vec = a->Clone(ctx);
    EXPECT_TRUE(vec->Type()->Is<I32>());
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_FALSE(vec->Packed());
}

TEST_F(VectorTest, Clone_Packed) {
    auto* a = create<Vector>(create<I32>(), 3u, true);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* vec = a->Clone(ctx);
    EXPECT_TRUE(vec->Type()->Is<I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TRUE(vec->Packed());
}

}  // namespace
}  // namespace tint::core::type
