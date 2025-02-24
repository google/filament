// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

namespace tint::resolver {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T, int ID = 0>
using alias = builder::alias<T, ID>;
template <typename T>
using alias1 = builder::alias1<T>;
template <typename T>
using alias2 = builder::alias2<T>;
template <typename T>
using alias3 = builder::alias3<T>;

namespace AttributeTests {
namespace {
enum class AttributeKind {
    kAlign,
    kBinding,
    kBlendSrc,
    kBuiltinPosition,
    kColor,
    kDiagnostic,
    kGroup,
    kId,
    kInputAttachmentIndex,
    kInterpolate,
    kInvariant,
    kLocation,
    kMustUse,
    kOffset,
    kSize,
    kStageCompute,
    kStride,
    kWorkgroupSize,
};
static std::ostream& operator<<(std::ostream& o, AttributeKind k) {
    switch (k) {
        case AttributeKind::kAlign:
            return o << "@align";
        case AttributeKind::kBinding:
            return o << "@binding";
        case AttributeKind::kBlendSrc:
            return o << "@blend_src";
        case AttributeKind::kBuiltinPosition:
            return o << "@builtin(position)";
        case AttributeKind::kColor:
            return o << "@color";
        case AttributeKind::kDiagnostic:
            return o << "@diagnostic";
        case AttributeKind::kGroup:
            return o << "@group";
        case AttributeKind::kId:
            return o << "@id";
        case AttributeKind::kInputAttachmentIndex:
            return o << "@input_attachment_index";
        case AttributeKind::kInterpolate:
            return o << "@interpolate";
        case AttributeKind::kInvariant:
            return o << "@invariant";
        case AttributeKind::kLocation:
            return o << "@location";
        case AttributeKind::kOffset:
            return o << "@offset";
        case AttributeKind::kMustUse:
            return o << "@must_use";
        case AttributeKind::kSize:
            return o << "@size";
        case AttributeKind::kStageCompute:
            return o << "@compute";
        case AttributeKind::kStride:
            return o << "@stride";
        case AttributeKind::kWorkgroupSize:
            return o << "@workgroup_size";
    }
    TINT_UNREACHABLE();
}

static bool IsBindingAttribute(AttributeKind kind) {
    switch (kind) {
        case AttributeKind::kBinding:
        case AttributeKind::kGroup:
            return true;
        default:
            return false;
    }
}

struct TestParams {
    Vector<AttributeKind, 2> attributes;
    std::string error;  // empty string (Pass) is an expected pass
};

static constexpr const char* Pass = "";

static std::vector<TestParams> OnlyDiagnosticValidFor(std::string thing) {
    return {TestParams{
                {AttributeKind::kAlign},
                "1:2 error: '@align' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kBinding},
                "1:2 error: '@binding' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kBlendSrc},
                "1:2 error: '@blend_src' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kBuiltinPosition},
                "1:2 error: '@builtin' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kColor},
                "1:2 error: '@color' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kDiagnostic},
                Pass,
            },
            TestParams{
                {AttributeKind::kGroup},
                "1:2 error: '@group' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kId},
                "1:2 error: '@id' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kInputAttachmentIndex},
                "1:2 error: '@input_attachment_index' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kInterpolate},
                "1:2 error: '@interpolate' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kInvariant},
                "1:2 error: '@invariant' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kLocation},
                "1:2 error: '@location' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kMustUse},
                "1:2 error: '@must_use' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kOffset},
                "1:2 error: '@offset' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kSize},
                "1:2 error: '@size' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kStageCompute},
                "1:2 error: '@compute' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kStride},
                "1:2 error: '@stride' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kWorkgroupSize},
                "1:2 error: '@workgroup_size' is not valid for " + thing,
            },
            TestParams{
                {AttributeKind::kBinding, AttributeKind::kGroup},
                "1:2 error: '@binding' is not valid for " + thing,
            }};
}

static std::ostream& operator<<(std::ostream& o, const TestParams& c) {
    return o << "attributes: " << c.attributes << ", expect pass: " << c.error.empty();
}

const ast::Attribute* CreateAttribute(const Source& source,
                                      ProgramBuilder& builder,
                                      AttributeKind kind) {
    switch (kind) {
        case AttributeKind::kAlign:
            return builder.MemberAlign(source, 4_i);
        case AttributeKind::kBinding:
            return builder.Binding(source, 1_a);
        case AttributeKind::kBuiltinPosition:
            return builder.Builtin(source, core::BuiltinValue::kPosition);
        case AttributeKind::kColor:
            return builder.Color(source, 2_a);
        case AttributeKind::kDiagnostic:
            return builder.DiagnosticAttribute(source, wgsl::DiagnosticSeverity::kInfo, "chromium",
                                               "unreachable_code");
        case AttributeKind::kGroup:
            return builder.Group(source, 1_a);
        case AttributeKind::kId:
            return builder.Id(source, 0_a);
        case AttributeKind::kInputAttachmentIndex:
            return builder.InputAttachmentIndex(source, 2_a);
        case AttributeKind::kBlendSrc:
            return builder.BlendSrc(source, 0_a);
        case AttributeKind::kInterpolate:
            return builder.Interpolate(source, core::InterpolationType::kLinear,
                                       core::InterpolationSampling::kCenter);
        case AttributeKind::kInvariant:
            return builder.Invariant(source);
        case AttributeKind::kLocation:
            return builder.Location(source, 0_a);
        case AttributeKind::kOffset:
            return builder.MemberOffset(source, 4_a);
        case AttributeKind::kMustUse:
            return builder.MustUse(source);
        case AttributeKind::kSize:
            return builder.MemberSize(source, 16_a);
        case AttributeKind::kStageCompute:
            return builder.Stage(source, ast::PipelineStage::kCompute);
        case AttributeKind::kStride:
            return builder.create<ast::StrideAttribute>(source, 4u);
        case AttributeKind::kWorkgroupSize:
            return builder.create<ast::WorkgroupAttribute>(source, builder.Expr(1_i));
    }
    TINT_UNREACHABLE() << kind;
}

struct TestWithParams : ResolverTestWithParam<TestParams> {
    void EnableExtensionIfNecessary(AttributeKind attribute) {
        switch (attribute) {
            case AttributeKind::kColor:
                Enable(wgsl::Extension::kChromiumExperimentalFramebufferFetch);
                break;
            case AttributeKind::kBlendSrc:
                Enable(wgsl::Extension::kDualSourceBlending);
                break;
            case AttributeKind::kInputAttachmentIndex:
                Enable(wgsl::Extension::kChromiumInternalInputAttachments);
                break;
            default:
                break;
        }
    }

    void EnableRequiredExtensions() {
        for (auto attribute : GetParam().attributes) {
            EnableExtensionIfNecessary(attribute);
        }
    }

