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

using ResolverBarycentricCoordExtensionTest = ResolverTest;

TEST_F(ResolverBarycentricCoordExtensionTest, UseBarycentricCoordWithoutExtensionError) {
    Structure("Inputs", Vector{
                            Member("a", ty.vec3<f32>(),
                                   Vector{Builtin(core::BuiltinValue::kBarycentricCoord)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of '@builtin(barycentric_coord)' requires enabling extension 'chromium_experimental_barycentric_coord')");
}

TEST_F(ResolverBarycentricCoordExtensionTest, UseBarycentricCoordWithExtension) {
    Enable(wgsl::Extension::kChromiumExperimentalBarycentricCoord);
    Structure("Inputs", Vector{
                            Member("a", ty.vec3<f32>(),
                                   Vector{Builtin(core::BuiltinValue::kBarycentricCoord)}),
                        });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBarycentricCoordExtensionTest, BarycentricCoordVec3I32Error) {
    Enable(wgsl::Extension::kChromiumExperimentalBarycentricCoord);
    Structure("Inputs", Vector{
                            Member("a", ty.vec3<i32>(),
                                   Vector{Builtin(core::BuiltinValue::kBarycentricCoord)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: store type of '@builtin(barycentric_coord)' must be 'vec3<f32>'");
}

TEST_F(ResolverBarycentricCoordExtensionTest, BarycentricCoordVertexShader) {
    Enable(wgsl::Extension::kChromiumExperimentalBarycentricCoord);
    Func(
        "main",
        Vector{Param("pi", ty.vec3<f32>(), Vector{Builtin(core::BuiltinValue::kBarycentricCoord)})},
        ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kVertex)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "error: '@builtin(barycentric_coord)' cannot be used for vertex shader input");
}

TEST_F(ResolverBarycentricCoordExtensionTest, BarycentricCoordFragmentShaderOutput) {
    Enable(wgsl::Extension::kChromiumExperimentalBarycentricCoord);

    Func("main", tint::Empty, ty.vec3<f32>(),
         Vector{
             Return(Call<vec3<f32>>()),
         },
         Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Builtin(Source{{1, 2}}, core::BuiltinValue::kBarycentricCoord)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:2 error: '@builtin(barycentric_coord)' cannot be used for fragment shader output");
}

}  // namespace
}  // namespace tint::resolver
