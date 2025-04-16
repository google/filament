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

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::core::ir {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(IR_ValidatorTest, Function) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_NoType) {
    auto* valid = b.Function("valid", ty.void_());
    valid->Block()->Append(b.Return(valid));

    auto* invalid = mod.CreateValue<ir::Function>(nullptr, ty.void_());
    invalid->SetBlock(mod.blocks.Create<ir::Block>());
    mod.SetName(invalid, "invalid");
    mod.functions.Push(invalid);
    invalid->Block()->Append(b.Return(invalid));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:1 error: functions must have type '<function>'
%invalid = func():void {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Duplicate) {
    auto* f = b.Function("my_func", ty.void_());
    // Function would auto-push by the builder, so this adds a duplicate
    mod.functions.Push(f);

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: function %my_func added to module multiple times
%my_func = func(%2:i32, %3:f32):void {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_DuplicateEntryPointNames) {
    auto* c = ComputeEntryPoint("dup");
    c->Block()->Append(b.Return(c));

    auto* f = FragmentEntryPoint("dup");
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:1 error: entry point name 'dup' is not unique
%dup_1 = @fragment func():void {  # %dup_1: 'dup'
^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_MultinBlock) {
    auto* f = b.Function("my_func", ty.void_());
    f->SetBlock(b.MultiInBlock());
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: root block for function cannot be a multi-in block
%my_func = func():void {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_DeadParameter) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.f32());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    p->Destroy();

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:1:17 error: destroyed parameter found in function parameter list
%my_func = func(%my_param:f32):void {
                ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParameterWithNullFunction) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.f32());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    p->SetFunction(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:17 error: function parameter has nullptr parent function
%my_func = func(%my_param:f32):void {
                ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParameterUsedInMultipleFunctions) {
    auto* p = b.FunctionParam("my_param", ty.f32());
    auto* f1 = b.Function("my_func1", ty.void_());
    auto* f2 = b.Function("my_func2", ty.void_());
    f1->SetParams({p});
    f2->SetParams({p});
    f1->Block()->Append(b.Return(f1));
    f2->Block()->Append(b.Return(f2));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:18 error: function parameter has incorrect parent function
%my_func1 = func(%my_param:f32):void {
                 ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParameterWithNullType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", nullptr);
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:17 error: function parameter has nullptr type
%my_func = func(%my_param:undef):void {
                ^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParameterDuplicated) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.u32());
    f->SetParams({p, p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:17 error: function parameter is not unique
%my_func = func(%my_param:u32%my_param:u32):void {
                ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_MultipleIOAnnotations) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("my_param", ty.vec4<f32>());
    p->SetBuiltin(BuiltinValue::kPosition);
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:27 error: input param has more than one IO annotation, [ @location, built-in ]
%my_func = @fragment func(%my_param:vec4<f32> [@location(0), @position]):void {
                          ^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_MultipleIOAnnotations) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.color = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.vec4<f32>(), attr},
                                               });
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:27 error: input param struct member has more than one IO annotation, [ built-in, @color ]
%my_func = @fragment func(%my_param:MyStruct):void {
                          ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_MissingIOAnnotations) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("my_param", ty.vec4<f32>());
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:27 error: input param must have at least one IO annotation, e.g. a binding point, a location, etc
%my_func = @fragment func(%my_param:vec4<f32>):void {
                          ^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_MissingIOAnnotations) {
    auto* f = ComputeEntryPoint("my_func");

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.vec4<f32>(), {}},
                                               });
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:54 error: input param struct members must have at least one IO annotation, e.g. a binding point, a location, etc
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%my_param:MyStruct):void {
                                                     ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_DuplicateAnnotations) {
    auto* f = ComputeEntryPoint("my_func");
    IOAttributes attr;
    attr.location = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.vec4<f32>(), attr},
                                               });
    auto* p = b.FunctionParam("my_param", str_ty);
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:54 error: input param struct member has same IO annotation, as top-level struct, '@location'
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%my_param:MyStruct [@location(0)]):void {
                                                     ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_WorkgroupPlusOtherIOAnnotation) {
    auto* f = ComputeEntryPoint("my_func");
    auto* p = b.FunctionParam("my_param", ty.ptr<workgroup, i32>());
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:54 error: input param has more than one IO annotation, [ @location, <workgroup> ]
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%my_param:ptr<workgroup, i32, read_write> [@location(0)]):void {
                                                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_WorkgroupPlusOtherIOAnnotations) {
    auto* f = ComputeEntryPoint("my_func");
    IOAttributes attr;
    attr.location = 0;
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"),
                             {
                                 {mod.symbols.New("a"), ty.ptr<workgroup, i32>(), attr},
                             });
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowPointersAndHandlesInStructures});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:54 error: input param struct member has more than one IO annotation, [ @location, <workgroup> ]
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%my_param:MyStruct):void {
                                                     ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParameterWithConstructibleType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.u32());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParameterWithPointerType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.ptr<function, i32>());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParameterWithTextureType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.external_texture());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParameterWithSamplerType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.sampler());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParameterWithVoidType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.void_());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:17 error: function parameter type, 'void', must be constructible, a pointer, or a handle
