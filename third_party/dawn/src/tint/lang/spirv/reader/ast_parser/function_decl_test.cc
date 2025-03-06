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
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::HasSubstr;

std::string Preamble() {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %100 "x_100"
    OpExecutionMode %100 OriginUpperLeft
  )";
}

/// @returns a SPIR-V assembly segment which assigns debug names
/// to particular IDs.
std::string Names(std::vector<std::string> ids) {
    StringStream outs;
    for (auto& id : ids) {
        outs << "    OpName %" << id << " \"" << id << "\"\n";
    }
    return outs.str();
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
    return CommonTypes() + R"(
    %v2float = OpTypeVector %float 2
    %v4float = OpTypeVector %float 4
    %v2_0 = OpConstantNull %v2float
    %sampler = OpTypeSampler
    %tex2d_f32 = OpTypeImage %float 2D 0 0 0 1 Unknown
    %sampled_image_2d_f32 = OpTypeSampledImage %tex2d_f32
    %ptr_sampler = OpTypePointer UniformConstant %sampler
    %ptr_tex2d_f32 = OpTypePointer UniformConstant %tex2d_f32
  )";
}

std::string MainBody() {
    return R"(
    %100 = OpFunction %void None %voidfn
    %entry_100 = OpLabel
    OpReturn
    OpFunctionEnd
  )";
}

TEST_F(SpirvASTParserTest, Emit_VoidFunctionWithoutParams) {
    auto p = parser(test::Assemble(Preamble() + CommonTypes() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )"));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.Emit());
    auto got = test::ToString(p->program());
    std::string expect = R"(fn x_100() {
  return;
}
)";
    EXPECT_EQ(got, expect);
}

TEST_F(SpirvASTParserTest, Emit_NonVoidResultType) {
    auto p = parser(test::Assemble(Preamble() + CommonTypes() + R"(
     %fn_ret_float = OpTypeFunction %float
     %200 = OpFunction %float None %fn_ret_float
     %entry = OpLabel
     OpReturnValue %float_0
     OpFunctionEnd
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.Emit());

    auto got = test::ToString(p->program());
    std::string expect = R"(fn x_200() -> f32 {
  return 0.0f;
}
)";
    EXPECT_THAT(got, HasSubstr(expect));
}

TEST_F(SpirvASTParserTest, Emit_MixedParamTypes) {
    auto p = parser(test::Assemble(Preamble() + Names({"a", "b", "c"}) + CommonTypes() + R"(
     %fn_mixed_params = OpTypeFunction %void %uint %float %int

     %200 = OpFunction %void None %fn_mixed_params
     %a = OpFunctionParameter %uint
     %b = OpFunctionParameter %float
     %c = OpFunctionParameter %int
     %mixed_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.Emit());

    auto got = test::ToString(p->program());
    std::string expect = R"(fn x_200(a : u32, b : f32, c : i32) {
  return;
}
)";
    EXPECT_THAT(got, HasSubstr(expect));
}

TEST_F(SpirvASTParserTest, Emit_GenerateParamNames) {
    auto p = parser(test::Assemble(Preamble() + CommonTypes() + R"(
     %fn_mixed_params = OpTypeFunction %void %uint %float %int

     %200 = OpFunction %void None %fn_mixed_params
     %14 = OpFunctionParameter %uint
     %15 = OpFunctionParameter %float
     %16 = OpFunctionParameter %int
     %mixed_entry = OpLabel
     OpReturn
     OpFunctionEnd
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.Emit());

    auto got = test::ToString(p->program());
    std::string expect = R"(fn x_200(x_14 : u32, x_15 : f32, x_16 : i32) {
  return;
}
)";
    EXPECT_THAT(got, HasSubstr(expect));
}

TEST_F(SpirvASTParserTest, Emit_FunctionDecl_ParamPtrTexture_ParamPtrSampler) {
    auto p = parser(test::Assemble(Preamble() + CommonHandleTypes() + R"(

     ; This is how Glslang generates functions that take texture and sampler arguments.
     ; It passes them by pointer.
     %fn_ty = OpTypeFunction %void %ptr_tex2d_f32 %ptr_sampler

     %200 = OpFunction %void None %fn_ty
     %14 = OpFunctionParameter %ptr_tex2d_f32
     %15 = OpFunctionParameter %ptr_sampler
     %mixed_entry = OpLabel
     ; access the texture, to give the handles usages.
     %im = OpLoad %tex2d_f32 %14
     %sam = OpLoad %sampler %15
     %imsam = OpSampledImage %sampled_image_2d_f32 %im %sam
     %20 = OpImageSampleImplicitLod %v4float %imsam %v2_0
     OpReturn
     OpFunctionEnd
  )" + MainBody()));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.Emit());

    auto got = test::ToString(p->program());
    std::string expect = R"(fn x_200(x_14 : texture_2d<f32>, x_15 : sampler) {
  let x_20 = textureSample(x_14, x_15, vec2f());
  return;
}
)";
    EXPECT_EQ(got, expect);
}

TEST_F(SpirvASTParserTest, Emit_FunctionDecl_ParamTexture_ParamSampler) {
    auto assembly = Preamble() + CommonHandleTypes() + R"(

     ; It is valid in SPIR-V to pass textures and samplers by value.
     %fn_ty = OpTypeFunction %void %tex2d_f32 %sampler

     %200 = OpFunction %void None %fn_ty
     %14 = OpFunctionParameter %tex2d_f32
     %15 = OpFunctionParameter %sampler
     %mixed_entry = OpLabel
     ; access the texture, to give the handles usages.
     %imsam = OpSampledImage %sampled_image_2d_f32 %14 %15
     %20 = OpImageSampleImplicitLod %v4float %imsam %v2_0
     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << assembly;
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.Emit());

    auto got = test::ToString(p->program());
    std::string expect = R"(fn x_200(x_14 : texture_2d<f32>, x_15 : sampler) {
  let x_20 = textureSample(x_14, x_15, vec2f());
  return;
}
)";
    EXPECT_EQ(got, expect);
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
