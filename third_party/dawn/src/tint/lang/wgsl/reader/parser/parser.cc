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

#include "src/tint/lang/wgsl/reader/parser/parser.h"

#include <limits>
#include <utility>

#include "src/tint/lang/core/attribute.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/break_if_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/increment_decrement_statement.h"
#include "src/tint/lang/wgsl/ast/input_attachment_index_attribute.h"
#include "src/tint/lang/wgsl/ast/invariant_attribute.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/reader/parser/classify_template_args.h"
#include "src/tint/lang/wgsl/reader/parser/lexer.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::wgsl::reader {
namespace {

using Void = Parser::Void;

/// An instance of Void that can be used to signal success for functions that return Expect<Void> or
/// Maybe<NoError>.
static constexpr Void kSuccess;

template <typename T>
using Expect = Parser::Expect<T>;

template <typename T>
using Maybe = Parser::Maybe<T>;

/// Controls the maximum number of times we'll call into the sync() and
/// unary_expression() functions from themselves. This is to guard against stack
/// overflow when there is an excessive number of blocks.
constexpr uint32_t kMaxParseDepth = 128;

/// The maximum number of tokens to look ahead to try and sync the
/// parser on error.
constexpr size_t const kMaxResynchronizeLookahead = 32;

// https://gpuweb.github.io/gpuweb/wgsl.html#reserved-keywords
//
// Must be called with an identifier token.
bool is_reserved(const Token& t) {
    auto s = t.to_str_view();
    return s == "NULL" || s == "Self" || s == "abstract" || s == "active" || s == "alignas" ||
           s == "alignof" || s == "as" || s == "asm" || s == "asm_fragment" || s == "async" ||
           s == "attribute" || s == "auto" || s == "await" || s == "become" ||
           s == "binding_array" || s == "cast" || s == "catch" || s == "class" || s == "co_await" ||
           s == "co_return" || s == "co_yield" || s == "coherent" || s == "column_major" ||
           s == "common" || s == "compile" || s == "compile_fragment" || s == "concept" ||
           s == "const_cast" || s == "consteval" || s == "constexpr" || s == "constinit" ||
           s == "crate" || s == "debugger" || s == "decltype" || s == "delete" || s == "demote" ||
           s == "demote_to_helper" || s == "do" || s == "dynamic_cast" || s == "enum" ||
           s == "explicit" || s == "export" || s == "extends" || s == "extern" || s == "external" ||
           s == "filter" || s == "final" || s == "finally" || s == "friend" || s == "from" ||
           s == "fxgroup" || s == "get" || s == "goto" || s == "groupshared" || s == "highp" ||
           s == "impl" || s == "implements" || s == "import" || s == "inline" ||
           s == "instanceof" || s == "interface" || s == "layout" || s == "lowp" || s == "macro" ||
           s == "macro_rules" || s == "match" || s == "mediump" || s == "meta" || s == "mod" ||
           s == "module" || s == "move" || s == "mut" || s == "mutable" || s == "namespace" ||
           s == "new" || s == "nil" || s == "noexcept" || s == "noinline" ||
           s == "nointerpolation" || s == "non_coherent" || s == "noncoherent" ||
           s == "noperspective" || s == "null" || s == "nullptr" || s == "of" || s == "operator" ||
           s == "package" || s == "packoffset" || s == "partition" || s == "pass" || s == "patch" ||
           s == "pixelfragment" || s == "precise" || s == "precision" || s == "premerge" ||
           s == "priv" || s == "protected" || s == "pub" || s == "public" || s == "readonly" ||
           s == "ref" || s == "regardless" || s == "register" || s == "reinterpret_cast" ||
           s == "require" || s == "resource" || s == "restrict" || s == "self" || s == "set" ||
           s == "shared" || s == "sizeof" || s == "smooth" || s == "snorm" || s == "static" ||
           s == "static_assert" || s == "static_cast" || s == "std" || s == "subroutine" ||
           s == "super" || s == "target" || s == "template" || s == "this" || s == "thread_local" ||
           s == "throw" || s == "trait" || s == "try" || s == "type" || s == "typedef" ||
           s == "typeid" || s == "typename" || s == "typeof" || s == "union" || s == "unless" ||
           s == "unorm" || s == "unsafe" || s == "unsized" || s == "use" || s == "using" ||
           s == "varying" || s == "virtual" || s == "volatile" || s == "wgsl" || s == "where" ||
           s == "with" || s == "writeonly" || s == "yield";
}

/// Enter-exit counters for block token types.
/// Used by sync_to() to skip over closing block tokens that were opened during
/// the forward scan.
struct BlockCounters {
    int brace = 0;    // {   }
    int bracket = 0;  // [   ]
    int paren = 0;    // (   )

    /// @return the current enter-exit depth for the given block token type. If
    /// `t` is not a block token type, then 0 is always returned.
    int consume(const Token& t) {
        if (t.Is(Token::Type::kBraceLeft)) {
            return brace++;
        }
        if (t.Is(Token::Type::kBraceRight)) {
            return brace--;
        }
        if (t.Is(Token::Type::kBracketLeft)) {
            return bracket++;
        }
        if (t.Is(Token::Type::kBracketRight)) {
            return bracket--;
        }
        if (t.Is(Token::Type::kParenLeft)) {
            return paren++;
        }
        if (t.Is(Token::Type::kParenRight)) {
            return paren--;
        }
        return 0;
    }
};
}  // namespace

/// RAII helper that combines a Source on construction with the last token's
/// source when implicitly converted to `Source`.
class Parser::MultiTokenSource {
  public:
    /// Constructor that starts with Source at the current peek position
    /// @param parser the parser
    explicit MultiTokenSource(Parser* parser)
        : MultiTokenSource(parser, parser->peek().source().Begin()) {}

    /// Constructor that starts with the input `start` Source
    /// @param parser the parser
    /// @param start the start source of the range
    MultiTokenSource(Parser* parser, const tint::Source& start) : parser_(parser), start_(start) {}

    /// @returns the Source that returns the combined source from start to the current last token's
    /// source.
    tint::Source operator()() const { return *this; }

    /// @returns the Source that returns the combined source from start to the current last token's
    /// source.
    operator tint::Source() const {
        auto end = parser_->last_source().End();
        if (end < start_) {
            end = start_;
        }
        return Source::Combine(start_, end);
    }

  private:
    Parser* parser_;
    tint::Source start_;
};

Parser::TypedIdentifier::TypedIdentifier() = default;

Parser::TypedIdentifier::TypedIdentifier(const TypedIdentifier&) = default;

Parser::TypedIdentifier::TypedIdentifier(ast::Type type_in, const ast::Identifier* name_in)
    : type(type_in), name(name_in) {}

Parser::TypedIdentifier::~TypedIdentifier() = default;

Parser::FunctionHeader::FunctionHeader() = default;

Parser::FunctionHeader::FunctionHeader(const FunctionHeader&) = default;

Parser::FunctionHeader::FunctionHeader(Source src,
                                       const ast::Identifier* n,
                                       VectorRef<const ast::Parameter*> p,
                                       ast::Type ret_ty,
                                       VectorRef<const ast::Attribute*> ret_attrs)
    : source(src),
      name(n),
      params(std::move(p)),
      return_type(ret_ty),
      return_type_attributes(std::move(ret_attrs)) {}

Parser::FunctionHeader::~FunctionHeader() = default;

Parser::FunctionHeader& Parser::FunctionHeader::operator=(const FunctionHeader& rhs) = default;

Parser::Parser(Source::File const* file) : file_(file) {}

Parser::~Parser() = default;

Parser::Failure::Errored Parser::AddError(const Source& source,
                                          std::string_view err,
                                          std::string_view use) {
    if (silence_diags_ == 0) {
        StringStream msg;
        msg << err;
        if (!use.empty()) {
            msg << " for " << use;
        }
        AddError(source, msg.str());
    }
    return Failure::kErrored;
}

Parser::Failure::Errored Parser::AddError(const Token& t, std::string_view err) {
    AddError(t.source(), err);
    return Failure::kErrored;
}

Parser::Failure::Errored Parser::AddError(const Source& source, std::string_view err) {
    if (silence_diags_ == 0) {
        builder_.Diagnostics().AddError(source) << err;
    }
    return Failure::kErrored;
}

Parser::Failure::Errored Parser::AddError(const Source& source, StyledText&& err) {
    if (silence_diags_ == 0) {
        builder_.Diagnostics().AddError(source) << std::move(err);
    }
    return Failure::kErrored;
}

void Parser::AddNote(const Source& source, std::string_view err) {
    if (silence_diags_ == 0) {
        builder_.Diagnostics().AddNote(source) << err;
    }
}

void Parser::deprecated(const Source& source, std::string_view msg) {
    if (silence_diags_ == 0) {
        builder_.Diagnostics().AddWarning(source) << "use of deprecated language feature: " << msg;
    }
}

const Token& Parser::next() {
    // If the next token is already an error or the end of file, stay there.
    if (tokens_[next_token_idx_].IsEof() || tokens_[next_token_idx_].IsError()) {
        return tokens_[next_token_idx_];
    }

    // Skip over any placeholder elements
    while (true) {
        if (!tokens_[next_token_idx_].IsPlaceholder()) {
            break;
        }
        next_token_idx_++;
    }
    last_source_idx_ = next_token_idx_;

    if (!tokens_[next_token_idx_].IsEof() && !tokens_[next_token_idx_].IsError()) {
        next_token_idx_++;
    }
    return tokens_[last_source_idx_];
}

const Token& Parser::peek(size_t count) {
    for (size_t idx = next_token_idx_; idx < tokens_.size(); idx++) {
        if (tokens_[idx].IsPlaceholder()) {
            continue;
        }
        if (count == 0) {
            return tokens_[idx];
        }
        count--;
    }
    // Walked off the end of the token list, return last token.
    return tokens_[tokens_.size() - 1];
}

bool Parser::peek_is(Token::Type tok, size_t idx) {
    return peek(idx).Is(tok);
}

void Parser::split_token(Token::Type lhs, Token::Type rhs) {
    if (DAWN_UNLIKELY(next_token_idx_ == 0)) {
        TINT_ICE() << "attempt to update placeholder at beginning of tokens";
    }
    if (DAWN_UNLIKELY(next_token_idx_ >= tokens_.size())) {
        TINT_ICE() << "attempt to update placeholder past end of tokens";
    }
    if (DAWN_UNLIKELY(!tokens_[next_token_idx_].IsPlaceholder())) {
        TINT_ICE() << "attempt to update non-placeholder token";
    }
    tokens_[next_token_idx_ - 1].SetType(lhs);
    tokens_[next_token_idx_].SetType(rhs);
}

Source Parser::last_source() const {
    return tokens_[last_source_idx_].source();
}

void Parser::InitializeLex() {
    Lexer l{file_};
    tokens_ = l.Lex();
    ClassifyTemplateArguments(tokens_);
}

bool Parser::Parse() {
    InitializeLex();
    translation_unit();
    return !has_error();
}

// translation_unit
//  : global_directive* global_decl* EOF
void Parser::translation_unit() {
    bool after_global_decl = false;
    while (continue_parsing()) {
        auto& p = peek();
        if (p.IsEof()) {
            break;
        }

        auto ed = global_directive(after_global_decl);
        if (!ed.matched && !ed.errored) {
            auto gd = global_decl();
            if (gd.matched) {
                after_global_decl = true;
            }

            if (!gd.matched && !gd.errored) {
                AddError(p, "unexpected token");
            }
        }

        if (builder_.Diagnostics().NumErrors() >= max_errors_) {
            AddError(Source{{}, p.source().file},
                     "stopping after " + std::to_string(max_errors_) + " errors");
            break;
        }
    }
}

// global_directive
//  : diagnostic_directive
//  | requires_directive
//  | enable_directive
Maybe<Void> Parser::global_directive(bool have_parsed_decl) {
    auto& p = peek();
    Maybe<Void> result = diagnostic_directive();
    if (!result.errored && !result.matched) {
        result = enable_directive();
    }
    if (!result.errored && !result.matched) {
        result = requires_directive();
    }

    if (result.matched && have_parsed_decl) {
        return AddError(p, "directives must come before all global declarations");
    }
    return result;
}

// diagnostic_directive
//  : diagnostic diagnostic_control SEMICOLON
Maybe<Void> Parser::diagnostic_directive() {
    auto decl = sync(Token::Type::kSemicolon, [&]() -> Maybe<Void> {
        if (!match(Token::Type::kDiagnostic)) {
            return Failure::kNoMatch;
        }

        auto source = last_source();
        auto control = expect_diagnostic_control();
        if (control.errored) {
            return Failure::kErrored;
        }

        if (!expect("diagnostic directive", Token::Type::kSemicolon)) {
            return Failure::kErrored;
        }

        auto* directive = create<ast::DiagnosticDirective>(source, std::move(control.value));
        builder_.AST().AddDiagnosticDirective(directive);

        return kSuccess;
    });

    if (decl.errored) {
        return Failure::kErrored;
    }
    return decl;
}

