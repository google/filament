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

TEST_F(SpirvWriterTest, Access_Array_Value_ConstantIndex) {
    auto* arr_val = b.FunctionParam("arr", ty.array(ty.i32(), 4));
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr_val});
    b.Append(func->Block(), [&] {
        auto* result = b.Access(ty.i32(), arr_val, 1_u);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpCompositeExtract %int %arr 1");
}

TEST_F(SpirvWriterTest, Access_Array_Pointer_ConstantIndex) {
    auto* func = b.Function("foo", ty.i32());
    b.Append(func->Block(), [&] {
        auto* arr_var = b.Var("arr", ty.ptr<function, array<i32, 4>>());
        auto* result = b.Access(ty.ptr<function, i32>(), arr_var, 1_u);
        b.Return(func, b.Load(result));
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAccessChain %_ptr_Function_int %arr %uint_1");
}

TEST_F(SpirvWriterTest, Access_Array_Pointer_DynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* arr_var = b.Var("arr", ty.ptr<function, array<i32, 4>>());
        auto* result = b.Access(ty.ptr<function, i32>(), arr_var, idx);
        b.Return(func, b.Load(result));
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %12 = OpBitcast %uint %idx
         %13 = OpExtInst %uint %14 UMin %12 %uint_3
     %result = OpAccessChain %_ptr_Function_int %arr %13
)");
}

TEST_F(SpirvWriterTest, Access_Matrix_Value_ConstantIndex) {
    auto* mat_val = b.FunctionParam("mat", ty.mat2x2(ty.f32()));
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({mat_val});
    b.Append(func->Block(), [&] {
        auto* result_vector = b.Access(ty.vec2(ty.f32()), mat_val, 1_u);
        auto* result_scalar = b.Access(ty.f32(), mat_val, 1_u, 0_u);
        b.Return(func, b.Multiply(ty.vec2<f32>(), result_vector, result_scalar));
        mod.SetName(result_vector, "result_vector");
        mod.SetName(result_scalar, "result_scalar");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result_vector = OpCompositeExtract %v2float %mat 1");
    EXPECT_INST("%result_scalar = OpCompositeExtract %float %mat 1 0");
}

TEST_F(SpirvWriterTest, Access_Matrix_Pointer_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* mat_var = b.Var("mat", ty.ptr<function, mat2x2<f32>>());
        auto* result_vector = b.Access(ty.ptr<function, vec2<f32>>(), mat_var, 1_u);
        auto* result_scalar = b.LoadVectorElement(result_vector, 0_u);
        b.Return(func);
        mod.SetName(result_vector, "result_vector");
        mod.SetName(result_scalar, "result_scalar");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result_vector = OpAccessChain %_ptr_Function_v2float %mat %uint_1");
    EXPECT_INST("%15 = OpAccessChain %_ptr_Function_float %result_vector %uint_0");
    EXPECT_INST("%result_scalar = OpLoad %float %15");
}

TEST_F(SpirvWriterTest, Access_Matrix_Pointer_DynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* mat_var = b.Var("mat", ty.ptr<function, mat2x2<f32>>());
        auto* result_vector = b.Access(ty.ptr<function, vec2<f32>>(), mat_var, idx);
        auto* result_scalar = b.LoadVectorElement(result_vector, idx);
        b.Return(func);
        mod.SetName(result_vector, "result_vector");
        mod.SetName(result_scalar, "result_scalar");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %14 = OpBitcast %uint %idx
         %15 = OpExtInst %uint %16 UMin %14 %uint_1
%result_vector = OpAccessChain %_ptr_Function_v2float %mat %15
         %20 = OpBitcast %uint %idx
         %21 = OpExtInst %uint %16 UMin %20 %uint_1
         %22 = OpAccessChain %_ptr_Function_float %result_vector %21
%result_scalar = OpLoad %float %22 None
)");
}

TEST_F(SpirvWriterTest, Access_Vector_Value_ConstantIndex) {
    auto* vec_val = b.FunctionParam("vec", ty.vec4(ty.i32()));
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({vec_val});
    b.Append(func->Block(), [&] {
        auto* result = b.Access(ty.i32(), vec_val, 1_u);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpCompositeExtract %int %vec 1");
}

TEST_F(SpirvWriterTest, Access_Vector_Value_DynamicIndex) {
    auto* vec_val = b.FunctionParam("vec", ty.vec4(ty.i32()));
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({vec_val, idx});
    b.Append(func->Block(), [&] {
        auto* result = b.Access(ty.i32(), vec_val, idx);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
          %9 = OpBitcast %uint %idx
         %10 = OpExtInst %uint %11 UMin %9 %uint_3
     %result = OpVectorExtractDynamic %int %vec %10
)");
}

TEST_F(SpirvWriterTest, Access_NestedVector_Value_DynamicIndex) {
    auto* val = b.FunctionParam("arr", ty.array(ty.array(ty.vec4(ty.i32()), 4), 4));
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({val, idx});
    b.Append(func->Block(), [&] {
        auto* result = b.Access(ty.i32(), val, 1_u, 2_u, idx);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %12 = OpBitcast %uint %idx
         %13 = OpExtInst %uint %14 UMin %12 %uint_3
         %17 = OpCompositeExtract %v4int %arr 1 2
     %result = OpVectorExtractDynamic %int %17 %13
)");
}

TEST_F(SpirvWriterTest, Access_Struct_Value_ConstantIndex) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.i32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    auto* str_val = b.FunctionParam("str", str);
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({str_val});
    b.Append(func->Block(), [&] {
        auto* result_a = b.Access(ty.i32(), str_val, 0_u);
        auto* result_b = b.Access(ty.i32(), str_val, 1_u, 2_u);
        b.Return(func, b.Add(ty.i32(), result_a, result_b));
        mod.SetName(result_a, "result_a");
        mod.SetName(result_b, "result_b");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result_a = OpCompositeExtract %int %str 0");
    EXPECT_INST("%result_b = OpCompositeExtract %int %str 1 2");
}

TEST_F(SpirvWriterTest, Access_Struct_Pointer_ConstantIndex) {
    auto* str =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.Register("a"), ty.i32()},
                                                   {mod.symbols.Register("b"), ty.vec4<i32>()},
                                               });
    auto* func = b.Function("foo", ty.vec4<i32>());
    b.Append(func->Block(), [&] {
        auto* str_var = b.Var("str", ty.ptr(function, str, read_write));
        auto* result_a = b.Access(ty.ptr<function, i32>(), str_var, 0_u);
        auto* result_b = b.Access(ty.ptr<function, vec4<i32>>(), str_var, 1_u);
        auto* val_a = b.Load(result_a);
        auto* val_b = b.Load(result_b);
        b.Return(func, b.Add(ty.vec4<i32>(), val_a, val_b));
        mod.SetName(result_a, "result_a");
        mod.SetName(result_b, "result_b");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result_a = OpAccessChain %_ptr_Function_int %str %uint_0");
    EXPECT_INST("%result_b = OpAccessChain %_ptr_Function_v4int %str %uint_1");
}

TEST_F(SpirvWriterTest, LoadVectorElement_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        auto* result = b.LoadVectorElement(vec_var, 1_u);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%10 = OpAccessChain %_ptr_Function_int %vec %uint_1");
    EXPECT_INST("%result = OpLoad %int %10");
}

TEST_F(SpirvWriterTest, LoadVectorElement_DynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        auto* result = b.LoadVectorElement(vec_var, idx);
        b.Return(func);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %12 = OpBitcast %uint %idx
         %13 = OpExtInst %uint %14 UMin %12 %uint_3
         %16 = OpAccessChain %_ptr_Function_int %vec %13
     %result = OpLoad %int %16 None
)");
}

TEST_F(SpirvWriterTest, StoreVectorElement_ConstantIndex) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, 1_u, b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%10 = OpAccessChain %_ptr_Function_int %vec %uint_1");
    EXPECT_INST("OpStore %10 %int_42");
}

TEST_F(SpirvWriterTest, StoreVectorElement_DynamicIndex) {
    auto* idx = b.FunctionParam("idx", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({idx});
    b.Append(func->Block(), [&] {
        auto* vec_var = b.Var("vec", ty.ptr<function, vec4<i32>>());
        b.StoreVectorElement(vec_var, idx, b.Constant(42_i));
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST(R"(
         %12 = OpBitcast %uint %idx
         %13 = OpExtInst %uint %14 UMin %12 %uint_3
         %16 = OpAccessChain %_ptr_Function_int %vec %13
               OpStore %16 %int_42 None
)");
}

}  // namespace
}  // namespace tint::spirv::writer
