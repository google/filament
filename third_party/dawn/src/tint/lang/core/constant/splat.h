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

#ifndef SRC_TINT_LANG_CORE_CONSTANT_SPLAT_H_
#define SRC_TINT_LANG_CORE_CONSTANT_SPLAT_H_

#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::constant {

/// Splat holds a single value, duplicated as all children.
///
/// Splat is used for zero-initializers, 'splat' initializers, or initializers where each element is
/// identical. Splat may be of a vector, matrix, array or structure type.
class Splat : public Castable<Splat, Value> {
  public:
    /// Constructor
    /// @param t the splat type
    /// @param e the splat element
    Splat(const core::type::Type* t, const Value* e);
    ~Splat() override;

    /// @returns the type of the splat
    const core::type::Type* Type() const override { return type; }

    /// Retrieve item at index @p i
    /// @param i the index to retrieve
    /// @returns the element, or nullptr if out of bounds
    const Value* Index(size_t i) const override { return i < count ? el : nullptr; }

    /// @copydoc Value::NumElements()
    size_t NumElements() const override { return count; }

    /// @returns true if the element is zero
    bool AllZero() const override { return el->AllZero(); }
    /// @returns true if the element is zero
    bool AnyZero() const override { return el->AnyZero(); }

    /// @returns the hash for the splat
    HashCode Hash() const override { return tint::Hash(type, el->Hash(), count); }

    /// Clones the constant into the provided context
    /// @param ctx the clone context
    /// @returns the cloned node
    const Splat* Clone(CloneContext& ctx) const override;

    /// The type of the splat element
    core::type::Type const* const type;
    /// The element stored in the splat
    const Value* el;
    /// The number of items in the splat
    const size_t count;

  protected:
    /// @returns a monostate variant.
    std::variant<std::monostate, AInt, AFloat> InternalValue() const override { return {}; }
};

}  // namespace tint::core::constant

#endif  // SRC_TINT_LANG_CORE_CONSTANT_SPLAT_H_
