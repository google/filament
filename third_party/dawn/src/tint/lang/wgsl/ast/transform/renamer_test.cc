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

#include "src/tint/lang/wgsl/ast/transform/renamer.h"

#include <memory>
#include <unordered_set>
#include <vector>

#include "gmock/gmock.h"
#include "src/tint/lang/core/builtin_type.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/utils/text/string.h"

namespace tint::ast::transform {
namespace {

constexpr const char kUnicodeIdentifier[] =  // "𝖎𝖉𝖊𝖓𝖙𝖎𝖋𝖎𝖊𝖗123"
    "\xf0\x9d\x96\x8e\xf0\x9d\x96\x89\xf0\x9d\x96\x8a\xf0\x9d\x96\x93"
    "\xf0\x9d\x96\x99\xf0\x9d\x96\x8e\xf0\x9d\x96\x8b\xf0\x9d\x96\x8e"
    "\xf0\x9d\x96\x8a\xf0\x9d\x96\x97\x31\x32\x33";

using ::testing::ContainerEq;

using RenamerTest = TransformTest;

TEST_F(RenamerTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, BasicModuleVertexIndex) {
    auto* src = R"(
fn test(vert_idx : u32) -> u32 {
  return vert_idx;
}

@vertex
fn entry(@builtin(vertex_index) vert_idx : u32
        ) -> @builtin(position) vec4<f32>  {
  _ = test(vert_idx);
  return vec4<f32>();
}
)";

    auto* expect = R"(
fn tint_symbol(tint_symbol_1 : u32) -> u32 {
  return tint_symbol_1;
}

@vertex
fn tint_symbol_2(@builtin(vertex_index) tint_symbol_1 : u32) -> @builtin(position) vec4<f32> {
  _ = tint_symbol(tint_symbol_1);
  return vec4<f32>();
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, RequestedNamesWithRenameAll) {
    auto* src = R"(
struct ShaderIO {
    @location(1) var1: f32,
    @location(3) @interpolate(flat) var3: u32,
    @builtin(position) pos: vec4f,
}

@vertex fn main(@builtin(vertex_index) vert_idx : u32)
     -> ShaderIO {
  var pos = array(
      vec2f(-1.0, 3.0),
      vec2f(-1.0, -3.0),
      vec2f(3.0, 0.0));

  var shaderIO: ShaderIO;
  shaderIO.var1 = 0.0;
  shaderIO.var3 = 1u;
  shaderIO.pos = vec4f(pos[vert_idx], 0.0, 1.0);

  return shaderIO;
}
)";

    auto* expect = R"(
struct tint_symbol {
  @location(1)
  user_var1 : f32,
  @location(3) @interpolate(flat)
  user_var3 : u32,
  @builtin(position)
  tint_symbol_1 : vec4f,
}

@vertex
fn tint_symbol_2(@builtin(vertex_index) tint_symbol_3 : u32) -> tint_symbol {
  var tint_symbol_1 = array(vec2f(-(1.0), 3.0), vec2f(-(1.0), -(3.0)), vec2f(3.0, 0.0));
  var tint_symbol_4 : tint_symbol;
  tint_symbol_4.user_var1 = 0.0;
  tint_symbol_4.user_var3 = 1u;
  tint_symbol_4.tint_symbol_1 = vec4f(tint_symbol_1[tint_symbol_3], 0.0, 1.0);
  return tint_symbol_4;
}
)";

    DataMap inputs;
    inputs.Add<Renamer::Config>(Renamer::Target::kAll,
                                /* remappings */
                                Renamer::Remappings{
                                    {"var1", "user_var1"},
                                    {"var3", "user_var3"},
                                });
    auto got = Run<Renamer>(src, inputs);

    EXPECT_EQ(expect, str(got));
}

using RenamerTestRequestedNamesWithoutRenameAll = TransformTestWithParam<Renamer::Target>;

TEST_P(RenamerTestRequestedNamesWithoutRenameAll, RequestedNames) {
    auto* src = R"(
struct ShaderIO {
    @location(1) var1: f32,
    @location(3) @interpolate(flat) var3: u32,
    @builtin(position) pos: vec4f,
}

@vertex fn entry_point(@builtin(vertex_index) vert_idx : u32)
     -> ShaderIO {
  var pos = array(
      vec2f(-1.0, 3.0),
      vec2f(-1.0, -3.0),
      vec2f(3.0, 0.0));

  var shaderIO: ShaderIO;
  shaderIO.var1 = 0.0;
  shaderIO.var3 = 1u;
  shaderIO.pos = vec4f(pos[vert_idx], 0.0, 1.0);

  return shaderIO;
}
)";

    auto* expect = R"(
struct ShaderIO {
  @location(1)
  user_var1 : f32,
  @location(3) @interpolate(flat)
  user_var3 : u32,
  @builtin(position)
  pos : vec4f,
}

@vertex
fn entry_point(@builtin(vertex_index) vert_idx : u32) -> ShaderIO {
  var pos = array(vec2f(-(1.0), 3.0), vec2f(-(1.0), -(3.0)), vec2f(3.0, 0.0));
  var shaderIO : ShaderIO;
  shaderIO.user_var1 = 0.0;
  shaderIO.user_var3 = 1u;
  shaderIO.pos = vec4f(pos[vert_idx], 0.0, 1.0);
  return shaderIO;
}
)";

    DataMap inputs;
    inputs.Add<Renamer::Config>(GetParam(),
                                /* remappings */
                                Renamer::Remappings{
                                    {"var1", "user_var1"},
                                    {"var3", "user_var3"},
                                });
    auto got = Run<Renamer>(src, inputs);

    EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(RenamerTestRequestedNamesWithoutRenameAll,
                         RenamerTestRequestedNamesWithoutRenameAll,
                         testing::Values(Renamer::Target::kGlslKeywords,
                                         Renamer::Target::kHlslKeywords,
                                         Renamer::Target::kMslKeywords));

TEST_F(RenamerTest, PreserveSwizzles) {
    auto* src = R"(
@vertex
fn entry() -> @builtin(position) vec4<f32> {
  var v : vec4<f32>;
  var rgba : f32;
  var xyzw : f32;
  var z : f32;
  return v.zyxw + v.rgab * v.z;
}
)";

    auto* expect = R"(
@vertex
fn tint_symbol() -> @builtin(position) vec4<f32> {
  var tint_symbol_1 : vec4<f32>;
  var tint_symbol_2 : f32;
  var tint_symbol_3 : f32;
  var tint_symbol_4 : f32;
  return (tint_symbol_1.zyxw + (tint_symbol_1.rgab * tint_symbol_1.z));
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, PreserveSwizzles_ThroughMaterialize) {
    auto* src = R"(
const v = vec4();
const x = v.x * 2u;
)";

    auto* expect = R"(
const tint_symbol = vec4();

const tint_symbol_1 = (tint_symbol.x * 2u);
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, PreserveBuiltins) {
    auto* src = R"(
@vertex
fn entry() -> @builtin(position) vec4<f32> {
  var blah : vec4<f32>;
  return abs(blah);
}
)";

    auto* expect = R"(
