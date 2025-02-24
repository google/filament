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

#include "src/tint/utils/text/color_mode.h"

#include "src/tint/utils/system/env.h"
#include "src/tint/utils/system/terminal.h"
#include "src/tint/utils/text/styled_text_printer.h"
#include "src/tint/utils/text/styled_text_theme.h"

namespace tint {

ColorMode ColorModeDefault() {
    if (!tint::TerminalSupportsColors(stdout)) {
        return ColorMode::kPlain;
    }
    if (auto res = tint::TerminalIsDark(stdout)) {
        return *res ? ColorMode::kDark : ColorMode::kLight;
    }
    if (auto env = tint::GetEnvVar("DARK_BACKGROUND_COLOR"); !env.empty()) {
        return env != "0" ? ColorMode::kDark : ColorMode::kLight;
    }
    if (auto env = tint::GetEnvVar("LIGHT_BACKGROUND_COLOR"); !env.empty()) {
        return env != "0" ? ColorMode::kLight : ColorMode::kDark;
    }
    return ColorMode::kDark;
}

std::unique_ptr<tint::StyledTextPrinter> CreatePrinter(ColorMode mode) {
    switch (mode) {
        case ColorMode::kLight:
            return tint::StyledTextPrinter::Create(stderr, tint::StyledTextTheme::kDefaultLight);
        case ColorMode::kDark:
            return tint::StyledTextPrinter::Create(stderr, tint::StyledTextTheme::kDefaultDark);
        case ColorMode::kPlain:
            break;
    }
    return tint::StyledTextPrinter::CreatePlain(stderr);
}

}  // namespace tint