// enable_directive :
// | 'enable' identifier (COMMA identifier)* COMMA? SEMICOLON
Maybe<Void> Parser::enable_directive() {
    return sync(Token::Type::kSemicolon, [&]() -> Maybe<Void> {
        MultiTokenSource decl_source(this);
        if (!match(Token::Type::kEnable)) {
            return Failure::kNoMatch;
        }

        if (peek_is(Token::Type::kParenLeft)) {
            // A common error case is writing `enable(foo);` instead of `enable foo;`.
            synchronized_ = false;
            return AddError(peek().source(), "enable directives don't take parenthesis");
        }

        Vector<const ast::Extension*, 4> extensions;
        while (continue_parsing()) {
            Source ext_src = peek().source();
            auto ext = expect_enum("extension", wgsl::ParseExtension, wgsl::kExtensionStrings);
            if (ext.errored) {
                return Failure::kErrored;
            }
            extensions.Push(create<ast::Extension>(ext_src, ext.value));

            if (!match(Token::Type::kComma)) {
                break;
            }
            if (peek_is(Token::Type::kSemicolon)) {
                break;
            }
        }

        if (!expect("enable directive", Token::Type::kSemicolon)) {
            return Failure::kErrored;
        }

        builder_.AST().AddEnable(create<ast::Enable>(decl_source(), std::move(extensions)));
        return kSuccess;
    });
}

// requires_directive
//  : require identifier (COMMA identifier)* COMMA? SEMICOLON
Maybe<Void> Parser::requires_directive() {
    return sync(Token::Type::kSemicolon, [&]() -> Maybe<Void> {
        MultiTokenSource decl_source(this);
        if (!match(Token::Type::kRequires)) {
            return Failure::kNoMatch;
        }

        // Match the require name.
        auto& t = peek();
        if (handle_error(t)) {
            // The token might itself be an error.
            return Failure::kErrored;
        }

        if (t.Is(Token::Type::kParenLeft)) {
            // A common error case is writing `require(foo);` instead of `require foo;`.
            synchronized_ = false;
            return AddError(t.source(), "requires directives don't take parenthesis");
        }

        ast::Requires::LanguageFeatures features;
        while (continue_parsing()) {
            auto& t2 = next();
            if (handle_error(t2)) {
                // The token might itself be an error.
                return Failure::kErrored;
            }

            // Match the require name.
            if (t2.IsIdentifier()) {
                auto feature = wgsl::ParseLanguageFeature(t2.to_str_view());
                if (feature == LanguageFeature::kUndefined) {
                    // Any identifier is a valid feature name, so we correctly handle new feature
                    // names getting added in the future, they just all get flagged as not
                    // supported.
                    return AddError(t2.source(), "feature '" + t2.to_str() + "' is not supported");
                }
                features.Add(feature);
            } else {
                return AddError(t2.source(), "invalid feature name for requires");
            }

            if (!match(Token::Type::kComma)) {
                break;
            }
            if (peek_is(Token::Type::kSemicolon)) {
                break;
            }
        }

        if (!expect("requires directive", Token::Type::kSemicolon)) {
            return Failure::kErrored;
        }

        builder_.AST().AddRequires(create<ast::Requires>(decl_source(), std::move(features)));
        return kSuccess;
    });
}

// global_decl
//  : SEMICOLON
//  | global_variable_decl SEMICOLON
//  | global_constant_decl SEMICOLON
//  | type_alias_decl SEMICOLON
//  | struct_decl
//  | function_decl
//  | const_assert_statement SEMICOLON
Maybe<Void> Parser::global_decl() {
    if (match(Token::Type::kSemicolon) || match(Token::Type::kEOF)) {
        return kSuccess;
    }

    bool errored = false;
    auto attrs = attribute_list();
    if (attrs.errored) {
        errored = true;
    }
    if (!continue_parsing()) {
        return Failure::kErrored;
    }

    auto decl = sync(Token::Type::kSemicolon, [&]() -> Maybe<Void> {
        auto gv = global_variable_decl(attrs.value);
        if (gv.errored) {
            return Failure::kErrored;
        }
        if (gv.matched) {
            if (!expect("variable declaration", Token::Type::kSemicolon)) {
                return Failure::kErrored;
            }

            builder_.AST().AddGlobalVariable(gv.value);
            return kSuccess;
        }

        auto gc = global_constant_decl(attrs.value);
        if (gc.errored) {
            return Failure::kErrored;
        }
        if (gc.matched) {
            // Avoid the cost of the string allocation for the common no-error case
            if (!peek().Is(Token::Type::kSemicolon)) {
                std::string kind = gc->Kind();
                if (!expect("'" + kind + "' declaration", Token::Type::kSemicolon)) {
                    return Failure::kErrored;
                }
            }
            return kSuccess;
        }

        auto ta = type_alias_decl();
        if (ta.errored) {
            return Failure::kErrored;
        }
        if (ta.matched) {
            if (!expect("type alias", Token::Type::kSemicolon)) {
                return Failure::kErrored;
            }

            builder_.AST().AddTypeDecl(ta.value);
            return kSuccess;
        }

        auto assertion = const_assert_statement();
        if (assertion.errored) {
            return Failure::kErrored;
        }
        if (assertion.matched) {
            builder_.AST().AddConstAssert(assertion.value);
            if (!expect("const assertion declaration", Token::Type::kSemicolon)) {
                return Failure::kErrored;
            }
            return kSuccess;
        }

        return Failure::kNoMatch;
    });

    if (decl.errored) {
        errored = true;
    }
    if (decl.matched) {
        if (expect_attributes_consumed(attrs.value).errored) {
            return Failure::kErrored;
        }
        return kSuccess;
    }

    auto str = struct_decl();
    if (str.errored) {
        errored = true;
    }
    if (str.matched) {
        if (expect_attributes_consumed(attrs.value).errored) {
            return Failure::kErrored;
        }
        return kSuccess;
    }

    auto func = function_decl(attrs.value);
    if (func.errored) {
        errored = true;
    }
    if (func.matched) {
        return kSuccess;
    }

    if (errored) {
        return Failure::kErrored;
    }

    // Invalid syntax found - try and determine the best error message

    // We have attributes parsed, but nothing to consume them?
    if (attrs.value.Length() > 0) {
        return AddError(next(), "expected declaration after attributes");
    }

    // We have a statement outside of a function?
    auto& t = peek();
    auto stat = without_diag([&] { return statement(); });
    if (stat.matched) {
        // Attempt to jump to the next '}' - the function might have just been
        // missing an opening line.
        sync_to(Token::Type::kBraceRight, true);
        return AddError(t, "statement found outside of function body");
    }
    if (!stat.errored) {
        // No match, no error - the parser might not have progressed.
        // Ensure we always make _some_ forward progress.
        next();
    }

    // The token might itself be an error.
    if (handle_error(t)) {
        return Failure::kErrored;
    }

    // Exhausted all attempts to make sense of where we're at.
    // Return a no-match

    return Failure::kNoMatch;
}

// global_variable_decl
//  : variable_attribute_list* variable_decl (EQUAL expression)?
Maybe<const ast::Variable*> Parser::global_variable_decl(AttributeList& attrs) {
    MultiTokenSource decl_source(this);
    auto decl = variable_decl();
    if (decl.errored) {
        return Failure::kErrored;
    }
    if (!decl.matched) {
        return Failure::kNoMatch;
    }

    const ast::Expression* initializer = nullptr;
    if (match(Token::Type::kEqual)) {
        auto expr = expression();
        if (expr.errored) {
            return Failure::kErrored;
        }
        if (!expr.matched) {
            return AddError(peek(), "missing initializer for 'var' declaration");
        }
        initializer = expr.value;
    }

    TINT_DEFER(attrs.Clear());

    return builder_.create<ast::Var>(decl_source(),               // source
                                     builder_.Ident(decl->name),  // symbol
                                     decl->type,                  // type
                                     decl->address_space,         // address space
                                     decl->access,                // access control
                                     initializer,                 // initializer
                                     std::move(attrs));           // attributes
}

// global_constant_decl :
//  | LET optionally_typed_ident global_const_initializer
//  | attribute* override optionally_typed_ident (equal expression)?
// global_const_initializer
//  : EQUAL const_expr
Maybe<const ast::Variable*> Parser::global_constant_decl(AttributeList& attrs) {
    bool is_overridable = false;
    const char* use = nullptr;
    MultiTokenSource decl_source(this);
    Source source;
    if (match(Token::Type::kConst)) {
        use = "'const' declaration";
    } else if (match(Token::Type::kOverride)) {
        use = "'override' declaration";
        is_overridable = true;
    } else if (match(Token::Type::kLet, &source)) {
        return AddError(source, "module-scope 'let' is invalid, use 'const'");
    } else {
        return Failure::kNoMatch;
    }

    auto decl = expect_optionally_typed_ident(use);
    if (decl.errored) {
        return Failure::kErrored;
    }

    bool has_initializer = false;
    if (is_overridable) {
        has_initializer = match(Token::Type::kEqual);
    } else {
        if (!expect(use, Token::Type::kEqual)) {
            return Failure::kErrored;
        }
        has_initializer = true;
    }

    const ast::Expression* initializer = nullptr;
    if (has_initializer) {
        auto expr = expression();
        if (expr.errored) {
            return Failure::kErrored;
        }
        if (!expr.matched) {
            return AddError(peek(), "missing initializer for " + std::string(use));
        }
        initializer = std::move(expr.value);
    }

    TINT_DEFER(attrs.Clear());
    if (is_overridable) {
        return builder_.Override(decl_source(),      // source
                                 decl->name,         // symbol
                                 decl->type,         // type
                                 initializer,        // initializer
                                 std::move(attrs));  // attributes
    }
    return builder_.GlobalConst(decl_source(),      // source
                                decl->name,         // symbol
                                decl->type,         // type
                                initializer,        // initializer
                                std::move(attrs));  // attributes
}

// variable_decl
//   : VAR variable_qualifier? optionally_typed_ident
//
// Note, the `( LESS_THAN address_space ( COMMA access_mode )? GREATER_THAN ) is pulled out into
// a `variable_qualifier` helper.
Maybe<Parser::VarDeclInfo> Parser::variable_decl() {
    Source source;
    if (!match(Token::Type::kVar, &source)) {
        return Failure::kNoMatch;
    }

    VariableQualifier vq;
    auto explicit_vq = variable_qualifier();
    if (explicit_vq.errored) {
        return Failure::kErrored;
    }
    if (explicit_vq.matched) {
        vq = explicit_vq.value;
    }

    auto decl = expect_optionally_typed_ident("variable declaration");
    if (decl.errored) {
        return Failure::kErrored;
    }

    return VarDeclInfo{decl->name->source, decl->name, vq.address_space, vq.access, decl->type};
}

Expect<Parser::TypedIdentifier> Parser::expect_ident_with_optional_type_specifier(
    std::string_view use,
    bool allow_inferred) {
    auto ident = expect_ident(use);
    if (ident.errored) {
        return Failure::kErrored;
    }

    if (allow_inferred && !peek_is(Token::Type::kColon)) {
        return TypedIdentifier{ast::Type{}, ident.value};
    }

    if (!expect(use, Token::Type::kColon)) {
        return Failure::kErrored;
    }

    auto& t = peek();
    auto type = type_specifier();
    if (type.errored) {
        return Failure::kErrored;
    }
    if (!type.matched) {
        return AddError(t.source(), "invalid type", use);
    }

    return TypedIdentifier{type.value, ident.value};
}

// optionally_typed_ident
//   : ident ( COLON typed_decl ) ?
Expect<Parser::TypedIdentifier> Parser::expect_optionally_typed_ident(std::string_view use) {
    return expect_ident_with_optional_type_specifier(use, /* allow_inferred */ true);
}

// ident_with_type_specifier
//   : IDENT COLON type_specifier
Expect<Parser::TypedIdentifier> Parser::expect_ident_with_type_specifier(std::string_view use) {
    return expect_ident_with_optional_type_specifier(use, /* allow_inferred */ false);
}

// variable_qualifier
//   : _template_args_start expression (COMMA expression)? COMMA? _template_args_end
Maybe<Parser::VariableQualifier> Parser::variable_qualifier() {
    if (!peek_is(Token::Type::kTemplateArgsLeft) && !peek_is(Token::Type::kLessThan)) {
        // Note: kLessThan will give a sensible error at expect_template_arg_block()
        return Failure::kNoMatch;
    }

    auto* use = "variable declaration";
    auto vq = expect_template_arg_block(use, [&]() -> Expect<VariableQualifier> {
        auto address_space = expect_expression("'var' address space");
        if (address_space.errored) {
            return Failure::kErrored;
        }
        // The first expression in this condition also matches trailing comma.
        if (match(Token::Type::kComma) && !peek().Is(Token::Type::kTemplateArgsRight)) {
            auto access = expect_expression("'var' access mode");
            if (access.errored) {
                return Failure::kErrored;
            }
            // Allow for trailing comma.
            match(Token::Type::kComma);
            return VariableQualifier{address_space.value, access.value};
        }
        return VariableQualifier{address_space.value};
    });

    if (vq.errored) {
        return Failure::kErrored;
    }

    return vq;
}

