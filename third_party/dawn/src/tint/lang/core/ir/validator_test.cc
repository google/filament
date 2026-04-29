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

#include "src/tint/lang/core/ir/validator_test.h"

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/text/string.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_ValidatorDeathTest = IR_ValidatorTest;

Function* IR_ValidatorTest::ComputeEntryPoint(const std::string& name) {
    return b.ComputeFunction(name);
}

Function* IR_ValidatorTest::FragmentEntryPoint(const std::string& name) {
    return b.Function(name, ty.void_(), Function::PipelineStage::kFragment);
}

Function* IR_ValidatorTest::VertexEntryPoint(const std::string& name) {
    auto* f = b.Function(name, ty.vec4f(), Function::PipelineStage::kVertex);
    f->SetReturnBuiltin(BuiltinValue::kPosition);
    return f;
}

void IR_ValidatorTest::AddBuiltinParam(Function* func,
                                       const std::string& name,
                                       BuiltinValue builtin,
                                       const core::type::Type* type) {
    auto* p = b.FunctionParam(name, type);
    p->SetBuiltin(builtin);
    func->AppendParam(p);
}

void IR_ValidatorTest::AddReturn(Function* func,
                                 const std::string& name,
                                 const core::type::Type* type,
                                 const IOAttributes& attr) {
    TINT_ASSERT(!func->ReturnType()->Is<core::type::Struct>())
        << "AddReturn does not support adding to structured returns";

    if (func->ReturnType() == ty.void_()) {
        func->SetReturnAttributes(attr);
        func->SetReturnType(type);
        return;
    }

    std::string old_name =
        func->ReturnAttributes().builtin == BuiltinValue::kPosition ? "pos" : "old_ret";
    auto* str_ty =
        ty.Struct(mod.symbols.New("OutputStruct"),
                  {
                      {mod.symbols.New(old_name), func->ReturnType(), func->ReturnAttributes()},
                      {mod.symbols.New(name), type, attr},
                  });

    func->SetReturnAttributes({});
    func->SetReturnType(str_ty);
}

void IR_ValidatorTest::AddBuiltinReturn(Function* func,
                                        const std::string& name,
                                        BuiltinValue builtin,
                                        const core::type::Type* type) {
    IOAttributes attr;
    attr.builtin = builtin;
    AddReturn(func, name, type, attr);
}

