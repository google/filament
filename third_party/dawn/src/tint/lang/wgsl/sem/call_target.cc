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

#include "src/tint/lang/wgsl/sem/call_target.h"

#include <utility>

#include "src/tint/utils/math/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::CallTarget);

namespace tint::sem {

CallTarget::CallTarget(core::EvaluationStage stage, bool must_use)
    : stage_(stage), must_use_(must_use) {}

CallTarget::CallTarget(const core::type::Type* return_type,
                       VectorRef<Parameter*> parameters,
                       core::EvaluationStage stage,
                       bool must_use)
    : stage_(stage), must_use_(must_use) {
    SetReturnType(return_type);
    for (auto* param : parameters) {
        AddParameter(param);
    }
    TINT_ASSERT(return_type);
}

CallTarget::CallTarget(const CallTarget&) = default;
CallTarget::~CallTarget() = default;

CallTargetSignature::CallTargetSignature() = default;

CallTargetSignature::CallTargetSignature(const core::type::Type* ret_ty,
                                         VectorRef<const sem::Parameter*> params)
    : return_type(ret_ty), parameters(std::move(params)) {}
CallTargetSignature::CallTargetSignature(const CallTargetSignature&) = default;
CallTargetSignature::~CallTargetSignature() = default;

int CallTargetSignature::IndexOf(core::ParameterUsage usage) const {
    for (size_t i = 0; i < parameters.Length(); i++) {
        if (parameters[i]->Usage() == usage) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

tint::HashCode CallTargetSignature::HashCode() const {
    auto hash = tint::Hash(parameters.Length());
    for (auto* p : parameters) {
        hash = HashCombine(hash, p->Type(), p->Usage());
    }
    return Hash(hash, return_type);
}

bool CallTargetSignature::operator==(const CallTargetSignature& other) const {
    if (return_type != other.return_type || parameters.Length() != other.parameters.Length()) {
        return false;
    }
    for (size_t i = 0; i < parameters.Length(); i++) {
        auto* a = parameters[i];
        auto* b = other.parameters[i];
        if (a->Type() != b->Type() || a->Usage() != b->Usage()) {
            return false;
        }
    }
    return true;
}

}  // namespace tint::sem
