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

// GEN_BUILD:CONDITION(tint_build_wgsl_writer)

#include <sstream>
#include <string>

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program_test.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl::writer {

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

IRToProgramTest::Result IRToProgramTest::Run() {
    Result result;

    result.ir = tint::core::ir::Disassembler(mod).Plain();

    ProgramOptions options;
    options.allowed_features = AllowedFeatures::Everything();
    auto output_program = IRToProgram(mod, options);
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
        return result;
    }

    result.wgsl = std::string(tint::TrimSpace(output->wgsl));
    if (!result.wgsl.empty()) {
        result.wgsl = "\n" + result.wgsl + "\n";
    }

    return result;
}

namespace {

TEST_F(IRToProgramTest, EmptyModule) {
    EXPECT_WGSL("");
}

TEST_F(IRToProgramTest, SingleFunction_Return) {
    auto* fn = b.Function("f", ty.void_());

    fn->Block()->Append(b.Return(fn));

    EXPECT_WGSL(R"(
fn f() {
}
)");
}

TEST_F(IRToProgramTest, SingleFunction_Return_i32) {
    auto* fn = b.Function("f", ty.i32());

    fn->Block()->Append(b.Return(fn, 42_i));

    EXPECT_WGSL(R"(
fn f() -> i32 {
  return 42i;
}
)");
}

TEST_F(IRToProgramTest, SingleFunction_Parameters) {
    auto* fn = b.Function("f", ty.i32());
    auto* i = b.FunctionParam("i", ty.i32());
    auto* u = b.FunctionParam("u", ty.u32());
    fn->SetParams({i, u});

    fn->Block()->Append(b.Return(fn, i));

    EXPECT_WGSL(R"(
fn f(i : i32, u : u32) -> i32 {
  return i;
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_Compute) {
    auto* fn = b.ComputeFunction("f", 3_u, 4_u, 5_u);

    fn->Block()->Append(b.Return(fn));

    EXPECT_WGSL(R"(
@compute @workgroup_size(3u, 4u, 5u)
fn f() {
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_Fragment) {
    auto* fn = b.Function("f", ty.void_(), core::ir::Function::PipelineStage::kFragment);

    fn->Block()->Append(b.Return(fn));

    EXPECT_WGSL(R"(
@fragment
fn f() {
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_Vertex) {
    auto* fn = b.Function("f", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    fn->SetReturnBuiltin(core::BuiltinValue::kPosition);

    fn->Block()->Append(b.Return(fn, b.Splat<vec4<f32>>(0_f)));

    EXPECT_WGSL(R"(
@vertex
fn f() -> @builtin(position) vec4<f32> {
  return vec4<f32>();
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_Parameter_BuiltinAndInvariant) {
    auto* fn = b.Function("f", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    auto* param = b.FunctionParam("input", ty.vec4<f32>());
    param->SetBuiltin(core::BuiltinValue::kPosition);
    param->SetInvariant(true);
    fn->SetParams({param});

    fn->Block()->Append(b.Return(fn));

    EXPECT_WGSL(R"(
@fragment
fn f(@builtin(position) @invariant input : vec4<f32>) {
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_Parameter_LocationAndInterpolation) {
    auto* fn = b.Function("f", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    auto* param = b.FunctionParam("input", ty.f32());
    param->SetLocation(2u);
    param->SetInterpolation(core::Interpolation{core::InterpolationType::kLinear,
                                                core::InterpolationSampling::kCentroid});
    fn->SetParams({param});

    fn->Block()->Append(b.Return(fn));

    EXPECT_WGSL(R"(
@fragment
fn f(@location(2u) @interpolate(linear, centroid) input : f32) {
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_Parameter_Color) {
    auto* fn = b.Function("f", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    auto* param = b.FunctionParam("input", ty.f32());
    param->SetColor(2u);
    fn->SetParams({param});

    fn->Block()->Append(b.Return(fn));

    EXPECT_WGSL(R"(
enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(2u) input : f32) {
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_ReturnAttribute_FragDepth) {
    auto* fn = b.Function("f", ty.f32(), core::ir::Function::PipelineStage::kFragment);
    fn->SetReturnBuiltin(core::BuiltinValue::kFragDepth);

    fn->Block()->Append(b.Return(fn, b.Constant(0.5_f)));

    EXPECT_WGSL(R"(
@fragment
fn f() -> @builtin(frag_depth) f32 {
  return 0.5f;
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_ReturnAttribute_SampleMask) {
    auto* fn = b.Function("f", ty.u32(), core::ir::Function::PipelineStage::kFragment);
    fn->SetReturnBuiltin(core::BuiltinValue::kSampleMask);

    fn->Block()->Append(b.Return(fn, b.Constant(3_u)));

    EXPECT_WGSL(R"(
@fragment
fn f() -> @builtin(sample_mask) u32 {
  return 3u;
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_ReturnAttribute_Invariant) {
    auto* fn = b.Function("f", ty.vec4<f32>(), core::ir::Function::PipelineStage::kVertex);
    fn->SetReturnBuiltin(core::BuiltinValue::kPosition);
    fn->SetReturnInvariant(true);

    fn->Block()->Append(b.Return(fn, b.Splat<vec4<f32>>(0_f)));

    EXPECT_WGSL(R"(
@vertex
fn f() -> @builtin(position) @invariant vec4<f32> {
  return vec4<f32>();
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_ReturnAttribute_Location) {
    auto* fn = b.Function("f", ty.vec4<f32>(), core::ir::Function::PipelineStage::kFragment);
    fn->SetReturnLocation(1);

    fn->Block()->Append(b.Return(fn, b.Splat<vec4<f32>>(0_f)));

    EXPECT_WGSL(R"(
@fragment
fn f() -> @location(1u) vec4<f32> {
  return vec4<f32>();
}
)");
}

namespace {
core::ir::FunctionParam* MakeBuiltinParam(core::ir::Builder& b,
                                          const core::type::Type* type,
                                          enum core::BuiltinValue builtin) {
    auto* param = b.FunctionParam(type);
    param->SetBuiltin(builtin);
    return param;
}
}  // namespace

TEST_F(IRToProgramTest, EntryPoint_ParameterAttribute_Compute) {
    auto* fn = b.ComputeFunction("f", 3_u, 4_u, 5_u);
    fn->SetParams({
        MakeBuiltinParam(b, ty.vec3<u32>(), core::BuiltinValue::kLocalInvocationId),
        MakeBuiltinParam(b, ty.u32(), core::BuiltinValue::kLocalInvocationIndex),
        MakeBuiltinParam(b, ty.vec3<u32>(), core::BuiltinValue::kGlobalInvocationId),
        MakeBuiltinParam(b, ty.vec3<u32>(), core::BuiltinValue::kWorkgroupId),
        MakeBuiltinParam(b, ty.vec3<u32>(), core::BuiltinValue::kNumWorkgroups),
        MakeBuiltinParam(b, ty.u32(), core::BuiltinValue::kSubgroupInvocationId),
        MakeBuiltinParam(b, ty.u32(), core::BuiltinValue::kSubgroupSize),
    });

    fn->Block()->Append(b.Return(fn));

    EXPECT_WGSL(R"(
enable subgroups;

@compute @workgroup_size(3u, 4u, 5u)
fn f(@builtin(local_invocation_id) v : vec3<u32>, @builtin(local_invocation_index) v_1 : u32, @builtin(global_invocation_id) v_2 : vec3<u32>, @builtin(workgroup_id) v_3 : vec3<u32>, @builtin(num_workgroups) v_4 : vec3<u32>, @builtin(subgroup_invocation_id) v_5 : u32, @builtin(subgroup_size) v_6 : u32) {
}
)");
}

TEST_F(IRToProgramTest, EntryPoint_ParameterAttribute_Fragment) {
    auto* fn = b.Function("f", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    fn->SetParams({
        MakeBuiltinParam(b, ty.bool_(), core::BuiltinValue::kFrontFacing),
        MakeBuiltinParam(b, ty.u32(), core::BuiltinValue::kSampleIndex),
        MakeBuiltinParam(b, ty.u32(), core::BuiltinValue::kSampleMask),
        MakeBuiltinParam(b, ty.u32(), core::BuiltinValue::kSubgroupSize),
    });

    fn->Block()->Append(b.Return(fn));

    EXPECT_WGSL(R"(
enable subgroups;

@fragment
fn f(@builtin(front_facing) v : bool, @builtin(sample_index) v_1 : u32, @builtin(sample_mask) v_2 : u32, @builtin(subgroup_size) v_3 : u32) {
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Unary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, UnaryOp_Negate) {
    auto* fn = b.Function("f", ty.i32());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Negation(ty.i32(), i)); });

    EXPECT_WGSL(R"(
fn f(i : i32) -> i32 {
  return -(i);
}
)");
}

TEST_F(IRToProgramTest, UnaryOp_Complement) {
    auto* fn = b.Function("f", ty.u32());
    auto* i = b.FunctionParam("i", ty.u32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Complement(ty.u32(), i)); });

    EXPECT_WGSL(R"(
fn f(i : u32) -> u32 {
  return ~(i);
}
)");
}

TEST_F(IRToProgramTest, UnaryOp_Not) {
    auto* fn = b.Function("f", ty.bool_());
    auto* i = b.FunctionParam("b", ty.bool_());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Not(ty.bool_(), i)); });

    EXPECT_WGSL(R"(
fn f(b : bool) -> bool {
  return !(b);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Binary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, BinaryOp_Add) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Add(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a + b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Subtract) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Subtract(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a - b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Multiply) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Multiply(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a * b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Divide) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Divide(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a / b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Modulo) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Modulo(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a % b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_And) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.And(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a & b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Or) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Or(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a | b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Xor) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Xor(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> i32 {
  return (a ^ b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_Equal) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.Equal(ty.bool_(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> bool {
  return (a == b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_NotEqual) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.NotEqual(ty.bool_(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> bool {
  return (a != b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LessThan) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.LessThan(ty.bool_(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> bool {
  return (a < b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_GreaterThan) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.GreaterThan(ty.bool_(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> bool {
  return (a > b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_LessThanEqual) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.LessThanEqual(ty.bool_(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> bool {
  return (a <= b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_GreaterThanEqual) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.i32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.GreaterThanEqual(ty.bool_(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : i32) -> bool {
  return (a >= b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_ShiftLeft) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.u32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.ShiftLeft(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : u32) -> i32 {
  return (a << b);
}
)");
}

TEST_F(IRToProgramTest, BinaryOp_ShiftRight) {
    auto* fn = b.Function("f", ty.i32());
    auto* pa = b.FunctionParam("a", ty.i32());
    auto* pb = b.FunctionParam("b", ty.u32());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] { b.Return(fn, b.ShiftRight(ty.i32(), pa, pb)); });

    EXPECT_WGSL(R"(
fn f(a : i32, b : u32) -> i32 {
  return (a >> b);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Type Construct
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, TypeConstruct_i32) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Construct<i32>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : i32) {
  var v : i32 = i32(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_u32) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Construct<u32>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : i32) {
  var v : u32 = u32(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_f32) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Construct<f32>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : i32) {
  var v : f32 = f32(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_bool) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Construct<bool>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : i32) {
  var v : bool = bool(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_struct) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.i32()},
                                                  {mod.symbols.New("b"), ty.u32()},
                                                  {mod.symbols.New("c"), ty.f32()},
                                              });

    auto* fn = b.Function("f", ty.void_());
    auto* x = b.FunctionParam("x", ty.i32());
    auto* y = b.FunctionParam("y", ty.u32());
    auto* z = b.FunctionParam("z", ty.f32());
    fn->SetParams({x, y, z});

    b.Append(fn->Block(), [&] {
        Var("v", b.Construct(S, x, y, z));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
struct S {
  a : i32,
  b : u32,
  c : f32,
}

fn f(x : i32, y : u32, z : f32) {
  var v : S = S(x, y, z);
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_array) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Construct<array<i32, 3u>>(i, i, i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : i32) {
  var v : array<i32, 3u> = array<i32, 3u>(i, i, i);
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_vec3i_Splat) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Construct<vec3<i32>>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : i32) {
  var v : vec3<i32> = vec3<i32>(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_vec3i_Scalars) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Construct<vec3<i32>>(i, i, i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : i32) {
  var v : vec3<i32> = vec3<i32>(i, i, i);
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_mat2x3f_Scalars) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.f32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Construct<mat2x3<f32>>(i, i, i, i, i, i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : f32) {
  var v : mat2x3<f32> = mat2x3<f32>(i, i, i, i, i, i);
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_mat2x3f_Columns) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.f32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        auto* col_0 = b.Construct<vec3<f32>>(i, i, i);
        auto* col_1 = b.Construct<vec3<f32>>(i, i, i);
        Var("v", b.Construct<mat2x3<f32>>(col_0, col_1));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : f32) {
  var v : mat2x3<f32> = mat2x3<f32>(vec3<f32>(i, i, i), vec3<f32>(i, i, i));
}
)");
}

TEST_F(IRToProgramTest, TypeConstruct_Inlining) {
    auto* fn = b.Function("f", ty.void_());
    auto* i0 = b.FunctionParam("i0", ty.i32());
    auto* i1 = b.FunctionParam("i1", ty.i32());
    auto* i2 = b.FunctionParam("i2", ty.i32());
    auto* i3 = b.FunctionParam("i3", ty.i32());
    auto* i4 = b.FunctionParam("i4", ty.i32());
    auto* i5 = b.FunctionParam("i5", ty.i32());
    fn->SetParams({i0, i1, i2, i3, i4, i5});

    b.Append(fn->Block(), [&] {
        auto* f3 = b.Construct<f32>(i3);
        auto* f4 = b.Construct<f32>(i4);
        auto* f5 = b.Construct<f32>(i5);
        auto* f0 = b.Construct<f32>(i0);
        auto* f2 = b.Construct<f32>(i2);
        auto* f1 = b.Construct<f32>(i1);
        Var("v", b.Construct<mat2x3<f32>>(f0, f1, f2, f3, f4, f5));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i0 : i32, i1 : i32, i2 : i32, i3 : i32, i4 : i32, i5 : i32) {
  var v : mat2x3<f32> = mat2x3<f32>(f32(i0), f32(i1), f32(i2), f32(i3), f32(i4), f32(i5));
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Type Convert
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, TypeConvert_i32_to_u32) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Convert<u32>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : i32) {
  var v : u32 = u32(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConvert_u32_to_f32) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.u32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Convert<f32>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : u32) {
  var v : f32 = f32(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConvert_f32_to_i32) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.f32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Convert<i32>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : f32) {
  var v : i32 = i32(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConvert_bool_to_u32) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.bool_());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Convert<u32>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : bool) {
  var v : u32 = u32(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConvert_vec3i_to_vec3u) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.vec3<i32>());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Convert<vec3<u32>>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : vec3<i32>) {
  var v : vec3<u32> = vec3<u32>(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConvert_vec3u_to_vec3f) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.vec3<u32>());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Convert<vec3<f32>>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(i : vec3<u32>) {
  var v : vec3<f32> = vec3<f32>(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConvert_mat2x3f_to_mat2x3h) {
    auto* fn = b.Function("f", ty.void_());
    auto* i = b.FunctionParam("i", ty.mat2x3<f32>());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        Var("v", b.Convert<mat2x3<f16>>(i));
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
enable f16;

fn f(i : mat2x3<f32>) {
  var v : mat2x3<f16> = mat2x3<f16>(i);
}
)");
}

TEST_F(IRToProgramTest, TypeConvert_Inlining) {
    auto* fn_g = b.Function("g", ty.void_());
    fn_g->SetParams({
        b.FunctionParam("a", ty.i32()),
        b.FunctionParam("b", ty.u32()),
        b.FunctionParam("c", ty.f32()),
    });
    b.Append(fn_g->Block(), [&] { b.Return(fn_g); });

    auto* fn_f = b.Function("f", ty.void_());
    auto* v = b.FunctionParam("v", ty.f16());
    fn_f->SetParams({v});
    b.Append(fn_f->Block(), [&] {
        auto* u = b.Convert<u32>(v);
        auto* f = b.Convert<f32>(v);
        auto* i = b.Convert<i32>(v);
        b.Call(fn_g, i, u, f);
        b.Return(fn_f);
    });

    EXPECT_WGSL(R"(
enable f16;

fn g(a : i32, b : u32, c : f32) {
}

fn f(v : f16) {
  g(i32(v), u32(v), f32(v));
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Short-circuiting binary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, ShortCircuit_And_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(pa);
        if_->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if_->True(), [&] { b.ExitIf(if_, pb); });
        b.Append(if_->False(), [&] { b.ExitIf(if_, false); });

        b.Return(fn, if_);
    });

    EXPECT_WGSL(R"(
fn f(a : bool, b : bool) -> bool {
  return (a && b);
}
)");
}

TEST_F(IRToProgramTest, ShortCircuit_And_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, pb); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });

        auto* if2 = b.If(if1);
        if2->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, pc); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

        b.Return(fn, if2);
    });

    EXPECT_WGSL(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a && b) && c);
}
)");
}

TEST_F(IRToProgramTest, ShortCircuit_And_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] {
            auto* if2 = b.If(pb);
            if2->SetResults(b.InstructionResult(ty.bool_()));
            b.Append(if2->True(), [&] { b.ExitIf(if2, pc); });
            b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

            b.ExitIf(if1, if2);
        });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });
        b.Return(fn, if1);
    });

    EXPECT_WGSL(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return (a && (b && c));
}
)");
}

TEST_F(IRToProgramTest, ShortCircuit_Or_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(pa);
        if_->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if_->True(), [&] { b.ExitIf(if_, true); });
        b.Append(if_->False(), [&] { b.ExitIf(if_, pb); });

        b.Return(fn, if_);
    });

    EXPECT_WGSL(R"(
fn f(a : bool, b : bool) -> bool {
  return (a || b);
}
)");
}

TEST_F(IRToProgramTest, ShortCircuit_Or_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, pb); });

        auto* if2 = b.If(if1);
        if2->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, pc); });

        b.Return(fn, if2);
    });

    EXPECT_WGSL(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a || b) || c);
}
)");
}

TEST_F(IRToProgramTest, ShortCircuit_Or_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] {
            auto* if2 = b.If(pb);
            if2->SetResults(b.InstructionResult(ty.bool_()));
            b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
            b.Append(if2->False(), [&] { b.ExitIf(if2, pc); });

            b.ExitIf(if1, if2);
        });

