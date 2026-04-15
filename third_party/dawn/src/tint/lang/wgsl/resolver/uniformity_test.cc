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

// GEN_BUILD:CONDITION(tint_build_wgsl_reader)

#include "src/tint/lang/wgsl/resolver/uniformity.h"

#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class UniformityAnalysisTestBase {
  protected:
    /// Build and resolve a program from a ProgramBuilder object.
    /// @param program the program
    /// @param should_pass true if `builder` program should pass the analysis, otherwise false
    /// @param src the shader source, if non-empty.
    void RunTest(Program&& program, bool should_pass, std::string src = "") {
        error_ = program.Diagnostics().Str();

        bool valid = program.IsValid();
        if (should_pass) {
            EXPECT_TRUE(valid) << error_ << "\n" << src;
            EXPECT_FALSE(program.Diagnostics().ContainsErrors());
        } else {
            if (kUniformityFailuresAsError) {
                EXPECT_FALSE(valid) << "\n" << src;
            } else {
                EXPECT_TRUE(valid) << error_ << "\n" << src;
            }
        }
    }

    /// Parse and resolve a WGSL shader.
    /// @param src the WGSL source code
    /// @param should_pass true if `src` should pass the analysis, otherwise false
    void RunTest(std::string src, bool should_pass) {
        wgsl::reader::Options options;
        options.allowed_features = wgsl::AllowedFeatures::Everything();
        auto file = std::make_unique<Source::File>("test", src);
        auto program = wgsl::reader::Parse(file.get(), options);
        return RunTest(std::move(program), should_pass, src);
    }

    /// Build and resolve a program from a ProgramBuilder object.
    /// @param builder the program builder
    /// @param should_pass true if `builder` program should pass the analysis, otherwise false
    void RunTest(ProgramBuilder&& builder, bool should_pass) {
        auto program = resolver::Resolve(builder);
        return RunTest(std::move(program), should_pass);
    }

    /// The error message from the parser or resolver, if any.
    std::string error_;
};

class UniformityAnalysisTest : public UniformityAnalysisTestBase, public ::testing::Test {};

class BasicTest : public UniformityAnalysisTestBase,
                  public ::testing::TestWithParam<std::tuple<int, int>> {
  public:
    /// Enum for the if-statement condition guarding a function call.
    enum Condition {
        // Uniform conditions:
        kTrue,
        kFalse,
        kLiteral,
        kModuleConst,
        kPipelineOverridable,
        kFuncLetUniformRhs,
        kFuncVarUniform,
        kFuncUniformRetVal,
        kUniformBuffer,
        kROStorageBuffer,
        kLastUniformCondition = kROStorageBuffer,
        // MayBeNonUniform conditions:
        kFuncLetNonUniformRhs,
        kFuncVarNonUniform,
        kFuncNonUniformRetVal,
        kRWStorageBuffer,
        // Scope-dependent conditions:
        kSubgroupId,
        // End of range marker:
        kEndOfConditionRange,
    };

    /// Enum for the function call statement.
    enum Function {
        // NoRestrictionFunctions:
        kUserNoRestriction,
        kMin,
        kTextureSampleLevel,
        kLastNoRestrictionFunction = kTextureSampleLevel,
        // RequiredToBeUniform functions:
        kUserRequiredToBeUniform,
        kWorkgroupBarrier,
        kStorageBarrier,
        kTextureBarrier,
        kWorkgroupUniformLoad,
        kLastNonFragmentFunction = kWorkgroupUniformLoad,
        kTextureSample,
        kTextureSampleBias,
        kTextureSampleCompare,
        kDpdx,
        kDpdxCoarse,
        kDpdxFine,
        kDpdy,
        kDpdyCoarse,
        kDpdyFine,
        kFwidth,
        kFwidthCoarse,
        kFwidthFine,
        kLastNonSubgroupFunction = kFwidthFine,
        // Subgroup functions:
        kSubgroupBallot,
        kSubgroupElect,
        kSubgroupBroadcast,
        kSubgroupBroadcastFirst,
        kSubgroupShuffle,
        kSubgroupShuffleXor,
        kSubgroupShuffleUp,
        kSubgroupShuffleDown,
        kSubgroupAdd,
        kSubgroupInclusiveAdd,
        kSubgroupExclusiveAdd,
        kSubgroupMul,
        kSubgroupInclusiveMul,
        kSubgroupExclusiveMul,
        kSubgroupAnd,
        kSubgroupOr,
        kSubgroupXor,
        kSubgroupMin,
        kSubgroupMax,
        kSubgroupAll,
        kSubgroupAny,
        kQuadBroadcast,
        kQuadSwapX,
        kQuadSwapY,
        kQuadSwapDiagonal,
        // Subgroup matrix functions:
        kSubgroupMatrixConstruct,
        kSubgroupMatrixLoad,
        kSubgroupMatrixStore,
        kSubgroupMatrixMultiply,
        kSubgroupMatrixMultiplyAccumulate,
        kSubgroupMatrixScalarAdd,
        kSubgroupMatrixScalarSubtract,
        kSubgroupMatrixScalarMultiply,
        // End of range marker:
        kEndOfFunctionRange,
    };

    /// Convert a condition to its string representation.
    static std::string ConditionToStr(Condition c) {
        switch (c) {
            case kTrue:
                return "true";
            case kFalse:
                return "false";
            case kLiteral:
                return "7 == 7";
            case kModuleConst:
                return "module_const == 0";
            case kPipelineOverridable:
                return "pipeline_overridable == 0";
            case kFuncLetUniformRhs:
                return "let_uniform_rhs == 0";
            case kFuncVarUniform:
                return "func_uniform == 0";
            case kFuncUniformRetVal:
                return "func_uniform_retval() == 0";
            case kUniformBuffer:
                return "u == 0";
            case kROStorageBuffer:
                return "ro == 0";
            case kFuncLetNonUniformRhs:
                return "let_nonuniform_rhs == 0";
            case kFuncVarNonUniform:
                return "func_non_uniform == 0";
            case kFuncNonUniformRetVal:
                return "func_nonuniform_retval() == 0";
            case kRWStorageBuffer:
                return "rw == 0";
            case kSubgroupId:
                return "subgroup_id == 0";
            case kEndOfConditionRange:
                return "<invalid>";
        }
        return "<invalid>";
    }

    /// Convert a function call to its string representation.
    static std::string FunctionCallToStr(Function f) {
        switch (f) {
            case kUserNoRestriction:
                return "user_no_restriction()";
            case kMin:
                return "_ = min(1, 1)";
            case kTextureSampleLevel:
                return "_ = textureSampleLevel(t, s, vec2(0.5, 0.5), 0.0)";
            case kUserRequiredToBeUniform:
                return "user_required_to_be_uniform()";
            case kWorkgroupBarrier:
                return "workgroupBarrier()";
            case kStorageBarrier:
                return "storageBarrier()";
            case kTextureBarrier:
                return "textureBarrier()";
            case kWorkgroupUniformLoad:
                return "_ = workgroupUniformLoad(&w)";
            case kTextureSample:
                return "_ = textureSample(t, s, vec2(0.5, 0.5))";
            case kTextureSampleBias:
                return "_ = textureSampleBias(t, s, vec2(0.5, 0.5), 2.0)";
            case kTextureSampleCompare:
                return "_ = textureSampleCompare(td, sc, vec2(0.5, 0.5), 0.5)";
            case kDpdx:
                return "_ = dpdx(1.0)";
            case kDpdxCoarse:
                return "_ = dpdxCoarse(1.0)";
            case kDpdxFine:
                return "_ = dpdxFine(1.0)";
            case kDpdy:
                return "_ = dpdy(1.0)";
            case kDpdyCoarse:
                return "_ = dpdyCoarse(1.0)";
            case kDpdyFine:
                return "_ = dpdyFine(1.0)";
            case kFwidth:
                return "_ = fwidth(1.0)";
            case kFwidthCoarse:
                return "_ = fwidthCoarse(1.0)";
            case kFwidthFine:
                return "_ = fwidthFine(1.0)";
            case kSubgroupBallot:
                return "_ = subgroupBallot(true)";
            case kSubgroupElect:
                return "_ = subgroupElect()";
            case kSubgroupBroadcast:
                return "_ = subgroupBroadcast(1.0, 0)";
            case kSubgroupBroadcastFirst:
                return "_ = subgroupBroadcastFirst(1.0)";
            case kSubgroupShuffle:
                return "_ = subgroupShuffle(1.0, 0)";
            case kSubgroupShuffleXor:
                return "_ = subgroupShuffleXor(1.0, 0)";
            case kSubgroupShuffleUp:
                return "_ = subgroupShuffleUp(1.0, 1)";
            case kSubgroupShuffleDown:
                return "_ = subgroupShuffleDown(1.0, 1)";
            case kSubgroupAdd:
                return "_ = subgroupAdd(1.0)";
            case kSubgroupInclusiveAdd:
                return "_ = subgroupInclusiveAdd(1.0)";
            case kSubgroupExclusiveAdd:
                return "_ = subgroupExclusiveAdd(1.0)";
            case kSubgroupMul:
                return "_ = subgroupMul(1.0)";
            case kSubgroupInclusiveMul:
                return "_ = subgroupInclusiveMul(1.0)";
            case kSubgroupExclusiveMul:
                return "_ = subgroupExclusiveMul(1.0)";
            case kSubgroupAnd:
                return "_ = subgroupAnd(1)";
            case kSubgroupOr:
                return "_ = subgroupOr(1)";
            case kSubgroupXor:
                return "_ = subgroupXor(1)";
            case kSubgroupMin:
                return "_ = subgroupMin(1.0)";
            case kSubgroupMax:
                return "_ = subgroupMax(1.0)";
            case kSubgroupAll:
                return "_ = subgroupAll(true)";
            case kSubgroupAny:
                return "_ = subgroupAny(true)";
            case kQuadBroadcast:
                return "_ = quadBroadcast(1.0, 0)";
            case kQuadSwapX:
                return "_ = quadSwapX(1.0)";
            case kQuadSwapY:
                return "_ = quadSwapY(1.0)";
            case kQuadSwapDiagonal:
                return "_ = quadSwapDiagonal(1.0)";
            case kSubgroupMatrixConstruct:
                return "_ = subgroup_matrix_result<f32, 8, 8>()";
            case kSubgroupMatrixLoad:
                return "_ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>("
                       "&subgroup_matrix_data, 0, false, 4)";
            case kSubgroupMatrixStore:
                return "subgroupMatrixStore(&subgroup_matrix_data, 0, "
                       "subgroup_matrix_right_zero, false, 4)";
            case kSubgroupMatrixMultiply:
                return "_ = subgroupMatrixMultiply<f32>("
                       "subgroup_matrix_left_zero, subgroup_matrix_right_zero)";
            case kSubgroupMatrixMultiplyAccumulate:
                return "_ = subgroupMatrixMultiplyAccumulate(subgroup_matrix_left_zero, "
                       "subgroup_matrix_right_zero, subgroup_matrix_result_zero)";
            case kSubgroupMatrixScalarAdd:
                return "_ = subgroupMatrixScalarAdd(subgroup_matrix_left_zero, 4.2f)";
            case kSubgroupMatrixScalarSubtract:
                return "_ = subgroupMatrixScalarSubtract(subgroup_matrix_left_zero, 4.2f)";
            case kSubgroupMatrixScalarMultiply:
                return "_ = subgroupMatrixScalarMultiply(subgroup_matrix_left_zero, 4.2f)";
            case kEndOfFunctionRange:
                return "<invalid>";
        }
        return "<invalid>";
    }

    /// @returns true if `c` is a condition that may be non-uniform.
    static bool MayBeNonUniform(Condition c, Function f) {
        if (c == kSubgroupId && f > kLastNonSubgroupFunction) {
            return false;
        }
        return c > kLastUniformCondition;
    }

    /// @returns true if `f` is a function call that is required to be uniform.
    static bool RequiredToBeUniform(Function f) { return f > kLastNoRestrictionFunction; }

    /// @returns true if `f` is incompatible to be tested with `c`.
    static bool ShouldSkip(Function f, Condition c) {
        return c == kSubgroupId && (f > kLastNonFragmentFunction && f <= kLastNonSubgroupFunction);
    }

    /// Convert a test parameter pair of condition+function to a string that can be used as part of
    /// a test name.
    static std::string ParamsToName(::testing::TestParamInfo<ParamType> params) {
        Condition c = static_cast<Condition>(std::get<0>(params.param));
        Function f = static_cast<Function>(std::get<1>(params.param));
        std::string name;
#define CASE(c)     \
    case c:         \
        name += #c; \
        break

        switch (c) {
            CASE(kTrue);
            CASE(kFalse);
            CASE(kLiteral);
            CASE(kModuleConst);
            CASE(kPipelineOverridable);
            CASE(kFuncLetUniformRhs);
            CASE(kFuncVarUniform);
            CASE(kFuncUniformRetVal);
            CASE(kUniformBuffer);
            CASE(kROStorageBuffer);
            CASE(kFuncLetNonUniformRhs);
            CASE(kFuncVarNonUniform);
            CASE(kFuncNonUniformRetVal);
            CASE(kRWStorageBuffer);
            CASE(kSubgroupId);
            case kEndOfConditionRange:
                break;
        }
        name += "_";
        switch (f) {
            CASE(kUserNoRestriction);
            CASE(kMin);
            CASE(kTextureSampleLevel);
            CASE(kUserRequiredToBeUniform);
            CASE(kWorkgroupBarrier);
            CASE(kStorageBarrier);
            CASE(kTextureBarrier);
            CASE(kWorkgroupUniformLoad);
            CASE(kTextureSample);
            CASE(kTextureSampleBias);
            CASE(kTextureSampleCompare);
            CASE(kDpdx);
            CASE(kDpdxCoarse);
            CASE(kDpdxFine);
            CASE(kDpdy);
            CASE(kDpdyCoarse);
            CASE(kDpdyFine);
            CASE(kFwidth);
            CASE(kFwidthCoarse);
            CASE(kFwidthFine);
            CASE(kSubgroupBallot);
            CASE(kSubgroupElect);
            CASE(kSubgroupBroadcast);
            CASE(kSubgroupBroadcastFirst);
            CASE(kSubgroupShuffle);
            CASE(kSubgroupShuffleXor);
            CASE(kSubgroupShuffleUp);
            CASE(kSubgroupShuffleDown);
            CASE(kSubgroupAdd);
            CASE(kSubgroupInclusiveAdd);
            CASE(kSubgroupExclusiveAdd);
            CASE(kSubgroupMul);
            CASE(kSubgroupInclusiveMul);
            CASE(kSubgroupExclusiveMul);
            CASE(kSubgroupAnd);
            CASE(kSubgroupOr);
            CASE(kSubgroupXor);
            CASE(kSubgroupMin);
            CASE(kSubgroupMax);
            CASE(kSubgroupAll);
            CASE(kSubgroupAny);
            CASE(kQuadBroadcast);
            CASE(kQuadSwapX);
            CASE(kQuadSwapY);
            CASE(kQuadSwapDiagonal);
            CASE(kSubgroupMatrixConstruct);
            CASE(kSubgroupMatrixLoad);
            CASE(kSubgroupMatrixStore);
            CASE(kSubgroupMatrixMultiply);
            CASE(kSubgroupMatrixMultiplyAccumulate);
            CASE(kSubgroupMatrixScalarAdd);
            CASE(kSubgroupMatrixScalarSubtract);
            CASE(kSubgroupMatrixScalarMultiply);
            case kEndOfFunctionRange:
                break;
        }
#undef CASE

        return name;
    }
};

