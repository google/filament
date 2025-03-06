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

#ifndef SRC_TINT_LANG_WGSL_READER_PARSER_PARSER_H_
#define SRC_TINT_LANG_WGSL_READER_PARSER_PARSER_H_

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/parser/detail.h"
#include "src/tint/lang/wgsl/reader/parser/token.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/utils/diagnostic/formatter.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::ast {
class BreakStatement;
class CallStatement;
class ContinueStatement;
class IfStatement;
class LoopStatement;
class ReturnStatement;
class SwitchStatement;
class VariableDeclStatement;
}  // namespace tint::ast

namespace tint::wgsl::reader {

class Lexer;

/// Struct holding information for a for loop
struct ForHeader {
    /// Constructor
    /// @param init the initializer statement
    /// @param cond the condition statement
    /// @param cont the continuing statement
    ForHeader(const ast::Statement* init, const ast::Expression* cond, const ast::Statement* cont);

    ~ForHeader();

    /// The for loop initializer
    const ast::Statement* initializer = nullptr;
    /// The for loop condition
    const ast::Expression* condition = nullptr;
    /// The for loop continuing statement
    const ast::Statement* continuing = nullptr;
};

/// Parser for WGSL source data
class Parser {
    /// Failure holds enumerator values used for the constructing an Expect and
    /// Match in an errored state.
    struct Failure {
        enum Errored { kErrored };
        enum NoMatch { kNoMatch };
    };

  public:
    /// Pre-determined small vector sizes for AST pointers
    //! @cond Doxygen_Suppress
    using AttributeList = Vector<const ast::Attribute*, 4>;
    using CaseSelectorList = Vector<const ast::CaseSelector*, 4>;
    using CaseStatementList = Vector<const ast::CaseStatement*, 4>;
    using ExpressionList = Vector<const ast::Expression*, 8>;
    using ParameterList = Vector<const ast::Parameter*, 8>;
    using StatementList = Vector<const ast::Statement*, 8>;
    using StructMemberList = Vector<const ast::StructMember*, 8>;
    //! @endcond

    /// Empty structure used by functions that do not return a value, but need to signal success /
    /// error with Expect<Void> or Maybe<NoError>.
    struct Void {};

    /// Expect is the return type of the parser methods that are expected to
    /// return a parsed value of type T, unless there was an parse error.
    /// In the case of a parse error the called method will have called
    /// AddError() and #errored will be set to true.
    template <typename T>
    struct Expect {
        /// An alias to the templated type T.
        using type = T;

        /// Don't allow an Expect to take a nullptr.
        inline Expect(std::nullptr_t) = delete;  // NOLINT

        /// Constructor for a successful parse.
        /// @param val the result value of the parse
        template <typename U>
        inline Expect(U&& val)  // NOLINT
            : value(std::forward<U>(val)) {}

        /// Constructor for parse error.
        inline Expect(Failure::Errored) : errored(true) {}  // NOLINT

        /// Copy constructor
        inline Expect(const Expect&) = default;
        /// Move constructor
        inline Expect(Expect&&) = default;
        /// Assignment operator
        /// @return this Expect
        inline Expect& operator=(const Expect&) = default;
        /// Assignment move operator
        /// @return this Expect
        inline Expect& operator=(Expect&&) = default;

        /// @return a pointer to the returned value. If T is a pointer or
        /// std::unique_ptr, operator->() automatically dereferences so that the
        /// return type will always be a pointer to a non-pointer type. #errored
        /// must be false to call.
        inline typename detail::OperatorArrow<T>::type operator->() {
            TINT_ASSERT(!errored);
            return detail::OperatorArrow<T>::ptr(value);
        }

        /// The expected value of a successful parse.
        /// Zero-initialized when there was a parse error.
        T value{};
        /// True if there was a error parsing.
        bool errored = false;
    };

    /// Maybe is the return type of the parser methods that attempts to match a
    /// grammar and return a parsed value of type T, or may parse part of the
    /// grammar and then hit a parse error.
    /// In the case of a successful grammar match, the Maybe will have #matched
    /// set to true.
    /// In the case of a parse error the called method will have called
    /// AddError() and the Maybe will have #errored set to true.
    template <typename T>
    struct Maybe {
        inline Maybe(std::nullptr_t) = delete;  // NOLINT

        /// Constructor for a successful parse.
        /// @param val the result value of the parse
        template <typename U>
        inline Maybe(U&& val)  // NOLINT
            : value(std::forward<U>(val)), matched(true) {}

        /// Constructor for parse error state.
        inline Maybe(Failure::Errored) : errored(true) {}  // NOLINT

        /// Constructor for the no-match state.
        inline Maybe(Failure::NoMatch) {}  // NOLINT

        /// Constructor from an Expect.
        /// @param e the Expect to copy this Maybe from
        template <typename U>
        inline Maybe(const Expect<U>& e)  // NOLINT
            : value(e.value), errored(e.errored), matched(!e.errored) {}

