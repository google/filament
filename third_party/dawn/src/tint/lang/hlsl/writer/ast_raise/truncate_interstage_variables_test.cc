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

#include "src/tint/lang/hlsl/writer/ast_raise/truncate_interstage_variables.h"
#include "src/tint/lang/wgsl/ast/transform/canonicalize_entry_point_io.h"

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::hlsl::writer {
namespace {

using ::testing::ContainerEq;

using TruncateInterstageVariablesTest = ast::transform::TransformTest;

TEST(TintCheckAllFieldsReflected, HlslWriterAstRaiseTruncateInterstageVariablesTest) {
    TINT_ASSERT_ALL_FIELDS_REFLECTED(TruncateInterstageVariables::Config);
}

TEST_F(TruncateInterstageVariablesTest, ShouldRunVertex) {
    auto* src = R"(
struct ShaderIO {
  @builtin(position) pos: vec4<f32>,
  @location(0) f_0: f32,
  @location(2) f_2: f32,
}
@vertex
fn f() -> ShaderIO {
  var io: ShaderIO;
  io.f_0 = 1.0;
  io.f_2 = io.f_2 + 3.0;
  return io;
}
)";

    {
        auto* expect =
            "error: missing transform data for tint::hlsl::writer::TruncateInterstageVariables";
        auto got = Run<TruncateInterstageVariables>(src);
        EXPECT_EQ(expect, str(got));
    }

    {
        // Empty interstage_locations: truncate all interstage variables, should run
        TruncateInterstageVariables::Config cfg;
        ast::transform::DataMap data;
        data.Add<TruncateInterstageVariables::Config>(cfg);
        EXPECT_TRUE(ShouldRun<TruncateInterstageVariables>(src, data));
    }

    {
        // All existing interstage_locations are marked: should not run
        TruncateInterstageVariables::Config cfg;
        cfg.interstage_locations[0] = true;
        cfg.interstage_locations[2] = true;
        ast::transform::DataMap data;
        data.Add<TruncateInterstageVariables::Config>(cfg);
        EXPECT_FALSE(ShouldRun<TruncateInterstageVariables>(src, data));
    }

    {
        // Partial interstage_locations are marked: should run
        TruncateInterstageVariables::Config cfg;
        cfg.interstage_locations[2] = true;
        ast::transform::DataMap data;
        data.Add<TruncateInterstageVariables::Config>(cfg);
        EXPECT_TRUE(ShouldRun<TruncateInterstageVariables>(src, data));
    }
}

TEST_F(TruncateInterstageVariablesTest, ShouldRunFragment) {
    auto* src = R"(
struct ShaderIO {
  @location(0) f_0: f32,
  @location(2) f_2: f32,
}
@fragment
fn f(io: ShaderIO) -> @location(1) vec4<f32> {
  return vec4<f32>(io.f_0, io.f_2, 0.0, 1.0);
}
)";

    TruncateInterstageVariables::Config cfg;
    cfg.interstage_locations[2] = true;

    ast::transform::DataMap data;
    data.Add<TruncateInterstageVariables::Config>(cfg);

    EXPECT_FALSE(ShouldRun<TruncateInterstageVariables>(src, data));
}

// Test that this transform should run after canoicalize entry point io, where shader io is already
// grouped into a struct.
TEST_F(TruncateInterstageVariablesTest, ShouldRunAfterCanonicalizeEntryPointIO) {
    auto* src = R"(
@vertex
fn f() -> @builtin(position) vec4<f32> {
  return vec4<f32>(1.0, 1.0, 1.0, 1.0);
}
)";

    TruncateInterstageVariables::Config cfg;
    cfg.interstage_locations[0] = true;
    ast::transform::DataMap data;
    data.Add<TruncateInterstageVariables::Config>(cfg);

    data.Add<ast::transform::CanonicalizeEntryPointIO::Config>(
        ast::transform::CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
    auto got = Run<ast::transform::CanonicalizeEntryPointIO>(src, data);

    // Inevitably entry point can write only one variable if not using struct
    // So the truncate won't run.
    EXPECT_FALSE(ShouldRun<TruncateInterstageVariables>(str(got), data));
}