        b.Return(fn, if1);
    });

    EXPECT_WGSL(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return (a || (b || c));
}
)");
}

TEST_F(IRToProgramTest, ShortCircuit_Mixed) {
    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn_d = b.Function("d", ty.bool_());
    b.Append(fn_d->Block(), [&] { b.Return(fn_d, true); });

    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, b.Call(ty.bool_(), fn_b)); });

        auto* if2 = b.If(if1);
        if2->SetResults(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] {
            auto* if3 = b.If(pc);
            if3->SetResults(b.InstructionResult(ty.bool_()));
            b.Append(if3->True(), [&] { b.ExitIf(if3, true); });
            b.Append(if3->False(), [&] { b.ExitIf(if3, b.Call(ty.bool_(), fn_d)); });

            b.ExitIf(if2, if3);
        });
        b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

        mod.SetName(if2, "l");
        b.Return(fn, if2);
    });

    EXPECT_WGSL(R"(
fn b() -> bool {
  return true;
}

fn d() -> bool {
  return true;
}

fn f(a : bool, c : bool) -> bool {
  return ((a || b()) && (c || d()));
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Compound assignment
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, CompoundAssign_Increment) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var<function, i32>("v");
        b.Store(v, b.Add(ty.i32(), b.Load(v), 1_i));

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var v : i32;
  v = (v + 1i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Decrement) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var<function, i32>("v");
        b.Store(v, b.Subtract(ty.i32(), b.Load(v), 1_i));

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var v : i32;
  v = (v - 1i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Add) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var<function, i32>("v");
        b.Store(v, b.Add(ty.i32(), b.Load(v), 8_i));

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var v : i32;
  v = (v + 8i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Subtract) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var<function, i32>("v");
        b.Store(v, b.Subtract(ty.i32(), b.Load(v), 8_i));

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var v : i32;
  v = (v - 8i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Multiply) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var<function, i32>("v");
        b.Store(v, b.Multiply(ty.i32(), b.Load(v), 8_i));

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var v : i32;
  v = (v * 8i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Divide) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var<function, i32>("v");
        b.Store(v, b.Divide(ty.i32(), b.Load(v), 8_i));

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var v : i32;
  v = (v / 8i);
}
)");
}

