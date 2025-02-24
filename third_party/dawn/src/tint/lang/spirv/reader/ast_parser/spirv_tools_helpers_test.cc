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

#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"

#include "gtest/gtest.h"
#include "spirv-tools/libspirv.hpp"
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser::test {

std::vector<uint32_t> Assemble(const std::string& spirv_assembly) {
    // TODO(dneto): Use ScopedTrace?

    // (The target environment doesn't affect assembly.
    spvtools::SpirvTools tools(SPV_ENV_UNIVERSAL_1_0);
    StringStream errors;
    std::vector<uint32_t> result;
    tools.SetMessageConsumer([&errors](spv_message_level_t, const char*,
                                       const spv_position_t& position, const char* message) {
        errors << "assembly error:" << position.line << ":" << position.column << ": " << message;
    });

    const auto success =
        tools.Assemble(spirv_assembly, &result, SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
    EXPECT_TRUE(success) << errors.str();

    return result;
}

std::string Disassemble(const std::vector<uint32_t>& spirv_module) {
    spvtools::SpirvTools tools(SPV_ENV_UNIVERSAL_1_0);
    StringStream errors;
    tools.SetMessageConsumer([&errors](spv_message_level_t, const char*,
                                       const spv_position_t& position, const char* message) {
        errors << "disassmbly error:" << position.line << ":" << position.column << ": " << message;
    });

    std::string result;
    const auto success =
        tools.Disassemble(spirv_module, &result, SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
    EXPECT_TRUE(success) << errors.str();

    return result;
}

}  // namespace tint::spirv::reader::ast_parser::test
