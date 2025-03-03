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

#include "src/tint/lang/core/type/array_count.h"

#include "src/tint/lang/core/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::ArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::core::type::ConstantArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::core::type::RuntimeArrayCount);

namespace tint::core::type {

ArrayCount::ArrayCount(size_t hash) : Base(hash) {}
ArrayCount::~ArrayCount() = default;

ConstantArrayCount::ConstantArrayCount(uint32_t val)
    : Base(static_cast<size_t>(tint::TypeCode::Of<ConstantArrayCount>().bits)), value(val) {}
ConstantArrayCount::~ConstantArrayCount() = default;

bool ConstantArrayCount::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<ConstantArrayCount>()) {
        return value == v->value;
    }
    return false;
}

std::string ConstantArrayCount::FriendlyName() const {
    return std::to_string(value);
}

ConstantArrayCount* ConstantArrayCount::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<ConstantArrayCount>(value);
}

RuntimeArrayCount::RuntimeArrayCount()
    : Base(static_cast<size_t>(tint::TypeCode::Of<RuntimeArrayCount>().bits)) {}
RuntimeArrayCount::~RuntimeArrayCount() = default;

bool RuntimeArrayCount::Equals(const UniqueNode& other) const {
    return other.Is<RuntimeArrayCount>();
}

std::string RuntimeArrayCount::FriendlyName() const {
    return "";
}

RuntimeArrayCount* RuntimeArrayCount::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<RuntimeArrayCount>();
}

}  // namespace tint::core::type
