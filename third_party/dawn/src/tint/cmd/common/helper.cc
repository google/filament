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

#include "src/tint/cmd/common/helper.h"

#include <cstdio>
#include <iostream>
#include <utility>
#include <vector>

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#include "src/tint/lang/spirv/reader/reader.h"
#endif

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/writer/writer.h"
#endif

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/reader.h"
#endif

#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/utils/diagnostic/formatter.h"
#include "src/tint/utils/rtti/traits.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"
#include "src/tint/utils/text/text_style.h"

namespace tint::cmd {
namespace {

enum class InputFormat {
    kUnknown,
    kWgsl,
    kSpirvBin,
    kSpirvAsm,
};

/// @param out the stream to write to
/// @param value the InputFormat
/// @returns @p out so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, InputFormat value) {
    switch (value) {
        case InputFormat::kUnknown:
            break;
        case InputFormat::kWgsl:
            return out << "wgsl";
        case InputFormat::kSpirvBin:
            return out << "spirv";
        case InputFormat::kSpirvAsm:
            return out << "spirv asm";
    }
    return out << "unknown";
}

InputFormat InputFormatFromFilename(const std::string& filename) {
    auto input_format = InputFormat::kUnknown;
    if (tint::HasSuffix(filename, ".wgsl")) {
        input_format = InputFormat::kWgsl;
    } else if (tint::HasSuffix(filename, ".spv")) {
        input_format = InputFormat::kSpirvBin;
    } else if (tint::HasSuffix(filename, ".spvasm")) {
        input_format = InputFormat::kSpirvAsm;
    }
    return input_format;
}

void PrintBindings(tint::inspector::Inspector& inspector, const std::string& ep_name) {
    auto bindings = inspector.GetResourceBindings(ep_name);
    if (!inspector.error().empty()) {
        std::cerr << "Failed to get bindings from Inspector: " << inspector.error() << "\n";
        exit(1);
    }
    for (auto& binding : bindings) {
        std::cout << "\t[" << binding.bind_group << "][" << binding.binding << "]:\n"
                  << "\t\t resource_type = " << ResourceTypeToString(binding.resource_type) << "\n"
                  << "\t\t dim = " << TextureDimensionToString(binding.dim) << "\n"
                  << "\t\t sampled_kind = " << SampledKindToString(binding.sampled_kind) << "\n"
                  << "\t\t image_format = " << TexelFormatToString(binding.image_format) << "\n\n";
    }
}

#if TINT_BUILD_SPV_READER
tint::Program ReadSpirv(const std::vector<uint32_t>& data, const LoadProgramOptions& opts) {
    if (opts.use_ir_reader) {
#if TINT_BUILD_WGSL_WRITER
        // Parse the SPIR-V binary to a core Tint IR module.
        auto result = tint::spirv::reader::ReadIR(data, opts.spirv_reader_options);
        if (result != Success) {
            std::cerr << "Failed to parse SPIR-V: " << result.Failure() << "\n";
            exit(1);
        }

        // Convert the IR module to a Program.
        tint::wgsl::writer::ProgramOptions writer_options;
        writer_options.allow_non_uniform_derivatives =
            opts.spirv_reader_options.allow_non_uniform_derivatives;
        writer_options.allowed_features = opts.spirv_reader_options.allowed_features;
        auto prog_result = tint::wgsl::writer::ProgramFromIR(result.Get(), writer_options);
        if (prog_result != Success) {
            std::cerr << "Failed to convert IR to Program:\n\n" << prog_result.Failure() << "\n\n";
            std::cerr << tint::core::ir::Disassembler(result.Get()).Plain() << "\n";
            exit(1);
        }

        return prog_result.Move();
#else
        std::cerr << "Tint not built with the WGSL writer enabled\n";
        exit(1);
#endif  // TINT_BUILD_WGSL_READER
    } else {
        return tint::spirv::reader::Read(data, opts.spirv_reader_options);
    }
}
#endif  // TINT_BUILD_SPV_READER

}  // namespace

void PrintWGSL(std::ostream& out, const tint::Program& program) {
#if TINT_BUILD_WGSL_WRITER
    auto result = tint::wgsl::writer::Generate(program);
    if (result == Success) {
        out << "\n" << result->wgsl << "\n";
    } else {
        out << result.Failure() << "\n";
    }
#else
    (void)out;
    (void)program;
#endif
}

