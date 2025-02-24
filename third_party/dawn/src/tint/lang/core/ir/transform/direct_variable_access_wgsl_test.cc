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

#include "src/tint/lang/core/ir/transform/direct_variable_access.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace {

static constexpr DirectVariableAccessOptions kTransformPrivate = {
    /* transform_private */ true,
    /* transform_function */ false,
};

static constexpr DirectVariableAccessOptions kTransformFunction = {
    /* transform_private */ false,
    /* transform_function */ true,
};

class DirectVariableAccessTest : public TransformTestBase<testing::Test> {
  public:
    std::string Run(std::string in,
                    const DirectVariableAccessOptions& transform_options = {},
                    const wgsl::writer::ProgramOptions program_options = {}) {
        wgsl::reader::Options parser_options;
        parser_options.allowed_features = wgsl::AllowedFeatures::Everything();
        Source::File file{"test", in};
        auto program_in = wgsl::reader::Parse(&file, parser_options);
        if (!program_in.IsValid()) {
            return "wgsl::reader::Parse() failed: \n" + program_in.Diagnostics().Str();
        }

        auto module = wgsl::reader::ProgramToIR(program_in);
        if (module != Success) {
            return "ProgramToIR() failed:\n" + module.Failure().reason.Str();
        }

        auto res = DirectVariableAccess(module.Get(), transform_options);
        if (res != Success) {
            return "DirectVariableAccess failed:\n" + res.Failure().reason.Str();
        }

        auto pre_raise = ir::Disassembler(module.Get()).Plain();

        if (auto raise = wgsl::writer::Raise(module.Get()); raise != Success) {
            return "wgsl::writer::Raise failed:\n" + res.Failure().reason.Str();
        }

        auto program_out = wgsl::writer::IRToProgram(module.Get(), program_options);
        if (!program_out.IsValid()) {
            return "wgsl::writer::IRToProgram() failed: \n" + program_out.Diagnostics().Str() +
                   "\n\nIR (pre):\n" + pre_raise +                                //
                   "\n\nIR (post):\n" + ir::Disassembler(module.Get()).Plain() +  //
                   "\n\nAST:\n" + Program::printer(program_out);
        }

        auto output = wgsl::writer::Generate(program_out, wgsl::writer::Options{});
        if (output != Success) {
            return "wgsl::writer::IRToProgram() failed: \n" + output.Failure().reason.Str() +
                   "\n\nIR:\n" + ir::Disassembler(module.Get()).Plain();
        }

        return "\n" + output->wgsl;
    }
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// remove uncalled
////////////////////////////////////////////////////////////////////////////////
namespace remove_uncalled {

using IR_DirectVariableAccessWgslTest_RemoveUncalled = DirectVariableAccessTest;

TEST_F(IR_DirectVariableAccessWgslTest_RemoveUncalled, PtrUniform) {
    auto* src = R"(
var<private> keep_me : i32 = 42i;

fn u(pre : i32, p : ptr<uniform, i32>, post : i32) -> i32 {
  return *(p);
}

)";

    auto* expect = R"(
var<private> keep_me : i32 = 42i;
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_RemoveUncalled, PtrStorage) {
    auto* src = R"(
var<private> keep_me : i32 = 42i;

fn s(pre : i32, p : ptr<storage, i32>, post : i32) -> i32 {
  return *(p);
}
)";

    auto* expect = R"(
var<private> keep_me : i32 = 42i;
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_RemoveUncalled, PtrWorkgroup) {
    auto* src = R"(
var<private> keep_me : i32 = 42i;

fn w(pre : i32, p : ptr<workgroup, i32>, post : i32) -> i32 {
  return *(p);
}

)";

    auto* expect = R"(
var<private> keep_me : i32 = 42i;
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_RemoveUncalled, PtrPrivate_Disabled) {
    auto* src = R"(
var<private> keep_me : i32 = 42i;

fn f(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}
)";

    auto* expect = src;

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_RemoveUncalled, PtrPrivate_Enabled) {
    auto* src = R"(
var<private> keep_me : i32 = 42i;

fn f(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}
)";

    auto* expect = R"(
var<private> keep_me : i32 = 42i;
)";

    auto got = Run(src, kTransformPrivate);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_RemoveUncalled, PtrFunction_Disabled) {
    auto* src = R"(
var<private> keep_me : i32 = 42i;

fn f(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *(p);
}
)";

    auto* expect = src;

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_RemoveUncalled, PtrFunction_Enabled) {
    auto* src = R"(
var<private> keep_me : i32 = 42i;

fn f(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *(p);
}
)";

    auto* expect = R"(
var<private> keep_me : i32 = 42i;
)";

    auto got = Run(src, kTransformFunction);

    EXPECT_EQ(expect, got);
}

}  // namespace remove_uncalled

////////////////////////////////////////////////////////////////////////////////
// pointer chains
////////////////////////////////////////////////////////////////////////////////
namespace pointer_chains_tests {

using IR_DirectVariableAccessWgslTest_PtrChains = DirectVariableAccessTest;

TEST_F(IR_DirectVariableAccessWgslTest_PtrChains, ConstantIndices) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 8>, 8>, 8>;

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p0 = &U;
  let p1 = &(*p0)[1];
  let p2 = &(*p1)[1+1];
  let p3 = &(*p2)[2*2 - 1];
  a(10, p3, 20);
}

fn c(p : ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>>) {
  let p0 = p;
  let p1 = &(*p0)[1];
  let p2 = &(*p1)[1+1];
  let p3 = &(*p2)[2*2 - 1];
  a(10, p3, 20);
}

fn d() {
  c(&U);
}
)";

    auto* expect =
        R"(
@group(0u) @binding(0u) var<uniform> U : array<array<array<vec4<i32>, 8u>, 8u>, 8u>;

fn a(pre : i32, p_indices : array<u32, 3u>, post : i32) -> vec4<i32> {
  return U[p_indices[0u]][p_indices[1u]][p_indices[2u]];
}

fn b() {
  a(10i, array<u32, 3u>(u32(1i), u32(2i), u32(3i)), 20i);
}

fn c() {
  a(10i, array<u32, 3u>(u32(1i), u32(2i), u32(3i)), 20i);
}

fn d() {
  c();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PtrChains, DynamicIndices) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 8>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}
fn third() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p0 = &U;
  let p1 = &(*p0)[first()];
  let p2 = &(*p1)[second()][third()];
  a(10, p2, 20);
}

