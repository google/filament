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

#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/texture.h"

namespace tint::core::type {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using TypeStructTest = TestHelper;

TEST_F(TypeStructTest, Creation) {
    auto name = Sym("S");
    auto* s =
        create<Struct>(name, tint::Empty, 4u /* align */, 8u /* size */, 16u /* size_no_padding */);
    EXPECT_EQ(s->Align(), 4u);
    EXPECT_EQ(s->Size(), 8u);
    EXPECT_EQ(s->SizeNoPadding(), 16u);
}

TEST_F(TypeStructTest, Equals) {
    auto* a = create<Struct>(Sym("a"), tint::Empty, 4u /* align */, 4u /* size */,
                             4u /* size_no_padding */);
    auto* b = create<Struct>(Sym("b"), tint::Empty, 4u /* align */, 4u /* size */,
                             4u /* size_no_padding */);

    EXPECT_TRUE(a->Equals(*a));
    EXPECT_FALSE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(TypeStructTest, FriendlyName) {
    auto name = Sym("my_struct");
    auto* s =
        create<Struct>(name, tint::Empty, 4u /* align */, 4u /* size */, 4u /* size_no_padding */);
    EXPECT_EQ(s->FriendlyName(), "my_struct");
}

TEST_F(TypeStructTest, Layout) {
    auto* inner_st =  //
        Structure("Inner", tint::Vector{
                               Member("a", ty.i32()),
                               Member("b", ty.u32()),
                               Member("c", ty.f32()),
                               Member("d", ty.vec3<f32>()),
                               Member("e", ty.mat4x2<f32>()),
                           });

    auto* outer_st = Structure("Outer", tint::Vector{
                                            Member("inner", ty("Inner")),
                                            Member("a", ty.i32()),
                                        });

    auto p = Build();
    ASSERT_TRUE(p.IsValid()) << p.Diagnostics();

    auto* sem_inner_st = p.Sem().Get(inner_st);
    auto* sem_outer_st = p.Sem().Get(outer_st);

    EXPECT_EQ(sem_inner_st->Layout().Plain(),
              R"(/*            align(16) size(64) */ struct Inner {
/* offset( 0) align( 4) size( 4) */   a : i32,
/* offset( 4) align( 4) size( 4) */   b : u32,
/* offset( 8) align( 4) size( 4) */   c : f32,
/* offset(12) align( 1) size( 4) */   // -- implicit field alignment padding --
/* offset(16) align(16) size(12) */   d : vec3<f32>,
/* offset(28) align( 1) size( 4) */   // -- implicit field alignment padding --
/* offset(32) align( 8) size(32) */   e : mat4x2<f32>,
/*                               */ };)");

    EXPECT_EQ(sem_outer_st->Layout().Plain(),
              R"(/*            align(16) size(80) */ struct Outer {
/* offset( 0) align(16) size(64) */   inner : Inner,
/* offset(64) align( 4) size( 4) */   a : i32,
/* offset(68) align( 1) size(12) */   // -- implicit struct size padding --
/*                               */ };)");
}

TEST_F(TypeStructTest, Location) {
    auto* st = Structure("st", tint::Vector{
                                   Member("a", ty.i32(), tint::Vector{Location(1_u)}),
                                   Member("b", ty.u32()),
                               });

    auto p = Build();
    ASSERT_TRUE(p.IsValid()) << p.Diagnostics();

    auto* sem = p.Sem().Get(st);
    ASSERT_EQ(2u, sem->Members().Length());

    EXPECT_EQ(sem->Members()[0]->Attributes().location, 1u);
    EXPECT_FALSE(sem->Members()[1]->Attributes().location.has_value());
}

TEST_F(TypeStructTest, IsConstructable) {
    auto* inner =  //
        Structure("Inner", tint::Vector{
                               Member("a", ty.i32()),
                               Member("b", ty.u32()),
                               Member("c", ty.f32()),
                               Member("d", ty.vec3<f32>()),
                               Member("e", ty.mat4x2<f32>()),
                           });

    auto* outer = Structure("Outer", tint::Vector{
                                         Member("inner", ty("Inner")),
                                         Member("a", ty.i32()),
                                     });

    auto* outer_runtime_sized_array =
        Structure("OuterRuntimeSizedArray", tint::Vector{
                                                Member("inner", ty("Inner")),
                                                Member("a", ty.i32()),
                                                Member("runtime_sized_array", ty.array<i32>()),
                                            });
    auto p = Build();
    ASSERT_TRUE(p.IsValid()) << p.Diagnostics();

    auto* sem_inner = p.Sem().Get(inner);
    auto* sem_outer = p.Sem().Get(outer);
    auto* sem_outer_runtime_sized_array = p.Sem().Get(outer_runtime_sized_array);

    EXPECT_TRUE(sem_inner->IsConstructible());
    EXPECT_TRUE(sem_outer->IsConstructible());
    EXPECT_FALSE(sem_outer_runtime_sized_array->IsConstructible());
}