TEST_F(IRToProgramTest, CompoundAssign_Xor) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var<function, i32>("v");
        b.Store(v, b.Xor(ty.i32(), b.Load(v), 8_i));

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var v : i32;
  v = (v ^ 8i);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// let
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, LetUsedOnce) {
    auto* fn = b.Function("f", ty.u32());
    auto* i = b.FunctionParam("i", ty.u32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        auto* v = b.Let("v", b.Complement(ty.u32(), i));
        b.Return(fn, v);
    });

    EXPECT_WGSL(R"(
fn f(i : u32) -> u32 {
  let v = ~(i);
  return v;
}
)");
}

TEST_F(IRToProgramTest, LetUsedTwice) {
    auto* fn = b.Function("f", ty.i32());
    auto* i = b.FunctionParam("i", ty.i32());
    fn->SetParams({i});

    b.Append(fn->Block(), [&] {
        auto* v = b.Let("v", b.Multiply(ty.i32(), i, 2_i));
        b.Return(fn, b.Add(ty.i32(), v, v));
    });

    EXPECT_WGSL(R"(
fn f(i : i32) -> i32 {
  let v = (i * 2i);
  return (v + v);
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Function-scope var
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, FunctionScopeVar_i32) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {  //
        Var<function, i32>("i");

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var i : i32;
}
)");
}