        /// Move from an Expect.
        /// @param e the Expect to move this Maybe from
        template <typename U>
        inline Maybe(Expect<U>&& e)  // NOLINT
            : value(std::move(e.value)), errored(e.errored), matched(!e.errored) {}

        /// Copy constructor
        inline Maybe(const Maybe&) = default;
        /// Move constructor
        inline Maybe(Maybe&&) = default;
        /// Assignment operator
        /// @return this Maybe
        inline Maybe& operator=(const Maybe&) = default;
        /// Assignment move operator
        /// @return this Maybe
        inline Maybe& operator=(Maybe&&) = default;

        /// @return a pointer to the returned value. If T is a pointer or
        /// std::unique_ptr, operator->() automatically dereferences so that the
        /// return type will always be a pointer to a non-pointer type. #errored
        /// must be false to call.
        inline typename detail::OperatorArrow<T>::type operator->() {
            TINT_ASSERT(!errored);
            return detail::OperatorArrow<T>::ptr(value);
        }

        /// The value of a successful parse.
        /// Zero-initialized when there was a parse error.
        T value{};
        /// True if there was a error parsing.
        bool errored = false;
        /// True if there was a error parsing.
        bool matched = false;
    };

    /// TypedIdentifier holds a parsed identifier and type. Returned by
    /// variable_ident_decl().
    struct TypedIdentifier {
        /// Constructor
        TypedIdentifier();
        /// Copy constructor
        /// @param other the FunctionHeader to copy
        TypedIdentifier(const TypedIdentifier& other);
        /// Constructor
        /// @param type_in parsed type
        /// @param name_in parsed identifier
        TypedIdentifier(ast::Type type_in, const ast::Identifier* name_in);
        /// Destructor
        ~TypedIdentifier();

        /// Parsed type. type.expr be nullptr for inferred types.
        ast::Type type;
        /// Parsed identifier.
        const ast::Identifier* name = nullptr;
    };

    /// FunctionHeader contains the parsed information for a function header.
    struct FunctionHeader {
        /// Constructor
        FunctionHeader();
        /// Copy constructor
        /// @param other the FunctionHeader to copy
        FunctionHeader(const FunctionHeader& other);
        /// Constructor
        /// @param src parsed header source
        /// @param n function name
        /// @param p function parameters
        /// @param ret_ty function return type
        /// @param ret_attrs return type attributes
        FunctionHeader(Source src,
                       const ast::Identifier* n,
                       VectorRef<const ast::Parameter*> p,
                       ast::Type ret_ty,
                       VectorRef<const ast::Attribute*> ret_attrs);
        /// Destructor
        ~FunctionHeader();
        /// Assignment operator
        /// @param other the FunctionHeader to copy
        /// @returns this FunctionHeader
        FunctionHeader& operator=(const FunctionHeader& other);

        /// Parsed header source
        Source source;
        /// Function name
        const ast::Identifier* name;
        /// Function parameters
        Vector<const ast::Parameter*, 8> params;
        /// Function return type
        ast::Type return_type;
        /// Function return type attributes
        AttributeList return_type_attributes;
    };

    /// VarDeclInfo contains the parsed information for variable declaration.
    struct VarDeclInfo {
        /// Variable declaration source
        Source source;
        /// Variable name
        const ast::Identifier* name = nullptr;
        /// Variable address space
        const ast::Expression* address_space = nullptr;
        /// Variable access control
        const ast::Expression* access = nullptr;
        /// Variable type
        ast::Type type;
    };

    /// VariableQualifier contains the parsed information for a variable qualifier
    struct VariableQualifier {
        /// The variable's address space
        const ast::Expression* address_space = nullptr;
        /// The variable's access control
        const ast::Expression* access = nullptr;
    };

    /// MatrixDimensions contains the column and row information for a matrix
    struct MatrixDimensions {
        /// The number of columns
        uint32_t columns = 0;
        /// The number of rows
        uint32_t rows = 0;
    };

    /// Creates a new parser using the given file
    /// @param file the input source file to parse
    explicit Parser(Source::File const* file);
    ~Parser();

    /// Reads tokens from the source file. This will be called automatically
    /// by |parse|.
    void InitializeLex();

    /// Run the parser
    /// @returns true if the parse was successful, false otherwise.
    bool Parse();

    /// set_max_diagnostics sets the maximum number of reported errors before
    /// aborting parsing.
    /// @param limit the new maximum number of errors
    void set_max_errors(size_t limit) { max_errors_ = limit; }

    /// @return the number of maximum number of reported errors before aborting
    /// parsing.
    size_t get_max_errors() const { return max_errors_; }

    /// @returns true if an error was encountered.
    bool has_error() const { return builder_.Diagnostics().ContainsErrors(); }

    /// @returns the parser error string
    std::string error() const {
        diag::Formatter formatter{{false, false, false, false}};
        return formatter.Format(builder_.Diagnostics()).Plain();
    }

