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

using ::testing::HasSubstr;

TEST_F(SpirvASTParserTest, NamedTypes_AnonStruct) {
    auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("struct S"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpirvASTParserTest, NamedTypes_NamedStruct) {
    auto p = parser(test::Assemble(R"(
    OpName %s "mystruct"
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("struct mystruct"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpirvASTParserTest, NamedTypes_Dup_EmitBoth) {
    auto p = parser(test::Assemble(R"(
    %uint = OpTypeInt 32 0
    %s = OpTypeStruct %uint %uint
    %s2 = OpTypeStruct %uint %uint
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error();
    EXPECT_THAT(test::ToString(p->program()), HasSubstr(R"(struct S {
  field0 : u32,
  field1 : u32,
}

struct S_1 {
  field0 : u32,
  field1 : u32,
})"));

    p->DeliberatelyInvalidSpirv();
}

// TODO(dneto): Should we make an alias for an un-decoratrd array with
// an OpName?

TEST_F(SpirvASTParserTest, NamedTypes_AnonRTArrayWithDecoration) {
    // Runtime arrays are always in SSBO, and those are always laid out.
    auto p = parser(test::Assemble(R"(
    OpDecorate %arr ArrayStride 8
    %uint = OpTypeInt 32 0
    %arr = OpTypeRuntimeArray %uint
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("RTArr = @stride(8) array<u32>;\n"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpirvASTParserTest, NamedTypes_AnonRTArray_Dup_EmitBoth) {
    auto p = parser(test::Assemble(R"(
    OpDecorate %arr ArrayStride 8
    OpDecorate %arr2 ArrayStride 8
    %uint = OpTypeInt 32 0
    %arr = OpTypeRuntimeArray %uint
    %arr2 = OpTypeRuntimeArray %uint
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(test::ToString(p->program()), HasSubstr(R"(alias RTArr = @stride(8) array<u32>;

alias RTArr_1 = @stride(8) array<u32>;
)"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpirvASTParserTest, NamedTypes_NamedRTArray) {
    auto p = parser(test::Assemble(R"(
    OpName %arr "myrtarr"
    OpDecorate %arr ArrayStride 8
    %uint = OpTypeInt 32 0
    %arr = OpTypeRuntimeArray %uint
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("myrtarr = @stride(8) array<u32>;\n"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpirvASTParserTest, NamedTypes_NamedArray) {
    auto p = parser(test::Assemble(R"(
    OpName %arr "myarr"
    OpDecorate %arr ArrayStride 8
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %arr = OpTypeArray %uint %uint_5
    %arr2 = OpTypeArray %uint %uint_5
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(test::ToString(p->program()), HasSubstr("myarr = @stride(8) array<u32, 5u>;"));

    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpirvASTParserTest, NamedTypes_AnonArray_Dup_EmitBoth) {
    auto p = parser(test::Assemble(R"(
    OpDecorate %arr ArrayStride 8
    OpDecorate %arr2 ArrayStride 8
    %uint = OpTypeInt 32 0
    %uint_5 = OpConstant %uint 5
    %arr = OpTypeArray %uint %uint_5
    %arr2 = OpTypeArray %uint %uint_5
  )"));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    EXPECT_THAT(test::ToString(p->program()), HasSubstr(R"(alias Arr = @stride(8) array<u32, 5u>;

alias Arr_1 = @stride(8) array<u32, 5u>;
)"));

    p->DeliberatelyInvalidSpirv();
}

// Make sure that we do not deduplicate nested structures, as this will break the names used for
// chained accessors.
TEST_F(SpirvASTParserTest, NamedTypes_NestedStructsDifferOnlyInMemberNames) {
    auto p = parser(test::Assemble(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %main "main"

               OpName %FooInner "FooInner"
               OpName %FooOuter "FooOuter"
               OpMemberName %FooInner 0 "foo_member"
               OpMemberName %FooOuter 0 "foo_inner"

               OpName %BarInner "BarInner"
               OpName %BarOuter "BarOuter"
               OpMemberName %BarInner 0 "bar_member"
               OpMemberName %BarOuter 0 "bar_inner"

       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0

   %FooInner = OpTypeStruct %int
   %FooOuter = OpTypeStruct %FooInner

   %BarInner = OpTypeStruct %int
   %BarOuter = OpTypeStruct %BarInner

     %FooPtr = OpTypePointer Private %FooOuter
     %FooVar = OpVariable %FooPtr Private

     %BarPtr = OpTypePointer Private %BarOuter
     %BarVar = OpVariable %BarPtr Private

    %ptr_int = OpTypePointer Private %int

       %main = OpFunction %void None %3
      %start = OpLabel
 %access_foo = OpAccessChain %ptr_int %FooVar %int_0 %int_0
 %access_bar = OpAccessChain %ptr_int %BarVar %int_0 %int_0
     %fooval = OpLoad %int %access_foo
     %barval = OpLoad %int %access_bar
               OpReturn
               OpFunctionEnd
  )"));

    EXPECT_TRUE(p->BuildAndParseInternalModule());

    auto program = p->program();
    EXPECT_EQ(test::ToString(program), R"(struct FooInner {
  foo_member : i32,
}

struct FooOuter {
  foo_inner : FooInner,
}

struct BarInner {
  bar_member : i32,
}

struct BarOuter {
  bar_inner : BarInner,
}

var<private> x_11 : FooOuter;

var<private> x_13 : BarOuter;

fn main_1() {
  let x_18 = x_11.foo_inner.foo_member;
  let x_19 = x_13.bar_inner.bar_member;
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn main() {
  main_1();
}
)");
}

// TODO(dneto): Handle arrays sized by a spec constant.
// Blocked by crbug.com/tint/32

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
