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

#include <cstring>

#include "src/tint/utils/system/terminal.h"
#include "src/tint/utils/text/styled_text_printer.h"

namespace tint {
namespace {

class Plain : public StyledTextPrinter {
  public:
    explicit Plain(FILE* f) : file_(f) {}

    void Print(const StyledText& text) override {
        auto plain = text.Plain();
        fwrite(plain.data(), 1, plain.size(), file_);
    }

  private:
    FILE* const file_;
};

}  // namespace

std::unique_ptr<StyledTextPrinter> StyledTextPrinter::CreatePlain(FILE* out) {
    return std::make_unique<Plain>(out);
}
std::unique_ptr<StyledTextPrinter> StyledTextPrinter::Create(FILE* out) {
    bool is_dark = TerminalIsDark(out).value_or(true);
    return Create(out, is_dark ? StyledTextTheme::kDefaultDark : StyledTextTheme::kDefaultLight);
}

StyledTextPrinter::~StyledTextPrinter() = default;

}  // namespace tint