    Vector<const ast::Attribute*, 2> CreateAttributes(ProgramBuilder& builder,
                                                      VectorRef<AttributeKind> kinds) {
        return Transform<2>(kinds, [&](AttributeKind kind, size_t index) {
            return CreateAttribute(Source{{static_cast<uint32_t>(index) * 2 + 1,
                                           static_cast<uint32_t>(index) * 2 + 2}},
                                   builder, kind);
        });
    }

    Vector<const ast::Attribute*, 2> CreateAttributes() {
        return CreateAttributes(*this, GetParam().attributes);
    }
};

#undef CHECK
#define CHECK()                                      \
    if (GetParam().error.empty()) {                  \
        EXPECT_TRUE(r()->Resolve()) << r()->error(); \
    } else {                                         \
        EXPECT_FALSE(r()->Resolve());                \
        EXPECT_EQ(GetParam().error, r()->error());   \
    }                                                \
    TINT_REQUIRE_SEMICOLON

namespace FunctionTests {
using VoidFunctionAttributeTest = TestWithParams;
TEST_P(VoidFunctionAttributeTest, IsValid) {
    EnableRequiredExtensions();

    Func(Source{{9, 9}}, "main", Empty, ty.void_(), Empty, CreateAttributes());

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    VoidFunctionAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            Pass,
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' can only be applied to functions that return a value)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(9:9 error: a compute shader must include '@workgroup_size' in its attributes)",
        },
        TestParams{
            {AttributeKind::kStageCompute, AttributeKind::kWorkgroupSize},
            Pass,
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is only valid for compute stages)",
        }));

using NonVoidFunctionAttributeTest = TestWithParams;
TEST_P(NonVoidFunctionAttributeTest, IsValid) {
    EnableRequiredExtensions();

    Func(Source{{9, 9}}, "main", Empty, ty.i32(), Vector{Return(1_i)}, CreateAttributes());

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    NonVoidFunctionAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            Pass,
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            Pass,
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(9:9 error: missing entry point IO attribute on return type)",
        },
        TestParams{
            {AttributeKind::kStageCompute, AttributeKind::kWorkgroupSize},
            R"(9:9 error: missing entry point IO attribute on return type)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for functions)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is only valid for compute stages)",
        }));
}  // namespace FunctionTests

namespace FunctionInputAndOutputTests {
using FunctionParameterAttributeTest = TestWithParams;
TEST_P(FunctionParameterAttributeTest, IsValid) {
    EnableRequiredExtensions();

    Func("main",
         Vector{
             Param("a", ty.vec4<f32>(), CreateAttributes()),
         },
         ty.void_(), tint::Empty);

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    FunctionParameterAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin' is not valid for non-entry point function parameters)",
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' is not valid for non-entry point function parameters)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' is not valid for non-entry point function parameters)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' is not valid for non-entry point function parameters)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for function parameters)",
        }));

using FunctionReturnTypeAttributeTest = TestWithParams;
TEST_P(FunctionReturnTypeAttributeTest, IsValid) {
    EnableRequiredExtensions();

    Func("main", tint::Empty, ty.f32(),
         Vector{
             Return(1_f),
         },
         tint::Empty, CreateAttributes());

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    FunctionReturnTypeAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for non-entry point function return types)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for non-entry point function return types)",
        }));
}  // namespace FunctionInputAndOutputTests

namespace EntryPointInputAndOutputTests {
using ComputeShaderParameterAttributeTest = TestWithParams;
TEST_P(ComputeShaderParameterAttributeTest, IsValid) {
    EnableRequiredExtensions();
    Func("main",
         Vector{
             Param("a", ty.vec4<f32>(), CreateAttributes()),
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    ComputeShaderParameterAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin(position)' cannot be used for compute shader input)",
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' can only be used for fragment shader input)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' cannot be used by compute shaders)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' cannot be used by compute shaders)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' cannot be used by compute shaders)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for function parameters)",
        }));

using FragmentShaderParameterAttributeTest = TestWithParams;
TEST_P(FragmentShaderParameterAttributeTest, IsValid) {
    EnableRequiredExtensions();
    auto* p = Param(Source{{9, 9}}, "a", ty.vec4<f32>(), CreateAttributes());
    Func("frag_main", Vector{p}, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    FragmentShaderParameterAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            Pass,
        },
        TestParams{
            {AttributeKind::kColor},
            Pass,
        },
        TestParams{
            {AttributeKind::kColor, AttributeKind::kLocation},
            R"(3:4 error: multiple entry point IO attributes
1:2 note: previously consumed '@color')",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(9:9 error: missing entry point IO attribute on parameter)",
        },
        TestParams{
            {AttributeKind::kInterpolate, AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@interpolate' can only be used with '@location')",
        },
        TestParams{
            {AttributeKind::kInterpolate, AttributeKind::kLocation},
            Pass,
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(9:9 error: missing entry point IO attribute on parameter)",
        },
        TestParams{
            {AttributeKind::kInvariant, AttributeKind::kBuiltinPosition},
            Pass,
        },
        TestParams{
            {AttributeKind::kLocation},
            Pass,
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for function parameters)",
        }));

using VertexShaderParameterAttributeTest = TestWithParams;
TEST_P(VertexShaderParameterAttributeTest, IsValid) {
    EnableRequiredExtensions();

    auto* p = Param(Source{{9, 9}}, "a", ty.vec4<f32>(), CreateAttributes());
    Func("vertex_main", Vector{p}, ty.vec4<f32>(),
         Vector{
             Return(Call<vec4<f32>>()),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    VertexShaderParameterAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin(position)' cannot be used for vertex shader input)",
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' can only be used for fragment shader input)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(9:9 error: missing entry point IO attribute on parameter)",
        },
        TestParams{
            {AttributeKind::kInterpolate, AttributeKind::kLocation},
            Pass,
        },
        TestParams{
            {AttributeKind::kInterpolate, AttributeKind::kBuiltinPosition},
            R"(3:4 error: '@builtin(position)' cannot be used for vertex shader input)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(9:9 error: missing entry point IO attribute on parameter)",
        },
        TestParams{
            {AttributeKind::kInvariant, AttributeKind::kLocation},
            R"(1:2 error: '@invariant' must be applied to a '@builtin(position)')",
        },
        TestParams{
            {AttributeKind::kInvariant, AttributeKind::kBuiltinPosition},
            R"(3:4 error: '@builtin(position)' cannot be used for vertex shader input)",
        },
        TestParams{
            {AttributeKind::kLocation},
            Pass,
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for function parameters)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for function parameters)",
        }));

using ComputeShaderReturnTypeAttributeTest = TestWithParams;
TEST_P(ComputeShaderReturnTypeAttributeTest, IsValid) {
    EnableRequiredExtensions();

    Func("main", tint::Empty, ty.vec4<f32>(),
         Vector{
             Return(Call<vec4<f32>>(1_f)),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         },
         CreateAttributes());

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    ComputeShaderReturnTypeAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin(position)' cannot be used for compute shader output)",
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' cannot be used by compute shaders)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' cannot be used by compute shaders)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' cannot be used by compute shaders)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for entry point return types)",
        }));