    /// @returns the Program. The program builder in the parser will be reset
    /// after this.
    Program program() { return resolver::Resolve(builder_); }

    /// @returns the program builder.
    ProgramBuilder& builder() { return builder_; }

    /// @returns the next token
    const Token& next();
    /// Peeks ahead and returns the token at `idx` ahead of the current position
    /// @param idx the index of the token to return
    /// @returns the token `idx` positions ahead without advancing
    const Token& peek(size_t idx = 0);
    /// Peeks ahead and returns true if the token at `idx` ahead of the current
    /// position is |tok|
    /// @param idx the index of the token to return
    /// @param tok the token to look for
    /// @returns true if the token `idx` positions ahead is |tok|
    bool peek_is(Token::Type tok, size_t idx = 0);
    /// @returns the last source location that was returned by `next()`
    Source last_source() const;
    /// Appends an error at `t` with the message `msg`
    /// @param t the token to associate the error with
    /// @param msg the error message
    /// @return `Failure::Errored::kError` so that you can combine an AddError()
    /// call and return on the same line.
    Failure::Errored AddError(const Token& t, std::string_view msg);
    /// Appends an error raised when parsing `use` at `t` with the message
    /// `msg`
    /// @param source the source to associate the error with
    /// @param msg the error message
    /// @param use a description of what was being parsed when the error was
    /// raised.
    /// @return `Failure::Errored::kError` so that you can combine an AddError()
    /// call and return on the same line.
    Failure::Errored AddError(const Source& source, std::string_view msg, std::string_view use);
    /// Appends an error at `source` with the message `msg`
    /// @param source the source to associate the error with
    /// @param msg the error message
    /// @return `Failure::Errored::kError` so that you can combine an AddError()
    /// call and return on the same line.
    Failure::Errored AddError(const Source& source, std::string_view msg);
    /// Appends an error at `source` with the message `msg`
    /// @param source the source to associate the error with
    /// @param msg the error message
    /// @return `Failure::Errored::kError` so that you can combine an AddError()
    /// call and return on the same line.
    Failure::Errored AddError(const Source& source, StyledText&& msg);
    /// Appends a note at `source` with the message `msg`
    /// @param source the source to associate the error with
    /// @param msg the note message
    void AddNote(const Source& source, std::string_view msg);
    /// Appends a deprecated-language-feature warning at `source` with the message
    /// `msg`
    /// @param source the source to associate the error with
    /// @param msg the warning message
    void deprecated(const Source& source, std::string_view msg);
    /// Parses the `translation_unit` grammar element
    void translation_unit();
    /// Parses the `global_directive` grammar element, erroring on parse failure.
    /// @param has_parsed_decl flag indicating if the parser has consumed a global declaration.
    /// @return true on parse success, otherwise an error or no-match.
    Maybe<Void> global_directive(bool has_parsed_decl);
    /// Parses the `diagnostic_directive` grammar element, erroring on parse failure.
    /// @return true on parse success, otherwise an error or no-match.
    Maybe<Void> diagnostic_directive();
    /// Parses the `enable_directive` grammar element, erroring on parse failure.
    /// @return true on parse success, otherwise an error or no-match.
    Maybe<Void> enable_directive();
    /// Parses the `requires_directive` grammar element, erroring on parse failure.
    /// @return true on parse success, otherwise an error or no-match.
    Maybe<Void> requires_directive();
    /// Parses the `global_decl` grammar element, erroring on parse failure.
    /// @return true on parse success, otherwise an error or no-match.
    Maybe<Void> global_decl();
    /// Parses a `global_variable_decl` grammar element with the initial
    /// `variable_attribute_list*` provided as `attrs`
    /// @returns the variable parsed or nullptr
    /// @param attrs the list of attributes for the variable declaration. If attributes are consumed
    ///        by the declaration, then this vector is cleared before returning.
    Maybe<const ast::Variable*> global_variable_decl(AttributeList& attrs);
    /// Parses a `global_constant_decl` grammar element with the initial
    /// `variable_attribute_list*` provided as `attrs`
    /// @returns the const object or nullptr
    /// @param attrs the list of attributes for the constant declaration. If attributes are consumed
    ///        by the declaration, then this vector is cleared before returning.
    Maybe<const ast::Variable*> global_constant_decl(AttributeList& attrs);
    /// Parses a `variable_decl` grammar element
    /// @returns the parsed variable declaration info
    Maybe<VarDeclInfo> variable_decl();
    /// Helper for parsing ident with an optional type declaration. Should not be called directly,
    /// use the specific version below.
    /// @param use a description of what was being parsed if an error was raised.
    /// @param allow_inferred allow the identifier to be parsed without a type
    /// @returns the parsed identifier, and possibly type, or empty otherwise
    Expect<TypedIdentifier> expect_ident_with_optional_type_specifier(std::string_view use,
                                                                      bool allow_inferred);
    /// Parses a `ident` or a `variable_ident_decl` grammar element, erroring on parse failure.
    /// @param use a description of what was being parsed if an error was raised.
    /// @returns the identifier or empty otherwise.
    Expect<TypedIdentifier> expect_optionally_typed_ident(std::string_view use);
    /// Parses a `variable_ident_decl` grammar element, erroring on parse failure.
    /// @param use a description of what was being parsed if an error was raised.
    /// @returns the identifier and type parsed or empty otherwise
    Expect<TypedIdentifier> expect_ident_with_type_specifier(std::string_view use);
    /// Parses a `variable_qualifier` grammar element
    /// @returns the variable qualifier information
    Maybe<VariableQualifier> variable_qualifier();
    /// Parses a `type_alias_decl` grammar element
    /// @returns the type alias or nullptr on error
    Maybe<const ast::Alias*> type_alias_decl();
    /// Parses a `type_specifier` grammar element
    /// @returns the parsed Type or nullptr if none matched.
    Maybe<ast::Type> type_specifier();
    /// Parses a `struct_decl` grammar element.
    /// @returns the struct type or nullptr on error
    Maybe<const ast::Struct*> struct_decl();
    /// Parses a `struct_body_decl` grammar element, erroring on parse failure.
    /// @returns the struct members
    Expect<StructMemberList> expect_struct_body_decl();
    /// Parses a `struct_member` grammar element, erroring on parse failure.
    /// @returns the struct member or nullptr
    Expect<const ast::StructMember*> expect_struct_member();
    /// Parses a `function_decl` grammar element with the initial
    /// `function_attribute_decl*` provided as `attrs`.
    /// @param attrs the list of attributes for the function declaration. If attributes are consumed
    ///        by the declaration, then this vector is cleared before returning.
    /// @returns the parsed function, nullptr otherwise
    Maybe<const ast::Function*> function_decl(AttributeList& attrs);
    /// Parses a `const_assert_statement` grammar element
    /// @returns returns the const assert, if it matched.
    Maybe<const ast::ConstAssert*> const_assert_statement();
    /// Parses a `function_header` grammar element
    /// @returns the parsed function header
    Maybe<FunctionHeader> function_header();
    /// Parses a `param_list` grammar element, erroring on parse failure.
    /// @returns the parsed variables
    Expect<ParameterList> expect_param_list();
    /// Parses a `param` grammar element, erroring on parse failure.
    /// @returns the parsed variable
    Expect<const ast::Parameter*> expect_param();
    /// Parses a `pipeline_stage` grammar element, erroring if the next token does
    /// not match a stage name.
    /// @returns the pipeline stage.
    Expect<ast::PipelineStage> expect_pipeline_stage();
    /// Parses a `compound_statement` grammar element, erroring on parse failure.
    /// @param use a description of what was being parsed if an error was raised
    /// @returns the parsed statements
    Expect<ast::BlockStatement*> expect_compound_statement(std::string_view use);
    /// Parses a `compound_statement` grammar element, with the attribute list provided as `attrs`.
    /// @param attrs the list of attributes for the statement
    /// @param use a description of what was being parsed if an error was raised
    /// @returns the parsed statements
    Expect<ast::BlockStatement*> expect_compound_statement(AttributeList& attrs,
                                                           std::string_view use);
    /// Parses a `paren_expression` grammar element, erroring on parse failure.
    /// @returns the parsed element or nullptr
    Expect<const ast::Expression*> expect_paren_expression();
    /// Parses a `statements` grammar element
    /// @returns the statements parsed
    Expect<StatementList> expect_statements();
    /// Parses a `statement` grammar element
    /// @returns the parsed statement or nullptr
    Maybe<const ast::Statement*> statement();
    /// Parses a `break_statement` grammar element
    /// @returns the parsed statement or nullptr
    Maybe<const ast::BreakStatement*> break_statement();
    /// Parses a `return_statement` grammar element
    /// @returns the parsed statement or nullptr
    Maybe<const ast::ReturnStatement*> return_statement();
    /// Parses a `continue_statement` grammar element
    /// @returns the parsed statement or nullptr
    Maybe<const ast::ContinueStatement*> continue_statement();
    /// Parses a `variable_statement` grammar element
    /// @returns the parsed variable or nullptr
    Maybe<const ast::VariableDeclStatement*> variable_statement();
    /// Parses a `if_statement` grammar element, with the attribute list provided as `attrs`.
    /// @param attrs the list of attributes for the statement
    /// @returns the parsed statement or nullptr
    Maybe<const ast::IfStatement*> if_statement(AttributeList& attrs);
    /// Parses a `switch_statement` grammar element
    /// @param attrs the list of attributes for the statement
    /// @returns the parsed statement or nullptr
    Maybe<const ast::SwitchStatement*> switch_statement(AttributeList& attrs);
    /// Parses a `switch_body` grammar element
    /// @returns the parsed statement or nullptr
    Maybe<const ast::CaseStatement*> switch_body();
    /// Parses a `case_selectors` grammar element
    /// @returns the list of literals
    Expect<CaseSelectorList> expect_case_selectors();
    /// Parses a `case_selector` grammar element
    /// @returns the selector
    Maybe<const ast::CaseSelector*> case_selector();
    /// Parses a `func_call_statement` grammar element
    /// @returns the parsed function call or nullptr
    Maybe<const ast::CallStatement*> func_call_statement();
    /// Parses a `loop_statement` grammar element, with the attribute list provided as `attrs`.
    /// @param attrs the list of attributes for the statement
    /// @returns the parsed loop or nullptr
    Maybe<const ast::LoopStatement*> loop_statement(AttributeList& attrs);
    /// Parses a `for_header` grammar element, erroring on parse failure.
    /// @returns the parsed for header or nullptr
    Expect<std::unique_ptr<ForHeader>> expect_for_header();
    /// Parses a `for_statement` grammar element, with the attribute list provided as `attrs`.
    /// @param attrs the list of attributes for the statement
    /// @returns the parsed for loop or nullptr
    Maybe<const ast::ForLoopStatement*> for_statement(AttributeList& attrs);
    /// Parses a `while_statement` grammar element, with the attribute list provided as `attrs`.
    /// @param attrs the list of attributes for the statement
    /// @returns the parsed while loop or nullptr
    Maybe<const ast::WhileStatement*> while_statement(AttributeList& attrs);
    /// Parses a `break_if_statement` grammar element
    /// @returns the parsed statement or nullptr
    Maybe<const ast::Statement*> break_if_statement();
    /// Parses a `continuing_compound_statement` grammar element
    /// @returns the parsed statements
    Maybe<const ast::BlockStatement*> continuing_compound_statement();
    /// Parses a `continuing_statement` grammar element
    /// @returns the parsed statements
    Maybe<const ast::BlockStatement*> continuing_statement();
    /// Parses a `const_literal` grammar element
    /// @returns the const literal parsed or nullptr if none found
    Maybe<const ast::LiteralExpression*> const_literal();
    /// Parses a `primary_expression` grammar element
    /// @returns the parsed expression or nullptr
    Maybe<const ast::Expression*> primary_expression();
    /// Parses a `argument_expression_list` grammar element, erroring on parse
    /// failure.
    /// @param use a description of what was being parsed if an error was raised
    /// @returns the list of arguments
    Expect<ExpressionList> expect_argument_expression_list(std::string_view use);
    /// Parses the recursive portion of the component_or_swizzle_specifier
    /// @param prefix the left side of the expression
    /// @returns the parsed expression or nullptr
    Maybe<const ast::Expression*> component_or_swizzle_specifier(const ast::Expression* prefix);
    /// Parses a `singular_expression` grammar elment
    /// @returns the parsed expression or nullptr
    Maybe<const ast::Expression*> singular_expression();
    /// Parses a `unary_expression` grammar element
    /// @returns the parsed expression or nullptr
    Maybe<const ast::Expression*> unary_expression();
    /// Parses the `expression` grammar rule
    /// @returns the parsed expression or nullptr
    Maybe<const ast::Expression*> expression();
    /// Parses the `expression` grammar rule
    /// @param use the use of the expression
    /// @returns the parsed expression or error
    Expect<const ast::Expression*> expect_expression(std::string_view use);
    /// Parses a comma separated expression list
    /// @param use the use of the expression list
    /// @param terminator the terminating token for the list
    /// @returns the parsed expression list or error
    Maybe<Parser::ExpressionList> expression_list(std::string_view use, Token::Type terminator);
    /// Parses a comma separated expression list, with at least one expression
    /// @param use the use of the expression list
    /// @param terminator the terminating token for the list
    /// @returns the parsed expression list or error
    Expect<Parser::ExpressionList> expect_expression_list(std::string_view use,
                                                          Token::Type terminator);
    /// Parses the `bitwise_expression.post.unary_expression` grammar element
    /// @param lhs the left side of the expression
    /// @param lhs_source the source span for the left side of the expression
    /// @returns the parsed expression or nullptr
    Maybe<const ast::Expression*> bitwise_expression_post_unary_expression(
        const ast::Expression* lhs,
        const Source& lhs_source);
    /// Parse the `multiplicative_operator` grammar element
    /// @returns the parsed operator if successful
    Maybe<core::BinaryOp> multiplicative_operator();
    /// Parses multiplicative elements
    /// @param lhs the left side of the expression
    /// @param lhs_source the source span for the left side of the expression
    /// @returns the parsed expression or `lhs` if no match
    Expect<const ast::Expression*> expect_multiplicative_expression_post_unary_expression(
        const ast::Expression* lhs,
        const Source& lhs_source);
    /// Parses additive elements
    /// @param lhs the left side of the expression
    /// @param lhs_source the source span for the left side of the expression
    /// @returns the parsed expression or `lhs` if no match
    Expect<const ast::Expression*> expect_additive_expression_post_unary_expression(
        const ast::Expression* lhs,
        const Source& lhs_source);
    /// Parses math elements
    /// @param lhs the left side of the expression
    /// @param lhs_source the source span for the left side of the expression
    /// @returns the parsed expression or `lhs` if no match
    Expect<const ast::Expression*> expect_math_expression_post_unary_expression(
        const ast::Expression* lhs,
        const Source& lhs_source);
    /// Parses a `unary_expression shift.post.unary_expression`
    /// @returns the parsed expression or nullptr
    Maybe<const ast::Expression*> shift_expression();
    /// Parses a `shift_expression.post.unary_expression` grammar element
    /// @param lhs the left side of the expression
    /// @param lhs_source the source span for the left side of the expression
    /// @returns the parsed expression or `lhs` if no match
    Expect<const ast::Expression*> expect_shift_expression_post_unary_expression(
        const ast::Expression* lhs,
        const Source& lhs_source);
    /// Parses a `unary_expression relational_expression.post.unary_expression`
    /// @returns the parsed expression or nullptr
    Maybe<const ast::Expression*> relational_expression();
    /// Parses a `relational_expression.post.unary_expression` grammar element
    /// @param lhs the left side of the expression
    /// @param lhs_source the source span for the left side of the expression
    /// @returns the parsed expression or `lhs` if no match
    Expect<const ast::Expression*> expect_relational_expression_post_unary_expression(
        const ast::Expression* lhs,
        const Source& lhs_source);
    /// Parse the `additive_operator` grammar element
    /// @returns the parsed operator if successful
    Maybe<core::BinaryOp> additive_operator();
    /// Parses a `compound_assignment_operator` grammar element
    /// @returns the parsed compound assignment operator
    Maybe<core::BinaryOp> compound_assignment_operator();
    /// Parses a `core_lhs_expression` grammar element
    /// @returns the parsed expression or a non-kMatched failure
    Maybe<const ast::Expression*> core_lhs_expression();
    /// Parses a `lhs_expression` grammar element
    /// @returns the parsed expression or a non-kMatched failure
    Maybe<const ast::Expression*> lhs_expression();
    /// Parses a `variable_updating_statement` grammar element
    /// @returns the parsed assignment or nullptr
    Maybe<const ast::Statement*> variable_updating_statement();
    /// Parses one or more attribute lists.
    /// @return the parsed attribute list, or an empty list on error.
    Maybe<AttributeList> attribute_list();
    /// Parses a single attribute of the following types:
    /// * `struct_attribute`
    /// * `struct_member_attribute`
    /// * `array_attribute`
    /// * `variable_attribute`
    /// * `global_const_attribute`
    /// * `function_attribute`
    /// @return the parsed attribute, or nullptr.
    Maybe<const ast::Attribute*> attribute();
    /// Parses a single attribute, reporting an error if the next token does not
    /// represent a attribute.
    /// @see #attribute for the full list of attributes this method parses.
    /// @return the parsed attribute.
    Expect<const ast::Attribute*> expect_attribute();
    /// Parses a severity_control_name grammar element.
    /// @return the parsed severity control name.
    Expect<wgsl::DiagnosticSeverity> expect_severity_control_name();
    /// Parses a diagnostic_control grammar element.
    /// @return the parsed diagnostic control.
    Expect<ast::DiagnosticControl> expect_diagnostic_control();
    /// Parses a diagnostic_rule_name grammar element.
    /// @return the parsed diagnostic rule name.
    Expect<const ast::DiagnosticRuleName*> expect_diagnostic_rule_name();

