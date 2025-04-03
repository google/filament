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

#include "src/tint/lang/core/ir/transform/single_entry_point.h"

#include <utility>

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/ir/type/array_count.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {
namespace {

class IR_SingleEntryPointTest : public TransformTest {
  protected:
    /// @returns a new entry point called @p name that references @p refs
    Function* EntryPoint(const char* name, std::initializer_list<Value*> refs = {}) {
        auto* func = Func(name, std::move(refs));
        func->SetStage(Function::PipelineStage::kFragment);
        return func;
    }

    /// @returns a new function called @p name that references @p refs
    Function* Func(const char* name, std::initializer_list<Value*> refs = {}) {
        auto* func = b.Function(name, ty.void_());
        b.Append(func->Block(), [&] {
            for (auto* ref : refs) {
                if (auto* f = ref->As<Function>()) {
                    b.Call(f);
                } else {
                    b.Let(ref->Type())->SetValue(ref);
                }
            }
            b.Return(func);
        });
        return func;
    }

    /// @returns a new module-scope variable called @p name
    InstructionResult* Var(const char* name) {
        auto* var = b.Var<private_, i32>(name);
        mod.root_block->Append(var);
        return var->Result();
    }

    /// @returns a new module-scope override called `name` with override `id` and possible
    /// `initializer`
    InstructionResult* Override(const char* name, uint16_t id, Value* initializer = nullptr) {
        auto* var = b.Override(name, ty.i32());
        var->SetOverrideId(OverrideId{id});
        if (initializer) {
            var->SetInitializer(initializer);
        }
        mod.root_block->Append(var);
        return var->Result();
    }
};
using IR_SingleEntryPointDeathTest = IR_SingleEntryPointTest;

TEST_F(IR_SingleEntryPointTest, EntryPointNotFound) {
    EntryPoint("main");

    auto* src = R"(
%main = @fragment func():void {
  $B1: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    EXPECT_DEATH_IF_SUPPORTED({ Run(SingleEntryPoint, "foo"); }, "internal compiler error");
}

TEST_F(IR_SingleEntryPointTest, NoChangesNeeded) {
    EntryPoint("main");

    auto* src = R"(
%main = @fragment func():void {
  $B1: {
    ret
  }
}
)";

    auto* expect = src;

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "main");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, TwoEntryPoints) {
    EntryPoint("foo");
    EntryPoint("bar");

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    ret
  }
}
%bar = @fragment func():void {
  $B2: {
    ret
  }
}
)";

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, DirectFunctionCalls) {
    auto* f1 = Func("f1");
    auto* f2 = Func("f2");
    auto* f3 = Func("f3");

    EntryPoint("foo", {f1, f2});
    EntryPoint("bar", {f3});

    auto* src = R"(
%f1 = func():void {
  $B1: {
    ret
  }
}
%f2 = func():void {
  $B2: {
    ret
  }
}
%f3 = func():void {
  $B3: {
    ret
  }
}
%foo = @fragment func():void {
  $B4: {
    %5:void = call %f1
    %6:void = call %f2
    ret
  }
}
%bar = @fragment func():void {
  $B5: {
    %8:void = call %f3
    ret
  }
}
)";

    auto* expect = R"(