%my_func = func(%my_param:void):void {
                ^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_InvariantWithPosition) {
    auto* f = b.Function("my_func", ty.void_(), Function::PipelineStage::kFragment);

    auto* p = b.FunctionParam("my_param", ty.vec4<f32>());
    p->SetInvariant(true);
    p->SetBuiltin(BuiltinValue::kPosition);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_InvariantWithoutPosition) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.vec4<f32>());
    p->SetInvariant(true);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:17 error: invariant can only decorate a param iff it is also decorated with position
%my_func = func(%my_param:vec4<f32> [@invariant]):void {
                ^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_InvariantWithPosition) {
    auto* f = b.Function("my_func", ty.void_(), Function::PipelineStage::kFragment);

    IOAttributes attr;
    attr.invariant = true;
    attr.builtin = BuiltinValue::kPosition;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4<f32>(), attr},
                                               });
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_InvariantWithoutPosition) {
    IOAttributes attr;
    attr.invariant = true;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4<f32>(), attr},
                                               });

    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:17 error: invariant can only decorate a param member iff it is also decorated with position
%my_func = func(%my_param:MyStruct):void {
                ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_BindingPointWithoutCapability) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.ptr<uniform, i32>());
    p->SetBindingPoint(0, 0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:17 error: input param to non-entry point function has a binding point set
%my_func = func(%my_param:ptr<uniform, i32, read> [@binding_point(0, 0)]):void {
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_MultipleIOAnnotations) {
    auto* f = VertexEntryPoint("my_func");
    f->SetReturnLocation(0);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: return values has more than one IO annotation, [ @location, built-in ]
%my_func = @vertex func():vec4<f32> [@location(0), @position] {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_MultipleIOAnnotations) {
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.location = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.vec4<f32>(), attr},
                                               });
    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: return values struct member has more than one IO annotation, [ @location, built-in ]
%my_func = @vertex func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Void_IOAnnotation) {
    auto* f = FragmentEntryPoint();
    f->SetReturnLocation(0);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:1:1 error: return values with void type should never be annotated
%f = @fragment func():void [@location(0)] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_NonVoid_MissingIOAnnotations) {
    auto* f = b.Function("my_func", ty.f32(), Function::PipelineStage::kFragment);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: return values must have at least one IO annotation, e.g. a binding point, a location, etc
%my_func = @fragment func():f32 {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_NonVoid_Struct_MissingIOAnnotations) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.f32(), {}},
                                                          });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kFragment);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: return values struct members must have at least one IO annotation, e.g. a binding point, a location, etc
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_InvariantWithPosition) {
    auto* f = b.Function("my_func", ty.vec4<f32>(), Function::PipelineStage::kVertex);
    f->SetReturnBuiltin(BuiltinValue::kPosition);
    f->SetReturnInvariant(true);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_InvariantWithoutPosition) {
    auto* f = b.Function("my_func", ty.vec4<f32>());
    f->SetReturnInvariant(true);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: invariant can only decorate outputs iff they are also position builtins
%my_func = func():vec4<f32> [@invariant] {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_InvariantWithPosition) {
    IOAttributes attr;
    attr.invariant = true;
    attr.builtin = BuiltinValue::kPosition;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4<f32>(), attr},
                                               });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_InvariantWithoutPosition) {
    IOAttributes attr;
    attr.invariant = true;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4<f32>(), attr},
                                               });

    auto* f = b.Function("my_func", str_ty);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: invariant can only decorate output members iff they are also position builtins