// type_alias_decl
//   : ALIAS IDENT EQUAL type_specifier
Maybe<const ast::Alias*> Parser::type_alias_decl() {
    MultiTokenSource source(this);
    if (!match(Token::Type::kAlias)) {
        return Failure::kNoMatch;
    }

    const char* use = "type alias";

    auto name = expect_ident(use);
    if (name.errored) {
        return Failure::kErrored;
    }

    if (!expect(use, Token::Type::kEqual)) {
        return Failure::kErrored;
    }

    auto type = type_specifier();
    if (type.errored) {
        return Failure::kErrored;
    }
    if (!type.matched) {
        return AddError(peek(), "invalid type alias");
    }

    return builder_.ty.alias(source(), name.value, type.value);
}

// type_specifier
//   : IDENTIFIER template_arguments?
Maybe<ast::Type> Parser::type_specifier() {
    MultiTokenSource source(this);
    auto& ident = peek();
    if (!match(Token::Type::kIdentifier)) {
        return Failure::kNoMatch;
    }

    if (!peek_is(Token::Type::kTemplateArgsLeft)) {
        return builder_.ty(builder_.Ident(source(), ident.to_str()));
    }

    auto args = expect_template_arg_block("type template arguments", [&] {
        return expect_expression_list("type template argument list",
                                      Token::Type::kTemplateArgsRight);
    });
    if (args.errored) {
        return Failure::kErrored;
    }
    return builder_.ty(builder_.Ident(source(), ident.to_str(), std::move(args.value)));
}

template <typename ENUM>
Expect<ENUM> Parser::expect_enum(std::string_view name,
                                 ENUM (*parse)(std::string_view str),
                                 Slice<const std::string_view> strings,
                                 std::string_view use) {
    auto& t = peek();
    auto ident = t.to_str();
    if (t.IsIdentifier()) {
        auto val = parse(ident);
        if (val != ENUM::kUndefined) {
            synchronized_ = true;
            next();
            return val;
        }
    }

    // Was the token itself an error?
    if (handle_error(t)) {
        return Failure::kErrored;
    }

    /// Create a sensible error message
    StyledText err;
    err << "expected " << name;

    if (!use.empty()) {
        err << " for " << use;
    }
    err << "\n";

    if (strings == wgsl::kExtensionStrings && !HasPrefix(ident, "chromium")) {
        // Filter out 'chromium' prefixed extensions. We don't want to advertise experimental
        // extensions to end users (unless it looks like they've actually mis-typed a chromium
        // extension name)
        Vector<std::string_view, 8> filtered;
        for (auto str : strings) {
            if (!HasPrefix(str, "chromium")) {
                filtered.Push(str);
            }
        }
        tint::SuggestAlternatives(ident, filtered.Slice(), err);
    } else {
        tint::SuggestAlternatives(ident, strings, err);
    }

    synchronized_ = false;
    return AddError(t.source(), std::move(err));
}

Expect<ast::Type> Parser::expect_type(std::string_view use) {
    auto type = type_specifier();
    if (type.errored) {
        return Failure::kErrored;
    }
    if (!type.matched) {
        return AddError(peek().source(), "invalid type", use);
    }
    return type.value;
}

// struct_decl
//   : STRUCT IDENT struct_body_decl
Maybe<const ast::Struct*> Parser::struct_decl() {
    MultiTokenSource source(this);

    if (!match(Token::Type::kStruct)) {
        return Failure::kNoMatch;
    }

    auto name = expect_ident("struct declaration");
    if (name.errored) {
        return Failure::kErrored;
    }

    auto body = expect_struct_body_decl();
    if (body.errored) {
        return Failure::kErrored;
    }

    return builder_.Structure(source(), name.value, std::move(body.value));
}

// struct_body_decl
//   : BRACE_LEFT (struct_member COMMA)* struct_member COMMA? BRACE_RIGHT
Expect<Parser::StructMemberList> Parser::expect_struct_body_decl() {
    return expect_brace_block("struct declaration", [&]() -> Expect<StructMemberList> {
        StructMemberList members;
        bool errored = false;
        while (continue_parsing()) {
            // Check for the end of the list.
            auto& t = peek();
            if (!t.IsIdentifier() && !t.Is(Token::Type::kAttr)) {
                break;
            }

            auto member = expect_struct_member();
            if (member.errored) {
                errored = true;
                if (!sync_to(Token::Type::kComma, /* consume: */ false)) {
                    return Failure::kErrored;
                }
            } else {
                members.Push(member.value);
            }

            if (!match(Token::Type::kComma)) {
                break;
            }
        }
        if (errored) {
            return Failure::kErrored;
        }
        return members;
    });
}

// struct_member
//   : attribute* ident_with_type_specifier
Expect<const ast::StructMember*> Parser::expect_struct_member() {
    auto attrs = attribute_list();
    if (attrs.errored) {
        return Failure::kErrored;
    }

    auto decl = expect_ident_with_type_specifier("struct member");
    if (decl.errored) {
        return Failure::kErrored;
    }

    return builder_.Member(decl->name->source, decl->name, decl->type, std::move(attrs.value));
}

// const_assert_statement
//   : STATIC_ASSERT expression
Maybe<const ast::ConstAssert*> Parser::const_assert_statement() {
    MultiTokenSource source(this);
    if (!match(Token::Type::kConstAssert)) {
        return Failure::kNoMatch;
    }

    auto condition = expression();
    if (condition.errored) {
        return Failure::kErrored;
    }
    if (!condition.matched) {
        return AddError(peek(), "unable to parse condition expression");
    }

    return create<ast::ConstAssert>(source(), condition.value);
}

// function_decl
//   : function_header compound_statement
Maybe<const ast::Function*> Parser::function_decl(AttributeList& attrs) {
    MultiTokenSource source(this);

    auto header = function_header();
    if (header.errored) {
        if (sync_to(Token::Type::kBraceLeft, /* consume: */ false)) {
            // There were errors in the function header, but the parser has managed to
            // resynchronize with the opening brace. As there's no outer
            // synchronization token for function declarations, attempt to parse the
            // function body. The AST isn't used as we've already errored, but this
            // catches any errors inside the body, and can help keep the parser in
            // sync.
            expect_compound_statement("function body");
        }
        return Failure::kErrored;
    }
    if (!header.matched) {
        return Failure::kNoMatch;
    }

    bool errored = false;

    auto body = expect_compound_statement("function body");
    if (body.errored) {
        errored = true;
    }

    if (errored) {
        return Failure::kErrored;
    }

    TINT_DEFER(attrs.Clear());

    return builder_.Func(source, header->name, header->params, header->return_type, body.value,
                         std::move(attrs), header->return_type_attributes);
}

// function_header
//   : FN IDENT PAREN_LEFT param_list PAREN_RIGHT return_type_specifier_optional
// return_type_specifier_optional
//   :
//   | ARROW attribute_list* type_specifier
Maybe<Parser::FunctionHeader> Parser::function_header() {
    Source source;
    if (!match(Token::Type::kFn, &source)) {
        return Failure::kNoMatch;
    }

    const char* use = "function declaration";
    bool errored = false;

    auto name = expect_ident(use);
    if (name.errored) {
        errored = true;
        if (!sync_to(Token::Type::kParenLeft, /* consume: */ false)) {
            return Failure::kErrored;
        }
    }

    auto params = expect_paren_block(use, [&] { return expect_param_list(); });
    if (params.errored) {
        errored = true;
        if (!synchronized_) {
            return Failure::kErrored;
        }
    }

    ast::Type return_type;
    AttributeList return_attributes;

    if (match(Token::Type::kArrow)) {
        auto attrs = attribute_list();
        if (attrs.errored) {
            return Failure::kErrored;
        }
        return_attributes = attrs.value;

        auto type = type_specifier();
        if (type.errored) {
            errored = true;
        } else if (!type.matched) {
            return AddError(peek(), "unable to determine function return type");
        } else {
            return_type = type.value;
        }
    } else {
        return_type = builder_.ty.void_();
    }

    if (errored) {
        return Failure::kErrored;
    }

    return FunctionHeader{
        source, name.value, std::move(params.value), return_type, std::move(return_attributes),
    };
}

// param_list
//   :
//   | (param COMMA)* param COMMA?
Expect<Parser::ParameterList> Parser::expect_param_list() {
    ParameterList ret;
    while (continue_parsing()) {
        // Check for the end of the list.
        auto& t = peek();
        if (!t.IsIdentifier() && !t.Is(Token::Type::kAttr)) {
            break;
        }

        auto param = expect_param();
        if (param.errored) {
            return Failure::kErrored;
        }
        ret.Push(param.value);

        if (!match(Token::Type::kComma)) {
            break;
        }
    }

    return ret;
}

// param
//   : attribute_list* ident COLON type_specifier
Expect<const ast::Parameter*> Parser::expect_param() {
    auto attrs = attribute_list();

    auto decl = expect_ident_with_type_specifier("parameter");
    if (decl.errored) {
        return Failure::kErrored;
    }

    return builder_.Param(decl->name->source,       // source
                          decl->name,               // symbol
                          decl->type,               // type
                          std::move(attrs.value));  // attributes
}

// compound_statement
//   : attribute* BRACE_LEFT statement* BRACE_RIGHT
Expect<ast::BlockStatement*> Parser::expect_compound_statement(std::string_view use) {
    auto attrs = attribute_list();
    if (attrs.errored) {
        return Failure::kErrored;
    }
    return expect_compound_statement(attrs.value, use);
}

// compound_statement
//   : attribute* BRACE_LEFT statement* BRACE_RIGHT
Expect<ast::BlockStatement*> Parser::expect_compound_statement(AttributeList& attrs,
                                                               std::string_view use) {
    MultiTokenSource source(this);
    auto stmts =
        expect_brace_block(use, [&]() -> Expect<StatementList> { return expect_statements(); });
    if (stmts.errored) {
        return Failure::kErrored;
    }
    TINT_DEFER(attrs.Clear());
    return create<ast::BlockStatement>(source(), stmts.value, std::move(attrs));
}

// paren_expression
//   : PAREN_LEFT expression PAREN_RIGHT
Expect<const ast::Expression*> Parser::expect_paren_expression() {
    return expect_paren_block("", [&]() -> Expect<const ast::Expression*> {
        auto expr = expression();
        if (expr.errored) {
            return Failure::kErrored;
        }
        if (!expr.matched) {
            return AddError(peek(), "unable to parse expression");
        }

        return expr.value;
    });
}

// statements
//   : statement*
Expect<Parser::StatementList> Parser::expect_statements() {
    bool errored = false;
    StatementList stmts;

    while (continue_parsing()) {
        auto stmt = statement();
        if (stmt.errored) {
            errored = true;
        } else if (stmt.matched) {
            stmts.Push(stmt.value);
        } else {
            break;
        }
    }

    if (errored) {
        return Failure::kErrored;
    }

    return stmts;
}

// statement
//   : SEMICOLON
//   | if_statement
//   | switch_statement
//   | loop_statement
//   | for_statement
//   | while_statement
//   | compound_statement
//   | non_block_statement   // Note, we inject an extra rule in here for simpler parsing
Maybe<const ast::Statement*> Parser::statement() {
    while (match(Token::Type::kSemicolon)) {
        // Skip empty statements
    }

    auto attrs = attribute_list();
    if (attrs.errored) {
        return Failure::kErrored;
    }
    TINT_DEFER(expect_attributes_consumed(attrs.value));

    // Non-block statements that error can resynchronize on semicolon.
    auto stmt = sync(Token::Type::kSemicolon, [&] { return non_block_statement(); });
    if (stmt.errored) {
        return Failure::kErrored;
    }
    if (stmt.matched) {
        return stmt;
    }

    auto stmt_if = if_statement(attrs.value);
    if (stmt_if.errored) {
        return Failure::kErrored;
    }
    if (stmt_if.matched) {
        return stmt_if.value;
    }

    auto sw = switch_statement(attrs.value);
    if (sw.errored) {
        return Failure::kErrored;
    }
    if (sw.matched) {
        return sw.value;
    }

    auto loop = loop_statement(attrs.value);
    if (loop.errored) {
        return Failure::kErrored;
    }
    if (loop.matched) {
        return loop.value;
    }

    auto stmt_for = for_statement(attrs.value);
    if (stmt_for.errored) {
        return Failure::kErrored;
    }
    if (stmt_for.matched) {
        return stmt_for.value;
    }

    auto stmt_while = while_statement(attrs.value);
    if (stmt_while.errored) {
        return Failure::kErrored;
    }
    if (stmt_while.matched) {
        return stmt_while.value;
    }

    if (peek_is(Token::Type::kBraceLeft)) {
        auto body = expect_compound_statement(attrs.value, "block statement");
        if (body.errored) {
            return Failure::kErrored;
        }
        return body.value;
    }

    return Failure::kNoMatch;
}

