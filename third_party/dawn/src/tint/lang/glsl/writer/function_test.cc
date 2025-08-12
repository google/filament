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

#include "src/tint/lang/glsl/writer/helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::glsl::writer {
namespace {

TEST_F(GlslWriterTest, Function_Empty) {
    auto* func = b.ComputeFunction("main");
    func->Block()->Append(b.Return(func));

    Options opts{};
    ASSERT_TRUE(Generate(opts, core::ir::Function::PipelineStage::kCompute))
        << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
    EXPECT_EQ(1u, output_.workgroup_info.x);
    EXPECT_EQ(1u, output_.workgroup_info.y);
    EXPECT_EQ(1u, output_.workgroup_info.z);
}

TEST_F(GlslWriterTest, Function_ComputeWgSize) {
    auto* func = b.ComputeFunction("main", 2_u, 4_u, 6_u);
    func->Block()->Append(b.Return(func));

    Options opts{};
    ASSERT_TRUE(Generate(opts, core::ir::Function::PipelineStage::kCompute))
        << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
layout(local_size_x = 2, local_size_y = 4, local_size_z = 6) in;
void main() {
}
)");
    EXPECT_EQ(2u, output_.workgroup_info.x);
    EXPECT_EQ(4u, output_.workgroup_info.y);
    EXPECT_EQ(6u, output_.workgroup_info.z);
}

TEST_F(GlslWriterTest, FunctionWithParams) {
    auto* func = b.Function("my_func", ty.void_());
    func->SetParams({b.FunctionParam("a", ty.f32()), b.FunctionParam("b", ty.i32())});
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(
void my_func(float a, int b) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
)");
}

TEST_F(GlslWriterTest, Function_Fragment_Precision) {
    auto* func = b.Function("main", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(output_.glsl, GlslHeader() + R"(precision highp float;
precision highp int;

void main() {
}
)");
    EXPECT_EQ(0u, output_.workgroup_info.x);
    EXPECT_EQ(0u, output_.workgroup_info.y);
    EXPECT_EQ(0u, output_.workgroup_info.z);
}

TEST_F(GlslWriterTest, WorkgroupStorageSizeEmpty) {
    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(0u, output_.workgroup_info.storage_size);
}

TEST_F(GlslWriterTest, WorkgroupStorageSizeSimple) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.f32())));
    auto* var2 = mod.root_block->Append(b.Var("var2", ty.ptr(workgroup, ty.i32())));

    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", var);
        b.Let("y", var2);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(32u, output_.workgroup_info.storage_size);
}

TEST_F(GlslWriterTest, WorkgroupStorageSizeCompoundTypes) {
    Vector members{
        ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 4u, 4u,
                                         core::IOAttributes{}),
        ty.Get<core::type::StructMember>(mod.symbols.New("b"), ty.array<i32, 4>(), 1u, 4u, 16u, 64u,
                                         core::IOAttributes{}),
    };

    // This struct should occupy 68 bytes. 4 from the i32 field, and another 64
    // from the 4-element array with 16-byte stride.
    auto* wg_struct_ty = ty.Struct(mod.symbols.New("WgStruct"), members);
    auto* str_var = mod.root_block->Append(b.Var("var_struct", ty.ptr(workgroup, wg_struct_ty)));

    // Plus another 4 bytes from this other workgroup-class f32.
    auto* f32_var = mod.root_block->Append(b.Var("var_f32", ty.ptr(workgroup, ty.f32())));

    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", f32_var);
        b.Let("y", str_var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(96u, output_.workgroup_info.storage_size);
}

TEST_F(GlslWriterTest, WorkgroupStorageSizeAlignmentPadding) {
    // vec3<f32> has an alignment of 16 but a size of 12. We leverage this to test
    // that our padded size calculation for workgroup storage is accurate.
    auto* var = mod.root_block->Append(b.Var("var_f32", ty.ptr(workgroup, ty.vec3<f32>())));

    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(16u, output_.workgroup_info.storage_size);
}

TEST_F(GlslWriterTest, WorkgroupStorageSizeStructAlignment) {
    // Per WGSL spec, a struct's size is the offset its last member plus the size
    // of its last member, rounded up to the alignment of its largest member. So
    // here the struct is expected to occupy 1024 bytes of workgroup storage.
    Vector members{
        ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 1024u, 4u,
                                         core::IOAttributes{}),
    };

    auto* wg_struct_ty = ty.Struct(mod.symbols.New("WgStruct"), members);
    auto* var = mod.root_block->Append(b.Var("var_f32", ty.ptr(workgroup, wg_struct_ty)));

    auto* func = b.ComputeFunction("main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.glsl;
    EXPECT_EQ(1024u, output_.workgroup_info.storage_size);
}

}  // namespace
}  // namespace tint::glsl::writer