TEST_F(TruncateInterstageVariablesTest, BasicVertexTrimLocationInMid) {
    auto* src = R"(
struct ShaderIO {
  @builtin(position) pos: vec4<f32>,
  @location(1) f_1: f32,
  @location(3) f_3: f32,
}
@vertex
fn f() -> ShaderIO {
  var io: ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = io.f_1 + 3.0;
  return io;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
  @location(1)
  f_1 : f32,
}

fn truncate_shader_output(io : ShaderIO) -> tint_symbol {
  return tint_symbol(io.pos, io.f_1);
}

struct ShaderIO {
  pos : vec4<f32>,
  f_1 : f32,
  f_3 : f32,
}

@vertex
fn f() -> tint_symbol {
  var io : ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = (io.f_1 + 3.0);
  return truncate_shader_output(io);
}
)";

    TruncateInterstageVariables::Config cfg;
    // fragment has input at @location(1)
    cfg.interstage_locations[1] = true;

    ast::transform::DataMap data;
    data.Add<TruncateInterstageVariables::Config>(cfg);

    auto got = Run<TruncateInterstageVariables>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(TruncateInterstageVariablesTest, BasicVertexTrimLocationAtEnd) {
    auto* src = R"(
struct ShaderIO {
  @builtin(position) pos: vec4<f32>,
  @location(1) f_1: f32,
  @location(3) f_3: f32,
}
@vertex
fn f() -> ShaderIO {
  var io: ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = io.f_1 + 3.0;
  return io;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
  @location(3)
  f_3 : f32,
}

fn truncate_shader_output(io : ShaderIO) -> tint_symbol {
  return tint_symbol(io.pos, io.f_3);
}

struct ShaderIO {
  pos : vec4<f32>,
  f_1 : f32,
  f_3 : f32,
}

@vertex
fn f() -> tint_symbol {
  var io : ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = (io.f_1 + 3.0);
  return truncate_shader_output(io);
}
)";

    TruncateInterstageVariables::Config cfg;
    // fragment has input at @location(3)
    cfg.interstage_locations[3] = true;

    ast::transform::DataMap data;
    data.Add<TruncateInterstageVariables::Config>(cfg);

    auto got = Run<TruncateInterstageVariables>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(TruncateInterstageVariablesTest, TruncateAllLocations) {
    auto* src = R"(
struct ShaderIO {
  @builtin(position) pos: vec4<f32>,
  @location(1) f_1: f32,
  @location(3) f_3: f32,
}
@vertex
fn f() -> ShaderIO {
  var io: ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = io.f_1 + 3.0;
  return io;
}
)";

    {
        auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
}

fn truncate_shader_output(io : ShaderIO) -> tint_symbol {
  return tint_symbol(io.pos);
}

struct ShaderIO {
  pos : vec4<f32>,
  f_1 : f32,
  f_3 : f32,
}

@vertex
fn f() -> tint_symbol {
  var io : ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = (io.f_1 + 3.0);
  return truncate_shader_output(io);
}
)";

        TruncateInterstageVariables::Config cfg;
        ast::transform::DataMap data;
        data.Add<TruncateInterstageVariables::Config>(cfg);

        auto got = Run<TruncateInterstageVariables>(src, data);

        EXPECT_EQ(expect, str(got));
    }
}

// Test that the transform only removes IO attributes and preserve other attributes in the old
// Shader IO struct.
TEST_F(TruncateInterstageVariablesTest, RemoveIOAttributes) {
    auto* src = R"(
struct ShaderIO {
  @builtin(position) @invariant pos: vec4<f32>,
  @location(1) f_1: f32,
  @location(3) @align(16) f_3: f32,
  @location(5) @interpolate(flat) @align(16) @size(16) f_5: u32,
}
@vertex
fn f() -> ShaderIO {
  var io: ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = io.f_1 + 3.0;
  io.f_5 = 1u;
  return io;
}
)";

    {
        auto* expect = R"(
struct tint_symbol {
  @builtin(position) @invariant
  pos : vec4<f32>,
  @location(3) @align(16)
  f_3 : f32,
  @location(5) @interpolate(flat) @align(16) @size(16)
  f_5 : u32,
}

fn truncate_shader_output(io : ShaderIO) -> tint_symbol {
  return tint_symbol(io.pos, io.f_3, io.f_5);
}

struct ShaderIO {
  pos : vec4<f32>,
  f_1 : f32,
  @align(16)
  f_3 : f32,
  @align(16) @size(16)
  f_5 : u32,
}

@vertex
fn f() -> tint_symbol {
  var io : ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = (io.f_1 + 3.0);
  io.f_5 = 1u;
  return truncate_shader_output(io);
}
)";

        TruncateInterstageVariables::Config cfg;
        // Missing @location[1] intentionally to make sure the transform run.
        cfg.interstage_locations[3] = true;
        cfg.interstage_locations[5] = true;

        ast::transform::DataMap data;
        data.Add<TruncateInterstageVariables::Config>(cfg);

        auto got = Run<TruncateInterstageVariables>(src, data);

        EXPECT_EQ(expect, str(got));
    }
}

