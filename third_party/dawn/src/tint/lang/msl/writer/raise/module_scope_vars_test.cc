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

#include "src/tint/lang/msl/writer/raise/module_scope_vars.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/sampled_texture.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer::raise {
namespace {

class MslWriter_ModuleScopeVarsTest : public core::ir::transform::TransformTest {
  public:
    void SetUp() override {
        capabilities.Add(core::ir::Capability::kAllowPointersAndHandlesInStructures,
                         core::ir::Capability::kAllowPrivateVarsInFunctions,
                         core::ir::Capability::kAllowAnyLetType);
    }
};

TEST_F(MslWriter_ModuleScopeVarsTest, NoModuleScopeVars) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* var = b.Var<function, i32>("v");
        b.Load(var);
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    %v:ptr<function, i32, read_write> = var
    %3:i32 = load %v
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, Private) {
    auto* var_a = b.Var("a", ty.ptr<private_, i32>());
    auto* var_b = b.Var("b", ty.ptr<private_, i32>());
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_a, b.Add<i32>(load_a, load_b));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<private, i32, read_write> = var
  %b:ptr<private, i32, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    %4:i32 = load %a
    %5:i32 = load %b
    %6:i32 = add %4, %5
    store %a, %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<private, i32, read_write> @offset(0)
  b:ptr<private, i32, read_write> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %a:ptr<private, i32, read_write> = var
    %b:ptr<private, i32, read_write> = var
    %4:tint_module_vars_struct = construct %a, %b
    %tint_module_vars:tint_module_vars_struct = let %4
    %6:ptr<private, i32, read_write> = access %tint_module_vars, 0u
    %7:i32 = load %6
    %8:ptr<private, i32, read_write> = access %tint_module_vars, 1u
    %9:i32 = load %8
    %10:i32 = add %7, %9
    %11:ptr<private, i32, read_write> = access %tint_module_vars, 0u
    store %11, %10
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, Private_WithInitializers) {
    auto* var_a = b.Var<private_>("a", 42_i);
    auto* var_b = b.Var<private_>("b", -1_i);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_a, b.Add<i32>(load_a, load_b));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<private, i32, read_write> = var, 42i
  %b:ptr<private, i32, read_write> = var, -1i
}

