// Copyright 2020 The Dawn & Tint Authors
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

#include <charconv>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/command/args.h"
#include "src/tint/utils/text/color_mode.h"

#if TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER

#include "src/tint/api/tint.h"
#include "src/tint/cmd/common/helper.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/transform/single_entry_point.h"
#include "src/tint/lang/core/ir/transform/substitute_overrides.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/msl/ir/transform/flatten_bindings.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/utils/command/cli.h"
#include "src/tint/utils/command/command.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/diagnostic/formatter.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#endif  // TINT_BUILD_WGSL_READER

#if TINT_BUILD_SPV_WRITER
#include "src/tint/lang/spirv/writer/helpers/generate_bindings.h"
#include "src/tint/lang/spirv/writer/writer.h"
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/writer/writer.h"
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
#include "src/tint/lang/msl/validate/validate.h"
#include "src/tint/lang/msl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/msl/writer/writer.h"
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/validate/validate.h"
#include "src/tint/lang/hlsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/hlsl/writer/writer.h"
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
#include "src/tint/lang/glsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/glsl/writer/writer.h"
#endif  // TINT_BUILD_GLSL_WRITER

#if TINT_BUILD_GLSL_VALIDATOR
#include "src/tint/lang/glsl/validate/validate.h"
#endif  // TINT_BUILD_GLSL_VALIDATOR

namespace {

/// Prints the given hash value in a format string that the end-to-end test runner can parse.
[[maybe_unused]] void PrintHash(uint32_t hash) {
    std::cout << "<<HASH: 0x" << std::hex << hash << ">>\n";
}

enum class Format : uint8_t {
    kUnknown,
    kNone,
    kSpirv,
    kSpvAsm,
    kWgsl,
    kMsl,
    kHlsl,
    kHlslFxc,
    kGlsl,
    kIr,
};

enum class ExeMode : uint8_t {
    kStandalone,
    kServer,
};

#if TINT_BUILD_HLSL_WRITER
constexpr uint32_t kMinShaderModelForDXC = 60u;
constexpr uint32_t kMaxSupportedShaderModelForDXC = 66u;
constexpr uint32_t kMinShaderModelForDP4aInHLSL = 64u;
constexpr uint32_t kMinShaderModelForPackUnpack4x8InHLSL = 66u;
#endif  // TINT_BUILD_HLSL_WRITER

struct Options {
    std::unique_ptr<tint::StyledTextPrinter> printer;

    std::string input_filename;
    std::string output_file = "-";  // Default to stdout

    std::unordered_set<uint32_t> skip_hash;
    tint::Hashmap<std::string, double, 8> overrides;

    std::string ep_name;
    bool emit_single_entry_point = false;

    Format format = Format::kUnknown;

    bool verbose = false;
    bool parse_only = false;
    bool disable_workgroup_init = false;
    bool disable_demote_to_helper = false;
    bool validate = false;
    bool print_hash = false;
    bool dump_inspector_bindings = false;

    bool rename_all = false;
    bool enable_robustness = true;

    bool dump_ir = false;
    bool use_ir_reader = false;

#if TINT_BUILD_SPV_READER
    tint::spirv::reader::Options spirv_reader_options;
#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_SPV_WRITER
    bool use_storage_input_output_16 = true;
    tint::spirv::writer::SpvVersion spirv_version = tint::spirv::writer::SpvVersion::kSpv13;
#endif  // TINT_BULD_SPV_WRITER

#if TINT_BUILD_MSL_WRITER
    std::string xcrun_path;

    bool use_argument_buffers = false;
    std::unordered_map<uint32_t, tint::msl::writer::ArgumentBufferInfo>
        group_to_argument_buffer_info;

    std::unordered_map<uint32_t, uint32_t> pixel_local_attachments;
    tint::msl::validate::MslVersion msl_version = tint::msl::validate::MslVersion::kMsl_2_3;
#endif

#if TINT_BUILD_HLSL_WRITER
    std::string fxc_path;
    std::string dxc_path;
    uint32_t hlsl_shader_model = kMinShaderModelForDXC;
    tint::hlsl::writer::PixelLocalOptions pixel_local_options;
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
    bool glsl_desktop = false;
    std::vector<uint32_t> bgra_swizzle;
#endif  // TINT_BUILD_GLSL_WRITER
};

/// @param filename the filename to inspect
/// @returns the inferred format for the filename suffix
Format InferFormat(const std::string& filename) {
    (void)filename;

#if TINT_BUILD_SPV_WRITER
    if (tint::HasSuffix(filename, ".spv")) {
        return Format::kSpirv;
    }
    if (tint::HasSuffix(filename, ".spvasm")) {
        return Format::kSpvAsm;
    }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
    if (tint::HasSuffix(filename, ".wgsl")) {
        return Format::kWgsl;
    }
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
    if (tint::HasSuffix(filename, ".metal")) {
        return Format::kMsl;
    }
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
    if (tint::HasSuffix(filename, ".hlsl")) {
        return Format::kHlsl;
    }
#endif  // TINT_BUILD_HLSL_WRITER

    return Format::kUnknown;
}

// The actual warning occurs on `std::from_chars(hash.data(), hash.data() + hash.size(), value,
// base);`, but disabling/enabling warnings cannot be done within function scope
TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
bool ParseArgs(tint::VectorRef<std::string_view> arguments, Options* opts, ExeMode exe_mode) {
    using namespace tint::cli;  // NOLINT(build/namespaces)

    tint::Vector<EnumName<Format>, 8> format_enum_names{
        EnumName(Format::kNone, "none"),
    };

#if TINT_BUILD_WGSL_WRITER
    format_enum_names.Emplace(Format::kWgsl, "wgsl");
#endif

#if TINT_BUILD_WGSL_READER
    format_enum_names.Emplace(Format::kIr, "ir");
#endif

#if TINT_BUILD_SPV_WRITER
    format_enum_names.Emplace(Format::kSpirv, "spirv");
    format_enum_names.Emplace(Format::kSpvAsm, "spvasm");
#endif

#if TINT_BUILD_MSL_WRITER
    format_enum_names.Emplace(Format::kMsl, "msl");
#endif

#if TINT_BUILD_HLSL_WRITER
    format_enum_names.Emplace(Format::kHlsl, "hlsl");
    format_enum_names.Emplace(Format::kHlslFxc, "hlsl-fxc");
#endif

#if TINT_BUILD_GLSL_WRITER
    format_enum_names.Emplace(Format::kGlsl, "glsl");
#endif

    OptionSet options;
    auto& fmt = options.Add<EnumOption<Format>>("format",
                                                R"(Output format.
If not provided, will be inferred from output filename extension:
  .spvasm -> spvasm
  .spv    -> spirv
  .wgsl   -> wgsl
  .metal  -> msl
  .hlsl   -> hlsl)",
                                                format_enum_names, ShortName{"f"});
    TINT_DEFER(opts->format = fmt.value.value_or(Format::kUnknown));

