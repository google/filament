// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_CONSTANT_INVALID_H_
#define SRC_TINT_LANG_CORE_CONSTANT_INVALID_H_

#include <variant>
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/invalid.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::constant {

/// Invalid represents an invalid constant, used as a placeholder in a failed parse / resolve.
class Invalid : public Castable<Invalid, Value> {
  public:
    /// Constructor
    /// @param ty the Invalid type
    explicit Invalid(const core::type::Invalid* ty);
    ~Invalid() override;

    /// @returns the type of the Invalid
    const core::type::Type* Type() const override { return type; }

    /// Retrieve item at index @p i
    /// @param i the index to retrieve
    /// @returns the element, or nullptr if out of bounds
    const Value* Index([[maybe_unused]] size_t i) const override { return nullptr; }

    /// @copydoc Value::NumElements()
    size_t NumElements() const override { return 0; }

    /// @returns true if the element is zero
    bool AllZero() const override { return false; }
    /// @returns true if the element is zero
    bool AnyZero() const override { return false; }

    /// @returns the hash for the Invalid
    HashCode Hash() const override { return tint::Hash(type); }

    /// Clones the constant into the provided context
    /// @param ctx the clone context
    /// @returns the cloned node
    const Invalid* Clone(CloneContext& ctx) const override;

    /// The Invalid type
    core::type::Invalid const* const type;

  protected:
    /// @returns a monostate variant.
    std::variant<std::monostate, AInt, AFloat> InternalValue() const override { return {}; }
};

}  // namespace tint::core::constant

#endif  // SRC_TINT_LANG_CORE_CONSTANT_INVALID_H_
