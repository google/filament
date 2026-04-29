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

TEST_F(SpirvWriterTest, Construct_Vector) {
    auto* func = b.Function("foo", ty.vec4i());
    func->SetParams({
        b.FunctionParam("a", ty.i32()),
        b.FunctionParam("b", ty.i32()),
        b.FunctionParam("c", ty.i32()),
        b.FunctionParam("d", ty.i32()),
    });
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec4i(), func->Params());
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x",
              b.Call(func, b.Zero(ty.i32()), b.Zero(ty.i32()), b.Zero(ty.i32()), b.Zero(ty.i32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpCompositeConstruct %v4int %a %b %c %d");
}

TEST_F(SpirvWriterTest, Construct_Matrix) {
    auto* func = b.Function("foo", ty.mat3x4<f32>());
    func->SetParams({
        b.FunctionParam("a", ty.vec4f()),
        b.FunctionParam("b", ty.vec4f()),
        b.FunctionParam("c", ty.vec4f()),
    });
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.mat3x4<f32>(), func->Params());
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.vec4f()), b.Zero(ty.vec4f()), b.Zero(ty.vec4f())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpCompositeConstruct %mat3v4float %a %b %c");
}

TEST_F(SpirvWriterTest, Construct_Array) {
    auto* func = b.Function("foo", ty.array<f32, 4>());
    func->SetParams({
        b.FunctionParam("a", ty.f32()),
        b.FunctionParam("b", ty.f32()),
        b.FunctionParam("c", ty.f32()),
        b.FunctionParam("d", ty.f32()),
    });
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.array<f32, 4>(), func->Params());
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x",
              b.Call(func, b.Zero(ty.f32()), b.Zero(ty.f32()), b.Zero(ty.f32()), b.Zero(ty.f32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpCompositeConstruct %_arr_float_uint_4 %a %b %c %d");
}

TEST_F(SpirvWriterTest, Construct_Struct) {
    auto* str = ty.Struct(mod.symbols.New("MyStruct"), {
                                                           {mod.symbols.Register("a"), ty.i32()},
                                                           {mod.symbols.Register("b"), ty.u32()},
                                                           {mod.symbols.Register("c"), ty.vec4f()},
                                                       });
    auto* func = b.Function("foo", str);
    func->SetParams({
        b.FunctionParam("a", ty.i32()),
        b.FunctionParam("b", ty.u32()),
        b.FunctionParam("c", ty.vec4f()),
    });
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(str, func->Params());
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32()), b.Zero(ty.u32()), b.Zero(ty.vec4f())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpCompositeConstruct %MyStruct %a %b %c");
}

TEST_F(SpirvWriterTest, Construct_Scalar_Identity) {
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({b.FunctionParam("arg", ty.i32())});
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.i32(), func->Params()[0]);
        b.Return(func, result);
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.i32())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

TEST_F(SpirvWriterTest, Construct_Vector_Identity) {
    auto* func = b.Function("foo", ty.vec4i());
    func->SetParams({b.FunctionParam("arg", ty.vec4i())});
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.vec4i(), func->Params()[0]);
        b.Return(func, result);
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func, b.Zero(ty.vec4i())));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("OpReturnValue %arg");
}

TEST_F(SpirvWriterTest, Construct_Array_ZeroValue) {
    auto* func = b.Function("foo", ty.array<f32, 4>());
    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.array<f32, 4>());
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Let("x", b.Call(func));
        b.Return(eb);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%result = OpConstantNull %_arr_float_uint_4");
}

TEST_F(SpirvWriterTest, Construct_SubgroupMatrix_ZeroValue) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Let("left", b.Construct(ty.subgroup_matrix_left(ty.f32(), 8, 4)));
        b.Let("right", b.Construct(ty.subgroup_matrix_right(ty.i32(), 4, 8)));
        b.Let("result", b.Construct(ty.subgroup_matrix_result(ty.u32(), 2, 2)));
        b.Return(func);
    });

    Options options;
    options.extensions.use_vulkan_memory_model = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%6 = OpTypeCooperativeMatrixKHR %float %uint_3 %uint_4 %uint_8 %uint_0");
    EXPECT_INST("%left = OpConstantNull %6");
    EXPECT_INST("%14 = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_8 %uint_4 %uint_1");
    EXPECT_INST("%right = OpConstantNull %14");
    EXPECT_INST("%18 = OpTypeCooperativeMatrixKHR %uint %uint_3 %uint_2 %uint_2 %uint_2");
    EXPECT_INST("%result = OpConstantNull %18");
}

