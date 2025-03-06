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

#include "src/tint/lang/core/type/binding_array.h"

#include <string>

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/symbol/symbol_table.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::BindingArray);

namespace tint::core::type {

BindingArray::BindingArray(const Type* element, uint32_t count)
    : Base(Hash(tint::TypeCode::Of<BindingArray>().bits, count), core::type::Flags{}),
      element_(element),
      count_(count) {
    TINT_ASSERT(element_);
}

bool BindingArray::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<BindingArray>()) {
        return o->element_ == element_ && o->count_ == count_;
    }
    return false;
}

std::string BindingArray::FriendlyName() const {
    StringStream out;
    out << "binding_array<" << element_->FriendlyName() << ", " << count_ << ">";
    return out.str();
}

TypeAndCount BindingArray::Elements([[maybe_unused]] const Type*, [[maybe_unused]] uint32_t) const {
    return {element_, count_};
}

const Type* BindingArray::Element(uint32_t index) const {
    return index < count_ ? element_ : nullptr;
}

BindingArray* BindingArray::Clone(CloneContext& ctx) const {
    auto* elem_ty = element_->Clone(ctx);
    return ctx.dst.mgr->Get<BindingArray>(elem_ty, count_);
}

}  // namespace tint::core::type
