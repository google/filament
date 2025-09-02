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

#include "src/tint/lang/spirv/reader/lower/decompose_strided_array.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/spirv/type/explicit_layout_array.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class SpirvReader_DecomposeStridedArrayTest : public core::ir::transform::TransformTest {
  protected:
    const spirv::type::ExplicitLayoutArray* Array(const core::type::Type* elem_ty,
                                                  uint32_t count,
                                                  uint32_t stride) {
        if (stride == 0) {
            stride = tint::RoundUp(elem_ty->Align(), elem_ty->Size());
        }
        return ty.Get<spirv::type::ExplicitLayoutArray>(
            elem_ty, ty.Get<core::type::ConstantArrayCount>(static_cast<uint32_t>(count)),
            elem_ty->Align(), stride * count, stride);
    }

    const spirv::type::ExplicitLayoutArray* RuntimeArray(const core::type::Type* elem_ty,
                                                         uint32_t stride) {
        if (stride == 0) {
            stride = tint::RoundUp(elem_ty->Align(), elem_ty->Size());
        }
        return ty.Get<spirv::type::ExplicitLayoutArray>(
            elem_ty, ty.Get<core::type::RuntimeArrayCount>(), elem_ty->Align(), stride, stride);
    }
};

TEST_F(SpirvReader_DecomposeStridedArrayTest, LoadElement) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<storage, u32, read_write>>(var, 4_u)));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u, 0u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, LoadArray) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(var));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.explicit_layout_array<u32, 8, stride=16> = load %var
    %value:spirv.explicit_layout_array<u32, 8, stride=16> = let %3
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:array<tint_padded_array_element, 8> = load %var
    %value:array<tint_padded_array_element, 8> = let %3
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, StoreElement) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Store(b.Access<ptr<storage, u32, read_write>>(var, 4_u), 42_u);
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u
    store %3, 42u
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u, 0u
    store %3, 42u
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, StoreArray) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* arr = b.Zero(array_type);
        b.Store(var, arr);
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    store %var, spirv.explicit_layout_array<u32, 8, stride=16>(0u)
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    store %var, array<tint_padded_array_element, 8>(tint_padded_array_element(0u))
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, LoadElement_ViaLet) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* let = b.Let("let", var);
        b.Let("value", b.Load(b.Access<ptr<storage, u32, read_write>>(let, 4_u)));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %let:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = let %var
    %4:ptr<storage, u32, read_write> = access %let, 4u
    %5:u32 = load %4
    %value:u32 = let %5
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %let:ptr<storage, array<tint_padded_array_element, 8>, read_write> = let %var
    %4:ptr<storage, u32, read_write> = access %let, 4u, 0u
    %5:u32 = load %4
    %value:u32 = let %5
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, LoadElement_ViaAccessLet) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* let = b.Let("let", b.Access<ptr<storage, u32, read_write>>(var, 4_u));
        b.Let("value", b.Load(let));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u
    %let:ptr<storage, u32, read_write> = let %3
    %5:u32 = load %let
    %value:u32 = let %5
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u, 0u
    %let:ptr<storage, u32, read_write> = let %3
    %5:u32 = load %let
    %value:u32 = let %5
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, LoadElement_ViaFuncParam) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* callee = b.Function("callee", ty.u32());
    auto* param = b.FunctionParam("param", ty.ptr(storage, array_type, read_write));
    callee->SetParams({param});
    b.Append(callee->Block(), [&] {
        auto* access = b.Access<ptr<storage, u32, read_write>>(param, 4_u);
        b.Return(callee, b.Load(access));
    });

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Call(callee, var));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = var undef @binding_point(0, 0)
}

%callee = func(%param:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write>):u32 {
  $B2: {
    %4:ptr<storage, u32, read_write> = access %param, 4u
    %5:u32 = load %4
    ret %5
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:u32 = call %callee, %var
    %value:u32 = let %7
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element, 8>, read_write> = var undef @binding_point(0, 0)
}

%callee = func(%param:ptr<storage, array<tint_padded_array_element, 8>, read_write>):u32 {
  $B2: {
    %4:ptr<storage, u32, read_write> = access %param, 4u, 0u
    %5:u32 = load %4
    ret %5
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:u32 = call %callee, %var
    %value:u32 = let %7
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, LoadArray_ViaFuncReturn) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* callee = b.Function("callee", array_type);
    b.Append(callee->Block(), [&] {  //
        b.Return(callee, b.Load(var));
    });

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Call(callee));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = var undef @binding_point(0, 0)
}