// non_block_statement (continued)
//   : return_statement SEMICOLON
//   | func_call_statement SEMICOLON
//   | variable_statement SEMICOLON
//   | break_statement SEMICOLON
//   | continue_statement SEMICOLON
//   | DISCARD SEMICOLON
//   | variable_updating_statement SEMICOLON
//   | const_assert_statement SEMICOLON
Maybe<const ast::Statement*> Parser::non_block_statement() {
    auto stmt = [&]() -> Maybe<const ast::Statement*> {
        auto ret_stmt = return_statement();
        if (ret_stmt.errored) {
            return Failure::kErrored;
        }
        if (ret_stmt.matched) {
            return ret_stmt.value;
        }

        auto func = func_call_statement();
        if (func.errored) {
            return Failure::kErrored;
        }
        if (func.matched) {
            return func.value;
        }

        auto var = variable_statement();
        if (var.errored) {
            return Failure::kErrored;
        }
        if (var.matched) {
            return var.value;
        }

        auto b = break_statement();
        if (b.errored) {
            return Failure::kErrored;
        }
        if (b.matched) {
            return b.value;
        }

        auto cont = continue_statement();
        if (cont.errored) {
            return Failure::kErrored;
        }
        if (cont.matched) {
            return cont.value;
        }

        Source source;
        if (match(Token::Type::kDiscard, &source)) {
            return builder_.Discard(source);
        }

        // Note, this covers assignment, increment and decrement
        auto assign = variable_updating_statement();
        if (assign.errored) {
            return Failure::kErrored;
        }
        if (assign.matched) {
            return assign.value;
        }

        auto stmt_static_assert = const_assert_statement();
        if (stmt_static_assert.errored) {
            return Failure::kErrored;
        }
        if (stmt_static_assert.matched) {
            return stmt_static_assert.value;
        }

        return Failure::kNoMatch;
    }();

    if (stmt.matched && !expect(stmt->Name(), Token::Type::kSemicolon)) {
        return Failure::kErrored;
    }
    return stmt;
}

// return_statement
//   : RETURN expression?
Maybe<const ast::ReturnStatement*> Parser::return_statement() {
    Source source;
    if (!match(Token::Type::kReturn, &source)) {
        return Failure::kNoMatch;
    }

    auto expr = expression();
    if (expr.errored) {
        return Failure::kErrored;
    }

    return expr.matched ? builder_.Return(source, expr.value) : builder_.Return(source);
}

// variable_statement
//   : variable_decl
//   | variable_decl EQUAL expression
//   | LET optionally_typed_ident EQUAL expression
//   | CONST optionally_typed_ident EQUAL expression
Maybe<const ast::VariableDeclStatement*> Parser::variable_statement() {
    MultiTokenSource decl_source_range(this);
    if (match(Token::Type::kConst)) {
        auto typed_ident = expect_optionally_typed_ident("'const' declaration");
        if (typed_ident.errored) {
            return Failure::kErrored;
        }

        auto decl_source = decl_source_range();

        if (!expect("'const' declaration", Token::Type::kEqual)) {
            return Failure::kErrored;
        }

        auto initializer = expression();
        if (initializer.errored) {
            return Failure::kErrored;
        }
        if (!initializer.matched) {
            return AddError(peek(), "missing initializer for 'const' declaration");
        }

        auto* const_ = builder_.Const(typed_ident->name->source,  // source
                                      typed_ident->name,          // symbol
                                      typed_ident->type,          // type
                                      initializer.value);         // initializer

        return create<ast::VariableDeclStatement>(decl_source, const_);
    }

    if (match(Token::Type::kLet)) {
        auto typed_ident = expect_optionally_typed_ident("'let' declaration");
        if (typed_ident.errored) {
            return Failure::kErrored;
        }

        auto decl_source = decl_source_range();

        if (!expect("'let' declaration", Token::Type::kEqual)) {
            return Failure::kErrored;
        }

        auto initializer = expression();
        if (initializer.errored) {
            return Failure::kErrored;
        }
        if (!initializer.matched) {
            return AddError(peek(), "missing initializer for 'let' declaration");
        }

        auto* let = builder_.Let(typed_ident->name->source,  // source
                                 typed_ident->name,          // symbol
                                 typed_ident->type,          // type
                                 initializer.value);         // initializer

        return create<ast::VariableDeclStatement>(decl_source, let);
    }

    auto decl = variable_decl();
    if (decl.errored) {
        return Failure::kErrored;
    }
    if (!decl.matched) {
        return Failure::kNoMatch;
    }

    auto decl_source = decl_source_range();

    const ast::Expression* initializer = nullptr;
    if (match(Token::Type::kEqual)) {
        auto initializer_expr = expression();
        if (initializer_expr.errored) {
            return Failure::kErrored;
        }
        if (!initializer_expr.matched) {
            return AddError(peek(), "missing initializer for 'var' declaration");
        }

        initializer = initializer_expr.value;
    }

    auto* var = builder_.create<ast::Var>(decl_source,                 // source
                                          builder_.Ident(decl->name),  // symbol
                                          decl->type,                  // type
                                          decl->address_space,         // address space
                                          decl->access,                // access control
                                          initializer,                 // initializer
                                          tint::Empty);                // attributes

    return create<ast::VariableDeclStatement>(var->source, var);
}

// if_statement
//   : attribute* if_clause else_if_clause* else_clause?
// if_clause:
//   : IF expression compound_stmt
// else_if_clause:
//   : ELSE IF expression compound_stmt
// else_clause
//   : ELSE compound_statement
Maybe<const ast::IfStatement*> Parser::if_statement(AttributeList& attrs) {
    // Parse if-else chains iteratively instead of recursively, to avoid
    // stack-overflow for long chains of if-else statements.

    struct IfInfo {
        Source source;
        const ast::Expression* condition;
        const ast::BlockStatement* body;
        AttributeList attributes;
    };

    // Parse an if statement, capturing the source, condition, and body statement.
    auto parse_if = [&]() -> Maybe<IfInfo> {
        Source source;
        if (!match(Token::Type::kIf, &source)) {
            return Failure::kNoMatch;
        }

        auto condition = expression();
        if (condition.errored) {
            return Failure::kErrored;
        }
        if (!condition.matched) {
            return AddError(peek(), "unable to parse condition expression");
        }

        auto body = expect_compound_statement("if statement");
        if (body.errored) {
            return Failure::kErrored;
        }

        TINT_DEFER(attrs.Clear());
        return IfInfo{source, condition.value, body.value, std::move(attrs)};
    };

    std::vector<IfInfo> statements;

    // Parse the first if statement.
    auto first_if = parse_if();
    if (first_if.errored) {
        return Failure::kErrored;
    } else if (!first_if.matched) {
        return Failure::kNoMatch;
    }
    statements.push_back(first_if.value);

    // Parse the components of every "else {if}" in the chain.
    const ast::Statement* last_stmt = nullptr;
    while (continue_parsing()) {
        if (!match(Token::Type::kElse)) {
            break;
        }

        // Try to parse an "else if".
        auto else_if = parse_if();
        if (else_if.errored) {
            return Failure::kErrored;
        } else if (else_if.matched) {
            statements.push_back(else_if.value);
            continue;
        }

        // If it wasn't an "else if", it must just be an "else".
        auto else_body = expect_compound_statement("else statement");
        if (else_body.errored) {
            return Failure::kErrored;
        }
        last_stmt = else_body.value;
        break;
    }

    // Now walk back through the statements to create their AST nodes.
    for (auto itr = statements.rbegin(); itr != statements.rend(); itr++) {
        last_stmt = create<ast::IfStatement>(itr->source, itr->condition, itr->body, last_stmt,
                                             std::move(itr->attributes));
    }

    return last_stmt->As<ast::IfStatement>();
}

// switch_statement
//   : attribute* SWITCH expression BRACKET_LEFT switch_body+ BRACKET_RIGHT
Maybe<const ast::SwitchStatement*> Parser::switch_statement(AttributeList& attrs) {
    Source source;
    if (!match(Token::Type::kSwitch, &source)) {
        return Failure::kNoMatch;
    }

    auto condition = expression();
    if (condition.errored) {
        return Failure::kErrored;
    }
    if (!condition.matched) {
        return AddError(peek(), "unable to parse selector expression");
    }

    auto body_attrs = attribute_list();
    if (body_attrs.errored) {
        return Failure::kErrored;
    }

    auto body = expect_brace_block("switch statement", [&]() -> Expect<CaseStatementList> {
        bool errored = false;
        CaseStatementList list;
        while (continue_parsing()) {
            auto stmt = switch_body();
            if (stmt.errored) {
                errored = true;
                continue;
            }
            if (!stmt.matched) {
                break;
            }
            list.Push(stmt.value);
        }
        if (errored) {
            return Failure::kErrored;
        }
        return list;
    });

    if (body.errored) {
        return Failure::kErrored;
    }

    TINT_DEFER(attrs.Clear());
    return create<ast::SwitchStatement>(source, condition.value, body.value, std::move(attrs),
                                        std::move(body_attrs.value));
}

// switch_body
//   : CASE case_selectors COLON? compound_statement
//   | DEFAULT COLON? compound_statement
Maybe<const ast::CaseStatement*> Parser::switch_body() {
    if (!peek_is(Token::Type::kCase) && !peek_is(Token::Type::kDefault)) {
        return Failure::kNoMatch;
    }

    auto& t = next();

    CaseSelectorList selector_list;
    if (t.Is(Token::Type::kCase)) {
        auto selectors = expect_case_selectors();
        if (selectors.errored) {
            return Failure::kErrored;
        }

        selector_list = std::move(selectors.value);
    } else {
        // Push the default case selector
        selector_list.Push(create<ast::CaseSelector>(t.source()));
    }

    // Consume the optional colon if present.
    match(Token::Type::kColon);

    const char* use = "case statement";
    auto body = expect_compound_statement(use);
    if (body.errored) {
        return Failure::kErrored;
    }

    return create<ast::CaseStatement>(t.source(), selector_list, body.value);
}

// case_selectors
//   : case_selector (COMMA case_selector)* COMMA?
Expect<Parser::CaseSelectorList> Parser::expect_case_selectors() {
    CaseSelectorList selectors;

    while (continue_parsing()) {
        auto expr = case_selector();
        if (expr.errored) {
            return Failure::kErrored;
        }
        if (!expr.matched) {
            break;
        }
        selectors.Push(expr.value);

        if (!match(Token::Type::kComma)) {
            break;
        }
    }

    if (selectors.IsEmpty()) {
        return AddError(peek(), "expected case selector expression or `default`");
    }

    return selectors;
}

// case_selector
//   : DEFAULT
//   | expression
Maybe<const ast::CaseSelector*> Parser::case_selector() {
    auto& p = peek();

    if (match(Token::Type::kDefault)) {
        return create<ast::CaseSelector>(p.source());
    }

    auto expr = expression();
    if (expr.errored) {
        return Failure::kErrored;
    }
    if (!expr.matched) {
        return Failure::kNoMatch;
    }
    return create<ast::CaseSelector>(p.source(), expr.value);
}

// loop_statement
//   : attribute* LOOP attribute* BRACKET_LEFT statements continuing_statement? BRACKET_RIGHT
Maybe<const ast::LoopStatement*> Parser::loop_statement(AttributeList& attrs) {
    Source source;
    if (!match(Token::Type::kLoop, &source)) {
        return Failure::kNoMatch;
    }

    auto body_attrs = attribute_list();
    if (body_attrs.errored) {
        return Failure::kErrored;
    }

    Maybe<const ast::BlockStatement*> continuing(Failure::kErrored);
    MultiTokenSource body_source(this);
    auto body = expect_brace_block("loop", [&]() -> Maybe<StatementList> {
        auto stmts = expect_statements();
        if (stmts.errored) {
            return Failure::kErrored;
        }

        continuing = continuing_statement();
        if (continuing.errored) {
            return Failure::kErrored;
        }
        return stmts;
    });
    if (body.errored) {
        return Failure::kErrored;
    }

    TINT_DEFER(attrs.Clear());
    return create<ast::LoopStatement>(
        source, create<ast::BlockStatement>(body_source(), body.value, std::move(body_attrs.value)),
        continuing.value, std::move(attrs));
}

ForHeader::ForHeader(const ast::Statement* init,
                     const ast::Expression* cond,
                     const ast::Statement* cont)
    : initializer(init), condition(cond), continuing(cont) {}

ForHeader::~ForHeader() = default;

// (variable_statement | variable_updating_statement | func_call_statement)?
Maybe<const ast::Statement*> Parser::for_header_initializer() {
    auto call = func_call_statement();
    if (call.errored) {
        return Failure::kErrored;
    }
    if (call.matched) {
        return call.value;
    }

    auto var = variable_statement();
    if (var.errored) {
        return Failure::kErrored;
    }
    if (var.matched) {
        return var.value;
    }

    auto assign = variable_updating_statement();
    if (assign.errored) {
        return Failure::kErrored;
    }
    if (assign.matched) {
        return assign.value;
    }

    return Failure::kNoMatch;
}

// (variable_updating_statement | func_call_statement)?
Maybe<const ast::Statement*> Parser::for_header_continuing() {
    auto call_stmt = func_call_statement();
    if (call_stmt.errored) {
        return Failure::kErrored;
    }
    if (call_stmt.matched) {
        return call_stmt.value;
    }

    auto assign = variable_updating_statement();
    if (assign.errored) {
        return Failure::kErrored;
    }
    if (assign.matched) {
        return assign.value;
    }

    return Failure::kNoMatch;
}

