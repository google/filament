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

#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/system/env.h"
#include "src/tint/utils/system/terminal.h"

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

bool TerminalSupportsColors(FILE* out) {
    return ConsoleHandleFrom(out) != INVALID_HANDLE_VALUE;
}

std::optional<bool> TerminalIsDark(FILE* out) {
    if (!GetEnvVar("WT_SESSION").empty()) {
        // Windows terminal does not support querying the palette
        // See: https://github.com/microsoft/terminal/issues/10639
        return std::nullopt;
    }

    if (HANDLE handle = ConsoleHandleFrom(out); handle != INVALID_HANDLE_VALUE) {
        if (HANDLE screen_buffer = CreateConsoleScreenBuffer(GENERIC_READ, FILE_SHARE_READ,
                                                             /* lpSecurityAttributes */ nullptr,
                                                             CONSOLE_TEXTMODE_BUFFER, nullptr)) {
            TINT_DEFER(CloseHandle(screen_buffer));
            CONSOLE_SCREEN_BUFFER_INFOEX info{};
            info.cbSize = sizeof(info);
            if (GetConsoleScreenBufferInfoEx(screen_buffer, &info)) {
                COLORREF background = info.ColorTable[(info.wAttributes & 0xf0) >> 4];
                // https://en.wikipedia.org/wiki/Relative_luminance
                float r = static_cast<float>((background >> 0) & 0xff) / 255.0f;
                float g = static_cast<float>((background >> 8) & 0xff) / 255.0f;
                float b = static_cast<float>((background >> 16) & 0xff) / 255.0f;
                return (0.2126f * r + 0.7152f * g + 0.0722f * b) < 0.5f;
            }
        }
    }

    // Unknown
    return std::nullopt;
}

}  // namespace tint
