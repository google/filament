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

#include "src/tint/lang/core/type/helper_test.h"

#include "src/tint/lang/core/type/array_count.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/u32.h"

namespace tint::core::type {
namespace {

using BindingArrayTest = TestHelper;

TEST_F(BindingArrayTest, Creation) {
    Manager ty;
    auto* t = ty.sampled_texture(TextureDimension::k2d, ty.f32());
    auto* a = ty.binding_array(t, 3u);

    EXPECT_EQ(a->ElemType(), t);
    EXPECT_TRUE(a->Count()->Is<ConstantArrayCount>());
    EXPECT_EQ(a->Count()->As<ConstantArrayCount>()->value, 3u);
}

TEST_F(BindingArrayTest, Hash) {
    Manager ty;
    auto* t = ty.sampled_texture(TextureDimension::k2d, ty.f32());
    auto* a = ty.binding_array(t, 3u);
    auto* a2 = ty.binding_array(t, 3u);

    EXPECT_EQ(a->unique_hash, a2->unique_hash);
}

TEST_F(BindingArrayTest, Equals) {
    Manager ty;
    auto* t1 = ty.sampled_texture(TextureDimension::k2d, ty.f32());
    auto* t2 = ty.sampled_texture(TextureDimension::k2d, ty.u32());

    auto* a = ty.binding_array(t1, 3u);
    auto* a2 = ty.binding_array(t1, 3u);
    auto* a_count = ty.binding_array(t1, 4u);
    auto* a_type = ty.binding_array(t2, 3u);

    EXPECT_EQ(a, a2);
    EXPECT_NE(a, a_count);
    EXPECT_NE(a, a_type);
}

TEST_F(BindingArrayTest, FriendlyName) {
    Manager ty;
    auto* t = ty.sampled_texture(TextureDimension::k2d, ty.f32());
    auto* a = ty.binding_array(t, 3u);
    EXPECT_EQ(a->FriendlyName(), "binding_array<texture_2d<f32>, 3>");
}

TEST_F(BindingArrayTest, Element) {
    Manager ty;
    auto* t = ty.sampled_texture(TextureDimension::k2d, ty.f32());
    auto* a = ty.binding_array(t, 3u);
    EXPECT_EQ(a->Element(2), t);
    EXPECT_EQ(a->Element(3), nullptr);
}

TEST_F(BindingArrayTest, Elements) {
    Manager ty;
    auto* t = ty.sampled_texture(TextureDimension::k2d, ty.f32());
    auto* a = ty.binding_array(t, 3u);
    EXPECT_EQ(a->Elements().type, t);
    EXPECT_EQ(a->Elements().count, 3u);
}

TEST_F(BindingArrayTest, Clone) {
    Manager ty;
    auto* t = ty.sampled_texture(TextureDimension::k2d, ty.f32());
    auto* a = ty.binding_array(t, 3u);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* s = a->Clone(ctx);
    EXPECT_TRUE(s->ElemType()->Is<SampledTexture>());
    EXPECT_TRUE(s->ElemType()->As<SampledTexture>()->Type()->Is<F32>());
    EXPECT_TRUE(s->Count()->Is<ConstantArrayCount>());
    EXPECT_EQ(s->Count()->As<ConstantArrayCount>()->value, 3u);
}

}  // namespace
}  // namespace tint::core::type