TEST_F(IRToProgramTest, FunctionScopeVar_i32_InitLiteral) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        Var("i", 42_i);

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var i : i32 = 42i;
}
)");
}

TEST_F(IRToProgramTest, FunctionScopeVar_Chained) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* va = Var("a", 42_i);

        auto* vb = Var("b", b.Load(va));

        Var("c", b.Load(vb));

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var a : i32 = 42i;
  var b : i32 = a;
  var c : i32 = b;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// If
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, If_CallFn) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    fn->SetParams({cond});

    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(cond);
        b.Append(if_->True(), [&] {
            b.Call(ty.void_(), fn_a);
            b.ExitIf(if_);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn a() {
}

fn f(cond : bool) {
  if (cond) {
    a();
  }
}
)");
}

TEST_F(IRToProgramTest, If_Return) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    fn->SetParams({cond});

    b.Append(fn->Block(), [&] {
        auto if_ = b.If(cond);
        b.Append(if_->True(), [&] { b.Return(fn); });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(cond : bool) {
  if (cond) {
    return;
  }
}
)");
}

TEST_F(IRToProgramTest, If_Return_i32) {
    auto* fn = b.Function("f", ty.i32());

    b.Append(fn->Block(), [&] {
        auto* cond = Var("cond", true);
        auto if_ = b.If(b.Load(cond));
        b.Append(if_->True(), [&] { b.Return(fn, 42_i); });

        b.Return(fn, 10_i);
    });

    EXPECT_WGSL(R"(
fn f() -> i32 {
  var cond : bool = true;
  if (cond) {
    return 42i;
  }
  return 10i;
}
)");
}

