
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

#include "src/tint/utils/text/styled_text_printer.h"

#include "src/tint/cmd/common/helper.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/inspector/entry_point.h"
#include "src/tint/utils/command/args.h"
#include "src/tint/utils/text/string.h"

namespace {

struct Options {
    bool show_help = false;

#if TINT_BUILD_SPV_READER
    tint::spirv::reader::Options spirv_reader_options;
#endif

    std::string input_filename;
    bool emit_json = false;
};

const char kUsage[] = R"(Usage: tint [options] <input-file>

 options:
   --json                    -- Emit JSON
   -h                        -- This help text

)";

bool ParseArgs(tint::VectorRef<std::string_view> args, Options* opts) {
    for (auto arg : args) {
        if (arg == "-h" || arg == "--help") {
            opts->show_help = true;
        } else if (arg == "--json") {
            opts->emit_json = true;
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

void EmitJson(const tint::Program& program) {
    tint::inspector::Inspector inspector(program);

    std::cout << "{\n\"extensions\": [\n";

    if (!inspector.GetUsedExtensionNames().empty()) {
        bool first = true;
        for (const auto& name : inspector.GetUsedExtensionNames()) {
            if (!first) {
                std::cout << ",";
            }
            first = false;
            std::cout << "\"" << name << "\"\n";
        }
    }
    std::cout << "],\n";
    std::cout << "\"entry_points\": [";

    auto stage_var = [&](const tint::inspector::StageVariable& var) {
        std::cout << "\n{\n\"name\": \"" << var.name << "\",\n";
        if (auto location = var.attributes.location) {
            std::cout << "\"location\": " << location.value() << ",\n"
                      << "\"interpolation\": {\n"
                      << "\"type\": \""
                      << tint::cmd::InterpolationTypeToString(var.interpolation_type) << "\",\n"
                      << "\"sampling\": \""
                      << tint::cmd::InterpolationSamplingToString(var.interpolation_sampling)
                      << "\"\n"
                      << "},\n";
        }
        if (auto color = var.attributes.color) {
            std::cout << "\"color\": " << color.value() << ",\n";
        }
        std::cout << "\"component_type\": \""
                  << tint::cmd::ComponentTypeToString(var.component_type) << "\",\n"
                  << "\"composition_type\": \""
                  << tint::cmd::CompositionTypeToString(var.composition_type) << "\"\n\n"
                  << "}";
    };

    auto entry_points = inspector.GetEntryPoints();
    bool first = true;
    for (auto& entry_point : entry_points) {
        if (!first) {
            std::cout << ",";
        }
        first = false;

        std::cout << "\n{\n"
                  << "\"name\": \"" << entry_point.name << "\",\n"
                  << "\"stage\": \"" << tint::cmd::EntryPointStageToString(entry_point.stage)
                  << "\",\n";

        if (entry_point.workgroup_size) {
            std::cout << "\"workgroup_size\": [";
            std::cout << entry_point.workgroup_size->x << ", " << entry_point.workgroup_size->y
                      << ", " << entry_point.workgroup_size->z << "],\n";
        }

        std::cout << "\"input_variables\": [";
        bool input_first = true;
        for (const auto& var : entry_point.input_variables) {
            if (!input_first) {
                std::cout << ",";
            }
            input_first = false;
            stage_var(var);
        }
        std::cout << "\n],\n";
        std::cout << "\"output_variables\": [";
        bool output_first = true;
        for (const auto& var : entry_point.output_variables) {
            if (!output_first) {
                std::cout << ",";
            }
            output_first = false;
            stage_var(var);
        }
        std::cout << "\n],\n";
        std::cout << "\"overrides\": [";

        bool override_first = true;
        for (const auto& var : entry_point.overrides) {
            if (!override_first) {
                std::cout << ",";
            }
            override_first = false;

            std::cout << "\n{\n"
                      << "\"name\": \"" << var.name << "\",\n"
                      << "\"id\": " << var.id.value << ",\n"
                      << "\"type\": \"" << tint::cmd::OverrideTypeToString(var.type) << "\",\n"
                      << "\"is_initialized\": " << (var.is_initialized ? "true" : "false") << ",\n"
                      << "\"is_id_specified\": " << (var.is_id_specified ? "true" : "false")
                      << "\n}";
        }
        std::cout << "\n],\n";

        std::cout << "\"bindings\": [";
        auto bindings = inspector.GetResourceBindings(entry_point.name);
        bool ep_first = true;
        for (auto& binding : bindings) {
            if (!ep_first) {
                std::cout << ",";
            }
            ep_first = false;

            std::cout << "\n{\n"
                      << "\"binding\": " << binding.binding << ",\n"
                      << "\"group\": " << binding.bind_group << ",\n"
                      << "\"size\": " << binding.size << ",\n"
                      << "\"resource_type\": \""
                      << tint::cmd::ResourceTypeToString(binding.resource_type) << "\",\n"
                      << "\"dimemsions\": \"" << tint::cmd::TextureDimensionToString(binding.dim)
                      << "\",\n"
                      << "\"sampled_kind\": \""
                      << tint::cmd::SampledKindToString(binding.sampled_kind) << "\",\n"
                      << "\"image_format\": \""
                      << tint::cmd::TexelFormatToString(binding.image_format) << "\"\n"
                      << "}";
        }
        std::cout << "\n]\n";
        std::cout << "}";
    }
    std::cout << "\n],\n";
    std::cout << "\"structures\": [";

    bool struct_first = true;
    for (const auto* ty : program.Types()) {
        if (!ty->Is<tint::core::type::Struct>()) {
            continue;
        }
        const auto* s = ty->As<tint::core::type::Struct>();

        if (!struct_first) {
            std::cout << ",";
        }
        struct_first = false;

        std::cout << "\n{\n"
                  << "\"name\": \"" << s->FriendlyName() << "\",\n"
                  << "\"align\": " << s->Align() << ",\n"
                  << "\"size\": " << s->Size() << ",\n"
                  << "\"members\": [";
        for (size_t i = 0; i < s->Members().Length(); ++i) {
            auto* const m = s->Members()[i];

            if (i != 0) {
                std::cout << ",";
            }
            std::cout << "\n";

            // Output field alignment padding, if any
            auto* const prev_member = (i == 0) ? nullptr : s->Members()[i - 1];
            if (prev_member) {
                uint32_t padding = m->Offset() - (prev_member->Offset() + prev_member->Size());
                if (padding > 0) {
                    size_t padding_offset = m->Offset() - padding;
                    std::cout << "{\n"
                              << "\"name\": \"implicit_padding\",\n"
                              << "\"offset\": " << padding_offset << ",\n"
                              << "\"align\": 1,\n"
                              << "\"size\": " << padding << "\n},\n";
                }
            }

            std::cout << "{\n"
                      << "\"name\": \"" << m->Name().Name() << "\",\n"
                      << "\"offset\": " << m->Offset() << ",\n"
                      << "\"align\": " << m->Align() << ",\n"
                      << "\"size\": " << m->Size() << "\n}";
        }
        std::cout << "\n]\n}";
    }
    std::cout << "\n]\n}\n";
}

void EmitText(const tint::Program& program) {
    auto printer = tint::StyledTextPrinter::Create(stdout);
    tint::inspector::Inspector inspector(program);
    if (!inspector.GetUsedExtensionNames().empty()) {
        std::cout << "Extensions:\n";
        for (const auto& name : inspector.GetUsedExtensionNames()) {
            std::cout << "\t" << name << "\n";
        }
    }
    std::cout << "\n";

    tint::cmd::PrintInspectorData(inspector);

    bool has_struct = false;
    for (const auto* ty : program.Types()) {
        if (!ty->Is<tint::core::type::Struct>()) {
            continue;
        }
        has_struct = true;
        break;
    }

    if (has_struct) {
        std::cout << "Structures\n";
        for (const auto* ty : program.Types()) {
            if (!ty->Is<tint::core::type::Struct>()) {
                continue;
            }
            const auto* s = ty->As<tint::core::type::Struct>();
            printer->Print(s->Layout() << "\n\n");
        }
    }
}

}  // namespace

int main(int argc, const char** argv) {
    tint::Vector<std::string_view, 8> args = tint::args::Vectorize(argc, argv);
    Options options;

    if (!ParseArgs(args, &options)) {
        std::cerr << "Failed to parse arguments.\n";
        return 1;
    }

    if (options.show_help) {
        std::cout << kUsage << "\n";
        return 0;
    }

    tint::cmd::LoadProgramOptions opts;
    opts.filename = options.input_filename;
#if TINT_BUILD_SPV_READER
    opts.spirv_reader_options = options.spirv_reader_options;
#endif

    auto info = tint::cmd::LoadProgramInfo(opts);

    if (options.emit_json) {
        EmitJson(info.program);
    } else {
        EmitText(info.program);
    }

    return 0;
}