%foo = @fragment func():void {
  $B2: {
    %4:i32 = load %a
    %5:i32 = load %b
    %6:i32 = add %4, %5
    store %a, %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<private, i32, read_write> @offset(0)
  b:ptr<private, i32, read_write> @offset(0)
}

%foo = @fragment func():void {
  $B1: {
    %a:ptr<private, i32, read_write> = var, 42i
    %b:ptr<private, i32, read_write> = var, -1i
    %4:tint_module_vars_struct = construct %a, %b
    %tint_module_vars:tint_module_vars_struct = let %4
    %6:ptr<private, i32, read_write> = access %tint_module_vars, 0u
    %7:i32 = load %6
    %8:ptr<private, i32, read_write> = access %tint_module_vars, 1u
    %9:i32 = load %8
    %10:i32 = add %7, %9
    %11:ptr<private, i32, read_write> = access %tint_module_vars, 0u
    store %11, %10
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, Storage) {
    auto* var_a = b.Var("a", ty.ptr<storage, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(load_a, load_b));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<storage, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
}

%foo = @fragment func():void {
  $B2: {
    %4:i32 = load %a
    %5:i32 = load %b
    %6:i32 = add %4, %5
    store %b, %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<storage, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
}

%foo = @fragment func(%a:ptr<storage, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B1: {
    %4:tint_module_vars_struct = construct %a, %b
    %tint_module_vars:tint_module_vars_struct = let %4
    %6:ptr<storage, i32, read> = access %tint_module_vars, 0u
    %7:i32 = load %6
    %8:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    %9:i32 = load %8
    %10:i32 = add %7, %9
    %11:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    store %11, %10
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, Uniform) {
    auto* var_a = b.Var("a", ty.ptr<uniform, i32>());
    auto* var_b = b.Var("b", ty.ptr<uniform, i32>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Add<i32>(load_a, load_b);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<uniform, i32, read> = var @binding_point(1, 2)
  %b:ptr<uniform, i32, read> = var @binding_point(3, 4)
}

%foo = @fragment func():void {
  $B2: {
    %4:i32 = load %a
    %5:i32 = load %b
    %6:i32 = add %4, %5
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<uniform, i32, read> @offset(0)
  b:ptr<uniform, i32, read> @offset(0)
}

%foo = @fragment func(%a:ptr<uniform, i32, read> [@binding_point(1, 2)], %b:ptr<uniform, i32, read> [@binding_point(3, 4)]):void {
  $B1: {
    %4:tint_module_vars_struct = construct %a, %b
    %tint_module_vars:tint_module_vars_struct = let %4
    %6:ptr<uniform, i32, read> = access %tint_module_vars, 0u
    %7:i32 = load %6
    %8:ptr<uniform, i32, read> = access %tint_module_vars, 1u
    %9:i32 = load %8
    %10:i32 = add %7, %9
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, HandleTypes) {
    auto* t = ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    auto* var_t = b.Var("t", ty.ptr<handle>(t));
    auto* var_s = b.Var("s", ty.ptr<handle>(ty.sampler()));
    var_t->SetBindingPoint(1, 2);
    var_s->SetBindingPoint(3, 4);
    mod.root_block->Append(var_t);
    mod.root_block->Append(var_s);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_t = b.Load(var_t);
        auto* load_s = b.Load(var_s);
        b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, load_t, load_s, b.Splat<vec2<f32>>(0_f));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %t:ptr<handle, texture_2d<f32>, read> = var @binding_point(1, 2)
  %s:ptr<handle, sampler, read> = var @binding_point(3, 4)
}

%foo = @fragment func():void {
  $B2: {
    %4:texture_2d<f32> = load %t
    %5:sampler = load %s
    %6:vec4<f32> = textureSample %4, %5, vec2<f32>(0.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  t:texture_2d<f32> @offset(0)
  s:sampler @offset(0)
}

%foo = @fragment func(%t:texture_2d<f32> [@binding_point(1, 2)], %s:sampler [@binding_point(3, 4)]):void {
  $B1: {
    %4:tint_module_vars_struct = construct %t, %s
    %tint_module_vars:tint_module_vars_struct = let %4
    %6:texture_2d<f32> = access %tint_module_vars, 0u
    %7:sampler = access %tint_module_vars, 1u
    %8:vec4<f32> = textureSample %6, %7, vec2<f32>(0.0f)
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, Workgroup) {
    auto* var_a = b.Var("a", ty.ptr<workgroup, i32>());
    auto* var_b = b.Var("b", ty.ptr<workgroup, i32>());
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* func = b.ComputeFunction("foo");
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_a, b.Add<i32>(load_a, load_b));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<workgroup, i32, read_write> = var
  %b:ptr<workgroup, i32, read_write> = var
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = load %a
    %5:i32 = load %b
    %6:i32 = add %4, %5
    store %a, %6
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<workgroup, i32, read_write> @offset(0)
  b:ptr<workgroup, i32, read_write> @offset(0)
}

tint_symbol_2 = struct @align(4) {
  tint_symbol:i32 @offset(0)
  tint_symbol_1:i32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func(%2:ptr<workgroup, tint_symbol_2, read_write>):void {
  $B1: {
    %a:ptr<workgroup, i32, read_write> = access %2, 0u
    %b:ptr<workgroup, i32, read_write> = access %2, 1u
    %5:tint_module_vars_struct = construct %a, %b
    %tint_module_vars:tint_module_vars_struct = let %5
    %7:ptr<workgroup, i32, read_write> = access %tint_module_vars, 0u
    %8:i32 = load %7
    %9:ptr<workgroup, i32, read_write> = access %tint_module_vars, 1u
    %10:i32 = load %9
    %11:i32 = add %8, %10
    %12:ptr<workgroup, i32, read_write> = access %tint_module_vars, 0u
    store %12, %11
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, MultipleAddressSpaces) {
    auto* var_a = b.Var("a", ty.ptr<uniform, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    auto* var_c = b.Var("c", ty.ptr<private_, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);
    mod.root_block->Append(var_c);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        auto* load_c = b.Load(var_c);
        b.Store(var_b, b.Add<i32>(load_a, b.Add<i32>(load_b, load_c)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<uniform, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
  %c:ptr<private, i32, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    %5:i32 = load %a
    %6:i32 = load %b
    %7:i32 = load %c
    %8:i32 = add %6, %7
    %9:i32 = add %5, %8
    store %b, %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<uniform, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
  c:ptr<private, i32, read_write> @offset(0)
}

%foo = @fragment func(%a:ptr<uniform, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B1: {
    %c:ptr<private, i32, read_write> = var
    %5:tint_module_vars_struct = construct %a, %b, %c
    %tint_module_vars:tint_module_vars_struct = let %5
    %7:ptr<uniform, i32, read> = access %tint_module_vars, 0u
    %8:i32 = load %7
    %9:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    %10:i32 = load %9
    %11:ptr<private, i32, read_write> = access %tint_module_vars, 2u
    %12:i32 = load %11
    %13:i32 = add %10, %12
    %14:i32 = add %8, %13
    %15:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    store %15, %14
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, EntryPointHasExistingParameters) {
    auto* var_a = b.Var("a", ty.ptr<storage, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    auto* param = b.FunctionParam<i32>("param");
    param->SetLocation(1_u);
    param->SetInterpolation(core::Interpolation{core::InterpolationType::kFlat});
    func->SetParams({param});
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(load_a, b.Add<i32>(load_b, param)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<storage, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
}

%foo = @fragment func(%param:i32 [@location(1), @interpolate(flat)]):void {
  $B2: {
    %5:i32 = load %a
    %6:i32 = load %b
    %7:i32 = add %6, %param
    %8:i32 = add %5, %7
    store %b, %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<storage, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
}

%foo = @fragment func(%param:i32 [@location(1), @interpolate(flat)], %a:ptr<storage, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B1: {
    %5:tint_module_vars_struct = construct %a, %b
    %tint_module_vars:tint_module_vars_struct = let %5
    %7:ptr<storage, i32, read> = access %tint_module_vars, 0u
    %8:i32 = load %7
    %9:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    %10:i32 = load %9
    %11:i32 = add %10, %param
    %12:i32 = add %8, %11
    %13:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    store %13, %12
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, CallFunctionThatUsesVars_NoArgs) {
    auto* var_a = b.Var("a", ty.ptr<storage, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(load_a, load_b));
        b.Return(foo);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(foo);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<storage, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
}

%foo = func():void {
  $B2: {
    %4:i32 = load %a
    %5:i32 = load %b
    %6:i32 = add %4, %5
    store %b, %6
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %8:void = call %foo
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<storage, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
}

%foo = func(%tint_module_vars:tint_module_vars_struct):void {
  $B1: {
    %3:ptr<storage, i32, read> = access %tint_module_vars, 0u
    %4:i32 = load %3
    %5:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    %6:i32 = load %5
    %7:i32 = add %4, %6
    %8:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    store %8, %7
    ret
  }
}
%main = @fragment func(%a:ptr<storage, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B2: {
    %12:tint_module_vars_struct = construct %a, %b
    %tint_module_vars_1:tint_module_vars_struct = let %12  # %tint_module_vars_1: 'tint_module_vars'
    %14:void = call %foo, %tint_module_vars_1
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, CallFunctionThatUsesVars_WithExistingParameters) {
    auto* var_a = b.Var("a", ty.ptr<storage, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* foo = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam<i32>("param");
    foo->SetParams({param});
    b.Append(foo->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(load_a, b.Add<i32>(load_b, param)));
        b.Return(foo);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(foo, 42_i);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<storage, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
}

%foo = func(%param:i32):void {
  $B2: {
    %5:i32 = load %a
    %6:i32 = load %b
    %7:i32 = add %6, %param
    %8:i32 = add %5, %7
    store %b, %8
    ret
  }
}
%main = @fragment func():void {
  $B3: {
    %10:void = call %foo, 42i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<storage, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
}

%foo = func(%param:i32, %tint_module_vars:tint_module_vars_struct):void {
  $B1: {
    %4:ptr<storage, i32, read> = access %tint_module_vars, 0u
    %5:i32 = load %4
    %6:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    %7:i32 = load %6
    %8:i32 = add %7, %param
    %9:i32 = add %5, %8
    %10:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    store %10, %9
    ret
  }
}
%main = @fragment func(%a:ptr<storage, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B2: {
    %14:tint_module_vars_struct = construct %a, %b
    %tint_module_vars_1:tint_module_vars_struct = let %14  # %tint_module_vars_1: 'tint_module_vars'
    %16:void = call %foo, 42i, %tint_module_vars_1
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, CallFunctionThatUsesVars_HandleTypes) {
    auto* t = ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    auto* var_t = b.Var("t", ty.ptr<handle>(t));
    auto* var_s = b.Var("s", ty.ptr<handle>(ty.sampler()));
    var_t->SetBindingPoint(1, 2);
    var_s->SetBindingPoint(3, 4);
    mod.root_block->Append(var_t);
    mod.root_block->Append(var_s);

    auto* foo = b.Function("foo", ty.vec4<f32>());
    auto* param = b.FunctionParam<i32>("param");
    foo->SetParams({param});
    b.Append(foo->Block(), [&] {
        auto* load_t = b.Load(var_t);
        auto* load_s = b.Load(var_s);
        auto* result = b.Call<vec4<f32>>(core::BuiltinFn::kTextureSample, load_t, load_s,
                                         b.Splat<vec2<f32>>(0_f));
        b.Return(foo, result);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Call(foo, 42_i);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %t:ptr<handle, texture_2d<f32>, read> = var @binding_point(1, 2)
  %s:ptr<handle, sampler, read> = var @binding_point(3, 4)
}

%foo = func(%param:i32):vec4<f32> {
  $B2: {
    %5:texture_2d<f32> = load %t
    %6:sampler = load %s
    %7:vec4<f32> = textureSample %5, %6, vec2<f32>(0.0f)
    ret %7
  }
}
%main = @fragment func():void {
  $B3: {
    %9:vec4<f32> = call %foo, 42i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  t:texture_2d<f32> @offset(0)
  s:sampler @offset(0)
}

%foo = func(%param:i32, %tint_module_vars:tint_module_vars_struct):vec4<f32> {
  $B1: {
    %4:texture_2d<f32> = access %tint_module_vars, 0u
    %5:sampler = access %tint_module_vars, 1u
    %6:vec4<f32> = textureSample %4, %5, vec2<f32>(0.0f)
    ret %6
  }
}
%main = @fragment func(%t:texture_2d<f32> [@binding_point(1, 2)], %s:sampler [@binding_point(3, 4)]):void {
  $B2: {
    %10:tint_module_vars_struct = construct %t, %s
    %tint_module_vars_1:tint_module_vars_struct = let %10  # %tint_module_vars_1: 'tint_module_vars'
    %12:vec4<f32> = call %foo, 42i, %tint_module_vars_1
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, CallFunctionThatUsesVars_OutOfOrder) {
    auto* var_a = b.Var("a", ty.ptr<storage, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);

    auto* foo = b.Function("foo", ty.void_());
    b.Append(foo->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(load_a, load_b));
        b.Return(foo);
    });

    b.Append(func->Block(), [&] {
        b.Call(foo);
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<storage, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
}

%main = @fragment func():void {
  $B2: {
    %4:void = call %foo
    ret
  }
}
%foo = func():void {
  $B3: {
    %6:i32 = load %a
    %7:i32 = load %b
    %8:i32 = add %6, %7
    store %b, %8
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<storage, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
}

%main = @fragment func(%a:ptr<storage, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B1: {
    %4:tint_module_vars_struct = construct %a, %b
    %tint_module_vars:tint_module_vars_struct = let %4
    %6:void = call %foo, %tint_module_vars
    ret
  }
}
%foo = func(%tint_module_vars_1:tint_module_vars_struct):void {  # %tint_module_vars_1: 'tint_module_vars'
  $B2: {
    %9:ptr<storage, i32, read> = access %tint_module_vars_1, 0u
    %10:i32 = load %9
    %11:ptr<storage, i32, read_write> = access %tint_module_vars_1, 1u
    %12:i32 = load %11
    %13:i32 = add %10, %12
    %14:ptr<storage, i32, read_write> = access %tint_module_vars_1, 1u
    store %14, %13
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

// Test that we do not add the structure to functions that do not need it.
TEST_F(MslWriter_ModuleScopeVarsTest, CallFunctionThatDoesNotUseVars) {
    auto* var_a = b.Var("a", ty.ptr<storage, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, 42_i);
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(load_a, b.Add<i32>(load_b, b.Call(foo))));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<storage, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
}

%foo = func():i32 {
  $B2: {
    ret 42i
  }
}
%main = @fragment func():void {
  $B3: {
    %5:i32 = load %a
    %6:i32 = load %b
    %7:i32 = call %foo
    %8:i32 = add %6, %7
    %9:i32 = add %5, %8
    store %b, %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<storage, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
}

%foo = func():i32 {
  $B1: {
    ret 42i
  }
}
%main = @fragment func(%a:ptr<storage, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B2: {
    %5:tint_module_vars_struct = construct %a, %b
    %tint_module_vars:tint_module_vars_struct = let %5
    %7:ptr<storage, i32, read> = access %tint_module_vars, 0u
    %8:i32 = load %7
    %9:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    %10:i32 = load %9
    %11:i32 = call %foo
    %12:i32 = add %10, %11
    %13:i32 = add %8, %12
    %14:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    store %14, %13
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

// Test that we *do* add the structure to functions that only have transitive uses.
TEST_F(MslWriter_ModuleScopeVarsTest, CallFunctionWithOnlyTransitiveUses) {
    auto* var_a = b.Var("a", ty.ptr<storage, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* bar = b.Function("bar", ty.i32());
    b.Append(bar->Block(), [&] {  //
        b.Return(bar, b.Load(var_a));
    });

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, b.Call(bar));
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(load_a, b.Add<i32>(load_b, b.Call(foo))));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<storage, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
}

%bar = func():i32 {
  $B2: {
    %4:i32 = load %a
    ret %4
  }
}
%foo = func():i32 {
  $B3: {
    %6:i32 = call %bar
    ret %6
  }
}
%main = @fragment func():void {
  $B4: {
    %8:i32 = load %a
    %9:i32 = load %b
    %10:i32 = call %foo
    %11:i32 = add %9, %10
    %12:i32 = add %8, %11
    store %b, %12
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<storage, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
}

%bar = func(%tint_module_vars:tint_module_vars_struct):i32 {
  $B1: {
    %3:ptr<storage, i32, read> = access %tint_module_vars, 0u
    %4:i32 = load %3
    ret %4
  }
}
%foo = func(%tint_module_vars_1:tint_module_vars_struct):i32 {  # %tint_module_vars_1: 'tint_module_vars'
  $B2: {
    %7:i32 = call %bar, %tint_module_vars_1
    ret %7
  }
}
%main = @fragment func(%a:ptr<storage, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B3: {
    %11:tint_module_vars_struct = construct %a, %b
    %tint_module_vars_2:tint_module_vars_struct = let %11  # %tint_module_vars_2: 'tint_module_vars'
    %13:ptr<storage, i32, read> = access %tint_module_vars_2, 0u
    %14:i32 = load %13
    %15:ptr<storage, i32, read_write> = access %tint_module_vars_2, 1u
    %16:i32 = load %15
    %17:i32 = call %foo, %tint_module_vars_2
    %18:i32 = add %16, %17
    %19:i32 = add %14, %18
    %20:ptr<storage, i32, read_write> = access %tint_module_vars_2, 1u
    store %20, %19
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

// Test that we *do* add the structure to functions that only have transitive uses, where that
// function is declared first.
TEST_F(MslWriter_ModuleScopeVarsTest, CallFunctionWithOnlyTransitiveUses_OutOfOrder) {
    auto* var_a = b.Var("a", ty.ptr<storage, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);

    auto* foo = b.Function("foo", ty.i32());

    auto* bar = b.Function("bar", ty.i32());
    b.Append(bar->Block(), [&] {  //
        b.Return(bar, b.Load(var_a));
    });

    b.Append(foo->Block(), [&] {  //
        b.Return(foo, b.Call(bar));
    });

    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(load_a, b.Add<i32>(load_b, b.Call(foo))));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<storage, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
}

%foo = func():i32 {
  $B2: {
    %4:i32 = call %bar
    ret %4
  }
}
%bar = func():i32 {
  $B3: {
    %6:i32 = load %a
    ret %6
  }
}
%main = @fragment func():void {
  $B4: {
    %8:i32 = load %a
    %9:i32 = load %b
    %10:i32 = call %foo
    %11:i32 = add %9, %10
    %12:i32 = add %8, %11
    store %b, %12
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<storage, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
}

%foo = func(%tint_module_vars:tint_module_vars_struct):i32 {
  $B1: {
    %3:i32 = call %bar, %tint_module_vars
    ret %3
  }
}
%bar = func(%tint_module_vars_1:tint_module_vars_struct):i32 {  # %tint_module_vars_1: 'tint_module_vars'
  $B2: {
    %6:ptr<storage, i32, read> = access %tint_module_vars_1, 0u
    %7:i32 = load %6
    ret %7
  }
}
%main = @fragment func(%a:ptr<storage, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B3: {
    %11:tint_module_vars_struct = construct %a, %b
    %tint_module_vars_2:tint_module_vars_struct = let %11  # %tint_module_vars_2: 'tint_module_vars'
    %13:ptr<storage, i32, read> = access %tint_module_vars_2, 0u
    %14:i32 = load %13
    %15:ptr<storage, i32, read_write> = access %tint_module_vars_2, 1u
    %16:i32 = load %15
    %17:i32 = call %foo, %tint_module_vars_2
    %18:i32 = add %16, %17
    %19:i32 = add %14, %18
    %20:ptr<storage, i32, read_write> = access %tint_module_vars_2, 1u
    store %20, %19
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, MultipleEntryPoints) {
    auto* var_a = b.Var("a", ty.ptr<uniform, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    auto* var_c = b.Var("c", ty.ptr<private_, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);
    mod.root_block->Append(var_c);

    auto* main_a = b.Function("main_a", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(main_a->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        auto* load_c = b.Load(var_c);
        b.Store(var_b, b.Add<i32>(load_a, b.Add<i32>(load_b, load_c)));
        b.Return(main_a);
    });

    auto* main_b = b.Function("main_b", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(main_b->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        auto* load_c = b.Load(var_c);
        b.Store(var_b, b.Multiply<i32>(load_a, b.Multiply<i32>(load_b, load_c)));
        b.Return(main_b);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<uniform, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
  %c:ptr<private, i32, read_write> = var
}

%main_a = @fragment func():void {
  $B2: {
    %5:i32 = load %a
    %6:i32 = load %b
    %7:i32 = load %c
    %8:i32 = add %6, %7
    %9:i32 = add %5, %8
    store %b, %9
    ret
  }
}
%main_b = @fragment func():void {
  $B3: {
    %11:i32 = load %a
    %12:i32 = load %b
    %13:i32 = load %c
    %14:i32 = mul %12, %13
    %15:i32 = mul %11, %14
    store %b, %15
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<uniform, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
  c:ptr<private, i32, read_write> @offset(0)
}

%main_a = @fragment func(%a:ptr<uniform, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B1: {
    %c:ptr<private, i32, read_write> = var
    %5:tint_module_vars_struct = construct %a, %b, %c
    %tint_module_vars:tint_module_vars_struct = let %5
    %7:ptr<uniform, i32, read> = access %tint_module_vars, 0u
    %8:i32 = load %7
    %9:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    %10:i32 = load %9
    %11:ptr<private, i32, read_write> = access %tint_module_vars, 2u
    %12:i32 = load %11
    %13:i32 = add %10, %12
    %14:i32 = add %8, %13
    %15:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    store %15, %14
    ret
  }
}
%main_b = @fragment func(%a_1:ptr<uniform, i32, read> [@binding_point(1, 2)], %b_1:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {  # %a_1: 'a', %b_1: 'b'
  $B2: {
    %c_1:ptr<private, i32, read_write> = var  # %c_1: 'c'
    %20:tint_module_vars_struct = construct %a_1, %b_1, %c_1
    %tint_module_vars_1:tint_module_vars_struct = let %20  # %tint_module_vars_1: 'tint_module_vars'
    %22:ptr<uniform, i32, read> = access %tint_module_vars_1, 0u
    %23:i32 = load %22
    %24:ptr<storage, i32, read_write> = access %tint_module_vars_1, 1u
    %25:i32 = load %24
    %26:ptr<private, i32, read_write> = access %tint_module_vars_1, 2u
    %27:i32 = load %26
    %28:i32 = mul %25, %27
    %29:i32 = mul %23, %28
    %30:ptr<storage, i32, read_write> = access %tint_module_vars_1, 1u
    store %30, %29
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, MultipleEntryPoints_DifferentUsageSets) {
    auto* var_a = b.Var("a", ty.ptr<uniform, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    auto* var_c = b.Var("c", ty.ptr<private_, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);
    mod.root_block->Append(var_c);

    auto* main_a = b.Function("main_a", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(main_a->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(load_a, load_b));
        b.Return(main_a);
    });

    auto* main_b = b.Function("main_b", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(main_b->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_c = b.Load(var_c);
        b.Store(var_c, b.Multiply<i32>(load_a, load_c));
        b.Return(main_b);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<uniform, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
  %c:ptr<private, i32, read_write> = var
}

%main_a = @fragment func():void {
  $B2: {
    %5:i32 = load %a
    %6:i32 = load %b
    %7:i32 = add %5, %6
    store %b, %7
    ret
  }
}
%main_b = @fragment func():void {
  $B3: {
    %9:i32 = load %a
    %10:i32 = load %c
    %11:i32 = mul %9, %10
    store %c, %11
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<uniform, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
  c:ptr<private, i32, read_write> @offset(0)
}

%main_a = @fragment func(%a:ptr<uniform, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B1: {
    %4:tint_module_vars_struct = construct %a, %b, unused
    %tint_module_vars:tint_module_vars_struct = let %4
    %6:ptr<uniform, i32, read> = access %tint_module_vars, 0u
    %7:i32 = load %6
    %8:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    %9:i32 = load %8
    %10:i32 = add %7, %9
    %11:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    store %11, %10
    ret
  }
}
%main_b = @fragment func(%a_1:ptr<uniform, i32, read> [@binding_point(1, 2)]):void {  # %a_1: 'a'
  $B2: {
    %c:ptr<private, i32, read_write> = var
    %15:tint_module_vars_struct = construct %a_1, unused, %c
    %tint_module_vars_1:tint_module_vars_struct = let %15  # %tint_module_vars_1: 'tint_module_vars'
    %17:ptr<uniform, i32, read> = access %tint_module_vars_1, 0u
    %18:i32 = load %17
    %19:ptr<private, i32, read_write> = access %tint_module_vars_1, 2u
    %20:i32 = load %19
    %21:i32 = mul %18, %20
    %22:ptr<private, i32, read_write> = access %tint_module_vars_1, 2u
    store %22, %21
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, MultipleEntryPoints_DifferentUsageSets_CommonHelper) {
    auto* var_a = b.Var("a", ty.ptr<uniform, i32, core::Access::kRead>());
    auto* var_b = b.Var("b", ty.ptr<storage, i32, core::Access::kReadWrite>());
    auto* var_c = b.Var("c", ty.ptr<private_, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);
    mod.root_block->Append(var_c);

    auto* foo = b.Function("foo", ty.i32());
    b.Append(foo->Block(), [&] {  //
        b.Return(foo, b.Load(var_a));
    });

    auto* main_a = b.Function("main_a", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(main_a->Block(), [&] {
        auto* load_b = b.Load(var_b);
        b.Store(var_b, b.Add<i32>(b.Call(foo), load_b));
        b.Return(main_a);
    });

    auto* main_b = b.Function("main_b", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(main_b->Block(), [&] {
        auto* load_c = b.Load(var_c);
        b.Store(var_c, b.Multiply<i32>(b.Call(foo), load_c));
        b.Return(main_b);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<uniform, i32, read> = var @binding_point(1, 2)
  %b:ptr<storage, i32, read_write> = var @binding_point(3, 4)
  %c:ptr<private, i32, read_write> = var
}

%foo = func():i32 {
  $B2: {
    %5:i32 = load %a
    ret %5
  }
}
%main_a = @fragment func():void {
  $B3: {
    %7:i32 = load %b
    %8:i32 = call %foo
    %9:i32 = add %8, %7
    store %b, %9
    ret
  }
}
%main_b = @fragment func():void {
  $B4: {
    %11:i32 = load %c
    %12:i32 = call %foo
    %13:i32 = mul %12, %11
    store %c, %13
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  a:ptr<uniform, i32, read> @offset(0)
  b:ptr<storage, i32, read_write> @offset(0)
  c:ptr<private, i32, read_write> @offset(0)
}

%foo = func(%tint_module_vars:tint_module_vars_struct):i32 {
  $B1: {
    %3:ptr<uniform, i32, read> = access %tint_module_vars, 0u
    %4:i32 = load %3
    ret %4
  }
}
%main_a = @fragment func(%a:ptr<uniform, i32, read> [@binding_point(1, 2)], %b:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B2: {
    %8:tint_module_vars_struct = construct %a, %b, unused
    %tint_module_vars_1:tint_module_vars_struct = let %8  # %tint_module_vars_1: 'tint_module_vars'
    %10:ptr<storage, i32, read_write> = access %tint_module_vars_1, 1u
    %11:i32 = load %10
    %12:i32 = call %foo, %tint_module_vars_1
    %13:i32 = add %12, %11
    %14:ptr<storage, i32, read_write> = access %tint_module_vars_1, 1u
    store %14, %13
    ret
  }
}
%main_b = @fragment func(%a_1:ptr<uniform, i32, read> [@binding_point(1, 2)]):void {  # %a_1: 'a'
  $B3: {
    %c:ptr<private, i32, read_write> = var
    %18:tint_module_vars_struct = construct %a_1, unused, %c
    %tint_module_vars_2:tint_module_vars_struct = let %18  # %tint_module_vars_2: 'tint_module_vars'
    %20:ptr<private, i32, read_write> = access %tint_module_vars_2, 2u
    %21:i32 = load %20
    %22:i32 = call %foo, %tint_module_vars_2
    %23:i32 = mul %22, %21
    %24:ptr<private, i32, read_write> = access %tint_module_vars_2, 2u
    store %24, %23
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

TEST_F(MslWriter_ModuleScopeVarsTest, VarsWithNoNames) {
    auto* var_a = b.Var(ty.ptr<uniform, i32, core::Access::kRead>());
    auto* var_b = b.Var(ty.ptr<storage, i32, core::Access::kReadWrite>());
    auto* var_c = b.Var(ty.ptr<private_, i32, core::Access::kReadWrite>());
    var_a->SetBindingPoint(1, 2);
    var_b->SetBindingPoint(3, 4);
    mod.root_block->Append(var_a);
    mod.root_block->Append(var_b);
    mod.root_block->Append(var_c);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* load_a = b.Load(var_a);
        auto* load_b = b.Load(var_b);
        auto* load_c = b.Load(var_c);
        b.Store(var_b, b.Add<i32>(load_a, b.Add<i32>(load_b, load_c)));
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %1:ptr<uniform, i32, read> = var @binding_point(1, 2)
  %2:ptr<storage, i32, read_write> = var @binding_point(3, 4)
  %3:ptr<private, i32, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    %5:i32 = load %1
    %6:i32 = load %2
    %7:i32 = load %3
    %8:i32 = add %6, %7
    %9:i32 = add %5, %8
    store %2, %9
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
tint_module_vars_struct = struct @align(1) {
  tint_symbol:ptr<uniform, i32, read> @offset(0)
  tint_symbol_1:ptr<storage, i32, read_write> @offset(0)
  tint_symbol_2:ptr<private, i32, read_write> @offset(0)
}

%foo = @fragment func(%2:ptr<uniform, i32, read> [@binding_point(1, 2)], %3:ptr<storage, i32, read_write> [@binding_point(3, 4)]):void {
  $B1: {
    %4:ptr<private, i32, read_write> = var
    %5:tint_module_vars_struct = construct %2, %3, %4
    %tint_module_vars:tint_module_vars_struct = let %5
    %7:ptr<uniform, i32, read> = access %tint_module_vars, 0u
    %8:i32 = load %7
    %9:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    %10:i32 = load %9
    %11:ptr<private, i32, read_write> = access %tint_module_vars, 2u
    %12:i32 = load %11
    %13:i32 = add %10, %12
    %14:i32 = add %8, %13
    %15:ptr<storage, i32, read_write> = access %tint_module_vars, 1u
    store %15, %14
    ret
  }
}
)";

    Run(ModuleScopeVars);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::msl::writer::raise