    const auto default_color_mode =
        exe_mode == ExeMode::kServer ? tint::ColorMode::kPlain : tint::ColorModeDefault();
    auto& col =
        options.Add<EnumOption<tint::ColorMode>>("color", "Use colored output",
                                                 tint::Vector{
                                                     EnumName{tint::ColorMode::kPlain, "off"},
                                                     EnumName{tint::ColorMode::kDark, "dark"},
                                                     EnumName{tint::ColorMode::kLight, "light"},
                                                 },
                                                 ShortName{"col"}, Default{default_color_mode});
    TINT_DEFER(opts->printer = CreatePrinter(*col.value));

    auto& ep = options.Add<StringOption>("entry-point", "Output single entry point",
                                         ShortName{"ep"}, Parameter{"name"});
    TINT_DEFER({
        if (ep.value.has_value()) {
            opts->ep_name = *ep.value;
            opts->emit_single_entry_point = true;
        }
    });

    auto& output = options.Add<StringOption>("output-name", "Output file name", ShortName{"o"},
                                             Parameter{"name"});
    TINT_DEFER(opts->output_file = output.value.value_or(""));

    auto& use_ir_reader = options.Add<BoolOption>(
        "use-ir-reader", "Use the IR for the SPIR-V reader", Default{false});
    TINT_DEFER(opts->use_ir_reader = *use_ir_reader.value);

    auto& disable_wg_init = options.Add<BoolOption>(
        "disable-workgroup-init", "Disable workgroup memory zero initialization", Default{false});
    TINT_DEFER(opts->disable_workgroup_init = *disable_wg_init.value);

    auto& disable_demote_to_helper = options.Add<BoolOption>(
        "disable-demote-to-helper", "Disable demote to helper for discard", Default{false});
    TINT_DEFER(opts->disable_demote_to_helper = *disable_demote_to_helper.value);

    auto& disable_robustness =
        options.Add<BoolOption>("disable-robustness", "Disable the robustness transform");
    TINT_DEFER({
        auto disable = disable_robustness.value.value_or(false);
        opts->enable_robustness = !disable;
    });

    auto& rename_all = options.Add<BoolOption>("rename-all", "Renames all symbols", Default{false});
    TINT_DEFER(opts->rename_all = *rename_all.value);

    auto& overrides = options.Add<StringOption>(
        "overrides", "Override values as IDENTIFIER=VALUE, comma-separated");

#if TINT_BUILD_HLSL_WRITER
    auto& pixel_local_attachment_formats =
        options.Add<StringOption>("pixel-local-attachment-formats",
                                  R"(Pixel local storage attachment formats, comma-separated
Each binding is of the form MEMBER_INDEX=ATTACHMENT_FORMAT,
where MEMBER_INDEX is the pixel-local structure member
index and ATTACHMENT_FORMAT is the format of the emitted
attachment, which can only be one of the below value:
R32Sint, R32Uint, R32Float.
)");

    std::stringstream hlslShaderModelStream;
    hlslShaderModelStream << R"(
An integer value to set the HLSL shader model for the generated HLSL
shader, which will only be used with option `--dxc`. Now only integers
in the range [)" << kMinShaderModelForDXC
                          << ", " << kMaxSupportedShaderModelForDXC
                          << "] are accepted. The integer \"6x\" represents shader model 6.x.";
    auto& hlsl_shader_model = options.Add<ValueOption<uint32_t>>(
        "hlsl-shader-model", hlslShaderModelStream.str(), Default{kMinShaderModelForDXC});
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_HLSL_WRITER || TINT_BUILD_MSL_WRITER
    auto& pixel_local_attachments =
        options.Add<StringOption>("pixel-local-attachments",
                                  R"(Pixel local storage attachment bindings, comma-separated
Each binding is of the form MEMBER_INDEX=ATTACHMENT_INDEX,
where MEMBER_INDEX is the pixel-local structure member
index and ATTACHMENT_INDEX is the index of the emitted
attachment.
)");
#endif

    auto& print_hash = options.Add<BoolOption>("print-hash", "Emit the hash of the output program",
                                               Default{false});
    TINT_DEFER(opts->print_hash = *print_hash.value);

    auto& skip_hash = options.Add<StringOption>(
        "skip-hash", R"(Skips validation if the hash of the output is equal to any
of the hash codes in the comma separated list of hashes)");
    TINT_DEFER({
        if (skip_hash.value.has_value()) {
            for (auto hash : tint::Split(*skip_hash.value, ",")) {
                uint32_t value = 0;
                int base = 10;
                if (hash.size() > 2 && hash[0] == '0' && (hash[1] == 'x' || hash[1] == 'X')) {
                    hash = hash.substr(2);
                    base = 16;
                }

                std::from_chars(hash.data(), hash.data() + hash.size(), value, base);
                opts->skip_hash.emplace(value);
            }
        }
    });

#if TINT_BUILD_SPV_READER
    auto& allow_nud =
        options.Add<BoolOption>("allow-non-uniform-derivatives",
                                R"(When using SPIR-V input, allow non-uniform derivatives by
inserting a module-scope directive to suppress any uniformity
violations that may be produced)",
                                Default{false});
    TINT_DEFER({
        if (allow_nud.value.value_or(false)) {
            opts->spirv_reader_options.allow_non_uniform_derivatives = true;
        }
    });

    auto& sampler_mapping = options.Add<StringOption>(
        "sampler-mapping",
        "Allows remapping the binding points of samplers from the SPIR-V file. "
        "This allows setting a correct binding point for samplers which were part of a combined "
        "texture/sampler pair. Entries are provided as binding point pairs (group, binding) and "
        "each provides a (source:destination) mapping. (e.g. 1,2:3,4) Multiple entries should "
        "be separated with a space.",
        Default{""});
#endif

#if TINT_BUILD_SPV_WRITER
    auto& use_storage_input_output_16 =
        options.Add<BoolOption>("use-storage-input-output-16",
                                "Use the StorageInputOutput16 SPIR-V capability", Default{true});
    TINT_DEFER(opts->use_storage_input_output_16 = *use_storage_input_output_16.value);

    tint::Vector<EnumName<tint::spirv::writer::SpvVersion>, 2> version_enum_names{
        EnumName(tint::spirv::writer::SpvVersion::kSpv13, "1.3"),
        EnumName(tint::spirv::writer::SpvVersion::kSpv14, "1.4"),
    };
    auto& spirv_version = options.Add<EnumOption<tint::spirv::writer::SpvVersion>>(
        "spirv-version", R"(Specify the SPIR-V binary version.
Valid values are 1.3 and 1.4)",
        version_enum_names, Default{tint::spirv::writer::SpvVersion::kSpv13});
    TINT_DEFER(opts->spirv_version = *spirv_version.value);
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_GLSL_WRITER
    auto& glsl_desktop = options.Add<BoolOption>(
        "glsl-desktop", "Set the version to the desktop GL instead of ES", Default{false});
    TINT_DEFER(opts->glsl_desktop = *glsl_desktop.value);

    auto& bgra_swizzle =
        options.Add<StringOption>("bgra-swizzle", "BGRA swizzle indices", Default{""});