fn c(p : ptr<uniform, array<array<array<vec4<i32>, 8>, 8>, 8>>) {
  let p0 = p;
  let p1 = &(*p0)[first()];
  let p2 = &(*p1)[second()][third()];
  a(10, p2, 20);
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<uniform> U : array<array<array<vec4<i32>, 8u>, 8u>, 8u>;

var<private> i : i32;

fn first() -> i32 {
  i = (i + 1i);
  return i;
}

fn second() -> i32 {
  i = (i + 1i);
  return i;
}

fn third() -> i32 {
  i = (i + 1i);
  return i;
}

fn a(pre : i32, p_indices : array<u32, 3u>, post : i32) -> vec4<i32> {
  return U[p_indices[0u]][p_indices[1u]][p_indices[2u]];
}

fn b() {
  let v = first();
  let v_1 = second();
  a(10i, array<u32, 3u>(u32(v), u32(v_1), u32(third())), 20i);
}

fn c() {
  let v_2 = first();
  let v_3 = second();
  a(10i, array<u32, 3u>(u32(v_2), u32(v_3), u32(third())), 20i);
}

fn d() {
  c();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PtrChains, DynamicIndicesForLoopInit) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  for (let p1 = &U[first()]; true; ) {
    a(10, &(*p1)[second()], 20);
  }
}

fn c(p : ptr<uniform, array<array<vec4<i32>, 8>, 8>>) {
  for (let p1 = &(*p)[first()]; true; ) {
    a(10, &(*p1)[second()], 20);
  }
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<uniform> U : array<array<vec4<i32>, 8u>, 8u>;

var<private> i : i32;

fn first() -> i32 {
  i = (i + 1i);
  return i;
}

fn second() -> i32 {
  i = (i + 1i);
  return i;
}

fn a(pre : i32, p_indices : array<u32, 2u>, post : i32) -> vec4<i32> {
  return U[p_indices[0u]][p_indices[1u]];
}

fn b() {
  for(let v = first(); true; ) {
    a(10i, array<u32, 2u>(u32(v), u32(second())), 20i);
  }
}

fn c() {
  for(let v_1 = first(); true; ) {
    a(10i, array<u32, 2u>(u32(v_1), u32(second())), 20i);
  }
}

fn d() {
  c();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PtrChains, DynamicIndicesForLoopCond) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p = &U[first()][second()];
  for (; a(10, p, 20).x < 4; ) {
    let body = 1;
  }
}

fn c(p : ptr<uniform, array<array<vec4<i32>, 8>, 8>>) {
  let p2 = &(*p)[first()][second()];
  for (; a(10, p2, 20).x < 4; ) {
    let body = 1;
  }
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<uniform> U : array<array<vec4<i32>, 8u>, 8u>;

var<private> i : i32;

fn first() -> i32 {
  i = (i + 1i);
  return i;
}

fn second() -> i32 {
  i = (i + 1i);
  return i;
}

fn a(pre : i32, p_indices : array<u32, 2u>, post : i32) -> vec4<i32> {
  return U[p_indices[0u]][p_indices[1u]];
}

fn b() {
  let v = first();
  let v_1 = second();
  while((a(10i, array<u32, 2u>(u32(v), u32(v_1)), 20i).x < 4i)) {
    let body = 1i;
  }
}

fn c() {
  let v_2 = first();
  let v_3 = second();
  while((a(10i, array<u32, 2u>(u32(v_2), u32(v_3)), 20i).x < 4i)) {
    let body = 1i;
  }
}

fn d() {
  c();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PtrChains, DynamicIndicesForLoopCont) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p = &U[first()][second()];
  for (var i = 0; i < 3; a(10, p, 20)) {
    i++;
  }
}

fn c(p : ptr<uniform, array<array<vec4<i32>, 8>, 8>>) {
  let p2 = &(*p)[first()][second()];
  for (var i = 0; i < 3; a(10, p2, 20)) {
    i++;
  }
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<uniform> U : array<array<vec4<i32>, 8u>, 8u>;

var<private> i : i32;

fn first() -> i32 {
  i = (i + 1i);
  return i;
}

fn second() -> i32 {
  i = (i + 1i);
  return i;
}

fn a(pre : i32, p_indices : array<u32, 2u>, post : i32) -> vec4<i32> {
  return U[p_indices[0u]][p_indices[1u]];
}

fn b() {
  let v = first();
  let v_1 = second();
  for(var i_1 : i32 = 0i; (i_1 < 3i); a(10i, array<u32, 2u>(u32(v), u32(v_1)), 20i)) {
    i_1 = (i_1 + 1i);
  }
}

fn c() {
  let v_2 = first();
  let v_3 = second();
  for(var i_2 : i32 = 0i; (i_2 < 3i); a(10i, array<u32, 2u>(u32(v_2), u32(v_3)), 20i)) {
    i_2 = (i_2 + 1i);
  }
}

fn d() {
  c();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PtrChains, DynamicIndicesWhileCond) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<array<vec4<i32>, 8>, 8>;

var<private> i : i32;
fn first() -> i32 {
  i++;
  return i;
}
fn second() -> i32 {
  i++;
  return i;
}

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let p = &U[first()][second()];
  while (a(10, p, 20).x < 4) {
    let body = 1;
  }
}

fn c(p : ptr<uniform, array<array<vec4<i32>, 8>, 8>>) {
  let p2 = &(*p)[first()][second()];
  while (a(10, p2, 20).x < 4) {
    let body = 1;
  }
}

fn d() {
  c(&U);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<uniform> U : array<array<vec4<i32>, 8u>, 8u>;

var<private> i : i32;

fn first() -> i32 {
  i = (i + 1i);
  return i;
}

fn second() -> i32 {
  i = (i + 1i);
  return i;
}

fn a(pre : i32, p_indices : array<u32, 2u>, post : i32) -> vec4<i32> {
  return U[p_indices[0u]][p_indices[1u]];
}

fn b() {
  let v = first();
  let v_1 = second();
  while((a(10i, array<u32, 2u>(u32(v), u32(v_1)), 20i).x < 4i)) {
    let body = 1i;
  }
}

fn c() {
  let v_2 = first();
  let v_3 = second();
  while((a(10i, array<u32, 2u>(u32(v_2), u32(v_3)), 20i).x < 4i)) {
    let body = 1i;
  }
}

fn d() {
  c();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}
}  // namespace pointer_chains_tests

