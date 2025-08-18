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
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
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

    ASSERT_TRUE(Generate()) << Error() << output_;
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

    ASSERT_TRUE(Generate()) << Error() << output_;
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

    ASSERT_TRUE(Generate()) << Error() << output_;
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

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %int None %5
        %arg = OpFunctionParameter %float
          %6 = OpLabel
     %result = OpFunctionCall %int %tint_f32_to_i32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_f32_to_i32
%tint_f32_to_i32 = OpFunction %int None %5
      %value = OpFunctionParameter %float
         %10 = OpLabel
         %11 = OpExtInst %float %12 NClamp %value %float_n2_14748365e_09 %float_2_14748352e_09
         %15 = OpConvertFToS %int %11
               OpReturnValue %15
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %18
         %19 = OpLabel
               OpReturn
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

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %uint None %5
        %arg = OpFunctionParameter %float
          %6 = OpLabel
     %result = OpFunctionCall %uint %tint_f32_to_u32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_f32_to_u32
%tint_f32_to_u32 = OpFunction %uint None %5
      %value = OpFunctionParameter %float
         %10 = OpLabel
         %11 = OpExtInst %float %12 NClamp %value %float_0 %float_4_29496704e_09
         %15 = OpConvertFToU %uint %11
               OpReturnValue %15
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %18
         %19 = OpLabel
               OpReturn
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

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %int None %5
        %arg = OpFunctionParameter %half
          %6 = OpLabel
     %result = OpFunctionCall %int %tint_f16_to_i32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_f16_to_i32
%tint_f16_to_i32 = OpFunction %int None %5
      %value = OpFunctionParameter %half
         %10 = OpLabel
         %11 = OpExtInst %half %12 NClamp %value %half_n0x1_ffcp_15 %half_0x1_ffcp_15
         %15 = OpConvertFToS %int %11
               OpReturnValue %15
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %18
         %19 = OpLabel
               OpReturn
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

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               ; Function foo
        %foo = OpFunction %uint None %5
        %arg = OpFunctionParameter %half
          %6 = OpLabel
     %result = OpFunctionCall %uint %tint_f16_to_u32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_f16_to_u32
