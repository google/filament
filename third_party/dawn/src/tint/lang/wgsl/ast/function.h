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

#ifndef SRC_TINT_LANG_WGSL_AST_FUNCTION_H_
#define SRC_TINT_LANG_WGSL_AST_FUNCTION_H_

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "src/tint/lang/wgsl/ast/attribute.h"
#include "src/tint/lang/wgsl/ast/binding_attribute.h"
#include "src/tint/lang/wgsl/ast/block_statement.h"
#include "src/tint/lang/wgsl/ast/builtin_attribute.h"
#include "src/tint/lang/wgsl/ast/group_attribute.h"
#include "src/tint/lang/wgsl/ast/location_attribute.h"
#include "src/tint/lang/wgsl/ast/parameter.h"
#include "src/tint/lang/wgsl/ast/pipeline_stage.h"
#include "src/tint/utils/symbol/symbol.h"

// Forward declarations
namespace tint::ast {
class Identifier;
class IdentifierExpression;
}  // namespace tint::ast

namespace tint::ast {

/// A Function statement.
class Function final : public Castable<Function, Node> {
  public:
    /// Create a function
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param source the variable source
    /// @param name the function name
    /// @param params the function parameters
    /// @param return_type the return type
    /// @param body the function body
    /// @param attributes the function attributes
    /// @param return_type_attributes the return type attributes
    Function(GenerationID pid,
             NodeID nid,
             const Source& source,
             const Identifier* name,
             VectorRef<const Parameter*> params,
             Type return_type,
             const BlockStatement* body,
             VectorRef<const Attribute*> attributes,
             VectorRef<const Attribute*> return_type_attributes);

    /// Destructor
    ~Function() override;

    /// @returns the functions pipeline stage or None if not set
    ast::PipelineStage PipelineStage() const;

    /// @returns true if this function is an entry point
    bool IsEntryPoint() const { return PipelineStage() != ast::PipelineStage::kNone; }

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Function* Clone(CloneContext& ctx) const override;

    /// The function name
    const Identifier* const name;

    /// The function params
    const tint::Vector<const Parameter*, 8> params;

    /// The function return type
    const Type return_type;

    /// The function body
    const BlockStatement* const body;

    /// The attributes attached to this function
    const tint::Vector<const Attribute*, 2> attributes;

    /// The attributes attached to the function return type.
    const tint::Vector<const Attribute*, 2> return_type_attributes;
};

/// A list of functions
class FunctionList : public tint::Vector<const Function*, 8> {
  public:
    /// Appends f to the end of the list
    /// @param f the function to append to this list
    void Add(const Function* f) { this->Push(f); }

    /// Returns the function with the given name
    /// @param sym the function symbol to search for
    /// @returns the associated function or nullptr if none exists
    const Function* Find(Symbol sym) const;

    /// Returns the function with the given name
    /// @param sym the function symbol to search for
    /// @param stage the pipeline stage
    /// @returns the associated function or nullptr if none exists
    const Function* Find(Symbol sym, ast::PipelineStage stage) const;

    /// @param stage the pipeline stage
    /// @returns true if the Builder contains an entrypoint function with
    /// the given stage
    bool HasStage(ast::PipelineStage stage) const;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_FUNCTION_H_
