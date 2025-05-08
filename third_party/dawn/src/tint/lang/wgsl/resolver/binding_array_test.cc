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

#include "src/tint/lang/core/type/binding_array.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverBindingArrayTest = ResolverTest;

TEST_F(ResolverBindingArrayTest, ValidGlobalDecl) {
    auto* var = GlobalVar(
        "a", Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(var)->UnwrapRef()->As<core::type::BindingArray>();
    ASSERT_NE(m, nullptr);
    ASSERT_TRUE(m->Count()->Is<core::type::ConstantArrayCount>());
    ASSERT_EQ(m->Count()->As<core::type::ConstantArrayCount>()->value, 4u);
    ASSERT_TRUE(m->ElemType()->Is<core::type::SampledTexture>());
    ASSERT_TRUE(m->ElemType()->As<core::type::SampledTexture>()->Type()->Is<core::type::F32>());
    ASSERT_EQ(m->ElemType()->As<core::type::SampledTexture>()->Dim(),
              core::type::TextureDimension::k2d);
}

TEST_F(ResolverBindingArrayTest, InvalidNoFeature) {
    GlobalVar(
        "a", Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(
        resolver.error(),
        R"(error: use of 'binding_array' requires the 'sized_binding_array'language feature, which is not allowed in the current environment)");
}

TEST_F(ResolverBindingArrayTest, InvalidGlobalDeclNoGroup) {
    GlobalVar(
        "a", Binding(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResolverBindingArrayTest, InvalidGlobalDeclNoBinding) {
    GlobalVar(
        "a", Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: resource variables require '@group' and '@binding' attributes)");
}

TEST_F(ResolverBindingArrayTest, InvalidGlobalDeclPrivate) {
    GlobalVar(
        "a", private_,
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: variables of type 'binding_array<texture_2d<f32>, 4>' must not specify an address space)");
}

TEST_F(ResolverBindingArrayTest, InvalidGlobalDeclWorkgroup) {
    GlobalVar(
        "a", workgroup,
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: variables of type 'binding_array<texture_2d<f32>, 4>' must not specify an address space)");
}

TEST_F(ResolverBindingArrayTest, InvalidGlobalDeclStorageWithHandleType) {
    GlobalVar(
        "a", storage, Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: type 'binding_array<texture_2d<f32>, 4>' cannot be used in address space 'storage' as it is non-host-shareable
note: while instantiating 'var' a)");
}

TEST_F(ResolverBindingArrayTest, InvalidGlobalDeclUniformWithHandleType) {
    GlobalVar(
        "a", uniform, Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: type 'binding_array<texture_2d<f32>, 4>' cannot be used in address space 'uniform' as it is non-host-shareable
note: while instantiating 'var' a)");
}

TEST_F(ResolverBindingArrayTest, InvalidGlobalDeclOverride) {
    Override("a", ty("binding_array",
                     ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: binding_array<texture_2d<f32>, 4> cannot be used as the type of a 'override')");
}

TEST_F(ResolverBindingArrayTest, InvalidFuncDecl) {
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("a",
                      ty("binding_array",
                         ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: function-scope 'var' must have a constructible type)");
}

TEST_F(ResolverBindingArrayTest, InvalidStructMember) {
    Structure(
        "S",
        Vector{
            Member("a", ty("binding_array",
                           ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u)),
        });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: binding_array<texture_2d<f32>, 4> cannot be used as the type of a structure member)");
}

TEST_F(ResolverBindingArrayTest, ValidFunctionParameter) {
    Func("foo",
         Vector{
             Param("a", ty("binding_array",
                           ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u)),
         },
         ty.void_(), Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBindingArrayTest, InvalidFunctionPointerParameterWithHandleType) {
    Func("foo",
         Vector{
             Param("a", ty.ptr<function>(ty(
                            "binding_array",
                            ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_u))),
         },
         ty.void_(), Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: pointer can not be formed to handle type binding_array<texture_2d<f32>, 4>)");
}

TEST_F(ResolverBindingArrayTest, InvalidNoTemplateArg) {
    GlobalVar("a", Binding(0_a), Group(0_a), ty("binding_array"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: expected '<' for 'binding_array')");
}

TEST_F(ResolverBindingArrayTest, InvalidNoTemplateType) {
    GlobalVar("a", Binding(0_a), Group(0_a), ty("binding_array", 4_u));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'binding_array' requires 2 template arguments)");
}

TEST_F(ResolverBindingArrayTest, InvalidNoTemplateCount) {
    GlobalVar("a", Binding(0_a), Group(0_a),
              ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32())));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'binding_array' requires 2 template arguments)");
}

TEST_F(ResolverBindingArrayTest, InvalidCountZero) {
    GlobalVar(
        "a", Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: array count (0) must be greater than 0)");
}

TEST_F(ResolverBindingArrayTest, ValidCountOne) {
    GlobalVar(
        "a", Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 1_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBindingArrayTest, InvalidCountNegative) {
    GlobalVar(
        "a", Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), -1_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: array count (-1) must be greater than 0)");
}

TEST_F(ResolverBindingArrayTest, ValidCountConst) {
    GlobalConst("count", ty.u32(), Call("u32", 8_a));
    GlobalVar("a", Binding(0_a), Group(0_a),
              ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                 "count"));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverBindingArrayTest, InvalidCountOverride) {
    Override("count", ty.u32());
    GlobalVar("a", Binding(0_a), Group(0_a),
              ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()),
                 "count"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: binding_array count must be a constant expression)");
}

TEST_F(ResolverBindingArrayTest, ValidSampledTexture) {
    GlobalVar(
        "a", Binding(0_a), Group(0_a),
        ty("binding_array", ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// TODO(393558555): Support these types in binding_array as well
TEST_F(ResolverBindingArrayTest, InvalidSampler) {
    GlobalVar("a", Binding(0_a), Group(0_a),
              ty("binding_array", ty.sampler(core::type::SamplerKind::kSampler), 4_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: binding_array element type must be a sampled texture type)");
}

TEST_F(ResolverBindingArrayTest, InvalidStorageTexture) {
    GlobalVar("a", Binding(0_a), Group(0_a),
              ty("binding_array",
                 ty.storage_texture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Float,
                                    core::Access::kReadWrite),
                 4_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: binding_array element type must be a sampled texture type)");
}

TEST_F(ResolverBindingArrayTest, InvalidUniformHostShareable) {
    GlobalVar("a", uniform, Binding(0_a), Group(0_a), ty("binding_array", ty.u32(), 4_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: binding_array element type must be a sampled texture type)");
}

TEST_F(ResolverBindingArrayTest, InvalidStorageHostShareable) {
    GlobalVar("a", storage, Binding(0_a), Group(0_a), ty("binding_array", ty.u32(), 4_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: binding_array element type must be a sampled texture type)");
}

TEST_F(ResolverBindingArrayTest, InvalidBindingArray) {
    GlobalVar("a", Binding(0_a), Group(0_a),
              ty("binding_array",
                 ty("binding_array",
                    ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()), 4_a),
                 4_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: binding_array element type must be a sampled texture type)");
}

// How to test as let / const / return since no constructor?

}  // namespace
}  // namespace tint::resolver