#endif  // TINT_BUILD_GLSL_WRITER

#if TINT_BUILD_MSL_WRITER
    auto& xcrun =
        options.Add<StringOption>("xcrun", R"(Path to xcrun executable, used to validate MSL output.
When specified, automatically enables MSL validation)",
                                  Parameter{"path"});
    TINT_DEFER({
        if (xcrun.value.has_value()) {
            opts->xcrun_path = *xcrun.value;
            opts->validate = true;
        }
    });

    auto& use_argument_buffers = options.Add<BoolOption>(
        "use-argument-buffers", "Use the Argument Buffers in MSL", Default{false});
    TINT_DEFER(opts->use_argument_buffers = *use_argument_buffers.value);

    auto& arg_buffer = options.Add<StringOption>(
        "argument-buffer",
        R"(Mapping for an argument buffer, format is GROUP=ARGUMENT_BUFFER_ID, comma separated)");

    auto& dynamic_buffer = options.Add<StringOption>(
        "dynamic-offset-buffer",
        R"(Mapping for a dynamic offset buffer, format is GROUP=DYNAMIC_BUFFER_ID, comma separated))");

    auto& dynamic_offset = options.Add<StringOption>(
        "dynamic-offset",
        R"(Mapping for dynamic buffers to be attached to the entry point, format is GROUP.BINDING=OFFSET, comma separated))");

    // Default to validating against MSL 2.3, which corresponds to macOS 11.0.
    tint::Vector<EnumName<tint::msl::validate::MslVersion>, 2> msl_version_enum_names{
        EnumName(tint::msl::validate::MslVersion::kMsl_2_3, "2.3"),
        EnumName(tint::msl::validate::MslVersion::kMsl_3_2, "3.2"),
    };
    auto& msl_version = options.Add<EnumOption<tint::msl::validate::MslVersion>>(
        "msl-version", R"(Specify the MSL version.
Valid values are 2.3 and 3.2)",
        msl_version_enum_names, Default{tint::msl::validate::MslVersion::kMsl_2_3});
    TINT_DEFER(opts->msl_version = *msl_version.value);
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
    auto& fxc_path =
        options.Add<StringOption>("fxc", R"(Path to FXC dll, used to validate HLSL output.
When specified, automatically enables HLSL validation)",
                                  Parameter{"path"});
    TINT_DEFER(opts->fxc_path = fxc_path.value.value_or(""));

    auto& dxc_path =
        options.Add<StringOption>("dxc", R"(Path to DXC dll, used to validate HLSL output.
When specified, automatically enables HLSL validation)",
                                  Parameter{"path"});
    TINT_DEFER(opts->dxc_path = dxc_path.value.value_or(""));
