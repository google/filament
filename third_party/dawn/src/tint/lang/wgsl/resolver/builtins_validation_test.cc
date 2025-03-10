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

#include <functional>
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/extension.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

template <typename T>
using DataType = builder::DataType<T>;
class ResolverBuiltinsValidationTest : public resolver::TestHelper, public testing::Test {};
namespace StageTest {
struct Params {
    builder::ast_type_func_ptr type;
    core::BuiltinValue builtin;
    ast::PipelineStage stage;
    bool is_valid;
};

template <typename T>
constexpr Params ParamsFor(core::BuiltinValue builtin, ast::PipelineStage stage, bool is_valid) {
    return Params{DataType<T>::AST, builtin, stage, is_valid};
}
static constexpr Params cases[] = {
    ParamsFor<vec4<f32>>(core::BuiltinValue::kPosition, ast::PipelineStage::kVertex, false),
    ParamsFor<vec4<f32>>(core::BuiltinValue::kPosition, ast::PipelineStage::kFragment, true),
    ParamsFor<vec4<f32>>(core::BuiltinValue::kPosition, ast::PipelineStage::kCompute, false),

    ParamsFor<u32>(core::BuiltinValue::kVertexIndex, ast::PipelineStage::kVertex, true),
    ParamsFor<u32>(core::BuiltinValue::kVertexIndex, ast::PipelineStage::kFragment, false),
    ParamsFor<u32>(core::BuiltinValue::kVertexIndex, ast::PipelineStage::kCompute, false),

    ParamsFor<u32>(core::BuiltinValue::kInstanceIndex, ast::PipelineStage::kVertex, true),
    ParamsFor<u32>(core::BuiltinValue::kInstanceIndex, ast::PipelineStage::kFragment, false),
    ParamsFor<u32>(core::BuiltinValue::kInstanceIndex, ast::PipelineStage::kCompute, false),

    ParamsFor<bool>(core::BuiltinValue::kFrontFacing, ast::PipelineStage::kVertex, false),
    ParamsFor<bool>(core::BuiltinValue::kFrontFacing, ast::PipelineStage::kFragment, true),
    ParamsFor<bool>(core::BuiltinValue::kFrontFacing, ast::PipelineStage::kCompute, false),

    ParamsFor<vec3<u32>>(core::BuiltinValue::kLocalInvocationId,
                         ast::PipelineStage::kVertex,
                         false),
    ParamsFor<vec3<u32>>(core::BuiltinValue::kLocalInvocationId,
                         ast::PipelineStage::kFragment,
                         false),
    ParamsFor<vec3<u32>>(core::BuiltinValue::kLocalInvocationId,
                         ast::PipelineStage::kCompute,
                         true),

    ParamsFor<u32>(core::BuiltinValue::kLocalInvocationIndex, ast::PipelineStage::kVertex, false),
    ParamsFor<u32>(core::BuiltinValue::kLocalInvocationIndex, ast::PipelineStage::kFragment, false),
    ParamsFor<u32>(core::BuiltinValue::kLocalInvocationIndex, ast::PipelineStage::kCompute, true),

    ParamsFor<vec3<u32>>(core::BuiltinValue::kGlobalInvocationId,
                         ast::PipelineStage::kVertex,
                         false),
    ParamsFor<vec3<u32>>(core::BuiltinValue::kGlobalInvocationId,
                         ast::PipelineStage::kFragment,
                         false),
    ParamsFor<vec3<u32>>(core::BuiltinValue::kGlobalInvocationId,
                         ast::PipelineStage::kCompute,
                         true),

    ParamsFor<vec3<u32>>(core::BuiltinValue::kWorkgroupId, ast::PipelineStage::kVertex, false),
    ParamsFor<vec3<u32>>(core::BuiltinValue::kWorkgroupId, ast::PipelineStage::kFragment, false),
    ParamsFor<vec3<u32>>(core::BuiltinValue::kWorkgroupId, ast::PipelineStage::kCompute, true),

    ParamsFor<vec3<u32>>(core::BuiltinValue::kNumWorkgroups, ast::PipelineStage::kVertex, false),
    ParamsFor<vec3<u32>>(core::BuiltinValue::kNumWorkgroups, ast::PipelineStage::kFragment, false),
    ParamsFor<vec3<u32>>(core::BuiltinValue::kNumWorkgroups, ast::PipelineStage::kCompute, true),

    ParamsFor<u32>(core::BuiltinValue::kSampleIndex, ast::PipelineStage::kVertex, false),
    ParamsFor<u32>(core::BuiltinValue::kSampleIndex, ast::PipelineStage::kFragment, true),
    ParamsFor<u32>(core::BuiltinValue::kSampleIndex, ast::PipelineStage::kCompute, false),

    ParamsFor<u32>(core::BuiltinValue::kSampleMask, ast::PipelineStage::kVertex, false),
    ParamsFor<u32>(core::BuiltinValue::kSampleMask, ast::PipelineStage::kFragment, true),
    ParamsFor<u32>(core::BuiltinValue::kSampleMask, ast::PipelineStage::kCompute, false),
};

using ResolverBuiltinsStageTest = ResolverTestWithParam<Params>;
TEST_P(ResolverBuiltinsStageTest, All_input) {
    const Params& params = GetParam();

    auto* p = GlobalVar("p", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    auto* input =
        Param("input", params.type(*this), Vector{Builtin(Source{{12, 34}}, params.builtin)});
    switch (params.stage) {
        case ast::PipelineStage::kVertex:
            Func("main", Vector{input}, ty.vec4<f32>(), Vector{Return(p)},
                 Vector{Stage(ast::PipelineStage::kVertex)},
                 Vector{
                     Builtin(Source{{12, 34}}, core::BuiltinValue::kPosition),
                 });
            break;
        case ast::PipelineStage::kFragment:
            Func("main", Vector{input}, ty.void_(), tint::Empty,
                 Vector{
                     Stage(ast::PipelineStage::kFragment),
                 },
                 {});
            break;
        case ast::PipelineStage::kCompute:
            Func("main", Vector{input}, ty.void_(), tint::Empty,
                 Vector{
                     Stage(ast::PipelineStage::kCompute),
                     WorkgroupSize(1_i),
                 });
            break;
        default:
            break;
    }

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        StringStream err;
        err << "12:34 error: '@builtin(" << params.builtin << ")' cannot be used for "
            << params.stage << " shader input";
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), err.str());
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         ResolverBuiltinsStageTest,
                         testing::ValuesIn(cases));

TEST_F(ResolverBuiltinsValidationTest, FragDepthIsInput_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(frag_depth) fd: f32,
    // ) -> @location(0) f32 { return 1.0; }
    Func("fs_main",
         Vector{
             Param("fd", ty.f32(),
                   Vector{
                       Builtin(Source{{12, 34}}, core::BuiltinValue::kFragDepth),
                   }),
         },
         ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: '@builtin(frag_depth)' cannot be used for fragment shader input");
}

TEST_F(ResolverBuiltinsValidationTest, FragDepthIsInputStruct_Fail) {
    // struct MyInputs {
    //   @builtin(frag_depth) ff: f32;
    // };
    // @fragment
    // fn fragShader(arg: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs",
                        Vector{
                            Member("frag_depth", ty.f32(),
                                   Vector{
                                       Builtin(Source{{12, 34}}, core::BuiltinValue::kFragDepth),
                                   }),
                        });

    Func("fragShader",
         Vector{
             Param("arg", ty.Of(s)),
         },
         ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: '@builtin(frag_depth)' cannot be used for fragment shader input
note: while analyzing entry point 'fragShader')");
}