%my_func = func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_UnnamedEntryPoint) {
    auto* f = b.Function(ty.void_(), ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(1_u), b.Constant(1_u)});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(R"(:1:1 error: entry points must have names
%1 = @compute @workgroup_size(1u, 1u, 1u) func():void {
^^
)")) << res.Failure();
}

// Parameterizing these tests is very difficult/unreadable due to the
// multiple layers of forwarding/templating occurring in the type manager, so
// writing them out explicitly instead.
TEST_F(IR_ValidatorTest, Function_NonConstructibleReturnType_ExternalTexture) {
    auto* f = b.Function(ty.external_texture());
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: function return type must be constructible
%1 = func():texture_external {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_NonConstructibleReturnType_Sampler) {
    auto* f = b.Function(ty.sampler());
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: function return type must be constructible
%1 = func():sampler {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_NonConstructibleReturnType_RuntimeArray) {
    auto* f = b.Function(ty.runtime_array(ty.f32()));
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: function return type must be constructible
%1 = func():array<f32> {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_NonConstructibleReturnType_Ptr) {
    auto* f = b.Function(ty.ptr<function, i32>());
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: function return type must be constructible
%1 = func():ptr<function, i32, read_write> {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_NonConstructibleReturnType_Ref) {
    auto* f = b.Function(ty.ref<function, u32>());
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: function return type must be constructible
%1 = func():ref<function, u32, read_write> {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Compute_NonVoidReturn) {
    auto* f = b.Function("my_func", ty.f32(), core::ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));
    f->SetReturnLocation(0);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:1 error: compute entry point must not have a return type, found 'f32'
%my_func = @compute @workgroup_size(1u, 1u, 1u) func():f32 [@location(0)] {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_MissingOnCompute) {
    auto* f = b.Function("f", ty.void_(), Function::PipelineStage::kCompute);
    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: compute entry point requires @workgroup_size
%f = @compute func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_NonCompute) {
    auto* f = FragmentEntryPoint();
    f->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: @workgroup_size only valid on compute entry point
%f = @fragment @workgroup_size(1u, 1u, 1u) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamUndefined) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({nullptr, b.Constant(2_u), b.Constant(3_u)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:1:1 error: a @workgroup_size param is undefined or missing a type
%f = @compute @workgroup_size(undef, 2u, 3u) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamWrongType) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_f), b.Constant(2_u), b.Constant(3_u)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:1 error: @workgroup_size params must be an 'i32' or 'u32', received 'f32'
%f = @compute @workgroup_size(1.0f, 2u, 3u) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamsSameType) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_i), b.Constant(3_u)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:1:1 error: @workgroup_size params must be all 'i32's or all 'u32's
%f = @compute @workgroup_size(1u, 2i, 3u) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamsTooSmall) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(-1_i), b.Constant(2_i), b.Constant(3_i)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: @workgroup_size params must be greater than 0
%f = @compute @workgroup_size(-1i, 2i, 3i) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_OverrideWithoutAllowOverrides) {
    auto* o = b.Override(ty.u32());
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({o->Result(), o->Result(), o->Result()});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: @workgroup_size param is not a constant value, and IR capability 'kAllowOverrides' is not set
%f = @compute @workgroup_size(%2, %2, %2) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_NonRootBlockOverride) {
    auto* f = ComputeEntryPoint();
    Override* o;
    b.Append(f->Block(), [&] {
        o = b.Override(ty.u32());
        b.Return(f);
    });
    f->SetWorkgroupSize({o->Result(), o->Result(), o->Result()});

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:1:1 error: @workgroup_size param defined by non-module scope value
%f = @compute @workgroup_size(%2, %2, %2) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Vertex_BasicPosition) {
    auto* f = b.Function("my_func", ty.vec4<f32>(), Function::PipelineStage::kVertex);
    f->SetReturnBuiltin(BuiltinValue::kPosition);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Vertex_StructPosition) {
    auto pos_ty = ty.vec4<f32>();
    auto pos_attr = IOAttributes();
    pos_attr.builtin = BuiltinValue::kPosition;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), pos_ty, pos_attr},
                                               });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Vertex_StructPositionAndClipDistances) {
    auto pos_ty = ty.vec4<f32>();
    auto pos_attr = IOAttributes();
    pos_attr.builtin = BuiltinValue::kPosition;

    auto clip_ty = ty.array<f32, 4>();
    auto clip_attr = IOAttributes();
    clip_attr.builtin = BuiltinValue::kClipDistances;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), pos_ty, pos_attr},
                                                   {mod.symbols.New("clip"), clip_ty, clip_attr},
                                               });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Vertex_StructOnlyClipDistances) {
    auto clip_ty = ty.array<f32, 4>();
    auto clip_attr = IOAttributes();
    clip_attr.builtin = BuiltinValue::kClipDistances;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("clip"), clip_ty, clip_attr},
                                               });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:5:1 error: position must be declared for vertex entry point output
