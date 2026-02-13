// Copyright 2023 The Dawn & Tint Authors
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
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::resolver {
namespace {

////////////////////////////////////////////////////////////////////////////////
// access
////////////////////////////////////////////////////////////////////////////////
using ResolverAccessUsedWithTemplateArgs = ResolverTestWithParam<std::string_view>;

TEST_P(ResolverAccessUsedWithTemplateArgs, Test) {
    // @group(0) @binding(0) var t : texture_storage_2d<rgba8unorm, ACCESS<i32>>;
    auto* tmpl = Ident(Source{{12, 34}}, GetParam(), "i32");
    GlobalVar("v", ty("texture_storage_2d", "rgba8unorm", tmpl), Group(0_u), Binding(0_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: access '" + std::string(GetParam()) +
                                "' does not take template arguments");
}

INSTANTIATE_TEST_SUITE_P(,
                         ResolverAccessUsedWithTemplateArgs,
                         testing::ValuesIn(core::kAccessStrings));

////////////////////////////////////////////////////////////////////////////////
// address space
////////////////////////////////////////////////////////////////////////////////
using ResolverAddressSpaceUsedWithTemplateArgs = ResolverTestWithParam<std::string_view>;

TEST_P(ResolverAddressSpaceUsedWithTemplateArgs, Test) {
    // fn f(p : ptr<ADDRESS_SPACE<T>, f32) {}

    auto* tmpl = Ident(Source{{12, 34}}, GetParam(), "T");
    Func("f", Vector{Param("p", ty("ptr", tmpl, ty.f32()))}, ty.void_(), tint::Empty);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: address space '" + std::string(GetParam()) +
                                "' does not take template arguments");
}

INSTANTIATE_TEST_SUITE_P(,
                         ResolverAddressSpaceUsedWithTemplateArgs,
                         testing::ValuesIn(core::kAddressSpaceStrings));

////////////////////////////////////////////////////////////////////////////////
// texel format
////////////////////////////////////////////////////////////////////////////////
using ResolverTexelFormatUsedWithTemplateArgs = ResolverTestWithParam<std::string_view>;

TEST_P(ResolverTexelFormatUsedWithTemplateArgs, Test) {
    // @group(0) @binding(0) var t : texture_storage_2d<TEXEL_FORMAT<T>, write>
    auto* tmpl = Ident(Source{{12, 34}}, GetParam(), "T");
    GlobalVar("t", ty("texture_storage_2d", ty(tmpl), "write"), Group(0_u), Binding(0_u));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: texel format '" + std::string(GetParam()) +
                                "' does not take template arguments");
}

INSTANTIATE_TEST_SUITE_P(,
                         ResolverTexelFormatUsedWithTemplateArgs,
                         testing::ValuesIn(core::kTexelFormatStrings));

}  // namespace
}  // namespace tint::resolver
