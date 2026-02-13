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

#include "src/tint/lang/core/ir/transform/value_to_let.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_ValueToLetTest = TransformTest;

TEST_F(IR_ValueToLetTest, Empty) {
    auto* expect = R"(
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, NoModify_Blah) {
    auto* func = b.Function("F", ty.void_());
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
%F = func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = src;

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, NoModify_Unsequenced) {
    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        auto* z = b.Let("z", b.Add<i32>(x, y));
        b.Return(fn, z);
    });

    auto* src = R"(
%F = func():i32 {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:i32 = add %x, %y
    %z:i32 = let %4
    ret %z
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = src;

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, NoModify_Bitcast) {
    auto* fn = b.Function("F", ty.u32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Let("x", 1_i);
        auto* y = b.Bitcast<u32>(x);
        b.Return(fn, y);
    });

    auto* src = R"(
%F = func():u32 {
  $B1: {
    %x:i32 = let 1i
    %3:u32 = bitcast %x
    ret %3
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = src;

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, NoModify_SequencedValueUsedWithNonSequenced) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Name("x", b.Call(rmw, 1_i));
        // select is called with one, inlinable sequenced operand and two non-sequenced values.
        auto* y = b.Name("y", b.Call<i32>(core::BuiltinFn::kSelect, 2_i, x, false));
        b.Return(fn, y);
    });

    auto* src = R"(
$B1: {  # root
  %i:ptr<private, i32, read_write> = var undef
}

