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

#ifndef SRC_TINT_LANG_WGSL_SEM_BLOCK_STATEMENT_H_
#define SRC_TINT_LANG_WGSL_SEM_BLOCK_STATEMENT_H_

#include <cstddef>
#include <vector>

#include "src/tint/lang/wgsl/sem/statement.h"

// Forward declarations
namespace tint::ast {
class BlockStatement;
class ContinueStatement;
class Variable;
}  // namespace tint::ast

namespace tint::sem {

/// Holds semantic information about a block, such as parent block and variables
/// declared in the block.
class BlockStatement : public Castable<BlockStatement, CompoundStatement> {
  public:
    /// Constructor
    /// @param declaration the AST node for this block statement
    /// @param parent the owning statement
    /// @param function the owning function
    BlockStatement(const ast::BlockStatement* declaration,
                   const CompoundStatement* parent,
                   const sem::Function* function);

    /// Destructor
    ~BlockStatement() override;

    /// @returns the AST block statement associated with this semantic block
    /// statement
    const ast::BlockStatement* Declaration() const;
};

/// The root block statement for a function
class FunctionBlockStatement final : public Castable<FunctionBlockStatement, BlockStatement> {
  public:
    /// Constructor
    /// @param function the owning function
    explicit FunctionBlockStatement(const sem::Function* function);

    /// Destructor
    ~FunctionBlockStatement() override;
};

/// Holds semantic information about a loop body block or for-loop body block
class LoopBlockStatement final : public Castable<LoopBlockStatement, BlockStatement> {
  public:
    /// Constructor
    /// @param declaration the AST node for this block statement
    /// @param parent the owning statement
    /// @param function the owning function
    LoopBlockStatement(const ast::BlockStatement* declaration,
                       const CompoundStatement* parent,
                       const sem::Function* function);

    /// Destructor
    ~LoopBlockStatement() override;

    /// @returns the first continue statement in this loop block, or nullptr if
    /// there are no continue statements in the block
    const ast::ContinueStatement* FirstContinue() const { return first_continue_; }

    /// @returns the number of variables declared before the first continue
    /// statement
    size_t NumDeclsAtFirstContinue() const { return num_decls_at_first_continue_; }

    /// Allows the resolver to record the first continue statement in the block
    /// and the number of variables declared prior to that statement.
    /// @param first_continue the first continue statement in the block
    /// @param num_decls the number of variable declarations before that continue
    void SetFirstContinue(const ast::ContinueStatement* first_continue, size_t num_decls);

  private:
    /// The first continue statement in this loop block.
    const ast::ContinueStatement* first_continue_ = nullptr;

    /// The number of variables declared before the first continue statement.
    size_t num_decls_at_first_continue_ = 0;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_BLOCK_STATEMENT_H_
