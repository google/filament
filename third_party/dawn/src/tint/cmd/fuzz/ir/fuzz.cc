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
#include <span>
#include <string>
#include <thread>

#include "src/tint/cmd/fuzz/common/runner.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/defer.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
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

static thread_local std::string_view currently_running;
#endif  // TINT_BUILD_IR_BINARY

void Register(const IRFuzzer& fuzzer) {
#if TINT_BUILD_WGSL_READER
    wgsl::Register({
        fuzzer.name,
        [fn = fuzzer.fn, fuzzer](const Program& program, const fuzz::wgsl::Context& context,
                                 std::span<const std::byte> data) {
            if (program.AST().Enables().Any(tint::wgsl::reader::IsUnsupportedByIR)) {
                if (context.options.verbose) {
                    std::cout << "   - Features are not supported by IR.\n";
                }
                return;
            }

            // Enable validation assertions.
            // Any validation failure after this point is a bug in Tint which we want to find.
            tint::wgsl::reader::IROptions ir_options{
                .dump_ir_when_validating = context.options.dump_ir_when_validating,
                .enable_validation_asserts = !context.options.disable_ir_validator,
            };

            auto ir = tint::wgsl::reader::ProgramToLoweredIR(program, ir_options);
            if (ir != Success) {
                return;
            }

            // Skip this fuzzer case if the component being fuzzed does not support one of the
            // properties used by the module.
            auto unsupported = ir.Get().properties & fuzzer.unsupported_properties;
            if (!unsupported.Empty()) {
                if (context.options.verbose) {
                    std::cout << "unsupported property '" << *unsupported.begin() << "'";
                }
                return;
            }

            // Validate the IR before running, because IR passes are not expected to handle invalid
            // inputs, so don't want spurious reports if the pass crashes.
            //
            // NOTE: Do not ICE here, because there is other fuzzer passes that specifically check
            // the WGSL->IR conversion doesn't create illegal IR. If an ICE occurred here it would
            // create duplicate issues that obscures where the issue actually lies.
            if (!context.options.disable_ir_validator) {
                if (auto val = core::ir::Validate(ir.Get(), "start " + std::string(fuzzer.name));
                    val != Success) {
                    if (context.options.verbose) {
                        std::cout << "   Failed to validate against before running\n";
                    }
                    return;
                }
            }

            // Copy relevant options from wgsl::Context to ir::Context
            fuzz::ir::Context ir_context;
            ir_context.options.filter = context.options.filter;
            ir_context.options.run_concurrently = context.options.run_concurrently;
            ir_context.options.verbose = context.options.verbose;
            ir_context.options.dxc = context.options.dxc;
#if TINT_BUILD_FUZZER_VULKAN_SUPPORT
            ir_context.options.vk_icd = context.options.vk_icd;
#endif
            ir_context.options.dump = context.options.dump;
            ir_context.options.dump_ir_when_validating = context.options.dump_ir_when_validating;
            ir_context.options.disable_ir_validator = context.options.disable_ir_validator;
            auto result = fn(ir.Get(), ir_context, data);
            if (result != Success) {
                if (context.options.verbose) {
                    std::cout << "   " << result.Failure() << "\n";
                }
                return;
            }

            if (!context.options.disable_ir_validator) {
                if (auto val =
                        tint::core::ir::Validate(ir.Get(), "finish " + std::string(fuzzer.name));
                    val != Success) {
                    TINT_ICE() << "Failed to validate against after running:\n"
                               << val.Failure() << "\n";
                }
            }
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
         std::span<const std::byte> data) {
    // Ensure that fuzzers are sorted. Without this, the fuzzers may be registered in any order,
    // leading to non-determinism, which we must avoid.
    TINT_STATIC_INIT(Fuzzers().Sort([](auto& a, auto& b) { return a.name < b.name; }));

    Context context;
    context.options = options;

    if (!context.options.disable_ir_validator) {
        auto mod = acquire_module();
        // Inputs that fail validation should not be run against the fuzzing passes, since they are
        // not expected to handle invalid inputs.
        if (tint::core::ir::Validate(mod, "Pre-run validation") != tint::Success) {
            if (context.options.verbose) {
                std::cout << "Failed to validate before running\n";
            }
            return;
        }
    }

    // Run each of the program fuzzer functions
    tint::fuzz::common::RunFuzzers(Fuzzers(), options, [&](const IRFuzzer& fuzzer, size_t i) {
        currently_running = fuzzer.name;
        if (options.verbose) {
            if (options.run_concurrently) {
                std::cout << " • [" << i << "] Running: " << currently_running << "\n";
            } else {
                std::cout << " • Running: " << currently_running << "\n";
            }
        }
        auto mod = acquire_module();
        mod.dump_ir_when_validating = context.options.dump_ir_when_validating;

        // Skip this fuzzer case if the component being fuzzed does not support one of the
        // properties used by the module.
        auto unsupported = mod.properties & fuzzer.unsupported_properties;
        if (!unsupported.Empty()) {
            if (context.options.verbose) {
                std::cout << "unsupported property '" << *unsupported.begin() << "'";
            }
            return;
        }

        // Enable validation assertions.
        // Any validation failure after this point is a bug in Tint which we want to find.
        mod.enable_validation_asserts = !context.options.disable_ir_validator;

        if (auto result = fuzzer.fn(mod, context, data); result != Success) {
            if (context.options.verbose) {
                std::cout << "   Failed to execute fuzzer: " << result.Failure() << "\n";
            }
            return;
        }

        if (!context.options.disable_ir_validator) {
            if (auto result =
                    tint::core::ir::Validate(mod, "finish " + std::string(currently_running));
                result != Success) {
                // Failing after running indicates the pass is doing something unexpected and
                // has violated its own post-conditions.
                TINT_ICE() << "Failed to validate after running:\n" << result.Failure() << "\n";
            }
        }
    });
}
#endif  // TINT_BUILD_IR_BINARY

}  // namespace tint::fuzz::ir
