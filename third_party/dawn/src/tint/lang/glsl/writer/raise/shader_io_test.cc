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

#include "src/tint/lang/glsl/writer/raise/shader_io.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::glsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class GlslWriter_ShaderIOTest : public core::ir::transform::TransformTest {
  public:
    GlslWriter_ShaderIOTest() {
        capabilities.Add(core::ir::Capability::kLoosenValidationForShaderIO);
    }
};

TEST_F(GlslWriter_ShaderIOTest, NoInputsOrOutputs) {
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

TEST_F(GlslWriter_ShaderIOTest, Parameters_NonStruct) {
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
$B1: {  # root
  %foo_front_facing:ptr<__in, bool, read> = var undef @builtin(front_facing)
  %foo_position:ptr<__in, vec4<f32>, read> = var undef @invariant @builtin(position)
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
    %13:bool = load %foo_front_facing
    %14:vec4<f32> = load %foo_position
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

TEST_F(GlslWriter_ShaderIOTest, Parameters_Struct) {
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

$B1: {  # root
  %foo_front_facing:ptr<__in, bool, read> = var undef @builtin(front_facing)
  %foo_position:ptr<__in, vec4<f32>, read> = var undef @invariant @builtin(position)
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
    %14:bool = load %foo_front_facing
    %15:vec4<f32> = load %foo_position
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

TEST_F(GlslWriter_ShaderIOTest, Parameters_Mixed) {
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

$B1: {  # root
  %foo_front_facing:ptr<__in, bool, read> = var undef @builtin(front_facing)
  %foo_position:ptr<__in, vec4<f32>, read> = var undef @invariant @builtin(position)
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
    %14:bool = load %foo_front_facing
    %15:vec4<f32> = load %foo_position
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

TEST_F(GlslWriter_ShaderIOTest, ReturnValue_NonStructBuiltin) {
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
$B1: {  # root
  %foo_position:ptr<__out, vec4<f32>, write> = var undef @invariant @builtin(position)
  %foo___point_size:ptr<__out, f32, write> = var undef @builtin(__point_size)
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
    %7:f32 = swizzle %6, x
    %8:f32 = swizzle %6, y
    %9:f32 = negation %8
    %10:f32 = swizzle %6, z
    %11:f32 = swizzle %6, w
    %12:f32 = mul 2.0f, %10
    %13:f32 = sub %12, %11
    %14:vec4<f32> = construct %7, %9, %13, %11
    store %foo_position, %14
    store %foo___point_size, 1.0f
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_ShaderIOTest, ReturnValue_NonStructLocation) {
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

TEST_F(GlslWriter_ShaderIOTest, ReturnValue_Struct) {
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

$B1: {  # root
  %foo_position:ptr<__out, vec4<f32>, write> = var undef @invariant @builtin(position)
  %foo_loc0_Output:ptr<__out, f32, write> = var undef @location(0)
  %foo_loc1_Output:ptr<__out, f32, write> = var undef @location(1) @interpolate(linear, sample)
  %foo___point_size:ptr<__out, f32, write> = var undef @builtin(__point_size)
}

%foo_inner = func():Outputs {
  $B2: {
    %6:vec4<f32> = construct 0.0f
    %7:Outputs = construct %6, 0.25f, 0.75f
    ret %7
  }
}
%foo = @vertex func():void {
  $B3: {
    %9:Outputs = call %foo_inner
    %10:vec4<f32> = access %9, 0u
    %11:f32 = swizzle %10, x
    %12:f32 = swizzle %10, y
    %13:f32 = negation %12
    %14:f32 = swizzle %10, z
    %15:f32 = swizzle %10, w
    %16:f32 = mul 2.0f, %14
    %17:f32 = sub %16, %15
    %18:vec4<f32> = construct %11, %13, %17, %15
    store %foo_position, %18
    %19:f32 = access %9, 1u
    store %foo_loc0_Output, %19
    %20:f32 = access %9, 2u
    store %foo_loc1_Output, %20
    store %foo___point_size, 1.0f
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_ShaderIOTest, ReturnValue_DualSourceBlending) {
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

TEST_F(GlslWriter_ShaderIOTest, Struct_SharedWithBuffer) {
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

$B1: {  # root
  %1:ptr<storage, Outputs, read> = var undef @binding_point(0, 0)
  %vert_position:ptr<__out, vec4<f32>, write> = var undef @builtin(position)
  %vert_loc0_Output:ptr<__out, vec4<f32>, write> = var undef @location(0)
  %vert___point_size:ptr<__out, f32, write> = var undef @builtin(__point_size)
}

%vert_inner = func():Outputs {
  $B2: {
    %6:Outputs = load %1
    ret %6
  }
}
%vert = @vertex func():void {
  $B3: {
    %8:Outputs = call %vert_inner
    %9:vec4<f32> = access %8, 0u
    %10:f32 = swizzle %9, x
    %11:f32 = swizzle %9, y
    %12:f32 = negation %11
    %13:f32 = swizzle %9, z
    %14:f32 = swizzle %9, w
    %15:f32 = mul 2.0f, %13
    %16:f32 = sub %15, %14
    %17:vec4<f32> = construct %10, %12, %16, %14
    store %vert_position, %17
    %18:vec4<f32> = access %8, 1u
    store %vert_loc0_Output, %18
    store %vert___point_size, 1.0f
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that we change the type of the sample mask builtin to an array for GLSL
TEST_F(GlslWriter_ShaderIOTest, SampleMask) {
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
  %foo_sample_mask:ptr<__in, array<i32, 1>, read> = var undef @builtin(sample_mask)
  %foo_loc0_Output:ptr<__out, f32, write> = var undef @location(0)
  %foo_sample_mask_1:ptr<__out, array<i32, 1>, write> = var undef @builtin(sample_mask)  # %foo_sample_mask_1: 'foo_sample_mask'
}

%foo_inner = func(%mask_in:u32):Outputs {
  $B2: {
    %6:Outputs = construct 0.5f, %mask_in
    ret %6
  }
}
%foo = @fragment func():void {
  $B3: {
    %8:array<i32, 1> = load %foo_sample_mask
    %9:i32 = access %8, 0u
    %10:u32 = convert %9
    %11:Outputs = call %foo_inner, %10
    %12:f32 = access %11, 0u
    store %foo_loc0_Output, %12
    %13:u32 = access %11, 1u
    %14:ptr<__out, i32, write> = access %foo_sample_mask_1, 0u
    %15:i32 = convert %13
    store %14, %15
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
TEST_F(GlslWriter_ShaderIOTest, InterpolationOnVertexInput) {
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

    auto* ep = b.Function("vert", ty.vec4f());
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    ep->SetReturnInvariant(true);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    auto* str_param = b.FunctionParam("input", str_ty);
    auto* ival = b.FunctionParam("ival", ty.i32());
    ival->SetLocation(2);
    ival->SetInterpolation(core::Interpolation{core::InterpolationType::kFlat});
    ep->SetParams({str_param, ival});

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  color:f32 @offset(0), @location(1), @interpolate(linear, sample)
}

%vert = @vertex func(%input:MyStruct, %ival:i32 [@location(2), @interpolate(flat)]):vec4<f32> [@invariant, @position] {
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
  %vert_loc2_Input:ptr<__in, i32, read> = var undef @location(2)
  %vert_position:ptr<__out, vec4<f32>, write> = var undef @invariant @builtin(position)
  %vert___point_size:ptr<__out, f32, write> = var undef @builtin(__point_size)
}

%vert_inner = func(%input:MyStruct, %ival:i32):vec4<f32> {
  $B2: {
    %8:vec4<f32> = construct 0.5f
    ret %8
  }
}
%vert = @vertex func():void {
  $B3: {
    %10:f32 = load %vert_loc1_Input
    %11:MyStruct = construct %10
    %12:i32 = load %vert_loc2_Input
    %13:vec4<f32> = call %vert_inner, %11, %12
    %14:f32 = swizzle %13, x
    %15:f32 = swizzle %13, y
    %16:f32 = negation %15
    %17:f32 = swizzle %13, z
    %18:f32 = swizzle %13, w
    %19:f32 = mul 2.0f, %17
    %20:f32 = sub %19, %18
    %21:vec4<f32> = construct %14, %16, %20, %18
    store %vert_position, %21
    store %vert___point_size, 1.0f
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

// Test that interpolation attributes are stripped from fragment struct outputs
TEST_F(GlslWriter_ShaderIOTest, InterpolationOnFragmentOutput_Struct) {
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

// Test that interpolation attributes are stripped from fragment struct outputs
TEST_F(GlslWriter_ShaderIOTest, InterpolationOnFragmentOutput_NonStruct) {
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

TEST_F(GlslWriter_ShaderIOTest, ClampFragDepth) {
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
  %foo_frag_depth:ptr<__out, f32, write> = var undef @builtin(frag_depth)
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
    store %foo_frag_depth, %14
    ret
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

TEST_F(GlslWriter_ShaderIOTest, BGRASwizzleSingleValue) {
    auto* ep = b.Function("vert", ty.vec4f());
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    ep->SetReturnInvariant(true);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    auto* val = b.FunctionParam("val", ty.vec4f());
    val->SetLocation(0);
    ep->SetParams({val});

    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%vert = @vertex func(%val:vec4<f32> [@location(0)]):vec4<f32> [@invariant, @position] {
  $B1: {
    %3:vec4<f32> = construct 0.5f
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %vert_loc0_Input:ptr<__in, vec4<f32>, read> = var undef @location(0)
  %vert_position:ptr<__out, vec4<f32>, write> = var undef @invariant @builtin(position)
  %vert___point_size:ptr<__out, f32, write> = var undef @builtin(__point_size)
}

%vert_inner = func(%val:vec4<f32>):vec4<f32> {
  $B2: {
    %6:vec4<f32> = construct 0.5f
    ret %6
  }
}
%vert = @vertex func():void {
  $B3: {
    %8:vec4<f32> = load %vert_loc0_Input
    %9:vec4<f32> = swizzle %8, zyxw
    %10:vec4<f32> = call %vert_inner, %9
    %11:f32 = swizzle %10, x
    %12:f32 = swizzle %10, y
    %13:f32 = negation %12
    %14:f32 = swizzle %10, z
    %15:f32 = swizzle %10, w
    %16:f32 = mul 2.0f, %14
    %17:f32 = sub %16, %15
    %18:vec4<f32> = construct %11, %13, %17, %15
    store %vert_position, %18
    store %vert___point_size, 1.0f
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.bgra_swizzle_locations.insert(0u);
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_ShaderIOTest, BGRASwizzleMultipleValueMixedTypes) {
    auto* ep = b.Function("vert", ty.vec4f());
    ep->SetReturnBuiltin(core::BuiltinValue::kPosition);
    ep->SetReturnInvariant(true);
    ep->SetStage(core::ir::Function::PipelineStage::kVertex);

    std::unordered_set<uint32_t> swizzled_locations;

    // Checks swizzling happens before conversion to the original type.
    auto* val1 = b.FunctionParam("val1", ty.f32());
    val1->SetLocation(5);
    swizzled_locations.insert(5);

    auto* val2 = b.FunctionParam("val2", ty.vec2f());
    val2->SetLocation(0);
    swizzled_locations.insert(0);

    auto* val3 = b.FunctionParam("val3", ty.vec3f());
    val3->SetLocation(3);
    swizzled_locations.insert(3);

    auto* val4 = b.FunctionParam("val4", ty.vec4f());
    val4->SetLocation(7);
    swizzled_locations.insert(7);

    // Checks that the sentinel doesn't get swizzled.
    auto* sentinel = b.FunctionParam("sentinel", ty.vec4f());
    sentinel->SetLocation(4);

    ep->SetParams({val1, val2, sentinel, val3, val4});
    b.Append(ep->Block(), [&] {  //
        b.Return(ep, b.Construct(ty.vec4f(), 0.5_f));
    });

    auto* src = R"(
%vert = @vertex func(%val1:f32 [@location(5)], %val2:vec2<f32> [@location(0)], %sentinel:vec4<f32> [@location(4)], %val3:vec3<f32> [@location(3)], %val4:vec4<f32> [@location(7)]):vec4<f32> [@invariant, @position] {
  $B1: {
    %7:vec4<f32> = construct 0.5f
    ret %7
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %vert_loc5_Input:ptr<__in, vec4<f32>, read> = var undef @location(5)
  %vert_loc0_Input:ptr<__in, vec4<f32>, read> = var undef @location(0)
  %vert_loc4_Input:ptr<__in, vec4<f32>, read> = var undef @location(4)
  %vert_loc3_Input:ptr<__in, vec4<f32>, read> = var undef @location(3)
  %vert_loc7_Input:ptr<__in, vec4<f32>, read> = var undef @location(7)
  %vert_position:ptr<__out, vec4<f32>, write> = var undef @invariant @builtin(position)
  %vert___point_size:ptr<__out, f32, write> = var undef @builtin(__point_size)
}

%vert_inner = func(%val1:f32, %val2:vec2<f32>, %sentinel:vec4<f32>, %val3:vec3<f32>, %val4:vec4<f32>):vec4<f32> {
  $B2: {
    %14:vec4<f32> = construct 0.5f
    ret %14
  }
}
%vert = @vertex func():void {
  $B3: {
    %16:vec4<f32> = load %vert_loc5_Input
    %17:f32 = swizzle %16, z
    %18:vec4<f32> = load %vert_loc0_Input
    %19:vec2<f32> = swizzle %18, zy
    %20:vec4<f32> = load %vert_loc4_Input
    %21:vec4<f32> = load %vert_loc3_Input
    %22:vec3<f32> = swizzle %21, zyx
    %23:vec4<f32> = load %vert_loc7_Input
    %24:vec4<f32> = swizzle %23, zyxw
    %25:vec4<f32> = call %vert_inner, %17, %19, %20, %22, %24
    %26:f32 = swizzle %25, x
    %27:f32 = swizzle %25, y
    %28:f32 = negation %27
    %29:f32 = swizzle %25, z
    %30:f32 = swizzle %25, w
    %31:f32 = mul 2.0f, %29
    %32:f32 = sub %31, %30
    %33:vec4<f32> = construct %26, %28, %32, %30
    store %vert_position, %33
    store %vert___point_size, 1.0f
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    config.bgra_swizzle_locations = swizzled_locations;
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_ShaderIOTest, WorkgroupIndex_ReuseExistingBuiltins) {
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
$B1: {  # root
  %foo_workgroup_id:ptr<__in, vec3<u32>, read> = var undef @builtin(workgroup_id)
  %foo_num_workgroups:ptr<__in, vec3<u32>, read> = var undef @builtin(num_workgroups)
}

%foo_inner = func(%wgid:vec3<u32>, %numwgs:vec3<u32>, %wgindex:u32):void {
  $B2: {
    %7:u32 = add %wgindex, 0u
    %x:u32 = let %7
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func():void {
  $B3: {
    %10:vec3<u32> = load %foo_workgroup_id
    %11:vec3<u32> = load %foo_num_workgroups
    %12:vec3<u32> = load %foo_workgroup_id
    %13:vec3<u32> = load %foo_num_workgroups
    %14:u32 = access %13, 0u
    %15:u32 = access %13, 1u
    %16:u32 = mul %14, %15
    %17:u32 = access %12, 2u
    %18:u32 = mul %17, %16
    %19:u32 = access %12, 1u
    %20:u32 = mul %19, %14
    %21:u32 = access %12, 0u
    %22:u32 = add %21, %20
    %23:u32 = add %22, %18
    %24:void = call %foo_inner, %10, %11, %23
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_ShaderIOTest, WorkgroupIndex_AddMissingBuiltins) {
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
$B1: {  # root
  %foo_workgroup_id:ptr<__in, vec3<u32>, read> = var undef @builtin(workgroup_id)
  %foo_num_workgroups:ptr<__in, vec3<u32>, read> = var undef @builtin(num_workgroups)
}

%foo_inner = func(%wgindex:u32):void {
  $B2: {
    %5:u32 = add %wgindex, 0u
    %x:u32 = let %5
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func():void {
  $B3: {
    %8:vec3<u32> = load %foo_workgroup_id
    %9:vec3<u32> = load %foo_num_workgroups
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

TEST_F(GlslWriter_ShaderIOTest, GlobalInvocationIndex_ReuseExistingBuiltins) {
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
$B1: {  # root
  %foo_num_workgroups:ptr<__in, vec3<u32>, read> = var undef @builtin(num_workgroups)
  %foo_global_invocation_id:ptr<__in, vec3<u32>, read> = var undef @builtin(global_invocation_id)
}

%foo_inner = func(%numwgs:vec3<u32>, %gindex:u32):void {
  $B2: {
    %6:u32 = add %gindex, 0u
    %x:u32 = let %6
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func():void {
  $B3: {
    %9:vec3<u32> = load %foo_num_workgroups
    %10:vec3<u32> = load %foo_num_workgroups
    %11:vec3<u32> = load %foo_global_invocation_id
    %12:u32 = access %11, 0u
    %13:u32 = access %11, 1u
    %14:u32 = access %11, 2u
    %15:u32 = access %10, 0u
    %16:u32 = access %10, 1u
    %17:u32 = mul %15, 3u
    %18:u32 = mul %16, 2u
    %19:u32 = mul %17, %18
    %20:u32 = mul %14, %19
    %21:u32 = mul %13, %17
    %22:u32 = add %12, %21
    %23:u32 = add %22, %20
    %24:void = call %foo_inner, %9, %23
    ret
  }
}
)";

    core::ir::transform::ImmediateDataLayout immediate_data;
    ShaderIOConfig config{immediate_data};
    Run(ShaderIO, config);

    EXPECT_EQ(expect, str());
}

TEST_F(GlslWriter_ShaderIOTest, GlobalInvocationIndex_AddMissingBuiltins) {
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
$B1: {  # root
  %foo_num_workgroups:ptr<__in, vec3<u32>, read> = var undef @builtin(num_workgroups)
  %foo_global_invocation_id:ptr<__in, vec3<u32>, read> = var undef @builtin(global_invocation_id)
}

%foo_inner = func(%gindex:u32):void {
  $B2: {
    %5:u32 = add %gindex, 0u
    %x:u32 = let %5
    ret
  }
}
%foo = @compute @workgroup_size(3u, 2u, 1u) func():void {
  $B3: {
    %8:vec3<u32> = load %foo_num_workgroups
    %9:vec3<u32> = load %foo_global_invocation_id
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
}  // namespace tint::glsl::writer::raise
