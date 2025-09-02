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

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverTexelBufferTest = ResolverTest;

struct FormatParams {
    core::TexelFormat format;
    bool is_valid;
};

#define VALID_FORMAT_CASE(FMT)       \
    FormatParams {                   \
        core::TexelFormat::FMT, true \
    }

static constexpr FormatParams kFormatCases[] = {
    VALID_FORMAT_CASE(kBgra8Unorm), VALID_FORMAT_CASE(kR32Float),   VALID_FORMAT_CASE(kR32Sint),
    VALID_FORMAT_CASE(kR32Uint),    VALID_FORMAT_CASE(kR8Unorm),    VALID_FORMAT_CASE(kRg32Float),
    VALID_FORMAT_CASE(kRg32Sint),   VALID_FORMAT_CASE(kRg32Uint),   VALID_FORMAT_CASE(kRgba16Float),
    VALID_FORMAT_CASE(kRgba16Sint), VALID_FORMAT_CASE(kRgba16Uint), VALID_FORMAT_CASE(kRgba32Float),
    VALID_FORMAT_CASE(kRgba32Sint), VALID_FORMAT_CASE(kRgba32Uint), VALID_FORMAT_CASE(kRgba8Sint),
    VALID_FORMAT_CASE(kRgba8Snorm), VALID_FORMAT_CASE(kRgba8Uint),  VALID_FORMAT_CASE(kRgba8Unorm),
};
#undef VALID_FORMAT_CASE

using TexelBufferFormatTest = ResolverTestWithParam<FormatParams>;

// Tests that the texel_buffer format is valid or invalid based on the
// format parameter.
TEST_P(TexelBufferFormatTest, Usage) {
    auto& params = GetParam();
    Require(wgsl::LanguageFeature::kTexelBuffers);

    auto tb = ty(Source{{12, 34}}, "texel_buffer", tint::ToString(params.format),
                 tint::ToString(core::Access::kRead));
    GlobalVar("a", tb, Group(0_a), Binding(0_a));

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: image format must be one of the texel formats "
                  "specified for texel buffers in https://gpuweb.github.io/"
                  "gpuweb/wgsl/#texel-formats");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverTexelBufferTest,
                         TexelBufferFormatTest,
                         testing::ValuesIn(kFormatCases));

// Requires 2 template arguments: format and access
TEST_F(ResolverTexelBufferTest, MissingAccess) {
    Require(wgsl::LanguageFeature::kTexelBuffers);
    auto tb = ty(Source{{12, 34}}, "texel_buffer", "rgba32float");
    GlobalVar("a", tb, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: 'texel_buffer' requires 2 template arguments)");
}

// Access must be read or read_write
TEST_F(ResolverTexelBufferTest, InvalidAccess) {
    Require(wgsl::LanguageFeature::kTexelBuffers);
    auto tb =
        ty(Source{{12, 34}}, "texel_buffer", "rgba32float", Expr(Source{{12, 34}}, "read_only"));
    GlobalVar("a", tb, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: unresolved access 'read_only'
12:34 note: Possible values: 'read', 'read_write', 'write')");
}

// Access must be read or read_write, not write
TEST_F(ResolverTexelBufferTest, WriteOnlyAccess) {
    Require(wgsl::LanguageFeature::kTexelBuffers);
    auto tb = ty(Source{{12, 34}}, "texel_buffer", "rgba32float", "write");
    GlobalVar("a", tb, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: texel_buffer access must be read or read_write)");
}

// Requires the texel_buffer language feature to be enabled
TEST_F(ResolverTexelBufferTest, FeatureRequired) {
    auto tb = ty(Source{{12, 34}}, "texel_buffer", "rgba32float", "read");
    GlobalVar("a", tb, Group(0_a), Binding(0_a));

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: use of 'texel_buffer' requires the texel_buffer language "
              "feature, which is not allowed in the current environment");
}

enum class SubtypeKind {
    kF16,
    kI8,
    kU8,
};

struct SubtypeParams {
    SubtypeKind kind;
};

static constexpr SubtypeParams kInvalidSubtypeCases[] = {
    {SubtypeKind::kF16},
    {SubtypeKind::kI8},
    {SubtypeKind::kU8},
};

using TexelBufferSubtypeTest = ResolverTestWithParam<SubtypeParams>;

// Storage subtype must be f32, i32 or u32
TEST_P(TexelBufferSubtypeTest, InvalidSubtype) {
    auto& params = GetParam();
    Require(wgsl::LanguageFeature::kTexelBuffers);

    const core::type::Type* subtype = nullptr;
    switch (params.kind) {
        case SubtypeKind::kF16:
            subtype = create<core::type::F16>();
            break;
        case SubtypeKind::kI8:
            subtype = create<core::type::I8>();
            break;
        case SubtypeKind::kU8:
            subtype = create<core::type::U8>();
            break;
    }

    auto* tb_ty = create<core::type::TexelBuffer>(core::TexelFormat::kRgba32Float,
                                                  core::Access::kRead, subtype);

    EXPECT_FALSE(v()->TexelBuffer(tb_ty, Source{{12, 34}}));
    EXPECT_EQ(r()->error(),
              "12:34 error: texel_buffer<format, access>: storage subtype must "
              "be f32, i32 or u32");
}

INSTANTIATE_TEST_SUITE_P(ResolverTexelBufferTest,
                         TexelBufferSubtypeTest,
                         testing::ValuesIn(kInvalidSubtypeCases));

// texel_buffer used as struct member
TEST_F(ResolverTexelBufferTest, StructMember) {
    Require(wgsl::LanguageFeature::kTexelBuffers);

    auto* s = Structure(
        "S", Vector{
                 Member("buf", ty(Source{{12, 34}}, "texel_buffer", "rgba32float", "read")),
             });

    GlobalVar("g", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead, Binding(0_a),
              Group(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: texel_buffer<rgba32float, read> cannot be used as the "
              "type of a structure member");
}

// texel_buffer used as function parameter
TEST_F(ResolverTexelBufferTest, FunctionParameter) {
    Require(wgsl::LanguageFeature::kTexelBuffers);

    Func("f", Vector{Param("p", ty(Source{{12, 34}}, "texel_buffer", "rgba32float", "read_write"))},
         ty.void_(), Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// Global variable missing binding or group attributes
TEST_F(ResolverTexelBufferTest, MissingBinding) {
    Require(wgsl::LanguageFeature::kTexelBuffers);
    GlobalVar(Source{{12, 34}}, "G", ty("texel_buffer", "rgba32float", "read"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: resource variables require '@group' and "
              "'@binding' attributes");
}

}  // namespace
}  // namespace tint::resolver