// Test the uniformity constraints for a function call inside a conditional statement.
TEST_P(BasicTest, ConditionalFunctionCall) {
    auto condition = static_cast<Condition>(std::get<0>(GetParam()));
    auto function = static_cast<Function>(std::get<1>(GetParam()));

    if (ShouldSkip(function, condition)) {
        return;
    }

    std::string function_decl;
    if (condition == kSubgroupId) {
        function_decl = R"(
@compute @workgroup_size(16)
fn foo(@builtin(subgroup_id) subgroup_id : u32) {
)";
    } else {
        function_decl = R"(
fn foo() {
)";
    }

    std::string src = R"(
enable subgroups;
enable chromium_experimental_subgroup_matrix;

var<private> p : i32;
var<workgroup> w : i32;
@group(0) @binding(0) var<uniform> u : i32;
@group(0) @binding(0) var<storage, read> ro : i32;
@group(0) @binding(0) var<storage, read_write> rw : i32;

@group(1) @binding(0) var t : texture_2d<f32>;
@group(1) @binding(1) var td : texture_depth_2d;
@group(1) @binding(2) var s : sampler;
@group(1) @binding(3) var sc : sampler_comparison;

@group(2) @binding(0) var<storage, read_write> subgroup_matrix_data : array<f32>;

const module_const : i32 = 42;
@id(42) override pipeline_overridable : i32;

fn user_no_restriction() {}
fn user_required_to_be_uniform() { workgroupBarrier(); }

fn func_uniform_retval() -> i32 { return u; }
fn func_nonuniform_retval() -> i32 { return rw; }

)" + function_decl + R"(
  let let_uniform_rhs = 7;
  let let_nonuniform_rhs = rw;

  var subgroup_matrix_left_zero: subgroup_matrix_left<f32, 8, 8>;
  var subgroup_matrix_right_zero: subgroup_matrix_right<f32, 8, 8>;
  var subgroup_matrix_result_zero: subgroup_matrix_result<f32, 8, 8>;

  var func_uniform = 7;
  var func_non_uniform = 7;
  func_non_uniform = rw;

  if ()" + ConditionToStr(condition) +
                      R"() {
    )" + FunctionCallToStr(function) +
                      R"(;
  }
}
)";

    bool should_pass = !(MayBeNonUniform(condition, function) && RequiredToBeUniform(function));
    RunTest(src, should_pass);
    if (!should_pass) {
        if (function > kLastNonSubgroupFunction) {
            EXPECT_THAT(error_, ::testing::HasSubstr(
                                    "must only be called from subgroup uniform control flow"));
        } else {
            EXPECT_THAT(error_,
                        ::testing::HasSubstr("must only be called from uniform control flow"));
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    UniformityAnalysisTest,
    BasicTest,
    ::testing::Combine(::testing::Range<int>(0, BasicTest::kEndOfConditionRange),
                       ::testing::Range<int>(0, BasicTest::kEndOfFunctionRange)),
    BasicTest::ParamsToName);

////////////////////////////////////////////////////////////////////////////////
/// Test specific function and parameter tags that are not tested above.
////////////////////////////////////////////////////////////////////////////////

TEST_F(UniformityAnalysisTest, ParameterNoRestriction_Pass) {
    // Pass a non-uniform value as an argument, and then try to use the return value for
    // control-flow guarding a barrier.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

var<private> p : i32;

fn foo(i : i32) -> i32 {
  if (i == 0) {
    // This assignment is non-uniform, but shouldn't affect the return value.
    p = 42;
  }
  return 7;
}

fn bar() {
  let x = foo(rw);
  if (x == 7) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ParameterRequiredToBeUniform_Pass) {
    // Pass a uniform value as an argument to a function that uses that parameter for control-flow
    // guarding a barrier.
    std::string src = R"(
@group(0) @binding(0) var<storage, read> ro : i32;

fn foo(i : i32) {
  if (i == 0) {
    workgroupBarrier();
  }
}

fn bar() {
  foo(ro);
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ParameterRequiredToBeUniform_Fail) {
    // Pass a non-uniform value as an argument to a function that uses that parameter for
    // control-flow guarding a barrier.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo(i : i32) {
  if (i == 0) {
    workgroupBarrier();
  }
}

fn bar() {
  foo(rw);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (i == 0) {
  ^^

test:4:8 note: parameter 'i' of 'foo' may be non-uniform
fn foo(i : i32) {
       ^

test:11:7 note: possibly non-uniform value passed here
  foo(rw);
      ^^

test:11:7 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  foo(rw);
      ^^
)");
}

TEST_F(UniformityAnalysisTest, ParameterRequiredToBeUniformForReturnValue_Pass) {
    // Pass a uniform value as an argument to a function that uses that parameter to produce the
    // return value, and then use the return value for control-flow guarding a barrier.
    std::string src = R"(
@group(0) @binding(0) var<storage, read> ro : i32;

fn foo(i : i32) -> i32 {
  return 1 + i;
}

fn bar() {
  if (foo(ro) == 7) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ParameterRequiredToBeUniformForReturnValue_Fail) {
    // Pass a non-uniform value as an argument to a function that uses that parameter to produce the
    // return value, and then use the return value for control-flow guarding a barrier.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo(i : i32) -> i32 {
  return 1 + i;
}

fn bar() {
  if (foo(rw) == 7) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (foo(rw) == 7) {
  ^^

test:9:11 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  if (foo(rw) == 7) {
          ^^
)");
}

////////////////////////////////////////////////////////////////////////////////
/// Test shader IO attributes.
////////////////////////////////////////////////////////////////////////////////

struct BuiltinEntry {
    std::string name;
    std::string type;
    bool uniform;
    BuiltinEntry(std::string n, std::string t, bool u) : name(n), type(t), uniform(u) {}
};

class ComputeBuiltin : public UniformityAnalysisTestBase,
                       public ::testing::TestWithParam<BuiltinEntry> {};
TEST_P(ComputeBuiltin, AsParam) {
    std::string src = R"(
enable subgroups;

@compute @workgroup_size(64)
fn main(@builtin()" + GetParam().name +
                      R"() b : )" + GetParam().type + R"() {
  if (all(vec3(b) == vec3(0u))) {
    workgroupBarrier();
  }
}
)";

    bool should_pass = GetParam().uniform;
    RunTest(src, should_pass);
    if (!should_pass) {
        EXPECT_EQ(
            error_,
            R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:6:3 note: control flow depends on possibly non-uniform value
  if (all(vec3(b) == vec3(0u))) {
  ^^

test:6:16 note: builtin 'b' of 'main' may be non-uniform
  if (all(vec3(b) == vec3(0u))) {
               ^
)");
    }
}

TEST_P(ComputeBuiltin, InStruct) {
    std::string src = R"(
enable subgroups;

struct S {
  @builtin()" + GetParam().name +
                      R"() b : )" + GetParam().type + R"(
}

@compute @workgroup_size(64)
fn main(s : S) {
  if (all(vec3(s.b) == vec3(0u))) {
    workgroupBarrier();
  }
}
)";

    bool should_pass = GetParam().uniform;
    RunTest(src, should_pass);
    if (!should_pass) {
        EXPECT_EQ(
            error_,
            R"(test:11:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:10:3 note: control flow depends on possibly non-uniform value
  if (all(vec3(s.b) == vec3(0u))) {
  ^^

test:10:16 note: parameter 's' of 'main' may be non-uniform
  if (all(vec3(s.b) == vec3(0u))) {
               ^
)");
    }
}

INSTANTIATE_TEST_SUITE_P(UniformityAnalysisTest,
                         ComputeBuiltin,
                         ::testing::Values(BuiltinEntry{"local_invocation_id", "vec3<u32>", false},
                                           BuiltinEntry{"local_invocation_index", "u32", false},
                                           BuiltinEntry{"global_invocation_id", "vec3<u32>", false},
                                           BuiltinEntry{"workgroup_id", "vec3<u32>", true},
                                           BuiltinEntry{"num_subgroups", "u32", true},
                                           BuiltinEntry{"num_workgroups", "vec3<u32>", true},
                                           BuiltinEntry{"subgroup_size", "u32", true}),
                         [](const ::testing::TestParamInfo<ComputeBuiltin::ParamType>& p) {
                             return p.param.name;
                         });

TEST_F(UniformityAnalysisTest, ComputeBuiltin_MixedAttributesInStruct) {
    // Mix both non-uniform and uniform shader IO attributes in the same structure. Even accessing
    // just uniform member causes non-uniformity in this case.
    std::string src = R"(
struct S {
  @builtin(num_workgroups) num_groups : vec3<u32>,
  @builtin(local_invocation_index) idx : u32,
}

@compute @workgroup_size(64)
fn main(s : S) {
  if (s.num_groups.x == 0u) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (s.num_groups.x == 0u) {
  ^^

test:9:7 note: parameter 's' of 'main' may be non-uniform
  if (s.num_groups.x == 0u) {
      ^
)");
}

class FragmentBuiltin : public UniformityAnalysisTestBase,
                        public ::testing::TestWithParam<BuiltinEntry> {};
TEST_P(FragmentBuiltin, AsParam) {
    std::string asScalar = "vec4(b).x";
    std::string src = "";
    if (GetParam().name == "subgroup_size") {
        src += "enable subgroups;\n";
    } else if (GetParam().name == "primitive_index") {
        src += "enable primitive_index;\n";
    } else if (GetParam().name == "barycentric_coord") {
        src += "enable chromium_experimental_barycentric_coord;\n";
        asScalar = "vec3(b).x";
    } else {
        src += "\n";
    }

    src += R"(
@fragment
fn main(@builtin()" +
           GetParam().name + R"() b : )" + GetParam().type + R"() {
  if (u32()" +
           asScalar + R"() == 0u) {
    _ = dpdx(0.5);
  }
}
)";

    bool should_pass = GetParam().uniform;
    RunTest(src, should_pass);
    if (!should_pass) {
        EXPECT_EQ(error_,
                  R"(test:6:9 error: 'dpdx' must only be called from uniform control flow
    _ = dpdx(0.5);
        ^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (u32()" + asScalar +
                      R"() == 0u) {
  ^^

test:5:16 note: builtin 'b' of 'main' may be non-uniform
  if (u32()" + asScalar +
                      R"() == 0u) {
               ^
)");
    }
}

TEST_P(FragmentBuiltin, InStruct) {
    std::string asScalar = "vec4(s.b).x";
    std::string src = "";
    if (GetParam().name == "subgroup_size") {
        src += "enable subgroups;\n";
    } else if (GetParam().name == "primitive_index") {
        src += "enable primitive_index;\n";
    } else if (GetParam().name == "barycentric_coord") {
        src += "enable chromium_experimental_barycentric_coord;\n";
        asScalar = "vec3(s.b).x";
    } else {
        src += "\n";
    }

    src += R"(
struct S {
  @builtin()" +
           GetParam().name + R"() b : )" + GetParam().type + R"(
}

@fragment
fn main(s : S) {
  if (u32()" +
           asScalar + R"() == 0u) {
    _ = dpdx(0.5);
  }
}
)";

    bool should_pass = GetParam().uniform;
    RunTest(src, should_pass);
    if (!should_pass) {
        EXPECT_EQ(error_,
                  R"(test:10:9 error: 'dpdx' must only be called from uniform control flow
    _ = dpdx(0.5);
        ^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (u32()" + asScalar +
                      R"() == 0u) {
  ^^

test:9:16 note: parameter 's' of 'main' may be non-uniform
  if (u32()" + asScalar +
                      R"() == 0u) {
               ^
)");
    }
}

INSTANTIATE_TEST_SUITE_P(UniformityAnalysisTest,
                         FragmentBuiltin,
                         ::testing::Values(BuiltinEntry{"position", "vec4<f32>", false},
                                           BuiltinEntry{"front_facing", "bool", false},
                                           BuiltinEntry{"sample_index", "u32", false},
                                           BuiltinEntry{"sample_mask", "u32", false},
                                           BuiltinEntry{"primitive_index", "u32", false},
                                           BuiltinEntry{"subgroup_size", "u32", false},
                                           BuiltinEntry{"barycentric_coord", "vec3<f32>", false}),
                         [](const ::testing::TestParamInfo<FragmentBuiltin::ParamType>& p) {
                             return p.param.name;
                         });

TEST_F(UniformityAnalysisTest, FragmentLocation) {
    std::string src = R"(
@fragment
fn main(@location(0) l : f32) {
  if (l == 0.0) {
    _ = dpdx(0.5);
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:5:9 error: 'dpdx' must only be called from uniform control flow
    _ = dpdx(0.5);
        ^^^^^^^^^

test:4:3 note: control flow depends on possibly non-uniform value
  if (l == 0.0) {
  ^^

test:4:7 note: user-defined input 'l' of 'main' may be non-uniform
  if (l == 0.0) {
      ^
)");
}

TEST_F(UniformityAnalysisTest, FragmentLocation_InStruct) {
    std::string src = R"(
struct S {
  @location(0) l : f32
}

@fragment
fn main(s : S) {
  if (s.l == 0.0) {
    _ = dpdx(0.5);
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:9 error: 'dpdx' must only be called from uniform control flow
    _ = dpdx(0.5);
        ^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (s.l == 0.0) {
  ^^

test:8:7 note: parameter 's' of 'main' may be non-uniform
  if (s.l == 0.0) {
      ^
)");
}

////////////////////////////////////////////////////////////////////////////////
/// Test subgroup matrix variable declarations.
////////////////////////////////////////////////////////////////////////////////

TEST_F(UniformityAnalysisTest, SubgroupMatrixVarDecl_Pass) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read> ro : i32;

fn foo() {
  if (ro == 0) {
    var sm : subgroup_matrix_result<f32, 8, 8>;
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixVarDecl_Fail) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  if (rw == 0) {
    var sm : subgroup_matrix_result<f32, 8, 8>;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(
        error_,
        R"(test:8:5 error: variables that contain subgroup matrix types cannot be declared in non-uniform control flow
    var sm : subgroup_matrix_result<f32, 8, 8>;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (rw == 0) {
  ^^

test:7:7 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  if (rw == 0) {
      ^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixVarDecl_InStruct_Fail) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

struct Inner {
  u : u32,
  sm : subgroup_matrix_result<f32, 8, 8>,
}

struct S {
  u : u32,
  inner : Inner,
}

@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  if (rw == 0) {
    var sm : S;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(
        error_,
        R"(test:18:5 error: variables that contain subgroup matrix types cannot be declared in non-uniform control flow
    var sm : S;
    ^^^^^^^^^^

test:17:3 note: control flow depends on possibly non-uniform value
  if (rw == 0) {
  ^^

test:17:7 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  if (rw == 0) {
      ^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixVarDecl_InArray_Fail) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

alias ArrayType = array<array<subgroup_matrix_result<f32, 8, 8>, 4>, 4>;

@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  if (rw == 0) {
    var sm : ArrayType;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(
        error_,
        R"(test:10:5 error: variables that contain subgroup matrix types cannot be declared in non-uniform control flow
    var sm : ArrayType;
    ^^^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (rw == 0) {
  ^^

test:9:7 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  if (rw == 0) {
      ^^
)");
}

////////////////////////////////////////////////////////////////////////////////
/// Test loop conditions and conditional break/continue statements.
////////////////////////////////////////////////////////////////////////////////

namespace LoopTest {

enum ControlFlowInterrupt {
    kBreak,
    kContinue,
    kReturn,
};
enum Condition {
    kNone,
    kUniform,
    kNonUniform,
};

using LoopTestParams = std::tuple<int, int>;

static std::string ToStr(ControlFlowInterrupt interrupt) {
    switch (interrupt) {
        case kBreak:
            return "break";
        case kContinue:
            return "continue";
        case kReturn:
            return "return";
    }
    return "";
}

static std::string ToStr(Condition condition) {
    switch (condition) {
        case kNone:
            return "uncondtiional";
        case kUniform:
            return "uniform";
        case kNonUniform:
            return "nonuniform";
    }
    return "";
}

class LoopTest : public UniformityAnalysisTestBase,
                 public ::testing::TestWithParam<LoopTestParams> {
  protected:
    std::string MakeInterrupt(ControlFlowInterrupt interrupt, Condition condition) {
        switch (condition) {
            case kNone:
                return ToStr(interrupt);
            case kUniform:
                return "if (uniform_var == 42) { " + ToStr(interrupt) + "; }";
            case kNonUniform:
                return "if (nonuniform_var == 42) { " + ToStr(interrupt) + "; }";
        }
        return "<invalid>";
    }
};

INSTANTIATE_TEST_SUITE_P(UniformityAnalysisTest,
                         LoopTest,
                         ::testing::Combine(::testing::Range<int>(0, kReturn + 1),
                                            ::testing::Range<int>(0, kNonUniform + 1)),
                         [](const ::testing::TestParamInfo<LoopTestParams>& p) {
                             ControlFlowInterrupt interrupt =
                                 static_cast<ControlFlowInterrupt>(std::get<0>(p.param));
                             auto condition = static_cast<Condition>(std::get<1>(p.param));
                             return ToStr(interrupt) + "_" + ToStr(condition);
                         });

TEST_P(LoopTest, Loop_CallInBody_InterruptAfter) {
    // Test control-flow interrupt in a loop after a function call that requires uniform control
    // flow.
    auto interrupt = static_cast<ControlFlowInterrupt>(std::get<0>(GetParam()));
    auto condition = static_cast<Condition>(std::get<1>(GetParam()));
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_var : i32;
@group(0) @binding(0) var<storage, read_write> nonuniform_var : i32;

fn foo() {
  loop {
    // Pretend that this isn't an infinite loop, in case the interrupt is a
    // continue statement.
    if (false) {
      break;
    }

    workgroupBarrier();
    )" + MakeInterrupt(interrupt, condition) +
                      R"(;
  }
}
)";

    if (condition == kNonUniform) {
        RunTest(src, false);
        EXPECT_THAT(
            error_,
            ::testing::StartsWith(
                R"(test:13:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();)"));
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("test:14:9 note: reading from read_write storage buffer "
                                         "'nonuniform_var' may result in a non-uniform value"));
    } else {
        RunTest(src, true);
    }
}

TEST_P(LoopTest, Loop_CallInBody_InterruptBefore) {
    // Test control-flow interrupt in a loop before a function call that requires uniform control
    // flow.
    auto interrupt = static_cast<ControlFlowInterrupt>(std::get<0>(GetParam()));
    auto condition = static_cast<Condition>(std::get<1>(GetParam()));
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_var : i32;
@group(0) @binding(0) var<storage, read_write> nonuniform_var : i32;

fn foo() {
  loop {
    // Pretend that this isn't an infinite loop, in case the interrupt is a
    // continue statement.
    if (false) {
      break;
    }

    )" + MakeInterrupt(interrupt, condition) +
                      R"(;
    workgroupBarrier();
  }
}
)";

    if (condition == kNonUniform) {
        RunTest(src, false);

        EXPECT_THAT(
            error_,
            ::testing::StartsWith(
                R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();)"));
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("test:13:9 note: reading from read_write storage buffer "
                                         "'nonuniform_var' may result in a non-uniform value"));
    } else {
        RunTest(src, true);
    }
}

TEST_P(LoopTest, Loop_CallInContinuing_InterruptInBody) {
    // Test control-flow interrupt in a loop with a function call that requires uniform control flow
    // in the continuing statement.
    auto interrupt = static_cast<ControlFlowInterrupt>(std::get<0>(GetParam()));
    auto condition = static_cast<Condition>(std::get<1>(GetParam()));
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_var : i32;
@group(0) @binding(0) var<storage, read_write> nonuniform_var : i32;

fn foo() {
  loop {
    // Pretend that this isn't an infinite loop, in case the interrupt is a
    // continue statement.
    if (false) {
      break;
    }

    )" + MakeInterrupt(interrupt, condition) +
                      R"(;
    continuing {
      workgroupBarrier();
    }
  }
}
)";

    if (condition == kNonUniform) {
        RunTest(src, false);
        EXPECT_THAT(
            error_,
            ::testing::StartsWith(
                R"(test:15:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();)"));
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("test:13:9 note: reading from read_write storage buffer "
                                         "'nonuniform_var' may result in a non-uniform value"));
    } else {
        RunTest(src, true);
    }
}

TEST_P(LoopTest, ForUnconditional_CallInBody_InterruptAfter) {
    auto interrupt = static_cast<ControlFlowInterrupt>(std::get<0>(GetParam()));
    auto condition = static_cast<Condition>(std::get<1>(GetParam()));
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_var : i32;
@group(0) @binding(0) var<storage, read_write> nonuniform_var : i32;

fn foo() {
  for ( ; ; ) {
    workgroupBarrier();
    )" + MakeInterrupt(interrupt, condition) +
                      R"(;
  }
}
)";
    const bool infinite_loop = interrupt == kContinue;
    const bool uniformity_failure = condition == kNonUniform;
    const bool expect_pass = !uniformity_failure && !infinite_loop;

    RunTest(src, expect_pass);
    // Check infinite loop first, because uniformity analysis may trigger later.
    if (infinite_loop) {
        EXPECT_FALSE(error_.empty());
    } else if (uniformity_failure) {
        EXPECT_THAT(
            error_,
            ::testing::StartsWith(
                R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();)"));
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("test:8:9 note: reading from read_write storage buffer "
                                         "'nonuniform_var' may result in a non-uniform value"));
    } else {
        EXPECT_TRUE(error_.empty());
    }
}

TEST_P(LoopTest, ForUnconditional_CallInBody_InterruptBefore) {
    auto interrupt = static_cast<ControlFlowInterrupt>(std::get<0>(GetParam()));
    auto condition = static_cast<Condition>(std::get<1>(GetParam()));
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_var : i32;
@group(0) @binding(0) var<storage, read_write> nonuniform_var : i32;

fn foo() {
  for ( ; ; ) {
    )" + MakeInterrupt(interrupt, condition) +
                      R"(;
    workgroupBarrier();
  }
}
)";
    const bool infinite_loop = interrupt == kContinue;
    const bool exit_immediately = (interrupt != kContinue) && condition == kNone;
    const bool uniformity_failure = condition == kNonUniform;
    const bool expect_pass = exit_immediately || (!uniformity_failure && !infinite_loop);

    RunTest(src, expect_pass);
    // Check infinite loop first, because uniformity analysis may trigger later.
    if (infinite_loop) {
        EXPECT_FALSE(error_.empty());
    } else if (uniformity_failure) {
        EXPECT_THAT(
            error_,
            ::testing::StartsWith(
                R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();)"))
            << src;
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("test:7:9 note: reading from read_write storage buffer "
                                         "'nonuniform_var' may result in a non-uniform value"))
            << src;
    } else if (exit_immediately) {
        EXPECT_FALSE(error_.empty());  // Warns that code is unreachable.
    } else {
        EXPECT_TRUE(error_.empty());
    }
}

TEST_P(LoopTest, ForConditional_CallInBody_InterruptAfter) {
    auto interrupt = static_cast<ControlFlowInterrupt>(std::get<0>(GetParam()));
    auto condition = static_cast<Condition>(std::get<1>(GetParam()));
    // The for-loop condition desugars to a conditional break, so the infinite
    // loop check will not trigger.
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_var : i32;
@group(0) @binding(0) var<storage, read_write> nonuniform_var : i32;

fn foo() {
  for ( ; true ; ) {
    workgroupBarrier();
    )" + MakeInterrupt(interrupt, condition) +
                      R"(;
  }
}
)";
    const bool uniformity_failure = condition == kNonUniform;
    const bool expect_pass = !uniformity_failure;

    RunTest(src, expect_pass);
    if (uniformity_failure) {
        EXPECT_THAT(
            error_,
            ::testing::StartsWith(
                R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();)"));
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("test:8:9 note: reading from read_write storage buffer "
                                         "'nonuniform_var' may result in a non-uniform value"));
    } else {
        EXPECT_TRUE(error_.empty());
    }
}

TEST_P(LoopTest, ForConditional_CallInBody_InterruptBefore) {
    auto interrupt = static_cast<ControlFlowInterrupt>(std::get<0>(GetParam()));
    auto condition = static_cast<Condition>(std::get<1>(GetParam()));
    // The for-loop condition desugars to a conditional break, so the infinite
    // loop check will not trigger.
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_var : i32;
@group(0) @binding(0) var<storage, read_write> nonuniform_var : i32;

fn foo() {
  for ( ; true ; ) {
    )" + MakeInterrupt(interrupt, condition) +
                      R"(;
    workgroupBarrier();
  }
}
)";
    // The barrier is unreachable whenever the interruption is unconditional:
    // - a continue will jump over the barrier to the end of the iteration.
    // - a break or return will exit the loop entirely.
    const bool barrier_unreachable = condition == kNone;
    const bool exit_immediately = (condition == kNone) && (interrupt != kContinue);
    const bool uniformity_failure = condition == kNonUniform;
    const bool expect_pass = exit_immediately || !uniformity_failure;

    RunTest(src, expect_pass);
    if (uniformity_failure) {
        EXPECT_THAT(
            error_,
            ::testing::StartsWith(
                R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();)"))
            << src;
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("test:7:9 note: reading from read_write storage buffer "
                                         "'nonuniform_var' may result in a non-uniform value"))
            << src;
    } else if (barrier_unreachable) {
        EXPECT_FALSE(error_.empty());  // Warns that code is unreachable.
    } else {
        EXPECT_TRUE(error_.empty()) << error_ << src;
    }
}

TEST_P(LoopTest, While_CallInBody_InterruptAfter) {
    auto interrupt = static_cast<ControlFlowInterrupt>(std::get<0>(GetParam()));
    auto condition = static_cast<Condition>(std::get<1>(GetParam()));
    // The while-loop condition desugars to a conditional break, so the infinite
    // loop check will not trigger.
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_var : i32;
@group(0) @binding(0) var<storage, read_write> nonuniform_var : i32;

fn foo() {
  while ( true ) {
    workgroupBarrier();
    )" + MakeInterrupt(interrupt, condition) +
                      R"(;
  }
}
)";
    const bool uniformity_failure = condition == kNonUniform;
    const bool expect_pass = !uniformity_failure;

    RunTest(src, expect_pass);
    if (uniformity_failure) {
        EXPECT_THAT(
            error_,
            ::testing::StartsWith(
                R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();)"));
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("test:8:9 note: reading from read_write storage buffer "
                                         "'nonuniform_var' may result in a non-uniform value"));
    } else {
        EXPECT_TRUE(error_.empty());
    }
}

TEST_P(LoopTest, While_CallInBody_InterruptBefore) {
    auto interrupt = static_cast<ControlFlowInterrupt>(std::get<0>(GetParam()));
    auto condition = static_cast<Condition>(std::get<1>(GetParam()));
    // The while-loop condition desugars to a conditional break, so the infinite
    // loop check will not trigger.
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_var : i32;
@group(0) @binding(0) var<storage, read_write> nonuniform_var : i32;

fn foo() {
  for ( ; true ; ) {
    )" + MakeInterrupt(interrupt, condition) +
                      R"(;
    workgroupBarrier();
  }
}
)";
    // The barrier is unreachable whenever the interruption is unconditional:
    // - a continue will jump over the barrier to the end of the iteration.
    // - a break or return will exit the loop entirely.
    const bool barrier_unreachable = condition == kNone;
    const bool exit_immediately = (condition == kNone) && (interrupt != kContinue);
    const bool uniformity_failure = condition == kNonUniform;
    const bool expect_pass = exit_immediately || !uniformity_failure;

    RunTest(src, expect_pass);
    if (uniformity_failure) {
        EXPECT_THAT(
            error_,
            ::testing::StartsWith(
                R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();)"))
            << src;
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("test:7:9 note: reading from read_write storage buffer "
                                         "'nonuniform_var' may result in a non-uniform value"))
            << src;
    } else if (barrier_unreachable) {
        EXPECT_FALSE(error_.empty());  // Warns that code is unreachable.
    } else {
        EXPECT_TRUE(error_.empty()) << error_ << src;
    }
}

TEST_F(UniformityAnalysisTest, Loop_CallInBody_UniformBreakInContinuing) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> n : i32;

fn foo() {
  var i = 0;
  loop {
    workgroupBarrier();
    continuing {
      i = i + 1;
      break if (i == n);
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_CallInBody_NonUniformBreakInContinuing) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  var i = 0;
  loop {
    workgroupBarrier();
    continuing {
      i = i + 1;
      break if (i == n);
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:10:7 note: control flow depends on possibly non-uniform value
      break if (i == n);
      ^^^^^

test:10:22 note: reading from read_write storage buffer 'n' may result in a non-uniform value
      break if (i == n);
                     ^
)");
}

TEST_F(UniformityAnalysisTest, Loop_CallInContinuing_UniformBreakInContinuing) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> n : i32;

fn foo() {
  var i = 0;
  loop {
    continuing {
      workgroupBarrier();
      i = i + 1;
      break if (i == n);
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_CallInContinuing_NonUniformBreakInContinuing) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  var i = 0;
  loop {
    continuing {
      workgroupBarrier();
      i = i + 1;
      break if (i == n);
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:10:7 note: control flow depends on possibly non-uniform value
      break if (i == n);
      ^^^^^

test:10:22 note: reading from read_write storage buffer 'n' may result in a non-uniform value
      break if (i == n);
                     ^
)");
}

class LoopDeadCodeTest : public UniformityAnalysisTestBase, public ::testing::TestWithParam<int> {};

INSTANTIATE_TEST_SUITE_P(UniformityAnalysisTest,
                         LoopDeadCodeTest,
                         ::testing::Range<int>(0, kReturn + 1),
                         [](const ::testing::TestParamInfo<LoopDeadCodeTest::ParamType>& p) {
                             return ToStr(static_cast<ControlFlowInterrupt>(p.param));
                         });

TEST_P(LoopDeadCodeTest, Loop_AfterInterrupt) {
    // Dead code after a control-flow interrupt in a loop shouldn't cause uniformity errors.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  loop {
    )" + ToStr(static_cast<ControlFlowInterrupt>(GetParam())) +
                      R"(;
    if (n == 42) {
      workgroupBarrier();
    }
    continuing {
      // Pretend that this isn't an infinite loop, in case the interrupt is a
      // continue statement.
      break if (false);
    }
  }
}
)";

    RunTest(src, true);
}

TEST_P(LoopDeadCodeTest, ForWithCond_AfterInterrupt) {
    // Dead code after a control-flow interrupt in a loop shouldn't cause uniformity errors.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  for ( ; true ; ) {
    )" + ToStr(static_cast<ControlFlowInterrupt>(GetParam())) +
                      R"(;
    if (n == 42) {
      workgroupBarrier();
    }
  }
}
)";

    RunTest(src, true);
}

TEST_P(LoopDeadCodeTest, ForWithoutCond_AfterInterrupt) {
    // Dead code after a control-flow interrupt in a loop shouldn't cause uniformity errors.
    const auto interrupt = static_cast<ControlFlowInterrupt>(GetParam());
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  for ( ; ; ) {
    )" + ToStr(interrupt) +
                      R"(;
    if (n == 42) {
      workgroupBarrier();
    }
  }
}
)";

    // The loop is infinite if the interrupt is kContinue, and rejected early.
    if (interrupt != kContinue) {
        RunTest(src, true);
    }
}

TEST_P(LoopDeadCodeTest, While_AfterInterrupt) {
    // Dead code after a control-flow interrupt in a loop shouldn't cause uniformity errors.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  while (true) {
    )" + ToStr(static_cast<ControlFlowInterrupt>(GetParam())) +
                      R"(;
    if (n == 42) {
      workgroupBarrier();
    }
  }
}
)";

    RunTest(src, true);
}

