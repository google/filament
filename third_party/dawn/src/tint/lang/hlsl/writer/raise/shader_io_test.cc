// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/raise/shader_io.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using HlslWriterTransformTest = core::ir::transform::TransformTest;

TEST_F(HlslWriterTransformTest, ShaderIONoInputsOrOutputs) {
    auto* ep = b.ComputeFunction("foo");
    b.Append(ep->Block(), [&] { b.Return(ep); });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NonStruct) {
    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);

    auto* position = b.FunctionParam("position", ty.vec4f());
    position->SetBuiltin(core::BuiltinValue::kPosition);
    position->SetInvariant(true);

    auto* color1 = b.FunctionParam("color1", ty.f32());
    color1->SetLocation(0);

    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample});

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({front_facing, position, color1, color2});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {
            b.Multiply(position, b.Add(color1, color2));
            b.ExitIf(ifelse);
        });
        b.Return(ep);
    });

    auto* src = R"(
%foo = @fragment func(%front_facing:bool [@front_facing], %position:vec4<f32> [@invariant, @position], %color1:f32 [@location(0)], %color2:f32 [@location(1), @interpolate(linear, sample)]):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %6:f32 = add %color1, %color2
        %7:vec4<f32> = mul %position, %6
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
  position:vec4<f32> @offset(16), @invariant, @builtin(position)
  front_facing:bool @offset(32), @builtin(front_facing)
}

%foo_inner = func(%front_facing:bool, %position:vec4<f32>, %color1:f32, %color2:f32):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %6:f32 = add %color1, %color2
        %7:vec4<f32> = mul %position, %6
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func(%inputs:foo_inputs):void {
  $B3: {
    %10:bool = access %inputs, 3u
    %11:vec4<f32> = access %inputs, 2u
    %12:f32 = access %11, 3u
    %13:f32 = div 1.0f, %12
    %14:vec3<f32> = swizzle %11, xyz
    %15:vec4<f32> = construct %14, %13
    %16:f32 = access %inputs, 0u
    %17:f32 = access %inputs, 1u
    %18:void = call %foo_inner, %10, %15, %16, %17
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("front_facing"),
                                     ty.bool_(),
                                     core::IOAttributes{
                                         .builtin = core::BuiltinValue::kFrontFacing,
                                     },
                                 },
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4f(),
                                     core::IOAttributes{
                                         .builtin = core::BuiltinValue::kPosition,
                                         .invariant = true,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         .location = 0u,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color2"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         .location = 1u,
                                         .interpolation =
                                             core::Interpolation{
                                                 core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample,
                                             },
                                     },
                                 },
                             });

    auto* str_param = b.FunctionParam("inputs", str_ty);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({str_param});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(b.Access(ty.bool_(), str_param, 0_i));
        b.Append(ifelse->True(), [&] {
            auto* position = b.Access(ty.vec4f(), str_param, 1_i);
            auto* color1 = b.Access(ty.f32(), str_param, 2_i);
            auto* color2 = b.Access(ty.f32(), str_param, 3_i);
            b.Multiply(position, b.Add(color1, color2));
            b.ExitIf(ifelse);
        });
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  front_facing:bool @offset(0), @builtin(front_facing)
  position:vec4<f32> @offset(16), @invariant, @builtin(position)
  color1:f32 @offset(32), @location(0)
  color2:f32 @offset(36), @location(1), @interpolate(linear, sample)
}

%foo = @fragment func(%inputs:Inputs):void {
  $B1: {
    %3:bool = access %inputs, 0i
    if %3 [t: $B2] {  # if_1
      $B2: {  # true
        %4:vec4<f32> = access %inputs, 1i
        %5:f32 = access %inputs, 2i
        %6:f32 = access %inputs, 3i
        %7:f32 = add %5, %6
        %8:vec4<f32> = mul %4, %7
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  front_facing:bool @offset(0)
  position:vec4<f32> @offset(16)
  color1:f32 @offset(32)
  color2:f32 @offset(36)
}

foo_inputs = struct @align(16) {
  Inputs_color1:f32 @offset(0), @location(0)
  Inputs_color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
  Inputs_position:vec4<f32> @offset(16), @invariant, @builtin(position)
  Inputs_front_facing:bool @offset(32), @builtin(front_facing)
}

%foo_inner = func(%inputs:Inputs):void {
  $B1: {
    %3:bool = access %inputs, 0i
    if %3 [t: $B2] {  # if_1
      $B2: {  # true
        %4:vec4<f32> = access %inputs, 1i
        %5:f32 = access %inputs, 2i
        %6:f32 = access %inputs, 3i
        %7:f32 = add %5, %6
        %8:vec4<f32> = mul %4, %7
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func(%inputs_1:foo_inputs):void {  # %inputs_1: 'inputs'
  $B3: {
    %11:bool = access %inputs_1, 3u
    %12:vec4<f32> = access %inputs_1, 2u
    %13:f32 = access %12, 3u
    %14:f32 = div 1.0f, %13
    %15:vec3<f32> = swizzle %12, xyz
    %16:vec4<f32> = construct %15, %14
    %17:f32 = access %inputs_1, 0u
    %18:f32 = access %inputs_1, 1u
    %19:Inputs = construct %11, %16, %17, %18
    %20:void = call %foo_inner, %19
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Mixed) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Inputs"), {
                                                 {
                                                     mod.symbols.New("position"),
                                                     ty.vec4f(),
                                                     core::IOAttributes{
                                                         .builtin = core::BuiltinValue::kPosition,
                                                         .invariant = true,
                                                     },
                                                 },
                                                 {
                                                     mod.symbols.New("color1"),
                                                     ty.f32(),
                                                     core::IOAttributes{
                                                         .location = 0u,
                                                     },
                                                 },
                                             });

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);

    auto* str_param = b.FunctionParam("inputs", str_ty);

    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample});

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({front_facing, str_param, color2});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {
            auto* position = b.Access(ty.vec4f(), str_param, 0_i);
            auto* color1 = b.Access(ty.f32(), str_param, 1_i);
            b.Multiply(position, b.Add(color1, color2));
            b.ExitIf(ifelse);
        });
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  position:vec4<f32> @offset(0), @invariant, @builtin(position)
  color1:f32 @offset(16), @location(0)
}

%foo = @fragment func(%front_facing:bool [@front_facing], %inputs:Inputs, %color2:f32 [@location(1), @interpolate(linear, sample)]):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %5:vec4<f32> = access %inputs, 0i
        %6:f32 = access %inputs, 1i
        %7:f32 = add %6, %color2
        %8:vec4<f32> = mul %5, %7
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  color1:f32 @offset(16)
}

foo_inputs = struct @align(16) {
  Inputs_color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
  Inputs_position:vec4<f32> @offset(16), @invariant, @builtin(position)
  front_facing:bool @offset(32), @builtin(front_facing)
}

