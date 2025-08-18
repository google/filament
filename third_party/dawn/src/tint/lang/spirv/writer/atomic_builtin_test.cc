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

#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/spirv/writer/common/helper_test.h"

#include "src/tint/lang/core/enums.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer {
namespace {

TEST_F(SpirvWriterTest, AtomicAdd_Storage) {
    auto* var = b.Var("var", ty.ptr(storage, ty.atomic(ty.i32())));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* ptr = b.Let("ptr", var);
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicAdd, ptr, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicIAdd %int %ptr %uint_1 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicAdd_Workgroup) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicAdd, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicIAdd %int %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicAnd) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicAnd, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicAnd %int %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicCompareExchangeWeak) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* cmp = b.FunctionParam("cmp", ty.i32());
    auto* val = b.FunctionParam("val", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({cmp, val});

    b.Append(func->Block(), [&] {
        auto* result_ty = core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32());
        auto* result =
            b.Call(result_ty, core::BuiltinFn::kAtomicCompareExchangeWeak, var, cmp, val);
        auto* original = b.Access(ty.i32(), result, 0_u);
        b.Return(func, original);
        mod.SetName(result, "result");
        mod.SetName(original, "original");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%9 = OpAtomicCompareExchange %int %var %uint_2 %uint_0 %uint_0 %val %cmp");
    EXPECT_INST("%13 = OpIEqual %bool %9 %cmp");
    EXPECT_INST("%result = OpCompositeConstruct %__atomic_compare_exchange_result_i32 %9 %13");
    EXPECT_INST("%original = OpCompositeExtract %int %result 0");
}

TEST_F(SpirvWriterTest, AtomicExchange) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicExchange, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicExchange %int %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicLoad) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* func = b.Function("foo", ty.i32());

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicLoad, var);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicLoad %int %var %uint_2 %uint_0");
}

TEST_F(SpirvWriterTest, AtomicMax_I32) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicMax, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicSMax %int %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicMax_U32) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.u32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kAtomicMax, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicUMax %uint %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicMin_I32) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicMin, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicSMin %int %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicMin_U32) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.u32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.u32());
    auto* func = b.Function("foo", ty.u32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.u32(), core::BuiltinFn::kAtomicMin, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicUMin %uint %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicOr) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicOr, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicOr %int %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicStore) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.void_());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, var, arg1);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("OpAtomicStore %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicSub) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicSub, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicISub %int %var %uint_2 %uint_0 %arg1");
}

TEST_F(SpirvWriterTest, AtomicXor) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.atomic(ty.i32()))));

    auto* arg1 = b.FunctionParam("arg1", ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arg1});

    b.Append(func->Block(), [&] {
        auto* result = b.Call(ty.i32(), core::BuiltinFn::kAtomicXor, var, arg1);
        b.Return(func, result);
        mod.SetName(result, "result");
    });

    ASSERT_TRUE(Generate()) << Error() << output_;
    EXPECT_INST("%result = OpAtomicXor %int %var %uint_2 %uint_0 %arg1");
}

}  // namespace
}  // namespace tint::spirv::writer