%f1 = func():void {
  $B1: {
    ret
  }
}
%f2 = func():void {
  $B2: {
    ret
  }
}
%foo = @fragment func():void {
  $B3: {
    %4:void = call %f1
    %5:void = call %f2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, DirectVariables) {
    auto* v1 = Var("v1");
    auto* v2 = Var("v2");
    auto* v3 = Var("v3");

    EntryPoint("foo", {v1, v2});
    EntryPoint("bar", {v3});

    auto* src = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var undef
  %v2:ptr<private, i32, read_write> = var undef
  %v3:ptr<private, i32, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %5:ptr<private, i32, read_write> = let %v1
    %6:ptr<private, i32, read_write> = let %v2
    ret
  }
}
%bar = @fragment func():void {
  $B3: {
    %8:ptr<private, i32, read_write> = let %v3
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var undef
  %v2:ptr<private, i32, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %4:ptr<private, i32, read_write> = let %v1
    %5:ptr<private, i32, read_write> = let %v2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, DirectOverrides) {
    auto* o1 = Override("o1", 1);
    auto* o2 = Override("o2", 2);
    auto* o3 = Override("o3", 3);

    EntryPoint("foo", {o1, o2});
    EntryPoint("bar", {o3});

    auto* src = R"(
$B1: {  # root
  %o1:i32 = override undef @id(1)
  %o2:i32 = override undef @id(2)
  %o3:i32 = override undef @id(3)
}

%foo = @fragment func():void {
  $B2: {
    %5:i32 = let %o1
    %6:i32 = let %o2
    ret
  }
}
%bar = @fragment func():void {
  $B3: {
    %8:i32 = let %o3
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %o1:i32 = override undef @id(1)
  %o2:i32 = override undef @id(2)
}

%foo = @fragment func():void {
  $B2: {
    %4:i32 = let %o1
    %5:i32 = let %o2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, DirectOverridesWithInitializer) {
    Value* init1 = nullptr;
    Value* init2 = nullptr;
    Value* init3 = nullptr;
    b.Append(mod.root_block, [&] {
        init1 = b.Multiply(ty.i32(), 2_i, 4_i)->Result();
        auto* x = b.Multiply(ty.i32(), 2_i, 4_i);
        init2 = b.Add(ty.i32(), x, 4_i)->Result();

        auto* y = b.Multiply(ty.i32(), 3_i, 5_i);
        init3 = b.Add(ty.i32(), y, 5_i)->Result();
    });

    auto* o1 = Override("o1", 1, init1);
    auto* o2 = Override("o2", 2, init2);
    auto* o3 = Override("o3", 3, init3);

    EntryPoint("foo", {o1, o2});
    EntryPoint("bar", {o3});

    auto* src = R"(
$B1: {  # root
  %1:i32 = mul 2i, 4i
  %2:i32 = mul 2i, 4i
  %3:i32 = add %2, 4i
  %4:i32 = mul 3i, 5i
  %5:i32 = add %4, 5i
  %o1:i32 = override %1 @id(1)
  %o2:i32 = override %3 @id(2)
  %o3:i32 = override %5 @id(3)
}

%foo = @fragment func():void {
  $B2: {
    %10:i32 = let %o1
    %11:i32 = let %o2
    ret
  }
}
%bar = @fragment func():void {
  $B3: {
    %13:i32 = let %o3
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %1:i32 = mul 2i, 4i
  %2:i32 = mul 2i, 4i
  %3:i32 = add %2, 4i
  %o1:i32 = override %1 @id(1)
  %o2:i32 = override %3 @id(2)
}

%foo = @fragment func():void {
  $B2: {
    %7:i32 = let %o1
    %8:i32 = let %o2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, TransitiveVariableReferences) {
    Var("unused_var");
    Func("unused_func");

    auto* v1 = Var("v1");
    auto* v2 = Var("v2");
    auto* v3 = Var("v3");

    auto* f1 = Func("f1", {v2, v3});
    auto* f2 = Func("f2", {f1});
    auto* f3 = Func("f3", {v1, f2});

    EntryPoint("foo", {f3});

    auto* src = R"(
$B1: {  # root
  %unused_var:ptr<private, i32, read_write> = var undef
  %v1:ptr<private, i32, read_write> = var undef
  %v2:ptr<private, i32, read_write> = var undef
  %v3:ptr<private, i32, read_write> = var undef
}

%unused_func = func():void {
  $B2: {
    ret
  }
}
%f1 = func():void {
  $B3: {
    %7:ptr<private, i32, read_write> = let %v2
    %8:ptr<private, i32, read_write> = let %v3
    ret
  }
}
%f2 = func():void {
  $B4: {
    %10:void = call %f1
    ret
  }
}
%f3 = func():void {
  $B5: {
    %12:ptr<private, i32, read_write> = let %v1
    %13:void = call %f2
    ret
  }
}
%foo = @fragment func():void {
  $B6: {
    %15:void = call %f3
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var undef
  %v2:ptr<private, i32, read_write> = var undef
  %v3:ptr<private, i32, read_write> = var undef
}

%f1 = func():void {
  $B2: {
    %5:ptr<private, i32, read_write> = let %v2
    %6:ptr<private, i32, read_write> = let %v3
    ret
  }
}
%f2 = func():void {
  $B3: {
    %8:void = call %f1
    ret
  }
}
%f3 = func():void {
  $B4: {
    %10:ptr<private, i32, read_write> = let %v1
    %11:void = call %f2
    ret
  }
}
%foo = @fragment func():void {
  $B5: {
    %13:void = call %f3
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, TransitiveOverrideReferences) {
    Override("unused_override", 5);
    Func("unused_func");

    auto* o1 = Override("o1", 1);
    auto* o2 = Override("o2", 2);
    auto* o3 = Override("o3", 3);

    auto* f1 = Func("f1", {o2, o3});
    auto* f2 = Func("f2", {f1});
    auto* f3 = Func("f3", {o1, f2});

    EntryPoint("foo", {f3});

    auto* src = R"(
$B1: {  # root
  %unused_override:i32 = override undef @id(5)
  %o1:i32 = override undef @id(1)
  %o2:i32 = override undef @id(2)
  %o3:i32 = override undef @id(3)
}

%unused_func = func():void {
  $B2: {
    ret
  }
}
%f1 = func():void {
  $B3: {
    %7:i32 = let %o2
    %8:i32 = let %o3
    ret
  }
}
%f2 = func():void {
  $B4: {
    %10:void = call %f1
    ret
  }
}
%f3 = func():void {
  $B5: {
    %12:i32 = let %o1
    %13:void = call %f2
    ret
  }
}
%foo = @fragment func():void {
  $B6: {
    %15:void = call %f3
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %o1:i32 = override undef @id(1)
  %o2:i32 = override undef @id(2)
  %o3:i32 = override undef @id(3)
}

%f1 = func():void {
  $B2: {
    %5:i32 = let %o2
    %6:i32 = let %o3
    ret
  }
}
%f2 = func():void {
  $B3: {
    %8:void = call %f1
    ret
  }
}
%f3 = func():void {
  $B4: {
    %10:i32 = let %o1
    %11:void = call %f2
    ret
  }
}
%foo = @fragment func():void {
  $B5: {
    %13:void = call %f3
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, RemoveMultipleFunctions) {
    auto* f1 = Func("f1");
    auto* f2 = Func("f2");
    auto* f3 = Func("f3");
    auto* f4 = Func("f4");
    auto* f5 = Func("f5");
    auto* f6 = Func("f6");
    auto* f7 = Func("f7");

    EntryPoint("foo", {f1, f5});
    EntryPoint("bar", {f2, f3, f4, f6, f7});

    auto* src = R"(
%f1 = func():void {
  $B1: {
    ret
  }
}
%f2 = func():void {
  $B2: {
    ret
  }
}
%f3 = func():void {
  $B3: {
    ret
  }
}
%f4 = func():void {
  $B4: {
    ret
  }
}
%f5 = func():void {
  $B5: {
    ret
  }
}
%f6 = func():void {
  $B6: {
    ret
  }
}
%f7 = func():void {
  $B7: {
    ret
  }
}
%foo = @fragment func():void {
  $B8: {
    %9:void = call %f1
    %10:void = call %f5
    ret
  }
}
%bar = @fragment func():void {
  $B9: {
    %12:void = call %f2
    %13:void = call %f3
    %14:void = call %f4
    %15:void = call %f6
    %16:void = call %f7
    ret
  }
}
)";

    auto* expect = R"(
%f1 = func():void {
  $B1: {
    ret
  }
}
%f5 = func():void {
  $B2: {
    ret
  }
}
%foo = @fragment func():void {
  $B3: {
    %4:void = call %f1
    %5:void = call %f5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, RemoveMultipleVariables) {
    auto* v1 = Var("v1");
    auto* v2 = Var("v2");
    auto* v3 = Var("v3");
    auto* v4 = Var("v4");
    auto* v5 = Var("v5");
    auto* v6 = Var("v6");
    auto* v7 = Var("v7");

    EntryPoint("foo", {v1, v5});
    EntryPoint("bar", {v1, v2, v3, v4, v5, v6, v7});

    auto* src = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var undef
  %v2:ptr<private, i32, read_write> = var undef
  %v3:ptr<private, i32, read_write> = var undef
  %v4:ptr<private, i32, read_write> = var undef
  %v5:ptr<private, i32, read_write> = var undef
  %v6:ptr<private, i32, read_write> = var undef
  %v7:ptr<private, i32, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %9:ptr<private, i32, read_write> = let %v1
    %10:ptr<private, i32, read_write> = let %v5
    ret
  }
}
%bar = @fragment func():void {
  $B3: {
    %12:ptr<private, i32, read_write> = let %v1
    %13:ptr<private, i32, read_write> = let %v2
    %14:ptr<private, i32, read_write> = let %v3
    %15:ptr<private, i32, read_write> = let %v4
    %16:ptr<private, i32, read_write> = let %v5
    %17:ptr<private, i32, read_write> = let %v6
    %18:ptr<private, i32, read_write> = let %v7
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var undef
  %v5:ptr<private, i32, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %4:ptr<private, i32, read_write> = let %v1
    %5:ptr<private, i32, read_write> = let %v5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, RemoveMultipleOverrides) {
    auto* o1 = Override("o1", 1);
    auto* o2 = Override("o2", 2);
    auto* o3 = Override("o3", 3);
    auto* o4 = Override("o4", 4);
    auto* o5 = Override("o5", 5);
    auto* o6 = Override("o6", 6);
    auto* o7 = Override("o7", 7);

    EntryPoint("foo", {o1, o5});
    EntryPoint("bar", {o1, o2, o3, o4, o5, o6, o7});

    auto* src = R"(
$B1: {  # root
  %o1:i32 = override undef @id(1)
  %o2:i32 = override undef @id(2)
  %o3:i32 = override undef @id(3)
  %o4:i32 = override undef @id(4)
  %o5:i32 = override undef @id(5)
  %o6:i32 = override undef @id(6)
  %o7:i32 = override undef @id(7)
}

%foo = @fragment func():void {
  $B2: {
    %9:i32 = let %o1
    %10:i32 = let %o5
    ret
  }
}
%bar = @fragment func():void {
  $B3: {
    %12:i32 = let %o1
    %13:i32 = let %o2
    %14:i32 = let %o3
    %15:i32 = let %o4
    %16:i32 = let %o5
    %17:i32 = let %o6
    %18:i32 = let %o7
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %o1:i32 = override undef @id(1)
  %o5:i32 = override undef @id(5)
}

%foo = @fragment func():void {
  $B2: {
    %4:i32 = let %o1
    %5:i32 = let %o5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, OverridesInWorkgroupSize) {
    auto* o1 = Override("o1", 1);
    auto* o2 = Override("o2", 2);
    auto* o3 = Override("o3", 3);

    auto* f1 = b.ComputeFunction("foo", o1, o2, o1);
    b.Append(f1->Block(), [&] { b.Return(f1); });

    auto* f2 = b.ComputeFunction("bar", o3, o3, o3);
    b.Append(f2->Block(), [&] { b.Return(f2); });

    auto* src = R"(
$B1: {  # root
  %o1:i32 = override undef @id(1)
  %o2:i32 = override undef @id(2)
  %o3:i32 = override undef @id(3)
}

%foo = @compute @workgroup_size(%o1, %o2, %o1) func():void {
  $B2: {
    ret
  }
}
%bar = @compute @workgroup_size(%o3, %o3, %o3) func():void {
  $B3: {
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %o1:i32 = override undef @id(1)
  %o2:i32 = override undef @id(2)
}

%foo = @compute @workgroup_size(%o1, %o2, %o1) func():void {
  $B2: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, OverrideInArrayType) {
    auto* o1 = Override("o1", 1);
    auto* o2 = Override("o2", 2);

    core::ir::Value* v1 = nullptr;
    core::ir::Value* v2 = nullptr;
    b.Append(mod.root_block, [&] {
        auto* c1 = ty.Get<core::ir::type::ValueArrayCount>(o1);
        auto* a1 = ty.Get<core::type::Array>(ty.i32(), c1, 4u, 4u, 4u, 4u);

        auto* c2 = ty.Get<core::ir::type::ValueArrayCount>(o2);
        auto* a2 = ty.Get<core::type::Array>(ty.i32(), c2, 4u, 4u, 4u, 4u);

        v1 = b.Var("a", ty.ptr(workgroup, a1, read_write))->Result();
        v2 = b.Var("b", ty.ptr(workgroup, a2, read_write))->Result();
    });

    EntryPoint("foo", {v1});
    EntryPoint("bar", {v2});

    auto* src = R"(
$B1: {  # root
  %o1:i32 = override undef @id(1)
  %o2:i32 = override undef @id(2)
  %a:ptr<workgroup, array<i32, %o1>, read_write> = var undef
  %b:ptr<workgroup, array<i32, %o2>, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %6:ptr<workgroup, array<i32, %o1>, read_write> = let %a
    ret
  }
}
%bar = @fragment func():void {
  $B3: {
    %8:ptr<workgroup, array<i32, %o2>, read_write> = let %b
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %o1:i32 = override undef @id(1)
  %a:ptr<workgroup, array<i32, %o1>, read_write> = var undef
}

%foo = @fragment func():void {
  $B2: {
    %4:ptr<workgroup, array<i32, %o1>, read_write> = let %a
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, OverrideWithComplexIncludingOverride) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* x = b.Override("x", ty.u32());
        x->SetOverrideId({2});
        auto* add = b.Add(ty.u32(), x, 4_u);
        o = b.Override(Source{{1, 2}}, "a", ty.u32());
        o->SetOverrideId({1});
        o->SetInitializer(add->Result());
    });

    auto* func = b.Function("foo", ty.u32());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });
    EntryPoint("foo", {func});
    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 4u
  %a:u32 = override %2 @id(1)
}

%foo = func():u32 {
  $B2: {
    ret %a
  }
}
%foo_1 = @fragment func():void {  # %foo_1: 'foo'
  $B3: {
    %6:u32 = call %foo
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 4u
  %a:u32 = override %2 @id(1)
}

%foo = func():u32 {
  $B2: {
    ret %a
  }
}
%foo_1 = @fragment func():void {  # %foo_1: 'foo'
  $B3: {
    %6:u32 = call %foo
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, OverrideInitVar) {
    core::ir::Override* x = nullptr;
    core::ir::Value* v1 = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({2});
        auto* add = b.Add(ty.u32(), x, 3_u);
        auto* var_local =
            b.Var("a", core::AddressSpace::kPrivate, ty.u32(), core::Access::kReadWrite);
        var_local->SetInitializer(add->Result());
        v1 = var_local->Result();
    });

    EntryPoint("foo", {v1});
    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 3u
  %a:ptr<private, u32, read_write> = var %2
}

%foo = @fragment func():void {
  $B2: {
    %5:ptr<private, u32, read_write> = let %a
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 3u
  %a:ptr<private, u32, read_write> = var %2
}

%foo = @fragment func():void {
  $B2: {
    %5:ptr<private, u32, read_write> = let %a
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, OverrideInitVarIntermediateUnused) {
    core::ir::Override* x = nullptr;
    core::ir::Value* v1 = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({2});
        auto* add_a = b.Add(ty.u32(), x, 3_u);
        auto* add_b = b.Add(ty.u32(), x, 3_u);
        auto* var_local =
            b.Var("a", core::AddressSpace::kPrivate, ty.u32(), core::Access::kReadWrite);
        auto* var_local_b =
            b.Var("b", core::AddressSpace::kPrivate, ty.u32(), core::Access::kReadWrite);
        var_local_b->SetInitializer(add_b->Result());
        var_local->SetInitializer(add_a->Result());
        v1 = var_local->Result();
    });

    EntryPoint("foo", {v1});
    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 3u
  %3:u32 = add %x, 3u
  %a:ptr<private, u32, read_write> = var %2
  %b:ptr<private, u32, read_write> = var %3
}

%foo = @fragment func():void {
  $B2: {
    %7:ptr<private, u32, read_write> = let %a
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 3u
  %a:ptr<private, u32, read_write> = var %2
}

%foo = @fragment func():void {
  $B2: {
    %5:ptr<private, u32, read_write> = let %a
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, OverideInitVarUnused) {
    core::ir::Override* x = nullptr;
    core::ir::Value* v1 = nullptr;
    b.Append(mod.root_block, [&] {
        x = b.Override("x", ty.u32());
        x->SetOverrideId({2});
        auto* add = b.Add(ty.u32(), x, 3_u);
        auto* var_local =
            b.Var("a", core::AddressSpace::kPrivate, ty.u32(), core::Access::kReadWrite);
        var_local->SetInitializer(add->Result());
        v1 = var_local->Result();
    });

    EntryPoint("foo", {});
    auto* src = R"(
$B1: {  # root
  %x:u32 = override undef @id(2)
  %2:u32 = add %x, 3u
  %a:ptr<private, u32, read_write> = var %2
}

%foo = @fragment func():void {
  $B2: {
    ret
  }
}
)";

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, OverrideCondConstExprFailure) {
    core::ir::Override* o = nullptr;
    b.Append(mod.root_block, [&] {
        auto* cond = b.Override("cond", ty.bool_());
        cond->SetOverrideId({0});
        auto* one_f32 = b.Override("one_f32", 1_f);
        one_f32->SetOverrideId({2});
        auto* constexpr_if = b.ConstExprIf(cond);
        constexpr_if->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(constexpr_if->True(), [&] {
            auto* three = b.Divide(ty.f32(), one_f32, 0.0_f);
            auto* four = b.Equal(ty.bool_(), three, 0.0_f);
            b.ExitIf(constexpr_if, four);
        });
        b.Append(constexpr_if->False(), [&] { b.ExitIf(constexpr_if, false); });
        o = b.Override(Source{{1, 2}}, "osrc", ty.bool_());
        o->SetOverrideId({1});
        o->SetInitializer(constexpr_if->Result());
    });

    auto* func = b.Function("foo2", ty.bool_());
    b.Append(func->Block(), [&] { b.Return(func, o->Result()); });
    EntryPoint("foo", {func});

    auto* src = R"(
$B1: {  # root
  %cond:bool = override undef @id(0)
  %one_f32:f32 = override 1.0f @id(2)
  %3:bool = constexpr_if %cond [t: $B2, f: $B3] {  # constexpr_if_1
    $B2: {  # true
      %4:f32 = div %one_f32, 0.0f
      %5:bool = eq %4, 0.0f
      exit_if %5  # constexpr_if_1
    }
    $B3: {  # false
      exit_if false  # constexpr_if_1
    }
  }
  %osrc:bool = override %3 @id(1)
}

%foo2 = func():bool {
  $B4: {
    ret %osrc
  }
}
%foo = @fragment func():void {
  $B5: {
    %9:bool = call %foo2
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %cond:bool = override undef @id(0)
  %one_f32:f32 = override 1.0f @id(2)
  %3:bool = constexpr_if %cond [t: $B2, f: $B3] {  # constexpr_if_1
    $B2: {  # true
      %4:f32 = div %one_f32, 0.0f
      %5:bool = eq %4, 0.0f
      exit_if %5  # constexpr_if_1
    }
    $B3: {  # false
      exit_if false  # constexpr_if_1
    }
  }
  %osrc:bool = override %3 @id(1)
}

%foo2 = func():bool {
  $B4: {
    ret %osrc
  }
}
%foo = @fragment func():void {
  $B5: {
    %9:bool = call %foo2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    capabilities = core::ir::Capabilities{core::ir::Capability::kAllowOverrides};
    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
