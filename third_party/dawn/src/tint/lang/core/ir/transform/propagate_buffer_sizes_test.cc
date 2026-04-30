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

#include "src/tint/lang/core/ir/transform/propagate_buffer_sizes.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_PropagateBufferSizesTest = TransformTest;

TEST_F(IR_PropagateBufferSizesTest, BufferLength_GlobalVariable_Noop) {
    auto* gv = b.Var("gv", ty.ptr(storage, ty.unsized_buffer(), read_write));
    gv->SetBindingPoint(0, 0);
    mod.root_block->Append(gv);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* call = b.Call(ty.u32(), BuiltinFn::kBufferLength, gv);
        b.Let("a", call);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %gv:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:u32 = bufferLength %gv
    %a:u32 = let %3
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(PropagateBufferSizes);

    EXPECT_EQ(src, str());
}

TEST_F(IR_PropagateBufferSizesTest, BufferArrayView_GlobalVariable_Direct) {
    auto* gv = b.Var("gv", ty.ptr(storage, ty.buffer(64), read_write));
    gv->SetBindingPoint(0, 0);
    mod.root_block->Append(gv);

    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* call =
            b.CallExplicit(ty.ptr(storage, ty.runtime_array(ty.u32())), BuiltinFn::kBufferArrayView,
                           Vector{ty.runtime_array(ty.u32())}, gv, 0_u, 32_u);
        b.Let("a", call);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %gv:ptr<storage, buffer<64>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = bufferArrayView<array<u32>> %gv, 0u, 32u
    %a:ptr<storage, array<u32>, read_write> = let %3
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv:ptr<storage, buffer<64>, read_write> = var undef @binding_point(0, 0)
}

