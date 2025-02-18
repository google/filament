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

#include "src/tint/api/tint.h"
#include "src/tint/cmd/common/helper.h"
#include "src/tint/utils/command/args.h"

#if TINT_BUILD_GLSL_WRITER
#include "src/tint/lang/glsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/glsl/writer/writer.h"
#endif  // TINT_BUILD_GLSL_WRITER

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/hlsl/writer/writer.h"
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_MSL_WRITER
#include "src/tint/lang/msl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/msl/writer/writer.h"
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_SPV_READER
#include "src/tint/lang/spirv/reader/reader.h"
#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_SPV_WRITER
#include "src/tint/lang/spirv/writer/helpers/generate_bindings.h"
#include "src/tint/lang/spirv/writer/writer.h"
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#endif  // TINT_BUILD_WGSL_READER

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/helpers/flatten_bindings.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#endif  // TINT_BUILD_WGSL_WRITER

namespace {

enum class Format {
    kUnknown,
    kNone,
    kSpirv,
    kWgsl,
    kMsl,
    kHlsl,
    kGlsl,
};

enum class Looper {
    kLoad,
    kIRGenerate,
    kWriter,
};

struct Options {
    bool show_help = false;

    std::string input_filename;
    Format format = Format::kUnknown;

    Looper loop = Looper::kLoad;
    uint32_t loop_count = 100;
};

const char kUsage[] = R"(Usage: tint-loopy [options] <input-file>

 options:
  --format <spirv|wgsl|msl|hlsl|none>  -- Generation format. Default SPIR-V.
  --loop <load,ir-gen,writer>          -- Item to loop
  --loop-count <num>                   -- Number of loops to run, default 100.
)";

Format parse_format(const std::string_view fmt) {
    (void)fmt;

#if TINT_BUILD_SPV_WRITER
    if (fmt == "spirv") {
        return Format::kSpirv;
    }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
    if (fmt == "wgsl") {
        return Format::kWgsl;
    }
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
    if (fmt == "msl") {
        return Format::kMsl;
    }
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
    if (fmt == "hlsl") {
        return Format::kHlsl;
    }
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
    if (fmt == "glsl") {
        return Format::kGlsl;
    }
#endif  // TINT_BUILD_GLSL_WRITER

    if (fmt == "none") {
        return Format::kNone;
    }

    return Format::kUnknown;
}

bool ParseArgs(tint::VectorRef<std::string_view> args, Options* opts) {
    for (size_t i = 1; i < args.Length(); ++i) {
        auto arg = args[i];
        if (arg == "--format") {
            ++i;
            if (i >= args.Length()) {
                std::cerr << "Missing value for --format argument.\n";
                return false;
            }
            opts->format = parse_format(args[i]);

            if (opts->format == Format::kUnknown) {
                std::cerr << "Unknown output format: " << args[i] << "\n";
                return false;
            }
        } else if (arg == "-h" || arg == "--help") {
            opts->show_help = true;
        } else if (arg == "--loop") {
            ++i;
            if (i >= args.Length()) {
                std::cerr << "Missing value for --loop argument.\n";
                return false;
            }
            if (args[i] == "load") {
                opts->loop = Looper::kLoad;
            } else if (args[i] == "ir-gen") {
                opts->loop = Looper::kIRGenerate;
            } else if (args[i] == "writer") {
                opts->loop = Looper::kWriter;
            } else {
                std::cerr << "Invalid loop value\n";
                return false;
            }
        } else if (arg == "--loop-count") {
            ++i;
            if (i >= args.Length()) {
                std::cerr << "Missing value for --loop-count argument.\n";
                return false;
            }
            int32_t val = atoi(std::string(args[i]).c_str());
            if (val <= 0) {
                std::cerr << "Loop count must be greater then 0\n";
                return false;
            }
            opts->loop_count = static_cast<uint32_t>(val);
        } else if (!arg.empty()) {
            if (arg[0] == '-') {
                std::cerr << "Unrecognized option: " << arg << "\n";
                return false;
            }
            if (!opts->input_filename.empty()) {
                std::cerr << "More than one input file specified: '" << opts->input_filename
                          << "' and '" << arg << "'\n";
                return false;
            }
            opts->input_filename = arg;
        }
    }
    return true;
}