////////////////////////////////////////////////////////////////////////////////
// 'uniform' address space
////////////////////////////////////////////////////////////////////////////////
namespace uniform_as_tests {

using IR_DirectVariableAccessWgslTest_UniformAS = DirectVariableAccessTest;

TEST_F(IR_DirectVariableAccessWgslTest_UniformAS, Param_ptr_i32_read) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : i32;

fn a(pre : i32, p : ptr<uniform, i32>, post : i32) -> i32 {
  return *p;
}

fn b() {
  a(10, &U, 20);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<uniform> U : i32;

fn a(pre : i32, post : i32) -> i32 {
  return U;
}

fn b() {
  a(10i, 20i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_UniformAS, Param_ptr_vec4i32_Via_array_DynamicRead) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> U : array<vec4<i32>, 8>;

fn a(pre : i32, p : ptr<uniform, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  let I = 3;
  a(10, &U[I], 20);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<uniform> U : array<vec4<i32>, 8u>;

fn a(pre : i32, p_indices : array<u32, 1u>, post : i32) -> vec4<i32> {
  return U[p_indices[0u]];
}

fn b() {
  let I = 3i;
  a(10i, array<u32, 1u>(u32(I)), 20i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_UniformAS, CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

@group(0) @binding(0) var<uniform> U : Outer;

fn f0(p : ptr<uniform, vec4<f32>>) -> f32 {
  return (*p).x;
}

fn f1(p : ptr<uniform, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&(*p)[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &(*p)[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of U
    res += f0(&U.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of U
    let p_vec = &U.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<uniform, Inner>) -> f32 {
  let p_mat = &(*p).mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<uniform, InnerArr>, p1 : ptr<uniform, mat3x4<f32>>) -> f32 {
  let p0_inner = &(*p0)[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<uniform, Outer>) -> f32 {
  return f3(&(*p).arr, &U.mat);
}

fn b() {
  f4(&U);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

struct Outer {
  arr : array<Inner, 4u>,
  mat : mat3x4<f32>,
}

@group(0u) @binding(0u) var<uniform> U : Outer;

fn f0(p_indices : array<u32, 1u>) -> f32 {
  return U.mat[p_indices[0u]].x;
}

fn f0_1(p_indices : array<u32, 2u>) -> f32 {
  return U.arr[p_indices[0u]].mat[p_indices[1u]].x;
}

fn f1() -> f32 {
  var res : f32;
  let v = f0(array<u32, 1u>(u32(1i)));
  res = (res + v);
  let v_1 = f0(array<u32, 1u>(u32(1i)));
  res = (res + v_1);
  let v_2 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_2);
  let v_3 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_3);
  return res;
}

fn f1_1(p_indices : array<u32, 1u>) -> f32 {
  let v_4 = p_indices[0u];
  var res : f32;
  let v_5 = f0_1(array<u32, 2u>(v_4, u32(1i)));
  res = (res + v_5);
  let v_6 = f0_1(array<u32, 2u>(v_4, u32(1i)));
  res = (res + v_6);
  let v_7 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_7);
  let v_8 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_8);
  return res;
}

fn f2(p_indices : array<u32, 1u>) -> f32 {
  return f1_1(array<u32, 1u>(p_indices[0u]));
}

fn f3() -> f32 {
  return (f2(array<u32, 1u>(u32(3i))) + f1());
}

fn f4() -> f32 {
  return f3();
}

fn b() {
  f4();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

}  // namespace uniform_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'storage' address space
////////////////////////////////////////////////////////////////////////////////
namespace storage_as_tests {

using IR_DirectVariableAccessWgslTest_StorageAS = DirectVariableAccessTest;

TEST_F(IR_DirectVariableAccessWgslTest_StorageAS, Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
struct str {
  i : i32,
};

@group(0) @binding(0) var<storage> S : str;

fn a(pre : i32, p : ptr<storage, i32>, post : i32) -> i32 {
  return *p;
}

fn b() {
  a(10, &S.i, 20);
}
)";

    auto* expect = R"(
struct str {
  i : i32,
}

@group(0u) @binding(0u) var<storage, read> S : str;

fn a(pre : i32, post : i32) -> i32 {
  return S.i;
}

fn b() {
  a(10i, 20i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_StorageAS, Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
struct str {
  arr : array<i32, 4>,
};

@group(0) @binding(0) var<storage, read_write> S : str;

fn a(pre : i32, p : ptr<storage, array<i32, 4>, read_write>, post : i32) {
  *p = array<i32, 4>();
}

fn b() {
  a(10, &S.arr, 20);
}
)";

    auto* expect = R"(
struct str {
  arr : array<i32, 4u>,
}

@group(0u) @binding(0u) var<storage, read_write> S : str;

fn a(pre : i32, post : i32) {
  S.arr = array<i32, 4u>();
}

fn b() {
  a(10i, 20i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_StorageAS, Param_ptr_vec4i32_Via_array_DynamicWrite) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read_write> S : array<vec4<i32>, 8>;

fn a(pre : i32, p : ptr<storage, vec4<i32>, read_write>, post : i32) {
  *p = vec4<i32>();
}

fn b() {
  let I = 3;
  a(10, &S[I], 20);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<storage, read_write> S : array<vec4<i32>, 8u>;

fn a(pre : i32, p_indices : array<u32, 1u>, post : i32) {
  S[p_indices[0u]] = vec4<i32>();
}

fn b() {
  let I = 3i;
  a(10i, array<u32, 1u>(u32(I)), 20i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_StorageAS, CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

@group(0) @binding(0) var<storage> S : Outer;

fn f0(p : ptr<storage, vec4<f32>>) -> f32 {
  return (*p).x;
}

fn f1(p : ptr<storage, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&(*p)[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &(*p)[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of S
    res += f0(&S.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of S
    let p_vec = &S.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<storage, Inner>) -> f32 {
  let p_mat = &(*p).mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<storage, InnerArr>, p1 : ptr<storage, mat3x4<f32>>) -> f32 {
  let p0_inner = &(*p0)[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<storage, Outer>) -> f32 {
  return f3(&(*p).arr, &S.mat);
}

fn b() {
  f4(&S);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

struct Outer {
  arr : array<Inner, 4u>,
  mat : mat3x4<f32>,
}

@group(0u) @binding(0u) var<storage, read> S : Outer;

fn f0(p_indices : array<u32, 1u>) -> f32 {
  return S.mat[p_indices[0u]].x;
}

fn f0_1(p_indices : array<u32, 2u>) -> f32 {
  return S.arr[p_indices[0u]].mat[p_indices[1u]].x;
}

fn f1() -> f32 {
  var res : f32;
  let v = f0(array<u32, 1u>(u32(1i)));
  res = (res + v);
  let v_1 = f0(array<u32, 1u>(u32(1i)));
  res = (res + v_1);
  let v_2 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_2);
  let v_3 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_3);
  return res;
}

fn f1_1(p_indices : array<u32, 1u>) -> f32 {
  let v_4 = p_indices[0u];
  var res : f32;
  let v_5 = f0_1(array<u32, 2u>(v_4, u32(1i)));
  res = (res + v_5);
  let v_6 = f0_1(array<u32, 2u>(v_4, u32(1i)));
  res = (res + v_6);
  let v_7 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_7);
  let v_8 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_8);
  return res;
}

fn f2(p_indices : array<u32, 1u>) -> f32 {
  return f1_1(array<u32, 1u>(p_indices[0u]));
}

fn f3() -> f32 {
  return (f2(array<u32, 1u>(u32(3i))) + f1());
}

fn f4() -> f32 {
  return f3();
}

fn b() {
  f4();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

}  // namespace storage_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'workgroup' address space
////////////////////////////////////////////////////////////////////////////////
namespace workgroup_as_tests {

using IR_DirectVariableAccessWgslTest_WorkgroupAS = DirectVariableAccessTest;

TEST_F(IR_DirectVariableAccessWgslTest_WorkgroupAS, Param_ptr_vec4i32_Via_array_StaticRead) {
    auto* src = R"(
var<workgroup> W : array<vec4<i32>, 8>;

fn a(pre : i32, p : ptr<workgroup, vec4<i32>>, post : i32) -> vec4<i32> {
  return *p;
}

fn b() {
  a(10, &W[3], 20);
}
)";

    auto* expect = R"(
var<workgroup> W : array<vec4<i32>, 8u>;

fn a(pre : i32, p_indices : array<u32, 1u>, post : i32) -> vec4<i32> {
  return W[p_indices[0u]];
}

fn b() {
  a(10i, array<u32, 1u>(u32(3i)), 20i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_WorkgroupAS, Param_ptr_vec4i32_Via_array_StaticWrite) {
    auto* src = R"(
var<workgroup> W : array<vec4<i32>, 8>;

fn a(pre : i32, p : ptr<workgroup, vec4<i32>>, post : i32) {
  *p = vec4<i32>();
}

fn b() {
  a(10, &W[3], 20);
}
)";

    auto* expect = R"(
var<workgroup> W : array<vec4<i32>, 8u>;

fn a(pre : i32, p_indices : array<u32, 1u>, post : i32) {
  W[p_indices[0u]] = vec4<i32>();
}

fn b() {
  a(10i, array<u32, 1u>(u32(3i)), 20i);
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_WorkgroupAS, CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

var<workgroup> W : Outer;

fn f0(p : ptr<workgroup, vec4<f32>>) -> f32 {
  return (*p).x;
}

fn f1(p : ptr<workgroup, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&(*p)[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &(*p)[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of W
    res += f0(&W.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of W
    let p_vec = &W.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<workgroup, Inner>) -> f32 {
  let p_mat = &(*p).mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<workgroup, InnerArr>, p1 : ptr<workgroup, mat3x4<f32>>) -> f32 {
  let p0_inner = &(*p0)[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<workgroup, Outer>) -> f32 {
  return f3(&(*p).arr, &W.mat);
}

fn b() {
  f4(&W);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

struct Outer {
  arr : array<Inner, 4u>,
  mat : mat3x4<f32>,
}

var<workgroup> W : Outer;

fn f0(p_indices : array<u32, 1u>) -> f32 {
  return W.mat[p_indices[0u]].x;
}

fn f0_1(p_indices : array<u32, 2u>) -> f32 {
  return W.arr[p_indices[0u]].mat[p_indices[1u]].x;
}

fn f1() -> f32 {
  var res : f32;
  let v = f0(array<u32, 1u>(u32(1i)));
  res = (res + v);
  let v_1 = f0(array<u32, 1u>(u32(1i)));
  res = (res + v_1);
  let v_2 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_2);
  let v_3 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_3);
  return res;
}

fn f1_1(p_indices : array<u32, 1u>) -> f32 {
  let v_4 = p_indices[0u];
  var res : f32;
  let v_5 = f0_1(array<u32, 2u>(v_4, u32(1i)));
  res = (res + v_5);
  let v_6 = f0_1(array<u32, 2u>(v_4, u32(1i)));
  res = (res + v_6);
  let v_7 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_7);
  let v_8 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_8);
  return res;
}

fn f2(p_indices : array<u32, 1u>) -> f32 {
  return f1_1(array<u32, 1u>(p_indices[0u]));
}

fn f3() -> f32 {
  return (f2(array<u32, 1u>(u32(3i))) + f1());
}

fn f4() -> f32 {
  return f3();
}

fn b() {
  f4();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

}  // namespace workgroup_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'private' address space
////////////////////////////////////////////////////////////////////////////////
namespace private_as_tests {

using IR_DirectVariableAccessWgslTest_PrivateAS = DirectVariableAccessTest;

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Enabled_Param_ptr_i32_read) {
    auto* src = R"(
fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}

var<private> P : i32;

fn b() {
  a(10, &(P), 20);
}
)";

    auto* expect = R"(
var<private> P : i32;

fn a(pre : i32, post : i32) -> i32 {
  return P;
}

fn b() {
  a(10i, 20i);
}
)";

    auto got = Run(src, kTransformPrivate);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Enabled_Param_ptr_i32_write) {
    auto* src = R"(
fn a(pre : i32, p : ptr<private, i32>, post : i32) {
  *(p) = 42;
}

var<private> P : i32;

fn b() {
  a(10, &(P), 20);
}
)";

    auto* expect = R"(
var<private> P : i32;

fn a(pre : i32, post : i32) {
  P = 42i;
}

fn b() {
  a(10i, 20i);
}
)";

    auto got = Run(src, kTransformPrivate);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Enabled_Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
struct str {
  i : i32,
};

fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *p;
}

var<private> P : str;

fn b() {
  a(10, &P.i, 20);
}
)";

    auto* expect = R"(
struct str {
  i : i32,
}

var<private> P : str;

fn a(pre : i32, post : i32) -> i32 {
  return P.i;
}

fn b() {
  a(10i, 20i);
}
)";

    auto got = Run(src, kTransformPrivate);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Disabled_Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
struct str {
  i : i32,
}

var<private> P : str;

fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}

fn b() {
  a(10i, &(P.i), 20i);
}
)";

    auto* expect = src;

    wgsl::writer::ProgramOptions program_options;
    program_options.allowed_features.features.emplace(
        wgsl::LanguageFeature::kUnrestrictedPointerParameters);
    auto got = Run(src, /* transform_options */ {}, program_options);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Enabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
struct str {
  arr : array<i32, 4>,
};

fn a(pre : i32, p : ptr<private, array<i32, 4>>, post : i32) {
  *p = array<i32, 4>();
}

var<private> P : str;

fn b() {
  a(10, &P.arr, 20);
}
)";

    auto* expect = R"(
struct str {
  arr : array<i32, 4u>,
}

var<private> P : str;

fn a(pre : i32, post : i32) {
  P.arr = array<i32, 4u>();
}

fn b() {
  a(10i, 20i);
}
)";

    auto got = Run(src, kTransformPrivate);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Disabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
struct str {
  arr : array<i32, 4u>,
}

var<private> P : str;

fn a(pre : i32, p : ptr<private, array<i32, 4u>>, post : i32) {
  *(p) = array<i32, 4u>();
}

fn b() {
  a(10i, &(P.arr), 20i);
}
)";

    auto* expect = src;

    wgsl::writer::ProgramOptions program_options;
    program_options.allowed_features.features.emplace(
        wgsl::LanguageFeature::kUnrestrictedPointerParameters);
    auto got = Run(src, /* transform_options */ {}, program_options);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Enabled_Param_ptr_i32_mixed) {
    auto* src = R"(
struct str {
  i : i32,
};

fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *p;
}

var<private> Pi : i32;
var<private> Ps : str;
var<private> Pa : array<i32, 4>;

fn b() {
  a(10, &Pi, 20);
  a(30, &Ps.i, 40);
  a(50, &Pa[2], 60);
}
)";

    auto* expect = R"(
var<private> Pi : i32;

struct str {
  i : i32,
}

var<private> Ps : str;

var<private> Pa : array<i32, 4u>;

fn a(pre : i32, post : i32) -> i32 {
  return Pi;
}

fn a_1(pre : i32, post : i32) -> i32 {
  return Ps.i;
}

fn a_2(pre : i32, p_indices : array<u32, 1u>, post : i32) -> i32 {
  return Pa[p_indices[0u]];
}

fn b() {
  a(10i, 20i);
  a_1(30i, 40i);
  a_2(50i, array<u32, 1u>(u32(2i)), 60i);
}
)";

    auto got = Run(src, kTransformPrivate);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Disabled_Param_ptr_i32_mixed) {
    auto* src = R"(
var<private> Pi : i32;

struct str {
  i : i32,
}

var<private> Ps : str;

var<private> Pa : array<i32, 4u>;

fn a(pre : i32, p : ptr<private, i32>, post : i32) -> i32 {
  return *(p);
}

fn b() {
  a(10i, &(Pi), 20i);
  a(30i, &(Ps.i), 40i);
  a(50i, &(Pa[2i]), 60i);
}
)";

    auto* expect = src;

    wgsl::writer::ProgramOptions program_options;
    program_options.allowed_features.features.emplace(
        wgsl::LanguageFeature::kUnrestrictedPointerParameters);
    auto got = Run(src, /* transform_options */ {}, program_options);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Enabled_CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
};

alias InnerArr = array<Inner, 4>;

struct Outer {
  arr : InnerArr,
  mat : mat3x4<f32>,
};

var<private> P : Outer;

fn f0(p : ptr<private, vec4<f32>>) -> f32 {
  return (*p).x;
}

fn f1(p : ptr<private, mat3x4<f32>>) -> f32 {
  var res : f32;
  {
    // call f0() with inline usage of p
    res += f0(&(*p)[1]);
  }
  {
    // call f0() with pointer-let usage of p
    let p_vec = &(*p)[1];
    res += f0(p_vec);
  }
  {
    // call f0() with inline usage of P
    res += f0(&P.arr[2].mat[1]);
  }
  {
    // call f0() with pointer-let usage of P
    let p_vec = &P.arr[2].mat[1];
    res += f0(p_vec);
  }
  return res;
}

fn f2(p : ptr<private, Inner>) -> f32 {
  let p_mat = &(*p).mat;
  return f1(p_mat);
}

fn f3(p0 : ptr<private, InnerArr>, p1 : ptr<private, mat3x4<f32>>) -> f32 {
  let p0_inner = &(*p0)[3];
  return f2(p0_inner) + f1(p1);
}

fn f4(p : ptr<private, Outer>) -> f32 {
  return f3(&(*p).arr, &P.mat);
}

fn b() {
  f4(&P);
}
)";

    auto* expect = R"(