@vertex
fn tint_symbol() -> @builtin(position) vec4<f32> {
  var tint_symbol_1 : vec4<f32>;
  return abs(tint_symbol_1);
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, PreserveBuiltinTypes) {
    auto* src = R"(
@compute @workgroup_size(1)
fn entry() {
  var a = modf(1.0).whole;
  var b = modf(1.0).fract;
  var c = frexp(1.0).fract;
  var d = frexp(1.0).exp;
}
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn tint_symbol() {
  var tint_symbol_1 = modf(1.0).whole;
  var tint_symbol_2 = modf(1.0).fract;
  var tint_symbol_3 = frexp(1.0).fract;
  var tint_symbol_4 = frexp(1.0).exp;
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, PreserveBuiltinTypes_ViaPointerDot) {
    auto* src = R"(
@compute @workgroup_size(1)
fn entry() {
  var m = modf(1.0);
  let p1 = &m;
  var f = frexp(1.0);
  let p2 = &f;

  var a = p1.whole;
  var b = p1.fract;
  var c = p2.fract;
  var d = p2.exp;
}
)";

    auto* expect = R"(
@compute @workgroup_size(1)
fn tint_symbol() {
  var tint_symbol_1 = modf(1.0);
  let tint_symbol_2 = &(tint_symbol_1);
  var tint_symbol_3 = frexp(1.0);
  let tint_symbol_4 = &(tint_symbol_3);
  var tint_symbol_5 = tint_symbol_2.whole;
  var tint_symbol_6 = tint_symbol_2.fract;
  var tint_symbol_7 = tint_symbol_4.fract;
  var tint_symbol_8 = tint_symbol_4.exp;
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, PreserveCoreDiagnosticRuleName) {
    auto* src = R"(
diagnostic(off, chromium.unreachable_code);

@diagnostic(off, derivative_uniformity)
@fragment
fn entry(@location(0) value : f32) -> @location(0) f32 {
  if (value > 0) {
    return dpdx(value);
    return 0.0;
  }
  return 1.0;
}
)";

    auto* expect = R"(
diagnostic(off, chromium.unreachable_code);

@diagnostic(off, derivative_uniformity) @fragment
fn tint_symbol(@location(0) tint_symbol_1 : f32) -> @location(0) f32 {
  if ((tint_symbol_1 > 0)) {
    return dpdx(tint_symbol_1);
    return 0.0;
  }
  return 1.0;
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, AttemptSymbolCollision) {
    auto* src = R"(
@vertex
fn entry() -> @builtin(position) vec4<f32> {
  var tint_symbol : vec4<f32>;
  var tint_symbol_2 : vec4<f32>;
  var tint_symbol_4 : vec4<f32>;
  return tint_symbol + tint_symbol_2 + tint_symbol_4;
}
)";

    auto* expect = R"(
@vertex
fn tint_symbol() -> @builtin(position) vec4<f32> {
  var tint_symbol_1 : vec4<f32>;
  var tint_symbol_2 : vec4<f32>;
  var tint_symbol_3 : vec4<f32>;
  return ((tint_symbol_1 + tint_symbol_2) + tint_symbol_3);
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, PreserveTexelFormatAndAccess) {
    auto src = R"(
@group(0) @binding(0) var texture : texture_storage_2d<rgba8unorm, write>;

fn f() {
  var dims = textureDimensions(texture);
}
)";

    auto expect = R"(
@group(0) @binding(0) var tint_symbol : texture_storage_2d<rgba8unorm, write>;

fn tint_symbol_1() {
  var tint_symbol_2 = textureDimensions(tint_symbol);
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerTest, PreserveAddressSpace) {
    auto src = R"(
var<private> p : i32;

fn f() {
  var v = p;
}
)";

    auto expect = R"(
var<private> tint_symbol : i32;

fn tint_symbol_1() {
  var tint_symbol_2 = tint_symbol;
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}
using RenamerTestGlsl = TransformTestWithParam<std::string>;
using RenamerTestHlsl = TransformTestWithParam<std::string>;
using RenamerTestMsl = TransformTestWithParam<std::string>;

TEST_P(RenamerTestGlsl, Keywords) {
    auto keyword = GetParam();

    auto src = R"(
@fragment
fn frag_main() {
  var )" + keyword +
               R"( : i32;
}
)";

    auto* expect = R"(
@fragment
fn frag_main() {
  var tint_symbol : i32;
}
)";

    DataMap inputs;
    inputs.Add<Renamer::Config>(Renamer::Target::kGlslKeywords);
    auto got = Run<Renamer>(src, inputs);

    EXPECT_EQ(expect, str(got));
}

TEST_P(RenamerTestHlsl, Keywords) {
    auto keyword = GetParam();

    auto src = R"(
@fragment
fn frag_main() {
  var )" + keyword +
               R"( : i32;
}
)";

    auto* expect = R"(
@fragment
fn frag_main() {
  var tint_symbol : i32;
}
)";

    DataMap inputs;
    inputs.Add<Renamer::Config>(Renamer::Target::kHlslKeywords);
    auto got = Run<Renamer>(src, inputs);

    EXPECT_EQ(expect, str(got));
}

TEST_P(RenamerTestMsl, Keywords) {
    auto keyword = GetParam();

    auto src = R"(
@fragment
fn frag_main() {
  var )" + keyword +
               R"( : i32;
}
)";

    auto* expect = R"(
