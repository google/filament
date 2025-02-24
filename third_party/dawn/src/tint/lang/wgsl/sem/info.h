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

#ifndef SRC_TINT_LANG_WGSL_SEM_INFO_H_
#define SRC_TINT_LANG_WGSL_SEM_INFO_H_

#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "src/tint/lang/wgsl/ast/diagnostic_control.h"
#include "src/tint/lang/wgsl/ast/node.h"
#include "src/tint/lang/wgsl/sem/node.h"
#include "src/tint/lang/wgsl/sem/type_mappings.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/ice/ice.h"

// Forward declarations
namespace tint::sem {
class Module;
class ValueExpression;
}  // namespace tint::sem
namespace tint::core::type {
class Node;
class Type;
}  // namespace tint::core::type

namespace tint::sem {

/// Info holds all the resolved semantic information for a Program.
class Info {
  public:
    /// Placeholder type used by Get() to provide a default value for EXPLICIT_SEM
    using InferFromAST = std::nullptr_t;

    /// Resolves to the return type of the Get() method given the desired semantic
    /// type and AST type.
    template <typename SEM, typename AST>
    using GetResultType =
        std::conditional_t<std::is_same<SEM, InferFromAST>::value, SemanticNodeTypeFor<AST>, SEM>;

    /// Constructor
    Info();

    /// Move constructor
    Info(Info&&);

    /// Destructor
    ~Info();

    /// Move assignment operator
    /// @param rhs the Program to move
    /// @return this Program
    Info& operator=(Info&& rhs);

    /// @param highest_node_id the last allocated (numerically highest) AST node identifier.
    void Reserve(ast::NodeID highest_node_id) {
        nodes_.Resize(std::max(highest_node_id.value + 1, static_cast<uint32_t>(nodes_.Length())));
    }

    /// Get looks up the semantic information for the AST node `ast_node`.
    /// @param ast_node the AST node
    /// @returns a pointer to the semantic node if found, otherwise nullptr
    template <typename SEM = InferFromAST,
              typename AST = CastableBase,
              typename RESULT = GetResultType<SEM, AST>>
    const RESULT* Get(const AST* ast_node) const {
        static_assert(std::is_same_v<SEM, InferFromAST> ||
                          !tint::traits::IsTypeOrDerived<SemanticNodeTypeFor<AST>, SEM>,
                      "explicit template argument is unnecessary");
        if (ast_node && ast_node->node_id.value < nodes_.Length()) {
            return As<RESULT>(nodes_[ast_node->node_id.value]);
        }
        return nullptr;
    }

    /// Convenience function that's an alias for Get<ValueExpression>()
    /// @param ast_node the AST node
    /// @returns a pointer to the semantic node if found, otherwise nullptr
    template <typename AST>
    const sem::ValueExpression* GetVal(const AST* ast_node) const {
        return Get<ValueExpression>(ast_node);
    }

    /// Add registers the semantic node `sem_node` for the AST node `ast_node`.
    /// @param ast_node the AST node
    /// @param sem_node the semantic node
    template <typename AST>
    void Add(const AST* ast_node, const SemanticNodeTypeFor<AST>* sem_node) {
        Reserve(ast_node->node_id);
        // Check there's no semantic info already existing for the AST node
        TINT_ASSERT(nodes_[ast_node->node_id.value] == nullptr);
        nodes_[ast_node->node_id.value] = sem_node;
    }

    /// Replace replaces any existing semantic node `sem_node` for the AST node `ast_node`.
    /// @param ast_node the AST node
    /// @param sem_node the new semantic node
    template <typename AST>
    void Replace(const AST* ast_node, const SemanticNodeTypeFor<AST>* sem_node) {
        Reserve(ast_node->node_id);
        nodes_[ast_node->node_id.value] = sem_node;
    }

    /// Wrap returns a new Info created with the contents of `inner`.
    /// The Info returned by Wrap is intended to temporarily extend the contents
    /// of an existing immutable Info.
    /// As the copied contents are owned by `inner`, `inner` must not be
    /// destructed or assigned while using the returned Info.
    /// @param inner the immutable Info to extend
    /// @return the Info that wraps `inner`
    static Info Wrap(const Info& inner) {
        Info out;
        out.nodes_ = inner.nodes_;
        out.module_ = inner.module_;
        return out;
    }

    /// Assigns the semantic module.
    /// @param module the module to assign.
    void SetModule(sem::Module* module) { module_ = module; }

    /// @returns the semantic module.
    const sem::Module* Module() const { return module_; }

    /// Determines the severity of a filterable diagnostic rule for the AST node `ast_node`.
    /// @param ast_node the AST node
    /// @param rule the diagnostic rule
    /// @returns the severity of the rule for that AST node
    wgsl::DiagnosticSeverity DiagnosticSeverity(const ast::Node* ast_node,
                                                wgsl::DiagnosticRule rule) const;

  private:
    // AST node index to semantic node
    tint::Vector<const CastableBase*, 0> nodes_;
    // The semantic module
    sem::Module* module_ = nullptr;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_INFO_H_
