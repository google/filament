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

#include "src/tint/lang/core/type/reference.h"

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/text/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::Reference);

namespace tint::core::type {

Reference::Reference(core::AddressSpace address_space, const Type* store_type, core::Access access)
    : Base(Hash(tint::TypeCode::Of<Reference>().bits), address_space, store_type, access) {}

bool Reference::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<Reference>()) {
        return o->AddressSpace() == AddressSpace() && o->StoreType() == StoreType() &&
               o->Access() == Access();
    }
    return false;
}

std::string Reference::FriendlyName() const {
    StringStream out;
    out << "ref<";
    if (AddressSpace() != core::AddressSpace::kUndefined) {
        out << AddressSpace() << ", ";
    }
    out << StoreType()->FriendlyName() << ", " << Access();
    out << ">";
    return out.str();
}

Reference::~Reference() = default;

Reference* Reference::Clone(CloneContext& ctx) const {
    auto* ty = StoreType()->Clone(ctx);
    return ctx.dst.mgr->Get<Reference>(AddressSpace(), ty, Access());
}

}  // namespace tint::core::type