/// Generate SPIR-V code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateSpirv(const tint::Program& program) {
#if TINT_BUILD_SPV_WRITER
    // Convert the AST program to an IR module.
    auto ir = tint::wgsl::reader::ProgramToLoweredIR(program);
    if (ir != tint::Success) {
        std::cerr << "Failed to generate IR: " << ir << "\n";
        return false;
    }

    tint::spirv::writer::Options gen_options;
    gen_options.bindings = tint::spirv::writer::GenerateBindings(ir.Get());

    // Generate SPIR-V from Tint IR.
    auto result = tint::spirv::writer::Generate(ir.Get(), gen_options);
    if (result != tint::Success) {
        tint::cmd::PrintWGSL(std::cerr, program);
        std::cerr << "Failed to generate SPIR-V: " << result.Failure() << "\n";
        return false;
    }

    return true;
#else
    (void)program;
    std::cerr << "SPIR-V writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_SPV_WRITER
}

/// Generate WGSL code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateWgsl(const tint::Program& program) {
#if TINT_BUILD_WGSL_WRITER
    tint::wgsl::writer::Options gen_options;
    auto result = tint::wgsl::writer::Generate(program, gen_options);
    if (result != tint::Success) {
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    return true;
#else
    (void)program;
    std::cerr << "WGSL writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_WGSL_WRITER
}

/// Generate MSL code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateMsl([[maybe_unused]] const tint::Program& program) {
#if !TINT_BUILD_MSL_WRITER
    std::cerr << "MSL writer not enabled in tint build\n";
    return false;
#else
    // Remap resource numbers to a flat namespace.
    // TODO(crbug.com/tint/1501): Do this via Options::BindingMap.
    const tint::Program* input_program = &program;
    auto flattened = tint::wgsl::FlattenBindings(program);
    if (flattened) {
        input_program = &*flattened;
    }

    // Convert the AST program to an IR module.
    auto ir = tint::wgsl::reader::ProgramToLoweredIR(*input_program);
    if (ir != tint::Success) {
        std::cerr << "Failed to generate IR: " << ir << "\n";
        return false;
    }

    tint::msl::writer::Options gen_options;
    gen_options.bindings = tint::msl::writer::GenerateBindings(ir.Get());
    gen_options.array_length_from_uniform.ubo_binding = 30;
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 0},
                                                                          0);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 1},
                                                                          1);

    // Generate MSL from Tint IR.
    auto result = tint::msl::writer::Generate(ir.Get(), gen_options);
    if (result != tint::Success) {
        tint::cmd::PrintWGSL(std::cerr, program);
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    return true;
#endif
}

/// Generate HLSL code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateHlsl(const tint::Program& program) {
#if TINT_BUILD_HLSL_WRITER
    tint::hlsl::writer::Options gen_options;
    gen_options.bindings = tint::hlsl::writer::GenerateBindings(program);
    auto result = tint::hlsl::writer::Generate(program, gen_options);
    if (result != tint::Success) {
        tint::cmd::PrintWGSL(std::cerr, program);
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    return true;
#else
    (void)program;
    std::cerr << "HLSL writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_HLSL_WRITER
}

/// Generate GLSL code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateGlsl(const tint::Program& program) {
#if TINT_BUILD_GLSL_WRITER
    // Convert the AST program to an IR module.
    auto ir = tint::wgsl::reader::ProgramToLoweredIR(program);
    if (ir != tint::Success) {
        std::cerr << "Failed to generate IR: " << ir << "\n";
        return false;
    }

    tint::glsl::writer::Options gen_options;
    gen_options.bindings = tint::glsl::writer::GenerateBindings(ir.Get());

    auto result = tint::glsl::writer::Generate(ir.Get(), gen_options, "");
    if (result == tint::Success) {
        tint::cmd::PrintWGSL(std::cerr, program);
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    return true;

#else
    (void)program;
    std::cerr << "GLSL writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_GLSL_WRITER
}

}  // namespace

