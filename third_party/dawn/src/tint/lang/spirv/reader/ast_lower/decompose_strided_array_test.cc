// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/ast_lower/decompose_strided_array.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/spirv/reader/ast_lower/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_lower/simplify_pointers.h"
#include "src/tint/lang/spirv/reader/ast_lower/unshadow.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::spirv::reader {
namespace {

using DecomposeStridedArrayTest = ast::transform::TransformTest;
using Unshadow = ast::transform::Unshadow;
using SimplifyPointers = ast::transform::SimplifyPointers;

TEST_F(DecomposeStridedArrayTest, ShouldRunEmptyModule) {
    ProgramBuilder b;
    EXPECT_FALSE(ShouldRun<DecomposeStridedArray>(resolver::Resolve(b)));
}

TEST_F(DecomposeStridedArrayTest, ShouldRunNonStridedArray) {
    // var<private> arr : array<f32, 4u>

    ProgramBuilder b;
    b.GlobalVar("arr", b.ty.array<f32, 4u>(), core::AddressSpace::kPrivate);
    EXPECT_FALSE(ShouldRun<DecomposeStridedArray>(resolver::Resolve(b)));
}

TEST_F(DecomposeStridedArrayTest, ShouldRunDefaultStridedArray) {
    // var<private> arr : @stride(4) array<f32, 4u>

    ProgramBuilder b;
    b.GlobalVar("arr",
                b.ty.array<f32, 4u>(Vector{
                    b.Stride(4),
                }),
                core::AddressSpace::kPrivate);
    EXPECT_TRUE(ShouldRun<DecomposeStridedArray>(resolver::Resolve(b)));
}

TEST_F(DecomposeStridedArrayTest, ShouldRunExplicitStridedArray) {
    // var<private> arr : @stride(16) array<f32, 4u>

    ProgramBuilder b;
    b.GlobalVar("arr",
                b.ty.array<f32, 4u>(Vector{
                    b.Stride(16),
                }),
                core::AddressSpace::kPrivate);
    EXPECT_TRUE(ShouldRun<DecomposeStridedArray>(resolver::Resolve(b)));
}

TEST_F(DecomposeStridedArrayTest, Empty) {
    auto* src = R"()";
    auto* expect = src;

    auto got = Run<DecomposeStridedArray>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, PrivateDefaultStridedArray) {
    // var<private> arr : @stride(4) array<f32, 4u>
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(4) array<f32, 4u> = arr;
    //   let b : f32 = arr[1];
    // }

    ProgramBuilder b;
    b.GlobalVar("arr",
                b.ty.array<f32, 4u>(Vector{
                    b.Stride(4),
                }),
                core::AddressSpace::kPrivate);
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(Vector{
                                b.Stride(4),
                            }),
                            b.Expr("arr"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor("arr", 1_i))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
var<private> arr : array<f32, 4u>;

@compute @workgroup_size(1i)
fn f() {
  let a : array<f32, 4u> = arr;
  let b : f32 = arr[1i];
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, PrivateDefaultStridedArray_ViaPointerIndex) {
    // var<private> arr : @stride(4) array<f32, 4u>
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(4) array<f32, 4u> = arr;
    //   let p = &arr;
    //   let b : f32 = p[1];
    // }

    ProgramBuilder b;
    b.GlobalVar("arr",
                b.ty.array<f32, 4u>(Vector{
                    b.Stride(4),
                }),
                core::AddressSpace::kPrivate);
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(Vector{
                                b.Stride(4),
                            }),
                            b.Expr("arr"))),
               b.Decl(b.Let("p", b.AddressOf(b.Expr("arr")))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor("p", 1_i))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
var<private> arr : array<f32, 4u>;

@compute @workgroup_size(1i)
fn f() {
  let a : array<f32, 4u> = arr;
  let b : f32 = arr[1i];
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, PrivateStridedArray) {
    // var<private> arr : @stride(32) array<f32, 4u>
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(32) array<f32, 4u> = a;
    //   let b : f32 = arr[1];
    // }

    ProgramBuilder b;
    b.GlobalVar("arr",
                b.ty.array<f32, 4u>(Vector{
                    b.Stride(32),
                }),
                core::AddressSpace::kPrivate);
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(Vector{
                                b.Stride(32),
                            }),
                            b.Expr("arr"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor("arr", 1_i))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct strided_arr {
  @size(32)
  el : f32,
}

var<private> arr : array<strided_arr, 4u>;

@compute @workgroup_size(1i)
fn f() {
  let a : array<strided_arr, 4u> = arr;
  let b : f32 = arr[1i].el;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadUniformStridedArray) {
    // struct S {
    //   a : @stride(32) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(32) array<f32, 4u> = s.a;
    //   let b : f32 = s.a[1];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{b.Member("a", b.ty.array<f32, 4u>(Vector{
                                                        b.Stride(32),
                                                    }))});
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(Vector{
                                b.Stride(32),
                            }),
                            b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct strided_arr {
  @size(32)
  el : f32,
}

