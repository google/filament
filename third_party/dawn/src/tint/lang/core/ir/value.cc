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

#include "src/tint/lang/core/ir/value.h"

#include <algorithm>

#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Value);

namespace tint::core::ir {

Value::Value(const core::type::Type* type) : type_(type) {}

Value::~Value() = default;

void Value::Destroy() {
    TINT_ASSERT(Alive());
    flags_.Add(Flag::kDead);
}

void Value::ForEachUseUnsorted(std::function<void(Usage use)> func) const {
    auto uses = uses_;
    for (auto& use : uses) {
        func(use);
    }
}

void Value::ForEachUseSorted(std::function<void(Usage use)> func) const {
    auto uses = UsagesSorted();
    for (auto& use : uses) {
        func(use);
    }
}

void Value::ReplaceAllUsesWith(std::function<Value*(Usage use)> replacer) {
    while (!uses_.IsEmpty()) {
        auto& use = *uses_.begin();
        auto* replacement = replacer(use);
        use->instruction->SetOperand(use->operand_index, replacement);
    }
}

void Value::ReplaceAllUsesWith(Value* replacement) {
    while (!uses_.IsEmpty()) {
        auto& use = *uses_.begin();
        use->instruction->SetOperand(use->operand_index, replacement);
    }
}

Vector<Usage, 4> Value::UsagesSorted() const {
    auto v = uses_.Vector();
    std::sort(v.begin(), v.end());
    return v;
}

}  // namespace tint::core::ir
