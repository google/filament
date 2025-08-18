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

#ifndef SRC_TINT_LANG_WGSL_SEM_MODULE_H_
#define SRC_TINT_LANG_WGSL_SEM_MODULE_H_

#include "src/tint/lang/wgsl/ast/diagnostic_control.h"
#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/lang/wgsl/sem/node.h"
#include "src/tint/utils/containers/vector.h"

// Forward declarations
namespace tint::ast {
class Node;
}  // namespace tint::ast

namespace tint::sem {

/// Module holds the top-level semantic types, functions and global variables
/// used by a Program.
class Module final : public Castable<Module, Node> {
  public:
    /// Constructor
    /// @param dep_ordered_decls the dependency-ordered module-scope declarations
    /// @param extensions the list of enabled extensions in the module
    Module(VectorRef<const ast::Node*> dep_ordered_decls, wgsl::Extensions extensions);

    /// Destructor
    ~Module() override;

    /// @returns the dependency-ordered global declarations for the module
    VectorRef<const ast::Node*> DependencyOrderedDeclarations() const { return dep_ordered_decls_; }

    /// @returns the list of enabled extensions in the module
    const wgsl::Extensions& Extensions() const { return extensions_; }

    /// Modifies the severity of a specific diagnostic rule for this module.
    /// @param rule the diagnostic rule
    /// @param severity the new diagnostic severity
    void SetDiagnosticSeverity(wgsl::DiagnosticRule rule, wgsl::DiagnosticSeverity severity);

    /// @returns the diagnostic severity modifications applied to this module
    const wgsl::DiagnosticRuleSeverities& DiagnosticSeverities() const {
        return diagnostic_severities_;
    }

  private:
    const tint::Vector<const ast::Node*, 64> dep_ordered_decls_;
    wgsl::Extensions extensions_;
    wgsl::DiagnosticRuleSeverities diagnostic_severities_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_MODULE_H_
