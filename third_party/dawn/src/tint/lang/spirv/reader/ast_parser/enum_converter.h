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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_PARSER_ENUM_CONVERTER_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_PARSER_ENUM_CONVERTER_H_

#include "spirv/unified1/spirv.h"
#include "spirv/unified1/spirv.hpp11"
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/spirv/reader/ast_parser/fail_stream.h"
#include "src/tint/lang/wgsl/ast/pipeline_stage.h"

namespace tint::spirv::reader::ast_parser {

/// A converter from SPIR-V enums to Tint AST enums.
class EnumConverter {
  public:
    /// Creates a new enum converter.
    /// @param fail_stream the error reporting stream.
    explicit EnumConverter(const FailStream& fail_stream);
    /// Destructor
    ~EnumConverter();

    /// Converts a SPIR-V execution model to a Tint pipeline stage.
    /// On failure, logs an error and returns kNone
    /// @param model the SPIR-V entry point execution model
    /// @returns a Tint AST pipeline stage
    ast::PipelineStage ToPipelineStage(spv::ExecutionModel model);

    /// Converts a SPIR-V storage class to a Tint address space.
    /// On failure, logs an error and returns kNone
    /// @param sc the SPIR-V storage class
    /// @returns a Tint AST address space
    core::AddressSpace ToAddressSpace(const spv::StorageClass sc);

    /// Converts a SPIR-V Builtin value a Tint Builtin.
    /// On failure, logs an error and returns kNone
    /// @param b the SPIR-V builtin
    /// @returns a Tint AST builtin
    core::BuiltinValue ToBuiltin(spv::BuiltIn b);

    /// Converts a possibly arrayed SPIR-V Dim to a Tint texture dimension.
    /// On failure, logs an error and returns kNone
    /// @param dim the SPIR-V Dim value
    /// @param arrayed true if the texture is arrayed
    /// @returns a Tint AST texture dimension
    core::type::TextureDimension ToDim(spv::Dim dim, bool arrayed);

    /// Converts a possibly arrayed SPIR-V Dim to a Tint texture dimension.
    /// On failure, logs an error and returns kNone
    /// @param dim the SPIR-V Dim value
    /// @param arrayed true if the texture is arrayed
    /// @returns a Tint AST texture dimension
    core::type::TextureDimension ToDim(SpvDim dim, bool arrayed) {
        return ToDim(static_cast<spv::Dim>(dim), arrayed);
    }

    /// Converts a SPIR-V Image Format to a TexelFormat
    /// On failure, logs an error and returns kNone
    /// @param fmt the SPIR-V format
    /// @returns a Tint AST format
    core::TexelFormat ToTexelFormat(spv::ImageFormat fmt);

    /// Converts a SPIR-V Image Format to a TexelFormat
    /// On failure, logs an error and returns kNone
    /// @param fmt the SPIR-V format
    /// @returns a Tint AST format
    core::TexelFormat ToTexelFormat(SpvImageFormat fmt) {
        return ToTexelFormat(static_cast<spv::ImageFormat>(fmt));
    }

  private:
    /// Registers a failure and returns a stream for log diagnostics.
    /// @returns a failure stream
    FailStream Fail() { return fail_stream_.Fail(); }

    FailStream fail_stream_;
};

}  // namespace tint::spirv::reader::ast_parser

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_PARSER_ENUM_CONVERTER_H_
