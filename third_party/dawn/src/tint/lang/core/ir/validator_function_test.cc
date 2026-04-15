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

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/validator_test.h"
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

TEST_F(IR_ValidatorTest, Function_MultipleEntryPoints_WithCapability) {
    auto* ep1 = ComputeEntryPoint("ep1");
    ep1->Block()->Append(b.Return(ep1));

    auto* ep2 = ComputeEntryPoint("ep2");
    ep2->Block()->Append(b.Return(ep2));

    auto res = ir::Validate(mod, Capabilities{
                                     Capability::kAllowMultipleEntryPoints,
                                 });
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_MultipleEntryPoints_WithoutCapability) {
    auto* ep1 = ComputeEntryPoint("ep1");
    ep1->Block()->Append(b.Return(ep1));

    auto* ep2 = ComputeEntryPoint("ep2");
    ep2->Block()->Append(b.Return(ep2));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:1 error: a module with multiple entry points requires the AllowMultipleEntryPoints capability
%ep2 = @compute @workgroup_size(1u, 1u, 1u) func():void {
^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_DuplicateEntryPointNames) {
    auto* c = ComputeEntryPoint("dup");
    c->Block()->Append(b.Return(c));

    auto* f = FragmentEntryPoint("dup");
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod, Capabilities{
                                     Capability::kAllowMultipleEntryPoints,
                                 });
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

    auto* p = b.FunctionParam("my_param", ty.vec4f());
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
                                                   {mod.symbols.New("a"), ty.vec4f(), attr},
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

    auto* p = b.FunctionParam("my_param", ty.vec4f());
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
                                                   {mod.symbols.New("a"), ty.vec4f(), {}},
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
    attr.builtin = BuiltinValue::kPosition;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.vec4f(), attr},
                                               });
    auto* p = b.FunctionParam("my_param", str_ty);
    p->SetBuiltin(BuiltinValue::kPosition);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:54 error: input param struct member has same IO annotation, as top-level struct, 'built-in'
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%my_param:MyStruct [@position]):void {
                                                     ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_WorkgroupPlusOtherIOAnnotation) {
    auto* f = ComputeEntryPoint("my_func");
    auto* p = b.FunctionParam("my_param", ty.ptr<workgroup, i32>());
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kMslAllowEntryPointInterface});
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

    auto res = ir::Validate(mod, Capabilities{Capability::kMslAllowEntryPointInterface});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:54 error: input param struct member has more than one IO annotation, [ @location, <workgroup> ]
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%my_param:MyStruct):void {
                                                     ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Location_InvalidType) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("my_param", ty.bool_());
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:27 error: fragment entry point params can only be a bool if decorated with @builtin(front_facing)
%my_func = @fragment func(%my_param:bool [@location(0)]):void {
                          ^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_Location_InvalidType) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.location = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.bool_(), attr},
                                               });
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:27 error: fragment entry point params can only be a bool if decorated with @builtin(front_facing)
%my_func = @fragment func(%my_param:MyStruct):void {
                          ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Location_Struct_WithCapability) {
    auto* f = FragmentEntryPoint("my_func");

    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.f32()},
                                                          });
    auto* p = b.FunctionParam("my_param", str_ty);
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowLocationForNumericElements});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Location_Struct_WithoutCapability) {
    auto* f = FragmentEntryPoint("my_func");

    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.f32()},
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
            R"(:5:27 error: input param with a location attribute must be a numeric scalar or vector, but has type MyStruct
%my_func = @fragment func(%my_param:MyStruct [@location(0)]):void {
                          ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_InputLocation_Duplicate_InParams) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p1 = b.FunctionParam("p1", ty.f32());
    p1->SetLocation(0);
    auto* p2 = b.FunctionParam("p2", ty.f32());
    p2->SetLocation(0);
    f->SetParams({p1, p2});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:51 error: duplicate location(0) on entry point input