%foo = func():void {
  $B2: {
    %3:ptr<storage, array<u32>, read_write> = bufferArrayView<array<u32>> %gv, 0u, 32u, 64u
    %a:ptr<storage, array<u32>, read_write> = let %3
    ret
  }
}
)";

    Run(PropagateBufferSizes);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PropagateBufferSizesTest, BufferLength_Parameter_Indirect) {
    auto* foo = b.Function("foo", ty.void_());
    auto* foo_p = b.FunctionParam("p", ty.ptr(storage, ty.unsized_buffer()));
    foo->SetParams({foo_p});
    b.Append(foo->Block(), [&] {
        auto* call = b.Call(ty.u32(), BuiltinFn::kBufferLength, foo_p);
        b.Let("a", call);
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    auto* bar_q = b.FunctionParam("q", ty.ptr(storage, ty.buffer(64)));
    bar->SetParams({bar_q});
    b.Append(bar->Block(), [&] {
        b.Call(ty.void_(), foo, bar_q);
        b.Return(bar);
    });

    auto* src = R"(
%foo = func(%p:ptr<storage, buffer, read_write>):void {
  $B1: {
    %3:u32 = bufferLength %p
    %a:u32 = let %3
    ret
  }
}
%bar = func(%q:ptr<storage, buffer<64>, read_write>):void {
  $B2: {
    %7:void = call %foo, %q
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<storage, buffer, read_write>):void {
  $B1: {
    %3:u32 = bufferLength %p, 64u
    %a:u32 = let %3
    ret
  }
}
%bar = func(%q:ptr<storage, buffer<64>, read_write>):void {
  $B2: {
    %7:void = call %foo, %q
    ret
  }
}
)";

    Run(PropagateBufferSizes);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PropagateBufferSizesTest, BufferView_Variable_Indirect) {
    auto* gv = b.Var("gv", ty.ptr(storage, ty.buffer(64), read_write));
    gv->SetBindingPoint(0, 0);
    mod.root_block->Append(gv);

    auto* foo = b.Function("foo", ty.void_());
    auto* foo_p = b.FunctionParam("p", ty.ptr(storage, ty.unsized_buffer()));
    foo->SetParams({foo_p});
    b.Append(foo->Block(), [&] {
        auto* call = b.CallExplicit(ty.ptr(storage, ty.u32()), BuiltinFn::kBufferView,
                                    Vector{ty.u32()}, foo_p, 0_u);
        b.Let("a", call);
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        b.Call(ty.void_(), foo, gv);
        b.Return(bar);
    });

    auto* src = R"(
$B1: {  # root
  %gv:ptr<storage, buffer<64>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%p:ptr<storage, buffer, read_write>):void {
  $B2: {
    %4:ptr<storage, u32, read_write> = bufferView<u32> %p, 0u
    %a:ptr<storage, u32, read_write> = let %4
    ret
  }
}
%bar = func():void {
  $B3: {
    %7:void = call %foo, %gv
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv:ptr<storage, buffer<64>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%p:ptr<storage, buffer, read_write>):void {
  $B2: {
    %4:ptr<storage, u32, read_write> = bufferView<u32> %p, 0u, 64u
    %a:ptr<storage, u32, read_write> = let %4
    ret
  }
}
%bar = func():void {
  $B3: {
    %7:void = call %foo, %gv
    ret
  }
}
)";

    Run(PropagateBufferSizes);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PropagateBufferSizesTest, BufferView_Variable_IndirectMultiPath) {
    auto* gv = b.Var("gv", ty.ptr(storage, ty.buffer(64), read_write));
    gv->SetBindingPoint(0, 0);
    mod.root_block->Append(gv);

    auto* foo = b.Function("foo", ty.void_());
    auto* foo_p = b.FunctionParam("p", ty.ptr(storage, ty.unsized_buffer()));
    foo->SetParams({foo_p});
    b.Append(foo->Block(), [&] {
        auto* call = b.CallExplicit(ty.ptr(storage, ty.u32()), BuiltinFn::kBufferView,
                                    Vector{ty.u32()}, foo_p, 0_u);
        b.Let("a", call);
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        b.Call(ty.void_(), foo, gv);
        b.Return(bar);
    });

    auto* baz = b.Function("baz", ty.void_());
    b.Append(baz->Block(), [&] {
        b.Call(ty.void_(), foo, gv);
        b.Return(baz);
    });

    auto* src = R"(
$B1: {  # root
  %gv:ptr<storage, buffer<64>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%p:ptr<storage, buffer, read_write>):void {
  $B2: {
    %4:ptr<storage, u32, read_write> = bufferView<u32> %p, 0u
    %a:ptr<storage, u32, read_write> = let %4
    ret
  }
}
%bar = func():void {
  $B3: {
    %7:void = call %foo, %gv
    ret
  }
}
%baz = func():void {
  $B4: {
    %9:void = call %foo, %gv
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv:ptr<storage, buffer<64>, read_write> = var undef @binding_point(0, 0)
}

%foo = func(%p:ptr<storage, buffer, read_write>):void {
  $B2: {
    %4:ptr<storage, u32, read_write> = bufferView<u32> %p, 0u, 64u
    %a:ptr<storage, u32, read_write> = let %4
    ret
  }
}
%bar = func():void {
  $B3: {
    %7:void = call %foo, %gv
    ret
  }
}
%baz = func():void {
  $B4: {
    %9:void = call %foo, %gv
    ret
  }
}
)";

    Run(PropagateBufferSizes);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PropagateBufferSizesTest, BufferLength_Parameter_MultiSource) {
    auto* foo = b.Function("foo", ty.void_());
    auto* foo_p = b.FunctionParam("p", ty.ptr(storage, ty.unsized_buffer()));
    foo->SetParams({foo_p});
    b.Append(foo->Block(), [&] {
        auto* call = b.Call(ty.u32(), BuiltinFn::kBufferLength, foo_p);
        b.Let("a", call);
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    auto* bar_q = b.FunctionParam("q", ty.ptr(storage, ty.buffer(128)));
    bar->SetParams({bar_q});
    b.Append(bar->Block(), [&] {
        b.Call(ty.void_(), foo, bar_q);
        b.Return(bar);
    });

    auto* baz = b.Function("baz", ty.void_());
    auto* baz_q = b.FunctionParam("r", ty.ptr(storage, ty.buffer(64)));
    baz->SetParams({baz_q});
    b.Append(baz->Block(), [&] {
        b.Call(ty.void_(), foo, baz_q);
        b.Return(baz);
    });

    auto* src = R"(
%foo = func(%p:ptr<storage, buffer, read_write>):void {
  $B1: {
    %3:u32 = bufferLength %p
    %a:u32 = let %3
    ret
  }
}
%bar = func(%q:ptr<storage, buffer<128>, read_write>):void {
  $B2: {
    %7:void = call %foo, %q
    ret
  }
}
%baz = func(%r:ptr<storage, buffer<64>, read_write>):void {
  $B3: {
    %10:void = call %foo, %r
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = func(%p:ptr<storage, buffer, read_write>, %3:u32):void {
  $B1: {
    %4:u32 = bufferLength %p, %3
    %a:u32 = let %4
    ret
  }
}
%bar = func(%q:ptr<storage, buffer<128>, read_write>):void {
  $B2: {
    %8:u32 = bufferLength %q, 128u
    %9:void = call %foo, %q, %8
    ret
  }
}
%baz = func(%r:ptr<storage, buffer<64>, read_write>):void {
  $B3: {
    %12:u32 = bufferLength %r, 64u
    %13:void = call %foo, %r, %12
    ret
  }
}
)";

    Run(PropagateBufferSizes);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PropagateBufferSizesTest, BufferView_Variable_MultiSource_LongChain) {
    auto* gv1 = b.Var("gv1", ty.ptr(storage, ty.unsized_buffer(), read_write));
    gv1->SetBindingPoint(0, 0);
    mod.root_block->Append(gv1);

    auto* gv2 = b.Var("gv2", ty.ptr(storage, ty.unsized_buffer(), read_write));
    gv2->SetBindingPoint(0, 1);
    mod.root_block->Append(gv2);

    auto* foo1 = b.Function("foo1", ty.void_());
    auto* foo1_p = b.FunctionParam("p1", ty.ptr(storage, ty.unsized_buffer()));
    foo1->SetParams({foo1_p});
    b.Append(foo1->Block(), [&] {
        auto* call = b.CallExplicit(ty.ptr(storage, ty.u32()), BuiltinFn::kBufferView,
                                    Vector{ty.u32()}, foo1_p, 0_u);
        b.Let("a", call);
        b.Return(foo1);
    });

    auto* foo2 = b.Function("foo2", ty.void_());
    auto* foo2_p = b.FunctionParam("p2", ty.ptr(storage, ty.unsized_buffer()));
    foo2->SetParams({foo2_p});
    b.Append(foo2->Block(), [&] {
        b.Call(ty.void_(), foo1, foo2_p);
        b.Return(foo2);
    });

    auto* foo3 = b.Function("foo3", ty.void_());
    auto* foo3_p = b.FunctionParam("p3", ty.ptr(storage, ty.unsized_buffer()));
    foo3->SetParams({foo3_p});
    b.Append(foo3->Block(), [&] {
        b.Call(ty.void_(), foo2, foo3_p);
        b.Return(foo3);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        b.Call(ty.void_(), foo3, gv1);
        b.Return(bar);
    });

    auto* baz = b.Function("baz", ty.void_());
    b.Append(baz->Block(), [&] {
        b.Call(ty.void_(), foo3, gv2);
        b.Return(baz);
    });

    auto* src = R"(
$B1: {  # root
  %gv1:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, buffer, read_write> = var undef @binding_point(0, 1)
}

%foo1 = func(%p1:ptr<storage, buffer, read_write>):void {
  $B2: {
    %5:ptr<storage, u32, read_write> = bufferView<u32> %p1, 0u
    %a:ptr<storage, u32, read_write> = let %5
    ret
  }
}
%foo2 = func(%p2:ptr<storage, buffer, read_write>):void {
  $B3: {
    %9:void = call %foo1, %p2
    ret
  }
}
%foo3 = func(%p3:ptr<storage, buffer, read_write>):void {
  $B4: {
    %12:void = call %foo2, %p3
    ret
  }
}
%bar = func():void {
  $B5: {
    %14:void = call %foo3, %gv1
    ret
  }
}
%baz = func():void {
  $B6: {
    %16:void = call %foo3, %gv2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv1:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, buffer, read_write> = var undef @binding_point(0, 1)
}

%foo1 = func(%p1:ptr<storage, buffer, read_write>, %5:u32):void {
  $B2: {
    %6:ptr<storage, u32, read_write> = bufferView<u32> %p1, 0u, %5
    %a:ptr<storage, u32, read_write> = let %6
    ret
  }
}
%foo2 = func(%p2:ptr<storage, buffer, read_write>, %10:u32):void {
  $B3: {
    %11:u32 = bufferLength %p2, %10
    %12:void = call %foo1, %p2, %11
    ret
  }
}
%foo3 = func(%p3:ptr<storage, buffer, read_write>, %15:u32):void {
  $B4: {
    %16:u32 = bufferLength %p3, %15
    %17:void = call %foo2, %p3, %16
    ret
  }
}
%bar = func():void {
  $B5: {
    %19:u32 = bufferLength %gv1
    %20:void = call %foo3, %gv1, %19
    ret
  }
}
%baz = func():void {
  $B6: {
    %22:u32 = bufferLength %gv2
    %23:void = call %foo3, %gv2, %22
    ret
  }
}
)";

    Run(PropagateBufferSizes);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PropagateBufferSizesTest, BufferView_Variable_MultiSource) {
    auto* gv1 = b.Var("gv1", ty.ptr(storage, ty.unsized_buffer(), read_write));
    gv1->SetBindingPoint(0, 0);
    mod.root_block->Append(gv1);

    auto* gv2 = b.Var("gv2", ty.ptr(storage, ty.unsized_buffer(), read_write));
    gv2->SetBindingPoint(0, 1);
    mod.root_block->Append(gv2);

    auto* foo = b.Function("foo", ty.void_());
    auto* foo_p = b.FunctionParam("p", ty.ptr(storage, ty.unsized_buffer()));
    foo->SetParams({foo_p});
    b.Append(foo->Block(), [&] {
        auto* call = b.CallExplicit(ty.ptr(storage, ty.u32()), BuiltinFn::kBufferView,
                                    Vector{ty.u32()}, foo_p, 0_u);
        b.Let("a", call);
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        b.Call(ty.void_(), foo, gv1);
        b.Return(bar);
    });

    auto* baz = b.Function("baz", ty.void_());
    b.Append(baz->Block(), [&] {
        b.Call(ty.void_(), foo, gv2);
        b.Return(baz);
    });

    auto* src = R"(
$B1: {  # root
  %gv1:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, buffer, read_write> = var undef @binding_point(0, 1)
}

%foo = func(%p:ptr<storage, buffer, read_write>):void {
  $B2: {
    %5:ptr<storage, u32, read_write> = bufferView<u32> %p, 0u
    %a:ptr<storage, u32, read_write> = let %5
    ret
  }
}
%bar = func():void {
  $B3: {
    %8:void = call %foo, %gv1
    ret
  }
}
%baz = func():void {
  $B4: {
    %10:void = call %foo, %gv2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv1:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, buffer, read_write> = var undef @binding_point(0, 1)
}

%foo = func(%p:ptr<storage, buffer, read_write>, %5:u32):void {
  $B2: {
    %6:ptr<storage, u32, read_write> = bufferView<u32> %p, 0u, %5
    %a:ptr<storage, u32, read_write> = let %6
    ret
  }
}
%bar = func():void {
  $B3: {
    %9:u32 = bufferLength %gv1
    %10:void = call %foo, %gv1, %9
    ret
  }
}
%baz = func():void {
  $B4: {
    %12:u32 = bufferLength %gv2
    %13:void = call %foo, %gv2, %12
    ret
  }
}
)";

    Run(PropagateBufferSizes);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_PropagateBufferSizesTest, LetChain) {
    auto* gv1 = b.Var("gv1", ty.ptr(storage, ty.unsized_buffer(), read_write));
    gv1->SetBindingPoint(0, 0);
    mod.root_block->Append(gv1);

    auto* gv2 = b.Var("gv2", ty.ptr(storage, ty.unsized_buffer(), read_write));
    gv2->SetBindingPoint(0, 1);
    mod.root_block->Append(gv2);

    auto* foo = b.Function("foo", ty.void_());
    auto* foo_p = b.FunctionParam("p", ty.ptr(storage, ty.unsized_buffer()));
    foo->SetParams({foo_p});
    b.Append(foo->Block(), [&] {
        auto* l1 = b.Let("l1", foo_p);
        auto* l2 = b.Let("l2", l1);
        auto* call = b.CallExplicit(ty.ptr(storage, ty.u32()), BuiltinFn::kBufferView,
                                    Vector{ty.u32()}, l2, 0_u);
        b.Let("a", call);
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        auto* l3 = b.Let("l3", gv1);
        b.Call(ty.void_(), foo, l3);
        b.Return(bar);
    });

    auto* baz = b.Function("baz", ty.void_());
    b.Append(baz->Block(), [&] {
        auto* l4 = b.Let("l4", gv2);
        auto* l5 = b.Let("l5", l4);
        b.Call(ty.void_(), foo, l5);
        b.Return(baz);
    });

    auto* src = R"(
$B1: {  # root
  %gv1:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, buffer, read_write> = var undef @binding_point(0, 1)
}

%foo = func(%p:ptr<storage, buffer, read_write>):void {
  $B2: {
    %l1:ptr<storage, buffer, read_write> = let %p
    %l2:ptr<storage, buffer, read_write> = let %l1
    %7:ptr<storage, u32, read_write> = bufferView<u32> %l2, 0u
    %a:ptr<storage, u32, read_write> = let %7
    ret
  }
}
%bar = func():void {
  $B3: {
    %l3:ptr<storage, buffer, read_write> = let %gv1
    %11:void = call %foo, %l3
    ret
  }
}
%baz = func():void {
  $B4: {
    %l4:ptr<storage, buffer, read_write> = let %gv2
    %l5:ptr<storage, buffer, read_write> = let %l4
    %15:void = call %foo, %l5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %gv1:ptr<storage, buffer, read_write> = var undef @binding_point(0, 0)
  %gv2:ptr<storage, buffer, read_write> = var undef @binding_point(0, 1)
}

%foo = func(%p:ptr<storage, buffer, read_write>, %5:u32):void {
  $B2: {
    %l1:ptr<storage, buffer, read_write> = let %p
    %l2:ptr<storage, buffer, read_write> = let %l1
    %8:ptr<storage, u32, read_write> = bufferView<u32> %l2, 0u, %5
    %a:ptr<storage, u32, read_write> = let %8
    ret
  }
}
%bar = func():void {
  $B3: {
    %l3:ptr<storage, buffer, read_write> = let %gv1
    %12:u32 = bufferLength %l3
    %13:void = call %foo, %l3, %12
    ret
  }
}
%baz = func():void {
  $B4: {
    %l4:ptr<storage, buffer, read_write> = let %gv2
    %l5:ptr<storage, buffer, read_write> = let %l4
    %17:u32 = bufferLength %l5
    %18:void = call %foo, %l5, %17
    ret
  }
}
)";

    Run(PropagateBufferSizes);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