// for_header
//   : for_header_initializer? SEMICOLON expression? SEMICOLON for_header_continuing?
Expect<std::unique_ptr<ForHeader>> Parser::expect_for_header() {
    auto initializer = for_header_initializer();
    if (initializer.errored) {
        return Failure::kErrored;
    }

    if (!expect("initializer in for loop", Token::Type::kSemicolon)) {
        return Failure::kErrored;
    }

    auto condition = expression();
    if (condition.errored) {
        return Failure::kErrored;
    }

    if (!expect("condition in for loop", Token::Type::kSemicolon)) {
        return Failure::kErrored;
    }

    auto continuing = for_header_continuing();
    if (continuing.errored) {
        return Failure::kErrored;
    }

    return std::make_unique<ForHeader>(initializer.value, condition.value, continuing.value);
}

// for_statement
//   : FOR PAREN_LEFT for_header PAREN_RIGHT compound_statement
Maybe<const ast::ForLoopStatement*> Parser::for_statement(AttributeList& attrs) {
    Source source;
    if (!match(Token::Type::kFor, &source)) {
        return Failure::kNoMatch;
    }

    auto header = expect_paren_block("for loop", [&] { return expect_for_header(); });
    if (header.errored) {
        return Failure::kErrored;
    }

    auto body = expect_compound_statement("for loop");
    if (body.errored) {
        return Failure::kErrored;
    }

    TINT_DEFER(attrs.Clear());
    return create<ast::ForLoopStatement>(source, header->initializer, header->condition,
                                         header->continuing, body.value, std::move(attrs));
}

// while_statement
//   :  attribute* WHILE expression compound_statement
Maybe<const ast::WhileStatement*> Parser::while_statement(AttributeList& attrs) {
    Source source;
    if (!match(Token::Type::kWhile, &source)) {
        return Failure::kNoMatch;
    }

    auto condition = expression();
    if (condition.errored) {
        return Failure::kErrored;
    }
    if (!condition.matched) {
        return AddError(peek(), "unable to parse while condition expression");
    }

    auto body = expect_compound_statement("while loop");
    if (body.errored) {
        return Failure::kErrored;
    }

    TINT_DEFER(attrs.Clear());
    return create<ast::WhileStatement>(source, condition.value, body.value, std::move(attrs));
}

// func_call_statement
//    : IDENT argument_expression_list
Maybe<const ast::CallStatement*> Parser::func_call_statement() {
    auto& t = peek();
    auto& t2 = peek(1);
    if (!t.IsIdentifier() || !t2.Is(Token::Type::kParenLeft)) {
        return Failure::kNoMatch;
    }

    next();  // Consume the first peek

    auto params = expect_argument_expression_list("function call");
    if (params.errored) {
        return Failure::kErrored;
    }

    return builder_.CallStmt(
        t.source(),
        builder_.Call(t.source(), builder_.Expr(t.source(), t.to_str()), std::move(params.value)));
}

// break_statement
//   : BREAK
Maybe<const ast::BreakStatement*> Parser::break_statement() {
    Source source;
    if (!match(Token::Type::kBreak, &source)) {
        return Failure::kNoMatch;
    }

    return create<ast::BreakStatement>(source);
}

// continue_statement
//   : CONTINUE
Maybe<const ast::ContinueStatement*> Parser::continue_statement() {
    Source source;
    if (!match(Token::Type::kContinue, &source)) {
        return Failure::kNoMatch;
    }

    return create<ast::ContinueStatement>(source);
}

// break_if_statement:
//    'break' 'if' expression semicolon
Maybe<const ast::Statement*> Parser::break_if_statement() {
    auto& t1 = peek();
    auto& t2 = peek(1);

    // Match both the `break` and `if` at the same time.
    if (!t1.Is(Token::Type::kBreak) || !t2.Is(Token::Type::kIf)) {
        return Failure::kNoMatch;
    }
    next();  // Consume the peek
    next();  // Consume the peek

    auto expr = expression();
    if (expr.errored) {
        return Failure::kErrored;
    }
    if (!expr.matched) {
        return AddError(t1, "expected expression for `break-if`");
    }
    if (!expect("`break-if` statement", Token::Type::kSemicolon)) {
        return Failure::kErrored;
    }

    return create<ast::BreakIfStatement>(t1.source(), expr.value);
}

// continuing_compound_statement:
//   attribute* BRACE_LEFT statement* break_if_statement? BRACE_RIGHT
Maybe<const ast::BlockStatement*> Parser::continuing_compound_statement() {
    auto attrs = attribute_list();
    if (attrs.errored) {
        return Failure::kErrored;
    }

    MultiTokenSource source(this);
    auto body = expect_brace_block("", [&]() -> Expect<StatementList> {
        StatementList stmts;

        while (continue_parsing()) {
            // Note, break-if has to parse before statements because statements includes `break`
            auto break_if = break_if_statement();
            if (break_if.errored) {
                return Failure::kErrored;
            }
            if (break_if.matched) {
                stmts.Push(break_if.value);
                continue;
            }

            auto stmt = statement();
            if (stmt.errored) {
                return Failure::kErrored;
            }
            if (!stmt.matched) {
                break;
            }
            stmts.Push(stmt.value);
        }

        return stmts;
    });
    if (body.errored) {
        return Failure::kErrored;
    }

    return create<ast::BlockStatement>(source(), body.value, std::move(attrs.value));
}

// continuing_statement
//   : CONTINUING continuing_compound_statement
Maybe<const ast::BlockStatement*> Parser::continuing_statement() {
    if (!match(Token::Type::kContinuing)) {
        return create<ast::BlockStatement>(Source{}, tint::Empty, tint::Empty);
    }

    return continuing_compound_statement();
}

// primary_expression
//   : const_literal
//   | IDENT argument_expression_list?
//   | paren_expression
//
// Note, PAREN_LEFT ( expression ( COMMA expression ) * COMMA? )? PAREN_RIGHT is replaced
// with `argument_expression_list`.
Maybe<const ast::Expression*> Parser::primary_expression() {
    auto lit = const_literal();
    if (lit.errored) {
        return Failure::kErrored;
    }
    if (lit.matched) {
        return lit.value;
    }

    auto& t = peek();
    if (t.IsIdentifier()) {
        MultiTokenSource source(this);
        next();

        const ast::Identifier* ident = nullptr;

        if (peek_is(Token::Type::kTemplateArgsLeft)) {
            auto tmpl_args = expect_template_arg_block("template arguments", [&] {
                return expect_expression_list("template argument list",
                                              Token::Type::kTemplateArgsRight);
            });
            ident = builder_.Ident(source(), t.to_str(), std::move(tmpl_args.value));
        } else {
            ident = builder_.Ident(source(), t.to_str());
        }

        if (peek_is(Token::Type::kParenLeft)) {
            auto params = expect_argument_expression_list("function call");
            if (params.errored) {
                return Failure::kErrored;
            }

            return builder_.Call(source(), ident, std::move(params.value));
        }

        return builder_.Expr(ident);
    }

    if (t.Is(Token::Type::kParenLeft)) {
        auto paren = expect_paren_expression();
        if (paren.errored) {
            return Failure::kErrored;
        }

        return paren.value;
    }

    return Failure::kNoMatch;
}

// component_or_swizzle_specifier
//   :
//   | BRACE_LEFT expression BRACE_RIGHT component_or_swizzle_specifier?
//   | PERIOD member_ident component_or_swizzle_specifier?
//   | PERIOD swizzle_name component_or_swizzle_specifier?
Maybe<const ast::Expression*> Parser::component_or_swizzle_specifier(
    const ast::Expression* prefix) {
    MultiTokenSource source(this, prefix->source);

    while (continue_parsing()) {
        if (match(Token::Type::kBracketLeft)) {
            auto res = sync(Token::Type::kBracketRight, [&]() -> Maybe<const ast::Expression*> {
                auto param = expression();
                if (param.errored) {
                    return Failure::kErrored;
                }
                if (!param.matched) {
                    return AddError(peek(), "unable to parse expression inside []");
                }

                if (!expect("index accessor", Token::Type::kBracketRight)) {
                    return Failure::kErrored;
                }

                return create<ast::IndexAccessorExpression>(source(), prefix, param.value);
            });

            if (res.errored) {
                return res;
            }
            prefix = res.value;
            continue;
        }

        if (match(Token::Type::kPeriod)) {
            auto ident = expect_ident("member accessor");
            if (ident.errored) {
                return Failure::kErrored;
            }

            prefix = builder_.MemberAccessor(source(), prefix, ident.value);
            continue;
        }

        return prefix;
    }

    return Failure::kErrored;
}

// argument_expression_list
//   : PAREN_LEFT ((expression COMMA)* expression COMMA?)? PAREN_RIGHT
Expect<Parser::ExpressionList> Parser::expect_argument_expression_list(std::string_view use) {
    return expect_paren_block(use, [&]() -> Expect<Parser::ExpressionList> {  //
        auto list = expression_list(use, Token::Type::kParenRight);
        if (list.errored) {
            return Failure::kErrored;
        }
        return list.value;
    });
}

// bitwise_expression.post.unary_expression
//   : AND unary_expression (AND unary_expression)*
//   | OR unary_expression (OR unary_expression)*
//   | XOR unary_expression (XOR unary_expression)*
Maybe<const ast::Expression*> Parser::bitwise_expression_post_unary_expression(
    const ast::Expression* lhs,
    const Source& lhs_source) {
    MultiTokenSource source(this, lhs_source);
    auto& t = peek();

    std::optional<core::BinaryOp> op;
    switch (t.type()) {
        case Token::Type::kAnd:
            op = core::BinaryOp::kAnd;
            break;
        case Token::Type::kOr:
            op = core::BinaryOp::kOr;
            break;
        case Token::Type::kXor:
            op = core::BinaryOp::kXor;
            break;
        default:
            return Failure::kNoMatch;
    }
    next();  // Consume t

    while (continue_parsing()) {
        auto rhs = unary_expression();
        if (rhs.errored) {
            return Failure::kErrored;
        }
        if (!rhs.matched) {
            return AddError(peek(), std::string("unable to parse right side of ") +
                                        std::string(t.to_name()) + " expression");
        }

        lhs = create<ast::BinaryExpression>(source(), *op, lhs, rhs.value);

        if (!match(t.type())) {
            return lhs;
        }
    }
    return Failure::kErrored;
}

// multiplicative_operator
//   : FORWARD_SLASH
//   | MODULO
//   | STAR
Maybe<core::BinaryOp> Parser::multiplicative_operator() {
    if (match(Token::Type::kForwardSlash)) {
        return core::BinaryOp::kDivide;
    }
    if (match(Token::Type::kMod)) {
        return core::BinaryOp::kModulo;
    }
    if (match(Token::Type::kStar)) {
        return core::BinaryOp::kMultiply;
    }

    return Failure::kNoMatch;
}

// multiplicative_expression.post.unary_expression
//   : (multiplicative_operator unary_expression)*
Expect<const ast::Expression*> Parser::expect_multiplicative_expression_post_unary_expression(
    const ast::Expression* lhs,
    const Source& lhs_source) {
    MultiTokenSource source(this, lhs_source);
    while (continue_parsing()) {
        auto& t = peek();

        auto op = multiplicative_operator();
        if (op.errored) {
            return Failure::kErrored;
        }
        if (!op.matched) {
            return lhs;
        }

        auto rhs = unary_expression();
        if (rhs.errored) {
            return Failure::kErrored;
        }
        if (!rhs.matched) {
            return AddError(peek(), std::string("unable to parse right side of ") +
                                        std::string(t.to_name()) + " expression");
        }

        lhs = create<ast::BinaryExpression>(source(), op.value, lhs, rhs.value);
    }
    return Failure::kErrored;
}

// additive_operator
//   : MINUS
//   | PLUS
//
// Note, this also splits a `--` token. This is currently safe as the only way to get into
// here is through additive expression and rules for where `--` are allowed are very restrictive.
Maybe<core::BinaryOp> Parser::additive_operator() {
    if (match(Token::Type::kPlus)) {
        return core::BinaryOp::kAdd;
    }

    auto& t = peek();
    if (t.Is(Token::Type::kMinusMinus)) {
        next();
        split_token(Token::Type::kMinus, Token::Type::kMinus);
    } else if (t.Is(Token::Type::kMinus)) {
        next();
    } else {
        return Failure::kNoMatch;
    }

    return core::BinaryOp::kSubtract;
}

