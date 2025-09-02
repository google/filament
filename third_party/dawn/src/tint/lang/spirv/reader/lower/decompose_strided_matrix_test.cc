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

#include "src/tint/lang/spirv/reader/lower/decompose_strided_matrix.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/spirv/type/explicit_layout_array.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class SpirvReader_DecomposeStridedMatrixTest : public core::ir::transform::TransformTest {
  protected:
    void SetUp() override { capabilities.Add(core::ir::Capability::kAllowNonCoreTypes); }

    /// Create a struct that has a matrix member sandwiched between two u32 members, optionally
    /// nested inside one or more arrays.
    const core::type::Struct* Struct(const core::type::Matrix* matrix_type,
                                     uint32_t matrix_stride,
                                     std::initializer_list<uint32_t> array_counts = {}) {
        uint32_t member_size = matrix_stride * matrix_type->Columns();
        const core::type::Type* member_type = matrix_type;
        for (uint32_t count : array_counts) {
            member_type = ty.array(member_type, count);
            member_size *= count;
        }
        auto* matrix_member =
            ty.Get<core::type::StructMember>(mod.symbols.New("b"), member_type, 1u, matrix_stride,
                                             matrix_stride, member_size, core::IOAttributes{});
        matrix_member->SetMatrixStride(matrix_stride);
        return ty.Struct(
            mod.symbols.New("S"),
            Vector{
                ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.u32(), 0u, 0u, 4u, 4u,
                                                 core::IOAttributes{}),
                matrix_member,
                ty.Get<core::type::StructMember>(mod.symbols.New("c"), ty.u32(), 2u,
                                                 matrix_member->Offset() + matrix_member->Size(),
                                                 4u, 4u, core::IOAttributes{}),
            });
    }
};

TEST_F(SpirvReader_DecomposeStridedMatrixTest, NaturalStride_CreateConstant) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 16);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Composite(struct_type, 42_u, b.Zero(matrix_type), 42_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16), @matrix_stride(16)
  c:u32 @offset(80)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %value:S = let S(42u, mat4x4<f32>(vec4<f32>(0.0f)), 42u)
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16), @matrix_stride(16)
  c:u32 @offset(80)
}

S_1 = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16)
  c:u32 @offset(80)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %value:S_1 = let S_1(42u, mat4x4<f32>(vec4<f32>(0.0f)), 42u)
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, NaturalStride_LoadMatrix) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 16);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<private_, mat4x4<f32>>>(var, 1_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16), @matrix_stride(16)
  c:u32 @offset(80)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 1u
    %4:mat4x4<f32> = load %3
    %value:mat4x4<f32> = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16), @matrix_stride(16)
  c:u32 @offset(80)
}

S_1 = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16)
  c:u32 @offset(80)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 1u
    %4:mat4x4<f32> = load %3
    %value:mat4x4<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, NaturalStride_ExtractMatrix) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 16);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* s = b.Let("s", b.Zero(struct_type));
        b.Let("value", b.Access<mat4x4<f32>>(s, 1_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16), @matrix_stride(16)
  c:u32 @offset(80)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %s:S = let S(0u, mat4x4<f32>(vec4<f32>(0.0f)), 0u)
    %3:mat4x4<f32> = access %s, 1u
    %value:mat4x4<f32> = let %3
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16), @matrix_stride(16)
  c:u32 @offset(80)
}

S_1 = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16)
  c:u32 @offset(80)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %s:S_1 = let S_1(0u, mat4x4<f32>(vec4<f32>(0.0f)), 0u)
    %3:mat4x4<f32> = access %s, 1u
    %value:mat4x4<f32> = let %3
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, NaturalStride_Construct) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 16);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* m = b.Let("m", b.Zero(matrix_type));
        b.Let("value", b.Construct(struct_type, 42_u, m, 42_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16), @matrix_stride(16)
  c:u32 @offset(80)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %m:mat4x4<f32> = let mat4x4<f32>(vec4<f32>(0.0f))
    %3:S = construct 42u, %m, 42u
    %value:S = let %3
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16), @matrix_stride(16)
  c:u32 @offset(80)
}

