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

#include "src/tint/lang/msl/writer/raise/shader_io.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using MslWriter_ShaderIOTest = core::ir::transform::TransformTest;

TEST_F(MslWriter_ShaderIOTest, NoInputsOrOutputs) {
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

TEST_F(MslWriter_ShaderIOTest, Parameters_NonStruct) {
    auto* ep = b.Function("foo", ty.void_());
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

    ep->SetParams({front_facing, position, color1, color2});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

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
foo_inputs = struct @align(4) {
  color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
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
%foo = @fragment func(%front_facing_1:bool [@front_facing], %position_1:vec4<f32> [@invariant, @position], %inputs:foo_inputs):void {  # %front_facing_1: 'front_facing', %position_1: 'position'
  $B3: {
    %12:f32 = access %inputs, 0u
    %13:f32 = access %inputs, 1u
    %14:void = call %foo_inner, %front_facing_1, %position_1, %12, %13
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Parameters_Struct) {
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

    auto* ep = b.Function("foo", ty.void_());
    auto* str_param = b.FunctionParam("inputs", str_ty);
    ep->SetParams({str_param});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

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

foo_inputs = struct @align(4) {
  Inputs_color1:f32 @offset(0), @location(0)
  Inputs_color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
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
%foo = @fragment func(%Inputs_front_facing:bool [@front_facing], %Inputs_position:vec4<f32> [@invariant, @position], %inputs_1:foo_inputs):void {  # %inputs_1: 'inputs'
  $B3: {
    %13:f32 = access %inputs_1, 0u
    %14:f32 = access %inputs_1, 1u
    %15:Inputs = construct %Inputs_front_facing, %Inputs_position, %13, %14
    %16:void = call %foo_inner, %15
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Parameters_Mixed) {
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

foo_inputs = struct @align(4) {
  Inputs_color1:f32 @offset(0), @location(0)
  color2:f32 @offset(4), @location(1), @interpolate(linear, sample)
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
%foo = @fragment func(%front_facing_1:bool [@front_facing], %Inputs_position:vec4<f32> [@invariant, @position], %inputs_1:foo_inputs):void {  # %front_facing_1: 'front_facing', %inputs_1: 'inputs'
  $B3: {
    %13:f32 = access %inputs_1, 0u
    %14:Inputs = construct %Inputs_position, %13
    %15:f32 = access %inputs_1, 1u
    %16:void = call %foo_inner, %front_facing_1, %14, %15
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, ReturnValue_NonStructBuiltin) {
    auto* ep = b.Function("foo", ty.vec4f());
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    ep->SetReturnInvariant(true);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var undef
    %6:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %6, %4
    %7:foo_outputs = load %tint_wrapper_result
    ret %7
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, ReturnValue_NonStructLocation) {
    auto* ep = b.Function("foo", ty.vec4f());
    ep->SetReturnLocation(1u);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var undef
    %6:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %6, %4
    %7:foo_outputs = load %tint_wrapper_result
    ret %7
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, ReturnValue_Struct) {
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

    auto* ep = b.Function("foo", str_ty);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    b.Append(ep->Block(), [&] {  //
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
  Outputs_position:vec4<f32> @offset(0), @invariant, @builtin(position)
  Outputs_color1:f32 @offset(16), @location(0)
  Outputs_color2:f32 @offset(20), @location(1), @interpolate(linear, sample)
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var undef
    %10:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %10, %6
    %11:ptr<function, f32, read_write> = access %tint_wrapper_result, 1u
    store %11, %7
    %12:ptr<function, f32, read_write> = access %tint_wrapper_result, 2u
    store %12, %8
    %13:foo_outputs = load %tint_wrapper_result
    ret %13
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, ReturnValue_DualSourceBlending) {
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var undef
    %8:ptr<function, f32, read_write> = access %tint_wrapper_result, 0u
    store %8, %5
    %9:ptr<function, f32, read_write> = access %tint_wrapper_result, 1u
    store %9, %6
    %10:foo_outputs = load %tint_wrapper_result
    ret %10
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Struct_SharedWithBuffer) {
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

vert_outputs = struct @align(16) {
  Outputs_position:vec4<f32> @offset(0), @builtin(position)
  Outputs_color:vec4<f32> @offset(16), @location(0)
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
    %7:vec4<f32> = access %5, 1u
    %tint_wrapper_result:ptr<function, vert_outputs, read_write> = var undef
    %9:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %9, %6
    %10:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 1u
    store %10, %7
    %11:vert_outputs = load %tint_wrapper_result
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that IO attributes are stripped from structures that are not used for the shader interface.
TEST_F(MslWriter_ShaderIOTest, StructWithAttributes_NotUsedForInterface) {
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

    auto* ep = b.Function("frag", ty.void_());
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
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

TEST_F(MslWriter_ShaderIOTest, EmitVertexPointSize) {
    auto* ep = b.Function("foo", ty.vec4f());
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
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
foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @builtin(position)
  vertex_point_size:f32 @offset(16), @builtin(__point_size)
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var undef
    %6:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %6, %4
    %7:ptr<function, f32, read_write> = access %tint_wrapper_result, 1u
    store %7, 1.0f
    %8:foo_outputs = load %tint_wrapper_result
    ret %8
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.emit_vertex_point_size = true;

    capabilities.Set(core::ir::Capability::kAllowPointSizeBuiltin, true);
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Color_NonStruct) {
    auto* ep = b.Function("foo", ty.void_());
    auto* color1 = b.FunctionParam("color1", ty.f32());
    color1->SetColor(1);
    auto* color2 = b.FunctionParam("color2", ty.f32());
    color2->SetColor(2);

    ep->SetParams({color1, color2});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {
        b.Add(color1, color2);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @fragment func(%color1:f32 [@color(1)], %color2:f32 [@color(2)]):void {
  $B1: {
    %4:f32 = add %color1, %color2
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_inputs = struct @align(4) {
  color1:f32 @offset(0), @color(1)
  color2:f32 @offset(4), @color(2)
}

%foo_inner = func(%color1:f32, %color2:f32):void {
  $B1: {
    %4:f32 = add %color1, %color2
    ret
  }
}
%foo = @fragment func(%inputs:foo_inputs):void {
  $B2: {
    %7:f32 = access %inputs, 0u
    %8:f32 = access %inputs, 1u
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

TEST_F(MslWriter_ShaderIOTest, UserSuppliedMask_WithoutFixedSampleMask) {
    core::IOAttributes loc;
    loc.location = 1u;
    core::IOAttributes mask;
    mask.builtin = core::BuiltinValue::kSampleMask;
    auto* outputs =
        ty.Struct(mod.symbols.New("Outputs"), {
                                                  {mod.symbols.New("color"), ty.vec4f(), loc},
                                                  {mod.symbols.New("mask"), ty.u32(), mask},
                                              });

    auto* ep = b.Function("foo", outputs);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(outputs, b.Splat(ty.vec4f(), 0.5_f), b.Constant(u32(0x10203040))));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  color:vec4<f32> @offset(0), @location(1)
  mask:u32 @offset(16), @builtin(sample_mask)
}

%foo = @fragment func():Outputs {
  $B1: {
    %2:Outputs = construct vec4<f32>(0.5f), 270544960u
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  color:vec4<f32> @offset(0)
  mask:u32 @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_color:vec4<f32> @offset(0), @location(1)
  Outputs_mask:u32 @offset(16), @builtin(sample_mask)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:Outputs = construct vec4<f32>(0.5f), 270544960u
    ret %2
  }
}
%foo = @fragment func():foo_outputs {
  $B2: {
    %4:Outputs = call %foo_inner
    %5:vec4<f32> = access %4, 0u
    %6:u32 = access %4, 1u
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var undef
    %8:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %8, %5
    %9:ptr<function, u32, read_write> = access %tint_wrapper_result, 1u
    store %9, %6
    %10:foo_outputs = load %tint_wrapper_result
    ret %10
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, FixedSampleMask) {
    auto* ep = b.Function("foo", ty.vec4f());
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0u);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Splat(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%foo = @fragment func():vec4<f32> [@location(0)] {
  $B1: {
    ret vec4<f32>(0.5f)
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
foo_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @location(0)
  tint_sample_mask:u32 @offset(16), @builtin(sample_mask)
}

%foo_inner = func():vec4<f32> {
  $B1: {
    ret vec4<f32>(0.5f)
  }
}
%foo = @fragment func():foo_outputs {
  $B2: {
    %3:vec4<f32> = call %foo_inner
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var undef
    %5:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %5, %3
    %6:ptr<function, u32, read_write> = access %tint_wrapper_result, 1u
    store %6, 12345678u
    %7:foo_outputs = load %tint_wrapper_result
    ret %7
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.fixed_sample_mask = 12345678u;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, FixedSampleMask_WithUserSuppliedMask) {
    core::IOAttributes loc;
    loc.location = 1u;
    core::IOAttributes mask;
    mask.builtin = core::BuiltinValue::kSampleMask;
    auto* outputs =
        ty.Struct(mod.symbols.New("Outputs"), {
                                                  {mod.symbols.New("color"), ty.vec4f(), loc},
                                                  {mod.symbols.New("mask"), ty.u32(), mask},
                                              });

    auto* ep = b.Function("foo", outputs);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(outputs, b.Splat(ty.vec4f(), 0.5_f), b.Constant(u32(0x10203040))));
    });

    auto* src = R"(
Outputs = struct @align(16) {
  color:vec4<f32> @offset(0), @location(1)
  mask:u32 @offset(16), @builtin(sample_mask)
}

%foo = @fragment func():Outputs {
  $B1: {
    %2:Outputs = construct vec4<f32>(0.5f), 270544960u
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(16) {
  color:vec4<f32> @offset(0)
  mask:u32 @offset(16)
}

foo_outputs = struct @align(16) {
  Outputs_color:vec4<f32> @offset(0), @location(1)
  Outputs_mask:u32 @offset(16), @builtin(sample_mask)
}

%foo_inner = func():Outputs {
  $B1: {
    %2:Outputs = construct vec4<f32>(0.5f), 270544960u
    ret %2
  }
}
%foo = @fragment func():foo_outputs {
  $B2: {
    %4:Outputs = call %foo_inner
    %5:vec4<f32> = access %4, 0u
    %6:u32 = access %4, 1u
    %7:u32 = and %6, 12345678u
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var undef
    %9:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %9, %5
    %10:ptr<function, u32, read_write> = access %tint_wrapper_result, 1u
    store %10, %7
    %11:foo_outputs = load %tint_wrapper_result
    ret %11
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.fixed_sample_mask = 12345678u;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Color_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"), {
                                                            {
                                                                mod.symbols.New("color1"),
                                                                ty.f32(),
                                                                core::IOAttributes{
                                                                    .color = 1u,
                                                                },
                                                            },
                                                            {
                                                                mod.symbols.New("color2"),
                                                                ty.f32(),
                                                                core::IOAttributes{
                                                                    .color = 2u,
                                                                },
                                                            },
                                                        });

    auto* ep = b.Function("foo", ty.void_());
    auto* str_param = b.FunctionParam("inputs", str_ty);
    ep->SetParams({str_param});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {
        auto* color1 = b.Access<f32>(str_param, 0_i);
        auto* color2 = b.Access<f32>(str_param, 1_i);
        b.Add(color1, color2);
        b.Return(ep);
    });

    auto* src = R"(
Inputs = struct @align(4) {
  color1:f32 @offset(0), @color(1)
  color2:f32 @offset(4), @color(2)
}

%foo = @fragment func(%inputs:Inputs):void {
  $B1: {
    %3:f32 = access %inputs, 0i
    %4:f32 = access %inputs, 1i
    %5:f32 = add %3, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inputs = struct @align(4) {
  color1:f32 @offset(0)
  color2:f32 @offset(4)
}

foo_inputs = struct @align(4) {
  Inputs_color1:f32 @offset(0), @color(1)
  Inputs_color2:f32 @offset(4), @color(2)
}

%foo_inner = func(%inputs:Inputs):void {
  $B1: {
    %3:f32 = access %inputs, 0i
    %4:f32 = access %inputs, 1i
    %5:f32 = add %3, %4
    ret
  }
}
%foo = @fragment func(%inputs_1:foo_inputs):void {  # %inputs_1: 'inputs'
  $B2: {
    %8:f32 = access %inputs_1, 0u
    %9:f32 = access %inputs_1, 1u
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

TEST_F(MslWriter_ShaderIOTest, UnnamedParameter) {
    auto* ep = b.Function("foo", ty.void_());
    auto* front_facing = b.FunctionParam(ty.bool_());
    front_facing->SetBuiltin(core::BuiltinValue::kFrontFacing);
    ep->SetParams({front_facing});
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);
    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] { b.ExitIf(ifelse); });
        b.Return(ep);
    });

    auto* src = R"(
%foo = @fragment func(%2:bool [@front_facing]):void {
  $B1: {
    if %2 [t: $B2] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo_inner = func(%2:bool):void {
  $B1: {
    if %2 [t: $B2] {  # if_1
      $B2: {  # true
        exit_if  # if_1
      }
    }
    ret
  }
}
%foo = @fragment func(%4:bool [@front_facing]):void {
  $B3: {
    %5:void = call %foo_inner, %4
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, ClampFragDepth) {
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

foo_outputs = struct @align(4) {
  Outputs_color:f32 @offset(0), @location(0)
  Outputs_depth:f32 @offset(4), @builtin(frag_depth)
}

$B1: {  # root
  %tint_immediate_data:ptr<immediate, tint_immediate_data_struct, read> = var undef
}

%foo_inner = func():Outputs {
  $B2: {
    %3:Outputs = construct 0.5f, 2.0f
    ret %3
  }
}
%foo = @fragment func():foo_outputs {
  $B3: {
    %5:Outputs = call %foo_inner
    %6:f32 = access %5, 0u
    %7:f32 = access %5, 1u
    %8:ptr<immediate, f32, read> = access %tint_immediate_data, 0u
    %9:f32 = load %8
    %10:ptr<immediate, f32, read> = access %tint_immediate_data, 1u
    %11:f32 = load %10
    %12:f32 = clamp %7, %9, %11
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var undef
    %14:ptr<function, f32, read_write> = access %tint_wrapper_result, 0u
    store %14, %6
    %15:ptr<function, f32, read_write> = access %tint_wrapper_result, 1u
    store %15, %12
    %16:foo_outputs = load %tint_wrapper_result
    ret %16
  }
}
)";

    core::ir::transform::PrepareImmediateDataConfig immediate_data_config;
    ASSERT_EQ(
        immediate_data_config.AddInternalImmediateData(4, mod.symbols.New("depth_min"), ty.f32()),
        Success);
    ASSERT_EQ(
        immediate_data_config.AddInternalImmediateData(8, mod.symbols.New("depth_max"), ty.f32()),
        Success);
    auto immediate_data = PrepareImmediateData(mod, immediate_data_config);
    EXPECT_EQ(immediate_data, Success);

    ShaderIOConfig config{immediate_data.Get()};
    config.depth_range_offsets = {4, 8};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, WorkgroupIndex_ReuseExistingBuiltins) {
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
%foo_inner = func(%wgid:vec3<u32>, %numwgs:vec3<u32>, %wgindex:u32):void {
  $B1: {
    %5:u32 = add %wgindex, 0u
    %x:u32 = let %5
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%wgid_1:vec3<u32> [@workgroup_id], %numwgs_1:vec3<u32> [@num_workgroups]):void {  # %wgid_1: 'wgid', %numwgs_1: 'numwgs'
  $B2: {
    %10:u32 = access %numwgs_1, 0u
    %11:u32 = access %numwgs_1, 1u
    %12:u32 = mul %10, %11
    %13:u32 = access %wgid_1, 2u
    %14:u32 = mul %13, %12
    %15:u32 = access %wgid_1, 1u
    %16:u32 = mul %15, %10
    %17:u32 = access %wgid_1, 0u
    %18:u32 = add %17, %16
    %19:u32 = add %18, %14
    %20:void = call %foo_inner, %wgid_1, %numwgs_1, %19
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, WorkgroupIndex_AddMissingBuiltins) {
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
%foo_inner = func(%wgindex:u32):void {
  $B1: {
    %3:u32 = add %wgindex, 0u
    %x:u32 = let %3
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%workgroup_id:vec3<u32> [@workgroup_id], %num_workgroups:vec3<u32> [@num_workgroups]):void {
  $B2: {
    %8:u32 = access %num_workgroups, 0u
    %9:u32 = access %num_workgroups, 1u
    %10:u32 = mul %8, %9
    %11:u32 = access %workgroup_id, 2u
    %12:u32 = mul %11, %10
    %13:u32 = access %workgroup_id, 1u
    %14:u32 = mul %13, %8
    %15:u32 = access %workgroup_id, 0u
    %16:u32 = add %15, %14
    %17:u32 = add %16, %12
    %18:void = call %foo_inner, %17
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, GlobalInvocationIndex_ReuseExistingBuiltins) {
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
%foo_inner = func(%numwgs:vec3<u32>, %gindex:u32):void {
  $B1: {
    %4:u32 = add %gindex, 0u
    %x:u32 = let %4
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%numwgs_1:vec3<u32> [@num_workgroups], %global_invocation_id:vec3<u32> [@global_invocation_id]):void {  # %numwgs_1: 'numwgs'
  $B2: {
    %9:u32 = access %global_invocation_id, 0u
    %10:u32 = access %global_invocation_id, 1u
    %11:u32 = access %global_invocation_id, 2u
    %12:u32 = access %numwgs_1, 0u
    %13:u32 = access %numwgs_1, 1u
    %14:u32 = mul %12, 3u
    %15:u32 = mul %13, 2u
    %16:u32 = mul %14, %15
    %17:u32 = mul %11, %16
    %18:u32 = mul %10, %14
    %19:u32 = add %9, %18
    %20:u32 = add %19, %17
    %21:void = call %foo_inner, %numwgs_1, %20
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, GlobalInvocationIndex_AddMissingBuiltins) {
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
%foo_inner = func(%gindex:u32):void {
  $B1: {
    %3:u32 = add %gindex, 0u
    %x:u32 = let %3
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func(%num_workgroups:vec3<u32> [@num_workgroups], %global_invocation_id:vec3<u32> [@global_invocation_id]):void {
  $B2: {
    %8:u32 = access %global_invocation_id, 0u
    %9:u32 = access %global_invocation_id, 1u
    %10:u32 = access %global_invocation_id, 2u
    %11:u32 = access %num_workgroups, 0u
    %12:u32 = access %num_workgroups, 1u
    %13:u32 = mul %11, 3u
    %14:u32 = mul %12, 2u
    %15:u32 = mul %13, %14
    %16:u32 = mul %10, %15
    %17:u32 = mul %9, %13
    %18:u32 = add %8, %17
    %19:u32 = add %18, %16
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

}  // namespace
}  // namespace tint::msl::writer::raise
