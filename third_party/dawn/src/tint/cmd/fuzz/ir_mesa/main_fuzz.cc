// Copyright 2026 The Dawn & Tint Authors
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

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <span>

#include "src/tint/cmd/fuzz/common/init.h"
#include "src/tint/cmd/fuzz/ir/fuzz.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/utils/compiler.h"

#if TINT_BUILD_IR_BINARY

#include "src/tint/lang/core/ir/binary/decode.h"
#include "src/tint/utils/macros/defer.h"

TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS();
#include "src/libfuzzer/libfuzzer_macro.h"
#include "src/tint/utils/protos/ir_fuzz/ir_fuzz.pb.h"
#include "testing/libfuzzer/proto/lpm_interface.h"
TINT_END_DISABLE_PROTOBUF_WARNINGS();

namespace {

tint::fuzz::ir::Options options;

}  // namespace

DEFINE_BINARY_PROTO_FUZZER(const tint::cmd::fuzz::ir::pb::Root& pb) {
    /// As the fuzzers are free to mutate the module, we need to deserialize a new module for each
    /// sub-fuzzer. Because the protobuf may error when deserializing and the module may be invalid,
    /// we early deserialize. If this fails, then we do not call Run().
    thread_local std::optional<tint::core::ir::Module> module;
    tint::core::ir::binary::DecoderOptions decoder_options{
        .strip_invalid_identifiers = options.strip_invalid_identifiers,
    };

    {
        auto decoded = tint::core::ir::binary::Decode(pb.module(), decoder_options);
        if (decoded != tint::Success) {
            return;  // Failed to decode
        }
        module = std::move(decoded.Move());
    }
    auto acquire_module = [&] {
        if (!module) {
            auto decoded = tint::core::ir::binary::Decode(pb.module(), decoder_options);
            TINT_ASSERT(decoded == tint::Success)
                << "module successfully decoded once, then failed a subsequent time\n"
                << decoded.Failure();

            module = std::move(decoded.Move());
        }

        auto out = std::move(module.value());
        module.reset();
        return out;
    };

    auto data = std::as_bytes(std::span(pb.data()));
    tint::fuzz::ir::Run(acquire_module, options, data);
}

// Explicitly specify the visibility to prevent the linker from stripping the function on macOS, as
// the LibFuzzer runtime uses dlsym() instead of calling the function directly.
extern "C" __attribute__((visibility("default"))) int LLVMFuzzerInitialize(int* argc,
                                                                           char*** argv) {
    int res = tint::fuzz::common::ParseFuzzerOptions(tint::fuzz::common::FuzzerType::kIR, argc,
                                                     argv, &options);
    if (res != 0) {
        return res;
    }

    // Force the fuzzer to only run the SPIR-V writer IR fuzzer.
    options.filter = "tint::spirv::writer::IRFuzzer";

#if TINT_BUILD_FUZZER_VULKAN_SUPPORT
    // Exit early if the Vulkan ICD JSON is not found.
    if (options.vk_icd.empty() || !std::filesystem::exists(options.vk_icd)) {
        std::cerr << "Vulkan ICD JSON not found or does not exist: '" << options.vk_icd
                  << "'. Exiting early.\n";
        exit(0);
    }
#endif

    return 0;
}

#endif
