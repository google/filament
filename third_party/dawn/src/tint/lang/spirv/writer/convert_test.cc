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

#include "src/tint/lang/spirv/writer/common/helper_test.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// A parameterized test case.
struct ConvertCase {
    /// The input type.
    TestElementType in;
    /// The output type.
    TestElementType out;
    /// The expected SPIR-V instruction.
    std::string spirv_inst;
    /// The expected SPIR-V result type name.
    std::string spirv_type_name;
};
std::string PrintCase(testing::TestParamInfo<ConvertCase> cc) {
    StringStream ss;
    ss << cc.param.in << "_to_" << cc.param.out;
    return ss.str();
}

using Convert = SpirvWriterTestWithParam<ConvertCase>;
TEST_P(Convert, Scalar) {
    auto& params = GetParam();
    auto* func = b.Function("foo", MakeScalarType(params.out));
    func->SetParams({b.FunctionParam("arg", MakeScalarType(params.in))});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(MakeScalarType(params.out), func->Params()[0]);
        mod.SetName(result, "result");
        b.Return(func, result);
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(MakeScalarType(params.in))));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %" + params.spirv_type_name + " %arg");
}
TEST_P(Convert, Vector) {
    auto& params = GetParam();
    auto* func = b.Function("foo", MakeVectorType(params.out));
    func->SetParams({b.FunctionParam("arg", MakeVectorType(params.in))});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(MakeVectorType(params.out), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(MakeVectorType(params.in))));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = " + params.spirv_inst + " %v2" + params.spirv_type_name + " %arg");
}
INSTANTIATE_TEST_SUITE_P(SpirvWriterTest,
                         Convert,
                         testing::Values(
                             // To f32.
                             ConvertCase{kF16, kF32, "OpFConvert", "float"},
                             ConvertCase{kI32, kF32, "OpConvertSToF", "float"},
                             ConvertCase{kU32, kF32, "OpConvertUToF", "float"},
                             ConvertCase{kBool, kF32, "OpSelect", "float"},

                             // To f16.
                             ConvertCase{kF32, kF16, "OpFConvert", "half"},
                             ConvertCase{kI32, kF16, "OpConvertSToF", "half"},
                             ConvertCase{kU32, kF16, "OpConvertUToF", "half"},
                             ConvertCase{kBool, kF16, "OpSelect", "half"},

                             // To i32.
                             // Note: ftoi cases are polyfilled and tested separately.
                             ConvertCase{kU32, kI32, "OpBitcast", "int"},
                             ConvertCase{kBool, kI32, "OpSelect", "int"},

                             // To u32.
                             // Note: ftoi cases are polyfilled and tested separately.
                             ConvertCase{kI32, kU32, "OpBitcast", "uint"},
                             ConvertCase{kBool, kU32, "OpSelect", "uint"},

                             // To bool.
                             ConvertCase{kF32, kBool, "OpFUnordNotEqual", "bool"},
                             ConvertCase{kF16, kBool, "OpFUnordNotEqual", "bool"},
                             ConvertCase{kI32, kBool, "OpINotEqual", "bool"},
                             ConvertCase{kU32, kBool, "OpINotEqual", "bool"}),
                         PrintCase);