%my_func = @fragment func(%p1:f32 [@location(0)], %p2:f32 [@location(0)]):void {
                                                  ^^^^^^^

:1:27 note: %p1 declared here
%my_func = @fragment func(%p1:f32 [@location(0)], %p2:f32 [@location(0)]):void {
                          ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_InputLocation_Duplicate_InParamAndMSV) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p1 = b.FunctionParam("p1", ty.f32());
    p1->SetLocation(0);
    f->SetParams({p1});

    auto* v = b.Var("v", AddressSpace::kIn, ty.f32());
    v->SetLocation(0);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Load(v);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:29 error: var: duplicate location(0) on entry point input
  %v:ptr<__in, f32, read> = var undef @location(0)
                            ^^^

:5:27 note: %p1 declared here
%my_func = @fragment func(%p1:f32 [@location(0)]):void {
                          ^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_OutputLocation_Duplicate_InReturnAndMSV) {
    auto* f = FragmentEntryPoint("my_func");
    f->SetReturnType(ty.f32());
    f->SetReturnLocation(0);

    auto* v = b.Var("v", AddressSpace::kOut, ty.f32());
    v->SetLocation(0);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Store(v, 1.0_f);
        b.Return(f, 1.0_f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:2:36 error: var: duplicate location(0) on entry point output
  %v:ptr<__out, f32, read_write> = var undef @location(0)
                                   ^^^

:5:1 note: %my_func declared here
%my_func = @fragment func():f32 [@location(0)] {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_OutputLocation_Duplicate_InReturnStruct) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.location = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr},
                                                   {mod.symbols.New("b"), ty.f32(), attr},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:1 error: duplicate location(0) on entry point output
%my_func = @fragment func():MyStruct {
^^^^^^^^

:6:1 note: %my_func declared here
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_Compute_InputLocation_InParam) {
    auto* f = ComputeEntryPoint("my_func");

    auto* p = b.FunctionParam("p", ty.f32());
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:54 error: location attribute is not valid for compute shader inputs
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%p:f32 [@location(0)]):void {
                                                     ^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_Compute_InputLocation_InMSV) {
    auto* f = ComputeEntryPoint("my_func");

    auto* v = b.Var("v", AddressSpace::kIn, ty.f32());
    v->SetLocation(0);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Load(v);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:29 error: var: location attribute is not valid for compute shader inputs
  %v:ptr<__in, f32, read> = var undef @location(0)
                            ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_Workgroup_NotCompute) {
    auto* f = FragmentEntryPoint("my_func");

    auto* v = b.Var("v", AddressSpace::kWorkgroup, ty.f32());
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Load(v);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:40 error: var: workgroup variable cannot be used in a fragment shader
  %v:ptr<workgroup, f32, read_write> = var undef
                                       ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_PixelLocal_NotFragment) {
    auto* f = ComputeEntryPoint("my_func");

    auto* str_ty = ty.Struct(mod.symbols.New("S"), {{mod.symbols.New("a"), ty.u32()}});
    auto* v = b.Var("v", AddressSpace::kPixelLocal, str_ty);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Load(v);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:40 error: var: pixel_local variable cannot be used in a compute shader
  %v:ptr<pixel_local, S, read_write> = var undef
                                       ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_In_Compute) {
    auto* f = ComputeEntryPoint("my_func");

    auto* v = b.Var("v", AddressSpace::kIn, ty.f32());
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Load(v);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:29 error: var: module scope variable must have at least one IO annotation, e.g. a binding point, a location, etc
  %v:ptr<__in, f32, read> = var undef
                            ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_SameLocation_InputAndOutput) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("p", ty.f32());
    p->SetLocation(0);
    f->SetParams({p});

    f->SetReturnType(ty.f32());
    f->SetReturnLocation(0);

    b.Append(f->Block(), [&] { b.Return(f, 1.0_f); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_SameLocation_DifferentEntryPoints) {
    auto* f1 = FragmentEntryPoint("f1");
    auto* p1 = b.FunctionParam("p1", ty.f32());
    p1->SetLocation(0);
    f1->SetParams({p1});
    b.Append(f1->Block(), [&] { b.Return(f1); });

    auto* f2 = FragmentEntryPoint("f2");
    auto* p2 = b.FunctionParam("p2", ty.f32());
    p2->SetLocation(0);
    f2->SetParams({p2});
    b.Append(f2->Block(), [&] { b.Return(f2); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowMultipleEntryPoints});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_Valid) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_TooManyLocations) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;

    IOAttributes attr2;
    attr2.location = 1;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                                   {mod.symbols.New("c"), ty.f32(), attr2},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:7:1 error: structs with blend_src members must have exactly 2 members with location annotations
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_Input) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    auto* p = b.FunctionParam("p", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:6:27 error: blend_src can only be used on fragment shader outputs
%my_func = @fragment func(%p:MyStruct):void {
                          ^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_NotFragment) {
    auto* f = ComputeEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:6:1 error: blend_src can only be used on fragment shader outputs
%my_func = @compute @workgroup_size(1u, 1u, 1u) func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_WrongMemberCount) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {{mod.symbols.New("a"), ty.f32(), attr0}});
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: structs with blend_src members must have exactly 2 members with location annotations
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_WrongLocation) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 1;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.location = 1;
    attr1.blend_src = 1;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:1 error: struct members with blend_src must be located at 0
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_MissingLocation) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.blend_src = 1;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:1 error: struct members with blend_src must be located at 0
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_DifferentMemberTypes) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.vec4f(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:1 error: blend_src type f32 does not match other blend_src type vec4<f32>
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_InvalidMemberType) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.bool_(), attr0},
                                                   {mod.symbols.New("b"), ty.bool_(), attr1},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:1 error: blend_src must be a numeric scalar or vector, but has type bool
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_MissingBlendSrc0) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.location = 0;
    attr.blend_src = 1;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr},
                                                   {mod.symbols.New("b"), ty.f32(), attr},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:1 error: if any @blend_src is used on an output, then @blend_src(0) and @blend_src(1) must be used
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_MissingBlendSrc1) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.location = 0;
    attr.blend_src = 0;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr},
                                                   {mod.symbols.New("b"), ty.f32(), attr},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:1 error: if any @blend_src is used on an output, then @blend_src(0) and @blend_src(1) must be used
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_InvalidBlendSrcValue) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 2;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:6:1 error: blend_src value must be 0 or 1
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_DuplicateBlendSrcValue) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;

    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 0;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:1 error: duplicate blend_src(0) on entry point output
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_DuplicateLocation_Unused) {
    auto* f = FragmentEntryPoint("my_func");

    // A valid blend_src struct at location 0.
    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;
    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    f->SetReturnType(str_ty);

    // Another output variable also at location 0.
    auto* v = b.Var("v", AddressSpace::kOut, ty.f32());
    v->SetLocation(0);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_DuplicateLocation_Used) {
    auto* f = FragmentEntryPoint("my_func");

    // A valid blend_src struct at location 0.
    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;
    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                                   {mod.symbols.New("b"), ty.f32(), attr1},
                                               });
    f->SetReturnType(str_ty);

    // Another output variable also at location 0.
    auto* v = b.Var("v", AddressSpace::kOut, ty.f32());
    v->SetLocation(0);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Store(v, 1.0_f);
        b.Unreachable();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:7:36 error: var: duplicate location(0) on entry point output
  %v:ptr<__out, f32, read_write> = var undef @location(0)
                                   ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_ArrayOfStructs) {
    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;
    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;
    auto* blend_struct_ty =
        ty.Struct(mod.symbols.New("BlendStruct"), {
                                                      {mod.symbols.New("a"), ty.f32(), attr0},
                                                      {mod.symbols.New("b"), ty.f32(), attr1},
                                                  });

    auto* array_ty = ty.array(blend_struct_ty, 2u);
    auto* v = b.Var("v", AddressSpace::kOut, array_ty);
    mod.root_block->Append(v);

    auto* f = FragmentEntryPoint("my_func");
    b.Append(f->Block(), [&] {
        b.Store(v, b.Zero(array_ty));
        b.Unreachable();
    });

    // Need to add Capability::kAllowUnannotatedModuleIOVariables to prevent earlier checks
    // rejecting the shader
    auto res = ir::Validate(mod, Capabilities{Capability::kAllowUnannotatedModuleIOVariables});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:7:54 error: var: blend_src cannot be used on members of non-top level structs
  %v:ptr<__out, array<BlendStruct, 2>, read_write> = var undef
                                                     ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_NestedStruct) {
    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;
    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;
    auto* blend_struct_ty =
        ty.Struct(mod.symbols.New("BlendStruct"), {
                                                      {mod.symbols.New("a"), ty.f32(), attr0},
                                                      {mod.symbols.New("b"), ty.f32(), attr1},
                                                  });

    auto* outer_struct_ty =
        ty.Struct(mod.symbols.New("OuterStruct"), {{mod.symbols.New("inner"), blend_struct_ty}});
    auto* v = b.Var("v", AddressSpace::kOut, outer_struct_ty);
    mod.root_block->Append(v);

    auto* f = FragmentEntryPoint("my_func");
    b.Append(f->Block(), [&] { b.Store(v, b.Zero(outer_struct_ty)); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowUnannotatedModuleIOVariables});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:11:44 error: var: blend_src cannot be used on members of non-top level structs
  %v:ptr<__out, OuterStruct, read_write> = var undef
                                           ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_PartialStruct) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.location = 0;
    attr.blend_src = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr},
                                               });
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: structs with blend_src members must have exactly 2 members with location annotations
%my_func = @fragment func():MyStruct {
^^^^^^^^

:5:1 error: if any @blend_src is used on an output, then @blend_src(0) and @blend_src(1) must be used
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_PartialStructAndMSV) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr0;
    attr0.location = 0;
    attr0.blend_src = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr0},
                                               });
    f->SetReturnType(str_ty);

    // MSV for the missing blend_src
    IOAttributes attr1;
    attr1.location = 0;
    attr1.blend_src = 1;
    auto* v = b.Var("v", AddressSpace::kOut, ty.f32());
    v->SetAttributes(attr1);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Store(v, 1.0_f);
        b.Unreachable();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:9:1 error: structs with blend_src members must have exactly 2 members with location annotations
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_NonMember_WithoutCapability) {
    auto* f = FragmentEntryPoint("my_func");

    auto* var0 = b.Var("var0", ty.ptr(AddressSpace::kOut, ty.f32()));
    var0->SetLocation(0);
    var0->SetBlendSrc(0);
    mod.root_block->Append(var0);

    auto* var1 = b.Var("var1", ty.ptr(AddressSpace::kOut, ty.f32()));
    var1->SetLocation(0);
    var1->SetBlendSrc(1);
    mod.root_block->Append(var1);

    b.Append(f->Block(), [&] {
        b.Store(var0, 1_f);
        b.Store(var1, 1_f);
        b.Return(f);
    });
    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:2:39 error: var: blend_src cannot be used on non-struct-member types
  %var0:ptr<__out, f32, read_write> = var undef @location(0) @blend_src(0)
                                      ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, EntryPoint_BlendSrc_NonMember_WithCapability) {
    auto* f = FragmentEntryPoint("my_func");

    auto* var0 = b.Var("var0", ty.ptr(AddressSpace::kOut, ty.f32()));
    var0->SetLocation(0);
    var0->SetBlendSrc(0);
    mod.root_block->Append(var0);

    auto* var1 = b.Var("var1", ty.ptr(AddressSpace::kOut, ty.f32()));
    var1->SetLocation(0);
    var1->SetBlendSrc(1);
    mod.root_block->Append(var1);

    b.Append(f->Block(), [&] {
        b.Store(var0, 1_f);
        b.Store(var1, 1_f);
        b.Return(f);
    });
    auto res = ir::Validate(mod, Capabilities{
                                     Capability::kLoosenValidationForShaderIO,
                                 });
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_WithLocation) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("p", ty.f32());
    p->SetLocation(0);
    p->SetInterpolation(Interpolation{.type = InterpolationType::kLinear,
                                      .sampling = InterpolationSampling::kCenter});
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_WithoutLocation) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("p", ty.f32());
    p->SetInterpolation(Interpolation{.type = InterpolationType::kLinear,
                                      .sampling = InterpolationSampling::kCenter});
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:27 error: interpolation attribute requires a location attribute
%my_func = @fragment func(%p:f32):void {
                          ^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_Struct_WithLocation) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.location = 0;
    attr.interpolation = {InterpolationType::kLinear, InterpolationSampling::kCenter};
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.f32(), attr},
                                                });
    auto* p = b.FunctionParam("p", str);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_Struct_WithoutLocation) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.interpolation = {InterpolationType::kLinear, InterpolationSampling::kCenter};
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.f32(), attr},
                                                });
    auto* p = b.FunctionParam("p", str);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:5:27 error: interpolation attribute requires a location attribute
