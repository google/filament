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

#ifndef SRC_TINT_UTILS_TEXT_STYLED_TEXT_THEME_H_
#define SRC_TINT_UTILS_TEXT_STYLED_TEXT_THEME_H_

#include <stdint.h>
#include <optional>

#include "src/tint/utils/math/hash.h"

/// Forward declarations
namespace tint {
class TextStyle;
}

namespace tint {

/// StyledTextTheme describes the display theming for TextStyles
struct StyledTextTheme {
    /// The default dark theme
    static const StyledTextTheme kDefaultDark;
    /// The default light theme
    static const StyledTextTheme kDefaultLight;

    /// Color holds a 24-bit RGB color
    struct Color {
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;

        /// Equality operator
        bool operator==(const Color& other) const {
            return r == other.r && g == other.g && b == other.b;
        }
        /// @returns a hash code of this Color
        tint::HashCode HashCode() const { return Hash(r, g, b); }
    };

    /// Attributes holds a number of optional attributes for a style of text.
    /// Attributes that are std::nullopt do not change the default style.
    struct Attributes {
        std::optional<Color> foreground;
        std::optional<Color> background;
        std::optional<bool> bold;
        std::optional<bool> underlined;
    };

    /// @returns Attributes from the text style @p text_style
    Attributes Get(TextStyle text_style) const;

    /// The theme's attributes for a compare-match
    Attributes compare_match;
    /// The theme's attributes for a compare-mismatch
    Attributes compare_mismatch;

    /// The theme's attributes for a success severity
    Attributes severity_success;
    /// The theme's attributes for a warning severity
    Attributes severity_warning;
    /// The theme's attributes for a failure severity
    Attributes severity_failure;
    /// The theme's attributes for a fatal severity
    Attributes severity_fatal;

    /// The theme's attributes for a code text style
    Attributes kind_code;
    /// The theme's attributes for a keyword token. This is applied on top #kind_code.
    Attributes kind_keyword;
    /// The theme's attributes for a variable token. This is applied on top #kind_code.
    Attributes kind_variable;
    /// The theme's attributes for a type token. This is applied on top #kind_code.
    Attributes kind_type;
    /// The theme's attributes for a function token. This is applied on top #kind_code.
    Attributes kind_function;
    /// The theme's attributes for a enum token. This is applied on top #kind_code.
    Attributes kind_enum;
    /// The theme's attributes for a literal token. This is applied on top #kind_code.
    Attributes kind_literal;
    /// The theme's attributes for a attribute token. This is applied on top #kind_code.
    Attributes kind_attribute;
    /// The theme's attributes for a comment token. This is applied on top #kind_code.
    Attributes kind_comment;
    /// The theme's attributes for a label token. This is applied on top #kind_code.
    Attributes kind_label;
    /// The theme's attributes for a instruction token. This is applied on top #kind_code.
    Attributes kind_instruction;

    /// The theme's attributes for a squiggle-highlight.
    Attributes kind_squiggle;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_TEXT_STYLED_TEXT_THEME_H_
