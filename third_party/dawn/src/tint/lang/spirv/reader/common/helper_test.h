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

#ifndef SRC_TINT_LANG_SPIRV_READER_COMMON_HELPER_TEST_H_
#define SRC_TINT_LANG_SPIRV_READER_COMMON_HELPER_TEST_H_

#include <string>
#include <vector>

#include "spirv-tools/libspirv.hpp"
#include "src/tint/utils/result.h"

namespace tint::spirv::reader {

/// Assemble a textual SPIR-V module into a SPIR-V binary.
/// @param spirv_asm the textual SPIR-V assembly
/// @returns the SPIR-V binary data, or an error string
inline Result<std::vector<uint32_t>, std::string> Assemble(
    std::string spirv_asm,
    spv_target_env spv_version = SPV_ENV_UNIVERSAL_1_0) {
    StringStream err;
    std::vector<uint32_t> binary;
    spvtools::SpirvTools tools(spv_version);
    tools.SetMessageConsumer(
        [&err](spv_message_level_t, const char*, const spv_position_t& pos, const char* msg) {
            err << "SPIR-V assembly failed:" << pos.line << ":" << pos.column << ": " << msg;
        });
    if (!tools.Assemble(spirv_asm, &binary, SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS)) {
        return err.str();
    }
    return binary;
}

}  // namespace tint::spirv::reader

#endif  // SRC_TINT_LANG_SPIRV_READER_COMMON_HELPER_TEST_H_
