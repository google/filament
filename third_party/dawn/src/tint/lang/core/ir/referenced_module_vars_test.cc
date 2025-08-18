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

#include "src/tint/lang/core/ir/referenced_module_vars.h"

#include <string>

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using ::testing::ElementsAre;

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_ReferencedModuleVarsTest = IRTestHelper;

TEST_F(IR_ReferencedModuleVarsTest, EmptyRootBlock) {
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

    ReferencedModuleVars<Module> vars(mod);
    auto& foo_vars = vars.TransitiveReferences(foo);
    EXPECT_TRUE(foo_vars.IsEmpty());
}

TEST_F(IR_ReferencedModuleVarsTest, DirectUse) {
    // Referenced.
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    // Not referenced.
    mod.root_block->Append(b.Var<workgroup, u32>("c"));

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
  %b:ptr<workgroup, u32, read_write> = var undef
  %c:ptr<workgroup, u32, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %5:u32 = load %a
    %6:u32 = load %b
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleVars<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, var_b));
}

TEST_F(IR_ReferencedModuleVarsTest, DirectUse_DeclarationOrder) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    auto* var_c = mod.root_block->Append(b.Var<workgroup, u32>("c"));
    auto* var_d = mod.root_block->Append(b.Var<workgroup, u32>("d"));
    auto* var_e = mod.root_block->Append(b.Var<workgroup, u32>("e"));

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Load(var_b);
        b.Load(var_e);
        b.Load(var_d);
        b.Load(var_c);
        b.Load(var_a);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
  %b:ptr<workgroup, u32, read_write> = var undef
  %c:ptr<workgroup, u32, read_write> = var undef
  %d:ptr<workgroup, u32, read_write> = var undef
  %e:ptr<workgroup, u32, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %7:u32 = load %b
    %8:u32 = load %e
    %9:u32 = load %d
    %10:u32 = load %c
    %11:u32 = load %a
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleVars<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, var_b, var_c, var_d, var_e));
}

TEST_F(IR_ReferencedModuleVarsTest, DirectUse_MultipleFunctions) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    auto* var_c = mod.root_block->Append(b.Var<workgroup, u32>("c"));

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_c);
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
}

%foo = func():void {
  $B2: {
    %5:u32 = load %a
    %6:u32 = load %b
    ret
  }
}
%bar = func():void {
  $B3: {
    %8:u32 = load %a
    %9:u32 = load %c
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

    ReferencedModuleVars<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, var_b));
    EXPECT_THAT(vars.TransitiveReferences(bar), ElementsAre(var_a, var_c));
    EXPECT_TRUE(vars.TransitiveReferences(zoo).IsEmpty());
}

TEST_F(IR_ReferencedModuleVarsTest, DirectUse_NestedInControlFlow) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    auto* var_c = mod.root_block->Append(b.Var<workgroup, u32>("c"));
    auto* var_d = mod.root_block->Append(b.Var<workgroup, u32>("c"));

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
                b.Load(var_d);
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
  %c_1:ptr<workgroup, u32, read_write> = var undef  # %c_1: 'c'
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
            %9:u32 = load %c_1
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

    ReferencedModuleVars<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, var_b, var_c, var_d));
}

TEST_F(IR_ReferencedModuleVarsTest, IndirectUse) {
    // Directly used by foo.
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    // Directly used by bar, called by zoo and foo.
    auto* var_b = mod.root_block->Append(b.Var<workgroup, u32>("b"));
    // Not used.
    mod.root_block->Append(b.Var<workgroup, u32>("c"));

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {  //
        b.Load(var_b);
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
  %b:ptr<workgroup, u32, read_write> = var undef
  %c:ptr<workgroup, u32, read_write> = var undef
}

%bar = func():void {
  $B2: {
    %5:u32 = load %b
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

    ReferencedModuleVars<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(bar), ElementsAre(var_b));
    EXPECT_THAT(vars.TransitiveReferences(zoo), ElementsAre(var_b));
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a, var_b));
}

TEST_F(IR_ReferencedModuleVarsTest, NoFunctionVars) {
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

    ReferencedModuleVars<Module> vars(mod);
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_a));
}

TEST_F(IR_ReferencedModuleVarsTest, Predicate) {
    auto* var_a = mod.root_block->Append(b.Var<workgroup, u32>("a"));
    auto* var_b = mod.root_block->Append(b.Var<private_, u32>("b"));
    auto* var_c = mod.root_block->Append(b.Var<workgroup, u32>("c"));
    auto* var_d = mod.root_block->Append(b.Var<private_, u32>("d"));
    auto* var_e = mod.root_block->Append(b.Var<workgroup, u32>("e"));

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {  //
        b.Load(var_a);
        b.Load(var_b);
        b.Load(var_c);
        b.Load(var_d);
        b.Load(var_e);
        b.Return(foo);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, u32, read_write> = var undef
  %b:ptr<private, u32, read_write> = var undef
  %c:ptr<workgroup, u32, read_write> = var undef
  %d:ptr<private, u32, read_write> = var undef
  %e:ptr<workgroup, u32, read_write> = var undef
}

%foo = func():void {
  $B2: {
    %7:u32 = load %a
    %8:u32 = load %b
    %9:u32 = load %c
    %10:u32 = load %d
    %11:u32 = load %e
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    ReferencedModuleVars<Module> vars(mod, [](const Var* var) {
        auto* view = var->Result()->Type()->As<type::MemoryView>();
        return view->AddressSpace() == AddressSpace::kPrivate;
    });
    EXPECT_THAT(vars.TransitiveReferences(foo), ElementsAre(var_b, var_d));
}

TEST_F(IR_ReferencedModuleVarsTest, ReferencesForNullFunction) {
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

    ReferencedModuleVars<Module> vars(mod);
    auto& null_vars = vars.TransitiveReferences(nullptr);
    EXPECT_TRUE(null_vars.IsEmpty());
}

}  // namespace
}  // namespace tint::core::ir
