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

#include "src/tint/lang/core/type/depth_texture.h"

#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"

namespace tint::core::type {
namespace {

using DepthTextureTest = TestHelper;

TEST_F(DepthTextureTest, Creation) {
    auto* a = create<DepthTexture>(TextureDimension::k2d);
    auto* b = create<DepthTexture>(TextureDimension::k2d);
    auto* c = create<DepthTexture>(TextureDimension::k2dArray);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST_F(DepthTextureTest, Hash) {
    auto* a = create<DepthTexture>(TextureDimension::k2d);
    auto* b = create<DepthTexture>(TextureDimension::k2d);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(DepthTextureTest, Equals) {
    auto* a = create<DepthTexture>(TextureDimension::k2d);
    auto* b = create<DepthTexture>(TextureDimension::k2d);
    auto* c = create<DepthTexture>(TextureDimension::k2dArray);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(DepthTextureTest, IsTexture) {
    DepthTexture d(TextureDimension::kCube);
    Texture* ty = &d;
    EXPECT_TRUE(ty->Is<DepthTexture>());
    EXPECT_FALSE(ty->Is<ExternalTexture>());
    EXPECT_FALSE(ty->Is<InputAttachment>());
    EXPECT_FALSE(ty->Is<SampledTexture>());
    EXPECT_FALSE(ty->Is<StorageTexture>());
}

TEST_F(DepthTextureTest, Dim) {
    DepthTexture d(TextureDimension::kCube);
    EXPECT_EQ(d.Dim(), TextureDimension::kCube);
}

TEST_F(DepthTextureTest, FriendlyName) {
    DepthTexture d(TextureDimension::kCube);
    EXPECT_EQ(d.FriendlyName(), "texture_depth_cube");
}

TEST_F(DepthTextureTest, Clone) {
    auto* a = create<DepthTexture>(TextureDimension::k2d);

    core::type::Manager mgr;
    core::type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* dt = a->Clone(ctx);
    EXPECT_EQ(dt->Dim(), TextureDimension::k2d);
}

}  // namespace
}  // namespace tint::core::type