S_1 = struct @align(16) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(16)
  c:u32 @offset(80)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %m:mat4x4<f32> = let mat4x4<f32>(vec4<f32>(0.0f))
    %3:S_1 = construct 42u, %m, 42u
    %value:S_1 = let %3
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadMatrixElement) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* access = b.Access<ptr<private_, vec4<f32>>>(var, 1_u, 3_u);
        b.Let("value", b.LoadVectorElement(access, 2_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %var, 1u, 3u
    %4:f32 = load_vector_element %3, 2u
    %value:f32 = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %var, 1u, 3u
    %4:f32 = load_vector_element %3, 2u
    %value:f32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadMatrixColumn) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<private_, vec4<f32>>>(var, 1_u, 2_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %var, 1u, 2u
    %4:vec4<f32> = load %3
    %value:vec4<f32> = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %var, 1u, 2u
    %4:vec4<f32> = load %3
    %value:vec4<f32> = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadMatrix) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<private_, mat4x4<f32>>>(var, 1_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 1u
    %4:mat4x4<f32> = load %3
    %value:mat4x4<f32> = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %var, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = load %3
    %5:vec4<f32> = access %4, 0u
    %6:vec4<f32> = access %4, 1u
    %7:vec4<f32> = access %4, 2u
    %8:vec4<f32> = access %4, 3u
    %9:mat4x4<f32> = construct %5, %6, %7, %8
    %value:mat4x4<f32> = let %9
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadMatrix_ViaLet) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* let = b.Let("ptr", b.Access<ptr<private_, mat4x4<f32>>>(var, 1_u));
        b.Let("value", b.Load(let));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 1u
    %ptr:ptr<private, mat4x4<f32>, read_write> = let %3
    %5:mat4x4<f32> = load %ptr
    %value:mat4x4<f32> = let %5
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %var, 1u
    %ptr:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = let %3
    %5:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = load %ptr
    %6:vec4<f32> = access %5, 0u
    %7:vec4<f32> = access %5, 1u
    %8:vec4<f32> = access %5, 2u
    %9:vec4<f32> = access %5, 3u
    %10:mat4x4<f32> = construct %6, %7, %8, %9
    %value:mat4x4<f32> = let %10
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadStruct) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(var));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:S = load %var
    %value:S = let %3
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:S_1 = load %var
    %value:S_1 = let %3
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadStruct_ViaLet) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* let = b.Let("ptr", var);
        b.Let("value", b.Load(let));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %ptr:ptr<private, S, read_write> = let %var
    %4:S = load %ptr
    %value:S = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %ptr:ptr<private, S_1, read_write> = let %var
    %4:S_1 = load %ptr
    %value:S_1 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadStruct_ExtractMatrix) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* struct_value = b.Let("struct_value", b.Load(var));
        b.Let("matrix_value", b.Access(matrix_type, struct_value, 1_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:S = load %var
    %struct_value:S = let %3
    %5:mat4x4<f32> = access %struct_value, 1u
    %matrix_value:mat4x4<f32> = let %5
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:S_1 = load %var
    %struct_value:S_1 = let %3
    %5:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %struct_value, 1u
    %6:vec4<f32> = access %5, 0u
    %7:vec4<f32> = access %5, 1u
    %8:vec4<f32> = access %5, 2u
    %9:vec4<f32> = access %5, 3u
    %10:mat4x4<f32> = construct %6, %7, %8, %9
    %matrix_value:mat4x4<f32> = let %10
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, StoreMatrixElement) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* access = b.Access<ptr<private_, vec4<f32>>>(var, 1_u, 3_u);
        b.StoreVectorElement(access, 2_u, 42_f);
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %var, 1u, 3u
    store_vector_element %3, 2u, 42.0f
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %var, 1u, 3u
    store_vector_element %3, 2u, 42.0f
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, StoreMatrixColumn) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Store(b.Access<ptr<private_, vec4<f32>>>(var, 1_u, 2_u), b.Zero<vec4<f32>>());
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %var, 1u, 2u
    store %3, vec4<f32>(0.0f)
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, vec4<f32>, read_write> = access %var, 1u, 2u
    store %3, vec4<f32>(0.0f)
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, StoreMatrix) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Store(b.Access<ptr<private_, mat4x4<f32>>>(var, 1_u), b.Zero<mat4x4<f32>>());
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 1u
    store %3, mat4x4<f32>(vec4<f32>(0.0f))
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %var, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f)
    store %3, %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, StoreStruct) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Store(var, b.Zero(struct_type));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    store %var, S(0u, mat4x4<f32>(vec4<f32>(0.0f)), 0u)
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    store %var, S_1(0u, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>(vec4<f32>(0.0f)), 0u)
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadMatrixFromFuncParam) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* foo = b.Function("foo", ty.void_());
    auto* param = b.FunctionParam("param", ty.ptr(function, struct_type));
    foo->SetParams({param});
    b.Append(foo->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<function, mat4x4<f32>>>(param, 1_u)));
        b.Return(foo);
    });

    auto* bar = b.Function("bar", ty.void_());
    b.Append(bar->Block(), [&] {
        auto* var = b.Var("var", ty.ptr(function, struct_type));
        b.Call(foo, var);
        b.Return(bar);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

%foo = func(%param:ptr<function, S, read_write>):void {
  $B1: {
    %3:ptr<function, mat4x4<f32>, read_write> = access %param, 1u
    %4:mat4x4<f32> = load %3
    %value:mat4x4<f32> = let %4
    ret
  }
}
%bar = func():void {
  $B2: {
    %var:ptr<function, S, read_write> = var undef
    %8:void = call %foo, %var
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

%foo = func(%param:ptr<function, S_1, read_write>):void {
  $B1: {
    %3:ptr<function, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %param, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = load %3
    %5:vec4<f32> = access %4, 0u
    %6:vec4<f32> = access %4, 1u
    %7:vec4<f32> = access %4, 2u
    %8:vec4<f32> = access %4, 3u
    %9:mat4x4<f32> = construct %5, %6, %7, %8
    %value:mat4x4<f32> = let %9
    ret
  }
}
%bar = func():void {
  $B2: {
    %var:ptr<function, S_1, read_write> = var undef
    %13:void = call %foo, %var
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ReturnStructFromFunction) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* f = b.Function("foo", struct_type);
    b.Append(f->Block(), [&] {
        auto* var = b.Var("var", ty.ptr(function, struct_type));
        b.Return(f, b.Load(var));
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

%foo = func():S {
  $B1: {
    %var:ptr<function, S, read_write> = var undef
    %3:S = load %var
    ret %3
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

%foo = func():S_1 {
  $B1: {
    %var:ptr<function, S_1, read_write> = var undef
    %3:S_1 = load %var
    ret %3
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadMatrix_StructNestedInArray) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);
    auto* array_type = ty.array(struct_type, 4);

    auto* var = b.Var("var", ty.ptr<private_>(array_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<private_, mat4x4<f32>>>(var, 2_u, 1_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, array<S, 4>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 2u, 1u
    %4:mat4x4<f32> = load %3
    %value:mat4x4<f32> = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, array<S_1, 4>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %var, 2u, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = load %3
    %5:vec4<f32> = access %4, 0u
    %6:vec4<f32> = access %4, 1u
    %7:vec4<f32> = access %4, 2u
    %8:vec4<f32> = access %4, 3u
    %9:mat4x4<f32> = construct %5, %6, %7, %8
    %value:mat4x4<f32> = let %9
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadMatrix_StructNestedInStridedArray) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);
    auto* strided_array = ty.Get<spirv::type::ExplicitLayoutArray>(
        struct_type, ty.Get<core::type::ConstantArrayCount>(4u), 16u, 1024u, 256u);

    auto* var = b.Var("var", ty.ptr<private_>(strided_array));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<private_, mat4x4<f32>>>(var, 2_u, 1_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, spirv.explicit_layout_array<S, 4, stride=256>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 2u, 1u
    %4:mat4x4<f32> = load %3
    %value:mat4x4<f32> = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

$B1: {  # root
  %var:ptr<private, spirv.explicit_layout_array<S_1, 4, stride=256>, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %var, 2u, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = load %3
    %5:vec4<f32> = access %4, 0u
    %6:vec4<f32> = access %4, 1u
    %7:vec4<f32> = access %4, 2u
    %8:vec4<f32> = access %4, 3u
    %9:mat4x4<f32> = construct %5, %6, %7, %8
    %value:mat4x4<f32> = let %9
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, LoadMatrix_StructNestedInStruct) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* inner_struct_type = Struct(matrix_type, 64);
    auto* outer_struct_type =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("a"), ty.u32()},
                                                {mod.symbols.New("b"), inner_struct_type},
                                            });

    auto* var = b.Var("var", ty.ptr<private_>(outer_struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<private_, mat4x4<f32>>>(var, 1_u, 1_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

Outer = struct @align(64) {
  a_1:u32 @offset(0)
  b_1:S @offset(64)
}

$B1: {  # root
  %var:ptr<private, Outer, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 1u, 1u
    %4:mat4x4<f32> = load %3
    %value:mat4x4<f32> = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

Outer = struct @align(64) {
  a_1:u32 @offset(0)
  b_1:S @offset(64)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

Outer_1 = struct @align(64) {
  a_1:u32 @offset(0)
  b_1:S_1 @offset(64)
}

$B1: {  # root
  %var:ptr<private, Outer_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %var, 1u, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = load %3
    %5:vec4<f32> = access %4, 0u
    %6:vec4<f32> = access %4, 1u
    %7:vec4<f32> = access %4, 2u
    %8:vec4<f32> = access %4, 3u
    %9:mat4x4<f32> = construct %5, %6, %7, %8
    %value:mat4x4<f32> = let %9
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ConstructAndAccess) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64);

    auto* f = b.Function("foo", matrix_type);
    auto* p0 = b.FunctionParam("p0", ty.u32());
    auto* p1 = b.FunctionParam("p1", matrix_type);
    auto* p2 = b.FunctionParam("p2", ty.u32());
    f->SetParams({p0, p1, p2});

    b.Append(f->Block(), [&] {
        auto* c = b.Construct(struct_type, p0, p1, p2);
        auto* a = b.Access(matrix_type, c, 1_u);
        b.Return(f, a);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

%foo = func(%p0:u32, %p1:mat4x4<f32>, %p2:u32):mat4x4<f32> {
  $B1: {
    %5:S = construct %p0, %p1, %p2
    %6:mat4x4<f32> = access %5, 1u
    ret %6
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

%foo = func(%p0:u32, %p1:mat4x4<f32>, %p2:u32):mat4x4<f32> {
  $B1: {
    %5:vec4<f32> = access %p1, 0u
    %6:vec4<f32> = access %p1, 1u
    %7:vec4<f32> = access %p1, 2u
    %8:vec4<f32> = access %p1, 3u
    %9:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct %5, %6, %7, %8
    %10:S_1 = construct %p0, %9, %p2
    %11:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %10, 1u
    %12:vec4<f32> = access %11, 0u
    %13:vec4<f32> = access %11, 1u
    %14:vec4<f32> = access %11, 2u
    %15:vec4<f32> = access %11, 3u
    %16:mat4x4<f32> = construct %12, %13, %14, %15
    ret %16
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ExistingStridedArray) {
    auto* strided_array = ty.Get<spirv::type::ExplicitLayoutArray>(
        ty.vec4<f32>(), ty.Get<core::type::ConstantArrayCount>(4u), 16u, 64u, 16u);
    auto* struct_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.u32()},
                                                   {mod.symbols.New("b"), strided_array},
                                               });

    auto* var = b.Var("var", ty.ptr<private_>(struct_ty));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* array_ptr = b.Access(ty.ptr(private_, strided_array), var, 1_u);
        b.Let("value", b.Load(array_ptr));
        b.Return(f);
    });

    auto* before = R"(
MyStruct = struct @align(16) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=16> @offset(16)
}

$B1: {  # root
  %var:ptr<private, MyStruct, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=16>, read_write> = access %var, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=16> = load %3
    %value:spirv.explicit_layout_array<vec4<f32>, 4, stride=16> = let %4
    ret
  }
}
)";
    auto* after = before;

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ArrayOfStridedMatrix_LoadMatrix) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64, {2, 3});

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<private_, mat4x4<f32>>>(var, 1_u, 2_u, 1_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 1u, 2u, 1u
    %4:mat4x4<f32> = load %3
    %value:mat4x4<f32> = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> @offset(64) @size(1536)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %var, 1u, 2u, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = load %3
    %5:vec4<f32> = access %4, 0u
    %6:vec4<f32> = access %4, 1u
    %7:vec4<f32> = access %4, 2u
    %8:vec4<f32> = access %4, 3u
    %9:mat4x4<f32> = construct %5, %6, %7, %8
    %value:mat4x4<f32> = let %9
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ArrayOfStridedMatrix_LoadArray) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64, {2, 3});

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Let("value", b.Load(b.Access<ptr<private_, array<array<mat4x4<f32>, 2>, 3>>>(var, 1_u)));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, array<array<mat4x4<f32>, 2>, 3>, read_write> = access %var, 1u
    %4:array<array<mat4x4<f32>, 2>, 3> = load %3
    %value:array<array<mat4x4<f32>, 2>, 3> = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> @offset(64) @size(1536)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3>, read_write> = access %var, 1u
    %4:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> = load %3
    %5:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = access %4, 0u
    %6:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %5, 0u
    %7:vec4<f32> = access %6, 0u
    %8:vec4<f32> = access %6, 1u
    %9:vec4<f32> = access %6, 2u
    %10:vec4<f32> = access %6, 3u
    %11:mat4x4<f32> = construct %7, %8, %9, %10
    %12:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %5, 1u
    %13:vec4<f32> = access %12, 0u
    %14:vec4<f32> = access %12, 1u
    %15:vec4<f32> = access %12, 2u
    %16:vec4<f32> = access %12, 3u
    %17:mat4x4<f32> = construct %13, %14, %15, %16
    %18:array<mat4x4<f32>, 2> = construct %11, %17
    %19:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = access %4, 1u
    %20:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %19, 0u
    %21:vec4<f32> = access %20, 0u
    %22:vec4<f32> = access %20, 1u
    %23:vec4<f32> = access %20, 2u
    %24:vec4<f32> = access %20, 3u
    %25:mat4x4<f32> = construct %21, %22, %23, %24
    %26:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %19, 1u
    %27:vec4<f32> = access %26, 0u
    %28:vec4<f32> = access %26, 1u
    %29:vec4<f32> = access %26, 2u
    %30:vec4<f32> = access %26, 3u
    %31:mat4x4<f32> = construct %27, %28, %29, %30
    %32:array<mat4x4<f32>, 2> = construct %25, %31
    %33:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = access %4, 2u
    %34:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %33, 0u
    %35:vec4<f32> = access %34, 0u
    %36:vec4<f32> = access %34, 1u
    %37:vec4<f32> = access %34, 2u
    %38:vec4<f32> = access %34, 3u
    %39:mat4x4<f32> = construct %35, %36, %37, %38
    %40:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %33, 1u
    %41:vec4<f32> = access %40, 0u
    %42:vec4<f32> = access %40, 1u
    %43:vec4<f32> = access %40, 2u
    %44:vec4<f32> = access %40, 3u
    %45:mat4x4<f32> = construct %41, %42, %43, %44
    %46:array<mat4x4<f32>, 2> = construct %39, %45
    %47:array<array<mat4x4<f32>, 2>, 3> = construct %18, %32, %46
    %value:array<array<mat4x4<f32>, 2>, 3> = let %47
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ArrayOfStridedMatrix_StoreMatrix) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64, {2, 3});

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Store(b.Access<ptr<private_, mat4x4<f32>>>(var, 1_u, 2_u, 1_u), b.Zero<mat4x4<f32>>());
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, mat4x4<f32>, read_write> = access %var, 1u, 2u, 1u
    store %3, mat4x4<f32>(vec4<f32>(0.0f))
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> @offset(64) @size(1536)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %var, 1u, 2u, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f)
    store %3, %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ArrayOfStridedMatrix_StoreArray) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64, {2, 3});

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Store(b.Access<ptr<private_, array<array<mat4x4<f32>, 2>, 3>>>(var, 1_u),
                b.Zero<array<array<mat4x4<f32>, 2>, 3>>());
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, array<array<mat4x4<f32>, 2>, 3>, read_write> = access %var, 1u
    store %3, array<array<mat4x4<f32>, 2>, 3>(array<mat4x4<f32>, 2>(mat4x4<f32>(vec4<f32>(0.0f))))
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> @offset(64) @size(1536)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3>, read_write> = access %var, 1u
    %4:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f)
    %5:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f)
    %6:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = construct %4, %5
    %7:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f)
    %8:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f)
    %9:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = construct %7, %8
    %10:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f)
    %11:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f), vec4<f32>(0.0f)
    %12:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = construct %10, %11
    %13:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> = construct %6, %9, %12
    store %3, %13
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ArrayOfStridedMatrix_StoreStruct) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64, {2, 3});

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        b.Store(var, b.Zero(struct_type));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    store %var, S(0u, array<array<mat4x4<f32>, 2>, 3>(array<mat4x4<f32>, 2>(mat4x4<f32>(vec4<f32>(0.0f)))), 0u)
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> @offset(64) @size(1536)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    store %var, S_1(0u, array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3>(array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>(spirv.explicit_layout_array<vec4<f32>, 4, stride=64>(vec4<f32>(0.0f)))), 0u)
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ArrayOfStridedMatrix_ConstructAndAccess) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* array_type = ty.array(ty.array(matrix_type, 2), 3);
    auto* struct_type = Struct(matrix_type, 64, {2, 3});

    auto* f = b.Function("foo", array_type);
    auto* p0 = b.FunctionParam("p0", ty.u32());
    auto* p1 = b.FunctionParam("p1", array_type);
    auto* p2 = b.FunctionParam("p2", ty.u32());
    f->SetParams({p0, p1, p2});

    b.Append(f->Block(), [&] {
        auto* c = b.Construct(struct_type, p0, p1, p2);
        auto* a = b.Access(array_type, c, 1_u);
        b.Return(f, a);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

%foo = func(%p0:u32, %p1:array<array<mat4x4<f32>, 2>, 3>, %p2:u32):array<array<mat4x4<f32>, 2>, 3> {
  $B1: {
    %5:S = construct %p0, %p1, %p2
    %6:array<array<mat4x4<f32>, 2>, 3> = access %5, 1u
    ret %6
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> @offset(64) @size(1536)
  c:u32 @offset(1600)
}

%foo = func(%p0:u32, %p1:array<array<mat4x4<f32>, 2>, 3>, %p2:u32):array<array<mat4x4<f32>, 2>, 3> {
  $B1: {
    %5:array<mat4x4<f32>, 2> = access %p1, 0u
    %6:mat4x4<f32> = access %5, 0u
    %7:vec4<f32> = access %6, 0u
    %8:vec4<f32> = access %6, 1u
    %9:vec4<f32> = access %6, 2u
    %10:vec4<f32> = access %6, 3u
    %11:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct %7, %8, %9, %10
    %12:mat4x4<f32> = access %5, 1u
    %13:vec4<f32> = access %12, 0u
    %14:vec4<f32> = access %12, 1u
    %15:vec4<f32> = access %12, 2u
    %16:vec4<f32> = access %12, 3u
    %17:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct %13, %14, %15, %16
    %18:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = construct %11, %17
    %19:array<mat4x4<f32>, 2> = access %p1, 1u
    %20:mat4x4<f32> = access %19, 0u
    %21:vec4<f32> = access %20, 0u
    %22:vec4<f32> = access %20, 1u
    %23:vec4<f32> = access %20, 2u
    %24:vec4<f32> = access %20, 3u
    %25:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct %21, %22, %23, %24
    %26:mat4x4<f32> = access %19, 1u
    %27:vec4<f32> = access %26, 0u
    %28:vec4<f32> = access %26, 1u
    %29:vec4<f32> = access %26, 2u
    %30:vec4<f32> = access %26, 3u
    %31:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct %27, %28, %29, %30
    %32:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = construct %25, %31
    %33:array<mat4x4<f32>, 2> = access %p1, 2u
    %34:mat4x4<f32> = access %33, 0u
    %35:vec4<f32> = access %34, 0u
    %36:vec4<f32> = access %34, 1u
    %37:vec4<f32> = access %34, 2u
    %38:vec4<f32> = access %34, 3u
    %39:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct %35, %36, %37, %38
    %40:mat4x4<f32> = access %33, 1u
    %41:vec4<f32> = access %40, 0u
    %42:vec4<f32> = access %40, 1u
    %43:vec4<f32> = access %40, 2u
    %44:vec4<f32> = access %40, 3u
    %45:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = construct %41, %42, %43, %44
    %46:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = construct %39, %45
    %47:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> = construct %18, %32, %46
    %48:S_1 = construct %p0, %47, %p2
    %49:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> = access %48, 1u
    %50:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = access %49, 0u
    %51:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %50, 0u
    %52:vec4<f32> = access %51, 0u
    %53:vec4<f32> = access %51, 1u
    %54:vec4<f32> = access %51, 2u
    %55:vec4<f32> = access %51, 3u
    %56:mat4x4<f32> = construct %52, %53, %54, %55
    %57:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %50, 1u
    %58:vec4<f32> = access %57, 0u
    %59:vec4<f32> = access %57, 1u
    %60:vec4<f32> = access %57, 2u
    %61:vec4<f32> = access %57, 3u
    %62:mat4x4<f32> = construct %58, %59, %60, %61
    %63:array<mat4x4<f32>, 2> = construct %56, %62
    %64:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = access %49, 1u
    %65:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %64, 0u
    %66:vec4<f32> = access %65, 0u
    %67:vec4<f32> = access %65, 1u
    %68:vec4<f32> = access %65, 2u
    %69:vec4<f32> = access %65, 3u
    %70:mat4x4<f32> = construct %66, %67, %68, %69
    %71:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %64, 1u
    %72:vec4<f32> = access %71, 0u
    %73:vec4<f32> = access %71, 1u
    %74:vec4<f32> = access %71, 2u
    %75:vec4<f32> = access %71, 3u
    %76:mat4x4<f32> = construct %72, %73, %74, %75
    %77:array<mat4x4<f32>, 2> = construct %70, %76
    %78:array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2> = access %49, 2u
    %79:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %78, 0u
    %80:vec4<f32> = access %79, 0u
    %81:vec4<f32> = access %79, 1u
    %82:vec4<f32> = access %79, 2u
    %83:vec4<f32> = access %79, 3u
    %84:mat4x4<f32> = construct %80, %81, %82, %83
    %85:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = access %78, 1u
    %86:vec4<f32> = access %85, 0u
    %87:vec4<f32> = access %85, 1u
    %88:vec4<f32> = access %85, 2u
    %89:vec4<f32> = access %85, 3u
    %90:mat4x4<f32> = construct %86, %87, %88, %89
    %91:array<mat4x4<f32>, 2> = construct %84, %90
    %92:array<array<mat4x4<f32>, 2>, 3> = construct %63, %77, %91
    ret %92
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, ArrayOfStridedMatrix_LoadMatrix_AccessChain) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* struct_type = Struct(matrix_type, 64, {2, 3});

    auto* var = b.Var("var", ty.ptr<private_>(struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* outer_array_ptr =
            b.Access(ty.ptr(private_, ty.array(ty.array(matrix_type, 2), 3)), var, 1_u);
        auto* inner_array_ptr =
            b.Access(ty.ptr(private_, ty.array(matrix_type, 2)), outer_array_ptr, 2_u);
        auto* matrix_ptr = b.Access(ty.ptr(private_, matrix_type), inner_array_ptr, 1_u);
        b.Let("value", b.Load(matrix_ptr));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, array<array<mat4x4<f32>, 2>, 3>, read_write> = access %var, 1u
    %4:ptr<private, array<mat4x4<f32>, 2>, read_write> = access %3, 2u
    %5:ptr<private, mat4x4<f32>, read_write> = access %4, 1u
    %6:mat4x4<f32> = load %5
    %value:mat4x4<f32> = let %6
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<mat4x4<f32>, 2>, 3> @offset(64) @size(1536), @matrix_stride(64)
  c:u32 @offset(1600)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3> @offset(64) @size(1536)
  c:u32 @offset(1600)
}

$B1: {  # root
  %var:ptr<private, S_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, array<array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, 3>, read_write> = access %var, 1u
    %4:ptr<private, array<spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, 2>, read_write> = access %3, 2u
    %5:ptr<private, spirv.explicit_layout_array<vec4<f32>, 4, stride=64>, read_write> = access %4, 1u
    %6:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> = load %5
    %7:vec4<f32> = access %6, 0u
    %8:vec4<f32> = access %6, 1u
    %9:vec4<f32> = access %6, 2u
    %10:vec4<f32> = access %6, 3u
    %11:mat4x4<f32> = construct %7, %8, %9, %10
    %value:mat4x4<f32> = let %11
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, AccessNonMatrixPointer) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* inner_struct_type = Struct(matrix_type, 64);
    auto* outer_struct_type =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("s"), inner_struct_type},
                                            });

    auto* var = b.Var("var", ty.ptr<private_>(outer_struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* access = b.Access<ptr<private_, u32>>(var, 0_u, 2_u);
        b.Let("value", b.Load(access));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

Outer = struct @align(64) {
  s:S @offset(0)
}

$B1: {  # root
  %var:ptr<private, Outer, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, u32, read_write> = access %var, 0u, 2u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

Outer = struct @align(64) {
  s:S @offset(0)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

Outer_1 = struct @align(64) {
  s:S_1 @offset(0)
}

$B1: {  # root
  %var:ptr<private, Outer_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<private, u32, read_write> = access %var, 0u, 2u
    %4:u32 = load %3
    %value:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

TEST_F(SpirvReader_DecomposeStridedMatrixTest, AccessNonMatrixValue) {
    auto* matrix_type = ty.mat4x4<f32>();
    auto* inner_struct_type = Struct(matrix_type, 64);
    auto* outer_struct_type =
        ty.Struct(mod.symbols.New("Outer"), {
                                                {mod.symbols.New("s"), inner_struct_type},
                                            });

    auto* var = b.Var("var", ty.ptr<private_>(outer_struct_type));
    mod.root_block->Append(var);

    auto* f = b.ComputeFunction("foo");
    b.Append(f->Block(), [&] {
        auto* struct_value = b.Load(var);
        b.Let("value", b.Access<u32>(struct_value, 0_u, 2_u));
        b.Return(f);
    });

    auto* before = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

Outer = struct @align(64) {
  s:S @offset(0)
}

$B1: {  # root
  %var:ptr<private, Outer, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:Outer = load %var
    %4:u32 = access %3, 0u, 2u
    %value:u32 = let %4
    ret
  }
}
)";
    auto* after = R"(
S = struct @align(64) {
  a:u32 @offset(0)
  b:mat4x4<f32> @offset(64) @size(256), @matrix_stride(64)
  c:u32 @offset(320)
}

Outer = struct @align(64) {
  s:S @offset(0)
}

S_1 = struct @align(64) {
  a:u32 @offset(0)
  b:spirv.explicit_layout_array<vec4<f32>, 4, stride=64> @offset(64)
  c:u32 @offset(320)
}

Outer_1 = struct @align(64) {
  s:S_1 @offset(0)
}

$B1: {  # root
  %var:ptr<private, Outer_1, read_write> = var undef
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:Outer_1 = load %var
    %4:u32 = access %3, 0u, 2u
    %value:u32 = let %4
    ret
  }
}
)";

    ASSERT_EQ(before, str());
    Run(DecomposeStridedMatrix);
    ASSERT_EQ(after, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower
