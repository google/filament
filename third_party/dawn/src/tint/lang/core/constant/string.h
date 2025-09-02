// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_CONSTANT_STRING_H_
#define SRC_TINT_LANG_CORE_CONSTANT_STRING_H_

#include <string>

#include "src/tint/lang/core/constant/manager.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/type/string.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::constant {

/// String holds a constant string value.
class String : public Castable<constant::String, Value> {
  public:
    /// Constructor
    /// @param t the scalar type
    /// @param v the scalar value
    String(const core::type::Type* t, std::string_view v) : type(t), value(v) {}
    ~String() override = default;

    /// @copydoc Value::Type()
    const core::type::Type* Type() const override { return type; }

    /// @return nullptr, as String does not allow individual character access
    const Value* Index(size_t) const override { return nullptr; }

    /// @copydoc Value::NumElements()
    size_t NumElements() const override { return 1; }

    /// @copydoc Value::AllZero()
    bool AllZero() const override { return false; }

    /// @copydoc Value::AnyZero()
    bool AnyZero() const override { return false; }

    /// @copydoc Value::Hash()
    HashCode Hash() const override { return tint::Hash(type, Value()); }

    /// Clones the constant into the provided context
    /// @param ctx the clone context
    /// @returns the cloned node
    const String* Clone(CloneContext& ctx) const override;

    /// @returns the string value
    inline std::string_view Value() const { return value; }

    /// The string type
    core::type::Type const* const type;
    /// The string value
    const std::string value;

  protected:
    /// @copydoc Value::InternalValue()
    std::variant<std::monostate, AInt, AFloat> InternalValue() const override { return {}; }
};

}  // namespace tint::core::constant

#endif  // SRC_TINT_LANG_CORE_CONSTANT_STRING_H_
