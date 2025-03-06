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

#ifndef SRC_TINT_LANG_CORE_CONSTANT_VALUE_H_
#define SRC_TINT_LANG_CORE_CONSTANT_VALUE_H_

#include <variant>

#include "src/tint/lang/core/constant/clone_context.h"
#include "src/tint/lang/core/constant/node.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::constant {

/// Value is the interface to a compile-time evaluated expression value.
class Value : public Castable<Value, Node> {
  public:
    /// Constructor
    Value();

    /// Destructor
    ~Value() override;

    /// @returns the type of the value
    virtual const core::type::Type* Type() const = 0;

    /// @param i the index of the element
    /// @returns the child element with the given index, or nullptr if there are no children, or
    /// the index is out of bounds.
    ///
    /// For arrays, this returns the i'th element of the array.
    /// For vectors, this returns the i'th element of the vector.
    /// For matrices, this returns the i'th column vector of the matrix.
    /// For structures, this returns the i'th member field of the structure.
    virtual const Value* Index(size_t i) const = 0;

    /// @return the number of elements held by this Value
    virtual size_t NumElements() const = 0;

    /// @returns true if child elements are positive-zero valued.
    virtual bool AllZero() const = 0;

    /// @returns true if any child elements are positive-zero valued.
    virtual bool AnyZero() const = 0;

    /// @returns a hash of the value.
    virtual HashCode Hash() const = 0;

    /// @returns the value as the given scalar or abstract value.
    template <typename T>
    T ValueAs() const {
        return std::visit(
            [](auto v) {
                if constexpr (std::is_same_v<decltype(v), std::monostate>) {
                    return T(0);
                } else {
                    return static_cast<T>(v);
                }
            },
            InternalValue());
    }

    /// @param b the value to compare too
    /// @returns true if this value is equal to @p b
    bool Equal(const Value* b) const;

    /// Clones the constant into the provided context
    /// @param ctx the clone context
    /// @returns the cloned node
    virtual const Value* Clone(CloneContext& ctx) const = 0;

  protected:
    /// @returns the value, if this is of a scalar value or abstract numeric, otherwise
    /// std::monostate.
    virtual std::variant<std::monostate, AInt, AFloat> InternalValue() const = 0;
};

}  // namespace tint::core::constant

#endif  // SRC_TINT_LANG_CORE_CONSTANT_VALUE_H_
