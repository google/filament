// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/reader/parser/classify_template_args.h"

#include <vector>

#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/ice/ice.h"

namespace tint::wgsl::reader {

namespace {

/// If the token at index @p idx is a '>>', '>=' or '>>=', then the token is split into two, with
/// the first being '>', otherwise MaybeSplit() will be a no-op.
/// @param tokens the vector of tokens
/// @param idx the index of the token to (maybe) split
void MaybeSplit(std::vector<Token>& tokens, size_t idx) {
    auto* cur = &tokens[idx];
    auto* next = &tokens[idx + 1];
    TINT_ASSERT(cur);
    TINT_ASSERT(next);
    switch (cur->type()) {
        case Token::Type::kShiftRight:  //  '>>'
            TINT_ASSERT(next->type() == Token::Type::kPlaceholder);
            cur->SetType(Token::Type::kGreaterThan);
            next->SetType(Token::Type::kGreaterThan);
            break;
        case Token::Type::kGreaterThanEqual:  //  '>='
            TINT_ASSERT(next->type() == Token::Type::kPlaceholder);
            cur->SetType(Token::Type::kGreaterThan);
            next->SetType(Token::Type::kEqual);
            break;
        case Token::Type::kShiftRightEqual:  // '>>='
            TINT_ASSERT(next->type() == Token::Type::kPlaceholder);
            cur->SetType(Token::Type::kGreaterThan);
            next->SetType(Token::Type::kGreaterThanEqual);
            break;
        default:
            break;
    }
}

}  // namespace

void ClassifyTemplateArguments(std::vector<Token>& tokens) {
    const size_t count = tokens.size();

    // The current expression nesting depth.
    // Each '(', '[' increments the depth.
    // Each ')', ']' decrements the depth.
    uint64_t expr_depth = 0;

    // A stack of '<' tokens.
    // Used to pair '<' and '>' tokens at the same expression depth.
    struct StackEntry {
        Token* token;         // A pointer to the opening '<' token
        uint64_t expr_depth;  // The value of 'expr_depth' for the opening '<'
    };
    Vector<StackEntry, 16> stack;

    for (size_t i = 0; i < count - 1; i++) {
        switch (tokens[i].type()) {
            case Token::Type::kIdentifier:
            case Token::Type::kVar: {
                auto& next = tokens[i + 1];
                if (next.type() == Token::Type::kLessThan) {
                    // ident '<'
                    // Push this '<' to the stack, along with the current nesting expr_depth.
                    stack.Push(StackEntry{&tokens[i + 1], expr_depth});
                    i++;  // Skip the '<'
                }
                break;
            }
            case Token::Type::kGreaterThan:       // '>'
            case Token::Type::kShiftRight:        // '>>'
            case Token::Type::kGreaterThanEqual:  // '>='
            case Token::Type::kShiftRightEqual:   // '>>='
                if (!stack.IsEmpty() && stack.Back().expr_depth == expr_depth) {
                    // '<' and '>' at same expr_depth, and no terminating tokens in-between.
                    // Consider both as a template argument list.
                    MaybeSplit(tokens, i);
                    stack.Pop().token->SetType(Token::Type::kTemplateArgsLeft);
                    tokens[i].SetType(Token::Type::kTemplateArgsRight);
                }
                break;

            case Token::Type::kParenLeft:    // '('
            case Token::Type::kBracketLeft:  // '['
                // Entering a nested expression
                expr_depth++;
                break;

            case Token::Type::kParenRight:    // ')'
            case Token::Type::kBracketRight:  // ']'
                // Exiting a nested expression
                // Pop the stack until we return to the current expression expr_depth
                while (!stack.IsEmpty() && stack.Back().expr_depth == expr_depth) {
                    stack.Pop();
                }
                if (expr_depth > 0) {
                    expr_depth--;
                }
                break;

            case Token::Type::kSemicolon:  // ';'
            case Token::Type::kBraceLeft:  // '{'
            case Token::Type::kEqual:      // '='
            case Token::Type::kColon:      // ':'
                // Expression terminating tokens. No opening template list can hold these tokens, so
                // clear the stack and expression depth.
                expr_depth = 0;
                stack.Clear();
                break;

            case Token::Type::kOrOr:    // '||'
            case Token::Type::kAndAnd:  // '&&'
                // Treat 'a < b || c > d' as a logical binary operator of two comparison operators
                // instead of a single template argument 'b||c'.
                // Use parentheses around 'b||c' to parse as a template argument list.
                while (!stack.IsEmpty() && stack.Back().expr_depth == expr_depth) {
                    stack.Pop();
                }
                break;

            default:
                break;
        }
    }
}

}  // namespace tint::wgsl::reader
