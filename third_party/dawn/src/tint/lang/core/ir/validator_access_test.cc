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
#include <tuple>

#include "gtest/gtest.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, Access_NoOperands) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        auto* access = b.Access(ty.f32(), obj, 0_i);
        access->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:14 error: access: expected at least 2 operands, got 0
    %3:f32 = access
             ^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_NoIndices) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:14 error: access: expected at least 2 operands, got 1
    %3:f32 = access %2
             ^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_NoResults) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        auto* access = b.Access(ty.f32(), obj, 0_i);
        access->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:13 error: access: expected exactly 1 results, got 0
    undef = access %2, 0i
            ^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_NullObject) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), nullptr, 0_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:21 error: access: operand is undefined
    %2:f32 = access undef, 0u
                    ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:25 error: access: operand is undefined
    %3:f32 = access %2, undef
                        ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_NegativeIndex) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, -1_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:25 error: access: constant index must be positive, got -1
    %3:f32 = access %2, -1i
                        ^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_OOB_Index_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 3_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:29 error: access: index out of bounds for type 'vec2<f32>'
    %3:f32 = access %2, 1u, 3u
                            ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_OOB_Index_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u, 3_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:3:55 error: access: index out of bounds for type 'ptr<private, array<f32, 2>, read_write>'
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^

:2:3 note: in block
  $B1: {
  ^^^

:3:55 note: acceptable range: [0..1]
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_StaticallyUnindexableType_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.f32());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:25 error: access: type 'f32' cannot be indexed
    %3:f32 = access %2, 1u
                        ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_StaticallyUnindexableType_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:3:51 error: access: type 'ptr<private, f32, read_write>' cannot be indexed
    %3:ptr<private, f32, read_write> = access %2, 1u
                                                  ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_DynamicallyUnindexableType_Value) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.i32()},
                                                          });

    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(str_ty);
    auto* idx = b.FunctionParam(ty.i32());
    f->SetParams({obj, idx});

    b.Append(f->Block(), [&] {
        b.Access(ty.i32(), obj, idx);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:8:25 error: access: type 'MyStruct' cannot be dynamically indexed
    %4:i32 = access %2, %3
                        ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_DynamicallyUnindexableType_Ptr) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.i32()},
                                                          });

    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, read_write>(str_ty));
    auto* idx = b.FunctionParam(ty.i32());
    f->SetParams({obj, idx});

    b.Append(f->Block(), [&] {
        b.Access(ty.i32(), obj, idx);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:8:25 error: access: type 'ptr<private, MyStruct, read_write>' cannot be dynamically indexed
    %4:i32 = access %2, %3
                        ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Type_Value_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.i32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:3:14 error: access: result of access chain is type 'f32' but instruction type is 'i32'
    %3:i32 = access %2, 1u, 1u
             ^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Type_Ptr_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, i32>(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:3:40 error: access: result of access chain is type 'ptr<private, f32, read_write>' but instruction type is 'ptr<private, i32, read_write>'
    %3:ptr<private, i32, read_write> = access %2, 1u, 1u
                                       ^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Type_Ptr_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:3:14 error: access: result of access chain is type 'ptr<private, f32, read_write>' but instruction type is 'f32'
    %3:f32 = access %2, 1u, 1u
             ^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, vec3<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:25 error: access: cannot obtain address of vector element
    %3:f32 = access %2, 1u
                        ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr_WithCapability) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, vec3<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowVectorElementPointer});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr_ViaMatrixPtr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:29 error: access: cannot obtain address of vector element
    %3:f32 = access %2, 1u, 1u
                            ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr_ViaMatrixPtr_WithCapability) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowVectorElementPointer});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Ptr_AddressSpace) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<storage, array<f32, 2>, read>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<uniform, f32, read>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:3:34 error: access: result of access chain is type 'ptr<storage, f32, read>' but instruction type is 'ptr<uniform, f32, read>'
    %3:ptr<uniform, f32, read> = access %2, 1u
                                 ^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Ptr_Access) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<storage, array<f32, 2>, read>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<storage, f32, read_write>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:3:40 error: access: result of access chain is type 'ptr<storage, f32, read>' but instruction type is 'ptr<storage, f32, read_write>'
    %3:ptr<storage, f32, read_write> = access %2, 1u
                                       ^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Access_IndexVector) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Access_IndexVector_ViaMatrix) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Access_ExtractPointerFromStruct) {
    auto* ptr = ty.ptr<private_, i32>();
    Vector<core::type::Manager::StructMemberDesc, 1> members{
        core::type::Manager::StructMemberDesc{mod.symbols.New("a"), ptr},
    };
    auto* str = ty.Struct(mod.symbols.New("MyStruct"), std::move(members));
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam("obj", str);
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ptr, obj, 0_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowPointersAndHandlesInStructures});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Load_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::Load>(b.InstructionResult(ty.i32()), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:19 error: load: operand is undefined
    %2:i32 = load undef
                  ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Load_SourceNotMemoryView) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* let = b.Let("l", 1_i);
        b.Append(mod.CreateInstruction<ir::Load>(b.InstructionResult(ty.f32()), let->Result(0)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:4:19 error: load: load source operand 'i32' is not a memory view
    %3:f32 = load %l
                  ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Load_TypeMismatch) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.CreateInstruction<ir::Load>(b.InstructionResult(ty.f32()), var->Result(0)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:4:19 error: load: result type 'f32' does not match source store type 'i32'
    %3:f32 = load %2
                  ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Load_MissingResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* load = mod.CreateInstruction<ir::Load>(nullptr, var->Result(0));
        load->ClearResults();
        b.Append(load);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:4:13 error: load: expected exactly 1 results, got 0
    undef = load %2
            ^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Load_NonReadableSource) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32, core::Access::kWrite>());
        b.Append(mod.CreateInstruction<ir::Load>(b.InstructionResult(ty.i32()), var->Result(0)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:4:19 error: load: load source operand has a non-readable access type, 'write'
    %3:i32 = load %2
                  ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Store_NullTo) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::Store>(nullptr, b.Constant(42_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:11 error: store: operand is undefined
    store undef, 42i
          ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Store_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.CreateInstruction<ir::Store>(var->Result(0), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:4:15 error: store: operand is undefined
    store %2, undef
              ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Store_NonEmptyResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* store = mod.CreateInstruction<ir::Store>(var->Result(0), b.Constant(42_i));
        store->SetResults(Vector{b.InstructionResult(ty.i32())});
        b.Append(store);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:4:5 error: store: expected exactly 0 results, got 1
    store %2, 42i
    ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Store_TargetNotMemoryView) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* let = b.Let("l", 1_i);
        b.Append(mod.CreateInstruction<ir::Store>(let->Result(0), b.Constant(42_u)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:4:11 error: store: store target operand 'i32' is not a memory view
    store %l, 42u
          ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Store_TypeMismatch) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.CreateInstruction<ir::Store>(var->Result(0), b.Constant(42_u)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:4:15 error: store: value type 'u32' does not match store type 'i32'
    store %2, 42u
              ^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Store_NoStoreType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* result = b.InstructionResult(ty.u32());
        result->SetType(nullptr);
        b.Append(mod.CreateInstruction<ir::Store>(result, b.Constant(42_u)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:11 error: store: operand type is undefined
    store %2, 42u
          ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Store_NoValueType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* val = b.Construct(ty.u32(), 42_u);
        val->Result(0)->SetType(nullptr);

        b.Append(mod.CreateInstruction<ir::Store>(var->Result(0), val->Result(0)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:5:15 error: store: operand type is undefined
    store %2, %3
              ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Store_NonWriteableTarget) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32, core::Access::kRead>());
        b.Append(mod.CreateInstruction<ir::Store>(var->Result(0), b.Constant(42_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:4:11 error: store: store target operand has a non-writeable access type, 'read'
    store %2, 42i
          ^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, LoadVectorElement_NullResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(
            mod.CreateInstruction<ir::LoadVectorElement>(nullptr, var->Result(0), b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:4:5 error: load_vector_element: result is undefined
    undef = load_vector_element %2, 1i
    ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, LoadVectorElement_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::LoadVectorElement>(b.InstructionResult(ty.f32()),
                                                              nullptr, b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:34 error: load_vector_element: operand is undefined
    %2:f32 = load_vector_element undef, 1i
                                 ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, LoadVectorElement_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.CreateInstruction<ir::LoadVectorElement>(b.InstructionResult(ty.f32()),
                                                              var->Result(0), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:4:38 error: load_vector_element: operand is undefined
    %3:f32 = load_vector_element %2, undef
                                     ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, LoadVectorElement_MissingResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        auto* load = b.LoadVectorElement(var, b.Constant(1_i));
        load->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:4:13 error: load_vector_element: expected exactly 1 results, got 0
    undef = load_vector_element %2, 1i
            ^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, LoadVectorElement_MissingOperands) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        auto* load = b.LoadVectorElement(var, b.Constant(1_i));
        load->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:4:14 error: load_vector_element: expected exactly 2 operands, got 0
    %3:f32 = load_vector_element
             ^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, StoreVectorElement_NullTo) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::StoreVectorElement>(nullptr, b.Constant(1_i),
                                                               b.Constant(2_f)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:3:26 error: store_vector_element: operand is undefined
    store_vector_element undef, 1i, 2.0f
                         ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, StoreVectorElement_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.CreateInstruction<ir::StoreVectorElement>(var->Result(0), nullptr,
                                                               b.Constant(2_f)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:4:30 error: store_vector_element: operand is undefined
    store_vector_element %2, undef, 2.0f
                             ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, StoreVectorElement_NullValue) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.CreateInstruction<ir::StoreVectorElement>(var->Result(0), b.Constant(1_i),
                                                               nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:4:34 error: store_vector_element: operand is undefined
    store_vector_element %2, 1i, undef
                                 ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, StoreVectorElement_MissingOperands) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        auto* store = b.StoreVectorElement(var, b.Constant(1_i), b.Constant(2_f));
        store->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:4:5 error: store_vector_element: expected exactly 3 operands, got 0
    store_vector_element
    ^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, StoreVectorElement_UnexpectedResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        auto* store = b.StoreVectorElement(var, b.Constant(1_i), b.Constant(2_f));
        store->SetResults(Vector{b.InstructionResult(ty.f32())});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(R"(:4:5 error: store_vector_element: expected exactly 0 results, got 1
    store_vector_element %2, 1i, 2.0f
    ^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_MissingValue) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 2, 1, 0});
        swizzle->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:5:20 error: swizzle: expected exactly 1 operands, got 0
    %4:vec4<f32> = swizzle undef, wzyx
                   ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_NullValue) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 2, 1, 0});
        swizzle->SetOperand(0, nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(error: swizzle: operand is undefined
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_MissingResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 2, 1, 0});
        swizzle->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:5:13 error: swizzle: expected exactly 1 results, got 0
    undef = swizzle %3, wzyx
            ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_NullResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 2, 1, 0});
        swizzle->SetResults(Vector<ir::InstructionResult*, 1>{nullptr});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:5:5 error: swizzle: result is undefined
    undef = swizzle %3, wzyx
    ^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_NoIndices) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 2, 1, 0});
        auto indices = Vector<uint32_t, 0>();
        swizzle->SetIndices(std::move(indices));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:5:20 error: swizzle: expected at least 1 indices
    %4:vec4<f32> = swizzle %3,
                   ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_TooManyIndices) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 2, 1, 0});
        auto indices = Vector<uint32_t, 5>{1, 1, 1, 1, 1};
        swizzle->SetIndices(std::move(indices));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:5:20 error: swizzle: expected at most 4 indices
    %4:vec4<f32> = swizzle %3, yyyyy
                   ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_InvalidIndices) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 2, 1, 0});
        auto indices = Vector<uint32_t, 4>{4, 3, 2, 1};
        swizzle->SetIndices(std::move(indices));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:5:20 error: swizzle: invalid index value
    %4:vec4<f32> = swizzle %3, wzy
                   ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_NotVector) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.f32()));
        b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 2, 1, 0});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(
                    R"(:5:20 error: swizzle: object of swizzle, %3, is not a vector, 'f32'
    %4:vec4<f32> = swizzle %3, wzyx
                   ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_TooSmallResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        b.Swizzle(ty.vec2<f32>(), b.Load(var), {3, 2, 1, 0});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:5:20 error: swizzle: result type 'vec2<f32>' does not match expected type, 'vec4<f32>'
    %4:vec2<f32> = swizzle %3, wzyx
                   ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_TooLargeResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 2});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(:5:20 error: swizzle: result type 'vec4<f32>' does not match expected type, 'vec2<f32>'
    %4:vec4<f32> = swizzle %3, wz
                   ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_WrongTypeResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        b.Swizzle(ty.vec2<u32>(), b.Load(var), {3, 2});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason.Str(),
        testing::HasSubstr(
            R"(5:20 error: swizzle: result type 'vec2<u32>' does not match expected type, 'vec2<f32>'
    %4:vec2<u32> = swizzle %3, wz
                   ^^^^^^^
)")) << res.Failure().reason.Str();
}

TEST_F(IR_ValidatorTest, Swizzle_OOBIndex) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec2<f32>()));
        b.Swizzle(ty.vec4<f32>(), b.Load(var), {3, 1, 1, 0});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr(R"(:5:20 error: swizzle: invalid index value
    %4:vec4<f32> = swizzle %3, wyyx
                   ^^^^^^^
)")) << res.Failure().reason.Str();
}

}  // namespace tint::core::ir
