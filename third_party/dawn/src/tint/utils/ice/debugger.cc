// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/utils/ice/debugger.h"

#ifdef TINT_ENABLE_BREAK_IN_DEBUGGER

#ifdef _MSC_VER
#include <Windows.h>
#elif defined(__linux__)
#include <signal.h>
#include <fstream>
#include <string>
#endif

#ifdef _MSC_VER
#define TINT_DEBUGGER_BREAK_DEFINED
void tint::debugger::Break() {
    if (::IsDebuggerPresent()) {
        ::DebugBreak();
    }
}

#elif defined(__linux__)

#define TINT_DEBUGGER_BREAK_DEFINED
void tint::debugger::Break() {
    // A process is being traced (debugged) if "/proc/self/status" contains a
    // line with "TracerPid: <non-zero-digit>...".
    bool is_traced = false;
    std::ifstream fin("/proc/self/status");
    std::string line;
    while (!is_traced && std::getline(fin, line)) {
        const char kPrefix[] = "TracerPid:\t";
        static constexpr int kPrefixLen = sizeof(kPrefix) - 1;
        if (line.length() > kPrefixLen && line.compare(0, kPrefixLen, kPrefix) == 0) {
            is_traced = line[kPrefixLen] != '0';
        }
    }

    if (is_traced) {
        raise(SIGTRAP);
    }
}
#endif  // platform

#endif  // TINT_ENABLE_BREAK_IN_DEBUGGER

#ifndef TINT_DEBUGGER_BREAK_DEFINED
void tint::debugger::Break() {}
#endif