struct Inner {
  mat : mat3x4<f32>,
}

struct Outer {
  arr : array<Inner, 4u>,
  mat : mat3x4<f32>,
}

var<private> P : Outer;

fn f0(p_indices : array<u32, 1u>) -> f32 {
  return P.mat[p_indices[0u]].x;
}

fn f0_1(p_indices : array<u32, 2u>) -> f32 {
  return P.arr[p_indices[0u]].mat[p_indices[1u]].x;
}

fn f1() -> f32 {
  var res : f32;
  let v = f0(array<u32, 1u>(u32(1i)));
  res = (res + v);
  let v_1 = f0(array<u32, 1u>(u32(1i)));
  res = (res + v_1);
  let v_2 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_2);
  let v_3 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_3);
  return res;
}

fn f1_1(p_indices : array<u32, 1u>) -> f32 {
  let v_4 = p_indices[0u];
  var res : f32;
  let v_5 = f0_1(array<u32, 2u>(v_4, u32(1i)));
  res = (res + v_5);
  let v_6 = f0_1(array<u32, 2u>(v_4, u32(1i)));
  res = (res + v_6);
  let v_7 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_7);
  let v_8 = f0_1(array<u32, 2u>(u32(2i), u32(1i)));
  res = (res + v_8);
  return res;
}