#endif  // TINT_BUILD_HLSL_WRITER

    auto& validate = options.Add<BoolOption>(
        "validate", "Validates the generated shader with all available validators", Default{false});
    TINT_DEFER(opts->validate = *validate.value);

    auto& dump_ir = options.Add<BoolOption>("dump-ir", "Writes the IR to stdout", Alias{"emit-ir"},
                                            Default{false});
    TINT_DEFER(opts->dump_ir = *dump_ir.value);

    auto& dump_inspector_bindings = options.Add<BoolOption>(
        "dump-inspector-bindings", "Dump reflection data about bindings to stdout",
        Alias{"emit-inspector-bindings"}, Default{false});
    TINT_DEFER(opts->dump_inspector_bindings = *dump_inspector_bindings.value);

    auto& parse_only =
        options.Add<BoolOption>("parse-only", "Stop after parsing the input", Default{false});
    TINT_DEFER(opts->parse_only = *parse_only.value);

    auto& verbose =
        options.Add<BoolOption>("verbose", "Verbose output", ShortName{"v"}, Default{false});
    TINT_DEFER(opts->verbose = *verbose.value);

    auto& help = options.Add<BoolOption>("help", "Show usage", ShortName{"h"});

    auto show_usage = [&] {
        std::cout << R"(Usage: tint [options] <input-file>

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

    if (overrides.value.has_value()) {
        for (const auto& o : tint::Split(*overrides.value, ",")) {
            auto parts = tint::Split(o, "=");
            if (parts.Length() != 2) {
                std::cerr << "override values must be of the form IDENTIFIER=VALUE";
                return false;
            }
            auto value = tint::strconv::ParseNumber<double>(parts[1]);
            if (value != tint::Success) {
                std::cerr << "invalid override value: " << parts[1];
                return false;
            }
            opts->overrides.Add(std::string(parts[0]), value.Get());
        }
    }

#if TINT_BUILD_SPV_READER
    if (!sampler_mapping.value->empty()) {
        auto str_to_bp = [](const std::string_view& str) -> std::optional<tint::BindingPoint> {
            auto parts = tint::Split(str, ",");
            if (parts.Length() != 2) {
                std::cerr << "A binding point requires a 'group,binding' pair, found "
                          << parts.Length() << " components instead of 2.\n";
                return std::nullopt;
            }

            uint32_t group = 0;
            std::from_chars(parts[0].data(), parts[0].data() + parts[0].size(), group);

            uint32_t binding = 0;
            std::from_chars(parts[1].data(), parts[1].data() + parts[0].size(), binding);

            return {tint::BindingPoint{group, binding}};
        };

        for (auto mapping : tint::Split(*sampler_mapping.value, " ")) {
            auto parts = tint::Split(mapping, ":");
            if (parts.Length() != 2) {
                std::cerr << "Expected source and destination binding points separated by a ':'\n";
                return false;
            }

            auto opt_src = str_to_bp(parts[0]);
            if (!opt_src.has_value()) {
                return false;
            }
            tint::BindingPoint src_bp = opt_src.value();

            auto opt_dst = str_to_bp(parts[1]);
            if (!opt_dst.has_value()) {
                return false;
            }
            tint::BindingPoint dst_bp = opt_dst.value();

            opts->spirv_reader_options.sampler_mappings.insert({src_bp, dst_bp});
        }
    }
#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_MSL_WRITER
    if (arg_buffer.value.has_value()) {
        for (auto ab : tint::Split(*arg_buffer.value, ",")) {
            auto parts = tint::Split(ab, "=");
            if (parts.Length() != 2) {
                std::cerr << "argument-buffer values must be of the form GROUP=ARGUMENT_BUFFER_ID";
                return false;
            }

            uint32_t group = 0;
            std::from_chars(parts[0].data(), parts[0].data() + parts[0].size(), group, 10);

            uint32_t idx = 0;
            std::from_chars(parts[1].data(), parts[1].data() + parts[1].size(), idx, 10);

            if (!opts->group_to_argument_buffer_info.contains(group)) {
                opts->group_to_argument_buffer_info.insert({group, {}});
            }
            opts->group_to_argument_buffer_info[group].id = idx;
        }
    }

    if (dynamic_buffer.value.has_value()) {
        for (auto db : tint::Split(*dynamic_buffer.value, ",")) {
            auto parts = tint::Split(db, "=");
            if (parts.Length() != 2) {
                std::cerr
                    << "dynamic-offset-buffer values must be of the form GROUP=DYNAMIC_BUFFER_ID";
                return false;
            }

            uint32_t group = 0;
            std::from_chars(parts[0].data(), parts[0].data() + parts[0].size(), group, 10);

            uint32_t idx = 0;
            std::from_chars(parts[1].data(), parts[1].data() + parts[1].size(), idx, 10);

            if (!opts->group_to_argument_buffer_info.contains(group)) {
                opts->group_to_argument_buffer_info.insert({group, {}});
            }
            opts->group_to_argument_buffer_info[group].dynamic_buffer_id = idx;
        }
    }

    if (dynamic_offset.value.has_value()) {
        for (auto val : tint::Split(*dynamic_offset.value, ",")) {
            auto parts = tint::Split(val, "=");
            if (parts.Length() != 2) {
                std::cerr << "dynamic-offset values must be of the form GROUP.BINDING=OFFSET";
                return false;
            }

            auto bind_point = tint::Split(parts[0], ".");
            if (bind_point.Length() != 2) {
                std::cerr << "dynamic-offset values must be of the form GROUP.BINDING=OFFSET";
                return false;
            }
            uint32_t group = 0;
            std::from_chars(bind_point[0].data(), bind_point[0].data() + bind_point[0].size(),
                            group, 10);
            uint32_t binding = 0;
            std::from_chars(bind_point[0].data(), bind_point[0].data() + bind_point[0].size(),
                            binding, 10);

            uint32_t offset = 0;
            std::from_chars(parts[1].data(), parts[1].data() + parts[1].size(), offset, 10);

            if (!opts->group_to_argument_buffer_info.contains(group)) {
                opts->group_to_argument_buffer_info.insert({group, {}});
            }
            opts->group_to_argument_buffer_info[group].binding_info_to_offset_index.insert(
                {binding, offset});
        }
    }
#endif

#if TINT_BUILD_HLSL_WRITER
    if (pixel_local_attachment_formats.value.has_value()) {
        auto binding_formats = tint::Split(*pixel_local_attachment_formats.value, ",");
        for (auto& binding_format : binding_formats) {
            auto values = tint::Split(binding_format, "=");
            if (values.Length() != 2) {
                std::cerr << "Invalid binding format " << pixel_local_attachment_formats.name
                          << ": " << binding_format << "\n";
                return false;
            }
            auto member_index = tint::strconv::ParseUint32(values[0]);
            if (member_index != tint::Success) {
                std::cerr << "Invalid member index for " << pixel_local_attachment_formats.name
                          << ": " << values[0] << "\n";
                return false;
            }
            auto format = values[1];
            tint::hlsl::writer::PixelLocalAttachment::TexelFormat texel_format =
                tint::hlsl::writer::PixelLocalAttachment::TexelFormat::kUndefined;
            if (format == "R32Sint") {
                texel_format = tint::hlsl::writer::PixelLocalAttachment::TexelFormat::kR32Sint;
            } else if (format == "R32Uint") {
                texel_format = tint::hlsl::writer::PixelLocalAttachment::TexelFormat::kR32Uint;
            } else if (format == "R32Float") {
                texel_format = tint::hlsl::writer::PixelLocalAttachment::TexelFormat::kR32Float;
            } else {
                std::cerr << "Invalid texel format for " << pixel_local_attachments.name << ": "
                          << format << "\n";
                return false;
            }
            // Add an entry with just the texel format for now. We will fill in the attachment index
            // below.
            tint::hlsl::writer::PixelLocalAttachment attachment{~0u, texel_format};
            opts->pixel_local_options.attachments.emplace(member_index.Get(), attachment);
        }
    }

    if (hlsl_shader_model.value.has_value()) {
        const uint32_t shader_model = *hlsl_shader_model.value;
        if (shader_model < kMinShaderModelForDXC || shader_model > kMaxSupportedShaderModelForDXC) {
            std::cerr << "Invalid HLSL shader model: " << shader_model << "\n";
            return false;
        }
        opts->hlsl_shader_model = shader_model;
    }
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_HLSL_WRITER || TINT_BUILD_MSL_WRITER
    if (pixel_local_attachments.value.has_value()) {
        auto bindings = tint::Split(*pixel_local_attachments.value, ",");
        for (auto& binding : bindings) {
            auto values = tint::Split(binding, "=");
            if (values.Length() != 2) {
                std::cerr << "Invalid binding " << pixel_local_attachments.name << ": " << binding
                          << "\n";
                return false;
            }
            auto member_index = tint::strconv::ParseUint32(values[0]);
            if (member_index != tint::Success) {
                std::cerr << "Invalid member index for " << pixel_local_attachments.name << ": "
                          << values[0] << "\n";
                return false;
            }
            auto attachment_index = tint::strconv::ParseUint32(values[1]);
            if (attachment_index != tint::Success) {
                std::cerr << "Invalid attachment index for " << pixel_local_attachments.name << ": "
                          << values[1] << "\n";
                return false;
            }
#if TINT_BUILD_MSL_WRITER
            opts->pixel_local_attachments.emplace(member_index.Get(), attachment_index.Get());
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
            // We've already set the format for this member index, now set the attachment index
            auto iter = opts->pixel_local_options.attachments.find(member_index.Get());
            if (iter == opts->pixel_local_options.attachments.end()) {
                std::cerr << "Missing pixel local format for member index: " << member_index.Get()
                          << "\n";
                return false;
            }
            iter->second.index = attachment_index.Get();
#endif  // TINT_BUILD_HLSL_WRITER
        }
    }
#endif  // TINT_BUILD_HLSL_WRITER || TINT_BUILD_MSL_WRITER

#if TINT_BUILD_GLSL_WRITER
    if (bgra_swizzle.value.has_value() && !bgra_swizzle.value.value().empty()) {
        for (auto val : tint::Split(*bgra_swizzle.value, ",")) {
            auto bgra_val = tint::strconv::ParseUint32(val);
            if (bgra_val != tint::Success) {
                std::cerr << "invalid bgra_swizzle value: " << val;
                return false;
            }
            opts->bgra_swizzle.push_back(bgra_val.Get());
        }
    }
#endif  // TINT_BUILD_GLSL_WRITER

    auto files = result.Get();
    if (files.Length() > 1) {
        std::cerr << "More than one input file specified: "
                  << tint::Join(Transform(files, tint::Quote), ", ") << "\n";
        return false;
    }
    if (files.Length() == 1) {
        opts->input_filename = files[0];
    }

    return true;
}
TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

