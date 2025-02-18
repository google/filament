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

#if TINT_BUILD_IS_WIN
#include <fcntl.h>  // _O_BINARY
#include <io.h>     // _setmode
#endif              // TINT_BUILD_IS_WIN

#include <fstream>
#include <iostream>

#include "src/tint/lang/wgsl/ls/serve.h"

namespace {

class StdinStream : public langsvr::Reader {
  public:
    /// @copydoc langsvr::Reader
    size_t Read(std::byte* out, size_t count) override { return fread(out, 1, count, stdin); }
};

class StdoutStream : public langsvr::Writer {
  public:
    /// @copydoc langsvr::Reader
    langsvr::Result<langsvr::SuccessType> Write(const std::byte* in, size_t count) override {
        fwrite(in, 1, count, stdout);
        fflush(stdout);
        return langsvr::Success;
    }
};

}  // namespace

int main() {
#if TINT_BUILD_IS_WIN
    // Change stdin & stdout from text mode to binary mode.
    // This ensures sequences of \r\n are not changed to \n.
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif  // TINT_BUILD_IS_WIN

    StdoutStream stdout_stream;
    StdinStream stdin_stream;

    if (auto res = tint::wgsl::ls::Serve(stdin_stream, stdout_stream); res != tint::Success) {
        std::cerr << res.Failure();
        return 1;
    }

    return 0;
}
