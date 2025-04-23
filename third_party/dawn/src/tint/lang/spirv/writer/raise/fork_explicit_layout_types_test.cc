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

#include "src/tint/lang/spirv/writer/raise/fork_explicit_layout_types.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_ForkExplicitLayoutTypesTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, NoModify_Struct_NotInHostShareable) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), ty.u32()},
                                                             });

    auto* wg_buffer = b.Var("wg_buffer", ty.ptr(workgroup, structure));
    mod.root_block->Append(wg_buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("let", b.Load(wg_buffer));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %wg_buffer:ptr<workgroup, MyStruct, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:MyStruct = load %wg_buffer
    %let:MyStruct = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(src, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, NoModify_Array_NotInHostShareable) {
    auto* wg_buffer = b.Var("wg_buffer", ty.ptr<workgroup, array<u32, 4>>());
    mod.root_block->Append(wg_buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Let("let", b.Load(wg_buffer));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %wg_buffer:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %3:array<u32, 4> = load %wg_buffer
    %let:array<u32, 4> = let %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(src, str());
}

// Test that we always add the struct flag to structures that require explicit layout decorations,
// even if we are not forking them.
TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, Struct_InHostShareable_NotShared) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    // Sharing with a function parameter shouldn't cause rewrite.
    auto* foo = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("param", structure);
    foo->AppendParam(param);
    b.Append(foo->Block(), [&] { b.Return(foo); });

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        // Sharing with a let shouldn't cause rewrite.
        b.Let("let", b.Load(buffer));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%param:MyStruct):void {
  $B2: {
    ret
  }
}
%foo_1 = func():void {  # %foo_1: 'foo'
  $B3: {
    %5:MyStruct = load %buffer
    %let:MyStruct = let %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%param:MyStruct):void {
  $B2: {
    ret
  }
}
%foo_1 = func():void {  # %foo_1: 'foo'
  $B3: {
    %5:MyStruct = load %buffer
    %let:MyStruct = let %5
    ret
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

// Test that we always modify arrays that require explicit layout decorations, since the type is
// used to signal to the printer that layout decorations are required.
TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, Array_InHostShareable_NotShared) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<u32, 4>>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, PushConstant_SharedWithPrivate) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                             });

    mod.root_block->Append(b.Var("buffer", ty.ptr(push_constant, structure)));
    mod.root_block->Append(b.Var("local", ty.ptr(private_, structure)));

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<push_constant, MyStruct, read> = var undef
  %local:ptr<private, MyStruct, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<push_constant, MyStruct_tint_explicit_layout, read> = var undef
  %local:ptr<private, MyStruct, read_write> = var undef
}

)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, Storage_SharedWithPrivate) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);
    mod.root_block->Append(b.Var("local", ty.ptr(private_, structure)));

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
  %local:ptr<private, MyStruct, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
  %local:ptr<private, MyStruct, read_write> = var undef
}

)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, Uniform_SharedWithPrivate) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(uniform, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);
    mod.root_block->Append(b.Var("local", ty.ptr(private_, structure)));

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct, read> = var undef @binding_point(0, 0)
  %local:ptr<private, MyStruct, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<uniform, MyStruct_tint_explicit_layout, read> = var undef @binding_point(0, 0)
  %local:ptr<private, MyStruct, read_write> = var undef
}

)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, Storage_SharedWithWorkgroup) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);
    mod.root_block->Append(b.Var("local", ty.ptr(workgroup, structure)));

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
  %local:ptr<workgroup, MyStruct, read_write> = var undef
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
  %local:ptr<workgroup, MyStruct, read_write> = var undef
}

)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, Storage_SharedWithInOut) {
    core::IOAttributes attributes;
    attributes.builtin = core::BuiltinValue::kSampleMask;
    auto* array = ty.array<u32, 1>();
    b.Append(mod.root_block, [&] {
        auto* buffer = b.Var("buffer", ty.ptr(storage, array));
        buffer->SetBindingPoint(0, 0);
        auto* mask_in = b.Var("mask_in", ty.ptr(core::AddressSpace::kIn, array));
        mask_in->SetAttributes(attributes);
        auto* mask_out = b.Var("mask_out", ty.ptr(core::AddressSpace::kOut, array));
        mask_out->SetAttributes(attributes);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32, 1>, read_write> = var undef @binding_point(0, 0)
  %mask_in:ptr<__in, array<u32, 1>, read> = var undef @builtin(sample_mask)
  %mask_out:ptr<__out, array<u32, 1>, read_write> = var undef @builtin(sample_mask)
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, spirv.explicit_layout_array<u32, 1>, read_write> = var undef @binding_point(0, 0)
  %mask_in:ptr<__in, array<u32, 1>, read> = var undef @builtin(sample_mask)
  %mask_out:ptr<__out, array<u32, 1>, read_write> = var undef @builtin(sample_mask)
}

)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, Storage_SharedWithFunction) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", b.Zero(structure));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var MyStruct(0u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var MyStruct(0u)
    ret
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, LoadFromStorage_Struct_Shared) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", b.Load(buffer));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:MyStruct = load %buffer
    %local:ptr<function, MyStruct, read_write> = var %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:MyStruct_tint_explicit_layout = load %buffer
    %4:MyStruct = call %tint_convert_explicit_layout, %3
    %local:ptr<function, MyStruct, read_write> = var %4
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:MyStruct_tint_explicit_layout):MyStruct {
  $B3: {
    %8:u32 = access %tint_source, 0u
    %9:u32 = access %tint_source, 1u
    %10:MyStruct = construct %8, %9
    ret %10
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, LoadFromStorage_NestedStruct_Shared) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.u32()},
                                                          {mod.symbols.New("b"), ty.u32()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("a"), inner},
                                                          {mod.symbols.New("b"), inner},
                                                      });

    auto* buffer = b.Var("buffer", ty.ptr(storage, outer));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", b.Load(buffer));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  a_1:Inner @offset(0)
  b_1:Inner @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:Outer = load %buffer
    %local:ptr<function, Outer, read_write> = var %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  a_1:Inner @offset(0)
  b_1:Inner @offset(8)
}

