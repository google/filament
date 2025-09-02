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
#include "src/tint/cmd/fuzz/ir/helpers/substitute_overrides_config.h"
#include "src/tint/lang/core/ir/binary/encode.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/wgsl/ast/module.h"
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
    std::string input_dirname;
    std::string output_dirname;

    bool dump_ir = false;
    bool dump_proto = false;
    bool verbose = false;
    bool batch_mode = false;
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
            << R"(Usage: ir_fuzz_as [options] '<input-file> <output-file>' or '<input-dir> <output-dir>'
The first form takes in a single WGSL shader via <input-file> and produces the corresponding IR test
binary in <output-file>.

In the second form, the input and output arguments are directories. The files it contained in
<input-dir> will be scanned and any .wgsl files will have acorresponding .tirb file generated in
<output-dir>.

If you are wanting to generate human readable IR from a WGSL file, either --emit-ir should be added
to options, or the tint CLI used

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
    if (args.Length() != 2) {
        std::cerr << "Expected exactly 2 args, found: "
                  << tint::Join(Transform(args, tint::Quote), ", ") << "\n";
        return false;
    }

    bool is_arg0_dir = is_directory(std::filesystem::path{args[0]});
    bool is_arg1_dir = is_directory(std::filesystem::path{args[1]});

    if (is_arg0_dir && is_arg1_dir) {
        opts->input_dirname = args[0];
        opts->output_dirname = args[1];
        opts->batch_mode = true;
    } else if (!is_arg0_dir && !is_arg1_dir) {
        opts->input_filename = args[0];
        opts->output_filename = args[1];
        opts->batch_mode = false;
    } else {
        std::cerr << "Expected args to either both be directories or both be files ('" << args[0]
                  << "' is " << (is_arg0_dir ? "DIR" : "FILE") << " and '" << args[1] << "' is "
                  << (is_arg1_dir ? "DIR" : "FILE") << "\n";
        return false;
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

    auto ir = tint::wgsl::reader::ProgramToLoweredIR(program);
    if (ir != tint::Success) {
        return ir.Failure();
    }

    auto cfg = tint::fuzz::ir::SubstituteOverridesConfig(ir.Get());
    auto substituteOverridesResult = tint::core::ir::transform::SubstituteOverrides(ir.Get(), cfg);
    if (substituteOverridesResult != tint::Success) {
        return substituteOverridesResult.Failure();
    }

    if (auto val = tint::core::ir::Validate(ir.Get()); val != tint::Success) {
        return val.Failure();
    }

    return ir.Move();
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

    if (!ParseArgs(arguments, &options)) {
        return EXIT_FAILURE;
    }

    if (!options.batch_mode) {
        if (!ProcessFile(options)) {
            return EXIT_FAILURE;
        }
    } else {
        for (auto const& input_entry :
             std::filesystem::directory_iterator{std::filesystem::path{options.input_dirname}}) {
            const std::string input_path = input_entry.path().string();
            if (input_path.substr(input_path.size() - 5) != ".wgsl") {
                continue;
            }

            options.input_filename = input_entry.path().string();

            auto output_entry = std::filesystem::path{options.output_dirname} /
                                input_entry.path().filename().replace_extension(".tirb");
            options.output_filename = output_entry.string();

            ProcessFile(options);  // Ignoring the return value, so that one bad file doesn't cause
                                   // the processing batch to stop.
        }
    }

    return EXIT_SUCCESS;
}