using FragmentShaderReturnTypeAttributeTest = TestWithParams;
TEST_P(FragmentShaderReturnTypeAttributeTest, IsValid) {
    EnableRequiredExtensions();

    Func(Source{{9, 9}}, "frag_main", tint::Empty, ty.vec4<f32>(),
         Vector{Return(Call<vec4<f32>>())},
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         CreateAttributes());

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    FragmentShaderReturnTypeAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBlendSrc, AttributeKind::kLocation},
            R"(1:2 error: '@blend_src' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin(position)' cannot be used for fragment shader output)",
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(9:9 error: missing entry point IO attribute on return type)",
        },
        TestParams{
            {AttributeKind::kInterpolate, AttributeKind::kLocation},
            Pass,
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(9:9 error: missing entry point IO attribute on return type)",
        },
        TestParams{
            {AttributeKind::kInvariant, AttributeKind::kLocation},
            R"(1:2 error: '@invariant' must be applied to a '@builtin(position)')",
        },
        TestParams{
            {AttributeKind::kLocation},
            Pass,
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBinding, AttributeKind::kGroup},
            R"(1:2 error: '@binding' is not valid for entry point return types)",
        }));

using VertexShaderReturnTypeAttributeTest = TestWithParams;
TEST_P(VertexShaderReturnTypeAttributeTest, IsValid) {
    EnableRequiredExtensions();
    auto attrs = CreateAttributes();
    // a vertex shader must include the 'position' builtin in its return type
    if (!GetParam().attributes.Any([](auto b) { return b == AttributeKind::kBuiltinPosition; })) {
        attrs.Push(Builtin(Source{{9, 9}}, core::BuiltinValue::kPosition));
    }
    Func("vertex_main", tint::Empty, ty.vec4<f32>(),
         Vector{
             Return(Call<vec4<f32>>()),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         std::move(attrs));

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    VertexShaderReturnTypeAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            Pass,
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' can only be used with '@location')",
        },
        TestParams{
            {AttributeKind::kInvariant},
            Pass,
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(9:9 error: multiple entry point IO attributes
1:2 note: previously consumed '@location')",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kBinding, AttributeKind::kGroup},
            R"(1:2 error: '@binding' is not valid for entry point return types)",
        },
        TestParams{
            {AttributeKind::kLocation, AttributeKind::kLocation},
            R"(3:4 error: duplicate location attribute
1:2 note: first attribute declared here)",
        }));

using EntryPointParameterAttributeTest = TestWithParams;
TEST_F(EntryPointParameterAttributeTest, DuplicateInternalAttribute) {
    auto* s = Param("s", ty.sampler(core::type::SamplerKind::kSampler),
                    Vector{
                        Binding(0_a),
                        Group(0_a),
                        Disable(ast::DisabledValidation::kBindingPointCollision),
                        Disable(ast::DisabledValidation::kEntryPointParameter),
                    });
    Func("f", Vector{s}, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

using EntryPointReturnTypeAttributeTest = ResolverTest;
TEST_F(EntryPointReturnTypeAttributeTest, DuplicateInternalAttribute) {
    Func("f", tint::Empty, ty.i32(), Vector{Return(1_i)},
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Disable(ast::DisabledValidation::kBindingPointCollision),
             Disable(ast::DisabledValidation::kEntryPointParameter),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
}  // namespace EntryPointInputAndOutputTests

namespace StructAndStructMemberTests {
using StructAttributeTest = TestWithParams;
TEST_P(StructAttributeTest, IsValid) {
    EnableRequiredExtensions();

    Structure("S", Vector{Member("a", ty.f32())}, CreateAttributes());

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    StructAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kColor},
            R"(1:2 error: '@color' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for 'struct' declarations)",
        },
        TestParams{
            {AttributeKind::kBinding, AttributeKind::kGroup},
            R"(1:2 error: '@binding' is not valid for 'struct' declarations)",
        }));

using StructMemberAttributeTest = TestWithParams;
TEST_P(StructMemberAttributeTest, IsValid) {
    EnableRequiredExtensions();
    Structure("S", Vector{Member("a", ty.vec4<f32>(), CreateAttributes())});

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    StructMemberAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            Pass,
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' can only be used with '@location(0)')",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            Pass,
        },
        TestParams{
            {AttributeKind::kColor},
            Pass,
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' can only be used with '@location')",
        },
        TestParams{
            {AttributeKind::kInterpolate, AttributeKind::kLocation},
            Pass,
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' must be applied to a position builtin)",
        },
        TestParams{
            {AttributeKind::kInvariant, AttributeKind::kBuiltinPosition},
            Pass,
        },
        TestParams{
            {AttributeKind::kLocation},
            Pass,
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kOffset},
            Pass,
        },
        TestParams{
            {AttributeKind::kSize},
            Pass,
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kBinding, AttributeKind::kGroup},
            R"(1:2 error: '@binding' is not valid for 'struct' members)",
        },
        TestParams{
            {AttributeKind::kAlign, AttributeKind::kAlign},
            R"(3:4 error: duplicate align attribute
1:2 note: first attribute declared here)",
        }));

