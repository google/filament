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

using ::testing::Eq;

std::string Preamble() {
    return R"(
    OpCapability Shader
    OpMemoryModel Logical Simple
    OpEntryPoint Fragment %main "x_100"
    OpExecutionMode %main OriginUpperLeft
  )";
}

std::string MainBody() {
    return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void
    %main = OpFunction %void None %voidfn
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd
  )";
}

TEST_F(SpirvASTParserTest, ConvertType_PreservesExistingFailure) {
    auto p = parser(std::vector<uint32_t>{});
    p->Fail() << "boing";
    auto* type = p->ConvertType(10);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("boing"));
}

TEST_F(SpirvASTParserTest, ConvertType_RequiresInternalRepresntation) {
    auto p = parser(std::vector<uint32_t>{});
    auto* type = p->ConvertType(10);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("ConvertType called when the internal module has not been built"));
}

TEST_F(SpirvASTParserTest, ConvertType_NotAnId) {
    auto assembly = Preamble() + MainBody();
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(900);
    EXPECT_EQ(type, nullptr);
    EXPECT_EQ(nullptr, type);
    EXPECT_THAT(p->error(), Eq("ID is not a SPIR-V type: 900"));
}

TEST_F(SpirvASTParserTest, ConvertType_IdExistsButIsNotAType) {
    auto assembly = R"(
     OpCapability Shader
     %1 = OpExtInstImport "GLSL.std.450"
     OpMemoryModel Logical Simple
     OpEntryPoint Fragment %main "x_100"
     OpExecutionMode %main OriginUpperLeft
)" + MainBody();
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(1);
    EXPECT_EQ(nullptr, type);
    EXPECT_THAT(p->error(), Eq("ID is not a SPIR-V type: 1"));
}

TEST_F(SpirvASTParserTest, ConvertType_UnhandledType) {
    // Pipes are an OpenCL type. Tint doesn't support them.
    auto p = parser(test::Assemble("%70 = OpTypePipe WriteOnly"));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(70);
    EXPECT_EQ(nullptr, type);
    EXPECT_THAT(p->error(), Eq("unknown SPIR-V type with ID 70: %70 = OpTypePipe WriteOnly"));
}

