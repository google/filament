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

using ResolverPixelLocalExtensionTest = ResolverTest;

TEST_F(ResolverPixelLocalExtensionTest, UseWithFramebufferFetch) {
    // enable chromium_experimental_pixel_local;
    // enable chromium_experimental_framebuffer_fetch;

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumExperimentalPixelLocal);
    Enable(Source{{56, 78}}, wgsl::Extension::kChromiumExperimentalFramebufferFetch);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: extension 'chromium_experimental_pixel_local' cannot be used with extension 'chromium_experimental_framebuffer_fetch'
56:78 note: 'chromium_experimental_framebuffer_fetch' enabled here)");
}

TEST_F(ResolverPixelLocalExtensionTest, AddressSpaceUsedWithExtension) {
    // enable chromium_experimental_pixel_local;
    // struct S { a : i32 }
    // var<pixel_local> v : S;
    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumExperimentalPixelLocal);

    Structure("S", Vector{Member("a", ty.i32())});

    GlobalVar("v", ty("S"), core::AddressSpace::kPixelLocal);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverPixelLocalExtensionTest, AddressSpaceUsedWithoutExtension) {
    // struct S { a : i32 }
    // var<pixel_local> v : S;

    Structure("S", Vector{Member("a", ty.i32())});

    AST().AddGlobalVariable(create<ast::Var>(
        /* name */ Ident("v"),
        /* type */ ty("S"),
        /* declared_address_space */ Expr(Source{{12, 34}}, core::AddressSpace::kPixelLocal),
        /* declared_access */ nullptr,
        /* initializer */ nullptr,
        /* attributes */ Empty));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'pixel_local' address space requires the 'chromium_experimental_pixel_local' extension enabled)");
}

TEST_F(ResolverPixelLocalExtensionTest, PixelLocalTwoVariablesUsedInEntryPoint) {
    // enable chromium_experimental_pixel_local;
    // struct S { i : i32 }
    // var<pixel_local> a : S;
    // var<pixel_local> b : S;
    // @compute @workgroup_size(1) fn main() {
    //   _ = a.i;
    //   _ = b.i;
    // }
    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{Member("i", ty.i32())});
    GlobalVar(Source{{1, 2}}, "a", ty("S"), core::AddressSpace::kPixelLocal);
    GlobalVar(Source{{3, 4}}, "b", ty("S"), core::AddressSpace::kPixelLocal);

    Func(Source{{5, 6}}, "main", {}, ty.void_(),
         Vector{Assign(Phony(), MemberAccessor("a", "i")),
                Assign(Phony(), MemberAccessor("b", "i"))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(5:6 error: entry point 'main' uses two different 'pixel_local' variables.
3:4 note: first 'pixel_local' variable declaration is here
1:2 note: second 'pixel_local' variable declaration is here)");
}

TEST_F(ResolverPixelLocalExtensionTest, PixelLocalTwoVariablesUsedInEntryPointWithFunctionGraph) {
    // enable chromium_experimental_pixel_local;
    // struct S { i : i32 }
    // var<pixel_local> a : S;
    // var<pixel_local> b : S;
    // fn uses_a() {
    //   _ = a.i;
    // }
    // fn uses_b() {
    //   _ = b.i;
    // }
    // @compute @workgroup_size(1) fn main() {
    //   uses_a();
    //   uses_b();
    // }
    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{Member("i", ty.i32())});
    GlobalVar(Source{{1, 2}}, "a", ty("S"), core::AddressSpace::kPixelLocal);
    GlobalVar(Source{{3, 4}}, "b", ty("S"), core::AddressSpace::kPixelLocal);

    Func(Source{{5, 6}}, "uses_a", {}, ty.void_(),
         Vector{Assign(Phony(), MemberAccessor("a", "i"))});
    Func(Source{{7, 8}}, "uses_b", {}, ty.void_(),
         Vector{Assign(Phony(), MemberAccessor("b", "i"))});

    Func(Source{{9, 10}}, "main", {}, ty.void_(),
         Vector{CallStmt(Call("uses_a")), CallStmt(Call("uses_b"))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(9:10 error: entry point 'main' uses two different 'pixel_local' variables.
3:4 note: first 'pixel_local' variable declaration is here
7:8 note: called by function 'uses_b'
9:10 note: called by entry point 'main'
1:2 note: second 'pixel_local' variable declaration is here
5:6 note: called by function 'uses_a'
9:10 note: called by entry point 'main')");
}

TEST_F(ResolverPixelLocalExtensionTest, VertexStageDirect) {
    // enable chromium_experimental_pixel_local;
    // struct S { i : 32; }
    // var<pixel_local> v : S;
    // @vertex fn F() -> @position vec4f {
    //   v.i = 42;
    //   return vec4f();
    // }
    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{Member("i", ty.i32())});
    GlobalVar(Source{{56, 78}}, "v", ty("S"), core::AddressSpace::kPixelLocal);
    Func("F", Empty, ty.vec4<f32>(),
         Vector{
             Assign(MemberAccessor(Expr(Source{{12, 34}}, "v"), "i"), 42_a),
             Return(Call<vec4<f32>>()),
         },
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{Builtin(core::BuiltinValue::kPosition)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: var with 'pixel_local' address space cannot be used by vertex pipeline stage
56:78 note: variable is declared here)");
}

TEST_F(ResolverPixelLocalExtensionTest, ComputeStageDirect) {
    // enable chromium_experimental_pixel_local;
    // struct S { i : 32; }
    // var<pixel_local> v : S;
    // @compute @workgroup_size(1) fn F() {
    //   v.i = 42;
    // }
    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{Member("i", ty.i32())});
    GlobalVar(Source{{56, 78}}, "v", ty("S"), core::AddressSpace::kPixelLocal);
    Func("F", Empty, ty.void_(),
         Vector{Assign(MemberAccessor(Expr(Source{{12, 34}}, "v"), "i"), 42_a)},
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_a)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: var with 'pixel_local' address space cannot be used by compute pipeline stage
56:78 note: variable is declared here)");
}