ProgramInfo LoadProgramInfo(const LoadProgramOptions& opts) {
    auto input_format = InputFormatFromFilename(opts.filename);

    auto load = [&]() -> ProgramInfo {
        switch (input_format) {
            case InputFormat::kUnknown:
                break;

            case InputFormat::kWgsl: {
#if TINT_BUILD_WGSL_READER
                std::vector<uint8_t> data;
                if (!ReadFile<uint8_t>(opts.filename, &data)) {
                    exit(1);
                }

                tint::wgsl::reader::Options options;
                options.allowed_features = tint::wgsl::AllowedFeatures::Everything();

                auto file = std::make_unique<tint::Source::File>(
                    opts.filename, std::string(data.begin(), data.end()));

                return ProgramInfo{
                    /* program */ tint::wgsl::reader::Parse(file.get(), options),
                    /* source_file */ std::move(file),
                };
#else
                std::cerr << "Tint not built with the WGSL reader enabled\n";
                exit(1);
#endif  // TINT_BUILD_WGSL_READER
            }
            case InputFormat::kSpirvBin: {
#if TINT_BUILD_SPV_READER
                std::vector<uint32_t> data;
                if (!ReadFile<uint32_t>(opts.filename, &data)) {
                    exit(1);
                }

                return ProgramInfo{
                    /* program */ ReadSpirv(data, opts),
                    /* source_file */ nullptr,
                };
#else
                std::cerr << "Tint not built with the SPIR-V reader enabled\n";
                exit(1);
#endif  // TINT_BUILD_SPV_READER
            }
            case InputFormat::kSpirvAsm: {
#if TINT_BUILD_SPV_READER
                std::vector<char> text;
                if (!ReadFile<char>(opts.filename, &text)) {
                    exit(1);
                }
                // Use Vulkan 1.1, since this is what Tint, internally, is expecting.
                spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
                tools.SetMessageConsumer([](spv_message_level_t, const char*,
                                            const spv_position_t& pos, const char* msg) {
                    std::cerr << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg << "\n";
                });
                std::vector<uint32_t> data;
                if (!tools.Assemble(text.data(), text.size(), &data,
                                    SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS)) {
                    exit(1);
                }

                auto file = std::make_unique<tint::Source::File>(
                    opts.filename, std::string(text.begin(), text.end()));

                return ProgramInfo{
                    /* program */ ReadSpirv(data, opts),
                    /* source_file */ std::move(file),
                };
#else
                std::cerr << "Tint not built with the SPIR-V reader enabled\n";
                exit(1);
#endif  // TINT_BUILD_SPV_READER
            }
        }

        std::cerr << "Unknown input format: " << input_format << "\n";
        exit(1);
    };

    ProgramInfo info = load();

    if (info.program.Diagnostics().Count() > 0) {
        if (!info.program.IsValid() && input_format != InputFormat::kWgsl) {
            // Invalid program from a non-wgsl source.
            // Print the WGSL, to help understand the diagnostics.
            PrintWGSL(std::cout, info.program);
        }

        tint::diag::Formatter formatter;
        if (opts.printer) {
            opts.printer->Print(formatter.Format(info.program.Diagnostics()));
        } else {
            tint::StyledTextPrinter::Create(stderr)->Print(
                formatter.Format(info.program.Diagnostics()));
        }
        // Flush any diagnostics written to stderr. We depend on these being emitted to the console
        // before the program for end-to-end tests.
        fflush(stderr);
    }

    if (!info.program.IsValid()) {
        exit(1);
    }

    return info;
}

void PrintInspectorData(tint::inspector::Inspector& inspector) {
    auto entry_points = inspector.GetEntryPoints();
    if (!inspector.error().empty()) {
        std::cerr << "Failed to get entry points from Inspector: " << inspector.error() << "\n";
        exit(1);
    }

    for (auto& entry_point : entry_points) {
        std::cout << "Entry Point = " << entry_point.name << " ("
                  << EntryPointStageToString(entry_point.stage) << ")\n";

        if (entry_point.workgroup_size) {
            std::cout << "  Workgroup Size (" << entry_point.workgroup_size->x << ", "
                      << entry_point.workgroup_size->y << ", " << entry_point.workgroup_size->z
                      << ")\n";
        }

        if (!entry_point.input_variables.empty()) {
            std::cout << "  Input Variables:\n";

            for (const auto& var : entry_point.input_variables) {
                std::cout << "\t";

                if (auto location = var.attributes.location) {
                    std::cout << "@location(" << location.value() << ") ";
                }

                if (auto color = var.attributes.color) {
                    std::cout << "@color(" << color.value() << ") ";
                }
                std::cout << var.name << "\n";
            }
        }
        if (!entry_point.output_variables.empty()) {
            std::cout << "  Output Variables:\n";

            for (const auto& var : entry_point.output_variables) {
                std::cout << "\t";

                if (auto location = var.attributes.location) {
                    std::cout << "@location(" << location.value() << ") ";
                }
                std::cout << var.name << "\n";
            }
        }
        if (!entry_point.overrides.empty()) {
            std::cout << "  Overrides:\n";

            for (const auto& var : entry_point.overrides) {
                std::cout << "\tname: " << var.name << "\n";
                std::cout << "\tid: " << var.id.value << "\n";
            }
        }

        auto bindings = inspector.GetResourceBindings(entry_point.name);
        if (!inspector.error().empty()) {
            std::cerr << "Failed to get bindings from Inspector: " << inspector.error() << "\n";
            exit(1);
        }

        if (!bindings.empty()) {
            std::cout << "  Bindings:\n";
            PrintBindings(inspector, entry_point.name);
            std::cout << "\n";
        }

        std::cout << "\n";
    }
}

