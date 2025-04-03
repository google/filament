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

namespace tint::core::ir {
class Binary;
class Function;
class FunctionParam;
class Loop;
class Var;
}  // namespace tint::core::ir

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
    explicit IntegerRangeAnalysis(Function* func);
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

    /// Note: This function is only for tests.
    /// Returns the pointer of the loop control variable in the given loop when its initializer
    /// meets the below requirements.
    /// - There are only two instructions in the loop initializer block.
    /// - The first instruction is to initialize the loop control variable
    ///   with a constant integer (signed or unsigned) value.
    /// - The second instruction is `next_iteration`.
    /// @param loop the Loop variable to investigate
    /// @returns the pointer of the loop control variable when its loop initializer meets the
    /// requirements, return nullptr otherwise.
    const Var* GetLoopControlVariableFromConstantInitializerForTest(const Loop* loop);

    /// Note: This function is only for tests.
    /// Returns the pointer of the binary operation that updates the loop control variable in the
    /// continuing block of the given loop if the loop meets the below requirements.
    /// - There are only 4 instructions in the loop initializer block.
    /// - The first instruction is to load the loop control variable into a temporary variable.
    /// - The second instruction is to add one or minus one to the temporary variable.
    /// - The third instruction is to store the value of the temporary variable into the loop
    ///   control variable.
    /// - The fourth instruction is `next_iteration`.
    /// @param loop the Loop variable to investigate.
    /// @param loop_control_variable the loop control variable to investigate.
    /// @returns the pointer of the binary operation that updates the loop control variable in the
    /// continuing block of the given loop if the loop meets all the requirements, return nullptr
    /// otherwise.
    const Binary* GetBinaryToUpdateLoopControlVariableInContinuingBlockForTest(
        const Loop* loop,
        const Var* loop_control_variable);

    /// Note: This function is only for tests
    /// Returns the pointer of the binary operation that compares the loop control variable with its
    /// limitations in the body block of the loop if the loop meets the below requirements:
    /// - The loop control variable is only used as the parameter of the load instruction.
    /// - The first instruction is to load the loop control variable into a temporary variable.
    /// - The second instruction is to compare the temporary variable with a constant value and save
    ///   the result to a boolean variable.
    /// - The second instruction cannot be a comparison that will never return true.
    /// - The third instruction is an `ifelse` expression that uses the boolean variable got in the
    ///   second instruction as the condition.
    // - The true block of the above `ifelse` expression doesn't contain `exit_loop`.
    // - The false block of the above `ifelse` expression only contains `exit_loop`.
    /// @param loop the Loop variable to investigate.
    /// @param loop_control_variable the loop control variable to investigate.
    /// @returns the pointer of the binary operation that compares the loop control variable with
    /// its limitations in the body block of the loop if the loop meets the below requirements,
    /// return nullptr otherwise.
    const Binary* GetBinaryToCompareLoopControlVariableInLoopBodyForTest(
        const Loop* loop,
        const Var* loop_control_variable);

  private:
    IntegerRangeAnalysis(const IntegerRangeAnalysis&) = delete;
    IntegerRangeAnalysis(IntegerRangeAnalysis&&) = delete;

    std::unique_ptr<IntegerRangeAnalysisImpl> impl_;
};

}  // namespace tint::core::ir::analysis

#endif  // SRC_TINT_LANG_CORE_IR_ANALYSIS_INTEGER_RANGE_ANALYSIS_H_
