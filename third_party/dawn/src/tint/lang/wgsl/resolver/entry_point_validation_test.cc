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

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/ast/builtin_attribute.h"
#include "src/tint/lang/wgsl/ast/location_attribute.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T>
using alias = builder::alias<T>;

class ResolverEntryPointValidationTest : public TestHelper, public testing::Test {};

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Location) {
    // @fragment
    // fn main() -> @location(0) f32 { return 1.0; }
    Func(Source{{12, 34}}, "main", tint::Empty, ty.f32(),
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

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Builtin) {
    // @vertex
    // fn main() -> @builtin(position) vec4<f32> { return vec4<f32>(); }
    Func(Source{{12, 34}}, "main", tint::Empty, ty.vec4<f32>(),
         Vector{
             Return(Call<vec4<f32>>()),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Missing) {
    // @vertex
    // fn main() -> f32 {
    //   return 1.0;
    // }
    Func(Source{{12, 34}}, "main", tint::Empty, ty.vec4<f32>(),
         Vector{
             Return(Call<vec4<f32>>()),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: missing entry point IO attribute on return type)");
}

TEST_F(ResolverEntryPointValidationTest, ReturnTypeAttribute_Multiple) {
    // @vertex
    // fn main() -> @location(0) @builtin(position) vec4<f32> {
    //   return vec4<f32>();
    // }
    Func(Source{{12, 34}}, "main", tint::Empty, ty.vec4<f32>(),
         Vector{
             Return(Call<vec4<f32>>()),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{
             Location(Source{{13, 43}}, 0_a),
             Builtin(Source{{14, 52}}, core::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed '@location')");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_Valid) {
    // struct Output {
    //   @location(0) a : f32;
    //   @builtin(frag_depth) b : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure(
        "Output", Vector{
                      Member("a", ty.f32(), Vector{Location(0_a)}),
                      Member("b", ty.f32(), Vector{Builtin(core::BuiltinValue::kFragDepth)}),
                  });
    Func(Source{{12, 34}}, "main", tint::Empty, ty.Of(output),
         Vector{
             Return(Call(ty.Of(output))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_MemberMultipleAttributes) {
    // struct Output {
    //   @location(0) @builtin(frag_depth) a : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure(
        "Output", Vector{
                      Member("a", ty.f32(),
                             Vector{Location(Source{{13, 43}}, 0_a),
                                    Builtin(Source{{14, 52}}, core::BuiltinValue::kFragDepth)}),
                  });
    Func(Source{{12, 34}}, "main", tint::Empty, ty.Of(output),
         Vector{
             Return(Call(ty.Of(output))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed '@location'
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_MemberMissingAttribute) {
    // struct Output {
    //   @location(0) a : f32;
    //   b : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output =
        Structure("Output", Vector{
                                Member(Source{{13, 43}}, "a", ty.f32(), Vector{Location(0_a)}),
                                Member(Source{{14, 52}}, "b", ty.f32(), {}),
                            });
    Func(Source{{12, 34}}, "main", tint::Empty, ty.Of(output),
         Vector{
             Return(Call(ty.Of(output))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(14:52 error: missing entry point IO attribute
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, ReturnType_Struct_DuplicateBuiltins) {
    // struct Output {
    //   @builtin(frag_depth) a : f32;
    //   @builtin(frag_depth) b : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure(
        "Output", Vector{
                      Member("a", ty.f32(), Vector{Builtin(core::BuiltinValue::kFragDepth)}),
                      Member("b", ty.f32(), Vector{Builtin(core::BuiltinValue::kFragDepth)}),
                  });
    Func(Source{{12, 34}}, "main", tint::Empty, ty.Of(output),
         Vector{
             Return(Call(ty.Of(output))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: '@builtin(frag_depth)' appears multiple times as pipeline output
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Location) {
    // @fragment
    // fn main(@location(0) param : f32) {}
    auto* param = Param("param", ty.f32(),
                        Vector{
                            Location(0_a),
                        });
    Func(Source{{12, 34}}, "main",
         Vector{
             param,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Missing) {
    // @fragment
    // fn main(param : f32) {}
    auto* param = Param(Source{{13, 43}}, "param", ty.vec4<f32>());
    Func(Source{{12, 34}}, "main",
         Vector{
             param,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(13:43 error: missing entry point IO attribute on parameter)");
}

TEST_F(ResolverEntryPointValidationTest, ParameterAttribute_Multiple) {
    // @fragment
    // fn main(@location(0) @builtin(sample_index) param : u32) {}
    auto* param = Param("param", ty.u32(),
                        Vector{
                            Location(Source{{13, 43}}, 0_a),
                            Builtin(Source{{14, 52}}, core::BuiltinValue::kSampleIndex),
                        });
    Func(Source{{12, 34}}, "main",
         Vector{
             param,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed '@location')");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_Valid) {
    // struct Input {
    //   @location(0) a : f32;
    //   @builtin(sample_index) b : u32;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* input = Structure(
        "Input", Vector{
                     Member("a", ty.f32(), Vector{Location(0_a)}),
                     Member("b", ty.u32(), Vector{Builtin(core::BuiltinValue::kSampleIndex)}),
                 });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         Vector{
             param,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_MemberMultipleAttributes) {
    // struct Input {
    //   @location(0) @builtin(sample_index) a : u32;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* input = Structure(
        "Input", Vector{
                     Member("a", ty.u32(),
                            Vector{Location(Source{{13, 43}}, 0_a),
                                   Builtin(Source{{14, 52}}, core::BuiltinValue::kSampleIndex)}),
                 });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         Vector{
             param,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: multiple entry point IO attributes
13:43 note: previously consumed '@location'
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_MemberMissingAttribute) {
    // struct Input {
    //   @location(0) a : f32;
    //   b : f32;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* input =
        Structure("Input", Vector{
                               Member(Source{{13, 43}}, "a", ty.f32(), Vector{Location(0_a)}),
                               Member(Source{{14, 52}}, "b", ty.f32(), {}),
                           });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         Vector{
             param,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(14:52 error: missing entry point IO attribute
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_DuplicateBuiltins) {
    // @fragment
    // fn main(@builtin(sample_index) param_a : u32,
    //         @builtin(sample_index) param_b : u32) {}
    auto* param_a = Param("param_a", ty.u32(),
                          Vector{
                              Builtin(core::BuiltinValue::kSampleIndex),
                          });
    auto* param_b = Param("param_b", ty.u32(),
                          Vector{
                              Builtin(core::BuiltinValue::kSampleIndex),
                          });
    Func(Source{{12, 34}}, "main",
         Vector{
             param_a,
             param_b,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: '@builtin(sample_index)' appears multiple times as pipeline input");
}

TEST_F(ResolverEntryPointValidationTest, Parameter_Struct_DuplicateBuiltins) {
    // struct InputA {
    //   @builtin(sample_index) a : u32;
    // };
    // struct InputB {
    //   @builtin(sample_index) a : u32;
    // };
    // @fragment
    // fn main(param_a : InputA, param_b : InputB) {}
    auto* input_a = Structure(
        "InputA", Vector{
                      Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSampleIndex)}),
                  });
    auto* input_b = Structure(
        "InputB", Vector{
                      Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kSampleIndex)}),
                  });
    auto* param_a = Param("param_a", ty.Of(input_a));
    auto* param_b = Param("param_b", ty.Of(input_b));
    Func(Source{{12, 34}}, "main",
         Vector{
             param_a,
             param_b,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: '@builtin(sample_index)' appears multiple times as pipeline input
12:34 note: while analyzing entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, VertexShaderMustReturnPosition) {
    // @vertex
    // fn main() {}
    Func(Source{{12, 34}}, "main", tint::Empty, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: a vertex shader must include the 'position' builtin "
              "in its return type");
}

TEST_F(ResolverEntryPointValidationTest, ImmediateAllowedWithEnable) {
    // enable chromium_experimental_immediate;
    // var<immediate> a : u32;
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar("a", ty.u32(), core::AddressSpace::kImmediate);

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverEntryPointValidationTest, ImmediateDisallowedWithoutEnable) {
    // var<immediate> a : u32;
    GlobalVar(Source{{1, 2}}, "a", ty.u32(), core::AddressSpace::kImmediate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "1:2 error: use of variable address space 'immediate' requires enabling extension "
              "'chromium_experimental_immediate'");
}

TEST_F(ResolverEntryPointValidationTest, ImmediateOneVariableUsedInEntryPoint) {
    // enable chromium_experimental_immediate;
    // var<immediate> a : u32;
    // @compute @workgroup_size(1) fn main() {
    //   _ = a;
    // }
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar("a", ty.u32(), core::AddressSpace::kImmediate);

    Func("main", {}, ty.void_(), Vector{Assign(Phony(), "a")},
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_a)});

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverEntryPointValidationTest, ImmediateTwoVariablesUsedInEntryPoint) {
    // enable chromium_experimental_immediate;
    // var<immediate> a : u32;
    // var<immediate> b : u32;
    // @compute @workgroup_size(1) fn main() {
    //   _ = a;
    //   _ = b;
    // }
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar(Source{{1, 2}}, "a", ty.u32(), core::AddressSpace::kImmediate);
    GlobalVar(Source{{3, 4}}, "b", ty.u32(), core::AddressSpace::kImmediate);

    Func(Source{{5, 6}}, "main", {}, ty.void_(), Vector{Assign(Phony(), "a"), Assign(Phony(), "b")},
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_a)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(5:6 error: entry point 'main' uses two different 'immediate' variables.
3:4 note: first 'immediate' variable declaration is here
1:2 note: second 'immediate' variable declaration is here)");
}

TEST_F(ResolverEntryPointValidationTest, ImmediateTwoVariablesUsedInEntryPointWithFunctionGraph) {
    // enable chromium_experimental_immediate;
    // var<immediate> a : u32;
    // var<immediate> b : u32;
    // fn uses_a() {
    //   _ = a;
    // }
    // fn uses_b() {
    //   _ = b;
    // }
    // @compute @workgroup_size(1) fn main() {
    //   uses_a();
    //   uses_b();
    // }
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar(Source{{1, 2}}, "a", ty.u32(), core::AddressSpace::kImmediate);
    GlobalVar(Source{{3, 4}}, "b", ty.u32(), core::AddressSpace::kImmediate);

    Func(Source{{5, 6}}, "uses_a", {}, ty.void_(), Vector{Assign(Phony(), "a")});
    Func(Source{{7, 8}}, "uses_b", {}, ty.void_(), Vector{Assign(Phony(), "b")});

    Func(Source{{9, 10}}, "main", {}, ty.void_(),
         Vector{CallStmt(Call("uses_a")), CallStmt(Call("uses_b"))},
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_a)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(9:10 error: entry point 'main' uses two different 'immediate' variables.
3:4 note: first 'immediate' variable declaration is here
7:8 note: called by function 'uses_b'
9:10 note: called by entry point 'main'
1:2 note: second 'immediate' variable declaration is here
5:6 note: called by function 'uses_a'
9:10 note: called by entry point 'main')");
}

TEST_F(ResolverEntryPointValidationTest, ImmediateTwoVariablesUsedInDifferentEntryPoint) {
    // enable chromium_experimental_immediate;
    // var<immediate> a : u32;
    // var<immediate> b : u32;
    // @compute @workgroup_size(1) fn uses_a() {
    //   _ = a;
    // }
    // @compute @workgroup_size(1) fn uses_b() {
    //   _ = a;
    // }
    Enable(wgsl::Extension::kChromiumExperimentalImmediate);
    GlobalVar("a", ty.u32(), core::AddressSpace::kImmediate);
    GlobalVar("b", ty.u32(), core::AddressSpace::kImmediate);

    Func("uses_a", {}, ty.void_(), Vector{Assign(Phony(), "a")},
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_a)});
    Func("uses_b", {}, ty.void_(), Vector{Assign(Phony(), "b")},
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_a)});

    EXPECT_TRUE(r()->Resolve());
}

namespace TypeValidationTests {
struct Params {
    builder::ast_type_func_ptr create_ast_type;
    bool is_valid;
};

template <typename T>
constexpr Params ParamsFor(bool is_valid) {
    return Params{DataType<T>::AST, is_valid};
}

using TypeValidationTest = resolver::ResolverTestWithParam<Params>;

static constexpr Params cases[] = {
    ParamsFor<f32>(true),           //
    ParamsFor<i32>(true),           //
    ParamsFor<u32>(true),           //
    ParamsFor<bool>(false),         //
    ParamsFor<vec2<f32>>(true),     //
    ParamsFor<vec3<f32>>(true),     //
    ParamsFor<vec4<f32>>(true),     //
    ParamsFor<mat2x2<f32>>(false),  //
    ParamsFor<mat3x3<f32>>(false),  //
    ParamsFor<mat4x4<f32>>(false),  //
    ParamsFor<alias<f32>>(true),    //
    ParamsFor<alias<i32>>(true),    //
    ParamsFor<alias<u32>>(true),    //
    ParamsFor<alias<bool>>(false),  //
    ParamsFor<f16>(true),           //
    ParamsFor<vec2<f16>>(true),     //
    ParamsFor<vec3<f16>>(true),     //
    ParamsFor<vec4<f16>>(true),     //
    ParamsFor<mat2x2<f16>>(false),  //
    ParamsFor<mat3x3<f16>>(false),  //
    ParamsFor<mat4x4<f16>>(false),  //
    ParamsFor<alias<f16>>(true),    //
};

TEST_P(TypeValidationTest, BareInputs) {
    // @fragment
    // fn main(@location(0) @interpolate(flat) a : *) {}
    auto params = GetParam();

    Enable(wgsl::Extension::kF16);

    auto* a = Param("a", params.create_ast_type(*this),
                    Vector{
                        Location(0_a),
                        Flat(),
                    });
    Func(Source{{12, 34}}, "main",
         Vector{
             a,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
    }
}

TEST_P(TypeValidationTest, StructInputs) {
    // struct Input {
    //   @location(0) @interpolate(flat) a : *;
    // };
    // @fragment
    // fn main(a : Input) {}
    auto params = GetParam();

    Enable(wgsl::Extension::kF16);

    auto* input = Structure(
        "Input", Vector{
                     Member("a", params.create_ast_type(*this), Vector{Location(0_a), Flat()}),
                 });
    auto* a = Param("a", ty.Of(input), {});
    Func(Source{{12, 34}}, "main",
         Vector{
             a,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
    }
}

TEST_P(TypeValidationTest, BareOutputs) {
    // @fragment
    // fn main() -> @location(0) * {
    //   return *();
    // }
    auto params = GetParam();

    Enable(wgsl::Extension::kF16);

    Func(Source{{12, 34}}, "main", tint::Empty, params.create_ast_type(*this),
         Vector{
             Return(Call(params.create_ast_type(*this))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(0_a),
         });

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
    }
}

TEST_P(TypeValidationTest, StructOutputs) {
    // struct Output {
    //   @location(0) a : *;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto params = GetParam();

    Enable(wgsl::Extension::kF16);

    auto* output =
        Structure("Output", Vector{
                                Member("a", params.create_ast_type(*this), Vector{Location(0_a)}),
                            });
    Func(Source{{12, 34}}, "main", tint::Empty, ty.Of(output),
         Vector{
             Return(Call(ty.Of(output))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverEntryPointValidationTest,
                         TypeValidationTest,
                         testing::ValuesIn(cases));

}  // namespace TypeValidationTests

namespace LocationAttributeTests {
namespace {
using LocationAttributeTests = ResolverTest;

TEST_F(LocationAttributeTests, Pass) {
    // @fragment
    // fn frag_main(@location(0) @interpolate(flat) a: i32) {}

    auto* p = Param(Source{{12, 34}}, "a", ty.i32(),
                    Vector{
                        Location(0_a),
                        Flat(),
                    });
    Func("frag_main",
         Vector{
             p,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(LocationAttributeTests, BadType_Input_bool) {
    // @fragment
    // fn frag_main(@location(0) a: bool) {}

    auto* p = Param(Source{{12, 34}}, "a", ty.bool_(),
                    Vector{
                        Location(Source{{34, 56}}, 0_a),
                    });
    Func("frag_main",
         Vector{
             p,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot apply '@location' to declaration of type 'bool'
34:56 note: '@location' must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadType_Output_Array) {
    // @fragment
    // fn frag_main()->@location(0) array<f32, 2> { return array<f32, 2>(); }

    Func(Source{{12, 34}}, "frag_main", tint::Empty, ty.array<f32, 2>(),
         Vector{
             Return(Call<array<f32, 2>>()),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         Vector{
             Location(Source{{34, 56}}, 0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot apply '@location' to declaration of type 'array<f32, 2>'
34:56 note: '@location' must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadType_Input_Struct) {
    // struct Input {
    //   a : f32;
    // };
    // @fragment
    // fn main(@location(0) param : Input) {}
    auto* input = Structure("Input", Vector{
                                         Member("a", ty.f32()),
                                     });
    auto* param = Param(Source{{12, 34}}, "param", ty.Of(input),
                        Vector{
                            Location(Source{{13, 43}}, 0_a),
                        });
    Func(Source{{12, 34}}, "main",
         Vector{
             param,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot apply '@location' to declaration of type 'Input'
13:43 note: '@location' must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadType_Input_Struct_NestedStruct) {
    // struct Inner {
    //   @location(0) b : f32;
    // };
    // struct Input {
    //   a : Inner;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* inner =
        Structure("Inner", Vector{
                               Member(Source{{13, 43}}, "a", ty.f32(), Vector{Location(0_a)}),
                           });
    auto* input = Structure("Input", Vector{
                                         Member(Source{{14, 52}}, "a", ty.Of(inner)),
                                     });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         Vector{
             param,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(14:52 error: nested structures cannot be used for entry point IO
12:34 note: while analyzing entry point 'main')");
}

TEST_F(LocationAttributeTests, BadType_Input_Struct_RuntimeArray) {
    // struct Input {
    //   @location(0) a : array<f32>;
    // };
    // @fragment
    // fn main(param : Input) {}
    auto* input = Structure(
        "Input", Vector{
                     Member(Source{{13, 43}}, "a", ty.array<f32>(), Vector{Location(0_a)}),
                 });
    auto* param = Param("param", ty.Of(input));
    Func(Source{{12, 34}}, "main",
         Vector{
             param,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(13:43 error: cannot apply '@location' to declaration of type 'array<f32>'
note: '@location' must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadMemberType_Input) {
    // struct S { @location(0) m: array<i32>; };
    // @fragment
    // fn frag_main( a: S) {}

    auto* m = Member(Source{{34, 56}}, "m", ty.array<i32>(),
                     Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    auto* s = Structure("S", Vector{m});
    auto* p = Param("a", ty.Of(s));

    Func("frag_main",
         Vector{
             p,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(34:56 error: cannot apply '@location' to declaration of type 'array<i32>'
12:34 note: '@location' must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadMemberType_Output) {
    // struct S { @location(0) m: atomic<i32>; };
    // @fragment
    // fn frag_main() -> S {}
    auto* m = Member(Source{{34, 56}}, "m", ty.atomic<i32>(),
                     Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    auto* s = Structure("S", Vector{m});

    Func("frag_main", tint::Empty, ty.Of(s),
         Vector{
             Return(Call(ty.Of(s))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         },
         {});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(34:56 error: cannot apply '@location' to declaration of type 'atomic<i32>'
12:34 note: '@location' must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, BadMemberType_Unused) {
    // struct S { @location(0) m: mat3x2<f32>; };

    auto* m = Member(Source{{34, 56}}, "m", ty.mat3x2<f32>(),
                     Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    Structure("S", Vector{m});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(34:56 error: cannot apply '@location' to declaration of type 'mat3x2<f32>'
12:34 note: '@location' must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, ReturnType_Struct_Valid) {
    // struct Output {
    //   @location(0) a : f32;
    //   @builtin(frag_depth) b : f32;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure(
        "Output", Vector{
                      Member("a", ty.f32(), Vector{Location(0_a)}),
                      Member("b", ty.f32(), Vector{Builtin(core::BuiltinValue::kFragDepth)}),
                  });
    Func(Source{{12, 34}}, "main", tint::Empty, ty.Of(output),
         Vector{
             Return(Call(ty.Of(output))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(LocationAttributeTests, ReturnType_Struct) {
    // struct Output {
    //   a : f32;
    // };
    // @vertex
    // fn main() -> @location(0) Output {
    //   return Output();
    // }
    auto* output = Structure("Output", Vector{
                                           Member("a", ty.f32()),
                                       });
    Func(Source{{12, 34}}, "main", tint::Empty, ty.Of(output),
         Vector{
             Return(Call(ty.Of(output))),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{
             Location(Source{{13, 43}}, 0_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot apply '@location' to declaration of type 'Output'
13:43 note: '@location' must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, ReturnType_Struct_NestedStruct) {
    // struct Inner {
    //   @location(0) b : f32;
    // };
    // struct Output {
    //   a : Inner;
    // };
    // @fragment
    // fn main() -> Output { return Output(); }
    auto* inner =
        Structure("Inner", Vector{
                               Member(Source{{13, 43}}, "a", ty.f32(), Vector{Location(0_a)}),
                           });
    auto* output = Structure("Output", Vector{
                                           Member(Source{{14, 52}}, "a", ty.Of(inner)),
                                       });
    Func(Source{{12, 34}}, "main", tint::Empty, ty.Of(output),
         Vector{
             Return(Call(ty.Of(output))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(14:52 error: nested structures cannot be used for entry point IO
12:34 note: while analyzing entry point 'main')");
}

TEST_F(LocationAttributeTests, ReturnType_Struct_RuntimeArray) {
    // struct Output {
    //   @location(0) a : array<f32>;
    // };
    // @fragment
    // fn main() -> Output {
    //   return Output();
    // }
    auto* output = Structure("Output", Vector{
                                           Member(Source{{13, 43}}, "a", ty.array<f32>(),
                                                  Vector{Location(Source{{12, 34}}, 0_a)}),
                                       });
    Func(Source{{12, 34}}, "main", tint::Empty, ty.Of(output),
         Vector{
             Return(Call(ty.Of(output))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(13:43 error: cannot apply '@location' to declaration of type 'array<f32>'
12:34 note: '@location' must only be applied to declarations of numeric scalar or numeric vector type)");
}

TEST_F(LocationAttributeTests, ComputeShaderLocation_Input) {
    Func("main", tint::Empty, ty.i32(),
         Vector{
             Return(Expr(1_i)),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         },
         Vector{
             Location(Source{{12, 34}}, 1_a),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@location' cannot be used by compute shaders)");
}

TEST_F(LocationAttributeTests, ComputeShaderLocation_Output) {
    auto* input = Param("input", ty.i32(),
                        Vector{
                            Location(Source{{12, 34}}, 0_u),
                        });
    Func("main", Vector{input}, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@location' cannot be used by compute shaders)");
}

TEST_F(LocationAttributeTests, ComputeShaderLocationStructMember_Output) {
    auto* m = Member("m", ty.i32(),
                     Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    auto* s = Structure("S", Vector{m});
    Func(Source{{56, 78}}, "main", tint::Empty, ty.Of(s),
         Vector{
             Return(Expr(Call(ty.Of(s)))),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@location' cannot be used by compute shaders
56:78 note: while analyzing entry point 'main')");
}

TEST_F(LocationAttributeTests, ComputeShaderLocationStructMember_Input) {
    auto* m = Member("m", ty.i32(),
                     Vector{
                         Location(Source{{12, 34}}, 0_u),
                     });
    auto* s = Structure("S", Vector{m});
    auto* input = Param("input", ty.Of(s));
    Func(Source{{56, 78}}, "main", Vector{input}, ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kCompute),
             create<ast::WorkgroupAttribute>(Source{{12, 34}}, Expr(1_i)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@location' cannot be used by compute shaders
56:78 note: while analyzing entry point 'main')");
}

TEST_F(LocationAttributeTests, Duplicate_input) {
    // @fragment
    // fn main(@location(1) param_a : f32,
    //         @location(1) param_b : f32) {}
    auto* param_a = Param("param_a", ty.f32(),
                          Vector{
                              Location(1_a),
                          });
    auto* param_b = Param("param_b", ty.f32(),
                          Vector{
                              Location(Source{{12, 34}}, 1_a),
                          });
    Func(Source{{12, 34}}, "main",
         Vector{
             param_a,
             param_b,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@location(1)' appears multiple times)");
}

TEST_F(LocationAttributeTests, Duplicate_struct) {
    // struct InputA {
    //   @location(1) a : f32;
    // };
    // struct InputB {
    //   @location(1) a : f32;
    // };
    // @fragment
    // fn main(param_a : InputA, param_b : InputB) {}
    auto* input_a = Structure("InputA", Vector{
                                            Member("a", ty.f32(), Vector{Location(1_a)}),
                                        });
    auto* input_b =
        Structure("InputB", Vector{
                                Member("a", ty.f32(), Vector{Location(Source{{34, 56}}, 1_a)}),
                            });
    auto* param_a = Param("param_a", ty.Of(input_a));
    auto* param_b = Param("param_b", ty.Of(input_b));
    Func(Source{{12, 34}}, "main",
         Vector{
             param_a,
             param_b,
         },
         ty.void_(), tint::Empty,
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(34:56 error: '@location(1)' appears multiple times
12:34 note: while analyzing entry point 'main')");
}

}  // namespace
}  // namespace LocationAttributeTests

}  // namespace
}  // namespace tint::resolver