int main(int argc, const char** argv) {
    tint::Vector<std::string_view, 8> args = tint::args::Vectorize(argc, argv);
    Options options;

    tint::Initialize();
    tint::SetInternalCompilerErrorReporter(&tint::cmd::TintInternalCompilerErrorReporter);

    if (!ParseArgs(args, &options)) {
        std::cerr << "Failed to parse arguments.\n";
        return 1;
    }

    if (options.show_help) {
        std::cout << kUsage << "\n";
        return 0;
    }

    // Implement output format defaults.
    if (options.format == Format::kUnknown) {
        options.format = Format::kSpirv;
    }

    std::unique_ptr<tint::Program> program;
    std::unique_ptr<tint::Source::File> source_file;

    if (options.loop == Looper::kLoad) {
        if (options.input_filename.size() > 5 &&
            options.input_filename.substr(options.input_filename.size() - 5) == ".wgsl") {
#if TINT_BUILD_WGSL_READER
            std::vector<uint8_t> data;
            if (!tint::cmd::ReadFile<uint8_t>(options.input_filename, &data)) {
                exit(1);
            }
            source_file = std::make_unique<tint::Source::File>(
                options.input_filename, std::string(data.begin(), data.end()));

            uint32_t loop_count = options.loop_count;
            for (uint32_t i = 0; i < loop_count; ++i) {
                program =
                    std::make_unique<tint::Program>(tint::wgsl::reader::Parse(source_file.get()));
            }
#else
            std::cerr << "Tint not built with the WGSL reader enabled\n";
            exit(1);
#endif  // TINT_BUILD_WGSL_READER
        } else {
#if TINT_BUILD_SPV_READER
            std::vector<uint32_t> data;
            if (!tint::cmd::ReadFile<uint32_t>(options.input_filename, &data)) {
                exit(1);
            }

            uint32_t loop_count = options.loop_count;
            for (uint32_t i = 0; i < loop_count; ++i) {
                program = std::make_unique<tint::Program>(tint::spirv::reader::Read(data, {}));
            }
#else
            std::cerr << "Tint not built with the SPIR-V reader enabled\n";
            exit(1);
#endif  // TINT_BUILD_SPV_READER
        }
    }

    // Load the program that will actually be used
    tint::cmd::LoadProgramOptions opts;
    opts.filename = options.input_filename;

    auto info = tint::cmd::LoadProgramInfo(opts);
#if TINT_BUILD_WGSL_READER
    {
        uint32_t loop_count = 1;
        if (options.loop == Looper::kIRGenerate) {
            loop_count = options.loop_count;
        }
        for (uint32_t i = 0; i < loop_count; ++i) {
            auto result = tint::wgsl::reader::ProgramToIR(info.program);
            if (result != tint::Success) {
                std::cerr << "Failed to build IR from program: " << result.Failure() << "\n";
            }
        }
    }
#endif  // TINT_BUILD_WGSL_READER

    bool success = false;
    {
        uint32_t loop_count = 1;
        if (options.loop == Looper::kWriter) {
            loop_count = options.loop_count;
        }

        switch (options.format) {
            case Format::kSpirv:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateSpirv(info.program);
                }
                break;
            case Format::kWgsl:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateWgsl(info.program);
                }
                break;
            case Format::kMsl:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateMsl(info.program);
                }
                break;
            case Format::kHlsl:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateHlsl(info.program);
                }
                break;
            case Format::kGlsl:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateGlsl(info.program);
                }
                break;
            case Format::kNone:
                break;
            default:
                std::cerr << "Unknown output format specified\n";
                return 1;
        }
    }
    if (success) {
        return 1;
    }

    return 0;
}