TEST_F(StructMemberAttributeTest, Align_Attribute_Const) {
    GlobalConst("val", ty.i32(), Expr(1_i));

    Structure("mystruct", Vector{Member("a", ty.f32(), Vector{MemberAlign("val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstNegative) {
    GlobalConst("val", ty.i32(), Expr(-2_i));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: '@align' value must be a positive, power-of-two integer)");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstPowerOfTwo) {
    GlobalConst("val", ty.i32(), Expr(3_i));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: '@align' value must be a positive, power-of-two integer)");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstF32) {
    GlobalConst("val", ty.f32(), Expr(1.23_f));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@align' value must be an 'i32' or 'u32')");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstU32) {
    GlobalConst("val", ty.u32(), Expr(2_u));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstAInt) {
    GlobalConst("val", Expr(2_a));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Align_Attribute_ConstAFloat) {
    GlobalConst("val", Expr(2.0_a));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberAlign(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@align' value must be an 'i32' or 'u32')");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_Var) {
    GlobalVar(Source{{1, 2}}, "val", ty.f32(), core::AddressSpace::kPrivate,
              core::Access::kUndefined, Expr(1.23_f));

    Structure(Source{{6, 4}}, "mystruct",
              Vector{Member(Source{{12, 5}}, "a", ty.f32(),
                            Vector{MemberAlign(Expr(Source{{12, 35}}, "val"))})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:35 error: 'var val' cannot be referenced at module-scope
1:2 note: 'var val' declared here)");
}

TEST_F(StructMemberAttributeTest, Align_Attribute_Override) {
    Override("val", ty.f32(), Expr(1.23_f));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberAlign(Expr(Source{{12, 34}}, "val"))})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @align requires a const-expression, but expression is an override-expression)");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_Const) {
    GlobalConst("val", ty.i32(), Expr(4_i));

    Structure("mystruct", Vector{Member("a", ty.f32(), Vector{MemberSize("val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstNegative) {
    GlobalConst("val", ty.i32(), Expr(-2_i));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@size' value must be a positive integer)");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstF32) {
    GlobalConst("val", ty.f32(), Expr(1.23_f));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@size' value must be an 'i32' or 'u32')");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstU32) {
    GlobalConst("val", ty.u32(), Expr(4_u));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstAInt) {
    GlobalConst("val", Expr(4_a));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StructMemberAttributeTest, Size_Attribute_ConstAFloat) {
    GlobalConst("val", Expr(2.0_a));

    Structure("mystruct",
              Vector{Member("a", ty.f32(), Vector{MemberSize(Source{{12, 34}}, "val")})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@size' value must be an 'i32' or 'u32')");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_Var) {
    GlobalVar(Source{{1, 2}}, "val", ty.f32(), core::AddressSpace::kPrivate,
              core::Access::kUndefined, Expr(1.23_f));

    Structure(Source{{6, 4}}, "mystruct",
              Vector{Member(Source{{12, 5}}, "a", ty.f32(),
                            Vector{MemberSize(Expr(Source{{12, 35}}, "val"))})});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:35 error: 'var val' cannot be referenced at module-scope
1:2 note: 'var val' declared here)");
}

TEST_F(StructMemberAttributeTest, Size_Attribute_Override) {
    Override("val", ty.f32(), Expr(1.23_f));

    Structure("mystruct",
              Vector{
                  Member("a", ty.f32(), Vector{MemberSize(Expr(Source{{12, 34}}, "val"))}),
              });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @size requires a const-expression, but expression is an override-expression)");
}

TEST_F(StructMemberAttributeTest, Size_On_RuntimeSizedArray) {
    Structure("mystruct",
              Vector{
                  Member("a", ty.array<i32>(), Vector{MemberSize(Source{{12, 34}}, 8_a)}),
              });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: '@size' can only be applied to members where the member's type size can be fully determined at shader creation time)");
}

}  // namespace StructAndStructMemberTests

using ArrayAttributeTest = TestWithParams;
TEST_P(ArrayAttributeTest, IsValid) {
    EnableRequiredExtensions();

    auto arr = ty.array(ty.f32(), CreateAttributes());
    Structure("S", Vector{
                       Member("a", arr),
                   });

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    ArrayAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kStride},
            Pass,
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kBinding, AttributeKind::kGroup},
            R"(1:2 error: '@binding' is not valid for 'array' types)",
        },
        TestParams{
            {AttributeKind::kStride, AttributeKind::kStride},
            R"(3:4 error: duplicate stride attribute
1:2 note: first attribute declared here)",
        }));

using VariableAttributeTest = TestWithParams;
TEST_P(VariableAttributeTest, IsValid) {
    EnableRequiredExtensions();

    if (GetParam().attributes.Any(IsBindingAttribute)) {
        GlobalVar(Source{{9, 9}}, "a", ty.sampler(core::type::SamplerKind::kSampler),
                  CreateAttributes());
    } else {
        GlobalVar(Source{{9, 9}}, "a", ty.f32(), core::AddressSpace::kPrivate, CreateAttributes());
    }

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    VariableAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(9:9 error: resource variables require '@group' and '@binding' attributes)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(9:9 error: resource variables require '@group' and '@binding' attributes)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for module-scope 'var')",
        },
        TestParams{
            {AttributeKind::kBinding, AttributeKind::kGroup},
            Pass,
        },
        TestParams{
            {AttributeKind::kBinding, AttributeKind::kGroup, AttributeKind::kBinding},
            R"(5:6 error: duplicate binding attribute
1:2 note: first attribute declared here)",
        }));

TEST_F(VariableAttributeTest, LocalVar) {
    auto* v = Var("a", ty.f32(), Vector{Binding(Source{{12, 34}}, 2_a)});

    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@binding' is not valid for function-scope 'var'");
}

TEST_F(VariableAttributeTest, LocalLet) {
    auto* v = Let("a", Vector{Binding(Source{{12, 34}}, 2_a)}, Expr(1_a));

    WrapInFunction(v);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@binding' is not valid for 'let' declaration");
}

using ConstantAttributeTest = TestWithParams;
TEST_P(ConstantAttributeTest, IsValid) {
    EnableRequiredExtensions();

    GlobalConst("a", ty.f32(), Expr(1.23_f), CreateAttributes());

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    ConstantAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kId},
            R"(1:2 error: '@id' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kInputAttachmentIndex},
            R"(1:2 error: '@input_attachment_index' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for 'const' declaration)",
        },
        TestParams{
            {AttributeKind::kBinding, AttributeKind::kGroup},
            R"(1:2 error: '@binding' is not valid for 'const' declaration)",
        }));

using OverrideAttributeTest = TestWithParams;
TEST_P(OverrideAttributeTest, IsValid) {
    EnableRequiredExtensions();

    Override("a", ty.f32(), Expr(1.23_f), CreateAttributes());

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    OverrideAttributeTest,
    testing::Values(
        TestParams{
            {AttributeKind::kAlign},
            R"(1:2 error: '@align' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kBinding},
            R"(1:2 error: '@binding' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kBlendSrc},
            R"(1:2 error: '@blend_src' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kBuiltinPosition},
            R"(1:2 error: '@builtin' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kDiagnostic},
            R"(1:2 error: '@diagnostic' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kGroup},
            R"(1:2 error: '@group' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kId},
            Pass,
        },
        TestParams{
            {AttributeKind::kInterpolate},
            R"(1:2 error: '@interpolate' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kInvariant},
            R"(1:2 error: '@invariant' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kLocation},
            R"(1:2 error: '@location' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kMustUse},
            R"(1:2 error: '@must_use' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kOffset},
            R"(1:2 error: '@offset' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kSize},
            R"(1:2 error: '@size' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kStageCompute},
            R"(1:2 error: '@compute' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kStride},
            R"(1:2 error: '@stride' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kWorkgroupSize},
            R"(1:2 error: '@workgroup_size' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kBinding, AttributeKind::kGroup},
            R"(1:2 error: '@binding' is not valid for 'override' declaration)",
        },
        TestParams{
            {AttributeKind::kId, AttributeKind::kId},
            R"(3:4 error: duplicate id attribute
1:2 note: first attribute declared here)",
        }));

using SwitchStatementAttributeTest = TestWithParams;
TEST_P(SwitchStatementAttributeTest, IsValid) {
    EnableRequiredExtensions();

    WrapInFunction(Switch(Expr(0_a), Vector{DefaultCase()}, CreateAttributes()));

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         SwitchStatementAttributeTest,
                         testing::ValuesIn(OnlyDiagnosticValidFor("switch statements")));