@fragment
fn frag_main() {
  var tint_symbol : i32;
}
)";

    DataMap inputs;
    inputs.Add<Renamer::Config>(Renamer::Target::kMslKeywords);
    auto got = Run<Renamer>(src, inputs);

    EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(RenamerTestGlsl,
                         RenamerTestGlsl,
                         testing::Values(  // "active",   // Also reserved in WGSL
                                           // "asm",       // WGSL keyword
                             "atomic_uint",
                             // "attribute",  // Also reserved in WGSL
                             // "bool",      // WGSL keyword
                             // "break",     // WGSL keyword
                             "buffer",
                             "bvec2",
                             "bvec3",
                             "bvec4",
                             //    "case",      // WGSL keyword
                             // "cast",  // Also reserved in WGSL
                             "centroid",
                             // "class",  // Also reserved in WGSL
                             // "coherent",  // Also reserved in WGSL
                             // "common",  // Also reserved in WGSL
                             // "const",     // WGSL keyword
                             // "continue",  // WGSL keyword
                             // "default",   // WGSL keyword
                             // "discard",   // WGSL keyword
                             "dmat2",
                             "dmat2x2",
                             "dmat2x3",
                             "dmat2x4",
                             "dmat3",
                             "dmat3x2",
                             "dmat3x3",
                             "dmat3x4",
                             "dmat4",
                             "dmat4x2",
                             "dmat4x3",
                             "dmat4x4",
                             // "do",         // WGSL keyword
                             "double",
                             "dvec2",
                             "dvec3",
                             "dvec4",
                             // "else"        // WGSL keyword
                             // "enum",       // WGSL keyword
                             // "extern",  // Also reserved in WGSL
                             // "external",  // Also reserved in WGSL
                             // "false",      // WGSL keyword
                             // "filter",  // Also reserved in WGSL
                             "fixed",
                             "flat",
                             "float",
                             // "for",        // WGSL keyword
                             "fvec2",
                             "fvec3",
                             "fvec4",
                             "gl_BaseInstance",
                             "gl_BaseVertex",
                             "gl_ClipDistance",
                             "gl_DepthRangeParameters",
                             "gl_DrawID",
                             "gl_FragCoord",
                             "gl_FragDepth",
                             "gl_FrontFacing",
                             "gl_GlobalInvocationID",
                             "gl_InstanceID",
                             "gl_LocalInvocationID",
                             "gl_LocalInvocationIndex",
                             "gl_NumSamples",
                             "gl_NumWorkGroups",
                             "gl_PerVertex",
                             "gl_PointCoord",
                             "gl_PointSize",
                             "gl_Position",
                             "gl_PrimitiveID",
                             "gl_SampleID",
                             "gl_SampleMask",
                             "gl_SampleMaskIn",
                             "gl_SamplePosition",
                             "gl_VertexID",
                             "gl_WorkGroupID",
                             "gl_WorkGroupSize",
                             // "goto",  // Also reserved in WGSL
                             "half",
                             // "highp",  // Also reserved in WGSL
                             "hvec2",
                             "hvec3",
                             "hvec4",
                             // "if",         // WGSL keyword
                             "iimage1D",
                             "iimage1DArray",
                             "iimage2D",
                             "iimage2DArray",
                             "iimage2DMS",
                             "iimage2DMSArray",
                             "iimage2DRect",
                             "iimage3D",
                             "iimageBuffer",
                             "iimageCube",
                             "iimageCubeArray",
                             "image1D",
                             "image1DArray",
                             "image2D",
                             "image2DArray",
                             "image2DMS",
                             "image2DMSArray",
                             "image2DRect",
                             "image3D",
                             "imageBuffer",
                             "imageCube",
                             "imageCubeArray",
                             "in",
                             // "inline",  // Also reserved in WGSL
                             // "inout",  // Also reserved in WGSL
                             "input",
                             "int",
                             // "interface",  // Also reserved in WGSL
                             // "invariant",  // Also reserved in WGSL
                             "isampler1D",
                             "isampler1DArray",
                             "isampler2D",
                             "isampler2DArray",
                             "isampler2DMS",
                             "isampler2DMSArray",
                             "isampler2DRect",
                             "isampler3D",
                             "isamplerBuffer",
                             "isamplerCube",
                             "isamplerCubeArray",
                             "ivec2",
                             "ivec3",
                             "ivec4",
                             // "layout",  // Also reserved in WGSL
                             "long",
                             // "lowp",  // Also reserved in WGSL
                             // "mat2x2",      // WGSL keyword
                             // "mat2x3",      // WGSL keyword
                             // "mat2x4",      // WGSL keyword
                             // "mat2",
                             "mat3",
                             // "mat3x2",      // WGSL keyword
                             // "mat3x3",      // WGSL keyword
                             // "mat3x4",      // WGSL keyword
                             "mat4",
                             // "mat4x2",      // WGSL keyword
                             // "mat4x3",      // WGSL keyword
                             // "mat4x4",      // WGSL keyword
                             // "mediump",  // Also reserved in WGSL
                             // "namespace",  // Also reserved in WGSL
                             // "noinline",  // Also reserved in WGSL
                             // "noncoherent",  // Reserved in WGSL
                             // "noperspective",  // Also reserved in WGSL
                             // "non_coherent",  // Reserved in WGSL
                             "out",
                             "output",
                             // "partition",  // Also reserved in WGSL
                             // "patch",  // Also reserved in WGSL
                             // "precise",  // Also reserved in WGSL
                             // "precision",  // Also reserved in WGSL
                             // "public",  // Also reserved in WGSL
                             // "readonly",  // Also reserved in WGSL
                             // "resource",  // Also reserved in WGSL
                             // "restrict",  // Also reserved in WGSL
                             // "return",     // WGSL keyword
                             "sample",
                             "sampler1D",
                             "sampler1DArray",
                             "sampler1DArrayShadow",
                             "sampler1DShadow",
                             "sampler2D",
                             "sampler2DArray",
                             "sampler2DArrayShadow",
                             "sampler2DMS",
                             "sampler2DMSArray",
                             "sampler2DRect",
                             "sampler2DRectShadow",
                             "sampler2DShadow",
                             "sampler3D",
                             "sampler3DRect",
                             "samplerBuffer",
                             "samplerCube",
                             "samplerCubeArray",
                             "samplerCubeArrayShadow",
                             "samplerCubeShadow",
                             // "shared"  // Also reserved in WGSL,
                             "short",
                             // "sizeof",  // Also reserved in WGSL
                             // "smooth",  // Also reserved in WGSL
                             // "static",  // Also reserved in WGSL
                             // "struct",     // WGSL keyword
                             // "subroutine",  // Also reserved in WGSL
                             "superp",
                             // "switch",     // WGSL keyword
                             // "template",  // Also reserved in WGSL
                             // "this",  // Also reserved in WGSL
                             // "true",       // WGSL keyword
                             // "typedef",    // WGSL keyword
                             "uimage1D",
                             "uimage1DArray",
                             "uimage2D",
                             "uimage2DArray",
                             "uimage2DMS",
                             "uimage2DMSArray",
                             "uimage2DRect",
                             "uimage3D",
                             "uimageBuffer",
                             "uimageCube",
                             "uimageCubeArray",
                             "uint",
                             // "uniform",    // WGSL keyword
                             // "union",  // Also reserved in WGSL
                             "unsigned",
                             "usampler1D",
                             "usampler1DArray",
                             "usampler2D",
                             "usampler2DArray",
                             "usampler2DMS",
                             "usampler2DMSArray",
                             "usampler2DRect",
                             "usampler3D",
                             "usamplerBuffer",
                             "usamplerCube",
                             "usamplerCubeArray",
                             // "using",      // WGSL keyword
                             "uvec2",
                             "uvec3",
                             "uvec4",
                             // "varying",  // Also reserved in WGSL
                             // "vec2",       // WGSL keyword
                             // "vec3",       // WGSL keyword
                             // "vec4",       // WGSL keyword
                             // "void",       // WGSL keyword
                             // "volatile",  // Also reserved in WGSL
                             // "while",      // WGSL keyword
                             // "writeonly",  // Also reserved in WGSL
                             kUnicodeIdentifier));

