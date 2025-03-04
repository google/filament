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

#ifndef SRC_TINT_LANG_WGSL_AST_WORKGROUP_ATTRIBUTE_H_
#define SRC_TINT_LANG_WGSL_AST_WORKGROUP_ATTRIBUTE_H_

#include <array>
#include <string>

#include "src/tint/lang/wgsl/ast/attribute.h"

// Forward declarations
namespace tint::ast {
class Expression;
}  // namespace tint::ast

namespace tint::ast {

/// A workgroup attribute
class WorkgroupAttribute final : public Castable<WorkgroupAttribute, Attribute> {
  public:
    /// constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param x the workgroup x dimension expression
    /// @param y the optional workgroup y dimension expression
    /// @param z the optional workgroup z dimension expression
    WorkgroupAttribute(GenerationID pid,
                       NodeID nid,
                       const Source& src,
                       const Expression* x,
                       const Expression* y = nullptr,
                       const Expression* z = nullptr);

    ~WorkgroupAttribute() override;

    /// @returns the workgroup dimensions
    std::array<const Expression*, 3> Values() const { return {x, y, z}; }

    /// @returns the WGSL name for the attribute
    std::string Name() const override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const WorkgroupAttribute* Clone(CloneContext& ctx) const override;

    /// The workgroup x dimension.
    const Expression* const x;
    /// The optional workgroup y dimension. May be null.
    const Expression* const y = nullptr;
    /// The optional workgroup z dimension. May be null.
    const Expression* const z = nullptr;
};

}  // namespace tint::ast

#endif  // SRC_TINT_LANG_WGSL_AST_WORKGROUP_ATTRIBUTE_H_