void PrintInspectorBindings(tint::inspector::Inspector& inspector) {
    std::cout << std::string(80, '-') << "\n";
    auto entry_points = inspector.GetEntryPoints();
    if (!inspector.error().empty()) {
        std::cerr << "Failed to get entry points from Inspector: " << inspector.error() << "\n";
        exit(1);
    }

    for (auto& entry_point : entry_points) {
        std::cout << "Entry Point = " << entry_point.name << "\n";
        PrintBindings(inspector, entry_point.name);
    }
    std::cout << std::string(80, '-') << "\n";
}

std::string EntryPointStageToString(tint::inspector::PipelineStage stage) {
    switch (stage) {
        case tint::inspector::PipelineStage::kVertex:
            return "vertex";
        case tint::inspector::PipelineStage::kFragment:
            return "fragment";
        case tint::inspector::PipelineStage::kCompute:
            return "compute";
    }
    return "Unknown";
}

std::string TextureDimensionToString(tint::inspector::ResourceBinding::TextureDimension dim) {
    switch (dim) {
        case tint::inspector::ResourceBinding::TextureDimension::kNone:
            return "None";
        case tint::inspector::ResourceBinding::TextureDimension::k1d:
            return "1d";
        case tint::inspector::ResourceBinding::TextureDimension::k2d:
            return "2d";
        case tint::inspector::ResourceBinding::TextureDimension::k2dArray:
            return "2dArray";
        case tint::inspector::ResourceBinding::TextureDimension::k3d:
            return "3d";
        case tint::inspector::ResourceBinding::TextureDimension::kCube:
            return "Cube";
        case tint::inspector::ResourceBinding::TextureDimension::kCubeArray:
            return "CubeArray";
    }

    return "Unknown";
}

std::string SampledKindToString(tint::inspector::ResourceBinding::SampledKind kind) {
    switch (kind) {
        case tint::inspector::ResourceBinding::SampledKind::kFloat:
            return "Float";
        case tint::inspector::ResourceBinding::SampledKind::kUInt:
            return "UInt";
        case tint::inspector::ResourceBinding::SampledKind::kSInt:
            return "SInt";
        case tint::inspector::ResourceBinding::SampledKind::kUnknown:
            break;
    }

    return "Unknown";
}

std::string TexelFormatToString(tint::inspector::ResourceBinding::TexelFormat format) {
    switch (format) {
        case tint::inspector::ResourceBinding::TexelFormat::kR32Uint:
            return "R32Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kR32Sint:
            return "R32Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kR32Float:
            return "R32Float";
        case tint::inspector::ResourceBinding::TexelFormat::kBgra8Unorm:
            return "Bgra8Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Unorm:
            return "Rgba8Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Snorm:
            return "Rgba8Snorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Uint:
            return "Rgba8Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Sint:
            return "Rgba8Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Uint:
            return "Rg32Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Sint:
            return "Rg32Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Float:
            return "Rg32Float";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Unorm:
            return "Rgba16Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Snorm:
            return "Rgba16Snorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Uint:
            return "Rgba16Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Sint:
            return "Rgba16Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Float:
            return "Rgba16Float";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Uint:
            return "Rgba32Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Sint:
            return "Rgba32Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Float:
            return "Rgba32Float";
        case tint::inspector::ResourceBinding::TexelFormat::kR8Unorm:
            return "R8Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kR8Snorm:
            return "R8Snorm";
        case tint::inspector::ResourceBinding::TexelFormat::kR8Uint:
            return "R8Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kR8Sint:
            return "R8Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg8Unorm:
            return "Rg8Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRg8Snorm:
            return "Rg8Snorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRg8Uint:
            return "Rg8Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg8Sint:
            return "Rg8Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kR16Unorm:
            return "R16Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kR16Snorm:
            return "R16Snorm";
        case tint::inspector::ResourceBinding::TexelFormat::kR16Uint:
            return "R16Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kR16Sint:
            return "R16Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kR16Float:
            return "R16Float";
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Unorm:
            return "Rg16Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Snorm:
            return "Rg16Snorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Uint:
            return "Rg16Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Sint:
            return "Rg16Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg16Float:
            return "Rg16Float";
        case tint::inspector::ResourceBinding::TexelFormat::kRgb10A2Uint:
            return "Rgb10A2Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgb10A2Unorm:
            return "Rgb10A2Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRg11B10Ufloat:
            return "Rg11B10Ufloat";
        case tint::inspector::ResourceBinding::TexelFormat::kNone:
            return "None";
    }
    return "Unknown";
}

