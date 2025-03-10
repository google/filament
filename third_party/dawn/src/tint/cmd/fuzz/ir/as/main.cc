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

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include "src/tint/api/tint.h"
#include "src/tint/cmd/common/helper.h"
#include "src/tint/lang/core/ir/binary/encode.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/utils/command/args.h"
#include "src/tint/utils/command/cli.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/color_mode.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"

TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS();
#include "src/tint/utils/protos/ir_fuzz/ir_fuzz.pb.h"
TINT_END_DISABLE_PROTOBUF_WARNINGS();

namespace {

struct Options {
    std::unique_ptr<tint::StyledTextPrinter> printer;

    std::string input_filename;
    std::string output_filename;
    std::string io_dirname;

    bool dump_ir = false;
    bool dump_proto = false;
    bool verbose = false;
};

bool ParseArgs(tint::VectorRef<std::string_view> arguments, Options* opts) {
    using namespace tint::cli;  // NOLINT(build/namespaces)

    OptionSet options;
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
        "output-filename", "Output file name, only usable if single input file provided",
        ShortName{"o"}, Parameter{"name"});
    TINT_DEFER(opts->output_filename = output.value.value_or(""));

    auto& dump_ir = options.Add<BoolOption>("dump-ir", "Writes the IR form of input to stdout",
                                            Alias{"emit-ir"}, Default{false});
    TINT_DEFER(opts->dump_ir = *dump_ir.value);

    auto& dump_proto = options.Add<BoolOption>(
        "dump-proto", "Writes the IR in the test case proto as a human readable text to stdout",
        Alias{"emit-proto"}, Default{false});
    TINT_DEFER(opts->dump_proto = *dump_proto.value);

    auto& verbose =
        options.Add<BoolOption>("verbose", "Enable additional internal logging", Default{false});
    TINT_DEFER(opts->verbose = *verbose.value);

    auto& help = options.Add<BoolOption>("help", "Show usage", ShortName{"h"});

    auto show_usage = [&] {
        std::cout
            << R"(Usage: ir_fuzz_as [options] [-o|--output-filename] <output-file> <input-file> or tint [options] <io-dir>
If a single WGSL file is provided, the suffix of the input file is not checked, and
'-o|--output-filename' must be provided.

If a directory is provided, the files it contains will be scanned and any .wgsl files will have a
corresponding .tirb file generated.

Passing in '-o|--output-filename' when providing a directory will cause a failure.

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
        if (is_directory(std::filesystem::path{args[0]})) {
            opts->io_dirname = args[0];
        } else {
            opts->input_filename = args[0];
        }
    }

    return true;
}

/// Dumps IR representation for a program.
/// @param program the program to generate
/// @param options the options that ir_fuzz-as was invoked with
/// @returns true on success
bool DumpIR(const tint::Program& program, const Options& options) {
    auto result = tint::wgsl::reader::ProgramToLoweredIR(program);
    if (result != tint::Success) {
        std::cerr << "Failed to build IR from program: " << result.Failure() << "\n";
        return false;
    }

    options.printer->Print(tint::core::ir::Disassembler(result.Get()).Text());
    options.printer->Print(tint::StyledText{} << "\n");

    return true;
}

/// Generate an IR module for a program, performs checking for unsupported
/// enables, and validation.
/// @param program the program to generate
/// @returns generated module on success, tint::failure on failure
tint::Result<tint::core::ir::Module> GenerateIrModule(const tint::Program& program) {
    if (program.AST().Enables().Any(tint::wgsl::reader::IsUnsupportedByIR)) {
        return tint::Failure{"Unsupported enable used in shader"};
    }

    auto transformed = tint::wgsl::ApplySubstituteOverrides(program);
    auto& src = transformed ? transformed.value() : program;
    if (!src.IsValid()) {
        return tint::Failure{src.Diagnostics()};
    }

    auto ir = tint::wgsl::reader::ProgramToLoweredIR(src);
    if (ir != tint::Success) {
        return ir.Failure();
    }

    if (auto val = tint::core::ir::Validate(ir.Get()); val != tint::Success) {
        return val.Failure();
    }

    return ir;
}

