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

#include "src/tint/lang/wgsl/sem/array_count.h"

#include "src/tint/lang/wgsl/ast/identifier.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::NamedOverrideArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::sem::UnnamedOverrideArrayCount);

namespace tint::sem {

NamedOverrideArrayCount::NamedOverrideArrayCount(const GlobalVariable* var)
    : Base(static_cast<size_t>(tint::TypeCode::Of<NamedOverrideArrayCount>().bits)),
      variable(var) {}
NamedOverrideArrayCount::~NamedOverrideArrayCount() = default;

bool NamedOverrideArrayCount::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<NamedOverrideArrayCount>()) {
        return variable == v->variable;
    }
    return false;
}

std::string NamedOverrideArrayCount::FriendlyName() const {
    return variable->Declaration()->name->symbol.Name();
}

core::type::ArrayCount* NamedOverrideArrayCount::Clone(core::type::CloneContext&) const {
    TINT_UNREACHABLE() << "Named override array count clone not available";
}

UnnamedOverrideArrayCount::UnnamedOverrideArrayCount(const ValueExpression* e)
    : Base(static_cast<size_t>(tint::TypeCode::Of<UnnamedOverrideArrayCount>().bits)), expr(e) {}
UnnamedOverrideArrayCount::~UnnamedOverrideArrayCount() = default;

bool UnnamedOverrideArrayCount::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<UnnamedOverrideArrayCount>()) {
        return expr == v->expr;
    }
    return false;
}

std::string UnnamedOverrideArrayCount::FriendlyName() const {
    return "[unnamed override-expression]";
}

core::type::ArrayCount* UnnamedOverrideArrayCount::Clone(core::type::CloneContext&) const {
    TINT_UNREACHABLE() << "Unnamed override array count clone not available";
}

}  // namespace tint::sem
