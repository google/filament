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

#ifndef SRC_TINT_UTILS_DIAGNOSTIC_FORMATTER_H_
#define SRC_TINT_UTILS_DIAGNOSTIC_FORMATTER_H_

#include <string>

// Forward declaration
namespace tint {
class StyledTextPrinter;
class StyledText;
}  // namespace tint
namespace tint::diag {
class Diagnostic;
class List;
}  // namespace tint::diag

namespace tint::diag {
/// Formatter are used to print a list of diagnostics messages.
class Formatter {
  public:
    /// Style controls the formatter's output style.
    struct Style {
        /// include the file path for each diagnostic
        bool print_file = true;
        /// include the severity for each diagnostic
        bool print_severity = true;
        /// include the source line(s) for the diagnostic
        bool print_line = true;
        /// print a newline at the end of a diagnostic list
        bool print_newline_at_end = true;
        /// width of a tab character
        size_t tab_width = 2u;
    };

    /// Constructor for the formatter using a default style.
    Formatter();

    /// Constructor for the formatter using the custom style.
    /// @param style the style used for the formatter.
    explicit Formatter(const Style& style);

    ~Formatter();

    /// @return the list of diagnostics `list` formatted to a string.
    /// @param list the list of diagnostic messages to format
    StyledText Format(const List& list) const;

  private:
    struct State;

    void Format(const Diagnostic& diag, StyledText& text) const;

    const Style style_;
};

}  // namespace tint::diag

#endif  // SRC_TINT_UTILS_DIAGNOSTIC_FORMATTER_H_
