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

#ifndef SRC_TINT_CMD_FUZZ_WGSL_FUZZ_H_
#define SRC_TINT_CMD_FUZZ_WGSL_FUZZ_H_

#include <iostream>
#include <string>
#include <tuple>
#include <utility>

#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/utils/bytes/buffer_reader.h"
#include "src/tint/utils/bytes/decoder.h"
#include "src/tint/utils/containers/enum_set.h"
#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/macros/static_init.h"

namespace tint::fuzz::wgsl {

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

/// ProgramProperties is an enumerator of flags used to describe characteristics of the input
/// program.
enum class ProgramProperties {
    /// The program has address spaces which have been shadowed
    kAddressSpacesShadowed,
    /// The program has builtin functions which have been shadowed
    kBuiltinFnsShadowed,
    /// The program has builtin types which have been shadowed
    kBuiltinTypesShadowed,
    /// The program has multiple entry points
    kMultipleEntryPoints,
};

/// Context holds information about the fuzzer options and the input program.
struct Context {
    /// The options used for Run()
    Options options;
    /// The properties of the input program
    EnumSet<ProgramProperties> program_properties;
};

/// ProgramFuzzer describes a fuzzer function that takes a WGSL program as input
struct ProgramFuzzer {
    /// @param name the name of the fuzzer
    /// @param fn the fuzzer function with the signature `void(const Program&, const Context&, ...)`
    /// @returns a ProgramFuzzer that invokes the function @p fn with the Program, Context, along
    /// with any additional arguments which are deserialized from the fuzzer input.
    template <typename... ARGS>
    static ProgramFuzzer Create(std::string_view name,
                                void (*fn)(const Program&, const Context&, ARGS...)) {
        if constexpr (sizeof...(ARGS) > 0) {
            auto fn_with_decode = [fn](const Program& program, const Context& context,
                                       Slice<const std::byte> data) {
                if (!data.data) {
                    if (context.options.verbose) {
                        std::cout << "   - Data expected but no data provided.\n";
                    }
                    return;
                }
                bytes::BufferReader reader{data};
                auto data_args = bytes::Decode<std::tuple<std::decay_t<ARGS>...>>(reader);
                if (data_args == Success) {
                    auto all_args =
                        std::tuple_cat(std::tuple<const Program&, const Context&>{program, context},
                                       data_args.Get());
                    std::apply(*fn, all_args);
                } else {
                    if (context.options.verbose) {
                        std::cout << "   - Failed to decode fuzzer argument data.\n";
                    }
                }
            };
            return ProgramFuzzer{name, std::move(fn_with_decode)};
        } else {
            return ProgramFuzzer{
                name,
                [fn](const Program& program, const Context& context, Slice<const std::byte>) {
                    fn(program, context);
                },
            };
        }
    }

    /// @param name the name of the fuzzer
    /// @param fn the fuzzer function with the signature `void(const Program&, ...)`
    /// @returns a ProgramFuzzer that invokes the function @p fn with the Program, along
    /// with any additional arguments which are deserialized from the fuzzer input.
    template <typename... ARGS>
    static ProgramFuzzer Create(std::string_view name, void (*fn)(const Program&, ARGS...)) {
        if constexpr (sizeof...(ARGS) > 0) {
            auto fn_with_decode = [fn](const Program& program, const Context& context,
                                       Slice<const std::byte> data) {
                if (!data.data) {
                    if (context.options.verbose) {
                        std::cout << "   - Data expected but no data provided.\n";
                    }
                    return;
                }
                bytes::BufferReader reader{data};
                auto data_args = bytes::Decode<std::tuple<std::decay_t<ARGS>...>>(reader);
                if (data_args == Success) {
                    auto all_args =
                        std::tuple_cat(std::tuple<const Program&>{program}, data_args.Get());
                    std::apply(*fn, all_args);
                } else {
                    if (context.options.verbose) {
                        std::cout << "   - Failed to decode fuzzer argument data.\n";
                    }
                }
            };
            return ProgramFuzzer{name, std::move(fn_with_decode)};
        } else {
            return ProgramFuzzer{
                name,
                [fn](const Program& program, const Context&, Slice<const std::byte>) {
                    fn(program);
                },
            };
        }
    }

    /// Name of the fuzzer function
    std::string_view name;
    /// The fuzzer function
    std::function<void(const Program&, const Context&, Slice<const std::byte> data)> fn;
};

/// Runs all the registered WGSL fuzzers with the supplied WGSL
/// @param wgsl the input WGSL
/// @param options the options for running the fuzzers
/// @param data additional data used for fuzzing
void Run(std::string_view wgsl, const Options& options, Slice<const std::byte> data);

/// Registers the fuzzer function with the WGSL fuzzer executable.
/// @param fuzzer the fuzzer
void Register(const ProgramFuzzer& fuzzer);

/// TINT_WGSL_PROGRAM_FUZZER registers the fuzzer function to run as part of `tint_wgsl_fuzzer`
/// The function must have one of the signatures:
/// • `void(const Program&, ...)`
/// • `void(const Program&, const Options&, ...)`
/// Where `...` is any number of deserializable parameters which are decoded from the base64
/// content of the WGSL comments.
/// @see bytes::Decode()
#define TINT_WGSL_PROGRAM_FUZZER(FUNCTION, ...)    \
    TINT_STATIC_INIT(::tint::fuzz::wgsl::Register( \
        ::tint::fuzz::wgsl::ProgramFuzzer::Create(#FUNCTION, FUNCTION, ##__VA_ARGS__)))

}  // namespace tint::fuzz::wgsl

#endif  // SRC_TINT_CMD_FUZZ_WGSL_FUZZ_H_