%my_func = @fragment func(%p:S):void {
                          ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_Struct_LocationOnStruct_WithCapability) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.interpolation = {InterpolationType::kLinear, InterpolationSampling::kCenter};
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.f32(), attr},
                                                });
    auto* p = b.FunctionParam("p", str);
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowLocationForNumericElements});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_Struct_LocationOnStruct_WithoutCapability) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.interpolation = {InterpolationType::kLinear, InterpolationSampling::kCenter};
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.f32(), attr},
                                                });
    auto* p = b.FunctionParam("p", str);
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:27 error: input param with a location attribute must be a numeric scalar or vector, but has type S
%my_func = @fragment func(%p:S [@location(0)]):void {
                          ^^^^
)")) << res.Failure();
}
TEST_F(IR_ValidatorTest, Function_Interpolate_Struct_WithoutCapability) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr_a;
    attr_a.location = 0;
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.f32(), attr_a},
                                                });
    auto* p = b.FunctionParam("p", str);
    p->SetInterpolation(Interpolation{InterpolationType::kLinear, InterpolationSampling::kCenter});
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:27 error: interpolation cannot be applied to a struct without 'kAllowLocationForNumericElements' capability
%my_func = @fragment func(%p:S):void {
                          ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_Struct_LocationOnAllMembers_WithCapability) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr_a;
    attr_a.location = 0;
    IOAttributes attr_b;
    attr_b.location = 1;
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.f32(), attr_a},
                                                    {mod.symbols.New("b"), ty.f32(), attr_b},
                                                });
    auto* p = b.FunctionParam("p", str);
    p->SetInterpolation(Interpolation{InterpolationType::kLinear, InterpolationSampling::kCenter});
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowLocationForNumericElements});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_Struct_LocationOnAllMembers_WithoutCapability) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr_a;
    attr_a.location = 0;
    IOAttributes attr_b;
    attr_b.location = 1;
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.f32(), attr_a},
                                                    {mod.symbols.New("b"), ty.f32(), attr_b},
                                                });
    auto* p = b.FunctionParam("p", str);
    p->SetInterpolation(Interpolation{InterpolationType::kLinear, InterpolationSampling::kCenter});
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:27 error: interpolation cannot be applied to a struct without 'kAllowLocationForNumericElements' capability
%my_func = @fragment func(%p:S):void {
                          ^^^^

)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_Struct_LocationOnSomeMembers_WithCapability) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr_a;
    attr_a.location = 0;
    auto* str = ty.Struct(mod.symbols.New("S"), {
                                                    {mod.symbols.New("a"), ty.f32(), attr_a},
                                                    {mod.symbols.New("b"), ty.f32()},
                                                });
    auto* p = b.FunctionParam("p", str);
    p->SetInterpolation(Interpolation{InterpolationType::kLinear, InterpolationSampling::kCenter});
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowLocationForNumericElements});
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:6:27 error: interpolation attribute requires a location attribute
%my_func = @fragment func(%p:S):void {
                          ^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_WithBuiltin_WithCapability) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("p", ty.u32());
    p->SetBuiltin(BuiltinValue::kSampleIndex);
    p->SetInterpolation(Interpolation{.type = InterpolationType::kFlat,
                                      .sampling = InterpolationSampling::kUndefined});
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kLoosenValidationForShaderIO});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_WithBuiltin_WithoutCapability) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("p", ty.u32());
    p->SetBuiltin(BuiltinValue::kSampleIndex);
    p->SetInterpolation(Interpolation{.type = InterpolationType::kFlat,
                                      .sampling = InterpolationSampling::kUndefined});
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:27 error: interpolation attribute requires a location attribute
%my_func = @fragment func(%p:u32 [@interpolate(flat), @sample_index]):void {
                          ^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Interpolate_Integral_NotFlat) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("p", ty.i32());
    p->SetLocation(0);
    p->SetInterpolation(Interpolation{InterpolationType::kLinear, InterpolationSampling::kCenter});
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:27 error: interpolation attribute type must be flat for integral types
%my_func = @fragment func(%p:i32 [@location(0), @interpolate(linear, center)]):void {
                          ^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Location_Integral_WithoutInterpolation_VertexInput) {
    auto* f = VertexEntryPoint("my_func");

    auto* p = b.FunctionParam("p", ty.i32());
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f, b.Zero<vec4f>()); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Location_Integral_WithoutInterpolation_VertexOutput) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"),
                  {
                      {mod.symbols.New("pos"), ty.vec4f(), {.builtin = BuiltinValue::kPosition}},
                      {mod.symbols.New("loc"), ty.vec4u(), {.location = 0u}},
                  });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:1 error: integral user-defined inputs and outputs must have an @interpolate(flat) attribute
