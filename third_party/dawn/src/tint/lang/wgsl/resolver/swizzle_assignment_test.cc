// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverSwizzleAssignmentTest = ResolverTest;

TEST_F(ResolverSwizzleAssignmentTest, LanguageFeatureDisabled) {
    // var v : vec4f;
    // v.xy = vec2(1);
    GlobalVar("v", ty.vec4<f32>(), core::AddressSpace::kPrivate);

    WrapInFunction(Assign(MemberAccessor(Source{{1, 2}}, "v", "xy"), Call<vec2<f32>>(1_f)));

    wgsl::AllowedFeatures allowed_features{};

    Resolver resolver{this, allowed_features};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(
        resolver.error(),
        R"(1:2 error: cannot assign to value of type 'swizzle<private, vec2<f32>, read_write, 4, 2>')");
}

TEST_F(ResolverSwizzleAssignmentTest, SimpleSwizzleAssignment) {
    // var v : vec4f;
    // v.xy = vec2(1);
    GlobalVar("v", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Assign(MemberAccessor(Source{{3, 4}}, "v", "xy"), Call<vec2<f32>>(1_f)));

    wgsl::AllowedFeatures allowed_features{};
    allowed_features.features.insert(wgsl::LanguageFeature::kSwizzleAssignment);

    Resolver resolver{this, allowed_features};
    EXPECT_TRUE(resolver.Resolve()) << resolver.error();
}

TEST_F(ResolverSwizzleAssignmentTest, SingleElementChainedSwizzle) {
    // var v : vec3u;
    // v.xyz.y = 1;
    GlobalVar("v", ty.vec3<u32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Assign(MemberAccessor(Source{{1, 2}}, MemberAccessor("v", "xyz"), "y"), 1_u));

    wgsl::AllowedFeatures allowed_features{};
    allowed_features.features.insert(wgsl::LanguageFeature::kSwizzleAssignment);

    Resolver resolver{this, allowed_features};
    EXPECT_TRUE(resolver.Resolve()) << resolver.error();
}

TEST_F(ResolverSwizzleAssignmentTest, MultiElementChainedSwizzle) {
    // var v : vec3i;
    // v.zyx.xy = vec2(1, 2);
    GlobalVar("v", ty.vec3<i32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Assign(MemberAccessor(Source{{1, 2}}, MemberAccessor("v", "zyx"), "xy"),
                          Call<vec2<i32>>(1_i, 2_i)));

    wgsl::AllowedFeatures allowed_features{};
    allowed_features.features.insert(wgsl::LanguageFeature::kSwizzleAssignment);

    Resolver resolver{this, allowed_features};
    EXPECT_TRUE(resolver.Resolve()) << resolver.error();
}

TEST_F(ResolverSwizzleAssignmentTest, InvalidVectorSwizzleAssignmentFails) {
    // var v : vec2f;
    // v.yz = vec2(1, 2);
    GlobalVar("v", ty.vec2<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(
        Assign(MemberAccessor("v", Ident(Source{{1, 2}}, "yz")), Call<vec2<f32>>(1_f, 2_f)));

    wgsl::AllowedFeatures allowed_features{};
    allowed_features.features.insert(wgsl::LanguageFeature::kSwizzleAssignment);

    Resolver resolver{this, allowed_features};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(), R"(1:2 error: invalid vector swizzle member)");
}

TEST_F(ResolverSwizzleAssignmentTest, InvalidDuplicateComponents) {
    // var v : vec4f;
    // v.xx = vec2(1, 2);
    GlobalVar("v", ty.vec2<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Assign(MemberAccessor(Source{{1, 2}}, "v", "xx"), Call<vec2<f32>>(1_f, 2_f)));

    wgsl::AllowedFeatures allowed_features{};
    allowed_features.features.insert(wgsl::LanguageFeature::kSwizzleAssignment);

    Resolver resolver{this, allowed_features};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(r()->error(),
              "1:2 error: cannot assign to vector swizzle with duplicate target components");
}

TEST_F(ResolverSwizzleAssignmentTest, InvalidDuplicateComponentsChainedSwizzle) {
    // var v : vec4f;
    // v.xxy.x = 1.0;
    GlobalVar("v", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    WrapInFunction(Assign(MemberAccessor(Source{{1, 2}}, MemberAccessor("v", "xxy"), "x"), 1_f));

    wgsl::AllowedFeatures allowed_features{};
    allowed_features.features.insert(wgsl::LanguageFeature::kSwizzleAssignment);

    Resolver resolver{this, allowed_features};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(r()->error(),
              "1:2 error: cannot assign to vector swizzle with duplicate target components");
}

}  // namespace
}  // namespace tint::resolver