TEST_F(ResolverBuiltinsValidationTest, StructBuiltinInsideEntryPoint_Ignored) {
    // struct S {
    //   @builtin(vertex_index) idx: u32;
    // };
    // @fragment
    // fn fragShader() { var s : S; }

    Structure("S", Vector{
                       Member("idx", ty.u32(),
                              Vector{
                                  Builtin(core::BuiltinValue::kVertexIndex),
                              }),
                   });

    Func("fragShader", tint::Empty, ty.void_(), Vector{Decl(Var("s", ty("S")))},
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve());
}

}  // namespace StageTest

TEST_F(ResolverBuiltinsValidationTest, PositionNotF32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(kPosition) p: vec4<u32>;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs",
                        Vector{
                            Member("position", ty.vec4<u32>(),
                                   Vector{
                                       Builtin(Source{{12, 34}}, core::BuiltinValue::kPosition),
                                   }),
                        });
    Func("fragShader",
         Vector{
             Param("arg", ty.Of(s)),
         },
         ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(position)' must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, PositionNotF32_ReturnType_Fail) {
    // @vertex
    // fn main() -> @builtin(position) f32 { return 1.0; }
    Func("main", tint::Empty, ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{
             Builtin(Source{{12, 34}}, core::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(position)' must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, PositionIsVec4h_Fail) {
    // @vertex
    // fn main() -> @builtin(position) vec4h { return vec4h(); }
    Enable(wgsl::Extension::kF16);
    Func("main", tint::Empty, ty.vec4<f16>(),
         Vector{
             Return(Call(ty.vec4<f16>())),
         },
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{
             Builtin(Source{{12, 34}}, core::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(position)' must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, FragDepthNotF32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(kFragDepth) p: i32;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs",
                        Vector{
                            Member("frag_depth", ty.i32(),
                                   Vector{
                                       Builtin(Source{{12, 34}}, core::BuiltinValue::kFragDepth),
                                   }),
                        });
    Func("fragShader", Vector{Param("arg", ty.Of(s))}, ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(frag_depth)' must be 'f32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleMaskNotU32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(sample_mask) m: f32;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs",
                        Vector{
                            Member("m", ty.f32(),
                                   Vector{
                                       Builtin(Source{{12, 34}}, core::BuiltinValue::kSampleMask),
                                   }),
                        });
    Func("fragShader", Vector{Param("arg", ty.Of(s))}, ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(sample_mask)' must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleMaskNotU32_ReturnType_Fail) {
    // @fragment
    // fn main() -> @builtin(sample_mask) i32 { return 1; }
    Func("main", tint::Empty, ty.i32(), Vector{Return(1_i)},
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Builtin(Source{{12, 34}}, core::BuiltinValue::kSampleMask),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(sample_mask)' must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleMaskIsNotU32_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(sample_mask) arg: bool
    // ) -> @location(0) f32 { return 1.0; }
    Func("fs_main",
         Vector{
             Param("arg", ty.bool_(),
                   Vector{
                       Builtin(Source{{12, 34}}, core::BuiltinValue::kSampleMask),
                   }),
         },
         ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(sample_mask)' must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleIndexIsNotU32_Struct_Fail) {
    // struct MyInputs {
    //   @builtin(sample_index) m: f32;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs",
                        Vector{
                            Member("m", ty.f32(),
                                   Vector{
                                       Builtin(Source{{12, 34}}, core::BuiltinValue::kSampleIndex),
                                   }),
                        });
    Func("fragShader", Vector{Param("arg", ty.Of(s))}, ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(sample_index)' must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, SampleIndexIsNotU32_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(sample_index) arg: bool
    // ) -> @location(0) f32 { return 1.0; }
    Func("fs_main",
         Vector{
             Param("arg", ty.bool_(),
                   Vector{
                       Builtin(Source{{12, 34}}, core::BuiltinValue::kSampleIndex),
                   }),
         },
         ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(sample_index)' must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, PositionIsNotF32_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(kPosition) p: vec3<f32>,
    // ) -> @location(0) f32 { return 1.0; }
    Func("fs_main",
         Vector{
             Param("p", ty.vec3<f32>(),
                   Vector{
                       Builtin(Source{{12, 34}}, core::BuiltinValue::kPosition),
                   }),
         },
         ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(position)' must be 'vec4<f32>'");
}

TEST_F(ResolverBuiltinsValidationTest, FragDepthIsNotF32_Fail) {
    // @fragment
    // fn fs_main() -> @builtin(kFragDepth) f32 { var fd: i32; return fd; }
    auto* fd = Var("fd", ty.i32());
    Func("fs_main", tint::Empty, ty.i32(),
         Vector{
             Decl(fd),
             Return(fd),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Builtin(Source{{12, 34}}, core::BuiltinValue::kFragDepth),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(frag_depth)' must be 'f32'");
}

TEST_F(ResolverBuiltinsValidationTest, VertexIndexIsNotU32_Fail) {
    // @vertex
    // fn main(
    //   @builtin(kVertexIndex) vi : f32,
    //   @builtin(kPosition) p :vec4<f32>
    // ) -> @builtin(kPosition) vec4<f32> { return vec4<f32>(); }
    auto* p = Param("p", ty.vec4<f32>(),
                    Vector{
                        Builtin(core::BuiltinValue::kPosition),
                    });
    auto* vi = Param("vi", ty.f32(),
                     Vector{
                         Builtin(Source{{12, 34}}, core::BuiltinValue::kVertexIndex),
                     });
    Func("main", Vector{vi, p}, ty.vec4<f32>(), Vector{Return(Expr("p"))},
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(vertex_index)' must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, InstanceIndexIsNotU32) {
    // @vertex
    // fn main(
    //   @builtin(kInstanceIndex) ii : f32,
    //   @builtin(kPosition) p :vec4<f32>
    // ) -> @builtin(kPosition) vec4<f32> { return vec4<f32>(); }
    auto* p = Param("p", ty.vec4<f32>(),
                    Vector{
                        Builtin(core::BuiltinValue::kPosition),
                    });
    auto* ii = Param("ii", ty.f32(),
                     Vector{
                         Builtin(Source{{12, 34}}, core::BuiltinValue::kInstanceIndex),
                     });
    Func("main", Vector{ii, p}, ty.vec4<f32>(), Vector{Return(Expr("p"))},
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(instance_index)' must be 'u32'");
}

TEST_F(ResolverBuiltinsValidationTest, FragmentBuiltin_Pass) {
    // @fragment
    // fn fs_main(
    //   @builtin(kPosition) p: vec4<f32>,
    //   @builtin(front_facing) ff: bool,
    //   @builtin(sample_index) si: u32,
    //   @builtin(sample_mask) sm : u32
    // ) -> @builtin(frag_depth) f32 { var fd: f32; return fd; }
    auto* p = Param("p", ty.vec4<f32>(),
                    Vector{
                        Builtin(core::BuiltinValue::kPosition),
                    });
    auto* ff = Param("ff", ty.bool_(),
                     Vector{
                         Builtin(core::BuiltinValue::kFrontFacing),
                     });
    auto* si = Param("si", ty.u32(),
                     Vector{
                         Builtin(core::BuiltinValue::kSampleIndex),
                     });
    auto* sm = Param("sm", ty.u32(),
                     Vector{
                         Builtin(core::BuiltinValue::kSampleMask),
                     });
    auto* var_fd = Var("fd", ty.f32());
    Func("fs_main", Vector{p, ff, si, sm}, ty.f32(),
         Vector{
             Decl(var_fd),
             Return(var_fd),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Builtin(core::BuiltinValue::kFragDepth),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, VertexBuiltin_Pass) {
    // @vertex
    // fn main(
    //   @builtin(vertex_index) vi : u32,
    //   @builtin(instance_index) ii : u32,
    // ) -> @builtin(position) vec4<f32> { var p :vec4<f32>; return p; }
    auto* vi = Param("vi", ty.u32(),
                     Vector{
                         Builtin(Source{{12, 34}}, core::BuiltinValue::kVertexIndex),
                     });

    auto* ii = Param("ii", ty.u32(),
                     Vector{
                         Builtin(Source{{12, 34}}, core::BuiltinValue::kInstanceIndex),
                     });
    auto* p = Var("p", ty.vec4<f32>());
    Func("main", Vector{vi, ii}, ty.vec4<f32>(),
         Vector{
             Decl(p),
             Return(p),
         },
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_Pass) {
    // @compute @workgroup_size(1)
    // fn main(
    //   @builtin(local_invocationId) li_id: vec3<u32>,
    //   @builtin(local_invocationIndex) li_index: u32,
    //   @builtin(global_invocationId) gi: vec3<u32>,
    //   @builtin(workgroup_id) wi: vec3<u32>,
    //   @builtin(num_workgroups) nwgs: vec3<u32>,
    // ) {}

    auto* li_id = Param("li_id", ty.vec3<u32>(),
                        Vector{
                            Builtin(core::BuiltinValue::kLocalInvocationId),
                        });
    auto* li_index = Param("li_index", ty.u32(),
                           Vector{
                               Builtin(core::BuiltinValue::kLocalInvocationIndex),
                           });
    auto* gi = Param("gi", ty.vec3<u32>(),
                     Vector{
                         Builtin(core::BuiltinValue::kGlobalInvocationId),
                     });
    auto* wi = Param("wi", ty.vec3<u32>(),
                     Vector{
                         Builtin(core::BuiltinValue::kWorkgroupId),
                     });
    auto* nwgs = Param("nwgs", ty.vec3<u32>(),
                       Vector{
                           Builtin(core::BuiltinValue::kNumWorkgroups),
                       });

    Func("main", Vector{li_id, li_index, gi, wi, nwgs}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kCompute),
                WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_WorkGroupIdNotVec3U32) {
    auto* wi = Param("wi", ty.f32(),
                     Vector{
                         Builtin(Source{{12, 34}}, core::BuiltinValue::kWorkgroupId),
                     });
    Func("main", Vector{wi}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kCompute),
                WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: store type of '@builtin(workgroup_id)' must be 'vec3<u32>')");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_NumWorkgroupsNotVec3U32) {
    auto* nwgs = Param("nwgs", ty.f32(),
                       Vector{
                           Builtin(Source{{12, 34}}, core::BuiltinValue::kNumWorkgroups),
                       });
    Func("main", Vector{nwgs}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kCompute),
                WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: store type of '@builtin(num_workgroups)' must be 'vec3<u32>')");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_GlobalInvocationNotVec3U32) {
    auto* gi = Param("gi", ty.vec3<i32>(),
                     Vector{
                         Builtin(Source{{12, 34}}, core::BuiltinValue::kGlobalInvocationId),
                     });
    Func("main", Vector{gi}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kCompute),
                WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: store type of '@builtin(global_invocation_id)' must be 'vec3<u32>')");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_LocalInvocationIndexNotU32) {
    auto* li_index = Param("li_index", ty.vec3<u32>(),
                           Vector{
                               Builtin(Source{{12, 34}}, core::BuiltinValue::kLocalInvocationIndex),
                           });
    Func("main", Vector{li_index}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kCompute),
                WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: store type of '@builtin(local_invocation_index)' must be 'u32')");
}

TEST_F(ResolverBuiltinsValidationTest, ComputeBuiltin_LocalInvocationNotVec3U32) {
    auto* li_id = Param("li_id", ty.vec2<u32>(),
                        Vector{
                            Builtin(Source{{12, 34}}, core::BuiltinValue::kLocalInvocationId),
                        });
    Func("main", Vector{li_id}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kCompute),
                WorkgroupSize(Expr(Source{Source::Location{12, 34}}, 2_i))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: store type of '@builtin(local_invocation_id)' must be 'vec3<u32>')");
}

TEST_F(ResolverBuiltinsValidationTest, FragmentBuiltinStruct_Pass) {
    // Struct MyInputs {
    //   @builtin(kPosition) p: vec4<f32>;
    //   @builtin(frag_depth) fd: f32;
    //   @builtin(sample_index) si: u32;
    //   @builtin(sample_mask) sm : u32;;
    // };
    // @fragment
    // fn fragShader(arg: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs", Vector{
                                        Member("position", ty.vec4<f32>(),
                                               Vector{
                                                   Builtin(core::BuiltinValue::kPosition),
                                               }),
                                        Member("front_facing", ty.bool_(),
                                               Vector{
                                                   Builtin(core::BuiltinValue::kFrontFacing),
                                               }),
                                        Member("sample_index", ty.u32(),
                                               Vector{
                                                   Builtin(core::BuiltinValue::kSampleIndex),
                                               }),
                                        Member("sample_mask", ty.u32(),
                                               Vector{
                                                   Builtin(core::BuiltinValue::kSampleMask),
                                               }),
                                    });
    Func("fragShader", Vector{Param("arg", ty.Of(s))}, ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, FrontFacingParamIsNotBool_Fail) {
    // @fragment
    // fn fs_main(
    //   @builtin(front_facing) is_front: i32;
    // ) -> @location(0) f32 { return 1.0; }

    auto* is_front = Param("is_front", ty.i32(),
                           Vector{
                               Builtin(Source{{12, 34}}, core::BuiltinValue::kFrontFacing),
                           });
    Func("fs_main", Vector{is_front}, ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(front_facing)' must be 'bool'");
}

TEST_F(ResolverBuiltinsValidationTest, FrontFacingMemberIsNotBool_Fail) {
    // struct MyInputs {
    //   @builtin(front_facing) pos: f32;
    // };
    // @fragment
    // fn fragShader(is_front: MyInputs) -> @location(0) f32 { return 1.0; }

    auto* s = Structure("MyInputs",
                        Vector{
                            Member("pos", ty.f32(),
                                   Vector{
                                       Builtin(Source{{12, 34}}, core::BuiltinValue::kFrontFacing),
                                   }),
                        });
    Func("fragShader", Vector{Param("is_front", ty.Of(s))}, ty.f32(),
         Vector{
             Return(1_f),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: store type of '@builtin(front_facing)' must be 'bool'");
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Scalar) {
    auto* builtin = Call("length", 1_f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec2) {
    auto* builtin = Call("length", Call<vec2<f32>>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec3) {
    auto* builtin = Call("length", Call<vec3<f32>>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Length_Float_Vec4) {
    auto* builtin = Call("length", Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Scalar) {
    auto* builtin = Call("distance", 1_f, 1_f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec2) {
    auto* builtin = Call("distance", Call<vec2<f32>>(1_f, 1_f), Call<vec2<f32>>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec3) {
    auto* builtin =
        Call("distance", Call<vec3<f32>>(1_f, 1_f, 1_f), Call<vec3<f32>>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Distance_Float_Vec4) {
    auto* builtin =
        Call("distance", Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f), Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat2x2) {
    auto* builtin = Call("determinant",
                         Call<mat2x2<f32>>(Call<vec2<f32>>(1_f, 1_f), Call<vec2<f32>>(1_f, 1_f)));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat3x3) {
    auto* builtin = Call("determinant", Call<mat3x3<f32>>(Call<vec3<f32>>(1_f, 1_f, 1_f),
                                                          Call<vec3<f32>>(1_f, 1_f, 1_f),
                                                          Call<vec3<f32>>(1_f, 1_f, 1_f)));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Determinant_Mat4x4) {
    auto* builtin = Call("determinant", Call<mat4x4<f32>>(Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f),
                                                          Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f),
                                                          Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f),
                                                          Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f)));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Scalar) {
    auto* builtin = Call("frexp", 1_f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<core::type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    EXPECT_TRUE(members[0]->Type()->Is<core::type::F32>());
    EXPECT_TRUE(members[1]->Type()->Is<core::type::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec2) {
    auto* builtin = Call("frexp", Call<vec2<f32>>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<core::type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<core::type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<core::type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<core::type::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[0]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(members[1]->Type()->As<core::type::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[1]->Type()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec3) {
    auto* builtin = Call("frexp", Call<vec3<f32>>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<core::type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<core::type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<core::type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[0]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(members[1]->Type()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[1]->Type()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Frexp_Vec4) {
    auto* builtin = Call("frexp", Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<core::type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<core::type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<core::type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<core::type::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[0]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(members[1]->Type()->As<core::type::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[1]->Type()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Scalar) {
    auto* builtin = Call("modf", 1_f);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<core::type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    EXPECT_TRUE(members[0]->Type()->Is<core::type::F32>());
    EXPECT_TRUE(members[1]->Type()->Is<core::type::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec2) {
    auto* builtin = Call("modf", Call<vec2<f32>>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<core::type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<core::type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<core::type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<core::type::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[0]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(members[1]->Type()->As<core::type::Vector>()->Width(), 2u);
    EXPECT_TRUE(members[1]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec3) {
    auto* builtin = Call("modf", Call<vec3<f32>>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<core::type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<core::type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<core::type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[0]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(members[1]->Type()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(members[1]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Modf_Vec4) {
    auto* builtin = Call("modf", Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    auto* res_ty = TypeOf(builtin)->As<core::type::Struct>();
    ASSERT_TRUE(res_ty != nullptr);
    auto members = res_ty->Members();
    ASSERT_EQ(members.Length(), 2u);
    ASSERT_TRUE(members[0]->Type()->Is<core::type::Vector>());
    ASSERT_TRUE(members[1]->Type()->Is<core::type::Vector>());
    EXPECT_EQ(members[0]->Type()->As<core::type::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[0]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
    EXPECT_EQ(members[1]->Type()->As<core::type::Vector>()->Width(), 4u);
    EXPECT_TRUE(members[1]->Type()->As<core::type::Vector>()->Type()->Is<core::type::F32>());
}

TEST_F(ResolverBuiltinsValidationTest, Cross_Float_Vec3) {
    auto* builtin = Call("cross", Call<vec3<f32>>(1_f, 1_f, 1_f), Call<vec3<f32>>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec2) {
    auto* builtin = Call("dot", Call<vec2<f32>>(1_f, 1_f), Call<vec2<f32>>(1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec3) {
    auto* builtin = Call("dot", Call<vec3<f32>>(1_f, 1_f, 1_f), Call<vec3<f32>>(1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Dot_Float_Vec4) {
    auto* builtin =
        Call("dot", Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f), Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Float_Scalar) {
    auto* builtin = Call("select", Expr(1_f), Expr(1_f), Expr(true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Integer_Scalar) {
    auto* builtin = Call("select", Expr(1_i), Expr(1_i), Expr(true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Boolean_Scalar) {
    auto* builtin = Call("select", Expr(true), Expr(true), Expr(true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Float_Vec2) {
    auto* builtin = Call("select", Call<vec2<f32>>(1_f, 1_f), Call<vec2<f32>>(1_f, 1_f),
                         Call<vec2<bool>>(true, true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Integer_Vec2) {
    auto* builtin = Call("select", Call<vec2<i32>>(1_i, 1_i), Call<vec2<i32>>(1_i, 1_i),
                         Call<vec2<bool>>(true, true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinsValidationTest, Select_Boolean_Vec2) {
    auto* builtin = Call("select", Call<vec2<bool>>(true, true), Call<vec2<bool>>(true, true),
                         Call<vec2<bool>>(true, true));
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

template <typename T>
class ResolverBuiltinsValidationTestWithParams : public resolver::TestHelper,
                                                 public testing::TestWithParam<T> {};

using FloatAllMatching =
    ResolverBuiltinsValidationTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(FloatAllMatching, Scalar) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Expr(f32(i + 1)));
    }
    auto* builtin = Call(name, params);
    Func("func", tint::Empty, ty.void_(),
         Vector{
             Assign(Phony(), builtin),
         },
         Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<core::type::F32>());
}

TEST_P(FloatAllMatching, Vec2) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec2<f32>>(f32(i + 1), f32(i + 1)));
    }
    auto* builtin = Call(name, params);
    Func("func", tint::Empty, ty.void_(),
         Vector{
             Assign(Phony(), builtin),
         },
         Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->IsFloatVector());
}

TEST_P(FloatAllMatching, Vec3) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec3<f32>>(f32(i + 1), f32(i + 1), f32(i + 1)));
    }
    auto* builtin = Call(name, params);
    Func("func", tint::Empty, ty.void_(),
         Vector{
             Assign(Phony(), builtin),
         },
         Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->IsFloatVector());
}

TEST_P(FloatAllMatching, Vec4) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec4<f32>>(f32(i + 1), f32(i + 1), f32(i + 1), f32(i + 1)));
    }
    auto* builtin = Call(name, params);
    Func("func", tint::Empty, ty.void_(),
         Vector{
             Assign(Phony(), builtin),
         },
         Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->IsFloatVector());
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         FloatAllMatching,
                         ::testing::Values(std::make_tuple("abs", 1),
                                           std::make_tuple("acos", 1),
                                           std::make_tuple("asin", 1),
                                           std::make_tuple("atan", 1),
                                           std::make_tuple("atan2", 2),
                                           std::make_tuple("ceil", 1),
                                           std::make_tuple("clamp", 3),
                                           std::make_tuple("cos", 1),
                                           std::make_tuple("cosh", 1),
                                           std::make_tuple("dpdx", 1),
                                           std::make_tuple("dpdxCoarse", 1),
                                           std::make_tuple("dpdxFine", 1),
                                           std::make_tuple("dpdy", 1),
                                           std::make_tuple("dpdyCoarse", 1),
                                           std::make_tuple("dpdyFine", 1),
                                           std::make_tuple("exp", 1),
                                           std::make_tuple("exp2", 1),
                                           std::make_tuple("floor", 1),
                                           std::make_tuple("fma", 3),
                                           std::make_tuple("fract", 1),
                                           std::make_tuple("fwidth", 1),
                                           std::make_tuple("fwidthCoarse", 1),
                                           std::make_tuple("fwidthFine", 1),
                                           std::make_tuple("inverseSqrt", 1),
                                           std::make_tuple("log", 1),
                                           std::make_tuple("log2", 1),
                                           std::make_tuple("max", 2),
                                           std::make_tuple("min", 2),
                                           std::make_tuple("mix", 3),
                                           std::make_tuple("pow", 2),
                                           std::make_tuple("round", 1),
                                           std::make_tuple("sign", 1),
                                           std::make_tuple("sin", 1),
                                           std::make_tuple("sinh", 1),
                                           std::make_tuple("smoothstep", 3),
                                           std::make_tuple("sqrt", 1),
                                           std::make_tuple("step", 2),
                                           std::make_tuple("tan", 1),
                                           std::make_tuple("tanh", 1),
                                           std::make_tuple("trunc", 1)));

using IntegerAllMatching =
    ResolverBuiltinsValidationTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(IntegerAllMatching, ScalarUnsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<u32>(1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<core::type::U32>());
}

TEST_P(IntegerAllMatching, Vec2Unsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec2<u32>>(1_u, 1_u));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->IsUnsignedIntegerVector());
}

TEST_P(IntegerAllMatching, Vec3Unsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec3<u32>>(1_u, 1_u, 1_u));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->IsUnsignedIntegerVector());
}

TEST_P(IntegerAllMatching, Vec4Unsigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec4<u32>>(1_u, 1_u, 1_u, 1_u));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->IsUnsignedIntegerVector());
}

TEST_P(IntegerAllMatching, ScalarSigned) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<i32>(1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->Is<core::type::I32>());
}

TEST_P(IntegerAllMatching, Vec2Signed) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec2<i32>>(1_i, 1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->IsSignedIntegerVector());
}

TEST_P(IntegerAllMatching, Vec3Signed) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec3<i32>>(1_i, 1_i, 1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->IsSignedIntegerVector());
}

TEST_P(IntegerAllMatching, Vec4Signed) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec4<i32>>(1_i, 1_i, 1_i, 1_i));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_TRUE(TypeOf(builtin)->IsSignedIntegerVector());
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         IntegerAllMatching,
                         ::testing::Values(std::make_tuple("abs", 1),
                                           std::make_tuple("clamp", 3),
                                           std::make_tuple("countOneBits", 1),
                                           std::make_tuple("max", 2),
                                           std::make_tuple("min", 2),
                                           std::make_tuple("reverseBits", 1)));

using BooleanVectorInput =
    ResolverBuiltinsValidationTestWithParams<std::tuple<std::string, uint32_t>>;

TEST_P(BooleanVectorInput, Vec2) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec2<bool>>(true, true));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(BooleanVectorInput, Vec3) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec3<bool>>(true, true, true));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(BooleanVectorInput, Vec4) {
    std::string name = std::get<0>(GetParam());
    uint32_t num_params = std::get<1>(GetParam());

    Vector<const ast::Expression*, 8> params;
    for (uint32_t i = 0; i < num_params; ++i) {
        params.Push(Call<vec4<bool>>(true, true, true, true));
    }
    auto* builtin = Call(name, params);
    WrapInFunction(builtin);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         BooleanVectorInput,
                         ::testing::Values(std::make_tuple("all", 1), std::make_tuple("any", 1)));

using DataPacking4x8 = ResolverBuiltinsValidationTestWithParams<std::string>;

TEST_P(DataPacking4x8, Float_Vec4) {
    auto name = GetParam();
    auto* builtin = Call(name, Call<vec4<f32>>(1_f, 1_f, 1_f, 1_f));
    WrapInFunction(builtin);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         DataPacking4x8,
                         ::testing::Values("pack4x8snorm", "pack4x8unorm"));

using DataPacking2x16 = ResolverBuiltinsValidationTestWithParams<std::string>;

TEST_P(DataPacking2x16, Float_Vec2) {
    auto name = GetParam();
    auto* builtin = Call(name, Call<vec2<f32>>(1_f, 1_f));
    WrapInFunction(builtin);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverBuiltinsValidationTest,
                         DataPacking2x16,
                         ::testing::Values("pack2x16snorm", "pack2x16unorm", "pack2x16float"));

using ExprMaker = std::function<const ast::Expression*(ast::Builder*)>;
template <typename T>
ExprMaker Mk(core::Number<T> v) {
    return [v](ast::Builder* b) -> const ast::Expression* { return b->Expr(v); };
}

// We''ll construct cases like this:
// fn foo() {
//   var s: STYPE;
//   _ = clamp(s, LOW, HIGH);
// }
struct ClampPartialConstCase {
    builder::ast_type_func_ptr sType;
    ExprMaker makeLow;
    ExprMaker makeHigh;
    bool expectPass = true;
    std::string lowStr = "";
    std::string highStr = "";
};

using ClampPartialConst =
    ResolverBuiltinsValidationTestWithParams<std::tuple<ClampPartialConstCase, bool, bool>>;

TEST_P(ClampPartialConst, Scalar) {
    auto [params, firstConst, secondConst] = GetParam();
    auto sTy = params.sType(*this);
    const ast::Expression* low = params.makeLow(this);
    const ast::Expression* high = params.makeHigh(this);

    const ast::Variable* lowDecl;
    if (firstConst) {
        lowDecl = Const("low", low);
    } else {
        lowDecl = Var("low", low);
    }
    const ast::Variable* highDecl;
    if (secondConst) {
        highDecl = Const("high", high);
    } else {
        highDecl = Var("high", high);
    }
    WrapInFunction(Var("s", sTy), lowDecl, highDecl,
                   Ignore(Call(Source{{12, 34}}, "clamp", "s", "low", "high")));

    const auto expectPass = params.expectPass || !(firstConst && secondConst);
    if (expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        StringStream ss;
        ss << "12:34 error: clamp called with 'low' (" << params.lowStr << ") greater than 'high' ("
           << params.highStr << ")";
        auto expect = ss.str();
        EXPECT_EQ(r()->error(), expect);
    }
}

TEST_P(ClampPartialConst, Vector) {
    auto [params, firstConst, secondConst] = GetParam();
    auto sTy = params.sType(*this);
    const ast::Expression* low = params.makeLow(this);
    const ast::Expression* high = params.makeHigh(this);

    const ast::Variable* lowDecl;
    if (firstConst) {
        lowDecl = Const("low", low);
    } else {
        lowDecl = Var("low", low);
    }
    const ast::Variable* highDecl;
    if (secondConst) {
        highDecl = Const("high", high);
    } else {
        highDecl = Var("high", high);
    }
    WrapInFunction(Var("s", sTy), lowDecl, highDecl,
                   Ignore(Call(Source{{12, 34}}, "clamp", Call(Ident("vec3"), "s", "s", "s"),
                               Call(Ident("vec3"), Expr(0_a), "low", Expr(0_a)),
                               Call(Ident("vec3"), Expr(1_a), "high", Expr(1_a)))));

    const auto expectPass = params.expectPass || !(firstConst && secondConst);
    if (expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        StringStream ss;
        ss << "12:34 error: clamp called with 'low' (" << params.lowStr << ") greater than 'high' ("
           << params.highStr << ")";
        auto expect = ss.str();
        EXPECT_EQ(r()->error(), expect);
    }
}

TEST_P(ClampPartialConst, VectorMixedRuntimeConstNotChecked) {
    auto [params, firstConst, secondConst] = GetParam();
    auto sTy = params.sType(*this);
    const ast::Expression* low = params.makeLow(this);
    const ast::Expression* high = params.makeHigh(this);
    // Some components of the low and high vector are runtime, so the other
    // components are not checked, and therefore do not generate errors.
    WrapInFunction(Var("s", sTy),
                   Ignore(Call(Source{{12, 34}}, "clamp", Call(Ident("vec2"), "s", "s"),
                               Call(Ident("vec2"), "s", low), Call(Ident("vec2"), "s", high))));

    EXPECT_TRUE(r()->Resolve());
}

std::vector<ClampPartialConstCase> clampCases() {
    return std::vector<ClampPartialConstCase>{
        // Simple passing cases.
        {DataType<f32>::AST, Mk(1_a), Mk(1_a), true},    // low == high
        {DataType<f32>::AST, Mk(1_a), Mk(2_a), true},    // low < high
        {DataType<f32>::AST, Mk(1.0_a), Mk(1_a), true},  // AFloat AInt
        {DataType<f32>::AST, Mk(1_a), Mk(2_f), true},    // AInt AFloat
        {DataType<f32>::AST, Mk(1_a), Mk(2_f), true},    // AFloat f32
        {DataType<f32>::AST, Mk(1_f), Mk(2.0_a), true},  // f32 AFloat
        {DataType<f32>::AST, Mk(1_f), Mk(2_f), true},    // f32 f32
        {DataType<u32>::AST, Mk(1_a), Mk(1_u), true},    // AInt u32
        {DataType<u32>::AST, Mk(1_u), Mk(2_u), true},    // u32 u32
        {DataType<i32>::AST, Mk(1_i), Mk(1_a), true},    // i32 AInt
        {DataType<i32>::AST, Mk(1_i), Mk(2_i), true},    // i32 i32

        // AInt AInt
        {DataType<f32>::AST, Mk(1_a), Mk(0_a), false, "1.0", "0.0"},
        {DataType<i32>::AST, Mk(1_a), Mk(0_a), false, "1", "0"},
        {DataType<u32>::AST, Mk(1_a), Mk(0_a), false, "1", "0"},
        // AFloat AInt
        {DataType<f32>::AST, Mk(1.0_a), Mk(0_a), false, "1.0", "0.0"},
        // AInt AFloat
        {DataType<f32>::AST, Mk(1_a), Mk(0.0_a), false, "1.0", "0.0"},

        // AFloat AFloat
        {DataType<f32>::AST, Mk(1.0_a), Mk(0.0_a), false, "1.0", "0.0"},
        // AFloat f32
        {DataType<f32>::AST, Mk(1.0_a), Mk(0_f), false, "1.0", "0.0"},
        // f32 AFloat
        {DataType<f32>::AST, Mk(1_f), Mk(0.0_a), false, "1.0", "0.0"},

        //  AInt f32
        {DataType<f32>::AST, Mk(1_a), Mk(0_f), false, "1.0", "0.0"},
        //  f32 AInt
        {DataType<f32>::AST, Mk(1_f), Mk(0_a), false, "1.0", "0.0"},
        //  f32 f32
        {DataType<f32>::AST, Mk(1_f), Mk(0_f), false, "1.0", "0.0"},

        //  AInt i32
        {DataType<i32>::AST, Mk(1_a), Mk(0_i), false, "1", "0"},
        //  i32 AInt
        {DataType<i32>::AST, Mk(1_i), Mk(0_a), false, "1", "0"},
        //  i32 i32
        {DataType<i32>::AST, Mk(1_i), Mk(0_i), false, "1", "0"},

        //  AInt u32
        {DataType<u32>::AST, Mk(1_a), Mk(0_u), false, "1", "0"},
        //  i32 AInt
        {DataType<u32>::AST, Mk(1_u), Mk(0_a), false, "1", "0"},
        //  i32 i32
        {DataType<u32>::AST, Mk(1_u), Mk(0_u), false, "1", "0"},
    };
}

INSTANTIATE_TEST_SUITE_P(Clamp,
                         ClampPartialConst,
                         ::testing::Combine(::testing::ValuesIn(clampCases()),
                                            ::testing::ValuesIn({true}),
                                            ::testing::ValuesIn({true})));

// We''ll construct cases like this:
// fn foo() {
//   var s: STYPE;
//   _ = smoothstep(LOW, HIGH, s);
// }
struct SmoothstepPartialConstCase {
    builder::ast_type_func_ptr sType;
    ExprMaker makeLow;
    ExprMaker makeHigh;
    bool expectPass = true;
    std::string lowStr = "";
    std::string highStr = "";
};

using SmoothstepPartialConst =
    ResolverBuiltinsValidationTestWithParams<std::tuple<SmoothstepPartialConstCase, bool, bool>>;

TEST_P(SmoothstepPartialConst, Scalar) {
    auto [params, firstConst, secondConst] = GetParam();
    auto sTy = params.sType(*this);
    const ast::Expression* low = params.makeLow(this);
    const ast::Expression* high = params.makeHigh(this);

    const ast::Variable* lowDecl;
    if (firstConst) {
        lowDecl = Const("low", low);
    } else {
        lowDecl = Var("low", low);
    }
    const ast::Variable* highDecl;
    if (secondConst) {
        highDecl = Const("high", high);
    } else {
        highDecl = Var("high", high);
    }
    WrapInFunction(Var("s", sTy), lowDecl, highDecl,
                   Ignore(Call(Source{{12, 34}}, "smoothstep", "low", "high", "s")));

    const auto expectPass = params.expectPass || !(firstConst && secondConst);
    if (expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        StringStream ss;
        ss << "12:34 error: smoothstep called with 'low' (" << params.lowStr
           << ") equal to 'high' (" << params.highStr << ")";
        auto expect = ss.str();
        EXPECT_EQ(r()->error(), expect);
    }
}

TEST_P(SmoothstepPartialConst, Vector) {
    auto [params, firstConst, secondConst] = GetParam();
    auto sTy = params.sType(*this);
    const ast::Expression* low = params.makeLow(this);
    const ast::Expression* high = params.makeHigh(this);

    const ast::Variable* lowDecl;
    if (firstConst) {
        lowDecl = Const("low", low);
    } else {
        lowDecl = Var("low", low);
    }
    const ast::Variable* highDecl;
    if (secondConst) {
        highDecl = Const("high", high);
    } else {
        highDecl = Var("high", high);
    }
    WrapInFunction(Var("s", sTy), lowDecl, highDecl,
                   Ignore(Call(Source{{12, 34}}, "smoothstep",
                               Call(Ident("vec3"), Expr(0_a), "low", Expr(0_a)),
                               Call(Ident("vec3"), Expr(1_a), "high", Expr(1_a)),
                               Call(Ident("vec3"), "s", "s", "s"))));

    const auto expectPass = params.expectPass || !(firstConst && secondConst);
    if (expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        StringStream ss;
        ss << "12:34 error: smoothstep called with 'low' (" << params.lowStr
           << ") equal to 'high' (" << params.highStr << ")";
        auto expect = ss.str();
        EXPECT_EQ(r()->error(), expect);
    }
}

TEST_P(SmoothstepPartialConst, VectorMixedRuntimeConstNotChecked) {
    auto [params, firstConst, secondConst] = GetParam();
    auto sTy = params.sType(*this);
    const ast::Expression* low = params.makeLow(this);
    const ast::Expression* high = params.makeHigh(this);
    // Some components of the low and high vector are runtime, so the other
    // components are not checked, and therefore do not generate errors.
    WrapInFunction(Var("s", sTy),
                   Ignore(Call(Source{{12, 34}}, "smoothstep", Call(Ident("vec2"), "s", low),
                               Call(Ident("vec2"), "s", high), Call(Ident("vec2"), "s", "s"))));

    EXPECT_TRUE(r()->Resolve());
}

std::vector<SmoothstepPartialConstCase> smoothstepCases() {
    return std::vector<SmoothstepPartialConstCase>{
        // Simple passing cases.
        {DataType<f32>::AST, Mk(1_a), Mk(2_a), true},    // low < high
        {DataType<f32>::AST, Mk(1.0_a), Mk(2_a), true},  // AFloat AInt
        {DataType<f32>::AST, Mk(1_a), Mk(2_f), true},    // AInt AFloat
        {DataType<f32>::AST, Mk(1_a), Mk(2_f), true},    // AFloat f32
        {DataType<f32>::AST, Mk(1_f), Mk(2.0_a), true},  // f32 AFloat
        {DataType<f32>::AST, Mk(1_f), Mk(2_f), true},    // f32 f32

        //  AInt AInt
        {DataType<f32>::AST, Mk(1_a), Mk(1_a), false, "1.0", "1.0"},
        {DataType<f32>::AST, Mk(1_a), Mk(0_a), true, "1.0", "0.0"},
        //  AFloat AInt
        {DataType<f32>::AST, Mk(1.0_a), Mk(1_a), false, "1.0", "1.0"},
        {DataType<f32>::AST, Mk(1.0_a), Mk(0_a), true, "1.0", "0.0"},
        //  AInt AFloat
        {DataType<f32>::AST, Mk(1_a), Mk(1.0_a), false, "1.0", "1.0"},
        {DataType<f32>::AST, Mk(1_a), Mk(0.0_a), true, "1.0", "0.0"},

        //  AFloat AFloat
        {DataType<f32>::AST, Mk(1.0_a), Mk(1.0_a), false, "1.0", "1.0"},
        {DataType<f32>::AST, Mk(1.0_a), Mk(0.0_a), true, "1.0", "0.0"},

        //  AInt f32
        {DataType<f32>::AST, Mk(1_a), Mk(1_f), false, "1.0", "1.0"},
        {DataType<f32>::AST, Mk(1_a), Mk(0_f), true, "1.0", "0.0"},
        //  f32 AInt
        {DataType<f32>::AST, Mk(1_f), Mk(1_a), false, "1.0", "1.0"},
        {DataType<f32>::AST, Mk(1_f), Mk(0_a), true, "1.0", "0.0"},

        //  AFloat f32
        {DataType<f32>::AST, Mk(1.0_a), Mk(1_f), false, "1.0", "1.0"},
        {DataType<f32>::AST, Mk(1.0_a), Mk(0_f), true, "1.0", "0.0"},

        //  f32 AFloat
        {DataType<f32>::AST, Mk(1_a), Mk(1.0_a), false, "1.0", "1.0"},
        {DataType<f32>::AST, Mk(1_a), Mk(0.0_a), true, "1.0", "0.0"},

        //  f32 f32
        {DataType<f32>::AST, Mk(1_f), Mk(1.0_f), false, "1.0", "1.0"},
        {DataType<f32>::AST, Mk(1_f), Mk(0.0_f), true, "1.0", "0.0"},
    };
}

INSTANTIATE_TEST_SUITE_P(Smoothstep,
                         SmoothstepPartialConst,
                         ::testing::Combine(::testing::ValuesIn(smoothstepCases()),
                                            ::testing::ValuesIn({true}),
                                            ::testing::ValuesIn({true})));

// We'll construct cases like this:
// fn foo() {
//   var e: ETYPE;
//   _ = insertBits(e, e, COUNT, OFFSET);
// }

struct InsertBitsPartialConstCase {
    builder::ast_type_func_ptr eType;
    ExprMaker makeOffset;
    ExprMaker makeCount;
    bool expectPass = true;
    int width = 32;
};

using InsertBitsPartialConst =
    ResolverBuiltinsValidationTestWithParams<std::tuple<InsertBitsPartialConstCase, bool, bool>>;

TEST_P(InsertBitsPartialConst, Scalar) {
    auto [params, firstConst, secondConst] = GetParam();
    auto eTy = params.eType(*this);
    const ast::Expression* offset = params.makeOffset(this);
    const ast::Expression* count = params.makeCount(this);
    const ast::Variable* offsetDecl;
    if (firstConst) {
        offsetDecl = Const("offset", offset);
    } else {
        offsetDecl = Var("offset", offset);
    }
    const ast::Variable* countDecl;
    if (secondConst) {
        countDecl = Const("count", count);
    } else {
        countDecl = Var("count", count);
    }
    WrapInFunction(Var("e", eTy), offsetDecl, countDecl,
                   Ignore(Call(Source{{12, 34}}, "insertBits", "e", "e", "offset", "count")));

    const auto expectPass = params.expectPass || !(firstConst && secondConst);

    if (expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        const std::string expect =
            "12:34 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'";
        EXPECT_EQ(r()->error(), expect);
    }
}

TEST_P(InsertBitsPartialConst, Vector) {
    auto [params, firstConst, secondConst] = GetParam();
    auto eTy = params.eType(*this);
    const ast::Expression* offset = params.makeOffset(this);
    const ast::Expression* count = params.makeCount(this);
    const ast::Variable* offsetDecl;
    if (firstConst) {
        offsetDecl = Const("offset", offset);
    } else {
        offsetDecl = Var("offset", offset);
    }
    const ast::Variable* countDecl;
    if (secondConst) {
        countDecl = Const("count", count);
    } else {
        countDecl = Var("count", count);
    }
    WrapInFunction(Var("e", eTy), offsetDecl, countDecl,            //
                   Ignore(Call(Source{{12, 34}}, "insertBits",      //
                               Call(Ident("vec3"), "e", "e", "e"),  //
                               Call(Ident("vec3"), "e", "e", "e"),  //
                               "offset", "count")));

    const auto expectPass = params.expectPass || !(firstConst && secondConst);

    if (expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        const std::string expect =
            "12:34 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'";
        EXPECT_EQ(r()->error(), expect);
    }
}

std::vector<InsertBitsPartialConstCase> insertBitsCases() {
    return std::vector<InsertBitsPartialConstCase>{
        // Simple passing cases.
        {DataType<u32>::AST, Mk(0_a), Mk(0_a), true},
        {DataType<i32>::AST, Mk(0_a), Mk(0_a), true},
        {DataType<u32>::AST, Mk(16_a), Mk(16_a), true},
        {DataType<i32>::AST, Mk(16_a), Mk(16_a), true},
        {DataType<u32>::AST, Mk(32_a), Mk(0_a), true},
        {DataType<i32>::AST, Mk(32_a), Mk(0_a), true},
        {DataType<u32>::AST, Mk(0_a), Mk(0_a), true},
        {DataType<i32>::AST, Mk(0_a), Mk(0_a), true},
        {DataType<u32>::AST, Mk(32_a), Mk(0_u), true},
        {DataType<i32>::AST, Mk(32_u), Mk(0_a), true},

        //  AInt AInt
        {DataType<u32>::AST, Mk(0_a), Mk(33_a), false},
        {DataType<u32>::AST, Mk(16_a), Mk(17_a), false},
        {DataType<u32>::AST, Mk(33_a), Mk(0_a), false},

        {DataType<i32>::AST, Mk(0_a), Mk(33_u), false},   // Aint u32
        {DataType<i32>::AST, Mk(16_u), Mk(17_a), false},  // u32 AInt
    };
}

INSTANTIATE_TEST_SUITE_P(InsertBits,
                         InsertBitsPartialConst,
                         ::testing::Combine(::testing::ValuesIn(insertBitsCases()),
                                            ::testing::ValuesIn({true}),
                                            ::testing::ValuesIn({true})));

// We'll construct cases like this:
// fn foo() {
//   var e: ETYPE;
//   _ = extractBits(e, COUNT, OFFSET);
// }

struct ExtractBitsPartialConstCase {
    builder::ast_type_func_ptr eType;
    ExprMaker makeOffset;
    ExprMaker makeCount;
    bool expectPass = true;
    int width = 32;
};

using ExtractBitsPartialConst =
    ResolverBuiltinsValidationTestWithParams<std::tuple<ExtractBitsPartialConstCase, bool, bool>>;

TEST_P(ExtractBitsPartialConst, Scalar) {
    auto [params, firstConst, secondConst] = GetParam();
    auto eTy = params.eType(*this);
    const ast::Expression* offset = params.makeOffset(this);
    const ast::Expression* count = params.makeCount(this);
    const ast::Variable* offsetDecl;
    if (firstConst) {
        offsetDecl = Const("offset", offset);
    } else {
        offsetDecl = Var("offset", offset);
    }
    const ast::Variable* countDecl;
    if (secondConst) {
        countDecl = Const("count", count);
    } else {
        countDecl = Var("count", count);
    }
    WrapInFunction(Var("e", eTy), offsetDecl, countDecl,
                   Ignore(Call(Source{{12, 34}}, "extractBits", "e", "offset", "count")));

    const auto expectPass = params.expectPass || !(firstConst && secondConst);

    if (expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        const std::string expect =
            "12:34 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'";
        EXPECT_EQ(r()->error(), expect);
    }
}

TEST_P(ExtractBitsPartialConst, Vector) {
    auto [params, firstConst, secondConst] = GetParam();
    auto eTy = params.eType(*this);
    const ast::Expression* offset = params.makeOffset(this);
    const ast::Expression* count = params.makeCount(this);
    const ast::Variable* offsetDecl;
    if (firstConst) {
        offsetDecl = Const("offset", offset);
    } else {
        offsetDecl = Var("offset", offset);
    }
    const ast::Variable* countDecl;
    if (secondConst) {
        countDecl = Const("count", count);
    } else {
        countDecl = Var("count", count);
    }
    WrapInFunction(Var("e", eTy), offsetDecl, countDecl,            //
                   Ignore(Call(Source{{12, 34}}, "extractBits",     //
                               Call(Ident("vec3"), "e", "e", "e"),  //
                               "offset", "count")));

    const auto expectPass = params.expectPass || !(firstConst && secondConst);

    if (expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        const std::string expect =
            "12:34 error: 'offset' + 'count' must be less than or equal to the bit width of 'e'";
        EXPECT_EQ(r()->error(), expect);
    }
}

std::vector<ExtractBitsPartialConstCase> extractBitsCases() {
    return std::vector<ExtractBitsPartialConstCase>{
        // Simple passing cases.
        {DataType<u32>::AST, Mk(0_a), Mk(0_a), true},
        {DataType<i32>::AST, Mk(0_a), Mk(0_a), true},
        {DataType<u32>::AST, Mk(16_a), Mk(16_a), true},
        {DataType<i32>::AST, Mk(16_a), Mk(16_a), true},
        {DataType<u32>::AST, Mk(32_a), Mk(0_a), true},
        {DataType<i32>::AST, Mk(32_a), Mk(0_a), true},
        {DataType<u32>::AST, Mk(0_a), Mk(0_a), true},
        {DataType<i32>::AST, Mk(0_a), Mk(0_a), true},
        {DataType<u32>::AST, Mk(32_a), Mk(0_u), true},
        {DataType<i32>::AST, Mk(32_u), Mk(0_a), true},

        //  AInt AInt
        {DataType<u32>::AST, Mk(0_a), Mk(33_a), false},
        {DataType<u32>::AST, Mk(16_a), Mk(17_a), false},
        {DataType<u32>::AST, Mk(33_a), Mk(0_a), false},

        {DataType<i32>::AST, Mk(0_a), Mk(33_u), false},   // Aint u32
        {DataType<i32>::AST, Mk(16_u), Mk(17_a), false},  // u32 AInt
    };
}

INSTANTIATE_TEST_SUITE_P(ExtractBits,
                         ExtractBitsPartialConst,
                         ::testing::Combine(::testing::ValuesIn(extractBitsCases()),
                                            ::testing::ValuesIn({true}),
                                            ::testing::ValuesIn({true})));

// We'll construct cases like this:
// fn foo() {
//   var x: XTYPE;
//   _ = ldexp(x, EXPONENT);
// }

struct LdexpPartialConstCase {
    builder::ast_type_func_ptr eType;
    ExprMaker makeExponent;
    bool expectPass = true;
    int highestAllowed = 128;
};

using LdexpPartialConst = ResolverBuiltinsValidationTestWithParams<LdexpPartialConstCase>;

TEST_P(LdexpPartialConst, Scalar) {
    auto params = GetParam();
    auto xTy = params.eType(*this);
    const ast::Expression* exponent = params.makeExponent(this);

    Enable(wgsl::Extension::kF16);
    WrapInFunction(Var("x", xTy), Ignore(Call(Source{{12, 34}}, "ldexp", "x", exponent)));

    if (params.expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        const std::string expect = "12:34 error: e2 must be less than or equal to " +
                                   std::to_string(params.highestAllowed);
        EXPECT_EQ(r()->error(), expect);
    }
}

TEST_P(LdexpPartialConst, Vector) {
    auto params = GetParam();
    auto xTy = params.eType(*this);
    const ast::Expression* exponent = params.makeExponent(this);

    Enable(wgsl::Extension::kF16);
    WrapInFunction(Var("x", xTy),                                   //
                   Ignore(Call(Source{{12, 34}}, "ldexp",           //
                               Call(Ident("vec3"), "x", "x", "x"),  //
                               Call(Ident("vec3"), Expr(0_a), exponent, Expr(1_a)))));

    if (params.expectPass) {
        EXPECT_TRUE(r()->Resolve());
    } else {
        EXPECT_FALSE(r()->Resolve());
        const std::string expect = "12:34 error: e2 must be less than or equal to " +
                                   std::to_string(params.highestAllowed);
        EXPECT_EQ(r()->error(), expect);
    }
}

std::vector<LdexpPartialConstCase> ldexpCases() {
    // Abstract Float cases don't apply here, because if the first parameter
    // is abstract float, then it must be const already.  So that case is
    // already checked by the full const-eval rules.
    return std::vector<LdexpPartialConstCase>{
        // Simple passing cases.
        {DataType<f32>::AST, Mk(128_a), true},
        {DataType<f32>::AST, Mk(128_i), true},
        {DataType<f32>::AST, Mk(-5000_a), true},
        {DataType<f32>::AST, Mk(-5000_i), true},
        {DataType<f16>::AST, Mk(16_a), true},
        {DataType<f16>::AST, Mk(16_i), true},
        {DataType<f16>::AST, Mk(-5000_a), true},
        {DataType<f16>::AST, Mk(-5000_i), true},

        // Failing cases
        {DataType<f32>::AST, Mk(129_a), false, 128},
        {DataType<f32>::AST, Mk(129_i), false, 128},
        {DataType<f32>::AST, Mk(5000_a), false, 128},
        {DataType<f32>::AST, Mk(5000_i), false, 128},
        {DataType<f16>::AST, Mk(17_a), false, 16},
        {DataType<f16>::AST, Mk(17_i), false, 16},
        {DataType<f16>::AST, Mk(5000_a), false, 16},
        {DataType<f16>::AST, Mk(5000_i), false, 16},
    };
}

INSTANTIATE_TEST_SUITE_P(Ldexp, LdexpPartialConst, ::testing::ValuesIn(ldexpCases()));

}  // namespace
}  // namespace tint::resolver