// additive_expression.pos.unary_expression
//   : (additive_operator unary_expression expect_multiplicative_expression.post.unary_expression)*
//
// This is `( additive_operator unary_expression ( multiplicative_operator unary_expression )* )*`
// split apart.
Expect<const ast::Expression*> Parser::expect_additive_expression_post_unary_expression(
    const ast::Expression* lhs,
    const Source& lhs_source) {
    MultiTokenSource source(this, lhs_source);
    while (continue_parsing()) {
        auto& t = peek();

        auto op = additive_operator();
        if (op.errored) {
            return Failure::kErrored;
        }
        if (!op.matched) {
            return lhs;
        }

        auto unary = unary_expression();
        if (unary.errored) {
            return Failure::kErrored;
        }
        if (!unary.matched) {
            return AddError(peek(), std::string("unable to parse right side of ") +
                                        std::string(t.to_name()) + " expression");
        }

        // The multiplicative binds tighter, so pass the unary into that and build that expression
        // before creating the additive expression.
        auto rhs = expect_multiplicative_expression_post_unary_expression(unary.value, lhs_source);
        if (rhs.errored) {
            return Failure::kErrored;
        }

        lhs = create<ast::BinaryExpression>(source(), op.value, lhs, rhs.value);
    }
    return Failure::kErrored;
}

// math_expression.post.unary_expression
//   : multiplicative_expression.post.unary_expression additive_expression.post.unary_expression
//
// This is `( multiplicative_operator unary_expression )* ( additive_operator unary_expression (
// multiplicative_operator unary_expression )* )*` split apart.
Expect<const ast::Expression*> Parser::expect_math_expression_post_unary_expression(
    const ast::Expression* lhs,
    const Source& lhs_source) {
    MultiTokenSource source(this, lhs_source);

    auto rhs = expect_multiplicative_expression_post_unary_expression(lhs, source);
    if (rhs.errored) {
        return Failure::kErrored;
    }

    return expect_additive_expression_post_unary_expression(rhs.value, source());
}

// shift_expression
//   : unary_expression shift_expression.post.unary_expression
Maybe<const ast::Expression*> Parser::shift_expression() {
    MultiTokenSource source(this);
    auto lhs = unary_expression();
    if (lhs.errored) {
        return Failure::kErrored;
    }
    if (!lhs.matched) {
        return Failure::kNoMatch;
    }
    return expect_shift_expression_post_unary_expression(lhs.value, source);
}

// shift_expression.post.unary_expression
//   : math_expression.post.unary_expression?
//   | SHIFT_LEFT unary_expression
//   | SHIFT_RIGHT unary_expression
//
// Note, add the `math_expression.post.unary_expression` is added here to make
// implementation simpler.
Expect<const ast::Expression*> Parser::expect_shift_expression_post_unary_expression(
    const ast::Expression* lhs,
    const Source& lhs_source) {
    MultiTokenSource source(this, lhs_source);

    auto& t = peek();
    if (match(Token::Type::kShiftLeft) || match(Token::Type::kShiftRight)) {
        std::string name;
        std::optional<core::BinaryOp> op;
        if (t.Is(Token::Type::kShiftLeft)) {
            op = core::BinaryOp::kShiftLeft;
            name = "<<";
        } else if (t.Is(Token::Type::kShiftRight)) {
            op = core::BinaryOp::kShiftRight;
            name = ">>";
        }

        auto& rhs_start = peek();
        auto rhs = unary_expression();
        if (rhs.errored) {
            return Failure::kErrored;
        }
        if (!rhs.matched) {
            return AddError(rhs_start,
                            std::string("unable to parse right side of ") + name + " expression");
        }
        return create<ast::BinaryExpression>(source(), *op, lhs, rhs.value);
    }

    return expect_math_expression_post_unary_expression(lhs, source);
}

// relational_expression
//   : unary_expression relational_expression.post.unary_expression
Maybe<const ast::Expression*> Parser::relational_expression() {
    MultiTokenSource source(this);
    auto lhs = unary_expression();
    if (lhs.errored) {
        return Failure::kErrored;
    }
    if (!lhs.matched) {
        return Failure::kNoMatch;
    }
    return expect_relational_expression_post_unary_expression(lhs.value, source);
}

// relational_expression.post.unary_expression
//   : shift_expression.post.unary_expression
//   | shift_expression.post.unary_expression EQUAL_EQUAL shift_expression
//   | shift_expression.post.unary_expression GREATER_THAN shift_expression
//   | shift_expression.post.unary_expression GREATER_THAN_EQUAL shift_expression
//   | shift_expression.post.unary_expression LESS_THAN shift_expression
//   | shift_expression.post.unary_expression LESS_THAN_EQUAL shift_expression
//   | shift_expression.post.unary_expression NOT_EQUAL shift_expression
//
// Note, a `shift_expression` element was added to simplify many of the right sides
Expect<const ast::Expression*> Parser::expect_relational_expression_post_unary_expression(
    const ast::Expression* lhs,
    const Source& lhs_source) {
    MultiTokenSource source(this, lhs_source);

    auto lhs_result = expect_shift_expression_post_unary_expression(lhs, source);
    if (lhs_result.errored) {
        return Failure::kErrored;
    }
    lhs = lhs_result.value;

    auto& tok_op = peek();

    std::optional<core::BinaryOp> op;
    switch (tok_op.type()) {
        case Token::Type::kLessThan:
            op = core::BinaryOp::kLessThan;
            break;
        case Token::Type::kGreaterThan:
            op = core::BinaryOp::kGreaterThan;
            break;
        case Token::Type::kLessThanEqual:
            op = core::BinaryOp::kLessThanEqual;
            break;
        case Token::Type::kGreaterThanEqual:
            op = core::BinaryOp::kGreaterThanEqual;
            break;
        case Token::Type::kEqualEqual:
            op = core::BinaryOp::kEqual;
            break;
        case Token::Type::kNotEqual:
            op = core::BinaryOp::kNotEqual;
            break;
        default:
            return lhs;
    }

    next();  // consume tok_op

    auto& tok_rhs = peek();
    auto rhs = shift_expression();
    if (rhs.errored) {
        return Failure::kErrored;
    }
    if (!rhs.matched) {
        return AddError(tok_rhs, std::string("unable to parse right side of ") +
                                     std::string(tok_op.to_name()) + " expression");
    }

    return create<ast::BinaryExpression>(source(), *op, lhs, rhs.value);
}

Expect<const ast::Expression*> Parser::expect_expression(std::string_view use) {
    auto& t = peek();
    auto expr = expression();
    if (expr.errored) {
        return Failure::kErrored;
    }
    if (expr.matched) {
        return expr.value;
    }
    return AddError(t, "expected expression for " + std::string(use));
}

Maybe<Parser::ExpressionList> Parser::expression_list(std::string_view use,
                                                      Token::Type terminator) {
    if (peek_is(terminator)) {
        return Failure::kNoMatch;
    }
    auto list = expect_expression_list(use, terminator);
    if (list.errored) {
        return Failure::kErrored;
    }
    return list.value;
}

Expect<Parser::ExpressionList> Parser::expect_expression_list(std::string_view use,
                                                              Token::Type terminator) {
    Parser::ExpressionList exprs;
    while (continue_parsing()) {
        auto expr = expect_expression(use);
        if (expr.errored) {
            return Failure::kErrored;
        }
        exprs.Push(expr.value);
        if (peek_is(terminator)) {
            break;
        }

        // Check if the next token is a template start, which was likely intended as a less-than.
        if (expect_next_not_template_list(expr->source).errored) {
            return Failure::kErrored;  // expect_next_not_template_list() raised an error.
        }
        if (!match(Token::Type::kComma)) {
            // Next expression is not a terminator or comma, so this is a parse error.

            // Check if last parsed expression was a templated identifier, which was likely indented
            // as a less-than / greater-than.
            if (expect_not_templated_ident_expr(expr.value).errored) {
                return Failure::kErrored;  // expect_not_templated_ident_expr() raised an error.
            }

            // Emit the expected ',' error
            expect(use, Token::Type::kComma);
            return Failure::kErrored;
        }
        if (peek_is(terminator)) {
            break;
        }
    }
    return exprs;
}

// expression
//   : unary_expression bitwise_expression.post.unary_expression
//   | unary_expression relational_expression.post.unary_expression
//   | unary_expression relational_expression.post.unary_expression and_and
//        relational_expression ( and_and relational_expression )*
//   | unary_expression relational_expression.post.unary_expression or_or
//        relational_expression ( or_or relational_expression )*
//
// Note, a `relational_expression` element was added to simplify many of the right sides
Maybe<const ast::Expression*> Parser::expression() {
    MultiTokenSource source(this);
    Source first_op;
    auto expr = [&]() -> Maybe<const ast::Expression*> {
        auto lhs = unary_expression();
        if (lhs.errored) {
            return Failure::kErrored;
        }
        if (!lhs.matched) {
            return Failure::kNoMatch;
        }

        first_op = peek().source();

        auto bitwise = bitwise_expression_post_unary_expression(lhs.value, source);
        if (bitwise.errored) {
            return Failure::kErrored;
        }
        if (bitwise.matched) {
            return bitwise.value;
        }

        auto relational = expect_relational_expression_post_unary_expression(lhs.value, source);
        if (relational.errored) {
            return Failure::kErrored;
        }
        auto* ret = relational.value;

        auto& t = peek();
        if (t.Is(Token::Type::kAndAnd) || t.Is(Token::Type::kOrOr)) {
            core::BinaryOp op;
            if (t.Is(Token::Type::kAndAnd)) {
                op = core::BinaryOp::kLogicalAnd;
            } else if (t.Is(Token::Type::kOrOr)) {
                op = core::BinaryOp::kLogicalOr;
            }

            while (continue_parsing()) {
                auto& n = peek();
                if (!n.Is(t.type())) {
                    break;
                }
                next();

                auto rhs = relational_expression();
                if (rhs.errored) {
                    return Failure::kErrored;
                }
                if (!rhs.matched) {
                    return AddError(peek(), std::string("unable to parse right side of ") +
                                                std::string(t.to_name()) + " expression");
                }

                ret = create<ast::BinaryExpression>(source(), op, ret, rhs.value);
            }
        }
        return ret;
    }();

    if (expr.matched) {
        // Note, expression is greedy an will consume all the operators of the same type
        // so, `a & a & a` would all be consumed above. If you see any binary operator
        // after this then it _must_ be a different one, and hence an error.
        if (auto* lhs = expr->As<ast::BinaryExpression>()) {
            if (auto& n = peek(); n.IsBinaryOperator()) {
                AddError(Source::Combine(first_op, n.source()),
                         std::string("mixing '") + ast::Operator(lhs->op) + "' and '" +
                             std::string(n.to_name()) + "' requires parenthesis");
                return Failure::kErrored;
            }
        }
    }

    return expr;
}

// singular_expression
//   : primary_expression postfix_expr
Maybe<const ast::Expression*> Parser::singular_expression() {
    auto prefix = primary_expression();
    if (prefix.errored) {
        return Failure::kErrored;
    }
    if (!prefix.matched) {
        return Failure::kNoMatch;
    }

    return component_or_swizzle_specifier(prefix.value);
}

// unary_expression
//   : singular_expression
//   | MINUS unary_expression
//   | BANG unary_expression
//   | TILDE unary_expression
//   | STAR unary_expression
//   | AND unary_expression
//
// The `primary_expression component_or_swizzle_specifier ?` is moved out into a
// `singular_expression`
Maybe<const ast::Expression*> Parser::unary_expression() {
    MultiTokenSource source(this);

    auto& t = peek();
    if (match(Token::Type::kPlusPlus) || match(Token::Type::kMinusMinus)) {
        AddError(source,
                 "prefix increment and decrement operators are reserved for a "
                 "future WGSL version");
        return Failure::kErrored;
    }

    core::UnaryOp op;
    if (match(Token::Type::kMinus)) {
        op = core::UnaryOp::kNegation;
    } else if (match(Token::Type::kBang)) {
        op = core::UnaryOp::kNot;
    } else if (match(Token::Type::kTilde)) {
        op = core::UnaryOp::kComplement;
    } else if (match(Token::Type::kStar)) {
        op = core::UnaryOp::kIndirection;
    } else if (match(Token::Type::kAnd)) {
        op = core::UnaryOp::kAddressOf;
    } else {
        return singular_expression();
    }

    if (parse_depth_ >= kMaxParseDepth) {
        // We've hit a maximum parser recursive depth.
        // We can't call into unary_expression() as we might stack overflow.
        // Instead, report an error
        AddError(peek(), "maximum parser recursive depth reached");
        return Failure::kErrored;
    }

    ++parse_depth_;
    auto expr = unary_expression();
    --parse_depth_;

    if (expr.errored) {
        return Failure::kErrored;
    }
    if (!expr.matched) {
        return AddError(
            peek(), "unable to parse right side of " + std::string(t.to_name()) + " expression");
    }

    return create<ast::UnaryOpExpression>(source(), op, expr.value);
}

