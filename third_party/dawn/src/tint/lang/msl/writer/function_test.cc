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

#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/msl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

TEST_F(MslWriterTest, Function_Empty) {
    auto* func = b.Function("foo", ty.void_());
    func->Block()->Append(b.Return(func));

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
void foo() {
}
)");

    // MSL doesn't inject an empty entry point, so in this case there is no result.
    EXPECT_EQ(output_.workgroup_info.x, 0u);
    EXPECT_EQ(output_.workgroup_info.y, 0u);
    EXPECT_EQ(output_.workgroup_info.z, 0u);
}

TEST_F(MslWriterTest, Function_EntryPoint_Compute) {
    auto* func = b.ComputeFunction("cmp_main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
kernel void cmp_main() {
}
)");

    EXPECT_EQ(output_.workgroup_info.x, 32u);
    EXPECT_EQ(output_.workgroup_info.y, 4u);
    EXPECT_EQ(output_.workgroup_info.z, 1u);
}

TEST_F(MslWriterTest, EntryPointParameterBufferBindingPoint) {
    auto* storage = b.Var("storage_var", ty.ptr(core::AddressSpace::kStorage, ty.i32()));
    auto* uniform = b.Var("uniform_var", ty.ptr(core::AddressSpace::kUniform, ty.i32()));
    storage->SetBindingPoint(0, 1);
    uniform->SetBindingPoint(0, 2);
    mod.root_block->Append(storage);
    mod.root_block->Append(uniform);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Load(storage);
        b.Load(uniform);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device int* storage_var;
  const constant int* uniform_var;
};

fragment void foo(device int* storage_var [[buffer(1)]], const constant int* uniform_var [[buffer(2)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.storage_var=storage_var, .uniform_var=uniform_var};
}
)");
    EXPECT_EQ(output_.workgroup_info.x, 0u);
    EXPECT_EQ(output_.workgroup_info.y, 0u);
    EXPECT_EQ(output_.workgroup_info.z, 0u);
}

TEST_F(MslWriterTest, EntryPointParameterHandleBindingPoint) {
    auto* t = ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d, ty.f32());
    auto* texture = b.Var("t", ty.ptr<handle>(t));
    auto* sampler = b.Var("s", ty.ptr<handle>(ty.sampler()));
    texture->SetBindingPoint(0, 1);
    sampler->SetBindingPoint(0, 2);
    mod.root_block->Append(texture);
    mod.root_block->Append(sampler);

    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        b.Load(texture);
        b.Load(sampler);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

struct tint_module_vars_struct {
  texture2d<float, access::sample> t;
  sampler s;
};

fragment void foo(texture2d<float, access::sample> t [[texture(1)]], sampler s [[sampler(2)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.t=t, .s=s};
}
)");
}

TEST_F(MslWriterTest, WorkgroupStorageSizeEmpty) {
    auto* func = b.ComputeFunction("cs_main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(0u, output_.workgroup_info.storage_size);
}

TEST_F(MslWriterTest, WorkgroupStorageSizeSimple) {
    auto* var = mod.root_block->Append(b.Var("var", ty.ptr(workgroup, ty.f32())));
    auto* var2 = mod.root_block->Append(b.Var("var2", ty.ptr(workgroup, ty.i32())));

    auto* func = b.ComputeFunction("cs_main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", var);
        b.Let("y", var2);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(32u, output_.workgroup_info.storage_size);
}

TEST_F(MslWriterTest, WorkgroupStorageSizeCompoundTypes) {
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

    auto* func = b.ComputeFunction("cs_main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", f32_var);
        b.Let("y", str_var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(96u, output_.workgroup_info.storage_size);
}

TEST_F(MslWriterTest, WorkgroupStorageSizeAlignmentPadding) {
    // vec3<f32> has an alignment of 16 but a size of 12. We leverage this to test
    // that our padded size calculation for workgroup storage is accurate.
    auto* var = mod.root_block->Append(b.Var("var_f32", ty.ptr(workgroup, ty.vec3<f32>())));

    auto* func = b.ComputeFunction("cs_main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(16u, output_.workgroup_info.storage_size);
}

TEST_F(MslWriterTest, WorkgroupStorageSizeStructAlignment) {
    // Per WGSL spec, a struct's size is the offset its last member plus the size
    // of its last member, rounded up to the alignment of its largest member. So
    // here the struct is expected to occupy 1024 bytes of workgroup storage.
    Vector members{
        ty.Get<core::type::StructMember>(mod.symbols.New("a"), ty.i32(), 0u, 0u, 1024u, 4u,
                                         core::IOAttributes{}),
    };

    auto* wg_struct_ty = ty.Struct(mod.symbols.New("WgStruct"), members);
    auto* var = mod.root_block->Append(b.Var("var_f32", ty.ptr(workgroup, wg_struct_ty)));

    auto* func = b.ComputeFunction("cs_main", 32_u, 4_u, 1_u);
    b.Append(func->Block(), [&] {  //
        b.Let("x", var);
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(1024u, output_.workgroup_info.storage_size);
}

}  // namespace
}  // namespace tint::msl::writer
