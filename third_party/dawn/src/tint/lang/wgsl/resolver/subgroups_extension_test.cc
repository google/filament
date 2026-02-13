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

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverSubgroupsExtensionTest = ResolverTest;

// Using a subgroup_size builtin attribute without subgroups enabled should fail.
TEST_F(ResolverSubgroupsExtensionTest, UseSubgroupSizeAttribWithoutExtensionError) {
    Structure("Inputs",
              Vector{
                  Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupSize)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of '@builtin(subgroup_size)' attribute requires enabling extension 'subgroups')");
}

// Using a subgroup_invocation_id builtin attribute without subgroups enabled should fail.
TEST_F(ResolverSubgroupsExtensionTest, UseSubgroupInvocationIdAttribWithoutExtensionError) {
    Structure("Inputs",
              Vector{
                  Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupInvocationId)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of '@builtin(subgroup_invocation_id)' attribute requires enabling extension 'subgroups')");
}

// Using a subgroup_size builtin attribute with subgroups enabled should pass.
TEST_F(ResolverSubgroupsExtensionTest, UseSubgroupSizeAttribWithExtension) {
    Enable(wgsl::Extension::kSubgroups);
    Structure("Inputs",
              Vector{
                  Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupSize)}),
              });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// Using a subgroup_invocation_id builtin attribute with subgroups enabled should pass.
TEST_F(ResolverSubgroupsExtensionTest, UseSubgroupInvocationIdAttribWithExtension) {
    Enable(wgsl::Extension::kSubgroups);
    Structure("Inputs",
              Vector{
                  Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupInvocationId)}),
              });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// Using an i32 for a subgroup_size builtin input should fail.
TEST_F(ResolverSubgroupsExtensionTest, SubgroupSizeI32Error) {
    Enable(wgsl::Extension::kSubgroups);
    Structure("Inputs",
              Vector{
                  Member("a", ty.i32(), Vector{Builtin(core::BuiltinValue::kSubgroupSize)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: store type of '@builtin(subgroup_size)' must be 'u32'");
}

// Using an i32 for a subgroup_invocation_id builtin input should fail.
TEST_F(ResolverSubgroupsExtensionTest, SubgroupInvocationIdI32Error) {
    Enable(wgsl::Extension::kSubgroups);
    Structure("Inputs",
              Vector{
                  Member("a", ty.i32(), Vector{Builtin(core::BuiltinValue::kSubgroupInvocationId)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: store type of '@builtin(subgroup_invocation_id)' must be 'u32'");
}

// Using builtin(subgroup_size) for anything other than a compute or fragment shader input should
// fail.
TEST_F(ResolverSubgroupsExtensionTest, SubgroupSizeVertexShader) {
    Enable(wgsl::Extension::kSubgroups);
    Func("main",
         Vector{Param("size", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupSize)})},
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kVertex)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "error: '@builtin(subgroup_size)' is only valid as a compute or fragment shader input");
}

TEST_F(ResolverSubgroupsExtensionTest, SubgroupSizeComputeShaderOutput) {
    Enable(wgsl::Extension::kSubgroups);

    Func("main", tint::Empty, ty.u32(),
         Vector{
             Return(Call<u32>()),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         },
         Vector{Builtin(Source{{1, 2}}, core::BuiltinValue::kSubgroupSize)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "1:2 error: '@builtin(subgroup_size)' is only valid as a compute or fragment shader input");
}

TEST_F(ResolverSubgroupsExtensionTest, SubgroupSizeComputeShaderStructOutput) {
    Enable(wgsl::Extension::kSubgroups);

    auto* s = Structure(
        "Output", Vector{
                      Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupSize)}),
                  });

    Func("main", tint::Empty, ty.Of(s),
         Vector{
             Return(Call(ty.Of(s), Call<u32>())),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: '@builtin(subgroup_size)' is only valid as a compute or fragment shader input
note: while analyzing entry point 'main')");
}

// Using builtin(subgroup_invocation_id) for anything other than a compute or fragment shader input
// should fail.
TEST_F(ResolverSubgroupsExtensionTest, SubgroupInvocationIdVertexShader) {
    Enable(wgsl::Extension::kSubgroups);
    Func("main",
         Vector{Param("id", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupInvocationId)})},
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kVertex)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: '@builtin(subgroup_invocation_id)' is only valid as a compute or fragment "
              "shader input");
}

TEST_F(ResolverSubgroupsExtensionTest, SubgroupInvocationIdComputeShaderOutput) {
    Enable(wgsl::Extension::kSubgroups);

    Func("main", tint::Empty, ty.u32(),
         Vector{
             Return(Call<u32>()),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         },
         Vector{Builtin(Source{{1, 2}}, core::BuiltinValue::kSubgroupInvocationId)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:2 error: '@builtin(subgroup_invocation_id)' is only valid as a compute or "
              "fragment shader input");
}

TEST_F(ResolverSubgroupsExtensionTest, UseSubgroupIdAttribWithoutExtensionError) {
    Structure("Inputs", Vector{
                            Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupId)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of '@builtin(subgroup_id)' attribute requires enabling extension 'chromium_experimental_subgroup_matrix')");
}

TEST_F(ResolverSubgroupsExtensionTest, UseSubgroupIdAttribWithExtension) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Structure("Inputs", Vector{
                            Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupId)}),
                        });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupsExtensionTest, SubgroupIdI32Error) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Structure("Inputs", Vector{
                            Member("a", ty.i32(), Vector{Builtin(core::BuiltinValue::kSubgroupId)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: store type of '@builtin(subgroup_id)' must be 'u32'");
}

TEST_F(ResolverSubgroupsExtensionTest, SubgroupIdFragmentShader) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("main", Vector{Param("size", ty.u32(), Vector{Builtin(core::BuiltinValue::kSubgroupId)})},
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: '@builtin(subgroup_id)' is only valid as a compute shader input");
}

TEST_F(ResolverSubgroupsExtensionTest, SubgroupIdComputeShaderOutput) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    Func("main", tint::Empty, ty.u32(),
         Vector{
             Return(Call<u32>()),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         },
         Vector{Builtin(Source{{1, 2}}, core::BuiltinValue::kSubgroupId)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:2 error: '@builtin(subgroup_id)' is only valid as a compute shader input");
}

// Using the subgroup_uniformity diagnostic rule without subgroups enabled should succeed.
TEST_F(ResolverSubgroupsExtensionTest, UseSubgroupUniformityRuleWithoutExtensionError) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kOff, "subgroup_uniformity");
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver
