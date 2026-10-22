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

#ifndef SRC_TINT_LANG_CORE_CONSTANT_COMPOSITE_H_
#define SRC_TINT_LANG_CORE_CONSTANT_COMPOSITE_H_

#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::constant {

/// Composite holds a number of mixed child values.
/// Composite may be of a vector, matrix, array or structure type.
/// If each element is the same type and value, then a Splat would be a more efficient constant
/// implementation. Use CreateComposite() to create the appropriate type.
class Composite : public Castable<Composite, Value> {
  public:
    /// Constructor
    /// @param t the compsite type
    /// @param els the composite elements
    /// @param all_0 true if all elements are 0
    /// @param any_0 true if any element is 0
    Composite(const core::type::Type* t, VectorRef<const Value*> els, bool all_0, bool any_0);
    ~Composite() override;

    /// @copydoc Value::Type()
    const core::type::Type* Type() const override { return type; }

    /// @copydoc Value::Index()
    const Value* Index(size_t i) const override {
        return i < elements.Length() ? elements[i] : nullptr;
    }

    /// @copydoc Value::NumElements()
    size_t NumElements() const override { return elements.Length(); }

    /// @copydoc Value::AllZero()
    bool AllZero() const override { return all_zero; }

    /// @copydoc Value::AnyZero()
    bool AnyZero() const override { return any_zero; }

    /// @copydoc Value::Hash()
    HashCode Hash() const override { return hash; }

    /// Clones the constant into the provided context
    /// @param ctx the clone context
    /// @returns the cloned node
    const Composite* Clone(CloneContext& ctx) const override;

    /// The composite type
    core::type::Type const* const type;
    /// The composite elements
    const Vector<const Value*, 4> elements;
    /// True if all elements are zero
    const bool all_zero;
    /// True if any element is zero
    const bool any_zero;
    /// The hash of the composite
    const HashCode hash;

  protected:
    /// @copydoc Value::InternalValue()
    std::variant<std::monostate, AInt, AFloat> InternalValue() const override { return {}; }

  private:
    HashCode CalcHash() {
        auto h = tint::Hash(type, all_zero, any_zero);
        for (auto* el : elements) {
            h = HashCombine(h, el->Hash());
        }
        return h;
    }
};

}  // namespace tint::core::constant

#endif  // SRC_TINT_LANG_CORE_CONSTANT_COMPOSITE_H_