%foo_inner = func(%front_facing:bool, %inputs:Inputs, %color2:f32):void {
  $B1: {
    if %front_facing [t: $B2] {  # if_1
      $B2: {  # true
        %5:vec4<f32> = access %inputs, 0i
        %6:f32 = access %inputs, 1i
        %7:f32 = add %6, %color2
        %8:vec4<f32> = mul %5, %7
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func(%inputs_1:foo_inputs):void {  # %inputs_1: 'inputs'
  $B3: {
    %11:bool = access %inputs_1, 3u
    %12:vec4<f32> = access %inputs_1, 2u
    %13:f32 = access %12, 3u
    %14:f32 = div 1.0f, %13
    %15:vec3<f32> = swizzle %12, xyz
    %16:vec4<f32> = construct %15, %14
    %17:f32 = access %inputs_1, 0u
    %18:Inputs = construct %16, %17
    %19:f32 = access %inputs_1, 1u
    %20:void = call %foo_inner, %11, %18, %19
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOReturnValue_NonStructBuiltin) {
    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    ep->SetReturnInvariant(true);

    b.Append(ep->Block(), [&] { b.Return(ep, b.Construct(ty.vec4f(), 0.5_f)); });

    auto* src = R"(
%foo = @vertex func():vec4<f32> [@invariant, @position] {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @invariant, @builtin(position)
}

%foo_inner = func():vec4<f32> {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %4:vec4<f32> = call %foo_inner
    %5:foo_outputs = construct %4
    ret %5
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOReturnValue_NonStructLocation) {
    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kFragment);
    ep->SetReturnLocation(1u);

    b.Append(ep->Block(), [&] { b.Return(ep, b.Construct(ty.vec4f(), 0.5_f)); });

    auto* src = R"(
%foo = @fragment func():vec4<f32> [@location(1)] {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @location(1)
}

%foo_inner = func():vec4<f32> {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
%foo = @fragment func():foo_outputs {
  $B2: {
    %4:vec4<f32> = call %foo_inner
    %5:foo_outputs = construct %4
    ret %5
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOReturnValue_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4f(),
                                     core::IOAttributes{
                                         .builtin = core::BuiltinValue::kPosition,
                                         .invariant = true,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         .location = 0u,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color2"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         .location = 1u,
                                         .interpolation =
                                             core::Interpolation{
                                                 core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample,
                                             },
                                     },
                                 },
                             });

    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {
        b.Return(ep, b.Construct(str_ty, b.Construct(ty.vec4f(), 0_f), 0.25_f, 0.75_f));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @invariant, @builtin(position)
  color1:f32 @offset(16), @location(0)
  color2:f32 @offset(20), @location(1), @interpolate(linear, sample)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.0f
    %3:Outputs = construct %2, 0.25f, 0.75f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  color1:f32 @offset(16)
  color2:f32 @offset(20)
}

foo_outputs = struct @align(16) {
  Outputs_color1:f32 @offset(0), @location(0)
  Outputs_color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
  Outputs_position:vec4<f32> @offset(16), @invariant, @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.0f
    %3:Outputs = construct %2, 0.25f, 0.75f
    ret %3
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %5:Outputs = call %foo_inner
    %6:vec4<f32> = access %5, 0u
    %7:f32 = access %5, 1u
    %8:f32 = access %5, 2u
    %9:foo_outputs = construct %7, %8, %6
    ret %9
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOReturnValue_DualSourceBlending) {
    auto* str_ty = ty.Struct(mod.symbols.New("Output"), {
                                                            {
                                                                mod.symbols.New("color1"),
                                                                ty.f32(),
                                                                core::IOAttributes{
                                                                    .location = 0u,
                                                                    .blend_src = 0u,
                                                                },
                                                            },
                                                            {
                                                                mod.symbols.New("color2"),
                                                                ty.f32(),
                                                                core::IOAttributes{
                                                                    .location = 0u,
                                                                    .blend_src = 1u,
                                                                },
                                                            },
                                                        });

    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] { b.Return(ep, b.Construct(str_ty, 0.25_f, 0.75_f)); });

    auto* src = R"(
Output = struct @align(4) {
  color1:f32 @offset(0), @location(0), @blend_src(0)
  color2:f32 @offset(4), @location(0), @blend_src(1)
}

%foo = @fragment func():Output {
  $B1: {
    %2:Output = construct 0.25f, 0.75f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Output = struct @align(4) {
  color1:f32 @offset(0)
  color2:f32 @offset(4)
}

foo_outputs = struct @align(4) {
  Output_color1:f32 @offset(0), @location(0), @blend_src(0)
  Output_color2:f32 @offset(4), @location(0), @blend_src(1)
}

%foo_inner = func():Output {
  $B1: {
    %2:Output = construct 0.25f, 0.75f
    ret %2
  }
}
%foo = @fragment func():foo_outputs {
  $B2: {
    %4:Output = call %foo_inner
    %5:f32 = access %4, 0u
    %6:f32 = access %4, 1u
    %7:foo_outputs = construct %5, %6
    ret %7
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOStruct_SharedWithBuffer) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"), {
                                                  {
                                                      mod.symbols.New("position"),
                                                      ty.vec4f(),
                                                      core::IOAttributes{
                                                          .builtin = core::BuiltinValue::kPosition,
                                                      },
                                                  },
                                                  {
                                                      mod.symbols.New("color"),
                                                      ty.vec3f(),
                                                      core::IOAttributes{
                                                          .location = 0u,
                                                      },
                                                  },
                                              });

    auto* var = b.Var(ty.ptr(storage, str_ty, read));
    var->SetBindingPoint(0, 0);

    auto* buffer = mod.root_block->Append(var);

    auto* ep = b.Function("vert", str_ty, core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] { b.Return(ep, b.Load(buffer)); });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color:vec3<f32> @offset(16), @location(0)
}

$B1: {  # root
  %1:ptr<storage, Outputs, read> = var undef @binding_point(0, 0)
}

%vert = @vertex func():Outputs {
  $B2: {
    %3:Outputs = load %1
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  color:vec3<f32> @offset(16)
}

vert_outputs = struct @align(16) {
  Outputs_color:vec3<f32> @offset(0), @location(0)
  Outputs_position:vec4<f32> @offset(16), @builtin(position)
}

$B1: {  # root
  %1:ptr<storage, Outputs, read> = var undef @binding_point(0, 0)
}

%vert_inner = func():Outputs {
  $B2: {
    %3:Outputs = load %1
    ret %3
  }
}
%vert = @vertex func():vert_outputs {
  $B3: {
    %5:Outputs = call %vert_inner
    %6:vec4<f32> = access %5, 0u
    %7:vec3<f32> = access %5, 1u
    %8:vert_outputs = construct %7, %6
    ret %8
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that IO attributes are stripped from structures that are not used for the shader interface.
TEST_F(HlslWriterTransformTest, ShaderIOStructWithAttributes_NotUsedForInterface) {
    auto* vec4f = ty.vec4f();
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"), {
                                                  {
                                                      mod.symbols.New("position"),
                                                      vec4f,
                                                      core::IOAttributes{
                                                          .builtin = core::BuiltinValue::kPosition,
                                                      },
                                                  },
                                                  {
                                                      mod.symbols.New("color"),
                                                      vec4f,
                                                      core::IOAttributes{
                                                          .location = 0u,
                                                      },
                                                  },
                                              });

    auto* var = b.Var(ty.ptr(storage, str_ty, core::Access::kReadWrite));
    var->SetBindingPoint(0, 0);

    auto* buffer = mod.root_block->Append(var);

    auto* ep = b.Function("frag", ty.void_(), core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {
        b.Store(buffer, b.Construct(str_ty));
        b.Return(ep);
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color:vec4<f32> @offset(16), @location(0)
}

$B1: {  # root
  %1:ptr<storage, Outputs, read_write> = var undef @binding_point(0, 0)
}

%frag = @fragment func():void {
  $B2: {
    %3:Outputs = construct
    store %1, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  color:vec4<f32> @offset(16)
}

$B1: {  # root
  %1:ptr<storage, Outputs, read_write> = var undef @binding_point(0, 0)
}

%frag = @fragment func():void {
  $B2: {
    %3:Outputs = construct
    store %1, %3
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOCompute) {
    auto* invoc = b.FunctionParam("invoc_id", ty.vec3u());
    invoc->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* ep = b.ComputeFunction("cmp");
    ep->SetParams({invoc});

    b.Append(ep->Block(), [&] {
        b.Let("a", invoc);
        b.Return(ep);
    });

    auto* src = R"(
%cmp = @compute @workgroup_size(1u, 1u, 1u) func(%invoc_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %a:vec3<u32> = let %invoc_id
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
cmp_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
}

%cmp_inner = func(%invoc_id:vec3<u32>):void {
  $B1: {
    %a:vec3<u32> = let %invoc_id
    ret
  }
}
%cmp = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:cmp_inputs):void {
  $B2: {
    %6:vec3<u32> = access %inputs, 0u
    %7:void = call %cmp_inner, %6
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Subgroup_NonStruct) {
    auto* subgroup_invocation_id = b.FunctionParam("id", ty.u32());
    subgroup_invocation_id->SetBuiltin(core::BuiltinValue::kSubgroupInvocationId);

    auto* subgroup_size = b.FunctionParam("size", ty.u32());
    subgroup_size->SetBuiltin(core::BuiltinValue::kSubgroupSize);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({subgroup_invocation_id, subgroup_size});

    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(subgroup_invocation_id, subgroup_size));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @fragment func(%id:u32 [@subgroup_invocation_id], %size:u32 [@subgroup_size]):void {
  $B1: {
    %4:u32 = mul %id, %size
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo_inner = func(%id:u32, %size:u32):void {
  $B1: {
    %4:u32 = mul %id, %size
    %x:u32 = let %4
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %7:u32 = hlsl.WaveGetLaneIndex
    %8:u32 = hlsl.WaveGetLaneCount
    %9:void = call %foo_inner, %7, %8
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Subgroup_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("id"),
                                     ty.u32(),
                                     core::IOAttributes{
                                         .builtin = core::BuiltinValue::kSubgroupInvocationId,
                                     },
                                 },
                                 {
                                     mod.symbols.New("size"),
                                     ty.u32(),
                                     core::IOAttributes{
                                         .builtin = core::BuiltinValue::kSubgroupSize,
                                     },
                                 },
                             });

    auto* str_param = b.FunctionParam("inputs", str_ty);

    auto* ep = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({str_param});

    b.Append(ep->Block(), [&] {
        auto* subgroup_invocation_id = b.Access(ty.u32(), str_param, 0_i);
        auto* subgroup_size = b.Access(ty.u32(), str_param, 1_i);
        b.Let("x", b.Multiply(subgroup_invocation_id, subgroup_size));
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(4) {
  id:u32 @offset(0), @builtin(subgroup_invocation_id)
  size:u32 @offset(4), @builtin(subgroup_size)
}

%foo = @fragment func(%inputs:Inputs):void {
  $B1: {
    %3:u32 = access %inputs, 0i
    %4:u32 = access %inputs, 1i
    %5:u32 = mul %3, %4
    %x:u32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  id:u32 @offset(0)
  size:u32 @offset(4)
}

%foo_inner = func(%inputs:Inputs):void {
  $B1: {
    %3:u32 = access %inputs, 0i
    %4:u32 = access %inputs, 1i
    %5:u32 = mul %3, %4
    %x:u32 = let %5
    ret
  }
}
%foo = @fragment func():void {
  $B2: {
    %8:u32 = hlsl.WaveGetLaneIndex
    %9:u32 = hlsl.WaveGetLaneCount
    %10:Inputs = construct %8, %9
    %11:void = call %foo_inner, %10
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Subgroup_WithNonSubgroupParamsFirst) {
    auto* invocation_id = b.FunctionParam("invoc_id", ty.vec3u());
    invocation_id->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* subgroup_invocation_id = b.FunctionParam("id", ty.u32());
    subgroup_invocation_id->SetBuiltin(core::BuiltinValue::kSubgroupInvocationId);

    auto* subgroup_size = b.FunctionParam("size", ty.u32());
    subgroup_size->SetBuiltin(core::BuiltinValue::kSubgroupSize);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({invocation_id, subgroup_invocation_id, subgroup_size});

    b.Append(ep->Block(), [&] {
        auto* x = b.Let("x", b.Multiply(subgroup_invocation_id, subgroup_size));
        b.Let("y", b.Add(x, b.Access(ty.u32(), invocation_id, 0_u)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%invoc_id:vec3<u32> [@local_invocation_id], %id:u32 [@subgroup_invocation_id], %size:u32 [@subgroup_size]):void {
  $B1: {
    %5:u32 = mul %id, %size
    %x:u32 = let %5
    %7:u32 = access %invoc_id, 0u
    %8:u32 = add %x, %7
    %y:u32 = let %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
}

%foo_inner = func(%invoc_id:vec3<u32>, %id:u32, %size:u32):void {
  $B1: {
    %5:u32 = mul %id, %size
    %x:u32 = let %5
    %7:u32 = access %invoc_id, 0u
    %8:u32 = add %x, %7
    %y:u32 = let %8
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B2: {
    %12:vec3<u32> = access %inputs, 0u
    %13:u32 = hlsl.WaveGetLaneIndex
    %14:u32 = hlsl.WaveGetLaneCount
    %15:void = call %foo_inner, %12, %13, %14
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Subgroup_WithNonSubgroupParamsLast) {
    auto* subgroup_invocation_id = b.FunctionParam("id", ty.u32());
    subgroup_invocation_id->SetBuiltin(core::BuiltinValue::kSubgroupInvocationId);

    auto* subgroup_size = b.FunctionParam("size", ty.u32());
    subgroup_size->SetBuiltin(core::BuiltinValue::kSubgroupSize);

    auto* invocation_id = b.FunctionParam("invoc_id", ty.vec3u());
    invocation_id->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({subgroup_invocation_id, subgroup_size, invocation_id});

    b.Append(ep->Block(), [&] {
        auto* x = b.Let("x", b.Multiply(subgroup_invocation_id, subgroup_size));
        b.Let("y", b.Add(x, b.Access(ty.u32(), invocation_id, 0_u)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@subgroup_invocation_id], %size:u32 [@subgroup_size], %invoc_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %5:u32 = mul %id, %size
    %x:u32 = let %5
    %7:u32 = access %invoc_id, 0u
    %8:u32 = add %x, %7
    %y:u32 = let %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
}

%foo_inner = func(%id:u32, %size:u32, %invoc_id:vec3<u32>):void {
  $B1: {
    %5:u32 = mul %id, %size
    %x:u32 = let %5
    %7:u32 = access %invoc_id, 0u
    %8:u32 = add %x, %7
    %y:u32 = let %8
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B2: {
    %12:u32 = hlsl.WaveGetLaneIndex
    %13:u32 = hlsl.WaveGetLaneCount
    %14:vec3<u32> = access %inputs, 0u
    %15:void = call %foo_inner, %12, %13, %14
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Subgroup_WithNonSubgroupParamsMiddle) {
    auto* subgroup_invocation_id = b.FunctionParam("id", ty.u32());
    subgroup_invocation_id->SetBuiltin(core::BuiltinValue::kSubgroupInvocationId);

    auto* invocation_id = b.FunctionParam("invoc_id", ty.vec3u());
    invocation_id->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* subgroup_size = b.FunctionParam("size", ty.u32());
    subgroup_size->SetBuiltin(core::BuiltinValue::kSubgroupSize);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({subgroup_invocation_id, invocation_id, subgroup_size});

    b.Append(ep->Block(), [&] {
        auto* x = b.Let("x", b.Multiply(subgroup_invocation_id, subgroup_size));
        b.Let("y", b.Add(x, b.Access(ty.u32(), invocation_id, 0_u)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@subgroup_invocation_id], %invoc_id:vec3<u32> [@local_invocation_id], %size:u32 [@subgroup_size]):void {
  $B1: {
    %5:u32 = mul %id, %size
    %x:u32 = let %5
    %7:u32 = access %invoc_id, 0u
    %8:u32 = add %x, %7
    %y:u32 = let %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
}

%foo_inner = func(%id:u32, %invoc_id:vec3<u32>, %size:u32):void {
  $B1: {
    %5:u32 = mul %id, %size
    %x:u32 = let %5
    %7:u32 = access %invoc_id, 0u
    %8:u32 = add %x, %7
    %y:u32 = let %8
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B2: {
    %12:u32 = hlsl.WaveGetLaneIndex
    %13:vec3<u32> = access %inputs, 0u
    %14:u32 = hlsl.WaveGetLaneCount
    %15:void = call %foo_inner, %12, %13, %14
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_NonStruct) {
    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({num_workgroups});

    b.Append(ep->Block(), [&] {
        b.Multiply(num_workgroups, num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%num_wgs:vec3<u32> [@num_workgroups]):void {
  $B1: {
    %3:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo_inner = func(%num_wgs:vec3<u32>):void {
  $B2: {
    %4:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %6:vec3<u32> = load %tint_num_workgroups
    %7:void = call %foo_inner, %6
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Immediate_NumWorkgroups_NonStruct) {
    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({num_workgroups});

    b.Append(ep->Block(), [&] {
        b.Multiply(num_workgroups, num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%num_wgs:vec3<u32> [@num_workgroups]):void {
  $B1: {
    %3:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_num_workgroups_start_offset:vec3<u32> @offset(0)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func(%num_wgs:vec3<u32>):void {
  $B2: {
    %4:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %6:ptr<immediate, vec3<u32>, read> = access %tint_immediate_data, 0u
    %7:vec3<u32> = load %6
    %8:void = call %foo_inner, %7
    ret
  }
}
)";

    // Refs to Raise() steps to setup push constant layout.
    constexpr uint32_t num_workgroups_offset = 0u;

    core::ir::transform::PrepareImmediateDataConfig prepare_immediate_data_layout_config;
    ASSERT_EQ(
        prepare_immediate_data_layout_config.AddInternalImmediateData(
            num_workgroups_offset, mod.symbols.New("tint_num_workgroups_start_offset"), ty.vec3u()),
        Success);
    auto layout =
        core::ir::transform::PrepareImmediateData(mod, prepare_immediate_data_layout_config);

    ShaderIOConfig config{layout.Get()};
    config.num_workgroups_start_offset = num_workgroups_offset;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("num_wgs"),
                                     ty.vec3u(),
                                     core::IOAttributes{
                                         .builtin = core::BuiltinValue::kNumWorkgroups,
                                     },
                                 },
                             });

    auto* str_param = b.FunctionParam("inputs", str_ty);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({str_param});

    b.Append(ep->Block(), [&] {
        auto* num_workgroups = b.Access(ty.vec3u(), str_param, 0_i);
        b.Multiply(num_workgroups, num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  num_wgs:vec3<u32> @offset(0), @builtin(num_workgroups)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:Inputs):void {
  $B1: {
    %3:vec3<u32> = access %inputs, 0i
    %4:vec3<u32> = mul %3, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  num_wgs:vec3<u32> @offset(0)
}

$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo_inner = func(%inputs:Inputs):void {
  $B2: {
    %4:vec3<u32> = access %inputs, 0i
    %5:vec3<u32> = mul %4, %4
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:vec3<u32> = load %tint_num_workgroups
    %8:Inputs = construct %7
    %9:void = call %foo_inner, %8
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Immediate_NumWorkgroups_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("num_wgs"),
                                     ty.vec3u(),
                                     core::IOAttributes{
                                         .builtin = core::BuiltinValue::kNumWorkgroups,
                                     },
                                 },
                             });

    auto* str_param = b.FunctionParam("inputs", str_ty);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({str_param});

    b.Append(ep->Block(), [&] {
        auto* num_workgroups = b.Access(ty.vec3u(), str_param, 0_i);
        b.Multiply(num_workgroups, num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(16) {
  num_wgs:vec3<u32> @offset(0), @builtin(num_workgroups)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:Inputs):void {
  $B1: {
    %3:vec3<u32> = access %inputs, 0i
    %4:vec3<u32> = mul %3, %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(16) {
  num_wgs:vec3<u32> @offset(0)
}

tint_immediate_data_struct = struct @align(16), @block {
  tint_num_workgroups_start_offset:vec3<u32> @offset(0)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func(%inputs:Inputs):void {
  $B2: {
    %4:vec3<u32> = access %inputs, 0i
    %5:vec3<u32> = mul %4, %4
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:ptr<immediate, vec3<u32>, read> = access %tint_immediate_data, 0u
    %8:vec3<u32> = load %7
    %9:Inputs = construct %8
    %10:void = call %foo_inner, %9
    ret
  }
}
)";

    // Refs to Raise() steps to setup push constant layout.
    constexpr uint32_t num_workgroups_offset = 0u;

    core::ir::transform::PrepareImmediateDataConfig prepare_immediate_data_layout_config;
    ASSERT_EQ(
        prepare_immediate_data_layout_config.AddInternalImmediateData(
            num_workgroups_offset, mod.symbols.New("tint_num_workgroups_start_offset"), ty.vec3u()),
        Success);
    auto layout =
        core::ir::transform::PrepareImmediateData(mod, prepare_immediate_data_layout_config);

    ShaderIOConfig config{layout.Get()};
    config.num_workgroups_start_offset = num_workgroups_offset;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_ExplicitBinding) {
    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({num_workgroups});

    b.Append(ep->Block(), [&] {
        b.Multiply(num_workgroups, num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%num_wgs:vec3<u32> [@num_workgroups]):void {
  $B1: {
    %3:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(1, 23)
}

%foo_inner = func(%num_wgs:vec3<u32>):void {
  $B2: {
    %4:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %6:vec3<u32> = load %tint_num_workgroups
    %7:void = call %foo_inner, %6
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.num_workgroups_binding = {1u, 23u};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_AutoBinding) {
    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({num_workgroups});

    b.Append(ep->Block(), [&] {
        b.Multiply(num_workgroups, num_workgroups);
        b.Return(ep);
    });

    b.Append(mod.root_block, [&] {
        for (uint32_t group = 0; group < 10; ++group) {
            auto* v = b.Var<core::AddressSpace::kStorage, i32>();
            v->SetBindingPoint(group, group + 1u);
        }
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<storage, i32, read_write> = var undef @binding_point(0, 1)
  %2:ptr<storage, i32, read_write> = var undef @binding_point(1, 2)
  %3:ptr<storage, i32, read_write> = var undef @binding_point(2, 3)
  %4:ptr<storage, i32, read_write> = var undef @binding_point(3, 4)
  %5:ptr<storage, i32, read_write> = var undef @binding_point(4, 5)
  %6:ptr<storage, i32, read_write> = var undef @binding_point(5, 6)
  %7:ptr<storage, i32, read_write> = var undef @binding_point(6, 7)
  %8:ptr<storage, i32, read_write> = var undef @binding_point(7, 8)
  %9:ptr<storage, i32, read_write> = var undef @binding_point(8, 9)
  %10:ptr<storage, i32, read_write> = var undef @binding_point(9, 10)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func(%num_wgs:vec3<u32> [@num_workgroups]):void {
  $B2: {
    %13:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %1:ptr<storage, i32, read_write> = var undef @binding_point(0, 1)
  %2:ptr<storage, i32, read_write> = var undef @binding_point(1, 2)
  %3:ptr<storage, i32, read_write> = var undef @binding_point(2, 3)
  %4:ptr<storage, i32, read_write> = var undef @binding_point(3, 4)
  %5:ptr<storage, i32, read_write> = var undef @binding_point(4, 5)
  %6:ptr<storage, i32, read_write> = var undef @binding_point(5, 6)
  %7:ptr<storage, i32, read_write> = var undef @binding_point(6, 7)
  %8:ptr<storage, i32, read_write> = var undef @binding_point(7, 8)
  %9:ptr<storage, i32, read_write> = var undef @binding_point(8, 9)
  %10:ptr<storage, i32, read_write> = var undef @binding_point(9, 10)
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(10, 0)
}

%foo_inner = func(%num_wgs:vec3<u32>):void {
  $B2: {
    %14:vec3<u32> = mul %num_wgs, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %16:vec3<u32> = load %tint_num_workgroups
    %17:void = call %foo_inner, %16
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_WithNonWorkgroupParamFirst) {
    auto* invocation_id = b.FunctionParam("invoc_id", ty.vec3u());
    invocation_id->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({invocation_id, num_workgroups});

    b.Append(ep->Block(), [&] {
        b.Multiply(b.Access(ty.u32(), invocation_id, 0_u), num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%invoc_id:vec3<u32> [@local_invocation_id], %num_wgs:vec3<u32> [@num_workgroups]):void {
  $B1: {
    %4:u32 = access %invoc_id, 0u
    %5:vec3<u32> = mul %4, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
}

$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo_inner = func(%invoc_id:vec3<u32>, %num_wgs:vec3<u32>):void {
  $B2: {
    %5:u32 = access %invoc_id, 0u
    %6:vec3<u32> = mul %5, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %9:vec3<u32> = access %inputs, 0u
    %10:vec3<u32> = load %tint_num_workgroups
    %11:void = call %foo_inner, %9, %10
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest,
       ShaderIOParameters_Immediate_NumWorkgroups_WithNonWorkgroupParamFirst) {
    auto* invocation_id = b.FunctionParam("invoc_id", ty.vec3u());
    invocation_id->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({invocation_id, num_workgroups});

    b.Append(ep->Block(), [&] {
        b.Multiply(b.Access(ty.u32(), invocation_id, 0_u), num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%invoc_id:vec3<u32> [@local_invocation_id], %num_wgs:vec3<u32> [@num_workgroups]):void {
  $B1: {
    %4:u32 = access %invoc_id, 0u
    %5:vec3<u32> = mul %4, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_num_workgroups_start_offset:vec3<u32> @offset(0)
}

foo_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func(%invoc_id:vec3<u32>, %num_wgs:vec3<u32>):void {
  $B2: {
    %5:u32 = access %invoc_id, 0u
    %6:vec3<u32> = mul %5, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %9:vec3<u32> = access %inputs, 0u
    %10:ptr<immediate, vec3<u32>, read> = access %tint_immediate_data, 0u
    %11:vec3<u32> = load %10
    %12:void = call %foo_inner, %9, %11
    ret
  }
}
)";

    // Refs to Raise() steps to setup push constant layout.
    constexpr uint32_t num_workgroups_offset = 0u;

    core::ir::transform::PrepareImmediateDataConfig prepare_immediate_data_layout_config;
    ASSERT_EQ(
        prepare_immediate_data_layout_config.AddInternalImmediateData(
            num_workgroups_offset, mod.symbols.New("tint_num_workgroups_start_offset"), ty.vec3u()),
        Success);
    auto layout =
        core::ir::transform::PrepareImmediateData(mod, prepare_immediate_data_layout_config);

    ShaderIOConfig config{layout.Get()};
    config.num_workgroups_start_offset = num_workgroups_offset;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroups_WithNonWorkgroupParamLast) {
    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* invocation_id = b.FunctionParam("invoc_id", ty.vec3u());
    invocation_id->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({num_workgroups, invocation_id});

    b.Append(ep->Block(), [&] {
        b.Multiply(b.Access(ty.u32(), invocation_id, 0_u), num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%num_wgs:vec3<u32> [@num_workgroups], %invoc_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %4:u32 = access %invoc_id, 0u
    %5:vec3<u32> = mul %4, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
}

$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo_inner = func(%num_wgs:vec3<u32>, %invoc_id:vec3<u32>):void {
  $B2: {
    %5:u32 = access %invoc_id, 0u
    %6:vec3<u32> = mul %5, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %9:vec3<u32> = load %tint_num_workgroups
    %10:vec3<u32> = access %inputs, 0u
    %11:void = call %foo_inner, %9, %10
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest,
       ShaderIOParameters_Immediate_NumWorkgroups_WithNonWorkgroupParamLast) {
    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* invocation_id = b.FunctionParam("invoc_id", ty.vec3u());
    invocation_id->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({num_workgroups, invocation_id});

    b.Append(ep->Block(), [&] {
        b.Multiply(b.Access(ty.u32(), invocation_id, 0_u), num_workgroups);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%num_wgs:vec3<u32> [@num_workgroups], %invoc_id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    %4:u32 = access %invoc_id, 0u
    %5:vec3<u32> = mul %4, %num_wgs
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_num_workgroups_start_offset:vec3<u32> @offset(0)
}

foo_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func(%num_wgs:vec3<u32>, %invoc_id:vec3<u32>):void {
  $B2: {
    %5:u32 = access %invoc_id, 0u
    %6:vec3<u32> = mul %5, %num_wgs
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %9:ptr<immediate, vec3<u32>, read> = access %tint_immediate_data, 0u
    %10:vec3<u32> = load %9
    %11:vec3<u32> = access %inputs, 0u
    %12:void = call %foo_inner, %10, %11
    ret
  }
}
)";

    // Refs to Raise() steps to setup push constant layout.
    constexpr uint32_t num_workgroups_offset = 0u;

    core::ir::transform::PrepareImmediateDataConfig prepare_immediate_data_layout_config;
    ASSERT_EQ(
        prepare_immediate_data_layout_config.AddInternalImmediateData(
            num_workgroups_offset, mod.symbols.New("tint_num_workgroups_start_offset"), ty.vec3u()),
        Success);
    auto layout =
        core::ir::transform::PrepareImmediateData(mod, prepare_immediate_data_layout_config);

    ShaderIOConfig config{layout.Get()};
    config.num_workgroups_start_offset = num_workgroups_offset;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumWorkgroupsAndSubgroups_Mixed) {
    auto* invocation_id = b.FunctionParam("invoc_id", ty.vec3u());
    invocation_id->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* invocation_index = b.FunctionParam("invoc_index", ty.u32());
    invocation_index->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);

    auto* subgroup_invocation_id = b.FunctionParam("sg_id", ty.u32());
    subgroup_invocation_id->SetBuiltin(core::BuiltinValue::kSubgroupInvocationId);

    auto* global_invocation_id = b.FunctionParam("glob_id", ty.vec3u());
    global_invocation_id->SetBuiltin(core::BuiltinValue::kGlobalInvocationId);

    auto* subgroup_size = b.FunctionParam("sg_size", ty.u32());
    subgroup_size->SetBuiltin(core::BuiltinValue::kSubgroupSize);

    auto* workgroup_id = b.FunctionParam("wg_id", ty.vec3u());
    workgroup_id->SetBuiltin(core::BuiltinValue::kWorkgroupId);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({invocation_id, num_workgroups, invocation_index, subgroup_invocation_id,
                   global_invocation_id, subgroup_size, workgroup_id});

    b.Append(ep->Block(), [&] {
        b.Let("l_invoc_id", invocation_id);
        b.Let("l_num_wgs", num_workgroups);
        b.Let("l_invoc_index", invocation_index);
        b.Let("l_sg_id", subgroup_invocation_id);
        b.Let("l_glob_id", global_invocation_id);
        b.Let("l_sg_size", subgroup_size);
        b.Let("l_wg_id", workgroup_id);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%invoc_id:vec3<u32> [@local_invocation_id], %num_wgs:vec3<u32> [@num_workgroups], %invoc_index:u32 [@local_invocation_index], %sg_id:u32 [@subgroup_invocation_id], %glob_id:vec3<u32> [@global_invocation_id], %sg_size:u32 [@subgroup_size], %wg_id:vec3<u32> [@workgroup_id]):void {
  $B1: {
    %l_invoc_id:vec3<u32> = let %invoc_id
    %l_num_wgs:vec3<u32> = let %num_wgs
    %l_invoc_index:u32 = let %invoc_index
    %l_sg_id:u32 = let %sg_id
    %l_glob_id:vec3<u32> = let %glob_id
    %l_sg_size:u32 = let %sg_size
    %l_wg_id:vec3<u32> = let %wg_id
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
  invoc_index:u32 @offset(12), @builtin(local_invocation_index)
  glob_id:vec3<u32> @offset(16), @builtin(global_invocation_id)
  wg_id:vec3<u32> @offset(32), @builtin(workgroup_id)
}

$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo_inner = func(%invoc_id:vec3<u32>, %num_wgs:vec3<u32>, %invoc_index:u32, %sg_id:u32, %glob_id:vec3<u32>, %sg_size:u32, %wg_id:vec3<u32>):void {
  $B2: {
    %l_invoc_id:vec3<u32> = let %invoc_id
    %l_num_wgs:vec3<u32> = let %num_wgs
    %l_invoc_index:u32 = let %invoc_index
    %l_sg_id:u32 = let %sg_id
    %l_glob_id:vec3<u32> = let %glob_id
    %l_sg_size:u32 = let %sg_size
    %l_wg_id:vec3<u32> = let %wg_id
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %19:vec3<u32> = access %inputs, 0u
    %20:vec3<u32> = load %tint_num_workgroups
    %21:u32 = access %inputs, 1u
    %22:u32 = hlsl.WaveGetLaneIndex
    %23:vec3<u32> = access %inputs, 2u
    %24:u32 = hlsl.WaveGetLaneCount
    %25:vec3<u32> = access %inputs, 3u
    %26:void = call %foo_inner, %19, %20, %21, %22, %23, %24, %25
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Immediate_NumWorkgroupsAndSubgroups_Mixed) {
    auto* invocation_id = b.FunctionParam("invoc_id", ty.vec3u());
    invocation_id->SetBuiltin(core::BuiltinValue::kLocalInvocationId);

    auto* num_workgroups = b.FunctionParam("num_wgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* invocation_index = b.FunctionParam("invoc_index", ty.u32());
    invocation_index->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);

    auto* subgroup_invocation_id = b.FunctionParam("sg_id", ty.u32());
    subgroup_invocation_id->SetBuiltin(core::BuiltinValue::kSubgroupInvocationId);

    auto* global_invocation_id = b.FunctionParam("glob_id", ty.vec3u());
    global_invocation_id->SetBuiltin(core::BuiltinValue::kGlobalInvocationId);

    auto* subgroup_size = b.FunctionParam("sg_size", ty.u32());
    subgroup_size->SetBuiltin(core::BuiltinValue::kSubgroupSize);

    auto* workgroup_id = b.FunctionParam("wg_id", ty.vec3u());
    workgroup_id->SetBuiltin(core::BuiltinValue::kWorkgroupId);

    auto* ep = b.ComputeFunction("foo");
    ep->SetParams({invocation_id, num_workgroups, invocation_index, subgroup_invocation_id,
                   global_invocation_id, subgroup_size, workgroup_id});

    b.Append(ep->Block(), [&] {
        b.Let("l_invoc_id", invocation_id);
        b.Let("l_num_wgs", num_workgroups);
        b.Let("l_invoc_index", invocation_index);
        b.Let("l_sg_id", subgroup_invocation_id);
        b.Let("l_glob_id", global_invocation_id);
        b.Let("l_sg_size", subgroup_size);
        b.Let("l_wg_id", workgroup_id);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%invoc_id:vec3<u32> [@local_invocation_id], %num_wgs:vec3<u32> [@num_workgroups], %invoc_index:u32 [@local_invocation_index], %sg_id:u32 [@subgroup_invocation_id], %glob_id:vec3<u32> [@global_invocation_id], %sg_size:u32 [@subgroup_size], %wg_id:vec3<u32> [@workgroup_id]):void {
  $B1: {
    %l_invoc_id:vec3<u32> = let %invoc_id
    %l_num_wgs:vec3<u32> = let %num_wgs
    %l_invoc_index:u32 = let %invoc_index
    %l_sg_id:u32 = let %sg_id
    %l_glob_id:vec3<u32> = let %glob_id
    %l_sg_size:u32 = let %sg_size
    %l_wg_id:vec3<u32> = let %wg_id
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(16), @block {
  tint_num_workgroups_start_offset:vec3<u32> @offset(0)
}

foo_inputs = struct @align(16) {
  invoc_id:vec3<u32> @offset(0), @builtin(local_invocation_id)
  invoc_index:u32 @offset(12), @builtin(local_invocation_index)
  glob_id:vec3<u32> @offset(16), @builtin(global_invocation_id)
  wg_id:vec3<u32> @offset(32), @builtin(workgroup_id)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func(%invoc_id:vec3<u32>, %num_wgs:vec3<u32>, %invoc_index:u32, %sg_id:u32, %glob_id:vec3<u32>, %sg_size:u32, %wg_id:vec3<u32>):void {
  $B2: {
    %l_invoc_id:vec3<u32> = let %invoc_id
    %l_num_wgs:vec3<u32> = let %num_wgs
    %l_invoc_index:u32 = let %invoc_index
    %l_sg_id:u32 = let %sg_id
    %l_glob_id:vec3<u32> = let %glob_id
    %l_sg_size:u32 = let %sg_size
    %l_wg_id:vec3<u32> = let %wg_id
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %19:vec3<u32> = access %inputs, 0u
    %20:ptr<immediate, vec3<u32>, read> = access %tint_immediate_data, 0u
    %21:vec3<u32> = load %20
    %22:u32 = access %inputs, 1u
    %23:u32 = hlsl.WaveGetLaneIndex
    %24:vec3<u32> = access %inputs, 2u
    %25:u32 = hlsl.WaveGetLaneCount
    %26:vec3<u32> = access %inputs, 3u
    %27:void = call %foo_inner, %19, %21, %22, %23, %24, %25, %26
    ret
  }
}
)";

    // Refs to Raise() steps to setup push constant layout.
    constexpr uint32_t num_workgroups_offset = 0u;

    core::ir::transform::PrepareImmediateDataConfig prepare_immediate_data_layout_config;
    ASSERT_EQ(
        prepare_immediate_data_layout_config.AddInternalImmediateData(
            num_workgroups_offset, mod.symbols.New("tint_num_workgroups_start_offset"), ty.vec3u()),
        Success);
    auto layout =
        core::ir::transform::PrepareImmediateData(mod, prepare_immediate_data_layout_config);

    ShaderIOConfig config{layout.Get()};
    config.num_workgroups_start_offset = num_workgroups_offset;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_ClipDistances_1) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;
    core::IOAttributes clip_distances_attr;
    clip_distances_attr.builtin = core::BuiltinValue::kClipDistances;
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                      {mod.symbols.New("clip_distances"), ty.array<f32, 1>(), clip_distances_attr},
                  });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* cd = b.Construct(ty.array<f32, 1>(), 0.0_f);
        b.Return(ep, b.Construct(str_ty, pos, cd));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distances:array<f32, 1> @offset(16), @builtin(clip_distances)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 1> = construct 0.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distances:array<f32, 1> @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_clip_distances0:f32 @offset(16), @builtin(clip_distances)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 1> = construct 0.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:array<f32, 1> = access %6, 1u
    %9:f32 = access %8, 0u
    %10:foo_outputs = construct %7, %9
    ret %10
  }
}
)";

    capabilities = core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector;
    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_ClipDistances_2) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;
    core::IOAttributes clip_distances_attr;
    clip_distances_attr.builtin = core::BuiltinValue::kClipDistances;
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                      {mod.symbols.New("clip_distances"), ty.array<f32, 2>(), clip_distances_attr},
                  });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* cd = b.Construct(ty.array<f32, 2>(), 0.0_f, 1.0_f);
        b.Return(ep, b.Construct(str_ty, pos, cd));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distances:array<f32, 2> @offset(16), @builtin(clip_distances)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 2> = construct 0.0f, 1.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distances:array<f32, 2> @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_clip_distances0:vec2<f32> @offset(16), @builtin(clip_distances)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 2> = construct 0.0f, 1.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:array<f32, 2> = access %6, 1u
    %9:f32 = access %8, 0u
    %10:f32 = access %8, 1u
    %11:vec2<f32> = construct %9, %10
    %12:foo_outputs = construct %7, %11
    ret %12
  }
}
)";

    capabilities = core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector;
    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_ClipDistances_3) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;
    core::IOAttributes clip_distances_attr;
    clip_distances_attr.builtin = core::BuiltinValue::kClipDistances;
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                      {mod.symbols.New("clip_distances"), ty.array<f32, 3>(), clip_distances_attr},
                  });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* cd = b.Construct(ty.array<f32, 3>(), 0.0_f, 1.0_f, 2.0_f);
        b.Return(ep, b.Construct(str_ty, pos, cd));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distances:array<f32, 3> @offset(16), @builtin(clip_distances)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 3> = construct 0.0f, 1.0f, 2.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distances:array<f32, 3> @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_clip_distances0:vec3<f32> @offset(16), @builtin(clip_distances)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 3> = construct 0.0f, 1.0f, 2.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:array<f32, 3> = access %6, 1u
    %9:f32 = access %8, 0u
    %10:f32 = access %8, 1u
    %11:f32 = access %8, 2u
    %12:vec3<f32> = construct %9, %10, %11
    %13:foo_outputs = construct %7, %12
    ret %13
  }
}
)";

    capabilities = core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector;
    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_ClipDistances_4) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;
    core::IOAttributes clip_distances_attr;
    clip_distances_attr.builtin = core::BuiltinValue::kClipDistances;
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                      {mod.symbols.New("clip_distances"), ty.array<f32, 4>(), clip_distances_attr},
                  });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* cd = b.Construct(ty.array<f32, 4>(), 0.0_f, 1.0_f, 2.0_f, 3.0_f);
        b.Return(ep, b.Construct(str_ty, pos, cd));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distances:array<f32, 4> @offset(16), @builtin(clip_distances)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 4> = construct 0.0f, 1.0f, 2.0f, 3.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distances:array<f32, 4> @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_clip_distances0:vec4<f32> @offset(16), @builtin(clip_distances)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 4> = construct 0.0f, 1.0f, 2.0f, 3.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:array<f32, 4> = access %6, 1u
    %9:f32 = access %8, 0u
    %10:f32 = access %8, 1u
    %11:f32 = access %8, 2u
    %12:f32 = access %8, 3u
    %13:vec4<f32> = construct %9, %10, %11, %12
    %14:foo_outputs = construct %7, %13
    ret %14
  }
}
)";

    capabilities = core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector;
    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_ClipDistances_5) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;
    core::IOAttributes clip_distances_attr;
    clip_distances_attr.builtin = core::BuiltinValue::kClipDistances;
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                      {mod.symbols.New("clip_distances"), ty.array<f32, 5>(), clip_distances_attr},
                  });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* cd = b.Construct(ty.array<f32, 5>(), 0.0_f, 1.0_f, 2.0_f, 3.0_f, 4.0_f);
        b.Return(ep, b.Construct(str_ty, pos, cd));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distances:array<f32, 5> @offset(16), @builtin(clip_distances)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 5> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distances:array<f32, 5> @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_clip_distances0:vec4<f32> @offset(16), @builtin(clip_distances)
  Outputs_clip_distances1:f32 @offset(32), @builtin(clip_distances)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 5> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:array<f32, 5> = access %6, 1u
    %9:f32 = access %8, 0u
    %10:f32 = access %8, 1u
    %11:f32 = access %8, 2u
    %12:f32 = access %8, 3u
    %13:vec4<f32> = construct %9, %10, %11, %12
    %14:f32 = access %8, 4u
    %15:foo_outputs = construct %7, %13, %14
    ret %15
  }
}
)";

    capabilities = core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector;
    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_ClipDistances_6) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;
    core::IOAttributes clip_distances_attr;
    clip_distances_attr.builtin = core::BuiltinValue::kClipDistances;
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                      {mod.symbols.New("clip_distances"), ty.array<f32, 6>(), clip_distances_attr},
                  });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* cd = b.Construct(ty.array<f32, 6>(), 0.0_f, 1.0_f, 2.0_f, 3.0_f, 4.0_f, 5.0_f);
        b.Return(ep, b.Construct(str_ty, pos, cd));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distances:array<f32, 6> @offset(16), @builtin(clip_distances)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 6> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distances:array<f32, 6> @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_clip_distances0:vec4<f32> @offset(16), @builtin(clip_distances)
  Outputs_clip_distances1:vec2<f32> @offset(32), @builtin(clip_distances)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 6> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:array<f32, 6> = access %6, 1u
    %9:f32 = access %8, 0u
    %10:f32 = access %8, 1u
    %11:f32 = access %8, 2u
    %12:f32 = access %8, 3u
    %13:vec4<f32> = construct %9, %10, %11, %12
    %14:f32 = access %8, 4u
    %15:f32 = access %8, 5u
    %16:vec2<f32> = construct %14, %15
    %17:foo_outputs = construct %7, %13, %16
    ret %17
  }
}
)";

    capabilities = core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector;
    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_ClipDistances_7) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;
    core::IOAttributes clip_distances_attr;
    clip_distances_attr.builtin = core::BuiltinValue::kClipDistances;
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                      {mod.symbols.New("clip_distances"), ty.array<f32, 7>(), clip_distances_attr},
                  });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* cd = b.Construct(ty.array<f32, 7>(), 0.0_f, 1.0_f, 2.0_f, 3.0_f, 4.0_f, 5.0_f, 6.0_f);
        b.Return(ep, b.Construct(str_ty, pos, cd));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distances:array<f32, 7> @offset(16), @builtin(clip_distances)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 7> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distances:array<f32, 7> @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_clip_distances0:vec4<f32> @offset(16), @builtin(clip_distances)
  Outputs_clip_distances1:vec3<f32> @offset(32), @builtin(clip_distances)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 7> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:array<f32, 7> = access %6, 1u
    %9:f32 = access %8, 0u
    %10:f32 = access %8, 1u
    %11:f32 = access %8, 2u
    %12:f32 = access %8, 3u
    %13:vec4<f32> = construct %9, %10, %11, %12
    %14:f32 = access %8, 4u
    %15:f32 = access %8, 5u
    %16:f32 = access %8, 6u
    %17:vec3<f32> = construct %14, %15, %16
    %18:foo_outputs = construct %7, %13, %17
    ret %18
  }
}
)";

    capabilities = core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector;
    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_ClipDistances_8) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;
    core::IOAttributes clip_distances_attr;
    clip_distances_attr.builtin = core::BuiltinValue::kClipDistances;
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                      {mod.symbols.New("clip_distances"), ty.array<f32, 8>(), clip_distances_attr},
                  });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* cd =
            b.Construct(ty.array<f32, 8>(), 0.0_f, 1.0_f, 2.0_f, 3.0_f, 4.0_f, 5.0_f, 6.0_f, 7.0_f);
        b.Return(ep, b.Construct(str_ty, pos, cd));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  clip_distances:array<f32, 8> @offset(16), @builtin(clip_distances)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 8> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  clip_distances:array<f32, 8> @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_clip_distances0:vec4<f32> @offset(16), @builtin(clip_distances)
  Outputs_clip_distances1:vec4<f32> @offset(32), @builtin(clip_distances)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:array<f32, 8> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:array<f32, 8> = access %6, 1u
    %9:f32 = access %8, 0u
    %10:f32 = access %8, 1u
    %11:f32 = access %8, 2u
    %12:f32 = access %8, 3u
    %13:vec4<f32> = construct %9, %10, %11, %12
    %14:f32 = access %8, 4u
    %15:f32 = access %8, 5u
    %16:f32 = access %8, 6u
    %17:f32 = access %8, 7u
    %18:vec4<f32> = construct %14, %15, %16, %17
    %19:foo_outputs = construct %7, %13, %18
    ret %19
  }
}
)";

    capabilities = core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector;
    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_ClipDistances_FirstMember) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;
    core::IOAttributes clip_distances_attr;
    clip_distances_attr.builtin = core::BuiltinValue::kClipDistances;
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"),
                  {
                      {mod.symbols.New("clip_distances"), ty.array<f32, 5>(), clip_distances_attr},
                      {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                  });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* cd = b.Construct(ty.array<f32, 5>(), 0.0_f, 1.0_f, 2.0_f, 3.0_f, 4.0_f);
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        b.Return(ep, b.Construct(str_ty, cd, pos));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  clip_distances:array<f32, 5> @offset(0), @builtin(clip_distances)
  position:vec4<f32> @offset(32), @builtin(position)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:array<f32, 5> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f
    %3:vec4<f32> = construct 0.5f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  clip_distances:array<f32, 5> @offset(0)
  position:vec4<f32> @offset(32)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_clip_distances0:vec4<f32> @offset(16), @builtin(clip_distances)
  Outputs_clip_distances1:f32 @offset(32), @builtin(clip_distances)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:array<f32, 5> = construct 0.0f, 1.0f, 2.0f, 3.0f, 4.0f
    %3:vec4<f32> = construct 0.5f
    %4:Outputs = construct %2, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:array<f32, 5> = access %6, 0u
    %8:f32 = access %7, 0u
    %9:f32 = access %7, 1u
    %10:f32 = access %7, 2u
    %11:f32 = access %7, 3u
    %12:vec4<f32> = construct %8, %9, %10, %11
    %13:f32 = access %7, 4u
    %14:vec4<f32> = access %6, 1u
    %15:foo_outputs = construct %14, %12, %13
    ret %15
  }
}
)";

    capabilities = core::ir::Capability::kAllowClipDistancesOnF32ScalarAndVector;
    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_TruncateInterstage_Disabled) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;

    core::IOAttributes loc0_attr{
        .location = 0,
    };
    core::IOAttributes loc1_attr{
        .location = 1,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    core::IOAttributes loc2_attr{
        .location = 2,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                                 {mod.symbols.New("loc0"), ty.f32(), loc0_attr},
                                 {mod.symbols.New("loc1"), ty.i32(), loc1_attr},
                                 {mod.symbols.New("loc2"), ty.vec3i(), loc2_attr},
                             });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* loc2 = b.Construct(ty.vec3i(), 3_i);
        b.Return(ep, b.Construct(str_ty, pos, 1_f, 2_i, loc2));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  loc0:f32 @offset(16), @location(0)
  loc1:i32 @offset(20), @location(1), @interpolate(flat)
  loc2:vec3<i32> @offset(32), @location(2), @interpolate(flat)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  loc0:f32 @offset(16)
  loc1:i32 @offset(20)
  loc2:vec3<i32> @offset(32)
}

foo_outputs = struct @align(16) {
  Outputs_loc0:f32 @offset(0), @location(0)
  Outputs_loc1:i32 @offset(4), @location(1), @interpolate(flat)
  Outputs_loc2:vec3<i32> @offset(16), @location(2), @interpolate(flat)
  Outputs_position:vec4<f32> @offset(32), @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:f32 = access %6, 1u
    %9:i32 = access %6, 2u
    %10:vec3<i32> = access %6, 3u
    %11:foo_outputs = construct %8, %9, %10, %7
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.truncate_interstage_variables = false;  // Disabled
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_TruncateInterstage_None) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;

    core::IOAttributes loc0_attr{
        .location = 0,
    };
    core::IOAttributes loc1_attr{
        .location = 1,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    core::IOAttributes loc2_attr{
        .location = 2,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                                 {mod.symbols.New("loc0"), ty.f32(), loc0_attr},
                                 {mod.symbols.New("loc1"), ty.i32(), loc1_attr},
                                 {mod.symbols.New("loc2"), ty.vec3i(), loc2_attr},
                             });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* loc2 = b.Construct(ty.vec3i(), 3_i);
        b.Return(ep, b.Construct(str_ty, pos, 1_f, 2_i, loc2));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  loc0:f32 @offset(16), @location(0)
  loc1:i32 @offset(20), @location(1), @interpolate(flat)
  loc2:vec3<i32> @offset(32), @location(2), @interpolate(flat)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  loc0:f32 @offset(16)
  loc1:i32 @offset(20)
  loc2:vec3<i32> @offset(32)
}

foo_outputs = struct @align(16) {
  Outputs_loc0:f32 @offset(0), @location(0)
  Outputs_loc1:i32 @offset(4), @location(1), @interpolate(flat)
  Outputs_loc2:vec3<i32> @offset(16), @location(2), @interpolate(flat)
  Outputs_position:vec4<f32> @offset(32), @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:f32 = access %6, 1u
    %9:i32 = access %6, 2u
    %10:vec3<i32> = access %6, 3u
    %11:foo_outputs = construct %8, %9, %10, %7
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.truncate_interstage_variables = true;
    config.interstage_locations[0] = true;
    config.interstage_locations[1] = true;
    config.interstage_locations[2] = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_TruncateInterstage_First) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;

    core::IOAttributes loc0_attr{
        .location = 0,
    };
    core::IOAttributes loc1_attr{
        .location = 1,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    core::IOAttributes loc2_attr{
        .location = 2,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                                 {mod.symbols.New("loc0"), ty.f32(), loc0_attr},
                                 {mod.symbols.New("loc1"), ty.i32(), loc1_attr},
                                 {mod.symbols.New("loc2"), ty.vec3i(), loc2_attr},
                             });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* loc2 = b.Construct(ty.vec3i(), 3_i);
        b.Return(ep, b.Construct(str_ty, pos, 1_f, 2_i, loc2));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  loc0:f32 @offset(16), @location(0)
  loc1:i32 @offset(20), @location(1), @interpolate(flat)
  loc2:vec3<i32> @offset(32), @location(2), @interpolate(flat)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  loc0:f32 @offset(16)
  loc1:i32 @offset(20)
  loc2:vec3<i32> @offset(32)
}

foo_outputs = struct @align(16) {
  Outputs_loc1:i32 @offset(0), @location(1), @interpolate(flat)
  Outputs_loc2:vec3<i32> @offset(16), @location(2), @interpolate(flat)
  Outputs_position:vec4<f32> @offset(32), @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:f32 = access %6, 1u
    %9:i32 = access %6, 2u
    %10:vec3<i32> = access %6, 3u
    %11:foo_outputs = construct %9, %10, %7
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.truncate_interstage_variables = true;
    config.interstage_locations[1] = true;
    config.interstage_locations[2] = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_TruncateInterstage_Middle) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;

    core::IOAttributes loc0_attr{
        .location = 0,
    };
    core::IOAttributes loc1_attr{
        .location = 1,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    core::IOAttributes loc2_attr{
        .location = 2,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                                 {mod.symbols.New("loc0"), ty.f32(), loc0_attr},
                                 {mod.symbols.New("loc1"), ty.i32(), loc1_attr},
                                 {mod.symbols.New("loc2"), ty.vec3i(), loc2_attr},
                             });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* loc2 = b.Construct(ty.vec3i(), 3_i);
        b.Return(ep, b.Construct(str_ty, pos, 1_f, 2_i, loc2));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  loc0:f32 @offset(16), @location(0)
  loc1:i32 @offset(20), @location(1), @interpolate(flat)
  loc2:vec3<i32> @offset(32), @location(2), @interpolate(flat)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  loc0:f32 @offset(16)
  loc1:i32 @offset(20)
  loc2:vec3<i32> @offset(32)
}

foo_outputs = struct @align(16) {
  Outputs_loc0:f32 @offset(0), @location(0)
  Outputs_loc2:vec3<i32> @offset(16), @location(2), @interpolate(flat)
  Outputs_position:vec4<f32> @offset(32), @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:f32 = access %6, 1u
    %9:i32 = access %6, 2u
    %10:vec3<i32> = access %6, 3u
    %11:foo_outputs = construct %8, %10, %7
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.truncate_interstage_variables = true;
    config.interstage_locations[0] = true;
    config.interstage_locations[2] = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_TruncateInterstage_Last) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;

    core::IOAttributes loc0_attr{
        .location = 0,
    };
    core::IOAttributes loc1_attr{
        .location = 1,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    core::IOAttributes loc2_attr{
        .location = 2,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                                 {mod.symbols.New("loc0"), ty.f32(), loc0_attr},
                                 {mod.symbols.New("loc1"), ty.i32(), loc1_attr},
                                 {mod.symbols.New("loc2"), ty.vec3i(), loc2_attr},
                             });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* loc2 = b.Construct(ty.vec3i(), 3_i);
        b.Return(ep, b.Construct(str_ty, pos, 1_f, 2_i, loc2));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  loc0:f32 @offset(16), @location(0)
  loc1:i32 @offset(20), @location(1), @interpolate(flat)
  loc2:vec3<i32> @offset(32), @location(2), @interpolate(flat)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  loc0:f32 @offset(16)
  loc1:i32 @offset(20)
  loc2:vec3<i32> @offset(32)
}

foo_outputs = struct @align(16) {
  Outputs_loc0:f32 @offset(0), @location(0)
  Outputs_loc1:i32 @offset(4), @location(1), @interpolate(flat)
  Outputs_position:vec4<f32> @offset(16), @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:f32 = access %6, 1u
    %9:i32 = access %6, 2u
    %10:vec3<i32> = access %6, 3u
    %11:foo_outputs = construct %8, %9, %7
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.truncate_interstage_variables = true;
    config.interstage_locations[0] = true;
    config.interstage_locations[1] = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_TruncateInterstage_FirstAndLast) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;

    core::IOAttributes loc0_attr{
        .location = 0,
    };
    core::IOAttributes loc1_attr{
        .location = 1,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    core::IOAttributes loc2_attr{
        .location = 2,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                                 {mod.symbols.New("loc0"), ty.f32(), loc0_attr},
                                 {mod.symbols.New("loc1"), ty.i32(), loc1_attr},
                                 {mod.symbols.New("loc2"), ty.vec3i(), loc2_attr},
                             });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* loc2 = b.Construct(ty.vec3i(), 3_i);
        b.Return(ep, b.Construct(str_ty, pos, 1_f, 2_i, loc2));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  loc0:f32 @offset(16), @location(0)
  loc1:i32 @offset(20), @location(1), @interpolate(flat)
  loc2:vec3<i32> @offset(32), @location(2), @interpolate(flat)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  loc0:f32 @offset(16)
  loc1:i32 @offset(20)
  loc2:vec3<i32> @offset(32)
}

foo_outputs = struct @align(16) {
  Outputs_loc1:i32 @offset(0), @location(1), @interpolate(flat)
  Outputs_position:vec4<f32> @offset(16), @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:f32 = access %6, 1u
    %9:i32 = access %6, 2u
    %10:vec3<i32> = access %6, 3u
    %11:foo_outputs = construct %9, %7
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.truncate_interstage_variables = true;
    config.interstage_locations[1] = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_TruncateInterstage_All) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;

    core::IOAttributes loc0_attr{
        .location = 0,
    };
    core::IOAttributes loc1_attr{
        .location = 1,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    core::IOAttributes loc2_attr{
        .location = 2,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                                 {mod.symbols.New("loc0"), ty.f32(), loc0_attr},
                                 {mod.symbols.New("loc1"), ty.i32(), loc1_attr},
                                 {mod.symbols.New("loc2"), ty.vec3i(), loc2_attr},
                             });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* loc2 = b.Construct(ty.vec3i(), 3_i);
        b.Return(ep, b.Construct(str_ty, pos, 1_f, 2_i, loc2));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  loc0:f32 @offset(16), @location(0)
  loc1:i32 @offset(20), @location(1), @interpolate(flat)
  loc2:vec3<i32> @offset(32), @location(2), @interpolate(flat)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0)
  loc0:f32 @offset(16)
  loc1:i32 @offset(20)
  loc2:vec3<i32> @offset(32)
}

foo_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct %2, 1.0f, 2i, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:vec4<f32> = access %6, 0u
    %8:f32 = access %6, 1u
    %9:i32 = access %6, 2u
    %10:vec3<i32> = access %6, 3u
    %11:foo_outputs = construct %7
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.truncate_interstage_variables = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_TruncateInterstage_NonLocationInBetween) {
    core::IOAttributes pos_attr;
    pos_attr.builtin = core::BuiltinValue::kPosition;

    core::IOAttributes loc0_attr{
        .location = 0,
    };
    core::IOAttributes loc1_attr{
        .location = 1,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    core::IOAttributes loc2_attr{
        .location = 2,
        .interpolation = core::Interpolation{core::InterpolationType::kFlat},
    };
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {mod.symbols.New("loc0"), ty.f32(), loc0_attr},
                                 {mod.symbols.New("position"), ty.vec4f(), pos_attr},
                                 {mod.symbols.New("loc1"), ty.i32(), loc1_attr},
                                 {mod.symbols.New("loc2"), ty.vec3i(), loc2_attr},
                             });
    auto* ep = b.Function("foo", str_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(ep->Block(), [&] {
        auto* pos = b.Construct(ty.vec4f(), 0.5_f);
        auto* loc2 = b.Construct(ty.vec3i(), 3_i);
        b.Return(ep, b.Construct(str_ty, 1_f, pos, 2_i, loc2));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  loc0:f32 @offset(0), @location(0)
  position:vec4<f32> @offset(16), @builtin(position)
  loc1:i32 @offset(32), @location(1), @interpolate(flat)
  loc2:vec3<i32> @offset(48), @location(2), @interpolate(flat)
}

%foo = @vertex func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct 1.0f, %2, 2i, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  loc0:f32 @offset(0)
  position:vec4<f32> @offset(16)
  loc1:i32 @offset(32)
  loc2:vec3<i32> @offset(48)
}

foo_outputs = struct @align(16) {
  Outputs_loc0:f32 @offset(0), @location(0)
  Outputs_loc2:vec3<i32> @offset(16), @location(2), @interpolate(flat)
  Outputs_position:vec4<f32> @offset(32), @builtin(position)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    %3:vec3<i32> = construct 3i
    %4:Outputs = construct 1.0f, %2, 2i, %3
    ret %4
  }
}
%foo = @vertex func():foo_outputs {
  $B2: {
    %6:Outputs = call %foo_inner
    %7:f32 = access %6, 0u
    %8:vec4<f32> = access %6, 1u
    %9:i32 = access %6, 2u
    %10:vec3<i32> = access %6, 3u
    %11:foo_outputs = construct %7, %10, %8
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.truncate_interstage_variables = true;
    config.interstage_locations[0] = true;
    config.interstage_locations[2] = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_FirstIndexOffset_VertexIndex) {
    auto* vert_idx = b.FunctionParam("vert_idx", ty.u32());
    vert_idx->SetBuiltin(core::BuiltinValue::kVertexIndex);

    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetParams({vert_idx});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {
        b.Add(vert_idx, vert_idx);
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func(%vert_idx:u32 [@vertex_index]):vec4<f32> [@position] {
  $B1: {
    %3:u32 = add %vert_idx, %vert_idx
    %4:vec4<f32> = construct 0.5f
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_first_index_offset_struct = struct @align(4) {
  vertex_index:u32 @offset(0)
  instance_index:u32 @offset(4)
}

foo_inputs = struct @align(4) {
  vert_idx:u32 @offset(0), @builtin(vertex_index)
}

foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %tint_first_index_offset:ptr<uniform, tint_first_index_offset_struct, read> = var undef @binding_point(12, 34)
}

%foo_inner = func(%vert_idx:u32):vec4<f32> {
  $B2: {
    %4:u32 = add %vert_idx, %vert_idx
    %5:vec4<f32> = construct 0.5f
    ret %5
  }
}
%foo = @vertex func(%inputs:foo_inputs):foo_outputs {
  $B3: {
    %8:u32 = access %inputs, 0u
    %9:ptr<uniform, u32, read> = access %tint_first_index_offset, 0u
    %10:u32 = load %9
    %11:u32 = add %8, %10
    %12:vec4<f32> = call %foo_inner, %11
    %13:foo_outputs = construct %12
    ret %13
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.first_index_offset_binding = {12, 34};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Immediate_FirstIndexOffset_VertexIndex) {
    auto* vert_idx = b.FunctionParam("vert_idx", ty.u32());
    vert_idx->SetBuiltin(core::BuiltinValue::kVertexIndex);

    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetParams({vert_idx});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {
        b.Add(vert_idx, vert_idx);
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func(%vert_idx:u32 [@vertex_index]):vec4<f32> [@position] {
  $B1: {
    %3:u32 = add %vert_idx, %vert_idx
    %4:vec4<f32> = construct 0.5f
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(4), @block {
  tint_first_index_offset:u32 @offset(0)
}

foo_inputs = struct @align(4) {
  vert_idx:u32 @offset(0), @builtin(vertex_index)
}

foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func(%vert_idx:u32):vec4<f32> {
  $B2: {
    %4:u32 = add %vert_idx, %vert_idx
    %5:vec4<f32> = construct 0.5f
    ret %5
  }
}
%foo = @vertex func(%inputs:foo_inputs):foo_outputs {
  $B3: {
    %8:u32 = access %inputs, 0u
    %9:ptr<immediate, u32, read> = access %tint_immediate_data, 0u
    %10:u32 = load %9
    %11:u32 = add %8, %10
    %12:vec4<f32> = call %foo_inner, %11
    %13:foo_outputs = construct %12
    ret %13
  }
}
)";

    // Refs to Raise() steps to setup push constant layout.
    constexpr uint32_t first_index_offset = 0u;

    core::ir::transform::PrepareImmediateDataConfig prepare_immediate_data_layout_config;
    ASSERT_EQ(prepare_immediate_data_layout_config.AddInternalImmediateData(
                  first_index_offset, mod.symbols.New("tint_first_index_offset"), ty.u32()),
              Success);
    auto layout =
        core::ir::transform::PrepareImmediateData(mod, prepare_immediate_data_layout_config);

    ShaderIOConfig config{layout.Get()};
    config.first_index_offset = first_index_offset;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_FirstIndexOffset_InstanceIndex) {
    auto* inst_idx = b.FunctionParam("inst_idx", ty.u32());
    inst_idx->SetBuiltin(core::BuiltinValue::kInstanceIndex);

    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetParams({inst_idx});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {
        b.Add(inst_idx, inst_idx);
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func(%inst_idx:u32 [@instance_index]):vec4<f32> [@position] {
  $B1: {
    %3:u32 = add %inst_idx, %inst_idx
    %4:vec4<f32> = construct 0.5f
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_first_index_offset_struct = struct @align(4) {
  vertex_index:u32 @offset(0)
  instance_index:u32 @offset(4)
}

foo_inputs = struct @align(4) {
  inst_idx:u32 @offset(0), @builtin(instance_index)
}

foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %tint_first_index_offset:ptr<uniform, tint_first_index_offset_struct, read> = var undef @binding_point(12, 34)
}

%foo_inner = func(%inst_idx:u32):vec4<f32> {
  $B2: {
    %4:u32 = add %inst_idx, %inst_idx
    %5:vec4<f32> = construct 0.5f
    ret %5
  }
}
%foo = @vertex func(%inputs:foo_inputs):foo_outputs {
  $B3: {
    %8:u32 = access %inputs, 0u
    %9:ptr<uniform, u32, read> = access %tint_first_index_offset, 1u
    %10:u32 = load %9
    %11:u32 = add %8, %10
    %12:vec4<f32> = call %foo_inner, %11
    %13:foo_outputs = construct %12
    ret %13
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.first_index_offset_binding = {12, 34};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Immediate_FirstIndexOffset_InstanceIndex) {
    auto* inst_idx = b.FunctionParam("inst_idx", ty.u32());
    inst_idx->SetBuiltin(core::BuiltinValue::kInstanceIndex);

    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetParams({inst_idx});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {
        b.Add(inst_idx, inst_idx);
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func(%inst_idx:u32 [@instance_index]):vec4<f32> [@position] {
  $B1: {
    %3:u32 = add %inst_idx, %inst_idx
    %4:vec4<f32> = construct 0.5f
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(4), @block {
  tint_first_instance_offset:u32 @offset(0)
}

foo_inputs = struct @align(4) {
  inst_idx:u32 @offset(0), @builtin(instance_index)
}

foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func(%inst_idx:u32):vec4<f32> {
  $B2: {
    %4:u32 = add %inst_idx, %inst_idx
    %5:vec4<f32> = construct 0.5f
    ret %5
  }
}
%foo = @vertex func(%inputs:foo_inputs):foo_outputs {
  $B3: {
    %8:u32 = access %inputs, 0u
    %9:ptr<immediate, u32, read> = access %tint_immediate_data, 0u
    %10:u32 = load %9
    %11:u32 = add %8, %10
    %12:vec4<f32> = call %foo_inner, %11
    %13:foo_outputs = construct %12
    ret %13
  }
}
)";

    // Refs to Raise() steps to setup push constant layout.
    constexpr uint32_t first_instance_offset = 0u;

    core::ir::transform::PrepareImmediateDataConfig prepare_immediate_data_layout_config;
    ASSERT_EQ(prepare_immediate_data_layout_config.AddInternalImmediateData(
                  first_instance_offset, mod.symbols.New("tint_first_instance_offset"), ty.u32()),
              Success);
    auto layout =
        core::ir::transform::PrepareImmediateData(mod, prepare_immediate_data_layout_config);

    ShaderIOConfig config{layout.Get()};
    config.first_instance_offset = first_instance_offset;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_FirstIndexOffset_Both) {
    auto* vert_idx = b.FunctionParam("vert_idx", ty.u32());
    vert_idx->SetBuiltin(core::BuiltinValue::kVertexIndex);

    auto* inst_idx = b.FunctionParam("inst_idx", ty.u32());
    inst_idx->SetBuiltin(core::BuiltinValue::kInstanceIndex);

    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetParams({vert_idx, inst_idx});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {
        b.Add(vert_idx, inst_idx);
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func(%vert_idx:u32 [@vertex_index], %inst_idx:u32 [@instance_index]):vec4<f32> [@position] {
  $B1: {
    %4:u32 = add %vert_idx, %inst_idx
    %5:vec4<f32> = construct 0.5f
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_first_index_offset_struct = struct @align(4) {
  vertex_index:u32 @offset(0)
  instance_index:u32 @offset(4)
}

foo_inputs = struct @align(4) {
  vert_idx:u32 @offset(0), @builtin(vertex_index)
  inst_idx:u32 @offset(4), @builtin(instance_index)
}

foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %tint_first_index_offset:ptr<uniform, tint_first_index_offset_struct, read> = var undef @binding_point(12, 34)
}

%foo_inner = func(%vert_idx:u32, %inst_idx:u32):vec4<f32> {
  $B2: {
    %5:u32 = add %vert_idx, %inst_idx
    %6:vec4<f32> = construct 0.5f
    ret %6
  }
}
%foo = @vertex func(%inputs:foo_inputs):foo_outputs {
  $B3: {
    %9:u32 = access %inputs, 0u
    %10:ptr<uniform, u32, read> = access %tint_first_index_offset, 0u
    %11:u32 = load %10
    %12:u32 = add %9, %11
    %13:u32 = access %inputs, 1u
    %14:ptr<uniform, u32, read> = access %tint_first_index_offset, 1u
    %15:u32 = load %14
    %16:u32 = add %13, %15
    %17:vec4<f32> = call %foo_inner, %12, %16
    %18:foo_outputs = construct %17
    ret %18
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.first_index_offset_binding = {12, 34};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_FirstIndexOffset_Immediate_Both) {
    auto* vert_idx = b.FunctionParam("vert_idx", ty.u32());
    vert_idx->SetBuiltin(core::BuiltinValue::kVertexIndex);

    auto* inst_idx = b.FunctionParam("inst_idx", ty.u32());
    inst_idx->SetBuiltin(core::BuiltinValue::kInstanceIndex);

    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetParams({vert_idx, inst_idx});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {
        b.Add(vert_idx, inst_idx);
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func(%vert_idx:u32 [@vertex_index], %inst_idx:u32 [@instance_index]):vec4<f32> [@position] {
  $B1: {
    %4:u32 = add %vert_idx, %inst_idx
    %5:vec4<f32> = construct 0.5f
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(4), @block {
  tint_first_index_offset:u32 @offset(0)
  tint_first_instance_offset:u32 @offset(4)
}

foo_inputs = struct @align(4) {
  vert_idx:u32 @offset(0), @builtin(vertex_index)
  inst_idx:u32 @offset(4), @builtin(instance_index)
}

foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func(%vert_idx:u32, %inst_idx:u32):vec4<f32> {
  $B2: {
    %5:u32 = add %vert_idx, %inst_idx
    %6:vec4<f32> = construct 0.5f
    ret %6
  }
}
%foo = @vertex func(%inputs:foo_inputs):foo_outputs {
  $B3: {
    %9:u32 = access %inputs, 0u
    %10:ptr<immediate, u32, read> = access %tint_immediate_data, 0u
    %11:u32 = load %10
    %12:u32 = add %9, %11
    %13:u32 = access %inputs, 1u
    %14:ptr<immediate, u32, read> = access %tint_immediate_data, 1u
    %15:u32 = load %14
    %16:u32 = add %13, %15
    %17:vec4<f32> = call %foo_inner, %12, %16
    %18:foo_outputs = construct %17
    ret %18
  }
}
)";

    // Refs to Raise() steps to setup push constant layout.
    constexpr uint32_t first_index_offset = 0u;
    constexpr uint32_t first_instance_offset = 4u;

    core::ir::transform::PrepareImmediateDataConfig prepare_immediate_data_layout_config;
    ASSERT_EQ(prepare_immediate_data_layout_config.AddInternalImmediateData(
                  first_index_offset, mod.symbols.New("tint_first_index_offset"), ty.u32()),
              Success);
    ASSERT_EQ(prepare_immediate_data_layout_config.AddInternalImmediateData(
                  first_instance_offset, mod.symbols.New("tint_first_instance_offset"), ty.u32()),
              Success);
    auto layout =
        core::ir::transform::PrepareImmediateData(mod, prepare_immediate_data_layout_config);

    ShaderIOConfig config{layout.Get()};
    config.first_index_offset = first_index_offset;
    config.first_instance_offset = first_instance_offset;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_FirstIndexOffset_BothReorder) {
    auto* inst_idx = b.FunctionParam("inst_idx", ty.u32());
    inst_idx->SetBuiltin(core::BuiltinValue::kInstanceIndex);

    auto* vert_idx = b.FunctionParam("vert_idx", ty.u32());
    vert_idx->SetBuiltin(core::BuiltinValue::kVertexIndex);

    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetParams({inst_idx, vert_idx});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {
        b.Add(vert_idx, inst_idx);
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func(%inst_idx:u32 [@instance_index], %vert_idx:u32 [@vertex_index]):vec4<f32> [@position] {
  $B1: {
    %4:u32 = add %vert_idx, %inst_idx
    %5:vec4<f32> = construct 0.5f
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_first_index_offset_struct = struct @align(4) {
  vertex_index:u32 @offset(0)
  instance_index:u32 @offset(4)
}

foo_inputs = struct @align(4) {
  vert_idx:u32 @offset(0), @builtin(vertex_index)
  inst_idx:u32 @offset(4), @builtin(instance_index)
}

foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %tint_first_index_offset:ptr<uniform, tint_first_index_offset_struct, read> = var undef @binding_point(12, 34)
}

%foo_inner = func(%inst_idx:u32, %vert_idx:u32):vec4<f32> {
  $B2: {
    %5:u32 = add %vert_idx, %inst_idx
    %6:vec4<f32> = construct 0.5f
    ret %6
  }
}
%foo = @vertex func(%inputs:foo_inputs):foo_outputs {
  $B3: {
    %9:u32 = access %inputs, 1u
    %10:ptr<uniform, u32, read> = access %tint_first_index_offset, 1u
    %11:u32 = load %10
    %12:u32 = add %9, %11
    %13:u32 = access %inputs, 0u
    %14:ptr<uniform, u32, read> = access %tint_first_index_offset, 0u
    %15:u32 = load %14
    %16:u32 = add %13, %15
    %17:vec4<f32> = call %foo_inner, %12, %16
    %18:foo_outputs = construct %17
    ret %18
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.first_index_offset_binding = {12, 34};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_Immediate_FirstIndexOffset_BothReorder) {
    auto* inst_idx = b.FunctionParam("inst_idx", ty.u32());
    inst_idx->SetBuiltin(core::BuiltinValue::kInstanceIndex);

    auto* vert_idx = b.FunctionParam("vert_idx", ty.u32());
    vert_idx->SetBuiltin(core::BuiltinValue::kVertexIndex);

    auto* ep = b.Function("foo", ty.vec4f(), core::ir::Function::PipelineStage::kVertex);
    ep->SetParams({inst_idx, vert_idx});
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {
        b.Add(vert_idx, inst_idx);
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func(%inst_idx:u32 [@instance_index], %vert_idx:u32 [@vertex_index]):vec4<f32> [@position] {
  $B1: {
    %4:u32 = add %vert_idx, %inst_idx
    %5:vec4<f32> = construct 0.5f
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_immediate_data_struct = struct @align(4), @block {
  tint_first_index_offset:u32 @offset(0)
  tint_first_instance_offset:u32 @offset(4)
}

foo_inputs = struct @align(4) {
  vert_idx:u32 @offset(0), @builtin(vertex_index)
  inst_idx:u32 @offset(4), @builtin(instance_index)
}

foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func(%inst_idx:u32, %vert_idx:u32):vec4<f32> {
  $B2: {
    %5:u32 = add %vert_idx, %inst_idx
    %6:vec4<f32> = construct 0.5f
    ret %6
  }
}
%foo = @vertex func(%inputs:foo_inputs):foo_outputs {
  $B3: {
    %9:u32 = access %inputs, 1u
    %10:ptr<immediate, u32, read> = access %tint_immediate_data, 1u
    %11:u32 = load %10
    %12:u32 = add %9, %11
    %13:u32 = access %inputs, 0u
    %14:ptr<immediate, u32, read> = access %tint_immediate_data, 0u
    %15:u32 = load %14
    %16:u32 = add %13, %15
    %17:vec4<f32> = call %foo_inner, %12, %16
    %18:foo_outputs = construct %17
    ret %18
  }
}
)";

    // Refs to Raise() steps to setup push constant layout.
    constexpr uint32_t first_index_offset = 0u;
    constexpr uint32_t first_instance_offset = 4u;

    core::ir::transform::PrepareImmediateDataConfig prepare_immediate_data_layout_config;
    ASSERT_EQ(prepare_immediate_data_layout_config.AddInternalImmediateData(
                  first_index_offset, mod.symbols.New("tint_first_index_offset"), ty.u32()),
              Success);
    ASSERT_EQ(prepare_immediate_data_layout_config.AddInternalImmediateData(
                  first_instance_offset, mod.symbols.New("tint_first_instance_offset"), ty.u32()),
              Success);
    auto layout =
        core::ir::transform::PrepareImmediateData(mod, prepare_immediate_data_layout_config);

    ShaderIOConfig config{layout.Get()};
    config.first_index_offset = first_index_offset;
    config.first_instance_offset = first_instance_offset;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_SubgroupId_NonLinearWorkgroupSize) {
    auto* subgroup_id = b.FunctionParam("id", ty.u32());
    subgroup_id->SetBuiltin(core::BuiltinValue::kSubgroupId);

    auto* ep = b.ComputeFunction("foo", 8_u, 8_u, 1_u);
    ep->SetParams({subgroup_id});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(subgroup_id, subgroup_id));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(8u, 8u, 1u) func(%id:u32 [@subgroup_id]):void {
  $B1: {
    %3:u32 = mul %id, %id
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_subgroup_id_counter:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo_inner = func(%id:u32):void {
  $B2: {
    %4:u32 = mul %id, %id
    %x:u32 = let %4
    ret
  }
}
%foo = @compute @workgroup_size(8u, 8u, 1u) func():void {
  $B3: {
    %7:void = atomicStore %tint_subgroup_id_counter, 0u
    %8:void = workgroupBarrier
    %tint_subgroup_id:ptr<function, u32, read_write> = var undef
    %10:bool = subgroupElect
    if %10 [t: $B4] {  # if_1
      $B4: {  # true
        %11:u32 = atomicAdd %tint_subgroup_id_counter, 1u
        store %tint_subgroup_id, %11
        exit_if  # if_1
      }
    }
    %12:u32 = load %tint_subgroup_id
    %13:u32 = subgroupBroadcastFirst %12
    %14:void = call %foo_inner, %13
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumSubgroups_NonLinearWorkgroupSize) {
    auto* num_subgroups = b.FunctionParam("num", ty.u32());
    num_subgroups->SetBuiltin(core::BuiltinValue::kNumSubgroups);

    auto* ep = b.ComputeFunction("foo", 8_u, 8_u, 1_u);
    ep->SetParams({num_subgroups});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(num_subgroups, num_subgroups));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(8u, 8u, 1u) func(%num:u32 [@num_subgroups]):void {
  $B1: {
    %3:u32 = mul %num, %num
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_subgroup_id_counter:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo_inner = func(%num:u32):void {
  $B2: {
    %4:u32 = mul %num, %num
    %x:u32 = let %4
    ret
  }
}
%foo = @compute @workgroup_size(8u, 8u, 1u) func():void {
  $B3: {
    %7:void = atomicStore %tint_subgroup_id_counter, 0u
    %8:void = workgroupBarrier
    %tint_subgroup_id:ptr<function, u32, read_write> = var undef
    %10:bool = subgroupElect
    if %10 [t: $B4] {  # if_1
      $B4: {  # true
        %11:u32 = atomicAdd %tint_subgroup_id_counter, 1u
        store %tint_subgroup_id, %11
        exit_if  # if_1
      }
    }
    %12:void = workgroupBarrier
    %13:u32 = atomicLoad %tint_subgroup_id_counter
    %14:void = call %foo_inner, %13
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_SubgroupId_Then_NumSubgroups_NonLinear) {
    auto* subgroup_id = b.FunctionParam("id", ty.u32());
    subgroup_id->SetBuiltin(core::BuiltinValue::kSubgroupId);

    auto* num_subgroups = b.FunctionParam("num", ty.u32());
    num_subgroups->SetBuiltin(core::BuiltinValue::kNumSubgroups);

    auto* ep = b.ComputeFunction("foo", 8_u, 8_u, 1_u);
    ep->SetParams({subgroup_id, num_subgroups});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(subgroup_id, num_subgroups));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(8u, 8u, 1u) func(%id:u32 [@subgroup_id], %num:u32 [@num_subgroups]):void {
  $B1: {
    %4:u32 = mul %id, %num
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_subgroup_id_counter:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo_inner = func(%id:u32, %num:u32):void {
  $B2: {
    %5:u32 = mul %id, %num
    %x:u32 = let %5
    ret
  }
}
%foo = @compute @workgroup_size(8u, 8u, 1u) func():void {
  $B3: {
    %8:void = atomicStore %tint_subgroup_id_counter, 0u
    %9:void = workgroupBarrier
    %tint_subgroup_id:ptr<function, u32, read_write> = var undef
    %11:bool = subgroupElect
    if %11 [t: $B4] {  # if_1
      $B4: {  # true
        %12:u32 = atomicAdd %tint_subgroup_id_counter, 1u
        store %tint_subgroup_id, %12
        exit_if  # if_1
      }
    }
    %13:u32 = load %tint_subgroup_id
    %14:u32 = subgroupBroadcastFirst %13
    %15:void = workgroupBarrier
    %16:u32 = atomicLoad %tint_subgroup_id_counter
    %17:void = call %foo_inner, %14, %16
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_NumSubgroups_Then_SubgroupId_NonLinear) {
    auto* num_subgroups = b.FunctionParam("num", ty.u32());
    num_subgroups->SetBuiltin(core::BuiltinValue::kNumSubgroups);

    auto* subgroup_id = b.FunctionParam("id", ty.u32());
    subgroup_id->SetBuiltin(core::BuiltinValue::kSubgroupId);

    auto* ep = b.ComputeFunction("foo", 8_u, 8_u, 1_u);
    ep->SetParams({num_subgroups, subgroup_id});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(subgroup_id, num_subgroups));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(8u, 8u, 1u) func(%num:u32 [@num_subgroups], %id:u32 [@subgroup_id]):void {
  $B1: {
    %4:u32 = mul %id, %num
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %tint_subgroup_id_counter:ptr<workgroup, atomic<u32>, read_write> = var undef
}

%foo_inner = func(%num:u32, %id:u32):void {
  $B2: {
    %5:u32 = mul %id, %num
    %x:u32 = let %5
    ret
  }
}
%foo = @compute @workgroup_size(8u, 8u, 1u) func():void {
  $B3: {
    %8:void = atomicStore %tint_subgroup_id_counter, 0u
    %9:void = workgroupBarrier
    %tint_subgroup_id:ptr<function, u32, read_write> = var undef
    %11:bool = subgroupElect
    if %11 [t: $B4] {  # if_1
      $B4: {  # true
        %12:u32 = atomicAdd %tint_subgroup_id_counter, 1u
        store %tint_subgroup_id, %12
        exit_if  # if_1
      }
    }
    %13:void = workgroupBarrier
    %14:u32 = atomicLoad %tint_subgroup_id_counter
    %15:u32 = load %tint_subgroup_id
    %16:u32 = subgroupBroadcastFirst %15
    %17:void = call %foo_inner, %14, %16
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_SubgroupIdAndNumSubgroups_LinearX) {
    auto* subgroup_id = b.FunctionParam("id", ty.u32());
    subgroup_id->SetBuiltin(core::BuiltinValue::kSubgroupId);

    auto* num_subgroups = b.FunctionParam("num", ty.u32());
    num_subgroups->SetBuiltin(core::BuiltinValue::kNumSubgroups);

    auto* ep = b.ComputeFunction("foo", 64_u, 1_u, 1_u);
    ep->SetParams({subgroup_id, num_subgroups});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(subgroup_id, num_subgroups));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(64u, 1u, 1u) func(%id:u32 [@subgroup_id], %num:u32 [@num_subgroups]):void {
  $B1: {
    %4:u32 = mul %id, %num
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(4) {
  local_invocation_index:u32 @offset(0), @builtin(local_invocation_index)
}

%foo_inner = func(%id:u32, %num:u32):void {
  $B1: {
    %4:u32 = mul %id, %num
    %x:u32 = let %4
    ret
  }
}
%foo = @compute @workgroup_size(64u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B2: {
    %8:u32 = access %inputs, 0u
    %9:u32 = hlsl.WaveGetLaneCount
    %10:u32 = div %8, %9
    %11:u32 = hlsl.WaveGetLaneCount
    %12:u32 = add 63u, %11
    %13:u32 = div %12, %11
    %14:void = call %foo_inner, %10, %13
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_SubgroupIdAndNumSubgroups_LinearY) {
    auto* subgroup_id = b.FunctionParam("id", ty.u32());
    subgroup_id->SetBuiltin(core::BuiltinValue::kSubgroupId);

    auto* num_subgroups = b.FunctionParam("num", ty.u32());
    num_subgroups->SetBuiltin(core::BuiltinValue::kNumSubgroups);

    auto* ep = b.ComputeFunction("foo", 1_u, 64_u, 1_u);
    ep->SetParams({subgroup_id, num_subgroups});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(subgroup_id, num_subgroups));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 64u, 1u) func(%id:u32 [@subgroup_id], %num:u32 [@num_subgroups]):void {
  $B1: {
    %4:u32 = mul %id, %num
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(4) {
  local_invocation_index:u32 @offset(0), @builtin(local_invocation_index)
}

%foo_inner = func(%id:u32, %num:u32):void {
  $B1: {
    %4:u32 = mul %id, %num
    %x:u32 = let %4
    ret
  }
}
%foo = @compute @workgroup_size(1u, 64u, 1u) func(%inputs:foo_inputs):void {
  $B2: {
    %8:u32 = access %inputs, 0u
    %9:u32 = hlsl.WaveGetLaneCount
    %10:u32 = div %8, %9
    %11:u32 = hlsl.WaveGetLaneCount
    %12:u32 = add 63u, %11
    %13:u32 = div %12, %11
    %14:void = call %foo_inner, %10, %13
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_SubgroupIdAndNumSubgroups_LinearZ) {
    auto* subgroup_id = b.FunctionParam("id", ty.u32());
    subgroup_id->SetBuiltin(core::BuiltinValue::kSubgroupId);

    auto* num_subgroups = b.FunctionParam("num", ty.u32());
    num_subgroups->SetBuiltin(core::BuiltinValue::kNumSubgroups);

    auto* ep = b.ComputeFunction("foo", 1_u, 1_u, 64_u);
    ep->SetParams({subgroup_id, num_subgroups});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(subgroup_id, num_subgroups));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 64u) func(%id:u32 [@subgroup_id], %num:u32 [@num_subgroups]):void {
  $B1: {
    %4:u32 = mul %id, %num
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(4) {
  local_invocation_index:u32 @offset(0), @builtin(local_invocation_index)
}

%foo_inner = func(%id:u32, %num:u32):void {
  $B1: {
    %4:u32 = mul %id, %num
    %x:u32 = let %4
    ret
  }
}
%foo = @compute @workgroup_size(1u, 1u, 64u) func(%inputs:foo_inputs):void {
  $B2: {
    %8:u32 = access %inputs, 0u
    %9:u32 = hlsl.WaveGetLaneCount
    %10:u32 = div %8, %9
    %11:u32 = hlsl.WaveGetLaneCount
    %12:u32 = add 63u, %11
    %13:u32 = div %12, %11
    %14:void = call %foo_inner, %10, %13
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest,
       ShaderIOParameters_SubgroupIdAndNumSubgroups_Linear_UserDeclaredLocalInvocationIndex) {
    auto* subgroup_id = b.FunctionParam("id", ty.u32());
    subgroup_id->SetBuiltin(core::BuiltinValue::kSubgroupId);

    auto* num_subgroups = b.FunctionParam("num", ty.u32());
    num_subgroups->SetBuiltin(core::BuiltinValue::kNumSubgroups);

    auto* local_invocation_index = b.FunctionParam("lid", ty.u32());
    local_invocation_index->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);

    auto* ep = b.ComputeFunction("foo", 64_u, 1_u, 1_u);
    ep->SetParams({subgroup_id, num_subgroups, local_invocation_index});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Multiply(subgroup_id, local_invocation_index));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(64u, 1u, 1u) func(%id:u32 [@subgroup_id], %num:u32 [@num_subgroups], %lid:u32 [@local_invocation_index]):void {
  $B1: {
    %5:u32 = mul %id, %lid
    %x:u32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(4) {
  lid:u32 @offset(0), @builtin(local_invocation_index)
}

%foo_inner = func(%id:u32, %num:u32, %lid:u32):void {
  $B1: {
    %5:u32 = mul %id, %lid
    %x:u32 = let %5
    ret
  }
}
%foo = @compute @workgroup_size(64u, 1u, 1u) func(%inputs:foo_inputs):void {
  $B2: {
    %9:u32 = access %inputs, 0u
    %10:u32 = hlsl.WaveGetLaneCount
    %11:u32 = div %9, %10
    %12:u32 = hlsl.WaveGetLaneCount
    %13:u32 = add 63u, %12
    %14:u32 = div %13, %12
    %15:u32 = access %inputs, 0u
    %16:void = call %foo_inner, %11, %14, %15
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_WorkgroupIndex_ReuseExistingBuiltins) {
    auto* workgroup_id = b.FunctionParam("wgid", ty.vec3u());
    workgroup_id->SetBuiltin(core::BuiltinValue::kWorkgroupId);

    auto* num_workgroups = b.FunctionParam("numwgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* workgroup_index = b.FunctionParam("wgindex", ty.u32());
    workgroup_index->SetBuiltin(core::BuiltinValue::kWorkgroupIndex);

    auto* ep = b.ComputeFunction("foo", 3_u, 2_u, 1_u);
    ep->SetParams({workgroup_id, num_workgroups, workgroup_index});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Add(workgroup_index, 0_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%wgid:vec3<u32> [@workgroup_id], %numwgs:vec3<u32> [@num_workgroups], %wgindex:u32 [@workgroup_index]):void {
  $B1: {
    %5:u32 = add %wgindex, 0u
    %x:u32 = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  wgid:vec3<u32> @offset(0), @builtin(workgroup_id)
}

$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo_inner = func(%wgid:vec3<u32>, %numwgs:vec3<u32>, %wgindex:u32):void {
  $B2: {
    %6:u32 = add %wgindex, 0u
    %x:u32 = let %6
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %10:vec3<u32> = access %inputs, 0u
    %11:vec3<u32> = load %tint_num_workgroups
    %12:vec3<u32> = access %inputs, 0u
    %13:u32 = access %11, 0u
    %14:u32 = access %11, 1u
    %15:u32 = mul %13, %14
    %16:u32 = access %12, 2u
    %17:u32 = mul %16, %15
    %18:u32 = access %12, 1u
    %19:u32 = mul %18, %13
    %20:u32 = access %12, 0u
    %21:u32 = add %20, %19
    %22:u32 = add %21, %17
    %23:void = call %foo_inner, %10, %11, %22
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_WorkgroupIndex_AddMissingBuiltins) {
    auto* workgroup_index = b.FunctionParam("wgindex", ty.u32());
    workgroup_index->SetBuiltin(core::BuiltinValue::kWorkgroupIndex);

    auto* ep = b.ComputeFunction("foo", 3_u, 2_u, 1_u);
    ep->SetParams({workgroup_index});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Add(workgroup_index, 0_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%wgindex:u32 [@workgroup_index]):void {
  $B1: {
    %3:u32 = add %wgindex, 0u
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  workgroup_id:vec3<u32> @offset(0), @builtin(workgroup_id)
}

$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo_inner = func(%wgindex:u32):void {
  $B2: {
    %4:u32 = add %wgindex, 0u
    %x:u32 = let %4
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %8:vec3<u32> = access %inputs, 0u
    %9:vec3<u32> = load %tint_num_workgroups
    %10:u32 = access %9, 0u
    %11:u32 = access %9, 1u
    %12:u32 = mul %10, %11
    %13:u32 = access %8, 2u
    %14:u32 = mul %13, %12
    %15:u32 = access %8, 1u
    %16:u32 = mul %15, %10
    %17:u32 = access %8, 0u
    %18:u32 = add %17, %16
    %19:u32 = add %18, %14
    %20:void = call %foo_inner, %19
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_GlobalInvocationIndex_ReuseExistingBuiltins) {
    auto* num_workgroups = b.FunctionParam("numwgs", ty.vec3u());
    num_workgroups->SetBuiltin(core::BuiltinValue::kNumWorkgroups);

    auto* global_index = b.FunctionParam("gindex", ty.u32());
    global_index->SetBuiltin(core::BuiltinValue::kGlobalInvocationIndex);

    auto* ep = b.ComputeFunction("foo", 3_u, 2_u, 1_u);
    ep->SetParams({num_workgroups, global_index});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Add(global_index, 0_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%numwgs:vec3<u32> [@num_workgroups], %gindex:u32 [@global_invocation_index]):void {
  $B1: {
    %4:u32 = add %gindex, 0u
    %x:u32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  global_invocation_id:vec3<u32> @offset(0), @builtin(global_invocation_id)
}

$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo_inner = func(%numwgs:vec3<u32>, %gindex:u32):void {
  $B2: {
    %5:u32 = add %gindex, 0u
    %x:u32 = let %5
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %9:vec3<u32> = load %tint_num_workgroups
    %10:vec3<u32> = access %inputs, 0u
    %11:u32 = access %10, 0u
    %12:u32 = access %10, 1u
    %13:u32 = access %10, 2u
    %14:u32 = access %9, 0u
    %15:u32 = access %9, 1u
    %16:u32 = mul %14, 3u
    %17:u32 = mul %15, 2u
    %18:u32 = mul %16, %17
    %19:u32 = mul %13, %18
    %20:u32 = mul %12, %16
    %21:u32 = add %11, %20
    %22:u32 = add %21, %19
    %23:void = call %foo_inner, %9, %22
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(HlslWriterTransformTest, ShaderIOParameters_GlobalInvocationIndex_AddMissingBuiltins) {
    auto* global_index = b.FunctionParam("gindex", ty.u32());
    global_index->SetBuiltin(core::BuiltinValue::kGlobalInvocationIndex);

    auto* ep = b.ComputeFunction("foo", 3_u, 2_u, 1_u);
    ep->SetParams({global_index});
    b.Append(ep->Block(), [&] {
        b.Let("x", b.Add(global_index, 0_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%gindex:u32 [@global_invocation_index]):void {
  $B1: {
    %3:u32 = add %gindex, 0u
    %x:u32 = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(16) {
  global_invocation_id:vec3<u32> @offset(0), @builtin(global_invocation_id)
}

$B1: {  # root
  %tint_num_workgroups:ptr<uniform, vec3<u32>, read> = var undef @binding_point(0, 0)
}

%foo_inner = func(%gindex:u32):void {
  $B2: {
    %4:u32 = add %gindex, 0u
    %x:u32 = let %4
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%inputs:foo_inputs):void {
  $B3: {
    %8:vec3<u32> = load %tint_num_workgroups
    %9:vec3<u32> = access %inputs, 0u
    %10:u32 = access %9, 0u
    %11:u32 = access %9, 1u
    %12:u32 = access %9, 2u
    %13:u32 = access %8, 0u
    %14:u32 = access %8, 1u
    %15:u32 = mul %13, 3u
    %16:u32 = mul %14, 2u
    %17:u32 = mul %15, %16
    %18:u32 = mul %12, %17
    %19:u32 = mul %11, %15
    %20:u32 = add %10, %19
    %21:u32 = add %20, %18
    %22:void = call %foo_inner, %21
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