%tint_f16_to_u32 = OpFunction %uint None %5
      %value = OpFunctionParameter %half
         %10 = OpLabel
         %11 = OpExtInst %half %12 NClamp %value %half_0x0p_0 %half_0x1_ffcp_15
         %15 = OpConvertFToU %uint %11
               OpReturnValue %15
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %18
         %19 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F32_to_I32_Vec2) {
    auto* func = b.Function("foo", ty.vec2<i32>());
    func->SetParams({b.FunctionParam("arg", ty.vec2<f32>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.vec2<i32>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %14 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg "arg"                    ; id %6
               OpName %result "result"              ; id %9
               OpName %tint_v2f32_to_v2i32 "tint_v2f32_to_v2i32"    ; id %10
               OpName %value "value"                                ; id %11
               OpName %unused_entry_point "unused_entry_point"      ; id %20

               ; Types, variables and constants
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
          %7 = OpTypeFunction %v2int %v2float
%float_n2_14748365e_09 = OpConstant %float -2.14748365e+09
         %15 = OpConstantComposite %v2float %float_n2_14748365e_09 %float_n2_14748365e_09
%float_2_14748352e_09 = OpConstant %float 2.14748352e+09
         %17 = OpConstantComposite %v2float %float_2_14748352e_09 %float_2_14748352e_09
       %void = OpTypeVoid
         %22 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %v2int None %7
        %arg = OpFunctionParameter %v2float
          %8 = OpLabel
     %result = OpFunctionCall %v2int %tint_v2f32_to_v2i32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_v2f32_to_v2i32
%tint_v2f32_to_v2i32 = OpFunction %v2int None %7
      %value = OpFunctionParameter %v2float
         %12 = OpLabel
         %13 = OpExtInst %v2float %14 NClamp %value %15 %17
         %19 = OpConvertFToS %v2int %13
               OpReturnValue %19
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %22
         %23 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F32_to_U32_Vec3) {
    auto* func = b.Function("foo", ty.vec3<u32>());
    func->SetParams({b.FunctionParam("arg", ty.vec3<f32>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.vec3<u32>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %14 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg "arg"                    ; id %6
               OpName %result "result"              ; id %9
               OpName %tint_v3f32_to_v3u32 "tint_v3f32_to_v3u32"    ; id %10
               OpName %value "value"                                ; id %11
               OpName %unused_entry_point "unused_entry_point"      ; id %19

               ; Types, variables and constants
       %uint = OpTypeInt 32 0
     %v3uint = OpTypeVector %uint 3
      %float = OpTypeFloat 32
    %v3float = OpTypeVector %float 3
          %7 = OpTypeFunction %v3uint %v3float
         %15 = OpConstantNull %v3float
%float_4_29496704e_09 = OpConstant %float 4.29496704e+09
         %16 = OpConstantComposite %v3float %float_4_29496704e_09 %float_4_29496704e_09 %float_4_29496704e_09
       %void = OpTypeVoid
         %21 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %v3uint None %7
        %arg = OpFunctionParameter %v3float
          %8 = OpLabel
     %result = OpFunctionCall %v3uint %tint_v3f32_to_v3u32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_v3f32_to_v3u32
%tint_v3f32_to_v3u32 = OpFunction %v3uint None %7
      %value = OpFunctionParameter %v3float
         %12 = OpLabel
         %13 = OpExtInst %v3float %14 NClamp %value %15 %16
         %18 = OpConvertFToU %v3uint %13
               OpReturnValue %18
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %21
         %22 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F16_to_I32_Vec2) {
    auto* func = b.Function("foo", ty.vec2<i32>());
    func->SetParams({b.FunctionParam("arg", ty.vec2<f16>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.vec2<i32>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
               OpCapability Float16
               OpCapability UniformAndStorageBuffer16BitAccess
               OpCapability StorageBuffer16BitAccess
         %14 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg "arg"                    ; id %6
               OpName %result "result"              ; id %9
               OpName %tint_v2f16_to_v2i32 "tint_v2f16_to_v2i32"    ; id %10
               OpName %value "value"                                ; id %11
               OpName %unused_entry_point "unused_entry_point"      ; id %20

               ; Types, variables and constants
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
       %half = OpTypeFloat 16
     %v2half = OpTypeVector %half 2
          %7 = OpTypeFunction %v2int %v2half
%half_n0x1_ffcp_15 = OpConstant %half -0x1.ffcp+15
         %15 = OpConstantComposite %v2half %half_n0x1_ffcp_15 %half_n0x1_ffcp_15
%half_0x1_ffcp_15 = OpConstant %half 0x1.ffcp+15
         %17 = OpConstantComposite %v2half %half_0x1_ffcp_15 %half_0x1_ffcp_15
       %void = OpTypeVoid
         %22 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %v2int None %7
        %arg = OpFunctionParameter %v2half
          %8 = OpLabel
     %result = OpFunctionCall %v2int %tint_v2f16_to_v2i32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_v2f16_to_v2i32
%tint_v2f16_to_v2i32 = OpFunction %v2int None %7
      %value = OpFunctionParameter %v2half
         %12 = OpLabel
         %13 = OpExtInst %v2half %14 NClamp %value %15 %17
         %19 = OpConvertFToS %v2int %13
               OpReturnValue %19
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %22
         %23 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

TEST_F(SpirvWriterTest, Convert_F16_to_U32_Vec4) {
    auto* func = b.Function("foo", ty.vec4<u32>());
    func->SetParams({b.FunctionParam("arg", ty.vec4<f16>())});
    b.Append(func->Block(), [&] {
        auto* result = b.Convert(ty.vec4<u32>(), func->Params()[0]);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(hader
               OpCapability Float16
               OpCapability UniformAndStorageBuffer16BitAccess
               OpCapability StorageBuffer16BitAccess
         %14 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %unused_entry_point "unused_entry_point"
               OpExecutionMode %unused_entry_point LocalSize 1 1 1

               ; Debug Information
               OpName %foo "foo"                    ; id %1
               OpName %arg "arg"                    ; id %6
               OpName %result "result"              ; id %9
               OpName %tint_v4f16_to_v4u32 "tint_v4f16_to_v4u32"    ; id %10
               OpName %value "value"                                ; id %11
               OpName %unused_entry_point "unused_entry_point"      ; id %19

               ; Types, variables and constants
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
       %half = OpTypeFloat 16
     %v4half = OpTypeVector %half 4
          %7 = OpTypeFunction %v4uint %v4half
         %15 = OpConstantNull %v4half
%half_0x1_ffcp_15 = OpConstant %half 0x1.ffcp+15
         %16 = OpConstantComposite %v4half %half_0x1_ffcp_15 %half_0x1_ffcp_15 %half_0x1_ffcp_15 %half_0x1_ffcp_15
       %void = OpTypeVoid
         %21 = OpTypeFunction %void

               ; Function foo
        %foo = OpFunction %v4uint None %7
        %arg = OpFunctionParameter %v4half
          %8 = OpLabel
     %result = OpFunctionCall %v4uint %tint_v4f16_to_v4u32 %arg
               OpReturnValue %result
               OpFunctionEnd

               ; Function tint_v4f16_to_v4u32
%tint_v4f16_to_v4u32 = OpFunction %v4uint None %7
      %value = OpFunctionParameter %v4half
         %12 = OpLabel
         %13 = OpExtInst %v4half %14 NClamp %value %15 %16
         %18 = OpConvertFToU %v4uint %13
               OpReturnValue %18
               OpFunctionEnd

               ; Function unused_entry_point
%unused_entry_point = OpFunction %void None %21
         %22 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