INSTANTIATE_TEST_SUITE_P(RenamerTestHlsl,
                         RenamerTestHlsl,
                         testing::Values("AddressU",
                                         "AddressV",
                                         "AddressW",
                                         "AllMemoryBarrier",
                                         "AllMemoryBarrierWithGroupSync",
                                         "AppendStructuredBuffer",
                                         "BINORMAL",
                                         "BLENDINDICES",
                                         "BLENDWEIGHT",
                                         "BlendState",
                                         "BorderColor",
                                         "Buffer",
                                         "ByteAddressBuffer",
                                         "COLOR",
                                         "CheckAccessFullyMapped",
                                         "ComparisonFunc",
                                         // "CompileShader",  // Also reserved in WGSL
                                         // "ComputeShader",  // Also reserved in WGSL
                                         "ConsumeStructuredBuffer",
                                         "D3DCOLORtoUBYTE4",
                                         "DEPTH",
                                         "DepthStencilState",
                                         "DepthStencilView",
                                         "DeviceMemoryBarrier",
                                         "DeviceMemroyBarrierWithGroupSync",
                                         // "DomainShader",  // Also reserved in WGSL
                                         "EvaluateAttributeAtCentroid",
                                         "EvaluateAttributeAtSample",
                                         "EvaluateAttributeSnapped",
                                         "FOG",
                                         "Filter",
                                         // "GeometryShader",  // Also reserved in WGSL
                                         "GetRenderTargetSampleCount",
                                         "GetRenderTargetSamplePosition",
                                         "GroupMemoryBarrier",
                                         "GroupMemroyBarrierWithGroupSync",
                                         // "Hullshader",  // Also reserved in WGSL
                                         "InputPatch",
                                         "InterlockedAdd",
                                         "InterlockedAnd",
                                         "InterlockedCompareExchange",
                                         "InterlockedCompareStore",
                                         "InterlockedExchange",
                                         "InterlockedMax",
                                         "InterlockedMin",
                                         "InterlockedOr",
                                         "InterlockedXor",
                                         "LineStream",
                                         "MaxAnisotropy",
                                         "MaxLOD",
                                         "MinLOD",
                                         "MipLODBias",
                                         "NORMAL",
                                         // "NULL",  // Also reserved in WGSL
                                         "Normal",
                                         "OutputPatch",
                                         "POSITION",
                                         "POSITIONT",
                                         "PSIZE",
                                         "PixelShader",
                                         "PointStream",
                                         "Process2DQuadTessFactorsAvg",
                                         "Process2DQuadTessFactorsMax",
                                         "Process2DQuadTessFactorsMin",
                                         "ProcessIsolineTessFactors",
                                         "ProcessQuadTessFactorsAvg",
                                         "ProcessQuadTessFactorsMax",
                                         "ProcessQuadTessFactorsMin",
                                         "ProcessTriTessFactorsAvg",
                                         "ProcessTriTessFactorsMax",
                                         "ProcessTriTessFactorsMin",
                                         "RWBuffer",
                                         "RWByteAddressBuffer",
                                         "RWStructuredBuffer",
                                         "RWTexture1D",
                                         "RWTexture1DArray",
                                         "RWTexture2D",
                                         "RWTexture2DArray",
                                         "RWTexture3D",
                                         "RasterizerState",
                                         "RenderTargetView",
                                         "SV_ClipDistance",
                                         "SV_Coverage",
                                         "SV_CullDistance",
                                         "SV_Depth",
                                         "SV_DepthGreaterEqual",
                                         "SV_DepthLessEqual",
                                         "SV_DispatchThreadID",
                                         "SV_DomainLocation",
                                         "SV_GSInstanceID",
                                         "SV_GroupID",
                                         "SV_GroupIndex",
                                         "SV_GroupThreadID",
                                         "SV_InnerCoverage",
                                         "SV_InsideTessFactor",
                                         "SV_InstanceID",
                                         "SV_IsFrontFace",
                                         "SV_OutputControlPointID",
                                         "SV_Position",
                                         "SV_PrimitiveID",
                                         "SV_RenderTargetArrayIndex",
                                         "SV_SampleIndex",
                                         "SV_StencilRef",
                                         "SV_Target",
                                         "SV_TessFactor",
                                         "SV_VertexArrayIndex",
                                         "SV_VertexID",
                                         "Sampler",
                                         "Sampler1D",
                                         "Sampler2D",
                                         "Sampler3D",
                                         "SamplerCUBE",
                                         "SamplerComparisonState",
                                         "SamplerState",
                                         "StructuredBuffer",
                                         "TANGENT",
                                         "TESSFACTOR",
                                         "TEXCOORD",
                                         "Texcoord",
                                         "Texture",
                                         "Texture1D",
                                         "Texture1DArray",
                                         "Texture2D",
                                         "Texture2DArray",
                                         "Texture2DMS",
                                         "Texture2DMSArray",
                                         "Texture3D",
                                         "TextureCube",
                                         "TextureCubeArray",
                                         "TriangleStream",
                                         "VFACE",
                                         "VPOS",
                                         "VertexShader",
                                         "abort",
                                         // "abs",  // WGSL builtin
                                         // "acos",  // WGSL builtin
                                         // "all",  // WGSL builtin
                                         "allow_uav_condition",
                                         // "any",  // WGSL builtin
                                         "asdouble",
                                         "asfloat",
                                         // "asin",  // WGSL builtin
                                         "asint",
                                         // "asm",  // WGSL keyword
                                         // "asm_fragment",  // Also reserved in WGSL
                                         "asuint",
                                         // "atan",  // WGSL builtin
                                         // "atan2",  // WGSL builtin
                                         // "auto",  // Also reserved in WGSL
                                         // "bool",  // WGSL keyword
                                         "bool1",
                                         "bool1x1",
                                         "bool1x2",
                                         "bool1x3",
                                         "bool1x4",
                                         "bool2",
                                         "bool2x1",
                                         "bool2x2",
                                         "bool2x3",
                                         "bool2x4",
                                         "bool3",
                                         "bool3x1",
                                         "bool3x2",
                                         "bool3x3",
                                         "bool3x4",
                                         "bool4",
                                         "bool4x1",
                                         "bool4x2",
                                         "bool4x3",
                                         "bool4x4",
                                         "branch",
                                         // "break",  // WGSL keyword
                                         // "call",  // WGSL builtin
                                         // "case",  // WGSL keyword
                                         // "catch",  // Also reserved in WGSL
                                         "cbuffer",
                                         // "ceil",  // WGSL builtin
                                         "centroid",
                                         "char",
                                         // "clamp",  // WGSL builtin
                                         // "class",  // Also reserved in WGSL
                                         "clip",
                                         // "column_major",  // Also reserved in WGSL
                                         // "compile",  // Also reserved in WGSL
                                         // "compile_fragment",  // Also reserved in WGSL
                                         // "const",  // WGSL keyword
                                         // "const_cast",  // Also reserved in WGSL
                                         // "continue",  // WGSL keyword
                                         // "cos",  // WGSL builtin
                                         // "cosh",  // WGSL builtin
                                         "countbits",
                                         // "cross",  // WGSL builtin
                                         "ddx",
                                         "ddx_coarse",
                                         "ddx_fine",
                                         "ddy",
                                         "ddy_coarse",
                                         "ddy_fine",
                                         // "default",  // WGSL keyword
                                         "degrees",
                                         // "delete",  // Also reserved in WGSL
                                         // "determinant",  // WGSL builtin
                                         // "discard",  // WGSL keyword
                                         // "distance",  // WGSL builtin
                                         // "do",  // WGSL keyword
                                         // "dot",  // WGSL builtin
                                         "double",
                                         "double1",
                                         "double1x1",
                                         "double1x2",
                                         "double1x3",
                                         "double1x4",
                                         "double2",
                                         "double2x1",
                                         "double2x2",
                                         "double2x3",
                                         "double2x4",
                                         "double3",
                                         "double3x1",
                                         "double3x2",
                                         "double3x3",
                                         "double3x4",
                                         "double4",
                                         "double4x1",
                                         "double4x2",
                                         "double4x3",
                                         "double4x4",
                                         "dst",
                                         "dword",
                                         "dword1",
                                         "dword1x1",
                                         "dword1x2",
                                         "dword1x3",
                                         "dword1x4",
                                         "dword2",
                                         "dword2x1",
                                         "dword2x2",
                                         "dword2x3",
                                         "dword2x4",
                                         "dword3",
                                         "dword3x1",
                                         "dword3x2",
                                         "dword3x3",
                                         "dword3x4",
                                         "dword4",
                                         "dword4x1",
                                         "dword4x2",
                                         "dword4x3",
                                         "dword4x4",
                                         // "dynamic_cast",  // Also reserved in WGSL
                                         // "else",  // WGSL keyword
                                         // "enum",  // WGSL keyword
                                         "errorf",
                                         // "exp",  // WGSL builtin
                                         // "exp2",  // WGSL builtin
                                         // "explicit",  // Also reserved in WGSL
                                         // "export",  // Also reserved in WGSL
                                         // "extern",  // Also reserved in WGSL
                                         "f16to32",
                                         "f32tof16",
                                         // "faceforward",  // WGSL builtin
                                         // "false",  // WGSL keyword
                                         "fastopt",
                                         "firstbithigh",
                                         "firstbitlow",
                                         "flatten",
                                         "float",
                                         "float1",
                                         "float1x1",
                                         "float1x2",
                                         "float1x3",
                                         "float1x4",
                                         "float2",
                                         "float2x1",
                                         "float2x2",
                                         "float2x3",
                                         "float2x4",
                                         "float3",
                                         "float3x1",
                                         "float3x2",
                                         "float3x3",
                                         "float3x4",
                                         "float4",
                                         "float4x1",
                                         "float4x2",
                                         "float4x3",
                                         "float4x4",
                                         // "floor",  // WGSL builtin
                                         // "fma",  // WGSL builtin
                                         "fmod",
                                         // "for",  // WGSL keyword
                                         "forcecase",
                                         "frac",
                                         // "frexp",  // WGSL builtin
                                         // "friend",  // Also reserved in WGSL
                                         // "fwidth",  // WGSL builtin
                                         // "fxgroup",  // Also reserved in WGSL
                                         // "goto",  // Also reserved in WGSL
                                         // "groupshared",  // Also reserved in WGSL
                                         "half",
                                         "half1",
                                         "half1x1",
                                         "half1x2",
                                         "half1x3",
                                         "half1x4",
                                         "half2",
                                         "half2x1",
                                         "half2x2",
                                         "half2x3",
                                         "half2x4",
                                         "half3",
                                         "half3x1",
                                         "half3x2",
                                         "half3x3",
                                         "half3x4",
                                         "half4",
                                         "half4x1",
                                         "half4x2",
                                         "half4x3",
                                         "half4x4",
                                         // "if",  // WGSL keyword
                                         // "in",  // WGSL keyword
                                         // "inline",  // Also reserved in WGSL
                                         // "inout",  // Also reserved in WGSL
                                         "int",
                                         "int1",
                                         "int1x1",
                                         "int1x2",
                                         "int1x3",
                                         "int1x4",
                                         "int2",
                                         "int2x1",
                                         "int2x2",
                                         "int2x3",
                                         "int2x4",
                                         "int3",
                                         "int3x1",
                                         "int3x2",
                                         "int3x3",
                                         "int3x4",
                                         "int4",
                                         "int4x1",
                                         "int4x2",
                                         "int4x3",
                                         "int4x4",
                                         // "interface",  // Also reserved in WGSL
                                         "isfinite",
                                         "isinf",
                                         "isnan",
                                         // "ldexp",  // WGSL builtin
                                         // "length",  // WGSL builtin
                                         "lerp",
                                         // "line",  // Also reserved in WGSL
                                         // "lineadj",  // Also reserved in WGSL
                                         "linear",
                                         "lit",
                                         // "log",  // WGSL builtin
                                         "log10",
                                         // "log2",  // WGSL builtin
                                         "long",
                                         // "loop",  // WGSL keyword
                                         "mad",
                                         "matrix",
                                         // "max",  // WGSL builtin
                                         // "min",  // WGSL builtin
                                         "min10float",
                                         "min10float1",
                                         "min10float1x1",
                                         "min10float1x2",
                                         "min10float1x3",
                                         "min10float1x4",
                                         "min10float2",
                                         "min10float2x1",
                                         "min10float2x2",
                                         "min10float2x3",
                                         "min10float2x4",
                                         "min10float3",
                                         "min10float3x1",
                                         "min10float3x2",
                                         "min10float3x3",
                                         "min10float3x4",
                                         "min10float4",
                                         "min10float4x1",
                                         "min10float4x2",
                                         "min10float4x3",
                                         "min10float4x4",
                                         "min12int",
                                         "min12int1",
                                         "min12int1x1",
                                         "min12int1x2",
                                         "min12int1x3",
                                         "min12int1x4",
                                         "min12int2",
                                         "min12int2x1",
                                         "min12int2x2",
                                         "min12int2x3",
                                         "min12int2x4",
                                         "min12int3",
                                         "min12int3x1",
                                         "min12int3x2",
                                         "min12int3x3",
                                         "min12int3x4",
                                         "min12int4",
                                         "min12int4x1",
                                         "min12int4x2",
                                         "min12int4x3",
                                         "min12int4x4",
                                         "min16float",
                                         "min16float1",
                                         "min16float1x1",
                                         "min16float1x2",
                                         "min16float1x3",
                                         "min16float1x4",
                                         "min16float2",
                                         "min16float2x1",
                                         "min16float2x2",
                                         "min16float2x3",
                                         "min16float2x4",
                                         "min16float3",
                                         "min16float3x1",
                                         "min16float3x2",
                                         "min16float3x3",
                                         "min16float3x4",
                                         "min16float4",
                                         "min16float4x1",
                                         "min16float4x2",
                                         "min16float4x3",
                                         "min16float4x4",
                                         "min16int",
                                         "min16int1",
                                         "min16int1x1",
                                         "min16int1x2",
                                         "min16int1x3",
                                         "min16int1x4",
                                         "min16int2",
                                         "min16int2x1",
                                         "min16int2x2",
                                         "min16int2x3",
                                         "min16int2x4",
                                         "min16int3",
                                         "min16int3x1",
                                         "min16int3x2",
                                         "min16int3x3",
                                         "min16int3x4",
                                         "min16int4",
                                         "min16int4x1",
                                         "min16int4x2",
                                         "min16int4x3",
                                         "min16int4x4",
                                         "min16uint",
                                         "min16uint1",
                                         "min16uint1x1",
                                         "min16uint1x2",
                                         "min16uint1x3",
                                         "min16uint1x4",
                                         "min16uint2",
                                         "min16uint2x1",
                                         "min16uint2x2",
                                         "min16uint2x3",
                                         "min16uint2x4",
                                         "min16uint3",
                                         "min16uint3x1",
                                         "min16uint3x2",
                                         "min16uint3x3",
                                         "min16uint3x4",
                                         "min16uint4",
                                         "min16uint4x1",
                                         "min16uint4x2",
                                         "min16uint4x3",
                                         "min16uint4x4",
                                         // "modf",  // WGSL builtin
                                         "msad4",
                                         "mul",
                                         // "mutable",  // Also reserved in WGSL
                                         // "namespace",  // Also reserved in WGSL
                                         // "new",  // Also reserved in WGSL
                                         // "nointerpolation",  // Also reserved in WGSL
                                         "noise",
                                         // "noperspective",  // Also reserved in WGSL
                                         // "normalize",  // WGSL builtin
                                         "numthreads",
                                         // "operator",  // Also reserved in WGSL
                                         // "out",  // WGSL keyword
                                         // "packoffset",  // Also reserved in WGSL
                                         // "pass",  // Also reserved in WGSL
                                         // "pixelfragment",  // Also reserved in WGSL
                                         "pixelshader",
                                         // "point",  // Also reserved in WGSL
                                         // "pow",  // WGSL builtin
                                         // "precise",  // Also reserved in WGSL
                                         "printf",
                                         // "private",  // WGSL keyword
                                         // "protected",  // Also reserved in WGSL
                                         // "public",  // Also reserved in WGSL
                                         "radians",
                                         "rcp",
                                         // "reflect",  // WGSL builtin
                                         "refract",
                                         // "register",  // Also reserved in WGSL
                                         // "reinterpret_cast",  // Also reserved in WGSL
                                         // "return",  // WGSL keyword
                                         // "reversebits",  // WGSL builtin
                                         // "round",  // WGSL builtin
                                         "row_major",
                                         "rsqrt",
                                         "sample",
                                         "sampler1D",
                                         "sampler2D",
                                         "sampler3D",
                                         "samplerCUBE",
                                         "sampler_state",
                                         "saturate",
                                         // "shared",  // Also reserved in WGSL
                                         "short",
                                         // "sign",  // WGSL builtin
                                         // "signed",  // Also reserved in WGSL
                                         // "sin",  // WGSL builtin
                                         "sincos",
                                         // "sinh",  // WGSL builtin
                                         // "sizeof",  // Also reserved in WGSL
                                         // "smoothstep",  // WGSL builtin
                                         // "snorm",  // Also reserved in WGSL
                                         // "sqrt",  // WGSL builtin
                                         "stateblock",
                                         "stateblock_state",
                                         // "static",  // Also reserved in WGSL
                                         // "static_cast",  // Also reserved in WGSL
                                         // "step",  // WGSL builtin
                                         "string",
                                         // "struct",  // WGSL keyword
                                         // "switch",  // WGSL keyword
                                         // "tan",  // WGSL builtin
                                         // "tanh",  // WGSL builtin
                                         "tbuffer",
                                         "technique",
                                         "technique10",
                                         "technique11",
                                         // "template",  // Also reserved in WGSL
                                         "tex1D",
                                         "tex1Dbias",
                                         "tex1Dgrad",
                                         "tex1Dlod",
                                         "tex1Dproj",
                                         "tex2D",
                                         "tex2Dbias",
                                         "tex2Dgrad",
                                         "tex2Dlod",
                                         "tex2Dproj",
                                         "tex3D",
                                         "tex3Dbias",
                                         "tex3Dgrad",
                                         "tex3Dlod",
                                         "tex3Dproj",
                                         "texCUBE",
                                         "texCUBEbias",
                                         "texCUBEgrad",
                                         "texCUBElod",
                                         "texCUBEproj",
                                         "texture",
                                         "texture1D",
                                         "texture1DArray",
                                         "texture2D",
                                         "texture2DArray",
                                         "texture2DMS",
                                         "texture2DMSArray",
                                         "texture3D",
                                         "textureCube",
                                         "textureCubeArray",
                                         // "this",  // Also reserved in WGSL
                                         // "throw",  // Also reserved in WGSL
                                         "transpose",
                                         "triangle",
                                         "triangleadj",
                                         // "true",  // WGSL keyword
                                         // "trunc",  // WGSL builtin
                                         // "try",  // Also reserved in WGSL
                                         // "typedef",  // WGSL keyword
                                         // "typename",  // Also reserved in WGSL
                                         "uint",
                                         "uint1",
                                         "uint1x1",
                                         "uint1x2",
                                         "uint1x3",
                                         "uint1x4",
                                         "uint2",
                                         "uint2x1",
                                         "uint2x2",
                                         "uint2x3",
                                         "uint2x4",
                                         "uint3",
                                         "uint3x1",
                                         "uint3x2",
                                         "uint3x3",
                                         "uint3x4",
                                         "uint4",
                                         "uint4x1",
                                         "uint4x2",
                                         "uint4x3",
                                         "uint4x4",
                                         // "uniform",  // WGSL keyword
                                         // "union",  // Also reserved in WGSL
                                         // "unorm",  // Also reserved in WGSL
                                         "unroll",
                                         "unsigned",
                                         // "using",  // WGSL reserved keyword
                                         "vector",
                                         "vertexfragment",
                                         "vertexshader",
                                         // "virtual",  // Also reserved in WGSL
                                         // "void",  // WGSL keyword
                                         // "volatile",  // Also reserved in WGSL
                                         // "while"  // WGSL reserved keyword
                                         kUnicodeIdentifier));

