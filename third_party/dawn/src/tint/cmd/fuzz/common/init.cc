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

#include "src/tint/cmd/fuzz/common/init.h"

#include <iostream>
#include <string>

#include "src/tint/cmd/fuzz/common/helper.h"
#include "src/tint/utils/command/cli.h"
#include "src/tint/utils/containers/vector.h"

namespace tint::fuzz::common {

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
int ParseFuzzerOptions(FuzzerType type, int* argc, char*** argv, Options* options) {
    tint::cli::OptionSet opts;

    tint::Vector<std::string_view, 8> arguments;
    for (int i = 1; i < *argc; i++) {
        std::string_view arg((*argv)[i]);
        if (!arg.empty()) {
            arguments.Push(arg);
        }
    }

    auto show_help = [&] {
        std::cerr << "Custom fuzzer options:" << '\n';
        opts.ShowHelp(std::cerr, true);
        std::cerr << '\n';
        // Change args to show libfuzzer help
        std::cerr << "Standard libfuzzer ";  // libfuzzer will print 'Usage:'
        static char help[] = "-help=1";
        *argc = 2;
        (*argv)[1] = help;
    };

    auto& opt_help = opts.Add<tint::cli::BoolOption>("help", "shows the usage");
    auto& opt_filter = opts.Add<tint::cli::StringOption>(
        "filter", "runs only the fuzzers with the given substring");
    auto& opt_concurrent =
        opts.Add<tint::cli::BoolOption>("concurrent", "runs the fuzzers concurrently");
    auto& opt_verbose =
        opts.Add<tint::cli::BoolOption>("verbose", "prints the name of each fuzzer before running");
    auto& opt_dxc = opts.Add<tint::cli::StringOption>("dxc", "path to DXC DLL");
#if TINT_BUILD_FUZZER_VULKAN_SUPPORT
    auto& opt_vk_icd = opts.Add<tint::cli::StringOption>("vk_icd", "path to Vulkan ICD JSON");
#endif
    auto& opt_dump =
        opts.Add<tint::cli::BoolOption>("dump", "dumps shader input/output from fuzzer");
    auto& opt_dump_ir =
        opts.Add<tint::cli::BoolOption>("dump-ir", "Dump IR at each stage of the compilation flow");
    auto& opt_disable_ir_validator =
        opts.Add<tint::cli::BoolOption>("disable-ir-validator", "Disable IR validation");

    tint::cli::BoolOption* opt_strip_invalid_identifiers = nullptr;
    if (type == FuzzerType::kIR) {
        opt_strip_invalid_identifiers = &opts.Add<tint::cli::BoolOption>(
            "strip-invalid-identifiers", "Strip invalid identifiers instead of erroring");
    }

    tint::cli::ParseOptions parse_opts;
    parse_opts.ignore_unknown = true;
    if (auto res = opts.Parse(arguments, parse_opts); res != tint::Success) {
        show_help();
        std::cerr << res.Failure();
        return 1;
    }

    if (opt_help.value.value_or(false)) {
        show_help();
        return 0;
    }

    options->filter = opt_filter.value.value_or("");
    options->run_concurrently = opt_concurrent.value.value_or(false);
    options->verbose = opt_verbose.value.value_or(false);
    options->dxc = opt_dxc.value.value_or(GetDefaultDxcPath(argv));
#if TINT_BUILD_FUZZER_VULKAN_SUPPORT
    options->vk_icd = opt_vk_icd.value.value_or(GetDefaultVkICDPath(argv));
#endif
    options->dump = opt_dump.value.value_or(false);
    options->dump_ir_when_validating = opt_dump_ir.value.value_or(false);
    options->disable_ir_validator = opt_disable_ir_validator.value.value_or(false);
    options->strip_invalid_identifiers = opt_strip_invalid_identifiers
                                             ? opt_strip_invalid_identifiers->value.value_or(false)
                                             : false;

    PrintDxcPathFound(options->dxc);
#if TINT_BUILD_FUZZER_VULKAN_SUPPORT
    PrintVkICDPathFound(options->vk_icd);
#endif

#if DAWN_ASAN_ENABLED() && !defined(NDEBUG)
    // TODO(crbug.com/352402877): Avoid DXC timeouts on asan + debug fuzzer builds
    if (options->dxc != "") {
        std::cout << "DXC validation disabled in asan + debug builds" << "\n";
        options->dxc = "";
    }
#endif

    return 0;
}
TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

}  // namespace tint::fuzz::common