    /// Splits a peekable token into to parts filling in the peekable fields.
    /// @param lhs the token to set in the current position
    /// @param rhs the token to set in the placeholder
    void split_token(Token::Type lhs, Token::Type rhs);

  private:
    /// ReturnType resolves to the return type for the function or lambda F.
    template <typename F>
    using ReturnType = typename std::invoke_result<F>::type;

    /// ResultType resolves to `T` for a `RESULT` of type Expect<T>.
    template <typename RESULT>
    using ResultType = typename RESULT::type;

    /// @returns true and consumes the next token if it equals `tok`
    /// @param source if not nullptr, the next token's source is written to this
    /// pointer, regardless of success or error
    bool match(Token::Type tok, Source* source = nullptr);
    /// Errors if the next token is not equal to `tok`
    /// Consumes the next token on match.
    /// expect() also updates #synchronized_, setting it to `true` if the next
    /// token is equal to `tok`, otherwise `false`.
    /// @param use a description of what was being parsed if an error was raised.
    /// @param tok the token to test against
    /// @returns true if the next token equals `tok`
    bool expect(std::string_view use, Token::Type tok);
    /// Parses a signed integer from the next token in the stream, erroring if the
    /// next token is not a signed integer.
    /// Consumes the next token on match.
    /// @param use a description of what was being parsed if an error was raised
    /// @returns the parsed integer.
    /// @param source if not nullptr, the next token's source is written to this
    /// pointer, regardless of success or error
    Expect<int32_t> expect_sint(std::string_view use, Source* source = nullptr);
    /// Parses a signed integer from the next token in the stream, erroring if
    /// the next token is not a signed integer or is negative.
    /// Consumes the next token if it is a signed integer (not necessarily
    /// negative).
    /// @param use a description of what was being parsed if an error was raised
    /// @returns the parsed integer.
    Expect<uint32_t> expect_positive_sint(std::string_view use);
    /// Parses a non-zero signed integer from the next token in the stream,
    /// erroring if the next token is not a signed integer or is less than 1.
    /// Consumes the next token if it is a signed integer (not necessarily
    /// >= 1).
    /// @param use a description of what was being parsed if an error was raised
    /// @returns the parsed integer.
    Expect<uint32_t> expect_nonzero_positive_sint(std::string_view use);
    /// Errors if the next token is not an identifier.
    /// Consumes the next token on match.
    /// @param use a description of what was being parsed if an error was raised
    /// @param kind a string describing the kind of identifier.
    ///             Examples: "identifier", "diagnostic name"
    /// @returns the parsed identifier.
    Expect<const ast::Identifier*> expect_ident(std::string_view use,
                                                std::string_view kind = "identifier");
    /// Parses a lexical block starting with the token `start` and ending with
    /// the token `end`. `body` is called to parse the lexical block body
    /// between the `start` and `end` tokens. If the `start` or `end` tokens
    /// are not matched then an error is generated and a zero-initialized `T` is
    /// returned. If `body` raises an error while parsing then a zero-initialized
    /// `T` is returned.
    /// @param start the token that begins the lexical block
    /// @param end the token that ends the lexical block
    /// @param use a description of what was being parsed if an error was raised
    /// @param body a function or lambda that is called to parse the lexical block
    /// body, with the signature: `Expect<Result>()` or `Maybe<Result>()`.
    /// @return the value returned by `body` if no errors are raised, otherwise
    /// an Expect with error state.
    template <typename F, typename T = ReturnType<F>>
    T expect_block(Token::Type start, Token::Type end, std::string_view use, F&& body);
    /// A convenience function that calls expect_block() passing
    /// `Token::Type::kParenLeft` and `Token::Type::kParenRight` for the `start`
    /// and `end` arguments, respectively.
    /// @param use a description of what was being parsed if an error was raised
    /// @param body a function or lambda that is called to parse the lexical block
    /// body, with the signature: `Expect<Result>()` or `Maybe<Result>()`.
    /// @return the value returned by `body` if no errors are raised, otherwise
    /// an Expect with error state.
    template <typename F, typename T = ReturnType<F>>
    T expect_paren_block(std::string_view use, F&& body);
    /// A convenience function that calls `expect_block` passing
    /// `Token::Type::kBraceLeft` and `Token::Type::kBraceRight` for the `start`
    /// and `end` arguments, respectively.
    /// @param use a description of what was being parsed if an error was raised
    /// @param body a function or lambda that is called to parse the lexical block
    /// body, with the signature: `Expect<Result>()` or `Maybe<Result>()`.
    /// @return the value returned by `body` if no errors are raised, otherwise
    /// an Expect with error state.
    template <typename F, typename T = ReturnType<F>>
    T expect_brace_block(std::string_view use, F&& body);
    /// A convenience function that calls `expect_block` passing
    /// `Token::Type::kLessThan` and `Token::Type::kGreaterThan` for the `start`
    /// and `end` arguments, respectively.
    /// @param use a description of what was being parsed if an error was raised
    /// @param body a function or lambda that is called to parse the lexical block
    /// body, with the signature: `Expect<Result>()` or `Maybe<Result>()`.
    /// @return the value returned by `body` if no errors are raised, otherwise
    /// an Expect with error state.
    template <typename F, typename T = ReturnType<F>>
    T expect_lt_gt_block(std::string_view use, F&& body);
    /// A convenience function that calls `expect_block` passing
    /// `Token::Type::kTemplateArgsLeft` and `Token::Type::kTemplateArgsRight` for the `start` and
    /// `end` arguments, respectively.
    /// @param use a description of what was being parsed if an error was raised
    /// @param body a function or lambda that is called to parse the lexical block body, with the
    /// signature: `Expect<Result>()` or `Maybe<Result>()`.
    /// @return the value returned by `body` if no errors are raised, otherwise an Expect with error
    /// state.
    template <typename F, typename T = ReturnType<F>>
    T expect_template_arg_block(std::string_view use, F&& body);