struct S {
  a : array<strided_arr, 4u>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : array<strided_arr, 4u> = s.a;
  let b : f32 = s.a[1i].el;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadUniformDefaultStridedArray) {
    // struct S {
    //   a : @stride(16) array<vec4<f32>, 4u>,
    // };
    // @group(0) @binding(0) var<uniform> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(16) array<vec4<f32>, 4u> = s.a;
    //   let b : f32 = s.a[1][2];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{b.Member("a", b.ty.array(b.ty.vec4<f32>(), 4_u,
                                                               Vector{
                                                                   b.Stride(16),
                                                               }))});
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.Func(
        "f", tint::Empty, b.ty.void_(),
        Vector{
            b.Decl(b.Let("a",
                         b.ty.array(b.ty.vec4<f32>(), 4_u,
                                    Vector{
                                        b.Stride(16),
                                    }),
                         b.MemberAccessor("s", "a"))),
            b.Decl(b.Let("b", b.ty.f32(),
                         b.IndexAccessor(b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i), 2_i))),
        },
        Vector{
            b.Stage(ast::PipelineStage::kCompute),
            b.WorkgroupSize(1_i),
        });

    auto* expect =
        R"(
struct S {
  a : array<vec4<f32>, 4u>,
}

@group(0) @binding(0) var<uniform> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : array<vec4<f32>, 4u> = s.a;
  let b : f32 = s.a[1i][2i];
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadStorageStridedArray) {
    // struct S {
    //   a : @stride(32) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(32) array<f32, 4u> = s.a;
    //   let b : f32 = s.a[1];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{b.Member("a", b.ty.array<f32, 4u>(Vector{
                                                        b.Stride(32),
                                                    }))});
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(Vector{
                                b.Stride(32),
                            }),
                            b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct strided_arr {
  @size(32)
  el : f32,
}

struct S {
  a : array<strided_arr, 4u>,
}

@group(0) @binding(0) var<storage> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : array<strided_arr, 4u> = s.a;
  let b : f32 = s.a[1i].el;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadStorageDefaultStridedArray) {
    // struct S {
    //   a : @stride(4) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : @stride(4) array<f32, 4u> = s.a;
    //   let b : f32 = s.a[1];
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{b.Member("a", b.ty.array<f32, 4u>(Vector{
                                                        b.Stride(4),
                                                    }))});
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("a",
                            b.ty.array<f32, 4u>(Vector{
                                b.Stride(4),
                            }),
                            b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i))),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct S {
  a : array<f32, 4u>,
}

@group(0) @binding(0) var<storage> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : array<f32, 4u> = s.a;
  let b : f32 = s.a[1i];
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, WriteStorageStridedArray) {
    // struct S {
    //   a : @stride(32) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.a = @stride(32) array<f32, 4u>();
    //   s.a = @stride(32) array<f32, 4u>(1.0, 2.0, 3.0, 4.0);
    //   s.a[1i] = 5.0;
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{b.Member("a", b.ty.array<f32, 4u>(Vector{
                                                        b.Stride(32),
                                                    }))});
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Assign(b.MemberAccessor("s", "a"), b.Call(b.ty.array<f32, 4u>(Vector{
                                                        b.Stride(32),
                                                    }))),
               b.Assign(b.MemberAccessor("s", "a"), b.Call(b.ty.array<f32, 4u>(Vector{
                                                               b.Stride(32),
                                                           }),
                                                           1_f, 2_f, 3_f, 4_f)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i), 5_f),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect =
        R"(