// compound_assignment_operator
//   : plus_equal
//   | minus_equal
//   | times_equal
//   | division_equal
//   | modulo_equal
//   | and_equal
//   | or_equal
//   | xor_equal
//   | shift_right_equal
//   | shift_left_equal
Maybe<core::BinaryOp> Parser::compound_assignment_operator() {
    std::optional<core::BinaryOp> compound_op;
    if (peek_is(Token::Type::kPlusEqual)) {
        compound_op = core::BinaryOp::kAdd;
    } else if (peek_is(Token::Type::kMinusEqual)) {
        compound_op = core::BinaryOp::kSubtract;
    } else if (peek_is(Token::Type::kTimesEqual)) {
        compound_op = core::BinaryOp::kMultiply;
    } else if (peek_is(Token::Type::kDivisionEqual)) {
        compound_op = core::BinaryOp::kDivide;
    } else if (peek_is(Token::Type::kModuloEqual)) {
        compound_op = core::BinaryOp::kModulo;
    } else if (peek_is(Token::Type::kAndEqual)) {
        compound_op = core::BinaryOp::kAnd;
    } else if (peek_is(Token::Type::kOrEqual)) {
        compound_op = core::BinaryOp::kOr;
    } else if (peek_is(Token::Type::kXorEqual)) {
        compound_op = core::BinaryOp::kXor;
    } else if (peek_is(Token::Type::kShiftLeftEqual)) {
        compound_op = core::BinaryOp::kShiftLeft;
    } else if (peek_is(Token::Type::kShiftRightEqual)) {
        compound_op = core::BinaryOp::kShiftRight;
    }
    if (compound_op) {
        next();
        return *compound_op;
    }
    return Failure::kNoMatch;
}

// core_lhs_expression
//   : ident
//   | PAREN_LEFT lhs_expression PAREN_RIGHT
Maybe<const ast::Expression*> Parser::core_lhs_expression() {
    auto& t = peek();
    if (t.IsIdentifier()) {
        next();

        return builder_.Expr(t.source(), t.to_str());
    }

    if (peek_is(Token::Type::kParenLeft)) {
        return expect_paren_block("", [&]() -> Expect<const ast::Expression*> {
            auto expr = lhs_expression();
            if (expr.errored) {
                return Failure::kErrored;
            }
            if (!expr.matched) {
                return AddError(t, "invalid expression");
            }
            return expr.value;
        });
    }

    return Failure::kNoMatch;
}

// lhs_expression
//   : core_lhs_expression component_or_swizzle_specifier ?
//   | AND lhs_expression
//   | STAR lhs_expression
Maybe<const ast::Expression*> Parser::lhs_expression() {
    auto core_expr = core_lhs_expression();
    if (core_expr.errored) {
        return Failure::kErrored;
    }
    if (core_expr.matched) {
        return component_or_swizzle_specifier(core_expr.value);
    }

    // Gather up all the `*`, `&` and `&&` tokens into a list and create all of the unary ops at
    // once instead of recursing. This handles the case where the fuzzer decides >8k `*`s would be
    // fun.
    struct LHSData {
        Source source;
        core::UnaryOp op;
    };
    Vector<LHSData, 4> ops;
    while (true) {
        auto& t = peek();
        if (!t.Is(Token::Type::kAndAnd) && !t.Is(Token::Type::kAnd) && !t.Is(Token::Type::kStar)) {
            break;
        }
        next();  // consume the peek

        if (t.Is(Token::Type::kAndAnd)) {
            // The first `&` is consumed as part of the `&&`, so we only push one of the two `&`s.
            split_token(Token::Type::kAnd, Token::Type::kAnd);
            ops.Push({t.source(), core::UnaryOp::kAddressOf});
        } else if (t.Is(Token::Type::kAnd)) {
            ops.Push({t.source(), core::UnaryOp::kAddressOf});
        } else if (t.Is(Token::Type::kStar)) {
            ops.Push({t.source(), core::UnaryOp::kIndirection});
        }
    }
    if (ops.IsEmpty()) {
        return Failure::kNoMatch;
    }

    auto& t = peek();
    auto expr = lhs_expression();
    if (expr.errored) {
        return Failure::kErrored;
    }
    if (!expr.matched) {
        return AddError(t, "missing expression");
    }

    const ast::Expression* ret = expr.value;
    // Consume the ops in reverse order so we have the correct AST ordering.
    for (auto& info : tint::Reverse(ops)) {
        ret = create<ast::UnaryOpExpression>(info.source, info.op, ret);
    }
    return ret;
}

// variable_updating_statement
//   : lhs_expression ( EQUAL | compound_assignment_operator ) expression
//   | lhs_expression MINUS_MINUS
//   | lhs_expression PLUS_PLUS
//   | UNDERSCORE EQUAL expression
//
// Note, this is a simplification of the recursive grammar statement with the `lhs_expression`
// substituted back into the expression.
Maybe<const ast::Statement*> Parser::variable_updating_statement() {
    auto& t = peek();

    // tint:295 - Test for `ident COLON` - this is invalid grammar, and without
    // special casing will error as "missing = for assignment", which is less
    // helpful than this error message:
    if (peek_is(Token::Type::kIdentifier) && peek_is(Token::Type::kColon, 1)) {
        return AddError(peek(0).source(), "expected 'var' for variable declaration");
    }

    Source source;
    const ast::Expression* lhs = nullptr;
    std::optional<core::BinaryOp> compound_op;
    if (peek_is(Token::Type::kUnderscore)) {
        next();  // Consume the peek.

        if (!expect("assignment", Token::Type::kEqual)) {
            return Failure::kErrored;
        }
        source = last_source();

        lhs = create<ast::PhonyExpression>(t.source());

    } else {
        auto lhs_result = lhs_expression();
        if (lhs_result.errored) {
            return Failure::kErrored;
        }
        if (!lhs_result.matched) {
            return Failure::kNoMatch;
        }

        lhs = lhs_result.value;

        // Handle increment and decrement statements.
        if (match(Token::Type::kPlusPlus)) {
            return create<ast::IncrementDecrementStatement>(last_source(), lhs, true);
        }
        if (match(Token::Type::kMinusMinus)) {
            return create<ast::IncrementDecrementStatement>(last_source(), lhs, false);
        }

        source = peek().source();
        auto compound_op_result = compound_assignment_operator();
        if (compound_op_result.errored) {
            return Failure::kErrored;
        }
        if (compound_op_result.matched) {
            compound_op = compound_op_result.value;
        } else {
            if (!expect("assignment", Token::Type::kEqual)) {
                return Failure::kErrored;
            }
        }
    }

    auto rhs = expression();
    if (rhs.errored) {
        return Failure::kErrored;
    }
    if (!rhs.matched) {
        return AddError(peek(), "unable to parse right side of assignment");
    }

    if (compound_op) {
        return create<ast::CompoundAssignmentStatement>(source, lhs, rhs.value, *compound_op);
    }
    return create<ast::AssignmentStatement>(source, lhs, rhs.value);
}

// const_literal
//   : INT_LITERAL
//   | FLOAT_LITERAL
//   | bool_literal
//
// bool_literal
//   : TRUE
//   | FALSE
Maybe<const ast::LiteralExpression*> Parser::const_literal() {
    auto& t = peek();
    if (match(Token::Type::kIntLiteral)) {
        return create<ast::IntLiteralExpression>(t.source(), t.to_i64(),
                                                 ast::IntLiteralExpression::Suffix::kNone);
    }
    if (match(Token::Type::kIntLiteral_I)) {
        return create<ast::IntLiteralExpression>(t.source(), t.to_i64(),
                                                 ast::IntLiteralExpression::Suffix::kI);
    }
    if (match(Token::Type::kIntLiteral_U)) {
        return create<ast::IntLiteralExpression>(t.source(), t.to_i64(),
                                                 ast::IntLiteralExpression::Suffix::kU);
    }
    if (match(Token::Type::kFloatLiteral)) {
        return create<ast::FloatLiteralExpression>(t.source(), t.to_f64(),
                                                   ast::FloatLiteralExpression::Suffix::kNone);
    }
    if (match(Token::Type::kFloatLiteral_F)) {
        return create<ast::FloatLiteralExpression>(t.source(), t.to_f64(),
                                                   ast::FloatLiteralExpression::Suffix::kF);
    }
    if (match(Token::Type::kFloatLiteral_H)) {
        return create<ast::FloatLiteralExpression>(t.source(), t.to_f64(),
                                                   ast::FloatLiteralExpression::Suffix::kH);
    }
    if (match(Token::Type::kTrue)) {
        return create<ast::BoolLiteralExpression>(t.source(), true);
    }
    if (match(Token::Type::kFalse)) {
        return create<ast::BoolLiteralExpression>(t.source(), false);
    }
    if (handle_error(t)) {
        return Failure::kErrored;
    }
    return Failure::kNoMatch;
}

Maybe<Parser::AttributeList> Parser::attribute_list() {
    bool errored = false;
    AttributeList attrs;

    while (continue_parsing()) {
        if (match(Token::Type::kAttr)) {
            if (auto attr = expect_attribute(); attr.errored) {
                errored = true;
            } else {
                attrs.Push(attr.value);
            }
        } else {
            break;
        }
    }

    if (errored) {
        return Failure::kErrored;
    }

    if (attrs.IsEmpty()) {
        return Failure::kNoMatch;
    }

    return attrs;
}

Expect<const ast::Attribute*> Parser::expect_attribute() {
    auto& t = peek();
    auto attr = attribute();
    if (attr.errored) {
        return Failure::kErrored;
    }
    if (attr.matched) {
        return attr.value;
    }
    return AddError(t, "expected attribute");
}

// attribute
//   : ATTR identifier ( PAREN_LEFT expression ( COMMA expression )? COMMA? PAREN_RIGHT )?
Maybe<const ast::Attribute*> Parser::attribute() {
    // Note, the ATTR is matched by the called `attribute_list` in this case, so it is not matched
    // here and this has to be an attribute.
    auto& t = peek();

    if (match(Token::Type::kConst)) {
        return AddError(t.source(), "const attribute may not appear in shaders");
    }
    if (match(Token::Type::kDiagnostic)) {
        auto control = expect_diagnostic_control();
        if (control.errored) {
            return Failure::kErrored;
        }
        return create<ast::DiagnosticAttribute>(t.source(), std::move(control.value));
    }

    auto attr = expect_enum("attribute", core::ParseAttribute, core::kAttributeStrings);
    if (attr.errored) {
        return Failure::kErrored;
    }

    uint32_t min = 1;
    uint32_t max = 1;
    switch (attr.value) {
        case core::Attribute::kCompute:
        case core::Attribute::kFragment:
        case core::Attribute::kInvariant:
        case core::Attribute::kMustUse:
        case core::Attribute::kVertex:
            min = 0;
            max = 0;
            break;
        case core::Attribute::kInterpolate:
            max = 2;
            break;
        case core::Attribute::kWorkgroupSize:
            max = 3;
            break;
        default:
            break;
    }

    // builtin_attr :
    //   '@' 'builtin' '(' builtin_value_name ',' ? ')'
    if (attr.value == core::Attribute::kBuiltin) {
        return expect_paren_block(
            "builtin attribute", [&]() -> Expect<const ast::BuiltinAttribute*> {
                auto name = expect_enum("builtin value name", core::ParseBuiltinValue,
                                        core::kBuiltinValueStrings);
                if (name.errored) {
                    return Failure::kErrored;
                }
                match(Token::Type::kComma);

                return builder_.Builtin(t.source(), name.value);
            });
    }

    // interpolate_attr :
    //   '@' 'interpolate' '(' interpolate_type_name ',' ? ')'
    // | '@' 'interpolate' '(' interpolate_type_name ',' interpolate_sampling_name ',' ? ')'
    if (attr.value == core::Attribute::kInterpolate) {
        return expect_paren_block(
            "interpolate attribute", [&]() -> Expect<const ast::InterpolateAttribute*> {
                auto type_name =
                    expect_enum("interpolation type name", core::ParseInterpolationType,
                                core::kInterpolationTypeStrings);
                if (type_name.errored) {
                    return Failure::kErrored;
                }
                if (!match(Token::Type::kComma) || peek().Is(Token::Type::kParenRight)) {
                    return builder_.Interpolate(t.source(), type_name.value);
                }

                auto sampling_name =
                    expect_enum("interpolation sampling name", core::ParseInterpolationSampling,
                                core::kInterpolationSamplingStrings);
                if (sampling_name.errored) {
                    return Failure::kErrored;
                }
                match(Token::Type::kComma);

                return builder_.Interpolate(t.source(), type_name.value, sampling_name.value);
            });
    }

    Vector<const ast::Expression*, 2> args;

    // Handle no parameter items which should have no parens
    if (min == 0) {
        auto& t2 = peek();
        if (match(Token::Type::kParenLeft)) {
            return AddError(t2.source(), t.to_str() + " attribute doesn't take parenthesis");
        }
    } else {
        auto res = expect_paren_block(t.to_str() + " attribute", [&]() -> Expect<bool> {
            while (continue_parsing()) {
                if (peek().Is(Token::Type::kParenRight)) {
                    break;
                }

                auto expr = expect_expression(t.to_str());
                if (expr.errored) {
                    return Failure::kErrored;
                }
                args.Push(expr.value);

                if (!match(Token::Type::kComma)) {
                    break;
                }
            }
            return true;
        });
        if (res.errored) {
            return Failure::kErrored;
        }

        if (args.IsEmpty() || args.Length() < min) {
            return AddError(t.source(),
                            t.to_str() + " expects" + (min != max ? " at least " : " ") +
                                std::to_string(min) + " argument" + (min != 1 ? "s" : ""));
        }
        if (args.Length() > max) {
            return AddError(t.source(), t.to_str() + " expects" + (min != max ? " at most " : " ") +
                                            std::to_string(max) + " argument" +
                                            (max != 1 ? "s" : "") + ", got " +
                                            std::to_string(args.Length()));
        }
    }

    switch (attr.value) {
        case core::Attribute::kAlign:
            return create<ast::StructMemberAlignAttribute>(t.source(), args[0]);
        case core::Attribute::kBinding:
            return create<ast::BindingAttribute>(t.source(), args[0]);
        case core::Attribute::kBlendSrc:
            return create<ast::BlendSrcAttribute>(t.source(), args[0]);
        case core::Attribute::kColor:
            return create<ast::ColorAttribute>(t.source(), args[0]);
        case core::Attribute::kCompute:
            return create<ast::StageAttribute>(t.source(), ast::PipelineStage::kCompute);
        case core::Attribute::kFragment:
            return create<ast::StageAttribute>(t.source(), ast::PipelineStage::kFragment);
        case core::Attribute::kGroup:
            return create<ast::GroupAttribute>(t.source(), args[0]);
        case core::Attribute::kId:
            return create<ast::IdAttribute>(t.source(), args[0]);
        case core::Attribute::kInputAttachmentIndex:
            return create<ast::InputAttachmentIndexAttribute>(t.source(), args[0]);
        case core::Attribute::kInvariant:
            return create<ast::InvariantAttribute>(t.source());
        case core::Attribute::kLocation:
            return builder_.Location(t.source(), args[0]);
        case core::Attribute::kMustUse:
            return create<ast::MustUseAttribute>(t.source());
        case core::Attribute::kSize:
            return builder_.MemberSize(t.source(), args[0]);
        case core::Attribute::kVertex:
            return create<ast::StageAttribute>(t.source(), ast::PipelineStage::kVertex);
        case core::Attribute::kWorkgroupSize:
            return create<ast::WorkgroupAttribute>(t.source(), args[0],
                                                   args.Length() > 1 ? args[1] : nullptr,
                                                   args.Length() > 2 ? args[2] : nullptr);
        default:
            return Failure::kNoMatch;
    }
}