Inner_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a_1:Inner_tint_explicit_layout @offset(0)
  b_1:Inner_tint_explicit_layout @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:Outer_tint_explicit_layout = load %buffer
    %4:Outer = call %tint_convert_explicit_layout, %3
    %local:ptr<function, Outer, read_write> = var %4
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:Outer_tint_explicit_layout):Outer {
  $B3: {
    %8:Inner_tint_explicit_layout = access %tint_source, 0u
    %9:Inner = call %tint_convert_explicit_layout_1, %8
    %11:Inner_tint_explicit_layout = access %tint_source, 1u
    %12:Inner = call %tint_convert_explicit_layout_1, %11
    %13:Outer = construct %9, %12
    ret %13
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:Inner_tint_explicit_layout):Inner {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B4: {
    %15:u32 = access %tint_source_1, 0u
    %16:u32 = access %tint_source_1, 1u
    %17:Inner = construct %15, %16
    ret %17
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, StoreToStorage_Struct_Shared) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure, read_write));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* local = b.Var("local", ty.ptr<function>(structure));
        b.Store(buffer, b.Load(local));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var undef
    %4:MyStruct = load %local
    store %buffer, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var undef
    %4:MyStruct = load %local
    %5:MyStruct_tint_explicit_layout = call %tint_convert_explicit_layout, %4
    store %buffer, %5
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:MyStruct):MyStruct_tint_explicit_layout {
  $B3: {
    %8:u32 = access %tint_source, 0u
    %9:u32 = access %tint_source, 1u
    %10:MyStruct_tint_explicit_layout = construct %8, %9
    ret %10
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, StoreToStorage_NestedStruct_Shared) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.u32()},
                                                          {mod.symbols.New("b"), ty.u32()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("a"), inner},
                                                          {mod.symbols.New("b"), inner},
                                                      });

    auto* buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* local = b.Var("local", ty.ptr<function>(outer));
        b.Store(buffer, b.Load(local));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  a_1:Inner @offset(0)
  b_1:Inner @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, Outer, read_write> = var undef
    %4:Outer = load %local
    store %buffer, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  a_1:Inner @offset(0)
  b_1:Inner @offset(8)
}

Inner_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a_1:Inner_tint_explicit_layout @offset(0)
  b_1:Inner_tint_explicit_layout @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, Outer, read_write> = var undef
    %4:Outer = load %local
    %5:Outer_tint_explicit_layout = call %tint_convert_explicit_layout, %4
    store %buffer, %5
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:Outer):Outer_tint_explicit_layout {
  $B3: {
    %8:Inner = access %tint_source, 0u
    %9:Inner_tint_explicit_layout = call %tint_convert_explicit_layout_1, %8
    %11:Inner = access %tint_source, 1u
    %12:Inner_tint_explicit_layout = call %tint_convert_explicit_layout_1, %11
    %13:Outer_tint_explicit_layout = construct %9, %12
    ret %13
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:Inner):Inner_tint_explicit_layout {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B4: {
    %15:u32 = access %tint_source_1, 0u
    %16:u32 = access %tint_source_1, 1u
    %17:Inner_tint_explicit_layout = construct %15, %16
    ret %17
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, InnerStructShared_OuterStructNotShared) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.u32()},
                                                          {mod.symbols.New("b"), ty.u32()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("a"), inner},
                                                          {mod.symbols.New("b"), inner},
                                                      });

    auto* buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* local = b.Var("local", ty.ptr<function>(inner));
        auto* load_inner = b.Load(local);
        b.Store(buffer, b.Construct(outer, load_inner, load_inner));
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  a_1:Inner @offset(0)
  b_1:Inner @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, Inner, read_write> = var undef
    %4:Inner = load %local
    %5:Outer = construct %4, %4
    store %buffer, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  a_1:Inner @offset(0)
  b_1:Inner @offset(8)
}