TEST_F(IRToProgramTest, If_CallFn_Else_CallFn) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b); });

    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    fn->SetParams({cond});

    b.Append(fn->Block(), [&] {
        auto if_ = b.If(cond);
        b.Append(if_->True(), [&] {
            b.Call(ty.void_(), fn_a);
            b.ExitIf(if_);
        });
        b.Append(if_->False(), [&] {
            b.Call(ty.void_(), fn_b);
            b.ExitIf(if_);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, If_Return_f32_Else_Return_f32) {
    auto* fn = b.Function("f", ty.f32());

    b.Append(fn->Block(), [&] {
        auto* cond = Var("cond", true);
        auto if_ = b.If(b.Load(cond));
        b.Append(if_->True(), [&] { b.Return(fn, 1.0_f); });
        b.Append(if_->False(), [&] { b.Return(fn, 2.0_f); });

        b.Unreachable();
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, If_Return_u32_Else_CallFn) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b); });

    auto* fn = b.Function("f", ty.u32());

    b.Append(fn->Block(), [&] {
        auto* cond = Var("cond", true);
        auto if_ = b.If(b.Load(cond));
        b.Append(if_->True(), [&] { b.Return(fn, 1_u); });
        b.Append(if_->False(), [&] {
            b.Call(ty.void_(), fn_a);
            b.ExitIf(if_);
        });
        b.Call(ty.void_(), fn_b);
        b.Return(fn, 2_u);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, If_CallFn_ElseIf_CallFn) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b); });

    auto* fn_c = b.Function("c", ty.void_());
    b.Append(fn_c->Block(), [&] { b.Return(fn_c); });

    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* cond = Var("cond", true);
        auto if1 = b.If(b.Load(cond));
        b.Append(if1->True(), [&] {
            b.Call(ty.void_(), fn_a);
            b.ExitIf(if1);
        });
        b.Append(if1->False(), [&] {
            auto* if2 = b.If(b.Constant(false));
            b.Append(if2->True(), [&] {
                b.Call(ty.void_(), fn_b);
                b.ExitIf(if2);
            });
            b.ExitIf(if1);
        });
        b.Call(ty.void_(), fn_c);

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, If_Else_Chain) {
    auto* x = b.Function("x", ty.bool_());
    auto* i = b.FunctionParam("i", ty.i32());
    x->SetParams({i});
    b.Append(x->Block(), [&] { b.Return(x, true); });

    auto* fn = b.Function("f", ty.void_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    auto* pd = b.FunctionParam("d", ty.bool_());
    fn->SetParams({pa, pb, pc, pd});

    b.Append(fn->Block(), [&] {
        auto if1 = b.If(pa);
        b.Append(if1->True(), [&] {
            b.Call(ty.void_(), x, 0_i);
            b.ExitIf(if1);
        });
        b.Append(if1->False(), [&] {
            auto* if2 = b.If(pb);
            b.Append(if2->True(), [&] {
                b.Call(ty.void_(), x, 1_i);
                b.ExitIf(if2);
            });
            b.Append(if2->False(), [&] {
                auto* if3 = b.If(pc);
                b.Append(if3->True(), [&] {
                    b.Call(ty.void_(), x, 2_i);
                    b.ExitIf(if3);
                });
                b.Append(if3->False(), [&] {
                    b.Call(ty.void_(), x, 3_i);
                    b.ExitIf(if3);
                });
                b.ExitIf(if2);
            });
            b.ExitIf(if1);
        });

        b.Return(fn);
    });
    EXPECT_WGSL(R"(
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
TEST_F(IRToProgramTest, Switch_Default) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var("v", 42_i);

        auto s = b.Switch(b.Load(v));
        b.Append(b.DefaultCase(s), [&] {
            b.Call(ty.void_(), fn_a);
            b.ExitSwitch(s);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, Switch_3_Cases) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b); });

    auto* fn_c = b.Function("c", ty.void_());
    b.Append(fn_c->Block(), [&] { b.Return(fn_c); });

    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var("v", 42_i);

        auto s = b.Switch(b.Load(v));
        b.Append(b.Case(s, {b.Constant(0_i)}), [&] {
            b.Call(ty.void_(), fn_a);
            b.ExitSwitch(s);
        });
        b.Append(b.Case(s, {b.Constant(1_i), nullptr}), [&] {
            b.Call(ty.void_(), fn_b);
            b.ExitSwitch(s);
        });
        b.Append(b.Case(s, {b.Constant(2_i)}), [&] {
            b.Call(ty.void_(), fn_c);
            b.ExitSwitch(s);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, Switch_3_Cases_AllReturn) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* v = Var("v", 42_i);

        auto s = b.Switch(b.Load(v));
        b.Append(b.Case(s, {b.Constant(0_i)}), [&] { b.Return(fn); });
        b.Append(b.Case(s, {b.Constant(1_i), nullptr}), [&] { b.Return(fn); });
        b.Append(b.Case(s, {b.Constant(2_i)}), [&] { b.Return(fn); });

        b.Call(ty.void_(), fn_a);
        b.Return(fn);
    });

    EXPECT_WGSL(R"(
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
)");
}

