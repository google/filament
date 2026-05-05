// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_REPLAY_ERROR_H_
#define SRC_DAWN_REPLAY_ERROR_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "absl/strings/str_format.h"
#include "dawn/common/Result.h"

namespace dawn::replay {

enum class InternalErrorType : uint32_t { None = 0, Internal = 1, OutOfMemory = 2 };

class [[nodiscard]] ErrorData {
  public:
    [[nodiscard]] static std::unique_ptr<ErrorData> Create(InternalErrorType type,
                                                           std::string message,
                                                           const char* file,
                                                           const char* function,
                                                           int line);
    ErrorData(InternalErrorType type, std::string message);

    InternalErrorType GetType() const;
    const std::string& GetMessage() const;

  private:
    InternalErrorType mType;
    std::string mMessage;
};

// MaybeError and ResultOrError are meant to be used as return value for function that are not
// expected to, but might fail. The handling of error is potentially much slower than successes.
using MaybeError = Result<void, ErrorData>;

template <typename T>
using ResultOrError = Result<T, ErrorData>;

// Returning a success is done like so:
//   return {}; // for Error
//   return SomethingOfTypeT; // for ResultOrError<T>
//
// Returning an error is done via:
//   return DAWN_MAKE_ERROR(errorType, "My error message");
//
// but shorthand version for specific error types are preferred:
//   return DAWN_INTERNAL_ERROR("My error message with details %s", details);
//
// There are different types of errors that should be used for different purpose:
//
//   - Out of memory: Replay ran out of memory.
//
//   - Internal: something happened that the replay didn't expect.
//
#define DAWN_MAKE_ERROR(TYPE, MESSAGE) \
    ::dawn::replay::ErrorData::Create(TYPE, MESSAGE, __FILE__, __func__, __LINE__)

// DAWN_INTERNAL_ERROR means Replay hit an unexpected error in the backend and should try to
// gracefully shut down.
#define DAWN_INTERNAL_ERROR(...) \
    DAWN_MAKE_ERROR(InternalErrorType::Internal, absl::StrFormat(__VA_ARGS__))

#define DAWN_FORMAT_INTERNAL_ERROR(...) \
    DAWN_MAKE_ERROR(InternalErrorType::Internal, absl::StrFormat(__VA_ARGS__))

#define DAWN_INTERNAL_ERROR_IF(EXPR, ...)                                                  \
    if (EXPR) [[unlikely]] {                                                               \
        return DAWN_MAKE_ERROR(InternalErrorType::Internal, absl::StrFormat(__VA_ARGS__)); \
    }                                                                                      \
    for (;;)                                                                               \
    break

#define DAWN_UNIMPLEMENTED_ERROR(MESSAGE) \
    DAWN_MAKE_ERROR(InternalErrorType::Internal, std::string("Unimplemented: ") + MESSAGE)

// DAWN_OUT_OF_MEMORY_ERROR means we ran out of memory. It may be used as a signal internally
// in Dawn to free up unused resources. Or, it may bubble up to the application to signal an
// allocation was too large or they should free some existing resources.
#define DAWN_OUT_OF_MEMORY_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::OutOfMemory, MESSAGE)

#define DAWN_CONCAT1(x, y) x##y
#define DAWN_CONCAT2(x, y) DAWN_CONCAT1(x, y)
#define DAWN_LOCAL_VAR(name) DAWN_CONCAT2(DAWN_CONCAT2(_localVar, __LINE__), name)

// When Errors aren't handled explicitly, calls to functions returning errors should be
// wrapped in an DAWN_TRY. It will return the error if any, otherwise keep executing
// the current function.
#define DAWN_TRY(EXPR) DAWN_TRY_WITH_CLEANUP(EXPR, {})

#define DAWN_TRY_WITH_CLEANUP(EXPR, BODY)                                       \
    {                                                                           \
        auto DAWN_LOCAL_VAR(Result) = EXPR;                                     \
        if (DAWN_LOCAL_VAR(Result).IsError()) [[unlikely]] {                    \
            auto DAWN_LOCAL_VAR(Error) = DAWN_LOCAL_VAR(Result).AcquireError(); \
            {                                                                   \
                BODY                                                            \
            } /* comment to force the formatter to insert a newline */          \
            return {std::move(DAWN_LOCAL_VAR(Error))};                          \
        }                                                                       \
    }                                                                           \
    for (;;)                                                                    \
    break

// DAWN_TRY_ASSIGN is the same as DAWN_TRY for ResultOrError and assigns the success
// value, if any, to VAR.
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
        if (DAWN_LOCAL_VAR(Result).IsError()) [[unlikely]] {                    \
            auto DAWN_LOCAL_VAR(Error) = DAWN_LOCAL_VAR(Result).AcquireError(); \
            {                                                                   \
                BODY                                                            \
            } /* comment to force the formatter to insert a newline */          \
            return (RET);                                                       \
        }                                                                       \
        VAR = DAWN_LOCAL_VAR(Result).AcquireSuccess();                          \
    }                                                                           \
    for (;;)                                                                    \
    break

}  // namespace dawn::replay

#endif  // SRC_DAWN_REPLAY_ERROR_H_
