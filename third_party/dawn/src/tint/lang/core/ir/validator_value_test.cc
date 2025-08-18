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

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, Var_RootBlock_NullOperand) {
    auto* v = b.Var(ty.ptr(private_, ty.i32()));
    v->SetOperands(Vector{static_cast<ir::Value*>(nullptr)});
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Var_RootBlock_TooManyOperands) {
    auto* v = b.Var(ty.ptr(private_, ty.i32()));
    v->SetOperands(Vector{b.Value(0_i), b.Value(1_i)});
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:38 error: var: expected exactly 1 operands, got 2
  %1:ptr<private, i32, read_write> = var 0i, 1i
                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_RootBlock_NullResult) {
    auto* v = mod.CreateInstruction<ir::Var>(nullptr);
    v->SetInitializer(b.Constant(0_i));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:2:3 error: var: result is undefined
  undef = var 0i
  ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_RootBlock_TooManyResults) {
    auto* v = b.Var(ty.ptr(private_, ty.i32()));
    v->SetInitializer(b.Constant(0_i));
    v->SetResults(Vector{b.InstructionResult<i32>(), b.InstructionResult<i32>()});
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:20 error: var: expected exactly 1 results, got 2
  %1:i32, %2:i32 = var 0i
                   ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_VoidType) {
    mod.root_block->Append(b.Var(ty.ptr(private_, ty.void_())));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:3 error: var: pointers to void are not permitted
  %1:ptr<private, void, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Function_NullResult) {
    auto* v = mod.CreateInstruction<ir::Var>(nullptr);
    v->SetInitializer(b.Constant(0_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:5 error: var: result is undefined
    undef = var 0i
    ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Function_NoResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInitializer(b.Constant(1_i));
        v->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:13 error: var: expected exactly 1 results, got 0
    undef = var 1i
            ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Function_TooManyResults) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, i32>();
        v->SetInitializer(b.Constant(0_i));
        v->SetResults(Vector{b.InstructionResult<i32>(), b.InstructionResult<i32>()});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:22 error: var: expected exactly 1 results, got 2
    %2:i32, %3:i32 = var 0i
                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Function_NonPtrResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->Result()->SetType(ty.f32());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:14 error: var: result type 'f32' must be a pointer or a reference
    %2:f32 = var undef
             ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Function_UnexpectedInputAttachmentIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInputAttachmentIndex(0);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:41 error: var: '@input_attachment_index' is not valid for non-handle var
    %2:ptr<function, f32, read_write> = var undef @input_attachment_index(0)
                                        ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Function_OutsideFunctionScope) {
    auto* v = b.Var<function, f32>();
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:39 error: var: vars in the 'function' address space must be in a function scope
  %1:ptr<function, f32, read_write> = var undef
                                      ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_NonFunction_InsideFunctionScope) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Var<private_, f32>();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:40 error: var: vars in a function scope must be in the 'function' address space
    %2:ptr<private, f32, read_write> = var undef
                                       ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Private_InsideFunctionScopeWithCapability) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Var<private_, f32>();
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowPrivateVarsInFunctions});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Private_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<private_, f32>();
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:38 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<private, f32, read_write> = var undef @input_attachment_index(0)
                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Immediate_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<immediate, f32>();
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:34 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<immediate, f32, read> = var undef @input_attachment_index(0)
                                 ^^^

)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Storage_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<storage, f32>();
    v->SetBindingPoint(0, 0);
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:38 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<storage, f32, read_write> = var undef @binding_point(0, 0) @input_attachment_index(0)
                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Uniform_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<uniform, f32>();
    v->SetBindingPoint(0, 0);
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:32 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<uniform, f32, read> = var undef @binding_point(0, 0) @input_attachment_index(0)
                               ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Workgroup_UnexpectedInputAttachmentIndex) {
    auto* v = b.Var<workgroup, f32>();
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:40 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<workgroup, f32, read_write> = var undef @input_attachment_index(0)
                                       ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Init_WrongType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInitializer(b.Constant(1_i));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:41 error: var: initializer type 'i32' does not match store type 'f32'
    %2:ptr<function, f32, read_write> = var 1i
                                        ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Init_NullType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* load_ret = load->Result();
        auto* j = b.Var<function, f32>("j");
        j->SetInitializer(load_ret);
        load_ret->SetType(nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:5:45 error: var: operand type is undefined
    %j:ptr<function, f32, read_write> = var %3
                                            ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Init_FunctionTypeInit) {
    auto* invalid = b.Function("invalid_init", ty.void_());
    b.Append(invalid->Block(), [&] { b.Return(invalid); });
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(invalid);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:8:41 error: var: initializer type '<function>' does not match store type 'f32'
    %i:ptr<function, f32, read_write> = var %invalid_init
                                        ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Init_InvalidAddressSpace) {
    auto* p = b.Var<private_, f32>("p");
    p->SetInitializer(b.Constant(1_f));
    mod.root_block->Append(p);
    auto* s = b.Var<storage, f32>("s");
    s->SetInitializer(b.Constant(1_f));
    mod.root_block->Append(s);
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>("v");
        v->SetInitializer(b.Constant(1_f));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:38 error: var: only variables in the function, private, or __out address space may be initialized
  %s:ptr<storage, f32, read_write> = var 1.0f
                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_HandleMissingBindingPoint) {
    auto* v = b.Var(ty.ptr<handle, i32>());
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:31 error: var: a resource variable is missing binding point
  %1:ptr<handle, i32, read> = var undef
                              ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_StorageMissingBindingPoint) {
    auto* v = b.Var(ty.ptr<storage, i32>());
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:38 error: var: a resource variable is missing binding point
  %1:ptr<storage, i32, read_write> = var undef
                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_UniformMissingBindingPoint) {
    auto* v = b.Var(ty.ptr<uniform, i32>());
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:32 error: var: a resource variable is missing binding point
  %1:ptr<uniform, i32, read> = var undef
                               ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_NonResourceWithBindingPoint) {
    auto* v = b.Var(ty.ptr<private_, i32>());
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:38 error: var: a non-resource variable has binding point
  %1:ptr<private, i32, read_write> = var undef @binding_point(0, 0)
                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Uniform_NotConstructible) {
    auto* v = b.Var<uniform, atomic<u32>>();
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:40 error: var: vars in the 'uniform' address space must be host-shareable and constructible
  %1:ptr<uniform, atomic<u32>, read> = var undef @binding_point(0, 0)
                                       ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Uniform_NotHostShareable) {
    auto* v = b.Var<uniform, bool>();
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:33 error: var: vars in the 'uniform' address space must be host-shareable and constructible
  %1:ptr<uniform, bool, read> = var undef @binding_point(0, 0)
                                ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Storage_NotHostShareable) {
    auto* v = b.Var<storage, bool, read>();
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:33 error: var: vars in the 'storage' address space must be host-shareable
  %1:ptr<storage, bool, read> = var undef @binding_point(0, 0)
                                ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_DuplicateBindingPoints_NeitherReferenced) {
    auto* var_a = b.Var<uniform, f32>();
    var_a->SetBindingPoint(1, 2);
    mod.root_block->Append(var_a);

    auto* var_b = b.Var<uniform, i32>();
    var_b->SetBindingPoint(1, 2);
    mod.root_block->Append(var_b);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Var_DuplicateBindingPoints_OnlyOneReferenced) {
    auto* var_a = b.Var<uniform, f32>();
    var_a->SetBindingPoint(1, 2);
    mod.root_block->Append(var_a);

    auto* var_b = b.Var<uniform, i32>();
    var_b->SetBindingPoint(1, 2);
    mod.root_block->Append(var_b);

    auto* f = FragmentEntryPoint();
    b.Append(f->Block(), [&] {
        b.Let(var_a->Result());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Var_DuplicateBindingPoints_ReferencedInDifferentFunctions) {
    auto* var_a = b.Var<uniform, f32>();
    var_a->SetBindingPoint(1, 2);
    mod.root_block->Append(var_a);

    auto* var_b = b.Var<uniform, i32>();
    var_b->SetBindingPoint(1, 2);
    mod.root_block->Append(var_b);

    auto* func_a = FragmentEntryPoint("func_a");
    b.Append(func_a->Block(), [&] {
        b.Let(var_a->Result());
        b.Return(func_a);
    });

    auto* func_b = FragmentEntryPoint("func_b");
    b.Append(func_b->Block(), [&] {
        b.Let(var_a->Result());
        b.Return(func_b);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowMultipleEntryPoints});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Var_DuplicateBindingPoints_BothReferenced) {
    auto* var_a = b.Var<uniform, f32>();
    var_a->SetBindingPoint(1, 2);
    mod.root_block->Append(var_a);

    auto* var_b = b.Var<uniform, i32>();
    var_b->SetBindingPoint(1, 2);
    mod.root_block->Append(var_b);

    auto* f = FragmentEntryPoint();
    b.Append(f->Block(), [&] {
        b.Let(var_a->Result());
        b.Let(var_b->Result());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:32 error: var: found non-unique binding point, [group: 1, binding: 2], being referenced in entry point, %f
  %2:ptr<uniform, i32, read> = var undef @binding_point(1, 2)
                               ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_DuplicateBindingPoints_CapabilityOverride) {
    auto* var_a = b.Var<uniform, f32>();
    var_a->SetBindingPoint(1, 2);
    mod.root_block->Append(var_a);

    auto* var_b = b.Var<uniform, i32>();
    var_b->SetBindingPoint(1, 2);
    mod.root_block->Append(var_b);

    auto* f = FragmentEntryPoint();
    b.Append(f->Block(), [&] {
        b.Let(var_a->Result());
        b.Let(var_b->Result());
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowDuplicateBindings});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Var_MultipleIOAnnotations) {
    auto* v = b.Var<AddressSpace::kIn, vec4<f32>>();
    v->SetBuiltin(BuiltinValue::kPosition);
    v->SetLocation(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:35 error: var: module scope variable has more than one IO annotation, [ @location, built-in ]
  %1:ptr<__in, vec4<f32>, read> = var undef @location(0) @builtin(position)
                                  ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Struct_MultipleIOAnnotations) {
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.color = 0;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr},
                                               });
    auto* v = b.Var(ty.ptr(AddressSpace::kOut, str_ty, read_write));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:41 error: var: module scope variable struct member has more than one IO annotation, [ built-in, @color ]
  %1:ptr<__out, MyStruct, read_write> = var undef
                                        ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_MissingIOAnnotations) {
    auto* v = b.Var<AddressSpace::kIn, vec4<f32>>();
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:35 error: var: module scope variable must have at least one IO annotation, e.g. a binding point, a location, etc
  %1:ptr<__in, vec4<f32>, read> = var undef
                                  ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Struct_MissingIOAnnotations) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.f32(), {}},
                                                          });
    auto* v = b.Var(ty.ptr(AddressSpace::kOut, str_ty, read_write));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:41 error: var: module scope variable struct members must have at least one IO annotation, e.g. a binding point, a location, etc
  %1:ptr<__out, MyStruct, read_write> = var undef
                                        ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Sampler_NonHandleAddressSpace) {
    auto* v = b.Var(ty.ptr(AddressSpace::kPrivate, ty.sampler(), read_write));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:3 error: var: handle types can only be declared in the 'handle' address space
  %1:ptr<private, sampler, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_Texture_NonHandleAddressSpace) {
    auto* v = b.Var(ty.ptr(AddressSpace::kPrivate, ty.sampler(), read_write));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:3 error: var: handle types can only be declared in the 'handle' address space
  %1:ptr<private, sampler, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_BindingArray_Texture_NonHandleAddressSpace) {
    auto* v = b.Var(ty.ptr(
        AddressSpace::kPrivate,
        ty.binding_array(ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u),
        read));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:3 error: var: handle types can only be declared in the 'handle' address space
  %1:ptr<private, binding_array<texture_2d<f32>, 4>, read> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_InputAttachementIndex_NonHandle) {
    auto* v = b.Var(ty.ptr(AddressSpace::kPrivate, ty.f32(), read_write));
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:38 error: var: '@input_attachment_index' is not valid for non-handle var
  %1:ptr<private, f32, read_write> = var undef @input_attachment_index(0)
                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_InputAttachementIndex_WrongType) {
    auto* v = b.Var(ty.ptr(AddressSpace::kHandle, ty.f32(), read_write));
    v->SetBindingPoint(0, 0);
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:37 error: var: '@input_attachment_index' is only valid for 'input_attachment' type var
  %1:ptr<handle, f32, read_write> = var undef @binding_point(0, 0) @input_attachment_index(0)
                                    ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_RuntimeArray_NonStorage) {
    auto* v = b.Var(ty.ptr(AddressSpace::kPrivate, ty.runtime_array(ty.f32()), read_write));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:3 error: var: runtime arrays must be in the 'storage' address space
  %1:ptr<private, array<f32>, read_write> = var undef
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Var_RuntimeArray_NonStorageInStruct) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"),
                             {
                                 {mod.symbols.New("a"), ty.runtime_array(ty.f32()), {}},
                             });
    auto* v = b.Var(ty.ptr(AddressSpace::kUniform, str_ty, read));
    v->SetBindingPoint(0, 0);
    v->SetInputAttachmentIndex(0);
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:3 error: var: runtime arrays must be in the 'storage' address space
  %1:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0) @input_attachment_index(0)
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_NullResult) {
    auto* v = mod.CreateInstruction<ir::Let>(nullptr, b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:5 error: let: result is undefined
    undef = let 1i
    ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_EmptyResults) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), b.Constant(1_i));
    v->ClearResults();

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:13 error: let: expected exactly 1 results, got 0
    undef = let 1i
            ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_NullValue) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.f32()), nullptr);
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:18 error: let: operand is undefined
    %2:f32 = let undef
                 ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_EmptyValue) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), b.Constant(1_i));
    v->ClearOperands();

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:14 error: let: expected exactly 1 operands, got 0
    %2:i32 = let
             ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_WrongType) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.f32()), b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:14 error: let: result type 'f32' does not match value type 'i32'
    %2:f32 = let 1i
             ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_VoidResultWithCapability) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* l = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.void_()), b.Constant(1_i));
        b.Append(l);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{ir::Capability::kAllowAnyLetType});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:3:15 error: let: result type cannot be void
    %2:void = let 1i
              ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_VoidResultWithoutCapability) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* l = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.void_()), b.Constant(1_i));
        b.Append(l);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:15 error: let: result type, 'void', must be concrete constructible type or a pointer type
    %2:void = let 1i
              ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_VoidValueWithCapability) {
    auto* v = b.Function("void_func", ty.void_());
    b.Append(v->Block(), [&] { b.Return(v); });

    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* l = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), b.Value(b.Call(v)));
        b.Append(l);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{ir::Capability::kAllowAnyLetType});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:9:14 error: let: value type cannot be void
    %4:i32 = let %3
             ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_VoidValueWithoutCapability) {
    auto* v = b.Function("void_func", ty.void_());
    b.Append(v->Block(), [&] { b.Return(v); });

    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* l = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), b.Value(b.Call(v)));
        b.Append(l);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:9:14 error: let: value type, 'void', must be concrete constructible type or a pointer type
    %4:i32 = let %3
             ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_NotConstructibleResult) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("p", ty.sampler());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        auto* l =
            mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.sampler()), b.Constant(1_i));
        b.Append(l);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:18 error: let: result type, 'sampler', must be concrete constructible type or a pointer type
    %3:sampler = let 1i
                 ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_NotConstructibleValue) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("p", ty.sampler());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        auto* l = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), p);
        b.Append(l);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:14 error: let: value type, 'sampler', must be concrete constructible type or a pointer type
    %3:i32 = let %p
             ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Let_CapabilityBypass) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("p", ty.sampler());
    f->AppendParam(p);

    b.Append(f->Block(), [&] {
        auto* l = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.sampler()), p);
        b.Append(l);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{ir::Capability::kAllowAnyLetType});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Phony_NullValue) {
    auto* v = mod.CreateInstruction<ir::Phony>(nullptr);
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod, Capabilities{ir::Capability::kAllowPhonyInstructions});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:19 error: phony: operand is undefined
    undef = phony undef
                  ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Phony_EmptyValue) {
    auto* v = mod.CreateInstruction<ir::Phony>(b.Constant(1_i));
    v->ClearOperands();

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod, Capabilities{ir::Capability::kAllowPhonyInstructions});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:13 error: phony: expected exactly 1 operands, got 0
    undef = phony
            ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Phony_MissingCapability) {
    auto* v = mod.CreateInstruction<ir::Phony>(b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:13 error: phony: missing capability 'kAllowPhonyInstructions'
    undef = phony 1i
            ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, PointerToNonStructPixelLocal) {
    auto* var = b.Var("var", ty.ptr<core::AddressSpace::kPixelLocal>(ty.i32()));
    mod.root_block->Append(var);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr("pixel_local var must be of type struct"))
        << res.Failure();
}

}  // namespace tint::core::ir
