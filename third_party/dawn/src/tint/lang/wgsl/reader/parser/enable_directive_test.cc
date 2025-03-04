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

#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

#include "src/tint/lang/wgsl/ast/enable.h"

namespace tint::wgsl::reader {
namespace {

using EnableDirectiveTest = WGSLParserTest;

// Test a valid enable directive.
TEST_F(EnableDirectiveTest, Single) {
    auto p = parser("enable f16;");
    p->enable_directive();
    EXPECT_FALSE(p->has_error()) << p->error();
    auto program = p->program();
    auto& ast = program.AST();
    ASSERT_EQ(ast.Enables().Length(), 1u);
    auto* enable = ast.Enables()[0];
    EXPECT_EQ(enable->source.range.begin.line, 1u);
    EXPECT_EQ(enable->source.range.begin.column, 1u);
    EXPECT_EQ(enable->source.range.end.line, 1u);
    EXPECT_EQ(enable->source.range.end.column, 12u);
    ASSERT_EQ(enable->extensions.Length(), 1u);
    EXPECT_EQ(enable->extensions[0]->name, wgsl::Extension::kF16);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.line, 1u);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.column, 8u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.line, 1u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.column, 11u);
    ASSERT_EQ(ast.GlobalDeclarations().Length(), 1u);
    EXPECT_EQ(ast.GlobalDeclarations()[0], enable);
}

// Test a valid enable directive.
TEST_F(EnableDirectiveTest, SingleTrailingComma) {
    auto p = parser("enable f16, ;");
    p->enable_directive();
    EXPECT_FALSE(p->has_error()) << p->error();
    auto program = p->program();
    auto& ast = program.AST();
    ASSERT_EQ(ast.Enables().Length(), 1u);
    auto* enable = ast.Enables()[0];
    EXPECT_EQ(enable->source.range.begin.line, 1u);
    EXPECT_EQ(enable->source.range.begin.column, 1u);
    EXPECT_EQ(enable->source.range.end.line, 1u);
    EXPECT_EQ(enable->source.range.end.column, 14u);
    ASSERT_EQ(enable->extensions.Length(), 1u);
    EXPECT_EQ(enable->extensions[0]->name, wgsl::Extension::kF16);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.line, 1u);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.column, 8u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.line, 1u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.column, 11u);
    ASSERT_EQ(ast.GlobalDeclarations().Length(), 1u);
    EXPECT_EQ(ast.GlobalDeclarations()[0], enable);
}

// Test a valid enable directive with multiple extensions.
TEST_F(EnableDirectiveTest, Multiple) {
    auto p = parser("enable f16, chromium_disable_uniformity_analysis, subgroups;");
    p->enable_directive();
    EXPECT_FALSE(p->has_error()) << p->error();
    auto program = p->program();
    auto& ast = program.AST();
    ASSERT_EQ(ast.Enables().Length(), 1u);
    auto* enable = ast.Enables()[0];
    ASSERT_EQ(enable->extensions.Length(), 3u);
    EXPECT_EQ(enable->extensions[0]->name, wgsl::Extension::kF16);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.line, 1u);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.column, 8u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.line, 1u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.column, 11u);
    EXPECT_EQ(enable->extensions[1]->name, wgsl::Extension::kChromiumDisableUniformityAnalysis);
    EXPECT_EQ(enable->extensions[1]->source.range.begin.line, 1u);
    EXPECT_EQ(enable->extensions[1]->source.range.begin.column, 13u);
    EXPECT_EQ(enable->extensions[1]->source.range.end.line, 1u);
    EXPECT_EQ(enable->extensions[1]->source.range.end.column, 49u);
    EXPECT_EQ(enable->extensions[2]->name, wgsl::Extension::kSubgroups);
    EXPECT_EQ(enable->extensions[2]->source.range.begin.line, 1u);
    EXPECT_EQ(enable->extensions[2]->source.range.begin.column, 51u);
    EXPECT_EQ(enable->extensions[2]->source.range.end.line, 1u);
    EXPECT_EQ(enable->extensions[2]->source.range.end.column, 60u);
    ASSERT_EQ(ast.GlobalDeclarations().Length(), 1u);
    EXPECT_EQ(ast.GlobalDeclarations()[0], enable);
}