TEST_F(IRToProgramTest, Switch_Nested) {
    auto* fn_a = b.Function("a", ty.void_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a); });

    auto* fn_b = b.Function("b", ty.void_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b); });

    auto* fn_c = b.Function("c", ty.void_());
    b.Append(fn_c->Block(), [&] { b.Return(fn_c); });

    auto* fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* v1 = Var("v1", 42_i);

        auto* v2 = Var("v2", 24_i);

        auto s1 = b.Switch(b.Load(v1));
        b.Append(b.Case(s1, {b.Constant(0_i)}), [&] {
            b.Call(ty.void_(), fn_a);
            b.ExitSwitch(s1);
        });
        b.Append(b.Case(s1, {b.Constant(1_i), nullptr}), [&] {
            auto s2 = b.Switch(b.Load(v2));
            b.Append(b.Case(s2, {b.Constant(0_i)}), [&] { b.ExitSwitch(s2); });
            b.Append(b.Case(s2, {b.Constant(1_i), nullptr}), [&] { b.Return(fn); });

            b.ExitSwitch(s1);
        });
        b.Append(b.Case(s1, {b.Constant(2_i)}), [&] {
            b.Call(ty.void_(), fn_c);
            b.ExitSwitch(s1);
        });

        b.Return(fn);
    });
    EXPECT_WGSL(R"(
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
TEST_F(IRToProgramTest, For_Empty) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {
            auto* i = Var("i", 0_i);
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* if_ = b.If(b.LessThan(ty.bool_(), b.Load(i), 5_i));
                b.Append(if_->True(), [&] { b.ExitIf(if_); });
                b.Append(if_->False(), [&] { b.ExitLoop(loop); });
                b.Continue(loop);
            });

            b.Append(loop->Continuing(), [&] {
                b.Store(i, b.Add(ty.i32(), b.Load(i), 1_i));
                b.NextIteration(loop);
            });
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  for(var i : i32 = 0i; (i < 5i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramTest, For_Empty_NoInit) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* i = Var("i", 0_i);

        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* if_ = b.If(b.LessThan(ty.bool_(), b.Load(i), 5_i));
            b.Append(if_->True(), [&] { b.ExitIf(if_); });
            b.Append(if_->False(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });

        b.Append(loop->Continuing(), [&] {
            b.Store(i, b.Add(ty.i32(), b.Load(i), 1_i));
            b.NextIteration(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  var i : i32 = 0i;
  for(; (i < 5i); i = (i + 1i)) {
  }
}
)");
}

TEST_F(IRToProgramTest, For_Empty_NoCont) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {
            auto* i = Var("i", 0_i);
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* if_ = b.If(b.LessThan(ty.bool_(), b.Load(i), 5_i));
                b.Append(if_->True(), [&] { b.ExitIf(if_); });
                b.Append(if_->False(), [&] { b.ExitLoop(loop); });
                b.Continue(loop);
            });
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  for(var i : i32 = 0i; (i < 5i); ) {
  }
}
)");
}