%my_func = @vertex func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Location_Integral_WithoutInterpolation_FragmentInput) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("p", ty.i32());
    p->SetLocation(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:27 error: integral user-defined inputs and outputs must have an @interpolate(flat) attribute
%my_func = @fragment func(%p:i32 [@location(0)]):void {
                          ^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Location_Integral_WithoutInterpolation_FragmentOutput) {
    auto* f = FragmentEntryPoint("my_func");
    f->SetReturnType(ty.u32());
    f->SetReturnLocation(0);

    b.Append(f->Block(), [&] { b.Return(f, 0_u); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

// Test that an integral builtin inside a struct with another member with a location attribute does
// not falsely trigger validation for required flat interpolation attributes.
TEST_F(IR_ValidatorTest, Function_IntegralBuiltin_InStructWithLocationMember) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"),
                  {
                      {mod.symbols.New("loc"), ty.vec4f(), {.location = 0u}},
                      {mod.symbols.New("mask"), ty.u32(), {.builtin = BuiltinValue::kSampleMask}},
                  });

    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("p", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
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

TEST_F(IR_ValidatorTest, Function_EntryPointParameterWithPointerType) {
    auto* f = b.Function("my_func", ty.void_(), Function::PipelineStage::kFragment);
    auto* p = b.FunctionParam("my_param", ty.ptr<function, u32>());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:1:27 error: entry point parameters cannot be pointers
%my_func = @fragment func(%my_param:ptr<function, u32, read_write>):void {
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_InvariantWithPosition) {
    auto* f = b.Function("my_func", ty.void_(), Function::PipelineStage::kFragment);

    auto* p = b.FunctionParam("my_param", ty.vec4f());
    p->SetInvariant(true);
    p->SetBuiltin(BuiltinValue::kPosition);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_InvariantWithoutPosition) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.vec4f());
    p->SetInvariant(true);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:17 error: invariant can only decorate a value if it is also decorated with position
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
                                                   {mod.symbols.New("pos"), ty.vec4f(), attr},
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
                                                   {mod.symbols.New("pos"), ty.vec4f(), attr},
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
            R"(:5:17 error: invariant can only decorate a value if it is also decorated with position
%my_func = func(%my_param:MyStruct):void {
                ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_StructNested_InvariantWithoutPosition) {
    IOAttributes attr;
    attr.invariant = true;

    auto* inner_ty =
        ty.Struct(mod.symbols.New("Inner"), {{mod.symbols.New("pos"), ty.vec4f(), attr}});

    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {{mod.symbols.New("i"), inner_ty}});

    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:9:17 error: invariant can only decorate a value if it is also decorated with position
%my_func = func(%my_param:MyStruct):void {
                ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Color_NonFragment) {
    auto* f = b.ComputeFunction("my_func");
    auto* p = b.FunctionParam("my_param", ty.vec4f());
    p->SetColor(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:54 error: color IO attributes cannot be declared for a compute shader input. They can only be used for a fragment shader input.
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%my_param:vec4<f32> [@color(0)]):void {
                                                     ^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_Color_NonFragment) {
    IOAttributes attr;
    attr.color = 0;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4f(), attr},
                                               });

    auto* f = b.ComputeFunction("my_func");
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:54 error: color IO attributes cannot be declared for a compute shader input. They can only be used for a fragment shader input.
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%my_param:MyStruct):void {
                                                     ^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Color) {
    auto* f = FragmentEntryPoint("my_func");
    f->SetReturnType(ty.vec4f());

    IOAttributes attr;
    attr.color = 0;
    f->SetReturnAttributes(attr);

    b.Append(f->Block(), [&] { b.Return(f, b.Zero(ty.vec4f())); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: color IO attributes cannot be declared for a fragment shader output. They can only be used for a fragment shader input.
%my_func = @fragment func():vec4<f32> {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_Color) {
    IOAttributes attr;
    attr.color = 0;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4f(), attr},
                                               });

    auto* f = FragmentEntryPoint("my_func");
    f->SetReturnType(str_ty);

    b.Append(f->Block(), [&] { b.Return(f, b.Zero(str_ty)); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: color IO attributes cannot be declared for a fragment shader output. They can only be used for a fragment shader input.
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_MSV_Color_Output) {
    auto* f = FragmentEntryPoint("my_func");

    auto* v = b.Var("v", AddressSpace::kOut, ty.vec4f());
    v->SetColor(0);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Store(v, b.Zero(ty.vec4f()));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:42 error: var: color IO attributes cannot be declared for a fragment shader output. They can only be used for a fragment shader input.
  %v:ptr<__out, vec4<f32>, read_write> = var undef
                                         ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_MSV_Struct_Color_Output) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.color = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("col"), ty.vec4f(), attr},
                                               });

    auto* v = b.Var("v", AddressSpace::kOut, str_ty);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Store(v, b.Zero(str_ty));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:41 error: var: color IO attributes cannot be declared for a fragment shader output. They can only be used for a fragment shader input.
  %v:ptr<__out, MyStruct, read_write> = var undef
                                        ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_MSV_Color_Input_Fragment) {
    auto* f = FragmentEntryPoint("my_func");

    auto* v = b.Var("v", AddressSpace::kIn, ty.vec4f());
    v->SetColor(0);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Load(v);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_MSV_Struct_Color_Input_Fragment) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.color = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("col"), ty.vec4f(), attr},
                                               });

    auto* v = b.Var("v", AddressSpace::kIn, str_ty);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Load(v);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_MSV_Color_Input_NonFragment) {
    auto* f = VertexEntryPoint("my_func");

    auto* v = b.Var("v", AddressSpace::kIn, ty.vec4f());
    v->SetColor(0);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Load(v);
        b.Return(f, b.Zero(ty.vec4f()));
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:35 error: var: color IO attributes cannot be declared for a vertex shader input. They can only be used for a fragment shader input.
  %v:ptr<__in, vec4<f32>, read> = var undef
                                  ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_MSV_Struct_Color_Input_NonFragment) {
    auto* f = VertexEntryPoint("my_func");

    IOAttributes attr;
    attr.color = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("col"), ty.vec4f(), attr},
                                               });
    auto* v = b.Var("v", AddressSpace::kIn, str_ty);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Store(v, b.Zero(str_ty));
        b.Return(f, b.Zero(ty.vec4f()));
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:34 error: var: color IO attributes cannot be declared for a vertex shader input. They can only be used for a fragment shader input.
  %v:ptr<__in, MyStruct, read> = var undef
                                 ^^^
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

