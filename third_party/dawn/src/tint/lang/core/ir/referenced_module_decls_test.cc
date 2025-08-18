// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/referenced_module_decls.h"

#include <string>

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using ::testing::ElementsAre;

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_ReferencedModuleDeclsTest = IRTestHelper;

TEST_F(IR_ReferencedModuleDeclsTest, EmptyRootBlock) {
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    auto& foo_vars = vars.TransitiveReferences(foo);
    EXPECT_TRUE(foo_vars.IsEmpty());
}

TEST_F(IR_ReferencedModuleDeclsTest, DirectUse) {
    // Referenced.
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    auto* inst_1 = mod.root_block->Append(b.Add(ty.i32(), 1_i, 2_i));
    auto* over_a = mod.root_block->Append(b.Override("o", inst_1));
    over_a->As<core::ir::Override>()->SetOverrideId(OverrideId{1});

    // Not referenced.
    mod.root_block->Append(b.Var<workgroup, u32>("c"));
    mod.root_block->Append(b.Override("p", ty.i32()));
    mod.root_block->Append(b.Multiply(ty.i32(), 2_i, 4_i));

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Let("c", over_a);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
  %b:ptr<workgroup, u32, read_write> = var undef
  %3:i32 = add 1i, 2i
  %o:i32 = override %3 @id(1)
  %c:ptr<workgroup, u32, read_write> = var undef
  %p:i32 = override undef
  %7:i32 = mul 2i, 4i
}

%foo = func():void {
  $B2: {
    %9:u32 = load %a
    %10:u32 = load %b
    %c_1:i32 = let %o  # %c_1: 'c'
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, var_b, over_a, inst_1));
}

TEST_F(IR_ReferencedModuleDeclsTest, DirectUse_DeclarationOrder) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    auto* var_c = mod.root_block->Append(b.Var<workgroup, u32>("c"));
    auto* over_d = mod.root_block->Append(b.Override("d", ty.i32()));
    auto* over_e = mod.root_block->Append(b.Override("e", ty.i32()));
    over_e->As<core::ir::Override>()->SetOverrideId(OverrideId{1});

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Load(var_b);
        b.Let("a", over_e);
        b.Let("b", over_d);
        b.Load(var_c);
        b.Load(var_a);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
  %b:ptr<workgroup, u32, read_write> = var undef
  %c:ptr<workgroup, u32, read_write> = var undef
  %d:i32 = override undef
  %e:i32 = override undef @id(1)
}

%foo = func():void {
  $B2: {
    %7:u32 = load %b
    %a_1:i32 = let %e  # %a_1: 'a'
    %b_1:i32 = let %d  # %b_1: 'b'
    %10:u32 = load %c
    %11:u32 = load %a
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, var_b, var_c, over_d, over_e));
}

TEST_F(IR_ReferencedModuleDeclsTest, DirectUse_MultipleFunctions) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    auto* var_c = mod.root_block->Append(b.Var<workgroup, u32>("c"));
    auto* over_d = mod.root_block->Append(b.Override("d", ty.i32()));

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Let("a", over_d);
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_c);
        b.Let("b", over_d);
        b.Return(bar);
    });

    auto* zoo = b.Function("zoo", ty.void_());
    b.Append(zoo->Block(), [&] {  //
        b.Return(zoo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
  %b:ptr<workgroup, u32, read_write> = var undef
  %c:ptr<workgroup, u32, read_write> = var undef
  %d:i32 = override undef
}

%foo = func():void {
  $B2: {
    %6:u32 = load %a
    %7:u32 = load %b
    %a_1:i32 = let %d  # %a_1: 'a'
    ret
  }
}
%bar = func():void {
  $B3: {
    %10:u32 = load %a
    %11:u32 = load %c
    %b_1:i32 = let %d  # %b_1: 'b'
    ret
  }
}
%zoo = func():void {
  $B4: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, var_b, over_d));
    EXPECT_THAT(vars.TransitiveReferences(bar), ElementsAre(var_a, var_c, over_d));
    EXPECT_TRUE(vars.TransitiveReferences(zoo).IsEmpty());
}

