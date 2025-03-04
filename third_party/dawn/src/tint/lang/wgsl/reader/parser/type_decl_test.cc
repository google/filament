// Copyright 2020 The Dawn & Tint Authors
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
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT

TEST_F(WGSLParserTest, TypeDecl_Invalid) {
    auto p = parser("1234");
    auto t = p->type_specifier();
    EXPECT_EQ(t.errored, false);
    EXPECT_EQ(t.matched, false);
    EXPECT_EQ(t.value, nullptr);
    EXPECT_FALSE(p->has_error());
}

TEST_F(WGSLParserTest, TypeDecl_Identifier) {
    auto p = parser("A");

    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ast::CheckIdentifier(t.value, "A");
    EXPECT_EQ(t->expr->source.range, (Source::Range{{1u, 1u}, {1u, 2u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Bool) {
    auto p = parser("bool");

    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ast::CheckIdentifier(t.value, "bool");
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(WGSLParserTest, TypeDecl_F16) {
    auto p = parser("f16");

    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ast::CheckIdentifier(t.value, "f16");
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
}

TEST_F(WGSLParserTest, TypeDecl_F32) {
    auto p = parser("f32");

    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ast::CheckIdentifier(t.value, "f32");
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
}

TEST_F(WGSLParserTest, TypeDecl_I32) {
    auto p = parser("i32");

    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ast::CheckIdentifier(t.value, "i32");
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
}

TEST_F(WGSLParserTest, TypeDecl_U32) {
    auto p = parser("u32");

    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ast::CheckIdentifier(t.value, "u32");
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
}

struct VecData {
    const char* input;
    size_t count;
    Source::Range range;
};
inline std::ostream& operator<<(std::ostream& out, VecData data) {
    out << std::string(data.input);
    return out;
}

class VecTest : public WGSLParserTestWithParam<VecData> {};

TEST_P(VecTest, Parse) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());
    ast::CheckIdentifier(t.value, ast::Template("vec" + std::to_string(params.count), "f32"));
    EXPECT_EQ(t.value->source.range, params.range);
}
INSTANTIATE_TEST_SUITE_P(WGSLParserTest,
                         VecTest,
                         testing::Values(VecData{"vec2<f32>", 2, {{1u, 1u}, {1u, 10u}}},
                                         VecData{"vec3<f32>", 3, {{1u, 1u}, {1u, 10u}}},
                                         VecData{"vec4<f32>", 4, {{1u, 1u}, {1u, 10u}}}));

class VecMissingType : public WGSLParserTestWithParam<VecData> {};

TEST_P(VecMissingType, Handles_Missing_Type) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_specifier();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:6: expected expression for type template argument list");
}
INSTANTIATE_TEST_SUITE_P(WGSLParserTest,
                         VecMissingType,
                         testing::Values(VecData{"vec2<>", 2, {}},
                                         VecData{"vec3<>", 3, {}},
                                         VecData{"vec4<>", 4, {}}));

TEST_F(WGSLParserTest, TypeDecl_Ptr) {
    auto p = parser("ptr<function, f32>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("ptr", "function", "f32"));
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 19u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Ptr_WithAccess) {
    auto p = parser("ptr<function, f32, read>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("ptr", "function", "f32", "read"));
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 25u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Ptr_ToVec) {
    auto p = parser("ptr<function, vec2<f32>>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("ptr", "function", ast::Template("vec2", "f32")));

    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 25}}));
}

TEST_F(WGSLParserTest, TypeDecl_Ptr_MissingCommaAfterAddressSpace) {
    auto p = parser("ptr<function f32>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:14: expected ',' for type template argument list");
}

TEST_F(WGSLParserTest, TypeDecl_Ptr_MissingCommaAfterAccess) {
    auto p = parser("ptr<function, f32 read>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:19: expected ',' for type template argument list");
}

TEST_F(WGSLParserTest, TypeDecl_Ptr_MissingAddressSpace) {
    auto p = parser("ptr<, f32>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), R"(1:5: expected expression for type template argument list)");
}

TEST_F(WGSLParserTest, TypeDecl_Ptr_MissingParams) {
    auto p = parser("ptr<>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), R"(1:5: expected expression for type template argument list)");
}

TEST_F(WGSLParserTest, TypeDecl_Atomic) {
    auto p = parser("atomic<f32>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("atomic", "f32"));
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 12u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Atomic_ToVec) {
    auto p = parser("atomic<vec2<f32>>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("atomic", ast::Template("vec2", "f32")));
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 18u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Atomic_MissingType) {
    auto p = parser("atomic<>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:8: expected expression for type template argument list");
}

TEST_F(WGSLParserTest, TypeDecl_Array_AbstractIntLiteralSize) {
    auto p = parser("array<f32, 5>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("array", "f32", 5_a));
}

TEST_F(WGSLParserTest, TypeDecl_Array_SintLiteralSize) {
    auto p = parser("array<f32, 5i>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("array", "f32", 5_i));
}

TEST_F(WGSLParserTest, TypeDecl_Array_UintLiteralSize) {
    auto p = parser("array<f32, 5u>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("array", "f32", 5_u));
}

TEST_F(WGSLParserTest, TypeDecl_Array_ConstantSize) {
    auto p = parser("array<f32, size>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("array", "f32", "size"));
}

TEST_F(WGSLParserTest, TypeDecl_Array_ExpressionSize) {
    auto p = parser("array<f32, size + 2>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    auto* arr = t->expr->identifier->As<ast::TemplatedIdentifier>();
    EXPECT_EQ(arr->symbol.Name(), "array");
    EXPECT_TRUE(arr->attributes.IsEmpty());

    ASSERT_EQ(arr->arguments.Length(), 2u);

    auto* ty = As<ast::IdentifierExpression>(arr->arguments[0]);
    ASSERT_NE(ty, nullptr);
    EXPECT_EQ(ty->identifier->symbol.Name(), "f32");

    auto* count = As<ast::BinaryExpression>(arr->arguments[1]);
    ASSERT_NE(count, nullptr);
    EXPECT_EQ(core::BinaryOp::kAdd, count->op);

    auto* count_lhs = As<ast::IdentifierExpression>(count->lhs);
    ASSERT_NE(count_lhs, nullptr);
    EXPECT_EQ(count_lhs->identifier->symbol.Name(), "size");

    auto* count_rhs = As<ast::IntLiteralExpression>(count->rhs);
    ASSERT_NE(count_rhs, nullptr);
    EXPECT_EQ(count_rhs->value, static_cast<int64_t>(2));
}

TEST_F(WGSLParserTest, TypeDecl_Array_Runtime) {
    auto p = parser("array<u32>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("array", "u32"));
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 11u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Array_Runtime_Vec) {
    auto p = parser("array<vec4<u32>>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    ast::CheckIdentifier(t.value, ast::Template("array", ast::Template("vec4", "u32")));
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 17u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Array_BadSize) {
    auto p = parser("array<f32, !>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:13: unable to parse right side of ! expression");
}

TEST_F(WGSLParserTest, TypeDecl_Array_MissingComma) {
    auto p = parser("array<f32 3>");
    auto t = p->type_specifier();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:11: expected ',' for type template argument list");
}

struct MatrixData {
    const char* input;
    size_t columns;
    size_t rows;
    Source::Range range;
};
inline std::ostream& operator<<(std::ostream& out, MatrixData data) {
    out << std::string(data.input);
    return out;
}

class MatrixTest : public WGSLParserTestWithParam<MatrixData> {};

TEST_P(MatrixTest, Parse) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();
    ASSERT_FALSE(p->has_error());

    std::string expected_name =
        "mat" + std::to_string(GetParam().columns) + "x" + std::to_string(GetParam().rows);

    ast::CheckIdentifier(t.value, ast::Template(expected_name, "f32"));
    EXPECT_EQ(t.value->source.range, params.range);
}
INSTANTIATE_TEST_SUITE_P(WGSLParserTest,
                         MatrixTest,
                         testing::Values(MatrixData{"mat2x2<f32>", 2, 2, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat2x3<f32>", 2, 3, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat2x4<f32>", 2, 4, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat3x2<f32>", 3, 2, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat3x3<f32>", 3, 3, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat3x4<f32>", 3, 4, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat4x2<f32>", 4, 2, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat4x3<f32>", 4, 3, {{1u, 1u}, {1u, 12u}}},
                                         MatrixData{"mat4x4<f32>", 4, 4, {{1u, 1u}, {1u, 12u}}}));

class MatrixMissingType : public WGSLParserTestWithParam<MatrixData> {};

TEST_P(MatrixMissingType, Handles_Missing_Type) {
    auto params = GetParam();
    auto p = parser(params.input);
    auto t = p->type_specifier();
    EXPECT_TRUE(t.errored);
    EXPECT_FALSE(t.matched);
    ASSERT_EQ(t.value, nullptr);
    ASSERT_TRUE(p->has_error());
    ASSERT_EQ(p->error(), "1:8: expected expression for type template argument list");
}
INSTANTIATE_TEST_SUITE_P(WGSLParserTest,
                         MatrixMissingType,
                         testing::Values(MatrixData{"mat2x2<>", 2, 2, {}},
                                         MatrixData{"mat2x3<>", 2, 3, {}},
                                         MatrixData{"mat2x4<>", 2, 4, {}},
                                         MatrixData{"mat3x2<>", 3, 2, {}},
                                         MatrixData{"mat3x3<>", 3, 3, {}},
                                         MatrixData{"mat3x4<>", 3, 4, {}},
                                         MatrixData{"mat4x2<>", 4, 2, {}},
                                         MatrixData{"mat4x3<>", 4, 3, {}},
                                         MatrixData{"mat4x4<>", 4, 4, {}}));

TEST_F(WGSLParserTest, TypeDecl_Sampler) {
    auto p = parser("sampler");

    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr) << p->error();

    ast::CheckIdentifier(t.value, "sampler");
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 8u}}));
}

TEST_F(WGSLParserTest, TypeDecl_Texture) {
    auto p = parser("texture_cube<f32>");

    auto t = p->type_specifier();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_NE(t.value, nullptr);

    ast::CheckIdentifier(t.value, ast::Template("texture_cube", "f32"));
    EXPECT_EQ(t.value->source.range, (Source::Range{{1u, 1u}, {1u, 18u}}));
}

}  // namespace
}  // namespace tint::wgsl::reader