TEST_F(IR_ValidatorTest, Function_EntryPointParam_BindingPointWithoutCapability) {
    auto* f = ComputeEntryPoint("my_func");
    auto* p = b.FunctionParam("my_param", ty.ptr<uniform, i32>());
    p->SetBindingPoint(0, 0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:54 error: binding_points are only valid on resource variables
%my_func = @compute @workgroup_size(1u, 1u, 1u) func(%my_param:ptr<uniform, i32, read> [@binding_point(0, 0)]):void {
                                                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_EntryPointParam_BindingPointWithCapability) {
    auto* f = ComputeEntryPoint("my_func");
    auto* p = b.FunctionParam("my_param", ty.ptr<uniform, i32>());
    p->SetBindingPoint(0, 0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kMslAllowEntryPointInterface});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Color_InvalidType) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("my_param", ty.mat4x4(ty.f32()));
    p->SetColor(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:1:27 error: color must be a numeric scalar or vector
%my_func = @fragment func(%my_param:mat4x4<f32> [@color(0)]):void {
                          ^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_InputIndexAttachment) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.input_attachment_index = 0;
    auto* p = b.FunctionParam("p", ty.u32());
    p->SetAttributes(attr);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:27 error: input attachment index IO attributes cannot be declared for a fragment shader input. They can only be used for a fragment shader resource.
%my_func = @fragment func(%p:u32):void {
                          ^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_InputIndexAttachment) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.input_attachment_index = 0;
    f->SetReturnAttributes(attr);
    f->SetReturnType(ty.u32());

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: input attachment index IO attributes cannot be declared for a fragment shader output. They can only be used for a fragment shader resource.
%my_func = @fragment func():u32 {
^^^^^^^^
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
            R"(:1:1 error: return value has more than one IO annotation, [ @location, built-in ]
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
                                                   {mod.symbols.New("a"), ty.vec4f(), attr},
                                               });
    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: return value struct member has more than one IO annotation, [ @location, built-in ]
%my_func = @vertex func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Location_InvalidType) {
    auto* f = FragmentEntryPoint("my_func");
    f->SetReturnType(ty.bool_());
    f->SetReturnLocation(0);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: return value with a location attribute must be a numeric scalar or vector, but has type bool
%my_func = @fragment func():bool [@location(0)] {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_Location_InvalidType) {
    IOAttributes attr;
    attr.location = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.bool_(), attr},
                                               });
    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kFragment);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: return value struct member with a location attribute must be a numeric scalar or vector, but has type bool