struct strided_arr {
  @size(32)
  el : f32,
}

struct S {
  a : array<strided_arr, 4u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  s.a = array<strided_arr, 4u>();
  s.a = array<strided_arr, 4u>(strided_arr(1.0f), strided_arr(2.0f), strided_arr(3.0f), strided_arr(4.0f));
  s.a[1i].el = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, WriteStorageDefaultStridedArray) {
    // struct S {
    //   a : @stride(4) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   s.a = @stride(4) array<f32, 4u>();
    //   s.a = @stride(4) array<f32, 4u>(1.0, 2.0, 3.0, 4.0);
    //   s.a[1] = 5.0;
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{
                                   b.Member("a", b.ty.array<f32, 4u>(Vector{
                                                     b.Stride(4),
                                                 })),
                               });
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Assign(b.MemberAccessor("s", "a"), b.Call(b.ty.array<f32, 4u>(Vector{
                                                        b.Stride(4),
                                                    }))),
               b.Assign(b.MemberAccessor("s", "a"), b.Call(b.ty.array<f32, 4u>(Vector{
                                                               b.Stride(4),
                                                           }),
                                                           1_f, 2_f, 3_f, 4_f)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i), 5_f),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect =
        R"(
struct S {
  a : array<f32, 4u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  s.a = array<f32, 4u>();
  s.a = array<f32, 4u>(1.0f, 2.0f, 3.0f, 4.0f);
  s.a[1i] = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, ReadWriteViaPointerLets) {
    // struct S {
    //   a : @stride(32) array<f32, 4u>,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a = &s.a;
    //   let b = &*&*(a);
    //   let c = *b;
    //   let d = (*b)[1];
    //   (*b) = @stride(32) array<f32, 4u>(1.0, 2.0, 3.0, 4.0);
    //   (*b)[1] = 5.0;
    // }
    ProgramBuilder b;
    auto* S = b.Structure("S", Vector{b.Member("a", b.ty.array<f32, 4u>(Vector{
                                                        b.Stride(32),
                                                    }))});
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("a", b.AddressOf(b.MemberAccessor("s", "a")))),
               b.Decl(b.Let("b", b.AddressOf(b.Deref(b.AddressOf(b.Deref("a")))))),
               b.Decl(b.Let("c", b.Deref("b"))),
               b.Decl(b.Let("d", b.IndexAccessor(b.Deref("b"), 1_i))),
               b.Assign(b.Deref("b"), b.Call(b.ty.array<f32, 4u>(Vector{
                                                 b.Stride(32),
                                             }),
                                             1_f, 2_f, 3_f, 4_f)),
               b.Assign(b.IndexAccessor(b.Deref("b"), 1_i), 5_f),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect =
        R"(
struct strided_arr {
  @size(32)
  el : f32,
}