INSTANTIATE_TEST_SUITE_P(
    RenamerTestMsl,
    RenamerTestMsl,
    testing::Values(
        // c++14 spec
        // "alignas",  // Also reserved in WGSL
        // "alignof",  // Also reserved in WGSL
        "and",
        "and_eq",
        // "asm",  // Also reserved in WGSL
        // "auto",  // Also reserved in WGSL
        "bitand",
        "bitor",
        // "bool",   // Also used in WGSL
        // "break",  // Also used in WGSL
        // "case",   // Also used in WGSL
        // "catch",  // Also reserved in WGSL
        "char",
        "char16_t",
        "char32_t",
        // "class",  // Also reserved in WGSL
        "compl",
        // "const",     // Also used in WGSL
        // "const_cast",  // Also reserved in WGSL
        // "constexpr",  // Also reserved in WGSL
        // "continue",  // Also used in WGSL
        // "decltype",  // Also reserved in WGSL
        // "default",   // Also used in WGSL
        // "delete",  // Also reserved in WGSL
        // "do",  // Also used in WGSL
        "double",
        // "dynamic_cast",  // Also reserved in WGSL
        // "else",  // Also used in WGSL
        // "enum",  // Also used in WGSL
        // "explicit",  // Also reserved in WGSL
        // "extern",  // Also reserved in WGSL
        // "false",  // Also used in WGSL
        // "final",  // Also reserved in WGSL
        "float",
        // "for",  // Also used in WGSL
        // "friend",  // Also reserved in WGSL
        // "goto",  // Also reserved in WGSL
        // "if",  // Also used in WGSL
        // "inline",  // Also reserved in WGSL
        "int",
        "long",
        // "mutable",  // Also reserved in WGSL
        // "namespace",  // Also reserved in WGSL
        // "new",  // Also reserved in WGSL
        // "noexcept",  // Also reserved in WGSL
        "not",
        "not_eq",
        // "nullptr",  // Also reserved in WGSL
        // "operator",  // Also reserved in WGSL
        "or",
        "or_eq",
        // "override", // Also used in WGSL
        // "private",  // Also used in WGSL
        // "protected",  // Also reserved in WGSL
        // "public",  // Also reserved in WGSL
        // "register",  // Also reserved in WGSL
        // "reinterpret_cast",  // Also reserved in WGSL
        // "return",  // Also used in WGSL
        "short",
        // "signed",  // Also reserved in WGSL
        // "sizeof",  // Also reserved in WGSL
        // "static",  // Also reserved in WGSL
        // "static_assert",  // Also reserved in WGSL
        // "static_cast",  // Also reserved in WGSL
        // "struct",  // Also used in WGSL
        // "switch",  // Also used in WGSL
        // "template",  // Also reserved in WGSL
        // "this",  // Also reserved in WGSL
        // "thread_local",  // Also reserved in WGSL
        // "throw",  // Also reserved in WGSL
        // "true",  // Also used in WGSL
        // "try",  // Also reserved in WGSL
        // "typedef",  // Also used in WGSL
        // "typeid",  // Also reserved in WGSL
        // "typename",  // Also reserved in WGSL
        // "union",  // Also reserved in WGSL
        "unsigned",
        // "using",  // WGSL reserved keyword
        // "virtual",  // Also reserved in WGSL
        // "void",  // Also used in WGSL
        // "volatile",  // Also reserved in WGSL
        "wchar_t",
        // "while",  // WGSL reserved keyword
        "xor",
        "xor_eq",

        // Metal Spec
        "access",
        // "array",  // Also used in WGSL
        "array_ref",
        "as_type",
        // "atomic",  // Also used in WGSL
        "atomic_bool",
        "atomic_int",
        "atomic_uint",
        "bool2",
        "bool3",
        "bool4",
        "buffer",
        "char2",
        "char3",
        "char4",
        "const_reference",
        "constant",
        "depth2d",
        "depth2d_array",
        "depth2d_ms",
        "depth2d_ms_array",
        "depthcube",
        "depthcube_array",
        "device",
        "discard_fragment",
        "float2",
        "float2x2",
        "float2x3",
        "float2x4",
        "float3",
        "float3x2",
        "float3x3",
        "float3x4",
        "float4",
        "float4x2",
        "float4x3",
        "float4x4",
        "fragment",
        "half",
        "half2",
        "half2x2",
        "half2x3",
        "half2x4",
        "half3",
        "half3x2",
        "half3x3",
        "half3x4",
        "half4",
        "half4x2",
        "half4x3",
        "half4x4",
        "imageblock",
        "int16_t",
        "int2",
        "int3",
        "int32_t",
        "int4",
        "int64_t",
        "int8_t",
        "kernel",
        "long2",
        "long3",
        "long4",
        "main",  // No functions called main
        "matrix",
        "metal",  // The namespace
        "packed_bool2",
        "packed_bool3",
        "packed_bool4",
        "packed_char2",
        "packed_char3",
        "packed_char4",
        "packed_float2",
        "packed_float3",
        "packed_float4",
        "packed_half2",
        "packed_half3",
        "packed_half4",
        "packed_int2",
        "packed_int3",
        "packed_int4",
        "packed_short2",
        "packed_short3",
        "packed_short4",
        "packed_uchar2",
        "packed_uchar3",
        "packed_uchar4",
        "packed_uint2",
        "packed_uint3",
        "packed_uint4",
        "packed_ushort2",
        "packed_ushort3",
        "packed_ushort4",
        "patch_control_point",
        "ptrdiff_t",
        "r16snorm",
        "r16unorm",
        "r8unorm",
        "reference",
        "rg11b10f",
        "rg16snorm",
        "rg16unorm",
        "rg8snorm",
        "rg8unorm",
        "rgb10a2",
        "rgb9e5",
        "rgba16snorm",
        "rgba16unorm",
        "rgba8snorm",
        "rgba8unorm",
        // "sampler",  // Also used in WGSL
        "short2",
        "short3",
        "short4",
        "size_t",
        "srgba8unorm",
        "texture",
        "texture1d",
        "texture1d_array",
        "texture2d",
        "texture2d_array",
        "texture2d_ms",
        "texture2d_ms_array",
        "texture3d",
        "texture_buffer",
        "texturecube",
        "texturecube_array",
        "thread",
        "threadgroup",
        "threadgroup_imageblock",
        "uchar",
        "uchar2",
        "uchar3",
        "uchar4",
        "uint",
        "uint16_t",
        "uint2",
        "uint3",
        "uint32_t",
        "uint4",
        "uint64_t",
        "uint8_t",
        "ulong2",
        "ulong3",
        "ulong4",
        // "uniform",  // Also used in WGSL
        "ushort",
        "ushort2",
        "ushort3",
        "ushort4",
        // "vec",  // WGSL reserved keyword
        "vertex",

        // https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf
        // Table 6.5. Constants for single-precision floating-point math
        // functions
        "MAXFLOAT",
        "HUGE_VALF",
        "INFINITY",
        "infinity",
        "NAN",
        "M_E_F",
        "M_LOG2E_F",
        "M_LOG10E_F",
        "M_LN2_F",
        "M_LN10_F",
        "M_PI_F",
        "M_PI_2_F",
        "M_PI_4_F",
        "M_1_PI_F",
        "M_2_PI_F",
        "M_2_SQRTPI_F",
        "M_SQRT2_F",
        "M_SQRT1_2_F",
        "MAXHALF",
        "HUGE_VALH",
        "M_E_H",
        "M_LOG2E_H",
        "M_LOG10E_H",
        "M_LN2_H",
        "M_LN10_H",
        "M_PI_H",
        "M_PI_2_H",
        "M_PI_4_H",
        "M_1_PI_H",
        "M_2_PI_H",
        "M_2_SQRTPI_H",
        "M_SQRT2_H",
        "M_SQRT1_2_H",
        // "while"  // WGSL reserved keyword
        kUnicodeIdentifier));

