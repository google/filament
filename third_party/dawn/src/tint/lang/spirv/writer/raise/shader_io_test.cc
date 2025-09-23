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

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/spirv/writer/raise/shader_io.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_ShaderIOTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_ShaderIOTest, NoInputsOrOutputs) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Return(ep);
    });

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

TEST_F(SpirvWriter_ShaderIOTest, Parameters_NonStruct) {
    auto* ep = b.Function("foo", ty.void_());
    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);
    auto* position = b.FunctionParam("position", ty.vec4<f32>());
    position->SetBuiltin(core::BuiltinValue::kPosition);
    position->SetInvariant(true);
    auto* color1 = b.FunctionParam("color1", ty.f32());
    color1->SetLocation(0);
    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample});

    ep->SetParams({front_facing, position, color1, color2});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {
            b.Multiply(ty.vec4<f32>(), position, b.Add(ty.f32(), color1, color2));
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
$B1: {  # root
  %foo_front_facing_Input:ptr<__in, bool, read> = var undef @builtin(front_facing)
  %foo_position_Input:ptr<__in, vec4<f32>, read> = var undef @invariant @builtin(position)
  %foo_loc0_Input:ptr<__in, f32, read> = var undef @location(0)
  %foo_loc1_Input:ptr<__in, f32, read> = var undef @location(1) @interpolate(linear, sample)
}

%foo_inner = func(%front_facing:bool, %position:vec4<f32>, %color1:f32, %color2:f32):void {
  $B2: {
    if %front_facing [t: $B3] {  # if_1
      $B3: {  # true
        %10:f32 = add %color1, %color2
        %11:vec4<f32> = mul %position, %10
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func():void {
  $B4: {
    %13:bool = load %foo_front_facing_Input
    %14:vec4<f32> = load %foo_position_Input
    %15:f32 = load %foo_loc0_Input
    %16:f32 = load %foo_loc1_Input
    %17:void = call %foo_inner, %13, %14, %15, %16
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Parameters_Struct) {
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
                                     ty.vec4<f32>(),
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

    auto* ep = b.Function("foo", ty.void_());
    auto* str_param = b.FunctionParam("inputs", str_ty);
    ep->SetParams({str_param});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(b.Access(ty.bool_(), str_param, 0_i));
        b.Append(ifelse->True(), [&] {
            auto* position = b.Access(ty.vec4<f32>(), str_param, 1_i);
            auto* color1 = b.Access(ty.f32(), str_param, 2_i);
            auto* color2 = b.Access(ty.f32(), str_param, 3_i);
            b.Multiply(ty.vec4<f32>(), position, b.Add(ty.f32(), color1, color2));
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

$B1: {  # root
  %foo_front_facing_Input:ptr<__in, bool, read> = var undef @builtin(front_facing)
  %foo_position_Input:ptr<__in, vec4<f32>, read> = var undef @invariant @builtin(position)
  %foo_loc0_Input:ptr<__in, f32, read> = var undef @location(0)
  %foo_loc1_Input:ptr<__in, f32, read> = var undef @location(1) @interpolate(linear, sample)
}

%foo_inner = func(%inputs:Inputs):void {
  $B2: {
    %7:bool = access %inputs, 0i
    if %7 [t: $B3] {  # if_1
      $B3: {  # true
        %8:vec4<f32> = access %inputs, 1i
        %9:f32 = access %inputs, 2i
        %10:f32 = access %inputs, 3i
        %11:f32 = add %9, %10
        %12:vec4<f32> = mul %8, %11
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func():void {
  $B4: {
    %14:bool = load %foo_front_facing_Input
    %15:vec4<f32> = load %foo_position_Input
    %16:f32 = load %foo_loc0_Input
    %17:f32 = load %foo_loc1_Input
    %18:Inputs = construct %14, %15, %16, %17
    %19:void = call %foo_inner, %18
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Parameters_Mixed) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Inputs"), {
                                                 {
                                                     mod.symbols.New("position"),
                                                     ty.vec4<f32>(),
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

    auto* ep = b.Function("foo", ty.void_());
    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);
    auto* str_param = b.FunctionParam("inputs", str_ty);
    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetLocation(1);
    color2->SetInterpolation(core::Interpolation{core::InterpolationType::kLinear,
                                                 core::InterpolationSampling::kSample});

    ep->SetParams({front_facing, str_param, color2});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {
            auto* position = b.Access(ty.vec4<f32>(), str_param, 0_i);
            auto* color1 = b.Access(ty.f32(), str_param, 1_i);
            b.Multiply(ty.vec4<f32>(), position, b.Add(ty.f32(), color1, color2));
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

$B1: {  # root
  %foo_front_facing_Input:ptr<__in, bool, read> = var undef @builtin(front_facing)
  %foo_position_Input:ptr<__in, vec4<f32>, read> = var undef @invariant @builtin(position)
  %foo_loc0_Input:ptr<__in, f32, read> = var undef @location(0)
  %foo_loc1_Input:ptr<__in, f32, read> = var undef @location(1) @interpolate(linear, sample)
}

%foo_inner = func(%front_facing:bool, %inputs:Inputs, %color2:f32):void {
  $B2: {
    if %front_facing [t: $B3] {  # if_1
      $B3: {  # true
        %9:vec4<f32> = access %inputs, 0i
        %10:f32 = access %inputs, 1i
        %11:f32 = add %10, %color2
        %12:vec4<f32> = mul %9, %11
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func():void {
  $B4: {
    %14:bool = load %foo_front_facing_Input
    %15:vec4<f32> = load %foo_position_Input
    %16:f32 = load %foo_loc0_Input
    %17:Inputs = construct %15, %16
    %18:f32 = load %foo_loc1_Input
    %19:void = call %foo_inner, %14, %17, %18
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_NonStructBuiltin) {
    auto* ep = b.Function("foo", ty.vec4<f32>());
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    ep->SetReturnInvariant(true);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f));
    });

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
$B1: {  # root
  %foo_position_Output:ptr<__out, vec4<f32>, write> = var undef @invariant @builtin(position)
}

%foo_inner = func():vec4<f32> {
  $B2: {
    %3:vec4<f32> = construct 0.5f
    ret %3
  }
}
%foo = @vertex func():void {
  $B3: {
    %5:vec4<f32> = call %foo_inner
    store %foo_position_Output, %5
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_NonStructLocation) {
    auto* ep = b.Function("foo", ty.vec4<f32>());
    ep->SetReturnLocation(1u);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f));
    });

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
$B1: {  # root
  %foo_loc1_Output:ptr<__out, vec4<f32>, write> = var undef @location(1)
}

%foo_inner = func():vec4<f32> {
  $B2: {
    %3:vec4<f32> = construct 0.5f
    ret %3
  }
}
%foo = @fragment func():void {
  $B3: {
    %5:vec4<f32> = call %foo_inner
    store %foo_loc1_Output, %5
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
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

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(str_ty, b.Construct(ty.vec4<f32>(), 0_f), 0.25_f, 0.75_f));
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

$B1: {  # root
  %foo_position_Output:ptr<__out, vec4<f32>, write> = var undef @invariant @builtin(position)
  %foo_loc0_Output:ptr<__out, f32, write> = var undef @location(0)
  %foo_loc1_Output:ptr<__out, f32, write> = var undef @location(1) @interpolate(linear, sample)
}

%foo_inner = func():Outputs {
  $B2: {
    %5:vec4<f32> = construct 0.0f
    %6:Outputs = construct %5, 0.25f, 0.75f
    ret %6
  }
}
%foo = @vertex func():void {
  $B3: {
    %8:Outputs = call %foo_inner
    %9:vec4<f32> = access %8, 0u
    store %foo_position_Output, %9
    %10:f32 = access %8, 1u
    store %foo_loc0_Output, %10
    %11:f32 = access %8, 2u
    store %foo_loc1_Output, %11
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_DualSourceBlending) {
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

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(str_ty, 0.25_f, 0.75_f));
    });

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

$B1: {  # root
  %foo_loc0_idx0_Output:ptr<__out, f32, write> = var undef @location(0) @blend_src(0)
  %foo_loc0_idx1_Output:ptr<__out, f32, write> = var undef @location(0) @blend_src(1)
}

%foo_inner = func():Output {
  $B2: {
    %4:Output = construct 0.25f, 0.75f
    ret %4
  }
}
%foo = @fragment func():void {
  $B3: {
    %6:Output = call %foo_inner
    %7:f32 = access %6, 0u
    store %foo_loc0_idx0_Output, %7
    %8:f32 = access %6, 1u
    store %foo_loc0_idx1_Output, %8
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Struct_SharedWithBuffer) {
    auto* vec4f = ty.vec4<f32>();
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

    auto* var = b.Var(ty.ptr(storage, str_ty, read));
    var->SetBindingPoint(0, 0);
    auto* buffer = mod.root_block->Append(var);

    auto* ep = b.Function("vert", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Load(buffer));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color:vec4<f32> @offset(16), @location(0)
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
  color:vec4<f32> @offset(16)
}

$B1: {  # root
  %1:ptr<storage, Outputs, read> = var undef @binding_point(0, 0)
  %vert_position_Output:ptr<__out, vec4<f32>, write> = var undef @builtin(position)
  %vert_loc0_Output:ptr<__out, vec4<f32>, write> = var undef @location(0)
}

%vert_inner = func():Outputs {
  $B2: {
    %5:Outputs = load %1
    ret %5
  }
}
%vert = @vertex func():void {
  $B3: {
    %7:Outputs = call %vert_inner
    %8:vec4<f32> = access %7, 0u
    store %vert_position_Output, %8
    %9:vec4<f32> = access %7, 1u
    store %vert_loc0_Output, %9
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that we change the type of the sample mask builtin to an array for SPIR-V.
TEST_F(SpirvWriter_ShaderIOTest, SampleMask) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("color"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         .location = 0u,
                                     },
                                 },
                                 {
                                     mod.symbols.New("mask"),
                                     ty.u32(),
                                     core::IOAttributes{
                                         .builtin = core::BuiltinValue::kSampleMask,
                                     },
                                 },
                             });

    auto* mask_in = b.FunctionParam("mask_in", ty.u32());
    mask_in->SetBuiltin(core::BuiltinValue::kSampleMask);

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);
    ep->SetParams({mask_in});

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(str_ty, 0.5_f, mask_in));
    });

    auto* src = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0), @location(0)
  mask:u32 @offset(4), @builtin(sample_mask)
}

%foo = @fragment func(%mask_in:u32 [@sample_mask]):Outputs {
  $B1: {
    %3:Outputs = construct 0.5f, %mask_in
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0)
  mask:u32 @offset(4)
}

$B1: {  # root
  %foo_sample_mask_Input:ptr<__in, array<u32, 1>, read> = var undef @builtin(sample_mask)
  %foo_loc0_Output:ptr<__out, f32, write> = var undef @location(0)
  %foo_sample_mask_Output:ptr<__out, array<u32, 1>, write> = var undef @builtin(sample_mask)
}

%foo_inner = func(%mask_in:u32):Outputs {
  $B2: {
    %6:Outputs = construct 0.5f, %mask_in
    ret %6
  }
}
%foo = @fragment func():void {
  $B3: {
    %8:ptr<__in, u32, read> = access %foo_sample_mask_Input, 0u
    %9:u32 = load %8
    %10:Outputs = call %foo_inner, %9
    %11:f32 = access %10, 0u
    store %foo_loc0_Output, %11
    %12:u32 = access %10, 1u
    %13:ptr<__out, u32, write> = access %foo_sample_mask_Output, 0u
    store %13, %12
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that interpolation attributes are stripped from vertex inputs.
TEST_F(SpirvWriter_ShaderIOTest, InterpolationOnVertexInput) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"),
                             {
                                 {
                                     mod.symbols.New("color"),
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

    auto* ep = b.Function("vert", ty.vec4<f32>());
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    ep->SetReturnInvariant(true);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    auto* str_param = b.FunctionParam("input", str_ty);
    auto* ival = b.FunctionParam("ival", ty.i32());
    ival->SetLocation(1);
    ival->SetInterpolation(core::Interpolation{core::InterpolationType::kFlat});
    ep->SetParams({str_param, ival});

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f));
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  color:f32 @offset(0), @location(1), @interpolate(linear, sample)
}

%vert = @vertex func(%input:MyStruct, %ival:i32 [@location(1), @interpolate(flat)]):vec4<f32> [@invariant, @position] {
  $B1: {
    %4:vec4<f32> = construct 0.5f
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  color:f32 @offset(0)
}

$B1: {  # root
  %vert_loc1_Input:ptr<__in, f32, read> = var undef @location(1)
  %vert_loc1_Input_1:ptr<__in, i32, read> = var undef @location(1)  # %vert_loc1_Input_1: 'vert_loc1_Input'
  %vert_position_Output:ptr<__out, vec4<f32>, write> = var undef @invariant @builtin(position)
}

%vert_inner = func(%input:MyStruct, %ival:i32):vec4<f32> {
  $B2: {
    %7:vec4<f32> = construct 0.5f
    ret %7
  }
}
%vert = @vertex func():void {
  $B3: {
    %9:f32 = load %vert_loc1_Input
    %10:MyStruct = construct %9
    %11:i32 = load %vert_loc1_Input_1
    %12:vec4<f32> = call %vert_inner, %10, %11
    store %vert_position_Output, %12
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that interpolation attributes are stripped from fragment struct outputs.
TEST_F(SpirvWriter_ShaderIOTest, InterpolationOnFragmentOutput_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"),
                             {
                                 {
                                     mod.symbols.New("color"),
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

    auto* ep = b.Function("frag1", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(str_ty, 0.5_f));
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  color:f32 @offset(0), @location(1), @interpolate(linear, sample)
}

%frag1 = @fragment func():MyStruct {
  $B1: {
    %2:MyStruct = construct 0.5f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  color:f32 @offset(0)
}

$B1: {  # root
  %frag1_loc1_Output:ptr<__out, f32, write> = var undef @location(1)
}

%frag1_inner = func():MyStruct {
  $B2: {
    %3:MyStruct = construct 0.5f
    ret %3
  }
}
%frag1 = @fragment func():void {
  $B3: {
    %5:MyStruct = call %frag1_inner
    %6:f32 = access %5, 0u
    store %frag1_loc1_Output, %6
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that interpolation attributes are stripped from fragment non-struct outputs.
TEST_F(SpirvWriter_ShaderIOTest, InterpolationOnFragmentOutput_NonStruct) {
    auto* ep = b.Function("frag2", ty.i32());
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0);
    ep->SetReturnInterpolation(core::Interpolation{core::InterpolationType::kFlat});

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Constant(42_i));
    });

    auto* src = R"(
%frag2 = @fragment func():i32 [@location(0), @interpolate(flat)] {
  $B1: {
    ret 42i
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %frag2_loc0_Output:ptr<__out, i32, write> = var undef @location(0)
}

%frag2_inner = func():i32 {
  $B2: {
    ret 42i
  }
}
%frag2 = @fragment func():void {
  $B3: {
    %4:i32 = call %frag2_inner
    store %frag2_loc0_Output, %4
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ClampFragDepth) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Outputs"), {
                                                  {
                                                      mod.symbols.New("color"),
                                                      ty.f32(),
                                                      core::IOAttributes{
                                                          .location = 0u,
                                                      },
                                                  },
                                                  {
                                                      mod.symbols.New("depth"),
                                                      ty.f32(),
                                                      core::IOAttributes{
                                                          .builtin = core::BuiltinValue::kFragDepth,
                                                      },
                                                  },
                                              });

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(str_ty, 0.5_f, 2_f));
    });

    auto* src = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0), @location(0)
  depth:f32 @offset(4), @builtin(frag_depth)
}

%foo = @fragment func():Outputs {
  $B1: {
    %2:Outputs = construct 0.5f, 2.0f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0)
  depth:f32 @offset(4)
}

tint_immediate_data_struct = struct @align(4), @block {
  depth_min:f32 @offset(4)
  depth_max:f32 @offset(8)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
  %foo_loc0_Output:ptr<__out, f32, write> = var undef @location(0)
  %foo_frag_depth_Output:ptr<__out, f32, write> = var undef @builtin(frag_depth)
}

%foo_inner = func():Outputs {
  $B2: {
    %5:Outputs = construct 0.5f, 2.0f
    ret %5
  }
}
%foo = @fragment func():void {
  $B3: {
    %7:Outputs = call %foo_inner
    %8:f32 = access %7, 0u
    store %foo_loc0_Output, %8
    %9:f32 = access %7, 1u
    %10:ptr<immediate, f32, read> = access %tint_immediate_data, 0u
    %11:f32 = load %10
    %12:ptr<immediate, f32, read> = access %tint_immediate_data, 1u
    %13:f32 = load %12
    %14:f32 = clamp %9, %11, %13
    store %foo_frag_depth_Output, %14
    ret
  }
}
)";

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    immediate_data_config.AddInternalImmediateData(4, mod.symbols.New("depth_min"), ty.f32());
    immediate_data_config.AddInternalImmediateData(8, mod.symbols.New("depth_max"), ty.f32());
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    ShaderIOConfig config{immediate_data.Get()};
    config.depth_range_offsets = {4, 8};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, EmitVertexPointSize) {
    auto* ep = b.Function("foo", ty.vec4<f32>());
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4<f32>(), 0.5_f));
    });

    auto* src = R"(
%foo = @vertex func():vec4<f32> [@position] {
  $B1: {
    %2:vec4<f32> = construct 0.5f
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %foo_position_Output:ptr<__out, vec4<f32>, write> = var undef @builtin(position)
  %foo___point_size_Output:ptr<__out, f32, write> = var undef @builtin(__point_size)
}

%foo_inner = func():vec4<f32> {
  $B2: {
    %4:vec4<f32> = construct 0.5f
    ret %4
  }
}
%foo = @vertex func():void {
  $B3: {
    %6:vec4<f32> = call %foo_inner
    store %foo_position_Output, %6
    store %foo___point_size_Output, 1.0f
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.emit_vertex_point_size = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, F16_IO_WithoutPolyfill) {
    auto* outputs = ty.Struct(mod.symbols.New("Outputs"), {
                                                              {
                                                                  mod.symbols.New("out1"),
                                                                  ty.f16(),
                                                                  core::IOAttributes{
                                                                      .location = 1u,
                                                                  },
                                                              },
                                                              {
                                                                  mod.symbols.New("out2"),
                                                                  ty.vec4<f16>(),
                                                                  core::IOAttributes{
                                                                      .location = 2u,
                                                                  },
                                                              },
                                                          });

    auto* in1 = b.FunctionParam("in1", ty.f16());
    auto* in2 = b.FunctionParam("in2", ty.vec4<f16>());
    in1->SetLocation(1);
    in2->SetLocation(2);
    auto* func = b.Function("main", outputs, core::ir::Function::PipelineStage::kFragment);
    func->SetParams({in1, in2});
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(outputs, in1, in2));
    });

    auto* src = R"(
Outputs = struct @align(8) {
  out1:f16 @offset(0), @location(1)
  out2:vec4<f16> @offset(8), @location(2)
}

%main = @fragment func(%in1:f16 [@location(1)], %in2:vec4<f16> [@location(2)]):Outputs {
  $B1: {
    %4:Outputs = construct %in1, %in2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(8) {
  out1:f16 @offset(0)
  out2:vec4<f16> @offset(8)
}

$B1: {  # root
  %main_loc1_Input:ptr<__in, f16, read> = var undef @location(1)
  %main_loc2_Input:ptr<__in, vec4<f16>, read> = var undef @location(2)
  %main_loc1_Output:ptr<__out, f16, write> = var undef @location(1)
  %main_loc2_Output:ptr<__out, vec4<f16>, write> = var undef @location(2)
}

%main_inner = func(%in1:f16, %in2:vec4<f16>):Outputs {
  $B2: {
    %8:Outputs = construct %in1, %in2
    ret %8
  }
}
%main = @fragment func():void {
  $B3: {
    %10:f16 = load %main_loc1_Input
    %11:vec4<f16> = load %main_loc2_Input
    %12:Outputs = call %main_inner, %10, %11
    %13:f16 = access %12, 0u
    store %main_loc1_Output, %13
    %14:vec4<f16> = access %12, 1u
    store %main_loc2_Output, %14
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.polyfill_f16_io = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, F16_IO_WithPolyfill) {
    auto* outputs = ty.Struct(mod.symbols.New("Outputs"), {
                                                              {
                                                                  mod.symbols.New("out1"),
                                                                  ty.f16(),
                                                                  core::IOAttributes{
                                                                      .location = 1u,
                                                                  },
                                                              },
                                                              {
                                                                  mod.symbols.New("out2"),
                                                                  ty.vec4<f16>(),
                                                                  core::IOAttributes{
                                                                      .location = 2u,
                                                                  },
                                                              },
                                                          });

    auto* in1 = b.FunctionParam("in1", ty.f16());
    auto* in2 = b.FunctionParam("in2", ty.vec4<f16>());
    in1->SetLocation(1);
    in2->SetLocation(2);
    auto* func = b.Function("main", outputs, core::ir::Function::PipelineStage::kFragment);
    func->SetParams({in1, in2});
    b.Append(func->Block(), [&] {  //
        b.Return(func, b.Construct(outputs, in1, in2));
    });

    auto* src = R"(
Outputs = struct @align(8) {
  out1:f16 @offset(0), @location(1)
  out2:vec4<f16> @offset(8), @location(2)
}

%main = @fragment func(%in1:f16 [@location(1)], %in2:vec4<f16> [@location(2)]):Outputs {
  $B1: {
    %4:Outputs = construct %in1, %in2
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(8) {
  out1:f16 @offset(0)
  out2:vec4<f16> @offset(8)
}

$B1: {  # root
  %main_loc1_Input:ptr<__in, f32, read> = var undef @location(1)
  %main_loc2_Input:ptr<__in, vec4<f32>, read> = var undef @location(2)
  %main_loc1_Output:ptr<__out, f32, write> = var undef @location(1)
  %main_loc2_Output:ptr<__out, vec4<f32>, write> = var undef @location(2)
}

%main_inner = func(%in1:f16, %in2:vec4<f16>):Outputs {
  $B2: {
    %8:Outputs = construct %in1, %in2
    ret %8
  }
}
%main = @fragment func():void {
  $B3: {
    %10:f32 = load %main_loc1_Input
    %11:f16 = convert %10
    %12:vec4<f32> = load %main_loc2_Input
    %13:vec4<f16> = convert %12
    %14:Outputs = call %main_inner, %11, %13
    %15:f16 = access %14, 0u
    %16:f32 = convert %15
    store %main_loc1_Output, %16
    %17:vec4<f16> = access %14, 1u
    %18:vec4<f32> = convert %17
    store %main_loc2_Output, %18
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.polyfill_f16_io = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
