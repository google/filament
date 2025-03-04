// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/helpers/append_vector.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"

#include "gmock/gmock.h"

namespace tint::wgsl {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class AppendVectorTest : public ::testing::Test, public ProgramBuilder {};

// AppendVector(vec2<i32>(1, 2), 3) -> vec3<i32>(1, 2, 3)
TEST_F(AppendVectorTest, Vec2i32_i32) {
    auto* scalar_1 = Expr(1_i);
    auto* scalar_2 = Expr(2_i);
    auto* scalar_3 = Expr(3_i);
    auto* vec_12 = Call<vec2<i32>>(scalar_1, scalar_2);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.Length(), 3u);
    EXPECT_EQ(vec_123->args[0], scalar_1);
    EXPECT_EQ(vec_123->args[1], scalar_2);
    EXPECT_EQ(vec_123->args[2], scalar_3);

    auto* call = Sem().Get<sem::Call>(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 3u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::I32>());
}

// AppendVector(vec2<i32>(1, 2), 3u) -> vec3<i32>(1, 2, i32(3u))
TEST_F(AppendVectorTest, Vec2i32_u32) {
    auto* scalar_1 = Expr(1_i);
    auto* scalar_2 = Expr(2_i);
    auto* scalar_3 = Expr(3_u);
    auto* vec_12 = Call<vec2<i32>>(scalar_1, scalar_2);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.Length(), 3u);
    EXPECT_EQ(vec_123->args[0], scalar_1);
    EXPECT_EQ(vec_123->args[1], scalar_2);
    auto* u32_to_i32 = vec_123->args[2]->As<ast::CallExpression>();
    ASSERT_NE(u32_to_i32, nullptr);
    ast::CheckIdentifier(u32_to_i32->target, "i32");

    ASSERT_EQ(u32_to_i32->args.Length(), 1u);
    EXPECT_EQ(u32_to_i32->args[0], scalar_3);

    auto* call = Sem().Get<sem::Call>(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 3u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(u32_to_i32));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::I32>());
}

// AppendVector(vec2<i32>(vec2<u32>(1u, 2u)), 3u) ->
//    vec3<i32>(vec2<i32>(vec2<u32>(1u, 2u)), i32(3u))
TEST_F(AppendVectorTest, Vec2i32FromVec2u32_u32) {
    auto* scalar_1 = Expr(1_u);
    auto* scalar_2 = Expr(2_u);
    auto* scalar_3 = Expr(3_u);
    auto* uvec_12 = Call<vec2<u32>>(scalar_1, scalar_2);
    auto* vec_12 = Call<vec2<i32>>(uvec_12);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.Length(), 2u);
    auto* v2u32_to_v2i32 = vec_123->args[0]->As<ast::CallExpression>();
    ASSERT_NE(v2u32_to_v2i32, nullptr);

    ast::CheckIdentifier(v2u32_to_v2i32->target, ast::Template("vec2", "i32"));
    EXPECT_EQ(v2u32_to_v2i32->args.Length(), 1u);
    EXPECT_EQ(v2u32_to_v2i32->args[0], uvec_12);

    auto* u32_to_i32 = vec_123->args[1]->As<ast::CallExpression>();
    ASSERT_NE(u32_to_i32, nullptr);
    ast::CheckIdentifier(u32_to_i32->target, "i32");
    ASSERT_EQ(u32_to_i32->args.Length(), 1u);
    EXPECT_EQ(u32_to_i32->args[0], scalar_3);

    auto* call = Sem().Get<sem::Call>(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(u32_to_i32));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);

    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
}

// AppendVector(vec2<i32>(1, 2), 3.0f) -> vec3<i32>(1, 2, i32(3.0f))
TEST_F(AppendVectorTest, Vec2i32_f32) {
    auto* scalar_1 = Expr(1_i);
    auto* scalar_2 = Expr(2_i);
    auto* scalar_3 = Expr(3_f);
    auto* vec_12 = Call<vec2<i32>>(scalar_1, scalar_2);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.Length(), 3u);
    EXPECT_EQ(vec_123->args[0], scalar_1);
    EXPECT_EQ(vec_123->args[1], scalar_2);
    auto* f32_to_i32 = vec_123->args[2]->As<ast::CallExpression>();
    ASSERT_NE(f32_to_i32, nullptr);
    ast::CheckIdentifier(f32_to_i32->target, "i32");
    ASSERT_EQ(f32_to_i32->args.Length(), 1u);
    EXPECT_EQ(f32_to_i32->args[0], scalar_3);

    auto* call = Sem().Get<sem::Call>(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 3u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(f32_to_i32));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::I32>());
}