    /// sync() calls the function `func`, and attempts to resynchronize the
    /// parser to the next found resynchronization token if `func` fails. If the
    /// next found resynchronization token is `tok`, then sync will also consume
    /// `tok`.
    ///
    /// sync() will transiently add `tok` to the parser's stack of
    /// synchronization tokens for the duration of the call to `func`. Once @p
    /// func returns,
    /// `tok` is removed from the stack of resynchronization tokens. sync calls
    /// may be nested, and so the number of resynchronization tokens is equal to
    /// the number of sync() calls in the current stack frame.
    ///
    /// sync() updates #synchronized_, setting it to `true` if the next
    /// resynchronization token found was `tok`, otherwise `false`.
    ///
    /// @param tok the token to attempt to synchronize the parser to if `func`
    /// fails.
    /// @param func a function or lambda with the signature: `Expect<Result>()` or
    /// `Maybe<Result>()`.
    /// @return the value returned by `func`
    template <typename F, typename T = ReturnType<F>>
    T sync(Token::Type tok, F&& func);
    /// sync_to() attempts to resynchronize the parser to the next found
    /// resynchronization token or `tok` (whichever comes first).
    ///
    /// Synchronization tokens are transiently defined by calls to sync().
    ///
    /// sync_to() updates #synchronized_, setting it to `true` if a
    /// resynchronization token was found and it was `tok`, otherwise `false`.
    ///
    /// @param tok the token to attempt to synchronize the parser to.
    /// @param consume if true and the next found resynchronization token is
    /// `tok` then sync_to() will also consume `tok`.
    /// @return the state of #synchronized_.
    /// @see sync().
    bool sync_to(Token::Type tok, bool consume);
    /// @return true if `t` is in the stack of resynchronization tokens.
    /// @see sync().
    bool is_sync_token(const Token& t) const;

