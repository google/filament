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

#ifndef SRC_TINT_LANG_CORE_IR_ANALYSIS_INTEGER_RANGE_ANALYSIS_H_
#define SRC_TINT_LANG_CORE_IR_ANALYSIS_INTEGER_RANGE_ANALYSIS_H_

#include <memory>
#include <variant>

#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/var.h"

namespace tint::core::ir::analysis {

/// The result of a integer range analysis: the upper and lower bound of a given integer variable.
/// The bound is inclusive, which means the value x being bound satisfies:
/// min_bound <= x <= max_bound.
struct IntegerRangeInfo {
    IntegerRangeInfo() = default;
    IntegerRangeInfo(int64_t min_bound, int64_t max_bound);
    IntegerRangeInfo(uint64_t min_bound, uint64_t max_bound);

    struct SignedIntegerRange {
        int64_t min_bound;
        int64_t max_bound;
    };
    struct UnsignedIntegerRange {
        uint64_t min_bound;
        uint64_t max_bound;
    };
    std::variant<std::monostate, SignedIntegerRange, UnsignedIntegerRange> range;
};

struct IntegerRangeAnalysisImpl;

/// IntegerRangeAnalysis is a helper used to analyze integer ranges.
class IntegerRangeAnalysis {
  public:
    /// Constructor
    /// @param func the function to cache analyses for
    explicit IntegerRangeAnalysis(ir::Function* func);
    ~IntegerRangeAnalysis();

    /// Returns the integer range info of a given parameter with given index, if it is an integer
    /// or an integer vector parameter. The index must not be over the maximum size of the vector
    /// and must be 0 if the parameter is an integer.
    /// Otherwise is not analyzable and returns nullptr. If it is the first time to query the info,
    /// the result will also be stored into a cache for future queries.
    /// @param param the variable to get information about
    /// @param index the vector component index when the parameter is a vector type. if the
    /// parameter is a scalar, then `index` must be zero.
    /// @returns the integer range info
    const IntegerRangeInfo* GetInfo(const FunctionParam* param, uint32_t index = 0);

  private:
    IntegerRangeAnalysis(const IntegerRangeAnalysis&) = delete;
    IntegerRangeAnalysis(IntegerRangeAnalysis&&) = delete;

    std::unique_ptr<IntegerRangeAnalysisImpl> impl_;
};

}  // namespace tint::core::ir::analysis

#endif  // SRC_TINT_LANG_CORE_IR_ANALYSIS_INTEGER_RANGE_ANALYSIS_H_
