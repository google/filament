// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_SPIRV_READER_PARSER_HELPER_TEST_H_
#define SRC_TINT_LANG_SPIRV_READER_PARSER_HELPER_TEST_H_

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/reader/common/helper_test.h"
#include "src/tint/lang/spirv/reader/parser/parser.h"

namespace tint::spirv::reader {

// Helper macro to run the parser and compare the disassembled IR to a string.
// Automatically prefixes the IR disassembly with a newline to improve formatting of tests.
#define EXPECT_IR_SPV(asm, ir, spv_version)                                                 \
    do {                                                                                    \
        auto result = Run(asm, spv_version);                                                \
        ASSERT_EQ(result, Success) << result.Failure();                                     \
        auto got = "\n" + result.Get();                                                     \
        ASSERT_THAT(got, testing::HasSubstr(ir)) << "GOT:\n" << got << "EXPECTED:\n" << ir; \
    } while (false)

// Helper macro to run the parser and compare the disassembled IR to a string.
// Automatically prefixes the IR disassembly with a newline to improve formatting of tests.
#define EXPECT_IR(asm, ir) EXPECT_IR_SPV(asm, ir, SPV_ENV_UNIVERSAL_1_0)

/// Base helper class for testing the SPIR-V parser implementation.
template <typename BASE>
class SpirvParserTestHelperBase : public BASE {
  protected:
    /// Run the parser on a SPIR-V module and return the Tint IR or an error string.
    /// @param spirv_asm the SPIR-V assembly to parse
    /// @returns the disassembled Tint IR or an error
    Result<std::string> Run(std::string spirv_asm,
                            spv_target_env spv_version = SPV_ENV_UNIVERSAL_1_0) {
        // Assemble the SPIR-V input.
        auto binary = Assemble(spirv_asm, spv_version);
        if (binary != Success) {
            return binary.Failure();
        }

        // Parse the SPIR-V to produce an IR module.
        auto parsed = Parse(Slice(binary.Get().data(), binary.Get().size()), options);
        if (parsed != Success) {
            return parsed.Failure();
        }

        // Validate the IR module against the capabilities supported by the SPIR-V dialect.
        auto validated =
            ValidateAndDumpIfNeeded(parsed.Get(), "spirv.test",
                                    core::ir::Capabilities{
                                        core::ir::Capability::kAllowMultipleEntryPoints,
                                        core::ir::Capability::kAllowOverrides,
                                        core::ir::Capability::kAllowPhonyInstructions,
                                        core::ir::Capability::kAllowVectorElementPointer,
                                        core::ir::Capability::kAllowNonCoreTypes,
                                        core::ir::Capability::kAllowStructMatrixDecorations,
                                    });
        if (validated != Success) {
            return validated.Failure();
        }

        // Return the disassembled IR module.
        return core::ir::Disassembler(parsed.Get()).Plain();
    }

    /// Parser options
    spirv::reader::Options options;
};

using SpirvParserTest = SpirvParserTestHelperBase<testing::Test>;
using SpirvParserDeathTest = SpirvParserTest;

template <typename T>
using SpirvParserTestWithParam = SpirvParserTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::spirv::reader

#endif  // SRC_TINT_LANG_SPIRV_READER_PARSER_HELPER_TEST_H_
