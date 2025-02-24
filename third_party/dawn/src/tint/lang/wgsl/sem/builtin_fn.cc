// Copyright 2020 The Dawn & Tint Authors
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

// Doxygen seems to trip over this file for some unknown reason. Disable.
//! @cond Doxygen_Suppress

#include "src/tint/lang/wgsl/sem/builtin_fn.h"

#include <utility>
#include <vector>

#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/utils/containers/transform.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::BuiltinFn);

namespace tint::sem {

const char* BuiltinFn::str() const {
    return wgsl::str(fn_);
}

BuiltinFn::BuiltinFn(wgsl::BuiltinFn type,
                     const core::type::Type* return_type,
                     VectorRef<Parameter*> parameters,
                     core::EvaluationStage eval_stage,
                     PipelineStageSet supported_stages,
                     const core::intrinsic::OverloadInfo& overload)
    : Base(return_type,
           std::move(parameters),
           eval_stage,
           overload.flags.Contains(core::intrinsic::OverloadFlag::kMustUse)),
      fn_(type),
      supported_stages_(supported_stages),
      overload_(overload) {}

BuiltinFn::~BuiltinFn() = default;

bool BuiltinFn::IsDeprecated() const {
    return overload_.flags.Contains(core::intrinsic::OverloadFlag::kIsDeprecated);
}

bool BuiltinFn::IsCoarseDerivative() const {
    return wgsl::IsCoarseDerivative(fn_);
}

bool BuiltinFn::IsFineDerivative() const {
    return wgsl::IsFineDerivative(fn_);
}

bool BuiltinFn::IsDerivative() const {
    return wgsl::IsDerivative(fn_);
}

bool BuiltinFn::IsTexture() const {
    return wgsl::IsTexture(fn_);
}

bool BuiltinFn::IsImageQuery() const {
    return wgsl::IsImageQuery(fn_);
}

bool BuiltinFn::IsDataPacking() const {
    return wgsl::IsDataPacking(fn_);
}

bool BuiltinFn::IsDataUnpacking() const {
    return wgsl::IsDataUnpacking(fn_);
}

bool BuiltinFn::IsBarrier() const {
    return wgsl::IsBarrier(fn_);
}

bool BuiltinFn::IsAtomic() const {
    return wgsl::IsAtomic(fn_);
}

bool BuiltinFn::IsPacked4x8IntegerDotProductBuiltin() const {
    return wgsl::IsPacked4x8IntegerDotProductBuiltin(fn_);
}

bool BuiltinFn::IsSubgroup() const {
    return wgsl::IsSubgroup(fn_);
}

bool BuiltinFn::IsQuadSwap() const {
    return wgsl::IsQuadSwap(fn_);
}

bool BuiltinFn::HasSideEffects() const {
    return wgsl::HasSideEffects(fn_);
}

wgsl::LanguageFeature BuiltinFn::RequiredLanguageFeature() const {
    if (fn_ == wgsl::BuiltinFn::kTextureBarrier) {
        return wgsl::LanguageFeature::kReadonlyAndReadwriteStorageTextures;
    }
    if (IsPacked4x8IntegerDotProductBuiltin()) {
        return wgsl::LanguageFeature::kPacked4X8IntegerDotProduct;
    }
    return wgsl::LanguageFeature::kUndefined;
}

}  // namespace tint::sem

//! @endcond