TEST_F(ResolverPixelLocalExtensionTest, FragmentStageDirect) {
    // enable chromium_experimental_pixel_local;
    // struct S { i : 32; }
    // var<pixel_local> v : S;
    // @fragment fn F() {
    //   v.i = 42;
    // }
    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{Member("i", ty.i32())});
    GlobalVar(Source{{56, 78}}, "v", ty("S"), core::AddressSpace::kPixelLocal);
    Func("F", Empty, ty.void_(),
         Vector{Assign(MemberAccessor(Expr(Source{{12, 34}}, "v"), "i"), 42_a)},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverPixelLocalExtensionTest, VertexStageIndirect) {
    // enable chromium_experimental_pixel_local;
    // struct S { i : 32; }
    // var<pixel_local> v : S;
    // fn X() { v = 42; }
    // fn Y() { .iX(); }
    // @vertex fn F() -> @position vec4f {
    //   X();
    //   return vec4f();
    // }
    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{Member("i", ty.i32())});
    GlobalVar(Source{{3, 4}}, "v", ty("S"), core::AddressSpace::kPixelLocal);
    Func(Source{{5, 6}}, "X", Empty, ty.void_(),
         Vector{Assign(MemberAccessor(Expr(Source{{1, 2}}, "v"), "i"), 42_a)});
    Func(Source{{7, 8}}, "Y", Empty, ty.void_(), Vector{CallStmt(Call("X"))});
    Func(Source{{9, 1}}, "F", Empty, ty.vec4<f32>(),
         Vector{
             CallStmt(Call("Y")),
             Return(Call<vec4<f32>>()),
         },
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{Builtin(core::BuiltinValue::kPosition)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: var with 'pixel_local' address space cannot be used by vertex pipeline stage
3:4 note: variable is declared here
5:6 note: called by function 'X'
7:8 note: called by function 'Y'
9:1 note: called by entry point 'F')");
}

