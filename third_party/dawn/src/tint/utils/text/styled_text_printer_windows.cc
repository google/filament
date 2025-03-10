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

// GEN_BUILD:CONDITION(tint_build_is_win)

#include <cstring>

#include "src/tint/utils/text/styled_text_printer.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

namespace tint {
namespace {

HANDLE ConsoleHandleFrom(FILE* file) {
    HANDLE handle = INVALID_HANDLE_VALUE;
    if (file == stdout) {
        handle = GetStdHandle(STD_OUTPUT_HANDLE);
    } else if (file == stderr) {
        handle = GetStdHandle(STD_ERROR_HANDLE);
    } else {
        return INVALID_HANDLE_VALUE;
    }

    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (GetConsoleScreenBufferInfo(handle, &info) == 0) {
        return INVALID_HANDLE_VALUE;
    }
    return handle;
}

}  // namespace

std::unique_ptr<StyledTextPrinter> StyledTextPrinter::Create(FILE* out,
                                                             const StyledTextTheme& theme) {
    if (HANDLE handle = ConsoleHandleFrom(out); handle != INVALID_HANDLE_VALUE) {
        SetConsoleOutputCP(CP_UTF8);
        if (SetConsoleMode(handle, ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
            return CreateANSI(out, theme, ANSIColors::k24Bit);
        }
    }
    return CreatePlain(out);
}

}  // namespace tint