%rmw = func(%p:i32):i32 {
  $B2: {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 {
  $B3: {
    %x:i32 = call %rmw, 1i
    %y:i32 = select 2i, %x, false
    ret %y
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = src;

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, NoModify_Inlinable_NestedCalls) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Name("x", b.Call(rmw, 1_i));
        auto* y = b.Name("y", b.Call(rmw, x));
        auto* z = b.Name("z", b.Call(rmw, y));
        b.Return(fn, z);
    });

    auto* src = R"(
$B1: {  # root
  %i:ptr<private, i32, read_write> = var undef
}

%rmw = func(%p:i32):i32 {
  $B2: {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 {
  $B3: {
    %x:i32 = call %rmw, 1i
    %y:i32 = call %rmw, %x
    %z:i32 = call %rmw, %y
    ret %z
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = src;

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, NoModify_LetUsedTwice) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        // No need to create more lets, as these are already in lets
        auto* x = b.Let("x", b.Call(rmw, 1_i));
        auto* y = b.Name("y", b.Add<i32>(x, x));
        b.Return(fn, y);
    });

    auto* src = R"(
$B1: {  # root
  %i:ptr<private, i32, read_write> = var undef
}

%rmw = func(%p:i32):i32 {
  $B2: {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 {
  $B3: {
    %8:i32 = call %rmw, 1i
    %x:i32 = let %8
    %y:i32 = add %x, %x
    ret %y
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = src;

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, NoModify_VarUsedTwice) {
    auto* p = b.FunctionParam<ptr<function, i32, read_write>>("p");
    auto* fn_g = b.Function("g", ty.i32());
    fn_g->SetParams({p});
    b.Append(fn_g->Block(), [&] { b.Return(fn_g, b.Load(p)); });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* x = b.Let("x", b.Call(fn_g, v));
        auto* y = b.Let("y", b.Call(fn_g, v));
        b.Return(fn, b.Add<i32>(x, y));
    });

    auto* src = R"(
%g = func(%p:ptr<function, i32, read_write>):i32 {
  $B1: {
    %3:i32 = load %p
    ret %3
  }
}
%F = func():i32 {
  $B2: {
    %v:ptr<function, i32, read_write> = var undef
    %6:i32 = call %g, %v
    %x:i32 = let %6
    %8:i32 = call %g, %v
    %y:i32 = let %8
    %10:i32 = add %x, %y
    ret %10
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = src;

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, VarLoadUsedTwice) {
    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* l = b.Name("l", b.Load(v));
        b.Return(fn, b.Add<i32>(l, l));
    });

    auto* src = R"(
%F = func():i32 {
  $B1: {
    %v:ptr<function, i32, read_write> = var undef
    %l:i32 = load %v
    %4:i32 = add %l, %l
    ret %4
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
%F = func():i32 {
  $B1: {
    %v:ptr<function, i32, read_write> = var undef
    %3:i32 = load %v
    %l:i32 = let %3
    %5:i32 = add %l, %l
    ret %5
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, VarLoad_ThenStore_ThenUse) {
    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* l = b.Name("l", b.Load(v));
        b.Store(v, 1_i);
        b.Return(fn, l);
    });

    auto* src = R"(
%F = func():i32 {
  $B1: {
    %v:ptr<function, i32, read_write> = var undef
    %l:i32 = load %v
    store %v, 1i
    ret %l
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
%F = func():i32 {
  $B1: {
    %v:ptr<function, i32, read_write> = var undef
    %3:i32 = load %v
    %l:i32 = let %3
    store %v, 1i
    ret %l
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, Call_ThenLoad_ThenUseCallBeforeLoad) {
    auto* v = b.Var<private_, i32>("v");
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {
        b.Store(v, 42_i);
        b.Return(foo, 1_i);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* c = b.Call<i32>(foo);
        auto* l = b.Name("l", b.Load(v));
        auto* add = b.Name("add", b.Add<i32>(c, l));
        b.Return(fn, add);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():i32 {
  $B3: {
    %4:i32 = call %foo
    %l:i32 = load %v
    %add:i32 = add %4, %l
    ret %add
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():i32 {
  $B3: {
    %4:i32 = call %foo
    %5:i32 = let %4
    %l:i32 = load %v
    %add:i32 = add %5, %l
    ret %add
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, Call_ThenLoad_ThenUseLoadBeforeCall) {
    auto* v = b.Var<private_, i32>("v");
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {
        b.Store(v, 42_i);
        b.Return(foo, 1_i);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* c = b.Call<i32>(foo);
        auto* l = b.Name("l", b.Load(v));
        auto* add = b.Name("add", b.Add<i32>(l, c));
        b.Return(fn, add);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():i32 {
  $B3: {
    %4:i32 = call %foo
    %l:i32 = load %v
    %add:i32 = add %l, %4
    ret %add
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():i32 {
  $B3: {
    %4:i32 = call %foo
    %5:i32 = let %4
    %l:i32 = load %v
    %add:i32 = add %l, %5
    ret %add
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, Call_WithUseThatIsNeverUsed) {
    auto* v = b.Var<private_, i32>("v");
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {
        b.Store(v, 42_i);
        b.Return(foo, 1_i);
    });

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* c = b.Name("call", b.Call<i32>(foo));
        b.Name("add", b.Add<i32>(c, 1_i));
        b.Return(fn);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():void {
  $B3: {
    %call:i32 = call %foo
    %add:i32 = add %call, 1i
    ret
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():void {
  $B3: {
    %call:i32 = call %foo
    %5:i32 = add %call, 1i
    %add:i32 = let %5
    ret
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, ConstructIsNeverUsed) {
    auto* v = b.Var<private_, i32>("v");
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {
        b.Store(v, 42_i);
        b.Return(foo, 1_i);
    });

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* c = b.Name("call", b.Call<i32>(foo));
        b.Name("construct", b.Construct<vec4<i32>>(c));
        b.Return(fn);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():void {
  $B3: {
    %call:i32 = call %foo
    %construct:vec4<i32> = construct %call
    ret
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():void {
  $B3: {
    %call:i32 = call %foo
    %5:vec4<i32> = construct %call
    %construct:vec4<i32> = let %5
    ret
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, ConvertIsNeverUsed) {
    auto* v = b.Var<private_, i32>("v");
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {
        b.Store(v, 42_i);
        b.Return(foo, 1_i);
    });

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* c = b.Name("call", b.Call<i32>(foo));
        b.Name("convert", b.Convert<u32>(c));
        b.Return(fn);
    });

    auto* src = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():void {
  $B3: {
    %call:i32 = call %foo
    %convert:u32 = convert %call
    ret
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
$B1: {  # root
  %v:ptr<private, i32, read_write> = var undef
}

%foo = func():i32 {
  $B2: {
    store %v, 42i
    ret 1i
  }
}
%F = func():void {
  $B3: {
    %call:i32 = call %foo
    %5:u32 = convert %call
    %convert:u32 = let %5
    ret
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, TwoCalls_ThenUseReturnValues) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Name("x", b.Call(rmw, 1_i));
        auto* y = b.Name("y", b.Call(rmw, 2_i));
        auto* z = b.Name("z", b.Add<i32>(x, y));
        b.Return(fn, z);
    });

    auto* src = R"(
$B1: {  # root
  %i:ptr<private, i32, read_write> = var undef
}

%rmw = func(%p:i32):i32 {
  $B2: {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 {
  $B3: {
    %x:i32 = call %rmw, 1i
    %y:i32 = call %rmw, 2i
    %z:i32 = add %x, %y
    ret %z
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
$B1: {  # root
  %i:ptr<private, i32, read_write> = var undef
}

%rmw = func(%p:i32):i32 {
  $B2: {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 {
  $B3: {
    %8:i32 = call %rmw, 1i
    %x:i32 = let %8
    %y:i32 = call %rmw, 2i
    %z:i32 = add %x, %y
    ret %z
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);

    Run(ValueToLet, ValueToLetConfig{});  // running a second time should be no-op

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, SequencedUsedInDifferentBlock) {
    auto* i = b.Var<private_, i32>("i");
    b.ir.root_block->Append(i);

    auto* p = b.FunctionParam<i32>("p");
    auto* rmw = b.Function("rmw", ty.i32());
    rmw->SetParams({p});
    b.Append(rmw->Block(), [&] {
        auto* v = b.Let("v", b.Add<i32>(b.Load(i), p));
        b.Store(i, v);
        b.Return(rmw, v);
    });

    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* x = b.Name("x", b.Call(rmw, 1_i));
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {  //
            b.Return(fn, x);
        });
        b.Return(fn, 2_i);
    });

    auto* src = R"(
$B1: {  # root
  %i:ptr<private, i32, read_write> = var undef
}

%rmw = func(%p:i32):i32 {
  $B2: {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 {
  $B3: {
    %x:i32 = call %rmw, 1i
    if true [t: $B4] {  # if_1
      $B4: {  # true
        ret %x
      }
    }
    ret 2i
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
$B1: {  # root
  %i:ptr<private, i32, read_write> = var undef
}

%rmw = func(%p:i32):i32 {
  $B2: {
    %4:i32 = load %i
    %5:i32 = add %4, %p
    %v:i32 = let %5
    store %i, %v
    ret %v
  }
}
%F = func():i32 {
  $B3: {
    %8:i32 = call %rmw, 1i
    %x:i32 = let %8
    if true [t: $B4] {  # if_1
      $B4: {  # true
        ret %x
      }
    }
    ret 2i
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);

    Run(ValueToLet, ValueToLetConfig{});  // running a second time should be no-op

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, NameMe1) {
    auto* fn = b.Function("F", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var<function, i32>("v");
        auto* x = b.Load(v);
        auto* y = b.Add<i32>(x, 1_i);
        b.Store(v, 2_i);
        b.Return(fn, y);
    });

    auto* src = R"(
%F = func():i32 {
  $B1: {
    %v:ptr<function, i32, read_write> = var undef
    %3:i32 = load %v
    %4:i32 = add %3, 1i
    store %v, 2i
    ret %4
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
%F = func():i32 {
  $B1: {
    %v:ptr<function, i32, read_write> = var undef
    %3:i32 = load %v
    %4:i32 = add %3, 1i
    %5:i32 = let %4
    store %v, 2i
    ret %5
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});

    EXPECT_EQ(str(), expect);

    Run(ValueToLet, ValueToLetConfig{});  // running a second time should be no-op

    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, NameMe2) {
    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* i = b.Name("i", b.Call<i32>(core::BuiltinFn::kMax, 1_i, 2_i));
        auto* v = b.Var<function>("v", i);
        auto* x = b.Name("x", b.Call<i32>(core::BuiltinFn::kMax, 3_i, 4_i));
        auto* y = b.Name("y", b.Load(v));
        auto* z = b.Name("z", b.Add<i32>(y, x));
        b.Store(v, z);
        b.Return(fn);
    });

    auto* src = R"(
%F = func():void {
  $B1: {
    %i:i32 = max 1i, 2i
    %v:ptr<function, i32, read_write> = var %i
    %x:i32 = max 3i, 4i
    %y:i32 = load %v
    %z:i32 = add %y, %x
    store %v, %z
    ret
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
%F = func():void {
  $B1: {
    %i:i32 = max 1i, 2i
    %v:ptr<function, i32, read_write> = var %i
    %x:i32 = max 3i, 4i
    %y:i32 = load %v
    %z:i32 = add %y, %x
    store %v, %z
    ret
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});
    EXPECT_EQ(str(), expect);

    Run(ValueToLet, ValueToLetConfig{});  // running a second time should be no-op
    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, TextureInline) {
    core::ir::Var* tex = nullptr;
    core::ir::Var* sampler = nullptr;
    b.Append(b.ir.root_block, [&] {
        tex = b.Var(ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d)));
        tex->SetBindingPoint(0, 0);

        sampler = b.Var(ty.ptr(handle, ty.sampler()));
        sampler->SetBindingPoint(0, 1);
    });

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* coords = b.Construct(ty.vec2<f32>(), b.Value(1_f), b.Value(2_f));

        auto* t = b.Load(tex);
        auto* s = b.Load(sampler);
        b.Let("x", b.Call<vec4<f32>>(core::BuiltinFn::kTextureGather, t, s, coords));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureGather %5, %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
$B1: {  # root
  %1:ptr<handle, texture_depth_2d, read> = var undef @binding_point(0, 0)
  %2:ptr<handle, sampler, read> = var undef @binding_point(0, 1)
}

%foo = @fragment func():void {
  $B2: {
    %4:vec2<f32> = construct 1.0f, 2.0f
    %5:texture_depth_2d = load %1
    %6:sampler = load %2
    %7:vec4<f32> = textureGather %5, %6, %4
    %x:vec4<f32> = let %7
    ret
  }
}
)";

    Run(ValueToLet, ValueToLetConfig{});
    EXPECT_EQ(str(), expect);

    Run(ValueToLet, ValueToLetConfig{});  // running a second time should be no-op
    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, AccessToLetWithFunctionParams) {
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] { b.Return(f, 0_i); });

    auto* g = b.Function("g", ty.i32());
    b.Append(g->Block(), [&] { b.Return(f, 0_i); });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* arr = b.Var("arr", ty.ptr<function, array<i32, 4>, read_write>());
        auto* c = b.Call(ty.i32(), f);
        auto* access = b.Access(ty.ptr<function, i32, read_write>(), arr, c);
        auto* p = b.Let("p", access);
        auto* c2 = b.Call(ty.i32(), g);
        b.Let("y", c2);
        b.Let("x", b.Load(p));
        b.Return(foo);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    ret 0i
  }
}
%g = func():i32 {
  $B2: {
    ret 0i
  }
}
%foo = func():void {
  $B3: {
    %arr:ptr<function, array<i32, 4>, read_write> = var undef
    %5:i32 = call %f
    %6:ptr<function, i32, read_write> = access %arr, %5
    %p:ptr<function, i32, read_write> = let %6
    %8:i32 = call %g
    %y:i32 = let %8
    %10:i32 = load %p
    %x:i32 = let %10
    ret
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    ret 0i
  }
}
%g = func():i32 {
  $B2: {
    ret 0i
  }
}
%foo = func():void {
  $B3: {
    %arr:ptr<function, array<i32, 4>, read_write> = var undef
    %5:i32 = call %f
    %6:i32 = let %5
    %7:ptr<function, i32, read_write> = access %arr, %6
    %8:i32 = call %g
    %y:i32 = let %8
    %10:i32 = load %7
    %x:i32 = let %10
    ret
  }
}
)";

    ValueToLetConfig cfg{};
    cfg.replace_pointer_lets = true;
    Run(ValueToLet, cfg);
    EXPECT_EQ(str(), expect);

    Run(ValueToLet, cfg);  // running a second time should be no-op
    EXPECT_EQ(str(), expect);
}

TEST_F(IR_ValueToLetTest, AccessToLetWithNestedFunctionParams) {
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] { b.Return(f, 0_i); });

    auto* g = b.Function("g", ty.i32());
    b.Append(g->Block(), [&] { b.Return(f, 0_i); });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* arr = b.Var("arr", ty.ptr<function, array<i32, 4>, read_write>());
        auto* c = b.Call(ty.i32(), f);
        auto* d = b.Add(ty.i32(), c, 1_i);
        auto* access = b.Access(ty.ptr<function, i32, read_write>(), arr, d);
        auto* p = b.Let("p", access);
        auto* c2 = b.Call(ty.i32(), g);
        b.Let("y", c2);
        b.Let("x", b.Load(p));
        b.Return(foo);
    });

    auto* src = R"(
%f = func():i32 {
  $B1: {
    ret 0i
  }
}
%g = func():i32 {
  $B2: {
    ret 0i
  }
}
%foo = func():void {
  $B3: {
    %arr:ptr<function, array<i32, 4>, read_write> = var undef
    %5:i32 = call %f
    %6:i32 = add %5, 1i
    %7:ptr<function, i32, read_write> = access %arr, %6
    %p:ptr<function, i32, read_write> = let %7
    %9:i32 = call %g
    %y:i32 = let %9
    %11:i32 = load %p
    %x:i32 = let %11
    ret
  }
}
)";
    EXPECT_EQ(str(), src);

    auto* expect = R"(
%f = func():i32 {
  $B1: {
    ret 0i
  }
}
%g = func():i32 {
  $B2: {
    ret 0i
  }
}
%foo = func():void {
  $B3: {
    %arr:ptr<function, array<i32, 4>, read_write> = var undef
    %5:i32 = call %f
    %6:i32 = add %5, 1i
    %7:i32 = let %6
    %8:ptr<function, i32, read_write> = access %arr, %7
    %9:i32 = call %g
    %y:i32 = let %9
    %11:i32 = load %8
    %x:i32 = let %11
    ret
  }
}
)";

    ValueToLetConfig cfg{};
    cfg.replace_pointer_lets = true;
    Run(ValueToLet, cfg);
    EXPECT_EQ(str(), expect);

    Run(ValueToLet, cfg);  // running a second time should be no-op
    EXPECT_EQ(str(), expect);
}

}  // namespace
}  // namespace tint::core::ir::transform
