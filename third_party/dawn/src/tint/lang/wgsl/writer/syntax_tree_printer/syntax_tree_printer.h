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

#ifndef SRC_TINT_LANG_WGSL_WRITER_SYNTAX_TREE_PRINTER_SYNTAX_TREE_PRINTER_H_
#define SRC_TINT_LANG_WGSL_WRITER_SYNTAX_TREE_PRINTER_SYNTAX_TREE_PRINTER_H_

#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/utils/text_generator.h"

// Forward declarations
namespace tint::core {
enum class BinaryOp;
}
namespace tint::ast {
class AssignmentStatement;
class Attribute;
class BinaryExpression;
class BitcastExpression;
class BlockStatement;
class BlockStatement;
class BreakIfStatement;
class BreakStatement;
class CallExpression;
class CaseStatement;
class CompoundAssignmentStatement;
class ConstAssert;
class ContinueStatement;
struct DiagnosticControl;
class DiscardStatement;
class Enable;
class Expression;
class ForLoopStatement;
class Function;
class Identifier;
class IdentifierExpression;
class IfStatement;
class IncrementDecrementStatement;
class IndexAccessorExpression;
class LiteralExpression;
class LoopStatement;
class MemberAccessorExpression;
class ReturnStatement;
class Statement;
class Statement;
class Statement;
class Struct;
class SwitchStatement;
class TypeDecl;
class UnaryOpExpression;
class Variable;
class WhileStatement;
}  // namespace tint::ast
namespace tint::core {
enum class TexelFormat : uint8_t;
}  // namespace tint::core

namespace tint::wgsl::writer {

/// Implementation class for AST generator
class SyntaxTreePrinter : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param program the program
    explicit SyntaxTreePrinter(const Program& program);
    ~SyntaxTreePrinter() override;

    /// Generates the result data
    /// @returns true on success.
    bool Generate();

    /// Handles generating a diagnostic control
    /// @param diagnostic the diagnostic control node
    void EmitDiagnosticControl(const ast::DiagnosticControl& diagnostic);
    /// Handles generating an enable directive
    /// @param enable the enable node
    void EmitEnable(const ast::Enable* enable);
    /// Handles generating a declared type
    /// @param ty the declared type to generate
    void EmitTypeDecl(const ast::TypeDecl* ty);
    /// Handles an index accessor expression
    /// @param expr the expression to emit
    void EmitIndexAccessor(const ast::IndexAccessorExpression* expr);
    /// Handles an assignment statement
    /// @param stmt the statement to emit
    void EmitAssign(const ast::AssignmentStatement* stmt);
    /// Handles generating a binary expression
    /// @param expr the binary expression
    void EmitBinary(const ast::BinaryExpression* expr);
    /// Handles generating a binary operator
    /// @param op the binary operator
    void EmitBinaryOp(const core::BinaryOp op);
    /// Handles a block statement
    /// @param stmt the statement to emit
    void EmitBlock(const ast::BlockStatement* stmt);
    /// Handles emitting the start of a block statement (including attributes)
    /// @param stmt the block statement to emit the header for
    void EmitBlockHeader(const ast::BlockStatement* stmt);
    /// Handles a break statement
    /// @param stmt the statement to emit
    void EmitBreak(const ast::BreakStatement* stmt);
    /// Handles a break-if statement
    /// @param stmt the statement to emit
    void EmitBreakIf(const ast::BreakIfStatement* stmt);
    /// Handles generating a call expression
    /// @param expr the call expression
    void EmitCall(const ast::CallExpression* expr);
    /// Handles a case statement
    /// @param stmt the statement
    void EmitCase(const ast::CaseStatement* stmt);
    /// Handles a compound assignment statement
    /// @param stmt the statement to emit
    void EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt);
    /// Handles generating a literal expression
    /// @param expr the literal expression expression
    void EmitLiteral(const ast::LiteralExpression* expr);
    /// Handles a continue statement
    /// @param stmt the statement to emit
    void EmitContinue(const ast::ContinueStatement* stmt);
    /// Handles generate an Expression
    /// @param expr the expression
    void EmitExpression(const ast::Expression* expr);
    /// Handles generating a function
    /// @param func the function to generate
    void EmitFunction(const ast::Function* func);
    /// Handles generating an identifier expression
    /// @param expr the identifier expression
    void EmitIdentifier(const ast::IdentifierExpression* expr);
    /// Handles generating an identifier
    /// @param ident the identifier
    void EmitIdentifier(const ast::Identifier* ident);
    /// Handles an if statement
    /// @param stmt the statement to emit
    void EmitIf(const ast::IfStatement* stmt);
    /// Handles an increment/decrement statement
    /// @param stmt the statement to emit
    void EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt);
    /// Handles generating a discard statement
    /// @param stmt the discard statement
    void EmitDiscard(const ast::DiscardStatement* stmt);
    /// Handles a loop statement
    /// @param stmt the statement to emit
    void EmitLoop(const ast::LoopStatement* stmt);
    /// Handles a for-loop statement
    /// @param stmt the statement to emit
    void EmitForLoop(const ast::ForLoopStatement* stmt);
    /// Handles a while statement
    /// @param stmt the statement to emit
    void EmitWhile(const ast::WhileStatement* stmt);
    /// Handles a member accessor expression
    /// @param expr the member accessor expression
    void EmitMemberAccessor(const ast::MemberAccessorExpression* expr);
    /// Handles return statements
    /// @param stmt the statement to emit
    void EmitReturn(const ast::ReturnStatement* stmt);
    /// Handles const assertion statements
    /// @param stmt the statement to emit
    void EmitConstAssert(const ast::ConstAssert* stmt);
    /// Handles statement
    /// @param stmt the statement to emit
    void EmitStatement(const ast::Statement* stmt);
    /// Handles a statement list
    /// @param stmts the statements to emit
    void EmitStatements(VectorRef<const ast::Statement*> stmts);
    /// Handles a statement list with an increased indentation
    /// @param stmts the statements to emit
    void EmitStatementsWithIndent(VectorRef<const ast::Statement*> stmts);
    /// Handles generating a switch statement
    /// @param stmt the statement to emit
    void EmitSwitch(const ast::SwitchStatement* stmt);
    /// Handles generating a struct declaration
    /// @param str the struct
    void EmitStructType(const ast::Struct* str);
    /// Handles emitting an image format
    /// @param fmt the format to generate
    void EmitImageFormat(const core::TexelFormat fmt);
    /// Handles a unary op expression
    /// @param expr the expression to emit
    void EmitUnaryOp(const ast::UnaryOpExpression* expr);
    /// Handles generating a variable
    /// @param var the variable to generate
    void EmitVariable(const ast::Variable* var);
    /// Handles generating a attribute list
    /// @param attrs the attribute list
    void EmitAttributes(VectorRef<const ast::Attribute*> attrs);

  private:
    const Program& program_;
};

}  // namespace tint::wgsl::writer

#endif  // SRC_TINT_LANG_WGSL_WRITER_SYNTAX_TREE_PRINTER_SYNTAX_TREE_PRINTER_H_
