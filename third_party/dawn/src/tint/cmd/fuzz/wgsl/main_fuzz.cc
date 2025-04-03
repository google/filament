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

#include <iostream>
#include <string>

#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/utils/command/cli.h"
#include "src/tint/utils/command/command.h"
#include "src/tint/utils/text/base64.h"
#include "src/tint/utils/text/string.h"

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/validate/validate.h"
#endif

namespace {

tint::fuzz::wgsl::Options options;

std::string get_default_dxc_path(char*** argv) {
    std::string default_dxc_path = "";
#if TINT_BUILD_HLSL_WRITER
    // Assume the DXC library is in the same directory as this executable
    std::string exe_path = (*argv)[0];
    exe_path = tint::ReplaceAll(exe_path, "\\", "/");
    auto pos = exe_path.rfind('/');
    if (pos != std::string::npos) {
        default_dxc_path = exe_path.substr(0, pos) + '/' + tint::hlsl::validate::kDxcDLLName;
    } else {
        // argv[0] doesn't contain path to exe, try relative to cwd
        default_dxc_path = tint::hlsl::validate::kDxcDLLName;
    }
#endif
    return default_dxc_path;
}

void print_dxc_path_found(const std::string& dxc_path) {
#if TINT_BUILD_HLSL_WRITER
    // Log whether the DXC library was found or not once at initialization.
    auto dxc = tint::Command::LookPath(dxc_path);
    if (dxc.Found()) {
        std::cout << "DXC library found: " << dxc.Path() << "\n";
    } else {
        std::cout << "DXC library not found: " << dxc_path << "\n";
    }
#endif
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* input, size_t size) {
    if (size > 0) {
        std::string_view wgsl(reinterpret_cast<const char*>(input), size);
        auto data = tint::DecodeBase64FromComments(wgsl);
        tint::fuzz::wgsl::Run(wgsl, options, data.Slice());
    }
    return 0;
}

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv) {
    tint::cli::OptionSet opts;

    tint::Vector<std::string_view, 8> arguments;
    for (int i = 1; i < *argc; i++) {
        std::string_view arg((*argv)[i]);
        if (!arg.empty()) {
            arguments.Push(arg);
        }
    }

    auto show_help = [&] {
        std::cerr << "Custom fuzzer options:\n";
        opts.ShowHelp(std::cerr, true);
        std::cerr << "\n";

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
    auto& opt_dump =
        opts.Add<tint::cli::BoolOption>("dump", "dumps shader input/output from fuzzer");

    tint::cli::ParseOptions parse_opts;
    parse_opts.ignore_unknown = true;
    if (auto res = opts.Parse(arguments, parse_opts); res != tint::Success) {
        show_help();
        std::cerr << res.Failure();
        return 0;
    }

    if (opt_help.value.value_or(false)) {
        show_help();
        return 0;
    }

    // Read optional user-supplied args or use default provided
    options.filter = opt_filter.value.value_or("");
    options.run_concurrently = opt_concurrent.value.value_or(false);
    options.verbose = opt_verbose.value.value_or(false);
    options.dxc = opt_dxc.value.value_or(get_default_dxc_path(argv));
    options.dump = opt_dump.value.value_or(false);

    print_dxc_path_found(options.dxc);
#if DAWN_ASAN_ENABLED() && !defined(NDEBUG)
    // TODO(crbug.com/352402877): Avoid DXC timeouts on asan + debug fuzzer builds
    std::cout << "DXC validation disabled in asan + debug builds" << "\n";
    options.dxc = "";
#endif

    return 0;
}
