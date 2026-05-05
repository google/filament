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

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace {

using SubgroupSizeControlExtensionTest = ResolverTest;

TEST_F(SubgroupSizeControlExtensionTest,
       UseSubgroupSizeControlExtensionWithSubgroupsExtensionError) {
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(SubgroupSizeControlExtensionTest,
       UseSubgroupSizeControlExtensionWithoutSubgroupsExtensionError) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: extension 'chromium_experimental_subgroup_size_control' cannot be used without extension 'subgroups')");
}

TEST_F(SubgroupSizeControlExtensionTest, RequiresExtension) {
    // Test without enabling the extension
    Enable(wgsl::Extension::kSubgroups);
    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
             SubgroupSize(Source{{12, 34}}, Expr(16_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@subgroup_size' requires enabling extension 'chromium_experimental_subgroup_size_control')");
}

TEST_F(SubgroupSizeControlExtensionTest, ValidWithExtension) {
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);

    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
             SubgroupSize(Expr(16_i)),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(SubgroupSizeControlExtensionTest, NotAnEntryPoint) {
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);

    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             SubgroupSize(Source{{12, 34}}, Expr(16_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@subgroup_size' is only valid for compute stages)");
}

TEST_F(SubgroupSizeControlExtensionTest, NotAComputeShader) {
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);

    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
             SubgroupSize(Source{{12, 34}}, Expr(16_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@subgroup_size' is only valid for compute stages)");
}

TEST_F(SubgroupSizeControlExtensionTest, InvalidValue_Negative) {
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);

    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
             SubgroupSize(Source{{12, 34}}, Expr(-1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@subgroup_size' argument must be at least 1)");
}

TEST_F(SubgroupSizeControlExtensionTest, InvalidValue_Zero) {
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);

    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
             SubgroupSize(Source{{12, 34}}, Expr(0_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@subgroup_size' argument must be at least 1)");
}

TEST_F(SubgroupSizeControlExtensionTest, InvalidValue_NotAPowerOfTwo) {
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);

    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
             SubgroupSize(Source{{12, 34}}, Expr(15_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@subgroup_size' argument must be a power of 2)");
}

TEST_F(SubgroupSizeControlExtensionTest, InvalidValue_Float) {
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);

    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
             SubgroupSize(Source{{12, 34}}, Expr(16.5_f)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: '@subgroup_size' argument must be a constant or override-expression of type 'abstract-integer', 'i32' or 'u32')");
}

TEST_F(SubgroupSizeControlExtensionTest, DuplicateAttribute) {
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupSizeControl);

    Func("main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
             SubgroupSize(Source{{12, 34}}, Expr(16_i)),
             SubgroupSize(Source{{56, 78}}, Expr(32_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(56:78 error: duplicate subgroup_size attribute
12:34 note: first attribute declared here)");
}

}  // namespace
}  // namespace tint::resolver
