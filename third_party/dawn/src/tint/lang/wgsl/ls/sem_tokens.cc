// Copyright 2024 The Dawn & Tint Authors
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

#include "langsvr/lsp/comparators.h"
#include "langsvr/lsp/lsp.h"
#include "langsvr/lsp/primitives.h"
#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/ast/member_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/struct.h"
#include "src/tint/lang/wgsl/ast/struct_member.h"
#include "src/tint/lang/wgsl/ast/type.h"
#include "src/tint/lang/wgsl/builtin_fn.h"
#include "src/tint/lang/wgsl/ls/sem_token.h"
#include "src/tint/lang/wgsl/ls/server.h"
#include "src/tint/lang/wgsl/ls/utils.h"
#include "src/tint/lang/wgsl/sem/builtin_enum_expression.h"
#include "src/tint/lang/wgsl/sem/function_expression.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/rtti/switch.h"

namespace lsp = langsvr::lsp;

namespace tint::wgsl::ls {

namespace {

/// Token describes a single semantic token, as returned by TextDocumentSemanticTokensFullRequest.
struct Token {
    /// The start position of the token
    lsp::Position position;
    /// The kind of token. Maps to enumerators in SemToken.
    size_t kind = 0;
    /// The length of the token in UTF-8 codepoints.
    size_t length = 0;
};

/// @returns a Token built from the source range @p range with the kind @p kind
Token TokenFromRange(const File& file, const tint::Source::Range& range, SemToken::Kind kind) {
    Token tok;
    tok.position = file.Conv(range.begin);
    tok.length = range.end.column - range.begin.column;
    tok.kind = kind;
    return tok;
}

/// @returns the token kind for the expression @p expr, or nullptr if the expression does not have a
/// token kind.
std::optional<SemToken::Kind> TokenKindFor(const sem::Expression* expr) {
    return Switch<std::optional<SemToken::Kind>>(
        Unwrap(expr),  //
        [](const sem::TypeExpression*) { return SemToken::kType; },
        [](const sem::VariableUser*) { return SemToken::kVariable; },
        [](const sem::FunctionExpression*) { return SemToken::kFunction; },
        [](const sem::BuiltinEnumExpression<wgsl::BuiltinFn>*) { return SemToken::kFunction; },
        [](const sem::BuiltinEnumExpressionBase*) { return SemToken::kEnumMember; },
        [](tint::Default) { return std::nullopt; });
}

/// @returns all the semantic tokens in the file @p file, in sequential order.
std::vector<Token> Tokens(File& file) {
    std::vector<Token> tokens;
    auto& sem = file.program.Sem();
    for (auto* node : file.nodes) {
        Switch(
            node,  //
            [&](const ast::IdentifierExpression* expr) {
                if (auto kind = TokenKindFor(sem.Get(expr))) {
                    tokens.push_back(TokenFromRange(file, expr->identifier->source.range, *kind));
                }
            },
            [&](const ast::Struct* str) {
                tokens.push_back(TokenFromRange(file, str->name->source.range, SemToken::kType));
            },
            [&](const ast::StructMember* member) {
                tokens.push_back(
                    TokenFromRange(file, member->name->source.range, SemToken::kMember));
            },
            [&](const ast::Variable* var) {
                tokens.push_back(
                    TokenFromRange(file, var->name->source.range, SemToken::kVariable));
            },
            [&](const ast::Function* fn) {
                tokens.push_back(TokenFromRange(file, fn->name->source.range, SemToken::kFunction));
            },
            [&](const ast::MemberAccessorExpression* a) {
                tokens.push_back(TokenFromRange(file, a->member->source.range, SemToken::kMember));
            });
    }
    std::sort(tokens.begin(), tokens.end(),
              [](const Token& a, const Token& b) { return a.position < b.position; });

    return tokens;
}

}  // namespace

typename lsp::TextDocumentSemanticTokensFullRequest::ResultType  //
Server::Handle(const lsp::TextDocumentSemanticTokensFullRequest& r) {
    typename lsp::TextDocumentSemanticTokensFullRequest::SuccessType result;

    if (auto file = files_.Get(r.text_document.uri)) {
        lsp::SemanticTokens out;
        // https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#textDocument_semanticTokens
        Token last;

        auto tokens = Tokens(**file);
        for (auto tok : tokens) {
            if (last.position.line != tok.position.line) {
                last.position.character = 0;
            }
            out.data.push_back(tok.position.line - last.position.line);
            out.data.push_back(tok.position.character - last.position.character);
            out.data.push_back(tok.length);
            out.data.push_back(static_cast<langsvr::lsp::Uinteger>(tok.kind));
            out.data.push_back(0);  // modifiers
            last = tok;
        }

        result = out;
    }

    return result;
}

}  // namespace tint::wgsl::ls