TEST_F(IRToProgramTest, For_ComplexBody) {
    auto* a = b.Function("a", ty.bool_());
    auto* v = b.FunctionParam("v", ty.i32());
    a->SetParams({v});
    b.Append(a->Block(), [&] { b.Return(a, b.Equal(ty.bool_(), v, 1_i)); });

    auto* fn = b.Function("f", ty.i32());

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {
            auto* i = Var("i", 0_i);
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* if1 = b.If(b.LessThan(ty.bool_(), b.Load(i), 5_i));
                b.Append(if1->True(), [&] { b.ExitIf(if1); });
                b.Append(if1->False(), [&] { b.ExitLoop(loop); });

                auto* if2 = b.If(b.Call(ty.bool_(), a, 42_i));
                b.Append(if2->True(), [&] { b.Return(fn, 1_i); });
                b.Append(if2->False(), [&] { b.Return(fn, 2_i); });
                b.Unreachable();
            });

            b.Append(loop->Continuing(), [&] {
                b.Store(i, b.Add(ty.i32(), b.Load(i), 1_i));
                b.NextIteration(loop);
            });
        });

        b.Return(fn, 3_i);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, For_ComplexBody_NoInit) {
    auto* a = b.Function("a", ty.bool_());
    auto* v = b.FunctionParam("v", ty.i32());
    a->SetParams({v});
    b.Append(a->Block(), [&] { b.Return(a, b.Equal(ty.bool_(), v, 1_i)); });

    auto* fn = b.Function("f", ty.i32());

    b.Append(fn->Block(), [&] {
        auto* i = Var("i", 0_i);

        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* if1 = b.If(b.LessThan(ty.bool_(), b.Load(i), 5_i));
            b.Append(if1->True(), [&] { b.ExitIf(if1); });
            b.Append(if1->False(), [&] { b.ExitLoop(loop); });

            auto* if2 = b.If(b.Call(ty.bool_(), a, 42_i));
            b.Append(if2->True(), [&] { b.Return(fn, 1_i); });
            b.Append(if2->False(), [&] { b.Return(fn, 2_i); });

            b.Continue(loop);
        });

        b.Append(loop->Continuing(), [&] {
            b.Store(i, b.Add(ty.i32(), b.Load(i), 1_i));
            b.NextIteration(loop);
        });

        b.Return(fn, 3_i);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, For_ComplexBody_NoCont) {
    auto* a = b.Function("a", ty.bool_());
    auto* v = b.FunctionParam("v", ty.i32());
    a->SetParams({v});
    b.Append(a->Block(), [&] { b.Return(a, b.Equal(ty.bool_(), v, 1_i)); });

    auto* fn = b.Function("f", ty.i32());

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {
            auto* i = Var("i", 0_i);
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* if1 = b.If(b.LessThan(ty.bool_(), b.Load(i), 5_i));
                b.Append(if1->True(), [&] { b.ExitIf(if1); });
                b.Append(if1->False(), [&] { b.ExitLoop(loop); });

                auto* if2 = b.If(b.Call(ty.bool_(), a, 42_i));
                b.Append(if2->True(), [&] { b.Return(fn, 1_i); });
                b.Append(if2->False(), [&] { b.Return(fn, 2_i); });

                b.Continue(loop);
            });
        });

        b.Return(fn, 3_i);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, For_CallInInitCondCont) {
    auto* fn_n = b.Function("n", ty.i32());
    auto* v = b.FunctionParam("v", ty.i32());
    fn_n->SetParams({v});
    b.Append(fn_n->Block(), [&] { b.Return(fn_n, b.Add(ty.i32(), v, 1_i)); });

    auto* fn_f = b.Function("f", ty.void_());

    b.Append(fn_f->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Initializer(), [&] {
            auto* i = Var("i", b.Call(ty.i32(), fn_n, 0_i));
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* load = b.Load(i);
                auto* call = b.Call(ty.i32(), fn_n, 1_i);
                auto* if_ = b.If(b.LessThan(ty.bool_(), load, call));
                b.Append(if_->True(), [&] { b.ExitIf(if_); });
                b.Append(if_->False(), [&] { b.ExitLoop(loop); });

                b.Continue(loop);
            });

            b.Append(loop->Continuing(), [&] {
                b.Store(i, b.Call(ty.i32(), fn_n, b.Load(i)));
                b.NextIteration(loop);
            });
        });

        b.Return(fn_f);
    });

    EXPECT_WGSL(R"(
fn n(v : i32) -> i32 {
  return (v + 1i);
}

fn f() {
  for(var i : i32 = n(0i); (i < n(1i)); i = n(i)) {
  }
}
)");
}

TEST_F(IRToProgramTest, For_IncInInit_Cmp) {
    // %b1 = block {  # root
    //   %i:ptr<storage, u32, read_write> = var @binding_point(0, 0)
    // }
    //
    // %f = func():void -> %b2 {
    //   %b2 = block {
    //     loop [i: %b3, b: %b4] {  # loop_1
    //       %b3 = block {  # initializer
    //         %3:u32 = load %i
    //         %4:u32 = add %3, 1u
    //         store %i, %4
    //         next_iteration %b4
    //       }
    //       %b4 = block {  # body
    //         %5:u32 = load %i
    //         %6:bool = lt %5, 10u
    //         if %6 [t: %b5, f: %b6] {  # if_1
    //           %b5 = block {  # true
    //             exit_if  # if_1
    //           }
    //           %b6 = block {  # false
    //             exit_loop  # loop_1
    //           }
    //         }
    //         continue %b7
    //       }
    //     }
    //     ret
    //   }
    // }

    b.Append(mod.root_block, [&] {
        auto* i = Var<storage, u32, read_write>();
        i->SetBindingPoint(0, 0);

        auto* fn_f = b.Function("f", ty.void_());

        b.Append(fn_f->Block(), [&] {
            auto* loop = b.Loop();

            b.Append(loop->Initializer(), [&] {
                auto* load_i = b.Load(i);
                auto* inc_i = b.Add(ty.u32(), load_i, 1_u);
                b.Store(i, inc_i);
                b.NextIteration(loop);
            });

            b.Append(loop->Body(), [&] {
                auto* load_i = b.Load(i);
                auto* cmp = b.LessThan(ty.bool_(), load_i, 10_u);
                auto* if_ = b.If(cmp);
                b.Append(if_->True(), [&] { b.ExitIf(if_); });
                b.Append(if_->False(), [&] { b.ExitLoop(loop); });
                b.Continue(loop);
            });

            b.Return(fn_f);
        });
    });

    EXPECT_WGSL(R"(
@group(0u) @binding(0u) var<storage, read_write> v : u32;

fn f() {
  for(v = (v + 1u); (v < 10u); ) {
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// While
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, While_Empty) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* cond = b.If(true);
            b.Append(cond->True(), [&] { b.ExitIf(cond); });
            b.Append(cond->False(), [&] { b.ExitLoop(loop); });

            b.Continue(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  while(true) {
  }
}
)");
}

TEST_F(IRToProgramTest, While_Cond) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    fn->SetParams({cond});

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* if_ = b.If(cond);
            b.Append(if_->True(), [&] { b.ExitIf(if_); });
            b.Append(if_->False(), [&] { b.ExitLoop(loop); });

            b.Continue(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(cond : bool) {
  while(cond) {
  }
}
)");
}

TEST_F(IRToProgramTest, While_Break) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* cond = b.If(true);
            b.Append(cond->True(), [&] { b.ExitIf(cond); });
            b.Append(cond->False(), [&] { b.ExitLoop(loop); });

            b.ExitLoop(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  while(true) {
    break;
  }
}
)");
}

