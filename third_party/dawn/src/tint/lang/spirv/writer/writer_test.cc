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

#include "gmock/gmock.h"

namespace tint::spirv::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(SpirvWriterTest, ModuleHeader) {
    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpCapability Shader");
    EXPECT_INST("OpMemoryModel Logical GLSL450");
}

TEST_F(SpirvWriterTest, ModuleHeader_VulkanMemoryModel) {
    Options opts;
    opts.use_vulkan_memory_model = true;

    ASSERT_TRUE(Generate(opts)) << Error() << output_;
    EXPECT_INST("OpExtension \"SPV_KHR_vulkan_memory_model\"");
    EXPECT_INST("OpCapability VulkanMemoryModel");
    EXPECT_INST("OpCapability VulkanMemoryModelDeviceScope");
    EXPECT_INST("OpMemoryModel Logical Vulkan");
}

TEST_F(SpirvWriterTest, Unreachable) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            auto* ifelse = b.If(true);
            b.Append(ifelse->True(), [&] {  //
                b.Continue(loop);
            });
            b.Append(ifelse->False(), [&] {  //
                b.Continue(loop);
            });
            b.Unreachable();

            b.Append(loop->Continuing(), [&] {  //
                b.NextIteration(loop);
            });
        });
        b.Return(func);
    });

    Options options;
    options.disable_robustness = true;
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
        %foo = OpFunction %void None %3
          %4 = OpLabel
               OpBranch %7
          %7 = OpLabel
               OpLoopMerge %8 %6 None
               OpBranch %5
          %5 = OpLabel
               OpSelectionMerge %9 None
               OpBranchConditional %true %10 %11
         %10 = OpLabel
               OpBranch %6
         %11 = OpLabel
               OpBranch %6
          %9 = OpLabel
               OpUnreachable
          %6 = OpLabel
               OpBranch %7
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)");
}

// Test that we fail gracefully when a function has too many parameters.
// See crbug.com/354748060.
TEST_F(SpirvWriterTest, TooManyFunctionParameters) {
    Vector<core::ir::FunctionParam*, 256> params;
    for (uint32_t i = 0; i < 256; i++) {
        params.Push(b.FunctionParam(ty.i32()));
    }
    auto* func = b.Function("foo", ty.void_());
    func->SetParams(std::move(params));
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    EXPECT_FALSE(Generate());
    EXPECT_THAT(Error(),
                testing::HasSubstr(
                    "Function 'foo' has more than 255 parameters after running Tint transforms"));
}

TEST_F(SpirvWriterTest, EntryPointName_Remapped) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "my_entry_point";
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST("OpEntryPoint GLCompute %main \"my_entry_point\"");
}

TEST_F(SpirvWriterTest, EntryPointName_NotRemapped) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    Options options;
    options.remapped_entry_point_name = "";
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST("OpEntryPoint GLCompute %main \"main\"");
}

TEST_F(SpirvWriterTest, StripAllNames) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.i32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    auto* func = b.ComputeFunction("main");
    auto* idx = b.FunctionParam("idx", ty.u32());
    idx->SetBuiltin(core::BuiltinValue::kLocalInvocationIndex);
    func->AppendParam(idx);
    b.Append(func->Block(), [&] {  //
        auto* var = b.Var("str", ty.ptr<function>(str));
        auto* val = b.Load(var);
        mod.SetName(val, "val");
        auto* a = b.Access<i32>(val, 0_u);
        mod.SetName(a, "a");
        b.Return(func);
    });

    Options options;
    options.strip_all_names = true;
    options.remapped_entry_point_name = "tint_entry_point";
    ASSERT_TRUE(Generate(options)) << Error() << output_;
    EXPECT_INST(R"(
               OpEntryPoint GLCompute %16 "tint_entry_point" %gl_LocalInvocationIndex
               OpExecutionMode %16 LocalSize 1 1 1

               ; Annotations
               OpDecorate %gl_LocalInvocationIndex BuiltIn LocalInvocationIndex
               OpMemberDecorate %_struct_11 0 Offset 0
               OpMemberDecorate %_struct_11 1 Offset 16

               ; Types, variables and constants
       %uint = OpTypeInt 32 0
%_ptr_Input_uint = OpTypePointer Input %uint
%gl_LocalInvocationIndex = OpVariable %_ptr_Input_uint Input    ; BuiltIn LocalInvocationIndex
       %void = OpTypeVoid
          %7 = OpTypeFunction %void %uint
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
 %_struct_11 = OpTypeStruct %int %v4int
%_ptr_Function__struct_11 = OpTypePointer Function %_struct_11
         %14 = OpConstantNull %_struct_11
         %17 = OpTypeFunction %void

               ; Function 4
          %4 = OpFunction %void None %7
          %6 = OpFunctionParameter %uint
          %8 = OpLabel
          %9 = OpVariable %_ptr_Function__struct_11 Function %14
         %15 = OpLoad %_struct_11 %9 None
               OpReturn
               OpFunctionEnd

               ; Function 16
         %16 = OpFunction %void None %17
         %18 = OpLabel
         %19 = OpLoad %uint %gl_LocalInvocationIndex None
         %20 = OpFunctionCall %void %4 %19
               OpReturn
               OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::spirv::writer
