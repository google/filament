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

#include "src/tint/lang/msl/validate/validate.h"

#include "src/tint/utils/command/command.h"
#include "src/tint/utils/file/tmpfile.h"

namespace tint::msl::validate {

Result Validate(const std::string& xcrun_path, const std::string& source, MslVersion version) {
    Result result;

    auto xcrun = tint::Command(xcrun_path);
    if (!xcrun.Found()) {
        result.output = "xcrun not found at '" + std::string(xcrun_path) + "'";
        result.failed = true;
        return result;
    }

    tint::TmpFile file(".metal");
    file << source;

    const char* version_str = nullptr;
    switch (version) {
        case MslVersion::kMsl_2_3:
            version_str = "-std=macos-metal2.3";
            break;
        case MslVersion::kMsl_3_2:
            version_str = "-std=macos-metal3.2";
            break;
    }

#ifdef _WIN32
    // On Windows, we should actually be running metal.exe from the Metal
    // Developer Tools for Windows
    auto res = xcrun("-x", "metal",  //
                     "-o", "NUL",    //
                     version_str,    //
                     "-c", file.Path());
#else
    auto res = xcrun("-sdk", "macosx", "metal",  //
                     "-o", "/dev/null",          //
                     version_str,                //
                     "-c", file.Path());
#endif
    if (!res.out.empty()) {
        if (!result.output.empty()) {
            result.output += "\n";
        }
        result.output += res.out;
    }
    if (!res.err.empty()) {
        if (!result.output.empty()) {
            result.output += "\n";
        }
        result.output += res.err;
    }
    result.failed = (res.error_code != 0);

    return result;
}

}  // namespace tint::msl::validate
