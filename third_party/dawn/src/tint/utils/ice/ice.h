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

#include <sstream>
#include <string>
#include <utility>

#include "src/tint/utils/macros/compiler.h"

namespace tint {

/// InternalCompilerError is a helper for reporting internal compiler errors.
/// Construct the InternalCompilerError with the source location of the ICE fault and append any
/// error details with the `<<` operator. When the InternalCompilerError is destructed, the
/// concatenated error message is passed to the InternalCompilerErrorReporter.
class InternalCompilerError {
  public:
    /// Constructor
    /// @param file the file containing the ICE
    /// @param line the line containing the ICE
    InternalCompilerError(const char* file, size_t line);

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

  private:
    InternalCompilerError(const InternalCompilerError&) = delete;
    InternalCompilerError(InternalCompilerError&&) = delete;

    char const* const file_;
    const size_t line_;
    std::stringstream msg_;
};

/// Function type used for registering an internal compiler error reporter
using InternalCompilerErrorReporter = void(const InternalCompilerError&);

/// Sets the global error reporter to be called in case of internal compiler
/// errors.
/// @param reporter the error reporter
void SetInternalCompilerErrorReporter(InternalCompilerErrorReporter* reporter);

}  // namespace tint

/// TINT_ICE() is a macro to invoke the InternalCompilerErrorReporter for an Internal Compiler
/// Error. The ICE message contains the callsite's file and line. Use the `<<` operator to append an
/// error message to the ICE.
#define TINT_ICE() tint::InternalCompilerError(__FILE__, __LINE__)

/// TINT_UNREACHABLE() is a macro invoke the InternalCompilerErrorReporter when an expectedly
/// unreachable statement is reached. The ICE message contains the callsite's file and line. Use the
/// `<<` operator to append an error message to the ICE.
#define TINT_UNREACHABLE() TINT_ICE() << "TINT_UNREACHABLE "

/// TINT_UNIMPLEMENTED() is a macro to invoke the InternalCompilerErrorReporter when unimplemented
/// code is executed. The ICE message contains the callsite's file and line. Use the `<<` operator
/// to append an error message to the ICE.
#define TINT_UNIMPLEMENTED() TINT_ICE() << "TINT_UNIMPLEMENTED "

/// TINT_ASSERT() is a macro for checking the expression is true, triggering a
/// TINT_ICE if it is not.
/// The ICE message contains the callsite's file and line.
#define TINT_ASSERT(condition)                           \
    do {                                                 \
        if (DAWN_UNLIKELY(!(condition))) {               \
            TINT_ICE() << "TINT_ASSERT(" #condition ")"; \
        }                                                \
    } while (false)

#endif  // SRC_TINT_UTILS_ICE_ICE_H_
