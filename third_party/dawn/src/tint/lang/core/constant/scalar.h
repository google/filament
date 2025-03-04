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

#ifndef SRC_TINT_LANG_CORE_CONSTANT_SCALAR_H_
#define SRC_TINT_LANG_CORE_CONSTANT_SCALAR_H_

#include "src/tint/lang/core/constant/manager.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::constant {

/// ScalarBase is the base class of all Scalar<T> specializations.
/// Used for querying whether a value is a scalar type.
class ScalarBase : public Castable<ScalarBase, Value> {
  public:
    ~ScalarBase() override;
};

/// Scalar holds a single scalar or abstract-numeric value.
template <typename T>
class Scalar : public Castable<Scalar<T>, ScalarBase> {
  public:
    static_assert(!std::is_same_v<UnwrapNumber<T>, T> || std::is_same_v<T, bool>,
                  "T must be a Number or bool");

    /// Constructor
    /// @param t the scalar type
    /// @param v the scalar value
    Scalar(const core::type::Type* t, T v) : type(t), value(v) {
        if constexpr (IsFloatingPoint<T>) {
            TINT_ASSERT(std::isfinite(v.value));
        }
    }
    ~Scalar() override = default;

    /// @copydoc Value::Type()
    const core::type::Type* Type() const override { return type; }

    /// @return nullptr, as Scalar does not hold any elements.
    const Value* Index(size_t) const override { return nullptr; }

    /// @copydoc Value::NumElements()
    size_t NumElements() const override { return 1; }

    /// @copydoc Value::AllZero()
    bool AllZero() const override { return IsZero(); }

    /// @copydoc Value::AnyZero()
    bool AnyZero() const override { return IsZero(); }

    /// @copydoc Value::Hash()
    HashCode Hash() const override { return tint::Hash(type, ValueOf()); }

    /// Clones the constant into the provided context
    /// @param ctx the clone context
    /// @returns the cloned node
    const Scalar* Clone(CloneContext& ctx) const override {
        auto* ty = type->Clone(ctx.type_ctx);
        return ctx.dst.Get<Scalar<T>>(ty, value);
    }

    /// @returns `value` if `T` is not a Number, otherwise ValueOf returns the inner value of the
    /// Number.
    inline auto ValueOf() const {
        if constexpr (std::is_same_v<UnwrapNumber<T>, T>) {
            return value;
        } else {
            return value.value;
        }
    }

    /// @returns true if `value` is zero.
    /// For floating point -0.0 equals 0.0, according to IEEE 754.
    inline bool IsZero() const {
        using N = UnwrapNumber<T>;
        return Number<N>(value) == Number<N>(0);
    }

    /// The scalar type
    core::type::Type const* const type;
    /// The scalar value
    const T value;

  protected:
    /// @copydoc Value::InternalValue()
    std::variant<std::monostate, AInt, AFloat> InternalValue() const override {
        if constexpr (IsFloatingPoint<UnwrapNumber<T>>) {
            return static_cast<AFloat>(value);
        } else {
            return static_cast<AInt>(value);
        }
    }
};

}  // namespace tint::core::constant

#endif  // SRC_TINT_LANG_CORE_CONSTANT_SCALAR_H_