struct S {
  a : array<strided_arr, 4u>,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let c = s.a;
  let d = s.a[1i].el;
  s.a = array<strided_arr, 4u>(strided_arr(1.0f), strided_arr(2.0f), strided_arr(3.0f), strided_arr(4.0f));
  s.a[1i].el = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, PrivateAliasedStridedArray) {
    // type ARR = @stride(32) array<f32, 4u>;
    // struct S {
    //   a : ARR,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : ARR = s.a;
    //   let b : f32 = s.a[1];
    //   s.a = ARR();
    //   s.a = ARR(1.0, 2.0, 3.0, 4.0);
    //   s.a[1] = 5.0;
    // }
    ProgramBuilder b;
    b.Alias("ARR", b.ty.array<f32, 4u>(Vector{
                       b.Stride(32),
                   }));
    auto* S = b.Structure("S", Vector{b.Member("a", b.ty("ARR"))});
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("a", b.ty("ARR"), b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b", b.ty.f32(), b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i))),
               b.Assign(b.MemberAccessor("s", "a"), b.Call("ARR")),
               b.Assign(b.MemberAccessor("s", "a"), b.Call("ARR", 1_f, 2_f, 3_f, 4_f)),
               b.Assign(b.IndexAccessor(b.MemberAccessor("s", "a"), 1_i), 5_f),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect = R"(
struct strided_arr {
  @size(32)
  el : f32,
}

alias ARR = array<strided_arr, 4u>;

struct S {
  a : ARR,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : ARR = s.a;
  let b : f32 = s.a[1i].el;
  s.a = ARR();
  s.a = ARR(strided_arr(1.0f), strided_arr(2.0f), strided_arr(3.0f), strided_arr(4.0f));
  s.a[1i].el = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}

TEST_F(DecomposeStridedArrayTest, PrivateNestedStridedArray) {
    // type ARR_A = @stride(8) array<f32, 2u>;
    // type ARR_B = @stride(128) array<@stride(16) array<ARR_A, 3u>, 4u>;
    // struct S {
    //   a : ARR_B,
    // };
    // @group(0) @binding(0) var<storage, read_write> s : S;
    //
    // @compute @workgroup_size(1)
    // fn f() {
    //   let a : ARR_B = s.a;
    //   let b : array<@stride(8) array<f32, 2u>, 3u> = s.a[3];
    //   let c = s.a[3][2];
    //   let d = s.a[3][2][1];
    //   s.a = ARR_B();
    //   s.a[3][2][1] = 5.0;
    // }

    ProgramBuilder b;
    b.Alias("ARR_A", b.ty.array<f32, 2>(Vector{
                         b.Stride(8),
                     }));
    b.Alias("ARR_B", b.ty.array(  //
                         b.ty.array(b.ty("ARR_A"), 3_u,
                                    Vector{
                                        b.Stride(16),
                                    }),
                         4_u,
                         Vector{
                             b.Stride(128),
                         }));
    auto* S = b.Structure("S", Vector{b.Member("a", b.ty("ARR_B"))});
    b.GlobalVar("s", b.ty.Of(S), core::AddressSpace::kStorage, core::Access::kReadWrite,
                b.Group(0_a), b.Binding(0_a));
    b.Func("f", tint::Empty, b.ty.void_(),
           Vector{
               b.Decl(b.Let("a", b.ty("ARR_B"), b.MemberAccessor("s", "a"))),
               b.Decl(b.Let("b",
                            b.ty.array(b.ty("ARR_A"), 3_u,
                                       Vector{
                                           b.Stride(16),
                                       }),
                            b.IndexAccessor(                 //
                                b.MemberAccessor("s", "a"),  //
                                3_i))),
               b.Decl(b.Let("c", b.ty("ARR_A"),
                            b.IndexAccessor(                     //
                                b.IndexAccessor(                 //
                                    b.MemberAccessor("s", "a"),  //
                                    3_i),
                                2_i))),
               b.Decl(b.Let("d", b.ty.f32(),
                            b.IndexAccessor(                         //
                                b.IndexAccessor(                     //
                                    b.IndexAccessor(                 //
                                        b.MemberAccessor("s", "a"),  //
                                        3_i),
                                    2_i),
                                1_i))),
               b.Assign(b.MemberAccessor("s", "a"), b.Call("ARR_B")),
               b.Assign(b.IndexAccessor(                         //
                            b.IndexAccessor(                     //
                                b.IndexAccessor(                 //
                                    b.MemberAccessor("s", "a"),  //
                                    3_i),
                                2_i),
                            1_i),
                        5_f),
           },
           Vector{
               b.Stage(ast::PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    auto* expect =
        R"(
struct strided_arr {
  @size(8)
  el : f32,
}

alias ARR_A = array<strided_arr, 2u>;

struct strided_arr_1 {
  @size(128)
  el : array<ARR_A, 3u>,
}

alias ARR_B = array<strided_arr_1, 4u>;

struct S {
  a : ARR_B,
}

@group(0) @binding(0) var<storage, read_write> s : S;

@compute @workgroup_size(1i)
fn f() {
  let a : ARR_B = s.a;
  let b : array<ARR_A, 3u> = s.a[3i].el;
  let c : ARR_A = s.a[3i].el[2i];
  let d : f32 = s.a[3i].el[2i][1i].el;
  s.a = ARR_B();
  s.a[3i].el[2i][1i].el = 5.0f;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, DecomposeStridedArray>(resolver::Resolve(b));

    EXPECT_EQ(expect, str(got));
}
}  // namespace
}  // namespace tint::spirv::reader