std::string ResourceTypeToString(tint::inspector::ResourceBinding::ResourceType type) {
    switch (type) {
        case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
            return "UniformBuffer";
        case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
            return "StorageBuffer";
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
            return "ReadOnlyStorageBuffer";
        case tint::inspector::ResourceBinding::ResourceType::kSampler:
            return "Sampler";
        case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
            return "ComparisonSampler";
        case tint::inspector::ResourceBinding::ResourceType::kSampledTexture:
            return "SampledTexture";
        case tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture:
            return "MultisampledTexture";
        case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
            return "WriteOnlyStorageTexture";
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageTexture:
            return "ReadOnlyStorageTexture";
        case tint::inspector::ResourceBinding::ResourceType::kReadWriteStorageTexture:
            return "ReadWriteStorageTexture";
        case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
            return "DepthTexture";
        case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
            return "DepthMultisampledTexture";
        case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
            return "ExternalTexture";
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyTexelBuffer:
            return "ReadOnlyTexelBuffer";
        case tint::inspector::ResourceBinding::ResourceType::kReadWriteTexelBuffer:
            return "ReadWriteTexelBuffer";
        case tint::inspector::ResourceBinding::ResourceType::kInputAttachment:
            return "InputAttachment";
    }

    return "Unknown";
}

std::string ComponentTypeToString(tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kUnknown:
            return "unknown";
        case tint::inspector::ComponentType::kF32:
            return "f32";
        case tint::inspector::ComponentType::kU32:
            return "u32";
        case tint::inspector::ComponentType::kI32:
            return "i32";
        case tint::inspector::ComponentType::kF16:
            return "f16";
    }
    return "unknown";
}

std::string CompositionTypeToString(tint::inspector::CompositionType type) {
    switch (type) {
        case tint::inspector::CompositionType::kUnknown:
            return "unknown";
        case tint::inspector::CompositionType::kScalar:
            return "scalar";
        case tint::inspector::CompositionType::kVec2:
            return "vec2";
        case tint::inspector::CompositionType::kVec3:
            return "vec3";
        case tint::inspector::CompositionType::kVec4:
            return "vec4";
    }
    return "unknown";
}

std::string InterpolationTypeToString(tint::inspector::InterpolationType type) {
    switch (type) {
        case tint::inspector::InterpolationType::kUnknown:
            return "unknown";
        case tint::inspector::InterpolationType::kPerspective:
            return "perspective";
        case tint::inspector::InterpolationType::kLinear:
            return "linear";
        case tint::inspector::InterpolationType::kFlat:
            return "flat";
    }
    return "unknown";
}

std::string InterpolationSamplingToString(tint::inspector::InterpolationSampling type) {
    switch (type) {
        case tint::inspector::InterpolationSampling::kUnknown:
            return "unknown";
        case tint::inspector::InterpolationSampling::kNone:
            return "none";
        case tint::inspector::InterpolationSampling::kCenter:
            return "center";
        case tint::inspector::InterpolationSampling::kCentroid:
            return "centroid";
        case tint::inspector::InterpolationSampling::kSample:
            return "sample";
        case tint::inspector::InterpolationSampling::kFirst:
            return "first";
        case tint::inspector::InterpolationSampling::kEither:
            return "either";
    }
    return "unknown";
}

std::string OverrideTypeToString(tint::inspector::Override::Type type) {
    switch (type) {
        case tint::inspector::Override::Type::kBool:
            return "bool";
        case tint::inspector::Override::Type::kFloat32:
            return "f32";
        case tint::inspector::Override::Type::kFloat16:
            return "f16";
        case tint::inspector::Override::Type::kUint32:
            return "u32";
        case tint::inspector::Override::Type::kInt32:
            return "i32";
    }
    return "unknown";
}

bool IsStdout(const std::string& name) {
    return name.empty() || name == "-";
}

}  // namespace tint::cmd