Inner_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a_1:Inner_tint_explicit_layout @offset(0)
  b_1:Inner_tint_explicit_layout @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, Inner, read_write> = var undef
    %4:Inner = load %local
    %5:Outer = construct %4, %4
    %6:Outer_tint_explicit_layout = call %tint_convert_explicit_layout, %5
    store %buffer, %6
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:Outer):Outer_tint_explicit_layout {
  $B3: {
    %9:Inner = access %tint_source, 0u
    %10:Inner_tint_explicit_layout = call %tint_convert_explicit_layout_1, %9
    %12:Inner = access %tint_source, 1u
    %13:Inner_tint_explicit_layout = call %tint_convert_explicit_layout_1, %12
    %14:Outer_tint_explicit_layout = construct %10, %13
    ret %14
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:Inner):Inner_tint_explicit_layout {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B4: {
    %16:u32 = access %tint_source_1, 0u
    %17:u32 = access %tint_source_1, 1u
    %18:Inner_tint_explicit_layout = construct %16, %17
    ret %18
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest,
       OuterStructNotShared_OneInnerStructShared_OneInnerStructNotShared) {
    auto* inner_shared =
        ty.Struct(mod.symbols.New("InnerShared"), {
                                                      {mod.symbols.New("a"), ty.u32()},
                                                      {mod.symbols.New("b"), ty.u32()},
                                                  });
    auto* inner_not_shared =
        ty.Struct(mod.symbols.New("InnerNotShared"), {
                                                         {mod.symbols.New("a"), ty.u32()},
                                                         {mod.symbols.New("b"), ty.u32()},
                                                     });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("a"), inner_shared},
                                                          {mod.symbols.New("b"), inner_not_shared},
                                                      });

    auto* buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", ty.ptr<function>(inner_shared));
        b.Store(buffer, b.Construct(outer, b.Zero(inner_shared), b.Zero(inner_not_shared)));
        b.Return(func);
    });

    auto* src = R"(
InnerShared = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

InnerNotShared = struct @align(4) {
  a_1:u32 @offset(0)
  b_1:u32 @offset(4)
}

Outer = struct @align(4) {
  a_2:InnerShared @offset(0)
  b_2:InnerNotShared @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, InnerShared, read_write> = var undef
    %4:Outer = construct InnerShared(0u), InnerNotShared(0u)
    store %buffer, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
InnerShared = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

InnerNotShared = struct @align(4), @spirv.explicit_layout {
  a_1:u32 @offset(0)
  b_1:u32 @offset(4)
}

Outer = struct @align(4) {
  a_2:InnerShared @offset(0)
  b_2:InnerNotShared @offset(8)
}

InnerShared_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a_2:InnerShared_tint_explicit_layout @offset(0)
  b_2:InnerNotShared @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, InnerShared, read_write> = var undef
    %4:Outer = construct InnerShared(0u), InnerNotShared(0u)
    %5:Outer_tint_explicit_layout = call %tint_convert_explicit_layout, %4
    store %buffer, %5
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:Outer):Outer_tint_explicit_layout {
  $B3: {
    %8:InnerShared = access %tint_source, 0u
    %9:InnerShared_tint_explicit_layout = call %tint_convert_explicit_layout_1, %8
    %11:InnerNotShared = access %tint_source, 1u
    %12:Outer_tint_explicit_layout = construct %9, %11
    ret %12
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:InnerShared):InnerShared_tint_explicit_layout {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B4: {
    %14:u32 = access %tint_source_1, 0u
    %15:u32 = access %tint_source_1, 1u
    %16:InnerShared_tint_explicit_layout = construct %14, %15
    ret %16
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, SharedStruct_MultipleLoadsAndStores) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* let = b.Let("let", b.Load(buffer));
        b.Var("local", b.Load(buffer));
        b.Store(buffer, b.Zero(structure));
        b.Store(buffer, let);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:MyStruct = load %buffer
    %let:MyStruct = let %3
    %5:MyStruct = load %buffer
    %local:ptr<function, MyStruct, read_write> = var %5
    store %buffer, MyStruct(0u)
    store %buffer, %let
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:MyStruct_tint_explicit_layout = load %buffer
    %4:MyStruct = call %tint_convert_explicit_layout, %3
    %let:MyStruct = let %4
    %7:MyStruct_tint_explicit_layout = load %buffer
    %8:MyStruct = call %tint_convert_explicit_layout, %7
    %local:ptr<function, MyStruct, read_write> = var %8
    %10:MyStruct_tint_explicit_layout = call %tint_convert_explicit_layout_1, MyStruct(0u)
    store %buffer, %10
    %12:MyStruct_tint_explicit_layout = call %tint_convert_explicit_layout_1, %let
    store %buffer, %12
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:MyStruct_tint_explicit_layout):MyStruct {
  $B3: {
    %14:u32 = access %tint_source, 0u
    %15:u32 = access %tint_source, 1u
    %16:MyStruct = construct %14, %15
    ret %16
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:MyStruct):MyStruct_tint_explicit_layout {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B4: {
    %18:u32 = access %tint_source_1, 0u
    %19:u32 = access %tint_source_1, 1u
    %20:MyStruct_tint_explicit_layout = construct %18, %19
    ret %20
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, SharedStruct_UsesViaLet) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", ty.ptr(function, structure));
        auto* let_ptr = b.Let("let_ptr", buffer);
        auto* load = b.Load(let_ptr);
        b.Store(let_ptr, load);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var undef
    %let_ptr:ptr<storage, MyStruct, read_write> = let %buffer
    %5:MyStruct = load %let_ptr
    store %let_ptr, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var undef
    %let_ptr:ptr<storage, MyStruct_tint_explicit_layout, read_write> = let %buffer
    %5:MyStruct_tint_explicit_layout = load %let_ptr
    %6:MyStruct = call %tint_convert_explicit_layout, %5
    %8:MyStruct_tint_explicit_layout = call %tint_convert_explicit_layout_1, %6
    store %let_ptr, %8
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:MyStruct_tint_explicit_layout):MyStruct {
  $B3: {
    %11:u32 = access %tint_source, 0u
    %12:u32 = access %tint_source, 1u
    %13:MyStruct = construct %11, %12
    ret %13
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:MyStruct):MyStruct_tint_explicit_layout {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B4: {
    %15:u32 = access %tint_source_1, 0u
    %16:u32 = access %tint_source_1, 1u
    %17:MyStruct_tint_explicit_layout = construct %15, %16
    ret %17
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, SharedStruct_AccessScalarMember) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), ty.u32()},
                                                             });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure, read_write));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", ty.ptr<function>(structure));
        auto* load = b.Load(b.Access(ty.ptr<storage, u32, read_write>(), buffer, 0_u));
        b.Store(b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_u), load);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var undef
    %4:ptr<storage, u32, read_write> = access %buffer, 0u
    %5:u32 = load %4
    %6:ptr<storage, u32, read_write> = access %buffer, 1u
    store %6, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var undef
    %4:ptr<storage, u32, read_write> = access %buffer, 0u
    %5:u32 = load %4
    %6:ptr<storage, u32, read_write> = access %buffer, 1u
    store %6, %5
    ret
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, SharedStruct_AccessStructMember) {
    auto* inner = ty.Struct(mod.symbols.New("Inner"), {
                                                          {mod.symbols.New("a"), ty.u32()},
                                                          {mod.symbols.New("b"), ty.u32()},
                                                      });
    auto* outer = ty.Struct(mod.symbols.New("Outer"), {
                                                          {mod.symbols.New("a"), inner},
                                                          {mod.symbols.New("b"), inner},
                                                      });

    auto* buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", ty.ptr<function>(outer));
        auto* load = b.Load(b.Access(ty.ptr(storage, inner, read_write), buffer, 0_u));
        b.Store(b.Access(ty.ptr(storage, inner, read_write), buffer, 1_u), load);
        b.Return(func);
    });

    auto* src = R"(