%my_func = @vertex func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Vertex_MissingPosition) {
    auto* f = b.Function("my_func", ty.vec4<f32>(), Function::PipelineStage::kVertex);
    f->SetReturnLocation(0);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:1:1 error: position must be declared for vertex entry point output
%my_func = @vertex func():vec4<f32> [@location(0)] {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_NonFragment_BoolInput) {
    auto* f = VertexEntryPoint();
    auto* p = b.FunctionParam("invalid", ty.bool_());
    p->SetLocation(0);
    f->AppendParam(p);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:19 error: entry point params can only be a bool for fragment shaders
%f = @vertex func(%invalid:bool [@location(0)]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_NonFragment_BoolOutput) {
    auto* f = VertexEntryPoint();
    IOAttributes attr;
    attr.location = 0;
    AddReturn(f, "invalid", ty.bool_(), attr);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:1 error: entry point return members can not be 'bool'
%f = @vertex func():OutputStruct {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Fragment_BoolInputWithoutFrontFacing) {
    auto* f = FragmentEntryPoint();
    auto* p = b.FunctionParam("invalid", ty.bool_());
    p->SetLocation(0);
    f->AppendParam(p);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:21 error: fragment entry point params can only be a bool if decorated with @builtin(front_facing)
%f = @fragment func(%invalid:bool [@location(0)]):void {
                    ^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Fragment_BoolOutput) {
    auto* f = FragmentEntryPoint();
    IOAttributes attr;
    attr.location = 0;
    AddReturn(f, "invalid", ty.bool_(), attr);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: entry point returns can not be 'bool'
%f = @fragment func():bool [@location(0)] {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_BoolOutput_via_MSV) {
    auto* f = ComputeEntryPoint();

    auto* v = b.Var(ty.ptr(AddressSpace::kOut, ty.bool_(), core::Access::kReadWrite));
    IOAttributes attr;
    attr.location = 0;
    v->SetAttributes(attr);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::Store>(v->Result(), b.Constant(b.ConstantValue(false))));
        b.Unreachable();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: IO address space values referenced by shader entry points can only be 'bool' if in the input space, used only by fragment shaders and decorated with @builtin(front_facing)
%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_BoolInputWithoutFrontFacing_via_MSV) {
    auto* f = FragmentEntryPoint();

    auto* invalid = b.Var("invalid", AddressSpace::kIn, ty.bool_());
    IOAttributes attr;
    attr.location = 0;
    invalid->SetAttributes(attr);
    mod.root_block->Append(invalid);

    b.Append(f->Block(), [&] {
        auto* l = b.Load(invalid);
        auto* v = b.Var("v", AddressSpace::kFunction, ty.bool_());
        v->SetInitializer(l->Result());
        b.Unreachable();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: input address space values referenced by fragment shaders can only be 'bool' if decorated with @builtin(front_facing)
%f = @fragment func():void {
^^
)")) << res.Failure();
}

}  // namespace tint::core::ir
