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
#include <string>

#include "src/tint/cmd/fuzz/common/init.h"
#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/utils/text/base64.h"
#include "src/tint/utils/text/string.h"
#include "src/utils/compiler.h"

namespace {

tint::fuzz::wgsl::Options options;

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* input, size_t size) {
    if (size > 0) {
        // SAFETY: The bounds of the `input` pointer are guaranteed to be of length `size` by the
        // libFuzzer framework.
        std::string_view wgsl =
            DAWN_UNSAFE_BUFFERS(std::string_view(reinterpret_cast<const char*>(input), size));
        auto data = tint::DecodeBase64FromComments(wgsl);
        tint::fuzz::wgsl::Run(wgsl, options, data.AsSpan());
    }
    return 0;
}

// Explicitly specify the visibility to prevent the linker from stripping the function on macOS, as
// the LibFuzzer runtime uses dlsym() instead of calling the function directly.
extern "C" __attribute__((visibility("default"))) int LLVMFuzzerInitialize(int* argc,
                                                                           char*** argv) {
    int res = tint::fuzz::common::ParseFuzzerOptions(tint::fuzz::common::FuzzerType::kWGSL, argc,
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
