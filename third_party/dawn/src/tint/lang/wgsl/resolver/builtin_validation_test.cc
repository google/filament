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

#include <unordered_set>

#include "src/tint/lang/wgsl/ast/builtin_texture_helper_test.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverBuiltinValidationTest = ResolverTest;

TEST_F(ResolverBuiltinValidationTest, FunctionTypeMustMatchReturnStatementType_void_fail) {
    // fn func { return workgroupBarrier(); }
    Func("func", tint::Empty, ty.void_(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "workgroupBarrier")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: builtin function 'workgroupBarrier' does not return a value");
}

TEST_F(ResolverBuiltinValidationTest, InvalidPipelineStageDirect) {
    // @compute @workgroup_size(1) fn func { return dpdx(1.0); }

    auto* dpdx = Call(Source{{3, 4}}, "dpdx", 1_f);
    Func(Source{{1, 2}}, "func", tint::Empty, ty.void_(),
         Vector{
             Assign(Phony(), dpdx),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "3:4 error: built-in cannot be used by compute pipeline stage");
}

TEST_F(ResolverBuiltinValidationTest, InvalidPipelineStageIndirect) {
    // fn f0 { return dpdx(1.0); }
    // fn f1 { f0(); }
    // fn f2 { f1(); }
    // @compute @workgroup_size(1) fn main { return f2(); }

    auto* dpdx = Call(Source{{3, 4}}, "dpdx", 1_f);
    Func(Source{{1, 2}}, "f0", tint::Empty, ty.void_(),
         Vector{
             Assign(Phony(), dpdx),
         });

    Func(Source{{3, 4}}, "f1", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("f0")),
         });

    Func(Source{{5, 6}}, "f2", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("f1")),
         });

    Func(Source{{7, 8}}, "main", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("f2")),
         },
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(3:4 error: built-in cannot be used by compute pipeline stage
1:2 note: called by function 'f0'
3:4 note: called by function 'f1'
5:6 note: called by function 'f2'
7:8 note: called by entry point 'main')");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsFunctionUsedAsFunction) {
    auto* mix = Func(Source{{12, 34}}, "mix", tint::Empty, ty.i32(),
                     Vector{
                         Return(1_i),
                     });
    auto* use = Call("mix");
    WrapInFunction(use);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get<sem::Call>(use);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Target(), Sem().Get(mix));
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsFunctionUsedAsVariable) {
    Func(Source{{12, 34}}, "mix", tint::Empty, ty.i32(),
         Vector{
             Return(1_i),
         });
    WrapInFunction(Decl(Var("v", Expr(Source{{56, 78}}, "mix"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: cannot use function 'mix' as value
12:34 note: function 'mix' declared here
56:78 note: are you missing '()'?)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsGlobalConstUsedAsVariable) {
    auto* mix = GlobalConst(Source{{12, 34}}, "mix", ty.i32(), Expr(1_i));
    auto* use = Expr("mix");
    WrapInFunction(Decl(Var("v", use)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get<sem::VariableUser>(use);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Variable(), Sem().Get(mix));
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsGlobalVarUsedAsVariable) {
    auto* mix =
        GlobalVar(Source{{12, 34}}, "mix", ty.i32(), Expr(1_i), core::AddressSpace::kPrivate);
    auto* use = Expr("mix");
    WrapInFunction(Decl(Var("v", use)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().GetVal(use)->UnwrapLoad()->As<sem::VariableUser>();
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Variable(), Sem().Get(mix));
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsAliasUsedAsFunction) {
    Alias(Source{{12, 34}}, "mix", ty.i32());
    WrapInFunction(Call(Source{{56, 78}}, "mix", 1_f, 2_f, 3_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(56:78 error: no matching constructor for 'i32(f32, f32, f32)'

2 candidate constructors:
 • 'i32(i32  ✗ ) -> i32'
 • 'i32() -> i32' where:
      ✗  overload expects 0 arguments, call passed 3 arguments

1 candidate conversion:
 • 'i32(T  ✓ ) -> i32' where:
      ✗  overload expects 1 argument, call passed 3 arguments
      ✓  'T' is 'abstract-int', 'abstract-float', 'f32', 'f16', 'u32' or 'bool'
)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsAliasUsedAsType) {
    auto* mix = Alias(Source{{12, 34}}, "mix", ty.i32());
    auto* use = Call("mix");
    WrapInFunction(use);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get<sem::Call>(use);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Type(), Sem().Get(mix));
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsStructUsedAsFunction) {
    Structure("mix", Vector{
                         Member("m", ty.i32()),
                     });
    WrapInFunction(Call(Source{{12, 34}}, "mix", 1_f, 2_f, 3_f));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: structure constructor has too many inputs: expected 1, found 3)");
}

TEST_F(ResolverBuiltinValidationTest, BuiltinRedeclaredAsStructUsedAsType) {
    auto* mix = Structure("mix", Vector{
                                     Member("m", ty.i32()),
                                 });
    auto* use = Call("mix");
    WrapInFunction(use);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    auto* sem = Sem().Get<sem::Call>(use);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Type(), Sem().Get(mix));
}

namespace texture_constexpr_args {

using TextureOverloadCase = ast::test::TextureOverloadCase;
using ValidTextureOverload = ast::test::ValidTextureOverload;
using TextureKind = ast::test::TextureKind;
using TextureDataType = ast::test::TextureDataType;

static std::vector<TextureOverloadCase> TextureCases(
    std::unordered_set<ValidTextureOverload> overloads) {
    std::vector<TextureOverloadCase> cases;
    for (auto c : TextureOverloadCase::ValidCases()) {
        if (overloads.count(c.overload)) {
            cases.push_back(c);
        }
    }
    return cases;
}

enum class Position {
    kFirst,
    kLast,
};

struct Parameter {
    const char* const name;
    const Position position;
    int min;
    int max;
};

class Constexpr {
  public:
    enum class Kind {
        kScalar,
        kVec2,
        kVec3,
        kVec3_Scalar_Vec2,
        kVec3_Vec2_Scalar,
        kEmptyVec2,
        kEmptyVec3,
    };

    Constexpr(int32_t invalid_idx, Kind k, int32_t x = 0, int32_t y = 0, int32_t z = 0)
        : invalid_index(invalid_idx), kind(k), values{x, y, z} {}

    const ast::Expression* operator()(Source src, ProgramBuilder& b) {
        switch (kind) {
            case Kind::kScalar:
                return b.Expr(src, i32(values[0]));
            case Kind::kVec2:
                return b.Call(src, b.ty.vec2<i32>(), i32(values[0]), i32(values[1]));
            case Kind::kVec3:
                return b.Call(src, b.ty.vec3<i32>(), i32(values[0]), i32(values[1]),
                              i32(values[2]));
            case Kind::kVec3_Scalar_Vec2:
                return b.Call(src, b.ty.vec3<i32>(), i32(values[0]),
                              b.Call<vec2<i32>>(i32(values[1]), i32(values[2])));
            case Kind::kVec3_Vec2_Scalar:
                return b.Call(src, b.ty.vec3<i32>(),
                              b.Call<vec2<i32>>(i32(values[0]), i32(values[1])), i32(values[2]));
            case Kind::kEmptyVec2:
                return b.Call(src, b.ty.vec2<i32>());
            case Kind::kEmptyVec3:
                return b.Call(src, b.ty.vec3<i32>());
        }
        return nullptr;
    }

    static const constexpr int32_t kValid = -1;
    const int32_t invalid_index;  // Expected error value, or kValid
    const Kind kind;
    const std::array<int32_t, 3> values;
};

static std::ostream& operator<<(std::ostream& out, Parameter param) {
    return out << param.name;
}

static std::ostream& operator<<(std::ostream& out, Constexpr expr) {
    switch (expr.kind) {
        case Constexpr::Kind::kScalar:
            return out << expr.values[0];
        case Constexpr::Kind::kVec2:
            return out << "vec2(" << expr.values[0] << ", " << expr.values[1] << ")";
        case Constexpr::Kind::kVec3:
            return out << "vec3(" << expr.values[0] << ", " << expr.values[1] << ", "
                       << expr.values[2] << ")";
        case Constexpr::Kind::kVec3_Scalar_Vec2:
            return out << "vec3(" << expr.values[0] << ", vec2(" << expr.values[1] << ", "
                       << expr.values[2] << "))";
        case Constexpr::Kind::kVec3_Vec2_Scalar:
            return out << "vec3(vec2(" << expr.values[0] << ", " << expr.values[1] << "), "
                       << expr.values[2] << ")";
        case Constexpr::Kind::kEmptyVec2:
            return out << "vec2()";
        case Constexpr::Kind::kEmptyVec3:
            return out << "vec3()";
    }
    return out;
}

using BuiltinTextureConstExprArgValidationTest =
    ResolverTestWithParam<std::tuple<TextureOverloadCase, Parameter, Constexpr>>;

TEST_P(BuiltinTextureConstExprArgValidationTest, Immediate) {
    auto& p = GetParam();
    auto overload = std::get<0>(p);
    auto param = std::get<1>(p);
    auto expr = std::get<2>(p);

    overload.BuildTextureVariable(this);
    overload.BuildSamplerVariable(this);

    auto args = overload.args(this);
    auto*& arg_to_replace = (param.position == Position::kFirst) ? args.Front() : args.Back();

    // BuildTextureVariable() uses a Literal for scalars, and a CallExpression for a vector
    // constructor.
    bool is_vector = arg_to_replace->Is<ast::CallExpression>();

    // Make the expression to be replaced, reachable. This keeps the resolver happy.
    WrapInFunction(arg_to_replace);

    arg_to_replace = expr(Source{{12, 34}}, *this);

    auto* call = Call(overload.function, args);
    auto* stmt = overload.returns_value ? static_cast<const ast::Statement*>(Assign(Phony(), call))
                                        : static_cast<const ast::Statement*>(CallStmt(call));

    // Call the builtin with the constexpr argument replaced
    Func("func", tint::Empty, ty.void_(),
         Vector{
             stmt,
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (expr.invalid_index == Constexpr::kValid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        StringStream err;
        if (is_vector) {
            err << "12:34 error: each component of the " << param.name
                << " argument must be at least " << param.min << " and at most " << param.max
                << ". " << param.name << " component " << expr.invalid_index << " is "
                << std::to_string(expr.values[static_cast<size_t>(expr.invalid_index)]);
        } else {
            err << "12:34 error: the " << param.name << " argument must be at least " << param.min
                << " and at most " << param.max << ". " << param.name << " is "
                << std::to_string(expr.values[static_cast<size_t>(expr.invalid_index)]);
        }
        EXPECT_EQ(r()->error(), err.str());
    }
}

TEST_P(BuiltinTextureConstExprArgValidationTest, GlobalConst) {
    auto& p = GetParam();
    auto overload = std::get<0>(p);
    auto param = std::get<1>(p);
    auto expr = std::get<2>(p);

    // Build the global texture and sampler variables
    overload.BuildTextureVariable(this);
    overload.BuildSamplerVariable(this);

    // Build the module-scope const 'G' with the offset value
    GlobalConst("G", expr({}, *this));

    auto args = overload.args(this);
    auto*& arg_to_replace = (param.position == Position::kFirst) ? args.Front() : args.Back();

    // BuildTextureVariable() uses a Literal for scalars, and a CallExpression for a vector
    // constructor.
    bool is_vector = arg_to_replace->Is<ast::CallExpression>();

    // Make the expression to be replaced, reachable. This keeps the resolver happy.
    WrapInFunction(arg_to_replace);

    arg_to_replace = Expr(Source{{12, 34}}, "G");

    auto* call = Call(overload.function, args);
    auto* stmt = overload.returns_value ? static_cast<const ast::Statement*>(Assign(Phony(), call))
                                        : static_cast<const ast::Statement*>(CallStmt(call));

    // Call the builtin with the constant-expression argument replaced
    Func("func", tint::Empty, ty.void_(),
         Vector{
             stmt,
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    if (expr.invalid_index == Constexpr::kValid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        StringStream err;
        if (is_vector) {
            err << "12:34 error: each component of the " << param.name
                << " argument must be at least " << param.min << " and at most " << param.max
                << ". " << param.name << " component " << expr.invalid_index << " is "
                << std::to_string(expr.values[static_cast<size_t>(expr.invalid_index)]);
        } else {
            err << "12:34 error: the " << param.name << " argument must be at least " << param.min
                << " and at most " << param.max << ". " << param.name << " is "
                << std::to_string(expr.values[static_cast<size_t>(expr.invalid_index)]);
        }
        EXPECT_EQ(r()->error(), err.str());
    }
}

TEST_P(BuiltinTextureConstExprArgValidationTest, GlobalVar) {
    auto& p = GetParam();
    auto overload = std::get<0>(p);
    auto param = std::get<1>(p);
    auto expr = std::get<2>(p);

    // Build the global texture and sampler variables
    overload.BuildTextureVariable(this);
    overload.BuildSamplerVariable(this);

    // Build the module-scope var 'G' with the offset value
    GlobalVar("G", expr({}, *this), core::AddressSpace::kPrivate);

    auto args = overload.args(this);
    auto*& arg_to_replace = (param.position == Position::kFirst) ? args.Front() : args.Back();

    // Make the expression to be replaced, reachable. This keeps the resolver happy.
    WrapInFunction(arg_to_replace);

    arg_to_replace = Expr(Source{{12, 34}}, "G");

    auto* call = Call(overload.function, args);
    auto* stmt = overload.returns_value ? static_cast<const ast::Statement*>(Assign(Phony(), call))
                                        : static_cast<const ast::Statement*>(CallStmt(call));

    // Call the builtin with the constant-expression argument replaced
    Func("func", tint::Empty, ty.void_(),
         Vector{
             stmt,
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    StringStream err;
    err << "12:34 error: the " << param.name << " argument must be a const-expression";
    EXPECT_EQ(r()->error(), err.str());
}
INSTANTIATE_TEST_SUITE_P(
    Offset2D,
    BuiltinTextureConstExprArgValidationTest,
    testing::Combine(testing::ValuesIn(TextureCases({
                         ValidTextureOverload::kSample2dOffsetF32,
                         ValidTextureOverload::kSample2dArrayOffsetF32,
                         ValidTextureOverload::kSampleDepth2dOffsetF32,
                         ValidTextureOverload::kSampleDepth2dArrayOffsetF32,
                         ValidTextureOverload::kSampleBias2dOffsetF32,
                         ValidTextureOverload::kSampleBias2dArrayOffsetF32,
                         ValidTextureOverload::kSampleLevel2dOffsetF32,
                         ValidTextureOverload::kSampleLevel2dArrayOffsetF32,
                         ValidTextureOverload::kSampleLevelDepth2dOffsetF32,
                         ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32,
                         ValidTextureOverload::kSampleGrad2dOffsetF32,
                         ValidTextureOverload::kSampleGrad2dArrayOffsetF32,
                         ValidTextureOverload::kSampleCompareDepth2dOffsetF32,
                         ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32,
                         ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32,
                         ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32,
                     })),
                     testing::Values(Parameter{"offset", Position::kLast, -8, 7}),
                     testing::Values(Constexpr{Constexpr::kValid, Constexpr::Kind::kEmptyVec2},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kVec2, -1, 1},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kVec2, 7, -8},
                                     Constexpr{0, Constexpr::Kind::kVec2, 8, 0},
                                     Constexpr{1, Constexpr::Kind::kVec2, 0, 8},
                                     Constexpr{0, Constexpr::Kind::kVec2, -9, 0},
                                     Constexpr{1, Constexpr::Kind::kVec2, 0, -9},
                                     Constexpr{0, Constexpr::Kind::kVec2, 8, 8},
                                     Constexpr{0, Constexpr::Kind::kVec2, -9, -9})));

INSTANTIATE_TEST_SUITE_P(
    Offset3D,
    BuiltinTextureConstExprArgValidationTest,
    testing::Combine(testing::ValuesIn(TextureCases({
                         ValidTextureOverload::kSample3dOffsetF32,
                         ValidTextureOverload::kSampleBias3dOffsetF32,
                         ValidTextureOverload::kSampleLevel3dOffsetF32,
                         ValidTextureOverload::kSampleGrad3dOffsetF32,
                     })),
                     testing::Values(Parameter{"offset", Position::kLast, -8, 7}),
                     testing::Values(Constexpr{Constexpr::kValid, Constexpr::Kind::kEmptyVec3},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kVec3, 0, 0, 0},
                                     Constexpr{Constexpr::kValid, Constexpr::Kind::kVec3, 7, -8, 7},
                                     Constexpr{0, Constexpr::Kind::kVec3, 10, 0, 0},
                                     Constexpr{1, Constexpr::Kind::kVec3, 0, 10, 0},
                                     Constexpr{2, Constexpr::Kind::kVec3, 0, 0, 10},
                                     Constexpr{0, Constexpr::Kind::kVec3, 10, 11, 12},
                                     Constexpr{0, Constexpr::Kind::kVec3_Scalar_Vec2, 10, 0, 0},
                                     Constexpr{1, Constexpr::Kind::kVec3_Scalar_Vec2, 0, 10, 0},
                                     Constexpr{2, Constexpr::Kind::kVec3_Scalar_Vec2, 0, 0, 10},
                                     Constexpr{0, Constexpr::Kind::kVec3_Scalar_Vec2, 10, 11, 12},
                                     Constexpr{0, Constexpr::Kind::kVec3_Vec2_Scalar, 10, 0, 0},
                                     Constexpr{1, Constexpr::Kind::kVec3_Vec2_Scalar, 0, 10, 0},
                                     Constexpr{2, Constexpr::Kind::kVec3_Vec2_Scalar, 0, 0, 10},
                                     Constexpr{0, Constexpr::Kind::kVec3_Vec2_Scalar, 10, 11,
                                               12})));

INSTANTIATE_TEST_SUITE_P(
    Component,
    BuiltinTextureConstExprArgValidationTest,
    testing::Combine(
        testing::ValuesIn(TextureCases({
            ValidTextureOverload::kGather2dF32, ValidTextureOverload::kGather2dOffsetF32,
            ValidTextureOverload::kGather2dArrayF32, ValidTextureOverload::kGatherCubeF32,
            // The below require mixed integer signedness.
            // See https://github.com/gpuweb/gpuweb/issues/3536
            // ValidTextureOverload::kGather2dArrayOffsetF32,
            // ValidTextureOverload::kGatherCubeArrayF32,
        })),
        testing::Values(Parameter{"component", Position::kFirst, 0, 3}),
        testing::Values(Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 0},
                        Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 1},
                        Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 2},
                        Constexpr{Constexpr::kValid, Constexpr::Kind::kScalar, 3},
                        Constexpr{0, Constexpr::Kind::kScalar, 4},
                        Constexpr{0, Constexpr::Kind::kScalar, 123},
                        Constexpr{0, Constexpr::Kind::kScalar, -1})));

}  // namespace texture_constexpr_args

using ResolverPacked4x8IntegerDotProductValidationTest = ResolverTest;

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Dot4I8Packed) {
    // fn func { return dot4I8Packed(1u, 2u); }
    Require(wgsl::LanguageFeature::kPacked4X8IntegerDotProduct);

    Func("func", tint::Empty, ty.i32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "dot4I8Packed",
                         Vector{Expr(1_u), Expr(2_u)})),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Dot4I8Packed_FeatureDisallowed) {
    // fn func { return dot4I8Packed(1u, 2u); }
    Func("func", tint::Empty, ty.i32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "dot4I8Packed",
                         Vector{Expr(1_u), Expr(2_u)})),
         });

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: built-in function 'dot4I8Packed' requires the "
              "'packed_4x8_integer_dot_product' language feature, which is not allowed in the "
              "current environment");
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Dot4U8Packed) {
    // fn func { return dot4U8Packed(1u, 2u); }
    Require(wgsl::LanguageFeature::kPacked4X8IntegerDotProduct);

    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "dot4U8Packed",
                         Vector{Expr(1_u), Expr(2_u)})),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Dot4U8Packed_FeatureDisallowed) {
    // fn func { return dot4U8Packed(1u, 2u); }
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "dot4U8Packed",
                         Vector{Expr(1_u), Expr(2_u)})),
         });

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: built-in function 'dot4U8Packed' requires the "
              "'packed_4x8_integer_dot_product' language feature, which is not allowed in the "
              "current environment");
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Pack4xI8) {
    // fn func { return pack4xI8(vec4i()); }
    Require(wgsl::LanguageFeature::kPacked4X8IntegerDotProduct);

    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "pack4xI8", Call<vec4<i32>>())),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Pack4xI8_FeatureDisallowed) {
    // fn func { return pack4xI8(vec4i()); }
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "pack4xI8", Call<vec4<i32>>())),
         });

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: built-in function 'pack4xI8' requires the "
              "'packed_4x8_integer_dot_product' language feature, which is not allowed in the "
              "current environment");
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Pack4xU8) {
    // fn func { return pack4xU8(vec4u()); }
    Require(wgsl::LanguageFeature::kPacked4X8IntegerDotProduct);

    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "pack4xU8", Call<vec4<u32>>())),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Pack4xU8_FeatureDisallowed) {
    // fn func { return pack4xU8(vec4u()); }
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "pack4xU8", Call<vec4<u32>>())),
         });

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: built-in function 'pack4xU8' requires the "
              "'packed_4x8_integer_dot_product' language feature, which is not allowed in the "
              "current environment");
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Pack4xI8Clamp) {
    // fn func { return pack4xI8Clamp(vec4i()); }
    Require(wgsl::LanguageFeature::kPacked4X8IntegerDotProduct);

    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "pack4xI8Clamp", Call<vec4<i32>>())),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Pack4xI8Clamp_FeatureDisallowed) {
    // fn func { return pack4xI8Clamp(vec4i()); }
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "pack4xI8Clamp", Call<vec4<i32>>())),
         });

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: built-in function 'pack4xI8Clamp' requires the "
              "'packed_4x8_integer_dot_product' language feature, which is not allowed in the "
              "current environment");
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Pack4xU8Clamp) {
    // fn func { return pack4xU8Clamp(vec4u()); }
    Require(wgsl::LanguageFeature::kPacked4X8IntegerDotProduct);

    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "pack4xU8Clamp", Call<vec4<u32>>())),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Pack4xU8Clamp_FeatureDisallowed) {
    // fn func { return pack4xU8Clamp(vec4u()); }
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "pack4xU8Clamp", Call<vec4<u32>>())),
         });

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: built-in function 'pack4xU8Clamp' requires the "
              "'packed_4x8_integer_dot_product' language feature, which is not allowed in the "
              "current environment");
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Unpack4xI8) {
    // fn func { return unpack4xI8(u32()); }
    Require(wgsl::LanguageFeature::kPacked4X8IntegerDotProduct);

    Func("func", tint::Empty, ty.vec4<i32>(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "unpack4xI8", Call<u32>())),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Unpack4xI8_FeatureDisallowed) {
    // fn func { return unpack4xI8(u32()); }
    Func("func", tint::Empty, ty.vec4<i32>(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "unpack4xI8", Call<u32>())),
         });

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: built-in function 'unpack4xI8' requires the "
              "'packed_4x8_integer_dot_product' language feature, which is not allowed in the "
              "current environment");
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Unpack4xU8) {
    // fn func { return unpack4xU8(u32()); }
    Require(wgsl::LanguageFeature::kPacked4X8IntegerDotProduct);

    Func("func", tint::Empty, ty.vec4<u32>(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "unpack4xU8", Call<u32>())),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverPacked4x8IntegerDotProductValidationTest, Unpack4xU8_FeatureDisallowed) {
    // fn func { return unpack4xU8(u32()); }
    Func("func", tint::Empty, ty.vec4<u32>(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "unpack4xU8", Call<u32>())),
         });

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: built-in function 'unpack4xU8' requires the "
              "'packed_4x8_integer_dot_product' language feature, which is not allowed in the "
              "current environment");
}

