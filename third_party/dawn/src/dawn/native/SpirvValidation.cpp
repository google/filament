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

#include "dawn/native/SpirvValidation.h"

#include <spirv-tools/libspirv.hpp>

#include <memory>
#include <sstream>
#include <string>

namespace dawn::native {

MaybeError ValidateSpirv(LogEmitter* logEmitter,
                         const uint32_t* spirv,
                         size_t wordCount,
                         bool spv14) {
    spvtools::SpirvTools spirvTools(spv14 ? SPV_ENV_VULKAN_1_1_SPIRV_1_4 : SPV_ENV_VULKAN_1_1);
    spirvTools.SetMessageConsumer([logEmitter](spv_message_level_t level, const char*,
                                               const spv_position_t& position,
                                               const char* message) {
        wgpu::LoggingType wgpuLogLevel;
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                wgpuLogLevel = wgpu::LoggingType::Error;
                break;
            case SPV_MSG_WARNING:
                wgpuLogLevel = wgpu::LoggingType::Warning;
                break;
            case SPV_MSG_INFO:
                wgpuLogLevel = wgpu::LoggingType::Info;
                break;
            default:
                wgpuLogLevel = wgpu::LoggingType::Error;
                break;
        }

        std::ostringstream ss;
        ss << "SPIRV line " << position.index << ": " << message << "\n";
        logEmitter->EmitLog(wgpuLogLevel, ss.str().c_str());
    });

    // Don't prepare to emit friendly names. The preparation costs
    // time by scanning the whole module and building a string table.
    spvtools::ValidatorOptions val_opts;
    val_opts.SetFriendlyNames(false);

    const bool valid = spirvTools.Validate(spirv, wordCount, val_opts);
    // Dump the generated SPIRV if it is invalid.
    if (!valid) {
        DumpSpirv(logEmitter, spirv, wordCount, &spirvTools);
    }

    DAWN_INVALID_IF(!valid, "Produced invalid SPIRV. Please file a bug at https://crbug.com/tint.");

    return {};
}

void DumpSpirv(LogEmitter* logEmitter,
               const uint32_t* spirv,
               size_t wordCount,
               spvtools::SpirvTools* spirvTools) {
    std::unique_ptr<spvtools::SpirvTools> inplaceSpirvTools;
    if (spirvTools == nullptr) {
        // Use the newest environment Dawn supports because disassembly supports older versions.
        inplaceSpirvTools = std::make_unique<spvtools::SpirvTools>(SPV_ENV_VULKAN_1_1_SPIRV_1_4);
        spirvTools = inplaceSpirvTools.get();
    }

    std::ostringstream dumpedMsg;
    std::string disassembly;
    if (spirvTools->Disassemble(
            spirv, wordCount, &disassembly,
            SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES | SPV_BINARY_TO_TEXT_OPTION_INDENT)) {
        dumpedMsg << "/* Dumped SPIRV disassembly */\n" << disassembly;
    } else {
        dumpedMsg << "/* Failed to disassemble SPIRV */";
    }
    logEmitter->EmitLog(wgpu::LoggingType::Info, dumpedMsg.str().c_str());
}

}  // namespace dawn::native
