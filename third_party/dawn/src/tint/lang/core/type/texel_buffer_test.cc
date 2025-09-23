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

#include "src/tint/lang/core/type/texel_buffer.h"

#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/void.h"

namespace tint::core::type {
namespace {

using TexelBufferTest = TestHelper;

TEST_F(TexelBufferTest, Creation) {
    Manager ty;
    auto* a = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kReadWrite);

    EXPECT_TRUE(a->Type()->Is<F32>());
    EXPECT_EQ(a->TexelFormat(), core::TexelFormat::kRgba32Float);
    EXPECT_EQ(a->Access(), core::Access::kReadWrite);
    EXPECT_EQ(a->Dim(), TextureDimension::k1d);
    EXPECT_EQ(a->FriendlyName(), "texel_buffer<rgba32float, read_write>");

    auto* b = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kRead);
    EXPECT_TRUE(b->Type()->Is<F32>());
    EXPECT_EQ(b->TexelFormat(), core::TexelFormat::kRgba32Float);
    EXPECT_EQ(b->Access(), core::Access::kRead);
    EXPECT_EQ(b->Dim(), TextureDimension::k1d);
    EXPECT_EQ(b->FriendlyName(), "texel_buffer<rgba32float, read>");
}

TEST_F(TexelBufferTest, Hash) {
    Manager ty;
    auto* a = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* b = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kReadWrite);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(TexelBufferTest, Equals) {
    Manager ty;
    auto* a = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* b = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* c = ty.texel_buffer(core::TexelFormat::kR32Float, core::Access::kReadWrite);
    auto* d = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(TexelBufferTest, Type) {
    Manager ty;
    auto* a = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    EXPECT_TRUE(a->Type()->Is<F32>());
}

TEST_F(TexelBufferTest, Clone) {
    Manager ty;
    auto* a = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kReadWrite);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* b = a->Clone(ctx);
    EXPECT_TRUE(b->Type()->Is<F32>());
    EXPECT_EQ(b->TexelFormat(), core::TexelFormat::kRgba32Float);
    EXPECT_EQ(b->Access(), core::Access::kReadWrite);
    EXPECT_EQ(b->Dim(), TextureDimension::k1d);
    EXPECT_EQ(b->FriendlyName(), "texel_buffer<rgba32float, read_write>");
}

TEST_F(TexelBufferTest, FriendlyName) {
    Manager ty;
    auto* a = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    EXPECT_EQ(a->FriendlyName(), "texel_buffer<rgba32float, read_write>");

    auto* b = ty.texel_buffer(core::TexelFormat::kRgba8Unorm, core::Access::kReadWrite);
    EXPECT_EQ(b->FriendlyName(), "texel_buffer<rgba8unorm, read_write>");

    auto* c = ty.texel_buffer(core::TexelFormat::kRgba32Float, core::Access::kRead);
    EXPECT_EQ(c->FriendlyName(), "texel_buffer<rgba32float, read>");

    auto* d = ty.texel_buffer(core::TexelFormat::kR32Float, core::Access::kReadWrite);
    EXPECT_EQ(d->FriendlyName(), "texel_buffer<r32float, read_write>");

    auto* e = ty.texel_buffer(core::TexelFormat::kRgba8Unorm, core::Access::kRead);
    EXPECT_EQ(e->FriendlyName(), "texel_buffer<rgba8unorm, read>");
}

TEST_F(TexelBufferTest, F32) {
    Manager ty;
    auto* a = ty.texel_buffer(core::TexelFormat::kR32Float, core::Access::kReadWrite);

    EXPECT_TRUE(a->Type()->Is<F32>());
    EXPECT_EQ(a->TexelFormat(), core::TexelFormat::kR32Float);
    EXPECT_EQ(a->Access(), core::Access::kReadWrite);
    EXPECT_EQ(a->Dim(), TextureDimension::k1d);
    EXPECT_EQ(a->FriendlyName(), "texel_buffer<r32float, read_write>");
}

TEST_F(TexelBufferTest, U32) {
    Manager ty;
    auto* a = ty.texel_buffer(core::TexelFormat::kR32Uint, core::Access::kReadWrite);

    EXPECT_TRUE(a->Type()->Is<U32>());
    EXPECT_EQ(a->TexelFormat(), core::TexelFormat::kR32Uint);
    EXPECT_EQ(a->Access(), core::Access::kReadWrite);
    EXPECT_EQ(a->Dim(), TextureDimension::k1d);
    EXPECT_EQ(a->FriendlyName(), "texel_buffer<r32uint, read_write>");
}

TEST_F(TexelBufferTest, I32) {
    Manager ty;
    auto* a = ty.texel_buffer(core::TexelFormat::kR32Sint, core::Access::kReadWrite);

    EXPECT_TRUE(a->Type()->Is<I32>());
    EXPECT_EQ(a->TexelFormat(), core::TexelFormat::kR32Sint);
    EXPECT_EQ(a->Access(), core::Access::kReadWrite);
    EXPECT_EQ(a->Dim(), TextureDimension::k1d);
    EXPECT_EQ(a->FriendlyName(), "texel_buffer<r32sint, read_write>");
}

}  // namespace
}  // namespace tint::core::type
