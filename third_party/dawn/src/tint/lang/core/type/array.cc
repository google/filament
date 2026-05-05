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

#include "src/tint/lang/core/type/array.h"

#include <string>

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/symbol/symbol_table.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::Array);

namespace tint::core::type {

namespace {

core::type::Flags FlagsFrom(const Type* element, const ArrayCount* count) {
    core::type::Flags flags;
    // Only constant-expression sized arrays are constructible
    if (count->Is<ConstantArrayCount>()) {
        if (element->IsConstructible()) {
            flags.Add(Flag::kConstructable);
        }
        if (element->HasCreationFixedFootprint()) {
            flags.Add(Flag::kCreationFixedFootprint);
        }
    }
    if (!count->Is<RuntimeArrayCount>()) {
        if (element->HasFixedFootprint()) {
            flags.Add(Flag::kFixedFootprint);
        }
    }
    if (element->IsHostShareable()) {
        flags.Add(Flag::kHostShareable);
    }
    return flags;
}

}  // namespace

const char* const Array::kErrExpectedConstantCount =
    "array size is an override-expression, when expected a constant-expression.\n"
    "Was the SubstituteOverride transform run?";

Array::Array(const Type* element, const ArrayCount* count, uint32_t size)
    : Base(Hash(tint::TypeCode::Of<Array>().bits, count, size), FlagsFrom(element, count)),
      element_(element),
      count_(count),
      size_(size) {
    TINT_ASSERT(count_);
    TINT_ASSERT(element_);
}

Array::Array(size_t hash, const Type* element, const ArrayCount* count, uint32_t size)
    : Base(hash, FlagsFrom(element, count)), element_(element), count_(count), size_(size) {
    TINT_ASSERT(count_);
    TINT_ASSERT(element_);
}

bool Array::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<Array>()) {
        return o->element_ == element_ && o->count_ == count_ && o->size_ == size_;
    }
    return false;
}

std::string Array::FriendlyName() const {
    StringStream out;
    out << "array<" << element_->FriendlyName();

    auto count_str = count_->FriendlyName();
    if (!count_str.empty()) {
        out << ", " << count_str;
    }

    out << ">";
    return out.str();
}

uint32_t Array::Size() const {
    return size_;
}

TypeAndCount Array::Elements(const Type* /* type_if_invalid = nullptr */,
                             uint32_t count_if_invalid /* = 0 */) const {
    uint32_t n = count_if_invalid;
    if (auto* const_count = count_->As<ConstantArrayCount>()) {
        n = const_count->value;
    }
    return {element_, n};
}

const Type* Array::Element(uint32_t index) const {
    if (auto* count = count_->As<ConstantArrayCount>()) {
        return index < count->value ? element_ : nullptr;
    }
    return element_;
}

Array* Array::Clone(CloneContext& ctx) const {
    auto* elem_ty = element_->Clone(ctx);
    auto* count = count_->Clone(ctx);

    return ctx.dst.mgr->Get<Array>(elem_ty, count, size_);
}

}  // namespace tint::core::type
