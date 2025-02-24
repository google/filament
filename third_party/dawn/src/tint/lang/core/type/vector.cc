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

#include "src/tint/lang/core/type/vector.h"

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::Vector);

namespace tint::core::type {

Vector::Vector(type::Type const* subtype, uint32_t width, bool packed /* = false */)
    : Base(Hash(tint::TypeCode::Of<Vector>().bits, width, subtype, packed),
           core::type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }),
      subtype_(subtype),
      width_(width),
      packed_(packed) {
    TINT_ASSERT(width_ > 1);
    TINT_ASSERT(width_ < 5);
}

Vector::~Vector() = default;

bool Vector::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<Vector>()) {
        return v->width_ == width_ && v->subtype_ == subtype_ && v->packed_ == packed_;
    }
    return false;
}

std::string Vector::FriendlyName() const {
    StringStream out;
    if (packed_) {
        out << "__packed_";
    }
    out << "vec" << width_ << "<" << subtype_->FriendlyName() << ">";
    return out.str();
}

uint32_t Vector::Size() const {
    return subtype_->Size() * width_;
}

uint32_t Vector::Align() const {
    switch (width_) {
        case 2:
            return subtype_->Size() * 2;
        case 3:
            return subtype_->Size() * (packed_ ? 1 : 4);
        case 4:
            return subtype_->Size() * 4;
    }
    return 0;  // Unreachable
}

Vector* Vector::Clone(CloneContext& ctx) const {
    auto* subtype = subtype_->Clone(ctx);
    return ctx.dst.mgr->Get<Vector>(subtype, width_, packed_);
}

TypeAndCount Vector::Elements(const type::Type* /* type_if_invalid = nullptr */,
                              uint32_t /* count_if_invalid = 0 */) const {
    return {subtype_, width_};
}

const Type* Vector::Element(uint32_t index) const {
    return index < width_ ? subtype_ : nullptr;
}

}  // namespace tint::core::type
