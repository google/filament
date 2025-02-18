// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_ERROR_H_
#define SRC_DAWN_NATIVE_ERROR_H_

#include <memory>
#include <string>
#include <utility>

#include "dawn/common/Result.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/webgpu_absl_format.h"

namespace dawn::native {

enum class InternalErrorType : uint32_t {
    None = 0,
    Validation = 1,
    DeviceLost = 2,
    Internal = 4,
    OutOfMemory = 8
};

// MaybeError and ResultOrError are meant to be used as return value for function that are not
// expected to, but might fail. The handling of error is potentially much slower than successes.
using MaybeError = Result<void, ErrorData>;

template <typename T>
using ResultOrError = Result<T, ErrorData>;

namespace detail {

template <typename T>
struct UnwrapResultOrError {
    using type = T;
};

template <typename T>
struct UnwrapResultOrError<ResultOrError<T>> {
    using type = T;
};

template <typename T>
struct IsResultOrError {
    static constexpr bool value = false;
};

template <typename T>
struct IsResultOrError<ResultOrError<T>> {
    static constexpr bool value = true;
};

}  // namespace detail

// Returning a success is done like so:
//   return {}; // for Error
//   return SomethingOfTypeT; // for ResultOrError<T>
//
// Returning an error is done via:
//   return DAWN_MAKE_ERROR(errorType, "My error message");
//
// but shorthand version for specific error types are preferred:
//   return DAWN_VALIDATION_ERROR("My error message with details %s", details);
//
// There are different types of errors that should be used for different purpose:
//
//   - Validation: these are errors that show the user did something bad, which causes the
//     whole call to be a no-op. It's most commonly found in the frontend but there can be some
//     backend specific validation in non-conformant backends too.
//
//   - Out of memory: creation of a Buffer or Texture failed because there isn't enough memory.
//     This is similar to validation errors in that the call becomes a no-op and returns an
//     error object, but is reported separated from validation to the user.
//
//   - Device loss: the backend driver reported that the GPU has been lost, which means all
//     previous commands magically disappeared and the only thing left to do is clean up.
//     Note: Device loss should be used rarely and in most case you want to use Internal
//     instead.
//
//   - Internal: something happened that the backend didn't expect, and it doesn't know
//     how to recover from that situation. This causes the device to be lost, but is separate
//     from device loss, because the GPU execution is still happening so we need to clean up
//     more gracefully.
//
//   - Unimplemented: same as Internal except it puts "unimplemented" in the error message for
//     more clarity.

#define DAWN_MAKE_ERROR(TYPE, MESSAGE) \
    ::dawn::native::ErrorData::Create(TYPE, MESSAGE, __FILE__, __func__, __LINE__)

#define DAWN_VALIDATION_ERROR(...) \
    DAWN_MAKE_ERROR(InternalErrorType::Validation, absl::StrFormat(__VA_ARGS__))

#define DAWN_INVALID_IF(EXPR, ...)                                                           \
    if (DAWN_UNLIKELY(EXPR)) {                                                               \
        return DAWN_MAKE_ERROR(InternalErrorType::Validation, absl::StrFormat(__VA_ARGS__)); \
    }                                                                                        \
    for (;;)                                                                                 \
    break

// DAWN_DEVICE_LOST_ERROR means that there was a real unrecoverable native device lost error.
// We can't even do a graceful shutdown because the Device is gone.
#define DAWN_DEVICE_LOST_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::DeviceLost, MESSAGE)

// DAWN_INTERNAL_ERROR means Dawn hit an unexpected error in the backend and should try to
// gracefully shut down.
#define DAWN_INTERNAL_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::Internal, MESSAGE)

#define DAWN_FORMAT_INTERNAL_ERROR(...) \
    DAWN_MAKE_ERROR(InternalErrorType::Internal, absl::StrFormat(__VA_ARGS__))

#define DAWN_INTERNAL_ERROR_IF(EXPR, ...)                                                  \
    if (DAWN_UNLIKELY(EXPR)) {                                                             \
        return DAWN_MAKE_ERROR(InternalErrorType::Internal, absl::StrFormat(__VA_ARGS__)); \
    }                                                                                      \
    for (;;)                                                                               \
    break

#define DAWN_UNIMPLEMENTED_ERROR(MESSAGE) \
    DAWN_MAKE_ERROR(InternalErrorType::Internal, std::string("Unimplemented: ") + MESSAGE)

// DAWN_OUT_OF_MEMORY_ERROR means we ran out of memory. It may be used as a signal internally in
// Dawn to free up unused resources. Or, it may bubble up to the application to signal an allocation
// was too large or they should free some existing resources.
#define DAWN_OUT_OF_MEMORY_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::OutOfMemory, MESSAGE)

template <typename T>
std::string MakeIncreaseLimitMessage(std::string_view limitName, T adapterLimitValue) {
    return absl::StrFormat(
        " This adapter supports a higher %s of %u, which can be specified in requiredLimits when "
        "calling requestDevice(). Limits differ by hardware, so always check the adapter limits "
        "prior to requesting a higher limit.",
        limitName, adapterLimitValue);
}

#define DAWN_INCREASE_LIMIT_MESSAGE(adapter, limitName, limitValue)                      \
    [&]() -> std::string {                                                               \
        ::dawn::native::SupportedLimits adapterLimits;                                   \
        wgpu::Status status = adapter->APIGetLimits(&adapterLimits);                     \
        DAWN_ASSERT(status == wgpu::Status::Success);                                    \
        if (limitValue > adapterLimits.limits.limitName) {                               \
            return "";                                                                   \
        }                                                                                \
        return ::dawn::native::MakeIncreaseLimitMessage(#limitName,                      \
                                                        adapterLimits.limits.limitName); \
    }()

#define DAWN_CONCAT1(x, y) x##y
#define DAWN_CONCAT2(x, y) DAWN_CONCAT1(x, y)
#define DAWN_LOCAL_VAR(name) DAWN_CONCAT2(DAWN_CONCAT2(_localVar, __LINE__), name)

// Backtrace information adds a lot of binary size with the name of all the files and functions,
// plus additional calls to AppendBacktrace. Only add the backtrace in Debug so as to save binary
// size in release. Most backtrace information useful to developers is already added via
// DAWN_TRY_CONTEXT anyway.
#if defined(DAWN_ENABLE_ASSERTS)
#define DAWN_APPEND_ERROR_BACKTRACE(error) error->AppendBacktrace(__FILE__, __func__, __LINE__)
#else  // defined(DAWN_ENABLE_ASSERTS)
#define DAWN_APPEND_ERROR_BACKTRACE(error) \
    for (;;)                               \
    break
#endif  // defined(DAWN_ENABLE_ASSERTS)

// When Errors aren't handled explicitly, calls to functions returning errors should be
// wrapped in an DAWN_TRY. It will return the error if any, otherwise keep executing
// the current function.
#define DAWN_TRY(EXPR) DAWN_TRY_WITH_CLEANUP(EXPR, {})

#define DAWN_TRY_CONTEXT(EXPR, ...) \
    DAWN_TRY_WITH_CLEANUP(EXPR,     \
                          { DAWN_LOCAL_VAR(Error)->AppendContext(absl::StrFormat(__VA_ARGS__)); })

#define DAWN_TRY_WITH_CLEANUP(EXPR, BODY)                                       \
    {                                                                           \
        auto DAWN_LOCAL_VAR(Result) = EXPR;                                     \
        if (DAWN_UNLIKELY(DAWN_LOCAL_VAR(Result).IsError())) {                  \
            auto DAWN_LOCAL_VAR(Error) = DAWN_LOCAL_VAR(Result).AcquireError(); \
            {BODY} /* comment to force the formatter to insert a newline */     \
            DAWN_APPEND_ERROR_BACKTRACE(DAWN_LOCAL_VAR(Error));                 \
            return {std::move(DAWN_LOCAL_VAR(Error))};                          \
        }                                                                       \
    }                                                                           \
    for (;;)                                                                    \
    break

// DAWN_TRY_ASSIGN is the same as DAWN_TRY for ResultOrError and assigns the success value, if
// any, to VAR.
#define DAWN_TRY_ASSIGN(VAR, EXPR) DAWN_TRY_ASSIGN_WITH_CLEANUP(VAR, EXPR, {})
#define DAWN_TRY_ASSIGN_CONTEXT(VAR, EXPR, ...) \
    DAWN_TRY_ASSIGN_WITH_CLEANUP(               \
        VAR, EXPR, { DAWN_LOCAL_VAR(Error)->AppendContext(absl::StrFormat(__VA_ARGS__)); })

// Argument helpers are used to determine which macro implementations should be called when
// overloading with different number of variables.
#define DAWN_ERROR_UNIMPLEMENTED_MACRO_(...) DAWN_UNREACHABLE()
#define DAWN_ERROR_GET_5TH_ARG_HELPER_(_1, _2, _3, _4, NAME, ...) NAME
#define DAWN_ERROR_GET_5TH_ARG_(args) DAWN_ERROR_GET_5TH_ARG_HELPER_ args

// DAWN_TRY_ASSIGN_WITH_CLEANUP is overloaded with 2 version so that users can override the
// return value of the macro when necessary. This is particularly useful if the function
// calling the macro may want to return void instead of the error, i.e. in a test where we may
// just want to assert and fail if the assign cannot go through. In both the cleanup and return
// clauses, users can use the `DAWN_LOCAL_VAR(Error)` variable to access the pointer to the
// acquired error.
//
// Example usages:
//     3 Argument Case:
//          Result res;
//          DAWN_TRY_ASSIGN_WITH_CLEANUP(
//              res, GetResultOrErrorFunction(), {
//                  AddAdditionalErrorInformation(DAWN_LOCAL_VAR(Error).get());
//              });
//
//     4 Argument Case:
//          bool FunctionThatReturnsBool() {
//              DAWN_TRY_ASSIGN_WITH_CLEANUP(
//                  res, GetResultOrErrorFunction(), {
//                      AddAdditionalErrorInformation(DAWN_LOCAL_VAR(Error).get());
//                  },
//                  false
//              );
//          }
#define DAWN_TRY_ASSIGN_WITH_CLEANUP(...)                                       \
    DAWN_ERROR_GET_5TH_ARG_((__VA_ARGS__, DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_4_, \
                             DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_3_,              \
                             DAWN_ERROR_UNIMPLEMENTED_MACRO_))                  \
    (__VA_ARGS__)

#define DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_3_(VAR, EXPR, BODY) \
    DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_4_(VAR, EXPR, BODY, std::move(DAWN_LOCAL_VAR(Error)))

#define DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_4_(VAR, EXPR, BODY, RET)              \
    {                                                                           \
        auto DAWN_LOCAL_VAR(Result) = EXPR;                                     \
        if (DAWN_UNLIKELY(DAWN_LOCAL_VAR(Result).IsError())) {                  \
            auto DAWN_LOCAL_VAR(Error) = DAWN_LOCAL_VAR(Result).AcquireError(); \
            {BODY} /* comment to force the formatter to insert a newline */     \
            DAWN_APPEND_ERROR_BACKTRACE(DAWN_LOCAL_VAR(Error));                 \
            return (RET);                                                       \
        }                                                                       \
        VAR = DAWN_LOCAL_VAR(Result).AcquireSuccess();                          \
    }                                                                           \
    for (;;)                                                                    \
    break

// Assert that errors are device loss so that we can continue with destruction
void IgnoreErrors(MaybeError maybeError);

wgpu::ErrorType ToWGPUErrorType(InternalErrorType type);
InternalErrorType FromWGPUErrorType(wgpu::ErrorType type);

absl::FormatConvertResult<absl::FormatConversionCharSet::kString |
                          absl::FormatConversionCharSet::kIntegral>
AbslFormatConvert(InternalErrorType value,
                  const absl::FormatConversionSpec& spec,
                  absl::FormatSink* s);

}  // namespace dawn::native

// Enable dawn enum bitmask for error types.
template <>
struct wgpu::IsWGPUBitmask<dawn::native::InternalErrorType> {
    static constexpr bool enable = true;
};

#endif  // SRC_DAWN_NATIVE_ERROR_H_
