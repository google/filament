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

#include "src/tint/lang/core/type/sampled_texture.h"

#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/void.h"

namespace tint::core::type {
namespace {

using SampledTextureTest = TestHelper;

TEST_F(SampledTextureTest, Creation) {
    Manager ty;
    auto* a = ty.sampled_texture(TextureDimension::kCube, ty.f32());
    auto* b = ty.sampled_texture(TextureDimension::kCube, ty.f32());
    auto* c = ty.sampled_texture(TextureDimension::k2d, ty.f32());
    auto* d = ty.sampled_texture(TextureDimension::kCube, ty.i32());

    EXPECT_TRUE(a->Type()->Is<F32>());
    EXPECT_EQ(a->Dim(), TextureDimension::kCube);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
}

TEST_F(SampledTextureTest, Hash) {
    Manager ty;
    auto* a = ty.sampled_texture(TextureDimension::kCube, ty.f32());
    auto* b = ty.sampled_texture(TextureDimension::kCube, ty.f32());

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(SampledTextureTest, Equals) {
    Manager ty;
    auto* a = ty.sampled_texture(TextureDimension::kCube, ty.f32());
    auto* b = ty.sampled_texture(TextureDimension::kCube, ty.f32());
    auto* c = ty.sampled_texture(TextureDimension::k2d, ty.f32());
    auto* d = ty.sampled_texture(TextureDimension::kCube, ty.i32());

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(SampledTextureTest, IsTexture) {
    F32 f32;
    SampledTexture s(TextureDimension::kCube, &f32);
    Texture* ty = &s;
    EXPECT_FALSE(ty->Is<DepthTexture>());
    EXPECT_FALSE(ty->Is<ExternalTexture>());
    EXPECT_FALSE(ty->Is<InputAttachment>());
    EXPECT_TRUE(ty->Is<SampledTexture>());
    EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(SampledTextureTest, Dim) {
    F32 f32;
    SampledTexture s(TextureDimension::k3d, &f32);
    EXPECT_EQ(s.Dim(), TextureDimension::k3d);
}

TEST_F(SampledTextureTest, Type) {
    F32 f32;
    SampledTexture s(TextureDimension::k3d, &f32);
    EXPECT_EQ(s.Type(), &f32);
}

TEST_F(SampledTextureTest, FriendlyName) {
    F32 f32;
    SampledTexture s(TextureDimension::k3d, &f32);
    EXPECT_EQ(s.FriendlyName(), "texture_3d<f32>");
}

TEST_F(SampledTextureTest, Clone) {
    Manager ty;
    auto* a = ty.sampled_texture(TextureDimension::kCube, ty.f32());

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mt = a->Clone(ctx);
    EXPECT_EQ(mt->Dim(), TextureDimension::kCube);
    EXPECT_TRUE(mt->Type()->Is<F32>());
}

}  // namespace
}  // namespace tint::core::type
