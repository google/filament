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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Parameters_Mixed) {
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ReturnValue_DualSourceBlending) {
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Struct_SharedByVertexAndFragment) {
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

$B1: {  # root
  %vert_position_Output:ptr<__out, vec4<f32>, write> = var undef @builtin(position)
  %vert_loc0_Output:ptr<__out, vec4<f32>, write> = var undef @location(0)
  %frag_position_Input:ptr<__in, vec4<f32>, read> = var undef @builtin(position)
  %frag_loc0_Input:ptr<__in, vec4<f32>, read> = var undef @location(0)
  %frag_loc0_Output:ptr<__out, vec4<f32>, write> = var undef @location(0)
}

%vert_inner = func():Interface {
  $B2: {
    %7:vec4<f32> = construct 0.0f
    %8:vec4<f32> = construct 1.0f
    %9:Interface = construct %7, %8
    ret %9
  }
}
%frag_inner = func(%inputs:Interface):vec4<f32> {
  $B3: {
    %12:vec4<f32> = access %inputs, 0u
    %13:vec4<f32> = access %inputs, 1u
    %14:vec4<f32> = add %12, %13
    ret %14
  }
}
%vert = @vertex func():void {
  $B4: {
    %16:Interface = call %vert_inner
    %17:vec4<f32> = access %16, 0u
    store %vert_position_Output, %17
    %18:vec4<f32> = access %16, 1u
    store %vert_loc0_Output, %18
    ret
  }
}
%frag = @fragment func():void {
  $B5: {
    %20:vec4<f32> = load %frag_position_Input
    %21:vec4<f32> = load %frag_loc0_Input
    %22:Interface = construct %20, %21
    %23:vec4<f32> = call %frag_inner, %22
    store %frag_loc0_Output, %23
    ret
  }
}
)";

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, Struct_SharedWithBuffer) {
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
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
                                         /* location */ 0u,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ std::nullopt,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                                 {
                                     mod.symbols.New("mask"),
                                     ty.u32(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kSampleMask,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that interpolation attributes are stripped from vertex inputs and fragment outputs.
TEST_F(SpirvWriter_ShaderIOTest, InterpolationOnVertexInputOrFragmentOutput) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {
                                                       mod.symbols.New("color"),
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

    // Vertex shader.
    {
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
    }

    // Fragment shader with struct output.
    {
        auto* ep = b.Function("frag1", str_ty);
        ep->SetStage(core::ir::Function::PipelineStage::kFragment);

        b.Append(ep->Block(), [&] {  //
            b.Return(ep, b.Construct(str_ty, 0.5_f));
        });
    }

    // Fragment shader with non-struct output.
    {
        auto* ep = b.Function("frag2", ty.i32());
        ep->SetStage(core::ir::Function::PipelineStage::kFragment);
        ep->SetReturnLocation(0);
        ep->SetReturnInterpolation(core::Interpolation{core::InterpolationType::kFlat});

        b.Append(ep->Block(), [&] {  //
            b.Return(ep, b.Constant(42_i));
        });
    }

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
%frag1 = @fragment func():MyStruct {
  $B2: {
    %6:MyStruct = construct 0.5f
    ret %6
  }
}
%frag2 = @fragment func():i32 [@location(0), @interpolate(flat)] {
  $B3: {
    ret 42i
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
  %frag1_loc1_Output:ptr<__out, f32, write> = var undef @location(1)
  %frag2_loc0_Output:ptr<__out, i32, write> = var undef @location(0)
}

%vert_inner = func(%input:MyStruct, %ival:i32):vec4<f32> {
  $B2: {
    %9:vec4<f32> = construct 0.5f
    ret %9
  }
}
%frag1_inner = func():MyStruct {
  $B3: {
    %11:MyStruct = construct 0.5f
    ret %11
  }
}
%frag2_inner = func():i32 {
  $B4: {
    ret 42i
  }
}
%vert = @vertex func():void {
  $B5: {
    %14:f32 = load %vert_loc1_Input
    %15:MyStruct = construct %14
    %16:i32 = load %vert_loc1_Input_1
    %17:vec4<f32> = call %vert_inner, %15, %16
    store %vert_position_Output, %17
    ret
  }
}
%frag1 = @fragment func():void {
  $B6: {
    %19:MyStruct = call %frag1_inner
    %20:f32 = access %19, 0u
    store %frag1_loc1_Output, %20
    ret
  }
}
%frag2 = @fragment func():void {
  $B7: {
    %22:i32 = call %frag2_inner
    store %frag2_loc0_Output, %22
    ret
  }
}
)";

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ClampFragDepth) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("color"),
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
                                     mod.symbols.New("depth"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kFragDepth,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
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

tint_push_constant_struct = struct @align(4), @block {
  depth_min:f32 @offset(4)
  depth_max:f32 @offset(8)
}

$B1: {  # root
  %tint_push_constants:ptr<push_constant, tint_push_constant_struct, read> = var undef
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
    %10:ptr<push_constant, f32, read> = access %tint_push_constants, 0u
    %11:f32 = load %10
    %12:ptr<push_constant, f32, read> = access %tint_push_constants, 1u
    %13:f32 = load %12
    %14:f32 = clamp %9, %11, %13
    store %foo_frag_depth_Output, %14
    ret
  }
}
)";

    core::ir::transform::PreparePushConstantsConfig push_constants_config;
    push_constants_config.AddInternalConstant(4, mod.symbols.New("depth_min"), ty.f32());
    push_constants_config.AddInternalConstant(8, mod.symbols.New("depth_max"), ty.f32());
    auto push_constants = PreparePushConstants(mod, push_constants_config);
    EXPECT_EQ(push_constants, Success);

    ShaderIOConfig config{push_constants.Get()};
    config.depth_range_offsets = {4, 8};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, ClampFragDepth_MultipleFragmentShaders) {
    auto* str_ty = ty.Struct(mod.symbols.New("Outputs"),
                             {
                                 {
                                     mod.symbols.New("color"),
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
                                     mod.symbols.New("depth"),
                                     ty.f32(),
                                     core::IOAttributes{
                                         /* location */ std::nullopt,
                                         /* blend_src */ std::nullopt,
                                         /* color */ std::nullopt,
                                         /* builtin */ core::BuiltinValue::kFragDepth,
                                         /* interpolation */ std::nullopt,
                                         /* invariant */ false,
                                     },
                                 },
                             });

    auto make_entry_point = [&](std::string_view name) {
        auto* ep = b.Function(name, str_ty);
        ep->SetStage(core::ir::Function::PipelineStage::kFragment);
        b.Append(ep->Block(), [&] {  //
            b.Return(ep, b.Construct(str_ty, 0.5_f, 2_f));
        });
    };
    make_entry_point("ep1");
    make_entry_point("ep2");
    make_entry_point("ep3");

    auto* src = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0), @location(0)
  depth:f32 @offset(4), @builtin(frag_depth)
}