TEST_P(LoopDeadCodeTest, Loop_AfterLoop_WithBreakIf) {
    // Code after a return-only loop body should not cause uniformity errors.
    const auto interrupt = static_cast<ControlFlowInterrupt>(GetParam());
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  loop {
    )" + ToStr(interrupt) +
                      R"(;
    continuing {
      // Pretend that this isn't an infinite loop, in case the interrupt is a
      // continue statement.
      break if (false);
    }
  }
  if (n == 42) {
    workgroupBarrier();
  }
}
)";

    const bool should_pass = interrupt == kReturn;
    RunTest(src, should_pass);
    if (!should_pass) {
        EXPECT_EQ(
            error_,
            R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (n == 42) {
  ^^

test:13:7 note: reading from read_write storage buffer 'n' may result in a non-uniform value
  if (n == 42) {
      ^
)");
    }
}

TEST_P(LoopDeadCodeTest, Loop_AfterLoop_WithoutBreakIf) {
    // Code after a return-only loop body should not cause uniformity errors.
    const auto interrupt = static_cast<ControlFlowInterrupt>(GetParam());
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  loop {
    )" + ToStr(interrupt) +
                      R"(;
    continuing { }
  }
  if (n == 42) {
    workgroupBarrier();
  }
}
)";
    // The loop is infinite if the interrupt is kContinue, and rejected early.
    if (interrupt != kContinue) {
        const bool should_pass = interrupt == kReturn;
        RunTest(src, should_pass);
        if (!should_pass) {
            EXPECT_EQ(
                error_,
                R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (n == 42) {
  ^^

test:9:7 note: reading from read_write storage buffer 'n' may result in a non-uniform value
  if (n == 42) {
      ^
)");
        }
    }
}

TEST_P(LoopDeadCodeTest, ForWithCond_AfterLoop) {
    // The for loop with a condition is desguared as:
    //    loop { if ! cond { break; } ... }
    // So the for loop behaviour always includes kNext, and code
    // after the for loop is always considered reachable by uniformity
    // analysis.
    const auto interrupt = static_cast<ControlFlowInterrupt>(GetParam());
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  for ( ; true ; ) {
    )" + ToStr(interrupt) +
                      R"(;
  }
  if (n == 42) {
    workgroupBarrier();
  }
}
)";

    // The loop is infinite if the interrupt is kContinue, and rejected early.
    if (interrupt != kContinue) {
        RunTest(src, false);
        EXPECT_EQ(
            error_,
            R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (n == 42) {
  ^^

test:8:7 note: reading from read_write storage buffer 'n' may result in a non-uniform value
  if (n == 42) {
      ^
)");
    }
}

TEST_P(LoopDeadCodeTest, ForWithoutCond_AfterLoop) {
    // Code after a return-only loop body should not cause uniformity errors.
    const auto interrupt = static_cast<ControlFlowInterrupt>(GetParam());
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  for ( ; ; ) {
    )" + ToStr(interrupt) +
                      R"(;
  }
  if (n == 42) {
    workgroupBarrier();
  }
}
)";

    // The loop is infinite if the interrupt is kContinue, and rejected early.
    if (interrupt != kContinue) {
        const bool should_pass = interrupt == kReturn;
        RunTest(src, should_pass);
        if (!should_pass) {
            EXPECT_EQ(
                error_,
                R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (n == 42) {
  ^^

test:8:7 note: reading from read_write storage buffer 'n' may result in a non-uniform value
  if (n == 42) {
      ^
)");
        }
    }
}

TEST_P(LoopDeadCodeTest, While_AfterLoop) {
    // The while loop is desguared as:
    //    loop { if ! cond { break; } ... }
    // So the while loop behaviour always includes kNext, and code
    // after the while loop is always considered reachable by uniformity
    // analysis.
    const auto interrupt = static_cast<ControlFlowInterrupt>(GetParam());
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  while ( true ) {
    )" + ToStr(interrupt) +
                      R"(;
  }
  if (n == 42) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (n == 42) {
  ^^

test:8:7 note: reading from read_write storage buffer 'n' may result in a non-uniform value
  if (n == 42) {
      ^
)");
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesNonUniformInLoopAfterBarrier) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable later in that loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       Loop_VarBecomesNonUniformInLoopAfterBarrier_ContinueAtEnd_NoContinuing) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable later in that loop. The loop ends with a continue statement, without a
    // continuing block.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    continue;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       Loop_VarBecomesNonUniformInLoopAfterBarrier_ContinueAtEnd_EmptyContinuing) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable later in that loop. The loop ends with a continue statement, without a
    // continuing block.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    continue;

    continuing {
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesNonUniformInLoopAfterBarrier_BreakAtEnd) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable later in that loop. End the loop with a break statement to prevent the
    // non-uniform value from causing an issue.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v == 0) {
      workgroupBarrier();
    }

    v = non_uniform;
    break;
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       Loop_VarBecomesNonUniformInLoopContinuing_BarrierInLoop_BodyHasBehaviorNext) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable in the continuing block. The behavior of the body is `next`.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v > 0) {
      workgroupBarrier();
    }
    continuing {
      v = non_uniform;
      break if false;
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v > 0) {
    ^^

test:11:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       Loop_VarBecomesNonUniformInLoopContinuing_BarrierInLoop_BodyHasBehaviorContinue) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable in the continuing block. The behavior of the body is `continue`.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v > 0) {
      workgroupBarrier();
    }
    continue;
    continuing {
      v = non_uniform;
      break if false;
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v > 0) {
    ^^

test:12:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       Loop_VarBecomesUniformInLoopContinuing_BarrierInLoop_BodyHasBehaviorContinue) {
    // Use a variable for a conditional barrier in a loop, and then assign a uniform value to
    // that variable in the continuing block. The behavior of the body is `continue`.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v > 0) {
      workgroupBarrier();
    }
    v = non_uniform;
    continue;
    continuing {
      v = 1;
      break if false;
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_ConditionalAssignNonUniformWithBreak_BarrierInLoop) {
    // In a conditional block, assign a non-uniform value and then break, then use a variable for a
    // conditional barrier later in the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (true) {
      v = non_uniform;
      break;
    }
    if (v == 0) {
      workgroupBarrier();
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_ConditionalAssignNonUniformWithConditionalBreak_BarrierInLoop) {
    // In a conditional block, assign a non-uniform value and then conditionally break, then use a
    // variable for a conditional barrier later in the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (true) {
      v = non_uniform;
      if (true) {
        break;
      }
    }
    if (v == 0) {
      workgroupBarrier();
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:13:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:8:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Loop_ConditionalAssignNonUniformWithBreak_BarrierAfterLoop) {
    // In a conditional block, assign a non-uniform value and then break, then use a variable for a
    // conditional barrier after the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (true) {
      v = non_uniform;
      break;
    }
    v = 5;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:15:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:14:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:8:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesUniformBeforeSomeExits_BarrierAfterLoop) {
    // Assign a non-uniform value, have two exit points only one of which assigns a uniform value,
    // then use a variable for a conditional barrier after the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (true) {
      break;
    }

    v = non_uniform;

    if (false) {
      v = 6;
      break;
    }
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:20:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:19:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:11:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesUniformBeforeAllExits_BarrierAfterLoop) {
    // Assign a non-uniform value, have two exit points both of which assigns a uniform value,
    // then use a variable for a conditional barrier after the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (true) {
      v = 5;
      break;
    }

    v = non_uniform;

    if (false) {
      v = 6;
      break;
    }
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_AssignNonUniformBeforeConditionalBreak_BarrierAfterLoop) {
    // Assign a non-uniform value and then break in a conditional block, then use a variable for a
    // conditional barrier after the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    v = non_uniform;
    if (true) {
      if (false) {
        v = 5;
      } else {
        break;
      }
      v = 5;
    }
    v = 5;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:20:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:19:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:7:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesNonUniformBeforeConditionalContinue_BarrierAtStart) {
    // Use a variable for a conditional barrier in a loop, assign a non-uniform value to
    // that variable later in that loop, then perform a conditional continue before assigning a
    // uniform value to that variable.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    if (true) {
      continue;
    }

    v = 5;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       Loop_VarBecomesUniformBeforeConditionalContinue_BarrierInContinuing) {
    // Use a variable for a conditional barrier in the continuing statement of a loop, assign a
    // non-uniform value to that variable later in that loop, then conditionally assign a uniform
    // value before continuing.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    v = non_uniform;

    if (false) {
      v = 5;
      continue;
    }

    continuing {
      if (v == 0) {
        workgroupBarrier();
      }
      break if (true);
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:16:9 error: 'workgroupBarrier' must only be called from uniform control flow
        workgroupBarrier();
        ^^^^^^^^^^^^^^^^

test:15:7 note: control flow depends on possibly non-uniform value
      if (v == 0) {
      ^^

test:7:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesUniformAfterConditionalContinue_BarrierInContinuing) {
    // Use a variable for a conditional barrier in the continuing statement of a loop, assign a
    // non-uniform value to that variable before a conditional continue, and then unconditionally
    // assign a uniform value before the end of the body.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  loop {
    var v = 0;
    if (true) {
      v = non_uniform;
      continue;
    }
    v = 0;
    continuing {
      if (v == 0) {
        workgroupBarrier();
      }
      break if true;
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:9 error: 'workgroupBarrier' must only be called from uniform control flow
        workgroupBarrier();
        ^^^^^^^^^^^^^^^^

test:13:7 note: control flow depends on possibly non-uniform value
      if (v == 0) {
      ^^

test:8:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesNonUniformBeforeBreak_BarrierInContinuing) {
    // Use a variable for a conditional barrier in the continuing statement of a loop that has a
    // continue statement. Assign a non-uniform value to that variable before a break statement.
    // The non-uniform value will never get to the continuing block, so there should be no error.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (true) {
      continue;
    }

    v = non_uniform;
    break;

    continuing {
      if (v == 0) {
        workgroupBarrier();
      }
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesNonUniformBeforeReturn_BarrierInContinuing) {
    // Use a variable for a conditional barrier in the continuing statement of a loop that has a
    // continue statement. Assign a non-uniform value to that variable before a return statement.
    // The non-uniform value will never get to the continuing block, so there should be no error.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (true) {
      continue;
    }

    v = non_uniform;
    return;

    continuing {
      if (v == 0) {
        workgroupBarrier();
      }
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesNonUniformBeforeConditionalContinue) {
    // Use a variable for a conditional barrier in a loop, assign a non-uniform value to
    // that variable later in that loop, then perform a conditional continue before assigning a
    // uniform value to that variable.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    if (true) {
      continue;
    }

    v = 5;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Loop_VarBecomesNonUniformInNestedLoopWithBreak_BarrierInLoop) {
    // Use a variable for a conditional barrier in a loop, then conditionally assign a non-uniform
    // value to that variable followed by a break in a nested loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    loop {
      if (true) {
        v = non_uniform;
        break;
      }
      v = 5;
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:14:13 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
        v = non_uniform;
            ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       Loop_VarBecomesNonUniformInNestedLoopWithBreak_BecomesUniformAgain_BarrierAfterLoop) {
    // Conditionally assign a non-uniform value followed by a break in a nested loop, assign a
    // uniform value in the outer loop, and then use a variable for a conditional barrier after the
    // loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  loop {
    if (false) {
      break;
    }

    loop {
      if (true) {
        v = non_uniform;
        break;
      }
    }
    v = 5;
  }
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_NonUniformValueNeverReachesContinuing) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  loop {
    var v = non_uniform;
    return;

    continuing {
      if (v == 0) {
        workgroupBarrier();
      }
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_NonUniformValueDeclaredInBody_UnreachableContinuing) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var condition = true;
  loop {
    var v = non_uniform;
    if (condition) {
      break;
    } else {
      break;
    }

    continuing {
      if (v == 0) {
        workgroupBarrier();
      }
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_NonUniformValueDeclaredInBody_MaybeReachesContinuing) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var condition = true;
  loop {
    var v = non_uniform;
    if (condition) {
      continue;
    } else {
      break;
    }

    continuing {
      if (v == 0) {
        workgroupBarrier();
      }
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:16:9 error: 'workgroupBarrier' must only be called from uniform control flow
        workgroupBarrier();
        ^^^^^^^^^^^^^^^^

test:15:7 note: control flow depends on possibly non-uniform value
      if (v == 0) {
      ^^

test:7:13 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    var v = non_uniform;
            ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Loop_NonUniformBreakInBody_Reconverge) {
    // Loops reconverge at exit, so test that we can call workgroupBarrier() after a loop that
    // contains a non-uniform conditional break.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  var i = 0;
  loop {
    if (i == n) {
      break;
    }
    i = i + 1;
  }
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Loop_ScopedVarDeclaredBeforeContinue) {
    // Check that we can handle a var being declared in a nested scope before a continue statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var i = 0;
  loop {
    if (true) {
      var i = non_uniform;
      continue;
    }
    continuing {
      if (i < 5) {
        workgroupBarrier();
      }
      i++;
      break if i >= 10;
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_CallInside_UniformCondition) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> n : i32;

fn foo() {
  for (var i = 0; i < n; i = i + 1) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_CallInside_NonUniformCondition) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  for (var i = 0; i < n; i = i + 1) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  for (var i = 0; i < n; i = i + 1) {
  ^^^

test:5:23 note: reading from read_write storage buffer 'n' may result in a non-uniform value
  for (var i = 0; i < n; i = i + 1) {
                      ^
)");
}

TEST_F(UniformityAnalysisTest, ForLoop_VarBecomesNonUniformInContinuing_BarrierInLoop) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable in the continuing statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; v = non_uniform) {
    if (v == 0) {
      workgroupBarrier();
      break;
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:6:31 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  for (var i = 0; i < 10; v = non_uniform) {
                              ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, ForLoop_VarBecomesUniformInContinuing_BarrierInLoop) {
    // Use a variable for a conditional barrier in a loop, and then assign a uniform value to that
    // variable in the continuing statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; v = 5) {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       ForLoop_VarBecomesUniformInContinuing_BarrierInLoop_BodyHasContinueBehavior) {
    // Use a variable for a conditional barrier in a loop, and then assign a uniform value to that
    // variable in the continuing statement. The body of the loop has the `continue` behavior.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; v = 5) {
    if (v == 0) {
      workgroupBarrier();
    }

    v = non_uniform;
    continue;
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_VarBecomesNonUniformInContinuing_BarrierAfterLoop) {
    // Use a variable for a conditional barrier after a loop, and assign a non-uniform value to
    // that variable in the continuing statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; v = non_uniform) {
    v = 5;
  }
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:6:31 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  for (var i = 0; i < 10; v = non_uniform) {
                              ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, ForLoop_VarBecomesUniformInContinuing_BarrierAfterLoop) {
    // Use a variable for a conditional barrier after a loop, and assign a uniform value to that
    // variable in the continuing statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; v = 5) {
    v = non_uniform;
  }
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       ForLoop_VarBecomesUniformInContinuing_BarrierAfterLoop_BodyHasContinueBehavior) {
    // Use a variable for a conditional barrier after a loop, and assign a uniform value to that
    // variable in the continuing statement. The body of the loop has the `continue` behavior;
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; v = 5) {
    v = non_uniform;
    continue;
  }
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_VarBecomesNonUniformInLoopAfterBarrier) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable later in that loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; i++) {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, ForLoop_VarBecomesNonUniformInLoopAfterBarrier_BeforeContinue) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable later in that loop, before a continue statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; i++) {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    continue;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       ForLoop_VarBecomesNonUniformInLoopAfterBarrier_BeforeContinue_NoContinuing) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable later in that loop, before a continue statement. The for loop has no continuing
    // statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (;;) {
    if (v != 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    continue;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v != 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, ForLoop_ConditionalAssignNonUniformWithBreak_BarrierInLoop) {
    // In a conditional block, assign a non-uniform value and then break, then use a variable for a
    // conditional barrier later in the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; i++) {
    if (true) {
      v = non_uniform;
      break;
    }
    if (v == 0) {
      workgroupBarrier();
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_ConditionalAssignNonUniformWithBreak_BarrierAfterLoop) {
    // In a conditional block, assign a non-uniform value and then break, then use a variable for a
    // conditional barrier after the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; i++) {
    if (true) {
      v = non_uniform;
      break;
    }
    v = 5;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:15:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:14:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:8:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, ForLoop_VarRemainsNonUniformAtLoopEnd_BarrierAfterLoop) {
    // Assign a non-uniform value, assign a uniform value before all explicit break points but leave
    // the value non-uniform at loop exit, then use a variable for a conditional barrier after the
    // loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; i++) {
    if (true) {
      v = 5;
      break;
    }

    v = non_uniform;

    if (true) {
      v = 6;
      break;
    }
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:21:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:20:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       ForLoop_VarBecomesNonUniformBeforeConditionalContinue_BarrierAtStart) {
    // Use a variable for a conditional barrier in a loop, assign a non-uniform value to
    // that variable later in that loop, then perform a conditional continue before assigning a
    // uniform value to that variable.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; i++) {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    if (true) {
      continue;
    }

    v = 5;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, ForLoop_VarBecomesNonUniformBeforeConditionalContinue) {
    // Use a variable for a conditional barrier in a loop, assign a non-uniform value to
    // that variable later in that loop, then perform a conditional continue before assigning a
    // uniform value to that variable.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  for (var i = 0; i < 10; i++) {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    if (true) {
      continue;
    }

    v = 5;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:7:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:12:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       ForLoop_InitializerVarBecomesNonUniformBeforeConditionalContinue_BarrierAtStart) {
    // Use a variable declared in a for-loop initializer for a conditional barrier in a loop, assign
    // a non-uniform value to that variable later in that loop and then execute a continue.
    // Tests that variables declared in the for-loop initializer are properly tracked.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  for (var i = 0; i < 10; i++) {
    if (i < 5) {
      workgroupBarrier();
    }
    if (true) {
      i = non_uniform;
      continue;
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  for (var i = 0; i < 10; i++) {
  ^^^

test:10:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      i = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, ForLoop_NonUniformBarrierInUnreachableContinuingStatement) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(i : i32) {
  if (i == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  for (;; bar(non_uniform)) {
    break;
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_VarBecomesUniformBeforeContinue_BarrierInContinuing) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(i : i32) {
  if (i == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  var v = non_uniform;
  for (;; bar(v)) {
    if (true) {
      v = 0;
      continue;
    }
    break;
  }

}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_NonUniformCondition_Reconverge) {
    // Loops reconverge at exit, so test that we can call workgroupBarrier() after a loop that has a
    // non-uniform condition.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  for (var i = 0; i < n; i = i + 1) {
  }
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_VarDeclaredInBody) {
    // Make sure that we can declare a variable inside the loop body without causing issues for
    // tracking local variables across iterations.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  var outer : i32;
  for (var i = 0; i < n; i = i + 1) {
    var inner : i32;
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_InitializerScope) {
    // Make sure that variables declared in a for-loop initializer are properly removed from the
    // local variable list, otherwise a parent control-flow statement will try to add edges to nodes
    // that no longer exist.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  if (n == 5) {
    for (var i = 0; i < n; i = i + 1) {
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ForLoop_ScopedVarDeclaredBeforeContinue) {
    // Check that we can handle a var being declared in a nested scope before a continue statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  for (var i = 0; i < 10; i++) {
    if (i < 5) {
      workgroupBarrier();
    }
    if (true) {
      var i = non_uniform;
      continue;
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, While_CallInside_UniformCondition) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> n : i32;

fn foo() {
  var i = 0;
  while (i < n) {
    workgroupBarrier();
    i = i + 1;
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, While_CallInside_NonUniformCondition) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  var i = 0;
  while (i < n) {
    workgroupBarrier();
    i = i + 1;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:6:3 note: control flow depends on possibly non-uniform value
  while (i < n) {
  ^^^^^

test:6:14 note: reading from read_write storage buffer 'n' may result in a non-uniform value
  while (i < n) {
             ^
)");
}

TEST_F(UniformityAnalysisTest, While_VarBecomesNonUniformInLoopAfterBarrier) {
    // Use a variable for a conditional barrier in a loop, and then assign a non-uniform value to
    // that variable later in that loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  var i = 0;
  while (i < 10) {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    i++;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:8:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:13:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, While_ConditionalAssignNonUniformWithBreak_BarrierInLoop) {
    // In a conditional block, assign a non-uniform value and then break, then use a variable for a
    // conditional barrier later in the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  var i = 0;
  while (i < 10) {
    if (true) {
      v = non_uniform;
      break;
    }
    if (v == 0) {
      workgroupBarrier();
    }
    i++;
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, While_ConditionalAssignNonUniformWithBreak_BarrierAfterLoop) {
    // In a conditional block, assign a non-uniform value and then break, then use a variable for a
    // conditional barrier after the loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  var i = 0;
  while (i < 10) {
    if (true) {
      v = non_uniform;
      break;
    }
    v = 5;
    i++;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:17:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:16:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:9:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, While_VarRemainsNonUniformAtLoopEnd_BarrierAfterLoop) {
    // Assign a non-uniform value, assign a uniform value before all explicit break points but leave
    // the value non-uniform at loop exit, then use a variable for a conditional barrier after the
    // loop.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  var i = 0;
  while (i < 10) {
    if (true) {
      v = 5;
      break;
    }

    v = non_uniform;

    if (true) {
      v = 6;
      break;
    }
    i++;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:23:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:22:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:13:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, While_VarBecomesNonUniformBeforeConditionalContinue_BarrierAtStart) {
    // Use a variable for a conditional barrier in a loop, assign a non-uniform value to
    // that variable later in that loop, then perform a conditional continue before assigning a
    // uniform value to that variable.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  var i = 0;
  while (i < 10) {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    if (true) {
      continue;
    }

    v = 5;
    i++;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:8:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:13:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, While_VarBecomesNonUniformBeforeConditionalContinue) {
    // Use a variable for a conditional barrier in a loop, assign a non-uniform value to
    // that variable later in that loop, then perform a conditional continue before assigning a
    // uniform value to that variable.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  var i = 0;
  while (i < 10) {
    if (v == 0) {
      workgroupBarrier();
      break;
    }

    v = non_uniform;
    if (true) {
      continue;
    }

    v = 5;
    i++;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:8:5 note: control flow depends on possibly non-uniform value
    if (v == 0) {
    ^^

test:13:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
    v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, While_NonUniformCondition_Reconverge) {
    // Loops reconverge at exit, so test that we can call workgroupBarrier() after a loop that has a
    // non-uniform condition.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> n : i32;

fn foo() {
  var i = 0;
  while (i < n) {
  }
  workgroupBarrier();
  i = i + 1;
}
)";

    RunTest(src, true);
}

// We need to enumerate the potential uniformity graphs, as defined
// in the WGSL spec. Some edges always exist, and sometimes they exist
// only if the behaviour of the loop body has certain behaviours.
//
// Analyzing loop { }, and loop { continuing { } }
//
// To test analysis of loops, we place code fragments in various key spots to:
//  - attack: generate non-uniform control flow (or not)
//  - detect: potentially observe non-uniform control flow (or not)
//  - a uniformity error should result if and only if there is a path
//    in the uniformity graph from the detector to the attacker.
//
// Define labels A, B, C, D for locations of statements:
//   A: before the loop
//   B: in the loop body
//   C: in the continuing statement
//   D: after the loop
//
// Without loss of generality, the attacker must occur dynamically before the
// detector.  We avoid generating cases that are covered by uniformity tests
// for straight-line code, and therefore avoid:
//  - when A is both attacker and detector
//  - when D is both attacker and detector
//  - when the attacker appears before the detector in B or C
//
//  The scenarios are as enumerated as pairs (attacker,detector) as follows:
//   loop without continuing block:
//    (A,B)  (A,D)
//    (B,B)  (B,D)
//
//   loop with continuing block:
//    (A,B)  (A,C)  (A,D)
//    (B,B)  (B,C)  (B,D)
//    (C,B)  (C,C)  (C,D)
//
//  Note: The cases (B,B), (C,B), (C,C) transmit non-uniformity via
//    the loop backedge.
//
// The detecting code fragment is always a call to workgroupBarrier().
// The attacking code fragment changes depending on where it is:
//  A:   if nonuniform_condition { return; }
//  B:   if nonuniform_condition { break; }
//  C:   break if nonuniform-condition;
//
//
// The attacker code fragment
//
// Nodes in the uniformity graph are:
//   CF: node before the loop
//   CF': node at the top of the loop body
//   CF1: node after statements in the body
//   CF2: node after statements in the continuing block
//   CFResult: resulting control flow node, after the loop.
//     Not named directly in the spec, but identified with the
//     node listed as the "resulting node" in the spec.
//
// Schematically the loops take these forms:
//
//  without continuing block:
//    // A
//    // CF
//    loop {
//         // CF'
//      s1 // B
//         // CF1
//    }
//    // D
//    // CFResult
//
//
//  with continuing block:
//    // A
//    // CF
//    loop {
//         // CF'
//      s1 // B
//         // CF1
//      continuing {
//        s2 // C
//           // CF2
//    }
//    // D
//    // CFResult
//
//
// The potential graphs including conditional edges are:
//
//  without continuing block:
//
//                   ?
//               +------------+
//               |            v
//    CF  <---  CF' <- s1 <- CF1
//    ^                       ^
//    |?                      |
//    CFResult ---------------+
//                   ?
//
//    CF' -> CF1 if Next or Continue in s1's behaviour.
//
//    CFResult -> CF1 if Return in s1's behaviour
//            else CFResult -> CF
//
//
//  with continuing block:
//
//   when behaviour of s1={Break}
//
//    CF <- s1 <- CF1
//    ^
//    |
//    CFResult
//
//   when behaviour of s1={Return} or {Break,Return}
//
//    CF <- s1 <- CF1
//                 ^
//                 |
//    CFResult ----+
//
//   else, i.e. behaviour of s1 includes Next or Continue
//                       ?
//               +------------------------+
//               |                        v
//    CF  <---  CF' <- s1 <- CF1 <- s2 <-CF2
//    ^          ^
//    |?         |?
//    CFResult --+
//
//
//    CF' -> CF2 if Next or Continue in s1's behaviour.
//
//    CFResult -> CF' if Return in s1's behaviour
//            else CFResult -> CF
//
// We'll use a concise representation of each tested scenario.
// A scenario is represented by a positional sequence of strings,
// representing the roles of A, B, C, D.
//
//  Attacker notation:
//    #   attacker
//
//  Detector notation:
//    _   detector
//
//  When a code node both attacks and detects, the detector always comes first.
//  That's because the non-loop tests already check for the cases where the
//  detector obviously follows the attacker.
//
//  Behaviour notation:
//    includes 'b!':  has unconditional break
//    includes 'b=':  has uniform break
//    includes 'bx':  has non-uniform break
//    includes 'r!':  has unconditional return
//    includes 'r=':  has uniform return
//    includes 'rx':  has non-uniform return
//    includes 'c!':  has unconditional continue
//    includes 'c=':  has uniform continue
//    includes 'cx':  has non-uniform continue
//    includes ' ':   empty statement; used to force a continuing block.
//  The Next behaviours are covered by combinations of the above.
//  For example:
//    if the full behaviour is "b=" then that implies a uniform Next.
//    if the full behaviour is "b=r!" then then Next is not in the behaviour.
//
//
// Analyzing for loops and while loops
//
// To analyze for loops and while loops, we need to also model the effect
// of the condition expressions. We'll use a string mapping:
//    ""    no condition (to be used in for loops without a condition)
//    "="   uniform condition
//    "x"   nonuniform condition
//
// The significant code locations are the same as for loop-without-continuing,
// and we reuse the labels for before the loop, the body, and after the loop:
//
//    // A
//    // CF
//    for ( ; optional-cond ; )
//         // CF'
//      s1 // B
//         // CF1
//    }
//    // D
//    // CFResult
//
//    // A
//    // CF
//    while ( ; cond ; )
//         // CF'
//      s1 // B
//         // CF1
//    }
//    // D
//    // CFResult
//
// A 'for' loop without a condition is structurally identical to a
// 'loop' without continuing. We will derive those cases automatically.
//
// A 'while' loop always has a condition, and is structurally very
// similar to a 'for' loop with a condition. (The for loop may have
// an initializer and update clause.)  We will derive the while
// loop cases automatically.

enum Verdict {
    // The shader is valid, and always passes uniformity analysis.
    kAccept,
    // The shader is valid any time the attacker is present.
    kAcceptLoopExitFromAttack,
    // The shader is valid but only when the attacker is present, and
    // the detector is absent.
    kAcceptLoopExitFromAttackWithoutDetect,
    // The shader is rejected by uniformity analysis.
    kReject,
    // The shader is rejected for reasons other than uniformity failure,
    // e.g. because of an infinite loop.
    kRejectForOtherReasons,
};

enum class Part {
    kBefore,
    kBody,
    kContinuing,
    kAfter,
};

// Encodes the role for a code fragment in the test scenario.
struct Role {
    Role(Part part_, std::string r) : part(part_), role(r) {
        // The following integrity checks are checked during the behave() method.
        //  -  at most one !
        //  -  ! occurs after uniform and non-uniform interruptions
    }

    // Returns a WGSL fragment that behaves according to the specified part
    // and role, and according to whether to enable the attack (if any) or
    // the detection mechanism (if any).
    std::string behave(bool enable_attack, bool enable_detect) const {
        int num_unconditional = 0;
        std::string_view remaining = role.data();
        std::ostringstream ss;
        while (!remaining.empty()) {
            switch (part) {
                case Part::kBody:
                    ss << "    ";
                    break;
                case Part::kContinuing:
                    ss << "      ";
                    break;
                default:
                    ss << "  ";
                    break;
            }
            if (remaining.starts_with(" ")) {
                remaining.remove_prefix(1);
                ss << ";/* empty statement */\n";
                continue;
            }
            if (remaining.starts_with("_")) {
                remaining.remove_prefix(1);
                if (enable_detect) {
                    ss << "workgroupBarrier();\n";
                } else {
                    ss << "/*disabled detect*/;\n";
                }
                continue;
            }
            if (remaining.starts_with("#")) {
                remaining.remove_prefix(1);
                if (enable_attack) {
                    switch (part) {
                        case Part::kBefore:
                            ss << "if nonuniform_cond { return; }\n";
                            break;
                        case Part::kBody:
                            ss << "if nonuniform_cond { break; }\n";
                            break;
                        case Part::kContinuing:
                            ss << "break if nonuniform_cond;\n";
                            break;
                        case Part::kAfter:
                            TINT_ICE() << "Don't test an attacker placed after the loop";
                    }
                } else {
                    ss << "/*disabled attack*/\n";
                }
                continue;
            }
            if (remaining.starts_with("b!")) {
                remaining.remove_prefix(2);
                ss << "break;\n";
                TINT_ASSERT(num_unconditional == 0) << "b! must not follow !";
                num_unconditional++;
                continue;
            }
            if (remaining.starts_with("r!")) {
                remaining.remove_prefix(2);
                ss << "return;\n";
                TINT_ASSERT(num_unconditional == 0) << "r! must not follow !";
                num_unconditional++;
                continue;
            }
            if (remaining.starts_with("c!")) {
                remaining.remove_prefix(2);
                ss << "continue;\n";
                TINT_ASSERT(num_unconditional == 0) << "c! must not follow !";
                num_unconditional++;
                continue;
            }
            if (remaining.starts_with("b=")) {
                remaining.remove_prefix(2);
                if (part == Part::kContinuing) {
                    ss << "break if uniform_cond;\n";
                } else {
                    ss << "if uniform_cond { break; }\n";
                }
                TINT_ASSERT(num_unconditional == 0) << "b= must not follow !";
                continue;
            }
            if (remaining.starts_with("bx")) {
                remaining.remove_prefix(2);
                if (part == Part::kContinuing) {
                    ss << "break if nonuniform_cond;\n";
                } else {
                    ss << "if nonuniform_cond { break; }\n";
                }
                TINT_ASSERT(num_unconditional == 0) << "bx must not follow !";
                continue;
            }
            if (remaining.starts_with("r=")) {
                remaining.remove_prefix(2);
                ss << "if uniform_cond { return; }\n";
                TINT_ASSERT(num_unconditional == 0) << "r= must not follow !";
                continue;
            }
            if (remaining.starts_with("rx")) {
                remaining.remove_prefix(2);
                ss << "if nonuniform_cond { return; }\n";
                TINT_ASSERT(num_unconditional == 0) << "rx must not follow !";
                continue;
            }
            if (remaining.starts_with("c=")) {
                remaining.remove_prefix(2);
                ss << "if uniform_cond { continue; }\n";
                TINT_ASSERT(num_unconditional == 0) << "c= must not follow !";
                continue;
            }
            if (remaining.starts_with("cx")) {
                remaining.remove_prefix(2);
                ss << "if nonuniform_cond { continue; }\n";
                TINT_ASSERT(num_unconditional == 0) << "cx must not follow !";
                continue;
            }
            TINT_ICE() << "invalid token " << remaining << "; in role spec " << role;
        }
        return ss.str();
    }
    const Part part = Part::kBefore;
    const std::string role = "";
};

enum LoopKind {
    kLoop,
    kFor,
    kWhile,
};

struct LoopGraphCase {
    LoopGraphCase(LoopKind kind_,
                  std::string before_spec,
                  std::string condition_spec_,
                  std::string body_spec,
                  std::string continuing_spec,
                  std::string after_spec,
                  Verdict verdict_)
        : kind(kind_),
          before(Part::kBefore, before_spec),
          condition_spec(condition_spec_),
          body(Part::kBody, body_spec),
          continuing(Part::kContinuing, continuing_spec),
          after(Part::kAfter, after_spec),
          verdict(verdict_) {
        TINT_ASSERT(kind == kLoop || kind == kFor || kind == kWhile);
        TINT_ASSERT(condition_spec == "" || condition_spec == "=" || condition_spec == "x");
        if (condition_spec.empty()) {
            TINT_ASSERT(kind == kLoop || kind == kFor);
        } else {
            TINT_ASSERT(kind == kFor || kind == kWhile);
        }
    }

    std::string condition() const {
        if (condition_spec == "") {
            return "";
        }
        if (condition_spec == "=") {
            return "uniform_cond";
        }
        if (condition_spec == "x") {
            return "nonuniform_cond";
        }
        TINT_UNREACHABLE();
    }
    std::string summary() const {
        std::ostringstream ss;
        switch (kind) {
            case kLoop:
                ss << "loop ";
                break;
            case kFor:
                ss << "for ";
                break;
            case kWhile:
                ss << "while ";
                break;
            default:
                TINT_UNREACHABLE() << "kind out of range " << static_cast<int>(kind);
        }
        ss << "\"" << condition_spec << "\" \"" << before.role << "\", \"" << body.role << "\", \""
           << continuing.role << "\", \"" << after.role << "\"";
        return ss.str();
    }

    std::string Shader(bool enable_attack, bool enable_detect) const {
        std::ostringstream ss;
        ss << "// " << summary() << "\n";
        ss << R"(@compute @workgroup_size(8)
fn main(@builtin(local_invocation_index) nonuniform_var: u32, @builtin(num_workgroups) nw: vec3u) {
  let uniform_cond = 42 == nw.x;
  let nonuniform_cond = 42 == nonuniform_var;
)";
        ss << before.behave(enable_attack, enable_detect);
        switch (kind) {
            case kLoop:
                ss << "  loop {\n";
                break;
            case kFor:
                ss << "  for ( ; " << condition() << " ; ) {\n";
                break;
            case kWhile:
                ss << "  while (" << condition() << ") {\n";
                break;
        }
        ss << body.behave(enable_attack, enable_detect);
        if (kind == kLoop && !continuing.role.empty()) {
            ss << "    continuing {\n";
            ss << continuing.behave(enable_attack, enable_detect);
            ss << "    }\n";
        }
        ss << "  }\n";
        ss << after.behave(enable_attack, enable_detect);
        ss << "}\n";
        return ss.str();
    }

    const LoopKind kind = kLoop;
    const Role before;
    const std::string condition_spec;
    const Role body;
    const Role continuing;
    const Role after;
    const Verdict verdict;
};

std::vector<LoopGraphCase> LoopGraphCases() {
    // L is for Loop
    auto L = [](std::string a, std::string b, std::string c, Verdict v) -> LoopGraphCase {
        return LoopGraphCase(kLoop, a, "", b, "", c, v);
    };
    // LC is for Loop with Continuing
    auto LC = [](std::string a, std::string b, std::string c, std::string d,
                 Verdict v) -> LoopGraphCase { return LoopGraphCase(kLoop, a, "", b, c, d, v); };
    // F is for-loop
    auto F = [](std::string before, std::string cond, std::string body, std::string after,
                Verdict v) -> LoopGraphCase {
        return LoopGraphCase(kFor, before, cond, body, "", after, v);
    };
    // F is while-loop
    auto W = [](std::string before, std::string cond, std::string body, std::string after,
                Verdict v) -> LoopGraphCase {
        return LoopGraphCase(kWhile, before, cond, body, "", after, v);
    };

    auto cases = std::vector<LoopGraphCase>{
        // clang-format off
        // Loop without continuing:

        // Scenario (A,B), i.e. A attacks, B detects
        // Tests CF <- CF'
        L("#", "_b!", "", kReject),
        L("#", "_b=", "", kReject),
        L("#", "_bx", "", kReject),
        L("#", "_r!", "", kReject),
        L("#", "_r=", "", kReject),
        L("#", "_rx", "", kReject),
        L("#", "_b=c!", "", kReject),  // add some continues
        L("#", "_b=c=", "", kReject),
        L("#", "_b=cx", "", kReject),
        L("#", "_r=c!", "", kReject),
        L("#", "_r=c=", "", kReject),
        L("#", "_r=cx", "", kReject),

        // Scenario (A,D)
        //   Return not in s1 behaviour
        //   By infinite loop rule, Break must be in s1's behavior.
        //     Tests CF <- CFResult
        L("#", "", "_", kRejectForOtherReasons),  // infinite loop
        L("#", "b=", "_", kReject),
        L("#", "bx", "_", kReject),
        L("#", "b!", "_", kReject),
        L("#", "b=c!", "_", kReject),
        L("#", "b=c=", "_", kReject),
        L("#", "b=cx", "_", kReject),
        L("#", "bxc!", "_", kReject),
        L("#", "bxc=", "_", kReject),
        L("#", "bxcx", "_", kReject),
        //   Return is in s1's behavior
        //     Break is also in s1's behavior
        //     The detector is only reached when Break is in s1's behavior.
        //     Tests CF <- CF1 <- CFResult
        L("#", "b=r!", "_", kReject),
        L("#", "b=r=", "_", kReject),
        L("#", "b=rx", "_", kReject),
        L("#", "bxr!", "_", kReject),
        L("#", "bxr=", "_", kReject),
        L("#", "bxrx", "_", kReject),
        //   Return is in s1's behavior
        //     Break is not in s1's behavior.
        //     Therefore the behaviour of the whole loop is {Return}
        //     Therefore by the recursive analysis rule for "s1 s2",
        //     there are no downstream connections in the graph.
        //     Any code after the loop can't be tainted.
        L("#", "r!", "_", kAccept),
        L("#", "r=", "_", kAccept),
        L("#", "rx", "_", kAccept),
        L("#", "c=r!", "_", kAccept),
        L("#", "c=r=", "_", kAccept),
        L("#", "c=rx", "_", kAccept),
        L("#", "cxr!", "_", kAccept),
        L("#", "cxr=", "_", kAccept),
        L("#", "cxrx", "_", kAccept),

        // Scenario (B,B).  Recall: Only check via the backage.
        // Backedge exists if and only if Next or Continue in s1's behaviour.
        // Test s1 <- CF1 <- CF' <- s1
        L("", "_#", "", kAcceptLoopExitFromAttackWithoutDetect),
        L("", "_#b=", "", kReject),
        L("", "_#bx", "", kReject),
        L("", "_#b!", "", kAccept),  // no backedge
        L("", "_#c=", "", kAcceptLoopExitFromAttackWithoutDetect),
        L("", "_#cx", "", kAcceptLoopExitFromAttackWithoutDetect),
        L("", "_#c!", "", kAcceptLoopExitFromAttackWithoutDetect),
        L("", "_#r=", "", kReject),
        L("", "_#rx", "", kReject),
        L("", "_#r!", "", kAccept),  // no backedge
        L("", "_#b=c=", "", kReject),
        L("", "_#b=cx", "", kReject),
        L("", "_#b=c!", "", kReject),
        L("", "_#bxc=", "", kReject),
        L("", "_#bxcx", "", kReject),
        L("", "_#bxc!", "", kReject),
        L("", "_#b=r=", "", kReject),
        L("", "_#b=rx", "", kReject),
        L("", "_#b=r!", "", kAccept),  // no backedge
        L("", "_#bxr=", "", kReject),
        L("", "_#bxrx", "", kReject),
        L("", "_#bxr!", "", kAccept),  // no backedge
        L("", "_#c=r=", "", kReject),
        L("", "_#c=rx", "", kReject),
        L("", "_#c=r!", "", kReject),  // continue provides backedge
        L("", "_#cxr=", "", kReject),
        L("", "_#cxrx", "", kReject),
        L("", "_#cxr!", "", kReject),  // continue provides backedege

        // Scenario (B,D)
        // Test CF1 <- CFResult
        // Edge exists if and only if Return is in s1's behaviour.
        L("", "#", "_", kAcceptLoopExitFromAttack),
        L("", "#b=", "_", kAccept),
        L("", "#bx", "_", kAccept),
        L("", "#b!", "_", kAccept),
        L("", "#c=b=", "_", kAccept),  // need a break to avoid infinite loop
        L("", "#cxb=", "_", kAccept),
        L("", "#r=", "_", kReject),    // The attacker provides the break
        L("", "#rx", "_", kReject),  // The attacker provides the break
        L("", "#r!", "_", kReject),
        // Move the attacker after the control construct.
        L("", "b=#", "_", kAccept),
        L("", "bx#", "_", kAccept),
        L("", "b!#", "_", kAccept),
        L("", "c=#b=", "_", kAccept),
        L("", "cx#b=", "_", kAccept),
        L("", "r=#", "_", kReject),   // The attacker provides the break
        L("", "rx#", "_", kReject),  // The attacker provides the break
        L("", "r!#", "_", kAccept),  // unreachable

        // Loop with continuing:
        //
        // Recall the scenarios are:
        //    (A,B)  (A,C)  (A,D)
        //    (B,B)  (B,C)  (B,D)
        //    (C,B)  (C,C)  (C,D)
        // Cases without C are those where the continuing block is not
        // involved in uniformity.  For those, cases are already derived
        // automatically from the corresponding case without the continuing
        // block. (An empty continuing{} construct is inserted by the
        // LoopGraphCases() function.)
        // What remains is:
        //           (A,C)
        //           (B,C)
        //    (C,B)  (C,C)  (C,D)
        //
        // Recall that a continuing block behaviour must be a subset of
        // {Next,Break}

        // Scenario (A,C)
        LC("#", "", "_", "", kRejectForOtherReasons),  // infinite loop
        LC("#", "", "_b=", "", kReject),
        LC("#", "", "_bx", "", kReject),
        LC("#", "", "_b!", "", kRejectForOtherReasons),  // invalid unconditional break
        LC("#", "b=", "c=", "", kRejectForOtherReasons),  // invalid behaviour
        LC("#", "b=", "cx", "", kRejectForOtherReasons),  // invalid behaviour
        LC("#", "b=", "c!", "", kRejectForOtherReasons),  // invalid behaviour
        LC("#", "b=", "r=", "", kRejectForOtherReasons),  // invalid behaviour
        LC("#", "b=", "rx", "", kRejectForOtherReasons),  // invalid behaviour
        LC("#", "b=", "r!", "", kRejectForOtherReasons),  // invalid behaviour

        // Scenario (B,C)
        //   Continuing block has behaviour {Next}
        LC("", "#b=", "_", "", kReject),
        LC("", "#bx", "_", "", kReject),
        LC("", "#b!", "_", "", kAccept),  // detector unreachable
        LC("", "#b=c=", "_", "", kReject),
        LC("", "#b=cx", "_", "", kReject),
        LC("", "#b=c!", "_", "", kReject),
        LC("", "#bxc=", "_", "", kReject),
        LC("", "#bxcx", "_", "", kReject),
        LC("", "#bxc!", "_", "", kReject),
        LC("", "#r=", "_", "", kReject),
        LC("", "#rx", "_", "", kReject),
        LC("", "#r!", "_", "", kAccept),  // detector unreachable
        LC("", "#r=c=", "_", "", kReject),
        LC("", "#r=cx", "_", "", kReject),
        LC("", "#r=c!", "_", "", kReject),
        LC("", "#rxc=", "_", "", kReject),
        LC("", "#rxcx", "_", "", kReject),
        LC("", "#rxc!", "_", "", kReject),
        //   Continuing block behaviour includes Break.
        //   (Exclude "b!" subcases because that's a naked 'break;',
        //   which is disallowed.
        LC("", "#", "_b=", "", kReject),
        LC("", "#", "_bx", "", kReject),
        LC("", "#b=", "_b=", "", kReject),
        LC("", "#bx", "_bx", "", kReject),
        LC("", "#b!", "_bx", "", kAccept),  // detector unreachable
        LC("", "#r=", "_b=", "", kReject),
        LC("", "#rx", "_bx", "", kReject),
        LC("", "#r!", "_bx", "", kAccept),  // detector unreachable

        // Scenario (C,B)
        // i.e. attacker in continuing block, detector in body.
        // This fails when there's an edge from CF' to CF2, which occurs
        // when the behavior of s1 includes Next or Continue.
        LC("", "_", "#", "", kAcceptLoopExitFromAttackWithoutDetect),  // not infinite; attacker is a break-if
        // Try "break" first.
        LC("", "_b=", "#", "", kReject),
        LC("", "_bx", "#", "", kReject),
        LC("", "_b!", "#", "", kAccept),  // attacker unreachable
        LC("", "_b=c=", "#", "", kReject),
        LC("", "_b=cx", "#", "", kReject),
        LC("", "_b=c!", "#", "", kReject),
        LC("", "_bxc=", "#", "", kReject),
        LC("", "_bxcx", "#", "", kReject),
        LC("", "_bxc!", "#", "", kReject),
        // Try "return" first.
        LC("", "_r=", "#", "", kReject),
        LC("", "_rx", "#", "", kReject),
        LC("", "_r!", "#", "", kAccept),  // attacker unreachable
        LC("", "_r=c=", "#", "", kReject),
        LC("", "_r=cx", "#", "", kReject),
        LC("", "_r=c!", "#", "", kReject),
        LC("", "_rxc=", "#", "", kReject),
        LC("", "_rxcx", "#", "", kReject),
        LC("", "_rxc!", "#", "", kReject),
        // Try "continue" first.
        LC("", "_c=", "#", "", kAcceptLoopExitFromAttackWithoutDetect),  // not infinite; attacker is a break-if
        LC("", "_cx", "#", "", kAcceptLoopExitFromAttackWithoutDetect),  // not infinite; attacker is a break-if
        LC("", "_c!", "#", "", kAcceptLoopExitFromAttackWithoutDetect),  // not infinite; attacker is a break-if
        LC("", "_c=b=", "#", "", kReject),
        LC("", "_c=bx", "#", "", kReject),
        LC("", "_c=b!", "#", "", kReject),
        LC("", "_cxb=", "#", "", kReject),
        LC("", "_cxbx", "#", "", kReject),
        LC("", "_cxb!", "#", "", kReject),
        LC("", "_c=r=", "#", "", kReject),
        LC("", "_c=rx", "#", "", kReject),
        LC("", "_c=r!", "#", "", kReject),
        LC("", "_cxr=", "#", "", kReject),
        LC("", "_cxrx", "#", "", kReject),
        LC("", "_cxr!", "#", "", kReject),

        // Scenario (C,C)
        // Continuing block has detector, then attacker.
        // Fails only when the continuing block is reachable,
        // and there is a backedge.
        LC("", "", "_#", "", kAcceptLoopExitFromAttackWithoutDetect),  // not infinite; attacker is a break-if
        LC("", "b=", "_#", "", kReject),
        LC("", "bx", "_#", "", kReject),
        LC("", "b!", "_#", "", kAccept),
        LC("", "r=", "_#", "", kReject),
        LC("", "rx", "_#", "", kReject),
        LC("", "r!", "_#", "", kAccept),
        LC("", "c=", "_#", "", kAcceptLoopExitFromAttackWithoutDetect),
        LC("", "cx", "_#", "", kAcceptLoopExitFromAttackWithoutDetect),
        LC("", "c!", "_#", "", kAcceptLoopExitFromAttackWithoutDetect),

        // Scenario (C,D)
        //  i.e. attacker in continuing block, detector after the loop.
        //
        //  Option 1: s1 behaviour is {Break,Return}, in which case uniformity
        //  after the loop depends on the body, not the continuing block.
        //
        //  Option 2: s1 behaviour intersects {Next,Continue}.
        //  Rejects when there is a path from CFResult to CF2.
        //  This occurs when both:
        //    CFResult -> CF' exists, i.e. Return in s1's behaviour, and
        //    CF' -> CF2 exists, i.e. Next or Continue in s1's behaviour.
        //  When Return is not in s1's behaviour, divergence from a break
        //  within the loop will resolve (reconverge) at the end of the loop.
        LC("", "", "#", "_", kAcceptLoopExitFromAttack),  // not infinite; attacker is a break-if
        LC("", "b=", "#", "_", kAccept),
        LC("", "bx", "#", "_", kAccept),
        LC("", "b!", "#", "_", kAccept),
        LC("", "b=c=", "#", "_", kAccept),
        LC("", "b=cx", "#", "_", kAccept),
        LC("", "b=c!", "#", "_", kAccept),
        LC("", "bxc=", "#", "_", kAccept),
        LC("", "bxcx", "#", "_", kAccept),
        LC("", "bxc!", "#", "_", kAccept),
        LC("", "b=r=", "#", "_", kReject),
        LC("", "b=rx", "#", "_", kReject),
        LC("", "b=r!", "#", "_", kAccept),  // s1 behavior is {Break,Return}
                                            // so CFResult -> CF1, uniform
        LC("", "bxr=", "#", "_", kReject),
        LC("", "bxrx", "#", "_", kReject),
        LC("", "bxr!", "#", "_", kReject),  // s1 behavior is {Break,Return}
                                            // so CFResult -> CF1, nonuniform
        LC("", "r=", "#", "_", kReject),
        LC("", "rx", "#", "_", kReject),
        LC("", "r!", "#", "_", kAccept),  // detector is unreachable
        LC("", "r=b=", "#", "_", kReject),
        LC("", "r=bx", "#", "_", kReject),
        LC("", "r=b!", "#", "_", kAccept),  // s1 behavior is {Break,Return}
                                            // so CFResult -> CF1, uniform
        LC("", "r=c=", "#", "_", kReject),
        LC("", "r=cx", "#", "_", kReject),
        LC("", "r=c!", "#", "_", kReject),
        LC("", "rxb=", "#", "_", kReject),
        LC("", "rxbx", "#", "_", kReject),
        LC("", "rxb!", "#", "_", kReject),  // s1 behavior is {Break,Return}
                                            // so CFResult -> CF1, nonuniform
        LC("", "rxc=", "#", "_", kReject),
        LC("", "rxcx", "#", "_", kReject),
        LC("", "rxc!", "#", "_", kReject),
        // Put "continue" first.
        LC("", "c=", "#", "_", kAcceptLoopExitFromAttack),
        LC("", "cx", "#", "_", kAcceptLoopExitFromAttack),
        LC("", "c!", "#", "_", kAcceptLoopExitFromAttack),
        LC("", "c=b=", "#", "_", kAccept),
        LC("", "c=bx", "#", "_", kAccept),
        LC("", "c=b!", "#", "_", kAccept),
        LC("", "cxb=", "#", "_", kAccept),
        LC("", "cxbx", "#", "_", kAccept),
        LC("", "cxb!", "#", "_", kAccept),
        LC("", "c=r=", "#", "_", kReject),
        LC("", "c=rx", "#", "_", kReject),
        LC("", "c=r!", "#", "_", kReject),
        LC("", "cxr=", "#", "_", kReject),
        LC("", "cxrx", "#", "_", kReject),
        LC("", "cxr!", "#", "_", kReject),

        // For loops
        // Note that most for-without-condition cases will be automatically
        // generated from loop-without-continuing, as they should have
        // isomorphic uniformity graphs.
        //
        // Trivial cases
        F("", "", "", "", kRejectForOtherReasons),  // infinite loop
        F("", "=", "", "", kAccept),
        F("", "x", "", "", kAccept),

        // For loops with a condition expression.
        // Scenario (A,B): attacker before loop, detector in body
        //   Uniform condition
        F("#", "=", "_", "", kReject),
        //     body has detector, then interruption
        F("#", "=", "_b=", "", kReject),
        F("#", "=", "_bx", "", kReject),
        F("#", "=", "_b!", "", kReject),
        F("#", "=", "_r=", "", kReject),
        F("#", "=", "_rx", "", kReject),
        F("#", "=", "_r!", "", kReject),
        F("#", "=", "_c=", "", kReject),
        F("#", "=", "_cx", "", kReject),
        F("#", "=", "_c!", "", kReject),
        //     body has interruption, then detector
        F("#", "=", "b=_", "", kReject),
        F("#", "=", "bx_", "", kReject),
        F("#", "=", "b!_", "", kAccept),  // detector not reachable
        F("#", "=", "r=_", "", kReject),
        F("#", "=", "rx_", "", kReject),
        F("#", "=", "r!_", "", kAccept),  // detector not reachable
        F("#", "=", "c=_", "", kReject),
        F("#", "=", "cx_", "", kReject),
        F("#", "=", "c!_", "", kAccept),  // detector not reachable
        //   Nonuniform condition
        F("#", "x", "_", "", kReject),
        //     body has detector, then interruption
        F("#", "x", "_b=", "", kReject),
        F("#", "x", "_bx", "", kReject),
        F("#", "x", "_b!", "", kReject),
        F("#", "x", "_r=", "", kReject),
        F("#", "x", "_rx", "", kReject),
        F("#", "x", "_r!", "", kReject),
        F("#", "x", "_c=", "", kReject),
        F("#", "x", "_cx", "", kReject),
        F("#", "x", "_c!", "", kReject),
        //     body has interruption, then detector
        F("#", "x", "b=_", "", kReject),
        F("#", "x", "bx_", "", kReject),
        F("#", "x", "b!_", "", kAccept),  // detector not reachable
        F("#", "x", "r=_", "", kReject),
        F("#", "x", "rx_", "", kReject),
        F("#", "x", "r!_", "", kAccept),  // detector not reachable
        F("#", "x", "c=_", "", kReject),
        F("#", "x", "cx_", "", kReject),
        F("#", "x", "c!_", "", kAccept),  // detector not reachable

        // Scenario (A,D): attacker before loop, detector after body
        //   Uniform condition
        F("#", "=", "", "_", kReject),
        F("#", "=", "b=", "_", kReject),
        F("#", "=", "bx", "_", kReject),
        F("#", "=", "b!", "_", kReject),
        F("#", "=", "c=", "_", kReject),
        F("#", "=", "cx", "_", kReject),
        F("#", "=", "c!", "_", kReject),
        F("#", "=", "r=", "_", kReject),
        F("#", "=", "rx", "_", kReject),
        F("#", "=", "r!", "_", kReject),  // loop condition is an implicit break
        //   Nonuniform condition
        F("#", "x", "", "_", kReject),
        F("#", "x", "b=", "_", kReject),
        F("#", "x", "bx", "_", kReject),
        F("#", "x", "b!", "_", kReject),
        F("#", "x", "c=", "_", kReject),
        F("#", "x", "cx", "_", kReject),
        F("#", "x", "c!", "_", kReject),
        F("#", "x", "r=", "_", kReject),
        F("#", "x", "rx", "_", kReject),
        F("#", "x", "r!", "_", kReject),  // loop condition is an implicit break

        // Scenario (B,D): attacker in body, detector after body
        // When the body behaviour includes Return, the after-loop node
        // links to the end of the body.
        //
        //  Uniform condition
        F("", "=", "#", "_", kAccept),
        F("", "=", "#b=", "_", kAccept),
        F("", "=", "#bx", "_", kAccept),
        F("", "=", "#b!", "_", kAccept),
        F("", "=", "#c=", "_", kAccept),
        F("", "=", "#cx", "_", kAccept),
        F("", "=", "#c!", "_", kAccept),
        F("", "=", "#r=", "_", kReject),
        F("", "=", "#rx", "_", kReject),
        F("", "=", "#r!", "_", kReject),
        //    Check return after others
        F("", "=", "#b=r=", "_", kReject),
        F("", "=", "#b=rx", "_", kReject),
        F("", "=", "#b=r!", "_", kReject),
        F("", "=", "#c=r=", "_", kReject),
        F("", "=", "#c=rx", "_", kReject),
        F("", "=", "#b=r!", "_", kReject),
        //    Check return before others
        F("", "=", "#r=b=", "_", kReject),
        F("", "=", "#r=bx", "_", kReject),
        F("", "=", "#r=b!", "_", kReject),
        F("", "=", "#r=c=", "_", kReject),
        F("", "=", "#r=cx", "_", kReject),
        F("", "=", "#r=c!", "_", kReject),
        F("", "=", "#rxb=", "_", kReject),
        F("", "=", "#rxbx", "_", kReject),
        F("", "=", "#rxb!", "_", kReject),
        F("", "=", "#rxc=", "_", kReject),
        F("", "=", "#rxcx", "_", kReject),
        F("", "=", "#rxc!", "_", kReject),
        //  Nonunifor condition
        F("", "x", "#", "_", kAccept),
        F("", "x", "#b=", "_", kAccept),
        F("", "x", "#bx", "_", kAccept),
        F("", "x", "#b!", "_", kAccept),
        F("", "x", "#c=", "_", kAccept),
        F("", "x", "#cx", "_", kAccept),
        F("", "x", "#c!", "_", kAccept),
        F("", "x", "#r=", "_", kReject),
        F("", "x", "#rx", "_", kReject),
        F("", "x", "#r!", "_", kReject),
        //    Check return after others
        F("", "x", "#b=r=", "_", kReject),
        F("", "x", "#b=rx", "_", kReject),
        F("", "x", "#b=r!", "_", kReject),
        F("", "x", "#c=r=", "_", kReject),
        F("", "x", "#c=rx", "_", kReject),
        F("", "=", "#b=r!", "_", kReject),
        //    Check return before others
        F("", "x", "#r=b=", "_", kReject),
        F("", "x", "#r=bx", "_", kReject),
        F("", "x", "#r=b!", "_", kReject),
        F("", "x", "#r=c=", "_", kReject),
        F("", "x", "#r=cx", "_", kReject),
        F("", "x", "#r=c!", "_", kReject),
        F("", "x", "#rxb=", "_", kReject),
        F("", "x", "#rxbx", "_", kReject),
        F("", "x", "#rxb!", "_", kReject),
        F("", "x", "#rxc=", "_", kReject),
        F("", "x", "#rxcx", "_", kReject),
        F("", "x", "#rxc!", "_", kReject),

        // While loops
        // While loops always have an expression.
        //
        // Trivial cases
        W("", "=", "", "", kAccept), W("", "x", "", "", kAccept),
        // We will automatically derive most while-loop tests from
        // the for-loop tests.
        //
        // The uniformity graph of a while loop nearly isomorphic to a
        // for-loop with a condition expression.  The only difference is
        // that for-loops have an initializer statement, and while-loops
        // do not.

        // clang-format on
    };

    // Use loop-without-continuing cases to derive:
    //  - loop-continuing cases, where the continiuing clause is empty
    //  - for-loop without a condition.
    for (size_t i = 0; i < cases.size(); i++) {
        // Take copy instead of a reference because the vector will expand at
        // some point, invalidating references.
        const auto case_ = cases[i];
        if (case_.kind == kLoop && case_.continuing.role.empty()) {
            // Create a new loop case, with an empty continuing clause.
            cases.emplace_back(kLoop, case_.before.role, "", case_.body.role, " ", case_.after.role,
                               case_.verdict);
            // Create a for-loop case without condition expression.
            cases.emplace_back(kFor, case_.before.role, "", case_.body.role, "", case_.after.role,
                               case_.verdict);
        }
        if (case_.kind == kFor && !case_.condition_spec.empty()) {
            // Create a while-loop case without condition expression.
            cases.emplace_back(kWhile, case_.before.role, case_.condition_spec, case_.body.role, "",
                               case_.after.role, case_.verdict);
        }
    }

    return cases;
}

class LoopGraphTest : public UniformityAnalysisTestBase,
                      public ::testing::TestWithParam<LoopGraphCase> {};

TEST_P(LoopGraphTest, BothAttackAndDetect) {
    const auto src = GetParam().Shader(true, true);
    const auto verdict = GetParam().verdict;
    const bool should_accept = verdict == kAccept || verdict == kAcceptLoopExitFromAttack;
    RunTest(src, should_accept);
    // The accept case may still generate warnings.
    // But failure cases always generate an error.
    if (!should_accept) {
        EXPECT_FALSE(error_.empty()) << "need an error string for:\n" << src;
    }
}

// Don't try the OnlyDetect case because many cases generate their
// own non-uniformity. For example, L("", "_bx", "") which translates
// to:
//    loop {
//       workgroupBarrier();
//       if nonuniform_var == 42 { break; }
//    }
// Those cases are already tested by using an explicit attacker.
// For example, the above is covered by L("", "_#c!", "")
//

TEST_P(LoopGraphTest, OnlyAttack) {
    const auto src = GetParam().Shader(true, false);
    const auto verdict = GetParam().verdict;
    const bool should_accept = verdict == kAccept || verdict == kAcceptLoopExitFromAttack ||
                               verdict == kAcceptLoopExitFromAttackWithoutDetect ||
                               verdict == kReject;
    RunTest(src, should_accept);
}

TEST_P(LoopGraphTest, NeitherAttackNorDetect) {
    const auto src = GetParam().Shader(false, false);
    const auto verdict = GetParam().verdict;
    // In the kAcceptLoopExitFromAttackWithoutDetect, the loop exit is
    // provided only by the attacker. Without the attacker, the loop
    // is statically infinite, and hence rejected.
    const bool should_accept = verdict == kAccept || verdict == kReject;
    RunTest(src, should_accept);
}

INSTANTIATE_TEST_SUITE_P(AllLoops, LoopGraphTest, ::testing::ValuesIn(LoopGraphCases()));
}  // namespace LoopTest

////////////////////////////////////////////////////////////////////////////////
/// If-else statement tests.
////////////////////////////////////////////////////////////////////////////////

TEST_F(UniformityAnalysisTest, IfElse_UniformCondition_BarrierInTrueBlock) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_global : i32;

fn foo() {
  if (uniform_global == 42) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_UniformCondition_BarrierInElseBlock) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_global : i32;

fn foo() {
  if (uniform_global == 42) {
  } else {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_UniformCondition_BarrierInElseIfBlock) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_global : i32;

fn foo() {
  if (uniform_global == 42) {
  } else if (true) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_NonUniformCondition_BarrierInTrueBlock) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (non_uniform == 42) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:5:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_NonUniformCondition_BarrierInElseBlock) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (non_uniform == 42) {
  } else {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:5:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_ShortCircuitingCondition_NonUniformLHS_And) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

var<private> p : i32;

fn main() {
  if ((non_uniform_global == 42) && false) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:7 note: control flow depends on possibly non-uniform value
  if ((non_uniform_global == 42) && false) {
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

test:7:8 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if ((non_uniform_global == 42) && false) {
       ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_ShortCircuitingCondition_NonUniformRHS_And) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

var<private> p : i32;

fn main() {
  if (false && (non_uniform_global == 42)) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (false && (non_uniform_global == 42)) {
  ^^

test:7:17 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (false && (non_uniform_global == 42)) {
                ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_ShortCircuitingCondition_NonUniformLHS_Or) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

var<private> p : i32;

fn main() {
  if ((non_uniform_global == 42) || true) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:7 note: control flow depends on possibly non-uniform value
  if ((non_uniform_global == 42) || true) {
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

test:7:8 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if ((non_uniform_global == 42) || true) {
       ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_ShortCircuitingCondition_NonUniformRHS_Or) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

var<private> p : i32;

fn main() {
  if (true || (non_uniform_global == 42)) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (true || (non_uniform_global == 42)) {
  ^^

test:7:16 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (true || (non_uniform_global == 42)) {
               ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_NonUniformCondition_BarrierInElseIfBlock) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (non_uniform == 42) {
  } else if (true) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:5:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_VarBecomesNonUniform_BeforeCondition) {
    // Use a function-scope variable for control-flow guarding a barrier, and then assign to that
    // variable before checking the condition.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v = 0;
  v = rw;
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:6:7 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  v = rw;
      ^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_VarBecomesNonUniform_AfterCondition) {
    // Use a function-scope variable for control-flow guarding a barrier, and then assign to that
    // variable after checking the condition.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v = 0;
  if (v == 0) {
    v = rw;
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_VarBecomesNonUniformInIf_BarrierInElse) {
    // Assign a non-uniform value to a variable in an if-block, and then use that variable for a
    // conditional barrier in the else block.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  if (true) {
    v = non_uniform;
  } else {
    if (v == 0) {
      workgroupBarrier();
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_AssignNonUniformInIf_AssignUniformInElse) {
    // Assign a non-uniform value to a variable in an if-block and a uniform value in the else
    // block, and then use that variable for a conditional barrier after the if-else statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  if (true) {
    if (true) {
      v = non_uniform;
    } else {
      v = 5;
    }
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:15:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:14:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:8:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_AssignNonUniformInIfWithReturn) {
    // Assign a non-uniform value to a variable in an if-block followed by a return, and then use
    // that variable for a conditional barrier after the if-else statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  if (true) {
    v = non_uniform;
    return;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_AssignNonUniformBeforeIf_BothBranchesAssignUniform) {
    // Assign a non-uniform value to a variable before and if-else statement, assign uniform values
    // in both branch of the if-else, and then use that variable for a conditional barrier after
    // the if-else statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  v = non_uniform;
  if (true) {
    v = 5;
  } else {
    v = 6;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_AssignNonUniformBeforeIf_OnlyTrueBranchAssignsUniform) {
    // Assign a non-uniform value to a variable before and if-else statement, assign a uniform value
    // in the true branch of the if-else, and then use that variable for a conditional barrier after
    // the if-else statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  v = non_uniform;
  if (true) {
    v = 5;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:12:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:11:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:6:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  v = non_uniform;
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, IfElse_AssignNonUniformBeforeIf_OnlyFalseBranchAssignsUniform) {
    // Assign a non-uniform value to a variable before and if-else statement, assign a uniform value
    // in the false branch of the if-else, and then use that variable for a conditional barrier
    // after the if-else statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  v = non_uniform;
  if (true) {
  } else {
    v = 5;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:13:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:12:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:6:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  v = non_uniform;
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       IfElse_AssignNonUniformBeforeIf_OnlyTrueBranchAssignsUniform_FalseBranchReturns) {
    // Assign a non-uniform value to a variable before and if-else statement, assign a uniform value
    // in the true branch of the if-else, leave the variable untouched in the false branch and just
    // return, and then use that variable for a conditional barrier after the if-else statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  v = non_uniform;
  if (true) {
    v = 5;
  } else {
    return;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       IfElse_AssignNonUniformBeforeIf_OnlyFalseBranchAssignsUniform_TrueBranchReturns) {
    // Assign a non-uniform value to a variable before and if-else statement, assign a uniform value
    // in the false branch of the if-else, leave the variable untouched in the true branch and just
    // return, and then use that variable for a conditional barrier after the if-else statement.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  v = non_uniform;
  if (true) {
    return;
  } else {
    v = 5;
  }

  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_NonUniformCondition_Reconverge) {
    // If statements reconverge at exit, so test that we can call workgroupBarrier() after an if
    // statement with a non-uniform condition.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (non_uniform == 42) {
  } else {
  }
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_ShortCircuitingNonUniformConditionLHS_Reconverge) {
    // If statements reconverge at exit, so test that we can call workgroupBarrier() after an if
    // statement with a non-uniform condition that uses short-circuiting.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (non_uniform == 42 || true) {
  }
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_ShortCircuitingNonUniformConditionRHS_Reconverge) {
    // If statements reconverge at exit, so test that we can call workgroupBarrier() after an if
    // statement with a non-uniform condition that uses short-circuiting.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (false && non_uniform == 42) {
  }
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_NonUniformFunctionCall_Reconverge) {
    // If statements reconverge at exit, so test that we can call workgroupBarrier() after an if
    // statement with a non-uniform condition.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar() {
  if (non_uniform == 42) {
    return;
  } else {
    return;
  }
}

fn foo() {
  if (non_uniform == 42) {
    bar();
  } else {
  }
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IfElse_NonUniformReturn_NoReconverge) {
    // If statements should not reconverge after non-uniform returns.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (non_uniform == 42) {
    return;
  } else {
  }
  workgroupBarrier();
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:3 error: 'workgroupBarrier' must only be called from uniform control flow
  workgroupBarrier();
  ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:5:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^
)");
}

////////////////////////////////////////////////////////////////////////////////
/// Switch statement tests.
////////////////////////////////////////////////////////////////////////////////

TEST_F(UniformityAnalysisTest, Switch_NonUniformCondition_BarrierInCase) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  switch (non_uniform) {
    case 42: {
      workgroupBarrier();
      break;
    }
    default: {
      break;
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  switch (non_uniform) {
  ^^^^^^

test:5:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  switch (non_uniform) {
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Switch_NonUniformCondition_BarrierInDefault) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  switch (non_uniform) {
    default: {
      workgroupBarrier();
      break;
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  switch (non_uniform) {
  ^^^^^^

test:5:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  switch (non_uniform) {
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Switch_NonUniformBreak) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  switch (condition) {
    case 42: {
      if (non_uniform == 42) {
        break;
      }
      workgroupBarrier();
    }
    default: {
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:11:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:8:7 note: control flow depends on possibly non-uniform value
      if (non_uniform == 42) {
      ^^

test:8:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      if (non_uniform == 42) {
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Switch_NonUniformBreakInDifferentCase) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  switch (condition) {
    case 0: {
      if (non_uniform == 42) {
        break;
      }
    }
    case 42: {
      workgroupBarrier();
    }
    default: {
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Switch_VarBecomesNonUniformInDifferentCase_WithBreak) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  var x = 0;
  switch (condition) {
    case 0: {
      x = non_uniform;
      break;
    }
    case 42: {
      if (x == 0) {
        workgroupBarrier();
      }
    }
    default: {
    }
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Switch_VarBecomesUniformInDifferentCase_WithBreak) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  var x = non_uniform;
  switch (condition) {
    case 0: {
      x = 5;
      break;
    }
    case 42: {
      if (x == 0) {
        workgroupBarrier();
      }
    }
    default: {
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:9 error: 'workgroupBarrier' must only be called from uniform control flow
        workgroupBarrier();
        ^^^^^^^^^^^^^^^^

test:13:7 note: control flow depends on possibly non-uniform value
      if (x == 0) {
      ^^

test:6:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var x = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Switch_VarBecomesNonUniformInCase_BarrierAfter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  var x = 0;
  switch (condition) {
    case 0: {
      x = non_uniform;
    }
    case 42: {
      x = 5;
    }
    default: {
      x = 6;
    }
  }
  if (x == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:19:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:18:3 note: control flow depends on possibly non-uniform value
  if (x == 0) {
  ^^

test:9:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      x = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Switch_VarBecomesUniformInAllCases_BarrierAfter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  var x = non_uniform;
  switch (condition) {
    case 0: {
      x = 4;
    }
    case 42: {
      x = 5;
    }
    default: {
      x = 6;
    }
  }
  if (x == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Switch_VarBecomesUniformInSomeCases_BarrierAfter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  var x = non_uniform;
  switch (condition) {
    case 0: {
      x = 4;
    }
    case 42: {
    }
    default: {
      x = 6;
    }
  }
  if (x == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:18:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:17:3 note: control flow depends on possibly non-uniform value
  if (x == 0) {
  ^^

test:6:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var x = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Switch_VarBecomesUniformInCasesThatDontReturn_BarrierAfter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  var x = non_uniform;
  switch (condition) {
    case 0: {
      x = 4;
    }
    case 42: {
      return;
    }
    default: {
      x = 6;
    }
  }
  if (x == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Switch_VarBecomesUniformAfterConditionalBreak_BarrierAfter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  var x = non_uniform;
  switch (condition) {
    case 0: {
      x = 4;
    }
    case 42: {
    }
    default: {
      if (false) {
        break;
      }
      x = 6;
    }
  }
  if (x == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:21:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:20:3 note: control flow depends on possibly non-uniform value
  if (x == 0) {
  ^^

test:6:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var x = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Switch_NestedInLoop_VarBecomesNonUniformWithBreak_BarrierInLoop) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  var x = 0;
  loop {
    if (x == 0) {
      workgroupBarrier();
      break;
    }

    switch (condition) {
      case 0: {
        x = non_uniform;
        break;
      }
      default: {
        x = 6;
      }
    }
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:8:5 note: control flow depends on possibly non-uniform value
    if (x == 0) {
    ^^

test:15:13 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
        x = non_uniform;
            ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Switch_NestedInLoop_VarBecomesNonUniformWithBreak_BarrierAfterLoop) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(0) var<uniform> condition : i32;

fn foo() {
  var x = 0;
  loop {
    if (false) {
      break;
    }
    switch (condition) {
      case 0: {
        x = non_uniform;
        break;
      }
      default: {
        x = 6;
      }
    }
    x = 5;
  }
  if (x == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Switch_NonUniformCondition_Reconverge) {
    // Switch statements reconverge at exit, so test that we can call workgroupBarrier() after a
    // switch statement that contains a non-uniform conditional break.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  switch (non_uniform) {
    default: {
      break;
    }
  }
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, Switch_NonUniformBreak_Reconverge) {
    // Switch statements reconverge at exit, so test that we can call workgroupBarrier() after a
    // switch statement that contains a non-uniform conditional break.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  switch (42) {
    default: {
      if (non_uniform == 0) {
        break;
      }
      break;
    }
  }
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

////////////////////////////////////////////////////////////////////////////////
/// Pointer tests.
////////////////////////////////////////////////////////////////////////////////

TEST_F(UniformityAnalysisTest, AssignNonUniformThroughPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  *&v = non_uniform;
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:6:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  *&v = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignNonUniformThroughCapturedPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  let pv = &v;
  *pv = non_uniform;
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:7:9 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  *pv = non_uniform;
        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignUniformThroughPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = non_uniform;
  *&v = 42;
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, AssignUniformThroughCapturedPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = non_uniform;
  let pv = &v;
  *pv = 42;
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, AssignUniformThroughCapturedPointer_InNonUniformControlFlow) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  let pv = &v;
  if (non_uniform == 0) {
    *pv = 42;
  }
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:11:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 0) {
  ^^

test:7:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 0) {
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformThroughPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = non_uniform;
  if (*&v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:6:3 note: control flow depends on possibly non-uniform value
  if (*&v == 0) {
  ^^

test:5:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformLocalThroughCapturedPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = non_uniform;
  let pv = &v;
  if (*pv == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (*pv == 0) {
  ^^

test:5:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformLocalThroughPointerParameter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  var v = non_uniform;
  bar(&v);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:4:8 note: parameter 'p' of 'bar' may point to a non-uniform value
fn bar(p : ptr<function, i32>) {
       ^

test:12:7 note: possibly non-uniform value passed via pointer here
  bar(&v);
      ^^

test:11:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformGlobalThroughCapturedPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  let pv = &non_uniform;
  if (*pv == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:6:3 note: control flow depends on possibly non-uniform value
  if (*pv == 0) {
  ^^

test:6:8 note: reading from 'pv' may result in a non-uniform value
  if (*pv == 0) {
       ^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformGlobalThroughPointerParameter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<storage, i32, read_write>) {
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  bar(&non_uniform);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:5:8 note: parameter 'p' of 'bar' may be non-uniform
  if (*p == 0) {
       ^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformGlobalThroughPointerParameter_ViaReturnValue) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<storage, i32, read_write>) -> i32 {
  return *p;
}

fn foo() {
  if (0 == bar(&non_uniform)) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (0 == bar(&non_uniform)) {
  ^^

test:9:12 note: return value of 'bar' may be non-uniform
  if (0 == bar(&non_uniform)) {
           ^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformThroughPointerParameter_BecomesUniformAfterUse) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  if (*p == 0) {
    workgroupBarrier();
  }
  *p = 0;
}

fn foo() {
  var v = non_uniform;
  bar(&v);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:4:8 note: parameter 'p' of 'bar' may point to a non-uniform value
fn bar(p : ptr<function, i32>) {
       ^

test:13:7 note: possibly non-uniform value passed via pointer here
  bar(&v);
      ^^

test:12:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformThroughPointerParameter_BecomesUniformAfterCall) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  var v = non_uniform;
  bar(&v);
  v = 0;
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:4:8 note: parameter 'p' of 'bar' may point to a non-uniform value
fn bar(p : ptr<function, i32>) {
       ^

test:12:7 note: possibly non-uniform value passed via pointer here
  bar(&v);
      ^^

test:11:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadUniformThroughPointer) {
    std::string src = R"(
fn foo() {
  var v = 42;
  if (*&v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, LoadUniformThroughCapturedPointer) {
    std::string src = R"(
fn foo() {
  var v = 42;
  let pv = &v;
  if (*pv == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, LoadUniformThroughPointerParameter) {
    std::string src = R"(
fn bar(p : ptr<function, i32>) {
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  var v = 42;
  bar(&v);
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, LoadUniformThroughNonUniformPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  // The contents of `v` are uniform.
  var v = array<i32, 4>();
  // The pointer `p` is non-uniform.
  let p = &v[non_uniform];
  if (*p == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:8:14 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  let p = &v[non_uniform];
             ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadUniformThroughNonUniformPointer_ViaParameter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, array<i32, 4>>) {
  // The pointer `p` is non-uniform.
  let local_p = &(*p)[non_uniform];
  if (*local_p == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  // The contents of `v` are uniform.
  var v = array<i32, 4>();
  let p = &v;
  bar(p);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (*local_p == 0) {
  ^^

test:6:23 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  let local_p = &(*p)[non_uniform];
                      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadUniformThroughNonUniformPointer_ViaParameterChain) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn zoo(p : ptr<function, i32>) {
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn bar(p : ptr<function, i32>) {
  zoo(p);
}

fn foo() {
  // The contents of `v` are uniform.
  var v = array<i32, 4>();
  // The pointer `p` is non-uniform.
  let p = &v[non_uniform];
  bar(p);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:4:8 note: parameter 'p' of 'zoo' may point to a non-uniform value
fn zoo(p : ptr<function, i32>) {
       ^

test:11:7 note: possibly non-uniform value passed via pointer here
  zoo(p);
      ^

test:10:8 note: reading from 'p' may result in a non-uniform value
fn bar(p : ptr<function, i32>) {
       ^

test:19:7 note: possibly non-uniform value passed via pointer here
  bar(p);
      ^

test:18:14 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  let p = &v[non_uniform];
             ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformThroughUniformPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var<storage, read> uniform_idx : i32;

fn foo() {
  // The contents of `v` are non-uniform.
  var v = array<i32, 4>(0, 0, 0, non_uniform);
  // The pointer `p` is uniform.
  let p = &v[uniform_idx];
  if (*p == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:11:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:10:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:7:34 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = array<i32, 4>(0, 0, 0, non_uniform);
                                 ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformThroughUniformPointer_ViaParameter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var<storage, read> uniform_idx : i32;

fn zoo(p : ptr<function, i32>) {
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn bar(p : ptr<function, i32>) {
  zoo(p);
}

fn foo() {
  // The contents of `v` are non-uniform.
  var v = array<i32, 4>(0, 0, 0, non_uniform);
  // The pointer `p` is uniform.
  let p = &v[uniform_idx];
  bar(p);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:6:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:5:8 note: parameter 'p' of 'zoo' may point to a non-uniform value
fn zoo(p : ptr<function, i32>) {
       ^

test:12:7 note: possibly non-uniform value passed via pointer here
  zoo(p);
      ^

test:11:8 note: reading from 'p' may result in a non-uniform value
fn bar(p : ptr<function, i32>) {
       ^

test:20:7 note: possibly non-uniform value passed via pointer here
  bar(p);
      ^

test:17:34 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = array<i32, 4>(0, 0, 0, non_uniform);
                                 ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, StoreNonUniformAfterCapturingPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  let pv = &v;
  v = non_uniform;
  if (*pv == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (*pv == 0) {
  ^^

test:7:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  v = non_uniform;
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, StoreUniformAfterCapturingPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = non_uniform;
  let pv = &v;
  v = 42;
  if (*pv == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, AssignNonUniformThroughLongChainOfPointers) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  let pv1 = &*&v;
  let pv2 = &*&*pv1;
  *&*&*pv2 = non_uniform;
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:8:14 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  *&*&*pv2 = non_uniform;
             ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, LoadNonUniformThroughLongChainOfPointers) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = non_uniform;
  let pv1 = &*&v;
  let pv2 = &*&*pv1;
  if (*&*&*pv2 == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (*&*&*pv2 == 0) {
  ^^

test:5:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignUniformThenNonUniformThroughDifferentPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  let pv1 = &v;
  let pv2 = &v;
  *pv1 = 42;
  *pv2 = non_uniform;
  if (*pv1 == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:11:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:10:3 note: control flow depends on possibly non-uniform value
  if (*pv1 == 0) {
  ^^

test:9:10 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  *pv2 = non_uniform;
         ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignNonUniformThenUniformThroughDifferentPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  var v = 0;
  let pv1 = &v;
  let pv2 = &v;
  *pv1 = non_uniform;
  *pv2 = 42;
  if (*pv1 == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, UnmodifiedPointerParameterNonUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
}

fn foo() {
  var v = non_uniform;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:11:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:10:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:8:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, UnmodifiedPointerParameterUniform) {
    std::string src = R"(
fn bar(p : ptr<function, i32>) {
}

fn foo() {
  var v = 42;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, AssignNonUniformThroughPointerInFunctionCall) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  *p = non_uniform;
}

fn foo() {
  var v = 0;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:12:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:11:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:10:7 note: contents of pointer may become non-uniform after calling 'bar'
  bar(&v);
      ^^
)");
}

TEST_F(UniformityAnalysisTest, AssignUniformThroughPointerInFunctionCall) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  *p = 42;
}

fn foo() {
  var v = non_uniform;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, AssignNonUniformThroughPointerInFunctionCallViaArg) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>, a : i32) {
  *p = a;
}

fn foo() {
  var v = 0;
  bar(&v, non_uniform);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:12:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:11:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:10:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  bar(&v, non_uniform);
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignNonUniformThroughPointerInFunctionCallViaPointerArg) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>, a : ptr<function, i32>) {
  *p = *a;
}

fn foo() {
  var v = 0;
  var a = non_uniform;
  bar(&v, &a);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:13:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:12:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:10:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var a = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignUniformThroughPointerInFunctionCallViaArg) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>, a : i32) {
  *p = a;
}

fn foo() {
  var v = non_uniform;
  bar(&v, 42);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, AssignUniformThroughPointerInFunctionCallViaPointerArg) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>, a : ptr<function, i32>) {
  *p = *a;
}

fn foo() {
  var v = non_uniform;
  var a = 42;
  bar(&v, &a);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, AssignNonUniformThroughPointerInFunctionCallChain) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn f3(p : ptr<function, i32>, a : ptr<function, i32>) {
  *p = *a;
}

fn f2(p : ptr<function, i32>, a : ptr<function, i32>) {
  f3(p, a);
}

fn f1(p : ptr<function, i32>, a : ptr<function, i32>) {
  f2(p, a);
}

fn foo() {
  var v = 0;
  var a = non_uniform;
  f1(&v, &a);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:21:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:20:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:18:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var a = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignUniformThroughPointerInFunctionCallChain) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn f3(p : ptr<function, i32>, a : ptr<function, i32>) {
  *p = *a;
}

fn f2(p : ptr<function, i32>, a : ptr<function, i32>) {
  f3(p, a);
}

fn f1(p : ptr<function, i32>, a : ptr<function, i32>) {
  f2(p, a);
}

fn foo() {
  var v = non_uniform;
  var a = 42;
  f1(&v, &a);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, MakePointerParamUniformInReturnExpression) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn zoo(p : ptr<function, i32>) -> i32 {
  *p = 5;
  return 6;
}

fn bar(p : ptr<function, i32>) -> i32 {
  *p = non_uniform;
  return zoo(p);
}

fn foo() {
  var v = 0;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, MakePointerParamNonUniformInReturnExpression) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn zoo(p : ptr<function, i32>) -> i32 {
  *p = non_uniform;
  return 6;
}

fn bar(p : ptr<function, i32>) -> i32 {
  *p = 5;
  return zoo(p);
}

fn foo() {
  var v = 0;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:18:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:17:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:16:7 note: contents of pointer may become non-uniform after calling 'bar'
  bar(&v);
      ^^
)");
}

TEST_F(UniformityAnalysisTest, PointerParamAssignNonUniformInTrueAndUniformInFalse) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  if (true) {
    *p = non_uniform;
  } else {
    *p = 5;
  }
}

fn foo() {
  var v = 0;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:16:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:15:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:14:7 note: contents of pointer may become non-uniform after calling 'bar'
  bar(&v);
      ^^
)");
}

TEST_F(UniformityAnalysisTest, ConditionalAssignNonUniformToPointerParamAndReturn) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  if (true) {
    *p = non_uniform;
    return;
  }
  *p = 5;
}

fn foo() {
  var v = 0;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:16:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:15:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:14:7 note: contents of pointer may become non-uniform after calling 'bar'
  bar(&v);
      ^^
)");
}

TEST_F(UniformityAnalysisTest, ConditionalAssignNonUniformToPointerParamAndBreakFromSwitch) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var<uniform> condition : i32;

fn bar(p : ptr<function, i32>) {
  switch (condition) {
    case 0 {
      if (true) {
        *p = non_uniform;
        break;
      }
      *p = 5;
    }
    default {
      *p = 6;
    }
  }
}

fn foo() {
  var v = 0;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:24:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:23:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:22:7 note: contents of pointer may become non-uniform after calling 'bar'
  bar(&v);
      ^^
)");
}

TEST_F(UniformityAnalysisTest, ConditionalAssignNonUniformToPointerParamAndBreakFromLoop) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  loop {
    if (true) {
      *p = non_uniform;
      break;
    }
    *p = 5;
  }
}

fn foo() {
  var v = 0;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:18:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:17:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:16:7 note: contents of pointer may become non-uniform after calling 'bar'
  bar(&v);
      ^^
)");
}

TEST_F(UniformityAnalysisTest, ConditionalAssignNonUniformToPointerParamAndContinue) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo(p : ptr<function, i32>) {
  loop {
    if (*p == 0) {
      workgroupBarrier();
      break;
    }

    if (true) {
      *p = non_uniform;
      continue;
    }
    *p = 5;
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:7 error: 'workgroupBarrier' must only be called from uniform control flow
      workgroupBarrier();
      ^^^^^^^^^^^^^^^^

test:6:5 note: control flow depends on possibly non-uniform value
    if (*p == 0) {
    ^^

test:12:12 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
      *p = non_uniform;
           ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, PointerParamMaybeBecomesUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  if (true) {
    *p = 5;
    return;
  }
}

fn foo() {
  var v = non_uniform;
  bar(&v);
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:15:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:14:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:12:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, PointerParamModifiedInNonUniformControlFlow) {
    std::string src = R"(
@binding(0) @group(0) var<storage, read_write> non_uniform_global : i32;

fn foo(p : ptr<function, i32>) {
  *p = 42;
}

@compute @workgroup_size(64)
fn main() {
  var a : i32;
  if (non_uniform_global == 0) {
    foo(&a);
  }

  if (a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:16:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:11:3 note: control flow depends on possibly non-uniform value
  if (non_uniform_global == 0) {
  ^^

test:11:7 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (non_uniform_global == 0) {
      ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, PointerParamAssumedModifiedInNonUniformControlFlow) {
    std::string src = R"(
@binding(0) @group(0) var<storage, read_write> non_uniform_global : i32;

fn foo(p : ptr<function, i32>) {
  // Do not modify 'p', uniformity analysis presently assumes it will be.
}

@compute @workgroup_size(64)
fn main() {
  var a : i32;
  if (non_uniform_global == 0) {
    foo(&a);
  }

  if (a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:16:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:11:3 note: control flow depends on possibly non-uniform value
  if (non_uniform_global == 0) {
  ^^

test:11:7 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (non_uniform_global == 0) {
      ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, PointerParamModifiedInNonUniformControlFlow_NestedCall) {
    std::string src = R"(
@binding(0) @group(0) var<storage, read_write> non_uniform_global : i32;

fn foo2(p : ptr<function, i32>) {
  *p = 42;
}

fn foo(p : ptr<function, i32>) {
  foo2(p);
}

@compute @workgroup_size(64)
fn main() {
  var a : i32;
  if (non_uniform_global == 0) {
    foo(&a);
  }

  if (a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:20:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:15:3 note: control flow depends on possibly non-uniform value
  if (non_uniform_global == 0) {
  ^^

test:15:7 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (non_uniform_global == 0) {
      ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, PointerParamModifiedInUniformControlFlow) {
    std::string src = R"(
@binding(0) @group(0) var<uniform> uniform_global : i32;

fn foo(p : ptr<function, i32>) {
  *p = 42;
}

@compute @workgroup_size(64)
fn main() {
  var a : i32;
  if (uniform_global == 0) {
    foo(&a);
  }

  if (a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, NonUniformPointerParameterBecomesUniform_AfterUse) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(a : ptr<function, i32>, b : ptr<function, i32>) {
  *b = *a;
  *a = 0;
}

fn foo() {
  var a = non_uniform;
  var b = 0;
  bar(&a, &b);
  if (b == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (b == 0) {
  ^^

test:10:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var a = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, NonUniformPointerParameterBecomesUniform_BeforeUse) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(a : ptr<function, i32>, b : ptr<function, i32>) {
  *a = 0;
  *b = *a;
}

fn foo() {
  var a = non_uniform;
  var b = 0;
  bar(&a, &b);
  if (b == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, UniformPointerParameterBecomesNonUniform_BeforeUse) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(a : ptr<function, i32>, b : ptr<function, i32>) {
  *a = non_uniform;
  *b = *a;
}

fn foo() {
  var a = 0;
  var b = 0;
  bar(&a, &b);
  if (b == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (b == 0) {
  ^^

test:12:11 note: contents of pointer may become non-uniform after calling 'bar'
  bar(&a, &b);
          ^^
)");
}

TEST_F(UniformityAnalysisTest, UniformPointerParameterBecomesNonUniform_AfterUse) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(a : ptr<function, i32>, b : ptr<function, i32>) {
  *b = *a;
  *a = non_uniform;
}

fn foo() {
  var a = 0;
  var b = 0;
  bar(&a, &b);
  if (b == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, NonUniformPointerParameterUpdatedInPlace) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  (*p)++;
}

fn foo() {
  var v = non_uniform;
  bar(&v);
  if (v == 1) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:12:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:11:3 note: control flow depends on possibly non-uniform value
  if (v == 1) {
  ^^

test:9:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var v = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, MultiplePointerParametersBecomeNonUniform) {
    // The analysis traverses the tree for each pointer parameter, and we need to make sure that we
    // reset the "visited" state of nodes in between these traversals to properly capture each of
    // their uniformity states.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(a : ptr<function, i32>, b : ptr<function, i32>) {
  *a = non_uniform;
  *b = non_uniform;
}

fn foo() {
  var a = 0;
  var b = 0;
  bar(&a, &b);
  if (b == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (b == 0) {
  ^^

test:12:11 note: contents of pointer may become non-uniform after calling 'bar'
  bar(&a, &b);
          ^^
)");
}

TEST_F(UniformityAnalysisTest, MultiplePointerParametersWithEdgesToEachOther) {
    // The analysis traverses the tree for each pointer parameter, and we need to make sure that we
    // reset the "visited" state of nodes in between these traversals to properly capture each of
    // their uniformity states.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(a : ptr<function, i32>, b : ptr<function, i32>, c : ptr<function, i32>) {
  *a = *a;
  *b = *b;
  *c = *a + *b;
}

fn foo() {
  var a = non_uniform;
  var b = 0;
  var c = 0;
  bar(&a, &b, &c);
  if (c == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:16:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:15:3 note: control flow depends on possibly non-uniform value
  if (c == 0) {
  ^^

test:11:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var a = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, MaximumNumberOfPointerParameters) {
    // Create a function with the maximum number of parameters, all pointers, to stress the
    // quadratic nature of the analysis.
    ProgramBuilder b;
    auto& ty = b.ty;

    // fn foo(p0 : ptr<function, i32>, p1 : ptr<function, i32>, ...) {
    //   let rhs = *p0 + *p1 + ... + *p244;
    //   *p1 = rhs;
    //   *p2 = rhs;
    //   ...
    //   *p254 = rhs;
    // }
    Vector<const ast::Parameter*, 8> params;
    Vector<const ast::Statement*, 8> foo_body;
    const ast::Expression* rhs_init = b.Deref("p0");
    for (int i = 1; i < 255; i++) {
        rhs_init = b.Add(rhs_init, b.Deref("p" + std::to_string(i)));
    }
    foo_body.Push(b.Decl(b.Let("rhs", rhs_init)));
    for (int i = 0; i < 255; i++) {
        params.Push(b.Param("p" + std::to_string(i), ty.ptr<function, i32>()));
        if (i > 0) {
            foo_body.Push(b.Assign(b.Deref("p" + std::to_string(i)), "rhs"));
        }
    }
    b.Func("foo", std::move(params), ty.void_(), foo_body);

    // var<private> non_uniform_global : i32;
    // fn main() {
    //   var v0 : i32;
    //   var v1 : i32;
    //   ...
    //   var v254 : i32;
    //   v0 = non_uniform_global;
    //   foo(&v0, &v1, ...,  &v254);
    //   if (v254 == 0) {
    //     workgroupBarrier();
    //   }
    // }
    b.GlobalVar("non_uniform_global", ty.i32(), core::AddressSpace::kPrivate);
    Vector<const ast::Statement*, 8> main_body;
    Vector<const ast::Expression*, 8> args;
    for (int i = 0; i < 255; i++) {
        auto name = "v" + std::to_string(i);
        main_body.Push(b.Decl(b.Var(name, ty.i32())));
        args.Push(b.AddressOf(name));
    }
    main_body.Push(b.Assign("v0", "non_uniform_global"));
    main_body.Push(b.CallStmt(b.Call("foo", args)));
    main_body.Push(b.If(b.Equal("v254", 0_i), b.Block(b.CallStmt(b.Call("workgroupBarrier")))));
    b.Func("main", tint::Empty, ty.void_(), main_body);

    RunTest(std::move(b), false);
    EXPECT_EQ(error_,
              R"(error: 'workgroupBarrier' must only be called from uniform control flow
note: control flow depends on possibly non-uniform value
note: reading from module-scope private variable 'non_uniform_global' may result in a non-uniform value)");
}

TEST_F(UniformityAnalysisTest, AssignUniformToPrivatePointerParameter_StillNonUniform) {
    std::string src = R"(
var<private> non_uniform : i32;

fn bar(p : ptr<private, i32>) {
  *p = 0;
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  bar(&non_uniform);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:6:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:6:8 note: parameter 'p' of 'bar' may be non-uniform
  if (*p == 0) {
       ^
)");
}

TEST_F(UniformityAnalysisTest, AssignUniformToWorkgroupPointerParameter_StillNonUniform) {
    std::string src = R"(
var<workgroup> non_uniform : i32;

fn bar(p : ptr<workgroup, i32>) {
  *p = 0;
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  bar(&non_uniform);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:6:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:6:8 note: parameter 'p' of 'bar' may be non-uniform
  if (*p == 0) {
       ^
)");
}

TEST_F(UniformityAnalysisTest, AssignUniformToStoragePointerParameter_StillNonUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<storage, i32, read_write>) {
  *p = 0;
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  bar(&non_uniform);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:6:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:6:8 note: parameter 'p' of 'bar' may be non-uniform
  if (*p == 0) {
       ^
)");
}

TEST_F(UniformityAnalysisTest, LoadFromReadOnlyStoragePointerParameter_AlwaysUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> non_uniform : i32;

fn bar(p : ptr<storage, i32, read>) {
  if (*p == 0) {
    workgroupBarrier();
  }
}

fn foo() {
  bar(&non_uniform);
}
)";

    RunTest(src, true);
}

////////////////////////////////////////////////////////////////////////////////
/// Tests to cover access to aggregate types.
////////////////////////////////////////////////////////////////////////////////

TEST_F(UniformityAnalysisTest, VectorElement_Uniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> v : vec4<i32>;

fn foo() {
  if (v[2] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, VectorElement_NonUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec4<i32>;

fn foo() {
  if (v[2] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (v[2] == 0) {
  ^^

test:5:7 note: reading from read_write storage buffer 'v' may result in a non-uniform value
  if (v[2] == 0) {
      ^
)");
}

TEST_F(UniformityAnalysisTest, VectorSwizzle_NonUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec4<i32>;

fn foo() {
  if (any(v.xy == vec2())) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (any(v.xy == vec2())) {
  ^^

test:5:11 note: reading from read_write storage buffer 'v' may result in a non-uniform value
  if (any(v.xy == vec2())) {
          ^
)");
}

TEST_F(UniformityAnalysisTest, VectorSwizzleFromPointer_NonUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> v : vec4<i32>;

fn foo() {
  if (any((&v).xy == vec2())) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (any((&v).xy == vec2())) {
  ^^

test:5:13 note: reading from read_write storage buffer 'v' may result in a non-uniform value
  if (any((&v).xy == vec2())) {
            ^
)");
}

TEST_F(UniformityAnalysisTest, VectorElement_BecomesNonUniform_BeforeCondition) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v[2] = rw;
  if (v[2] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (v[2] == 0) {
  ^^

test:6:10 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  v[2] = rw;
         ^^
)");
}

TEST_F(UniformityAnalysisTest, VectorElement_BecomesNonUniform_AfterCondition) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  if (v[2] == 0) {
    v[2] = rw;
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, VectorElement_DifferentElementBecomesNonUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v[1] = rw;
  if (v[2] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (v[2] == 0) {
  ^^

test:6:10 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  v[1] = rw;
         ^^
)");
}

TEST_F(UniformityAnalysisTest, VectorElement_ElementBecomesUniform) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v[1] = rw;
  v[1] = 42;
  if (v[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (v[1] == 0) {
  ^^

test:6:10 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  v[1] = rw;
         ^^
)");
}

TEST_F(UniformityAnalysisTest, VectorElement_VectorBecomesUniform_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v[1] = rw;
  v = vec4(1, 2, 3, 4);
  if (v[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, VectorElementViaMember_VectorBecomesUniform_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v.y = rw;
  v = vec4(1, 2, 3, 4);
  if (v.y == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, VectorElement_VectorBecomesUniform_ThroughPointer_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v[1] = rw;
  *(&v) = vec4(1, 2, 3, 4);
  if (v[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       VectorElement_VectorBecomesUniform_ThroughPointerChain_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v[1] = rw;
  *(&(*(&(*(&v))))) = vec4(1, 2, 3, 4);
  if (v[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       VectorElement_VectorBecomesUniform_ThroughCapturedPointer_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v[1] = rw;
  let p = &v;
  *p = vec4(1, 2, 3, 4);
  if (v[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, VectorElement_VectorBecomesUniform_PartialAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v[1] = rw;
  v = vec4(1, 2, 3, v[3]);
  if (v[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (v[1] == 0) {
  ^^

test:6:10 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  v[1] = rw;
         ^^
)");
}

TEST_F(UniformityAnalysisTest,
       VectorElement_VectorBecomesUniform_PartialAssignment_ViaPointerDerefIndex) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  let p = &v;
  (*p)[1] = rw;
  v = vec4(1, 2, 3, v[3]);
  if (v[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (v[1] == 0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  (*p)[1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest,
       VectorElement_VectorBecomesUniform_PartialAssignment_ViaPointerIndex) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  let p = &v;
  p[1] = rw;
  v = vec4(1, 2, 3, v[3]);
  if (v[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (v[1] == 0) {
  ^^

test:7:10 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  p[1] = rw;
         ^^
)");
}

TEST_F(UniformityAnalysisTest, VectorElementViaMember_VectorBecomesUniform_PartialAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v.y = rw;
  v = vec4(1, 2, 3, v.w);
  if (v.y == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (v.y == 0) {
  ^^

test:6:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  v.y = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest,
       VectorElementViaMember_VectorBecomesUniform_PartialAssignment_ViaPointerDerefDot) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  let p = &v;
  (*p).y = rw;
  v = vec4(1, 2, 3, v.w);
  if (v.y == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (v.y == 0) {
  ^^

test:7:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  (*p).y = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest,
       VectorElementViaMember_VectorBecomesUniform_PartialAssignment_ViaPointerDot) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  let p = &v;
  p.y = rw;
  v = vec4(1, 2, 3, v.w);
  if (v.y == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (v.y == 0) {
  ^^

test:7:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  p.y = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, VectorElement_DifferentElementBecomesUniform) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, the whole vector
    // is still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v : vec4<i32>;
  v[1] = rw;
  v[2] = 42;
  if (v[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (v[1] == 0) {
  ^^

test:6:10 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  v[1] = rw;
         ^^
)");
}

TEST_F(UniformityAnalysisTest, VectorElement_NonUniform_AnyBuiltin) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

fn foo() {
  var v : vec4<i32>;
  v[1] = non_uniform_global;
  if (any(v == vec4(42))) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (any(v == vec4(42))) {
  ^^

test:6:10 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  v[1] = non_uniform_global;
         ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, MatrixElement_ElementBecomesUniform) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  m[1][1] = 42.0;
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:6:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest, MatrixElement_ElementBecomesUniform_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  m = mat3x3<f32>(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0), vec3(7.0, 8.0, 9.0));
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, MatrixElement_ElementBecomesUniform_ThroughPointer_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  *(&m) = mat3x3<f32>(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0), vec3(7.0, 8.0, 9.0));
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_ElementBecomesUniform_ThroughPointerChain_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  *(&(*(&(*(&m))))) = mat3x3<f32>(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0), vec3(7.0, 8.0, 9.0));
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_ElementBecomesUniform_ThroughCapturedPointer_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  let p = &m;
  *p = mat3x3<f32>(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0), vec3(7.0, 8.0, 9.0));
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, MatrixElement_ColumnBecomesUniform) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  m[1] = vec3(0.0, 42.0, 0.0);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:6:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest, MatrixElement_ColumnBecomesUniform_ThroughPartialPointer) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  *(&(m[1])) = vec3(0.0, 42.0, 0.0);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:6:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest, MatrixElement_ColumnBecomesUniform_ThroughPartialPointerChain) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  *(&(*(&(m[1])))) = vec3(0.0, 42.0, 0.0);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:6:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest, MatrixElement_ColumnBecomesUniform_ThroughCapturedPartialPointer) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  let p = &m[1];
  m[1][1] = rw;
  *p = vec3(0.0, 42.0, 0.0);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_ColumnBecomesUniform_ThroughCapturedPartialPointer_PointerDerefIndex) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  let p = &m[1];
  m[1][1] = rw;
  (*p)[0] = 0.0;
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_ColumnBecomesUniform_ThroughCapturedPartialPointer_PointerIndex) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  let p = &m[1];
  m[1][1] = rw;
  p[0] = 0.0;
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_ColumnBecomesUniform_ThroughCapturedPartialPointer_PointerDerefDot) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  let p = &m[1];
  m[1][1] = rw;
  (*p).x = 0.0;
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_ColumnBecomesUniform_ThroughCapturedPartialPointer_PointerDot) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  let p = &m[1];
  m[1][1] = rw;
  p.x = 0.0;
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_ColumnBecomesUniform_ThroughCapturedPartialPointerChain) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  let p = &m[1];
  m[1][1] = rw;
  *(&(*p)) = vec3(0.0, 42.0, 0.0);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest, MatrixElement_ColumnBecomesUniform_ThroughCapturedPointer) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  let p = &m;
  m[1][1] = rw;
  (*p)[1] = vec3(0.0, 42.0, 0.0);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest, MatrixElement_MatrixBecomesUniform_PartialAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  m = mat3x3<f32>(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0), m[2]);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:6:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_MatrixBecomesUniform_PartialAssignment_ThroughPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  *(&m) = mat3x3<f32>(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0), m[2]);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:6:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_MatrixBecomesUniform_PartialAssignment_ThroughCapturedPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  let p = &m;
  m[1][1] = rw;
  *p = mat3x3<f32>(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0), (*p)[2]);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest,
       MatrixElement_MatrixBecomesUniform_PartialAssignment_ThroughCapturedPointerChain) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  let p = &(*(&m));
  m[1][1] = rw;
  *p = mat3x3<f32>(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0), (*p)[2]);
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:7:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest, MatrixElement_DifferentElementBecomesUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : f32;

fn foo() {
  var m : mat3x3<f32>;
  m[1][1] = rw;
  m[2][2] = 42.0;
  if (m[1][1] == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (m[1][1] == 0.0) {
  ^^

test:6:13 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  m[1][1] = rw;
            ^^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_Uniform) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read> s : S;

fn foo() {
  if (s.b == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, StructMember_NonUniform) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> s : S;

fn foo() {
  if (s.b == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (s.b == 0) {
  ^^

test:9:7 note: reading from read_write storage buffer 's' may result in a non-uniform value
  if (s.b == 0) {
      ^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_BecomesNonUniform_BeforeCondition) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  s.b = rw;
  if (s.b == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:12:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:11:3 note: control flow depends on possibly non-uniform value
  if (s.b == 0) {
  ^^

test:10:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.b = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_BecomesNonUniform_AfterCondition) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  if (s.b == 0) {
    s.b = rw;
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, StructMember_DifferentMemberBecomesNonUniform) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  s.a = rw;
  if (s.b == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:12:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:11:3 note: control flow depends on possibly non-uniform value
  if (s.b == 0) {
  ^^

test:10:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_MemberBecomesUniform) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to a member, that member is
    // still considered to be non-uniform.
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  s.a = rw;
  s.a = 0;
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:13:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:12:3 note: control flow depends on possibly non-uniform value
  if (s.a == 0) {
  ^^

test:10:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_MemberBecomesUniformThroughCapturedPointer) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to a member, that member is
    // still considered to be non-uniform.
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  let p = &s;
  s.a = rw;
  (*p).a = 0;
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (s.a == 0) {
  ^^

test:11:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_MemberBecomesUniformThroughPartialPointer) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to a member, that member is
    // still considered to be non-uniform.
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  s.a = rw;
  *&s.a = 0;
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:13:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:12:3 note: control flow depends on possibly non-uniform value
  if (s.a == 0) {
  ^^

test:10:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_MemberBecomesUniformThroughCapturedPartialPointer) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to a member, that member is
    // still considered to be non-uniform.
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  let p = &s.a;
  s.a = rw;
  (*p) = 0;
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (s.a == 0) {
  ^^

test:11:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_StructBecomesUniform_FullAssignment) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  s.a = rw;
  s = S(1, 2);
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, StructMember_StructBecomesUniform_PartialAssignment) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  s.a = rw;
  s = S(1, s.b);
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:13:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:12:3 note: control flow depends on possibly non-uniform value
  if (s.a == 0) {
  ^^

test:10:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_StructBecomesUniform_FullAssignment_ThroughPointer) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  s.a = rw;
  *(&s) = S(1, 2);
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       StructMember_StructBecomesUniform_FullAssignment_ThroughCapturedPointer) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  let p = &s;
  s.a = rw;
  *p = S(1, 2);
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       StructMember_StructBecomesUniform_FullAssignment_ThroughCapturedPointerChain) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  let p = &(*(&s));
  s.a = rw;
  *p = S(1, 2);
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, StructMember_StructBecomesUniform_PartialAssignment_ThroughPointer) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  s.a = rw;
  *(&s) = S(1, (*(&s)).b);
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:13:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:12:3 note: control flow depends on possibly non-uniform value
  if (s.a == 0) {
  ^^

test:10:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest,
       StructMember_StructBecomesUniform_PartialAssignment_ThroughCapturedPointer) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  let p = &s;
  s.a = rw;
  *p = S(1, (*p).b);
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (s.a == 0) {
  ^^

test:11:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest,
       StructMember_StructBecomesUniform_PartialAssignment_ThroughCapturedPointerChain) {
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  let p = &(*(&s));
  s.a = rw;
  *p = S(1, (*p).b);
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (s.a == 0) {
  ^^

test:11:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, StructMember_DifferentMemberBecomesUniform) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to a member, the whole struct
    // is still considered to be non-uniform.
    std::string src = R"(
struct S {
  a : i32,
  b : i32,
}
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var s : S;
  s.a = rw;
  s.b = 0;
  if (s.a == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:13:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:12:3 note: control flow depends on possibly non-uniform value
  if (s.a == 0) {
  ^^

test:10:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  s.a = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_Uniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> arr : array<i32>;

fn foo() {
  if (arr[7] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ArrayElement_NonUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<i32>;

fn foo() {
  if (arr[7] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (arr[7] == 0) {
  ^^

test:5:7 note: reading from read_write storage buffer 'arr' may result in a non-uniform value
  if (arr[7] == 0) {
      ^^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_BecomesNonUniform_BeforeCondition) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[2] = rw;
  if (arr[2] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (arr[2] == 0) {
  ^^

test:6:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[2] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_BecomesNonUniform_AfterCondition) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  if (arr[2] == 0) {
    arr[2] = rw;
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ArrayElement_DifferentElementBecomesNonUniform) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[1] = rw;
  if (arr[2] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (arr[2] == 0) {
  ^^

test:6:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[1] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_DifferentElementBecomesNonUniformThroughPartialPointer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  let pa = &arr[1];
  *pa = rw;
  if (arr[2] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (arr[2] == 0) {
  ^^

test:7:9 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  *pa = rw;
        ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_ElementBecomesUniform) {
    // For aggregate types, we conservatively consider them to be forever non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, that element is
    // still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[1] = rw;
  arr[1] = 42;
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (arr[1] == 0) {
  ^^

test:6:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[1] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_ElementBecomesUniform_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[1] = rw;
  arr = array<i32, 4>(1, 2, 3, 4);
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ArrayElement_ElementBecomesUniform_PartialAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[1] = rw;
  arr = array<i32, 4>(1, 2, 3, arr[3]);
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (arr[1] == 0) {
  ^^

test:6:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[1] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_DifferentElementBecomesUniform) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element, the whole array
    // is still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[1] = rw;
  arr[2] = 42;
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (arr[1] == 0) {
  ^^

test:6:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[1] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_ElementBecomesUniform_ThroughPartialPointer) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element through a
    // pointer, the whole array is still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[1] = rw;
  *(&(arr[2])) = 42;
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (arr[1] == 0) {
  ^^

test:6:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[1] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_ElementBecomesUniform_ThroughPartialPointerChain) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element through a
    // pointer, the whole array is still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[1] = rw;
  *(&(*(&(*(&(arr[2])))))) = 42;
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (arr[1] == 0) {
  ^^

test:6:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[1] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_ElementBecomesUniform_ThroughCapturedPartialPointer) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element through a
    // pointer, the whole array is still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  let pa = &arr[2];
  arr[1] = rw;
  *pa = 42;
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (arr[1] == 0) {
  ^^

test:7:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[1] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_ElementBecomesUniform_ThroughCapturedPartialPointerChain) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element through a
    // pointer, the whole array is still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  let pa = &(*(&arr[2]));
  arr[1] = rw;
  *pa = 42;
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (arr[1] == 0) {
  ^^

test:7:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[1] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_ElementBecomesUniform_ThroughCapturedPointer) {
    // For aggregate types, we conservatively consider them to be non-uniform once they
    // become non-uniform. Test that after assigning a uniform value to an element through a
    // pointer, the whole array is still considered to be non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  let pa = &arr;
  arr[1] = rw;
  (*pa)[2] = 42;
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (arr[1] == 0) {
  ^^

test:7:12 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  arr[1] = rw;
           ^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_ArrayBecomesUniform_ThroughPointer_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[1] = rw;
  *(&arr) = array<i32, 4>();
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_ArrayBecomesUniform_ThroughPointerChain_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[1] = rw;
  *(&(*(&(*(&arr))))) = array<i32, 4>();
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_ArrayBecomesUniform_ThroughCapturedPointer_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  let pa = &arr;
  arr[1] = rw;
  *pa = array<i32, 4>();
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_ArrayBecomesUniform_ThroughCapturedPointerChain_FullAssignment) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var arr : array<i32, 4>;
  let pa = &(*(&arr));
  arr[1] = rw;
  *pa = array<i32, 4>();
  if (arr[1] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ArrayElement_AssignUniformToElementWithNonUniformIndex) {
    std::string src = R"(
var<private> non_uniform : i32;

fn foo() {
  var arr : array<i32, 4>;
  arr[non_uniform] = 0;
  if (arr[0] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (arr[0] == 0) {
  ^^

test:6:7 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  arr[non_uniform] = 0;
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, ArrayElement_AssignUniformToElementWithNonUniformIndex_ViaPointer) {
    std::string src = R"(
var<private> non_uniform : i32;

fn foo() {
  var arr : array<i32, 4>;
  *&(arr[non_uniform]) = 0;
  if (arr[0] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (arr[0] == 0) {
  ^^

test:6:10 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  *&(arr[non_uniform]) = 0;
         ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_AssignUniformToElementWithNonUniformIndex_ViaPointer_ImplicitDeref) {
    std::string src = R"(
var<private> non_uniform : i32;

fn foo() {
  var arr : array<i32, 4>;
  (&arr)[non_uniform] = 0;
  if (arr[0] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (arr[0] == 0) {
  ^^

test:6:10 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  (&arr)[non_uniform] = 0;
         ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_AssignUniformToElementWithNonUniformIndex_ViaStoredPointer) {
    std::string src = R"(
var<private> non_uniform : i32;

fn foo() {
  var arr : array<i32, 4>;
  let p = &(arr[non_uniform]);
  *p = 0;
  if (arr[0] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (arr[0] == 0) {
  ^^

test:6:17 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  let p = &(arr[non_uniform]);
                ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_AssignUniformToElementWithNonUniformIndex_ViaPointerParameter) {
    std::string src = R"(
var<private> non_uniform : i32;

fn foo(param : ptr<function, array<i32, 4>>) {
  let p = &((*param)[non_uniform]);
  *p = 0;
  if ((*param)[0] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if ((*param)[0] == 0) {
  ^^

test:5:22 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  let p = &((*param)[non_uniform]);
                     ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_AssignUniformToElementWithNonUniformIndex_ViaPointerParameter_ImplicitDeref) {
    std::string src = R"(
var<private> non_uniform : i32;

fn foo(param : ptr<function, array<i32, 4>>) {
  let p = &(param[non_uniform]);
  *p = 0;
  if ((*param)[0] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if ((*param)[0] == 0) {
  ^^

test:5:19 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  let p = &(param[non_uniform]);
                  ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest,
       ArrayElement_AssignUniformToElementWithNonUniformIndex_ViaPartialPointerParameter) {
    std::string src = R"(
var<private> non_uniform : i32;

fn bar(p : ptr<function, i32>) {
  *p = 0;
}

fn foo() {
  var arr : array<i32, 4>;
  let p = &(arr[non_uniform]);
  bar(p);
  if (arr[0] == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:13:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:12:3 note: control flow depends on possibly non-uniform value
  if (arr[0] == 0) {
  ^^

test:10:17 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  let p = &(arr[non_uniform]);
                ^^^^^^^^^^^
)");
}

////////////////////////////////////////////////////////////////////////////////
/// Miscellaneous statement and expression tests.
////////////////////////////////////////////////////////////////////////////////

TEST_F(UniformityAnalysisTest, NonUniformDiscard) {
    // Non-uniform discard statements should not cause uniformity issues.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (non_uniform == 42) {
    discard;
  }
  _ = dpdx(1.0);
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, FunctionReconvergesOnExit) {
    // Call a function that has returns during non-uniform control flow, and test that the analysis
    // reconverges when returning to the caller.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

var<private> p : i32;

fn foo() {
  if (rw == 0) {
    p = 42;
    return;
  }
  p = 5;
  return;
}

fn main() {
  foo();
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, TypeInitializer) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

fn foo() {
  if (i32(non_uniform_global) == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (i32(non_uniform_global) == 0) {
  ^^

test:5:11 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (i32(non_uniform_global) == 0) {
          ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Conversion) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

fn foo() {
  if (f32(non_uniform_global) == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (f32(non_uniform_global) == 0.0) {
  ^^

test:5:11 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (f32(non_uniform_global) == 0.0) {
          ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Bitcast) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

fn foo() {
  if (bitcast<f32>(non_uniform_global) == 0.0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (bitcast<f32>(non_uniform_global) == 0.0) {
  ^^

test:5:20 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (bitcast<f32>(non_uniform_global) == 0.0) {
                   ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, CompoundAssignment_NonUniformRHS) {
    // Use compound assignment with a non-uniform RHS on a variable.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v = 0;
  v += rw;
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:6:8 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  v += rw;
       ^^
)");
}

TEST_F(UniformityAnalysisTest, CompoundAssignment_UniformRHS_StillNonUniform) {
    // Use compound assignment with a uniform RHS on a variable that is already non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v = rw;
  v += 1;
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:5:11 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  var v = rw;
          ^^
)");
}

TEST_F(UniformityAnalysisTest, CompoundAssignment_Global) {
    // Use compound assignment on a global variable.
    // Tests that we do not assume there is always a variable node for the LHS, but we still process
    // the expression.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

var<private> v : array<i32, 4>;

fn bar(p : ptr<function, i32>) -> i32 {
  if (*p == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var f = rw;
  v[bar(&f)] += 1;
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:6:8 note: parameter 'p' of 'bar' may point to a non-uniform value
fn bar(p : ptr<function, i32>) -> i32 {
       ^

test:15:9 note: possibly non-uniform value passed via pointer here
  v[bar(&f)] += 1;
        ^^

test:14:11 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  var f = rw;
          ^^
)");
}

TEST_F(UniformityAnalysisTest, IncDec_StillNonUniform) {
    // Use increment on a variable that is already non-uniform.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  var v = rw;
  v++;
  if (v == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (v == 0) {
  ^^

test:5:11 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  var v = rw;
          ^^
)");
}

TEST_F(UniformityAnalysisTest, IncDec_Global) {
    // Use increment on a global variable.
    // Tests that we do not assume there is always a variable node for the LHS, but we still process
    // the expression.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> rw : i32;

var<private> v : array<i32, 4>;

fn bar(p : ptr<function, i32>) -> i32 {
  if (*p == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var f = rw;
  v[bar(&f)]++;
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:6:8 note: parameter 'p' of 'bar' may point to a non-uniform value
fn bar(p : ptr<function, i32>) -> i32 {
       ^

test:15:9 note: possibly non-uniform value passed via pointer here
  v[bar(&f)]++;
        ^^

test:14:11 note: reading from read_write storage buffer 'rw' may result in a non-uniform value
  var f = rw;
          ^^
)");
}

TEST_F(UniformityAnalysisTest, AssignmentEval_LHS_Then_RHS_Pass) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn b(p : ptr<function, i32>) -> i32 {
  *p = non_uniform;
  return 0;
}

fn a(p : ptr<function, i32>) -> i32 {
  if (*p == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var i = 0;
  var arr : array<i32, 4>;
  arr[a(&i)] = arr[b(&i)];
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, AssignmentEval_LHS_Then_RHS_Fail) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn a(p : ptr<function, i32>) -> i32 {
  *p = non_uniform;
  return 0;
}

fn b(p : ptr<function, i32>) -> i32 {
  if (*p == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var i = 0;
  var arr : array<i32, 4>;
  arr[a(&i)] = arr[b(&i)];
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:11:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:10:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:9:6 note: parameter 'p' of 'b' may point to a non-uniform value
fn b(p : ptr<function, i32>) -> i32 {
     ^

test:19:22 note: possibly non-uniform value passed via pointer here
  arr[a(&i)] = arr[b(&i)];
                     ^^

test:19:9 note: contents of pointer may become non-uniform after calling 'a'
  arr[a(&i)] = arr[b(&i)];
        ^^
)");
}

TEST_F(UniformityAnalysisTest, AssignmentEval_LHSContainsViolation) {
    std::string src = R"(
var<private> non_uniform : i32;

fn bar(cond : i32) -> i32 {
  if (cond == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var arr : array<i32, 4>;
  arr[bar(non_uniform)] = 0;
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (cond == 0) {
  ^^

test:4:8 note: parameter 'cond' of 'bar' may be non-uniform
fn bar(cond : i32) -> i32 {
       ^^^^

test:13:11 note: possibly non-uniform value passed here
  arr[bar(non_uniform)] = 0;
          ^^^^^^^^^^^

test:13:11 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  arr[bar(non_uniform)] = 0;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignmentEval_LHSContainsViolation_ViaExplicitDeref) {
    std::string src = R"(
var<private> non_uniform : i32;

fn bar(cond : i32) -> i32 {
  if (cond == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var arr : array<i32, 4>;
  *&(arr[bar(non_uniform)]) = 0;
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (cond == 0) {
  ^^

test:4:8 note: parameter 'cond' of 'bar' may be non-uniform
fn bar(cond : i32) -> i32 {
       ^^^^

test:13:14 note: possibly non-uniform value passed here
  *&(arr[bar(non_uniform)]) = 0;
             ^^^^^^^^^^^

test:13:14 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  *&(arr[bar(non_uniform)]) = 0;
             ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignmentEval_LHSContainsViolation_ViaPointerArrayIndex) {
    std::string src = R"(
var<private> non_uniform : i32;

fn bar(cond : i32) -> i32 {
  if (cond == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var arr : array<i32, 4>;
  (&arr)[bar(non_uniform)] = 0;
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (cond == 0) {
  ^^

test:4:8 note: parameter 'cond' of 'bar' may be non-uniform
fn bar(cond : i32) -> i32 {
       ^^^^

test:13:14 note: possibly non-uniform value passed here
  (&arr)[bar(non_uniform)] = 0;
             ^^^^^^^^^^^

test:13:14 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  (&arr)[bar(non_uniform)] = 0;
             ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, AssignmentEval_LHSContainsViolation_ViaPointerMemberAccessor) {
    std::string src = R"(
var<private> non_uniform : i32;

fn bar(cond : i32) -> i32 {
  if (cond == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var arr : array<vec4i, 4>;
  (&(arr[bar(non_uniform)])).y = 0;
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (cond == 0) {
  ^^

test:4:8 note: parameter 'cond' of 'bar' may be non-uniform
fn bar(cond : i32) -> i32 {
       ^^^^

test:13:14 note: possibly non-uniform value passed here
  (&(arr[bar(non_uniform)])).y = 0;
             ^^^^^^^^^^^

test:13:14 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  (&(arr[bar(non_uniform)])).y = 0;
             ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, CompoundAssignmentEval_LHS_Then_RHS_Pass) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn b(p : ptr<function, i32>) -> i32 {
  *p = non_uniform;
  return 0;
}

fn a(p : ptr<function, i32>) -> i32 {
  if (*p == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var i = 0;
  var arr : array<i32, 4>;
  arr[a(&i)] += arr[b(&i)];
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, CompoundAssignmentEval_LHS_Then_RHS_Fail) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn a(p : ptr<function, i32>) -> i32 {
  *p = non_uniform;
  return 0;
}

fn b(p : ptr<function, i32>) -> i32 {
  if (*p == 0) {
    workgroupBarrier();
  }
  return 0;
}

fn foo() {
  var i = 0;
  var arr : array<i32, 4>;
  arr[a(&i)] += arr[b(&i)];
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:11:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:10:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:9:6 note: parameter 'p' of 'b' may point to a non-uniform value
fn b(p : ptr<function, i32>) -> i32 {
     ^

test:19:23 note: possibly non-uniform value passed via pointer here
  arr[a(&i)] += arr[b(&i)];
                      ^^

test:19:9 note: contents of pointer may become non-uniform after calling 'a'
  arr[a(&i)] += arr[b(&i)];
        ^^
)");
}

TEST_F(UniformityAnalysisTest, CompoundAssignmentEval_RHS_Makes_LHS_NonUniform_After_Load) {
    // Test that the LHS is loaded from before the RHS makes is evaluated.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) -> i32 {
  *p = non_uniform;
  return 0;
}

fn foo() {
  var i = 0;
  var arr : array<i32, 4>;
  i += arr[bar(&i)];
  if (i == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, CompoundAssignmentEval_RHS_Makes_LHS_Uniform_After_Load) {
    // Test that the LHS is loaded from before the RHS makes is evaluated.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) -> i32 {
  *p = 0;
  return 0;
}

fn foo() {
  var i = non_uniform;
  var arr : array<i32, 4>;
  i += arr[bar(&i)];
  if (i == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (i == 0) {
  ^^

test:10:11 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  var i = non_uniform;
          ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, CompoundAssignmentEval_LHS_OnlyOnce) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) -> i32 {
  if (*p == 0) {
    workgroupBarrier();
  }
  *p = non_uniform;
  return 0;
}

fn foo(){
  var f : i32 = 0;
  var arr : array<i32, 4>;
  arr[bar(&f)] += 1;
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, IncDec_LHS_OnlyOnce) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar(p : ptr<function, i32>) -> i32 {
  if (*p == 0) {
    workgroupBarrier();
  }
  *p = non_uniform;
  return 0;
}

fn foo(){
  var f : i32 = 0;
  var arr : array<i32, 4>;
  arr[bar(&f)]++;
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ShortCircuiting_UniformLHS) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read> uniform_global : i32;

fn main() {
  let b = (uniform_global == 0) && (dpdx(1.0) == 0.0);
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ShortCircuiting_NonUniformLHS) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

fn main() {
  let b = (non_uniform_global == 0) && (dpdx(1.0) == 0.0);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:5:41 error: 'dpdx' must only be called from uniform control flow
  let b = (non_uniform_global == 0) && (dpdx(1.0) == 0.0);
                                        ^^^^^^^^^

test:5:11 note: control flow depends on possibly non-uniform value
  let b = (non_uniform_global == 0) && (dpdx(1.0) == 0.0);
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

test:5:12 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  let b = (non_uniform_global == 0) && (dpdx(1.0) == 0.0);
           ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, ShortCircuiting_ReconvergeLHS) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

fn main() {
  let b = (non_uniform_global == 0) && false;
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ShortCircuiting_ReconvergeRHS) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

fn main() {
  let b = false && (non_uniform_global == 0);
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ShortCircuiting_ReconvergeBoth) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

fn main() {
  let b = (non_uniform_global != 0) && (non_uniform_global != 42);
  workgroupBarrier();
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, DeadCode_AfterReturn) {
    // Dead code after a return statement shouldn't cause uniformity errors.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  return;
  if (non_uniform == 42) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ArrayLength) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<f32>;

fn foo() {
  for (var i = 0u; i < arrayLength(&arr); i++) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ArrayLength_OnPtrArg) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<f32>;

fn bar(p : ptr<storage, array<f32>, read_write>) {
  for (var i = 0u; i < arrayLength(p); i++) {
    workgroupBarrier();
  }
}

fn foo() {
  bar(&arr);
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ArrayLength_PtrArgRequiredToBeUniformForRetval_Pass) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<f32>;

fn length(p : ptr<storage, array<f32>, read_write>) -> u32 {
  return arrayLength(p);
}

fn foo() {
  for (var i = 0u; i < length(&arr); i++) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ArrayLength_PtrArgRequiredToBeUniformForOtherPtrResult_Pass) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<f32>;

fn length(p : ptr<storage, array<f32>, read_write>, out : ptr<function, u32>) {
  *out = arrayLength(p);
}

fn foo() {
  var len : u32;
  length(&arr, &len);
  if (len > 10) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, ArrayLength_PtrArgRequiresUniformityAndAffectsReturnValue) {
    // Test that a single pointer argument can directly require uniformity as well as affecting the
    // uniformity of the return value.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> arr : array<u32>;

fn bar(p : ptr<storage, array<u32>, read_write>) -> u32 {
  // This requires `p` to always be uniform.
  if (arrayLength(p) == 10) {
    workgroupBarrier();
  }

  // This requires the contents of `p` to be uniform in order for the return value to be uniform.
  return (*p)[0];
}

fn foo() {
  let p = &arr;
  // We pass a uniform pointer, so the direct uniformity requirement on the parameter is satisfied.
  if (0 == bar(p)) {
    // This will fail as the return value of `p` is non-uniform due to non-uniform contents of `p`.
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:19:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:17:3 note: control flow depends on possibly non-uniform value
  if (0 == bar(p)) {
  ^^

test:17:12 note: return value of 'bar' may be non-uniform
  if (0 == bar(p)) {
           ^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, WorkgroupUniformLoad) {
    std::string src = R"(
const wgsize = 4;
var<workgroup> data : array<u32, wgsize>;

@compute @workgroup_size(wgsize)
fn main(@builtin(local_invocation_index) idx : u32) {
  data[idx] = idx + 1;
  if (workgroupUniformLoad(&data[0]) > 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, WorkgroupUniformLoad_ViaPtrArg) {
    std::string src = R"(
const wgsize = 4;
var<workgroup> data : array<u32, wgsize>;

fn foo(p : ptr<workgroup, u32>) -> u32 {
  return workgroupUniformLoad(p);
}

@compute @workgroup_size(wgsize)
fn main(@builtin(local_invocation_index) idx : u32) {
  data[idx] = idx + 1;
  if (foo(&data[0]) > 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, WorkgroupUniformLoad_NonUniformPtr) {
    std::string src = R"(
const wgsize = 4;
var<workgroup> data : array<u32, wgsize>;

@compute @workgroup_size(wgsize)
fn main(@builtin(local_invocation_index) idx : u32) {
  data[idx] = idx + 1;
  if (workgroupUniformLoad(&data[idx]) > 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:28 error: 'workgroupUniformLoad' requires argument 0 to be uniform
  if (workgroupUniformLoad(&data[idx]) > 0) {
                           ^^^^^^^^^^

test:8:34 note: builtin 'idx' of 'main' may be non-uniform
  if (workgroupUniformLoad(&data[idx]) > 0) {
                                 ^^^
)");
}

TEST_F(UniformityAnalysisTest, WorkgroupUniformLoad_NonUniformPtr_ViaPtrArg) {
    std::string src = R"(
const wgsize = 4;
var<workgroup> data : array<u32, wgsize>;

fn foo(p : ptr<workgroup, u32>) -> u32 {
  return workgroupUniformLoad(p);
}

@compute @workgroup_size(wgsize)
fn main(@builtin(local_invocation_index) idx : u32) {
  data[idx] = idx + 1;
  if (foo(&data[idx]) > 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:31 error: 'workgroupUniformLoad' requires argument 0 to be uniform
  return workgroupUniformLoad(p);
                              ^

test:6:31 note: parameter 'p' of 'foo' may be non-uniform
  return workgroupUniformLoad(p);
                              ^

test:12:11 note: possibly non-uniform value passed here
  if (foo(&data[idx]) > 0) {
          ^^^^^^^^^^

test:12:17 note: builtin 'idx' of 'main' may be non-uniform
  if (foo(&data[idx]) > 0) {
                ^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupShuffleDown_Delta) {
    std::string src = R"(
enable subgroups;

var<private> delta: u32;

fn foo() {
  _ = subgroupShuffleDown(1.0, delta);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:32 error: 'subgroupShuffleDown' requires argument 1 to be uniform
  _ = subgroupShuffleDown(1.0, delta);
                               ^^^^^

test:7:32 note: reading from module-scope private variable 'delta' may result in a non-uniform value
  _ = subgroupShuffleDown(1.0, delta);
                               ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupShuffleUp_Delta) {
    std::string src = R"(
enable subgroups;

var<private> delta: u32;

fn foo() {
  _ = subgroupShuffleUp(1.0, delta);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:30 error: 'subgroupShuffleUp' requires argument 1 to be uniform
  _ = subgroupShuffleUp(1.0, delta);
                             ^^^^^

test:7:30 note: reading from module-scope private variable 'delta' may result in a non-uniform value
  _ = subgroupShuffleUp(1.0, delta);
                             ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupShuffleXor_Delta) {
    std::string src = R"(
enable subgroups;

var<private> delta: u32;

fn foo() {
  _ = subgroupShuffleXor(1.0, delta);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:7:31 error: 'subgroupShuffleXor' requires argument 1 to be uniform
  _ = subgroupShuffleXor(1.0, delta);
                              ^^^^^

test:7:31 note: reading from module-scope private variable 'delta' may result in a non-uniform value
  _ = subgroupShuffleXor(1.0, delta);
                              ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, WorkgroupAtomics) {
    std::string src = R"(
var<workgroup> a : atomic<i32>;

fn foo() {
  if (atomicAdd(&a, 1) == 1) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (atomicAdd(&a, 1) == 1) {
  ^^

test:5:7 note: return value of 'atomicAdd' may be non-uniform
  if (atomicAdd(&a, 1) == 1) {
      ^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, StorageAtomics) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> a : atomic<i32>;

fn foo() {
  if (atomicAdd(&a, 1) == 1) {
    storageBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'storageBarrier' must only be called from uniform control flow
    storageBarrier();
    ^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (atomicAdd(&a, 1) == 1) {
  ^^

test:5:7 note: return value of 'atomicAdd' may be non-uniform
  if (atomicAdd(&a, 1) == 1) {
      ^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, StorageTextureLoad_ReadOnly) {
    std::string src = R"(
@group(0) @binding(0) var t : texture_storage_2d<r32sint, read>;

fn foo() {
  if (textureLoad(t, vec2()).r == 0) {
    storageBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, StorageTextureLoad_ReadWrite) {
    std::string src = R"(
@group(0) @binding(0) var t : texture_storage_2d<r32sint, read_write>;

fn foo() {
  if (textureLoad(t, vec2()).r == 0) {
    storageBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'storageBarrier' must only be called from uniform control flow
    storageBarrier();
    ^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (textureLoad(t, vec2()).r == 0) {
  ^^

test:5:7 note: return value of 'textureLoad' may be non-uniform
  if (textureLoad(t, vec2()).r == 0) {
      ^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, DisableAnalysisWithExtension) {
    std::string src = R"(
enable chromium_disable_uniformity_analysis;

@group(0) @binding(0) var<storage, read_write> rw : i32;

fn foo() {
  if (rw == 0) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, true);
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixConstructor_NestedInStruct) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

struct Inner {
  u : u32,
  m : subgroup_matrix_result<f32, 8, 8>,
}

struct S {
  u : u32,
  inner : Inner,
}

fn foo() {
  if (non_uniform_global == 0) {
    _ = S();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:18:9 error: 'S' must only be called from subgroup uniform control flow
    _ = S();
        ^^^

test:17:3 note: control flow depends on possibly non-uniform value
  if (non_uniform_global == 0) {
  ^^

test:17:7 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (non_uniform_global == 0) {
      ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixConstructor_NestedInArray) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> non_uniform_global : i32;

alias ArrayType = array<array<subgroup_matrix_result<f32, 8, 8>, 4>, 4>;

fn foo() {
  if (non_uniform_global == 0) {
    _ = ArrayType();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:9 error: 'ArrayType' must only be called from subgroup uniform control flow
    _ = ArrayType();
        ^^^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (non_uniform_global == 0) {
  ^^

test:9:7 note: reading from read_write storage buffer 'non_uniform_global' may result in a non-uniform value
  if (non_uniform_global == 0) {
      ^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixConstructor_NonUniformArgument) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: f32;

fn foo() {
  _ = subgroup_matrix_result<f32, 8, 8>(non_uniform);
}
)";

    RunTest(src, false);
    EXPECT_EQ(
        error_,
        R"(test:7:41 error: subgroup_matrix_result<f32, 8, 8> constructor requires argument 0 to be uniform
  _ = subgroup_matrix_result<f32, 8, 8>(non_uniform);
                                        ^^^^^^^^^^^

test:7:41 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  _ = subgroup_matrix_result<f32, 8, 8>(non_uniform);
                                        ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixLoad_NonUniformPointer) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<array<f32, 64>, 4>;

fn foo() {
  let p = &buffer[non_uniform];
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>(p, 0, false, 4);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:61 error: 'subgroupMatrixLoad' requires argument 0 to be uniform
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>(p, 0, false, 4);
                                                            ^

test:9:19 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  let p = &buffer[non_uniform];
                  ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixLoad_NonUniformOffset) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn foo() {
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>(&buffer, non_uniform, false, 4);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:70 error: 'subgroupMatrixLoad' requires argument 1 to be uniform
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>(&buffer, non_uniform, false, 4);
                                                                     ^^^^^^^^^^^

test:9:70 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>(&buffer, non_uniform, false, 4);
                                                                     ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixLoad_NonUniformStride) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn foo() {
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>(&buffer, 0, false, non_uniform);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:9:80 error: 'subgroupMatrixLoad' requires argument 3 to be uniform
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>(&buffer, 0, false, non_uniform);
                                                                               ^^^^^^^^^^^

test:9:80 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>>(&buffer, 0, false, non_uniform);
                                                                               ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixStore_NonUniformPointer) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<array<f32, 64>, 4>;

fn foo() {
  let p = &buffer[non_uniform];
  let value = subgroup_matrix_result<f32, 8, 8>();
  subgroupMatrixStore(p, 0, value, false, 4);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:11:23 error: 'subgroupMatrixStore' requires argument 0 to be uniform
  subgroupMatrixStore(p, 0, value, false, 4);
                      ^

test:9:19 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  let p = &buffer[non_uniform];
                  ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixStore_NonUniformOffset) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn foo() {
  let value = subgroup_matrix_result<f32, 8, 8>();
  subgroupMatrixStore(&buffer, non_uniform, value, false, 4);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:32 error: 'subgroupMatrixStore' requires argument 1 to be uniform
  subgroupMatrixStore(&buffer, non_uniform, value, false, 4);
                               ^^^^^^^^^^^

test:10:32 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  subgroupMatrixStore(&buffer, non_uniform, value, false, 4);
                               ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixStore_NonUniformValue) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn bar() -> subgroup_matrix_result<f32, 8, 8> {
  var value1 = subgroup_matrix_result<f32, 8, 8>(1);
  var value2 = subgroup_matrix_result<f32, 8, 8>(2);
  if (non_uniform == 1) {
    return value1;
  } else {
    return value2;
  }
}

fn foo() {
  subgroupMatrixStore(&buffer, 0, bar(), false, 4);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:19:35 error: 'subgroupMatrixStore' requires argument 2 to be uniform
  subgroupMatrixStore(&buffer, 0, bar(), false, 4);
                                  ^^^^^

test:19:35 note: return value of 'bar' may be non-uniform
  subgroupMatrixStore(&buffer, 0, bar(), false, 4);
                                  ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixStore_NonUniformStride) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn foo() {
  let value = subgroup_matrix_result<f32, 8, 8>();
  subgroupMatrixStore(&buffer, 0, value, false, non_uniform);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:49 error: 'subgroupMatrixStore' requires argument 4 to be uniform
  subgroupMatrixStore(&buffer, 0, value, false, non_uniform);
                                                ^^^^^^^^^^^

test:10:49 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  subgroupMatrixStore(&buffer, 0, value, false, non_uniform);
                                                ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixMultiply_NonUniformLHS) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn bar() -> subgroup_matrix_left<f32, 8, 8> {
  var value1 = subgroup_matrix_left<f32, 8, 8>(1);
  var value2 = subgroup_matrix_left<f32, 8, 8>(2);
  if (non_uniform == 1) {
    return value1;
  } else {
    return value2;
  }
}

fn foo() {
  let rhs = subgroup_matrix_right<f32, 8, 8>();
  _ = subgroupMatrixMultiply<f32>(bar(), rhs);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:20:35 error: 'subgroupMatrixMultiply' requires argument 0 to be uniform
  _ = subgroupMatrixMultiply<f32>(bar(), rhs);
                                  ^^^^^

test:20:35 note: return value of 'bar' may be non-uniform
  _ = subgroupMatrixMultiply<f32>(bar(), rhs);
                                  ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixMultiply_NonUniformRHS) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn bar() -> subgroup_matrix_right<f32, 8, 8> {
  var value1 = subgroup_matrix_right<f32, 8, 8>(1);
  var value2 = subgroup_matrix_right<f32, 8, 8>(2);
  if (non_uniform == 1) {
    return value1;
  } else {
    return value2;
  }
}

fn foo() {
  let lhs = subgroup_matrix_left<f32, 8, 8>();
  _ = subgroupMatrixMultiply<f32>(lhs, bar());
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:20:40 error: 'subgroupMatrixMultiply' requires argument 1 to be uniform
  _ = subgroupMatrixMultiply<f32>(lhs, bar());
                                       ^^^^^

test:20:40 note: return value of 'bar' may be non-uniform
  _ = subgroupMatrixMultiply<f32>(lhs, bar());
                                       ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixMultiplyAccumulate_NonUniformLHS) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn bar() -> subgroup_matrix_left<f32, 8, 8> {
  var value1 = subgroup_matrix_left<f32, 8, 8>(1);
  var value2 = subgroup_matrix_left<f32, 8, 8>(2);
  if (non_uniform == 1) {
    return value1;
  } else {
    return value2;
  }
}

fn foo() {
  let rhs = subgroup_matrix_right<f32, 8, 8>();
  let acc = subgroup_matrix_result<f32, 8, 8>();
  _ = subgroupMatrixMultiplyAccumulate(bar(), rhs, acc);
}
)";

    RunTest(src, false);
    EXPECT_EQ(
        error_,
        R"(test:21:40 error: 'subgroupMatrixMultiplyAccumulate' requires argument 0 to be uniform
  _ = subgroupMatrixMultiplyAccumulate(bar(), rhs, acc);
                                       ^^^^^

test:21:40 note: return value of 'bar' may be non-uniform
  _ = subgroupMatrixMultiplyAccumulate(bar(), rhs, acc);
                                       ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixMultiplyAccumulate_NonUniformRHS) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn bar() -> subgroup_matrix_right<f32, 8, 8> {
  var value1 = subgroup_matrix_right<f32, 8, 8>(1);
  var value2 = subgroup_matrix_right<f32, 8, 8>(2);
  if (non_uniform == 1) {
    return value1;
  } else {
    return value2;
  }
}

fn foo() {
  let lhs = subgroup_matrix_left<f32, 8, 8>();
  let acc = subgroup_matrix_result<f32, 8, 8>();
  _ = subgroupMatrixMultiplyAccumulate(lhs, bar(), acc);
}
)";

    RunTest(src, false);
    EXPECT_EQ(
        error_,
        R"(test:21:45 error: 'subgroupMatrixMultiplyAccumulate' requires argument 1 to be uniform
  _ = subgroupMatrixMultiplyAccumulate(lhs, bar(), acc);
                                            ^^^^^

test:21:45 note: return value of 'bar' may be non-uniform
  _ = subgroupMatrixMultiplyAccumulate(lhs, bar(), acc);
                                            ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, SubgroupMatrixMultiplyAccumulate_NonUniformAcc) {
    std::string src = R"(
enable chromium_experimental_subgroup_matrix;

var<private> non_uniform: u32;

var<workgroup> buffer : array<f32, 64>;

fn bar() -> subgroup_matrix_result<f32, 8, 8> {
  var value1 = subgroup_matrix_result<f32, 8, 8>(1);
  var value2 = subgroup_matrix_result<f32, 8, 8>(2);
  if (non_uniform == 1) {
    return value1;
  } else {
    return value2;
  }
}

fn foo() {
  let lhs = subgroup_matrix_left<f32, 8, 8>();
  let rhs = subgroup_matrix_right<f32, 8, 8>();
  _ = subgroupMatrixMultiplyAccumulate(lhs, rhs, bar());
}
)";

    RunTest(src, false);
    EXPECT_EQ(
        error_,
        R"(test:21:50 error: 'subgroupMatrixMultiplyAccumulate' requires argument 2 to be uniform
  _ = subgroupMatrixMultiplyAccumulate(lhs, rhs, bar());
                                                 ^^^^^

test:21:50 note: return value of 'bar' may be non-uniform
  _ = subgroupMatrixMultiplyAccumulate(lhs, rhs, bar());
                                                 ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, StressGraphTraversalDepth) {
    // Create a function with a very long sequence of variable declarations and assignments to
    // test traversals of very deep graphs. This requires a non-recursive traversal algorithm.
    ProgramBuilder b;
    auto& ty = b.ty;

    // var<private> v0 : i32 = 0i;
    // fn foo() {
    //   let v1 = v0;
    //   let v2 = v1;
    //   ...
    //   let v{N} = v{N-1};
    //   if (v{N} == 0) {
    //     workgroupBarrier();
    //   }
    // }
    b.GlobalVar("v0", ty.i32(), core::AddressSpace::kPrivate, b.Expr(0_i));
    Vector<const ast::Statement*, 8> foo_body;
    std::string v_last = "v0";
    for (int i = 1; i < 100000; i++) {
        auto v = "v" + std::to_string(i);
        foo_body.Push(b.Decl(b.Var(v, b.Expr(v_last))));
        v_last = v;
    }
    foo_body.Push(b.If(b.Equal(v_last, 0_i), b.Block(b.CallStmt(b.Call("workgroupBarrier")))));
    b.Func("foo", tint::Empty, ty.void_(), foo_body);

    RunTest(std::move(b), false);
    EXPECT_EQ(error_,
              R"(error: 'workgroupBarrier' must only be called from uniform control flow
note: control flow depends on possibly non-uniform value
note: reading from module-scope private variable 'v0' may result in a non-uniform value)");
}

////////////////////////////////////////////////////////////////////////////////
/// Tests for diagnostic filter rules.
////////////////////////////////////////////////////////////////////////////////

class UniformityAnalysisDiagnosticFilterTest
    : public UniformityAnalysisTestBase,
      public ::testing::TestWithParam<wgsl::DiagnosticSeverity> {
  protected:
    // TODO(jrprice): Remove this in favour of tint::ToString() when we change "note" to "info".
    const char* ToStr(wgsl::DiagnosticSeverity severity) {
        switch (severity) {
            case wgsl::DiagnosticSeverity::kError:
                return "error";
            case wgsl::DiagnosticSeverity::kWarning:
                return "warning";
            case wgsl::DiagnosticSeverity::kInfo:
                return "note";
            default:
                return "<undefined>";
        }
    }
};

// Test each diagnostic rule against directives and attributes on functions.

TEST_P(UniformityAnalysisDiagnosticFilterTest, Directive_DerivativeUniformity) {
    auto& param = GetParam();
    StringStream ss;
    ss << "diagnostic(" << param << ", derivative_uniformity);" << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;

fn foo() {
  if (non_uniform == 42) {
    let color = textureSample(t, s, vec2(0, 0));
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);

    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'textureSample' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, Directive_SubgroupUniformity_Callsite) {
    auto& param = GetParam();
    StringStream ss;
    ss << "enable subgroups;\n"
       << "diagnostic(" << param << ", subgroup_uniformity);" << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (non_uniform == 42) {
    _ = subgroupElect();
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);

    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'subgroupElect' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, Directive_SubgroupUniformity_ShuffleDelta) {
    auto& param = GetParam();
    StringStream ss;
    ss << "enable subgroups;\n"
       << "diagnostic(" << param << ", subgroup_uniformity);" << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : u32;

fn foo() {
  _ = subgroupShuffleUp(1.0, non_uniform);
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);

    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'subgroupShuffleUp' requires argument 1 to be uniform";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, Directive_SubgroupMatrixUniformity_BuiltinFunction) {
    auto& param = GetParam();
    StringStream ss;
    ss << "enable chromium_experimental_subgroup_matrix;\n"
       << "diagnostic(" << param << ", chromium.subgroup_matrix_uniformity);" << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

@group(0) @binding(1) var<storage, read_write> data : array<f32>;

fn foo() {
  if (non_uniform == 42) {
    _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>>(&data, 0, false, 4);
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);

    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'subgroupMatrixLoad' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, Directive_SubgroupMatrixUniformity_Constructor) {
    auto& param = GetParam();
    StringStream ss;
    ss << "enable chromium_experimental_subgroup_matrix;\n"
       << "diagnostic(" << param << ", chromium.subgroup_matrix_uniformity);" << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

@group(0) @binding(1) var<storage, read_write> data : array<f32>;

fn foo() {
  if (non_uniform == 42) {
    _ = subgroup_matrix_left<f32, 8, 8>(1);
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);

    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'subgroup_matrix_left' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, Directive_SubgroupMatrixUniformity_VarDecl) {
    auto& param = GetParam();
    StringStream ss;
    ss << "enable chromium_experimental_subgroup_matrix;\n"
       << "diagnostic(" << param << ", chromium.subgroup_matrix_uniformity);" << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

@group(0) @binding(1) var<storage, read_write> data : array<f32>;

fn foo() {
  if (non_uniform == 42) {
    var sm : subgroup_matrix_left<f32, 8, 8>;
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);

    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": variables that contain subgroup matrix types";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest,
       Directive_SubgroupMatrixUniformity_BuiltinFunctionArgument) {
    auto& param = GetParam();
    StringStream ss;
    ss << "enable chromium_experimental_subgroup_matrix;\n"
       << "diagnostic(" << param << ", chromium.subgroup_matrix_uniformity);" << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : u32;

@group(0) @binding(1) var<storage, read_write> data : array<f32>;

fn foo() {
  _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>>(&data, non_uniform, false, 4);
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);

    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'subgroupMatrixLoad' requires argument 1 to be uniform";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnFunction_DerivativeUniformity) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;
)" << "@diagnostic("
       << param << ", derivative_uniformity)" <<
        R"(fn foo() {
  if (non_uniform == 42) {
    let color = textureSample(t, s, vec2(0, 0));
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'textureSample' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnFunction_SubgroupUniformity_Callsite) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
enable subgroups;
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
)" << "@diagnostic("
       << param << ", subgroup_uniformity)" <<
        R"(fn foo() {
  if (non_uniform == 42) {
    _ = subgroupElect();
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'subgroupElect' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest,
       AttributeOnFunction_SubgroupUniformity_ShuffleDelta) {
    auto& param = GetParam();
    StringStream ss;
    ss << "enable subgroups;\n"
       << "diagnostic(" << param << ", subgroup_uniformity);" << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : u32;
)" << "@diagnostic("
       << param << ", subgroup_uniformity)" <<
        R"(fn foo() {
  _ = subgroupShuffleUp(1.0, non_uniform);
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);

    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'subgroupShuffleUp' requires argument 1 to be uniform";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest,
       AttributeOnFunction_SubgroupMatrixUniformity_Callsite) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
enable chromium_experimental_subgroup_matrix;
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

@group(0) @binding(1) var<storage, read_write> data : array<f32>;

)" << "@diagnostic("
       << param << ", chromium.subgroup_matrix_uniformity)" <<
        R"(fn foo() {
  if (non_uniform == 42) {
    _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>>(&data, 0, false, 4);
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'subgroupMatrixLoad' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

// Test the various places that the attributes can be used against just one diagnostic rule, to
// avoid over-parameterizing the tests.

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnBlock) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;
fn foo() {
  if (non_uniform == 42))"
       << "@diagnostic(" << param << ", derivative_uniformity)" << R"({
    let color = textureSample(t, s, vec2(0, 0));
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'textureSample' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnForStatement_CallInInitializer) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)"
       << R"(for (var b = (non_uniform == 42 && dpdx(1.0) > 0.0); false;) {
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnForStatement_CallInCondition) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(for (; non_uniform == 42 && dpdx(1.0) > 0.0;) {
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnForStatement_CallInIncrement) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)"
       << R"(for (var b = false; false; b = (non_uniform == 42 && dpdx(1.0) > 0.0)) {
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnForStatement_CallInBody) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(for (; non_uniform == 42;) {
    let color = textureSample(t, s, vec2(0, 0));
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'textureSample' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnIfStatement_CallInCondition) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(if (non_uniform == 42 && dpdx(1.0) > 0.0) {
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnIfStatement_CallInBody) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(if (non_uniform == 42) {
    let color = textureSample(t, s, vec2(0, 0));
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'textureSample' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnIfStatement_CallInElse) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(if (non_uniform == 42) {
  } else {
    let color = textureSample(t, s, vec2(0, 0));
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'textureSample' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnLoopStatement_CallInBody) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(loop {
    _ = dpdx(1.0);
    continuing {
      break if non_uniform == 0;
    }
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnLoopStatement_CallInContinuing) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(loop {
    continuing {
      _ = dpdx(1.0);
      break if non_uniform == 0;
    }
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnLoopBody_CallInBody) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  loop )"
       << "@diagnostic(" << param << ", derivative_uniformity)" << R"( {
    _ = dpdx(1.0);
    continuing {
      break if non_uniform == 0;
    }
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnLoopBody_CallInContinuing) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  loop )"
       << "@diagnostic(" << param << ", derivative_uniformity)" << R"( {
    continuing {
      _ = dpdx(1.0);
      break if non_uniform == 0;
    }
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnLoopContinuing_CallInContinuing) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  loop {
    continuing )"
       << "@diagnostic(" << param << ", derivative_uniformity)" << R"( {
      _ = dpdx(1.0);
      break if non_uniform == 0;
    }
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnSwitchStatement_CallInCondition) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)"
       << R"(switch (i32(non_uniform == 42 && dpdx(1.0) > 0.0)) {
    default {}
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnSwitchStatement_CallInBody) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(switch (non_uniform) {
    default {
      let color = textureSample(t, s, vec2(0, 0));
    }
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'textureSample' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnSwitchBody_CallInBody) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;
fn foo() {
  switch (non_uniform))"
       << "@diagnostic(" << param << ", derivative_uniformity)" << R"( {
    default {
      let color = textureSample(t, s, vec2(0, 0));
    }
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'textureSample' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnWhileStatement_CallInCondition) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(while (non_uniform == 42 && dpdx(1.0) > 0.0) {
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'dpdx' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

TEST_P(UniformityAnalysisDiagnosticFilterTest, AttributeOnWhileStatement_CallInBody) {
    auto& param = GetParam();
    StringStream ss;
    ss << R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;
@group(0) @binding(1) var t : texture_2d<f32>;
@group(0) @binding(2) var s : sampler;
fn foo() {
  )" << "@diagnostic("
       << param << ", derivative_uniformity)" << R"(while (non_uniform == 42) {
    let color = textureSample(t, s, vec2(0, 0));
  }
}
)";

    RunTest(ss.str(), param != wgsl::DiagnosticSeverity::kError);
    if (param == wgsl::DiagnosticSeverity::kOff) {
        EXPECT_TRUE(error_.empty());
    } else {
        StringStream err;
        err << ToStr(param) << ": 'textureSample' must only be called";
        EXPECT_THAT(error_, ::testing::HasSubstr(err.str()));
    }
}

INSTANTIATE_TEST_SUITE_P(UniformityAnalysisTest,
                         UniformityAnalysisDiagnosticFilterTest,
                         ::testing::Values(wgsl::DiagnosticSeverity::kError,
                                           wgsl::DiagnosticSeverity::kWarning,
                                           wgsl::DiagnosticSeverity::kInfo,
                                           wgsl::DiagnosticSeverity::kOff));

TEST_F(UniformityAnalysisDiagnosticFilterTest, AttributeOnFunction_CalledByAnotherFunction) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

@diagnostic(info, derivative_uniformity)
fn bar() {
  _ = dpdx(1.0);
}

fn foo() {
  if (non_uniform == 42) {
    bar();
  }
}
)";

    RunTest(src, true);
    EXPECT_THAT(error_, ::testing::HasSubstr("note: 'dpdx' must only be called"));
}

TEST_F(UniformityAnalysisDiagnosticFilterTest, AttributeOnFunction_RequirementOnParameter) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

@diagnostic(info, derivative_uniformity)
fn bar(x : i32) {
  if (x == 0) {
    _ = dpdx(1.0);
  }
}

fn foo() {
  bar(non_uniform);
}
)";

    RunTest(src, true);
    EXPECT_THAT(error_, ::testing::HasSubstr("note: 'dpdx' must only be called"));
}

TEST_F(UniformityAnalysisDiagnosticFilterTest, AttributeOnFunction_BuiltinInChildCall) {
    // Make sure that the diagnostic filter does not descend into functions called by the function
    // with the attribute.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn bar() {
  _ = dpdx(1.0);
}

@diagnostic(off, derivative_uniformity)
fn foo() {
  if (non_uniform == 42) {
    bar();
  }
}
)";

    RunTest(src, false);
    EXPECT_THAT(error_, ::testing::HasSubstr(": 'dpdx' must only be called"));
}

TEST_F(UniformityAnalysisDiagnosticFilterTest, MixOfGlobalAndLocalFilters) {
    // Test that a global filter is overridden by a local attribute, and that we find multiple
    // violations until an error is found.
    std::string src = R"(
diagnostic(info, derivative_uniformity);

@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn a() {
  if (non_uniform == 42) {
    _ = dpdx(1.0);
  }
}

@diagnostic(off, derivative_uniformity)
fn b() {
  if (non_uniform == 42) {
    _ = dpdx(1.0);
  }
}

@diagnostic(info, derivative_uniformity)
fn c() {
  if (non_uniform == 42) {
    _ = dpdx(1.0);
  }
}

@diagnostic(warning, derivative_uniformity)
fn d() {
  if (non_uniform == 42) {
    _ = dpdx(1.0);
  }
}

@diagnostic(error, derivative_uniformity)
fn e() {
  if (non_uniform == 42) {
    _ = dpdx(1.0);
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:8:9 note: 'dpdx' must only be called from uniform control flow
    _ = dpdx(1.0);
        ^^^^^^^^^

test:7:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:7:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^

test:22:9 note: 'dpdx' must only be called from uniform control flow
    _ = dpdx(1.0);
        ^^^^^^^^^

test:21:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:21:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^

test:29:9 warning: 'dpdx' must only be called from uniform control flow
    _ = dpdx(1.0);
        ^^^^^^^^^

test:28:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:28:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^

test:36:9 error: 'dpdx' must only be called from uniform control flow
    _ = dpdx(1.0);
        ^^^^^^^^^

test:35:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:35:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisDiagnosticFilterTest, BuiltinReturnValueNotAffected) {
    // Make sure that a diagnostic filter does not affect the uniformity of the return value of a
    // derivative builtin.
    std::string src = R"(
fn foo() {
  var x: f32;

  @diagnostic(off,derivative_uniformity) {
    x = dpdx(1.0);
  }

  if (x < 0.5) {
    _ = dpdy(1.0); // Should trigger an error
  }
}

)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:10:9 error: 'dpdy' must only be called from uniform control flow
    _ = dpdy(1.0); // Should trigger an error
        ^^^^^^^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (x < 0.5) {
  ^^

test:6:9 note: return value of 'dpdx' may be non-uniform
    x = dpdx(1.0);
        ^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisDiagnosticFilterTest,
       ParameterRequiredToBeUniform_With_ParameterRequiredToBeUniformForReturnValue) {
    // Make sure that both requirements on parameters are captured.
    std::string src = R"(
@diagnostic(info,derivative_uniformity)
fn foo(x : bool) -> bool {
  if (x) {
    _ = dpdx(1.0); // Should trigger an info
  }
  return x;
}

var<private> non_uniform: bool;

@diagnostic(error,derivative_uniformity)
fn bar() {
  let ret = foo(non_uniform);
  if (ret) {
    _ = dpdy(1.0); // Should trigger an error
  }
}

)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:16:9 error: 'dpdy' must only be called from uniform control flow
    _ = dpdy(1.0); // Should trigger an error
        ^^^^^^^^^

test:15:3 note: control flow depends on possibly non-uniform value
  if (ret) {
  ^^

test:14:17 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  let ret = foo(non_uniform);
                ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisDiagnosticFilterTest, BarriersNotAffected) {
    // Make sure that the diagnostic filter does not affect barriers.
    std::string src = R"(
diagnostic(off, derivative_uniformity);

@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  if (non_uniform == 42) {
    _ = dpdx(1.0);
  }
}

fn bar() {
  if (non_uniform == 42) {
    workgroupBarrier();
  }
}

)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:14:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:13:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:13:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^
)");
}

////////////////////////////////////////////////////////////////////////////////
/// Tests for the quality of the error messages produced by the analysis.
////////////////////////////////////////////////////////////////////////////////

TEST_F(UniformityAnalysisTest, Error_CallUserThatCallsBuiltinDirectly) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo() {
  workgroupBarrier();
}

fn main() {
  if (non_uniform == 42) {
    foo();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:5:3 error: 'workgroupBarrier' must only be called from uniform control flow
  workgroupBarrier();
  ^^^^^^^^^^^^^^^^

test:10:5 note: called by 'foo' from 'main'
    foo();
    ^^^

test:9:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:9:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Error_CallUserThatCallsBuiltinIndirectly) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn zoo() {
  workgroupBarrier();
}

fn bar() {
  zoo();
}

fn foo() {
  bar();
}

fn main() {
  if (non_uniform == 42) {
    foo();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:5:3 error: 'workgroupBarrier' must only be called from uniform control flow
  workgroupBarrier();
  ^^^^^^^^^^^^^^^^

test:18:5 note: called indirectly by 'foo' from 'main'
    foo();
    ^^^

test:17:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:17:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Error_ParametersRequireUniformityInChain) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn zoo(a : i32) {
  if (a == 42) {
    workgroupBarrier();
  }
}

fn bar(b : i32) {
  zoo(b);
}

fn foo(c : i32) {
  bar(c);
}

fn main() {
  foo(non_uniform);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:5:3 note: control flow depends on possibly non-uniform value
  if (a == 42) {
  ^^

test:4:8 note: parameter 'a' of 'zoo' may be non-uniform
fn zoo(a : i32) {
       ^

test:11:7 note: possibly non-uniform value passed here
  zoo(b);
      ^

test:11:7 note: parameter 'b' of 'bar' may be non-uniform
  zoo(b);
      ^

test:15:7 note: possibly non-uniform value passed here
  bar(c);
      ^

test:15:7 note: parameter 'c' of 'foo' may be non-uniform
  bar(c);
      ^

test:19:7 note: possibly non-uniform value passed here
  foo(non_uniform);
      ^^^^^^^^^^^

test:19:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  foo(non_uniform);
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Error_ReturnValueMayBeNonUniformChain) {
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn zoo() -> i32 {
  return non_uniform;
}

fn bar() -> i32 {
  return zoo();
}

fn foo() -> i32 {
  return bar();
}

fn main() {
  if (foo() == 42) {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:18:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:17:3 note: control flow depends on possibly non-uniform value
  if (foo() == 42) {
  ^^

test:17:7 note: return value of 'foo' may be non-uniform
  if (foo() == 42) {
      ^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Error_CallsiteAndParameterRequireUniformity) {
    // Test that we report a violation for the callsite of a function when it has multiple
    // uniformity requirements.
    std::string src = R"(
@group(0) @binding(0) var<storage, read_write> non_uniform : i32;

fn foo(v : i32) {
  if (v == 0) {
    workgroupBarrier();
  }
}

fn main() {
  if (non_uniform == 42) {
    foo(0);
  }
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:6:5 error: 'workgroupBarrier' must only be called from uniform control flow
    workgroupBarrier();
    ^^^^^^^^^^^^^^^^

test:12:5 note: called by 'foo' from 'main'
    foo(0);
    ^^^

test:11:3 note: control flow depends on possibly non-uniform value
  if (non_uniform == 42) {
  ^^

test:11:7 note: reading from read_write storage buffer 'non_uniform' may result in a non-uniform value
  if (non_uniform == 42) {
      ^^^^^^^^^^^
)");
}

TEST_F(UniformityAnalysisTest, Error_PointerParameterContentsRequiresUniformity_AfterControlFlow) {
    // Test that we can find the correct source of uniformity inside a function called with a
    // pointer parameter, when the pointer contents is used after control flow that introduces extra
    // nodes for merging the pointer contents.
    std::string src = R"(
var<private> non_uniform : i32;

fn foo(p : ptr<function, i32>) {
  for (var i = 0; i < 3; i++) {
    continue;
  }
  if (*p == 0) {
    return;
  }
  _ = dpdx(1.0);
}

fn main() {
  var f = non_uniform;
  foo(&f);
}
)";

    RunTest(src, false);
    EXPECT_EQ(error_,
              R"(test:11:7 error: 'dpdx' must only be called from uniform control flow
  _ = dpdx(1.0);
      ^^^^^^^^^

test:8:3 note: control flow depends on possibly non-uniform value
  if (*p == 0) {
  ^^

test:4:8 note: parameter 'p' of 'foo' may point to a non-uniform value
fn foo(p : ptr<function, i32>) {
       ^

test:16:7 note: possibly non-uniform value passed via pointer here
  foo(&f);
      ^^

test:15:11 note: reading from module-scope private variable 'non_uniform' may result in a non-uniform value
  var f = non_uniform;
          ^^^^^^^^^^^
)");
}

class SubgroupUniformityTest : public UniformityAnalysisTestBase,
                               public ::testing::TestWithParam<int> {
  public:
    /// Enum for the function call statement.
    enum Function {
        // Subgroup functions:
        kSubgroupBallot,
        kSubgroupElect,
        kSubgroupBroadcast,
        kSubgroupBroadcastFirst,
        kSubgroupShuffle,
        kSubgroupShuffleXor,
        kSubgroupShuffleUp,
        kSubgroupShuffleDown,
        kSubgroupAdd,
        kSubgroupInclusiveAdd,
        kSubgroupExclusiveAdd,
        kSubgroupMul,
        kSubgroupInclusiveMul,
        kSubgroupExclusiveMul,
        kSubgroupAnd,
        kSubgroupOr,
        kSubgroupXor,
        kSubgroupMin,
        kSubgroupMax,
        kSubgroupAll,
        kSubgroupAny,
        kQuadBroadcast,
        kQuadSwapX,
        kQuadSwapY,
        kQuadSwapDiagonal,
        // End of range marker:
        kEndOfFunctionRange,
    };

    /// @returns true if `f` is uniform in subgroup scope.
    static bool UniformSubgroup(Function f) {
        switch (f) {
            case kSubgroupAdd:
            case kSubgroupAll:
            case kSubgroupAnd:
            case kSubgroupAny:
            case kSubgroupBallot:
            case kSubgroupBroadcast:
            case kSubgroupBroadcastFirst:
            case kSubgroupMax:
            case kSubgroupMin:
            case kSubgroupMul:
            case kSubgroupOr:
            case kSubgroupXor:
                return true;
            default:
                return false;
        }
    }

    /// Convert a function call to its string representation.
    static std::string FunctionToStr(Function f) {
        switch (f) {
            case kSubgroupBallot:
                return "subgroupBallot(true).x == 0";
            case kSubgroupElect:
                return "subgroupElect()";
            case kSubgroupBroadcast:
                return "subgroupBroadcast(1.0, 0) == 0";
            case kSubgroupBroadcastFirst:
                return "subgroupBroadcastFirst(1.0) == 0";
            case kSubgroupShuffle:
                return "subgroupShuffle(1.0, 0) == 0";
            case kSubgroupShuffleXor:
                return "subgroupShuffleXor(1.0, 0) == 0";
            case kSubgroupShuffleUp:
                return "subgroupShuffleUp(1.0, 1) == 0";
            case kSubgroupShuffleDown:
                return "subgroupShuffleDown(1.0, 1) == 0";
            case kSubgroupAdd:
                return "subgroupAdd(1.0) == 0";
            case kSubgroupInclusiveAdd:
                return "subgroupInclusiveAdd(1.0) == 0";
            case kSubgroupExclusiveAdd:
                return "subgroupExclusiveAdd(1.0) == 0";
            case kSubgroupMul:
                return "subgroupMul(1.0) == 0";
            case kSubgroupInclusiveMul:
                return "subgroupInclusiveMul(1.0) == 0";
            case kSubgroupExclusiveMul:
                return "subgroupExclusiveMul(1.0) == 0";
            case kSubgroupAnd:
                return "subgroupAnd(1) == 0";
            case kSubgroupOr:
                return "subgroupOr(1) == 0";
            case kSubgroupXor:
                return "subgroupXor(1) == 0";
            case kSubgroupMin:
                return "subgroupMin(1.0) == 0";
            case kSubgroupMax:
                return "subgroupMax(1.0) == 0";
            case kSubgroupAll:
                return "subgroupAll(true)";
            case kSubgroupAny:
                return "subgroupAny(true)";
            case kQuadBroadcast:
                return "quadBroadcast(1.0, 0) == 0";
            case kQuadSwapX:
                return "quadSwapX(1.0) == 0";
            case kQuadSwapY:
                return "quadSwapY(1.0) == 0";
            case kQuadSwapDiagonal:
                return "quadSwapDiagonal(1.0) == 0";
            case kEndOfFunctionRange:
                return "<invalid>";
        }
        return "<invalid>";
    }

    /// Convert a test parameter function to a string that can be used as part of
    /// a test name.
    static std::string ParamsToName(::testing::TestParamInfo<ParamType> params) {
        Function f = static_cast<Function>(params.param);
        std::string name;
#define CASE(c)     \
    case c:         \
        name += #c; \
        break

        switch (f) {
            CASE(kSubgroupBallot);
            CASE(kSubgroupElect);
            CASE(kSubgroupBroadcast);
            CASE(kSubgroupBroadcastFirst);
            CASE(kSubgroupShuffle);
            CASE(kSubgroupShuffleXor);
            CASE(kSubgroupShuffleUp);
            CASE(kSubgroupShuffleDown);
            CASE(kSubgroupAdd);
            CASE(kSubgroupInclusiveAdd);
            CASE(kSubgroupExclusiveAdd);
            CASE(kSubgroupMul);
            CASE(kSubgroupInclusiveMul);
            CASE(kSubgroupExclusiveMul);
            CASE(kSubgroupAnd);
            CASE(kSubgroupOr);
            CASE(kSubgroupXor);
            CASE(kSubgroupMin);
            CASE(kSubgroupMax);
            CASE(kSubgroupAll);
            CASE(kSubgroupAny);
            CASE(kQuadBroadcast);
            CASE(kQuadSwapX);
            CASE(kQuadSwapY);
            CASE(kQuadSwapDiagonal);
            case kEndOfFunctionRange:
                break;
#undef CASE
        }
        return name;
    }
};

TEST_P(SubgroupUniformityTest, Workgroup) {
    auto function = static_cast<Function>(GetParam());

    std::string src = R"(
enable subgroups;

fn foo() {
  if ()" + FunctionToStr(function) +
                      R"() {
    workgroupBarrier();
  }
}
)";

    RunTest(src, false);
    EXPECT_THAT(error_, ::testing::HasSubstr("must only be called from uniform control flow"));
}

TEST_P(SubgroupUniformityTest, Subgroup) {
    auto function = static_cast<Function>(GetParam());

    std::string src = R"(
enable subgroups;

fn foo() {
  if ()" + FunctionToStr(function) +
                      R"() {
    _ = subgroupAny(true);
  }
}
)";

    const bool should_pass = UniformSubgroup(function);
    RunTest(src, should_pass);
    if (!should_pass) {
        EXPECT_THAT(error_,
                    ::testing::HasSubstr("must only be called from subgroup uniform control flow"));
    }
}

INSTANTIATE_TEST_SUITE_P(UniformityAnalysisTest,
                         SubgroupUniformityTest,
                         ::testing::Range<int>(0, SubgroupUniformityTest::kEndOfFunctionRange),
                         SubgroupUniformityTest::ParamsToName);

}  // namespace
}  // namespace tint::resolver