TEST_F(IR_ValidatorTest, RootBlock_Var) {
    mod.root_block->Append(b.Var(ty.ptr<private_, i32>()));
    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, RootBlock_NonVar) {
    auto* l = b.Loop();
    l->Body()->Append(b.Continue(l));

    mod.root_block->Append(l);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:3 error: loop: root block: invalid instruction: tint::core::ir::Loop
  loop [b: $B2] {  # loop_1
  ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, RootBlock_Let) {
    mod.root_block->Append(b.Let("a", 1_f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:2:12 error: let: root block: invalid instruction: tint::core::ir::Let
  %a:f32 = let 1.0f
           ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, RootBlock_LetWithAllowModuleScopeLets) {
    mod.root_block->Append(b.Let("a", 1_f));

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowModuleScopeLets});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, RootBlock_Construct) {
    mod.root_block->Append(b.Construct(ty.vec2f(), 1_f, 2_f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:18 error: construct: root block: invalid instruction: tint::core::ir::Construct
  %1:vec2<f32> = construct 1.0f, 2.0f
                 ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, RootBlock_ConstructWithAllowModuleScopeLets) {
    mod.root_block->Append(b.Construct(ty.vec2f(), 1_f, 2_f));

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowModuleScopeLets});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, RootBlock_VarBlockMismatch) {
    auto* var = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(var);

    auto* f = b.Function("f", ty.void_());
    f->Block()->Append(b.Return(f));
    var->SetBlock(f->Block());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:38 error: var: instruction in root block does not have root block as parent
  %1:ptr<private, i32, read_write> = var undef
                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, RootBlock_ModuleScopeRuntimeExpression) {
    auto* v = b.Var("v", ty.ptr(workgroup, ty.atomic(ty.u32())));
    mod.root_block->Append(v);

    auto* load = b.Call(ty.u32(), core::BuiltinFn::kAtomicLoad, v->Result(0));
    mod.root_block->Append(load);

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            +R"(:3:12 error: atomicLoad: instruction is not evaluatable at pipeline creation time
  %2:u32 = atomicLoad %v
           ^^^^^^^^^^)"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, RootBlock_VarWithRuntimeInitializer) {
    auto* v = b.Var("v", ty.ptr(workgroup, ty.atomic(ty.u32())));
    mod.root_block->Append(v);

    // This will also cause the same error on atomicLoad as above, but this test is interested in
    // the later error for the var.
    auto* init = b.Call(ty.u32(), core::BuiltinFn::kAtomicLoad, v);
    mod.root_block->Append(init);
    mod.root_block->Append(b.Var("a", init));

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:4:39 error: var: instruction is not evaluatable at pipeline creation time
  %a:ptr<function, u32, read_write> = var %2
                                      ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Scalar_WrongArgType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct<u32>(42_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:24 error: construct: scalar construct argument type 'i32' does not match result type 'u32'
    %2:u32 = construct 42i
                       ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Scalar_TooManyArguments) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct<u32>(42_u, 10_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:14 error: construct: scalar construct must not have more than one argument
    %2:u32 = construct 42u, 10u
             ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_SubgroupMatrix_WrongArgType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(ty.subgroup_matrix_left(ty.f32(), 2, 3), f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:42 error: construct: subgroup matrix construct argument type '<function>' does not match matrix shader scalar type 'f32'
    %2:subgroup_matrix_left<f32, 2, 3> = construct %f
                                         ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_SubgroupMatrix_TooManyArguments) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(ty.subgroup_matrix_left(ty.f32(), 2, 3), 42_f, 43_f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:42 error: construct: subgroup matrix construct must not have more than 1 argument
    %2:subgroup_matrix_left<f32, 2, 3> = construct 42.0f, 43.0f
                                         ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Array_WrongArgType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct<array<u32, 4>>(1_u, 2_u, 3_i, 4_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:42 error: construct: type 'i32' of argument 2 does not match expected type 'u32'
    %2:array<u32, 4> = construct 1u, 2u, 3i, 4u
                                         ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Array_WrongArgCount) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct<array<u32, 4>>(1_u, 2_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:24 error: construct: array has 4 elements, but construct provides 2 arguments
    %2:array<u32, 4> = construct 1u, 2u
                       ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Vector_1arg_WrongType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct<vec2u>(b.Zero<vec2f>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(error: construct: no matching overload for vec2<u32> constructor
    %2:vec2<u32> = construct vec2<f32>(0.0f)
                   ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Vector_2arg_WrongType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct<vec2u>(b.Zero<f32>(), b.Zero<f32>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(error: construct: no matching overload for vec2<u32> constructor
    %2:vec2<u32> = construct 0.0f, 0.0f
                   ^^^^^^^^^
)"));
}

TEST_F(IR_ValidatorTest, Construct_Matrix_NoArgs) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(ty.mat2x2<f32>());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Matrix_Scalar) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(ty.mat2x2<f32>(), 1_f, 2_f, 3_f, 4_f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Matrix_ColumnVectors) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* v1 = b.Composite(ty.vec2f(), 1_f, 2_f);
        auto* v2 = b.Composite(ty.vec2f(), 3_f, 4_f);
        b.Construct(ty.mat2x2<f32>(), v1, v2);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Matrix_MixedScalarVector) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(ty.mat2x2<f32>(), 1_f, b.Composite(ty.vec2f(), 2_f, 3_f));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr("error: construct: no matching overload for mat2x2<f32> constructor"));
}

TEST_F(IR_ValidatorTest, Construct_Matrix_Scalar_WrongType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(ty.mat2x2<f32>(), 1_f, 2_f, 3_h, 4_f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr("error: construct: no matching overload for mat2x2<f32> constructor"));
}

TEST_F(IR_ValidatorTest, Construct_Struct_ZeroValue) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Struct_ValidArgs) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, 2_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Struct_UnusedArgs) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, b.Unused());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Struct_NotEnoughArgs) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:8:19 error: construct: structure has 2 members, but construct provides 1 arguments
    %2:MyStruct = construct 1i
                  ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Struct_TooManyArgs) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, 2_u, 3_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:8:19 error: construct: structure has 2 members, but construct provides 3 arguments
    %2:MyStruct = construct 1i, 2u, 3i
                  ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Struct_WrongArgType) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, 2_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:8:33 error: construct: type 'i32' of argument 1 does not match expected type 'u32'
    %2:MyStruct = construct 1i, 2i
                                ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_NullArg) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:33 error: construct: operand is undefined
    %2:MyStruct = construct 1i, undef
                                ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_NullResult) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Construct(str_ty, 1_i, 2_u);
        c->SetResult(nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:5 error: construct: result is undefined
    undef = construct 1i, 2u
    ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_EmptyResult) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Construct(str_ty, 1_i, 2_u);
        c->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:8:13 error: construct: expected exactly 1 results, got 0
    undef = construct 1i, 2u
            ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Texture) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:3:26 error: construct: type is not constructible
    %2:texture_2d<f32> = construct
                         ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_TextureInStruct_WithCapability) {
    auto* tex_ty = ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), tex_ty},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kMslAllowEntryPointInterface});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_NonConstructible_WithStructCapability) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(ty.void_());
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kMslAllowEntryPointInterface});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:3:15 error: construct: type is not constructible
    %2:void = construct
              ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_MissingArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Convert(ty.i32(), 1_f);
        c->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:14 error: convert: expected exactly 1 operands, got 0
    %2:i32 = convert
             ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_NullArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Convert(ty.i32(), nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:22 error: convert: operand is undefined
    %2:i32 = convert undef
                     ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_MissingResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Convert(ty.i32(), 1_f);
        c->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:13 error: convert: expected exactly 1 results, got 0
    undef = convert 1.0f
            ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_NullResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Convert(ty.i32(), 1_f);
        c->SetResult(nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:5 error: convert: result is undefined
    undef = convert 1.0f
    ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_ScalarToScalar) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.f32());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.i32(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_ScalarIdentity) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.f32());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.f32(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:14 error: convert: No defined converter for 'f32' -> 'f32'
    %3:f32 = convert %p
             ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_ScalarToVec) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.f32());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.vec2f(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:20 error: convert: No defined converter for 'f32' -> 'vec2<f32>'
    %3:vec2<f32> = convert %p
                   ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_ScalarToMat) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.f32());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.mat2x3<f32>(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:22 error: convert: No defined converter for 'f32' -> 'mat2x3<f32>'
    %3:mat2x3<f32> = convert %p
                     ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_VecToVec) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.vec2u());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.vec2f(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_VecIdentity) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.vec3f());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.vec3f(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:20 error: convert: No defined converter for 'vec3<f32>' -> 'vec3<f32>'
    %3:vec3<f32> = convert %p
                   ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_VecToVec_WidthMismatch) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.vec2u());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.vec4f(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:20 error: convert: No defined converter for 'vec2<u32>' -> 'vec4<f32>'
    %3:vec4<f32> = convert %p
                   ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_VecToScalar) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.vec2u());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.f32(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:14 error: convert: No defined converter for 'vec2<u32>' -> 'f32'
    %3:f32 = convert %p
             ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_VecToMat) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.vec2u());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.mat3x2<f32>(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:22 error: convert: No defined converter for 'vec2<u32>' -> 'mat3x2<f32>'
    %3:mat3x2<f32> = convert %p
                     ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_MatToMat) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.mat4x4<f32>());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.mat4x4<f16>(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_MatToMatIdentity) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.mat3x3<f32>());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.mat3x3<f32>(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:22 error: convert: No defined converter for 'mat3x3<f32>' -> 'mat3x3<f32>'
    %3:mat3x3<f32> = convert %p
                     ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_MatToMat_ShapeMismatch) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.mat4x4<f32>());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.mat2x2<f32>(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:22 error: convert: No defined converter for 'mat4x4<f32>' -> 'mat2x2<f32>'
    %3:mat2x2<f32> = convert %p
                     ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_MatToScalar) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.mat4x4<f32>());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.f32(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:14 error: convert: No defined converter for 'mat4x4<f32>' -> 'f32'
    %3:f32 = convert %p
             ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_MatToVec) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.mat4x4<f32>());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.vec4f(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:20 error: convert: No defined converter for 'mat4x4<f32>' -> 'vec4<f32>'
    %3:vec4<f32> = convert %p
                   ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_4xU8ToU32) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.vec4<u8>());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.u32(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllow8BitIntegers});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:14 error: convert: No defined converter for 'vec4<u8>' -> 'u32'
    %3:u32 = convert %p
             ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_U32To4xU8) {
    auto* f = b.Function("f", ty.void_());
    auto* p = b.FunctionParam("p", ty.u32());
    f->AppendParam(p);
    b.Append(f->Block(), [&] {
        b.Convert(ty.vec4<u8>(), p);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllow8BitIntegers});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:19 error: convert: No defined converter for 'u32' -> 'vec4<u8>'
    %3:vec4<u8> = convert %p
                  ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_PtrToVal) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", 0_u);
        b.Convert(ty.u32(), v->Result());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:4:14 error: convert: No defined converter for 'ptr<function, u32, read_write>' -> 'u32'
    %3:u32 = convert %v
             ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Convert_PtrToPtr) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* v = b.Var("v", 0_u);
        b.Convert(v->Result()->Type(), v->Result());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:4:41 error: convert: not defined for result type, 'ptr<function, u32, read_write>'
    %3:ptr<function, u32, read_write> = convert %v
                                        ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Block_NoTerminator) {
    b.Function("my_func", ty.void_());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:3 error: block does not end in a terminator instruction
  $B1: {
  ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Block_VarBlockMismatch) {
    auto* var = b.Var(ty.ptr<function, i32>());

    auto* f = b.Function("f", ty.void_());
    f->Block()->Append(var);
    f->Block()->Append(b.Return(f));

    auto* g = b.Function("g", ty.void_());
    g->Block()->Append(b.Return(g));

    var->SetBlock(g->Block());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:41 error: var: block instruction does not have same block as parent
    %2:ptr<function, i32, read_write> = var undef
                                        ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Block_DeadParameter) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.f32());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] { b.NextIteration(l, nullptr); });
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Return(f);
    });

    p->Destroy();

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:7:12 error: destroyed parameter found in block parameter list
      $B3 (%my_param:f32): {  # body
           ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Block_ParameterWithNullBlock) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.f32());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] { b.NextIteration(l, nullptr); });
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Return(f);
    });

    p->SetBlock(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:7:12 error: block parameter has nullptr parent block
      $B3 (%my_param:f32): {  # body
           ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Block_ParameterUsedInMultipleBlocks) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.f32());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] { b.NextIteration(l, nullptr); });
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.Continue(l, p); });
        l->Continuing()->SetParams({p});
        b.Append(l->Continuing(), [&] { b.NextIteration(l, p); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:7:12 error: block parameter has incorrect parent block
      $B3 (%my_param:f32): {  # body
           ^^^^^^^^^

:10:7 note: parent block declared here
      $B4 (%my_param:f32): {  # continuing
      ^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Block_Parameter_Void) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.void_());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] { b.NextIteration(l, nullptr); });
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.Continue(l); });
        b.Append(l->Continuing(), [&] { b.NextIteration(l, p); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:7:12 error: block parameter type cannot be void
      $B3 (%my_param:void): {  # body
           ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Instruction_AppendedDead) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    auto* ret = sb.Return(f);

    v->Destroy();
    v->InsertBefore(ret);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:3:5 error: var: destroyed instruction found in instruction list
    <destroyed tint::core::ir::Var)"));
}