TEST_F(ResolverBuiltinValidationTest, WorkgroupUniformLoad_WrongAddressSpace) {
    // @group(0) @binding(0) var<storage, read_write> v : i32;
    // fn foo() {
    //   workgroupUniformLoad(&v);
    // }
    GlobalVar("v", ty.i32(), core::AddressSpace::kStorage, core::Access::kReadWrite,
              Vector{Group(0_a), Binding(0_a)});
    WrapInFunction(CallStmt(Call("workgroupUniformLoad", AddressOf(Source{{12, 34}}, "v"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: no matching call to 'workgroupUniformLoad(ptr<storage, i32, read_write>)'

1 candidate function:
 • 'workgroupUniformLoad(ptr<workgroup, T, read_write>  ✗ ) -> T'
)");
}

TEST_F(ResolverBuiltinValidationTest, WorkgroupUniformLoad_Atomic) {
    // var<workgroup> v : atomic<i32>;
    // fn foo() {
    //   workgroupUniformLoad(&v);
    // }
    GlobalVar("v", ty.atomic<i32>(), core::AddressSpace::kWorkgroup);
    WrapInFunction(CallStmt(Call("workgroupUniformLoad", AddressOf(Source{{12, 34}}, "v"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: workgroupUniformLoad must not be called with an argument that contains an atomic type)");
}

TEST_F(ResolverBuiltinValidationTest, WorkgroupUniformLoad_AtomicInArray) {
    // var<workgroup> v : array<atomic<i32>, 4>;
    // fn foo() {
    //   workgroupUniformLoad(&v);
    // }
    GlobalVar("v", ty.array(ty.atomic<i32>(), 4_a), core::AddressSpace::kWorkgroup);
    WrapInFunction(CallStmt(Call("workgroupUniformLoad", AddressOf(Source{{12, 34}}, "v"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: workgroupUniformLoad must not be called with an argument that contains an atomic type)");
}

TEST_F(ResolverBuiltinValidationTest, WorkgroupUniformLoad_AtomicInStruct) {
    // struct Inner { a : array<atomic<i32, 4> }
    // struct S { i : Inner }
    // var<workgroup> v : array<S, 4>;
    // fn foo() {
    //   workgroupUniformLoad(&v);
    // }
    Structure("Inner", Vector{Member("a", ty.array(ty.atomic<i32>(), 4_a))});
    Structure("S", Vector{Member("i", ty("Inner"))});
    GlobalVar(Source{{12, 34}}, "v", ty.array(ty("S"), 4_a), core::AddressSpace::kWorkgroup);
    WrapInFunction(CallStmt(Call("workgroupUniformLoad", AddressOf("v"))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: workgroupUniformLoad must not be called with an argument that contains an atomic type)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupShuffleLaneArgMustBeNonNeg) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupShuffle", 1_u, Expr(Source{{12, 34}}, -1_i))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: the sourceLaneIndex argument of subgroupShuffle must be greater than or equal to zero)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupShuffleLaneArgMustLessThan128Signed) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupShuffle", 1_u, Expr(Source{{12, 34}}, 128_i))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: the sourceLaneIndex argument of subgroupShuffle must be less than 128)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupShuffleLaneArgMustLessThan128) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupShuffle", 1_u, Expr(Source{{12, 34}}, 128_u))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: the sourceLaneIndex argument of subgroupShuffle must be less than 128)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupShuffleUpDeltaArgMustLessThan128) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupShuffleUp", 1_u, Expr(Source{{12, 34}}, 128_u))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: the delta argument of subgroupShuffleUp must be less than 128)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupShuffleDownDeltaArgMustLessThan128) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupShuffleDown", 1_u, Expr(Source{{12, 34}}, 128_u))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: the delta argument of subgroupShuffleDown must be less than 128)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupShuffleXorMaskArgMustLessThan128) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupShuffleXor", 1_u, Expr(Source{{12, 34}}, 128_u))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: the mask argument of subgroupShuffleXor must be less than 128)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBallotWithoutExtension) {
    // fn func { return subgroupBallot(true); }
    Func("func", tint::Empty, ty.vec4<u32>(),
         Vector{
             Return(Call(Source{Source::Location{12, 34}}, "subgroupBallot", true)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: cannot call built-in function 'subgroupBallot' without extension 'subgroups')");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBallotWithExtension) {
    // enable subgroups;
    // fn func -> vec4<u32> { return subgroupBallot(true); }
    Enable(wgsl::Extension::kSubgroups);

    Func("func", tint::Empty, ty.vec4<u32>(),
         Vector{
             Return(Call("subgroupBallot", true)),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBallotWithoutArgument) {
    // enable subgroups;
    // fn func -> vec4<u32> { return subgroupBallot(); }
    Enable(wgsl::Extension::kSubgroups);

    Func("func", tint::Empty, ty.vec4<u32>(),
         Vector{
             Return(Call("subgroupBallot")),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: no matching call to 'subgroupBallot()'

1 candidate function:
 • 'subgroupBallot(bool  ✗ ) -> vec4<u32>'
)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastWithoutExtension) {
    // fn func -> i32 { return subgroupBroadcast(1,0); }
    Func("func", tint::Empty, ty.i32(),
         Vector{
             Return(Call(Source{{12, 34}}, "subgroupBroadcast", 1_i, 0_u)),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: cannot call built-in function 'subgroupBroadcast' without extension 'subgroups')");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastWithExtension) {
    // enable subgroups;
    // fn func -> i32 { return subgroupBroadcast(1,0); }
    Enable(wgsl::Extension::kSubgroups);

    Func("func", tint::Empty, ty.i32(),
         Vector{
             Return(Call("subgroupBroadcast", 1_i, 0_u)),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastWithoutSubgroupsF16Extension_F16) {
    // enable f16;
    // enable subgroups;
    // fn func -> f16 { return subgroupBroadcast(1.h,0); }
    Enable(wgsl::Extension::kF16);
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.f16(),
         Vector{
             Return(Call(Source{{12, 34}}, "subgroupBroadcast", 1_h, 0_u)),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastWithoutSubgroupsExtension_F16) {
    // enable f16;
    // fn func -> f16 { return subgroupBroadcast(1.h,0); }
    Enable(wgsl::Extension::kF16);
    Func("func", tint::Empty, ty.f16(),
         Vector{
             Return(Call(Source{{12, 34}}, "subgroupBroadcast", 1_h, 0_u)),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: cannot call built-in function 'subgroupBroadcast' without extension 'subgroups')");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastWithoutShaderF16Extension_F16) {
    // enable f16;
    // fn func -> f16 { return subgroupBroadcast(1.h,0); }
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.f16(),
         Vector{
             Return(Call(Source{{12, 34}}, "subgroupBroadcast", 1_h, 0_u)),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'f16' type used without 'f16' extension enabled)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastWithSubgroupsF16WithoutShaderF16Extension) {
    // enable f16;
    // fn func -> f16 { return subgroupBroadcast(1.h,0); }
    Enable(wgsl::Extension::kSubgroups);
    Enable(wgsl::Extension::kSubgroupsF16);
    Func("func", tint::Empty, ty.f16(),
         Vector{
             Return(Call(Source{{12, 34}}, "subgroupBroadcast", 1_h, 0_u)),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: 'f16' type used without 'f16' extension enabled
error: extension 'subgroups_f16' cannot be used without extension 'f16')");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastWithExtensions_F16) {
    // enable f16;
    // enable subgroups;
    // enable subgroups_f16;
    // fn func -> f16 { return subgroupBroadcast(1.h,0); }
    Enable(wgsl::Extension::kF16);
    Enable(wgsl::Extension::kSubgroups);
    // TODO(crbug.com/380244620): Remove when 'subgroups_f16' has been fully deprecated.
    Enable(wgsl::Extension::kSubgroupsF16);

    Func("func", tint::Empty, ty.f16(),
         Vector{
             Return(Call("subgroupBroadcast", 1_h, 0_u)),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastWithoutExtension_VecF16) {
    // enable f16;
    // enable subgroups;
    // fn func -> vec4<f16> { return subgroupBroadcast(vec4(1.h),0); }
    Enable(wgsl::Extension::kF16);
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.vec4<f16>(),
         Vector{
             Return(Call(Source{{12, 34}}, "subgroupBroadcast", Call(ty.vec4<f16>(), 1_h), 0_u)),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastWithExtensions_VecF16) {
    // enable f16;
    // enable subgroups;
    // enable subgroups_f16;
    // fn func -> vec4<f16> { return subgroupBroadcast(vec4(1.h),0); }
    Enable(wgsl::Extension::kF16);
    Enable(wgsl::Extension::kSubgroups);
    // TODO(crbug.com/380244620): Remove when 'subgroups_f16' has been fully deprecated.
    Enable(wgsl::Extension::kSubgroupsF16);

    Func("func", tint::Empty, ty.vec4<f16>(),
         Vector{
             Return(Call("subgroupBroadcast", Call(ty.vec4<f16>(), 1_h), 0_u)),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubroupBroadcastInComputeStage) {
    // @vertex fn func { dpdx(1.0); }

    Enable(wgsl::Extension::kSubgroups);

    auto* call = Call("subgroupBroadcast", 1_f, 0_u);
    Func(Source{{1, 2}}, "func", tint::Empty, ty.void_(), Vector{Ignore(call)},
         Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubroupBroadcastInVertexStageIsError) {
    // @vertex fn func { dpdx(1.0); }

    Enable(wgsl::Extension::kSubgroups);

    auto* call = Call(Source{{3, 4}}, "subgroupBroadcast", 1_f, 0_u);
    Func("func", tint::Empty, ty.vec4<f32>(), Vector{Ignore(call), Return(Call(ty.vec4<f32>()))},
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{Builtin(core::BuiltinValue::kPosition)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "3:4 error: built-in cannot be used by vertex pipeline stage");
}

TEST_F(ResolverBuiltinValidationTest, SubroupBroadcastInFragmentStageIsValid) {
    // @vertex fn func { dpdx(1.0); }

    Enable(wgsl::Extension::kSubgroups);

    auto* call = Call(Source{{3, 4}}, "subgroupBroadcast", 1_f, 0_u);
    Func("func",
         Vector{Param("pos", ty.vec4<f32>(), Vector{Builtin(core::BuiltinValue::kPosition)})},
         ty.void_(), Vector{Ignore(call)},
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastValueF32) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.f32(),
         Vector{
             Return(Call("subgroupBroadcast", 1_f, 0_u)),
         });
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastValueI32) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.i32(),
         Vector{
             Return(Call("subgroupBroadcast", 1_i, 0_u)),
         });
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastValueU32) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupBroadcast", 1_u, 0_u)),
         });
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastLaneArgMustBeConst) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("lane", Expr(1_u))),
             Ignore(Call("subgroupBroadcast", 1_f, Ident(Source{{12, 34}}, "lane"))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: the sourceLaneIndex argument of subgroupBroadcast must be a const-expression)");
}

TEST_F(ResolverBuiltinValidationTest, QuadBroadcastIdArgMustBeConst) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("id", Expr(1_u))),
             Ignore(Call("quadBroadcast", 1_f, Ident(Source{{12, 34}}, "id"))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: the id argument of quadBroadcast must be a const-expression)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastLaneArgMustBeNonNeg) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupBroadcast", 1_u, Expr(Source{{12, 34}}, -1_i))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: the sourceLaneIndex argument of subgroupBroadcast must be greater than or equal to zero)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastLaneArgMustLessThan128Signed) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupBroadcast", 1_u, Expr(Source{{12, 34}}, 128_i))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: the sourceLaneIndex argument of subgroupBroadcast must be less than 128)");
}

TEST_F(ResolverBuiltinValidationTest, SubgroupBroadcastLaneArgMustLessThan128) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("subgroupBroadcast", 1_u, Expr(Source{{12, 34}}, 128_u))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: the sourceLaneIndex argument of subgroupBroadcast must be less than 128)");
}

TEST_F(ResolverBuiltinValidationTest, QuadBroadcastIdArgMustBeNonNeg) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("quadBroadcast", 1_u, Expr(Source{{12, 34}}, -1_i))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: the id argument of quadBroadcast must be greater than or equal to zero)");
}

TEST_F(ResolverBuiltinValidationTest, QuadBroadcastIdArgMustBeNonNeg4) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("quadBroadcast", 1_u, Expr(Source{{12, 34}}, -4_i))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: the id argument of quadBroadcast must be greater than or equal to zero)");
}