// Test a valid enable directive with multiple extensions.
TEST_F(EnableDirectiveTest, MultipleTrailingComma) {
    auto p = parser("enable f16, chromium_disable_uniformity_analysis, subgroups,;");
    p->enable_directive();
    EXPECT_FALSE(p->has_error()) << p->error();
    auto program = p->program();
    auto& ast = program.AST();
    ASSERT_EQ(ast.Enables().Length(), 1u);
    auto* enable = ast.Enables()[0];
    ASSERT_EQ(enable->extensions.Length(), 3u);
    EXPECT_EQ(enable->extensions[0]->name, wgsl::Extension::kF16);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.line, 1u);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.column, 8u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.line, 1u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.column, 11u);
    EXPECT_EQ(enable->extensions[1]->name, wgsl::Extension::kChromiumDisableUniformityAnalysis);
    EXPECT_EQ(enable->extensions[1]->source.range.begin.line, 1u);
    EXPECT_EQ(enable->extensions[1]->source.range.begin.column, 13u);
    EXPECT_EQ(enable->extensions[1]->source.range.end.line, 1u);
    EXPECT_EQ(enable->extensions[1]->source.range.end.column, 49u);
    EXPECT_EQ(enable->extensions[2]->name, wgsl::Extension::kSubgroups);
    EXPECT_EQ(enable->extensions[2]->source.range.begin.line, 1u);
    EXPECT_EQ(enable->extensions[2]->source.range.begin.column, 51u);
    EXPECT_EQ(enable->extensions[2]->source.range.end.line, 1u);
    EXPECT_EQ(enable->extensions[2]->source.range.end.column, 60u);
    ASSERT_EQ(ast.GlobalDeclarations().Length(), 1u);
    EXPECT_EQ(ast.GlobalDeclarations()[0], enable);
}

// Test multiple enable directives for a same extension.
TEST_F(EnableDirectiveTest, EnableSameLine) {
    auto p = parser(R"(
enable f16;
enable f16;
)");
    p->translation_unit();
    EXPECT_FALSE(p->has_error()) << p->error();
    auto program = p->program();
    auto& ast = program.AST();
    ASSERT_EQ(ast.Enables().Length(), 2u);
    auto* enable_a = ast.Enables()[0];
    auto* enable_b = ast.Enables()[1];
    ASSERT_EQ(enable_a->extensions.Length(), 1u);
    EXPECT_EQ(enable_a->extensions[0]->name, wgsl::Extension::kF16);
    EXPECT_EQ(enable_a->extensions[0]->source.range.begin.line, 2u);
    EXPECT_EQ(enable_a->extensions[0]->source.range.begin.column, 8u);
    EXPECT_EQ(enable_a->extensions[0]->source.range.end.line, 2u);
    EXPECT_EQ(enable_a->extensions[0]->source.range.end.column, 11u);
    ASSERT_EQ(enable_b->extensions.Length(), 1u);
    EXPECT_EQ(enable_b->extensions[0]->name, wgsl::Extension::kF16);
    EXPECT_EQ(enable_b->extensions[0]->source.range.begin.line, 3u);
    EXPECT_EQ(enable_b->extensions[0]->source.range.begin.column, 8u);
    EXPECT_EQ(enable_b->extensions[0]->source.range.end.line, 3u);
    EXPECT_EQ(enable_b->extensions[0]->source.range.end.column, 11u);
    ASSERT_EQ(ast.GlobalDeclarations().Length(), 2u);
    EXPECT_EQ(ast.GlobalDeclarations()[0], enable_a);
    EXPECT_EQ(ast.GlobalDeclarations()[1], enable_b);
}

// Test an unknown extension identifier.
TEST_F(EnableDirectiveTest, InvalidExtension) {
    auto p = parser("enable NotAValidExtensionName;");
    p->enable_directive();
    // Error when unknown extension found
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:8: expected extension
Possible values: 'clip_distances', 'dual_source_blending', 'f16', 'subgroups', 'subgroups_f16')");
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.Enables().Length(), 0u);
    EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
}

TEST_F(EnableDirectiveTest, InvalidExtensionSuggest) {
    auto p = parser("enable f15;");
    p->enable_directive();
    // Error when unknown extension found
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:8: expected extension
Did you mean 'f16'?
Possible values: 'clip_distances', 'dual_source_blending', 'f16', 'subgroups', 'subgroups_f16')");
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.Enables().Length(), 0u);
    EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
}

// Test an unknown extension identifier, starting with 'chromium'
TEST_F(EnableDirectiveTest, InvalidChromiumExtension) {
    auto p = parser("enable chromium_blah;");
    p->enable_directive();
    // Error when unknown extension found
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:8: expected extension
Possible values: 'chromium_disable_uniformity_analysis', 'chromium_experimental_framebuffer_fetch', 'chromium_experimental_pixel_local', 'chromium_experimental_push_constant', 'chromium_experimental_subgroup_matrix', 'chromium_internal_graphite', 'chromium_internal_input_attachments', 'chromium_internal_relaxed_uniform_layout', 'clip_distances', 'dual_source_blending', 'f16', 'subgroups', 'subgroups_f16')");
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.Enables().Length(), 0u);
    EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
}