using SwitchBodyAttributeTest = TestWithParams;
TEST_P(SwitchBodyAttributeTest, IsValid) {
    EnableRequiredExtensions();

    WrapInFunction(Switch(Expr(0_a), Vector{DefaultCase()}, tint::Empty, CreateAttributes()));

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         SwitchBodyAttributeTest,
                         testing::ValuesIn(OnlyDiagnosticValidFor("'switch' body")));

using IfStatementAttributeTest = TestWithParams;
TEST_P(IfStatementAttributeTest, IsValid) {
    EnableRequiredExtensions();

    WrapInFunction(If(Expr(true), Block(), ElseStmt(), CreateAttributes()));

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         IfStatementAttributeTest,
                         testing::ValuesIn(OnlyDiagnosticValidFor("if statements")));

using ForStatementAttributeTest = TestWithParams;
TEST_P(ForStatementAttributeTest, IsValid) {
    EnableRequiredExtensions();

    WrapInFunction(For(nullptr, Expr(false), nullptr, Block(), CreateAttributes()));

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ForStatementAttributeTest,
                         testing::ValuesIn(OnlyDiagnosticValidFor("for statements")));

using LoopStatementAttributeTest = TestWithParams;
TEST_P(LoopStatementAttributeTest, IsValid) {
    EnableRequiredExtensions();

    WrapInFunction(Loop(Block(Return()), Block(), CreateAttributes()));

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         LoopStatementAttributeTest,
                         testing::ValuesIn(OnlyDiagnosticValidFor("loop statements")));

using WhileStatementAttributeTest = TestWithParams;
TEST_P(WhileStatementAttributeTest, IsValid) {
    EnableRequiredExtensions();

    WrapInFunction(While(Expr(false), Block(), CreateAttributes()));

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         WhileStatementAttributeTest,
                         testing::ValuesIn(OnlyDiagnosticValidFor("while statements")));

using BlockStatementTest = TestWithParams;
TEST_P(BlockStatementTest, CompoundStatement) {
    Func("foo", tint::Empty, ty.void_(),
         Vector{
             Block(Vector{Return()}, CreateAttributes()),
         });

    CHECK();
}
TEST_P(BlockStatementTest, FunctionBody) {
    Func("foo", tint::Empty, ty.void_(), Block(Vector{Return()}, CreateAttributes()));

    CHECK();
}
TEST_P(BlockStatementTest, IfStatementBody) {
    Func("foo", tint::Empty, ty.void_(),
         Vector{
             If(Expr(true), Block(Vector{Return()}, CreateAttributes())),
         });

    CHECK();
}
TEST_P(BlockStatementTest, ElseStatementBody) {
    Func("foo", tint::Empty, ty.void_(),
         Vector{
             If(Expr(true), Block(Vector{Return()}),
                Else(Block(Vector{Return()}, CreateAttributes()))),
         });

    CHECK();
}
TEST_P(BlockStatementTest, ForStatementBody) {
    Func("foo", tint::Empty, ty.void_(),
         Vector{
             For(nullptr, Expr(true), nullptr, Block(Vector{Break()}, CreateAttributes())),
         });

    CHECK();
}
TEST_P(BlockStatementTest, LoopStatementBody) {
    Func("foo", tint::Empty, ty.void_(),
         Vector{
             Loop(Block(Vector{Break()}, CreateAttributes())),
         });

    CHECK();
}
TEST_P(BlockStatementTest, WhileStatementBody) {
    Func("foo", tint::Empty, ty.void_(),
         Vector{
             While(Expr(true), Block(Vector{Break()}, CreateAttributes())),
         });

    CHECK();
}
TEST_P(BlockStatementTest, CaseStatementBody) {
    Func("foo", tint::Empty, ty.void_(),
         Vector{
             Switch(1_a, Case(CaseSelector(1_a), Block(Vector{Break()}, CreateAttributes())),
                    DefaultCase(Block({}))),
         });

    CHECK();
}
TEST_P(BlockStatementTest, DefaultStatementBody) {
    Func("foo", tint::Empty, ty.void_(),
         Vector{
             Switch(1_a, Case(CaseSelector(1_a), Block()),
                    DefaultCase(Block(Vector{Break()}, CreateAttributes()))),
         });

    CHECK();
}
INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         BlockStatementTest,
                         testing::ValuesIn(OnlyDiagnosticValidFor("block statements")));

}  // namespace
}  // namespace AttributeTests

namespace ArrayStrideTests {
namespace {

struct Params {
    builder::ast_type_func_ptr create_el_type;
    uint32_t stride;
    bool should_pass;
};

template <typename T>
constexpr Params ParamsFor(uint32_t stride, bool should_pass) {
    return Params{DataType<T>::AST, stride, should_pass};
}

struct TestWithParams : ResolverTestWithParam<Params> {};

using ArrayStrideTest = TestWithParams;
TEST_P(ArrayStrideTest, All) {
    auto& params = GetParam();
    ast::Type el_ty = params.create_el_type(*this);

    StringStream ss;
    ss << "el_ty: " << FriendlyName(el_ty) << ", stride: " << params.stride
       << ", should_pass: " << params.should_pass;
    SCOPED_TRACE(ss.str());

    auto arr = ty.array(el_ty, 4_u,
                        Vector{
                            create<ast::StrideAttribute>(Source{{12, 34}}, params.stride),
                        });

    GlobalVar("myarray", arr, core::AddressSpace::kPrivate);

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: arrays decorated with the stride attribute must have a stride that "
                  "is at least the size of the element type, and be a multiple of the element "
                  "type's alignment value");
    }
}

struct SizeAndAlignment {
    uint32_t size;
    uint32_t align;
};
constexpr SizeAndAlignment default_u32 = {4, 4};
constexpr SizeAndAlignment default_i32 = {4, 4};
constexpr SizeAndAlignment default_f32 = {4, 4};
constexpr SizeAndAlignment default_vec2 = {8, 8};
constexpr SizeAndAlignment default_vec3 = {12, 16};
constexpr SizeAndAlignment default_vec4 = {16, 16};
constexpr SizeAndAlignment default_mat2x2 = {16, 8};
constexpr SizeAndAlignment default_mat3x3 = {48, 16};
constexpr SizeAndAlignment default_mat4x4 = {64, 16};

