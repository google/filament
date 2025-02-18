// Copyright 2020 The Dawn & Tint Authors
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

#include "gmock/gmock.h"
#include "src/tint/lang/spirv/reader/ast_parser/function.h"
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

std::string Preamble() {
    return R"(
     OpCapability Shader
     OpMemoryModel Logical Simple
     OpEntryPoint Fragment %100 "x_100"
     OpExecutionMode %100 OriginUpperLeft
)";
}

std::string CommonTypes() {
    return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1
    %float_0 = OpConstant %float 0.0
  )";
}

std::string CommonHandleTypes() {
    return R"(
    OpName %t "t"
    OpName %s "s"
    OpDecorate %t DescriptorSet 0
    OpDecorate %t Binding 0
    OpDecorate %s DescriptorSet 0
    OpDecorate %s Binding 1
    )" + CommonTypes() +
           R"(

    %v2float = OpTypeVector %float 2
    %v4float = OpTypeVector %float 4
    %v2_0 = OpConstantNull %v2float
    %sampler = OpTypeSampler
    %tex2d_f32 = OpTypeImage %float 2D 0 0 0 1 Unknown
    %sampled_image_2d_f32 = OpTypeSampledImage %tex2d_f32
    %ptr_sampler = OpTypePointer UniformConstant %sampler
    %ptr_tex2d_f32 = OpTypePointer UniformConstant %tex2d_f32

    %t = OpVariable %ptr_tex2d_f32 UniformConstant
    %s = OpVariable %ptr_sampler UniformConstant
  )";
}

TEST_F(SpirvASTParserTest, EmitStatement_VoidCallNoParams) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void

     %50 = OpFunction %void None %voidfn
     %entry_50 = OpLabel
     OpReturn
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %void %50
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    const auto got = test::ToString(p->program());
    const char* expect = R"(fn x_50() {
  return;
}

fn x_100_1() {
  x_50();
  return;
}

@fragment
fn x_100() {
  x_100_1();
}
)";
    EXPECT_EQ(expect, got);
}

TEST_F(SpirvASTParserTest, EmitStatement_ScalarCallNoParams) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %uintfn = OpTypeFunction %uint
     %val = OpConstant %uint 42

     %50 = OpFunction %uint None %uintfn
     %entry_50 = OpLabel
     OpReturnValue %val
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %uint %50
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    tint::Vector<const ast::Statement*, 4> f100;
    {
        auto fe = p->function_emitter(100);
        EXPECT_TRUE(fe.EmitBody()) << p->error();
        f100 = fe.ast_body();
    }
    tint::Vector<const ast::Statement*, 4> f50;
    {
        auto fe = p->function_emitter(50);
        EXPECT_TRUE(fe.EmitBody()) << p->error();
        f50 = fe.ast_body();
    }
    auto program = p->program();
    EXPECT_THAT(test::ToString(program, f100), HasSubstr("let x_1 = x_50();\nreturn;"));
    EXPECT_THAT(test::ToString(program, f50), HasSubstr("return 42u;"));
}

TEST_F(SpirvASTParserTest, EmitStatement_ScalarCallNoParamsUsedTwice) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %uintfn = OpTypeFunction %uint
     %val = OpConstant %uint 42
     %ptr_uint = OpTypePointer Function %uint

     %50 = OpFunction %uint None %uintfn
     %entry_50 = OpLabel
     OpReturnValue %val
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %10 = OpVariable %ptr_uint Function
     %1 = OpFunctionCall %uint %50
     OpStore %10 %1
     OpStore %10 %1
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error();
    tint::Vector<const ast::Statement*, 4> f100;
    {
        auto fe = p->function_emitter(100);
        EXPECT_TRUE(fe.EmitBody()) << p->error();
        f100 = fe.ast_body();
    }
    tint::Vector<const ast::Statement*, 4> f50;
    {
        auto fe = p->function_emitter(50);
        EXPECT_TRUE(fe.EmitBody()) << p->error();
        f50 = fe.ast_body();
    }
    auto program = p->program();
    EXPECT_EQ(test::ToString(program, f100), R"(var x_10 : u32;
let x_1 = x_50();
x_10 = x_1;
x_10 = x_1;
return;
)");
    EXPECT_THAT(test::ToString(program, f50), HasSubstr("return 42u;"));
}

