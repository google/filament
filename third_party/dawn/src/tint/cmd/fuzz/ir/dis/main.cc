// Copyright 2024 The Dawn & Tint Authors
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

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "src/tint/api/tint.h"
#include "src/tint/cmd/common/helper.h"
#include "src/tint/lang/core/ir/binary/decode.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/spirv/writer/writer.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/command/args.h"
#include "src/tint/utils/command/cli.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/color_mode.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"

#include "spirv-tools/libspirv.hpp"

TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS();
#include "src/tint/utils/protos/ir_fuzz/ir_fuzz.pb.h"
TINT_END_DISABLE_PROTOBUF_WARNINGS();

namespace {

/// @param data spriv shader to be converted
/// @returns human readable text representation of a SPIR-V shader
tint::Result<std::string> DisassembleSpv(const std::vector<uint32_t>& data) {
    std::string spv_errors;
    spv_target_env target_env = SPV_ENV_VULKAN_1_1;

    auto msg_consumer = [&spv_errors](spv_message_level_t level, const char*,
                                      const spv_position_t& position, const char* message) {
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                spv_errors +=
                    "error: line " + std::to_string(position.index) + ": " + message + "\n";
                break;
            case SPV_MSG_WARNING:
                spv_errors +=
                    "warning: line " + std::to_string(position.index) + ": " + message + "\n";
                break;
            case SPV_MSG_INFO:
                spv_errors +=
                    "info: line " + std::to_string(position.index) + ": " + message + "\n";
                break;
            case SPV_MSG_DEBUG:
                break;
        }
    };

    spvtools::SpirvTools tools(target_env);
    tools.SetMessageConsumer(msg_consumer);

    std::string result;
    if (!tools.Disassemble(
            data, &result,
            SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES)) {
        return tint::Failure(spv_errors);
    }

    return result;
}

enum class Format : uint8_t {
    kUnknown,
    kSpirv,
    kSpvAsm,
    kWgsl,
};

/// @param filename the filename to inspect
/// @returns the inferred format for the filename suffix
Format InferFormat(const std::string& filename) {
    if (tint::HasSuffix(filename, ".spv")) {
        return Format::kSpirv;
    }
    if (tint::HasSuffix(filename, ".spvasm")) {
        return Format::kSpvAsm;
    }

    if (tint::HasSuffix(filename, ".wgsl")) {
        return Format::kWgsl;
    }

    return Format::kUnknown;
}

struct Options {
    std::unique_ptr<tint::StyledTextPrinter> printer;

    std::string input_filename;
    std::string output_filename;

    Format format = Format::kUnknown;

    bool validate = false;
    bool dump_wgsl = false;
    bool dump_spirv = false;
};

bool ParseArgs(tint::VectorRef<std::string_view> arguments, Options* opts) {
    using namespace tint::cli;  // NOLINT(build/namespaces)

    OptionSet options;

    tint::Vector<EnumName<Format>, 3> format_enum_names{
        EnumName(Format::kSpirv, "spirv"),
        EnumName(Format::kSpvAsm, "spvasm"),
        EnumName(Format::kWgsl, "wgsl"),
    };

    auto& fmt = options.Add<EnumOption<Format>>("format",
                                                R"(Output format to be written to file.
If not provided, will be inferred from output filename extension:
  .spvasm -> spvasm
  .spv    -> spirv
  .wgsl   -> wgsl)",
                                                format_enum_names, ShortName{"f"});
    TINT_DEFER(opts->format = fmt.value.value_or(Format::kUnknown));

    auto& col = options.Add<EnumOption<tint::ColorMode>>(
        "color", "Use colored output",
        tint::Vector{
            EnumName{tint::ColorMode::kPlain, "off"},
            EnumName{tint::ColorMode::kDark, "dark"},
            EnumName{tint::ColorMode::kLight, "light"},
        },
        ShortName{"col"}, Default{tint::ColorModeDefault()});
    TINT_DEFER(opts->printer = CreatePrinter(*col.value));

    auto& output = options.Add<StringOption>(
        "output-filename",
        "Output file name for shader output, if not specified nothing will be written to file",
        ShortName{"o"}, Parameter{"name"});
    TINT_DEFER(opts->output_filename = output.value.value_or(""));

    auto& validate = options.Add<BoolOption>("validate", "Runs the IR validator on the source IR",
                                             ShortName{"V"}, Default{false});
    TINT_DEFER(opts->validate = *validate.value);

    auto& dump_wgsl = options.Add<BoolOption>(
        "dump-wgsl", "Writes the WGSL form of input to stdout, may fail due to validation errors",
        Alias{"emit-wgsl"}, Default{false});
    TINT_DEFER(opts->dump_wgsl = *dump_wgsl.value);

    auto& dump_spirv = options.Add<BoolOption>(
        "dump-spirv",
        "Writes the SPIR-V form of input to stdout, may fail due to validation errors",
        Alias{"emit-spirv"}, Default{false});
    TINT_DEFER(opts->dump_spirv = *dump_spirv.value);

    auto& help = options.Add<BoolOption>("help", "Show usage", ShortName{"h"});

    auto show_usage = [&] {
        std::cout << R"(Usage: ir_fuzz_dis [options] <input-file>

Options:
)";
        options.ShowHelp(std::cout);
    };

    auto result = options.Parse(arguments);
    if (result != tint::Success) {
        std::cerr << result.Failure() << "\n";
        show_usage();
        return false;
    }

    if (help.value.value_or(false)) {
        show_usage();
        return false;
    }

    auto args = result.Get();
    if (args.Length() > 1) {
        std::cerr << "More than one input arg specified: "
                  << tint::Join(Transform(args, tint::Quote), ", ") << "\n";
        return false;
    }

    if (args.Length() == 1) {
        opts->input_filename = args[0];
    }

    return true;
}