std::string ExpandBuiltinType(std::string_view name) {
    if (name == "array") {
        return "array<i32, 4>";
    }
    if (name == "atomic") {
        return "atomic<i32>";
    }
    if (name == "mat2x2") {
        return "mat2x2<f32>";
    }
    if (name == "mat2x2f") {
        return "mat2x2<f32>";
    }
    if (name == "mat2x2h") {
        return "mat2x2<f16>";
    }
    if (name == "mat2x3") {
        return "mat2x3<f32>";
    }
    if (name == "mat2x3f") {
        return "mat2x3<f32>";
    }
    if (name == "mat2x3h") {
        return "mat2x3<f16>";
    }
    if (name == "mat2x4") {
        return "mat2x4<f32>";
    }
    if (name == "mat2x4f") {
        return "mat2x4<f32>";
    }
    if (name == "mat2x4h") {
        return "mat2x4<f16>";
    }
    if (name == "mat3x2") {
        return "mat3x2<f32>";
    }
    if (name == "mat3x2f") {
        return "mat3x2<f32>";
    }
    if (name == "mat3x2h") {
        return "mat3x2<f16>";
    }
    if (name == "mat3x3") {
        return "mat3x3<f32>";
    }
    if (name == "mat3x3f") {
        return "mat3x3<f32>";
    }
    if (name == "mat3x3h") {
        return "mat3x3<f16>";
    }
    if (name == "mat3x4") {
        return "mat3x4<f32>";
    }
    if (name == "mat3x4f") {
        return "mat3x4<f32>";
    }
    if (name == "mat3x4h") {
        return "mat3x4<f16>";
    }
    if (name == "mat4x2") {
        return "mat4x2<f32>";
    }
    if (name == "mat4x2f") {
        return "mat4x2<f32>";
    }
    if (name == "mat4x2h") {
        return "mat4x2<f16>";
    }
    if (name == "mat4x3") {
        return "mat4x3<f32>";
    }
    if (name == "mat4x3f") {
        return "mat4x3<f32>";
    }
    if (name == "mat4x3h") {
        return "mat4x3<f16>";
    }
    if (name == "mat4x4") {
        return "mat4x4<f32>";
    }
    if (name == "mat4x4f") {
        return "mat4x4<f32>";
    }
    if (name == "mat4x4h") {
        return "mat4x4<f16>";
    }
    if (name == "ptr") {
        return "ptr<function, i32>";
    }
    if (name == "vec2") {
        return "vec2<f32>";
    }
    if (name == "vec2f") {
        return "vec2<f32>";
    }
    if (name == "vec2h") {
        return "vec2<f16>";
    }
    if (name == "vec2i") {
        return "vec2<i32>";
    }
    if (name == "vec2u") {
        return "vec2<u32>";
    }
    if (name == "vec3") {
        return "vec3<f32>";
    }
    if (name == "vec3f") {
        return "vec3<f32>";
    }
    if (name == "vec3h") {
        return "vec3<f16>";
    }
    if (name == "vec3i") {
        return "vec3<i32>";
    }
    if (name == "vec3u") {
        return "vec3<u32>";
    }
    if (name == "vec4") {
        return "vec4<f32>";
    }
    if (name == "vec4f") {
        return "vec4<f32>";
    }
    if (name == "vec4h") {
        return "vec4<f16>";
    }
    if (name == "vec4i") {
        return "vec4<i32>";
    }
    if (name == "vec4u") {
        return "vec4<u32>";
    }
    return std::string(name);
}