    /// If `t` is an error token, then `synchronized_` is set to false and the
    /// token's error is appended to the builder's diagnostics. If `t` is not an
    /// error token, then this function does nothing and false is returned.
    /// @returns true if `t` is an error, otherwise false.
    bool handle_error(const Token& t);

    /// @returns true if #synchronized_ is true and the number of reported errors
    /// is less than #max_errors_.
    bool continue_parsing() {
        return synchronized_ && builder_.Diagnostics().NumErrors() < max_errors_;
    }

    /// without_diag() calls the function `func` muting any diagnostics found while executing the
    /// function. This can be used to silence spew when attempting to resynchronize the parser.
    /// @param func a function or lambda with the signature: `Expect<Result>()` or
    /// `Maybe<Result>()`.
    /// @return the value returned by `func`
    template <typename F, typename T = ReturnType<F>>
    T without_diag(F&& func);

    /// Reports an error if the attribute list `list` is not empty.
    /// Used to ensure that all attributes are consumed.
    Expect<Void> expect_attributes_consumed(VectorRef<const ast::Attribute*> list);

    /// Raises an error if the next token is the start of a template list.
    /// Used to hint to the user that the parser interpreted the following as a templated identifier
    /// expression:
    ///
    /// ```
    /// a < b, c >
    ///   ^~~~~~~~
    /// ```
    Expect<Void> expect_next_not_template_list(const Source& lhs_source);

