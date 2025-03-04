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

#include "src/tint/lang/core/constant/value.h"

#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/invalid.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Value);

namespace tint::core::constant {

Value::Value() = default;

Value::~Value() = default;

/// Equal returns true if the constants `a` and `b` are of the same type and value.
bool Value::Equal(const constant::Value* b) const {
    if (this == b) {
        return true;
    }
    if (Hash() != b->Hash()) {
        return false;
    }
    if (Type() != b->Type()) {
        return false;
    }

    auto elements_equal = [&](size_t count) {
        if (count == 0) {
            return true;
        }

        // Avoid per-element comparisons if the constants are splats
        bool a_is_splat = Is<Splat>();
        bool b_is_splat = b->Is<Splat>();
        if (a_is_splat && b_is_splat) {
            return Index(0)->Equal(b->Index(0));
        }

        if (a_is_splat) {
            auto* el_a = Index(0);
            for (size_t i = 0; i < count; i++) {
                if (!el_a->Equal(b->Index(i))) {
                    return false;
                }
            }
            return true;
        }

        if (b_is_splat) {
            auto* el_b = b->Index(0);
            for (size_t i = 0; i < count; i++) {
                if (!Index(i)->Equal(el_b)) {
                    return false;
                }
            }
            return true;
        }

        // Per-element comparison
        for (size_t i = 0; i < count; i++) {
            if (!Index(i)->Equal(b->Index(i))) {
                return false;
            }
        }
        return true;
    };

    return Switch(
        Type(),  //
        [&](const core::type::Vector* vec) { return elements_equal(vec->Width()); },
        [&](const core::type::Matrix* mat) { return elements_equal(mat->Columns()); },
        [&](const core::type::Struct* str) { return elements_equal(str->Members().Length()); },
        [&](const core::type::Array* arr) {
            if (auto n = arr->ConstantCount()) {
                return elements_equal(*n);
            }
            return false;
        },
        [&](const core::type::Invalid*) { return true; },
        [&](Default) { return InternalValue() == b->InternalValue(); });
}

}  // namespace tint::core::constant