INSTANTIATE_TEST_SUITE_P(ResolverAttributeValidationTest,
                         ArrayStrideTest,
                         testing::Values(
                             // Succeed because stride >= element size (while being multiple of
                             // element alignment)
                             ParamsFor<u32>(default_u32.size, true),
                             ParamsFor<i32>(default_i32.size, true),
                             ParamsFor<f32>(default_f32.size, true),
                             ParamsFor<vec2<f32>>(default_vec2.size, true),
                             // vec3's default size is not a multiple of its alignment
                             // ParamsFor<vec3<f32>, default_vec3.size, true},
                             ParamsFor<vec4<f32>>(default_vec4.size, true),
                             ParamsFor<mat2x2<f32>>(default_mat2x2.size, true),
                             ParamsFor<mat3x3<f32>>(default_mat3x3.size, true),
                             ParamsFor<mat4x4<f32>>(default_mat4x4.size, true),

                             // Fail because stride is < element size
                             ParamsFor<u32>(default_u32.size - 1, false),
                             ParamsFor<i32>(default_i32.size - 1, false),
                             ParamsFor<f32>(default_f32.size - 1, false),
                             ParamsFor<vec2<f32>>(default_vec2.size - 1, false),
                             ParamsFor<vec3<f32>>(default_vec3.size - 1, false),
                             ParamsFor<vec4<f32>>(default_vec4.size - 1, false),
                             ParamsFor<mat2x2<f32>>(default_mat2x2.size - 1, false),
                             ParamsFor<mat3x3<f32>>(default_mat3x3.size - 1, false),
                             ParamsFor<mat4x4<f32>>(default_mat4x4.size - 1, false),

                             // Succeed because stride equals multiple of element alignment
                             ParamsFor<u32>(default_u32.align * 7, true),
                             ParamsFor<i32>(default_i32.align * 7, true),
                             ParamsFor<f32>(default_f32.align * 7, true),
                             ParamsFor<vec2<f32>>(default_vec2.align * 7, true),
                             ParamsFor<vec3<f32>>(default_vec3.align * 7, true),
                             ParamsFor<vec4<f32>>(default_vec4.align * 7, true),
                             ParamsFor<mat2x2<f32>>(default_mat2x2.align * 7, true),
                             ParamsFor<mat3x3<f32>>(default_mat3x3.align * 7, true),
                             ParamsFor<mat4x4<f32>>(default_mat4x4.align * 7, true),

                             // Fail because stride is not multiple of element alignment
                             ParamsFor<u32>((default_u32.align - 1) * 7, false),
                             ParamsFor<i32>((default_i32.align - 1) * 7, false),
                             ParamsFor<f32>((default_f32.align - 1) * 7, false),
                             ParamsFor<vec2<f32>>((default_vec2.align - 1) * 7, false),
                             ParamsFor<vec3<f32>>((default_vec3.align - 1) * 7, false),
                             ParamsFor<vec4<f32>>((default_vec4.align - 1) * 7, false),
                             ParamsFor<mat2x2<f32>>((default_mat2x2.align - 1) * 7, false),
                             ParamsFor<mat3x3<f32>>((default_mat3x3.align - 1) * 7, false),
                             ParamsFor<mat4x4<f32>>((default_mat4x4.align - 1) * 7, false)));

TEST_F(ArrayStrideTest, DuplicateAttribute) {
    auto arr = ty.array(Source{{12, 34}}, ty.i32(), 4_u,
                        Vector{
                            create<ast::StrideAttribute>(Source{{12, 34}}, 4u),
                            create<ast::StrideAttribute>(Source{{56, 78}}, 4u),
                        });

    GlobalVar("myarray", arr, core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate stride attribute
12:34 note: first attribute declared here)");
}

}  // namespace
}  // namespace ArrayStrideTests