TEST_F(IR_ValidatorTest, Instruction_NullInstructionResultInstruction) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    v->Result()->SetInstruction(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: var: result instruction is undefined
    %2:ptr<function, f32, read_write> = var undef
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Instruction_DuplicateResultOneCall) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        auto* r1 = b.InstructionResult(ty.u32());
        l->SetResults(Vector{r1, r1});
        b.Append(l->Body(), [&] { b.Unreachable(); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:13 error: loop: result was seen previously as a result
    %2:u32, %2:u32 = loop [b: $B2] {  # loop_1
            ^^^^^^)"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, Instruction_WrongInstructionResultInstruction) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    auto* v2 = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    v->SetResult(v2->Result());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:4:5 error: var: result instruction does not match instruction (possible double usage)
    %2:ptr<function, f32, read_write> = var undef
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Instruction_TooManyResultInstruction) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    v->SetResults(Vector{v->Results()[0], v->Results()[0]});
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:3:76 error: var: expected exactly 1 results, got 2
    %2:ptr<function, f32, read_write>, %2:ptr<function, f32, read_write> = var undef
                                                                           ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Instruction_VoidResultWithName) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* result = b.Call<void>(core::BuiltinFn::kWorkgroupBarrier);
        mod.SetName(result, "result");
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: workgroupBarrier: void results must not have names
    %result:void = workgroupBarrier
    ^^^^^^^^^^^^)"))
        << res.Failure();
}