/// Infer any missing option values, then validate that the provide values are
/// usable
/// @param options set of values provided by the user to the binary
/// @returns true if the options are usable, otherwise false. Prints error messages to STDERR
bool InferAndValidateOptions(Options& options) {
    if (options.format == Format::kUnknown && !options.output_filename.empty()) {
        options.format = InferFormat(options.output_filename);
        if (options.format == Format::kUnknown) {
            std::cerr << "Unable to determine output format from filename, "
                      << options.output_filename << "\n";
            return false;
        }
    }

    if (options.format != Format::kUnknown && options.output_filename.empty()) {
        std::cerr << "Format provided, but no output filename provided\n";
        return false;
    }

    return true;
}

/// @returns a fuzzer test case protobuf for the given file
/// @param options program options that contains the filename to be read, etc.
tint::Result<tint::cmd::fuzz::ir::pb::Root> GenerateFuzzCaseProto(const Options& options) {
    tint::cmd::fuzz::ir::pb::Root fuzz_pb;

    std::fstream input(options.input_filename, std::ios::in | std::ios::binary);
    if (!fuzz_pb.ParseFromIstream(&input)) {
        return tint::Failure{"Unable to parse bytes into test case protobuf"};
    }

    return std::move(fuzz_pb);
}

/// Prints IR text representation to STDOUT
/// @param options options passed into the binary
/// @param module IR module parsed from input protobuf
void EmitIR(const Options& options, tint::core::ir::Module& module) {
    const auto ir_text = tint::core::ir::Disassembler(module).Text();
    if (options.output_filename.empty()) {
        options.printer->Print(ir_text);
        options.printer->Print(tint::StyledText{} << "\n");
    }
}

/// Prints WGSL shader to STDOUT or to a file as determined by options
/// @param options options passed into the binary
/// @param module IR module parsed from input protobuf
/// @returns true if all operations succeeded, otherwise false. Prints error messages to STDERR
bool EmitWGSL(const Options& options, tint::core::ir::Module& module) {
    if (!options.dump_wgsl && options.format != Format::kWgsl) {
        return true;
    }

    tint::wgsl::writer::ProgramOptions writer_options;
    auto output = tint::wgsl::writer::WgslFromIR(module, writer_options);
    if (output != tint::Success) {
        std::cerr << "Failed to convert IR to WGSL Program: " << output.Failure() << "\n";
        return false;
    }

    if (options.dump_wgsl) {
        options.printer->Print(tint::StyledText{} << output->wgsl);
        options.printer->Print(tint::StyledText{} << "\n");
    }

    if (options.format == Format::kWgsl) {
        if (!tint::cmd::WriteFile(options.output_filename, "w", output->wgsl)) {
            std::cerr << "Unable to print WGSL to file, " << options.output_filename << "\n";
            return false;
        }
    }

    return true;
}

