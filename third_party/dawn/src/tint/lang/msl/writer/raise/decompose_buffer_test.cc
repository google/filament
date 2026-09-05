// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/raise/decompose_buffer.h"

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class MslWriter_DecomposeBufferTest : public core::ir::transform::TransformTest {
  protected:
    void SetUp() override {
        mod.properties.Add(core::ir::Property::kAllow16BitFloats,
                           core::ir::Property::kAllowMslEntryPointInterface,
                           core::ir::Property::kAllowBufferTypes);
    }
};

TEST_F(MslWriter_DecomposeBufferTest, BufferView_u32_ZeroOffset) {
    auto* gv = b.Var("gv", ty.ptr(storage, ty.unsized_buffer()));
    gv->SetBindingPoint(0, 0);
    mod.root_block->Append(gv);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.u32()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.u32()}, gv, 0_u);
        b.Load(view);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %gv:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = bufferView<u32> %gv, 0u
    %4:u32 = load %3
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv:ptr<storage, array<u8>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = msl.pointer_offset<u32> %gv, 0u
    %4:u32 = load %3
    ret
  }
}
)";

    Run(DecomposeBuffer);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_DecomposeBufferTest, BufferArrayView_ArrayU32) {
    auto* gv = b.Var("gv", ty.ptr(storage, ty.unsized_buffer()));
    gv->SetBindingPoint(0, 0);
    mod.root_block->Append(gv);

    auto* func = b.Function("foo", ty.void_());
    auto* offset = b.FunctionParam("offset", ty.i32());
    auto* size = b.FunctionParam("size", ty.i32());
    func->SetParams({offset, size});
    b.Append(func->Block(), [&] {
        auto* view = b.CallExplicit(
            ty.ptr(storage, ty.runtime_array(ty.u32())), core::BuiltinFn::kBufferArrayView,
            Vector<core::ir::TemplateParameter, 1>{ty.runtime_array(ty.u32())}, gv, offset, size);
        b.Load(b.Access(ty.ptr(storage, ty.u32()), view, 4_u));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %gv:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%offset:i32, %size:i32):void {
  $B2: {
    %5:ptr<storage, array<u32>, read_write> = bufferArrayView<array<u32>> %gv, %offset, %size
    %6:ptr<storage, u32, read_write> = access %5, 4u
    %7:u32 = load %6
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv:ptr<storage, array<u8>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%offset:i32, %size:i32):void {
  $B2: {
    %5:u32 = bitcast<u32> %offset
    %6:ptr<storage, array<u32>, read_write> = msl.pointer_offset<array<u32>> %gv, %5
    %7:ptr<storage, u32, read_write> = access %6, 4u
    %8:u32 = load %7
    ret
  }
}
)";

    Run(DecomposeBuffer);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_DecomposeBufferTest, MultipleCallsFromVariable) {
    auto* gv = b.Var("gv", ty.ptr(storage, ty.unsized_buffer()));
    gv->SetBindingPoint(0, 0);
    mod.root_block->Append(gv);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* c1 = b.CallExplicit(ty.ptr(storage, ty.u32()), core::BuiltinFn::kBufferView,
                                  Vector<core::ir::TemplateParameter, 1>{ty.u32()}, gv, 0_u);
        b.Load(c1);
        auto* c2 = b.CallExplicit(
            ty.ptr(storage, ty.runtime_array(ty.u32())), core::BuiltinFn::kBufferArrayView,
            Vector<core::ir::TemplateParameter, 1>{ty.runtime_array(ty.u32())}, gv, 16_u, 32_u);
        b.Load(b.Access(ty.ptr(storage, ty.u32()), c2, 1_u));
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %gv:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = bufferView<u32> %gv, 0u
    %4:u32 = load %3
    %5:ptr<storage, array<u32>, read_write> = bufferArrayView<array<u32>> %gv, 16u, 32u
    %6:ptr<storage, u32, read_write> = access %5, 1u
    %7:u32 = load %6
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv:ptr<storage, array<u8>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = msl.pointer_offset<u32> %gv, 0u
    %4:u32 = load %3
    %5:ptr<storage, array<u32>, read_write> = msl.pointer_offset<array<u32>> %gv, 16u
    %6:ptr<storage, u32, read_write> = access %5, 1u
    %7:u32 = load %6
    ret
  }
}
)";

    Run(DecomposeBuffer);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_DecomposeBufferTest, UnifyParameters) {
    auto* large = b.Var("large", ty.ptr(storage, ty.buffer(1024)));
    large->SetBindingPoint(0, 0);
    mod.root_block->Append(large);
    auto* small = b.Var("small", ty.ptr(workgroup, ty.buffer(512)));
    mod.root_block->Append(small);

    auto* baz = b.Function("baz", ty.void_());
    auto* baz_p1 = b.FunctionParam("baz_p1", ty.ptr(storage, ty.unsized_buffer()));
    auto* baz_p2 = b.FunctionParam("baz_p2", ty.ptr(workgroup, ty.unsized_buffer()));
    baz->SetParams({baz_p1, baz_p2});
    b.Append(baz->Block(), [&] { b.Return(baz); });

    auto* bar = b.Function("bar", ty.void_());
    auto* bar_p1 = b.FunctionParam("bar_p1", ty.ptr(storage, ty.buffer(512)));
    auto* bar_p2 = b.FunctionParam("bar_p2", ty.ptr(workgroup, ty.buffer(256)));
    bar->SetParams({bar_p1, bar_p2});
    b.Append(bar->Block(), [&] {
        b.Call(ty.void_(), baz, bar_p1, bar_p2);
        b.Return(bar);
    });

    auto* foobar = b.Function("foobar", ty.void_());
    auto* foobar_p1 = b.FunctionParam("foobar_p1", ty.ptr(storage, ty.buffer(512)));
    auto* foobar_p2 = b.FunctionParam("foobar_p2", ty.ptr(workgroup, ty.buffer(256)));
    foobar->SetParams({foobar_p1, foobar_p2});
    b.Append(foobar->Block(), [&] {
        b.Call(ty.void_(), bar, foobar_p1, foobar_p2);
        b.Return(foobar);
    });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Call(ty.void_(), foobar, large, small);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %large:ptr<storage, buffer<1024>, read_write> = var undef @binding_point(0, 0)
  %small:ptr<workgroup, buffer<512>, read_write> = var undef
}