TEST_F(TruncateInterstageVariablesTest, MultipleEntryPointsSharingStruct) {
    auto* src = R"(
struct ShaderIO {
  @builtin(position) pos: vec4<f32>,
  @location(1) f_1: f32,
  @location(3) f_3: f32,
  @location(5) f_5: f32,
}

@vertex
fn f1() -> ShaderIO {
  var io: ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = 1.0;
  return io;
}

@vertex
fn f2() -> ShaderIO {
  var io: ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_5 = 2.0;
  return io;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
  @location(3)
  f_3 : f32,
}

fn truncate_shader_output(io : ShaderIO) -> tint_symbol {
  return tint_symbol(io.pos, io.f_3);
}

struct ShaderIO {
  pos : vec4<f32>,
  f_1 : f32,
  f_3 : f32,
  f_5 : f32,
}

@vertex
fn f1() -> tint_symbol {
  var io : ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = 1.0;
  return truncate_shader_output(io);
}

@vertex
fn f2() -> tint_symbol {
  var io : ShaderIO;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_5 = 2.0;
  return truncate_shader_output(io);
}
)";

    TruncateInterstageVariables::Config cfg;
    // fragment has input at @location(3)
    cfg.interstage_locations[3] = true;

    ast::transform::DataMap data;
    data.Add<TruncateInterstageVariables::Config>(cfg);

    auto got = Run<TruncateInterstageVariables>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(TruncateInterstageVariablesTest, MultipleEntryPoints) {
    auto* src = R"(
struct ShaderIO1 {
  @builtin(position) pos: vec4<f32>,
  @location(1) f_1: f32,
  @location(3) f_3: f32,
  @location(5) f_5: f32,
}

@vertex
fn f1() -> ShaderIO1 {
  var io: ShaderIO1;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = 1.0;
  return io;
}

struct ShaderIO2 {
  @builtin(position) pos: vec4<f32>,
  @location(2) f_2: f32,
}

@vertex
fn f2() -> ShaderIO2 {
  var io: ShaderIO2;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_2 = 2.0;
  return io;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
  @location(3)
  f_3 : f32,
}

fn truncate_shader_output(io : ShaderIO1) -> tint_symbol {
  return tint_symbol(io.pos, io.f_3);
}

struct tint_symbol_1 {
  @builtin(position)
  pos : vec4<f32>,
}

fn truncate_shader_output_1(io : ShaderIO2) -> tint_symbol_1 {
  return tint_symbol_1(io.pos);
}

struct ShaderIO1 {
  pos : vec4<f32>,
  f_1 : f32,
  f_3 : f32,
  f_5 : f32,
}

@vertex
fn f1() -> tint_symbol {
  var io : ShaderIO1;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = 1.0;
  return truncate_shader_output(io);
}

struct ShaderIO2 {
  pos : vec4<f32>,
  f_2 : f32,
}

@vertex
fn f2() -> tint_symbol_1 {
  var io : ShaderIO2;
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_2 = 2.0;
  return truncate_shader_output_1(io);
}
)";

    TruncateInterstageVariables::Config cfg;
    // fragment has input at @location(3)
    cfg.interstage_locations[3] = true;

    ast::transform::DataMap data;
    data.Add<TruncateInterstageVariables::Config>(cfg);

    auto got = Run<TruncateInterstageVariables>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(TruncateInterstageVariablesTest, MultipleReturnStatements) {
    auto* src = R"(
struct ShaderIO {
  @builtin(position) pos: vec4<f32>,
  @location(1) f_1: f32,
  @location(3) f_3: f32,
}
@vertex
fn f(@builtin(vertex_index) vid: u32) -> ShaderIO {
  var io: ShaderIO;
  if (vid > 10u) {
    io.f_1 = 2.0;
    return io;
  }
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = io.f_1 + 3.0;
  return io;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @builtin(position)
  pos : vec4<f32>,
  @location(3)
  f_3 : f32,
}

fn truncate_shader_output(io : ShaderIO) -> tint_symbol {
  return tint_symbol(io.pos, io.f_3);
}

struct ShaderIO {
  pos : vec4<f32>,
  f_1 : f32,
  f_3 : f32,
}

@vertex
fn f(@builtin(vertex_index) vid : u32) -> tint_symbol {
  var io : ShaderIO;
  if ((vid > 10u)) {
    io.f_1 = 2.0;
    return truncate_shader_output(io);
  }
  io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  io.f_1 = 1.0;
  io.f_3 = (io.f_1 + 3.0);
  return truncate_shader_output(io);
}
)";

    TruncateInterstageVariables::Config cfg;
    // fragment has input at @location(3)
    cfg.interstage_locations[3] = true;

    ast::transform::DataMap data;
    data.Add<TruncateInterstageVariables::Config>(cfg);

    auto got = Run<TruncateInterstageVariables>(src, data);

    EXPECT_EQ(expect, str(got));
}

TEST_F(TruncateInterstageVariablesTest, LocationOutOfRange) {
    auto* src = R"(
struct ShaderIO {
  @builtin(position) pos: vec4<f32>,
  @location(0) f_0: f32,
  @location(30) f_2: f32,
}
@vertex
fn f() -> ShaderIO {
  var io: ShaderIO;
  io.f_0 = 1.0;
  io.f_2 = io.f_2 + 3.0;
  return io;
}
)";

    // Return error when location >= 30 (maximum supported number of inter-stage shader variables)
    auto* expect = "error: The location (30) of f_2 in ShaderIO exceeds the maximum value (29).";

    TruncateInterstageVariables::Config cfg;
    ast::transform::DataMap data;
    data.Add<TruncateInterstageVariables::Config>(cfg);

    auto got = Run<TruncateInterstageVariables>(src, data);
    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::hlsl::writer