Inner = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  a_1:Inner @offset(0)
  b_1:Inner @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, Outer, read_write> = var undef
    %4:ptr<storage, Inner, read_write> = access %buffer, 0u
    %5:Inner = load %4
    %6:ptr<storage, Inner, read_write> = access %buffer, 1u
    store %6, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
Inner = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer = struct @align(4) {
  a_1:Inner @offset(0)
  b_1:Inner @offset(8)
}

Inner_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

Outer_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a_1:Inner_tint_explicit_layout @offset(0)
  b_1:Inner_tint_explicit_layout @offset(8)
}

$B1: {  # root
  %buffer:ptr<storage, Outer_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, Outer, read_write> = var undef
    %4:ptr<storage, Inner_tint_explicit_layout, read_write> = access %buffer, 0u
    %5:Inner_tint_explicit_layout = load %4
    %6:Inner = call %tint_convert_explicit_layout, %5
    %8:ptr<storage, Inner_tint_explicit_layout, read_write> = access %buffer, 1u
    %9:Inner_tint_explicit_layout = call %tint_convert_explicit_layout_1, %6
    store %8, %9
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:Inner_tint_explicit_layout):Inner {
  $B3: {
    %12:u32 = access %tint_source, 0u
    %13:u32 = access %tint_source, 1u
    %14:Inner = construct %12, %13
    ret %14
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:Inner):Inner_tint_explicit_layout {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B4: {
    %16:u32 = access %tint_source_1, 0u
    %17:u32 = access %tint_source_1, 1u
    %18:Inner_tint_explicit_layout = construct %16, %17
    ret %18
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, SharedStruct_MultipleVars) {
    auto* structure = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), ty.u32()},
                                                             });

    auto* buffer_0 = b.Var("buffer_0", ty.ptr(storage, structure));
    auto* buffer_1 = b.Var("buffer_1", ty.ptr(storage, structure));
    auto* buffer_2 = b.Var("buffer_2", ty.ptr(storage, structure));
    buffer_0->SetBindingPoint(0, 0);
    buffer_1->SetBindingPoint(0, 1);
    buffer_2->SetBindingPoint(0, 2);
    mod.root_block->Append(buffer_0);
    mod.root_block->Append(buffer_1);
    mod.root_block->Append(buffer_2);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", b.Zero(structure));
        b.Let("let_0", b.Load(buffer_0));
        b.Let("let_1", b.Load(buffer_1));
        b.Let("let_2", b.Load(buffer_2));
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer_0:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
  %buffer_1:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 1)
  %buffer_2:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 2)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var MyStruct(0u)
    %6:MyStruct = load %buffer_0
    %let_0:MyStruct = let %6
    %8:MyStruct = load %buffer_1
    %let_1:MyStruct = let %8
    %10:MyStruct = load %buffer_2
    %let_2:MyStruct = let %10
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %buffer_0:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
  %buffer_1:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 1)
  %buffer_2:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 2)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, MyStruct, read_write> = var MyStruct(0u)
    %6:MyStruct_tint_explicit_layout = load %buffer_0
    %7:MyStruct = call %tint_convert_explicit_layout, %6
    %let_0:MyStruct = let %7
    %10:MyStruct_tint_explicit_layout = load %buffer_1
    %11:MyStruct = call %tint_convert_explicit_layout, %10
    %let_1:MyStruct = let %11
    %13:MyStruct_tint_explicit_layout = load %buffer_2
    %14:MyStruct = call %tint_convert_explicit_layout, %13
    %let_2:MyStruct = let %14
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:MyStruct_tint_explicit_layout):MyStruct {
  $B3: {
    %17:u32 = access %tint_source, 0u
    %18:u32 = access %tint_source, 1u
    %19:MyStruct = construct %17, %18
    ret %19
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, MultipleSharedStructs) {
    auto* s0 = ty.Struct(mod.symbols.New("S_0"), {
                                                     {mod.symbols.New("a"), ty.u32()},
                                                     {mod.symbols.New("b"), ty.u32()},
                                                 });
    auto* s1 = ty.Struct(mod.symbols.New("S_1"), {
                                                     {mod.symbols.New("c"), ty.f32()},
                                                     {mod.symbols.New("d"), ty.f32()},
                                                 });
    auto* s2 = ty.Struct(mod.symbols.New("S_2"), {
                                                     {mod.symbols.New("e"), ty.i32()},
                                                     {mod.symbols.New("f"), ty.i32()},
                                                 });

    auto* buffer_0 = b.Var("buffer_0", ty.ptr(storage, s0));
    auto* buffer_1 = b.Var("buffer_1", ty.ptr(storage, s1));
    auto* buffer_2 = b.Var("buffer_2", ty.ptr(storage, s2));
    buffer_0->SetBindingPoint(0, 0);
    buffer_1->SetBindingPoint(0, 1);
    buffer_2->SetBindingPoint(0, 2);
    mod.root_block->Append(buffer_0);
    mod.root_block->Append(buffer_1);
    mod.root_block->Append(buffer_2);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", b.Zero(s0));
        b.Var("local", b.Zero(s1));
        b.Var("local", b.Zero(s2));
        b.Let("let_0", b.Load(buffer_0));
        b.Let("let_1", b.Load(buffer_1));
        b.Let("let_2", b.Load(buffer_2));
        b.Return(func);
    });

    auto* src = R"(