%baz = func(%baz_p1:ptr<storage, buffer, read_write>, %baz_p2:ptr<workgroup, buffer, read_write>):void {
  $B2: {
    ret
  }
}
%bar = func(%bar_p1:ptr<storage, buffer<512>, read_write>, %bar_p2:ptr<workgroup, buffer<256>, read_write>):void {
  $B3: {
    %9:void = call %baz, %bar_p1, %bar_p2
    ret
  }
}
%foobar = func(%foobar_p1:ptr<storage, buffer<512>, read_write>, %foobar_p2:ptr<workgroup, buffer<256>, read_write>):void {
  $B4: {
    %13:void = call %bar, %foobar_p1, %foobar_p2
    ret
  }
}
%foo = func():void {
  $B5: {
    %15:void = call %foobar, %large, %small
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %large:ptr<storage, array<u8, 1024>, read_write> = var undef @binding_point(0, 0)
  %small:ptr<workgroup, array<u8, 512>, read_write> = var undef
}

%baz = func(%baz_p1:ptr<storage, array<u8>, read_write>, %baz_p2:ptr<workgroup, array<u8>, read_write>):void {
  $B2: {
    ret
  }
}
%bar = func(%bar_p1:ptr<storage, array<u8, 512>, read_write>, %bar_p2:ptr<workgroup, array<u8, 256>, read_write>):void {
  $B3: {
    %9:ptr<storage, array<u8>, read_write> = msl.pointer_offset<array<u8>> %bar_p1, 0u
    %10:ptr<workgroup, array<u8>, read_write> = msl.pointer_offset<array<u8>> %bar_p2, 0u
    %11:void = call %baz, %9, %10
    ret
  }
}
%foobar = func(%foobar_p1:ptr<storage, array<u8, 512>, read_write>, %foobar_p2:ptr<workgroup, array<u8, 256>, read_write>):void {
  $B4: {
    %15:void = call %bar, %foobar_p1, %foobar_p2
    ret
  }
}
%foo = func():void {
  $B5: {
    %17:ptr<storage, array<u8, 512>, read_write> = msl.pointer_offset<array<u8, 512>> %large, 0u
    %18:ptr<workgroup, array<u8, 256>, read_write> = msl.pointer_offset<array<u8, 256>> %small, 0u
    %19:void = call %foobar, %17, %18
    ret
  }
}
)";

    Run(DecomposeBuffer);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_DecomposeBufferTest, UnifyParameters_Lets) {
    auto* large = b.Var("large", ty.ptr(storage, ty.buffer(1024)));
    large->SetBindingPoint(0, 0);
    mod.root_block->Append(large);
    auto* small = b.Var("small", ty.ptr(workgroup, ty.buffer(512)));
    mod.root_block->Append(small);

    auto* baz = b.Function("baz", ty.void_());
    auto* baz_p1 = b.FunctionParam("baz_p1", ty.ptr(storage, ty.unsized_buffer()));
    auto* baz_p2 = b.FunctionParam("baz_p2", ty.ptr(workgroup, ty.unsized_buffer()));
    baz->SetParams({baz_p1, baz_p2});
    b.Append(baz->Block(), [&] { b.Return(baz); });

    auto* bar = b.Function("bar", ty.void_());
    auto* bar_p1 = b.FunctionParam("bar_p1", ty.ptr(storage, ty.buffer(512)));
    auto* bar_p2 = b.FunctionParam("bar_p2", ty.ptr(workgroup, ty.buffer(256)));
    bar->SetParams({bar_p1, bar_p2});
    b.Append(bar->Block(), [&] {
        auto* l1 = b.Let("l1", bar_p1);
        auto* l2 = b.Let("l2", bar_p2);
        b.Call(ty.void_(), baz, l1, l2);
        b.Return(bar);
    });

    auto* foobar = b.Function("foobar", ty.void_());
    auto* foobar_p1 = b.FunctionParam("foobar_p1", ty.ptr(storage, ty.buffer(512)));
    auto* foobar_p2 = b.FunctionParam("foobar_p2", ty.ptr(workgroup, ty.buffer(256)));
    foobar->SetParams({foobar_p1, foobar_p2});
    b.Append(foobar->Block(), [&] {
        auto* l3 = b.Let("l3", foobar_p1);
        b.Call(ty.void_(), bar, l3, foobar_p2);
        b.Return(foobar);
    });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Call(ty.void_(), foobar, large, small);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %large:ptr<storage, buffer<1024>, read_write> = var undef @binding_point(0, 0)
  %small:ptr<workgroup, buffer<512>, read_write> = var undef
}