std::vector<std::string_view> ConstructableTypes() {
    std::vector<std::string_view> out;
    for (auto type : core::kBuiltinTypeStrings) {
        if (type != "ptr" && type != "atomic" && !tint::HasPrefix(type, "sampler") &&
            !tint::HasPrefix(type, "texture") && !tint::HasPrefix(type, "__") &&
            !tint::HasPrefix(type, "input_attachment") &&
            !tint::HasPrefix(type, "subgroup_matrix")) {
            out.push_back(type);
        }
    }
    return out;
}

using RenamerBuiltinTypeTest = TransformTestWithParam<std::string_view>;

TEST_P(RenamerBuiltinTypeTest, PreserveTypeUsage) {
    auto expand = [&](std::string_view source) {
        return tint::ReplaceAll(source, "$type", ExpandBuiltinType(GetParam()));
    };

    auto src = expand(R"(
enable f16;

fn x(v : $type) -> $type {
  const a : $type = $type();
  let b : $type = a;
  var c : $type = b;
  return c;
}

struct y {
  a : $type,
}
)");

    auto expect = expand(R"(
enable f16;

fn tint_symbol(tint_symbol_1 : $type) -> $type {
  const tint_symbol_2 : $type = $type();
  let tint_symbol_3 : $type = tint_symbol_2;
  var tint_symbol_4 : $type = tint_symbol_3;
  return tint_symbol_4;
}

struct tint_symbol_5 {
  tint_symbol_2 : $type,
}
)");

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}
TEST_P(RenamerBuiltinTypeTest, PreserveTypeInitializer) {
    auto expand = [&](std::string_view source) {
        return tint::ReplaceAll(source, "$type", ExpandBuiltinType(GetParam()));
    };

    auto src = expand(R"(
enable f16;

@fragment
fn f() {
  var v : $type = $type();
}
)");

    auto expect = expand(R"(
enable f16;

@fragment
fn tint_symbol() {
  var tint_symbol_1 : $type = $type();
}
)");

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(RenamerBuiltinTypeTest, PreserveTypeConversion) {
    if (std::string_view(GetParam()) == "array") {
        return;  // Cannot value convert arrays.
    }

    auto expand = [&](std::string_view source) {
        return tint::ReplaceAll(source, "$type", ExpandBuiltinType(GetParam()));
    };

    auto src = expand(R"(
enable f16;

@fragment
fn f() {
  var v : $type = $type($type());
}
)");

    auto expect = expand(R"(
enable f16;

@fragment
fn tint_symbol() {
  var tint_symbol_1 : $type = $type($type());
}
)");

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(RenamerBuiltinTypeTest, PreserveTypeExpression) {
    auto src = R"(
enable f16;

@fragment
fn f() {
  var v : array<f32, 2> = array<f32, 2>();
}
)";

    auto expect = R"(
enable f16;

@fragment
fn tint_symbol() {
  var tint_symbol_1 : array<f32, 2> = array<f32, 2>();
}
)";

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(RenamerBuiltinTypeTest, RenameShadowedByAlias) {
    auto expand = [&](std::string_view source) {
        std::string_view ty = GetParam();
        auto out = tint::ReplaceAll(source, "$name", ty);
        out = tint::ReplaceAll(out, "$type", ExpandBuiltinType(ty));
        out = tint::ReplaceAll(out, "$other_type", ty == "i32" ? "u32" : "i32");
        return out;
    };

    auto src = expand(R"(
alias $name = $other_type;

@fragment
fn f() {
  var v : $other_type = $name();
}
)");

    auto expect = expand(R"(
alias tint_symbol = $other_type;

@fragment
fn tint_symbol_1() {
  var tint_symbol_2 : $other_type = tint_symbol();
}
)");

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(RenamerBuiltinTypeTest, RenameShadowedByStruct) {
    auto expand = [&](std::string_view source) {
        std::string_view ty = GetParam();
        auto out = tint::ReplaceAll(source, "$name", ty);
        out = tint::ReplaceAll(out, "$type", ExpandBuiltinType(ty));
        out = tint::ReplaceAll(out, "$other_type", ty == "i32" ? "u32" : "i32");
        return out;
    };

    auto src = expand(R"(
struct $name {
  i : $other_type,
}

@fragment
fn f() {
  var a = $name();
  var b = a.i;
}
)");

    auto expect = expand(R"(
struct tint_symbol {
  tint_symbol_1 : $other_type,
}

@fragment
fn tint_symbol_2() {
  var tint_symbol_3 = tint_symbol();
  var tint_symbol_4 = tint_symbol_3.tint_symbol_1;
}
)");

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(RenamerBuiltinTypeTest,
                         RenamerBuiltinTypeTest,
                         testing::ValuesIn(ConstructableTypes()));

/// @return WGSL builtin identifier keywords
std::vector<std::string_view> Identifiers() {
    std::vector<std::string_view> out;
    for (auto ident : core::kBuiltinTypeStrings) {
        if (!tint::HasPrefix(ident, "__")) {
            out.push_back(ident);
        }
    }
    for (auto ident : core::kAddressSpaceStrings) {
        if (!tint::HasPrefix(ident, "_")) {
            out.push_back(ident);
        }
    }
    for (auto ident : core::kTexelFormatStrings) {
        out.push_back(ident);
    }
    for (auto ident : core::kAccessStrings) {
        out.push_back(ident);
    }
    return out;
}

using RenamerBuiltinIdentifierTest = TransformTestWithParam<std::string_view>;

TEST_P(RenamerBuiltinIdentifierTest, GlobalConstName) {
    auto expand = [&](std::string_view source) {
        return tint::ReplaceAll(source, "$name", GetParam());
    };

    auto src = expand(R"(
const $name = 42;

fn f() {
  const v = $name;
}
)");

    auto expect = expand(R"(
const tint_symbol = 42;

fn tint_symbol_1() {
  const tint_symbol_2 = tint_symbol;
}
)");

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(RenamerBuiltinIdentifierTest, LocalVarName) {
    auto expand = [&](std::string_view source) {
        return tint::ReplaceAll(source, "$name", GetParam());
    };

    auto src = expand(R"(
fn f() {
  var $name = 42;
}
)");

    auto expect = expand(R"(
fn tint_symbol() {
  var tint_symbol_1 = 42;
}
)");

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(RenamerBuiltinIdentifierTest, FunctionName) {
    auto expand = [&](std::string_view source) {
        return tint::ReplaceAll(source, "$name", GetParam());
    };

    auto src = expand(R"(
fn $name() {
}

fn f() {
  $name();
}
)");

    auto expect = expand(R"(
fn tint_symbol() {
}

fn tint_symbol_1() {
  tint_symbol();
}
)");

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_P(RenamerBuiltinIdentifierTest, StructName) {
    auto expand = [&](std::string_view source) {
        std::string_view name = GetParam();
        auto out = tint::ReplaceAll(source, "$name", name);
        return tint::ReplaceAll(out, "$other_type", name == "i32" ? "u32" : "i32");
    };

    auto src = expand(R"(
struct $name {
  i : $other_type,
}

fn f() {
  var x = $name();
}
)");

    auto expect = expand(R"(
struct tint_symbol {
  tint_symbol_1 : $other_type,
}

fn tint_symbol_2() {
  var tint_symbol_3 = tint_symbol();
}
)");

    auto got = Run<Renamer>(src);

    EXPECT_EQ(expect, str(got));
}

INSTANTIATE_TEST_SUITE_P(RenamerBuiltinIdentifierTest,
                         RenamerBuiltinIdentifierTest,
                         testing::ValuesIn(Identifiers()));

}  // namespace
}  // namespace tint::ast::transform