TEST_F(IR_ReferencedModuleDeclsTest, DirectUse_NestedInControlFlow) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    auto* var_c = mod.root_block->Append(b.Var<workgroup, u32>("c"));
    auto* over_d = mod.root_block->Append(b.Override("c", ty.i32()));

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        auto* ifelse = b.If(true);
        b.Append(ifelse->True(), [&] {
            b.Load(var_a);
            b.ExitIf(ifelse);
        });
        b.Append(ifelse->False(), [&] {
            auto* loop = b.Loop();
            b.Append(loop->Initializer(), [&] {
                b.Load(var_b);
                b.NextIteration(loop);
            });
            b.Append(loop->Body(), [&] {
                b.Load(var_c);
                b.Continue(loop);
            });
            b.Append(loop->Continuing(), [&] {
                b.Load(over_d);
                b.NextIteration(loop);
            });
            b.ExitIf(ifelse);
        });
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
  %b:ptr<workgroup, u32, read_write> = var undef
  %c:ptr<workgroup, u32, read_write> = var undef
  %c_1:i32 = override undef  # %c_1: 'c'
}

%foo = func():void {
  $B2: {
    if true [t: $B3, f: $B4] {  # if_1
      $B3: {  # true
        %6:u32 = load %a
        exit_if  # if_1
      }
      $B4: {  # false
        loop [i: $B5, b: $B6, c: $B7] {  # loop_1
          $B5: {  # initializer
            %7:u32 = load %b
            next_iteration  # -> $B6
          }
          $B6: {  # body
            %8:u32 = load %c
            continue  # -> $B7
          }
          $B7: {  # continuing
            %9:i32 = load %c_1
            next_iteration  # -> $B6
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, var_b, var_c, over_d));
}

TEST_F(IR_ReferencedModuleDeclsTest, IndirectUse) {
    // Directly used by foo.
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    // Directly used by bar, called by zoo and foo.
    auto* over_d = mod.root_block->Append(b.Override("b", ty.i32()));
    // Not used.
    mod.root_block->Append(b.Var<workgroup, u32>("c"));

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {  //
        b.Load(over_d);
        b.Return(bar);
    });

    auto* zoo = b.Function("zoo", ty.void_());
    b.Append(zoo->Block(), [&] {  //
        b.Call(bar);
        b.Return(zoo);
    });

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Load(var_a);
        b.Call(zoo);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
  %b:i32 = override undef
  %c:ptr<workgroup, u32, read_write> = var undef
}

%bar = func():void {
  $B2: {
    %5:i32 = load %b
    ret
  }
}
%zoo = func():void {
  $B3: {
    %7:void = call %bar
    ret
  }
}
%foo = func():void {
  $B4: {
    %9:u32 = load %a
    %10:void = call %zoo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(bar), ElementsAre(over_d));
    EXPECT_THAT(vars.TransitiveReferences(zoo), ElementsAre(over_d));
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, over_d));
}

TEST_F(IR_ReferencedModuleDeclsTest, NoFunctionVars) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        auto* var_b = b.Var<function, u32>("b");
        b.Load(var_a);
        b.Load(var_b);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:u32 = load %a
    %5:u32 = load %b
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a));
}

TEST_F(IR_ReferencedModuleDeclsTest, ReferencesForNullFunction) {
    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo);
    });

    auto* src = R"(
%foo = func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    auto& null_vars = vars.TransitiveReferences(nullptr);
    EXPECT_TRUE(null_vars.IsEmpty());
}

TEST_F(IR_ReferencedModuleDeclsTest, WorkgroupSize) {
    auto* over_a = mod.root_block->Append(b.Override("o", ty.u32()));
    over_a->As<core::ir::Override>()->SetOverrideId(OverrideId{1});

    auto* foo = b.ComputeFunction("foo");
    foo->SetWorkgroupSize(over_a->Result(), b.Constant(1_u), b.Constant(1_u));
    b.Append(foo->Block(), [&] {  //
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %o:u32 = override undef @id(1)
}

%foo = @compute @workgroup_size(%o, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(over_a));
}

TEST_F(IR_ReferencedModuleDeclsTest, ArrayTypeCount) {
    auto* over_a = mod.root_block->Append(b.Override("o", ty.u32()));
    over_a->As<core::ir::Override>()->SetOverrideId(OverrideId{1});

    auto* c1 = ty.Get<core::ir::type::ValueArrayCount>(over_a->Result());
    auto* a1 = ty.Get<core::type::Array>(ty.i32(), c1, 4u, 4u, 4u, 4u);

    auto* var_a = mod.root_block->Append(b.Var("a", ty.ptr(workgroup, a1, read_write)));

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Let("l", var_a);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %o:u32 = override undef @id(1)
  %a:ptr<workgroup, array<i32, %o>, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %l:ptr<workgroup, array<i32, %o>, read_write> = let %a
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleDecls<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, over_a));
}

}  // namespace
}  // namespace tint::core::ir