TEST_F(SpirvASTParserTest, ConvertType_Void) {
    auto p = parser(test::Assemble(Preamble() + "%1 = OpTypeVoid" + R"(
   %voidfn = OpTypeFunction %1
   %main = OpFunction %1 None %voidfn
   %entry = OpLabel
   OpReturn
   OpFunctionEnd
  )"));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(1);
    EXPECT_TRUE(type->Is<Void>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_Bool) {
    auto p = parser(test::Assemble(Preamble() + "%100 = OpTypeBool" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(100);
    EXPECT_TRUE(type->Is<Bool>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_I32) {
    auto p = parser(test::Assemble(Preamble() + "%2 = OpTypeInt 32 1" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(2);
    EXPECT_TRUE(type->Is<I32>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_U32) {
    auto p = parser(test::Assemble(Preamble() + "%3 = OpTypeInt 32 0" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(3);
    EXPECT_TRUE(type->Is<U32>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_F32) {
    auto p = parser(test::Assemble(Preamble() + "%4 = OpTypeFloat 32" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(4);
    EXPECT_TRUE(type->Is<F32>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_F16) {
    auto p = parser(test::Assemble(Preamble() + "%4 = OpTypeFloat 16" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(4);
    EXPECT_TRUE(type->Is<F16>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_BadIntWidth) {
    auto p = parser(test::Assemble(Preamble() + "%5 = OpTypeInt 17 1" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(5);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("unhandled integer width: 17"));
}

TEST_F(SpirvASTParserTest, ConvertType_BadFloatWidth) {
    auto p = parser(test::Assemble(Preamble() + "%6 = OpTypeFloat 19" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(6);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("unhandled float width: 19"));
}

TEST_F(SpirvASTParserTest, DISABLED_ConvertType_InvalidVectorElement) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %5 = OpTypePipe ReadOnly
    %20 = OpTypeVector %5 2
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(20);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("unknown SPIR-V type: 5"));
}

TEST_F(SpirvASTParserTest, ConvertType_VecOverF32) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %float = OpTypeFloat 32
    %20 = OpTypeVector %float 2
    %30 = OpTypeVector %float 3
    %40 = OpTypeVector %float 4
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* v2xf32 = p->ConvertType(20);
    EXPECT_TRUE(v2xf32->Is<Vector>());
    EXPECT_TRUE(v2xf32->As<Vector>()->type->Is<F32>());
    EXPECT_EQ(v2xf32->As<Vector>()->size, 2u);

    auto* v3xf32 = p->ConvertType(30);
    EXPECT_TRUE(v3xf32->Is<Vector>());
    EXPECT_TRUE(v3xf32->As<Vector>()->type->Is<F32>());
    EXPECT_EQ(v3xf32->As<Vector>()->size, 3u);

    auto* v4xf32 = p->ConvertType(40);
    EXPECT_TRUE(v4xf32->Is<Vector>());
    EXPECT_TRUE(v4xf32->As<Vector>()->type->Is<F32>());
    EXPECT_EQ(v4xf32->As<Vector>()->size, 4u);

    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_VecOverF16) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %float = OpTypeFloat 16
    %20 = OpTypeVector %float 2
    %30 = OpTypeVector %float 3
    %40 = OpTypeVector %float 4
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* v2xf16 = p->ConvertType(20);
    EXPECT_TRUE(v2xf16->Is<Vector>());
    EXPECT_TRUE(v2xf16->As<Vector>()->type->Is<F16>());
    EXPECT_EQ(v2xf16->As<Vector>()->size, 2u);

    auto* v3xf16 = p->ConvertType(30);
    EXPECT_TRUE(v3xf16->Is<Vector>());
    EXPECT_TRUE(v3xf16->As<Vector>()->type->Is<F16>());
    EXPECT_EQ(v3xf16->As<Vector>()->size, 3u);

    auto* v4xf16 = p->ConvertType(40);
    EXPECT_TRUE(v4xf16->Is<Vector>());
    EXPECT_TRUE(v4xf16->As<Vector>()->type->Is<F16>());
    EXPECT_EQ(v4xf16->As<Vector>()->size, 4u);

    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_VecOverI32) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %int = OpTypeInt 32 1
    %20 = OpTypeVector %int 2
    %30 = OpTypeVector %int 3
    %40 = OpTypeVector %int 4
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* v2xi32 = p->ConvertType(20);
    EXPECT_TRUE(v2xi32->Is<Vector>());
    EXPECT_TRUE(v2xi32->As<Vector>()->type->Is<I32>());
    EXPECT_EQ(v2xi32->As<Vector>()->size, 2u);

    auto* v3xi32 = p->ConvertType(30);
    EXPECT_TRUE(v3xi32->Is<Vector>());
    EXPECT_TRUE(v3xi32->As<Vector>()->type->Is<I32>());
    EXPECT_EQ(v3xi32->As<Vector>()->size, 3u);

    auto* v4xi32 = p->ConvertType(40);
    EXPECT_TRUE(v4xi32->Is<Vector>());
    EXPECT_TRUE(v4xi32->As<Vector>()->type->Is<I32>());
    EXPECT_EQ(v4xi32->As<Vector>()->size, 4u);

    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_VecOverU32) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %uint = OpTypeInt 32 0
    %20 = OpTypeVector %uint 2
    %30 = OpTypeVector %uint 3
    %40 = OpTypeVector %uint 4
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* v2xu32 = p->ConvertType(20);
    EXPECT_TRUE(v2xu32->Is<Vector>());
    EXPECT_TRUE(v2xu32->As<Vector>()->type->Is<U32>());
    EXPECT_EQ(v2xu32->As<Vector>()->size, 2u);

    auto* v3xu32 = p->ConvertType(30);
    EXPECT_TRUE(v3xu32->Is<Vector>());
    EXPECT_TRUE(v3xu32->As<Vector>()->type->Is<U32>());
    EXPECT_EQ(v3xu32->As<Vector>()->size, 3u);

    auto* v4xu32 = p->ConvertType(40);
    EXPECT_TRUE(v4xu32->Is<Vector>());
    EXPECT_TRUE(v4xu32->As<Vector>()->type->Is<U32>());
    EXPECT_EQ(v4xu32->As<Vector>()->size, 4u);

    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, DISABLED_ConvertType_InvalidMatrixElement) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %5 = OpTypePipe ReadOnly
    %10 = OpTypeVector %5 2
    %20 = OpTypeMatrix %10 2
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(20);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("unknown SPIR-V type: 5"));
}

TEST_F(SpirvASTParserTest, ConvertType_MatrixOverF32) {
    // Matrices are only defined over floats.
    auto p = parser(test::Assemble(Preamble() + R"(
    %float = OpTypeFloat 32
    %v2 = OpTypeVector %float 2
    %v3 = OpTypeVector %float 3
    %v4 = OpTypeVector %float 4
    ; First digit is rows
    ; Second digit is columns
    %22 = OpTypeMatrix %v2 2
    %23 = OpTypeMatrix %v2 3
    %24 = OpTypeMatrix %v2 4
    %32 = OpTypeMatrix %v3 2
    %33 = OpTypeMatrix %v3 3
    %34 = OpTypeMatrix %v3 4
    %42 = OpTypeMatrix %v4 2
    %43 = OpTypeMatrix %v4 3
    %44 = OpTypeMatrix %v4 4
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* m22 = p->ConvertType(22);
    EXPECT_TRUE(m22->Is<Matrix>());
    EXPECT_TRUE(m22->As<Matrix>()->type->Is<F32>());
    EXPECT_EQ(m22->As<Matrix>()->rows, 2u);
    EXPECT_EQ(m22->As<Matrix>()->columns, 2u);

    auto* m23 = p->ConvertType(23);
    EXPECT_TRUE(m23->Is<Matrix>());
    EXPECT_TRUE(m23->As<Matrix>()->type->Is<F32>());
    EXPECT_EQ(m23->As<Matrix>()->rows, 2u);
    EXPECT_EQ(m23->As<Matrix>()->columns, 3u);

    auto* m24 = p->ConvertType(24);
    EXPECT_TRUE(m24->Is<Matrix>());
    EXPECT_TRUE(m24->As<Matrix>()->type->Is<F32>());
    EXPECT_EQ(m24->As<Matrix>()->rows, 2u);
    EXPECT_EQ(m24->As<Matrix>()->columns, 4u);

    auto* m32 = p->ConvertType(32);
    EXPECT_TRUE(m32->Is<Matrix>());
    EXPECT_TRUE(m32->As<Matrix>()->type->Is<F32>());
    EXPECT_EQ(m32->As<Matrix>()->rows, 3u);
    EXPECT_EQ(m32->As<Matrix>()->columns, 2u);

    auto* m33 = p->ConvertType(33);
    EXPECT_TRUE(m33->Is<Matrix>());
    EXPECT_TRUE(m33->As<Matrix>()->type->Is<F32>());
    EXPECT_EQ(m33->As<Matrix>()->rows, 3u);
    EXPECT_EQ(m33->As<Matrix>()->columns, 3u);

    auto* m34 = p->ConvertType(34);
    EXPECT_TRUE(m34->Is<Matrix>());
    EXPECT_TRUE(m34->As<Matrix>()->type->Is<F32>());
    EXPECT_EQ(m34->As<Matrix>()->rows, 3u);
    EXPECT_EQ(m34->As<Matrix>()->columns, 4u);

    auto* m42 = p->ConvertType(42);
    EXPECT_TRUE(m42->Is<Matrix>());
    EXPECT_TRUE(m42->As<Matrix>()->type->Is<F32>());
    EXPECT_EQ(m42->As<Matrix>()->rows, 4u);
    EXPECT_EQ(m42->As<Matrix>()->columns, 2u);

    auto* m43 = p->ConvertType(43);
    EXPECT_TRUE(m43->Is<Matrix>());
    EXPECT_TRUE(m43->As<Matrix>()->type->Is<F32>());
    EXPECT_EQ(m43->As<Matrix>()->rows, 4u);
    EXPECT_EQ(m43->As<Matrix>()->columns, 3u);

    auto* m44 = p->ConvertType(44);
    EXPECT_TRUE(m44->Is<Matrix>());
    EXPECT_TRUE(m44->As<Matrix>()->type->Is<F32>());
    EXPECT_EQ(m44->As<Matrix>()->rows, 4u);
    EXPECT_EQ(m44->As<Matrix>()->columns, 4u);

    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_MatrixOverF16) {
    // Matrices are only defined over floats.
    auto p = parser(test::Assemble(Preamble() + R"(
    %float = OpTypeFloat 16
    %v2 = OpTypeVector %float 2
    %v3 = OpTypeVector %float 3
    %v4 = OpTypeVector %float 4
    ; First digit is rows
    ; Second digit is columns
    %22 = OpTypeMatrix %v2 2
    %23 = OpTypeMatrix %v2 3
    %24 = OpTypeMatrix %v2 4
    %32 = OpTypeMatrix %v3 2
    %33 = OpTypeMatrix %v3 3
    %34 = OpTypeMatrix %v3 4
    %42 = OpTypeMatrix %v4 2
    %43 = OpTypeMatrix %v4 3
    %44 = OpTypeMatrix %v4 4
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* m22 = p->ConvertType(22);
    EXPECT_TRUE(m22->Is<Matrix>());
    EXPECT_TRUE(m22->As<Matrix>()->type->Is<F16>());
    EXPECT_EQ(m22->As<Matrix>()->rows, 2u);
    EXPECT_EQ(m22->As<Matrix>()->columns, 2u);

    auto* m23 = p->ConvertType(23);
    EXPECT_TRUE(m23->Is<Matrix>());
    EXPECT_TRUE(m23->As<Matrix>()->type->Is<F16>());
    EXPECT_EQ(m23->As<Matrix>()->rows, 2u);
    EXPECT_EQ(m23->As<Matrix>()->columns, 3u);

    auto* m24 = p->ConvertType(24);
    EXPECT_TRUE(m24->Is<Matrix>());
    EXPECT_TRUE(m24->As<Matrix>()->type->Is<F16>());
    EXPECT_EQ(m24->As<Matrix>()->rows, 2u);
    EXPECT_EQ(m24->As<Matrix>()->columns, 4u);

    auto* m32 = p->ConvertType(32);
    EXPECT_TRUE(m32->Is<Matrix>());
    EXPECT_TRUE(m32->As<Matrix>()->type->Is<F16>());
    EXPECT_EQ(m32->As<Matrix>()->rows, 3u);
    EXPECT_EQ(m32->As<Matrix>()->columns, 2u);

    auto* m33 = p->ConvertType(33);
    EXPECT_TRUE(m33->Is<Matrix>());
    EXPECT_TRUE(m33->As<Matrix>()->type->Is<F16>());
    EXPECT_EQ(m33->As<Matrix>()->rows, 3u);
    EXPECT_EQ(m33->As<Matrix>()->columns, 3u);

    auto* m34 = p->ConvertType(34);
    EXPECT_TRUE(m34->Is<Matrix>());
    EXPECT_TRUE(m34->As<Matrix>()->type->Is<F16>());
    EXPECT_EQ(m34->As<Matrix>()->rows, 3u);
    EXPECT_EQ(m34->As<Matrix>()->columns, 4u);

    auto* m42 = p->ConvertType(42);
    EXPECT_TRUE(m42->Is<Matrix>());
    EXPECT_TRUE(m42->As<Matrix>()->type->Is<F16>());
    EXPECT_EQ(m42->As<Matrix>()->rows, 4u);
    EXPECT_EQ(m42->As<Matrix>()->columns, 2u);

    auto* m43 = p->ConvertType(43);
    EXPECT_TRUE(m43->Is<Matrix>());
    EXPECT_TRUE(m43->As<Matrix>()->type->Is<F16>());
    EXPECT_EQ(m43->As<Matrix>()->rows, 4u);
    EXPECT_EQ(m43->As<Matrix>()->columns, 3u);

    auto* m44 = p->ConvertType(44);
    EXPECT_TRUE(m44->Is<Matrix>());
    EXPECT_TRUE(m44->As<Matrix>()->type->Is<F16>());
    EXPECT_EQ(m44->As<Matrix>()->rows, 4u);
    EXPECT_EQ(m44->As<Matrix>()->columns, 4u);

    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_RuntimeArray) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %uint = OpTypeInt 32 0
    %10 = OpTypeRuntimeArray %uint
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(10);
    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->UnwrapAll()->Is<Array>());
    auto* arr_type = type->UnwrapAll()->As<Array>();
    ASSERT_NE(arr_type, nullptr);
    EXPECT_EQ(arr_type->size, 0u);
    EXPECT_EQ(arr_type->stride, 0u);
    auto* elem_type = arr_type->type;
    ASSERT_NE(elem_type, nullptr);
    EXPECT_TRUE(elem_type->Is<U32>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_RuntimeArray_InvalidDecoration) {
    auto p = parser(test::Assemble(Preamble() + R"(
    OpDecorate %10 Block
    %uint = OpTypeInt 32 0
    %10 = OpTypeRuntimeArray %uint
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    auto* type = p->ConvertType(10);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(),
                Eq("invalid array type ID 10: unknown decoration 2 with 1 total words"));
}

TEST_F(SpirvASTParserTest, ConvertType_RuntimeArray_ArrayStride_Valid) {
    auto p = parser(test::Assemble(Preamble() + R"(
    OpDecorate %10 ArrayStride 64
    %uint = OpTypeInt 32 0
    %10 = OpTypeRuntimeArray %uint
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    auto* type = p->ConvertType(10);
    ASSERT_NE(type, nullptr);
    auto* arr_type = type->UnwrapAll()->As<Array>();
    ASSERT_NE(arr_type, nullptr);
    EXPECT_EQ(arr_type->size, 0u);
    EXPECT_EQ(arr_type->stride, 64u);
}

TEST_F(SpirvASTParserTest, ConvertType_RuntimeArray_ArrayStride_ZeroIsError) {
    auto p = parser(test::Assemble(Preamble() + R"(
    OpDecorate %10 ArrayStride 0
    %uint = OpTypeInt 32 0
    %10 = OpTypeRuntimeArray %uint
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    auto* type = p->ConvertType(10);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("invalid array type ID 10: ArrayStride can't be 0"));
}

TEST_F(SpirvASTParserTest, ConvertType_Array) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %uint = OpTypeInt 32 0
    %uint_42 = OpConstant %uint 42
    %10 = OpTypeArray %uint %uint_42
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(10);
    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->Is<Array>());
    auto* arr_type = type->As<Array>();
    ASSERT_NE(arr_type, nullptr);
    EXPECT_EQ(arr_type->size, 42u);
    EXPECT_EQ(arr_type->stride, 0u);
    auto* elem_type = arr_type->type;
    ASSERT_NE(elem_type, nullptr);
    EXPECT_TRUE(elem_type->Is<U32>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_ArrayBadLengthIsSpecConstantValue) {
    auto p = parser(test::Assemble(Preamble() + R"(
    OpDecorate %uint_42 SpecId 12
    %uint = OpTypeInt 32 0
    %uint_42 = OpSpecConstant %uint 42
    %10 = OpTypeArray %uint %uint_42
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(10);
    ASSERT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("Array type 10 length is a specialization constant"));
}

TEST_F(SpirvASTParserTest, ConvertType_ArrayBadLengthIsSpecConstantExpr) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %uint = OpTypeInt 32 0
    %uint_42 = OpConstant %uint 42
    %sum = OpSpecConstantOp %uint IAdd %uint_42 %uint_42
    %10 = OpTypeArray %uint %sum
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(10);
    ASSERT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("Array type 10 length is a specialization constant"));
}

// TODO(dneto): Maybe add a test where the length operand is not a constant.
// E.g. it's the ID of a type.  That won't validate, and the SPIRV-Tools
// optimizer representation doesn't handle it and asserts out instead.

TEST_F(SpirvASTParserTest, ConvertType_ArrayBadTooBig) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %uint64 = OpTypeInt 64 0
    %uint64_big = OpConstant %uint64 5000000000
    %10 = OpTypeArray %uint64 %uint64_big
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(10);
    ASSERT_EQ(type, nullptr);
    // TODO(dneto): Right now it's rejected earlier in the flow because
    // we can't even utter the uint64 type.
    EXPECT_THAT(p->error(), Eq("unhandled integer width: 64"));
}

TEST_F(SpirvASTParserTest, ConvertType_Array_InvalidDecoration) {
    auto p = parser(test::Assemble(Preamble() + R"(
    OpDecorate %10 Block
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %10 = OpTypeArray %uint %uint_5
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    auto* type = p->ConvertType(10);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(),
                Eq("invalid array type ID 10: unknown decoration 2 with 1 total words"));
}

TEST_F(SpirvASTParserTest, ConvertType_ArrayStride_Valid) {
    auto p = parser(test::Assemble(Preamble() + R"(
    OpDecorate %10 ArrayStride 8
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %10 = OpTypeArray %uint %uint_5
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(10);
    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->UnwrapAll()->Is<Array>());
    auto* arr_type = type->UnwrapAll()->As<Array>();
    ASSERT_NE(arr_type, nullptr);
    EXPECT_EQ(arr_type->stride, 8u);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_ArrayStride_ZeroIsError) {
    auto p = parser(test::Assemble(Preamble() + R"(
    OpDecorate %10 ArrayStride 0
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %10 = OpTypeArray %uint %uint_5
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(10);
    ASSERT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("invalid array type ID 10: ArrayStride can't be 0"));
}

TEST_F(SpirvASTParserTest, ConvertType_StructEmpty) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %10 = OpTypeStruct
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(10);
    EXPECT_EQ(type, nullptr);
    EXPECT_EQ(p->error(),
              "WGSL does not support empty structures. can't convert type: %10 = "
              "OpTypeStruct");
}

TEST_F(SpirvASTParserTest, ConvertType_StructTwoMembers) {
    auto p = parser(test::Assemble(Preamble() + R"(
    %uint = OpTypeInt 32 0
    %float = OpTypeFloat 32
    %10 = OpTypeStruct %uint %float
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->RegisterUserAndStructMemberNames());

    auto* type = p->ConvertType(10);
    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->Is<Struct>());

    auto str = type->Build(p->builder());
    Program program = p->program();
    EXPECT_EQ(test::ToString(program, str), "S");
}

TEST_F(SpirvASTParserTest, ConvertType_StructWithBlockDecoration) {
    auto p = parser(test::Assemble(Preamble() + R"(
    OpDecorate %10 Block
    %uint = OpTypeInt 32 0
    %10 = OpTypeStruct %uint
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->RegisterUserAndStructMemberNames());

    auto* type = p->ConvertType(10);
    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->Is<Struct>());

    auto str = type->Build(p->builder());
    Program program = p->program();
    EXPECT_EQ(test::ToString(program, str), "S");
}

TEST_F(SpirvASTParserTest, ConvertType_StructWithMemberDecorations) {
    auto p = parser(test::Assemble(Preamble() + R"(
    OpMemberDecorate %10 0 Offset 0
    OpMemberDecorate %10 1 Offset 8
    OpMemberDecorate %10 2 Offset 16
    %float = OpTypeFloat 32
    %vec = OpTypeVector %float 2
    %mat = OpTypeMatrix %vec 2
    %10 = OpTypeStruct %float %vec %mat
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->RegisterUserAndStructMemberNames());

    auto* type = p->ConvertType(10);
    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->Is<Struct>());

    auto str = type->Build(p->builder());
    Program program = p->program();
    EXPECT_EQ(test::ToString(program, str), "S");
}

TEST_F(SpirvASTParserTest, ConvertType_Struct_NoDeduplication) {
    // Prove that distinct SPIR-V structs map to distinct WGSL types.
    auto p = parser(test::Assemble(Preamble() + R"(
    %uint = OpTypeInt 32 0
    %10 = OpTypeStruct %uint
    %11 = OpTypeStruct %uint
  )" + MainBody()));
    EXPECT_TRUE(p->BuildAndParseInternalModule());

    auto* type10 = p->ConvertType(10);
    ASSERT_NE(type10, nullptr);
    EXPECT_TRUE(type10->Is<Struct>());
    auto* struct_type10 = type10->As<Struct>();
    ASSERT_NE(struct_type10, nullptr);
    EXPECT_EQ(struct_type10->members.size(), 1u);
    EXPECT_TRUE(struct_type10->members[0]->Is<U32>());

    auto* type11 = p->ConvertType(11);
    ASSERT_NE(type11, nullptr);
    EXPECT_TRUE(type11->Is<Struct>());
    auto* struct_type11 = type11->As<Struct>();
    ASSERT_NE(struct_type11, nullptr);
    EXPECT_EQ(struct_type11->members.size(), 1u);
    EXPECT_TRUE(struct_type11->members[0]->Is<U32>());

    // They map to distinct types in WGSL
    EXPECT_NE(type11, type10);
}

TEST_F(SpirvASTParserTest, ConvertType_Array_NoDeduplication) {
    // Prove that distinct SPIR-V arrays map to distinct WGSL types.
    auto assembly = Preamble() + R"(
    %uint = OpTypeInt 32 0
    %10 = OpTypeStruct %uint
    %11 = OpTypeStruct %uint
    %uint_1 = OpConstant %uint 1
    %20 = OpTypeArray %10 %uint_1
    %21 = OpTypeArray %11 %uint_1
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());

    auto* type20 = p->ConvertType(20);
    ASSERT_NE(type20, nullptr);
    EXPECT_TRUE(type20->Is<Array>());

    auto* type21 = p->ConvertType(21);
    ASSERT_NE(type21, nullptr);
    EXPECT_TRUE(type21->Is<Array>());

    // They map to distinct types in WGSL
    EXPECT_NE(type21, type20);
}

TEST_F(SpirvASTParserTest, ConvertType_RuntimeArray_NoDeduplication) {
    // Prove that distinct SPIR-V runtime arrays map to distinct WGSL types.
    // The implementation already de-duplicates them because it knows
    // runtime-arrays normally have stride decorations.
    auto assembly = Preamble() + R"(
    %uint = OpTypeInt 32 0
    %10 = OpTypeStruct %uint
    %11 = OpTypeStruct %uint
    %20 = OpTypeRuntimeArray %10
    %21 = OpTypeRuntimeArray %11
    %22 = OpTypeRuntimeArray %10
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());

    auto* type20 = p->ConvertType(20);
    ASSERT_NE(type20, nullptr);
    EXPECT_TRUE(type20->Is<Alias>());
    EXPECT_TRUE(type20->UnwrapAll()->Is<Array>());
    EXPECT_EQ(type20->UnwrapAll()->As<Array>()->size, 0u);

    auto* type21 = p->ConvertType(21);
    ASSERT_NE(type21, nullptr);
    EXPECT_TRUE(type21->Is<Alias>());
    EXPECT_TRUE(type21->UnwrapAll()->Is<Array>());
    EXPECT_EQ(type21->UnwrapAll()->As<Array>()->size, 0u);

    auto* type22 = p->ConvertType(22);
    ASSERT_NE(type22, nullptr);
    EXPECT_TRUE(type22->Is<Alias>());
    EXPECT_TRUE(type22->UnwrapAll()->Is<Array>());
    EXPECT_EQ(type22->UnwrapAll()->As<Array>()->size, 0u);

    // They map to distinct types in WGSL
    EXPECT_NE(type21, type20);
    EXPECT_NE(type22, type21);
    EXPECT_NE(type22, type20);
}

// TODO(dneto): Demonstrate other member decorations. Blocked on
// crbug.com/tint/30
// TODO(dneto): Demonstrate multiple member deocrations. Blocked on
// crbug.com/tint/30

TEST_F(SpirvASTParserTest, ConvertType_InvalidPointeetype) {
    // Disallow pointer-to-function
    auto p = parser(test::Assemble(Preamble() + R"(
  %void = OpTypeVoid
  %42 = OpTypeFunction %void
  %3 = OpTypePointer Input %42

%voidfn = OpTypeFunction %void
%main = OpFunction %void None %voidfn
%entry = OpLabel
OpReturn
OpFunctionEnd
  )"));
    EXPECT_TRUE(p->BuildInternalModule()) << p->error();

    auto* type = p->ConvertType(3);
    EXPECT_EQ(type, nullptr);
    EXPECT_THAT(p->error(), Eq("SPIR-V pointer type with ID 3 has invalid pointee type 42"));
}

TEST_F(SpirvASTParserTest, DISABLED_ConvertType_InvalidAddressSpace) {
    // Disallow invalid address space
    auto p = parser(test::Assemble(Preamble() + R"(
  %1 = OpTypeFloat 32
  %3 = OpTypePointer !999 %1   ; Special syntax to inject 999 as the address space
  )" + MainBody()));
    // TODO(dneto): I can't get it past module building.
    EXPECT_FALSE(p->BuildInternalModule()) << p->error();
}

TEST_F(SpirvASTParserTest, ConvertType_PointerInput) {
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Input %float
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(3);
    EXPECT_TRUE(type->Is<Pointer>());
    auto* ptr_ty = type->As<Pointer>();
    EXPECT_NE(ptr_ty, nullptr);
    EXPECT_TRUE(ptr_ty->type->Is<F32>());
    EXPECT_EQ(ptr_ty->address_space, core::AddressSpace::kPrivate);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_PointerOutput) {
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Output %float
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(3);
    EXPECT_TRUE(type->Is<Pointer>());
    auto* ptr_ty = type->As<Pointer>();
    EXPECT_NE(ptr_ty, nullptr);
    EXPECT_TRUE(ptr_ty->type->Is<F32>());
    EXPECT_EQ(ptr_ty->address_space, core::AddressSpace::kPrivate);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_PointerUniform) {
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Uniform %float
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(3);
    EXPECT_TRUE(type->Is<Pointer>());
    auto* ptr_ty = type->As<Pointer>();
    EXPECT_NE(ptr_ty, nullptr);
    EXPECT_TRUE(ptr_ty->type->Is<F32>());
    EXPECT_EQ(ptr_ty->address_space, core::AddressSpace::kUniform);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_PointerWorkgroup) {
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Workgroup %float
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(3);
    EXPECT_TRUE(type->Is<Pointer>());
    auto* ptr_ty = type->As<Pointer>();
    EXPECT_NE(ptr_ty, nullptr);
    EXPECT_TRUE(ptr_ty->type->Is<F32>());
    EXPECT_EQ(ptr_ty->address_space, core::AddressSpace::kWorkgroup);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_PointerStorageBuffer) {
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer StorageBuffer %float
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(3);
    EXPECT_TRUE(type->Is<Pointer>());
    auto* ptr_ty = type->As<Pointer>();
    EXPECT_NE(ptr_ty, nullptr);
    EXPECT_TRUE(ptr_ty->type->Is<F32>());
    EXPECT_EQ(ptr_ty->address_space, core::AddressSpace::kStorage);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_PointerPrivate) {
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Private %float
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(3);
    EXPECT_TRUE(type->Is<Pointer>());
    auto* ptr_ty = type->As<Pointer>();
    EXPECT_NE(ptr_ty, nullptr);
    EXPECT_TRUE(ptr_ty->type->Is<F32>());
    EXPECT_EQ(ptr_ty->address_space, core::AddressSpace::kPrivate);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_PointerFunction) {
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %3 = OpTypePointer Function %float
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(3);
    EXPECT_TRUE(type->Is<Pointer>());
    auto* ptr_ty = type->As<Pointer>();
    EXPECT_NE(ptr_ty, nullptr);
    EXPECT_TRUE(ptr_ty->type->Is<F32>());
    EXPECT_EQ(ptr_ty->address_space, core::AddressSpace::kFunction);
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_PointerToPointer) {
    // FYI:  The reader suports pointer-to-pointer even while WebGPU does not.
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %42 = OpTypePointer Output %float
  %3 = OpTypePointer Input %42
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(3);
    EXPECT_NE(type, nullptr);
    EXPECT_TRUE(type->Is<Pointer>());

    auto* ptr_ty = type->As<Pointer>();
    EXPECT_NE(ptr_ty, nullptr);
    EXPECT_EQ(ptr_ty->address_space, core::AddressSpace::kPrivate);
    EXPECT_TRUE(ptr_ty->type->Is<Pointer>());

    auto* ptr_ptr_ty = ptr_ty->type->As<Pointer>();
    EXPECT_NE(ptr_ptr_ty, nullptr);
    EXPECT_EQ(ptr_ptr_ty->address_space, core::AddressSpace::kPrivate);
    EXPECT_TRUE(ptr_ptr_ty->type->Is<F32>());

    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_Sampler_PretendVoid) {
    // We fake the type suport for samplers, images, and sampled images.
    auto p = parser(test::Assemble(Preamble() + R"(
  %1 = OpTypeSampler
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(1);
    EXPECT_TRUE(type->Is<Void>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_Image_PretendVoid) {
    // We fake the type suport for samplers, images, and sampled images.
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %1 = OpTypeImage %float 2D 0 0 0 1 Unknown
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(1);
    EXPECT_TRUE(type->Is<Void>());
    EXPECT_TRUE(p->error().empty());
}

TEST_F(SpirvASTParserTest, ConvertType_SampledImage_PretendVoid) {
    auto p = parser(test::Assemble(Preamble() + R"(
  %float = OpTypeFloat 32
  %im = OpTypeImage %float 2D 0 0 0 1 Unknown
  %1 = OpTypeSampledImage %im
  )" + MainBody()));
    EXPECT_TRUE(p->BuildInternalModule());

    auto* type = p->ConvertType(1);
    EXPECT_TRUE(type->Is<Void>());
    EXPECT_TRUE(p->error().empty());
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
