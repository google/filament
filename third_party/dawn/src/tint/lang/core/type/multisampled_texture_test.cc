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

#include "src/tint/lang/core/type/multisampled_texture.h"

#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"

namespace tint::core::type {
namespace {

using MultisampledTextureTest = TestHelper;

TEST_F(MultisampledTextureTest, Creation) {
    auto* a = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* b = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* c = create<MultisampledTexture>(TextureDimension::k3d, create<F32>());
    auto* d = create<MultisampledTexture>(TextureDimension::k2d, create<I32>());
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

TEST_F(MultisampledTextureTest, Hash) {
    auto* a = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* b = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(MultisampledTextureTest, Equals) {
    auto* a = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* b = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());
    auto* c = create<MultisampledTexture>(TextureDimension::k3d, create<F32>());
    auto* d = create<MultisampledTexture>(TextureDimension::k2d, create<I32>());
    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(MultisampledTextureTest, IsTexture) {
    F32 f32;
    MultisampledTexture s(TextureDimension::kCube, &f32);
    Texture* ty = &s;
    EXPECT_FALSE(ty->Is<DepthTexture>());
    EXPECT_FALSE(ty->Is<ExternalTexture>());
    EXPECT_FALSE(ty->Is<InputAttachment>());
    EXPECT_TRUE(ty->Is<MultisampledTexture>());
    EXPECT_FALSE(ty->Is<SampledTexture>());
    EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(MultisampledTextureTest, Dim) {
    F32 f32;
    MultisampledTexture s(TextureDimension::k3d, &f32);
    EXPECT_EQ(s.Dim(), TextureDimension::k3d);
}

TEST_F(MultisampledTextureTest, Type) {
    F32 f32;
    MultisampledTexture s(TextureDimension::k3d, &f32);
    EXPECT_EQ(s.Type(), &f32);
}

TEST_F(MultisampledTextureTest, FriendlyName) {
    F32 f32;
    MultisampledTexture s(TextureDimension::k3d, &f32);
    EXPECT_EQ(s.FriendlyName(), "texture_multisampled_3d<f32>");
}

TEST_F(MultisampledTextureTest, Clone) {
    auto* a = create<MultisampledTexture>(TextureDimension::k2d, create<F32>());

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mt = a->Clone(ctx);
    EXPECT_EQ(mt->Dim(), TextureDimension::k2d);
    EXPECT_TRUE(mt->Type()->Is<F32>());
}

}  // namespace
}  // namespace tint::core::type