namespace ResourceTests {
namespace {

using ResourceAttributeTest = ResolverTest;
TEST_F(ResourceAttributeTest, UniformBufferMissingBinding) {
    auto* s = Structure("S", Vector{
                                 Member("x", ty.i32()),
                             });
    GlobalVar(Source{{12, 34}}, "G", ty.Of(s), core::AddressSpace::kUniform);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResourceAttributeTest, StorageBufferMissingBinding) {
    auto* s = Structure("S", Vector{
                                 Member("x", ty.i32()),
                             });
    GlobalVar(Source{{12, 34}}, "G", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResourceAttributeTest, TextureMissingBinding) {
    GlobalVar(Source{{12, 34}}, "G", ty.depth_texture(core::type::TextureDimension::k2d));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResourceAttributeTest, SamplerMissingBinding) {
    GlobalVar(Source{{12, 34}}, "G", ty.sampler(core::type::SamplerKind::kSampler));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResourceAttributeTest, BindingPairMissingBinding) {
    GlobalVar(Source{{12, 34}}, "G", ty.sampler(core::type::SamplerKind::kSampler), Group(1_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResourceAttributeTest, BindingPairMissingGroup) {
    GlobalVar(Source{{12, 34}}, "G", ty.sampler(core::type::SamplerKind::kSampler), Binding(1_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResourceAttributeTest, BindingPointUsedTwiceByEntryPoint) {
    GlobalVar(Source{{12, 34}}, "A",
              ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding(1_a),
              Group(2_a));
    GlobalVar(Source{{56, 78}}, "B",
              ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding(1_a),
              Group(2_a));

    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("a", ty.vec4<f32>(),
                      Call("textureLoad", "A", Call<vec2<i32>>(1_i, 2_i), 0_i))),
             Decl(Var("b", ty.vec4<f32>(),
                      Call("textureLoad", "B", Call<vec2<i32>>(1_i, 2_i), 0_i))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(56:78 error: entry point 'F' references multiple variables that use the same resource binding '@group(2)', '@binding(1)'
12:34 note: first resource binding usage declared here)");
}

TEST_F(ResourceAttributeTest, BindingPointUsedTwiceByDifferentEntryPoints) {
    GlobalVar(Source{{12, 34}}, "A",
              ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding(1_a),
              Group(2_a));
    GlobalVar(Source{{56, 78}}, "B",
              ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding(1_a),
              Group(2_a));

    Func("F_A", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("a", ty.vec4<f32>(),
                      Call("textureLoad", "A", Call<vec2<i32>>(1_i, 2_i), 0_i))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    Func("F_B", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("b", ty.vec4<f32>(),
                      Call("textureLoad", "B", Call<vec2<i32>>(1_i, 2_i), 0_i))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResourceAttributeTest, BindingPointOnNonResource) {
    GlobalVar(Source{{12, 34}}, "G", ty.f32(), core::AddressSpace::kPrivate, Binding(1_a),
              Group(2_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: non-resource variables must not have '@group' or '@binding' attributes)");
}

}  // namespace
}  // namespace ResourceTests

namespace WorkgroupAttributeTests {
namespace {

using WorkgroupAttribute = ResolverTest;
TEST_F(WorkgroupAttribute, NotAnEntryPoint) {
    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@workgroup_size' is only valid for compute stages)");
}

TEST_F(WorkgroupAttribute, NotAComputeShader) {
    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@workgroup_size' is only valid for compute stages)");
}

TEST_F(WorkgroupAttribute, DuplicateAttribute) {
    Func(Source{{12, 34}}, "main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(Source{{12, 34}}, 1_i, nullptr, nullptr),
             WorkgroupSize(Source{{56, 78}}, 2_i, nullptr, nullptr),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate workgroup_size attribute
12:34 note: first attribute declared here)");
}

}  // namespace
}  // namespace WorkgroupAttributeTests

namespace InterpolateTests {
namespace {

using InterpolateTest = ResolverTest;

struct Params {
    core::InterpolationType type;
    core::InterpolationSampling sampling;
    bool should_pass;
};

struct TestWithParams : ResolverTestWithParam<Params> {};

using InterpolateParameterTest = TestWithParams;
TEST_P(InterpolateParameterTest, All) {
    auto& params = GetParam();
    Func("main",
         Vector{
             Param("a", ty.f32(),
                   Vector{
                       Location(0_a),
                       Interpolate(Source{{12, 34}}, params.type, params.sampling),
                   }),
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(
            r()->error(),
            params.type == core::InterpolationType::kFlat
                ? R"(12:34 error: flat interpolation can only use 'first' and 'either' sampling parameters)"
                : R"(12:34 error: 'first' and 'either' sampling parameters can only be used with flat interpolation)");
    }
}

TEST_P(InterpolateParameterTest, IntegerScalar) {
    auto& params = GetParam();
    Func("main",
         Vector{
             Param("a", ty.i32(),
                   Vector{
                       Location(0_a),
                       Interpolate(Source{{12, 34}}, params.type, params.sampling),
                   }),
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.type != core::InterpolationType::kFlat) {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(
            r()->error(),
            R"(12:34 error: interpolation type must be 'flat' for integral user-defined IO types)");
    } else if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(
            r()->error(),
            params.type == core::InterpolationType::kFlat
                ? R"(12:34 error: flat interpolation can only use 'first' and 'either' sampling parameters)"
                : R"(12:34 error: 'first' and 'either' sampling parameters can only be used with flat interpolation)");
    }
}

TEST_P(InterpolateParameterTest, IntegerVector) {
    auto& params = GetParam();
    Func("main",
         Vector{
             Param("a", ty.vec4<u32>(),
                   Vector{
                       Location(0_a),
                       Interpolate(Source{{12, 34}}, params.type, params.sampling),
                   }),
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.type != core::InterpolationType::kFlat) {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(
            r()->error(),
            R"(12:34 error: interpolation type must be 'flat' for integral user-defined IO types)");
    } else if (params.should_pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(
            r()->error(),
            params.type == core::InterpolationType::kFlat
                ? R"(12:34 error: flat interpolation can only use 'first' and 'either' sampling parameters)"
                : R"(12:34 error: 'first' and 'either' sampling parameters can only be used with flat interpolation)");
    }
}

INSTANTIATE_TEST_SUITE_P(
    ResolverAttributeValidationTest,
    InterpolateParameterTest,
    testing::Values(
        Params{core::InterpolationType::kPerspective, core::InterpolationSampling::kUndefined,
               true},
        Params{core::InterpolationType::kPerspective, core::InterpolationSampling::kCenter, true},
        Params{core::InterpolationType::kPerspective, core::InterpolationSampling::kCentroid, true},
        Params{core::InterpolationType::kPerspective, core::InterpolationSampling::kSample, true},
        Params{core::InterpolationType::kPerspective, core::InterpolationSampling::kFirst, false},
        Params{core::InterpolationType::kPerspective, core::InterpolationSampling::kEither, false},

        Params{core::InterpolationType::kLinear, core::InterpolationSampling::kUndefined, true},
        Params{core::InterpolationType::kLinear, core::InterpolationSampling::kCenter, true},
        Params{core::InterpolationType::kLinear, core::InterpolationSampling::kCentroid, true},
        Params{core::InterpolationType::kLinear, core::InterpolationSampling::kSample, true},
        Params{core::InterpolationType::kLinear, core::InterpolationSampling::kFirst, false},
        Params{core::InterpolationType::kLinear, core::InterpolationSampling::kEither, false},

        Params{core::InterpolationType::kFlat, core::InterpolationSampling::kUndefined, true},
        Params{core::InterpolationType::kFlat, core::InterpolationSampling::kCenter, false},
        Params{core::InterpolationType::kFlat, core::InterpolationSampling::kCentroid, false},
        Params{core::InterpolationType::kFlat, core::InterpolationSampling::kSample, false},
        Params{core::InterpolationType::kFlat, core::InterpolationSampling::kFirst, true},
        Params{core::InterpolationType::kFlat, core::InterpolationSampling::kEither, true}));

TEST_F(InterpolateTest, FragmentInput_Integer_MissingFlatInterpolation) {
    Func("main", Vector{Param(Source{{12, 34}}, "a", ty.i32(), Vector{Location(0_a)})}, ty.void_(),
         tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: integral user-defined fragment inputs must have a '@interpolate(flat)' attribute)");
}

TEST_F(InterpolateTest, VertexOutput_Integer_MissingFlatInterpolation) {
    auto* s = Structure(
        "S", Vector{
                 Member("pos", ty.vec4<f32>(), Vector{Builtin(core::BuiltinValue::kPosition)}),
                 Member(Source{{12, 34}}, "u", ty.u32(), Vector{Location(0_a)}),
             });
    Func("main", tint::Empty, ty.Of(s),
         Vector{
             Return(Call(ty.Of(s))),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: integral user-defined vertex outputs must have a '@interpolate(flat)' attribute
note: while analyzing entry point 'main')");
}

using GroupAndBindingTest = ResolverTest;

TEST_F(GroupAndBindingTest, Const_I32) {
    GlobalConst("b", Expr(4_i));
    GlobalConst("g", Expr(2_i));
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding("b"),
              Group("g"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(GroupAndBindingTest, Const_U32) {
    GlobalConst("b", Expr(4_u));
    GlobalConst("g", Expr(2_u));
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding("b"),
              Group("g"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(GroupAndBindingTest, Const_AInt) {
    GlobalConst("b", Expr(4_a));
    GlobalConst("g", Expr(2_a));
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding("b"),
              Group("g"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(GroupAndBindingTest, Binding_NonConstant) {
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              Binding(Call<u32>(Call(Source{{12, 34}}, "dpdx", 1_a))), Group(1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @binding requires a const-expression, but expression is a runtime-expression)");
}

TEST_F(GroupAndBindingTest, Binding_Negative) {
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              Binding(Source{{12, 34}}, -2_i), Group(1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@binding' value must be non-negative)");
}

TEST_F(GroupAndBindingTest, Binding_F32) {
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              Binding(Source{{12, 34}}, 2.0_f), Group(1_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@binding' must be an 'i32' or 'u32' value)");
}

TEST_F(GroupAndBindingTest, Binding_AFloat) {
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
              Binding(Source{{12, 34}}, 2.0_a), Group(1_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@binding' must be an 'i32' or 'u32' value)");
}

TEST_F(GroupAndBindingTest, Group_NonConstant) {
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding(2_u),
              Group(Call<u32>(Call(Source{{12, 34}}, "dpdx", 1_a))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @group requires a const-expression, but expression is a runtime-expression)");
}

TEST_F(GroupAndBindingTest, Group_Negative) {
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding(2_u),
              Group(Source{{12, 34}}, -1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@group' value must be non-negative)");
}

TEST_F(GroupAndBindingTest, Group_F32) {
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding(2_u),
              Group(Source{{12, 34}}, 1.0_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@group' must be an 'i32' or 'u32' value)");
}

TEST_F(GroupAndBindingTest, Group_AFloat) {
    GlobalVar("val", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), Binding(2_u),
              Group(Source{{12, 34}}, 1.0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@group' must be an 'i32' or 'u32' value)");
}

using IdTest = ResolverTest;

TEST_F(IdTest, Const_I32) {
    Override("val", ty.f32(), Vector{Id(1_i)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(IdTest, Const_U32) {
    Override("val", ty.f32(), Vector{Id(1_u)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(IdTest, Const_AInt) {
    Override("val", ty.f32(), Vector{Id(1_a)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(IdTest, NonConstant) {
    Override("val", ty.f32(), Vector{Id(Call<u32>(Call(Source{{12, 34}}, "dpdx", 1_a)))});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @id requires a const-expression, but expression is a runtime-expression)");
}

TEST_F(IdTest, Negative) {
    Override("val", ty.f32(), Vector{Id(Source{{12, 34}}, -1_i)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@id' value must be non-negative)");
}

TEST_F(IdTest, F32) {
    Override("val", ty.f32(), Vector{Id(Source{{12, 34}}, 1_f)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@id' must be an 'i32' or 'u32' value)");
}

TEST_F(IdTest, AFloat) {
    Override("val", ty.f32(), Vector{Id(Source{{12, 34}}, 1.0_a)});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@id' must be an 'i32' or 'u32' value)");
}

enum class LocationAttributeType {
    kEntryPointParameter,
    kEntryPointReturnType,
    kStructureMember,
};

struct LocationTest : ResolverTestWithParam<LocationAttributeType> {
    void Build(const ast::Expression* location_value) {
        switch (GetParam()) {
            case LocationAttributeType::kEntryPointParameter:
                Func("main",
                     Vector{Param(Source{{12, 34}}, "a", ty.i32(),
                                  Vector{
                                      Location(Source{{12, 34}}, location_value),
                                      Flat(),
                                  })},
                     ty.void_(), tint::Empty,
                     Vector{
                         Stage(ast::PipelineStage::kFragment),
                     });
                return;
            case LocationAttributeType::kEntryPointReturnType:
                Func("main", tint::Empty, ty.f32(),
                     Vector{
                         Return(1_a),
                     },
                     Vector{
                         Stage(ast::PipelineStage::kFragment),
                     },
                     Vector{
                         Location(Source{{12, 34}}, location_value),
                     });
                return;
            case LocationAttributeType::kStructureMember:
                Structure("S", Vector{
                                   Member("m", ty.f32(),
                                          Vector{
                                              Location(Source{{12, 34}}, location_value),
                                          }),
                               });
                return;
        }
    }
};

TEST_P(LocationTest, Const_I32) {
    Build(Expr(0_i));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(LocationTest, Const_U32) {
    Build(Expr(0_u));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(LocationTest, Const_AInt) {
    Build(Expr(0_a));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(LocationTest, NonConstant) {
    Build(Call<u32>(Call(Source{{12, 34}}, "dpdx", 1_a)));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: @location value requires a const-expression, but expression is a runtime-expression)");
}

TEST_P(LocationTest, Negative) {
    Build(Expr(-1_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@location' value must be non-negative)");
}

TEST_P(LocationTest, F32) {
    Build(Expr(1_f));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@location' must be an 'i32' or 'u32' value)");
}

TEST_P(LocationTest, AFloat) {
    Build(Expr(1.0_a));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@location' must be an 'i32' or 'u32' value)");
}

INSTANTIATE_TEST_SUITE_P(LocationTest,
                         LocationTest,
                         testing::Values(LocationAttributeType::kEntryPointParameter,
                                         LocationAttributeType::kEntryPointReturnType,
                                         LocationAttributeType::kStructureMember));

}  // namespace
}  // namespace InterpolateTests

namespace InternalAttributeDeps {
namespace {

class TestAttribute : public Castable<TestAttribute, ast::InternalAttribute> {
  public:
    TestAttribute(GenerationID pid, ast::NodeID nid, const ast::IdentifierExpression* dep)
        : Base(pid, nid, Vector{dep}) {}
    std::string InternalName() const override { return "test_attribute"; }
    const Node* Clone(ast::CloneContext&) const override { return nullptr; }
};

using InternalAttributeDepsTest = ResolverTest;
TEST_F(InternalAttributeDepsTest, Dependency) {
    auto* ident = Expr("v");
    auto* attr = ASTNodes().Create<TestAttribute>(ID(), AllocateNodeID(), ident);
    auto* f = Func("f", tint::Empty, ty.void_(), tint::Empty, Vector{attr});
    auto* v = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* user = As<sem::VariableUser>(Sem().Get(ident));
    ASSERT_NE(user, nullptr);

    auto* var = Sem().Get(v);
    EXPECT_EQ(user->Variable(), var);

    auto* fn = Sem().Get(f);
    EXPECT_THAT(fn->DirectlyReferencedGlobals(), testing::ElementsAre(var));
    EXPECT_THAT(fn->TransitivelyReferencedGlobals(), testing::ElementsAre(var));
}

}  // namespace
}  // namespace InternalAttributeDeps

namespace RowMajorAttributeTests {

using RowMajorAttributeTest = ResolverTest;

TEST_F(RowMajorAttributeTest, StructMember_Matrix) {
    Structure("S", Vector{
                       Member(Source{{12, 34}}, "m", ty.mat3x4<f32>(), Vector{RowMajor()}),
                   });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(RowMajorAttributeTest, StructMember_ArrayOfMatrix) {
    Structure("S",
              Vector{
                  Member(Source{{12, 34}}, "arr", ty.array<mat3x4<f32>, 4>(), Vector{RowMajor()}),
              });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(RowMajorAttributeTest, StructMember_NonMatrix) {
    Structure("S", Vector{
                       Member(Source{{12, 34}}, "f", ty.vec4<f32>(), Vector{RowMajor()}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: '@row_major' can only be applied to matrices or arrays of matrices)");
}

TEST_F(RowMajorAttributeTest, StructMember_ArrayOfNonMatrix) {
    Structure("S",
              Vector{
                  Member(Source{{12, 34}}, "arr", ty.array<vec4<f32>, 4>(), Vector{RowMajor()}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: '@row_major' can only be applied to matrices or arrays of matrices)");
}

TEST_F(RowMajorAttributeTest, Variable) {
    GlobalVar(Source{{12, 34}}, "v", ty.mat3x4<f32>(), Vector{RowMajor()});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: '@row_major' is not valid for module-scope 'var')");
}

}  // namespace RowMajorAttributeTests

}  // namespace tint::resolver

TINT_INSTANTIATE_TYPEINFO(tint::resolver::InternalAttributeDeps::TestAttribute);
