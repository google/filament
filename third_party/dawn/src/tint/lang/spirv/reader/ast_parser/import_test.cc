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
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::UnorderedElementsAre;

using SpvParserImportTest = SpirvASTParserTest;

TEST_F(SpvParserImportTest, Import_NoImport) {
    auto p = parser(test::Assemble("%1 = OpTypeVoid"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto program_ast = test::ToString(p->program());
    EXPECT_THAT(program_ast, Not(HasSubstr("Import")));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserImportTest, Import_ImportGlslStd450) {
    auto p = parser(test::Assemble(R"(%1 = OpExtInstImport "GLSL.std.450")"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());
    EXPECT_THAT(p->glsl_std_450_imports(), ElementsAre(1));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserImportTest, Import_NonSemantic_IgnoredImport) {
    auto p = parser(test::Assemble(R"(%40 = OpExtInstImport "NonSemantic.ClspvReflection.1")"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserImportTest, Import_NonSemantic_IgnoredExtInsts) {
    // This is the clspv-compiled output of this OpenCL C:
    //    kernel void foo(global int*A) { A=A; }
    // It emits NonSemantic.ClspvReflection.1 extended instructions.
    // But *tweaked*:
    //    - to remove gl_WorkgroupSize
    //    - to add LocalSize execution mode
    //    - to move one of the ExtInsts into the globals-and-constants
    //      section
    //    - to move one of the ExtInsts into the function body.
    auto p = parser(test::Assemble(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpExtension "SPV_KHR_non_semantic_info"
         %20 = OpExtInstImport "NonSemantic.ClspvReflection.1"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %15 "foo"
               OpExecutionMode %15 LocalSize 1 1 1
               OpSource OpenCL_C 120
         %21 = OpString "foo"
         %23 = OpString "A"
               OpDecorate %_runtimearr_uint ArrayStride 4
               OpMemberDecorate %_struct_3 0 Offset 0
               OpDecorate %_struct_3 Block
               OpDecorate %12 DescriptorSet 0
               OpDecorate %12 Binding 0
               OpDecorate %7 SpecId 0
               OpDecorate %8 SpecId 1
               OpDecorate %9 SpecId 2
       %void = OpTypeVoid
         %24 = OpExtInst %void %20 ArgumentInfo %23
       %uint = OpTypeInt 32 0
%_runtimearr_uint = OpTypeRuntimeArray %uint
  %_struct_3 = OpTypeStruct %_runtimearr_uint
%_ptr_StorageBuffer__struct_3 = OpTypePointer StorageBuffer %_struct_3
     %v3uint = OpTypeVector %uint 3
%_ptr_Private_v3uint = OpTypePointer Private %v3uint
          %7 = OpSpecConstant %uint 1
          %8 = OpSpecConstant %uint 1
          %9 = OpSpecConstant %uint 1
         %14 = OpTypeFunction %void
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
         %12 = OpVariable %_ptr_StorageBuffer__struct_3 StorageBuffer
         %15 = OpFunction %void Const %14
         %16 = OpLabel
         %19 = OpAccessChain %_ptr_StorageBuffer_uint %12 %uint_0 %uint_0
         %22 = OpExtInst %void %20 Kernel %15 %21
               OpReturn
               OpFunctionEnd
         %25 = OpExtInst %void %20 ArgumentStorageBuffer %22 %uint_0 %uint_0 %uint_0 %24
         %28 = OpExtInst %void %20 SpecConstantWorkgroupSize %uint_0 %uint_1 %uint_2
)"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_TRUE(p->error().empty());

    p->SkipDumpingPending("crbug.com/tint/1041 track access mode in spirv-reader parser type");
}

// TODO(dneto): We don't currently support other kinds of extended instruction
// imports.

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
