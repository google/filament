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

#ifndef SRC_TINT_UTILS_ICE_ICE_H_
#define SRC_TINT_UTILS_ICE_ICE_H_

#include <optional>
#include <sstream>
#include <string>
#include <utility>

#include "src/tint/utils/macros/compiler.h"

namespace tint {

/// An instance of InternalCompilerErrorCallbackInfo can be used to set up a callback so that the
/// user of Tint can capture an ICE message before crashing.
struct InternalCompilerErrorCallbackInfo {
    void (*callback)(std::string err, void* userdata) = nullptr;
    void* userdata = nullptr;
};
using InternalCompilerErrorCallback = std::optional<InternalCompilerErrorCallbackInfo>;

/// InternalCompilerError is a helper for reporting internal compiler errors.
/// Construct the InternalCompilerError with the source location of the ICE fault and append any
/// error details with the `<<` operator. When the InternalCompilerError is destructed, the
/// concatenated error message is passed to the provided ICE callback, or printed to stderr.
class InternalCompilerError {
  public:
    /// Constructor
    /// @param file the file containing the ICE
    /// @param line the line containing the ICE
    /// @param callback an optional callback to call with the error message
    InternalCompilerError(const char* file,
                          size_t line,
                          InternalCompilerErrorCallback callback = std::nullopt);

    /// Destructor.
    /// Adds the internal compiler error message to the diagnostics list, calls the
    /// InternalCompilerErrorReporter if one is set, then terminates the process.
    [[noreturn]] ~InternalCompilerError();

    /// Appends `arg` to the ICE message.
    /// @param arg the argument to append to the ICE message
    /// @returns this object so calls can be chained
    template <typename T>
    InternalCompilerError& operator<<(T&& arg) {
        msg_ << std::forward<T>(arg);
        return *this;
    }

    /// @returns the file that triggered the ICE
    const char* File() const { return file_; }

    /// @returns the line that triggered the ICE
    size_t Line() const { return line_; }

    /// @returns the ICE message
    std::string Message() const { return msg_.str(); }

    /// @returns the ICE file, line and message
    std::string Error() const;

    /// This operator overload exists so that we can use an InternalCompilerError object on the
    /// right-hand side of a short-circuiting operator, which is how the TINT_ASSERT macro works.
    operator bool() const { return false; }

  private:
    InternalCompilerError(const InternalCompilerError&) = delete;
    InternalCompilerError(InternalCompilerError&&) = delete;

    char const* const file_;
    const size_t line_;
    std::stringstream msg_;
    const InternalCompilerErrorCallback callback_info_;
};

}  // namespace tint

/// TINT_ICE() is a macro to produce an Internal Compiler Error. The ICE message contains the
/// callsite's file and line. Use the `<<` operator to append an error message to the ICE. An
/// optional callback can be provided so that the user of Tint can capture the ICE before crashing.
#define TINT_ICE(...) tint::InternalCompilerError(__FILE__, __LINE__ __VA_OPT__(, __VA_ARGS__))

/// TINT_UNREACHABLE() is a macro produce an Internal Compiler Error when an expectedly unreachable
/// statement is reached. The ICE message contains the callsite's file and line. Use the `<<`
/// operator to append an error message to the ICE. An optional callback can be provided so that the
/// user of Tint can capture the ICE before crashing.
#define TINT_UNREACHABLE(...) TINT_ICE(__VA_ARGS__) << "TINT_UNREACHABLE "

/// TINT_UNIMPLEMENTED() is a macro to produce an Internal Compiler Error when an unimplemented
/// codepath is executed. The ICE message contains the callsite's file and line. Use the `<<`
/// operator to append an error message to the ICE. An optional callback can be provided so that the
/// user of Tint can capture the ICE before crashing.
#define TINT_UNIMPLEMENTED(...) TINT_ICE(__VA_ARGS__) << "TINT_UNIMPLEMENTED "

/// TINT_ASSERT() is a macro for checking the expression is true, triggering a TINT_ICE if it is
/// not. The ICE message contains the callsite's file and line. Use the `<<` operator to append an
/// error message to the ICE. An optional callback can be provided so that the user of Tint can
/// capture the ICE before crashing.
#define TINT_ASSERT(condition, ...) \
    DAWN_LIKELY((condition)) || TINT_ICE(__VA_ARGS__) << "TINT_ASSERT(" #condition ") "

#endif  // SRC_TINT_UTILS_ICE_ICE_H_
