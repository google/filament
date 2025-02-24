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

#ifndef SRC_TINT_UTILS_TEXT_STYLED_TEXT_PRINTER_H_
#define SRC_TINT_UTILS_TEXT_STYLED_TEXT_PRINTER_H_

#include <memory>
#include <string>

#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_theme.h"

/// Forward declarations
namespace tint {
class TextStyle;
}

namespace tint {

/// StyledTextPrinter is the interface for printing text with a style.
class StyledTextPrinter {
  public:
    /// @returns a Printer using the default theme.
    /// @param out the file to print to.
    static std::unique_ptr<StyledTextPrinter> Create(FILE* out);

    /// @returns a Printer using the theme @p theme.
    /// @param out the file to print to.
    /// @param theme the custom theme to use.
    static std::unique_ptr<StyledTextPrinter> Create(FILE* out, const StyledTextTheme& theme);

    /// @returns a Printer that emits non-styled text.
    /// @param out the file to print to.
    static std::unique_ptr<StyledTextPrinter> CreatePlain(FILE* out);

    /// Enumerator of ANSI terminal color support.
    enum class ANSIColors {
        k8Bit,   // Palette of 256 colors 'xterm-256color'
        k24Bit,  // 8-bit per [R,G,B]
    };

    /// @returns a Printer that uses ANSI escape sequences and theme @p theme.
    /// @param out the file to print to.
    /// @param theme the custom theme to use.
    static std::unique_ptr<StyledTextPrinter> CreateANSI(FILE* out,
                                                         const StyledTextTheme& theme,
                                                         ANSIColors colors);

    /// Destructor
    virtual ~StyledTextPrinter();

    /// Prints the styled text to the printer.
    /// @param text the text to print.
    virtual void Print(const StyledText& text) = 0;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_TEXT_STYLED_TEXT_PRINTER_H_