// AppendVector(vec3<i32>(1, 2, 3), 4) -> vec4<i32>(1, 2, 3, 4)
TEST_F(AppendVectorTest, Vec3i32_i32) {
    auto* scalar_1 = Expr(1_i);
    auto* scalar_2 = Expr(2_i);
    auto* scalar_3 = Expr(3_i);
    auto* scalar_4 = Expr(4_i);
    auto* vec_123 = Call<vec3<i32>>(scalar_1, scalar_2, scalar_3);
    WrapInFunction(vec_123, scalar_4);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_123, scalar_4);

    auto* vec_1234 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_1234, nullptr);
    ASSERT_EQ(vec_1234->args.Length(), 4u);
    EXPECT_EQ(vec_1234->args[0], scalar_1);
    EXPECT_EQ(vec_1234->args[1], scalar_2);
    EXPECT_EQ(vec_1234->args[2], scalar_3);
    EXPECT_EQ(vec_1234->args[3], scalar_4);

    auto* call = Sem().Get<sem::Call>(vec_1234);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 4u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(scalar_3));
    EXPECT_EQ(call->Arguments()[3], Sem().Get(scalar_4));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 4u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 4u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[3]->Type()->Is<core::type::I32>());
}

// AppendVector(vec_12, 3) -> vec3<i32>(vec_12, 3)
TEST_F(AppendVectorTest, Vec2i32Var_i32) {
    GlobalVar("vec_12", ty.vec2<i32>(), core::AddressSpace::kPrivate);
    auto* vec_12 = Expr("vec_12");
    auto* scalar_3 = Expr(3_i);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.Length(), 2u);
    EXPECT_EQ(vec_123->args[0], vec_12);
    EXPECT_EQ(vec_123->args[1], scalar_3);

    auto* call = Sem().Get<sem::Call>(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
}

// AppendVector(1, 2, scalar_3) -> vec3<i32>(1, 2, scalar_3)
TEST_F(AppendVectorTest, Vec2i32_i32Var) {
    GlobalVar("scalar_3", ty.i32(), core::AddressSpace::kPrivate);
    auto* scalar_1 = Expr(1_i);
    auto* scalar_2 = Expr(2_i);
    auto* scalar_3 = Expr("scalar_3");
    auto* vec_12 = Call<vec2<i32>>(scalar_1, scalar_2);
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.Length(), 3u);
    EXPECT_EQ(vec_123->args[0], scalar_1);
    EXPECT_EQ(vec_123->args[1], scalar_2);
    EXPECT_EQ(vec_123->args[2], scalar_3);

    auto* call = Sem().Get<sem::Call>(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 3u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(scalar_1));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_2));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 3u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::I32>());
}

// AppendVector(vec_12, scalar_3) -> vec3<i32>(vec_12, scalar_3)
TEST_F(AppendVectorTest, Vec2i32Var_i32Var) {
    GlobalVar("vec_12", ty.vec2<i32>(), core::AddressSpace::kPrivate);
    GlobalVar("scalar_3", ty.i32(), core::AddressSpace::kPrivate);
    auto* vec_12 = Expr("vec_12");
    auto* scalar_3 = Expr("scalar_3");
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.Length(), 2u);
    EXPECT_EQ(vec_123->args[0], vec_12);
    EXPECT_EQ(vec_123->args[1], scalar_3);

    auto* call = Sem().Get<sem::Call>(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
}