/// Prints SPIR-V shader to STDOUT or to a file as determined by options
/// @param options options passed into the binary
/// @param module IR module parsed from input protobuf
/// @returns true if all operations succeeded, otherwise false. Prints error messages to STDERR
bool EmitSpv(const Options& options, tint::core::ir::Module& module) {
    if (!options.dump_spirv && options.format != Format::kSpirv &&
        options.format != Format::kSpvAsm) {
        return true;
    }
    tint::spirv::writer::Options gen_options;
    auto spv = tint::spirv::writer::Generate(module, gen_options);

    if (spv != tint::Success) {
        std::cerr << "Failed to convert IR to SPIR-V: " << spv.Failure() << "\n";
        return false;
    }

    if (options.dump_spirv || options.format == Format::kSpvAsm) {
        auto spv_asm = DisassembleSpv(spv.Get().spirv);
        if (spv_asm != tint::Success) {
            std::cerr << "Failed to disassemble SPIR-V: " << spv_asm.Failure() << "\n";
            return false;
        }

        if (options.dump_spirv) {
            options.printer->Print(tint::StyledText{} << spv_asm.Get());
            options.printer->Print(tint::StyledText{} << "\n");
        }

        if (options.format == Format::kSpvAsm) {
            if (!tint::cmd::WriteFile(options.output_filename, "w", spv_asm.Get())) {
                std::cerr << "Unable to print SPIR-V text to file, " << options.output_filename
                          << "\n";
                return false;
            }
        }
    }

    if (options.format == Format::kSpirv) {
        if (!tint::cmd::WriteFile(options.output_filename, "wb", spv.Get().spirv)) {
            std::cerr << "Unable to print SPIR-V to file, " << options.output_filename << "\n";
            return false;
        }
    }
    return true;
}

/// Converts and displays the given test case file as determined by the options
///
/// NB: There is multiple ::Decode calls, because each emission step may modify the passed in Module
/// and modules are intentionally non-copyable.
///
/// @param options options passed into the binary
/// @returns true if all operations succeeded, otherwise false. Prints error messages to STDERR
bool Run(const Options& options) {
    auto fuzz_pb = GenerateFuzzCaseProto(options);
    if (fuzz_pb != tint::Success) {
        std::cerr << "Failed to read test case protobuf: " << fuzz_pb.Failure() << "\n";
        return false;
    }

    {
        auto module = tint::core::ir::binary::Decode(fuzz_pb.Get().module());
        if (module != tint::Success) {
            std::cerr << "Unable to decode ir protobuf from test case protobuf: "
                      << module.Failure() << "\n";
            return false;
        }
    }

    {
        auto module = tint::core::ir::binary::Decode(fuzz_pb.Get().module());
        EmitIR(options, module.Get());

        if (options.validate) {
            auto res = tint::core::ir::Validate(
                module.Get(), tint::core::ir::Capabilities{
                                  tint::core::ir::Capability::kAllow8BitIntegers,
                                  tint::core::ir::Capability::kAllow64BitIntegers,
                                  tint::core::ir::Capability::kAllowClipDistancesOnF32,
                                  tint::core::ir::Capability::kAllowHandleVarsWithoutBindings,
                                  tint::core::ir::Capability::kAllowModuleScopeLets,
                                  tint::core::ir::Capability::kAllowOverrides,
                                  tint::core::ir::Capability::kAllowPointersAndHandlesInStructures,
                                  tint::core::ir::Capability::kAllowRefTypes,
                                  tint::core::ir::Capability::kAllowVectorElementPointer,
                                  tint::core::ir::Capability::kAllowPrivateVarsInFunctions,
                                  tint::core::ir::Capability::kAllowPhonyInstructions,
                                  tint::core::ir::Capability::kAllowAnyLetType,
                              });
            if (res == tint::Success) {
                std::cout << "IR module is valid.\n";
            } else {
                std::cerr << res.Failure();
            }
        }
    }

    {
        auto module = tint::core::ir::binary::Decode(fuzz_pb.Get().module());
        if (!EmitWGSL(options, module.Get())) {
            return false;
        }
    }

    {
        auto module = tint::core::ir::binary::Decode(fuzz_pb.Get().module());
        if (!EmitSpv(options, module.Get())) {
            return false;
        }
    }

    return true;
}

}  // namespace

int main(int argc, const char** argv) {
    tint::Vector<std::string_view, 8> arguments = tint::args::Vectorize(argc, argv);
    Options options;

    tint::Initialize();

    if (!ParseArgs(arguments, &options)) {
        return EXIT_FAILURE;
    }

    if (!InferAndValidateOptions(options)) {
        return EXIT_FAILURE;
    }

    if (!Run(options)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
