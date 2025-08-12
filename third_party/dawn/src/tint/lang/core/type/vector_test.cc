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

#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/void.h"

namespace tint::core::type {
namespace {

using VectorTest = TestHelper;

TEST_F(VectorTest, Creation) {
    Manager ty;
    auto* a = ty.vec2(ty.i32());
    auto* b = ty.vec2(ty.i32());
    auto* c = ty.vec2(ty.f32());
    auto* d = ty.vec3(ty.f32());

    EXPECT_EQ(a->Type(), ty.i32());
    EXPECT_EQ(a->Width(), 2u);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

TEST_F(VectorTest, Creation_Packed) {
    Manager ty;
    auto* v = ty.vec3(ty.f32());
    auto* p1 = ty.Get<Vector>(ty.f32(), 3u, true);
    auto* p2 = ty.Get<Vector>(ty.f32(), 3u, true);

    EXPECT_FALSE(v->Packed());

    EXPECT_EQ(p1->Type(), ty.f32());
    EXPECT_EQ(p1->Width(), 3u);
    EXPECT_TRUE(p1->Packed());

    EXPECT_NE(v, p1);
    EXPECT_EQ(p1, p2);
}

TEST_F(VectorTest, Hash) {
    Manager ty;
    auto* a = ty.vec2(ty.i32());
    auto* b = ty.vec2(ty.i32());

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(VectorTest, Equals) {
    Manager ty;
    auto* a = ty.vec2(ty.i32());
    auto* b = ty.vec2(ty.i32());
    auto* c = ty.vec2(ty.f32());
    auto* d = ty.vec3(ty.f32());

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(VectorTest, FriendlyName) {
    Manager ty;
    auto* f32 = ty.f32();
    auto* v = ty.vec3(f32);
    EXPECT_EQ(v->FriendlyName(), "vec3<f32>");
}

TEST_F(VectorTest, FriendlyName_Packed) {
    Manager ty;
    auto* f32 = ty.f32();
    auto* v = ty.Get<Vector>(f32, 3u, true);
    EXPECT_EQ(v->FriendlyName(), "__packed_vec3<f32>");
}

TEST_F(VectorTest, Clone) {
    Manager ty;
    auto* a = ty.vec2(ty.i32());

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* vec = a->Clone(ctx);
    EXPECT_TRUE(vec->Type()->Is<I32>());
    EXPECT_EQ(vec->Width(), 2u);
    EXPECT_FALSE(vec->Packed());
}

TEST_F(VectorTest, Clone_Packed) {
    Manager ty;
    auto* a = ty.Get<Vector>(ty.i32(), 3u, true);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* vec = a->Clone(ctx);
    EXPECT_TRUE(vec->Type()->Is<I32>());
    EXPECT_EQ(vec->Width(), 3u);
    EXPECT_TRUE(vec->Packed());
}

}  // namespace
}  // namespace tint::core::type