fn f2(p_indices : array<u32, 1u>) -> f32 {
  return f1_1(array<u32, 1u>(p_indices[0u]));
}

fn f3() -> f32 {
  return (f2(array<u32, 1u>(u32(3i))) + f1());
}

fn f4() -> f32 {
  return f3();
}

fn b() {
  f4();
}
)";

    auto got = Run(src, kTransformPrivate);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_PrivateAS, Disabled_CallChaining) {
    auto* src = R"(
struct Inner {
  mat : mat3x4<f32>,
}

struct Outer {
  arr : array<Inner, 4u>,
  mat : mat3x4<f32>,
}

var<private> P : Outer;

fn f0(p : ptr<private, vec4<f32>>) -> f32 {
  return (*(p)).x;
}

fn f1(p : ptr<private, mat3x4<f32>>) -> f32 {
  var res : f32;
  let v = f0(&((*(p))[1i]));
  res = (res + v);
  let p_vec = &((*(p))[1i]);
  let v_1 = f0(p_vec);
  res = (res + v_1);
  let v_2 = f0(&(P.arr[2i].mat[1i]));
  res = (res + v_2);
  let p_vec_1 = &(P.arr[2i].mat[1i]);
  let v_3 = f0(p_vec_1);
  res = (res + v_3);
  return res;
}

fn f2(p : ptr<private, Inner>) -> f32 {
  let p_mat = &((*(p)).mat);
  return f1(p_mat);
}

fn f3(p0 : ptr<private, array<Inner, 4u>>, p1 : ptr<private, mat3x4<f32>>) -> f32 {
  let p0_inner = &((*(p0))[3i]);
  return (f2(p0_inner) + f1(p1));
}

fn f4(p : ptr<private, Outer>) -> f32 {
  return f3(&((*(p)).arr), &(P.mat));
}

fn b() {
  f4(&(P));
}
)";

    auto* expect = src;

    wgsl::writer::ProgramOptions program_options;
    program_options.allowed_features.features.emplace(
        wgsl::LanguageFeature::kUnrestrictedPointerParameters);
    auto got = Run(src, /* transform_options */ {}, program_options);

    EXPECT_EQ(expect, got);
}

}  // namespace private_as_tests