TEST_F(IR_ValidatorTest, Instruction_DeadOperand) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.f32());
    result->Destroy();
    v->SetInitializer(result);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:45 error: var: operand is not alive
    %2:ptr<function, f32, read_write> = var %3
                                            ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Instruction_OperandUsageRemoved) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.f32());
    v->SetInitializer(result);
    result->RemoveUsage({v, 0u});

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:45 error: var: operand missing usage
    %2:ptr<function, f32, read_write> = var %3
                                            ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Instruction_OrphanedInstruction) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    auto* load = sb.Load(v);
    sb.Return(f);

    load->Remove();

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(error: load: orphaned instruction: load
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Binary_LHS_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* bin = mod.CreateInstruction<ir::CoreBinary>(b.InstructionResult(ty.i32()), BinaryOp::kAdd,
                                                      nullptr, b.Constant(2_i));
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:18 error: binary: operand is undefined
    %2:i32 = add undef, 2i
                 ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Binary_RHS_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());

    auto* bin = mod.CreateInstruction<ir::CoreBinary>(b.InstructionResult(ty.i32()), BinaryOp::kAdd,
                                                      b.Constant(2_i), nullptr);
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:22 error: binary: operand is undefined
    %2:i32 = add 2i, undef
                     ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Binary_Result_Nullptr) {
    auto* bin = mod.CreateInstruction<ir::CoreBinary>(nullptr, BinaryOp::kAdd, b.Constant(3_i),
                                                      b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:5 error: binary: result is undefined
    undef = add 3i, 2i
    ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Binary_MissingOperands) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* add = sb.Add(sb.Constant(1_i), sb.Constant(2_i));
    add->ClearOperands();
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: binary: expected exactly 2 operands, got 0
    %2:i32 = add
    ^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Binary_MissingResult) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* add = sb.Add(sb.Constant(1_i), sb.Constant(2_i));
    add->ClearResults();
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: binary: expected exactly 1 results, got 0
    undef = add 1i, 2i
    ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Binary_LogicalOr) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Binary(core::BinaryOp::kLogicalOr, ty.bool_(), b.Constant(true), b.Constant(true));
        b.Return(func);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: binary: logical-or is not valid in the IR
    %2:bool = logical-or true, true
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Binary_LogicalAnd) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Binary(core::BinaryOp::kLogicalAnd, ty.bool_(), b.Constant(false), b.Constant(false));
        b.Return(func);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: binary: logical-and is not valid in the IR
    %2:bool = logical-and false, false
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Binary_Valid) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Add(b.Constant(1_i), b.Constant(2_i));
        b.Return(func);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Binary_TooManyOperands) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* add = b.Add(b.Constant(1_i), b.Constant(2_i));
        add->PushOperand(b.Constant(3_i));
        b.Return(func);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: binary: expected exactly 2 operands, got 3
    %2:i32 = add 1i, 2i, 3i
    ^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Binary_OperandWrongType_Func) {
    auto* func = b.Function("foo", ty.void_());
    auto* other_func = b.Function("other", ty.void_());
    b.Append(other_func->Block(), [&] { b.Return(other_func); });

    b.Append(func->Block(), [&] {
        b.Add(b.Constant(1_i), other_func);
        b.Return(func);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:3:5 error: binary: no matching overload for 'operator + (i32, <function>)'
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unary_Value_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Negation(nullptr);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:5 error: unary: result is undefined
    undef = negation undef
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:3:22 error: unary: operand is undefined
    undef = negation undef
                     ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unary_Result_Nullptr) {
    auto* bin = mod.CreateInstruction<ir::CoreUnary>(nullptr, UnaryOp::kNegation, b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:5 error: unary: result is undefined
    undef = negation 2i
    ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unary_ResultTypeNotMatchValueType) {
    auto* bin = b.Append(b.ir.CreateInstruction<ir::CoreUnary>(b.InstructionResult(ty.f32()),
                                                               UnaryOp::kComplement, b.Value(2_i)));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:5 error: unary: result value type 'f32' does not match complement result type 'i32'
    %2:f32 = complement 2i
    ^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unary_MissingOperands) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* u = b.Negation(2_f);
    u->ClearOperands();
    sb.Append(u);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: unary: expected exactly 1 operands, got 0
    %2:f32 = negation
    ^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unary_MissingResults) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* u = b.Negation(2_f);
    u->ClearResults();
    sb.Append(u);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: unary: expected exactly 1 results, got 0
    undef = negation 2.0f
    ^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unary_Valid) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Negation(b.Constant(1_i));
        b.Return(func);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Unary_TooManyOperands) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        // Manually create a negation with an extra operand.
        auto* neg = b.Negation(b.Constant(1_i));
        neg->PushOperand(b.Constant(2_i));
        b.Return(func);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:5 error: unary: expected exactly 1 operands, got 2
    %2:i32 = negation 1i, 2i
    ^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Unary_OperandWrongType) {
    auto* other_func = b.Function("other", ty.void_());
    b.Append(other_func->Block(), [&] { b.Return(other_func); });

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Negation(other_func);
        b.Return(func);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:8:5 error: unary: no matching overload for 'operator - (<function>)'
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Scoping_UseBeforeDecl) {
    auto* f = b.Function("my_func", ty.void_());

    auto* y = b.Add(2_i, 3_i);
    auto* x = b.Add(y, 1_i);

    f->Block()->Append(x);
    f->Block()->Append(y);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:3:18 error: binary: %3 is not in scope
    %2:i32 = add %3, 1i
                 ^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Scoping_UseAfterNestedDecl_InTerminator) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* outer = b.If(true);
        outer->AddResult(b.InstructionResult<u32>());

        b.Append(outer->True(), [&] {
            Let* decl = nullptr;
            auto* inner = b.If(true);
            b.Append(inner->True(), [&] {
                decl = b.Let("decl", 42_u);
                b.ExitIf(inner);
            });
            b.ExitIf(outer, decl);
        });

        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:11:17 error: exit_if: %decl is not in scope
        exit_if %decl  # if_1
                ^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Scoping_UseBeforeDecl_InControlFlow) {
    auto* f = b.Function("my_func", ty.void_());
    auto* decl = b.Let("decl", 42_u);
    auto* if_ = b.If(true);
    b.Append(if_->True(), [&] {
        b.Let("use", decl);
        b.ExitIf(if_);
    });
    f->Block()->Append(if_);
    f->Block()->Append(decl);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:5:24 error: let: %decl is not in scope
        %use:u32 = let %decl
                       ^^^^^

:4:7 note: in block
      $B2: {  # true
      ^^^

:9:5 note: %decl declared here
    %decl:u32 = let 42u
    ^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, OverrideWithoutCapability) {
    b.Append(mod.root_block, [&] { b.Override("a", 1_u); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:12 error: override: root block: invalid instruction: tint::core::ir::Override
  %a:u32 = override 1u
           ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, InstructionInRootBlockWithoutOverrideCap) {
    b.Append(mod.root_block, [&] { b.Add(3_u, 2_u); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:3 error: binary: root block: invalid instruction: tint::core::ir::CoreBinary
  %1:u32 = add 3u, 2u
  ^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, OverrideWithCapability) {
    b.Append(mod.root_block, [&] {
        auto* o = b.Override(ty.u32());
        o->SetOverrideId(OverrideId{1});
    });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, OverrideWithValue) {
    b.Append(mod.root_block, [&] {
        auto* z = b.Override(ty.u32());
        z->SetOverrideId(OverrideId{2});
        auto* init = b.Add(z, 2_u);

        b.Override("a", init);
    });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, OverrideWithInvalidType) {
    b.Append(mod.root_block, [&] { b.Override(ty.vec3u()); });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:2:18 error: override: override type 'vec3<u32>' is not a scalar
  %1:vec3<u32> = override undef
                 ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, OverrideWithMismatchedInitializerType) {
    b.Append(mod.root_block, [&] {
        auto* init = b.Constant(1_i);
        auto* o = b.Override(ty.u32());
        o->SetInitializer(init);
    });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:12 error: override: override type 'u32' does not match initializer type 'i32'
  %1:u32 = override 1i
           ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, OverrideDuplicateId) {
    b.Append(mod.root_block, [&] {
        auto* o = b.Override(ty.u32());
        o->SetOverrideId(OverrideId{2});

        auto* o2 = b.Override(ty.i32());
        o2->SetOverrideId(OverrideId{2});
    });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:3:12 error: override: duplicate override id encountered: 2
  %2:i32 = override undef @id(2)
           ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, InstructionInRootBlockOnlyUsedInRootBlock) {
    core::ir::Value* init = nullptr;
    b.Append(mod.root_block, [&] {
        auto* z = b.Override(ty.u32());
        z->SetOverrideId(OverrideId{2});
        init = b.Add(z, 2_u)->Result();
        b.Override("a", init);
    });

    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Add(init, 2_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:3 error: binary: root block: instruction used outside of root block tint::core::ir::CoreBinary
  %2:u32 = add %1, 2u
  ^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, OverrideArrayInvalidValue) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        o = b.Override(ty.u32());

        auto* c1 = ty.Get<core::ir::type::ValueArrayCount>(o->Result());
        auto* a1 = ty.Get<core::type::Array>(ty.i32(), c1, 4u);

        b.Var("a", ty.ptr(workgroup, a1, read_write));
    });
    o->Destroy();

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:2:51 error: var: %2 is not in scope
  %a:ptr<workgroup, array<i32, %2>, read_write> = var undef
                                                  ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, OverrideWithoutIdOrInitializer) {
    b.Append(mod.root_block, [&] { b.Override(ty.u32()); });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:12 error: override: must have an id or an initializer
  %1:u32 = override undef
           ^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, ValidateIfNeeded_Disabled) {
    auto* l = b.Loop();
    l->Body()->Append(b.Continue(l));
    mod.root_block->Append(l);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);

    // The module is invalid but the assertion should not fire when the flag is disabled.
    mod.enable_validation_asserts = false;
    AssertValid(mod);
}

TEST_F(IR_ValidatorDeathTest, ValidateIfNeeded_Enabled) {
    auto* l = b.Loop();
    l->Body()->Append(b.Continue(l));
    mod.root_block->Append(l);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);

    mod.enable_validation_asserts = true;

#if TINT_ENABLE_IR_VALIDATION_ASSERTS
    // The module is invalid and the assertion should trigger an ICE because the flag is enabled.
    EXPECT_DEATH_IF_SUPPORTED(
        {  //
            AssertValid(mod);
        },
        "internal compiler error");
#else
    // If validation assertions are disabled at build time, this should have no effect.
    AssertValid(mod);
#endif
}

}  // namespace tint::core::ir