%ep1 = @fragment func():Outputs {
  $B1: {
    %2:Outputs = construct 0.5f, 2.0f
    ret %2
  }
}
%ep2 = @fragment func():Outputs {
  $B2: {
    %4:Outputs = construct 0.5f, 2.0f
    ret %4
  }
}
%ep3 = @fragment func():Outputs {
  $B3: {
    %6:Outputs = construct 0.5f, 2.0f
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Outputs = struct @align(4) {
  color:f32 @offset(0)
  depth:f32 @offset(4)
}

tint_push_constant_struct = struct @align(4), @block {
  depth_min:f32 @offset(4)
  depth_max:f32 @offset(8)
}

$B1: {  # root
  %tint_push_constants:ptr<push_constant, tint_push_constant_struct, read> = var undef
  %ep1_loc0_Output:ptr<__out, f32, write> = var undef @location(0)
  %ep1_frag_depth_Output:ptr<__out, f32, write> = var undef @builtin(frag_depth)
  %ep2_loc0_Output:ptr<__out, f32, write> = var undef @location(0)
  %ep2_frag_depth_Output:ptr<__out, f32, write> = var undef @builtin(frag_depth)
  %ep3_loc0_Output:ptr<__out, f32, write> = var undef @location(0)
  %ep3_frag_depth_Output:ptr<__out, f32, write> = var undef @builtin(frag_depth)
}

%ep1_inner = func():Outputs {
  $B2: {
    %9:Outputs = construct 0.5f, 2.0f
    ret %9
  }
}
%ep2_inner = func():Outputs {
  $B3: {
    %11:Outputs = construct 0.5f, 2.0f
    ret %11
  }
}
%ep3_inner = func():Outputs {
  $B4: {
    %13:Outputs = construct 0.5f, 2.0f
    ret %13
  }
}
%ep1 = @fragment func():void {
  $B5: {
    %15:Outputs = call %ep1_inner
    %16:f32 = access %15, 0u
    store %ep1_loc0_Output, %16
    %17:f32 = access %15, 1u
    %18:ptr<push_constant, f32, read> = access %tint_push_constants, 0u
    %19:f32 = load %18
    %20:ptr<push_constant, f32, read> = access %tint_push_constants, 1u
    %21:f32 = load %20
    %22:f32 = clamp %17, %19, %21
    store %ep1_frag_depth_Output, %22
    ret
  }
}
%ep2 = @fragment func():void {
  $B6: {
    %24:Outputs = call %ep2_inner
    %25:f32 = access %24, 0u
    store %ep2_loc0_Output, %25
    %26:f32 = access %24, 1u
    %27:ptr<push_constant, f32, read> = access %tint_push_constants, 0u
    %28:f32 = load %27
    %29:ptr<push_constant, f32, read> = access %tint_push_constants, 1u
    %30:f32 = load %29
    %31:f32 = clamp %26, %28, %30
    store %ep2_frag_depth_Output, %31
    ret
  }
}
%ep3 = @fragment func():void {
  $B7: {
    %33:Outputs = call %ep3_inner
    %34:f32 = access %33, 0u
    store %ep3_loc0_Output, %34
    %35:f32 = access %33, 1u
    %36:ptr<push_constant, f32, read> = access %tint_push_constants, 0u
    %37:f32 = load %36
    %38:ptr<push_constant, f32, read> = access %tint_push_constants, 1u
    %39:f32 = load %38
    %40:f32 = clamp %35, %37, %39
    store %ep3_frag_depth_Output, %40
    ret
  }
}
)";

    core::ir::transform::PreparePushConstantsConfig push_constants_config;
    push_constants_config.AddInternalConstant(4, mod.symbols.New("depth_min"), ty.f32());
    push_constants_config.AddInternalConstant(8, mod.symbols.New("depth_max"), ty.f32());
    auto push_constants = PreparePushConstants(mod, push_constants_config);
    EXPECT_EQ(push_constants, Success);

    ShaderIOConfig config{push_constants.Get()};
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
    config.emit_vertex_point_size = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, F16_IO_WithoutPolyfill) {
    auto* outputs =
        ty.Struct(mod.symbols.New("Outputs"), {
                                                  {
                                                      mod.symbols.New("out1"),
                                                      ty.f16(),
                                                      core::IOAttributes{
                                                          /* location */ 1u,
                                                          /* blend_src */ std::nullopt,
                                                          /* color */ std::nullopt,
                                                          /* builtin */ std::nullopt,
                                                          /* interpolation */ std::nullopt,
                                                          /* invariant */ false,
                                                      },
                                                  },
                                                  {
                                                      mod.symbols.New("out2"),
                                                      ty.vec4<f16>(),
                                                      core::IOAttributes{
                                                          /* location */ 2u,
                                                          /* blend_src */ std::nullopt,
                                                          /* color */ std::nullopt,
                                                          /* builtin */ std::nullopt,
                                                          /* interpolation */ std::nullopt,
                                                          /* invariant */ false,
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
    config.polyfill_f16_io = false;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ShaderIOTest, F16_IO_WithPolyfill) {
    auto* outputs =
        ty.Struct(mod.symbols.New("Outputs"), {
                                                  {
                                                      mod.symbols.New("out1"),
                                                      ty.f16(),
                                                      core::IOAttributes{
                                                          /* location */ 1u,
                                                          /* blend_src */ std::nullopt,
                                                          /* color */ std::nullopt,
                                                          /* builtin */ std::nullopt,
                                                          /* interpolation */ std::nullopt,
                                                          /* invariant */ false,
                                                      },
                                                  },
                                                  {
                                                      mod.symbols.New("out2"),
                                                      ty.vec4<f16>(),
                                                      core::IOAttributes{
                                                          /* location */ 2u,
                                                          /* blend_src */ std::nullopt,
                                                          /* color */ std::nullopt,
                                                          /* builtin */ std::nullopt,
                                                          /* interpolation */ std::nullopt,
                                                          /* invariant */ false,
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

    core::ir::transform::PushConstantLayout push_constants;
    ShaderIOConfig config{push_constants};
    config.polyfill_f16_io = true;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
