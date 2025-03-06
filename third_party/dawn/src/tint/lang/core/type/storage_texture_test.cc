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

#include "src/tint/lang/core/type/storage_texture.h"

#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"

namespace tint::core::type {
namespace {

struct StorageTextureTest : public TestHelper {
    StorageTexture* Create(TextureDimension dims, core::TexelFormat fmt, core::Access access) {
        auto* subtype = StorageTexture::SubtypeFor(fmt, Types());
        return create<StorageTexture>(dims, fmt, access, subtype);
    }
};

TEST_F(StorageTextureTest, Creation) {
    auto* a =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* b =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* c =
        Create(TextureDimension::k2d, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* d =
        Create(TextureDimension::kCube, core::TexelFormat::kR32Float, core::Access::kReadWrite);
    auto* e = Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kRead);

    EXPECT_TRUE(a->Type()->Is<F32>());
    EXPECT_EQ(a->Dim(), TextureDimension::kCube);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(StorageTextureTest, Hash) {
    auto* a =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* b =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(StorageTextureTest, Equals) {
    auto* a =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* b =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* c =
        Create(TextureDimension::k2d, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* d =
        Create(TextureDimension::kCube, core::TexelFormat::kR32Float, core::Access::kReadWrite);
    auto* e = Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(StorageTextureTest, Dim) {
    auto* s = Create(TextureDimension::k2dArray, core::TexelFormat::kRgba32Float,
                     core::Access::kReadWrite);
    EXPECT_EQ(s->Dim(), TextureDimension::k2dArray);
}

TEST_F(StorageTextureTest, Format) {
    auto* s = Create(TextureDimension::k2dArray, core::TexelFormat::kRgba32Float,
                     core::Access::kReadWrite);
    EXPECT_EQ(s->TexelFormat(), core::TexelFormat::kRgba32Float);
}

TEST_F(StorageTextureTest, FriendlyName) {
    auto* s = Create(TextureDimension::k2dArray, core::TexelFormat::kRgba32Float,
                     core::Access::kReadWrite);
    EXPECT_EQ(s->FriendlyName(), "texture_storage_2d_array<rgba32float, read_write>");
}

TEST_F(StorageTextureTest, F32) {
    auto* s = Create(TextureDimension::k2dArray, core::TexelFormat::kRgba32Float,
                     core::Access::kReadWrite);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->Type()->Is<F32>());
}

TEST_F(StorageTextureTest, U32) {
    auto* subtype = StorageTexture::SubtypeFor(core::TexelFormat::kRg32Uint, Types());
    auto* s = create<StorageTexture>(TextureDimension::k2dArray, core::TexelFormat::kRg32Uint,
                                     core::Access::kReadWrite, subtype);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->Type()->Is<U32>());
}

TEST_F(StorageTextureTest, I32) {
    auto* subtype = StorageTexture::SubtypeFor(core::TexelFormat::kRgba32Sint, Types());
    auto* s = create<StorageTexture>(TextureDimension::k2dArray, core::TexelFormat::kRgba32Sint,
                                     core::Access::kReadWrite, subtype);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->Type()->Is<I32>());
}

TEST_F(StorageTextureTest, Clone) {
    auto* a =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mt = a->Clone(ctx);
    EXPECT_EQ(mt->Dim(), TextureDimension::kCube);
    EXPECT_EQ(mt->TexelFormat(), core::TexelFormat::kRgba32Float);
    EXPECT_TRUE(mt->Type()->Is<F32>());
}

}  // namespace
}  // namespace tint::core::type