[[maybe_unused]] tint::diag::Result<tint::core::ir::transform::SubstituteOverridesConfig>
CreateOverrideMap(const Options& options, tint::inspector::Inspector& inspector) {
    auto override_names = inspector.GetNamedOverrideIds();

    tint::core::ir::transform::SubstituteOverridesConfig cfg;
    cfg.map.reserve(options.overrides.Count());
    for (auto& override : options.overrides) {
        const auto& override_name = override.key.Value();
        const auto& override_value = override.value;
        if (override_name.empty()) {
            return tint::diag::Failure("empty override name");
        }

        auto num = tint::strconv::ParseNumber<decltype(tint::OverrideId::value)>(override_name);
        if (num == tint::Success) {
            tint::OverrideId id{num.Get()};
            cfg.map.emplace(id, override_value);
            continue;
        }

        auto it = override_names.find(override_name);
        if (it == override_names.end()) {
            return tint::diag::Failure("unknown override '" + override_name + "'");
        }
        cfg.map.emplace(it->second, override_value);
    }
    return cfg;
}

#if TINT_BUILD_SPV_WRITER
std::string Disassemble(const std::vector<uint32_t>& data) {
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
        std::cerr << spv_errors << "\n";
    }
    return result;
}
#endif  // TINT_BUILD_SPV_WRITER

/// Generate SPIR-V code for a program.
/// @param options the options that Tint was invoked with
/// @param inspector the inspector
/// @param ir the module to generate
/// @returns true on success
[[maybe_unused]]
bool GenerateSpirv([[maybe_unused]] const Options& options,
                   [[maybe_unused]] tint::inspector::Inspector& inspector,
                   [[maybe_unused]] tint::core::ir::Module& ir) {
#if TINT_BUILD_SPV_WRITER
    tint::spirv::writer::Options gen_options;
    if (options.rename_all) {
        gen_options.remapped_entry_point_name = "tint_entry_point";
        gen_options.strip_all_names = true;
    }
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.use_storage_input_output_16 = options.use_storage_input_output_16;
    gen_options.spirv_version = options.spirv_version;

    auto entry_point = inspector.GetEntryPoint(options.ep_name);

    // Immediate data Offset must be 4-byte aligned.
    uint32_t offset = tint::RoundUp(4u, entry_point.immediate_data_size);

    if (entry_point.frag_depth_used) {
        // Place the RangeOffset immediate data member after user-defined immediate data (if
        // any).
        gen_options.depth_range_offsets = {offset + 0, offset + 4};
        offset += 8;
    }

    gen_options.bindings = tint::spirv::writer::GenerateBindings(ir);

    // Enable the Vulkan Memory Model if needed.
    for (auto* ty : ir.Types()) {
        if (ty->Is<tint::core::type::SubgroupMatrix>()) {
            gen_options.use_vulkan_memory_model = true;
        }
    }

    // Check that the module and options are supported by the backend.
    auto check = tint::spirv::writer::CanGenerate(ir, gen_options);
    if (check != tint::Success) {
        std::cerr << check.Failure() << "\n";
        return false;
    }

    // Generate SPIR-V from Tint IR.
    auto result = tint::spirv::writer::Generate(ir, gen_options);
    if (result != tint::Success) {
        options.printer->Print(tint::core::ir::Disassembler(ir).Text());
        std::cerr << "Failed to generate SPIR-V: " << result.Failure() << "\n";
        return false;
    }

    if (options.format == Format::kSpvAsm) {
        if (!tint::cmd::WriteFile(options.output_file, "w", Disassemble(result.Get().spirv))) {
            return false;
        }
    } else {
        if (!tint::cmd::WriteFile(options.output_file, "wb", result.Get().spirv)) {
            return false;
        }
    }

    const auto hash = tint::CRC32(result.Get().spirv.data(), result.Get().spirv.size());
    if (options.print_hash) {
        PrintHash(hash);
    }

    if (options.validate && options.skip_hash.count(hash) == 0) {
        // Use Vulkan 1.1, since this is what Tint, internally, uses.
        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
        tools.SetMessageConsumer(
            [](spv_message_level_t, const char*, const spv_position_t& pos, const char* msg) {
                std::cerr << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg << "\n";
            });
        if (!tools.Validate(result.Get().spirv.data(), result.Get().spirv.size(),
                            spvtools::ValidatorOptions())) {
            return false;
        }
    }

    return true;
#else
    std::cerr << "SPIR-V writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_SPV_WRITER
}

