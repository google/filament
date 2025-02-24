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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::resolver {
namespace {

using DualSourceBlendingExtensionTest = ResolverTest;

// Using the @blend_src attribute without dual_source_blending enabled should fail.
TEST_F(DualSourceBlendingExtensionTest, UseBlendSrcAttribWithoutExtensionError) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 0_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@blend_src' requires enabling extension 'dual_source_blending')");
}

class DualSourceBlendingExtensionTests : public ResolverTest {
  public:
    DualSourceBlendingExtensionTests() { Enable(wgsl::Extension::kDualSourceBlending); }
};

// Using an F32 as a @blend_src value should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcF32Error) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 0_f)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' value must be 'i32' or 'u32'");
}

// Using a floating point number as a @blend_src value should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcFloatValueError) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 1.0_a)}),
                        });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' value must be 'i32' or 'u32'");
}

// Using a number less than zero as a @blend_src value should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcNegativeValue) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, -1_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' value must be zero or one");
}

// Using a number greater than one as a @blend_src value should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcValueAboveOne) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 2_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' value must be zero or one");
}

// Using a @blend_src value at the same location multiple times should fail.
TEST_F(DualSourceBlendingExtensionTests, DuplicateBlendSrces) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                            Member(Source{{12, 34}}, "b", ty.vec4<f32>(),
                                   Vector{Location(Source{{12, 34}}, 0_a), BlendSrc(0_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@location(0) @blend_src(0)' appears multiple times");
}

// Using the @blend_src attribute without a location attribute should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcWithMissingLocationAttribute_Struct) {
    Structure("Output", Vector{
                            Member(Source{{12, 34}}, "a", ty.vec4<f32>(),
                                   Vector{BlendSrc(Source{{12, 34}}, 1_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' can only be used with '@location(0)'");
}

// Using a @blend_src attribute on a struct member while there is only one member in the struct
// should fail.
TEST_F(DualSourceBlendingExtensionTests, StructMemberBlendSrcAttribute_OnlyBlendSrc0) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 0_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src(1)' is missing when '@blend_src' is used");
}

// Using a @blend_src attribute on a struct member while there is only one member in the struct
// should fail.
TEST_F(DualSourceBlendingExtensionTests, StructMemberBlendSrcAttribute_OnlyBlendSrc1) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(),
                                   Vector{Location(0_a), BlendSrc(Source{{12, 34}}, 1_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src(0)' is missing when '@blend_src' is used");
}

// Using the a @blend_src attribute with a non-zero location should fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcWithNonZeroLocation_Struct) {
    Structure("Output", Vector{
                            Member("a", ty.vec4<f32>(),
                                   Vector{Location(1_a), BlendSrc(Source{{12, 34}}, 0_a)}),
                        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: '@blend_src' can only be used with '@location(0)'");
}

// Using @blend_src and @location(0) on two members and having another without @location should not
// fail.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcAndNonLocationNonBlendSrc) {
    // struct S {
    //   a : vec4<f32>,
    //   @location(0) @blend_src(0) b : vec4<f32>,
    //   @location(0) @blend_src(1) c : vec4<f32>,
    // };
    Structure("S",
              Vector{
                  Member("a", ty.vec4<f32>()),
                  Member("b", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(Source{{3, 4}}, 0_a)}),
                  Member("c", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(Source{{5, 6}}, 1_a)}),
              });

    EXPECT_TRUE(r()->Resolve());
}

// Using @blend_src and @location(0) on two members and having another member with @location but
// without @blend_src should fail.
TEST_F(DualSourceBlendingExtensionTests, ZeroLocationAndNonBlendSrcBeforeBlendSrc) {
    // struct S {
    //   @location(0) a : vec4<f32>,
    //   @location(0) @blend_src(0) b : vec4<f32>,
    //   @location(0) @blend_src(1) c : vec4<f32>,
    // };
    Structure("S", Vector{
                       Member("a", ty.vec4<f32>(), Vector{Location(0_a)}),
                       Member(Source{{12, 34}}, "b", ty.vec4<f32>(),
                              Vector{Location(0_a), BlendSrc(0_a)}),
                       Member("c", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(1_a)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: '@blend_src' and '@location' are used on one member while another member with '@location' doesn't use '@blend_src')");
}

// Using @blend_src and @location(0) on two members and having another member with @location but
// without @blend_src should fail.
TEST_F(DualSourceBlendingExtensionTests, ZeroLocationAndNonBlendSrcAfterBlendSrc) {
    // struct S {
    //   @location(0) @blend_src(0) a : vec4<f32>,
    //   @location(0) b : vec4<f32>,
    //   @location(0) @blend_src(1) c : vec4<f32>,
    // };
    Structure("S", Vector{
                       Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                       Member(Source{{12, 34}}, "b", ty.vec4<f32>(), Vector{Location(0_a)}),
                       Member("c", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(1_a)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: '@blend_src' and '@location' are used on one member while another member with '@location' doesn't use '@blend_src')");
}

// Using @blend_src and @location(0) on two members and having another member with @location but
// without @blend_src should fail.
TEST_F(DualSourceBlendingExtensionTests, NonZeroLocationAndNonBlendSrcBeforeBlendSrc) {
    // struct S {
    //   @location(1) a : vec4<f32>,
    //   @location(0) @blend_src(0) b : vec4<f32>,
    //   @location(0) @blend_src(1) c : vec4<f32>,
    // };
    Structure("S", Vector{
                       Member("a", ty.vec4<f32>(), Vector{Location(1_a)}),
                       Member(Source{{12, 34}}, "b", ty.vec4<f32>(),
                              Vector{Location(0_a), BlendSrc(0_a)}),
                       Member("c", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(1_a)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: '@blend_src' and '@location' are used on one member while another member with '@location' doesn't use '@blend_src')");
}

// Using @blend_src and @location(0) on two members and having another member with @location but
// without @blend_src should fail.
TEST_F(DualSourceBlendingExtensionTests, NonZeroLocationAndNonBlendSrcAfterBlendSrc) {
    // struct S {
    //   @location(0) @blend_src(0) a : vec4<f32>,
    //   @location(1) b : vec4<f32>,
    //   @location(0) @blend_src(1) c : vec4<f32>,
    // };
    Structure("S", Vector{
                       Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                       Member(Source{{12, 34}}, "b", ty.vec4<f32>(), Vector{Location(1_a)}),
                       Member("c", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(1_a)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: '@blend_src' and '@location' are used on one member while another member with '@location' doesn't use '@blend_src')");
}

// The members with @blend_src and @location(0) must have same type.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcTypes_DifferentWidth) {
    // struct S {
    //   @location(0) @blend_src(0) a : vec4<f32>,
    //   @location(0) @blend_src(1) b : vec2<f32>,
    // };
    Structure("S",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                  Member("b", ty.vec2<f32>(), Vector{Location(0_a), BlendSrc(Source{{1, 2}}, 1_a)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(1:2 error: All the outputs with '@blend_src' must have same type)");
}

// The members with @blend_src and @location(0) must have same type.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcTypes_DifferentElementType) {
    // struct S {
    //   @location(0) @blend_src(0) a : vec4<f32>,
    //   @location(0) @blend_src(1) b : vec4<i32>,
    // };
    Structure("S",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                  Member("b", ty.vec4<i32>(), Vector{Location(0_a), BlendSrc(Source{{1, 2}}, 1_a)}),
              });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(1:2 error: All the outputs with '@blend_src' must have same type)");
}

// It is not allowed to use a struct with @blend_src as fragment shader input.
TEST_F(DualSourceBlendingExtensionTests, BlendSrcAsFragmentInput) {
    // struct S {
    //   @location(0) @blend_src(0) a : vec4<f32>,
    //   @location(0) @blend_src(1) b : vec4<f32>,
    // };
    // @fragment fn F(s_in : S) -> S { return S(); }
    Structure("S", Vector{
                       Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                       Member("b", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(1_a)}),
                   });
    Func("F", Vector{Param("s_in", ty("S"))}, ty("S"), Vector{Return(Call("S"))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: '@blend_src' can only be used for fragment shader output
note: while analyzing entry point 'F')");
}

// It is not allowed to use @blend_src on the fragment output declaration.
TEST_F(DualSourceBlendingExtensionTest, BlendSrcOnNonStructFragmentOutput) {
    // enable dual_source_blending;
    // @fragment fn F() -> @location(0) @blend_src(0) vec4<f32> {
    //     return vec4<f32>();
    // }
    Enable(wgsl::Extension::kDualSourceBlending);
    Func("F", Empty, ty.vec4<f32>(), Vector{Return(Call("vec4f"))},
         Vector{Stage(ast::PipelineStage::kFragment)},
         Vector{Location(0_a), BlendSrc(Source{{1, 2}}, 0_a)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(1:2 error: '@blend_src' is not valid for entry point return types)");
}

class DualSourceBlendingExtensionTestWithParams : public ResolverTestWithParam<int> {
  public:
    DualSourceBlendingExtensionTestWithParams() { Enable(wgsl::Extension::kDualSourceBlending); }
};

// Rendering to multiple render targets while using dual source blending should fail.
TEST_P(DualSourceBlendingExtensionTestWithParams,
       MultipleRenderTargetsNotAllowed_BlendSrcAndNoBlendSrc) {
    // struct S {
    //   @location(0) @blend_src(0) a : vec4<f32>,
    //   @location(0) @blend_src(1) b : vec4<f32>,
    //   @location(n)           c : vec4<f32>,
    // };
    // fn F() -> S { return S(); }
    Structure("S",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                  Member("b", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(Source{{1, 2}}, 1_a)}),
                  Member(Source{{3, 4}}, "c", ty.vec4<f32>(), Vector{Location(AInt(GetParam()))}),
              });
    Func("F", Empty, ty("S"), Vector{Return(Call("S"))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(3:4 error: '@blend_src' and '@location' are used on one member while another member with '@location' doesn't use '@blend_src')");
}

TEST_P(DualSourceBlendingExtensionTestWithParams,
       MultipleRenderTargetsNotAllowed_NonZeroLocationThenBlendSrc) {
    // struct S {
    //   @location(n)           a : vec4<f32>,
    //   @location(0) @blend_src(0) b : vec4<f32>,
    //   @location(0) @blend_src(1) c : vec4<f32>,
    // };
    // fn F() -> S { return S(); }
    Structure("S",
              Vector{
                  Member("a", ty.vec4<f32>(), Vector{Location(AInt(GetParam()))}),
                  Member(Source{{1, 2}}, "b", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(0_a)}),
                  Member("c", ty.vec4<f32>(), Vector{Location(0_a), BlendSrc(Source{{3, 4}}, 1_a)}),
              });
    Func(Source{{5, 6}}, "F", Empty, ty("S"), Vector{Return(Call("S"))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: '@blend_src' and '@location' are used on one member while another member with '@location' doesn't use '@blend_src')");
}

INSTANTIATE_TEST_SUITE_P(DualSourceBlendingExtensionTests,
                         DualSourceBlendingExtensionTestWithParams,
                         testing::Values(1, 2, 3, 4, 5, 6, 7));

}  // namespace
}  // namespace tint::resolver