S_0 = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

S_1 = struct @align(4) {
  c:f32 @offset(0)
  d:f32 @offset(4)
}

S_2 = struct @align(4) {
  e:i32 @offset(0)
  f:i32 @offset(4)
}

$B1: {  # root
  %buffer_0:ptr<storage, S_0, read_write> = var undef @binding_point(0, 0)
  %buffer_1:ptr<storage, S_1, read_write> = var undef @binding_point(0, 1)
  %buffer_2:ptr<storage, S_2, read_write> = var undef @binding_point(0, 2)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, S_0, read_write> = var S_0(0u)
    %local_1:ptr<function, S_1, read_write> = var S_1(0.0f)  # %local_1: 'local'
    %local_2:ptr<function, S_2, read_write> = var S_2(0i)  # %local_2: 'local'
    %8:S_0 = load %buffer_0
    %let_0:S_0 = let %8
    %10:S_1 = load %buffer_1
    %let_1:S_1 = let %10
    %12:S_2 = load %buffer_2
    %let_2:S_2 = let %12
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S_0 = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

S_1 = struct @align(4) {
  c:f32 @offset(0)
  d:f32 @offset(4)
}

S_2 = struct @align(4) {
  e:i32 @offset(0)
  f:i32 @offset(4)
}

S_0_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

S_1_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  c:f32 @offset(0)
  d:f32 @offset(4)
}

S_2_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  e:i32 @offset(0)
  f:i32 @offset(4)
}

