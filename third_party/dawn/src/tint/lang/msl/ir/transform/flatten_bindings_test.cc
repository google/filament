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

#include "src/tint/lang/msl/ir/transform/flatten_bindings.h"

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::msl::ir::transform {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

using FlattenBindingsTest = core::ir::IRTestHelper;

TEST_F(FlattenBindingsTest, NoBindings) {
    auto* f = b.ComputeFunction("main");
    b.Append(f->Block(), [&] { b.Return(f); });

    auto flattened = FlattenBindings(mod);
    EXPECT_TRUE(flattened == Success);
}

TEST_F(FlattenBindingsTest, AlreadyFlat) {
    b.Append(mod.root_block, [&] {
        auto* a = b.Var("a", ty.ptr(uniform, ty.i32(), read));
        a->SetBindingPoint(0, 0);

        auto* b_ = b.Var("b", ty.ptr(uniform, ty.i32(), read));
        b_->SetBindingPoint(0, 1);

        auto* c = b.Var("c", ty.ptr(uniform, ty.i32(), read));
        c->SetBindingPoint(0, 2);
    });

    auto flattened = FlattenBindings(mod);
    EXPECT_TRUE(flattened == Success);
}

TEST_F(FlattenBindingsTest, NotFlat_SingleNamespace) {
    core::ir::Var* a;
    core::ir::Var* b_;
    core::ir::Var* c;
    b.Append(mod.root_block, [&] {
        a = b.Var("a", ty.ptr(uniform, ty.i32(), read));
        a->SetBindingPoint(0, 0);

        b_ = b.Var("b", ty.ptr(uniform, ty.i32(), read));
        b_->SetBindingPoint(1, 1);

        c = b.Var("c", ty.ptr(uniform, ty.i32(), read));
        c->SetBindingPoint(2, 2);
    });

    auto flattened = FlattenBindings(mod);
    EXPECT_TRUE(flattened == Success);

    ASSERT_TRUE(a->BindingPoint().has_value());
    EXPECT_EQ(a->BindingPoint()->group, 0u);
    EXPECT_EQ(a->BindingPoint()->binding, 0u);

    ASSERT_TRUE(b_->BindingPoint().has_value());
    EXPECT_EQ(b_->BindingPoint()->group, 0u);
    EXPECT_EQ(b_->BindingPoint()->binding, 1u);

    ASSERT_TRUE(c->BindingPoint().has_value());
    EXPECT_EQ(c->BindingPoint()->group, 0u);
    EXPECT_EQ(c->BindingPoint()->binding, 2u);
}

TEST_F(FlattenBindingsTest, NotFlat_MultipleNamespaces) {
    core::ir::Var* a;
    core::ir::Var* b_;
    core::ir::Var* c;

    core::ir::Var* s1;
    core::ir::Var* s2;

    core::ir::Var* t1;
    core::ir::Var* t2;
    core::ir::Var* t3;
    core::ir::Var* t4;
    core::ir::Var* t5;
    core::ir::Var* t6;

    b.Append(mod.root_block, [&] {
        a = b.Var("a", ty.ptr(uniform, ty.i32(), read));
        a->SetBindingPoint(0, 0);

        b_ = b.Var("b", ty.ptr(storage, ty.i32(), read_write));
        b_->SetBindingPoint(1, 1);

        c = b.Var("c", ty.ptr(storage, ty.i32(), read));
        c->SetBindingPoint(2, 2);

        s1 = b.Var("sampler1", ty.ptr(handle, ty.sampler(), read));
        s1->SetBindingPoint(3, 3);

        s2 = b.Var("sampler2", ty.ptr(handle, ty.comparison_sampler(), read));
        s2->SetBindingPoint(4, 4);

        t1 = b.Var(
            "texture1",
            ty.ptr(handle, ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), read));
        t1->SetBindingPoint(5, 5);
        t2 = b.Var(
            "texture2",
            ty.ptr(handle, ty.multisampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                   read));
        t2->SetBindingPoint(6, 6);
        t3 = b.Var("texture3", ty.ptr(handle,
                                      ty.storage_texture(core::type::TextureDimension::k2d,
                                                         core::TexelFormat::kR32Float, write),
                                      read));
        t3->SetBindingPoint(7, 7);
        t4 = b.Var("texture4",
                   ty.ptr(handle, ty.depth_texture(core::type::TextureDimension::k2d), read));
        t4->SetBindingPoint(8, 8);
        t5 = b.Var(
            "texture5",
            ty.ptr(handle, ty.depth_multisampled_texture(core::type::TextureDimension::k2d), read));
        t5->SetBindingPoint(9, 9);
        t6 = b.Var("texture6", ty.ptr(handle, ty.external_texture(), read));
        t6->SetBindingPoint(10, 10);
    });

    auto flattened = FlattenBindings(mod);
    EXPECT_TRUE(flattened == Success);

    ASSERT_TRUE(a->BindingPoint().has_value());
    EXPECT_EQ(a->BindingPoint()->group, 0u);
    EXPECT_EQ(a->BindingPoint()->binding, 0u);

    ASSERT_TRUE(b_->BindingPoint().has_value());
    EXPECT_EQ(b_->BindingPoint()->group, 0u);
    EXPECT_EQ(b_->BindingPoint()->binding, 1u);

    ASSERT_TRUE(c->BindingPoint().has_value());
    EXPECT_EQ(c->BindingPoint()->group, 0u);
    EXPECT_EQ(c->BindingPoint()->binding, 2u);

    ASSERT_TRUE(s1->BindingPoint().has_value());
    EXPECT_EQ(s1->BindingPoint()->group, 0u);
    EXPECT_EQ(s1->BindingPoint()->binding, 0u);

    ASSERT_TRUE(s2->BindingPoint().has_value());
    EXPECT_EQ(s2->BindingPoint()->group, 0u);
    EXPECT_EQ(s2->BindingPoint()->binding, 1u);

    ASSERT_TRUE(t1->BindingPoint().has_value());
    EXPECT_EQ(t1->BindingPoint()->group, 0u);
    EXPECT_EQ(t1->BindingPoint()->binding, 0u);

    ASSERT_TRUE(t2->BindingPoint().has_value());
    EXPECT_EQ(t2->BindingPoint()->group, 0u);
    EXPECT_EQ(t2->BindingPoint()->binding, 1u);

    ASSERT_TRUE(t3->BindingPoint().has_value());
    EXPECT_EQ(t3->BindingPoint()->group, 0u);
    EXPECT_EQ(t3->BindingPoint()->binding, 2u);

    ASSERT_TRUE(t4->BindingPoint().has_value());
    EXPECT_EQ(t4->BindingPoint()->group, 0u);
    EXPECT_EQ(t4->BindingPoint()->binding, 3u);

    ASSERT_TRUE(t5->BindingPoint().has_value());
    EXPECT_EQ(t5->BindingPoint()->group, 0u);
    EXPECT_EQ(t5->BindingPoint()->binding, 4u);

    ASSERT_TRUE(t6->BindingPoint().has_value());
    EXPECT_EQ(t6->BindingPoint()->group, 0u);
    EXPECT_EQ(t6->BindingPoint()->binding, 5u);
}

}  // namespace
}  // namespace tint::msl::ir::transform
