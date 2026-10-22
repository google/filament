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

#ifndef SRC_TINT_LANG_WGSL_AST_MODULE_H_
#define SRC_TINT_LANG_WGSL_AST_MODULE_H_

#include <string>

#include "src/tint/lang/wgsl/ast/const_assert.h"
#include "src/tint/lang/wgsl/ast/diagnostic_directive.h"
#include "src/tint/lang/wgsl/ast/enable.h"
#include "src/tint/lang/wgsl/ast/function.h"
#include "src/tint/lang/wgsl/ast/requires.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::ast {

class TypeDecl;

/// Module holds the top-level AST types, functions and global variables used by
/// a Program.
class Module final : public Castable<Module, Node> {
  public:
    /// Constructor
    /// @param nid the unique node identifier
    /// @param src the source of this node
    Module(NodeID nid, const Source& src);

    /// Constructor
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param global_decls the list of global types, functions, and variables, in
    /// the order they were declared in the source program
    Module(NodeID nid, const Source& src, VectorRef<const Node*> global_decls);

    /// Destructor
    ~Module() override;

    /// @returns the declaration-ordered global declarations for the module
    const auto& GlobalDeclarations() const { return global_declarations_; }

    /// Add a global variable to the module
    /// @param var the variable to add
    void AddGlobalVariable(const Variable* var);

    /// @returns true if the module has the global declaration `decl`
    /// @param decl the declaration to check
    bool HasGlobalDeclaration(Node* decl) const {
        for (auto* d : global_declarations_) {
            if (d == decl) {
                return true;
            }
        }
        return false;
    }

    /// Adds a global declaration to the module.
    /// @param decl the declaration to add
    void AddGlobalDeclaration(const tint::ast::Node* decl);

    /// @returns the global variables for the module
    const auto& GlobalVariables() const { return global_variables_; }

    /// @returns the global variables for the module
    auto& GlobalVariables() { return global_variables_; }

    /// @returns the global variable declarations of kind 'T' for the module
    template <typename T>
        requires(traits::IsTypeOrDerived<T, Variable>)
    auto Globals() const {
        tint::Vector<const T*, 32> out;
        out.Reserve(global_variables_.Length());
        for (auto* global : global_variables_) {
            if (auto* var = global->As<T>()) {
                out.Push(var);
            }
        }
        return out;
    }

    /// Add a diagnostic directive to the module
    /// @param diagnostic the diagnostic directive to add
    void AddDiagnosticDirective(const DiagnosticDirective* diagnostic);

    /// Add a enable directive to the module
    /// @param ext the enable directive to add
    void AddEnable(const Enable* ext);

    /// Add a requires directive to the module
    /// @param req the requires directive to add
    void AddRequires(const Requires* req);

    /// @returns the diagnostic directives for the module
    const auto& DiagnosticDirectives() const { return diagnostic_directives_; }

    /// @returns the extension set for the module
    const auto& Enables() const { return enables_; }

    /// @returns the requires directives for the module
    const auto& Requires() const { return requires_; }

    /// Add a global const assertion to the module
    /// @param assertion the const assert to add
    void AddConstAssert(const ConstAssert* assertion);

    /// @returns the list of global const assertions
    const auto& ConstAsserts() const { return const_asserts_; }

    /// Adds a type declaration to the module
    /// @param decl the type declaration to add
    void AddTypeDecl(const TypeDecl* decl);

    /// @returns the TypeDecl registered as a TypeDecl()
    /// @param name the name of the type to search for
    const TypeDecl* LookupType(Symbol name) const;

    /// @returns the declared types in the module
    const auto& TypeDecls() const { return type_decls_; }

    /// Add a function to the module
    /// @param func the function to add
    void AddFunction(const Function* func);

    /// @returns the functions declared in the module
    const FunctionList& Functions() const { return functions_; }

    /// @returns true if the module has any 'override' declarations
    bool HasOverrides() const;

  private:
    /// Adds `decl` to either:
    /// * #global_declarations_
    /// * #type_decls_
    /// * #functions_
    void BinGlobalDeclaration(const tint::ast::Node* decl);

    tint::Vector<const Node*, 64> global_declarations_;
    tint::Vector<const TypeDecl*, 16> type_decls_;
    FunctionList functions_;
    tint::Vector<const Variable*, 32> global_variables_;
    tint::Vector<const DiagnosticDirective*, 8> diagnostic_directives_;
    tint::Vector<const Enable*, 8> enables_;
    tint::Vector<const ast::Requires*, 8> requires_;
    tint::Vector<const ConstAssert*, 8> const_asserts_;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_MODULE_H_