    /// Raises an error if the parsed expression is a templated identifier expression
    /// Used to hint to the user that the parser intepreted the following as a templated identifier
    /// expression:
    ///
    /// ```
    /// a < b, c > d
    /// ^^^^^^^^^^
    ///    expr
    /// ```
    Expect<Void> expect_not_templated_ident_expr(const ast::Expression* expr);

    /// Parses the given enum, providing sensible error messages if the next token does not match
    /// any of the enum values.
    /// @param name the name of the enumerator
    /// @param parse the optimized function used to parse the enum
    /// @param strings the list of possible strings in the enum
    /// @param use an optional description of what was being parsed if an error was raised.
    template <typename ENUM>
    Expect<ENUM> expect_enum(std::string_view name,
                             ENUM (*parse)(std::string_view str),
                             Slice<const std::string_view> strings,
                             std::string_view use = "");

    Expect<ast::Type> expect_type(std::string_view use);

    Maybe<const ast::Statement*> non_block_statement();
    Maybe<const ast::Statement*> for_header_initializer();
    Maybe<const ast::Statement*> for_header_continuing();

    class MultiTokenSource;

    /// Creates a new `ast::Node` owned by the Module. When the Module is
    /// destructed, the `ast::Node` will also be destructed.
    /// @param args the arguments to pass to the constructor
    /// @returns the node pointer
    template <typename T, typename... ARGS>
    T* create(ARGS&&... args) {
        return builder_.create<T>(std::forward<ARGS>(args)...);
    }

    Source::File const* const file_;
    std::vector<Token> tokens_;
    size_t next_token_idx_ = 0;
    size_t last_source_idx_ = 0;
    bool synchronized_ = true;
    uint32_t parse_depth_ = 0;
    std::vector<Token::Type> sync_tokens_;
    int silence_diags_ = 0;
    ProgramBuilder builder_;
    size_t max_errors_ = 25;
};

}  // namespace tint::wgsl::reader

#endif  // SRC_TINT_LANG_WGSL_READER_PARSER_PARSER_H_
