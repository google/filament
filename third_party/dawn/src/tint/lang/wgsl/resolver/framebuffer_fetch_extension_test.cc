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

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using FramebufferFetchExtensionTest = ResolverTest;

TEST_F(FramebufferFetchExtensionTest, ColorParamUsedWithExtension) {
    // enable chromium_experimental_framebuffer_fetch;
    // fn f(@color(2) p : vec4<f32>) {}

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumExperimentalFramebufferFetch);

    auto* ast_param = Param("p", ty.vec4<f32>(), Vector{Color(2_a)});
    Func("f", Vector{ast_param}, ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_param = Sem().Get(ast_param);
    ASSERT_NE(sem_param, nullptr);
    EXPECT_EQ(sem_param->Attributes().color, 2u);
}

TEST_F(FramebufferFetchExtensionTest, ColorParamUsedWithoutExtension) {
    // enable chromium_experimental_framebuffer_fetch;
    // struct S {
    //   @color(2) c : vec4<f32>,
    // }

    Func("f", Vector{Param("p", ty.vec4<f32>(), Vector{Color(Source{{12, 34}}, 2_a)})}, ty.void_(),
         Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@color' requires enabling extension 'chromium_experimental_framebuffer_fetch')");
}

TEST_F(FramebufferFetchExtensionTest, ColorMemberUsedWithExtension) {
    // enable chromium_experimental_framebuffer_fetch;
    // fn f(@color(2) p : vec4<f32>) {}

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumExperimentalFramebufferFetch);

    auto* ast_member = Member("c", ty.vec4<f32>(), Vector{Color(2_a)});
    Structure("S", Vector{ast_member});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_member = Sem().Get(ast_member);
    ASSERT_NE(sem_member, nullptr);
    EXPECT_EQ(sem_member->Attributes().color, 2u);
}

TEST_F(FramebufferFetchExtensionTest, ColorMemberUsedWithoutExtension) {
    // enable chromium_experimental_framebuffer_fetch;
    // fn f(@color(2) p : vec4<f32>) {}

    Structure("S", Vector{Member("c", ty.vec4<f32>(), Vector{Color(Source{{12, 34}}, 2_a)})});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: use of '@color' requires enabling extension 'chromium_experimental_framebuffer_fetch')");
}

TEST_F(FramebufferFetchExtensionTest, DuplicateColorParams) {
    // enable chromium_experimental_framebuffer_fetch;
    // fn f(@color(1) a : vec4<f32>, @color(2) b : vec4<f32>, @color(1) c : vec4<f32>) {}

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumExperimentalFramebufferFetch);

    Func("f",
         Vector{
             Param("a", ty.vec4<f32>(), Vector{Color(1_a)}),
             Param("b", ty.vec4<f32>(), Vector{Color(2_a)}),
             Param("c", ty.vec4<f32>(), Vector{Color(Source{{1, 2}}, 1_a)}),
         },
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(1:2 error: '@color(1)' appears multiple times)");
}

TEST_F(FramebufferFetchExtensionTest, DuplicateColorStruct) {
    // enable chromium_experimental_framebuffer_fetch;
    // struct S {
    //   @color(1) a : vec4<f32>,
    //   @color(2) b : vec4<f32>,
    //   @color(1) c : vec4<f32>,
    // }
    // fn f(s : S) {}

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumExperimentalFramebufferFetch);

    Structure("S", Vector{
                       Member("a", ty.vec4<f32>(), Vector{Color(1_a)}),
                       Member("b", ty.vec4<f32>(), Vector{Color(2_a)}),
                       Member("c", ty.vec4<f32>(), Vector{Color(Source{{1, 2}}, 1_a)}),
                   });

    Func("f", Vector{Param("s", ty("S"))}, ty.void_(), Empty,
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(1:2 error: '@color(1)' appears multiple times)");
}

TEST_F(FramebufferFetchExtensionTest, DuplicateColorParamAndStruct) {
    // enable chromium_experimental_framebuffer_fetch;
    // struct S {
    //   @color(1) b : vec4<f32>,
    //   @color(2) c : vec4<f32>,
    // }
    // fn f(@color(2) a : vec4<f32>, s : S, @color(3) d : vec4<f32>) {}

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumExperimentalFramebufferFetch);

    Structure("S", Vector{
                       Member("b", ty.vec4<f32>(), Vector{Color(1_a)}),
                       Member("c", ty.vec4<f32>(), Vector{Color(Source{{1, 2}}, 2_a)}),
                   });

    Func("f",
         Vector{
             Param("a", ty.vec4<f32>(), Vector{Color(2_a)}),
             Param("s", ty("S")),
             Param("d", ty.vec4<f32>(), Vector{Color(3_a)}),
         },
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(1:2 error: '@color(2)' appears multiple times
note: while analyzing entry point 'f')");
}

namespace type_tests {
struct Case {
    builder::ast_type_func_ptr type;
    std::string name;
    bool pass;
};

static std::ostream& operator<<(std::ostream& o, const Case& c) {
    return o << c.name;
}

template <typename T>
Case Pass() {
    return Case{builder::DataType<T>::AST, builder::DataType<T>::Name(), true};
}

template <typename T>
Case Fail() {
    return Case{builder::DataType<T>::AST, builder::DataType<T>::Name(), false};
}

using FramebufferFetchExtensionTest_Types = ResolverTestWithParam<Case>;

TEST_P(FramebufferFetchExtensionTest_Types, Param) {
    // enable chromium_experimental_framebuffer_fetch;
    // fn f(@color(1) a : <type>) {}

    Enable(wgsl::Extension::kChromiumExperimentalFramebufferFetch);

    Func("f",
         Vector{Param(Source{{12, 34}}, "p", GetParam().type(*this),
                      Vector{Color(Source{{56, 78}}, 2_a)})},
         ty.void_(), Empty, Vector{Stage(ast::PipelineStage::kFragment)});

    if (GetParam().pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        auto expected =
            ReplaceAll(R"(12:34 error: cannot apply '@color' to declaration of type '$TYPE'
56:78 note: '@color' must only be applied to declarations of numeric scalar or numeric vector type)",
                       "$TYPE", GetParam().name);
        EXPECT_EQ(r()->error(), expected);
    }
}

TEST_P(FramebufferFetchExtensionTest_Types, Struct) {
    // struct S {
    //   @color(2) c : <type>,
    // }

    Enable(wgsl::Extension::kChromiumExperimentalFramebufferFetch);

    Structure("S", Vector{
                       Member(Source{{12, 34}}, "c", GetParam().type(*this),
                              Vector{Color(Source{{56, 78}}, 2_a)}),
                   });

    if (GetParam().pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        auto expected =
            ReplaceAll(R"(12:34 error: cannot apply '@color' to declaration of type '$TYPE'
56:78 note: '@color' must only be applied to declarations of numeric scalar or numeric vector type)",
                       "$TYPE", GetParam().name);
        EXPECT_EQ(r()->error(), expected);
    }
}

INSTANTIATE_TEST_SUITE_P(Valid,
                         FramebufferFetchExtensionTest_Types,
                         testing::Values(Pass<i32>(),
                                         Pass<u32>(),
                                         Pass<f32>(),
                                         Pass<vec2<f32>>(),
                                         Pass<vec3<i32>>(),
                                         Pass<vec4<u32>>()));

INSTANTIATE_TEST_SUITE_P(Invalid,
                         FramebufferFetchExtensionTest_Types,
                         testing::Values(Fail<bool>(), Fail<array<u32, 4>>()));

}  // namespace type_tests

}  // namespace
}  // namespace tint::resolver
