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

#ifndef SRC_TINT_CMD_FUZZ_IR_FUZZ_H_
#define SRC_TINT_CMD_FUZZ_IR_FUZZ_H_

#include <functional>
#include <string>
#include <tuple>
#include <utility>

#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/bytes/buffer_reader.h"
#include "src/tint/utils/bytes/decoder.h"
#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/macros/static_init.h"

namespace tint::core::ir {
class Module;
}

namespace tint::fuzz::ir {

/// Options for Run()
struct Options {
    /// If not empty, only run the fuzzers with the given substring.
    std::string filter;
    /// If true, the fuzzers will be run concurrently on separate threads.
    bool run_concurrently = false;
    /// If true, print the fuzzer name to stdout before running.
    bool verbose = false;
    /// If not empty, load DXC from this path when fuzzing HLSL generation.
    std::string dxc;
    /// If true, dump shader input/output text to stdout
    bool dump = false;
};

/// Context holds information about the fuzzer options and the input program.
struct Context {
    /// The options used for Run()
    Options options;
};

/// IRFuzzer describes a fuzzer function that takes a IR module as input
struct IRFuzzer {
    /// @param name the name of the fuzzer
    /// @param fn the fuzzer function
    /// @param pre_capabilities the capabilities that are used before the fuzzer runs
    /// @param post_capabilities the capabilities that are used after the fuzzer runs
    /// @returns an IRFuzzer that invokes the function @p fn with the IR module, along with any
    /// additional arguments which are deserialized from the fuzzer input.
    template <typename... ARGS>
    static IRFuzzer Create(std::string_view name,
                           Result<SuccessType> (*fn)(core::ir::Module&, const Context&, ARGS...),
                           core::ir::Capabilities pre_capabilities,
                           core::ir::Capabilities post_capabilities) {
        if constexpr (sizeof...(ARGS) > 0) {
            auto fn_with_decode = [fn](core::ir::Module& module, const Context& context,
                                       Slice<const std::byte> data) -> Result<SuccessType> {
                if (!data.data) {
                    return Failure{"Invalid data"};
                }

                bytes::BufferReader reader{data};
                auto data_args = bytes::Decode<std::tuple<std::decay_t<ARGS>...>>(reader);
                if (data_args != Success) {
                    return data_args.Failure();
                }

                auto all_args =
                    std::tuple_cat(std::tuple<core::ir::Module&, const Context&>{module, context},
                                   data_args.Get());
                return std::apply(*fn, all_args);
            };
            return IRFuzzer{name, std::move(fn_with_decode), pre_capabilities, post_capabilities};
        } else {
            return IRFuzzer{
                name,
                [fn](core::ir::Module& module, const Context& context,
                     Slice<const std::byte>) -> Result<SuccessType> { return fn(module, context); },
                pre_capabilities,
                post_capabilities,
            };
        }
    }

    /// @param name the name of the fuzzer
    /// @param fn the fuzzer function
    /// @param capabilities the capabilities that are used before and after the fuzzer runs
    /// @returns an IRFuzzer that invokes the function @p fn with the IR module, along with any
    /// additional arguments which are deserialized from the fuzzer input.
    template <typename... ARGS>
    static IRFuzzer Create(std::string_view name,
                           Result<SuccessType> (*fn)(core::ir::Module&, const Context&, ARGS...),
                           core::ir::Capabilities capabilities) {
        return Create(name, fn, capabilities, capabilities);
    }

    /// Name of the fuzzer function
    std::string_view name;
    /// The fuzzer function
    /// Takes in the module and any sidecar data, returns true iff transform succeeded in running,
    /// otherwise false
    std::function<
        Result<SuccessType>(core::ir::Module&, const Context&, Slice<const std::byte> data)>
        fn;
    /// The IR capabilities that are used before the fuzzer runs.
    core::ir::Capabilities pre_capabilities;
    /// The IR capabilities that are used after the fuzzer runs.
    core::ir::Capabilities post_capabilities;
};

/// Registers the fuzzer function with the IR fuzzer executable.
/// @param fuzzer the fuzzer
void Register([[maybe_unused]] const IRFuzzer& fuzzer);

#if TINT_BUILD_IR_BINARY
/// Runs all the registered IR fuzzers with the supplied IR module
/// @param acquire_module a function to obtain an IR module
/// @param options the options for running the fuzzers
/// @param data additional data used for fuzzing
void Run(const std::function<tint::core::ir::Module()>& acquire_module,
         const Options& options,
         Slice<const std::byte> data);
#endif  // TINT_BUILD_IR_BINARY

/// TINT_IR_MODULE_FUZZER registers the fuzzer function, the variadic args are either a single
/// Capabilities to use before and after the function runs, or two different Capabilities, one for
/// before and one for after. See Create above for more details.
#define TINT_IR_MODULE_FUZZER(FUNCTION, ...)     \
    TINT_STATIC_INIT(::tint::fuzz::ir::Register( \
        ::tint::fuzz::ir::IRFuzzer::Create(#FUNCTION, FUNCTION, __VA_ARGS__)))

}  // namespace tint::fuzz::ir

#endif  // SRC_TINT_CMD_FUZZ_IR_FUZZ_H_