// Test an enable directive missing ending semicolon.
TEST_F(EnableDirectiveTest, MissingEndingSemicolon) {
    auto p = parser("enable f16");
    p->translation_unit();
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:11: expected ';' for enable directive");
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.Enables().Length(), 0u);
    EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
}

// Test the special error message when enable are used with parenthesis.
TEST_F(EnableDirectiveTest, ParenthesisSpecialCase) {
    auto p = parser("enable(f16);");
    p->translation_unit();
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:7: enable directives don't take parenthesis");
    auto program = p->program();
    auto& ast = program.AST();
    EXPECT_EQ(ast.Enables().Length(), 0u);
    EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
}

// Test using invalid tokens in an enable directive.
TEST_F(EnableDirectiveTest, InvalidTokens) {
    {
        auto p = parser("enable f16<;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), "1:11: expected ';' for enable directive");
        auto program = p->program();
        auto& ast = program.AST();
        EXPECT_EQ(ast.Enables().Length(), 0u);
        EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
    }
    {
        auto p = parser("enable <f16;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), R"(1:8: expected extension
Possible values: 'clip_distances', 'dual_source_blending', 'f16', 'subgroups', 'subgroups_f16')");
        auto program = p->program();
        auto& ast = program.AST();
        EXPECT_EQ(ast.Enables().Length(), 0u);
        EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
    }
    {
        auto p = parser("enable =;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), R"(1:8: expected extension
Possible values: 'clip_distances', 'dual_source_blending', 'f16', 'subgroups', 'subgroups_f16')");
        auto program = p->program();
        auto& ast = program.AST();
        EXPECT_EQ(ast.Enables().Length(), 0u);
        EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
    }
    {
        auto p = parser("enable vec4;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), R"(1:8: expected extension
Did you mean 'f16'?
Possible values: 'clip_distances', 'dual_source_blending', 'f16', 'subgroups', 'subgroups_f16')");
        auto program = p->program();
        auto& ast = program.AST();
        EXPECT_EQ(ast.Enables().Length(), 0u);
        EXPECT_EQ(ast.GlobalDeclarations().Length(), 0u);
    }
}

// Test an enable directive go after other global declarations.
TEST_F(EnableDirectiveTest, FollowingOtherGlobalDecl) {
    auto p = parser(R"(
var<private> t: f32 = 0f;
enable f16;
)");
    p->translation_unit();
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "3:1: directives must come before all global declarations");
    auto program = p->program();
    auto& ast = program.AST();
    // Accept the enable directive although it caused an error
    ASSERT_EQ(ast.Enables().Length(), 1u);
    auto* enable = ast.Enables()[0];
    ASSERT_EQ(enable->extensions.Length(), 1u);
    EXPECT_EQ(enable->extensions[0]->name, wgsl::Extension::kF16);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.line, 3u);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.column, 8u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.line, 3u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.column, 11u);
    ASSERT_EQ(ast.GlobalDeclarations().Length(), 2u);
    EXPECT_EQ(ast.GlobalDeclarations()[1], enable);
}

// Test an enable directive go after an empty semicolon.
TEST_F(EnableDirectiveTest, FollowingEmptySemicolon) {
    auto p = parser(R"(
;
enable f16;
)");
    p->translation_unit();
    // An empty semicolon is treated as a global declaration
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "3:1: directives must come before all global declarations");
    auto program = p->program();
    auto& ast = program.AST();
    // Accept the enable directive although it cause an error
    ASSERT_EQ(ast.Enables().Length(), 1u);
    auto* enable = ast.Enables()[0];
    ASSERT_EQ(enable->extensions.Length(), 1u);
    EXPECT_EQ(enable->extensions[0]->name, wgsl::Extension::kF16);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.line, 3u);
    EXPECT_EQ(enable->extensions[0]->source.range.begin.column, 8u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.line, 3u);
    EXPECT_EQ(enable->extensions[0]->source.range.end.column, 11u);
    ASSERT_EQ(ast.GlobalDeclarations().Length(), 1u);
    EXPECT_EQ(ast.GlobalDeclarations()[0], enable);
}

}  // namespace
}  // namespace tint::wgsl::reader