TEST_F(SpirvASTParserTest, EmitStatement_CallWithParams) {
    auto p = parser(test::Assemble(Preamble() + R"(
     %void = OpTypeVoid
     %voidfn = OpTypeFunction %void
     %uint = OpTypeInt 32 0
     %uintfn_uint_uint = OpTypeFunction %uint %uint %uint
     %val = OpConstant %uint 42
     %val2 = OpConstant %uint 84

     %50 = OpFunction %uint None %uintfn_uint_uint
     %51 = OpFunctionParameter %uint
     %52 = OpFunctionParameter %uint
     %entry_50 = OpLabel
     %sum = OpIAdd %uint %51 %52
     OpReturnValue %sum
     OpFunctionEnd

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %uint %50 %val %val2
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto program_ast_str = test::ToString(p->program());
    const std::string expected = R"(fn x_50(x_51 : u32, x_52 : u32) -> u32 {
  return (x_51 + x_52);
}

fn x_100_1() {
  let x_1 = x_50(42u, 84u);
  return;
}

@fragment
fn x_100() {
  x_100_1();
}
)";
    EXPECT_EQ(program_ast_str, expected);
}

std::string HelperFunctionPtrHandle() {
    return R"(
     ; This is how Glslang generates functions that take texture and sampler arguments.
     ; It passes them by pointer.
     %fn_ty = OpTypeFunction %void %ptr_tex2d_f32 %ptr_sampler

     %200 = OpFunction %void None %fn_ty
     %14 = OpFunctionParameter %ptr_tex2d_f32
     %15 = OpFunctionParameter %ptr_sampler
     %helper_entry = OpLabel
     ; access the texture, to give the handles usages.
     %helper_im = OpLoad %tex2d_f32 %14
     %helper_sam = OpLoad %sampler %15
     %helper_imsam = OpSampledImage %sampled_image_2d_f32 %helper_im %helper_sam
     %20 = OpImageSampleImplicitLod %v4float %helper_imsam %v2_0
     OpReturn
     OpFunctionEnd
     )";
}

std::string HelperFunctionHandle() {
    return R"(
     ; It is valid in SPIR-V to pass textures and samplers by value.
     %fn_ty = OpTypeFunction %void %tex2d_f32 %sampler

     %200 = OpFunction %void None %fn_ty
     %14 = OpFunctionParameter %tex2d_f32
     %15 = OpFunctionParameter %sampler
     %helper_entry = OpLabel
     ; access the texture, to give the handles usages.
     %helper_imsam = OpSampledImage %sampled_image_2d_f32 %14 %15
     %20 = OpImageSampleImplicitLod %v4float %helper_imsam %v2_0
     OpReturn
     OpFunctionEnd
     )";
}

TEST_F(SpirvASTParserTest, Emit_FunctionCall_HandlePtrParams_Direct) {
    auto assembly = Preamble() + CommonHandleTypes() + HelperFunctionPtrHandle() + R"(

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFunctionCall %void %200 %t %s
     OpReturn
     OpFunctionEnd
  )";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    const auto got = test::ToString(p->program(), fe.ast_body());

    const std::string expect = R"(x_200(t, s);
return;
)";
    EXPECT_EQ(got, expect);
}

TEST_F(SpirvASTParserTest, Emit_FunctionCall_HandlePtrParams_CopyObject) {
    auto assembly = Preamble() + CommonHandleTypes() + HelperFunctionPtrHandle() + R"(

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel

     %copy_t = OpCopyObject %ptr_tex2d_f32 %t
     %copy_s = OpCopyObject %ptr_sampler %s
     %1 = OpFunctionCall %void %200 %copy_t %copy_s
     OpReturn
     OpFunctionEnd
  )";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    const auto got = test::ToString(p->program(), fe.ast_body());

    const std::string expect = R"(x_200(t, s);
return;
)";
    EXPECT_EQ(got, expect);
}

TEST_F(SpirvASTParserTest, Emit_FunctionCall_HandleParams_Load) {
    auto assembly = Preamble() + CommonHandleTypes() + HelperFunctionHandle() + R"(

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %im = OpLoad %tex2d_f32 %t
     %sam = OpLoad %sampler %s
     %1 = OpFunctionCall %void %200 %im %sam
     OpReturn
     OpFunctionEnd
  )";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    const auto got = test::ToString(p->program(), fe.ast_body());

    const std::string expect = R"(x_200(t, s);
return;
)";
    EXPECT_EQ(got, expect);
}

TEST_F(SpirvASTParserTest, Emit_FunctionCall_HandleParams_LoadsAndCopyObject) {
    auto assembly = Preamble() + CommonHandleTypes() + HelperFunctionHandle() + R"(

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel

     %copy_t = OpCopyObject %ptr_tex2d_f32 %t
     %copy_s = OpCopyObject %ptr_sampler %s
     %im = OpLoad %tex2d_f32 %copy_t
     %sam = OpLoad %sampler %copy_s
     %copy_im = OpCopyObject %tex2d_f32 %im
     %copy_sam = OpCopyObject %sampler %sam
     %1 = OpFunctionCall %void %200 %copy_im %copy_sam
     OpReturn
     OpFunctionEnd
  )";

    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    const auto got = test::ToString(p->program(), fe.ast_body());

    const std::string expect = R"(x_200(t, s);
return;
)";
    EXPECT_EQ(got, expect);
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
