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

#include "src/tint/lang/core/constant/splat.h"

#include "src/tint/lang/core/constant/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::constant::Splat);

namespace tint::core::constant {

namespace {

/// Asserts that the element type of @p in_type matches the type of @p value, and that the type has
/// at least one element.
/// @returns the number of elements in @p in_type
inline size_t GetCountAndAssertType(const core::type::Type* in_type, const constant::Value* value) {
    auto elements = in_type->Elements();
    TINT_ASSERT(!elements.type || elements.type == value->Type());
    TINT_ASSERT(elements.count > 0);
    return elements.count;
}

}  // namespace

Splat::Splat(const core::type::Type* t, const constant::Value* e)
    : type(t), el(e), count(GetCountAndAssertType(t, e)) {}

Splat::~Splat() = default;

const Splat* Splat::Clone(CloneContext& ctx) const {
    auto* ty = type->Clone(ctx.type_ctx);
    auto* element = el->Clone(ctx);
    return ctx.dst.Splat(ty, element);
}

}  // namespace tint::core::constant