////////////////////////////////////////////////////////////////////////////////
// 'function' address space
////////////////////////////////////////////////////////////////////////////////
namespace function_as_tests {

using IR_DirectVariableAccessWgslTest_FunctionAS = DirectVariableAccessTest;

TEST_F(IR_DirectVariableAccessWgslTest_FunctionAS, Enabled_LocalPtr) {
    auto* src = R"(
fn f() {
  var v : i32;
  let p = &(v);
  var x : i32 = *(p);
}
)";

    auto* expect = src;  // Nothing changes

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_FunctionAS, Enabled_Param_ptr_i32_read) {
    auto* src = R"(
fn a(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *(p);
}

fn b() {
  var F : i32;
  a(10, &(F), 20);
}
)";

    auto* expect = R"(
fn a(pre : i32, p_root : ptr<function, i32>, post : i32) -> i32 {
  return *(p_root);
}

fn b() {
  var F : i32;
  a(10i, &(F), 20i);
}
)";

    auto got = Run(src, kTransformFunction);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_FunctionAS, Enabled_Param_ptr_i32_write) {
    auto* src = R"(
fn a(pre : i32, p : ptr<function, i32>, post : i32) {
  *(p) = 42;
}

fn b() {
  var F : i32;
  a(10, &(F), 20);
}
)";

    auto* expect = R"(
fn a(pre : i32, p_root : ptr<function, i32>, post : i32) {
  *(p_root) = 42i;
}

fn b() {
  var F : i32;
  a(10i, &(F), 20i);
}
)";

    auto got = Run(src, kTransformFunction);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_FunctionAS, Enabled_Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
struct str {
  i : i32,
};

fn a(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *p;
}

fn b() {
  var F : str;
  a(10, &F.i, 20);
}
)";

    auto* expect = R"(
struct str {
  i : i32,
}

fn a(pre : i32, p_root : ptr<function, str>, post : i32) -> i32 {
  return (*(p_root)).i;
}

fn b() {
  var F : str;
  a(10i, &(F), 20i);
}
)";

    auto got = Run(src, kTransformFunction);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_FunctionAS, Enabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
struct str {
  arr : array<i32, 4>,
};

fn a(pre : i32, p : ptr<function, array<i32, 4>>, post : i32) {
  *p = array<i32, 4>();
}

fn b() {
  var F : str;
  a(10, &F.arr, 20);
}
)";

    auto* expect = R"(
struct str {
  arr : array<i32, 4u>,
}

fn a(pre : i32, p_root : ptr<function, str>, post : i32) {
  (*(p_root)).arr = array<i32, 4u>();
}

fn b() {
  var F : str;
  a(10i, &(F), 20i);
}
)";

    auto got = Run(src, kTransformFunction);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_FunctionAS, Enabled_Param_ptr_i32_mixed) {
    auto* src = R"(
struct str {
  i : i32,
};

fn a(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *p;
}

fn b() {
  var Fi : i32;
  var Fs : str;
  var Fa : array<i32, 4>;

  a(10, &Fi, 20);
  a(30, &Fs.i, 40);
  a(50, &Fa[2], 60);
}
)";

    auto* expect = R"(
fn a(pre : i32, p_root : ptr<function, i32>, post : i32) -> i32 {
  return *(p_root);
}

struct str {
  i : i32,
}

fn a_1(pre : i32, p_root : ptr<function, str>, post : i32) -> i32 {
  return (*(p_root)).i;
}

fn a_2(pre : i32, p_root : ptr<function, array<i32, 4u>>, p_indices : array<u32, 1u>, post : i32) -> i32 {
  return (*(p_root))[p_indices[0u]];
}

fn b() {
  var Fi : i32;
  var Fs : str;
  var Fa : array<i32, 4u>;
  a(10i, &(Fi), 20i);
  a_1(30i, &(Fs), 40i);
  a_2(50i, &(Fa), array<u32, 1u>(u32(2i)), 60i);
}
)";

    auto got = Run(src, kTransformFunction);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_FunctionAS, Disabled_Param_ptr_i32_Via_struct_read) {
    auto* src = R"(
fn a(pre : i32, p : ptr<function, i32>, post : i32) -> i32 {
  return *(p);
}

struct str {
  i : i32,
}

fn b() {
  var F : str;
  a(10i, &(F.i), 20i);
}
)";

    auto* expect = src;

    wgsl::writer::ProgramOptions program_options;
    program_options.allowed_features.features.emplace(
        wgsl::LanguageFeature::kUnrestrictedPointerParameters);
    auto got = Run(src, /* transform_options */ {}, program_options);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_FunctionAS, Disabled_Param_ptr_arr_i32_Via_struct_write) {
    auto* src = R"(
fn a(pre : i32, p : ptr<function, array<i32, 4u>>, post : i32) {
  *(p) = array<i32, 4u>();
}

struct str {
  arr : array<i32, 4u>,
}

fn b() {
  var F : str;
  a(10i, &(F.arr), 20i);
}
)";

    auto* expect = src;

    wgsl::writer::ProgramOptions program_options;
    program_options.allowed_features.features.emplace(
        wgsl::LanguageFeature::kUnrestrictedPointerParameters);
    auto got = Run(src, /* transform_options */ {}, program_options);

    EXPECT_EQ(expect, got);
}

}  // namespace function_as_tests

