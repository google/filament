// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/analysis/integer_range_analysis.h"

#include <limits>

#include "src/tint/lang/core/ir/traverse.h"

namespace tint::core::ir::analysis {

IntegerRangeInfo::IntegerRangeInfo(int64_t min_bound, int64_t max_bound) {
    TINT_ASSERT(min_bound <= max_bound);
    range = SignedIntegerRange{min_bound, max_bound};
}

IntegerRangeInfo::IntegerRangeInfo(uint64_t min_bound, uint64_t max_bound) {
    TINT_ASSERT(min_bound <= max_bound);
    range = UnsignedIntegerRange{min_bound, max_bound};
}

struct IntegerRangeAnalysisImpl {
    explicit IntegerRangeAnalysisImpl(Function* func) : function_(func) {}

    const IntegerRangeInfo* GetInfo(const FunctionParam* param, uint32_t index) {
        if (!param->Type()->IsIntegerScalarOrVector()) {
            return nullptr;
        }

        const auto& info = integer_function_param_range_info_map_.GetOrAdd(
            param, [&]() -> Vector<IntegerRangeInfo, 3> {
                if (param->Builtin() == core::BuiltinValue::kLocalInvocationIndex) {
                    // We shouldn't be trying to use range analysis on a module that has
                    // non-constant workgroup sizes, since we will always have replaced pipeline
                    // overrides with constant values early in the pipeline.
                    TINT_ASSERT(function_->WorkgroupSizeAsConst().has_value());
                    std::array<uint32_t, 3> workgroup_size =
                        function_->WorkgroupSizeAsConst().value();
                    uint64_t max_bound =
                        workgroup_size[0] * workgroup_size[1] * workgroup_size[2] - 1u;
                    constexpr uint64_t kMinBound = 0;

                    return {IntegerRangeInfo(kMinBound, max_bound)};
                }

                if (param->Builtin() == core::BuiltinValue::kLocalInvocationId) {
                    TINT_ASSERT(function_->WorkgroupSizeAsConst().has_value());
                    std::array<uint32_t, 3> workgroup_size =
                        function_->WorkgroupSizeAsConst().value();

                    constexpr uint64_t kMinBound = 0;
                    Vector<IntegerRangeInfo, 3> integerRanges;
                    for (uint32_t size_x_y_z : workgroup_size) {
                        integerRanges.Push({kMinBound, size_x_y_z - 1u});
                    }
                    return integerRanges;
                }

                if (param->Type()->IsUnsignedIntegerScalar()) {
                    return {IntegerRangeInfo(0, std::numeric_limits<uint64_t>::max())};
                } else {
                    TINT_ASSERT(param->Type()->IsSignedIntegerScalar());
                    return {IntegerRangeInfo(std::numeric_limits<int64_t>::min(),
                                             std::numeric_limits<int64_t>::max())};
                }
            });

        TINT_ASSERT(info.Length() > index);
        return &info[index];
    }

  private:
    Function* function_;
    Hashmap<const FunctionParam*, Vector<IntegerRangeInfo, 3>, 4>
        integer_function_param_range_info_map_;
};

IntegerRangeAnalysis::IntegerRangeAnalysis(Function* func)
    : impl_(new IntegerRangeAnalysisImpl(func)) {}
IntegerRangeAnalysis::~IntegerRangeAnalysis() = default;

const IntegerRangeInfo* IntegerRangeAnalysis::GetInfo(const FunctionParam* param, uint32_t index) {
    return impl_->GetInfo(param, index);
}
}  // namespace tint::core::ir::analysis