%my_func = @fragment func():MyStruct {
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
        testing::HasSubstr(R"(:1:1 error: return value with void type should never be annotated
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
            R"(:1:1 error: return value must have at least one IO annotation, e.g. a binding point, a location, etc
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
            R"(:5:1 error: return value struct members must have at least one IO annotation, e.g. a binding point, a location, etc
%my_func = @fragment func():MyStruct {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_InvariantWithPosition) {
    auto* f = b.Function("my_func", ty.vec4f(), Function::PipelineStage::kVertex);
    f->SetReturnBuiltin(BuiltinValue::kPosition);
    f->SetReturnInvariant(true);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_InvariantWithoutPosition) {
    auto* f = b.Function("my_func", ty.vec4f());
    f->SetReturnInvariant(true);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: invariant can only decorate a value if it is also decorated with position
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
                                                   {mod.symbols.New("pos"), ty.vec4f(), attr},
                                               });

    auto* f = VertexEntryPoint("my_func");
    f->SetReturnType(str_ty);
    f->SetReturnAttributes({});
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_InvariantWithoutPosition_ViaMSV) {
    auto* f = VertexEntryPoint("my_func");

    IOAttributes attr;
    attr.invariant = true;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4f(), attr},
                                               });

    auto* v = b.Var("v", AddressSpace::kOut, str_ty);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Store(v, b.Zero(str_ty));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:6:41 error: var: invariant can only decorate a value if it is also decorated with position
  %v:ptr<__out, MyStruct, read_write> = var undef
                                        ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_InvariantWithoutPosition) {
    IOAttributes attr;
    attr.invariant = true;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4f(), attr},
                                               });

    auto* f = VertexEntryPoint("my_func");
    f->SetReturnType(str_ty);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:5:1 error: invariant can only decorate a value if it is also decorated with position
