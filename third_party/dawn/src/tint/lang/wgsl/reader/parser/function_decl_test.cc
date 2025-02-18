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

#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, FunctionDecl) {
    auto p = parser("fn main(a : i32, b : f32) { return; }");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto f = p->function_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(f.errored);
    EXPECT_TRUE(f.matched);
    ASSERT_NE(f.value, nullptr);

    EXPECT_EQ(f->source.range.begin, (Source::Location{1, 1}));
    EXPECT_EQ(f->source.range.end, (Source::Location{1, 38}));

    EXPECT_EQ(f->name->symbol, p->builder().Symbols().Get("main"));
    EXPECT_EQ(f->return_type, nullptr);

    ASSERT_EQ(f->params.Length(), 2u);
    EXPECT_EQ(f->params[0]->name->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(f->params[1]->name->symbol, p->builder().Symbols().Get("b"));

    EXPECT_EQ(f->return_type, nullptr);

    auto* body = f->body;
    ASSERT_EQ(body->statements.Length(), 1u);
    EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(WGSLParserTest, FunctionDecl_Unicode) {
    const std::string function_ident =  // "𝗳𝘂𝗻𝗰𝘁𝗶𝗼𝗻"
        "\xf0\x9d\x97\xb3\xf0\x9d\x98\x82\xf0\x9d\x97\xbb\xf0\x9d\x97\xb0\xf0\x9d"
        "\x98\x81\xf0\x9d\x97\xb6\xf0\x9d\x97\xbc\xf0\x9d\x97\xbb";

    const std::string param_a_ident =  // "𝓹𝓪𝓻𝓪𝓶_𝓪"
        "\xf0\x9d\x93\xb9\xf0\x9d\x93\xaa\xf0\x9d\x93\xbb\xf0\x9d\x93\xaa\xf0\x9d"
        "\x93\xb6\x5f\xf0\x9d\x93\xaa";

    const std::string param_b_ident =  // "𝕡𝕒𝕣𝕒𝕞_𝕓"
        "\xf0\x9d\x95\xa1\xf0\x9d\x95\x92\xf0\x9d\x95\xa3\xf0\x9d\x95\x92\xf0\x9d"
        "\x95\x9e\x5f\xf0\x9d\x95\x93";

    std::string src = "fn $function($param_a : i32, $param_b : f32) { return; }";
    src = tint::ReplaceAll(src, "$function", function_ident);
    src = tint::ReplaceAll(src, "$param_a", param_a_ident);
    src = tint::ReplaceAll(src, "$param_b", param_b_ident);

    auto p = parser(src);
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto f = p->function_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(f.errored);
    EXPECT_TRUE(f.matched);
    ASSERT_NE(f.value, nullptr);

    EXPECT_EQ(f->source.range.begin, (Source::Location{1, 1}));
    EXPECT_EQ(f->source.range.end, (Source::Location{1, 114}));

    EXPECT_EQ(f->name->symbol, p->builder().Symbols().Get(function_ident));
    EXPECT_EQ(f->return_type, nullptr);

    ASSERT_EQ(f->params.Length(), 2u);
    EXPECT_EQ(f->params[0]->name->symbol, p->builder().Symbols().Get(param_a_ident));
    EXPECT_EQ(f->params[1]->name->symbol, p->builder().Symbols().Get(param_b_ident));

    EXPECT_EQ(f->return_type, nullptr);

    auto* body = f->body;
    ASSERT_EQ(body->statements.Length(), 1u);
    EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(WGSLParserTest, FunctionDecl_AttributeList) {
    auto p = parser("@workgroup_size(2, 3, 4) fn main() { return; }");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attrs.errored);
    ASSERT_TRUE(attrs.matched);
    auto f = p->function_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(f.errored);
    EXPECT_TRUE(f.matched);
    ASSERT_NE(f.value, nullptr);

    EXPECT_EQ(f->source.range.begin, (Source::Location{1, 26}));
    EXPECT_EQ(f->source.range.end, (Source::Location{1, 47}));

    EXPECT_EQ(f->name->symbol, p->builder().Symbols().Get("main"));
    EXPECT_EQ(f->return_type, nullptr);
    ASSERT_EQ(f->params.Length(), 0u);

    auto& attributes = f->attributes;
    ASSERT_EQ(attributes.Length(), 1u);
    ASSERT_TRUE(attributes[0]->Is<ast::WorkgroupAttribute>());

    auto values = attributes[0]->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 2);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 3);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    auto* body = f->body;
    ASSERT_EQ(body->statements.Length(), 1u);
    EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(WGSLParserTest, FunctionDecl_AttributeList_MultipleEntries) {
    auto p = parser(R"(
@workgroup_size(2, 3, 4) @compute
fn main() { return; })");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attrs.errored);
    ASSERT_TRUE(attrs.matched);
    auto f = p->function_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(f.errored);
    EXPECT_TRUE(f.matched);
    ASSERT_NE(f.value, nullptr);

    EXPECT_EQ(f->source.range.begin, (Source::Location{3, 1}));
    EXPECT_EQ(f->source.range.end, (Source::Location{3, 22}));

    EXPECT_EQ(f->name->symbol, p->builder().Symbols().Get("main"));
    EXPECT_EQ(f->return_type, nullptr);
    ASSERT_EQ(f->params.Length(), 0u);

    auto& attributes = f->attributes;
    ASSERT_EQ(attributes.Length(), 2u);

    ASSERT_TRUE(attributes[0]->Is<ast::WorkgroupAttribute>());
    auto values = attributes[0]->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 2);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 3);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(attributes[1]->Is<ast::StageAttribute>());
    EXPECT_EQ(attributes[1]->As<ast::StageAttribute>()->stage, ast::PipelineStage::kCompute);

    auto* body = f->body;
    ASSERT_EQ(body->statements.Length(), 1u);
    EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(WGSLParserTest, FunctionDecl_AttributeList_MultipleLists) {
    auto p = parser(R"(
@workgroup_size(2, 3, 4)
@compute
fn main() { return; })");
    auto attributes = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attributes.errored);
    ASSERT_TRUE(attributes.matched);
    auto f = p->function_decl(attributes.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(f.errored);
    EXPECT_TRUE(f.matched);
    ASSERT_NE(f.value, nullptr);

    EXPECT_EQ(f->source.range.begin, (Source::Location{4, 1}));
    EXPECT_EQ(f->source.range.end, (Source::Location{4, 22}));

    EXPECT_EQ(f->name->symbol, p->builder().Symbols().Get("main"));
    EXPECT_EQ(f->return_type, nullptr);
    ASSERT_EQ(f->params.Length(), 0u);

    auto& attrs = f->attributes;
    ASSERT_EQ(attrs.Length(), 2u);

    ASSERT_TRUE(attrs[0]->Is<ast::WorkgroupAttribute>());
    auto values = attrs[0]->As<ast::WorkgroupAttribute>()->Values();

    ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->value, 2);
    EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->value, 3);
    EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->value, 4);
    EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->suffix,
              ast::IntLiteralExpression::Suffix::kNone);

    ASSERT_TRUE(attrs[1]->Is<ast::StageAttribute>());
    EXPECT_EQ(attrs[1]->As<ast::StageAttribute>()->stage, ast::PipelineStage::kCompute);

    auto* body = f->body;
    ASSERT_EQ(body->statements.Length(), 1u);
    EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(WGSLParserTest, FunctionDecl_ReturnTypeAttributeList) {
    auto p = parser("fn main() -> @location(1) f32 { return 1.0; }");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto f = p->function_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(f.errored);
    EXPECT_TRUE(f.matched);
    ASSERT_NE(f.value, nullptr);

    EXPECT_EQ(f->source.range.begin, (Source::Location{1, 1}));
    EXPECT_EQ(f->source.range.end, (Source::Location{1, 46}));

    EXPECT_EQ(f->name->symbol, p->builder().Symbols().Get("main"));
    ASSERT_NE(f->return_type, nullptr);

    ast::CheckIdentifier(f->return_type, "f32");

    ASSERT_EQ(f->params.Length(), 0u);

    auto& attributes = f->attributes;
    EXPECT_EQ(attributes.Length(), 0u);

    auto& ret_type_attributes = f->return_type_attributes;
    ASSERT_EQ(ret_type_attributes.Length(), 1u);
    auto* loc = ret_type_attributes[0]->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc != nullptr);
    EXPECT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());

    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(1u, exp->value);

    auto* body = f->body;
    ASSERT_EQ(body->statements.Length(), 1u);
    EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(WGSLParserTest, FunctionDecl_MustUse) {
    auto p = parser("@must_use fn main() { return; }");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attrs.errored);
    ASSERT_TRUE(attrs.matched);
    auto f = p->function_decl(attrs.value);
    EXPECT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(f.errored);
    EXPECT_TRUE(f.matched);
    ASSERT_NE(f.value, nullptr);

    EXPECT_EQ(f->source.range.begin, (Source::Location{1, 11}));
    EXPECT_EQ(f->source.range.end, (Source::Location{1, 32}));

    auto& attributes = f->attributes;
    ASSERT_EQ(attributes.Length(), 1u);
    ASSERT_TRUE(attributes[0]->Is<ast::MustUseAttribute>());
}

TEST_F(WGSLParserTest, FunctionDecl_InvalidHeader) {
    auto p = parser("fn main() -> { }");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto f = p->function_decl(attrs.value);
    EXPECT_TRUE(f.errored);
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(f.value, nullptr);
    EXPECT_EQ(p->error(), "1:14: unable to determine function return type");
}

TEST_F(WGSLParserTest, FunctionDecl_InvalidBody) {
    auto p = parser("fn main() { return }");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto f = p->function_decl(attrs.value);
    EXPECT_TRUE(f.errored);
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(f.value, nullptr);
    EXPECT_EQ(p->error(), "1:20: expected ';' for return statement");
}

TEST_F(WGSLParserTest, FunctionDecl_MissingLeftBrace) {
    auto p = parser("fn main() return; }");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    auto f = p->function_decl(attrs.value);
    EXPECT_TRUE(f.errored);
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(f.value, nullptr);
    EXPECT_EQ(p->error(), "1:11: expected '{' for function body");
}

}  // namespace
}  // namespace tint::wgsl::reader
