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

// GEN_BUILD:CONDITION(tint_build_wgsl_reader && tint_build_wgsl_writer)

#include "gtest/gtest.h"

#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

class IRToProgramRoundtripTest : public testing::Test {
  public:
    struct Result {
        /// The resulting WGSL
        std::string wgsl;
        /// The resulting AST
        std::string ast;
        /// The resulting IR before raising
        std::string ir_pre_raise;
        /// The resulting IR after raising
        std::string ir_post_raise;
        /// The resulting error
        std::string err;
        /// The expected WGSL
        std::string expected;
    };

    /// @return the round-tripped string and the expected string
    Result Run(std::string input_wgsl, std::string expected_wgsl) {
        std::string input{tint::TrimSpace(input_wgsl)};

        Result result;

        wgsl::reader::Options options;
        options.allowed_features = wgsl::AllowedFeatures::Everything();
        Source::File file("test.wgsl", std::string(input));
        auto ir_module = wgsl::reader::WgslToIR(&file, options);
        if (ir_module != Success) {
            return result;
        }

        result.ir_pre_raise = core::ir::Disassembler(ir_module.Get()).Plain();

        if (auto res = tint::wgsl::writer::Raise(ir_module.Get()); res != Success) {
            result.err = res.Failure().reason.Str();
            return result;
        }

        result.ir_post_raise = core::ir::Disassembler(ir_module.Get()).Plain();

        writer::ProgramOptions program_options;
        program_options.allowed_features = AllowedFeatures::Everything();
        auto output_program = wgsl::writer::IRToProgram(ir_module.Get(), program_options);
        if (!output_program.IsValid()) {
            result.err = output_program.Diagnostics().Str();
            result.ast = Program::printer(output_program);
            return result;
        }

        auto output = wgsl::writer::Generate(output_program, {});
        if (output != Success) {
            std::stringstream ss;
            ss << "wgsl::Generate() errored: " << output.Failure();
            result.err = ss.str();
            result.ast = Program::printer(output_program);
            return result;
        }

        result.expected = expected_wgsl.empty() ? input : tint::TrimSpace(expected_wgsl);
        if (!result.expected.empty()) {
            result.expected = "\n" + result.expected + "\n";
        }

        result.wgsl = std::string(tint::TrimSpace(output->wgsl));
        if (!result.wgsl.empty()) {
            result.wgsl = "\n" + result.wgsl + "\n";
        }

        return result;
    }

    Result Run(std::string wgsl) { return Run(wgsl, wgsl); }
};

std::ostream& operator<<(std::ostream& o, const IRToProgramRoundtripTest::Result& res) {
    if (!res.err.empty()) {
        o << "============================\n"
          << "== Error                  ==\n"
          << "============================\n"
          << res.err << "\n\n";
    }
    if (!res.ir_pre_raise.empty()) {
        o << "============================\n"
          << "== IR (pre-raise)         ==\n"
          << "============================\n"
          << res.ir_pre_raise << "\n\n";
    }
    if (!res.ir_post_raise.empty()) {
        o << "============================\n"
          << "== IR (post-raise)        ==\n"
          << "============================\n"
          << res.ir_post_raise << "\n\n";
    }
    if (!res.ast.empty()) {
        o << "============================\n"
          << "== AST                    ==\n"
          << "============================\n"
          << res.ast << "\n\n";
    }
    return o;
}

#define RUN_TEST(...)                                       \
    do {                                                    \
        if (auto res = Run(__VA_ARGS__); res.err.empty()) { \
            EXPECT_EQ(res.expected, res.wgsl) << res;       \
        } else {                                            \
            FAIL() << res;                                  \
        }                                                   \
    } while (false)

TEST_F(IRToProgramRoundtripTest, EmptyModule) {
    RUN_TEST("");
}

TEST_F(IRToProgramRoundtripTest, SingleFunction_Empty) {
    RUN_TEST(R"(
fn f() {
}
)");
}

TEST_F(IRToProgramRoundtripTest, SingleFunction_Return) {
    RUN_TEST(R"(
fn f() {
  return;
}
)",
             R"(
fn f() {
}
)");
}

