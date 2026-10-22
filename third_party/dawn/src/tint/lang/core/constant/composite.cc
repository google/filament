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

#include "src/tint/lang/core/constant/composite.h"

#include <utility>

#include "src/tint/lang/core/constant/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Composite);

namespace tint::core::constant {

Composite::Composite(const core::type::Type* t, VectorRef<const Value*> els, bool all_0, bool any_0)
    : type(t), elements(std::move(els)), all_zero(all_0), any_zero(any_0), hash(CalcHash()) {
    const size_t n = elements.Length();
    TINT_ASSERT(n == t->Elements().count);
    for (size_t i = 0; i < n; i++) {
        TINT_ASSERT(t->Element(static_cast<uint32_t>(i)) == elements[i]->Type());
    }
}

Composite::~Composite() = default;

const Composite* Composite::Clone(CloneContext& ctx) const {
    auto* ty = type->Clone(ctx.type_ctx);
    Vector<const Value*, 4> els;
    for (const auto* el : elements) {
        els.Push(el->Clone(ctx));
    }
    return ctx.dst.Get<Composite>(ty, std::move(els), all_zero, any_zero);
}

}  // namespace tint::core::constant