TEST_F(SpirvWriterTest, Construct_SubgroupMatrix_SingleValue) {
    auto* func = b.Function("foo", ty.void_());
    auto* arg = b.FunctionParam("arg", ty.i32());
    func->SetParams({arg});
    b.Append(func->Block(), [&] {
        b.Let("left", b.Construct(ty.subgroup_matrix_left(ty.i32(), 8, 4), arg));
        b.Let("right", b.Construct(ty.subgroup_matrix_right(ty.i32(), 4, 8), arg));
        b.Let("result", b.Construct(ty.subgroup_matrix_result(ty.i32(), 2, 2), arg));
        b.Return(func);
    });

    auto* eb = b.ComputeFunction("main");
    b.Append(eb->Block(), [&] {
        b.Call(func, b.Zero(ty.i32()));
        b.Return(eb);
    });

    Options options;
    options.extensions.use_vulkan_memory_model = true;
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST("%7 = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_4 %uint_8 %uint_0");
    EXPECT_INST("%14 = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_8 %uint_4 %uint_1");
    EXPECT_INST("%17 = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_2 %uint_2 %uint_2");
    EXPECT_INST("%left = OpCompositeConstruct %7 %arg");
    EXPECT_INST("%right = OpCompositeConstruct %14 %arg");
    EXPECT_INST("%result = OpCompositeConstruct %17 %arg");
}

TEST_F(SpirvWriterTest, Construct_ArrayOfSubgroupMatrix_ZeroValue) {
    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Let("left", b.Construct(ty.array(ty.subgroup_matrix_left(ty.f32(), 8, 4), 4u)));
        b.Let("right", b.Construct(ty.array(ty.subgroup_matrix_right(ty.i32(), 4, 8), 4u)));
        b.Let("result", b.Construct(ty.array(ty.subgroup_matrix_result(ty.u32(), 2, 2), 4u)));
        b.Return(func);
    });

    Options options{
        .entry_point_name = "main",
        .extensions =
            {
                .use_vulkan_memory_model = true,
            },
    };
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
          %7 = OpTypeCooperativeMatrixKHR %float %uint_3 %uint_4 %uint_8 %uint_0
%_arr_7_uint_4 = OpTypeArray %7 %uint_4
       %left = OpConstantNull %_arr_7_uint_4
        %int = OpTypeInt 32 1
     %uint_1 = OpConstant %uint 1
         %16 = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_8 %uint_4 %uint_1
%_arr_16_uint_4 = OpTypeArray %16 %uint_4
      %right = OpConstantNull %_arr_16_uint_4
     %uint_2 = OpConstant %uint 2
         %21 = OpTypeCooperativeMatrixKHR %uint %uint_3 %uint_2 %uint_2 %uint_2
%_arr_21_uint_4 = OpTypeArray %21 %uint_4
     %result = OpConstantNull %_arr_21_uint_4
)");
}

TEST_F(SpirvWriterTest, Construct_StructOfSubgroupMatrix_ZeroValue) {
    auto* str = ty.Struct(
        mod.symbols.New("MyStruct"),
        {
            {mod.symbols.Register("left"), ty.array(ty.subgroup_matrix_left(ty.f32(), 8, 4), 4u)},
            {mod.symbols.Register("right"), ty.array(ty.subgroup_matrix_right(ty.i32(), 4, 8), 4u)},
            {mod.symbols.Register("res"), ty.array(ty.subgroup_matrix_result(ty.u32(), 2, 2), 4u)},
        });

    auto* func = b.ComputeFunction("main");
    b.Append(func->Block(), [&] {
        b.Let("s", b.Construct(str));
        b.Return(func);
    });

    Options options{
        .entry_point_name = "main",
        .extensions =
            {
                .use_vulkan_memory_model = true,
            },
    };
    auto result = Generate(options);
    ASSERT_EQ(result, Success) << result.Failure() << output_;
    EXPECT_INST(R"(
          %8 = OpTypeCooperativeMatrixKHR %float %uint_3 %uint_4 %uint_8 %uint_0
%_arr_8_uint_4 = OpTypeArray %8 %uint_4
        %int = OpTypeInt 32 1
     %uint_1 = OpConstant %uint 1
         %16 = OpTypeCooperativeMatrixKHR %int %uint_3 %uint_8 %uint_4 %uint_1
%_arr_16_uint_4 = OpTypeArray %16 %uint_4
     %uint_2 = OpConstant %uint 2
         %20 = OpTypeCooperativeMatrixKHR %uint %uint_3 %uint_2 %uint_2 %uint_2
%_arr_20_uint_4 = OpTypeArray %20 %uint_4
   %MyStruct = OpTypeStruct %_arr_8_uint_4 %_arr_16_uint_4 %_arr_20_uint_4
          %s = OpConstantNull %MyStruct
)");
}

}  // namespace
}  // namespace tint::spirv::writer
