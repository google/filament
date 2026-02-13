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

#include "src/tint/lang/core/type/atomic.h"

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::Atomic);

namespace tint::core::type {

Atomic::Atomic(const core::type::Type* subtype)
    : Base(Hash(tint::TypeCode::Of<Atomic>().bits, subtype),
           core::type::Flags{
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
               Flag::kHostShareable,
           }),
      subtype_(subtype) {
    TINT_ASSERT(!subtype->Is<Reference>());
}

bool Atomic::Equals(const core::type::UniqueNode& other) const {
    if (auto* o = other.As<Atomic>()) {
        return o->subtype_ == subtype_;
    }
    return false;
}

std::string Atomic::FriendlyName() const {
    StringStream out;
    out << "atomic<" << subtype_->FriendlyName() << ">";
    return out.str();
}

uint32_t Atomic::Size() const {
    return subtype_->Size();
}

uint32_t Atomic::Align() const {
    return subtype_->Align();
}

Atomic::~Atomic() = default;

Atomic* Atomic::Clone(CloneContext& ctx) const {
    auto* ty = subtype_->Clone(ctx);
    return ctx.dst.mgr->Get<Atomic>(ty);
}

}  // namespace tint::core::type