TEST_F(SpirvWriterTest, Convert_Mat2x3_F16_to_F32) {
    auto* func = b.Function("foo", ty.mat2x3<f32>());
    func->SetParams({b.FunctionParam("arg", ty.mat2x3<f16>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.mat2x3<f32>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.mat2x3<f16>())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
         %11 = OpCompositeExtract %v3half %arg 0
         %12 = OpFConvert %v3float %11
         %13 = OpCompositeExtract %v3half %arg 1
         %14 = OpFConvert %v3float %13
     %result = OpCompositeConstruct %mat2v3float %12 %14
)");
}

TEST_F(SpirvWriterTest, Convert_Mat4x2_F32_to_F16) {
    auto* func = b.Function("foo", ty.mat4x2<f16>());
    func->SetParams({b.FunctionParam("arg", ty.mat4x2<f32>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.mat4x2<f16>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.mat4x2<f32>())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
         %11 = OpCompositeExtract %v2float %arg 0
         %12 = OpFConvert %v2half %11
         %13 = OpCompositeExtract %v2float %arg 1
         %14 = OpFConvert %v2half %13
         %15 = OpCompositeExtract %v2float %arg 2
         %16 = OpFConvert %v2half %15
         %17 = OpCompositeExtract %v2float %arg 3
         %18 = OpFConvert %v2half %17
     %result = OpCompositeConstruct %mat4v2half %12 %14 %16 %18
)");
}

TEST_F(SpirvWriterTest, Convert_F32_to_I32) {
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({b.FunctionParam("arg", ty.f32())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.i32(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.f32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %int None %5
        %arg = OpFunctionParameter %float
          %6 = OpLabel
     %result = OpFunctionCall %int %tint_f32_to_i32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %11
         %12 = OpLabel
          %x = OpFunctionCall %int %foo %float_0
               OpReturn
               OpFunctionEnd

               ; Function tint_f32_to_i32
%tint_f32_to_i32 = OpFunction %int None %5
      %value = OpFunctionParameter %float
         %16 = OpLabel
         %17 = OpExtInst %float %18 NClamp %value %float_n2_14748365e_09 %float_2_14748352e_09
         %21 = OpConvertFToS %int %17
               OpReturnValue %21
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F32_to_U32) {
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({b.FunctionParam("arg", ty.f32())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.u32(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.f32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %uint None %5
        %arg = OpFunctionParameter %float
          %6 = OpLabel
     %result = OpFunctionCall %uint %tint_f32_to_u32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %11
         %12 = OpLabel
          %x = OpFunctionCall %uint %foo %float_0
               OpReturn
               OpFunctionEnd

               ; Function tint_f32_to_u32
%tint_f32_to_u32 = OpFunction %uint None %5
      %value = OpFunctionParameter %float
         %16 = OpLabel
         %17 = OpExtInst %float %18 NClamp %value %float_0 %float_4_29496704e_09
         %20 = OpConvertFToU %uint %17
               OpReturnValue %20
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F16_to_I32) {
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({b.FunctionParam("arg", ty.f16())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.i32(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.f16())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %int None %5
        %arg = OpFunctionParameter %half
          %6 = OpLabel
     %result = OpFunctionCall %int %tint_f16_to_i32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %11
         %12 = OpLabel
          %x = OpFunctionCall %int %foo %half_0x0p_0
               OpReturn
               OpFunctionEnd

               ; Function tint_f16_to_i32
%tint_f16_to_i32 = OpFunction %int None %5
      %value = OpFunctionParameter %half
         %16 = OpLabel
         %17 = OpExtInst %half %18 NClamp %value %half_n0x1_ffcp_15 %half_0x1_ffcp_15
         %21 = OpConvertFToS %int %17
               OpReturnValue %21
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F16_to_U32) {
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({b.FunctionParam("arg", ty.f16())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.u32(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.f16())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %uint None %5
        %arg = OpFunctionParameter %half
          %6 = OpLabel
     %result = OpFunctionCall %uint %tint_f16_to_u32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %11
         %12 = OpLabel
          %x = OpFunctionCall %uint %foo %half_0x0p_0
               OpReturn
               OpFunctionEnd

               ; Function tint_f16_to_u32
%tint_f16_to_u32 = OpFunction %uint None %5
      %value = OpFunctionParameter %half
         %16 = OpLabel
         %17 = OpExtInst %half %18 NClamp %value %half_0x0p_0 %half_0x1_ffcp_15
         %20 = OpConvertFToU %uint %17
               OpReturnValue %20
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F32_to_I32_Vec2) {
    auto* func = b.Function("foo", ty.vec2i());
    func->SetParams({b.FunctionParam("arg", ty.vec2f())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.vec2i(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.vec2f())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
         %20 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg "arg"                    ; id %6
               OpName %result "result"              ; id %9
               OpName %main "main"                  ; id %11
               OpName %x "x"                        ; id %15
               OpName %tint_v2f32_to_v2i32 "tint_v2f32_to_v2i32"    ; id %10
               OpName %value "value"                                ; id %17

               ; Types, variables and constants
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
          %7 = OpTypeFunction %v2int %v2float
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
         %16 = OpConstantNull %v2float
%float_n2_14748365e_09 = OpConstant %float -2.14748365e+09
         %21 = OpConstantComposite %v2float %float_n2_14748365e_09 %float_n2_14748365e_09
%float_2_14748352e_09 = OpConstant %float 2.14748352e+09
         %23 = OpConstantComposite %v2float %float_2_14748352e_09 %float_2_14748352e_09

               ; Function foo
        %foo = OpFunction %v2int None %7
        %arg = OpFunctionParameter %v2float
          %8 = OpLabel
     %result = OpFunctionCall %v2int %tint_v2f32_to_v2i32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %13
         %14 = OpLabel
          %x = OpFunctionCall %v2int %foo %16
               OpReturn
               OpFunctionEnd

               ; Function tint_v2f32_to_v2i32
%tint_v2f32_to_v2i32 = OpFunction %v2int None %7
      %value = OpFunctionParameter %v2float
         %18 = OpLabel
         %19 = OpExtInst %v2float %20 NClamp %value %21 %23
         %25 = OpConvertFToS %v2int %19
               OpReturnValue %25
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F32_to_U32_Vec3) {
    auto* func = b.Function("foo", ty.vec3u());
    func->SetParams({b.FunctionParam("arg", ty.vec3f())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.vec3u(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.vec3f())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
         %20 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg "arg"                    ; id %6
               OpName %result "result"              ; id %9
               OpName %main "main"                  ; id %11
               OpName %x "x"                        ; id %15
               OpName %tint_v3f32_to_v3u32 "tint_v3f32_to_v3u32"    ; id %10
               OpName %value "value"                                ; id %17

               ; Types, variables and constants
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %7 = OpTypeFunction %v3uint %v3float
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
         %16 = OpConstantNull %v3float
%float_4_29496704e_09 = OpConstant %float 4.29496704e+09
         %21 = OpConstantComposite %v3float %float_4_29496704e_09 %float_4_29496704e_09 %float_4_29496704e_09

               ; Function foo
        %foo = OpFunction %v3uint None %7
        %arg = OpFunctionParameter %v3float
          %8 = OpLabel
     %result = OpFunctionCall %v3uint %tint_v3f32_to_v3u32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %13
         %14 = OpLabel
          %x = OpFunctionCall %v3uint %foo %16
               OpReturn
               OpFunctionEnd

               ; Function tint_v3f32_to_v3u32
%tint_v3f32_to_v3u32 = OpFunction %v3uint None %7
      %value = OpFunctionParameter %v3float
         %18 = OpLabel
         %19 = OpExtInst %v3float %20 NClamp %value %16 %21
         %23 = OpConvertFToU %v3uint %19
               OpReturnValue %23
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F16_to_I32_Vec2) {
    auto* func = b.Function("foo", ty.vec2i());
    func->SetParams({b.FunctionParam("arg", ty.vec2h())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.vec2i(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.vec2h())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
               OpCapability Float16
               OpCapability StorageBuffer16BitAccess
         %20 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg "arg"                    ; id %6
               OpName %result "result"              ; id %9
               OpName %main "main"                  ; id %11
               OpName %x "x"                        ; id %15
               OpName %tint_v2f16_to_v2i32 "tint_v2f16_to_v2i32"    ; id %10
               OpName %value "value"                                ; id %17

               ; Types, variables and constants
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
       %half = OpTypeFloat 16
     %v2half = OpTypeVector %half 2
          %7 = OpTypeFunction %v2int %v2half
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
         %16 = OpConstantNull %v2half
%half_n0x1_ffcp_15 = OpConstant %half -0x1.ffcp+15
         %21 = OpConstantComposite %v2half %half_n0x1_ffcp_15 %half_n0x1_ffcp_15
%half_0x1_ffcp_15 = OpConstant %half 0x1.ffcp+15
         %23 = OpConstantComposite %v2half %half_0x1_ffcp_15 %half_0x1_ffcp_15

               ; Function foo
        %foo = OpFunction %v2int None %7
        %arg = OpFunctionParameter %v2half
          %8 = OpLabel
     %result = OpFunctionCall %v2int %tint_v2f16_to_v2i32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %13
         %14 = OpLabel
          %x = OpFunctionCall %v2int %foo %16
               OpReturn
               OpFunctionEnd

               ; Function tint_v2f16_to_v2i32
%tint_v2f16_to_v2i32 = OpFunction %v2int None %7
      %value = OpFunctionParameter %v2half
         %18 = OpLabel
         %19 = OpExtInst %v2half %20 NClamp %value %21 %23
         %25 = OpConvertFToS %v2int %19
               OpReturnValue %25
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F16_to_U32_Vec4) {
    auto* func = b.Function("foo", ty.vec4u());
    func->SetParams({b.FunctionParam("arg", ty.vec4h())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.vec4u(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.vec4h())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(Shader
               OpCapability Float16
               OpCapability StorageBuffer16BitAccess
         %20 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg "arg"                    ; id %6
               OpName %result "result"              ; id %9
               OpName %main "main"                  ; id %11
               OpName %x "x"                        ; id %15
               OpName %tint_v4f16_to_v4u32 "tint_v4f16_to_v4u32"    ; id %10
               OpName %value "value"                                ; id %17

               ; Types, variables and constants
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
       %half = OpTypeFloat 16
     %v4half = OpTypeVector %half 4
          %7 = OpTypeFunction %v4uint %v4half
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
         %16 = OpConstantNull %v4half
%half_0x1_ffcp_15 = OpConstant %half 0x1.ffcp+15
         %21 = OpConstantComposite %v4half %half_0x1_ffcp_15 %half_0x1_ffcp_15 %half_0x1_ffcp_15 %half_0x1_ffcp_15

               ; Function foo
        %foo = OpFunction %v4uint None %7
        %arg = OpFunctionParameter %v4half
          %8 = OpLabel
     %result = OpFunctionCall %v4uint %tint_v4f16_to_v4u32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function main
       %main = OpFunction %void None %13
         %14 = OpLabel
          %x = OpFunctionCall %v4uint %foo %16
               OpReturn
               OpFunctionEnd

               ; Function tint_v4f16_to_v4u32
%tint_v4f16_to_v4u32 = OpFunction %v4uint None %7
      %value = OpFunctionParameter %v4half
         %18 = OpLabel
         %19 = OpExtInst %v4half %20 NClamp %value %16 %21
         %23 = OpConvertFToU %v4uint %19
               OpReturnValue %23
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
