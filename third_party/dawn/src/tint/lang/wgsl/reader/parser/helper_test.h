// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_READER_PARSER_HELPER_TEST_H_
#define SRC_TINT_LANG_WGSL_READER_PARSER_HELPER_TEST_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/reader/parser/parser.h"

namespace tint::wgsl::reader {

/// WGSL Parser test class
class WGSLParserTest : public testing::Test {
  public:
    /// Constructor
    WGSLParserTest();
    ~WGSLParserTest() override;

    /// Retrieves the parser from the helper
    /// @param str the string to parse
    /// @returns the parser implementation
    std::unique_ptr<Parser> parser(const std::string& str) {
        auto file = std::make_unique<Source::File>("test.wgsl", str);
        auto impl = std::make_unique<Parser>(file.get());
        impl->InitializeLex();
        files_.emplace_back(std::move(file));
        return impl;
    }

  private:
    std::vector<std::unique_ptr<Source::File>> files_;
};

/// WGSL Parser test class with param
template <typename T>
class WGSLParserTestWithParam : public testing::TestWithParam<T>, public ProgramBuilder {
  public:
    /// Constructor
    WGSLParserTestWithParam() = default;
    ~WGSLParserTestWithParam() override = default;

    /// Retrieves the parser from the helper
    /// @param str the string to parse
    /// @returns the parser implementation
    std::unique_ptr<Parser> parser(const std::string& str) {
        auto file = std::make_unique<Source::File>("test.wgsl", str);
        auto impl = std::make_unique<Parser>(file.get());
        impl->InitializeLex();
        files_.emplace_back(std::move(file));
        return impl;
    }

  private:
    std::vector<std::unique_ptr<Source::File>> files_;
};

}  // namespace tint::wgsl::reader

#endif  // SRC_TINT_LANG_WGSL_READER_PARSER_HELPER_TEST_H_
