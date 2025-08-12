// Copyright 2025 The Dawn & Tint Authors
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

#include "src/utils/chromium_test_compat/chromium_test_compat.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace dawn {
void SubstituteChromiumArgs(int argc, char** argv) {
    std::string testSummaryOutputArg("--test-launcher-summary-output=");

    for (int i = 0; i < argc; i++) {
        std::string_view argument(argv[i]);

        // Look to replace "--test-launcher-summary-output=" with "--gtest_output=json:". The former
        // is a Chromium-specific flag for where to output results in a Chromium-specific format,
        // while the latter is gtest flag to output results in a gtest-specific format.
        if (argument.length() > testSummaryOutputArg.length()) {
            auto prefix = argument.substr(0, testSummaryOutputArg.length());
            if (prefix != testSummaryOutputArg) {
                continue;
            }
            auto argValue = argument.substr(testSummaryOutputArg.length());
            std::string replacementArg("--gtest_output=json:");
            replacementArg += argValue;
            std::cout << "Replacing " << argument << " with " << replacementArg << "\n";
            size_t bufferSize = replacementArg.length() + 1;
            argv[i] = new char[bufferSize];
            int charsWritten = std::snprintf(argv[i], bufferSize, "%s", replacementArg.c_str());

            if (size_t(charsWritten) != replacementArg.length()) {
                abort();
            }
        }
    }
}
}  // namespace dawn
