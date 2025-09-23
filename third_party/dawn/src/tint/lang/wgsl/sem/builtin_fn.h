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

#ifndef SRC_TINT_LANG_WGSL_SEM_BUILTIN_FN_H_
#define SRC_TINT_LANG_WGSL_SEM_BUILTIN_FN_H_

#include <string>
#include <vector>

#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/lang/wgsl/sem/call_target.h"
#include "src/tint/lang/wgsl/sem/pipeline_stage_set.h"
#include "src/tint/utils/math/hash.h"

// Forward declarations
namespace tint::core::intrinsic {
struct OverloadInfo;
}

namespace tint::sem {

/// BuiltinFn holds the semantic information for a builtin function.
class BuiltinFn final : public Castable<BuiltinFn, CallTarget> {
  public:
    /// Constructor
    /// @param type the builtin type
    /// @param return_type the return type for the builtin call
    /// @param parameters the parameters for the builtin overload
    /// @param eval_stage the earliest evaluation stage for a call to the builtin
    /// @param supported_stages the pipeline stages that this builtin can be used in
    /// @param overload the builtin table overload
    BuiltinFn(wgsl::BuiltinFn type,
              const core::type::Type* return_type,
              VectorRef<Parameter*> parameters,
              core::EvaluationStage eval_stage,
              PipelineStageSet supported_stages,
              const core::intrinsic::OverloadInfo& overload);

    /// Destructor
    ~BuiltinFn() override;

    /// @return the type of the builtin
    wgsl::BuiltinFn Fn() const { return fn_; }

    /// @return the pipeline stages that this builtin can be used in
    PipelineStageSet SupportedStages() const { return supported_stages_; }

    /// @return true if the builtin overload is considered deprecated
    bool IsDeprecated() const;

    /// @returns the name of the builtin function type. The spelling, including
    /// case, matches the name in the WGSL spec.
    const char* str() const;

    /// @returns true if builtin is a coarse derivative builtin
    bool IsCoarseDerivative() const;

    /// @returns true if builtin is a fine a derivative builtin
    bool IsFineDerivative() const;

    /// @returns true if builtin is a derivative builtin
    bool IsDerivative() const;

    /// @returns true if builtin is a texture operation builtin
    bool IsTexture() const;

    /// @returns true if builtin is a image query builtin
    bool IsImageQuery() const;

    /// @returns true if builtin is a data packing builtin
    bool IsDataPacking() const;

    /// @returns true if builtin is a data unpacking builtin
    bool IsDataUnpacking() const;

    /// @returns true if builtin is a barrier builtin
    bool IsBarrier() const;

    /// @returns true if builtin is a atomic builtin
    bool IsAtomic() const;

    /// @returns true if builtin is a builtin defined in the language extension
    /// `packed_4x8_integer_dot_product`.
    bool IsPacked4x8IntegerDotProductBuiltin() const;

    /// @returns true if builtin is a subgroup builtin (defined in the extension `subgroups`).
    bool IsSubgroup() const;

    /// @returns true if builtin is a subgroup matrix builtin (defined in the extension
    /// `subgroup_matrix`).
    bool IsSubgroupMatrix() const;

    /// @returns true if builtin is a quadSwap builtin
    bool IsQuadSwap() const;

    /// @returns true if builtin is a texel buffer builtin
    bool IsTexelBuffer() const;

    /// @returns true if intrinsic may have side-effects (i.e. writes to at least
    /// one of its inputs)
    bool HasSideEffects() const;

    /// @returns the required language feature of this builtin function. Returns
    /// wgsl::LanguageFeature::kUndefined if no language feature is required.
    wgsl::LanguageFeature RequiredLanguageFeature() const;

    /// @returns the builtin table overload info
    const core::intrinsic::OverloadInfo& Overload() const { return overload_; }

    /// @return the hash code for this object
    tint::HashCode HashCode() const {
        return Hash(Fn(), SupportedStages(), ReturnType(), Parameters(), IsDeprecated());
    }

  private:
    const wgsl::BuiltinFn fn_;
    const PipelineStageSet supported_stages_;
    const core::intrinsic::OverloadInfo& overload_;
};

/// Constant value used by the degrees() builtin
static constexpr double kRadToDeg = 57.295779513082322865;

/// Constant value used by the radians() builtin
static constexpr double kDegToRad = 0.017453292519943295474;

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_BUILTIN_FN_H_