%my_func = @vertex func():MyStruct [@position] {
^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Return_InvariantWithoutPosition_ViaMSV) {
    auto* f = VertexEntryPoint("my_func");

    auto* v = b.Var("v", AddressSpace::kOut, ty.vec4f());
    v->SetInvariant(true);
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Store(v, b.Zero(ty.vec4f()));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:2:42 error: var: invariant can only decorate a value if it is also decorated with position
  %v:ptr<__out, vec4<f32>, read_write> = var undef @invariant
                                         ^^^
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

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_InvalidValueKind) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.FunctionParam("p", ty.u32()), b.Constant(3_u)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:1:1 error: @workgroup_size must be an InstructionResult or a Constant
%f = @compute @workgroup_size(1u, %p, 3u) func():void {
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

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamZero) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(0_i), b.Constant(2_i), b.Constant(3_i)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: @workgroup_size params must be greater than 0
%f = @compute @workgroup_size(0i, 2i, 3i) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamsTooLarge) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1048576_i), b.Constant(1048576_i), b.Constant(1048576_i)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: workgroup grid size cannot exceed 0xffffffff
%f = @compute @workgroup_size(1048576i, 1048576i, 1048576i) func():void {
^^
)")) << res.Failure();
}

// Test the case where the intermediate workgroup product overflows a uint64_t and wraps back around
// to be a valid uint32_t value.
TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamsTooLarge_U64Overflow) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize(
        {b.Constant(1526726656_i), b.Constant(1526726656_i), b.Constant(1526726656_i)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: workgroup grid size cannot exceed 0xffffffff
%f = @compute @workgroup_size(1526726656i, 1526726656i, 1526726656i) func():void {
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

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ModuleScopeRuntimeExpression) {
    auto* f = ComputeEntryPoint();

    auto* v = b.Var("v", ty.ptr(workgroup, ty.atomic(ty.u32())));
    mod.root_block->Append(v);

    auto* load = b.Call(ty.u32(), core::BuiltinFn::kAtomicLoad, v->Result(0));
    mod.root_block->Append(load);

    f->SetWorkgroupSize({load->Result(0), b.Constant(1_u), b.Constant(1_u)});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:3:12 error: atomicLoad: instruction is not evaluatable at pipeline creation time
  %2:u32 = atomicLoad %v
           ^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_NonCompute) {
    auto* f = FragmentEntryPoint();
    f->SetSubgroupSize(b.Constant(16_i));

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: @subgroup_size only valid on compute entry point
%f = @fragment @subgroup_size(16i) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_Nullptr) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_u), b.Constant(3_u)});
    f->SetSubgroupSize(nullptr);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:1:1 error: a @subgroup_size param must have a value
%f = @compute @workgroup_size(1u, 2u, 3u) @subgroup_size(undef) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_ParamWrongType) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_u), b.Constant(3_u)});
    f->SetSubgroupSize(b.Constant(1_f));

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(
                    R"(:1:1 error: @subgroup_size param must be an 'i32' or 'u32', received 'f32'
%f = @compute @workgroup_size(1u, 2u, 3u) @subgroup_size(1.0f) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_ParamTooSmall) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_u), b.Constant(3_u)});
    f->SetSubgroupSize(b.Constant(-16_i));

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: @subgroup_size param must be greater than 0
%f = @compute @workgroup_size(1u, 2u, 3u) @subgroup_size(-16i) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_ParamZero) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_u), b.Constant(3_u)});
    f->SetSubgroupSize(b.Constant(0_u));

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: @subgroup_size param must be greater than 0
%f = @compute @workgroup_size(1u, 2u, 3u) @subgroup_size(0u) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_ParamNonPowerOfTwo) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_u), b.Constant(3_u)});
    f->SetSubgroupSize(b.Constant(15_u));

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: @subgroup_size param must be a power of 2
%f = @compute @workgroup_size(1u, 2u, 3u) @subgroup_size(15u) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_ParamPowerOfTwo) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_u), b.Constant(3_u)});
    f->SetSubgroupSize(b.Constant(32_u));

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_OverrideWithoutAllowOverrides) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_u), b.Constant(3_u)});

    auto* o = b.Override(ty.u32());
    f->SetSubgroupSize(o->Result());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:1 error: @subgroup_size param is not a constant value, and IR capability 'kAllowOverrides' is not set