// AppendVector(vec_12, scalar_3) -> vec3<i32>(vec_12, i32(scalar_3))
TEST_F(AppendVectorTest, Vec2i32Var_f32Var) {
    GlobalVar("vec_12", ty.vec2<i32>(), core::AddressSpace::kPrivate);
    GlobalVar("scalar_3", ty.f32(), core::AddressSpace::kPrivate);
    auto* vec_12 = Expr("vec_12");
    auto* scalar_3 = Expr("scalar_3");
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.Length(), 2u);
    EXPECT_EQ(vec_123->args[0], vec_12);
    auto* f32_to_i32 = vec_123->args[1]->As<ast::CallExpression>();
    ASSERT_NE(f32_to_i32, nullptr);
    ast::CheckIdentifier(f32_to_i32->target, "i32");
    ASSERT_EQ(f32_to_i32->args.Length(), 1u);
    EXPECT_EQ(f32_to_i32->args[0], scalar_3);

    auto* call = Sem().Get<sem::Call>(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(f32_to_i32));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
}

// AppendVector(vec_12, scalar_3) -> vec3<bool>(vec_12, scalar_3)
TEST_F(AppendVectorTest, Vec2boolVar_boolVar) {
    GlobalVar("vec_12", ty.vec2<bool>(), core::AddressSpace::kPrivate);
    GlobalVar("scalar_3", ty.bool_(), core::AddressSpace::kPrivate);
    auto* vec_12 = Expr("vec_12");
    auto* scalar_3 = Expr("scalar_3");
    WrapInFunction(vec_12, scalar_3);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec_12, scalar_3);

    auto* vec_123 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_123, nullptr);
    ASSERT_EQ(vec_123->args.Length(), 2u);
    EXPECT_EQ(vec_123->args[0], vec_12);
    EXPECT_EQ(vec_123->args[1], scalar_3);

    auto* call = Sem().Get<sem::Call>(vec_123);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 2u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_12));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(scalar_3));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 3u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::Bool>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 2u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::Vector>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::Bool>());
}

// AppendVector(vec3<i32>(), 4) -> vec3<bool>(0, 0, 0, 4)
TEST_F(AppendVectorTest, ZeroVec3i32_i32) {
    auto* scalar = Expr(4_i);
    auto* vec000 = Call<vec3<i32>>();
    WrapInFunction(vec000, scalar);

    resolver::Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_TRUE(resolver.Resolve());
    ASSERT_THAT(resolver.Diagnostics(), testing::IsEmpty());

    auto* append = AppendVector(this, vec000, scalar);

    auto* vec_0004 = As<ast::CallExpression>(append->Declaration());
    ASSERT_NE(vec_0004, nullptr);
    ASSERT_EQ(vec_0004->args.Length(), 4u);
    for (size_t i = 0; i < 3; i++) {
        auto* literal = As<ast::IntLiteralExpression>(vec_0004->args[i]);
        ASSERT_NE(literal, nullptr);
        EXPECT_EQ(literal->value, 0);
    }
    EXPECT_EQ(vec_0004->args[3], scalar);

    auto* call = Sem().Get<sem::Call>(vec_0004);
    ASSERT_NE(call, nullptr);
    ASSERT_EQ(call->Arguments().Length(), 4u);
    EXPECT_EQ(call->Arguments()[0], Sem().Get(vec_0004->args[0]));
    EXPECT_EQ(call->Arguments()[1], Sem().Get(vec_0004->args[1]));
    EXPECT_EQ(call->Arguments()[2], Sem().Get(vec_0004->args[2]));
    EXPECT_EQ(call->Arguments()[3], Sem().Get(scalar));

    auto* ctor = call->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(ctor, nullptr);
    ASSERT_TRUE(ctor->ReturnType()->Is<core::type::Vector>());
    EXPECT_EQ(ctor->ReturnType()->As<core::type::Vector>()->Width(), 4u);
    EXPECT_TRUE(ctor->ReturnType()->As<core::type::Vector>()->Type()->Is<core::type::I32>());
    EXPECT_EQ(ctor->ReturnType(), call->Type());

    ASSERT_EQ(ctor->Parameters().Length(), 4u);
    EXPECT_TRUE(ctor->Parameters()[0]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[1]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[2]->Type()->Is<core::type::I32>());
    EXPECT_TRUE(ctor->Parameters()[3]->Type()->Is<core::type::I32>());
}

}  // namespace
}  // namespace tint::wgsl