////////////////////////////////////////////////////////////////////////////////
// builtin function calls
////////////////////////////////////////////////////////////////////////////////
namespace builtin_fn_calls {

using IR_DirectVariableAccessWgslTest_BuiltinFn = DirectVariableAccessTest;

TEST_F(IR_DirectVariableAccessWgslTest_BuiltinFn, ArrayLength) {
    auto* src = R"(
@group(0) @binding(0) var<storage> S : array<f32>;

fn len(p : ptr<storage, array<f32>>) -> u32 {
  return arrayLength(p);
}

fn f() {
  let n = len(&S);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<storage, read> S : array<f32>;

fn len() -> u32 {
  return arrayLength(&(S));
}

fn f() {
  let n = len();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_BuiltinFn, WorkgroupUniformLoad) {
    auto* src = R"(
var<workgroup> W : f32;

fn load(p : ptr<workgroup, f32>) -> f32 {
  return workgroupUniformLoad(p);
}

fn f() {
  let v = load(&W);
}
)";

    auto* expect = R"(
var<workgroup> W : f32;

fn load() -> f32 {
  return workgroupUniformLoad(&(W));
}

fn f() {
  let v = load();
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

}  // namespace builtin_fn_calls

////////////////////////////////////////////////////////////////////////////////
// complex tests
////////////////////////////////////////////////////////////////////////////////
namespace complex_tests {

using IR_DirectVariableAccessWgslTest_Complex = DirectVariableAccessTest;

TEST_F(IR_DirectVariableAccessWgslTest_Complex, Param_ptr_mixed_vec4i32_ViaMultiple) {
    auto* src = R"(
struct str {
  i : vec4<i32>,
};

@group(0) @binding(0) var<uniform> U     : vec4<i32>;
@group(0) @binding(1) var<uniform> U_str   : str;
@group(0) @binding(2) var<uniform> U_arr   : array<vec4<i32>, 8>;
@group(0) @binding(3) var<uniform> U_arr_arr : array<array<vec4<i32>, 8>, 4>;

@group(1) @binding(0) var<storage> S     : vec4<i32>;
@group(1) @binding(1) var<storage> S_str   : str;
@group(1) @binding(2) var<storage> S_arr   : array<vec4<i32>, 8>;
@group(1) @binding(3) var<storage> S_arr_arr : array<array<vec4<i32>, 8>, 4>;

          var<workgroup> W     : vec4<i32>;
          var<workgroup> W_str   : str;
          var<workgroup> W_arr   : array<vec4<i32>, 8>;
          var<workgroup> W_arr_arr : array<array<vec4<i32>, 8>, 4>;

fn fn_u(p : ptr<uniform, vec4<i32>>) -> vec4<i32> {
  return *p;
}

fn fn_s(p : ptr<storage, vec4<i32>>) -> vec4<i32> {
  return *p;
}

fn fn_w(p : ptr<workgroup, vec4<i32>>) -> vec4<i32> {
  return *p;
}

fn b() {
  let I = 3;
  let J = 4;

  let u           = fn_u(&U);
  let u_str       = fn_u(&U_str.i);
  let u_arr0      = fn_u(&U_arr[0]);
  let u_arr1      = fn_u(&U_arr[1]);
  let u_arrI      = fn_u(&U_arr[I]);
  let u_arr1_arr0 = fn_u(&U_arr_arr[1][0]);
  let u_arr2_arrI = fn_u(&U_arr_arr[2][I]);
  let u_arrI_arr2 = fn_u(&U_arr_arr[I][2]);
  let u_arrI_arrJ = fn_u(&U_arr_arr[I][J]);

  let s           = fn_s(&S);
  let s_str       = fn_s(&S_str.i);
  let s_arr0      = fn_s(&S_arr[0]);
  let s_arr1      = fn_s(&S_arr[1]);
  let s_arrI      = fn_s(&S_arr[I]);
  let s_arr1_arr0 = fn_s(&S_arr_arr[1][0]);
  let s_arr2_arrI = fn_s(&S_arr_arr[2][I]);
  let s_arrI_arr2 = fn_s(&S_arr_arr[I][2]);
  let s_arrI_arrJ = fn_s(&S_arr_arr[I][J]);

  let w           = fn_w(&W);
  let w_str       = fn_w(&W_str.i);
  let w_arr0      = fn_w(&W_arr[0]);
  let w_arr1      = fn_w(&W_arr[1]);
  let w_arrI      = fn_w(&W_arr[I]);
  let w_arr1_arr0 = fn_w(&W_arr_arr[1][0]);
  let w_arr2_arrI = fn_w(&W_arr_arr[2][I]);
  let w_arrI_arr2 = fn_w(&W_arr_arr[I][2]);
  let w_arrI_arrJ = fn_w(&W_arr_arr[I][J]);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<uniform> U : vec4<i32>;

struct str {
  i : vec4<i32>,
}

@group(0u) @binding(1u) var<uniform> U_str : str;

@group(0u) @binding(2u) var<uniform> U_arr : array<vec4<i32>, 8u>;

@group(0u) @binding(3u) var<uniform> U_arr_arr : array<array<vec4<i32>, 8u>, 4u>;

@group(1u) @binding(0u) var<storage, read> S : vec4<i32>;

@group(1u) @binding(1u) var<storage, read> S_str : str;

@group(1u) @binding(2u) var<storage, read> S_arr : array<vec4<i32>, 8u>;

@group(1u) @binding(3u) var<storage, read> S_arr_arr : array<array<vec4<i32>, 8u>, 4u>;

var<workgroup> W : vec4<i32>;

var<workgroup> W_str : str;

var<workgroup> W_arr : array<vec4<i32>, 8u>;

var<workgroup> W_arr_arr : array<array<vec4<i32>, 8u>, 4u>;

fn fn_u() -> vec4<i32> {
  return U;
}

fn fn_u_1() -> vec4<i32> {
  return U_str.i;
}

fn fn_u_2(p_indices : array<u32, 1u>) -> vec4<i32> {
  return U_arr[p_indices[0u]];
}

fn fn_u_3(p_indices : array<u32, 2u>) -> vec4<i32> {
  return U_arr_arr[p_indices[0u]][p_indices[1u]];
}

fn fn_s() -> vec4<i32> {
  return S;
}

fn fn_s_1() -> vec4<i32> {
  return S_str.i;
}

fn fn_s_2(p_indices : array<u32, 1u>) -> vec4<i32> {
  return S_arr[p_indices[0u]];
}

fn fn_s_3(p_indices : array<u32, 2u>) -> vec4<i32> {
  return S_arr_arr[p_indices[0u]][p_indices[1u]];
}

fn fn_w() -> vec4<i32> {
  return W;
}

fn fn_w_1() -> vec4<i32> {
  return W_str.i;
}

fn fn_w_2(p_indices : array<u32, 1u>) -> vec4<i32> {
  return W_arr[p_indices[0u]];
}

fn fn_w_3(p_indices : array<u32, 2u>) -> vec4<i32> {
  return W_arr_arr[p_indices[0u]][p_indices[1u]];
}

fn b() {
  let I = 3i;
  let J = 4i;
  let u = fn_u();
  let u_str = fn_u_1();
  let u_arr0 = fn_u_2(array<u32, 1u>(u32(0i)));
  let u_arr1 = fn_u_2(array<u32, 1u>(u32(1i)));
  let u_arrI = fn_u_2(array<u32, 1u>(u32(I)));
  let u_arr1_arr0 = fn_u_3(array<u32, 2u>(u32(1i), u32(0i)));
  let u_arr2_arrI = fn_u_3(array<u32, 2u>(u32(2i), u32(I)));
  let u_arrI_arr2 = fn_u_3(array<u32, 2u>(u32(I), u32(2i)));
  let u_arrI_arrJ = fn_u_3(array<u32, 2u>(u32(I), u32(J)));
  let s = fn_s();
  let s_str = fn_s_1();
  let s_arr0 = fn_s_2(array<u32, 1u>(u32(0i)));
  let s_arr1 = fn_s_2(array<u32, 1u>(u32(1i)));
  let s_arrI = fn_s_2(array<u32, 1u>(u32(I)));
  let s_arr1_arr0 = fn_s_3(array<u32, 2u>(u32(1i), u32(0i)));
  let s_arr2_arrI = fn_s_3(array<u32, 2u>(u32(2i), u32(I)));
  let s_arrI_arr2 = fn_s_3(array<u32, 2u>(u32(I), u32(2i)));
  let s_arrI_arrJ = fn_s_3(array<u32, 2u>(u32(I), u32(J)));
  let w = fn_w();
  let w_str = fn_w_1();
  let w_arr0 = fn_w_2(array<u32, 1u>(u32(0i)));
  let w_arr1 = fn_w_2(array<u32, 1u>(u32(1i)));
  let w_arrI = fn_w_2(array<u32, 1u>(u32(I)));
  let w_arr1_arr0 = fn_w_3(array<u32, 2u>(u32(1i), u32(0i)));
  let w_arr2_arrI = fn_w_3(array<u32, 2u>(u32(2i), u32(I)));
  let w_arrI_arr2 = fn_w_3(array<u32, 2u>(u32(I), u32(2i)));
  let w_arrI_arrJ = fn_w_3(array<u32, 2u>(u32(I), u32(J)));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_Complex, Indexing) {
    auto* src = R"(
@group(0) @binding(0) var<storage> S : array<array<array<array<i32, 9>, 9>, 9>, 50>;

fn a(i : i32) -> i32 { return i; }

fn b(p : ptr<storage, array<array<array<i32, 9>, 9>, 9>>) -> i32 {
  return (*p) [ a( (*p)[0][1][2]    )]
              [ a( (*p)[a(3)][4][5] )]
              [ a( (*p)[6][a(7)][8] )];
}

fn c() {
  let v = b(&S[42]);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<storage, read> S : array<array<array<array<i32, 9u>, 9u>, 9u>, 50u>;

fn a(i : i32) -> i32 {
  return i;
}

fn b(p_indices : array<u32, 1u>) -> i32 {
  let v_1 = &(S[p_indices[0u]]);
  return (*(v_1))[a((*(v_1))[0i][1i][2i])][a((*(v_1))[a(3i)][4i][5i])][a((*(v_1))[6i][a(7i)][8i])];
}

fn c() {
  let v = b(array<u32, 1u>(u32(42i)));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_Complex, IndexingInPtrCall) {
    auto* src = R"(
@group(0) @binding(0) var<storage> S : array<array<array<array<i32, 9>, 9>, 9>, 50>;

fn a(pre : i32, i : ptr<storage, i32>, post : i32) -> i32 {
  return *i;
}

fn b(p : ptr<storage, array<array<array<i32, 9>, 9>, 9>>) -> i32 {
  return a(10, &(*p)[ a( 20, &(*p)[0][1][2], 30 )]
                    [ a( 40, &(*p)[3][4][5], 50 )]
                    [ a( 60, &(*p)[6][7][8], 70 )], 80);
}

fn c() {
  let v = b(&S[42]);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<storage, read> S : array<array<array<array<i32, 9u>, 9u>, 9u>, 50u>;

fn a(pre : i32, i_indices : array<u32, 4u>, post : i32) -> i32 {
  return S[i_indices[0u]][i_indices[1u]][i_indices[2u]][i_indices[3u]];
}

fn b(p_indices : array<u32, 1u>) -> i32 {
  let v_1 = p_indices[0u];
  let v_2 = a(20i, array<u32, 4u>(v_1, u32(0i), u32(1i), u32(2i)), 30i);
  let v_3 = a(40i, array<u32, 4u>(v_1, u32(3i), u32(4i), u32(5i)), 50i);
  return a(10i, array<u32, 4u>(v_1, u32(v_2), u32(v_3), u32(a(60i, array<u32, 4u>(v_1, u32(6i), u32(7i), u32(8i)), 70i))), 80i);
}

fn c() {
  let v = b(array<u32, 1u>(u32(42i)));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

TEST_F(IR_DirectVariableAccessWgslTest_Complex, IndexingDualPointers) {
    auto* src = R"(
@group(0) @binding(0) var<storage> S : array<array<array<i32, 9>, 9>, 50>;
@group(0) @binding(0) var<uniform> U : array<array<array<vec4<i32>, 9>, 9>, 50>;

fn a(i : i32) -> i32 { return i; }

fn b(s : ptr<storage, array<array<i32, 9>, 9>>,
     u : ptr<uniform, array<array<vec4<i32>, 9>, 9>>) -> i32 {
  return (*s) [ a( (*u)[0][1].x    )]
              [ a( (*u)[a(3)][4].y )];
}

fn c() {
  let v = b(&S[42], &U[24]);
}
)";

    auto* expect = R"(
@group(0u) @binding(0u) var<storage, read> S : array<array<array<i32, 9u>, 9u>, 50u>;

@group(0u) @binding(0u) var<uniform> U : array<array<array<vec4<i32>, 9u>, 9u>, 50u>;

fn a(i : i32) -> i32 {
  return i;
}

fn b(s_indices : array<u32, 1u>, u_indices : array<u32, 1u>) -> i32 {
  let v_1 = &(U[u_indices[0u]]);
  return S[s_indices[0u]][a((*(v_1))[0i][1i].x)][a((*(v_1))[a(3i)][4i].y)];
}

fn c() {
  let v = b(array<u32, 1u>(u32(42i)), array<u32, 1u>(u32(24i)));
}
)";

    auto got = Run(src);

    EXPECT_EQ(expect, got);
}

}  // namespace complex_tests

}  // namespace
}  // namespace tint::core::ir::transform
