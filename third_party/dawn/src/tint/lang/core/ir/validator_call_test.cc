// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/validator_test.h"

#include <string>

#include "gtest/gtest.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, CallToFunctionOutsideModule) {
    auto* f = b.Function("f", ty.void_());
    auto* g = b.Function("g", ty.void_());
    mod.functions.Pop();  // Remove g

    b.Append(f->Block(), [&] {
        b.Call(g);
        b.Return(f);
    });
    b.Append(g->Block(), [&] { b.Return(g); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:20 error: call: %g is not part of the module
    %2:void = call %g
                   ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToEntryPointFunction) {
    auto* f = b.Function("f", ty.void_());
    auto* g = ComputeEntryPoint("g");

    b.Append(f->Block(), [&] {
        b.Call(g);
        b.Return(f);
    });
    b.Append(g->Block(), [&] { b.Return(g); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:20 error: call: call target must not have a pipeline stage
    %2:void = call %g
                   ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToFunctionTooFewArguments) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>(), b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, 42_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:8:20 error: call: function has 2 parameters, but call provides 1 arguments
    %5:void = call %g, 42i
                   ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToFunctionTooManyArguments) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>(), b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, 1_i, 2_i, 3_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:8:20 error: call: function has 2 parameters, but call provides 3 arguments
    %5:void = call %g, 1i, 2i, 3i
                   ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToFunctionWrongArgType) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>(), b.FunctionParam<i32>(), b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, 1_i, 2_f, 3_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:8:28 error: call: type 'i32' of function parameter 1 does not match argument type 'f32'
    %6:void = call %g, 1i, 2.0f, 3i
                           ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToFunctionNullArg) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:8:24 error: call: operand is undefined
    %4:void = call %g, undef
                       ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToNullFunction) {
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->SetOperands(Vector{static_cast<ir::Value*>(nullptr)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:8:20 error: call: operand is undefined
    %3:void = call undef
                   ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToFunctionNoResult) {
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:13 error: call: expected exactly 1 results, got 0
    undef = call %g
            ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToFunctionNoOperands) {
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:15 error: call: expected at least 1 operands, got 0
    %3:void = call
              ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToNonFunctionTarget) {
    auto* g = b.Function("g", ty.void_());
    mod.functions.Pop();  // Remove g, since it isn't actually going to be used, it is just
                          // needed to create the UserCall before mangling it

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->SetOperands(Vector{b.Value(0_i)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:20 error: call: target not defined or not a function
    %2:void = call 0i
                   ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToBuiltin_MissingResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(ty.f32(), BuiltinFn::kAbs, 1_f);
        c->SetResults(Vector{static_cast<ir::InstructionResult*>(nullptr)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:5 error: abs: result is undefined
    undef = abs 1.0f
    ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToBuiltin_MismatchResultType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(ty.f32(), BuiltinFn::kAbs, 1_f);
        c->Result()->SetType(ty.i32());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:14 error: abs: call result type 'i32' does not match builtin return type 'f32'
    %2:i32 = abs 1.0f
             ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToBuiltin_MissingArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* c = b.Call(ty.f32(), BuiltinFn::kAbs, load->Result());
        c->SetOperands(Vector{static_cast<ir::Value*>(nullptr)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:5:18 error: abs: operand is undefined
    %4:f32 = abs undef
                 ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToBuiltin_OutOfScopeArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* c = b.Call(ty.f32(), BuiltinFn::kAbs, load->Result());
        auto* new_load = b.Load(i);
        c->SetOperands(Vector{new_load->Result()});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:5:18 error: abs: %5 is not in scope
    %4:f32 = abs %5
                 ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToBuiltin_TooFewResults) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* too_few_call = b.Call(ty.f32(), BuiltinFn::kAbs, load->Result());
        too_few_call->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:13 error: abs: expected exactly 1 results, got 0
    undef = abs %3
            ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToBuiltin_TooManyResults) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* too_many_call = b.Call(ty.f32(), BuiltinFn::kAbs, load->Result());
        too_many_call->SetResults(Vector{b.InstructionResult<f32>(), b.InstructionResult<f32>()});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:22 error: abs: expected exactly 1 results, got 2
    %4:f32, %5:f32 = abs %3
                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToBuiltin_TooFewArgs) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* too_few_call = b.Call(ty.f32(), BuiltinFn::kAbs, load->Result());
        too_few_call->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:14 error: abs: no matching call to 'abs()')"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, CallToBuiltin_TooManyArgs) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* too_many_call = b.Call(ty.f32(), BuiltinFn::kAbs, load->Result());
        too_many_call->SetOperands(Vector{load->Result(), load->Result()});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:14 error: abs: no matching call to 'abs(f32, f32)')"))
        << res.Failure();
}

// Test that a user declared structure cannot be used as the result type for a frexp builtin, even
// if it has the correct name and shape.
// See crbug.com/396344373.
TEST_F(IR_ValidatorTest, CallToBuiltin_Frexp_UserDeclaredResultStruct) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("__frexp_result_f32"), {
                                                             {mod.symbols.New("fract"), ty.f32()},
                                                             {mod.symbols.New("exp"), ty.i32()},
                                                         });

    auto* f = b.Function("f", ty.f32());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(str_ty, BuiltinFn::kFrexp, 1_f);
        b.Return(f, b.Access<f32>(c, 0_u));
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(error: frexp: call result type '__frexp_result_f32' does not match builtin return type '__frexp_result_f32'
    %2:__frexp_result_f32 = frexp 1.0f
                            ^^^^^
)")) << res.Failure();
}

// Test that a user declared structure cannot be used as the result type for a modf builtin, even
// if it has the correct name and shape.
// See crbug.com/396344373.
TEST_F(IR_ValidatorTest, CallToBuiltin_Modf_UserDeclaredResultStruct) {
    auto* str_ty = ty.Struct(mod.symbols.New("__modf_result_vec4_f32"),
                             {
                                 {mod.symbols.New("fract"), ty.vec4<f32>()},
                                 {mod.symbols.New("whole"), ty.vec4<f32>()},
                             });

    auto* f = b.Function("f", ty.vec4<f32>());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(str_ty, BuiltinFn::kModf, b.Splat<vec4<f32>>(1_f));
        b.Return(f, b.Access<vec4<f32>>(c, 0_u));
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(error: modf: call result type '__modf_result_vec4_f32' does not match builtin return type '__modf_result_vec4_f32'
    %2:__modf_result_vec4_f32 = modf vec4<f32>(1.0f)
                                ^^^^
)")) << res.Failure();
}

// Test that a user declared structure cannot be used as the result type for an
// atomicCompareExchangeWeak builtin, even if it has the correct name and shape.
// See crbug.com/396344373.
TEST_F(IR_ValidatorTest, CallToBuiltin_AtomicCompareExchange_UserDeclaredResultStruct) {
    auto* str_ty = ty.Struct(mod.symbols.New("__atomic_compare_exchange_result_u32"),
                             {
                                 {mod.symbols.New("old_value"), ty.u32()},
                                 {mod.symbols.New("exchanged"), ty.bool_()},
                             });

    auto* a = b.Var("a", ty.ptr<workgroup, atomic<u32>>());
    mod.root_block->Append(a);

    auto* f = b.Function("f", ty.u32());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(str_ty, BuiltinFn::kAtomicCompareExchangeWeak, a, 0_u, 1_u);
        b.Return(f, b.Access<u32>(c, 0_u));
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(error: atomicCompareExchangeWeak: call result type '__atomic_compare_exchange_result_u32' does not match builtin return type '__atomic_compare_exchange_result_u32'
    %3:__atomic_compare_exchange_result_u32 = atomicCompareExchangeWeak %a, 0u, 1u
                                              ^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

}  // namespace tint::core::ir