$B1: {  # root
  %buffer_0:ptr<storage, S_0_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
  %buffer_1:ptr<storage, S_1_tint_explicit_layout, read_write> = var undef @binding_point(0, 1)
  %buffer_2:ptr<storage, S_2_tint_explicit_layout, read_write> = var undef @binding_point(0, 2)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, S_0, read_write> = var S_0(0u)
    %local_1:ptr<function, S_1, read_write> = var S_1(0.0f)  # %local_1: 'local'
    %local_2:ptr<function, S_2, read_write> = var S_2(0i)  # %local_2: 'local'
    %8:S_0_tint_explicit_layout = load %buffer_0
    %9:S_0 = call %tint_convert_explicit_layout, %8
    %let_0:S_0 = let %9
    %12:S_1_tint_explicit_layout = load %buffer_1
    %13:S_1 = call %tint_convert_explicit_layout_1, %12
    %let_1:S_1 = let %13
    %16:S_2_tint_explicit_layout = load %buffer_2
    %17:S_2 = call %tint_convert_explicit_layout_2, %16
    %let_2:S_2 = let %17
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:S_0_tint_explicit_layout):S_0 {
  $B3: {
    %21:u32 = access %tint_source, 0u
    %22:u32 = access %tint_source, 1u
    %23:S_0 = construct %21, %22
    ret %23
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:S_1_tint_explicit_layout):S_1 {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B4: {
    %25:f32 = access %tint_source_1, 0u
    %26:f32 = access %tint_source_1, 1u
    %27:S_1 = construct %25, %26
    ret %27
  }
}
%tint_convert_explicit_layout_2 = func(%tint_source_2:S_2_tint_explicit_layout):S_2 {  # %tint_convert_explicit_layout_2: 'tint_convert_explicit_layout', %tint_source_2: 'tint_source'
  $B5: {
    %29:i32 = access %tint_source_2, 0u
    %30:i32 = access %tint_source_2, 1u
    %31:S_2 = construct %29, %30
    ret %31
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, LoadFromStorage_Array_Shared) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<u32, 4>>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", b.Load(buffer));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:array<u32, 4> = load %buffer
    %local:ptr<function, array<u32, 4>, read_write> = var %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:spirv.explicit_layout_array<u32, 4> = load %buffer
    %4:array<u32, 4> = call %tint_convert_explicit_layout, %3
    %local:ptr<function, array<u32, 4>, read_write> = var %4
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:spirv.explicit_layout_array<u32, 4>):array<u32, 4> {
  $B3: {
    %8:ptr<function, array<u32, 4>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %10:bool = gte %idx, 4u
        if %10 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %11:u32 = access %tint_source, %idx
        %12:ptr<function, u32, read_write> = access %8, %idx
        store %12, %11
        continue  # -> $B6
      }
      $B6: {  # continuing
        %13:u32 = add %idx, 1u
        next_iteration %13  # -> $B5
      }
    }
    %14:array<u32, 4> = load %8
    ret %14
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, LoadFromStorage_NestedArray_Shared) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<array<u32, 4>, 3>>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", b.Load(buffer));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<array<u32, 4>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:array<array<u32, 4>, 3> = load %buffer
    %local:ptr<function, array<array<u32, 4>, 3>, read_write> = var %3
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 4>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 4>, 3> = load %buffer
    %4:array<array<u32, 4>, 3> = call %tint_convert_explicit_layout, %3
    %local:ptr<function, array<array<u32, 4>, 3>, read_write> = var %4
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 4>, 3>):array<array<u32, 4>, 3> {
  $B3: {
    %8:ptr<function, array<array<u32, 4>, 3>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %10:bool = gte %idx, 3u
        if %10 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %11:spirv.explicit_layout_array<u32, 4> = access %tint_source, %idx
        %12:array<u32, 4> = call %tint_convert_explicit_layout_1, %11
        %14:ptr<function, array<u32, 4>, read_write> = access %8, %idx
        store %14, %12
        continue  # -> $B6
      }
      $B6: {  # continuing
        %15:u32 = add %idx, 1u
        next_iteration %15  # -> $B5
      }
    }
    %16:array<array<u32, 4>, 3> = load %8
    ret %16
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:spirv.explicit_layout_array<u32, 4>):array<u32, 4> {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B8: {
    %18:ptr<function, array<u32, 4>, read_write> = var undef
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %20:bool = gte %idx_1, 4u
        if %20 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %21:u32 = access %tint_source_1, %idx_1
        %22:ptr<function, u32, read_write> = access %18, %idx_1
        store %22, %21
        continue  # -> $B11
      }
      $B11: {  # continuing
        %23:u32 = add %idx_1, 1u
        next_iteration %23  # -> $B10
      }
    }
    %24:array<u32, 4> = load %18
    ret %24
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, StoreToStorage_Array_Shared) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<u32, 4>, read_write>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* local = b.Var("local", ty.ptr<function, array<u32, 4>>());
        b.Store(buffer, b.Load(local));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<u32, 4>, read_write> = var undef
    %4:array<u32, 4> = load %local
    store %buffer, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<u32, 4>, read_write> = var undef
    %4:array<u32, 4> = load %local
    %5:spirv.explicit_layout_array<u32, 4> = call %tint_convert_explicit_layout, %4
    store %buffer, %5
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:array<u32, 4>):spirv.explicit_layout_array<u32, 4> {
  $B3: {
    %8:ptr<function, spirv.explicit_layout_array<u32, 4>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %10:bool = gte %idx, 4u
        if %10 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %11:u32 = access %tint_source, %idx
        %12:ptr<function, u32, read_write> = access %8, %idx
        store %12, %11
        continue  # -> $B6
      }
      $B6: {  # continuing
        %13:u32 = add %idx, 1u
        next_iteration %13  # -> $B5
      }
    }
    %14:spirv.explicit_layout_array<u32, 4> = load %8
    ret %14
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, StoreToStorage_NestedArray_Shared) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, array<array<u32, 4>, 3>, read_write>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* local = b.Var("local", ty.ptr<function, array<array<u32, 4>, 3>>());
        b.Store(buffer, b.Load(local));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<array<u32, 4>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<array<u32, 4>, 3>, read_write> = var undef
    %4:array<array<u32, 4>, 3> = load %local
    store %buffer, %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 4>, 3>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<array<u32, 4>, 3>, read_write> = var undef
    %4:array<array<u32, 4>, 3> = load %local
    %5:spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 4>, 3> = call %tint_convert_explicit_layout, %4
    store %buffer, %5
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:array<array<u32, 4>, 3>):spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 4>, 3> {
  $B3: {
    %8:ptr<function, spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 4>, 3>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %10:bool = gte %idx, 3u
        if %10 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %11:array<u32, 4> = access %tint_source, %idx
        %12:spirv.explicit_layout_array<u32, 4> = call %tint_convert_explicit_layout_1, %11
        %14:ptr<function, spirv.explicit_layout_array<u32, 4>, read_write> = access %8, %idx
        store %14, %12
        continue  # -> $B6
      }
      $B6: {  # continuing
        %15:u32 = add %idx, 1u
        next_iteration %15  # -> $B5
      }
    }
    %16:spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 4>, 3> = load %8
    ret %16
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:array<u32, 4>):spirv.explicit_layout_array<u32, 4> {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B8: {
    %18:ptr<function, spirv.explicit_layout_array<u32, 4>, read_write> = var undef
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %20:bool = gte %idx_1, 4u
        if %20 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %21:u32 = access %tint_source_1, %idx_1
        %22:ptr<function, u32, read_write> = access %18, %idx_1
        store %22, %21
        continue  # -> $B11
      }
      $B11: {  # continuing
        %23:u32 = add %idx_1, 1u
        next_iteration %23  # -> $B10
      }
    }
    %24:spirv.explicit_layout_array<u32, 4> = load %18
    ret %24
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, SharedArray_UsesViaLet) {
    auto* array = ty.array<u32, 4>();
    auto* buffer = b.Var("buffer", ty.ptr(storage, array));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", ty.ptr(function, array));
        auto* let_ptr = b.Let("let_ptr", buffer);
        auto* load = b.Load(let_ptr);
        b.Store(let_ptr, load);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<u32, 4>, read_write> = var undef
    %let_ptr:ptr<storage, array<u32, 4>, read_write> = let %buffer
    %5:array<u32, 4> = load %let_ptr
    store %let_ptr, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<u32, 4>, read_write> = var undef
    %let_ptr:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = let %buffer
    %5:spirv.explicit_layout_array<u32, 4> = load %let_ptr
    %6:array<u32, 4> = call %tint_convert_explicit_layout, %5
    %8:spirv.explicit_layout_array<u32, 4> = call %tint_convert_explicit_layout_1, %6
    store %let_ptr, %8
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:spirv.explicit_layout_array<u32, 4>):array<u32, 4> {
  $B3: {
    %11:ptr<function, array<u32, 4>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %13:bool = gte %idx, 4u
        if %13 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %14:u32 = access %tint_source, %idx
        %15:ptr<function, u32, read_write> = access %11, %idx
        store %15, %14
        continue  # -> $B6
      }
      $B6: {  # continuing
        %16:u32 = add %idx, 1u
        next_iteration %16  # -> $B5
      }
    }
    %17:array<u32, 4> = load %11
    ret %17
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:array<u32, 4>):spirv.explicit_layout_array<u32, 4> {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B8: {
    %19:ptr<function, spirv.explicit_layout_array<u32, 4>, read_write> = var undef
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %21:bool = gte %idx_1, 4u
        if %21 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %22:u32 = access %tint_source_1, %idx_1
        %23:ptr<function, u32, read_write> = access %19, %idx_1
        store %23, %22
        continue  # -> $B11
      }
      $B11: {  # continuing
        %24:u32 = add %idx_1, 1u
        next_iteration %24  # -> $B10
      }
    }
    %25:spirv.explicit_layout_array<u32, 4> = load %19
    ret %25
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, SharedArray_AccessScalarMember) {
    auto* array = ty.array<u32, 4>();
    auto* buffer = b.Var("buffer", ty.ptr(storage, array, read_write));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", ty.ptr<function>(array));
        auto* load = b.Load(b.Access(ty.ptr<storage, u32, read_write>(), buffer, 1_u));
        b.Store(b.Access(ty.ptr<storage, u32, read_write>(), buffer, 2_u), load);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<u32, 4>, read_write> = var undef
    %4:ptr<storage, u32, read_write> = access %buffer, 1u
    %5:u32 = load %4
    %6:ptr<storage, u32, read_write> = access %buffer, 2u
    store %6, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<u32, 4>, read_write> = var undef
    %4:ptr<storage, u32, read_write> = access %buffer, 1u
    %5:u32 = load %4
    %6:ptr<storage, u32, read_write> = access %buffer, 2u
    store %6, %5
    ret
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, SharedArray_AccessNestedArray) {
    auto* inner = ty.array<u32, 4>();
    auto* outer = ty.array(inner, 4);
    auto* buffer = b.Var("buffer", ty.ptr(storage, outer, read_write));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", ty.ptr<function>(outer));
        auto* load = b.Load(b.Access(ty.ptr(storage, inner, read_write), buffer, 1_u));
        b.Store(b.Access(ty.ptr(storage, inner, read_write), buffer, 2_u), load);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer:ptr<storage, array<array<u32, 4>, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<array<u32, 4>, 4>, read_write> = var undef
    %4:ptr<storage, array<u32, 4>, read_write> = access %buffer, 1u
    %5:array<u32, 4> = load %4
    %6:ptr<storage, array<u32, 4>, read_write> = access %buffer, 2u
    store %6, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer:ptr<storage, spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 4>, 4>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<array<u32, 4>, 4>, read_write> = var undef
    %4:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = access %buffer, 1u
    %5:spirv.explicit_layout_array<u32, 4> = load %4
    %6:array<u32, 4> = call %tint_convert_explicit_layout, %5
    %8:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = access %buffer, 2u
    %9:spirv.explicit_layout_array<u32, 4> = call %tint_convert_explicit_layout_1, %6
    store %8, %9
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:spirv.explicit_layout_array<u32, 4>):array<u32, 4> {
  $B3: {
    %12:ptr<function, array<u32, 4>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %14:bool = gte %idx, 4u
        if %14 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %15:u32 = access %tint_source, %idx
        %16:ptr<function, u32, read_write> = access %12, %idx
        store %16, %15
        continue  # -> $B6
      }
      $B6: {  # continuing
        %17:u32 = add %idx, 1u
        next_iteration %17  # -> $B5
      }
    }
    %18:array<u32, 4> = load %12
    ret %18
  }
}
%tint_convert_explicit_layout_1 = func(%tint_source_1:array<u32, 4>):spirv.explicit_layout_array<u32, 4> {  # %tint_convert_explicit_layout_1: 'tint_convert_explicit_layout', %tint_source_1: 'tint_source'
  $B8: {
    %20:ptr<function, spirv.explicit_layout_array<u32, 4>, read_write> = var undef
    loop [i: $B9, b: $B10, c: $B11] {  # loop_2
      $B9: {  # initializer
        next_iteration 0u  # -> $B10
      }
      $B10 (%idx_1:u32): {  # body
        %22:bool = gte %idx_1, 4u
        if %22 [t: $B12] {  # if_2
          $B12: {  # true
            exit_loop  # loop_2
          }
        }
        %23:u32 = access %tint_source_1, %idx_1
        %24:ptr<function, u32, read_write> = access %20, %idx_1
        store %24, %23
        continue  # -> $B11
      }
      $B11: {  # continuing
        %25:u32 = add %idx_1, 1u
        next_iteration %25  # -> $B10
      }
    }
    %26:spirv.explicit_layout_array<u32, 4> = load %20
    ret %26
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, SharedArray_MultipleVars) {
    auto* array = ty.array<u32, 4>();
    auto* buffer_0 = b.Var("buffer_0", ty.ptr(storage, array));
    auto* buffer_1 = b.Var("buffer_1", ty.ptr(storage, array));
    auto* buffer_2 = b.Var("buffer_2", ty.ptr(storage, array));
    buffer_0->SetBindingPoint(0, 0);
    buffer_1->SetBindingPoint(0, 1);
    buffer_2->SetBindingPoint(0, 2);
    mod.root_block->Append(buffer_0);
    mod.root_block->Append(buffer_1);
    mod.root_block->Append(buffer_2);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Var("local", b.Zero(array));
        b.Let("let_0", b.Load(buffer_0));
        b.Let("let_1", b.Load(buffer_1));
        b.Let("let_2", b.Load(buffer_2));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %buffer_0:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 0)
  %buffer_1:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 1)
  %buffer_2:ptr<storage, array<u32, 4>, read_write> = var undef @binding_point(0, 2)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<u32, 4>, read_write> = var array<u32, 4>(0u)
    %6:array<u32, 4> = load %buffer_0
    %let_0:array<u32, 4> = let %6
    %8:array<u32, 4> = load %buffer_1
    %let_1:array<u32, 4> = let %8
    %10:array<u32, 4> = load %buffer_2
    %let_2:array<u32, 4> = let %10
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %buffer_0:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = var undef @binding_point(0, 0)
  %buffer_1:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = var undef @binding_point(0, 1)
  %buffer_2:ptr<storage, spirv.explicit_layout_array<u32, 4>, read_write> = var undef @binding_point(0, 2)
}