%callee = func():spirv.explicit_layout_array<u32, 8, stride=16> {
  $B2: {
    %3:spirv.explicit_layout_array<u32, 8, stride=16> = load %var
    ret %3
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %5:spirv.explicit_layout_array<u32, 8, stride=16> = call %callee
    %value:spirv.explicit_layout_array<u32, 8, stride=16> = let %5
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element, 8>, read_write> = var undef @binding_point(0, 0)
}

%callee = func():array<tint_padded_array_element, 8> {
  $B2: {
    %3:array<tint_padded_array_element, 8> = load %var
    ret %3
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %5:array<tint_padded_array_element, 8> = call %callee
    %value:array<tint_padded_array_element, 8> = let %5
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, NestedArray_LoadInnerElement) {
    auto* inner_array_type = Array(ty.u32(), 8, 16);
    auto* outer_array_type = Array(inner_array_type, 8, 1024);
    auto* var = b.Var("var", ty.ptr(storage, outer_array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<storage, u32, read_write>>(var, 2_u, 3_u)));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 8, stride=16>, 8, stride=1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 2u, 3u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

tint_padded_array_element_1 = struct @align(4) {
  tint_element_1:array<tint_padded_array_element, 8> @offset(0) @size(1024)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element_1, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 2u, 0u, 3u, 0u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, NestedArray_LoadInnerArray) {
    auto* inner_array_type = Array(ty.u32(), 8, 16);
    auto* outer_array_type = Array(inner_array_type, 8, 1024);
    auto* var = b.Var("var", ty.ptr(storage, outer_array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, inner_array_type, read_write), var, 2_u);
        b.Let("value", b.Load(access));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 8, stride=16>, 8, stride=1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = access %var, 2u
    %4:spirv.explicit_layout_array<u32, 8, stride=16> = load %3
    %value:spirv.explicit_layout_array<u32, 8, stride=16> = let %4
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

tint_padded_array_element_1 = struct @align(4) {
  tint_element_1:array<tint_padded_array_element, 8> @offset(0) @size(1024)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element_1, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, array<tint_padded_array_element, 8>, read_write> = access %var, 2u, 0u
    %4:array<tint_padded_array_element, 8> = load %3
    %value:array<tint_padded_array_element, 8> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, NestedArray_LoadOuterArray) {
    auto* inner_array_type = Array(ty.u32(), 8, 16);
    auto* outer_array_type = Array(inner_array_type, 8, 1024);
    auto* var = b.Var("var", ty.ptr(storage, outer_array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(var));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 8, stride=16>, 8, stride=1024>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 8, stride=16>, 8, stride=1024> = load %var
    %value:spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 8, stride=16>, 8, stride=1024> = let %3
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

tint_padded_array_element_1 = struct @align(4) {
  tint_element_1:array<tint_padded_array_element, 8> @offset(0) @size(1024)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element_1, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:array<tint_padded_array_element_1, 8> = load %var
    %value:array<tint_padded_array_element_1, 8> = let %3
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, NestedArray_LoadInnerElement_ViaFuncParam) {
    auto* inner_array_type = Array(ty.u32(), 8, 16);
    auto* outer_array_type = Array(inner_array_type, 8, 1024);
    auto* var = b.Var("var", ty.ptr(storage, outer_array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* callee = b.Function("callee", ty.u32());
    auto* param = b.FunctionParam("param", ty.ptr(storage, inner_array_type, read_write));
    callee->SetParams({param});
    b.Append(callee->Block(), [&] {
        auto* access = b.Access<ptr<storage, u32, read_write>>(param, 4_u);
        b.Return(callee, b.Load(access));
    });

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* access = b.Access(ty.ptr(storage, inner_array_type, read_write), var, 2_u);
        b.Let("value", b.Call(callee, access));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<spirv.explicit_layout_array<u32, 8, stride=16>, 8, stride=1024>, read_write> = var undef @binding_point(0, 0)
}

%callee = func(%param:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write>):u32 {
  $B2: {
    %4:ptr<storage, u32, read_write> = access %param, 4u
    %5:u32 = load %4
    ret %5
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = access %var, 2u
    %8:u32 = call %callee, %7
    %value:u32 = let %8
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

tint_padded_array_element_1 = struct @align(4) {
  tint_element_1:array<tint_padded_array_element, 8> @offset(0) @size(1024)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element_1, 8>, read_write> = var undef @binding_point(0, 0)
}

%callee = func(%param:ptr<storage, array<tint_padded_array_element, 8>, read_write>):u32 {
  $B2: {
    %4:ptr<storage, u32, read_write> = access %param, 4u, 0u
    %5:u32 = load %4
    ret %5
  }
}
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B3: {
    %7:ptr<storage, array<tint_padded_array_element, 8>, read_write> = access %var, 2u, 0u
    %8:u32 = call %callee, %7
    %value:u32 = let %8
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, NestedArray_OuterIsNotExplicit) {
    auto* inner_array_type = Array(ty.u32(), 8, 16);
    auto* outer_array_type = ty.array(inner_array_type, 8);
    auto* var = b.Var("var", ty.ptr(storage, outer_array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<storage, u32, read_write>>(var, 2_u, 3_u)));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, array<spirv.explicit_layout_array<u32, 8, stride=16>, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 2u, 3u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<array<tint_padded_array_element, 8>, 8>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 2u, 3u, 0u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, Struct_LoadElement) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* struct_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), array_type},
                                                             });
    auto* var = b.Var("var", ty.ptr(storage, struct_ty, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<storage, u32, read_write>>(var, 1_u, 2_u)));
        b.Return(f);
    });

    auto* before = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<u32, 8, stride=16> @offset(4)
}

$B1: {  # root
  %var:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 1u, 2u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";
    auto* after = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<u32, 8, stride=16> @offset(4)
}

tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

MyStruct_1 = struct @align(4) {
  a:u32 @offset(0)
  b:array<tint_padded_array_element, 8> @offset(4)
}

$B1: {  # root
  %var:ptr<storage, MyStruct_1, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 1u, 2u, 0u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, Struct_LoadWhole) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* struct_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), array_type},
                                                             });
    auto* var = b.Var("var", ty.ptr(storage, struct_ty, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(var));
        b.Return(f);
    });

    auto* before = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<u32, 8, stride=16> @offset(4)
}