Expect<Void> Parser::expect_attributes_consumed(VectorRef<const ast::Attribute*> in) {
    if (in.IsEmpty()) {
        return kSuccess;
    }
    AddError(in[0]->source, "unexpected attributes");
    return Failure::kErrored;
}

Expect<Void> Parser::expect_next_not_template_list(const Source& lhs_source) {
    Source end;
    if (!match(Token::Type::kTemplateArgsLeft, &end)) {
        return kSuccess;
    }

    // Try to find end of template
    for (size_t i = 0; i < 32; i++) {
        if (auto& t = peek(i); t.type() == Token::Type::kTemplateArgsRight) {
            end = t.source();
        }
    }
    Source template_source = lhs_source;
    template_source.range.end = end.range.end;
    AddError(template_source, "parsed as template list");

    if (auto rhs = expression(); rhs.matched) {
        Source lt_source = lhs_source;
        lt_source.range.end = rhs->source.range.end;
        AddNote(lt_source,
                "if this is intended to be a less-than expression then wrap in parentheses");
    }
    return Failure::kErrored;
}

Expect<Void> Parser::expect_not_templated_ident_expr(const ast::Expression* expr) {
    auto* ident_expr = expr->As<ast::IdentifierExpression>();
    if (!ident_expr) {
        return kSuccess;
    }
    auto* ident = ident_expr->identifier->As<ast::TemplatedIdentifier>();
    if (!ident) {
        return kSuccess;
    }

    AddError(ident->source, "parsed as template list");

    if (auto rhs = expression(); rhs.matched) {
        Source gt_source = ident->arguments.Back()->source;
        gt_source.range.end = rhs->source.range.end;
        AddNote(gt_source,
                "if this is intended to be a greater-than expression then wrap in parentheses");
    }
    return Failure::kErrored;
}

// severity_control_name
//   : 'error'
//   | 'warning'
//   | 'info'
//   | 'off'
Expect<wgsl::DiagnosticSeverity> Parser::expect_severity_control_name() {
    return expect_enum("severity control", wgsl::ParseDiagnosticSeverity,
                       wgsl::kDiagnosticSeverityStrings);
}

// diagnostic_control
// : PAREN_LEFT severity_control_name COMMA diagnostic_rule_name COMMA ? PAREN_RIGHT
Expect<ast::DiagnosticControl> Parser::expect_diagnostic_control() {
    return expect_paren_block("diagnostic control", [&]() -> Expect<ast::DiagnosticControl> {
        auto severity_control = expect_severity_control_name();
        if (severity_control.errored) {
            return Failure::kErrored;
        }

        if (!expect("diagnostic control", Token::Type::kComma)) {
            return Failure::kErrored;
        }

        auto rule_name = expect_diagnostic_rule_name();
        if (rule_name.errored) {
            return Failure::kErrored;
        }
        match(Token::Type::kComma);

        return ast::DiagnosticControl(severity_control.value, rule_name.value);
    });
}

// diagnostic_rule_name :
// | diagnostic_name_token
// | diagnostic_name_token '.' diagnostic_name_token
Expect<const ast::DiagnosticRuleName*> Parser::expect_diagnostic_rule_name() {
    if (peek_is(Token::Type::kPeriod, 1)) {
        auto category = expect_ident("", "diagnostic rule category");
        if (category.errored) {
            return Failure::kErrored;
        }
        if (!expect("diagnostic rule", Token::Type::kPeriod)) {
            return Failure::kErrored;
        }
        auto name = expect_ident("", "diagnostic rule name");
        if (name.errored) {
            return Failure::kErrored;
        }
        return builder_.DiagnosticRuleName(category.value, name.value);
    }
    auto name = expect_ident("", "diagnostic rule name");
    if (name.errored) {
        return Failure::kErrored;
    }
    return builder_.DiagnosticRuleName(name.value);
}

bool Parser::match(Token::Type tok, Source* source /*= nullptr*/) {
    auto& t = peek();

    if (source != nullptr) {
        *source = t.source();
    }

    if (t.Is(tok)) {
        next();
        return true;
    }
    return false;
}

bool Parser::expect(std::string_view use, Token::Type tok) {
    auto& t = peek();
    if (t.Is(tok)) {
        next();
        synchronized_ = true;
        return true;
    }

    // Special case to split `>>` and `>=` tokens if we are looking for a `>`.
    if (tok == Token::Type::kGreaterThan &&
        (t.Is(Token::Type::kShiftRight) || t.Is(Token::Type::kGreaterThanEqual))) {
        next();

        // Push the second character to the token queue.
        if (t.Is(Token::Type::kShiftRight)) {
            split_token(Token::Type::kGreaterThan, Token::Type::kGreaterThan);
        } else if (t.Is(Token::Type::kGreaterThanEqual)) {
            split_token(Token::Type::kGreaterThan, Token::Type::kEqual);
        }

        synchronized_ = true;
        return true;
    }

    // Error cases
    synchronized_ = false;
    if (handle_error(t)) {
        return false;
    }

    StringStream err;
    if (tok == Token::Type::kTemplateArgsLeft && t.type() == Token::Type::kLessThan) {
        err << "missing closing '>'";
    } else {
        err << "expected '" << Token::TypeToName(tok) << "'";
    }
    if (!use.empty()) {
        err << " for " << use;
    }
    AddError(t, err.str());
    return false;
}

Expect<int32_t> Parser::expect_sint(std::string_view use, Source* source /* = nullptr */) {
    auto& t = peek();
    if (source) {
        *source = t.source();
    }
    if (!t.Is(Token::Type::kIntLiteral) && !t.Is(Token::Type::kIntLiteral_I)) {
        return AddError(t.source(), "expected signed integer literal", use);
    }

    int64_t val = t.to_i64();
    if ((val > std::numeric_limits<int32_t>::max()) ||
        (val < std::numeric_limits<int32_t>::min())) {
        // TODO(crbug.com/tint/1504): Test this when abstract int is implemented
        return AddError(t.source(), "value overflows i32", use);
    }

    next();
    return static_cast<int32_t>(t.to_i64());
}

Expect<uint32_t> Parser::expect_positive_sint(std::string_view use) {
    Source source;
    auto sint = expect_sint(use, &source);
    if (sint.errored) {
        return Failure::kErrored;
    }

    if (sint.value < 0) {
        return AddError(source, std::string(use) + " must be positive");
    }

    return static_cast<uint32_t>(sint.value);
}

Expect<uint32_t> Parser::expect_nonzero_positive_sint(std::string_view use) {
    Source source;
    auto sint = expect_sint(use, &source);
    if (sint.errored) {
        return Failure::kErrored;
    }

    if (sint.value <= 0) {
        return AddError(source, std::string(use) + " must be greater than 0");
    }

    return static_cast<uint32_t>(sint.value);
}

Expect<const ast::Identifier*> Parser::expect_ident(std::string_view use,
                                                    std::string_view kind /* = "identifier" */) {
    auto& t = peek();
    if (t.IsIdentifier()) {
        synchronized_ = true;
        next();

        if (is_reserved(t)) {
            return AddError(t.source(), "'" + t.to_str() + "' is a reserved keyword");
        }

        return builder_.Ident(t.source(), t.to_str());
    }
    if (handle_error(t)) {
        return Failure::kErrored;
    }
    synchronized_ = false;
    return AddError(t.source(), "expected " + std::string(kind), use);
}

template <typename F, typename T>
T Parser::expect_block(Token::Type start, Token::Type end, std::string_view use, F&& body) {
    if (!expect(use, start)) {
        return Failure::kErrored;
    }

    return sync(end, [&]() -> T {
        auto res = body();

        if (res.errored) {
            return Failure::kErrored;
        }

        if (!expect(use, end)) {
            return Failure::kErrored;
        }

        return res;
    });
}

template <typename F, typename T>
T Parser::expect_paren_block(std::string_view use, F&& body) {
    return expect_block(Token::Type::kParenLeft, Token::Type::kParenRight, use,
                        std::forward<F>(body));
}

template <typename F, typename T>
T Parser::expect_brace_block(std::string_view use, F&& body) {
    return expect_block(Token::Type::kBraceLeft, Token::Type::kBraceRight, use,
                        std::forward<F>(body));
}

template <typename F, typename T>
T Parser::expect_lt_gt_block(std::string_view use, F&& body) {
    return expect_block(Token::Type::kLessThan, Token::Type::kGreaterThan, use,
                        std::forward<F>(body));
}

template <typename F, typename T>
T Parser::expect_template_arg_block(std::string_view use, F&& body) {
    return expect_block(Token::Type::kTemplateArgsLeft, Token::Type::kTemplateArgsRight, use,
                        std::forward<F>(body));
}

template <typename F, typename T>
T Parser::sync(Token::Type tok, F&& body) {
    if (parse_depth_ >= kMaxParseDepth) {
        // We've hit a maximum parser recursive depth.
        // We can't call into body() as we might stack overflow.
        // Instead, report an error...
        AddError(peek(), "maximum parser recursive depth reached");
        // ...and try to resynchronize. If we cannot resynchronize to `tok` then
        // synchronized_ is set to false, and the parser knows that forward progress
        // is not being made.
        sync_to(tok, /* consume: */ true);
        return Failure::kErrored;
    }

    sync_tokens_.push_back(tok);

    ++parse_depth_;
    auto result = body();
    --parse_depth_;

    if (DAWN_UNLIKELY(sync_tokens_.back() != tok)) {
        TINT_ICE() << "sync_tokens is out of sync";
    }
    sync_tokens_.pop_back();

    if (result.errored) {
        sync_to(tok, /* consume: */ true);
    }

    return result;
}

bool Parser::sync_to(Token::Type tok, bool consume) {
    // Clear the synchronized state - gets set to true again on success.
    synchronized_ = false;

    BlockCounters counters;

    for (size_t i = 0; i < kMaxResynchronizeLookahead; i++) {
        auto& t = peek(i);
        if (counters.consume(t) > 0) {
            continue;  // Nested block
        }
        if (!t.Is(tok) && !is_sync_token(t)) {
            continue;  // Not a synchronization point
        }

        // Synchronization point found.

        // Skip any tokens we don't understand, bringing us to just before the
        // resync point.
        while (i-- > 0) {
            next();
        }

        // Is this synchronization token |tok|?
        if (t.Is(tok)) {
            if (consume) {
                next();
            }
            synchronized_ = true;
            return true;
        }
        break;
    }

    return false;
}

bool Parser::is_sync_token(const Token& t) const {
    for (auto r : sync_tokens_) {
        if (t.Is(r)) {
            return true;
        }
    }
    return false;
}

bool Parser::handle_error(const Token& t) {
    // The token might itself be an error.
    if (t.IsError()) {
        synchronized_ = false;
        AddError(t.source(), t.to_str());
        return true;
    }
    return false;
}

template <typename F, typename T>
T Parser::without_diag(F&& body) {
    silence_diags_++;
    auto result = body();
    silence_diags_--;
    return result;
}

}  // namespace tint::wgsl::reader
