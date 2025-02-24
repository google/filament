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

#ifndef SRC_TINT_LANG_WGSL_SEM_ARRAY_COUNT_H_
#define SRC_TINT_LANG_WGSL_SEM_ARRAY_COUNT_H_

#include <string>

#include "src/tint/lang/core/type/array_count.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::sem {

/// The variant of an ArrayCount when the count is a named override variable.
/// Example:
/// ```
/// override N : i32;
/// type arr = array<i32, N>
/// ```
class NamedOverrideArrayCount final
    : public Castable<NamedOverrideArrayCount, core::type::ArrayCount> {
  public:
    /// Constructor
    /// @param var the `override` variable
    explicit NamedOverrideArrayCount(const GlobalVariable* var);
    ~NamedOverrideArrayCount() override;

    /// @param other the other node
    /// @returns true if this array count is equal @p other
    bool Equals(const core::type::UniqueNode& other) const override;

    /// @returns the friendly name for this array count
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    core::type::ArrayCount* Clone(core::type::CloneContext& ctx) const override;

    /// The `override` variable.
    const GlobalVariable* variable;
};

/// The variant of an ArrayCount when the count is an unnamed override variable.
/// Example:
/// ```
/// override N : i32;
/// type arr = array<i32, N*2>
/// ```
class UnnamedOverrideArrayCount final
    : public Castable<UnnamedOverrideArrayCount, core::type::ArrayCount> {
  public:
    /// Constructor
    /// @param e the override expression
    explicit UnnamedOverrideArrayCount(const ValueExpression* e);
    ~UnnamedOverrideArrayCount() override;

    /// @param other the other node
    /// @returns true if this array count is equal @p other
    bool Equals(const core::type::UniqueNode& other) const override;

    /// @returns the friendly name for this array count
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    core::type::ArrayCount* Clone(core::type::CloneContext& ctx) const override;

    /// The unnamed override expression.
    /// Note: Each AST expression gets a unique semantic expression node, so two equivalent AST
    /// expressions will not result in the same `expr` pointer. This property is important to ensure
    /// that two array declarations with equivalent AST expressions do not compare equal.
    /// For example, consider:
    /// ```
    /// override size : u32;
    /// var<workgroup> a : array<f32, size * 2>;
    /// var<workgroup> b : array<f32, size * 2>;
    /// ```
    // The array count for `a` and `b` have equivalent AST expressions, but the types for `a` and
    // `b` must not compare equal.
    const ValueExpression* expr;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_ARRAY_COUNT_H_