$B1: {  # root
  %var:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:MyStruct = load %var
    %value:MyStruct = let %3
    ret
  }
}
)";
    auto* after = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<u32, 8, stride=16> @offset(4)
}

tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

MyStruct_1 = struct @align(4) {
  a:u32 @offset(0)
  b:array<tint_padded_array_element, 8> @offset(4)
}

$B1: {  # root
  %var:ptr<storage, MyStruct_1, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:MyStruct_1 = load %var
    %value:MyStruct_1 = let %3
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, Struct_StoreWhole) {
    auto* array_type = Array(ty.u32(), 8, 16);
    auto* struct_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), array_type},
                                                             });
    auto* var = b.Var("var", ty.ptr(storage, struct_ty, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Store(var, b.Zero(struct_ty));
        b.Return(f);
    });

    auto* before = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<u32, 8, stride=16> @offset(4)
}

$B1: {  # root
  %var:ptr<storage, MyStruct, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    store %var, MyStruct(0u, spirv.explicit_layout_array<u32, 8, stride=16>(0u))
    ret
  }
}
)";
    auto* after = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<u32, 8, stride=16> @offset(4)
}

tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

MyStruct_1 = struct @align(4) {
  a:u32 @offset(0)
  b:array<tint_padded_array_element, 8> @offset(4)
}

