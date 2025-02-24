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

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/msl/writer/raise/shader_io.h"

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

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Parameters_NonStruct) {
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

    ShaderIOConfig config;
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
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kFrontFacing,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ true,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color2"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 1u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */
                                         core::Interpolation{
                                             core::InterpolationType::kLinear,
                                             core::InterpolationSampling::kSample,
                                         },
                                         /* invariant */ false,
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

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Parameters_Mixed) {
    auto* str_ty = ty.Struct(mod.symbols.New("Inputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ true,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
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

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, ReturnValue_NonStructBuiltin) {
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var
    %6:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %6, %4
    %7:foo_outputs = load %tint_wrapper_result
    ret %7
  }
}
)";

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, ReturnValue_NonStructLocation) {
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var
    %6:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %6, %4
    %7:foo_outputs = load %tint_wrapper_result
    ret %7
  }
}
)";

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, ReturnValue_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     ty.vec4<f32>(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ true,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color1"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color2"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ 1u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */
                                         core::Interpolation{
                                             core::InterpolationType::kLinear,
                                             core::InterpolationSampling::kSample,
                                         },
                                         /* invariant */ false,
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var
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

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, ReturnValue_DualSourceBlending) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Output"), {
                                                 {
                                                     mod.symbols.New("color1"),
                                                     ty.f32(),
                                                     core::IOAttributes{
                                                         /* location */ 0u,
                                                         /* blend_src */ 0u,
                                                         /* color */ std::nullopt,
                                                         /* builtin */ std::nullopt,
                                                         /* interpolation */ std::nullopt,
                                                         /* invariant */ false,
                                                     },
                                                 },
                                                 {
                                                     mod.symbols.New("color2"),
                                                     ty.f32(),
                                                     core::IOAttributes{
                                                         /* location */ 0u,
                                                         /* blend_src */ 1u,
                                                         /* color */ std::nullopt,
                                                         /* builtin */ std::nullopt,
                                                         /* interpolation */ std::nullopt,
                                                         /* invariant */ false,
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var
    %8:ptr<function, f32, read_write> = access %tint_wrapper_result, 0u
    store %8, %5
    %9:ptr<function, f32, read_write> = access %tint_wrapper_result, 1u
    store %9, %6
    %10:foo_outputs = load %tint_wrapper_result
    ret %10
  }
}
)";

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Struct_SharedByVertexAndFragment) {
    auto* vec4f = ty.vec4<f32>();
    auto* str_ty = ty.Struct(mod.symbols.New("Interface"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     vec4f,
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color"),
                                     vec4f,
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                             });

    // Vertex shader.
    {
        auto* ep = b.Function("vert", str_ty);
        ep->SetStage(core::ir::Function::PipelineStage::kVertex);

        b.Append(ep->Block(), [&] {  //
            auto* position = b.Construct(vec4f, 0_f);
            auto* color = b.Construct(vec4f, 1_f);
            b.Return(ep, b.Construct(str_ty, position, color));
        });
    }

    // Fragment shader.
    {
        auto* ep = b.Function("frag", vec4f);
        auto* inputs = b.FunctionParam("inputs", str_ty);
        ep->SetStage(core::ir::Function::PipelineStage::kFragment);
        ep->SetParams({inputs});
        ep->SetReturnLocation(0u);

        b.Append(ep->Block(), [&] {  //
            auto* position = b.Access(vec4f, inputs, 0_u);
            auto* color = b.Access(vec4f, inputs, 1_u);
            b.Return(ep, b.Add(vec4f, position, color));
        });
    }

    auto* src = R"(
Interface = struct @align(16) {
  position:vec4<f32> @offset(0), @builtin(position)
  color:vec4<f32> @offset(16), @location(0)
}

%vert = @vertex func():Interface {
  $B1: {
    %2:vec4<f32> = construct 0.0f
    %3:vec4<f32> = construct 1.0f
    %4:Interface = construct %2, %3
    ret %4
  }
}
%frag = @fragment func(%inputs:Interface):vec4<f32> [@location(0)] {
  $B2: {
    %7:vec4<f32> = access %inputs, 0u
    %8:vec4<f32> = access %inputs, 1u
    %9:vec4<f32> = add %7, %8
    ret %9
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Interface = struct @align(16) {
  position:vec4<f32> @offset(0)
  color:vec4<f32> @offset(16)
}

vert_outputs = struct @align(16) {
  Interface_position:vec4<f32> @offset(0), @builtin(position)
  Interface_color:vec4<f32> @offset(16), @location(0)
}

frag_inputs = struct @align(16) {
  Interface_color:vec4<f32> @offset(0), @location(0)
}

frag_outputs = struct @align(16) {
  tint_symbol:vec4<f32> @offset(0), @location(0)
}

%vert_inner = func():Interface {
  $B1: {
    %2:vec4<f32> = construct 0.0f
    %3:vec4<f32> = construct 1.0f
    %4:Interface = construct %2, %3
    ret %4
  }
}
%frag_inner = func(%inputs:Interface):vec4<f32> {
  $B2: {
    %7:vec4<f32> = access %inputs, 0u
    %8:vec4<f32> = access %inputs, 1u
    %9:vec4<f32> = add %7, %8
    ret %9
  }
}
%vert = @vertex func():vert_outputs {
  $B3: {
    %11:Interface = call %vert_inner
    %12:vec4<f32> = access %11, 0u
    %13:vec4<f32> = access %11, 1u
    %tint_wrapper_result:ptr<function, vert_outputs, read_write> = var
    %15:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %15, %12
    %16:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 1u
    store %16, %13
    %17:vert_outputs = load %tint_wrapper_result
    ret %17
  }
}
%frag = @fragment func(%Interface_position:vec4<f32> [@position], %inputs_1:frag_inputs):frag_outputs {  # %inputs_1: 'inputs'
  $B4: {
    %21:vec4<f32> = access %inputs_1, 0u
    %22:Interface = construct %Interface_position, %21
    %23:vec4<f32> = call %frag_inner, %22
    %tint_wrapper_result_1:ptr<function, frag_outputs, read_write> = var  # %tint_wrapper_result_1: 'tint_wrapper_result'
    %25:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result_1, 0u
    store %25, %23
    %26:frag_outputs = load %tint_wrapper_result_1
    ret %26
  }
}
)";

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Struct_SharedWithBuffer) {
    auto* vec4f = ty.vec4<f32>();
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     vec4f,
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color"),
                                     vec4f,
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
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
  %1:ptr<storage, Outputs, read> = var @binding_point(0, 0)
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
  %1:ptr<storage, Outputs, read> = var @binding_point(0, 0)
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
    %tint_wrapper_result:ptr<function, vert_outputs, read_write> = var
    %9:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %9, %6
    %10:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 1u
    store %10, %7
    %11:vert_outputs = load %tint_wrapper_result
    ret %11
  }
}
)";

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that IO attributes are stripped from structures that are not used for the shader interface.
TEST_F(MslWriter_ShaderIOTest, StructWithAttributes_NotUsedForInterface) {
    auto* vec4f = ty.vec4<f32>();
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("position"),
                                     vec4f,
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kPosition,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("color"),
                                     vec4f,
                                     core::IOAttributes{
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                             });

    auto* var = b.Var(ty.ptr(storage, str_ty, core::Access::kWrite));
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
  %1:ptr<storage, Outputs, write> = var @binding_point(0, 0)
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
  %1:ptr<storage, Outputs, write> = var @binding_point(0, 0)
}