%f = @compute @workgroup_size(1u, 2u, 3u) @subgroup_size(%2) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_NonRootBlockOverride) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_u), b.Constant(3_u)});

    Override* o;
    b.Append(f->Block(), [&] {
        o = b.Override(ty.u32());
        b.Return(f);
    });
    f->SetSubgroupSize(o->Result());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(R"(:1:1 error: @subgroup_size param defined by non-module scope value
%f = @compute @workgroup_size(1u, 2u, 3u) @subgroup_size(%2) func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_SubgroupSize_RootBlockOverride) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_u), b.Constant(3_u)});

    auto* o = b.Override(ty.u32());
    o->SetOverrideId(OverrideId{1});
    mod.root_block->Append(o);
    f->SetSubgroupSize(o->Result());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowOverrides});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_Vertex_BasicPosition) {
    auto* f = b.Function("my_func", ty.vec4f(), Function::PipelineStage::kVertex);
    f->SetReturnBuiltin(BuiltinValue::kPosition);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Vertex_StructPosition) {
    auto pos_ty = ty.vec4f();
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
    auto pos_ty = ty.vec4f();
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
    auto* f = b.Function("my_func", ty.vec4f(), Function::PipelineStage::kVertex);
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
                testing::HasSubstr(R"(:6:1 error: entry point returns can not be 'bool'
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
    v->SetLocation(0);
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
            R"(:2:37 error: var: IO address space values referenced by shader entry points can only be 'bool' if in the input space, used only by fragment shaders and decorated with @builtin(front_facing)
  %1:ptr<__out, bool, read_write> = var undef @location(0)
                                    ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_BoolInputWithoutFrontFacing_via_MSV) {
    auto* f = FragmentEntryPoint();

    auto* invalid = b.Var("invalid", AddressSpace::kIn, ty.bool_());
    invalid->SetLocation(0);
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
            R"(:2:36 error: var: input address space values referenced by fragment shaders can only be 'bool' if decorated with @builtin(front_facing)
  %invalid:ptr<__in, bool, read> = var undef @location(0)
                                   ^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_EntryPoint_PtrToWorkgroup) {
    auto* f = FragmentEntryPoint();
    auto* p = b.FunctionParam("invalid", ty.ptr<workgroup, i32>());
    f->AppendParam(p);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(
        res.Failure().reason,
        testing::HasSubstr(
            R"(:1:21 error: input param to entry point cannot be a ptr in the 'workgroup' address space
%f = @fragment func(%invalid:ptr<workgroup, i32, read_write>):void {
                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_DirectRecursion) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: recursive function calls are not allowed
%f = func():void {
^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_IndirectRecursion) {
    auto* f1 = b.Function("f1", ty.void_());
    auto* f2 = b.Function("f2", ty.void_());
    auto* f3 = b.Function("f3", ty.void_());

    b.Append(f1->Block(), [&] {
        b.Call(f2);
        b.Return(f1);
    });
    b.Append(f2->Block(), [&] {
        b.Call(f3);
        b.Return(f2);
    });
    b.Append(f3->Block(), [&] {
        b.Call(f1);
        b.Return(f3);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason,
                testing::HasSubstr(R"(:1:1 error: recursive function calls are not allowed
%f1 = func():void {
^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_ParamPixelLocal) {
    auto* f = FragmentEntryPoint();
    auto* p = b.FunctionParam("invalid", ty.ptr<core::AddressSpace::kPixelLocal>(ty.i32()));
    f->AppendParam(p);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:1:21 error: pixel_local param must be of type struct
%f = @fragment func(%invalid:ptr<pixel_local, i32, read_write>):void {
                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)")) << res.Failure();
}

TEST_F(IR_ValidatorTest, Function_Param_Color_F16) {
    auto* f = FragmentEntryPoint("my_func");
    auto* p = b.FunctionParam("my_param", ty.f16());
    p->SetColor(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowNonCoreTypes});
    EXPECT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_Param_Color_Bool) {
    auto* f = FragmentEntryPoint("my_func");
    auto* p = b.FunctionParam("my_param", ty.bool_());
    p->SetColor(0);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason, testing::HasSubstr(
                                          R"(:1:27 error: color must be a numeric scalar or vector
%my_func = @fragment func(%my_param:bool [@color(0)]):void {
                          ^^^^^^^^^^^^^^
)")) << res.Failure();
}

}  // namespace tint::core::ir