%foo = func():void {
  $B2: {
    %local:ptr<function, array<u32, 4>, read_write> = var array<u32, 4>(0u)
    %6:spirv.explicit_layout_array<u32, 4> = load %buffer_0
    %7:array<u32, 4> = call %tint_convert_explicit_layout, %6
    %let_0:array<u32, 4> = let %7
    %10:spirv.explicit_layout_array<u32, 4> = load %buffer_1
    %11:array<u32, 4> = call %tint_convert_explicit_layout, %10
    %let_1:array<u32, 4> = let %11
    %13:spirv.explicit_layout_array<u32, 4> = load %buffer_2
    %14:array<u32, 4> = call %tint_convert_explicit_layout, %13
    %let_2:array<u32, 4> = let %14
    ret
  }
}
%tint_convert_explicit_layout = func(%tint_source:spirv.explicit_layout_array<u32, 4>):array<u32, 4> {
  $B3: {
    %17:ptr<function, array<u32, 4>, read_write> = var undef
    loop [i: $B4, b: $B5, c: $B6] {  # loop_1
      $B4: {  # initializer
        next_iteration 0u  # -> $B5
      }
      $B5 (%idx:u32): {  # body
        %19:bool = gte %idx, 4u
        if %19 [t: $B7] {  # if_1
          $B7: {  # true
            exit_loop  # loop_1
          }
        }
        %20:u32 = access %tint_source, %idx
        %21:ptr<function, u32, read_write> = access %17, %idx
        store %21, %20
        continue  # -> $B6
      }
      $B6: {  # continuing
        %22:u32 = add %idx, 1u
        next_iteration %22  # -> $B5
      }
    }
    %23:array<u32, 4> = load %17
    ret %23
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_ForkExplicitLayoutTypesTest, ArrayLength) {
    auto* structure =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("i"), ty.u32()},
                                                   {mod.symbols.New("arr"), ty.array<u32>()},
                                               });

    auto* buffer = b.Var("buffer", ty.ptr(storage, structure));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), BuiltinFn::kArrayLength, buffer, 1_u);
        b.Return(func);
    });

    auto* src = R"(
MyStruct = struct @align(4) {
  i:u32 @offset(0)
  arr:array<u32> @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:u32 = spirv.array_length %buffer, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
MyStruct = struct @align(4) {
  i:u32 @offset(0)
  arr:array<u32> @offset(4)
}

MyStruct_tint_explicit_layout = struct @align(4), @spirv.explicit_layout {
  i:u32 @offset(0)
  arr:spirv.explicit_layout_array<u32, > @offset(4)
}

$B1: {  # root
  %buffer:ptr<storage, MyStruct_tint_explicit_layout, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:u32 = spirv.array_length %buffer, 1u
    ret
  }
}
)";

    Run(ForkExplicitLayoutTypes);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