%frag = @fragment func():void {
  $B2: {
    %3:Outputs = construct
    store %1, %3
    ret
  }
}
)";

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, EmitVertexPointSize) {
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var
    %6:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %6, %4
    %7:ptr<function, f32, read_write> = access %tint_wrapper_result, 1u
    store %7, 1.0f
    %8:foo_outputs = load %tint_wrapper_result
    ret %8
  }
}
)";

    ShaderIOConfig config;
    config.emit_vertex_point_size = true;
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
        b.Add<f32>(color1, color2);
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

    ShaderIOConfig config;
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
                                                  {mod.symbols.New("color"), ty.vec4<f32>(), loc},
                                                  {mod.symbols.New("mask"), ty.u32(), mask},
                                              });

    auto* ep = b.Function("foo", outputs);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep,
                 b.Construct(outputs, b.Splat(ty.vec4<f32>(), 0.5_f), b.Constant(u32(0x10203040))));
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var
    %8:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %8, %5
    %9:ptr<function, u32, read_write> = access %tint_wrapper_result, 1u
    store %9, %6
    %10:foo_outputs = load %tint_wrapper_result
    ret %10
  }
}
)";

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, FixedSampleMask) {
    auto* ep = b.Function("foo", ty.vec4<f32>());
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0u);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Splat(ty.vec4<f32>(), 0.5_f));
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var
    %5:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %5, %3
    %6:ptr<function, u32, read_write> = access %tint_wrapper_result, 1u
    store %6, 12345678u
    %7:foo_outputs = load %tint_wrapper_result
    ret %7
  }
}
)";

    ShaderIOConfig config;
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
                                                  {mod.symbols.New("color"), ty.vec4<f32>(), loc},
                                                  {mod.symbols.New("mask"), ty.u32(), mask},
                                              });

    auto* ep = b.Function("foo", outputs);
    ep->SetStage(core::ir::Function::PipelineStage::kFragment);

    b.Append(ep->Block(), [&] {  //
        b.Return(ep,
                 b.Construct(outputs, b.Splat(ty.vec4<f32>(), 0.5_f), b.Constant(u32(0x10203040))));
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
    %tint_wrapper_result:ptr<function, foo_outputs, read_write> = var
    %9:ptr<function, vec4<f32>, read_write> = access %tint_wrapper_result, 0u
    store %9, %5
    %10:ptr<function, u32, read_write> = access %tint_wrapper_result, 1u
    store %10, %7
    %11:foo_outputs = load %tint_wrapper_result
    ret %11
  }
}
)";

    ShaderIOConfig config;
    config.fixed_sample_mask = 12345678u;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ShaderIOTest, Color_Struct) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("Inputs"), {
                                                 {
                                                     mod.symbols.New("color1"),
                                                     ty.f32(),
                                                     core::IOAttributes{
                                                         /* location */ std::nullopt,
                                                         /* blend_src */ std::nullopt,
                                                         /* color */ 1u,
                                                         /* builtin */ std::nullopt,
                                                         /* interpolation */ std::nullopt,
                                                         /* invariant */ false,
                                                     },
                                                 },
                                                 {
                                                     mod.symbols.New("color2"),
                                                     ty.f32(),
                                                     core::IOAttributes{
                                                         /* location */ std::nullopt,
                                                         /* blend_src */ std::nullopt,
                                                         /* color */ 2u,
                                                         /* builtin */ std::nullopt,
                                                         /* interpolation */ std::nullopt,
                                                         /* invariant */ false,
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
        b.Add<f32>(color1, color2);
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

    ShaderIOConfig config;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
