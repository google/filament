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

#include <string>

#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

#include "gtest/gtest.h"

namespace tint::ast::transform {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

// Inherit from Transform so we have access to protected methods
struct CreateASTTypeForTest : public testing::Test, public Transform {
    ApplyResult Apply(const Program&, const DataMap&, DataMap&) const override {
        return SkipTransform;
    }

    Type create(std::function<core::type::Type*(ProgramBuilder&)> create_sem_type) {
        ProgramBuilder sem_type_builder;
        auto* sem_type = create_sem_type(sem_type_builder);
        Program program = resolver::Resolve(sem_type_builder);
        program::CloneContext ctx(&ast_type_builder, &program, false);
        return CreateASTTypeFor(ctx, sem_type);
    }

    ProgramBuilder ast_type_builder;
};

TEST_F(CreateASTTypeForTest, Basic) {
    auto check = [&](Type ty, const char* expect) { CheckIdentifier(ty->identifier, expect); };

    check(create([](ProgramBuilder& b) { return b.create<core::type::I32>(); }), "i32");
    check(create([](ProgramBuilder& b) { return b.create<core::type::U32>(); }), "u32");
    check(create([](ProgramBuilder& b) { return b.create<core::type::F32>(); }), "f32");
    check(create([](ProgramBuilder& b) { return b.create<core::type::Bool>(); }), "bool");
    EXPECT_EQ(create([](ProgramBuilder& b) { return b.create<core::type::Void>(); }), nullptr);
}

TEST_F(CreateASTTypeForTest, Matrix) {
    auto mat = create([](ProgramBuilder& b) {
        auto* column_type = b.create<core::type::Vector>(b.create<core::type::F32>(), 2u);
        return b.create<core::type::Matrix>(column_type, 3u);
    });

    CheckIdentifier(mat, Template("mat3x2", "f32"));
}

TEST_F(CreateASTTypeForTest, Vector) {
    auto vec = create([](ProgramBuilder& b) {
        return b.create<core::type::Vector>(b.create<core::type::F32>(), 2u);
    });

    CheckIdentifier(vec, Template("vec2", "f32"));
}

TEST_F(CreateASTTypeForTest, ArrayImplicitStride) {
    auto arr = create([](ProgramBuilder& b) {
        return b.create<core::type::Array>(b.create<core::type::F32>(),
                                           b.create<core::type::ConstantArrayCount>(2u), 4u, 4u,
                                           32u, 32u);
    });

    CheckIdentifier(arr, Template("array", "f32", 2_u));
    auto* tmpl_attr = arr->identifier->As<TemplatedIdentifier>();
    ASSERT_NE(tmpl_attr, nullptr);
    EXPECT_TRUE(tmpl_attr->attributes.IsEmpty());
}

TEST_F(CreateASTTypeForTest, ArrayNonImplicitStride) {
    auto arr = create([](ProgramBuilder& b) {
        return b.create<core::type::Array>(b.create<core::type::F32>(),
                                           b.create<core::type::ConstantArrayCount>(2u), 4u, 4u,
                                           64u, 32u);
    });
    CheckIdentifier(arr, Template("array", "f32", 2_u));
    auto* tmpl_attr = arr->identifier->As<TemplatedIdentifier>();
    ASSERT_NE(tmpl_attr, nullptr);
    ASSERT_EQ(tmpl_attr->attributes.Length(), 1u);
    ASSERT_TRUE(tmpl_attr->attributes[0]->Is<StrideAttribute>());
    ASSERT_EQ(tmpl_attr->attributes[0]->As<StrideAttribute>()->stride, 64u);
}

// crbug.com/tint/1764
TEST_F(CreateASTTypeForTest, AliasedArrayWithComplexOverrideLength) {
    // override O = 123;
    // type A = array<i32, O*2>;
    //
    // var<workgroup> W : A;
    //
    ProgramBuilder b;
    auto* arr_len = b.Mul("O", 2_a);
    b.Override("O", b.Expr(123_a));
    auto* alias = b.Alias("A", b.ty.array(b.ty.i32(), arr_len));

    Program program(resolver::Resolve(b));

    auto* arr_ty = program.Sem().Get(alias);

    program::CloneContext ctx(&ast_type_builder, &program, false);
    auto ast_ty = CreateASTTypeFor(ctx, arr_ty);
    CheckIdentifier(ast_ty, "A");
}

TEST_F(CreateASTTypeForTest, Struct) {
    auto str = create([](ProgramBuilder& b) {
        auto* decl = b.Structure("S", {});
        return b.create<sem::Struct>(decl, decl->name->symbol, tint::Empty, 4u /* align */,
                                     4u /* size */, 4u /* size_no_padding */);
    });

    CheckIdentifier(str, "S");
}

TEST_F(CreateASTTypeForTest, PrivatePointer) {
    auto ptr = create([](ProgramBuilder& b) {
        return b.create<core::type::Pointer>(core::AddressSpace::kPrivate,
                                             b.create<core::type::I32>(), core::Access::kReadWrite);
    });

    CheckIdentifier(ptr, Template("ptr", "private", "i32"));
}

TEST_F(CreateASTTypeForTest, StorageReadWritePointer) {
    auto ptr = create([](ProgramBuilder& b) {
        return b.create<core::type::Pointer>(core::AddressSpace::kStorage,
                                             b.create<core::type::I32>(), core::Access::kReadWrite);
    });

    CheckIdentifier(ptr, Template("ptr", "storage", "i32", "read_write"));
}

}  // namespace
}  // namespace tint::ast::transform
