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

#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"

#include "spirv-tools/libspirv.hpp"
#include "src/tint/lang/spirv/writer/common/binary_writer.h"

namespace tint::spirv::writer {

std::string Disassemble(const std::vector<uint32_t>& data, uint32_t options /* = 0u */) {
    std::string spv_errors;
    spv_target_env target_env = SPV_ENV_VULKAN_1_1;

    auto msg_consumer = [&spv_errors](spv_message_level_t level, const char*,
                                      const spv_position_t& position, const char* message) {
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                spv_errors +=
                    "error: line " + std::to_string(position.index) + ": " + message + "\n";
                break;
            case SPV_MSG_WARNING:
                spv_errors +=
                    "warning: line " + std::to_string(position.index) + ": " + message + "\n";
                break;
            case SPV_MSG_INFO:
                spv_errors +=
                    "info: line " + std::to_string(position.index) + ": " + message + "\n";
                break;
            case SPV_MSG_DEBUG:
                break;
        }
    };

    spvtools::SpirvTools tools(target_env);
    tools.SetMessageConsumer(msg_consumer);

    std::string result;
    if (!tools.Disassemble(data, &result, SPV_BINARY_TO_TEXT_OPTION_NO_HEADER | options)) {
        return "*** Invalid SPIR-V ***\n" + spv_errors;
    }
    return result;
}

std::string DumpModule(Module& module) {
    BinaryWriter writer;
    writer.WriteHeader(module.IdBound());
    writer.WriteModule(module);
    return Disassemble(writer.Result());
}

std::string DumpInstruction(const Instruction& inst) {
    BinaryWriter writer;
    writer.WriteHeader(kDefaultMaxIdBound);
    writer.WriteInstruction(inst);
    return Disassemble(writer.Result());
}

std::string DumpInstructions(const InstructionList& insts) {
    BinaryWriter writer;
    writer.WriteHeader(kDefaultMaxIdBound);
    for (const auto& inst : insts) {
        writer.WriteInstruction(inst);
    }
    return Disassemble(writer.Result());
}

}  // namespace tint::spirv::writer
