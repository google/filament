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

#include "src/tint/cmd/fuzz/wgsl/fuzz.h"

#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/allowed_features.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/function.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/struct.h"
#include "src/tint/lang/wgsl/ast/variable.h"
#include "src/tint/lang/wgsl/enums.h"
#include "src/tint/lang/wgsl/reader/options.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/static_init.h"
#include "src/tint/utils/rtti/switch.h"

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#endif

namespace tint::fuzz::wgsl {
namespace {

/// @returns a reference to the static list of registered ProgramFuzzers.
/// @note this is not a global, as the static initializers that register fuzzers may be called
/// before this vector is constructed.
Vector<ProgramFuzzer, 32>& Fuzzers() {
    static Vector<ProgramFuzzer, 32> fuzzers;
    return fuzzers;
}

thread_local std::string_view currently_running;

bool IsAddressSpace(std::string_view name) {
    return tint::core::ParseAddressSpace(name) != tint::core::AddressSpace::kUndefined;
}

bool IsBuiltinFn(std::string_view name) {
    return tint::wgsl::ParseBuiltinFn(name) != tint::wgsl::BuiltinFn::kNone;
}

bool IsBuiltinType(std::string_view name) {
    return tint::core::ParseBuiltinType(name) != tint::core::BuiltinType::kUndefined;
}

/// Scans @p program for patterns, returning a set of ProgramProperties.
EnumSet<ProgramProperties> ScanProgramProperties(const Program& program) {
    EnumSet<ProgramProperties> out;
    auto check = [&](std::string_view name) {
        if (IsAddressSpace(name)) {
            out.Add(ProgramProperties::kAddressSpacesShadowed);
        }
        if (IsBuiltinFn(name)) {
            out.Add(ProgramProperties::kBuiltinFnsShadowed);
        }
        if (IsBuiltinType(name)) {
            out.Add(ProgramProperties::kBuiltinTypesShadowed);
        }
    };

    for (auto* node : program.ASTNodes().Objects()) {
        tint::Switch(
            node,  //
            [&](const ast::Variable* variable) { check(variable->name->symbol.NameView()); },
            [&](const ast::Function* fn) { check(fn->name->symbol.NameView()); },
            [&](const ast::Struct* str) { check(str->name->symbol.NameView()); },
            [&](const ast::Alias* alias) { check(alias->name->symbol.NameView()); });

        if (out.Contains(ProgramProperties::kBuiltinFnsShadowed) &&
            out.Contains(ProgramProperties::kBuiltinTypesShadowed)) {
            break;  // Early exit - nothing more to find.
        }
    }

    // Check for multiple entry points
    bool entry_point_found = false;
    for (auto* fn : program.AST().Functions()) {
        if (fn->IsEntryPoint()) {
            if (entry_point_found) {
                out.Add(ProgramProperties::kMultipleEntryPoints);
                break;
            }
            entry_point_found = true;
        }
    }

    return out;
}

}  // namespace

void Register(const ProgramFuzzer& fuzzer) {
    Fuzzers().Push(fuzzer);
}

void Run(std::string_view wgsl, const Options& options, Slice<const std::byte> data) {
#if TINT_BUILD_WGSL_WRITER
    // Register the Program printer. This is used for debugging purposes.
    tint::Program::printer = [](const tint::Program& program) {
        auto result = tint::wgsl::writer::Generate(program);
        if (result != Success) {
            return result.Failure().reason;
        }
        return result->wgsl;
    };
#endif

    if (options.dump) {
        std::cout << "Dumping input WGSL:\n" << wgsl << "\n";
    }

    // Ensure that fuzzers are sorted. Without this, the fuzzers may be registered in any order,
    // leading to non-determinism, which we must avoid.
    TINT_STATIC_INIT(Fuzzers().Sort([](auto& a, auto& b) { return a.name < b.name; }));

    // Create a Source::File to hand to the parser.
    tint::Source::File file("test.wgsl", wgsl);

    // Parse the WGSL program.
    tint::wgsl::reader::Options parse_options;
    parse_options.allowed_features = tint::wgsl::AllowedFeatures::Everything();
    auto program = tint::wgsl::reader::Parse(&file, parse_options);
    if (!program.IsValid()) {
        if (options.verbose) {
            std::cerr << "invalid WGSL program:\n" << program.Diagnostics() << "\n";
        }
        return;
    }

    Context context;
    context.options = options;
    context.program_properties = ScanProgramProperties(program);

    bool ran_atleast_once = false;

    // Run each of the program fuzzer functions
    if (options.run_concurrently) {
        size_t n = Fuzzers().Length();
        tint::Vector<std::thread, 32> threads;
        threads.Reserve(n);
        for (size_t i = 0; i < n; i++) {
            if (!options.filter.empty() &&
                Fuzzers()[i].name.find(options.filter) == std::string::npos) {
                continue;
            }
            ran_atleast_once = true;

            threads.Push(std::thread([i, &program, &data, &context] {
                auto& fuzzer = Fuzzers()[i];
                currently_running = fuzzer.name;
                if (context.options.verbose) {
                    std::cout << " • [" << i << "] Running: " << currently_running << "\n";
                }
                fuzzer.fn(program, context, data);
            }));
        }
        for (auto& thread : threads) {
            thread.join();
        }
    } else {
        TINT_DEFER(currently_running = "");
        for (auto& fuzzer : Fuzzers()) {
            if (!options.filter.empty() && fuzzer.name.find(options.filter) == std::string::npos) {
                continue;
            }
            ran_atleast_once = true;

            currently_running = fuzzer.name;
            if (options.verbose) {
                std::cout << " • Running: " << currently_running << "\n";
            }
            fuzzer.fn(program, context, data);
        }
    }

    if (!options.filter.empty() && !ran_atleast_once) {
        std::cerr << "ERROR: --filter=" << options.filter << " did not match any fuzzers\n";
        exit(EXIT_FAILURE);
    }
}

}  // namespace tint::fuzz::wgsl
