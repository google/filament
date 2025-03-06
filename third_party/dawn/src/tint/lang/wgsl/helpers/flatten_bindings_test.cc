// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/helpers/flatten_bindings.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::wgsl {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

class FlattenBindingsTest : public ::testing::Test {};

TEST_F(FlattenBindingsTest, NoBindings) {
    ProgramBuilder b;
    Program program(resolver::Resolve(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    auto flattened = FlattenBindings(program);
    EXPECT_FALSE(flattened);
}

TEST_F(FlattenBindingsTest, AlreadyFlat) {
    ProgramBuilder b;
    b.GlobalVar("a", b.ty.i32(), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("b", b.ty.i32(), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(1_a));
    b.GlobalVar("c", b.ty.i32(), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(2_a));

    Program program(resolver::Resolve(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    auto flattened = FlattenBindings(program);
    EXPECT_FALSE(flattened);
}

TEST_F(FlattenBindingsTest, NotFlat_SingleNamespace) {
    ProgramBuilder b;
    b.GlobalVar("a", b.ty.i32(), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("b", b.ty.i32(), core::AddressSpace::kUniform, b.Group(1_a), b.Binding(1_a));
    b.GlobalVar("c", b.ty.i32(), core::AddressSpace::kUniform, b.Group(2_a), b.Binding(2_a));
    b.WrapInFunction(b.Expr("a"), b.Expr("b"), b.Expr("c"));

    Program program(resolver::Resolve(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    auto flattened = FlattenBindings(program);
    EXPECT_TRUE(flattened);

    auto& vars = flattened->AST().GlobalVariables();

    auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[0]);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Attributes().binding_point->group, 0u);
    EXPECT_EQ(sem->Attributes().binding_point->binding, 0u);

    sem = flattened->Sem().Get<sem::GlobalVariable>(vars[1]);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Attributes().binding_point->group, 0u);
    EXPECT_EQ(sem->Attributes().binding_point->binding, 1u);

    sem = flattened->Sem().Get<sem::GlobalVariable>(vars[2]);
    ASSERT_NE(sem, nullptr);
    EXPECT_EQ(sem->Attributes().binding_point->group, 0u);
    EXPECT_EQ(sem->Attributes().binding_point->binding, 2u);
}

TEST_F(FlattenBindingsTest, NotFlat_MultipleNamespaces) {
    ProgramBuilder b;

    const size_t num_buffers = 3;
    b.GlobalVar("buffer1", b.ty.i32(), core::AddressSpace::kUniform, b.Group(0_a), b.Binding(0_a));
    b.GlobalVar("buffer2", b.ty.i32(), core::AddressSpace::kStorage, b.Group(1_a), b.Binding(1_a));
    b.GlobalVar("buffer3", b.ty.i32(), core::AddressSpace::kStorage, core::Access::kRead,
                b.Group(2_a), b.Binding(2_a));

    const size_t num_samplers = 2;
    b.GlobalVar("sampler1", b.ty.sampler(core::type::SamplerKind::kSampler), b.Group(3_a),
                b.Binding(3_a));
    b.GlobalVar("sampler2", b.ty.sampler(core::type::SamplerKind::kComparisonSampler), b.Group(4_a),
                b.Binding(4_a));

    const size_t num_textures = 6;
    b.GlobalVar("texture1", b.ty.sampled_texture(core::type::TextureDimension::k2d, b.ty.f32()),
                b.Group(5_a), b.Binding(5_a));
    b.GlobalVar("texture2",
                b.ty.multisampled_texture(core::type::TextureDimension::k2d, b.ty.f32()),
                b.Group(6_a), b.Binding(6_a));
    b.GlobalVar("texture3",
                b.ty.storage_texture(core::type::TextureDimension::k2d,
                                     core::TexelFormat::kR32Float, core::Access::kWrite),
                b.Group(7_a), b.Binding(7_a));
    b.GlobalVar("texture4", b.ty.depth_texture(core::type::TextureDimension::k2d), b.Group(8_a),
                b.Binding(8_a));
    b.GlobalVar("texture5", b.ty.depth_multisampled_texture(core::type::TextureDimension::k2d),
                b.Group(9_a), b.Binding(9_a));
    b.GlobalVar("texture6", b.ty.external_texture(), b.Group(10_a), b.Binding(10_a));

    b.WrapInFunction(b.Assign(b.Phony(), "buffer1"), b.Assign(b.Phony(), "buffer2"),
                     b.Assign(b.Phony(), "buffer3"), b.Assign(b.Phony(), "sampler1"),
                     b.Assign(b.Phony(), "sampler2"), b.Assign(b.Phony(), "texture1"),
                     b.Assign(b.Phony(), "texture2"), b.Assign(b.Phony(), "texture3"),
                     b.Assign(b.Phony(), "texture4"), b.Assign(b.Phony(), "texture5"),
                     b.Assign(b.Phony(), "texture6"));

    Program program(resolver::Resolve(b));
    ASSERT_TRUE(program.IsValid()) << program.Diagnostics();

    auto flattened = FlattenBindings(program);
    EXPECT_TRUE(flattened);

    auto& vars = flattened->AST().GlobalVariables();

    for (size_t i = 0; i < num_buffers; ++i) {
        auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[i]);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->Attributes().binding_point->group, 0u);
        EXPECT_EQ(sem->Attributes().binding_point->binding, i);
    }
    for (size_t i = 0; i < num_samplers; ++i) {
        auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[i + num_buffers]);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->Attributes().binding_point->group, 0u);
        EXPECT_EQ(sem->Attributes().binding_point->binding, i);
    }
    for (size_t i = 0; i < num_textures; ++i) {
        auto* sem = flattened->Sem().Get<sem::GlobalVariable>(vars[i + num_buffers + num_samplers]);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->Attributes().binding_point->group, 0u);
        EXPECT_EQ(sem->Attributes().binding_point->binding, i);
    }
}

}  // namespace
}  // namespace tint::wgsl
