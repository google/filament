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

#ifndef SRC_TINT_UTILS_RESULT_H_
#define SRC_TINT_UTILS_RESULT_H_

#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/traits.h"

namespace tint {

/// Empty structure that can be used as the SUCCESS_TYPE for a Result.
struct SuccessType {};

/// An instance of SuccessType that can be used as a generic success value for a Result.
static constexpr const SuccessType Success;

/// The default Result error type.
struct Failure {
    /// Constructor with no diagnostics
    Failure();

    /// Constructor with a single diagnostic
    /// @param err the single error diagnostic
    explicit Failure(std::string_view err);

    std::string reason;
};

/// Write the Failure to the given stream
/// @param out the output stream
/// @param failure the Failure
/// @returns the output stream
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, const Failure& failure) {
    return out << failure.reason;
}

/// Result is a helper for functions that need to return a value, or an failure value.
/// Result can be constructed with either a 'success' or 'failure' value.
/// @tparam SUCCESS_TYPE the 'success' value type.
/// @tparam FAILURE_TYPE the 'failure' value type. Defaults to FailureType which provides no
///         information about the failure, except that something failed. Must not be the same type
///         as SUCCESS_TYPE.
template <typename SUCCESS_TYPE, typename FAILURE_TYPE = Failure>
struct [[nodiscard]] Result {
    static_assert(!std::is_same_v<SUCCESS_TYPE, FAILURE_TYPE>,
                  "Result must not have the same type for SUCCESS_TYPE and FAILURE_TYPE");

    /// Default constructor initializes to invalid state
    Result() : value(std::monostate{}) {}

    /// Constructor
    /// @param success the success result
    Result(const SUCCESS_TYPE& success)  // NOLINT(runtime/explicit):
        : value{success} {}

    /// Constructor
    /// @param success the success result
    Result(SUCCESS_TYPE&& success)  // NOLINT(runtime/explicit):
        : value(std::move(SUCCESS_TYPE(std::move(success)))) {}

    /// Constructor
    /// @param failure the failure result
    Result(const FAILURE_TYPE& failure)  // NOLINT(runtime/explicit):
        : value{failure} {}

    /// Constructor
    /// @param failure the failure result
    Result(FAILURE_TYPE&& failure)  // NOLINT(runtime/explicit):
        : value{std::move(failure)} {}

    /// Copy constructor with success / failure casting
    /// @param other the Result to copy
    template <typename S,
              typename F,
              typename = std::void_t<decltype(SUCCESS_TYPE{std::declval<S>()}),
                                     decltype(FAILURE_TYPE{std::declval<F>()})>>
    Result(const Result<S, F>& other) {  // NOLINT(runtime/explicit):
        if (other == Success) {
            value = SUCCESS_TYPE{other.Get()};
        } else {
            value = FAILURE_TYPE{other.Failure()};
        }
    }

    /// @returns the success value
    /// @warning attempting to call this when the Result holds an failure will result in UB.
    const SUCCESS_TYPE* operator->() const {
        Validate();
        return &(Get());
    }

    /// @returns the success value
    /// @warning attempting to call this when the Result holds an failure will result in UB.
    SUCCESS_TYPE* operator->() {
        Validate();
        return &(Get());
    }

    /// @returns the success value
    /// @warning attempting to call this when the Result holds an failure value will result in UB.
    const SUCCESS_TYPE& Get() const {
        Validate();
        return std::get<SUCCESS_TYPE>(value);
    }

    /// @returns the success value
    /// @warning attempting to call this when the Result holds an failure value will result in UB.
    SUCCESS_TYPE& Get() {
        Validate();
        return std::get<SUCCESS_TYPE>(value);
    }

    /// @returns the success value
    /// @warning attempting to call this when the Result holds an failure value will result in UB.
    SUCCESS_TYPE&& Move() {
        Validate();
        return std::get<SUCCESS_TYPE>(std::move(value));
    }

    /// @returns the failure value
    /// @warning attempting to call this when the Result holds a success value will result in UB.
    const FAILURE_TYPE& Failure() const {
        Validate();
        return std::get<FAILURE_TYPE>(value);
    }

    /// Equality operator
    /// @param other the Result to compare this Result to
    /// @returns true if this Result is equal to @p other
    template <typename T>
    bool operator==(const Result& other) const {
        return value == other.value;
    }

    /// Equality operator
    /// @param val the value to compare this Result to
    /// @returns true if this result holds a success or failure value equal to `value`
    template <typename T>
    bool operator==(const T& val) const {
        Validate();

        using D = std::decay_t<T>;
        static constexpr bool is_success = std::is_same_v<D, tint::SuccessType>;  // T == Success
        static constexpr bool is_success_ty =
            std::is_same_v<D, SUCCESS_TYPE> ||
            (traits::IsStringLike<SUCCESS_TYPE> && traits::IsStringLike<D>);  // T == SUCCESS_TYPE
        static constexpr bool is_failure_ty =
            std::is_same_v<D, FAILURE_TYPE> ||
            (traits::IsStringLike<FAILURE_TYPE> && traits::IsStringLike<D>);  // T == FAILURE_TYPE

        static_assert(is_success || is_success_ty || is_failure_ty,
                      "unsupported type for Result equality operator");
        static_assert(!(is_success_ty && is_failure_ty),
                      "ambiguous success / failure type for Result equality operator");

        if constexpr (is_success) {
            return std::holds_alternative<SUCCESS_TYPE>(value);
        } else if constexpr (is_success_ty) {
            if (auto* v = std::get_if<SUCCESS_TYPE>(&value)) {
                return *v == val;
            }
            return false;
        } else if constexpr (is_failure_ty) {
            if (auto* v = std::get_if<FAILURE_TYPE>(&value)) {
                return *v == val;
            }
            return false;
        }
    }
    /// Inequality operator
    /// @param val the value to compare this Result to
    /// @returns false if this result holds a success or failure value equal to `value`
    template <typename T>
    bool operator!=(const T& val) const {
        return !(*this == val);
    }

  private:
    void Validate() const { TINT_ASSERT(!std::holds_alternative<std::monostate>(value)); }

    /// The result. Either a success of failure value.
    std::variant<std::monostate, SUCCESS_TYPE, FAILURE_TYPE> value;
};

/// Writes the result to the stream.
/// @param out the stream to write to
/// @param res the result
/// @return the stream so calls can be chained
template <typename STREAM, typename SUCCESS, typename FAILURE>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, const Result<SUCCESS, FAILURE>& res) {
    if (res == Success) {
        if constexpr (traits::HasOperatorShiftLeft<STREAM&, SUCCESS>) {
            return out << "success: " << res.Get();
        } else {
            return out << "success";
        }
    } else {
        if constexpr (traits::HasOperatorShiftLeft<STREAM&, FAILURE>) {
            return out << "failure: " << res.Failure();
        } else {
            return out << "failure";
        }
    }
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_RESULT_H_