TEST_F(IRToProgramTest, While_IfBreak) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    fn->SetParams({cond});

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* if1 = b.If(true);
            b.Append(if1->True(), [&] { b.ExitIf(if1); });
            b.Append(if1->False(), [&] { b.ExitLoop(loop); });

            auto* if2 = b.If(cond);
            b.Append(if2->True(), [&] { b.ExitLoop(loop); });

            b.Continue(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(cond : bool) {
  while(true) {
    if (cond) {
      break;
    }
  }
}
)");
}

TEST_F(IRToProgramTest, While_BreakAfterStatement) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* let = b.Let("cond", true);
            auto* cond = b.If(let);
            b.Append(cond->True(), [&] { b.ExitIf(cond); });
            b.Append(cond->False(), [&] { b.ExitLoop(loop); });

            b.ExitLoop(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  loop {
    let cond = true;
    if (cond) {
    } else {
      break;
    }
    break;
  }
}
)");
}

// Test that only the first "if continue then break" instruction is treated as the loop condition.
// See crbug.com/351700183.
TEST_F(IRToProgramTest, While_IfBreakInFalse) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    fn->SetParams({cond});

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* if1 = b.If(true);
            b.Append(if1->True(), [&] { b.ExitIf(if1); });
            b.Append(if1->False(), [&] { b.ExitLoop(loop); });

            auto* if2 = b.If(cond);
            b.Append(if2->True(), [&] { b.ExitIf(if2); });
            b.Append(if2->False(), [&] { b.ExitLoop(loop); });

            b.Continue(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(cond : bool) {
  while(true) {
    if (cond) {
    } else {
      break;
    }
  }
}
)");
}

TEST_F(IRToProgramTest, While_IfReturn) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    fn->SetParams({cond});

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* if1 = b.If(true);
            b.Append(if1->True(), [&] { b.ExitIf(if1); });
            b.Append(if1->False(), [&] { b.ExitLoop(loop); });

            auto* if2 = b.If(cond);
            b.Append(if2->True(), [&] { b.Return(fn); });

            b.Continue(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(cond : bool) {
  while(true) {
    if (cond) {
      return;
    }
  }
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Loop
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, Loop_Break) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f() {
  loop {
    break;
  }
}
)");
}

TEST_F(IRToProgramTest, Loop_IfBreak) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    fn->SetParams({cond});

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* if_ = b.If(cond);
            b.Append(if_->True(), [&] { b.ExitLoop(loop); });
            b.Continue(loop);
        });

        b.Return(fn);
    });
    EXPECT_WGSL(R"(
fn f(cond : bool) {
  loop {
    if (cond) {
      break;
    }
  }
}
)");
}

TEST_F(IRToProgramTest, Loop_IfReturn) {
    auto* fn = b.Function("f", ty.void_());
    auto* cond = b.FunctionParam("cond", ty.bool_());
    fn->SetParams({cond});

    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* if_ = b.If(cond);
            b.Append(if_->True(), [&] { b.Return(fn); });
            b.Continue(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
fn f(cond : bool) {
  loop {
    if (cond) {
      return;
    }
  }
}
)");
}

TEST_F(IRToProgramTest, Loop_IfContinuing) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* cond = Var("cond", false);

        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* if_ = b.If(b.Load(cond));
            b.Append(if_->True(), [&] { b.Return(fn); });
            b.Continue(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Store(cond, true);
            b.NextIteration(loop);
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, Loop_VarsDeclaredOutsideAndInside) {
    auto* fn = b.Function("f", ty.void_());

    b.Append(fn->Block(), [&] {
        auto* var_b = Var("b", 1_i);

        auto* loop = b.Loop();

        b.Append(loop->Body(), [&] {
            auto* var_a = Var("a", 2_i);

            auto* body_load_a = b.Load(var_a);
            auto* body_load_b = b.Load(var_b);
            auto* if_ = b.If(b.Equal(ty.bool_(), body_load_a, body_load_b));
            b.Append(if_->True(), [&] { b.Return(fn); });
            b.Append(if_->False(), [&] { b.ExitIf(if_); });
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {
                auto* cont_load_a = b.Load(var_a);
                auto* cont_load_b = b.Load(var_b);
                b.Store(var_b, b.Add(ty.i32(), cont_load_a, cont_load_b));
                b.NextIteration(loop);
            });
        });

        b.Return(fn);
    });

    EXPECT_WGSL(R"(
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

TEST_F(IRToProgramTest, Enable_ChromiumExperimentalFramebufferFetch_StructColor) {
    core::type::Manager::StructMemberDesc member;
    member.name = mod.symbols.New("a");
    member.type = ty.f32();
    member.attributes.color = 2u;

    auto* S = ty.Struct(mod.symbols.New("S"), {member});

    auto* fn = b.Function("f", ty.void_());
    fn->SetParams({b.FunctionParam(S)});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    EXPECT_WGSL(R"(
enable chromium_experimental_framebuffer_fetch;

struct S {
  @color(2u)
  a : f32,
}

fn f(v : S) {
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// chromium_internal_graphite
////////////////////////////////////////////////////////////////////////////////
TEST_F(IRToProgramTest, Enable_ChromiumInternalGraphite_SubgroupBallot) {
    b.Append(b.ir.root_block, [&] {
        auto t = b.Var("T", ty.ref<core::AddressSpace::kHandle>(ty.Get<core::type::StorageTexture>(
                                core::type::TextureDimension::k2d, core::TexelFormat::kR8Unorm,
                                core::Access::kRead, ty.f32())));
        t->SetBindingPoint(0, 0);
    });

    EXPECT_WGSL(R"(
enable chromium_internal_graphite;

@group(0u) @binding(0u) var T : texture_storage_2d<r8unorm, read>;
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