TEST_F(ResolverPixelLocalExtensionTest, ComputeStageIndirect) {
    // enable chromium_experimental_pixel_local;
    // struct S { i : 32; }
    // var<pixel_local> v : S;
    // fn X() { v = 42; }
    // fn Y() { .iX(); }
    // @compute @workgroup_size(1) fn F() {
    //   Y();
    // }
    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{Member("i", ty.i32())});
    GlobalVar(Source{{3, 4}}, "v", ty("S"), core::AddressSpace::kPixelLocal);
    Func(Source{{5, 6}}, "X", Empty, ty.void_(),
         Vector{Assign(MemberAccessor(Expr(Source{{1, 2}}, "v"), "i"), 42_a)});
    Func(Source{{7, 8}}, "Y", Empty, ty.void_(), Vector{CallStmt(Call("X"))});
    Func(Source{{9, 1}}, "F", Empty, ty.void_(), Vector{CallStmt(Call("Y"))},
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_a)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: var with 'pixel_local' address space cannot be used by compute pipeline stage
3:4 note: variable is declared here
5:6 note: called by function 'X'
7:8 note: called by function 'Y'
9:1 note: called by entry point 'F')");
}

TEST_F(ResolverPixelLocalExtensionTest, FragmentStageIndirect) {
    // enable chromium_experimental_pixel_local;
    // struct S { i : 32; }
    // var<pixel_local> v : S;
    // fn X() { v = 42; }
    // fn Y() { .iX(); }
    // @fragment fn F() {
    //   Y();
    // }
    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{Member("i", ty.i32())});
    GlobalVar(Source{{3, 4}}, "v", ty("S"), core::AddressSpace::kPixelLocal);
    Func(Source{{5, 6}}, "X", Empty, ty.void_(),
         Vector{Assign(MemberAccessor(Expr(Source{{1, 2}}, "v"), "i"), 42_a)});
    Func(Source{{7, 8}}, "Y", Empty, ty.void_(), Vector{CallStmt(Call("X"))});
    Func(Source{{9, 1}}, "F", Empty, ty.void_(), Vector{CallStmt(Call("Y"))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
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

using ResolverPixelLocalExtensionTest_Types = ResolverTestWithParam<Case>;

TEST_P(ResolverPixelLocalExtensionTest_Types, Direct) {
    // enable chromium_experimental_pixel_local;
    // var<pixel_local> v : <type>;

    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    GlobalVar(Source{{12, 34}}, "v", GetParam().type(*this), core::AddressSpace::kPixelLocal);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: 'pixel_local' variable only support struct storage types)");
}

TEST_P(ResolverPixelLocalExtensionTest_Types, Struct) {
    // enable chromium_experimental_pixel_local;
    // struct S {
    //   a : i32,
    //   m : <type>,
    // }
    // var<pixel_local> v : S;

    Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    Structure("S", Vector{
                       Member("a", ty.i32()),
                       Member(Source{{12, 34}}, "m", GetParam().type(*this)),
                   });
    GlobalVar(Source{{56, 78}}, "v", ty("S"), core::AddressSpace::kPixelLocal);

    if (GetParam().pass) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(
            r()->error(),
            R"(12:34 error: 'struct' members used in the 'pixel_local' address space can only be of the type 'i32', 'u32' or 'f32'
56:78 note: 'struct S' used in the 'pixel_local' address space here)");
    }
}

INSTANTIATE_TEST_SUITE_P(Valid,
                         ResolverPixelLocalExtensionTest_Types,
                         testing::Values(Pass<i32>(),  //
                                         Pass<u32>(),  //
                                         Pass<f32>()));

INSTANTIATE_TEST_SUITE_P(Invalid,
                         ResolverPixelLocalExtensionTest_Types,
                         testing::Values(Fail<bool>(),
                                         Fail<atomic<i32>>(),
                                         Fail<vec2<f32>>(),
                                         Fail<vec3<i32>>(),
                                         Fail<vec4<u32>>(),
                                         Fail<array<u32, 4>>()));

}  // namespace type_tests

}  // namespace
}  // namespace tint::resolver
