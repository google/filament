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

#include "src/tint/cmd/fuzz/ir/fuzz.h"

#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/defer.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/cmd/fuzz/ir/helpers/substitute_overrides_config.h"
#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/lang/core/ir/transform/substitute_overrides.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#endif

namespace tint::fuzz::ir {

#if TINT_BUILD_IR_BINARY
/// @returns a reference to the static list of registered IRFuzzers.
/// @note this is not a global, as the static initializers that register fuzzers may be called
/// before this vector is constructed.
Vector<IRFuzzer, 32>& Fuzzers() {
    static Vector<IRFuzzer, 32> fuzzers;
    return fuzzers;
}

thread_local std::string_view currently_running;
#endif  // TINT_BUILD_IR_BINARY

void Register(const IRFuzzer& fuzzer) {
#if TINT_BUILD_WGSL_READER
    wgsl::Register({
        fuzzer.name,
        [fn = fuzzer.fn, pre_capabilities = fuzzer.pre_capabilities](
            const Program& program, const fuzz::wgsl::Context& context,
            Slice<const std::byte> data) {
            if (program.AST().Enables().Any(tint::wgsl::reader::IsUnsupportedByIR)) {
                if (context.options.verbose) {
                    std::cout << "   - Features are not supported by IR.\n";
                }
                return;
            }

            auto ir = tint::wgsl::reader::ProgramToLoweredIR(program);
            if (ir != Success) {
                return;
            }

            auto cfg = SubstituteOverridesConfig(ir.Get());
            auto substituteOverridesResult =
                tint::core::ir::transform::SubstituteOverrides(ir.Get(), cfg);
            if (substituteOverridesResult != Success) {
                if (context.options.verbose) {
                    std::cout << "   - Substitute overrides failed.\n";
                }
                return;
            }

            // Validate the IR against the fuzzer's preconditions before running.
            // We don't consider validation failure here to be an issue, as it only signals that
            // there is a bug somewhere in the components run above. Those components have their own
            // IR fuzzers.
            if (auto val = core::ir::Validate(ir.Get(), pre_capabilities); val != Success) {
                if (context.options.verbose) {
                    std::cout
                        << "   Failed to validate against fuzzer capabilities before running\n";
                }
                return;
            }

            // Copy relevant options from wgsl::Context to ir::Context
            fuzz::ir::Context ir_context;
            ir_context.options.filter = context.options.filter;
            ir_context.options.run_concurrently = context.options.run_concurrently;
            ir_context.options.verbose = context.options.verbose;
            ir_context.options.dxc = context.options.dxc;
            ir_context.options.dump = context.options.dump;
            [[maybe_unused]] auto result = fn(ir.Get(), ir_context, data);
        },
    });
#endif

#if TINT_BUILD_IR_BINARY
    Fuzzers().Push(fuzzer);
#endif  // TINT_BUILD_IR_BINARY
}

#if TINT_BUILD_IR_BINARY
void Run(const std::function<tint::core::ir::Module()>& acquire_module,
         const Options& options,
         Slice<const std::byte> data) {
    // Ensure that fuzzers are sorted. Without this, the fuzzers may be registered in any order,
    // leading to non-determinism, which we must avoid.
    TINT_STATIC_INIT(Fuzzers().Sort([](auto& a, auto& b) { return a.name < b.name; }));

    Context context;
    context.options = options;

    bool ran_atleast_once = false;

    // Run each of the program fuzzer functions
    if (options.run_concurrently) {
        const size_t n = Fuzzers().Length();
        tint::Vector<std::thread, 32> threads;
        threads.Reserve(n);
        for (size_t i = 0; i < n; i++) {
            if (!options.filter.empty() &&
                Fuzzers()[i].name.find(options.filter) == std::string::npos) {
                continue;
            }
            ran_atleast_once = true;

            threads.Push(std::thread([i, &acquire_module, &data, &context] {
                auto& fuzzer = Fuzzers()[i];
                currently_running = fuzzer.name;
                if (context.options.verbose) {
                    std::cout << " • [" << i << "] Running: " << currently_running << '\n';
                }
                auto mod = acquire_module();
                if (tint::core::ir::Validate(mod, fuzzer.pre_capabilities) != tint::Success) {
                    // Failing before running indicates that this input violates the pre-conditions
                    // for this pass, so should be skipped.
                    if (context.options.verbose) {
                        std::cout
                            << "   Failed to validate against fuzzer capabilities before running\n";
                    }
                    return;
                }

                if (auto result = fuzzer.fn(mod, context, data); result != Success) {
                    if (context.options.verbose) {
                        std::cout << "   Failed to execute fuzzer: " << result.Failure() << "\n";
                    }
                }

                if (auto result = tint::core::ir::Validate(mod, fuzzer.post_capabilities);
                    result != Success) {
                    // Failing after running indicates the pass is doing something unexpected and
                    // has violated its own post-conditions.
                    TINT_ICE() << "Failed to validate against fuzzer capabilities after running:\n"
                               << result.Failure() << "\n";
                }
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
                std::cout << " • Running: " << currently_running << '\n';
            }
            auto mod = acquire_module();
            if (tint::core::ir::Validate(mod, fuzzer.pre_capabilities) != tint::Success) {
                // Failing before running indicates that this input violates the pre-conditions for
                // this pass, so should be skipped.
                if (options.verbose) {
                    std::cout
                        << "   Failed to validate against fuzzer capabilities before running\n";
                }
                continue;
            }

            if (auto result = fuzzer.fn(mod, context, data); result != Success) {
                if (options.verbose) {
                    std::cout << "   Failed to execute fuzzer: " << result.Failure() << "\n";
                }
                continue;
            }

            if (auto result = tint::core::ir::Validate(mod, fuzzer.post_capabilities);
                result != Success) {
                // Failing after running indicates the pass is doing something unexpected and
                // has violated its own post-conditions.
                TINT_ICE() << "Failed to validate against fuzzer capabilities after running:\n"
                           << result.Failure() << "\n";
            }
        }
    }

    if (!options.filter.empty() && !ran_atleast_once) {
        std::cerr << "ERROR: --filter=" << options.filter << " did not match any fuzzers\n";
        exit(EXIT_FAILURE);
    }
}
#endif  // TINT_BUILD_IR_BINARY

}  // namespace tint::fuzz::ir
