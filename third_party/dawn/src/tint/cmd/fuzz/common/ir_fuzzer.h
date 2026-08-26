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

#ifndef SRC_TINT_CMD_FUZZ_COMMON_IR_FUZZER_H_
#define SRC_TINT_CMD_FUZZ_COMMON_IR_FUZZER_H_

#include <cstddef>
#include <functional>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "src/tint/cmd/fuzz/common/options.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/bytes/buffer_reader.h"
#include "src/tint/utils/bytes/decoder.h"
#include "src/tint/utils/macros/static_init.h"
#include "src/tint/utils/result.h"

namespace tint::fuzz::ir {

/// Context holds information about the fuzzer options and the input program.
struct Context {
    /// The options used for Run()
    tint::fuzz::common::Options options;
};

/// IRFuzzer describes a fuzzer function that takes a IR module as input
struct IRFuzzer {
    /// @param name the name of the fuzzer
    /// @param fn the fuzzer function
    /// @param unsupported_properties the properties that are not supported by the fuzzer
    /// @returns an IRFuzzer that invokes the function @p fn with the IR module, along with any
    /// additional arguments which are deserialized from the fuzzer input.
    template <typename... ARGS>
    static IRFuzzer Create(std::string_view name,
                           Result<SuccessType> (*fn)(core::ir::Module&, const Context&, ARGS...),
                           core::ir::Properties unsupported_properties = {}) {
        if constexpr (sizeof...(ARGS) > 0) {
            auto fn_with_decode = [fn](core::ir::Module& module, const Context& context,
                                       std::span<const std::byte> data) -> Result<SuccessType> {
                if (data.empty()) {
                    return Failure{"Data expected but no data provided."};
                }

                bytes::BufferReader reader{data};
                auto data_args = bytes::Decode<std::tuple<std::decay_t<ARGS>...>>(reader);
                if (data_args != Success) {
                    return Failure("Failed to decode fuzzer argument data: " +
                                   data_args.Failure().reason);
                }

                auto all_args =
                    std::tuple_cat(std::tuple<core::ir::Module&, const Context&>{module, context},
                                   data_args.Get());
                return std::apply(*fn, all_args);
            };
            return IRFuzzer{
                name,
                std::move(fn_with_decode),
                unsupported_properties,
            };
        } else {
            return IRFuzzer{
                name,
                [fn](core::ir::Module& module, const Context& context, std::span<const std::byte>)
                    -> Result<SuccessType> { return fn(module, context); },
                unsupported_properties,
            };
        }
    }

    /// Name of the fuzzer function
    std::string_view name;
    /// The fuzzer function
    /// Takes in the module and any sidecar data, returns true iff transform succeeded in running,
    /// otherwise false
    std::function<
        Result<SuccessType>(core::ir::Module&, const Context&, std::span<const std::byte> data)>
        fn;
    /// The IR properties that are not supported by the component being fuzzed.
    core::ir::Properties unsupported_properties;
};

/// Registers the fuzzer function with the IR fuzzer executable.
/// @param fuzzer the fuzzer
void Register([[maybe_unused]] const IRFuzzer& fuzzer);

/// TINT_IR_MODULE_FUZZER registers the fuzzer function, the optional argument is a list of IR
/// properties that are not supported by the fuzzer.
#define TINT_IR_MODULE_FUZZER(FUNCTION, ...)     \
    TINT_STATIC_INIT(::tint::fuzz::ir::Register( \
        ::tint::fuzz::ir::IRFuzzer::Create(#FUNCTION, FUNCTION __VA_OPT__(, ) __VA_ARGS__)))

}  // namespace tint::fuzz::ir

#endif  // SRC_TINT_CMD_FUZZ_COMMON_IR_FUZZER_H_