TEST_F(ResolverBuiltinValidationTest, QuadBroadcastIdArgMustLessThan4Signed) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("quadBroadcast", 1_u, Expr(Source{{12, 34}}, 4_i))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: the id argument of quadBroadcast must be less than 4)");
}

TEST_F(ResolverBuiltinValidationTest, QuadBroadcastIdArgMustLessThan4) {
    Enable(wgsl::Extension::kSubgroups);
    Func("func", tint::Empty, ty.u32(),
         Vector{
             Return(Call("quadBroadcast", 1_u, Expr(Source{{12, 34}}, 4_u))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: the id argument of quadBroadcast must be less than 4)");
}

TEST_F(ResolverBuiltinValidationTest, TextureBarrier) {
    // fn func { textureBarrier(); }
    Func("func", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call(Source{Source::Location{12, 34}}, "textureBarrier")),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBuiltinValidationTest, TextureBarrier_FeatureDisallowed) {
    // fn func { textureBarrier(); }
    Func("func", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call(Source{Source::Location{12, 34}}, "textureBarrier")),
         });

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(
        resolver.error(),
        R"(12:34 error: built-in function 'textureBarrier' requires the 'readonly_and_readwrite_storage_textures' language feature, which is not allowed in the current environment)");
}

}  // namespace
}  // namespace tint::resolver