TEST_F(TypeStructTest, HasCreationFixedFootprint) {
    auto* inner =  //
        Structure("Inner", tint::Vector{
                               Member("a", ty.i32()),
                               Member("b", ty.u32()),
                               Member("c", ty.f32()),
                               Member("d", ty.vec3<f32>()),
                               Member("e", ty.mat4x2<f32>()),
                               Member("f", ty.array<f32, 32>()),
                           });

    auto* outer = Structure("Outer", tint::Vector{
                                         Member("inner", ty("Inner")),
                                     });

    auto* outer_with_runtime_sized_array =
        Structure("OuterRuntimeSizedArray", tint::Vector{
                                                Member("inner", ty("Inner")),
                                                Member("runtime_sized_array", ty.array<i32>()),
                                            });

    auto p = Build();
    ASSERT_TRUE(p.IsValid()) << p.Diagnostics();

    auto* sem_inner = p.Sem().Get(inner);
    auto* sem_outer = p.Sem().Get(outer);
    auto* sem_outer_with_runtime_sized_array = p.Sem().Get(outer_with_runtime_sized_array);

    EXPECT_TRUE(sem_inner->HasCreationFixedFootprint());
    EXPECT_TRUE(sem_outer->HasCreationFixedFootprint());
    EXPECT_FALSE(sem_outer_with_runtime_sized_array->HasCreationFixedFootprint());
}

TEST_F(TypeStructTest, HasFixedFootprint) {
    auto* inner =  //
        Structure("Inner", tint::Vector{
                               Member("a", ty.i32()),
                               Member("b", ty.u32()),
                               Member("c", ty.f32()),
                               Member("d", ty.vec3<f32>()),
                               Member("e", ty.mat4x2<f32>()),
                               Member("f", ty.array<f32, 32>()),
                           });

    auto* outer = Structure("Outer", tint::Vector{
                                         Member("inner", ty("Inner")),
                                     });

    auto* outer_with_runtime_sized_array =
        Structure("OuterRuntimeSizedArray", tint::Vector{
                                                Member("inner", ty("Inner")),
                                                Member("runtime_sized_array", ty.array<i32>()),
                                            });

    auto p = Build();
    ASSERT_TRUE(p.IsValid()) << p.Diagnostics();

    auto* sem_inner = p.Sem().Get(inner);
    auto* sem_outer = p.Sem().Get(outer);
    auto* sem_outer_with_runtime_sized_array = p.Sem().Get(outer_with_runtime_sized_array);

    EXPECT_TRUE(sem_inner->HasFixedFootprint());
    EXPECT_TRUE(sem_outer->HasFixedFootprint());
    EXPECT_FALSE(sem_outer_with_runtime_sized_array->HasFixedFootprint());
}

TEST_F(TypeStructTest, Clone) {
    core::IOAttributes attrs_location_2;
    attrs_location_2.location = 2;

    auto* s = create<Struct>(
        Sym("my_struct"),
        tint::Vector{
            create<StructMember>(Sym("b"), create<Vector>(create<F32>(), 3u), 0u, 0u, 16u, 12u,
                                 attrs_location_2),
            create<StructMember>(Sym("a"), create<I32>(), 1u, 16u, 4u, 4u, core::IOAttributes{})},
        4u /* align */, 8u /* size */, 16u /* size_no_padding */);

    GenerationID id;
    SymbolTable new_st{id};

    core::type::Manager mgr;
    core::type::CloneContext ctx{{&Symbols()}, {&new_st, &mgr}};

    auto* st = s->Clone(ctx);

    EXPECT_TRUE(new_st.Get("my_struct").IsValid());
    EXPECT_EQ(st->Name().Name(), "my_struct");

    EXPECT_EQ(st->Align(), 4u);
    EXPECT_EQ(st->Size(), 8u);
    EXPECT_EQ(st->SizeNoPadding(), 16u);

    auto members = st->Members();
    ASSERT_EQ(members.Length(), 2u);

    EXPECT_EQ(members[0]->Name().Name(), "b");
    EXPECT_TRUE(members[0]->Type()->Is<Vector>());
    EXPECT_EQ(members[0]->Index(), 0u);
    EXPECT_EQ(members[0]->Offset(), 0u);
    EXPECT_EQ(members[0]->Align(), 16u);
    EXPECT_EQ(members[0]->Size(), 12u);

    EXPECT_EQ(members[1]->Name().Name(), "a");
    EXPECT_TRUE(members[1]->Type()->Is<I32>());
    EXPECT_EQ(members[1]->Index(), 1u);
    EXPECT_EQ(members[1]->Offset(), 16u);
    EXPECT_EQ(members[1]->Align(), 4u);
    EXPECT_EQ(members[1]->Size(), 4u);
}

}  // namespace
}  // namespace tint::core::type