/// @returns a fuzzer test case protobuf for the given program.
/// @param program the program to generate
tint::Result<tint::cmd::fuzz::ir::pb::Root> GenerateFuzzCaseProto(const tint::Program& program) {
    auto module = GenerateIrModule(program);
    if (module != tint::Success) {
        std::cerr << "Failed to generate lowered IR from program: " << module.Failure() << "\n";
        return tint::Failure();
    }

    tint::cmd::fuzz::ir::pb::Root fuzz_pb;
    {
        auto ir_pb = tint::core::ir::binary::EncodeToProto(module.Get());
        if (ir_pb != tint::Success) {
            std::cerr << " Failed to encode IR to proto: " << ir_pb.Failure() << "\n";
            return tint::Failure();
        }
        fuzz_pb.set_allocated_module(ir_pb.Get().release());
    }

    return std::move(fuzz_pb);
}

/// Write out fuzzer test case protobuf in binary format
/// @param proto test case proto to write out
/// @param options the options that ir_fuzz_as was invoked with
/// @returns true on success
bool WriteTestCaseProto(const tint::cmd::fuzz::ir::pb::Root& proto, const Options& options) {
    tint::Vector<std::byte, 0> buffer;
    size_t len = proto.ByteSizeLong();
    buffer.Resize(len);
    if (len > 0) {
        if (!proto.SerializeToArray(&buffer[0], static_cast<int>(len))) {
            std::cerr << "Failed to serialize test case protobuf";
            return false;
        }
    }

    if (!tint::cmd::WriteFile(options.output_filename, "wb", ToStdVector(buffer))) {
        std::cerr << "Failed to write protobuf binary out to file '" << options.output_filename
                  << "'\n";
        return false;
    }

    return true;
}

/// Dumps IR from test case proto in a human-readable format
/// @param proto test case proto to dump IR from
/// @param options the options that ir_fuzz_as was invoked with
void DumpTestCaseProtoDebug(const tint::cmd::fuzz::ir::pb::Root& proto, const Options& options) {
    options.printer->Print(tint::StyledText{} << proto.module().DebugString());
    options.printer->Print(tint::StyledText{} << "\n");
}

bool ProcessFile(const Options& options) {
    if (options.verbose) {
        options.printer->Print(tint::StyledText{} << "Processing '" << options.input_filename
                                                  << "'\n");
    }

    tint::cmd::LoadProgramOptions opts;
    opts.filename = options.input_filename;
    opts.printer = options.printer.get();

    auto info = tint::cmd::LoadProgramInfo(opts);

    if (options.dump_ir) {
        DumpIR(info.program, options);
    }

    auto proto = GenerateFuzzCaseProto(info.program);
    if (proto != tint::Success) {
        return false;
    }

    if (options.dump_proto) {
        DumpTestCaseProtoDebug(proto.Get(), options);
    }

    if (!options.output_filename.empty()) {
        if (!WriteTestCaseProto(proto.Get(), options)) {
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
    tint::SetInternalCompilerErrorReporter(&tint::cmd::TintInternalCompilerErrorReporter);

    if (!ParseArgs(arguments, &options)) {
        return EXIT_FAILURE;
    }

    if (!options.input_filename.empty() && !options.io_dirname.empty()) {
        std::cerr << "Somehow both input_filename '" << options.input_filename
                  << ", and io_dirname '" << options.io_dirname
                  << "' were set after parsing arguments\n";
        return EXIT_FAILURE;
    }

    if (options.output_filename.empty() && !options.dump_ir && !options.dump_proto &&
        options.io_dirname.empty()) {
        std::cerr << "None of --output-name, --dump-ir, --dump-proto, or <io-dir> were provided, "
                     "so no output would be generated...\n";
        return EXIT_FAILURE;
    }

    if (!options.input_filename.empty()) {
        if (!ProcessFile(options)) {
            return EXIT_FAILURE;
        }
    } else {
        tint::Vector<std::string, 8> wgsl_filenames;

        // Need to collect the WGSL filenames and then process them in a second phase, so that the
        // contents of the directory isn't changing during the iteration.
        for (auto const& io_entry :
             std::filesystem::directory_iterator{std::filesystem::path{options.io_dirname}}) {
            const std::string entry_filename = io_entry.path().string();
            if (entry_filename.substr(entry_filename.size() - 5) == ".wgsl") {
                wgsl_filenames.Push(std::move(entry_filename));
            }
        }

        for (auto const& input_filename : wgsl_filenames) {
            const auto output_filename =
                std::string(input_filename.substr(0, input_filename.size() - 5)) + ".tirb";
            options.input_filename = input_filename;
            options.output_filename = output_filename;

            ProcessFile(options);  // Ignoring the return value, so that one bad file doesn't cause
                                   // the processing batch to stop.
        }
    }

    return EXIT_SUCCESS;
}
