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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_DEPENDENCY_GRAPH_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_DEPENDENCY_GRAPH_H_

#include <string>
#include <vector>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::resolver {

/// ResolvedIdentifier holds the resolution of an ast::Identifier.
/// Can hold one of:
/// - UnresolvedIdentifier
/// - const ast::TypeDecl*  (as const ast::Node*)
/// - const ast::Variable*  (as const ast::Node*)
/// - const ast::Function*  (as const ast::Node*)
/// - wgsl::BuiltinFn
/// - core::Access
/// - core::AddressSpace
/// - core::BuiltinType
/// - core::TexelFormat
class ResolvedIdentifier {
  public:
    /// UnresolvedIdentifier is the variant value used to represent an unresolved identifier
    struct UnresolvedIdentifier {
        /// Name of the unresolved identifier
        std::string name;
    };

    /// Constructor
    /// @param value the resolved identifier value
    template <typename T>
    ResolvedIdentifier(T value) : value_(value) {}  // NOLINT(runtime/explicit)

    /// @return the UnresolvedIdentifier if the identifier was not resolved
    const UnresolvedIdentifier* Unresolved() const {
        if (auto n = std::get_if<UnresolvedIdentifier>(&value_)) {
            return n;
        }
        return nullptr;
    }

    /// @return the node pointer if the ResolvedIdentifier holds an AST node, otherwise nullptr
    const ast::Node* Node() const {
        if (auto n = std::get_if<const ast::Node*>(&value_)) {
            return *n;
        }
        return nullptr;
    }

    /// @return the builtin function if the ResolvedIdentifier holds wgsl::BuiltinFn, otherwise
    /// wgsl::BuiltinFn::kNone
    wgsl::BuiltinFn BuiltinFn() const {
        if (auto n = std::get_if<wgsl::BuiltinFn>(&value_)) {
            return *n;
        }
        return wgsl::BuiltinFn::kNone;
    }

    /// @return the access if the ResolvedIdentifier holds core::Access, otherwise
    /// core::Access::kUndefined
    core::Access Access() const {
        if (auto n = std::get_if<core::Access>(&value_)) {
            return *n;
        }
        return core::Access::kUndefined;
    }

    /// @return the address space if the ResolvedIdentifier holds core::AddressSpace, otherwise
    /// core::AddressSpace::kUndefined
    core::AddressSpace AddressSpace() const {
        if (auto n = std::get_if<core::AddressSpace>(&value_)) {
            return *n;
        }
        return core::AddressSpace::kUndefined;
    }

    /// @return the builtin type if the ResolvedIdentifier holds core::BuiltinType, otherwise
    /// core::BuiltinType::kUndefined
    core::BuiltinType BuiltinType() const {
        if (auto n = std::get_if<core::BuiltinType>(&value_)) {
            return *n;
        }
        return core::BuiltinType::kUndefined;
    }

    /// @return the texel format if the ResolvedIdentifier holds type::TexelFormat, otherwise
    /// type::TexelFormat::kUndefined
    core::TexelFormat TexelFormat() const {
        if (auto n = std::get_if<core::TexelFormat>(&value_)) {
            return *n;
        }
        return core::TexelFormat::kUndefined;
    }

    /// @param value the value to compare the ResolvedIdentifier to
    /// @return true if the ResolvedIdentifier is equal to @p value
    template <typename T>
    bool operator==(const T& value) const {
        if (auto n = std::get_if<T>(&value_)) {
            return *n == value;
        }
        return false;
    }

    /// @param other the other value to compare to this
    /// @return true if this ResolvedIdentifier and @p other are not equal
    template <typename T>
    bool operator!=(const T& other) const {
        return !(*this == other);
    }

    /// @return a description of the resolved symbol
    std::string String() const;

  private:
    std::variant<UnresolvedIdentifier,
                 const ast::Node*,
                 wgsl::BuiltinFn,
                 core::Access,
                 core::AddressSpace,
                 core::BuiltinType,
                 core::TexelFormat>
        value_;
};

/// DependencyGraph holds information about module-scope declaration dependency
/// analysis and symbol resolutions.
struct DependencyGraph {
    /// Constructor
    DependencyGraph();
    /// Move-constructor
    DependencyGraph(DependencyGraph&&);
    /// Destructor
    ~DependencyGraph();

    /// Build() performs symbol resolution and dependency analysis on `module`,
    /// populating `output` with the resulting dependency graph.
    /// @param module the AST module to analyse
    /// @param diagnostics the diagnostic list to populate with errors / warnings
    /// @param output the resulting DependencyGraph
    /// @returns true on success, false on error
    static bool Build(const ast::Module& module, diag::List& diagnostics, DependencyGraph& output);

    /// All globals in dependency-sorted order.
    Vector<const ast::Node*, 32> ordered_globals;

    /// Map of ast::Identifier to a ResolvedIdentifier
    Hashmap<const ast::Identifier*, ResolvedIdentifier, 64> resolved_identifiers;

    /// Map of ast::Variable to a type, function, or variable that is shadowed by
    /// the variable key. A declaration (X) shadows another (Y) if X and Y use
    /// the same symbol, and X is declared in a sub-scope of the scope that
    /// declares Y.
    Hashmap<const ast::Variable*, const ast::Node*, 16> shadows;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_DEPENDENCY_GRAPH_H_