$B1: {  # root
  %var:ptr<storage, MyStruct_1, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    store %var, MyStruct_1(0u, array<tint_padded_array_element, 8>(tint_padded_array_element(0u)))
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, RuntimeArray) {
    auto* array_type = RuntimeArray(ty.u32(), 16);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* access = b.Access<ptr<storage, u32, read_write>>(var, 4_u);
        b.Let("value", b.Load(access));
        b.Store(access, b.Add<u32>(b.Load(access), 1_u));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, stride=16>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u
    %4:u32 = load %3
    %value:u32 = let %4
    %6:u32 = load %3
    %7:u32 = add %6, 1u
    store %3, %7
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

$B1: {  # root
  %var:ptr<storage, array<tint_padded_array_element>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u, 0u
    %4:u32 = load %3
    %value:u32 = let %4
    %6:u32 = load %3
    %7:u32 = add %6, 1u
    store %3, %7
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, MultipleVariables_DifferentElementTypes) {
    auto* array_type_a = Array(ty.u32(), 8, 16);
    auto* var_a = b.Var("a", ty.ptr(storage, array_type_a, read_write));
    var_a->SetBindingPoint(0, 0);
    mod.root_block->Append(var_a);

    auto* array_type_b = Array(ty.i32(), 4, 8);
    auto* var_b = b.Var("b", ty.ptr(storage, array_type_b, read_write));
    var_b->SetBindingPoint(0, 1);
    mod.root_block->Append(var_b);

    auto* array_type_c = Array(ty.f32(), 2, 32);
    auto* var_c = b.Var("c", ty.ptr(storage, array_type_c, read_write));
    var_c->SetBindingPoint(0, 2);
    mod.root_block->Append(var_c);

    auto* array_type_d = Array(ty.vec4<f32>(), 1, 16);
    auto* var_d = b.Var("d", ty.ptr(storage, array_type_d, read_write));
    var_d->SetBindingPoint(0, 3);
    mod.root_block->Append(var_d);

    auto* array_type_e = RuntimeArray(ty.vec2<i32>(), 64);
    auto* var_e = b.Var("e", ty.ptr(storage, array_type_e, read_write));
    var_e->SetBindingPoint(0, 4);
    mod.root_block->Append(var_e);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("let_a", b.Load(b.Access<ptr<storage, u32, read_write>>(var_a, 1_u)));
        b.Let("let_b", b.Load(b.Access<ptr<storage, i32, read_write>>(var_b, 2_u)));
        b.Let("let_c", b.Load(b.Access<ptr<storage, f32, read_write>>(var_c, 0_u)));
        b.Let("let_d", b.Load(b.Access<ptr<storage, vec4<f32>, read_write>>(var_d, 0_u)));
        b.Let("let_e", b.Load(b.Access<ptr<storage, vec2<i32>, read_write>>(var_e, 3_u)));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %a:ptr<storage, spirv.explicit_layout_array<u32, 8, stride=16>, read_write> = var undef @binding_point(0, 0)
  %b:ptr<storage, spirv.explicit_layout_array<i32, 4, stride=8>, read_write> = var undef @binding_point(0, 1)
  %c:ptr<storage, spirv.explicit_layout_array<f32, 2, stride=32>, read_write> = var undef @binding_point(0, 2)
  %d:ptr<storage, spirv.explicit_layout_array<vec4<f32>, 1, stride=16>, read_write> = var undef @binding_point(0, 3)
  %e:ptr<storage, spirv.explicit_layout_array<vec2<i32>, stride=64>, read_write> = var undef @binding_point(0, 4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %7:ptr<storage, u32, read_write> = access %a, 1u
    %8:u32 = load %7
    %let_a:u32 = let %8
    %10:ptr<storage, i32, read_write> = access %b, 2u
    %11:i32 = load %10
    %let_b:i32 = let %11
    %13:ptr<storage, f32, read_write> = access %c, 0u
    %14:f32 = load %13
    %let_c:f32 = let %14
    %16:ptr<storage, vec4<f32>, read_write> = access %d, 0u
    %17:vec4<f32> = load %16
    %let_d:vec4<f32> = let %17
    %19:ptr<storage, vec2<i32>, read_write> = access %e, 3u
    %20:vec2<i32> = load %19
    %let_e:vec2<i32> = let %20
    ret
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

tint_padded_array_element_1 = struct @align(4) {
  tint_element_1:i32 @offset(0) @size(8)
}

tint_padded_array_element_2 = struct @align(4) {
  tint_element_2:f32 @offset(0) @size(32)
}

tint_padded_array_element_3 = struct @align(8) {
  tint_element_3:vec2<i32> @offset(0) @size(64)
}

$B1: {  # root
  %a:ptr<storage, array<tint_padded_array_element, 8>, read_write> = var undef @binding_point(0, 0)
  %b:ptr<storage, array<tint_padded_array_element_1, 4>, read_write> = var undef @binding_point(0, 1)
  %c:ptr<storage, array<tint_padded_array_element_2, 2>, read_write> = var undef @binding_point(0, 2)
  %d:ptr<storage, array<vec4<f32>, 1>, read_write> = var undef @binding_point(0, 3)
  %e:ptr<storage, array<tint_padded_array_element_3>, read_write> = var undef @binding_point(0, 4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %7:ptr<storage, u32, read_write> = access %a, 1u, 0u
    %8:u32 = load %7
    %let_a:u32 = let %8
    %10:ptr<storage, i32, read_write> = access %b, 2u, 0u
    %11:i32 = load %10
    %let_b:i32 = let %11
    %13:ptr<storage, f32, read_write> = access %c, 0u, 0u
    %14:f32 = load %13
    %let_c:f32 = let %14
    %16:ptr<storage, vec4<f32>, read_write> = access %d, 0u
    %17:vec4<f32> = load %16
    %let_d:vec4<f32> = let %17
    %19:ptr<storage, vec2<i32>, read_write> = access %e, 3u, 0u
    %20:vec2<i32> = load %19
    %let_e:vec2<i32> = let %20
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

// Test that a natural stride is not converted to a struct
TEST_F(SpirvReader_DecomposeStridedArrayTest, NaturalStride) {
    auto* array_type = RuntimeArray(ty.u32(), 4);
    auto* var = b.Var("var", ty.ptr(storage, array_type, read_write));
    var->SetBindingPoint(0, 0);
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* access = b.Access<ptr<storage, u32, read_write>>(var, 4_u);
        b.Let("value", b.Load(access));
        b.Store(access, b.Add<u32>(b.Load(access), 1_u));
        b.Return(f);
    });

    auto* before = R"(
$B1: {  # root
  %var:ptr<storage, spirv.explicit_layout_array<u32, stride=4>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u
    %4:u32 = load %3
    %value:u32 = let %4
    %6:u32 = load %3
    %7:u32 = add %6, 1u
    store %3, %7
    ret
  }
}
)";
    auto* after = R"(
$B1: {  # root
  %var:ptr<storage, array<u32>, read_write> = var undef @binding_point(0, 0)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %var, 4u
    %4:u32 = load %3
    %value:u32 = let %4
    %6:u32 = load %3
    %7:u32 = add %6, 1u
    store %3, %7
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, PreserveIOAttributes) {
    auto* array_type = Array(ty.f32(), 4, 4);
    auto* struct_ty = ty.Struct(
        mod.symbols.New("MyStruct"),
        {
            {mod.symbols.New("a"), ty.vec4<f32>(), {.builtin = core::BuiltinValue::kPosition}},
            {mod.symbols.New("b"), array_type, {.builtin = core::BuiltinValue::kClipDistances}},
        });
    auto* var = b.Var("var", ty.ptr(private_, struct_ty));
    mod.root_block->Append(var);

    auto* f = b.Function("foo", struct_ty, core::ir::Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] {
        b.Store(b.Access<ptr<private_, f32>>(var, 1_u, 2_u), 3_f);
        b.Return(f, b.Load(var));
    });

    auto* before = R"(
MyStruct = struct @align(16) {
  a:vec4<f32> @offset(0), @builtin(position)
  b:spirv.explicit_layout_array<f32, 4, stride=4> @offset(16), @builtin(clip_distances)
}

$B1: {  # root
  %var:ptr<private, MyStruct, read_write> = var undef
}

%foo = @vertex func():MyStruct {
  $B2: {
    %3:ptr<private, f32, read_write> = access %var, 1u, 2u
    store %3, 3.0f
    %4:MyStruct = load %var
    ret %4
  }
}
)";
    auto* after = R"(
MyStruct = struct @align(16) {
  a:vec4<f32> @offset(0), @builtin(position)
  b:spirv.explicit_layout_array<f32, 4, stride=4> @offset(16), @builtin(clip_distances)
}

MyStruct_1 = struct @align(16) {
  a:vec4<f32> @offset(0), @builtin(position)
  b:array<f32, 4> @offset(16), @builtin(clip_distances)
}

$B1: {  # root
  %var:ptr<private, MyStruct_1, read_write> = var undef
}

%foo = @vertex func():MyStruct_1 {
  $B2: {
    %3:ptr<private, f32, read_write> = access %var, 1u, 2u
    store %3, 3.0f
    %4:MyStruct_1 = load %var
    ret %4
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, ConstructStridedArray) {
    auto* array_type = Array(ty.u32(), 4, 16);

    auto* f = b.Function("foo", ty.u32());
    auto* e1 = b.FunctionParam("e1", ty.u32());
    auto* e2 = b.FunctionParam("e2", ty.u32());
    auto* e3 = b.FunctionParam("e3", ty.u32());
    auto* e4 = b.FunctionParam("e4", ty.u32());
    f->SetParams({e1, e2, e3, e4});
    b.Append(f->Block(), [&] {
        auto* arr = b.Let("arr", b.Construct(array_type, e1, e2, e3, e4));
        auto* el = b.Let("el", b.Access<u32>(arr, 2_u));
        b.Return(f, el);
    });

    auto* before = R"(
%foo = func(%e1:u32, %e2:u32, %e3:u32, %e4:u32):u32 {
  $B1: {
    %6:spirv.explicit_layout_array<u32, 4, stride=16> = construct %e1, %e2, %e3, %e4
    %arr:spirv.explicit_layout_array<u32, 4, stride=16> = let %6
    %8:u32 = access %arr, 2u
    %el:u32 = let %8
    ret %el
  }
}
)";
    auto* after = R"(
tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

%foo = func(%e1:u32, %e2:u32, %e3:u32, %e4:u32):u32 {
  $B1: {
    %6:tint_padded_array_element = construct %e1
    %7:tint_padded_array_element = construct %e2
    %8:tint_padded_array_element = construct %e3
    %9:tint_padded_array_element = construct %e4
    %10:array<tint_padded_array_element, 4> = construct %6, %7, %8, %9
    %arr:array<tint_padded_array_element, 4> = let %10
    %12:u32 = access %arr, 2u, 0u
    %el:u32 = let %12
    ret %el
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedArrayTest, ConstructStructOfStridedArray) {
    auto* array_type = Array(ty.u32(), 4, 16);
    auto* struct_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                                 {mod.symbols.New("a"), ty.u32()},
                                                                 {mod.symbols.New("b"), array_type},
                                                             });

    auto* f = b.Function("foo", ty.u32());
    auto* e1 = b.FunctionParam("e1", ty.u32());
    auto* e2 = b.FunctionParam("e2", ty.u32());
    auto* e3 = b.FunctionParam("e3", ty.u32());
    auto* e4 = b.FunctionParam("e4", ty.u32());
    f->SetParams({e1, e2, e3, e4});
    b.Append(f->Block(), [&] {
        auto* arr =
            b.Let("arr", b.Construct(struct_ty, 42_u, b.Construct(array_type, e1, e2, e3, e4)));
        auto* el = b.Let("el", b.Access<u32>(arr, 1_u, 2_u));
        b.Return(f, el);
    });

    auto* before = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<u32, 4, stride=16> @offset(4)
}

%foo = func(%e1:u32, %e2:u32, %e3:u32, %e4:u32):u32 {
  $B1: {
    %6:spirv.explicit_layout_array<u32, 4, stride=16> = construct %e1, %e2, %e3, %e4
    %7:MyStruct = construct 42u, %6
    %arr:MyStruct = let %7
    %9:u32 = access %arr, 1u, 2u
    %el:u32 = let %9
    ret %el
  }
}
)";
    auto* after = R"(
MyStruct = struct @align(4) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<u32, 4, stride=16> @offset(4)
}

tint_padded_array_element = struct @align(4) {
  tint_element:u32 @offset(0) @size(16)
}

MyStruct_1 = struct @align(4) {
  a:u32 @offset(0)
  b:array<tint_padded_array_element, 4> @offset(4)
}

%foo = func(%e1:u32, %e2:u32, %e3:u32, %e4:u32):u32 {
  $B1: {
    %6:tint_padded_array_element = construct %e1
    %7:tint_padded_array_element = construct %e2
    %8:tint_padded_array_element = construct %e3
    %9:tint_padded_array_element = construct %e4
    %10:array<tint_padded_array_element, 4> = construct %6, %7, %8, %9
    %11:MyStruct_1 = construct 42u, %10
    %arr:MyStruct_1 = let %11
    %13:u32 = access %arr, 1u, 2u, 0u
    %el:u32 = let %13
    ret %el
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedArray);
    ASSERT_EQ(after, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower
