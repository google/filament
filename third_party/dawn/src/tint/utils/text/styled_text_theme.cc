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

#include "src/tint/utils/text/styled_text_theme.h"
#include "src/tint/utils/text/text_style.h"

namespace tint {

const StyledTextTheme StyledTextTheme::kDefaultDark{
    /* compare_match */ StyledTextTheme::Attributes{
        /* foreground */ std::nullopt,
        /* background */ Color{20, 100, 20},
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* compare_mismatch */
    StyledTextTheme::Attributes{
        /* foreground */ std::nullopt,
        /* background */ Color{120, 20, 20},
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* severity_success */
    StyledTextTheme::Attributes{
        /* foreground */ Color{0, 200, 0},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* severity_warning */
    StyledTextTheme::Attributes{
        /* foreground */ Color{200, 200, 0},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* severity_failure */
    StyledTextTheme::Attributes{
        /* foreground */ Color{200, 0, 0},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* severity_fatal */
    StyledTextTheme::Attributes{
        /* foreground */ Color{200, 0, 200},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_code */
    StyledTextTheme::Attributes{
        /* foreground */ Color{212, 212, 212},
        /* background */ Color{43, 43, 43},
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_keyword */
    StyledTextTheme::Attributes{
        /* foreground */ Color{197, 134, 192},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_variable */
    StyledTextTheme::Attributes{
        /* foreground */ Color{156, 220, 254},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_type */
    StyledTextTheme::Attributes{
        /* foreground */ Color{78, 201, 176},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_function */
    StyledTextTheme::Attributes{
        /* foreground */ Color{220, 220, 170},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_enum */
    StyledTextTheme::Attributes{
        /* foreground */ Color{79, 193, 255},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_literal */
    StyledTextTheme::Attributes{
        /* foreground */ Color{181, 206, 168},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_attribute */
    StyledTextTheme::Attributes{
        /* foreground */ Color{156, 220, 254},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_comment */
    StyledTextTheme::Attributes{
        /* foreground */ Color{106, 153, 85},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_label */
    StyledTextTheme::Attributes{
        /* foreground */ Color{180, 140, 140},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_instruction */
    StyledTextTheme::Attributes{
        /* foreground */ Color{220, 220, 170},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_squiggle */
    StyledTextTheme::Attributes{
        /* foreground */ Color{0, 200, 255},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
};

const StyledTextTheme StyledTextTheme::kDefaultLight{
    /* compare_match */ StyledTextTheme::Attributes{
        /* foreground */ std::nullopt,
        /* background */ Color{190, 240, 190},
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* compare_mismatch */
    StyledTextTheme::Attributes{
        /* foreground */ std::nullopt,
        /* background */ Color{240, 190, 190},
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* severity_success */
    StyledTextTheme::Attributes{
        /* foreground */ Color{0, 200, 0},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* severity_warning */
    StyledTextTheme::Attributes{
        /* foreground */ Color{200, 200, 0},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* severity_failure */
    StyledTextTheme::Attributes{
        /* foreground */ Color{200, 0, 0},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* severity_fatal */
    StyledTextTheme::Attributes{
        /* foreground */ Color{200, 0, 200},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_code */
    StyledTextTheme::Attributes{
        /* foreground */ Color{10, 10, 10},
        /* background */ Color{248, 248, 248},
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_keyword */
    StyledTextTheme::Attributes{
        /* foreground */ Color{175, 0, 219},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_variable */
    StyledTextTheme::Attributes{
        /* foreground */ Color{0, 16, 128},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_type */
    StyledTextTheme::Attributes{
        /* foreground */ Color{38, 127, 153},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_function */
    StyledTextTheme::Attributes{
        /* foreground */ Color{121, 94, 38},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_enum */
    StyledTextTheme::Attributes{
        /* foreground */ Color{0, 112, 193},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_literal */
    StyledTextTheme::Attributes{
        /* foreground */ Color{9, 134, 88},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_attribute */
    StyledTextTheme::Attributes{
        /* foreground */ Color{156, 220, 254},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_comment */
    StyledTextTheme::Attributes{
        /* foreground */ Color{0, 128, 0},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_label */
    StyledTextTheme::Attributes{
        /* foreground */ Color{180, 140, 140},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_instruction */
    StyledTextTheme::Attributes{
        /* foreground */ Color{121, 94, 38},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
    /* kind_squiggle */
    StyledTextTheme::Attributes{
        /* foreground */ Color{0, 200, 255},
        /* background */ std::nullopt,
        /* bold */ std::nullopt,
        /* underlined */ std::nullopt,
    },
};

StyledTextTheme::Attributes StyledTextTheme::Get(TextStyle text_style) const {
    Attributes out;
    out.bold = false;
    out.underlined = false;

    auto apply = [&](const Attributes& in) {
        if (in.foreground) {
            out.foreground = in.foreground;
        }
        if (in.background) {
            out.background = in.background;
        }
        if (in.bold) {
            out.bold = in.bold;
        }
        if (in.underlined) {
            out.underlined = in.underlined;
        }
    };

    if (text_style.HasSeverity()) {
        if (text_style.IsSuccess()) {
            apply(severity_success);
        } else if (text_style.IsWarning()) {
            apply(severity_warning);
        } else if (text_style.IsError()) {
            apply(severity_failure);
        } else if (text_style.IsFatal()) {
            apply(severity_fatal);
        }
    }

    if (text_style.HasKind()) {
        if (text_style.IsCode()) {
            apply(kind_code);

            if (text_style.IsKeyword()) {
                apply(kind_keyword);
            } else if (text_style.IsVariable()) {
                apply(kind_variable);
            } else if (text_style.IsType()) {
                apply(kind_type);
            } else if (text_style.IsFunction()) {
                apply(kind_function);
            } else if (text_style.IsEnum()) {
                apply(kind_enum);
            } else if (text_style.IsLiteral()) {
                apply(kind_literal);
            } else if (text_style.IsAttribute()) {
                apply(kind_attribute);
            } else if (text_style.IsComment()) {
                apply(kind_comment);
            } else if (text_style.IsLabel()) {
                apply(kind_label);
            } else if (text_style.IsInstruction()) {
                apply(kind_instruction);
            }
        }
        if (text_style.IsSquiggle()) {
            apply(kind_squiggle);
        }
    }

    if (text_style.HasCompare()) {
        if (text_style.IsMatch()) {
            apply(compare_match);
        } else {
            apply(compare_mismatch);
        }
    }

    if (text_style.IsBold()) {
        out.bold = true;
    }
    if (text_style.IsUnderlined()) {
        out.underlined = true;
    }

    return out;
}

}  // namespace tint