/// Generate WGSL code for a program.
/// @param options the options that Tint was invoked with
/// @param inspector the inspector
/// @param program the program to generate
/// @returns true on success
bool GenerateWgsl([[maybe_unused]] Options& options,
                  [[maybe_unused]] tint::inspector::Inspector& inspector,
                  [[maybe_unused]] tint::Program& program) {
#if TINT_BUILD_WGSL_WRITER
    auto result = tint::wgsl::writer::Generate(program);
    if (result != tint::Success) {
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    if (!tint::cmd::WriteFile(options.output_file, "w", result->wgsl)) {
        return false;
    }

    const auto hash = tint::CRC32(result->wgsl.data(), result->wgsl.size());
    if (options.print_hash) {
        PrintHash(hash);
    }

#if TINT_BUILD_WGSL_READER
    if (options.validate && options.skip_hash.count(hash) == 0) {
        // Attempt to re-parse the output program with Tint's WGSL reader.
        tint::wgsl::reader::Options parser_options;
        parser_options.allowed_features = tint::wgsl::AllowedFeatures::Everything();
        auto source = std::make_unique<tint::Source::File>(options.input_filename, result->wgsl);
        auto reparsed_program = tint::wgsl::reader::Parse(source.get(), parser_options);
        if (!reparsed_program.IsValid()) {
            tint::diag::Formatter diag_formatter;
            options.printer->Print(diag_formatter.Format(reparsed_program.Diagnostics()));
            return false;
        }
    }
#endif  // TINT_BUILD_WGSL_READER

    return true;
#else
    std::cerr << "WGSL writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_WGSL_WRITER
}

/// Generate MSL code for a program.
/// @param options the options that Tint was invoked with
/// @param inspector the inspector
/// @param ir the module to generate
/// @returns true on success
[[maybe_unused]]
bool GenerateMsl([[maybe_unused]] const Options& options,
                 [[maybe_unused]] tint::inspector::Inspector& inspector,
                 [[maybe_unused]] tint::core::ir::Module& ir) {
#if TINT_BUILD_MSL_WRITER
    if (!options.use_argument_buffers) {
        // Remap resource numbers to a flat namespace.
        auto res = tint::msl::ir::transform::FlattenBindings(ir);
        if (res != tint::Success) {
            std::cerr << "Failed to flatten bindings: " << res.Failure().reason << "\n";
            return false;
        }
    }

    // Set up the backend options.
    tint::msl::writer::Options gen_options;
    if (options.rename_all) {
        gen_options.remapped_entry_point_name = "tint_entry_point";
        gen_options.strip_all_names = true;
    }
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.pixel_local_attachments = options.pixel_local_attachments;
    gen_options.bindings = tint::msl::writer::GenerateBindings(ir, options.use_argument_buffers);
    // TODO(crbug.com/366291600): Replace ubo with immediate block for end2end tests
    gen_options.array_length_from_constants.ubo_binding = 30;
    gen_options.disable_demote_to_helper = options.disable_demote_to_helper;
    gen_options.use_argument_buffers = options.use_argument_buffers;
    gen_options.group_to_argument_buffer_info = options.group_to_argument_buffer_info;

    // Add array_length_from_constants entries for all storage buffers with runtime sized arrays.
    std::unordered_set<tint::BindingPoint> storage_bindings;
    for (auto* inst : *ir.root_block) {
        auto* var = inst->As<tint::core::ir::Var>();
        if (!var) {
            continue;
        }

        auto bp = var->BindingPoint();
        if (!bp.has_value()) {
            continue;
        }

        auto* ty = var->Result()->Type()->UnwrapPtr();
        if (!ty->HasFixedFootprint()) {
            if (storage_bindings.insert(bp.value()).second) {
                gen_options.array_length_from_constants.bindpoint_to_size_index.emplace(
                    bp.value(), static_cast<uint32_t>(storage_bindings.size() - 1));
            }
        }
    }

    // Check that the module and options are supported by the backend.
    auto check = tint::msl::writer::CanGenerate(ir, gen_options);
    if (check != tint::Success) {
        std::cerr << check.Failure() << "\n";
        return false;
    }

    auto result = tint::msl::writer::Generate(ir, gen_options);
    if (result != tint::Success) {
        options.printer->Print(tint::core::ir::Disassembler(ir).Text());
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    if (!tint::cmd::WriteFile(options.output_file, "w", result->msl)) {
        return false;
    }

    const auto hash = tint::CRC32(result->msl.c_str());
    if (options.print_hash) {
        PrintHash(hash);
    }

    if (options.validate && options.skip_hash.count(hash) == 0) {
        tint::msl::validate::Result res;
#if TINT_BUILD_IS_MAC
        res = tint::msl::validate::ValidateUsingMetal(result->msl, options.msl_version);
#else
#ifdef _WIN32
        const char* default_xcrun_exe = "metal.exe";
#else
        const char* default_xcrun_exe = "xcrun";
#endif  // _WIN32
        auto xcrun = tint::Command::LookPath(
            options.xcrun_path.empty() ? default_xcrun_exe : std::string(options.xcrun_path));
        if (xcrun.Found()) {
            res = tint::msl::validate::Validate(xcrun.Path(), result->msl, options.msl_version);
        } else {
            res.output = "xcrun executable not found. Cannot validate.";
            res.failed = true;
        }
#endif  // TINT_BUILD_IS_MAC
        if (res.failed) {
            std::cerr << res.output << "\n";
            return false;
        }
    }

    return true;
#else
    std::cerr << "MSL writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_MSL_WRITER
}

/// Generate HLSL code for a program.
/// @param options the options that Tint was invoked with
/// @param inspector the inspector
/// @param ir the module to generate
/// @returns true on success
[[maybe_unused]]
bool GenerateHlsl([[maybe_unused]] const Options& options,
                  [[maybe_unused]] tint::inspector::Inspector& inspector,
                  [[maybe_unused]] tint::core::ir::Module& ir) {
#if TINT_BUILD_HLSL_WRITER
    const bool for_fxc = options.format == Format::kHlslFxc;
    // Set up the backend options.
    tint::hlsl::writer::Options gen_options;
    if (options.rename_all) {
        gen_options.remapped_entry_point_name = "tint_entry_point";
        gen_options.strip_all_names = true;
    }
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.pixel_local = options.pixel_local_options;
    gen_options.polyfill_dot_4x8_packed = options.hlsl_shader_model < kMinShaderModelForDP4aInHLSL;
    gen_options.polyfill_pack_unpack_4x8 =
        options.hlsl_shader_model < kMinShaderModelForPackUnpack4x8InHLSL;
    gen_options.compiler = for_fxc ? tint::hlsl::writer::Options::Compiler::kFXC
                                   : tint::hlsl::writer::Options::Compiler::kDXC;
    gen_options.bindings = tint::hlsl::writer::GenerateBindings(ir);

    // Check that the module and options are supported by the backend.
    auto check = tint::hlsl::writer::CanGenerate(ir, gen_options);
    if (check != tint::Success) {
        std::cerr << check.Failure() << "\n";
        return false;
    }

    auto result = tint::hlsl::writer::Generate(ir, gen_options);
    if (result != tint::Success) {
        options.printer->Print(tint::core::ir::Disassembler(ir).Text());
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    if (!tint::cmd::WriteFile(options.output_file, "w", result->hlsl)) {
        return false;
    }

    const auto hash = tint::CRC32(result->hlsl.c_str());
    if (options.print_hash) {
        PrintHash(hash);
    }

    const bool validate =
        (options.validate || !options.fxc_path.empty() || !options.dxc_path.empty()) &&
        (options.skip_hash.count(hash) == 0);

    if (validate && !for_fxc) {
        // DXC validation
        tint::hlsl::validate::Result dxc_res;
        const std::string dxc_path =
            options.dxc_path.empty() ? tint::hlsl::validate::kDxcDLLName : options.dxc_path;
        auto dxc = tint::Command::LookPath(dxc_path);
        if (dxc.Found()) {
            uint32_t hlsl_shader_model = options.hlsl_shader_model;
            bool dxc_require_16bit_types = false;
            for (auto* ty : ir.Types()) {
                if (ty->Is<tint::core::type::F16>()) {
                    dxc_require_16bit_types = true;
                    break;
                }
            }
            if (options.verbose) {
                std::cout << "Validating with DXC: " << dxc.Path() << "\n";
            }
            dxc_res = tint::hlsl::validate::ValidateUsingDXC(
                dxc.Path(), result->hlsl, result->entry_point_name, result->pipeline_stage,
                dxc_require_16bit_types, hlsl_shader_model);
        } else {
            dxc_res.failed = true;
            dxc_res.output = "DXC executable '" + dxc_path + "' not found. Cannot validate.";
        }

        if (dxc_res.failed) {
            std::cerr << "DXC validation failure:\n" << dxc_res.output << "\n";
            return false;
        }
        if (options.verbose) {
            std::cout << "Passed DXC validation. Compiler output:\n" << dxc_res.output << "\n";
        }
    }

    if (validate && for_fxc) {
        // FXC validation
#ifndef _WIN32
        std::cerr << "FXC can only be used on Windows.\n";
        return false;
#else
        tint::hlsl::validate::Result fxc_res;
        auto fxc = tint::Command::LookPath(
            options.fxc_path.empty() ? tint::hlsl::validate::kFxcDLLName : options.fxc_path);
        if (fxc.Found()) {
            if (options.verbose) {
                std::cout << "Validating with FXC: " << fxc.Path() << "\n";
            }
            fxc_res = tint::hlsl::validate::ValidateUsingFXC(
                fxc.Path(), result->hlsl, result->entry_point_name, result->pipeline_stage);
        } else {
            fxc_res.failed = true;
            fxc_res.output = "FXC DLL '" + options.fxc_path + "' not found. Cannot validate.";
        }

        if (fxc_res.failed) {
            std::cerr << "FXC validation failure:\n" << fxc_res.output << "\n";
            return false;
        }
        if (options.verbose) {
            std::cout << "Passed FXC validation. Compiler output:\n" << fxc_res.output << "\n";
        }
#endif  // _WIN32
    }

    return true;
#else
    std::cerr << "HLSL writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_HLSL_WRITER
}

/// Generate GLSL code for a program.
/// @param options the options that Tint was invoked with
/// @param inspector the inspector
/// @param ir the module to generate
/// @returns true on success
[[maybe_unused]]
bool GenerateGlsl([[maybe_unused]] const Options& options,
                  [[maybe_unused]] tint::inspector::Inspector& inspector,
                  [[maybe_unused]] tint::core::ir::Module& ir) {
#if TINT_BUILD_GLSL_WRITER
    tint::glsl::writer::Options gen_options;
    gen_options.strip_all_names = options.rename_all;
    if (options.glsl_desktop) {
        gen_options.version =
            tint::glsl::writer::Version(tint::glsl::writer::Version::Standard::kDesktop, 4, 6);
    } else {
        gen_options.version = tint::glsl::writer::Version();
    }

    gen_options.disable_robustness = !options.enable_robustness;

    auto entry_point = inspector.GetEntryPoint(options.ep_name);

    // Immediate data Offset must be 4-byte aligned.
    uint32_t offset = tint::RoundUp(4u, entry_point.immediate_data_size);

    if (entry_point.instance_index_used) {
        // Place the first_instance immediate data member after user-defined immediate data (if
        // any).
        gen_options.first_instance_offset = offset;
        offset += 4;
    }
    if (entry_point.frag_depth_used) {
        gen_options.depth_range_offsets = {offset + 0, offset + 4};
        offset += 8;
    }

    for (auto idx : options.bgra_swizzle) {
        gen_options.bgra_swizzle_locations.insert({idx});
    }

    // Generate binding options.
    gen_options.bindings = tint::glsl::writer::GenerateBindings(ir);

    // Check that the module and options are supported by the backend.
    auto check = tint::glsl::writer::CanGenerate(ir, gen_options);
    if (check != tint::Success) {
        std::cerr << check.Failure() << "\n";
        return false;
    }

    // Generate GLSL.
    auto result = tint::glsl::writer::Generate(ir, gen_options);
    if (result != tint::Success) {
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    if (!tint::cmd::WriteFile(options.output_file, "w", result->glsl)) {
        return false;
    }

    const auto hash = tint::CRC32(result->glsl.c_str());
    if (options.print_hash) {
        PrintHash(hash);
    }

    if (options.validate && options.skip_hash.count(hash) == 0) {
#if !TINT_BUILD_GLSL_VALIDATOR
        std::cerr << "GLSL validator not enabled in tint build\n";
        return false;
#else
        // If there is no entry point name there is nothing to validate
        if (options.ep_name != "") {
            tint::core::ir::Function::PipelineStage stage =
                tint::core::ir::Function::PipelineStage::kCompute;
            switch (entry_point.stage) {
                case tint::inspector::PipelineStage::kCompute:
                    stage = tint::core::ir::Function::PipelineStage::kCompute;
                    break;
                case tint::inspector::PipelineStage::kVertex:
                    stage = tint::core::ir::Function::PipelineStage::kVertex;
                    break;
                case tint::inspector::PipelineStage::kFragment:
                    stage = tint::core::ir::Function::PipelineStage::kFragment;
                    break;
            }

            auto val = tint::glsl::validate::Validate(result->glsl, stage);
            if (val != tint::Success) {
                std::cerr << val.Failure();
                return false;
            }
        }
#endif  // !TINT_BUILD_GLSL_VALIDATOR
    }
    return true;
#else
    std::cerr << "GLSL writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_GLSL_WRITER
}

/// Generate IR code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool DumpIR([[maybe_unused]] const tint::Program& program,
            [[maybe_unused]] const Options& options) {
#if TINT_BUILD_WGSL_READER
    auto result = tint::wgsl::reader::ProgramToLoweredIR(program);
    if (result != tint::Success) {
        std::cerr << "Failed to build IR from program: " << result.Failure() << "\n";
        return false;
    }

    options.printer->Print(tint::core::ir::Disassembler(result.Get()).Text());
    options.printer->Print(tint::StyledText{} << "\n");

    return true;
#else
    std::cerr << "WGSL reader not enabled in tint build\n";
    return false;
#endif
}

/// Generate backend code for a program.
/// @param options the options that Tint was invoked with
/// @param inspector the inspector
/// @param program the program to generate
/// @returns true on success
bool Generate([[maybe_unused]] const Options& options,
              [[maybe_unused]] tint::inspector::Inspector& inspector,
              [[maybe_unused]] const tint::Program& program) {
#if TINT_BUILD_WGSL_READER
    // Convert the AST program to an IR module.
    auto ir = tint::wgsl::reader::ProgramToLoweredIR(program);
    if (ir != tint::Success) {
        std::cerr << "Failed to generate IR: " << ir << "\n";
        return false;
    }

    // Strip the module down to a single entry point.
    if (options.ep_name != "") {
        auto singleEntryPointResult =
            tint::core::ir::transform::SingleEntryPoint(ir.Get(), options.ep_name);
        if (singleEntryPointResult != tint::Success) {
            std::cerr << "SingleEntryPoint failed:\n" << singleEntryPointResult.Failure() << "\n";
            return false;
        }
    }

    // Run SubstituteOverrides to replace override instructions with constants.
    // This needs to run after SingleEntryPoint which removes unused overrides.
    auto substitute_override_cfg = CreateOverrideMap(options, inspector);
    if (substitute_override_cfg != tint::Success) {
        std::cerr << "Failed to create override map: " << substitute_override_cfg.Failure() << "\n";
        return false;
    }
    auto substituteOverridesResult =
        tint::core::ir::transform::SubstituteOverrides(ir.Get(), substitute_override_cfg.Get());
    if (substituteOverridesResult != tint::Success) {
        std::cerr << "SubstituteOverrides failed:\n" << substituteOverridesResult.Failure() << "\n";
        return false;
    }

    switch (options.format) {
        case Format::kSpirv:
        case Format::kSpvAsm:
            return GenerateSpirv(options, inspector, ir.Get());
        case Format::kMsl:
            return GenerateMsl(options, inspector, ir.Get());
        case Format::kHlsl:
        case Format::kHlslFxc:
            return GenerateHlsl(options, inspector, ir.Get());
        case Format::kGlsl:
            return GenerateGlsl(options, inspector, ir.Get());
        case Format::kWgsl:
            TINT_UNREACHABLE();
        case Format::kNone:
            break;
        default:
            std::cerr << "Unknown output format specified\n";
            break;
    }
#else
    std::cerr << "Cannot convert WGSL programs to Tint IR without the WGSL reader\n";
#endif  // TINT_BUILD_WGSL_READER
    return false;
}

int Run(tint::VectorRef<std::string_view> arguments, ExeMode exe_mode) {
    Options options;

    if (!ParseArgs(arguments, &options, exe_mode)) {
        return 1;
    }

    if (exe_mode == ExeMode::kServer && options.format == Format::kSpirv) {
        std::cerr << "Cannot emit binary SPIR-V to stdout in server mode\n";
        return 1;
    }

    // Implement output format defaults.
    if (options.format == Format::kUnknown) {
        // Try inferring from filename.
        options.format = InferFormat(options.output_file);
    }
    if (options.format == Format::kUnknown) {
        // Ultimately, default to SPIR-V assembly. That's nice for interactive use.
        options.format = Format::kSpvAsm;
    }

    tint::cmd::LoadProgramOptions opts;
    opts.filename = options.input_filename;
    opts.printer = options.printer.get();
#if TINT_BUILD_SPV_READER
    opts.use_ir_reader = options.use_ir_reader;
    opts.spirv_reader_options = options.spirv_reader_options;
    // Allow the shader-f16 extension
    opts.spirv_reader_options.allowed_features = tint::wgsl::AllowedFeatures::Everything();
#endif

    auto info = tint::cmd::LoadProgramInfo(opts);

    if (options.parse_only) {
        return 1;
    }

    if (options.dump_ir || options.format == Format::kIr) {
        auto res = DumpIR(info.program, options);
        if (options.format == Format::kIr) {
            return static_cast<int>(res);
        }
    }

    tint::inspector::Inspector inspector(info.program);
    if (options.dump_inspector_bindings) {
        tint::cmd::PrintInspectorBindings(inspector);
    }
    // Handle WGSL special as we want multiple shaders in a single file, unlike other formats where
    // we run single entry point.
    if (options.format == Format::kWgsl) {
        return GenerateWgsl(options, inspector, info.program) ? 0 : 1;
    }

    if (inspector.GetEntryPoints().empty()) {
        return Generate(options, inspector, info.program) ? 0 : 1;
    }

    bool success = true;
    std::string outfile_name = options.output_file;
    auto entry_points = inspector.GetEntryPoints();
    for (auto& entry_point : entry_points) {
        if (options.emit_single_entry_point && entry_point.name != options.ep_name) {
            continue;
        }

        // If we're emitting to stdout, add a separator between the entry points. If we're going
        // to a file, and we aren't emitting a single entry point, add the entry point name into
        // the output file name.
        if (tint::cmd::IsStdout(options.output_file)) {
            if (entry_points.size() > 1) {
                if (options.format == Format::kSpvAsm) {
                    std::cout << ";\n; ";
                } else {
                    std::cout << "//\n// ";
                }
                std::cout << entry_point.name << "\n";
                if (options.format == Format::kSpvAsm) {
                    std::cout << ";\n";
                } else {
                    std::cout << "//\n";
                }
            }
        } else if (!options.emit_single_entry_point) {
            auto pos = outfile_name.rfind(".");
            if (pos == std::string_view::npos) {
                options.output_file = outfile_name + "." + entry_point.name;
            } else {
                options.output_file = outfile_name.substr(0, pos) + "." + entry_point.name + "." +
                                      outfile_name.substr(pos + 1);
            }
        }

        options.ep_name = entry_point.name;
        success &= Generate(options, inspector, info.program);

        if (options.emit_single_entry_point) {
            break;
        }
    }
    return success ? 0 : 1;
}

/// Run a server that accepts arguments on stdin.
/// @returns 0 on success, non-zero on failure
int RunServer() {
    // Each line read from stdin will invoke Tint with the supplied arguments.
    // Output on stdout and stderr will be delimited with \0 characters.
    // The server will exit on failure or if stdin is closed.
    while (!std::cin.eof()) {
        // Read the next set of arguments.
        std::string arg_line;
        std::getline(std::cin, arg_line);

        // Split the arguments by whitespace, taking double-quotes into account.
        std::istringstream arg_in(arg_line);
        tint::Vector<std::string, 8> arg_tokens;
        while (!arg_in.eof()) {
            std::string arg;
            arg_in >> std::quoted(arg, '"', '\0');
            if (!arg.empty()) {
                arg_tokens.Push(arg);
            }
        }

        // Run Tint with the provided arguments.
        auto arguments =
            tint::Transform(arg_tokens, [](const std::string& arg) -> std::string_view {
                return std::string_view(arg);  //
            });
        auto ret = Run(arguments, ExeMode::kServer);
        if (ret != 0) {
            // The Tint invocation failed, so exit the server.
            return ret;
        }

        // Delimit stdout and stderr with \0 and flush them.
        std::cout << '\0' << std::flush;
        std::cerr << '\0' << std::flush;
    }
    return 0;
}

}  // namespace

int main(int argc, const char** argv) {
    tint::Vector<std::string_view, 8> arguments = tint::args::Vectorize(argc, argv);

    tint::Initialize();

    if (arguments.Length() > 0 && arguments[0] == "--server") {
        if (arguments.Length() > 1) {
            std::cerr << "--server must not be used with any other arguments\n";
            return 1;
        }
        return RunServer();
    }

    return Run(arguments, ExeMode::kStandalone);
}