%baz = func(%baz_p1:ptr<storage, buffer, read_write>, %baz_p2:ptr<workgroup, buffer, read_write>):void {
  $B2: {
    ret
  }
}
%bar = func(%bar_p1:ptr<storage, buffer<512>, read_write>, %bar_p2:ptr<workgroup, buffer<256>, read_write>):void {
  $B3: {
    %l1:ptr<storage, buffer<512>, read_write> = let %bar_p1
    %l2:ptr<workgroup, buffer<256>, read_write> = let %bar_p2
    %11:void = call %baz, %l1, %l2
    ret
  }
}
%foobar = func(%foobar_p1:ptr<storage, buffer<512>, read_write>, %foobar_p2:ptr<workgroup, buffer<256>, read_write>):void {
  $B4: {
    %l3:ptr<storage, buffer<512>, read_write> = let %foobar_p1
    %16:void = call %bar, %l3, %foobar_p2
    ret
  }
}
%foo = func():void {
  $B5: {
    %18:void = call %foobar, %large, %small
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %large:ptr<storage, array<u8, 1024>, read_write> = var undef @binding_point(0, 0)
  %small:ptr<workgroup, array<u8, 512>, read_write> = var undef
}

%baz = func(%baz_p1:ptr<storage, array<u8>, read_write>, %baz_p2:ptr<workgroup, array<u8>, read_write>):void {
  $B2: {
    ret
  }
}
%bar = func(%bar_p1:ptr<storage, array<u8, 512>, read_write>, %bar_p2:ptr<workgroup, array<u8, 256>, read_write>):void {
  $B3: {
    %l1:ptr<storage, array<u8, 512>, read_write> = let %bar_p1
    %l2:ptr<workgroup, array<u8, 256>, read_write> = let %bar_p2
    %11:ptr<storage, array<u8>, read_write> = msl.pointer_offset<array<u8>> %l1, 0u
    %12:ptr<workgroup, array<u8>, read_write> = msl.pointer_offset<array<u8>> %l2, 0u
    %13:void = call %baz, %11, %12
    ret
  }
}
%foobar = func(%foobar_p1:ptr<storage, array<u8, 512>, read_write>, %foobar_p2:ptr<workgroup, array<u8, 256>, read_write>):void {
  $B4: {
    %l3:ptr<storage, array<u8, 512>, read_write> = let %foobar_p1
    %18:void = call %bar, %l3, %foobar_p2
    ret
  }
}
%foo = func():void {
  $B5: {
    %20:ptr<storage, array<u8, 512>, read_write> = msl.pointer_offset<array<u8, 512>> %large, 0u
    %21:ptr<workgroup, array<u8, 256>, read_write> = msl.pointer_offset<array<u8, 256>> %small, 0u
    %22:void = call %foobar, %20, %21
    ret
  }
}
)";

    Run(DecomposeBuffer);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_DecomposeBufferTest, UnifyParameters_Lets_Unused) {
    auto* baz = b.Function("baz", ty.void_());
    auto* baz_p1 = b.FunctionParam("baz_p1", ty.ptr(storage, ty.unsized_buffer()));
    auto* baz_p2 = b.FunctionParam("baz_p2", ty.ptr(workgroup, ty.unsized_buffer()));
    baz->SetParams({baz_p1, baz_p2});
    b.Append(baz->Block(), [&] {
        b.Let("l1", baz_p1);
        b.Let("l2", baz_p2);
        b.Return(baz);
    });

    auto* src = R"(
%baz = func(%baz_p1:ptr<storage, buffer, read_write>, %baz_p2:ptr<workgroup, buffer, read_write>):void {
  $B1: {
    %l1:ptr<storage, buffer, read_write> = let %baz_p1
    %l2:ptr<workgroup, buffer, read_write> = let %baz_p2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%baz = func(%baz_p1:ptr<storage, array<u8>, read_write>, %baz_p2:ptr<workgroup, array<u8>, read_write>):void {
  $B1: {
    %l1:ptr<storage, array<u8>, read_write> = let %baz_p1
    %l2:ptr<workgroup, array<u8>, read_write> = let %baz_p2
    ret
  }
}
)";

    Run(DecomposeBuffer);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_DecomposeBufferTest, UnifyParameters_WithUses) {
    auto* large = b.Var("large", ty.ptr(storage, ty.buffer(1024)));
    large->SetBindingPoint(0, 0);
    mod.root_block->Append(large);
    auto* small = b.Var("small", ty.ptr(workgroup, ty.buffer(512)));
    mod.root_block->Append(small);

    auto* baz = b.Function("baz", ty.void_());
    auto* baz_p1 = b.FunctionParam("baz_p1", ty.ptr(storage, ty.array(ty.u32(), 64)));
    auto* baz_p2 = b.FunctionParam("baz_p2", ty.ptr(workgroup, ty.runtime_array(ty.u32())));
    baz->SetParams({baz_p1, baz_p2});
    b.Append(baz->Block(), [&] {
        b.Let("let", baz_p2);
        b.Return(baz);
    });

    auto* bar = b.Function("bar", ty.void_());
    auto* bar_p1 = b.FunctionParam("bar_p1", ty.ptr(storage, ty.buffer(512)));
    auto* bar_p2 = b.FunctionParam("bar_p2", ty.ptr(workgroup, ty.buffer(256)));
    bar->SetParams({bar_p1, bar_p2});
    b.Append(bar->Block(), [&] {
        auto* v1 = b.CallExplicit(
            ty.ptr(storage, ty.array(ty.u32(), 64)), core::BuiltinFn::kBufferView,
            Vector<core::ir::TemplateParameter, 1>{ty.array(ty.u32(), 64)}, bar_p1, 32_u);
        auto* v2 = b.CallExplicit(
            ty.ptr(workgroup, ty.runtime_array(ty.u32())), core::BuiltinFn::kBufferArrayView,
            Vector<core::ir::TemplateParameter, 1>{ty.runtime_array(ty.u32())}, bar_p2, 4_u, 12_u);
        b.Call(ty.void_(), baz, v1, v2);
        b.Return(bar);
    });

    auto* foobar = b.Function("foobar", ty.void_());
    auto* foobar_p1 = b.FunctionParam("foobar_p1", ty.ptr(storage, ty.buffer(512)));
    auto* foobar_p2 = b.FunctionParam("foobar_p2", ty.ptr(workgroup, ty.buffer(256)));
    foobar->SetParams({foobar_p1, foobar_p2});
    b.Append(foobar->Block(), [&] {
        b.Call(ty.void_(), bar, foobar_p1, foobar_p2);
        b.Return(foobar);
    });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        b.Call(ty.void_(), foobar, large, small);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %large:ptr<storage, buffer<1024>, read_write> = var undef @binding_point(0, 0)
  %small:ptr<workgroup, buffer<512>, read_write> = var undef
}

%baz = func(%baz_p1:ptr<storage, array<u32, 64>, read_write>, %baz_p2:ptr<workgroup, array<u32>, read_write>):void {
  $B2: {
    %let:ptr<workgroup, array<u32>, read_write> = let %baz_p2
    ret
  }
}
%bar = func(%bar_p1:ptr<storage, buffer<512>, read_write>, %bar_p2:ptr<workgroup, buffer<256>, read_write>):void {
  $B3: {
    %10:ptr<storage, array<u32, 64>, read_write> = bufferView<array<u32, 64>> %bar_p1, 32u
    %11:ptr<workgroup, array<u32>, read_write> = bufferArrayView<array<u32>> %bar_p2, 4u, 12u
    %12:void = call %baz, %10, %11
    ret
  }
}
%foobar = func(%foobar_p1:ptr<storage, buffer<512>, read_write>, %foobar_p2:ptr<workgroup, buffer<256>, read_write>):void {
  $B4: {
    %16:void = call %bar, %foobar_p1, %foobar_p2
    ret
  }
}
%foo = func():void {
  $B5: {
    %18:void = call %foobar, %large, %small
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %large:ptr<storage, array<u8, 1024>, read_write> = var undef @binding_point(0, 0)
  %small:ptr<workgroup, array<u8, 512>, read_write> = var undef
}

%baz = func(%baz_p1:ptr<storage, array<u32, 64>, read_write>, %baz_p2:ptr<workgroup, array<u32>, read_write>):void {
  $B2: {
    %let:ptr<workgroup, array<u32>, read_write> = let %baz_p2
    ret
  }
}
%bar = func(%bar_p1:ptr<storage, array<u8, 512>, read_write>, %bar_p2:ptr<workgroup, array<u8, 256>, read_write>):void {
  $B3: {
    %10:ptr<storage, array<u32, 64>, read_write> = msl.pointer_offset<array<u32, 64>> %bar_p1, 32u
    %11:ptr<workgroup, array<u32>, read_write> = msl.pointer_offset<array<u32>> %bar_p2, 4u
    %12:void = call %baz, %10, %11
    ret
  }
}
%foobar = func(%foobar_p1:ptr<storage, array<u8, 512>, read_write>, %foobar_p2:ptr<workgroup, array<u8, 256>, read_write>):void {
  $B4: {
    %16:void = call %bar, %foobar_p1, %foobar_p2
    ret
  }
}
%foo = func():void {
  $B5: {
    %18:ptr<storage, array<u8, 512>, read_write> = msl.pointer_offset<array<u8, 512>> %large, 0u
    %19:ptr<workgroup, array<u8, 256>, read_write> = msl.pointer_offset<array<u8, 256>> %small, 0u
    %20:void = call %foobar, %18, %19
    ret
  }
}
)";

    Run(DecomposeBuffer);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_DecomposeBufferTest, FunctionCall_MultiCallSite_SameCaller) {
    auto* gv1 = b.Var("gv1", ty.ptr(storage, ty.unsized_buffer()));
    gv1->SetBindingPoint(0, 0);
    mod.root_block->Append(gv1);
    auto* gv2 = b.Var("gv2", ty.ptr(storage, ty.buffer(128)));
    gv2->SetBindingPoint(0, 0);
    mod.root_block->Append(gv2);
    auto* gv3 = b.Var("gv3", ty.ptr(storage, ty.runtime_array(ty.u32())));
    gv3->SetBindingPoint(0, 0);
    mod.root_block->Append(gv3);

    auto* bar = b.Function("bar", ty.void_());
    auto* param = b.FunctionParam("param", ty.ptr(storage, ty.runtime_array(ty.u32())));
    bar->SetParams({param});
    b.Append(bar->Block(), [&] {
        b.Let("let", param);
        b.Return(bar);
    });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* v1 = b.CallExplicit(
            ty.ptr(storage, ty.runtime_array(ty.u32())), core::BuiltinFn::kBufferArrayView,
            Vector<core::ir::TemplateParameter, 1>{ty.runtime_array(ty.u32())}, gv1, 16_u, 64_u);
        b.Call(ty.void_(), bar, v1);
        auto* v2 = b.CallExplicit(
            ty.ptr(storage, ty.runtime_array(ty.u32())), core::BuiltinFn::kBufferView,
            Vector<core::ir::TemplateParameter, 1>{ty.runtime_array(ty.u32())}, gv2, 32_u);
        b.Call(ty.void_(), bar, v2);
        b.Call(ty.void_(), bar, gv3);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %gv1:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, buffer<128>, read_write> = var undef @binding_point(0, 0)
  %gv3:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param:ptr<storage, array<u32>, read_write>):void {
  $B2: {
    %let:ptr<storage, array<u32>, read_write> = let %param
    ret
  }
}
%foo = func():void {
  $B3: {
    %8:ptr<storage, array<u32>, read_write> = bufferArrayView<array<u32>> %gv1, 16u, 64u
    %9:void = call %bar, %8
    %10:ptr<storage, array<u32>, read_write> = bufferView<array<u32>> %gv2, 32u
    %11:void = call %bar, %10
    %12:void = call %bar, %gv3
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv1:ptr<storage, array<u8>, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, array<u8, 128>, read_write> = var undef @binding_point(0, 0)
  %gv3:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param:ptr<storage, array<u32>, read_write>):void {
  $B2: {
    %let:ptr<storage, array<u32>, read_write> = let %param
    ret
  }
}
%foo = func():void {
  $B3: {
    %8:ptr<storage, array<u32>, read_write> = msl.pointer_offset<array<u32>> %gv1, 16u
    %9:void = call %bar, %8
    %10:ptr<storage, array<u32>, read_write> = msl.pointer_offset<array<u32>> %gv2, 32u
    %11:void = call %bar, %10
    %12:void = call %bar, %gv3
    ret
  }
}
)";

    Run(DecomposeBuffer);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_DecomposeBufferTest, FunctionCall_MultiCallSite_SameCaller_WithLets) {
    auto* gv1 = b.Var("gv1", ty.ptr(storage, ty.unsized_buffer()));
    gv1->SetBindingPoint(0, 0);
    mod.root_block->Append(gv1);
    auto* gv2 = b.Var("gv2", ty.ptr(storage, ty.buffer(128)));
    gv2->SetBindingPoint(0, 0);
    mod.root_block->Append(gv2);
    auto* gv3 = b.Var("gv3", ty.ptr(storage, ty.runtime_array(ty.u32())));
    gv3->SetBindingPoint(0, 0);
    mod.root_block->Append(gv3);

    auto* bar = b.Function("bar", ty.void_());
    auto* param = b.FunctionParam("param", ty.ptr(storage, ty.runtime_array(ty.u32())));
    bar->SetParams({param});
    b.Append(bar->Block(), [&] {
        b.Let("let", param);
        b.Return(bar);
    });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* v1 = b.CallExplicit(
            ty.ptr(storage, ty.runtime_array(ty.u32())), core::BuiltinFn::kBufferArrayView,
            Vector<core::ir::TemplateParameter, 1>{ty.runtime_array(ty.u32())}, gv1, 16_u, 64_u);
        auto* l1 = b.Let("l1", v1);
        b.Call(ty.void_(), bar, l1);
        auto* l2 = b.Let("l2", gv2);
        auto* v2 = b.CallExplicit(
            ty.ptr(storage, ty.runtime_array(ty.u32())), core::BuiltinFn::kBufferView,
            Vector<core::ir::TemplateParameter, 1>{ty.runtime_array(ty.u32())}, l2, 32_u);
        b.Call(ty.void_(), bar, v2);
        b.Call(ty.void_(), bar, gv3);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %gv1:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, buffer<128>, read_write> = var undef @binding_point(0, 0)
  %gv3:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param:ptr<storage, array<u32>, read_write>):void {
  $B2: {
    %let:ptr<storage, array<u32>, read_write> = let %param
    ret
  }
}
%foo = func():void {
  $B3: {
    %8:ptr<storage, array<u32>, read_write> = bufferArrayView<array<u32>> %gv1, 16u, 64u
    %l1:ptr<storage, array<u32>, read_write> = let %8
    %10:void = call %bar, %l1
    %l2:ptr<storage, buffer<128>, read_write> = let %gv2
    %12:ptr<storage, array<u32>, read_write> = bufferView<array<u32>> %l2, 32u
    %13:void = call %bar, %12
    %14:void = call %bar, %gv3
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv1:ptr<storage, array<u8>, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, array<u8, 128>, read_write> = var undef @binding_point(0, 0)
  %gv3:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%bar = func(%param:ptr<storage, array<u32>, read_write>):void {
  $B2: {
    %let:ptr<storage, array<u32>, read_write> = let %param
    ret
  }
}
%foo = func():void {
  $B3: {
    %8:ptr<storage, array<u32>, read_write> = msl.pointer_offset<array<u32>> %gv1, 16u
    %l1:ptr<storage, array<u32>, read_write> = let %8
    %10:void = call %bar, %l1
    %l2:ptr<storage, array<u8, 128>, read_write> = let %gv2
    %12:ptr<storage, array<u32>, read_write> = msl.pointer_offset<array<u32>> %l2, 32u
    %13:void = call %bar, %12
    %14:void = call %bar, %gv3
    ret
  }
}
)";

    Run(DecomposeBuffer);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
