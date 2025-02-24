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

#include "src/tint/utils/text/styled_text_printer.h"
#include "src/tint/utils/text/text_style.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

#define ENABLE_PRINTER_TESTS 0  // Print styled text as part of the unit tests
#if ENABLE_PRINTER_TESTS

using StyledTextPrinterTest = testing::Test;

TEST_F(StyledTextPrinterTest, Themed) {
    auto printer = StyledTextPrinter::Create(stdout);
    printer->Print(StyledText{} << style::Plain << "Plain\n"
                                << style::Bold << "Bold\n"
                                << style::Underlined << "Underlined\n"
                                << style::Success << "Success\n"
                                << style::Warning << "Warning\n"
                                << style::Error << "Error\n"
                                << style::Fatal << "Fatal\n"
                                << style::Code << "Code\n"
                                << style::Keyword << "Keyword\n"
                                << style::Variable << "Variable\n"
                                << style::Type << "Type\n"
                                << style::Function << "Function\n"
                                << style::Enum << "Enum\n"
                                << style::Literal << "Literal\n"
                                << style::Attribute << "Attribute\n"
                                << style::Squiggle << "Squiggle\n");
}

#endif  // ENABLE_PRINTER_TESTS

}  // namespace
}  // namespace tint
