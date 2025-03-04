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

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverF16ExtensionTest = ResolverTest;

TEST_F(ResolverF16ExtensionTest, TypeUsedWithExtension) {
    // enable f16;
    // var<private> v : f16;
    Enable(wgsl::Extension::kF16);

    GlobalVar("v", ty.f16(), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, TypeUsedWithoutExtension) {
    // var<private> v : f16;
    GlobalVar("v", ty.f16(Source{{12, 34}}), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'f16' type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeUsedWithExtension) {
    // enable f16;
    // var<private> v : vec2<f16>;
    Enable(wgsl::Extension::kF16);

    GlobalVar("v", ty.vec2<f16>(), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeUsedWithoutExtension) {
    // var<private> v : vec2<f16>;
    GlobalVar("v", ty.vec2(ty.f16(Source{{12, 34}})), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'f16' type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeInitUsedWithExtension) {
    // enable f16;
    // var<private> v = vec2<f16>();
    Enable(wgsl::Extension::kF16);

    GlobalVar("v", Call<vec2<f16>>(), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeInitUsedWithoutExtension) {
    // var<private> v = vec2<f16>();
    GlobalVar("v", Call(ty.vec2(ty.f16(Source{{12, 34}}))), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'f16' type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeConvUsedWithExtension) {
    // enable f16;
    // var<private> v = vec2<f16>(vec2<f32>());
    Enable(wgsl::Extension::kF16);

    GlobalVar("v", Call<vec2<f16>>(Call<vec2<f32>>()), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, Vec2TypeConvUsedWithoutExtension) {
    // var<private> v = vec2<f16>(vec2<f32>());
    GlobalVar("v", Call(ty.vec2(ty.f16(Source{{12, 34}})), Call<vec2<f32>>()),
              core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'f16' type used without 'f16' extension enabled");
}

TEST_F(ResolverF16ExtensionTest, F16LiteralUsedWithExtension) {
    // enable f16;
    // var<private> v = 16h;
    Enable(wgsl::Extension::kF16);

    GlobalVar("v", Expr(16_h), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverF16ExtensionTest, F16LiteralUsedWithoutExtension) {
    // var<private> v = 16h;
    GlobalVar("v", Expr(Source{{12, 34}}, 16_h), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'f16' type used without 'f16' extension enabled");
}

using ResolverF16ExtensionBuiltinTypeAliasTest = ResolverTestWithParam<const char*>;

TEST_P(ResolverF16ExtensionBuiltinTypeAliasTest, Vec2hTypeUsedWithExtension) {
    // enable f16;
    // var<private> v : vec2h;
    Enable(wgsl::Extension::kF16);

    GlobalVar("v", ty(Source{{12, 34}}, GetParam()), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(ResolverF16ExtensionBuiltinTypeAliasTest, Vec2hTypeUsedWithoutExtension) {
    // var<private> v : vec2h;
    GlobalVar("v", ty(Source{{12, 34}}, GetParam()), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: 'f16' type used without 'f16' extension enabled");
}

INSTANTIATE_TEST_SUITE_P(ResolverF16ExtensionBuiltinTypeAliasTest,
                         ResolverF16ExtensionBuiltinTypeAliasTest,
                         testing::Values("mat2x2h",
                                         "mat2x3h",
                                         "mat2x4h",
                                         "mat3x2h",
                                         "mat3x3h",
                                         "mat3x4h",
                                         "mat4x2h",
                                         "mat4x3h",
                                         "mat4x4h",
                                         "vec2h",
                                         "vec3h",
                                         "vec4h"));

}  // namespace
}  // namespace tint::resolver
