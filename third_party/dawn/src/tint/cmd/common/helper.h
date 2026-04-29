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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/439062058): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#ifndef SRC_TINT_CMD_COMMON_HELPER_H_
#define SRC_TINT_CMD_COMMON_HELPER_H_

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "src/tint/lang/wgsl/inspector/inspector.h"
#include "src/tint/utils/diagnostic/source.h"

#if TINT_BUILD_SPV_READER
#include "src/tint/lang/spirv/reader/common/options.h"
#endif

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/writer/common/options.h"
#endif

// Forward declarations
namespace tint {
class Program;
class StyledTextPrinter;
}  // namespace tint

namespace tint::cmd {

enum class InputFormat {
    kUnknown,
    kWgsl,
    kSpirvBin,
    kSpirvAsm,
};

/// Information on a loaded program
struct ProgramInfo {
    /// The loaded program
    tint::Program program;
    /// The source file information
    std::unique_ptr<tint::Source::File> source_file;
};

/// PrintWGSL writes the WGSL of the program to the provided ostream, if the
/// WGSL writer is enabled, otherwise it does nothing.
/// @param out the output stream to write the WGSL to
/// @param program the program
void PrintWGSL(std::ostream& out, const tint::Program& program);

/// Prints inspector data information to stderr
/// @param inspector the inspector to print.
void PrintInspectorData(tint::inspector::Inspector& inspector);

/// Prints inspector binding information to stderr
/// @param inspector the inspector to print.
void PrintInspectorBindings(tint::inspector::Inspector& inspector);

/// Options for the LoadProgramInfo call
struct LoadProgramOptions {
    /// The file to be loaded, might be "-" for stdin
    std::string filename;
    /// The input format. Optional for files, mandatory for stdin
    InputFormat input_format;
#if TINT_BUILD_SPV_READER
    /// Spirv-reader options
    tint::spirv::reader::Options spirv_reader_options;
#endif
#if TINT_BUILD_WGSL_WRITER
    /// WGSL writer options
    tint::wgsl::writer::Options wgsl_writer_options;
#endif
    /// The text printer to use for output
    StyledTextPrinter* printer = nullptr;
};

/// Loads the source and program information for the given file.
/// If the file cannot be loaded then an error is printed and the program is aborted before
/// returning.
/// @param opts the loading options
ProgramInfo LoadProgramInfo(const LoadProgramOptions& opts);

/// @param stage the pipeline stage
/// @returns the string representation
std::string EntryPointStageToString(tint::inspector::PipelineStage stage);

/// @param dim the dimension
/// @returns the text name
std::string TextureDimensionToString(tint::inspector::ResourceBinding::TextureDimension dim);

/// @param kind the sample kind
/// @returns the text name
std::string SampledKindToString(tint::inspector::ResourceBinding::SampledKind kind);

/// @param type the sampler type
/// @returns the text name
std::string SamplerTypeToString(tint::inspector::ResourceBinding::SamplerType type);

/// @param format the texel format
/// @returns the text name
std::string TexelFormatToString(tint::inspector::ResourceBinding::TexelFormat format);

/// @param type the resource type
/// @returns the text name
std::string ResourceTypeToString(tint::inspector::ResourceBinding::ResourceType type);

/// @param type the composition type
/// @return the text name
std::string CompositionTypeToString(tint::inspector::CompositionType type);

/// @param type the component type
/// @return the text name
std::string ComponentTypeToString(tint::inspector::ComponentType type);

/// @param type the interpolation sampling type
/// @return the text name
std::string InterpolationSamplingToString(tint::inspector::InterpolationSampling type);

/// @param type the interpolation type
/// @return the text name
std::string InterpolationTypeToString(tint::inspector::InterpolationType type);

/// @param type the override type
/// @return the text name
std::string OverrideTypeToString(tint::inspector::Override::Type type);

/// Returns true if the given `name` is either empty or `-` which signifies `stdout` is selected.
bool IsStdout(const std::string& name);

/// Returns true if the given `name` is either empty or `-` which signifies `stdin` is selected.
bool IsStdin(const std::string& name);

/// On Windows, sets the standard input stream stdin into binary mode. Non-Windows platforms are
/// not affected. If stdin is in text mode reading the byte 0x1A might be interpreted as EOF signal.
void SetStdinModeBinary();

/// Writes the given `buffer` to standard output. If any error occurs, returns false and outputs
/// error message to standard error. The ContainerT type must have data() and size() methods, like
/// `std::string` and `std::vector` do.
/// @returns true on success
/// @private
template <typename ContainerT>
bool WriteStdoutImpl(const ContainerT& buffer) {
    size_t written =
        fwrite(buffer.data(), sizeof(typename ContainerT::value_type), buffer.size(), stdout);
    if (buffer.size() != written) {
        std::cerr << "Could not write all output to standard output\n";
        return false;
    }
    fflush(stdout);
    return true;
}

/// Writes the given `buffer` into the file named as `output_file` using the given `mode`. If any
/// error occurs, returns false and outputs error message to standard error. The ContainerT type
/// must have data() and size() methods, like `std::string` and `std::vector` do.
/// @returns true on success
/// @private
template <typename ContainerT>
bool WriteFileImpl(const std::string& output_file,
                   const std::string mode,
                   const ContainerT& buffer) {
    FILE* file = nullptr;

#if defined(_MSC_VER)
    fopen_s(&file, output_file.c_str(), mode.c_str());
#else
    file = fopen(output_file.c_str(), mode.c_str());
#endif
    if (!file) {
        std::cerr << "Could not open file " << output_file << " for writing\n";
        return false;
    }

    size_t written =
        fwrite(buffer.data(), sizeof(typename ContainerT::value_type), buffer.size(), file);
    if (buffer.size() != written) {
        std::cerr << "Could not write to file " << output_file << "\n";
        fclose(file);
        return false;
    }
    fclose(file);

    return true;
}

/// Writes the given `buffer` into the file named as `output_file` using the
/// given `mode`.  If `output_file` is empty or "-", writes to standard
/// output. If any error occurs, returns false and outputs error message to
/// standard error. The ContainerT type must have data() and size() methods,
/// like `std::string` and `std::vector` do.
/// @returns true on success
template <typename ContainerT>
bool WriteFile(const std::string& output_file, const std::string mode, const ContainerT& buffer) {
    if (IsStdout(output_file)) {
        return WriteStdoutImpl(buffer);
    }
    return WriteFileImpl(output_file, mode, buffer);
}

/// Copies the content from the file named `input_file` to `buffer`,
/// assuming each element in the file is of type `T`.  If any error occurs,
/// writes error messages to the standard error stream and returns false.
/// Assumes the size of a `T` object is divisible by its required alignment.
/// @returns true if we successfully read the file.
template <typename T>
bool ReadFileImpl(const std::string& input_file, std::vector<T>* buffer) {
    FILE* file = nullptr;
#if defined(_MSC_VER)
    fopen_s(&file, input_file.c_str(), "rb");
#else
    file = fopen(input_file.c_str(), "rb");
#endif
    if (!file) {
        std::cerr << "Failed to open " << input_file << "\n";
        return false;
    }

    fseek(file, 0, SEEK_END);
    const auto file_size = static_cast<size_t>(ftell(file));
    if (0 != (file_size % sizeof(T))) {
        std::cerr << "File " << input_file
                  << " does not contain an integral number of objects: " << file_size
                  << " bytes in the file, require " << sizeof(T) << " bytes per object\n";
        fclose(file);
        return false;
    }
    fseek(file, 0, SEEK_SET);

    buffer->clear();
    buffer->resize(file_size / sizeof(T));

    size_t bytes_read = fread(buffer->data(), 1, file_size, file);
    fclose(file);
    if (bytes_read != file_size) {
        std::cerr << "Failed to read " << input_file << "\n";
        return false;
    }

    return true;
}

/// Copies the content from stdin to `buffer`,
/// assuming each element in the stream is of type `T`.
/// writes error messages to the standard error stream and returns false.
/// Assumes the size of a `T` object is divisible by its required alignment.
/// @returns true if we successfully read from stream.
template <typename T>
bool ReadStdinImpl(std::vector<T>* buffer) {
    buffer->clear();
    SetStdinModeBinary();
    constexpr size_t kItemsPerChunk = 1024;
    constexpr size_t kBytesPerChunk = sizeof(T) * kItemsPerChunk;
    std::vector<T> chunk(kItemsPerChunk);
    while (!std::feof(stdin)) {
        size_t bytes_read = std::fread(chunk.data(), 1, kBytesPerChunk, stdin);
        if (bytes_read == 0) {
            if (std::ferror(stdin)) {
                std::perror("Error reading from standard input");
                return false;
            }
            // EOF
            break;
        }
        if (bytes_read % sizeof(T) != 0) {
            std::cerr << "Encountered non-aligned read from input stream, read '" << bytes_read
                      << "' bytes, which is not an integral number of objects of size '"
                      << sizeof(T) << "'\n";
            return false;
        }
        // std::min for extra safety
        size_t items_read = std::min(chunk.size(), bytes_read / sizeof(T));
        buffer->reserve(buffer->size() + items_read);
        std::copy(chunk.begin(), chunk.begin() + int32_t(items_read), std::back_inserter(*buffer));
    }
    return true;
}

/// Copies the content from the file named `input_file` to `buffer`.
/// If `input_file` is empty or "-", reads from standard input.
/// assuming each element in the file is of type `T`.  If any error occurs,
/// writes error messages to the standard error stream and returns false.
/// @returns true on success
template <typename T>
bool ReadFile(const std::string& input_file, std::vector<T>* buffer) {
    if (!buffer) {
        std::cerr << "The buffer pointer was null\n";
        return false;
    }

    if (IsStdin(input_file)) {
        return ReadStdinImpl(buffer);
    }
    return ReadFileImpl(input_file, buffer);
}

/// @param str the string to quote
/// @returns @p str quoted with <code>'</code>
inline std::string Quote(std::string_view str) {
    return "'" + std::string(str) + "'";
}

}  // namespace tint::cmd

#endif  // SRC_TINT_CMD_COMMON_HELPER_H_