TEST_F(IRToProgramRoundtripTest, SingleFunction_Return_i32) {
    RUN_TEST(R"(
fn f() -> i32 {
  return 42i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, SingleFunction_Parameters) {
    RUN_TEST(R"(
fn f(i : i32, u : u32) -> i32 {
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, SingleFunction_UnrestrictedPointerParameters) {
    RUN_TEST(R"(
fn f(p : ptr<uniform, i32>) -> i32 {
  return *(p);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Struct declaration
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, StructDecl_Scalars) {
    RUN_TEST(R"(
struct S {
  a : i32,
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberAlign) {
    RUN_TEST(R"(
struct S {
  a : i32,
  @align(32u)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberSize) {
    RUN_TEST(R"(
struct S {
  a : i32,
  @size(32u)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberLocation) {
    RUN_TEST(R"(
struct S {
  a : i32,
  @location(1u)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberIndex) {
    RUN_TEST(R"(
enable dual_source_blending;

struct S {
  a : i32,
  @location(0u) @blend_src(0u)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberBuiltin) {
    RUN_TEST(R"(
struct S {
  a : i32,
  @builtin(position)
  b : vec4<f32>,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberInterpolateType) {
    RUN_TEST(R"(
struct S {
  a : i32,
  @location(1u) @interpolate(flat)
  b : u32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberInterpolateTypeSampling) {
    RUN_TEST(R"(
struct S {
  a : i32,
  @location(1u) @interpolate(perspective, centroid)
  b : f32,
  c : f32,
}

var<private> v : S;
)");
}

TEST_F(IRToProgramRoundtripTest, StructDecl_MemberInvariant) {
    RUN_TEST(R"(
struct S {
  a : i32,
  @builtin(position) @invariant
  b : vec4<f32>,
  c : f32,
}

var<private> v : S;
)");
}

////////////////////////////////////////////////////////////////////////////////
// Function Call
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, FnCall_NoArgs_NoRet) {
    RUN_TEST(R"(
fn a() {
}

fn b() {
  a();
}
)");
}

TEST_F(IRToProgramRoundtripTest, FnCall_NoArgs_Ret_i32) {
    RUN_TEST(R"(
fn a() -> i32 {
  return 1i;
}

fn b() {
  var i : i32 = a();
}
)");
}

TEST_F(IRToProgramRoundtripTest, FnCall_3Args_NoRet) {
    RUN_TEST(R"(
fn a(x : i32, y : u32, z : f32) {
}

fn b() {
  a(1i, 2u, 3.0f);
}
)");
}

TEST_F(IRToProgramRoundtripTest, FnCall_3Args_Ret_f32) {
    RUN_TEST(R"(
fn a(x : i32, y : u32, z : f32) -> f32 {
  return z;
}

fn b() {
  var v : f32 = a(1i, 2u, 3.0f);
}
)");
}

TEST_F(IRToProgramRoundtripTest, FnCall_PtrArgs) {
    RUN_TEST(R"(
var<private> y : i32 = 2i;

fn a(px : ptr<function, i32>, py : ptr<private, i32>) -> i32 {
  return (*(px) + *(py));
}

fn b() -> i32 {
  var x : i32 = 1i;
  return a(&(x), &(y));
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Core Builtin Call
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_Stmt) {
    RUN_TEST(R"(
fn f() {
  workgroupBarrier();
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_Expr) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) {
  var i : i32 = max(a, b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_PhonyAssignment) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) {
  _ = max(a, b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_UnusedLet) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) {
  let unused = max(a, b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_PtrArg) {
    RUN_TEST(R"(
@group(0u) @binding(0u) var<storage, read> v : array<u32>;

fn foo() -> u32 {
  return arrayLength(&(v));
}
)");
}

TEST_F(IRToProgramRoundtripTest, CoreBuiltinCall_DisableDerivativeUniformity) {
    RUN_TEST(R"(
fn f(in : f32) {
  let x = dpdx(in);
  let y = dpdy(in);
}
)",
             R"(
diagnostic(off, derivative_uniformity);

fn f(in : f32) {
  let x = dpdx(in);
  let y = dpdy(in);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Type Construct
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, TypeConstruct_i32) {
    RUN_TEST(R"(
fn f(i : i32) {
  var v : i32 = i32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_u32) {
    RUN_TEST(R"(
fn f(i : u32) {
  var v : u32 = u32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_f32) {
    RUN_TEST(R"(
fn f(i : f32) {
  var v : f32 = f32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_bool) {
    RUN_TEST(R"(
fn f(i : bool) {
  var v : bool = bool(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_struct) {
    RUN_TEST(R"(
struct S {
  a : i32,
  b : u32,
  c : f32,
}

fn f(a : i32, b : u32, c : f32) {
  var v : S = S(a, b, c);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_array) {
    RUN_TEST(R"(
fn f(i : i32) {
  var v : array<i32, 3u> = array<i32, 3u>(i, i, i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_vec3i_Splat) {
    RUN_TEST(R"(
fn f(i : i32) {
  var v : vec3<i32> = vec3<i32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_vec3i_Scalars) {
    RUN_TEST(R"(
fn f(i : i32) {
  var v : vec3<i32> = vec3<i32>(i, i, i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_mat2x3f_Scalars) {
    RUN_TEST(R"(
fn f(i : f32) {
  var v : mat2x3<f32> = mat2x3<f32>(i, i, i, i, i, i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConstruct_mat2x3f_Columns) {
    RUN_TEST(R"(
fn f(i : f32) {
  var v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(i, i, i), vec3<f32>(i, i, i));
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Type Convert
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, TypeConvert_i32_to_u32) {
    RUN_TEST(R"(
fn f(i : i32) {
  var v : u32 = u32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_u32_to_f32) {
    RUN_TEST(R"(
fn f(i : u32) {
  var v : f32 = f32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_f32_to_i32) {
    RUN_TEST(R"(
fn f(i : f32) {
  var v : i32 = i32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_bool_to_u32) {
    RUN_TEST(R"(
fn f(i : bool) {
  var v : u32 = u32(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_vec3i_to_vec3u) {
    RUN_TEST(R"(
fn f(i : vec3<i32>) {
  var v : vec3<u32> = vec3<u32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_vec3u_to_vec3f) {
    RUN_TEST(R"(
fn f(i : vec3<u32>) {
  var v : vec3<f32> = vec3<f32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, TypeConvert_mat2x3f_to_mat2x3h) {
    RUN_TEST(R"(
enable f16;

fn f(i : mat2x3<f32>) {
  var v : mat2x3<f16> = mat2x3<f16>(i);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Bitcast
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Bitcast_i32_to_u32) {
    RUN_TEST(R"(
fn f(i : i32) {
  var v : u32 = bitcast<u32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, Bitcast_u32_to_f32) {
    RUN_TEST(R"(
fn f(i : u32) {
  var v : f32 = bitcast<f32>(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, Bitcast_f32_to_i32) {
    RUN_TEST(R"(
fn f(i : f32) {
  var v : i32 = bitcast<i32>(i);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Discard
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Discard) {
    RUN_TEST(R"(
fn f() {
  discard;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Access
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Access_Value_vec3f_1) {
    RUN_TEST(R"(
fn f(v : vec3<f32>) -> f32 {
  return v[1];
}
)",
             R"(
fn f(v : vec3<f32>) -> f32 {
  return v.y;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_vec3f_1) {
    RUN_TEST(R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v[1];
}
)",
             R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v.y;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_vec3f_z) {
    RUN_TEST(R"(
fn f(v : vec3<f32>) -> f32 {
  return v.z;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_vec3f_z) {
    RUN_TEST(R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v.z;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_vec3f_g) {
    RUN_TEST(R"(
fn f(v : vec3<f32>) -> f32 {
  return v.g;
}
)",
             R"(
fn f(v : vec3<f32>) -> f32 {
  return v.y;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_vec3f_g) {
    RUN_TEST(R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v.g;
}
)",
             R"(
var<private> v : vec3<f32>;

fn f() -> f32 {
  return v.y;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_vec3f_i) {
    RUN_TEST(R"(
fn f(v : vec3<f32>, i : i32) -> f32 {
  return v[i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_vec3f_i) {
    RUN_TEST(R"(
var<private> v : vec3<f32>;

fn f(i : i32) -> f32 {
  return v[i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_mat3x2f_1_0) {
    RUN_TEST(R"(
fn f(m : mat3x2<f32>) -> f32 {
  return m[1][0];
}
)",
             R"(
fn f(m : mat3x2<f32>) -> f32 {
  return m[1i].x;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_mat3x2f_1_0) {
    RUN_TEST(R"(
var<private> m : mat3x2<f32>;

fn f() -> f32 {
  return m[1][0];
}
)",
             R"(
var<private> m : mat3x2<f32>;

fn f() -> f32 {
  return m[1i].x;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_mat3x2f_u_0) {
    RUN_TEST(R"(
fn f(m : mat3x2<f32>, u : u32) -> f32 {
  return m[u][0];
}
)",
             R"(
fn f(m : mat3x2<f32>, u : u32) -> f32 {
  return m[u].x;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_mat3x2f_u_0) {
    RUN_TEST(R"(
var<private> m : mat3x2<f32>;

fn f(u : u32) -> f32 {
  return m[u][0];
}
)",
             R"(
var<private> m : mat3x2<f32>;

fn f(u : u32) -> f32 {
  return m[u].x;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_mat3x2f_u_i) {
    RUN_TEST(R"(
fn f(m : mat3x2<f32>, u : u32, i : i32) -> f32 {
  return m[u][i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_mat3x2f_u_i) {
    RUN_TEST(R"(
var<private> m : mat3x2<f32>;

fn f(u : u32, i : i32) -> f32 {
  return m[u][i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_array_0u) {
    RUN_TEST(R"(
fn f(a : array<i32, 4u>) -> i32 {
  return a[0u];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_array_0u) {
    RUN_TEST(R"(
var<private> a : array<i32, 4u>;

fn f() -> i32 {
  return a[0u];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Value_array_i) {
    RUN_TEST(R"(
fn f(a : array<i32, 4u>, i : i32) -> i32 {
  return a[i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Ref_array_i) {
    RUN_TEST(R"(
var<private> a : array<i32, 4u>;

fn f(i : i32) -> i32 {
  return a[i];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ValueStruct) {
    RUN_TEST(R"(
struct Y {
  a : i32,
  b : i32,
  c : i32,
}

struct X {
  a : i32,
  b : Y,
  c : i32,
}

fn f(x : X) -> i32 {
  return x.b.c;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ReferenceStruct) {
    RUN_TEST(R"(
struct Y {
  a : i32,
  b : i32,
  c : i32,
}

struct X {
  a : i32,
  b : Y,
  c : i32,
}

fn f() -> i32 {
  var x : X;
  return x.b.c;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfArrayOfArray_123) {
    RUN_TEST(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v_1 : array<array<array<i32, 3u>, 4u>, 5u>;
  return v_1[a(1i)][a(2i)][a(3i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfArrayOfArray_213) {
    RUN_TEST(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v_1 : array<array<array<i32, 3u>, 4u>, 5u>;
  let v_2 = a(2i);
  return v_1[a(1i)][v_2][a(3i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfArrayOfArray_312) {
    RUN_TEST(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v_1 : array<array<array<i32, 3u>, 4u>, 5u>;
  let v_2 = a(3i);
  return v_1[a(1i)][a(2i)][v_2];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfArrayOfArray_321) {
    RUN_TEST(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> i32 {
  var v_1 : array<array<array<i32, 3u>, 4u>, 5u>;
  let v_2 = a(3i);
  let v_3 = a(2i);
  return v_1[a(1i)][v_3][v_2];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfMat3x4f_123) {
    RUN_TEST(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  return array<mat3x4<f32>, 5u>()[a(1i)][a(2i)][a(3i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfMat3x4f_213) {
    RUN_TEST(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  let v_1 = array<mat3x4<f32>, 5u>();
  let v_2 = a(2i);
  return v_1[a(1i)][v_2][a(3i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfMat3x4f_312) {
    RUN_TEST(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  let v_1 = array<mat3x4<f32>, 5u>();
  let v_2 = a(3i);
  return v_1[a(1i)][a(2i)][v_2];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_ArrayOfMat3x4f_321) {
    RUN_TEST(R"(
fn a(v : i32) -> i32 {
  return 1i;
}

fn f() -> f32 {
  let v_1 = array<mat3x4<f32>, 5u>();
  let v_2 = a(3i);
  let v_3 = a(2i);
  return v_1[a(1i)][v_3][v_2];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_UsePartialChains) {
    RUN_TEST(R"(
var<private> a : array<array<array<i32, 4u>, 5u>, 6u>;

fn f(i : i32) -> i32 {
  let p1 = &(a[i]);
  let p2 = &((*(p1))[i]);
  let p3 = &((*(p2))[i]);
  let v1 = *(p1);
  let v2 = *(p2);
  let v3 = *(p3);
  return v3;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Swizzle
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Access_Vec3_Value_xy) {
    RUN_TEST(R"(
fn f(v : vec3<f32>) -> vec2<f32> {
  return v.xy;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Value_yz) {
    RUN_TEST(R"(
fn f(v : vec3<f32>) -> vec2<f32> {
  return v.yz;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Value_yzx) {
    RUN_TEST(R"(
fn f(v : vec3<f32>) -> vec3<f32> {
  return v.yzx;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Value_yzxy) {
    RUN_TEST(R"(
fn f(v : vec3<f32>) -> vec4<f32> {
  return v.yzxy;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Pointer_xy) {
    RUN_TEST(R"(
fn f(v : ptr<function, vec3<f32>>) -> vec2<f32> {
  return (*(v)).xy;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Pointer_yz) {
    RUN_TEST(R"(
fn f(v : ptr<function, vec3<f32>>) -> vec2<f32> {
  return (*(v)).yz;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Pointer_yzx) {
    RUN_TEST(R"(
fn f(v : ptr<function, vec3<f32>>) -> vec3<f32> {
  return (*(v)).yzx;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Access_Vec3_Pointer_yzxy) {
    RUN_TEST(R"(
fn f(v : ptr<function, vec3<f32>>) -> vec4<f32> {
  return (*(v)).yzxy;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Unary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, UnaryOp_Negate) {
    RUN_TEST(R"(
fn f(i : i32) -> i32 {
  return -(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, UnaryOp_Complement) {
    RUN_TEST(R"(
fn f(i : u32) -> u32 {
  return ~(i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, UnaryOp_Not) {
    RUN_TEST(R"(
fn f(b : bool) -> bool {
  return !(b);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Binary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, BinaryOp_Add) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a + b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Subtract) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a - b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Multiply) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a * b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Divide) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a / b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Modulo) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a % b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_And) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a & b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Or) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a | b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Xor) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a ^ b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_Equal) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> bool {
  return (a == b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_NotEqual) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> bool {
  return (a != b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_LessThan) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> bool {
  return (a < b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_GreaterThan) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> bool {
  return (a > b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_LessThanEqual) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> bool {
  return (a <= b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_GreaterThanEqual) {
    RUN_TEST(R"(
fn f(a : i32, b : i32) -> bool {
  return (a >= b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_ShiftLeft) {
    RUN_TEST(R"(
fn f(a : i32, b : u32) -> i32 {
  return (a << b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, BinaryOp_ShiftRight) {
    RUN_TEST(R"(
fn f(a : i32, b : u32) -> i32 {
  return (a >> b);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Short-circuiting binary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Param_2) {
    RUN_TEST(R"(
fn f(a : bool, b : bool) -> bool {
  return (a && b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Param_3_ab_c) {
    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a && b) && c);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Param_3_a_bc) {
    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a && b) && c);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Let_2) {
    RUN_TEST(R"(
fn f(a : bool, b : bool) -> bool {
  let l = (a && b);
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Let_3_ab_c) {
    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = ((a && b) && c);
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Let_3_a_bc) {
    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = (a && (b && c));
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Call_2) {
    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn f() -> bool {
  return (a() && b());
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Call_3_ab_c) {
    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return ((a() && b()) && c());
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_And_Call_3_a_bc) {
    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return (a() && (b() && c()));
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Param_2) {
    RUN_TEST(R"(
fn f(a : bool, b : bool) -> bool {
  return (a || b);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Param_3_ab_c) {
    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a || b) || c);
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Param_3_a_bc) {
    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return (a || (b || c));
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Let_2) {
    RUN_TEST(R"(
fn f(a : bool, b : bool) -> bool {
  let l = (a || b);
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Let_3_ab_c) {
    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = ((a || b) || c);
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Let_3_a_bc) {
    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = (a || (b || c));
  return l;
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Call_2) {
    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn f() -> bool {
  return (a() || b());
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Call_3_ab_c) {
    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return ((a() || b()) || c());
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Or_Call_3_a_bc) {
    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return (a() || (b() || c()));
}
)");
}

TEST_F(IRToProgramRoundtripTest, ShortCircuit_Mixed) {
    RUN_TEST(R"(
fn b() -> bool {
  return true;
}

fn d() -> bool {
  return true;
}

fn f(a : bool, c : bool) -> bool {
  let l = ((a || b()) && (c || d()));
  return l;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Assignment
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfArrayOfArrayAccess_123456) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  v[e(1i)][e(2i)][e(3i)] = v[e(4i)][e(5i)][e(6i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfArrayOfArrayAccess_261345) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  v[e(1i)][v_2][e(3i)] = v[e(4i)][e(5i)][v_3];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfArrayOfArrayAccess_532614) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  v[e(1i)][v_4][v_3] = v[e(4i)][v_2][v_5];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfMatrixAccess_123456) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  v[e(1i)][e(2i)][e(3i)] = v[e(4i)][e(5i)][e(6i)];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfMatrixAccess_261345) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  v[e(1i)][v_2][e(3i)] = v[e(4i)][e(5i)][v_3];
}
)");
}

TEST_F(IRToProgramRoundtripTest, Assign_ArrayOfMatrixAccess_532614) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  v[e(1i)][v_4][v_3] = v[e(4i)][v_2][v_5];
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Compound assignment
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, CompoundAssign_Increment) {
    RUN_TEST(R"(
fn f() {
  var v : i32;
  v++;
}
)",
             R"(
fn f() {
  var v : i32;
  v = (v + 1i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Decrement) {
    RUN_TEST(R"(
fn f() {
  var v : i32;
  v++;
}
)",
             R"(
fn f() {
  var v : i32;
  v = (v + 1i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Add) {
    RUN_TEST(R"(
fn f() {
  var v : i32;
  v += 8i;
}
)",
             R"(
fn f() {
  var v : i32;
  v = (v + 8i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Subtract) {
    RUN_TEST(R"(
fn f() {
  var v : i32;
  v -= 8i;
}
)",
             R"(
fn f() {
  var v : i32;
  v = (v - 8i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Multiply) {
    RUN_TEST(R"(
fn f() {
  var v : i32;
  v *= 8i;
}
)",
             R"(
fn f() {
  var v : i32;
  v = (v * 8i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Divide) {
    RUN_TEST(R"(
fn f() {
  var v : i32;
  v /= 8i;
}
)",
             R"(
fn f() {
  var v : i32;
  v = (v / 8i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_Xor) {
    RUN_TEST(R"(
fn f() {
  var v : i32;
  v ^= 8i;
}
)",
             R"(
fn f() {
  var v : i32;
  v = (v ^ 8i);
}
)");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfArrayOfArrayAccess_123456) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  v[e(1i)][e(2i)][e(3i)] += v[e(4i)][e(5i)][e(6i)];
}
)",
             R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_1 = &(v[e(1i)][e(2i)][e(3i)]);
  let v_2 = v[e(4i)][e(5i)][e(6i)];
  *(v_1) = (*(v_1) + v_2);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfArrayOfArrayAccess_261345) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  v[e(1i)][v_2][e(3i)] += v[e(4i)][e(5i)][v_3];
}
)",
             R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  let v_1 = &(v[e(1i)][v_2][e(3i)]);
  let v_4 = v[e(4i)][e(5i)][v_3];
  *(v_1) = (*(v_1) + v_4);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfArrayOfArrayAccess_532614) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  v[e(1i)][v_4][v_3] += v[e(4i)][v_2][v_5];
}
)",
             R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<array<array<i32, 5u>, 5u>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  let v_1 = &(v[e(1i)][v_4][v_3]);
  let v_6 = v[e(4i)][v_2][v_5];
  *(v_1) = (*(v_1) + v_6);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfMatrixAccess_123456) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  v[e(1i)][e(2i)][e(3i)] += v[e(4i)][e(5i)][e(6i)];
}
)",
             R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_1 = &(v[e(1i)][e(2i)]);
  let v_2 = e(3i);
  let v_3 = v[e(4i)][e(5i)][e(6i)];
  (*(v_1))[v_2] = ((*(v_1))[v_2] + v_3);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfMatrixAccess_261345) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  v[e(1i)][v_2][e(3i)] += v[e(4i)][e(5i)][v_3];
}
)",
             R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(2i);
  let v_3 = e(6i);
  let v_1 = &(v[e(1i)][v_2]);
  let v_4 = e(3i);
  let v_5 = v[e(4i)][e(5i)][v_3];
  (*(v_1))[v_4] = ((*(v_1))[v_4] + v_5);
})");
}

TEST_F(IRToProgramRoundtripTest, CompoundAssign_ArrayOfMatrixAccess_532614) {
    RUN_TEST(R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  v[e(1i)][v_4][v_3] += v[e(4i)][v_2][v_5];
}
)",
             R"(
fn e(i : i32) -> i32 {
  return i;
}

fn f() {
  var v : array<mat3x4<f32>, 5u>;
  let v_2 = e(5i);
  let v_3 = e(3i);
  let v_4 = e(2i);
  let v_5 = e(6i);
  let v_1 = &(v[e(1i)][v_4]);
  let v_6 = v[e(4i)][v_2][v_5];
  (*(v_1))[v_3] = ((*(v_1))[v_3] + v_6);
})");
}

////////////////////////////////////////////////////////////////////////////////
// Phony Assignment
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, PhonyAssign_PrivateVar) {
    RUN_TEST(R"(
var<private> p : i32;

fn f() {
  _ = p;
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_FunctionVar) {
    RUN_TEST(R"(
fn f() {
  var i : i32;
  _ = i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_FunctionLet) {
    RUN_TEST(R"(
fn f() {
  let i : i32 = 42i;
  _ = i;
}
)",
             R"(
fn f() {
  let i = 42i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_HandleVar) {
    RUN_TEST(R"(
@group(0u) @binding(0u) var t : texture_2d<f32>;

fn f() {
  _ = t;
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_Constant) {
    RUN_TEST(R"(
fn f() {
  _ = 42i;
}
)",
             R"(
fn f() {
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_Call) {
    RUN_TEST(R"(
fn v() -> i32 {
  return 42;
}

fn f() {
  _ = v();
}
)",
             R"(
fn v() -> i32 {
  return 42i;
}

fn f() {
  v();
}
)");
}

TEST_F(IRToProgramRoundtripTest, PhonyAssign_Conversion) {
    RUN_TEST(R"(
fn f() {
  let i = 42i;
  _ = u32(i);
}
)",
             R"(
fn f() {
  let i = 42i;
  _ = u32(i);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// let
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, LetUsedOnce) {
    RUN_TEST(R"(
fn f(i : u32) -> u32 {
  let v = ~(i);
  return v;
}
)");
}

TEST_F(IRToProgramRoundtripTest, LetUsedTwice) {
    RUN_TEST(R"(
fn f(i : i32) -> i32 {
  let v = (i * 2i);
  return (v + v);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Module-scope var
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_i32) {
    RUN_TEST(R"(
var<private> v : i32 = 1i;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_u32) {
    RUN_TEST(R"(
var<private> v : u32 = 1u;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_f32) {
    RUN_TEST(R"(
var<private> v : f32 = 1.0f;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_f16) {
    RUN_TEST(R"(
enable f16;

var<private> v : f16 = 1.0h;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_bool) {
    RUN_TEST(R"(
var<private> v : bool = true;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_array_NoArgs) {
    RUN_TEST(R"(
var<private> v : array<i32, 4u> = array<i32, 4u>();
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_array_Zero) {
    RUN_TEST(R"(
var<private> v : array<i32, 4u> = array<i32, 4u>(0i, 0i, 0i, 0i);
var<private> v : array<i32, 4u> = array<i32, 4u>();
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_array_SameValue) {
    RUN_TEST(R"(
var<private> v : array<i32, 4u> = array<i32, 4u>(4i, 4i, 4i, 4i);
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_array_DifferentValues) {
    RUN_TEST(R"(
var<private> v : array<i32, 4u> = array<i32, 4u>(1i, 2i, 3i, 4i);
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_struct_NoArgs) {
    RUN_TEST(R"(
struct S {
  i : i32,
  u : u32,
  f : f32,
}

var<private> s : S = S();
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_struct_Zero) {
    RUN_TEST(R"(
struct S {
  i : i32,
  u : u32,
  f : f32,
}

var<private> s : S = S(0i, 0u, 0f);
)",
             R"(
struct S {
  i : i32,
  u : u32,
  f : f32,
}

var<private> s : S = S();
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_struct_SameValue) {
    RUN_TEST(R"(
struct S {
  a : i32,
  b : i32,
  c : i32,
}

var<private> s : S = S(4i, 4i, 4i);
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_struct_DifferentValues) {
    RUN_TEST(R"(
struct S {
  a : i32,
  b : i32,
  c : i32,
}

var<private> s : S = S(1i, 2i, 3i);
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_vec3f_NoArgs) {
    RUN_TEST(R"(
var<private> v : vec3<f32> = vec3<f32>();
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_vec3f_Zero) {
    RUN_TEST(R"(
var<private> v : vec3<f32> = vec3<f32>(0f);",
             "var<private> v : vec3<f32> = vec3<f32>();
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_vec3f_Splat) {
    RUN_TEST(R"(
var<private> v : vec3<f32> = vec3<f32>(1.0f);
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_vec3f_Scalars) {
    RUN_TEST(R"(
var<private> v : vec3<f32> = vec3<f32>(1.0f, 2.0f, 3.0f);
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_NoArgs) {
    RUN_TEST(R"(
var<private> v : mat2x3<f32> = mat2x3<f32>();
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_Scalars_SameValue) {
    RUN_TEST(R"(
var<private> v : mat2x3<f32> = mat2x3<f32>(4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f);
var<private> v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(4.0f), vec3<f32>(4.0f));
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_Scalars) {
    RUN_TEST(R"(
var<private> v : mat2x3<f32> = mat2x3<f32>(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
var<private> v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(4.0f, 5.0f, 6.0f));
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_Columns) {
    RUN_TEST(
        R"(
var<private> v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(4.0f, 5.0f, 6.0f));
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Private_mat2x3f_Columns_SameValue) {
    RUN_TEST(R"(
var<private> v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(4.0f, 4.0f, 4.0f), vec3<f32>(4.0f, 4.0f, 4.0f));
var<private> v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(4.0f), vec3<f32>(4.0f));
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Uniform_vec4i) {
    RUN_TEST(R"(
@group(10u) @binding(20u) var<uniform> v : vec4<i32>;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_StorageRead_u32) {
    RUN_TEST(R"(
@group(10u) @binding(20u) var<storage, read> v : u32;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_StorageReadWrite_i32) {
    RUN_TEST(R"(
@group(10u) @binding(20u) var<storage, read_write> v : i32;
)");
}
TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Handle_Texture2D) {
    RUN_TEST(R"(
@group(0u) @binding(0u) var t : texture_2d<f32>;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Handle_Sampler) {
    RUN_TEST(R"(
@group(0u) @binding(0u) var s : sampler;
)");
}

TEST_F(IRToProgramRoundtripTest, ModuleScopeVar_Handle_SamplerCmp) {
    RUN_TEST(R"(
@group(0u) @binding(0u) var s : sampler_comparison;
)");
}

////////////////////////////////////////////////////////////////////////////////
// Function-scope var
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, FunctionScopeVar_i32) {
    RUN_TEST(R"(
fn f() {
  var i : i32;
}
)");
}

TEST_F(IRToProgramRoundtripTest, FunctionScopeVar_i32_InitLiteral) {
    RUN_TEST(R"(
fn f() {
  var i : i32 = 42i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, FunctionScopeVar_Chained) {
    RUN_TEST(R"(
fn f() {
  var a : i32 = 42i;
  var b : i32 = a;
  var c : i32 = b;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Function-scope let
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, FunctionScopeLet_i32) {
    RUN_TEST(R"(
fn f(i : i32) -> i32 {
  let a = (42i + i);
  let b = (24i + i);
  let c = (a + b);
  return c;
}
)");
}

TEST_F(IRToProgramRoundtripTest, FunctionScopeLet_ptr) {
    RUN_TEST(R"(
fn f() -> i32 {
  var a : array<i32, 3u>;
  let b = &(a[1i]);
  let c = *(b);
  return c;
}
)");
}

TEST_F(IRToProgramRoundtripTest, FunctionScopeLet_NoConstEvalError) {
    // Tests that 'a' and 'b' are preserved as a let.
    // If their constant values were inlined, then the initializer for 'c' would be treated as a
    // constant expression instead of the authored runtime expression. Evaluating '1 / 0' as a
    // constant expression is a WGSL validation error.
    RUN_TEST(R"(
fn f() {
  let a = 1i;
  let b = 0i;
  let c = (a / b);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// If
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, If_CallFn) {
    RUN_TEST(R"(
fn a() {
}

fn f(cond : bool) {
  if (cond) {
    a();
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Return) {
    RUN_TEST(R"(
fn f(cond : bool) {
  if (cond) {
    return;
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Return_i32) {
    RUN_TEST(R"(
fn f() -> i32 {
  var cond : bool = true;
  if (cond) {
    return 42i;
  }
  return 10i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_CallFn_Else_CallFn) {
    RUN_TEST(R"(
fn a() {
}

fn b() {
}

fn f(cond : bool) {
  if (cond) {
    a();
  } else {
    b();
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Return_f32_Else_Return_f32) {
    RUN_TEST(R"(
fn f() -> f32 {
  var cond : bool = true;
  if (cond) {
    return 1.0f;
  } else {
    return 2.0f;
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Return_u32_Else_CallFn) {
    RUN_TEST(R"(
fn a() {
}

fn b() {
}

fn f() -> u32 {
  var cond : bool = true;
  if (cond) {
    return 1u;
  } else {
    a();
  }
  b();
  return 2u;
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_CallFn_ElseIf_CallFn) {
    RUN_TEST(R"(
fn a() {
}

fn b() {
}

fn c() {
}

fn f() {
  var cond : bool = true;
  if (cond) {
    a();
  } else if (false) {
    b();
  }
  c();
}
)");
}

TEST_F(IRToProgramRoundtripTest, If_Else_Chain) {
    RUN_TEST(R"(
fn x(i : i32) -> bool {
  return true;
}

fn f(a : bool, b : bool, c : bool, d : bool) {
  if (a) {
    x(0i);
  } else if (b) {
    x(1i);
  } else if (c) {
    x(2i);
  } else {
    x(3i);
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Switch
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Switch_Default) {
    RUN_TEST(R"(
fn a() {
}

fn f() {
  var v : i32 = 42i;
  switch(v) {
    default: {
      a();
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Switch_3_Cases) {
    RUN_TEST(R"(
fn a() {
}

fn b() {
}

fn c() {
}

fn f() {
  var v : i32 = 42i;
  switch(v) {
    case 0i: {
      a();
    }
    case 1i, default: {
      b();
    }
    case 2i: {
      c();
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Switch_3_Cases_AllReturn) {
    RUN_TEST(R"(
fn a() {
}

fn f() {
  var v : i32 = 42i;
  switch(v) {
    case 0i: {
      return;
    }
    case 1i, default: {
      return;
    }
    case 2i: {
      return;
    }
  }
  a();
}
)",
             R"(
fn a() {
}

fn f() {
  var v : i32 = 42i;
  switch(v) {
    case 0i: {
      return;
    }
    case 1i, default: {
      return;
    }
    case 2i: {
      return;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Switch_Nested) {
    RUN_TEST(R"(
fn a() {
}

fn b() {
}

fn c() {
}

fn f() {
  var v1 : i32 = 42i;
  var v2 : i32 = 24i;
  switch(v1) {
    case 0i: {
      a();
    }
    case 1i, default: {
      switch(v2) {
        case 0i: {
        }
        case 1i, default: {
          return;
        }
      }
    }
    case 2i: {
      c();
    }
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// For
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, For_Empty) {
    RUN_TEST(R"(
fn f() {
  for(var i : i32 = 0i; (i < 5i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_Empty_NoInit) {
    RUN_TEST(R"(
fn f() {
  var i : i32 = 0i;
  for(; (i < 5i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_Empty_NoCond) {
    RUN_TEST(R"(
fn f() {
  for(var i : i32 = 0i; ; i = (i + 1i)) {
    break;
  }
}
)",
             R"(
fn f() {
  {
    var i : i32 = 0i;
    loop {
      break;

      continuing {
        i = (i + 1i);
      }
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_Empty_NoCont) {
    RUN_TEST(R"(
fn f() {
  for(var i : i32 = 0i; (i < 5i); ) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_ComplexBody) {
    RUN_TEST(R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  for(var i : i32 = 0i; (i < 5i); i = (i + 1i)) {
    if (a(42i)) {
      return 1i;
    } else {
      return 2i;
    }
  }
  return 3i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_ComplexBody_NoInit) {
    RUN_TEST(R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  var i : i32 = 0i;
  for(; (i < 5i); i = (i + 1i)) {
    if (a(42i)) {
      return 1i;
    } else {
      return 2i;
    }
  }
  return 3i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_ComplexBody_NoCond) {
    RUN_TEST(R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  for(var i : i32 = 0i; ; i = (i + 1i)) {
    if (a(42i)) {
      return 1i;
    } else {
      return 2i;
    }
  }
}
)",
             R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  {
    var i : i32 = 0i;
    loop {
      if (a(42i)) {
        return 1i;
      } else {
        return 2i;
      }

      continuing {
        i = (i + 1i);
      }
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_ComplexBody_NoCont) {
    RUN_TEST(R"(
fn a(v : i32) -> bool {
  return (v == 1i);
}

fn f() -> i32 {
  for(var i : i32 = 0i; (i < 5i); ) {
    if (a(42i)) {
      return 1i;
    } else {
      return 2i;
    }
  }
  return 3i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_CallInInitCondCont) {
    RUN_TEST(R"(
fn n(v : i32) -> i32 {
  return (v + 1i);
}

fn f() {
  for(var i : i32 = n(0i); (i < n(1i)); i = n(i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_AssignAsInit) {
    RUN_TEST(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i = 0i; (i < 10i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_CompoundAssignAsInit) {
    RUN_TEST(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i += 0i; (i < 10i); i = (i + 1i)) {
  }
}
)",
             R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i = (i + 0i); (i < 10i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_IncrementAsInit) {
    RUN_TEST(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i++; (i < 10i); i = (i + 1i)) {
  }
}
)",
             R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i = (i + 1i); (i < 10i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_DecrementAsInit) {
    RUN_TEST(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i--; (i < 10i); i = (i + 1i)) {
  }
}
)",
             R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(i = (i - 1i); (i < 10i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, For_CallAsInit) {
    RUN_TEST(R"(
fn n() {
}

fn f() {
  var i : i32 = 0i;
  for(n(); (i < 10i); i = (i + 1i)) {
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// While
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, While_Empty) {
    RUN_TEST(R"(
fn f() {
  while(true) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, While_Cond) {
    RUN_TEST(R"(
fn f(cond : bool) {
  while(cond) {
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, While_Break) {
    RUN_TEST(R"(
fn f() {
  while(true) {
    break;
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, While_IfBreak) {
    RUN_TEST(R"(
fn f(cond : bool) {
  while(true) {
    if (cond) {
      break;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, While_IfReturn) {
    RUN_TEST(R"(
fn f(cond : bool) {
  while(true) {
    if (cond) {
      return;
    }
  }
}
)");
}

// Test case for crbug.com/351700183.
TEST_F(IRToProgramRoundtripTest, While_ConditionAndBreak) {
    RUN_TEST(R"(
fn f() {
  while(true) {
    if (false) {
    } else {
      break;
    }
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Loop
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Loop_Break) {
    RUN_TEST(R"(
fn f() {
  loop {
    break;
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_IfBreak) {
    RUN_TEST(R"(
fn f(cond : bool) {
  loop {
    if (cond) {
      break;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_IfReturn) {
    RUN_TEST(R"(
fn f(cond : bool) {
  loop {
    if (cond) {
      return;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_IfContinuing) {
    RUN_TEST(R"(
fn f() {
  var cond : bool = false;
  loop {
    if (cond) {
      return;
    }

    continuing {
      cond = true;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_VarsDeclaredOutsideAndInside) {
    RUN_TEST(R"(
fn f() {
  var b : i32 = 1i;
  loop {
    var a : i32 = 2i;
    if ((a == b)) {
      return;
    }

    continuing {
      b = (a + b);
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_BreakIf_EmptyBody) {
    RUN_TEST(R"(
fn f() {
  loop {

    continuing {
      break if false;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_BreakIf_NotFalse) {
    RUN_TEST(R"(
fn f() {
  loop {
    if (false) {
    } else {
      break;
    }

    continuing {
       break if !false;
    }
  }
}
)",
             R"(
fn f() {
  loop {
    if (!(false)) {
      break;
    }

    continuing {
      break if true;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_BreakIf_NotTrue) {
    RUN_TEST(R"(
fn f() {
  loop {
    if (false) {
    } else {
      break;
    }

    continuing {
       break if !true;
    }
  }
}
)",
             R"(
fn f() {
  loop {
    if (!(false)) {
      break;
    }

    continuing {
      break if false;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Loop_WithReturn) {
    RUN_TEST(R"(
fn f() {
  loop {
    let i = 42i;
    return;
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Shadowing tests
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_Fn) {
    RUN_TEST(R"(
fn f32() {
  var v = mat4x4f();
}
)",
             R"(
fn f32_1() {
  var v : mat4x4<f32> = mat4x4<f32>();
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_Struct) {
    RUN_TEST(R"(
struct f32 {
  v : i32,
}

fn f(s : f32) {
  let f = vec2f(1.0f);
}
)",
             R"(
struct f32_1 {
  v : i32,
}

fn f(s : f32_1) {
  let f_1 = vec2<f32>(1.0f);
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_ModVar) {
    RUN_TEST(R"(
var<private> f32 : vec2f = vec2f(0.0f, 1.0f);
)",
             R"(
var<private> f32_1 : vec2<f32> = vec2<f32>(0.0f, 1.0f);
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_ModVar2) {
    RUN_TEST(R"(
var<private> f32 : i32 = 1i;

var<private> v = vec2(1.0).x;
)",
             R"(
var<private> f32_1 : i32 = 1i;

var<private> v : f32 = 1.0f;
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_f32_With_Alias) {
    RUN_TEST(R"(
alias f32 = i32;

fn f() {
  var v = vec3(1.0f, 2.0f, 3.0f);
}
)",
             R"(
fn f() {
  var v : vec3<f32> = vec3<f32>(1.0f, 2.0f, 3.0f);
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_Struct_With_FnVar) {
    RUN_TEST(R"(
struct S {
  i : i32,
}

fn f() -> i32 {
  var S_1 : S = S();
  return S_1.i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_Struct_With_Param) {
    RUN_TEST(R"(
struct S {
  i : i32,
}

fn f(S_1 : S) -> i32 {
  return S_1.i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_ModVar_With_FnVar) {
    RUN_TEST(R"(
var<private> i : i32 = 1i;

fn f() -> i32 {
  i = (i + 1i);
  var i_1 : i32 = (i + 1i);
  return i_1;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_ModVar_With_FnLet) {
    RUN_TEST(R"(
var<private> i : i32 = 1i;

fn f() -> i32 {
  i = (i + 1i);
  let i_1 = (i + 1i);
  return i_1;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_IfVar) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  if (true) {
    i = (i + 1i);
    var i_1 : i32 = (i + 1i);
    i_1 = (i_1 + 1i);
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_IfLet) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  if (true) {
    i = (i + 1i);
    let i_1 = (i + 1i);
    return i_1;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_WhileVar) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  while((i < 4i)) {
    var i_1 : i32 = (i + 1i);
    return i_1;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_WhileLet) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  while((i < 4i)) {
    let i_1 = (i + 1i);
    return i_1;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_ForInitVar) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  for(var i_1 : f32 = 0.0f; (i_1 < 4.0f); ) {
    let j = i_1;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_ForInitLet) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  for(let i_1 = 0.0f; (i_1 < 4.0f); ) {
    let j = i_1;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_ForBodyVar) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  for(var x : i32 = 0i; (i < 4i); ) {
    var i_1 : i32 = (i + 1i);
    return i_1;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_ForBodyLet) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  for(var x : i32 = 0i; (i < 4i); ) {
    let i_1 = (i + 1i);
    return i_1;
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_LoopBodyVar) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  loop {
    if ((i == 2i)) {
      break;
    }
    var i_1 : i32 = (i + 1i);
    if ((i_1 == 3i)) {
      break;
    }
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_LoopBodyLet) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  loop {
    if ((i == 2i)) {
      break;
    }
    let i_1 = (i + 1i);
    if ((i_1 == 3i)) {
      break;
    }
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_LoopContinuingVar) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  loop {
    if ((i == 2i)) {
      break;
    }

    continuing {
      var i_1 : i32 = (i + 1i);
      break if (i_1 > 2i);
    }
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_LoopContinuingLet) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  loop {
    if ((i == 2i)) {
      break;
    }

    continuing {
      let i_1 = (i + 1i);
      break if (i_1 > 2i);
    }
  }
  return i;
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_SwitchCaseVar) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  switch(i) {
    case 0i: {
      return i;
    }
    case 1i: {
      var i_1 : i32 = (i + 1i);
      return i_1;
    }
    default: {
      return i;
    }
  }
}
)");
}

TEST_F(IRToProgramRoundtripTest, Shadow_FnVar_With_SwitchCaseLet) {
    RUN_TEST(R"(
fn f() -> i32 {
  var i : i32;
  switch(i) {
    case 0i: {
      return i;
    }
    case 1i: {
      let i_1 = (i + 1i);
      return i_1;
    }
    default: {
      return i;
    }
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// chromium_internal_input_attachments
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, Call_InputAttachmentLoad) {
    RUN_TEST(R"(
enable chromium_internal_input_attachments;

@group(0u) @binding(0u) @input_attachment_index(3u) var input_tex : input_attachment<f32>;

@fragment
fn main() -> @location(0u) vec4<f32> {
  return inputAttachmentLoad(input_tex);
}
)");
}

TEST_F(IRToProgramRoundtripTest, WorkgroupSizeLargerThanI32) {
    RUN_TEST(R"(
@compute @workgroup_size(4294967295u, 1u, 1u)
fn main() {
}
)");
}

TEST_F(IRToProgramRoundtripTest, BindingLargerThanI32) {
    RUN_TEST(R"(
@group(0u) @binding(4000000000u) var s : sampler;
)");
}

TEST_F(IRToProgramRoundtripTest, GroupLargerThanI32) {
    RUN_TEST(R"(
@group(4000000000u) @binding(0u) var s : sampler;
)");
}

TEST_F(IRToProgramRoundtripTest, LocationInputLargerThanI32) {
    RUN_TEST(R"(
@fragment
fn main(@location(4000000000u) color : vec4<f32>) {
}
)");
}

TEST_F(IRToProgramRoundtripTest, LocationOutputLargerThanI32) {
    RUN_TEST(R"(
@fragment
fn main() -> @location(4000000000u) vec4<f32> {
  return vec4<f32>();
}
)");
}

// Test that we do not try to declare or name the unnameable builtin structure types.
// See crbug.com/350518995.
TEST_F(IRToProgramRoundtripTest, BuiltinStructInLetAndVar) {
    RUN_TEST(R"(
fn a(x : f32) {
  let b = frexp(x);
}

fn c(y : f32) {
  var d = frexp(y);
}
)");
}

// Test that we do not try to name the unnameable builtin structure types in array declarations.
// See crbug.com/353249345.
TEST_F(IRToProgramRoundtripTest, BuiltinStructInInferredArrayType) {
    RUN_TEST(R"(
fn a(x : f32) {
  let y = array(frexp(x));
}
)");
}

// Test that we do not try to name the unnameable builtin structure types in nested array
// declarations. See crbug.com/380898799.
TEST_F(IRToProgramRoundtripTest, BuiltinStructInInferredNestedArrayType) {
    RUN_TEST(R"(
fn a(x : f32) {
  let y = array(array(frexp(x)));
}
)");
}

// Test that we rename declarations that shadow builtin types when they are used in arrays.
// See crbug.com/380903161.
TEST_F(IRToProgramRoundtripTest, BuiltinTypeNameShadowedAndUsedInArray) {
    RUN_TEST(R"(
fn a(f32 : f32) {
  let x = array(1.0f);
}
)",
             R"(
fn a(f32_1 : f32) {
  let x = array<f32, 1u>(1.0f);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// chromium_experimental_subgroup_matrix
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramRoundtripTest, SubgroupMatrixConstruct) {
    RUN_TEST(R"(
enable chromium_experimental_subgroup_matrix;

fn f() {
  var m = subgroup_matrix_left<f32, 8, 8>>();
}
)");
}

TEST_F(IRToProgramRoundtripTest, SubgroupMatrixLoad) {
    RUN_TEST(R"(
enable chromium_experimental_subgroup_matrix;

@group(0u) @binding(0u) var<storage, read_write> buffer : array<f32, 64u>;

fn f() {
  let l = subgroupMatrixLoad<subgroup_matrix_left<f32, 4, 2>>(&(buffer), 0u, false, 4u);
  let r = subgroupMatrixLoad<subgroup_matrix_right<f32, 2, 4>>(&(buffer), 32u, true, 8u);
}
)");
}

}  // namespace
}  // namespace tint::wgsl
