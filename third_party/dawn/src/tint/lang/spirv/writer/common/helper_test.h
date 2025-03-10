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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_HELPER_TEST_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_HELPER_TEST_H_

#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "spirv-tools/libspirv.hpp"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"
#include "src/tint/lang/spirv/writer/writer.h"

namespace tint::spirv::writer {

using namespace tint::core::number_suffixes;  // NOLINT

// Helper macro to check whether the SPIR-V output contains an instruction, dumping the full output
// if the instruction was not present.
#define EXPECT_INST(inst) ASSERT_THAT(output_, testing::HasSubstr(inst)) << output_

/// The element type of a test.
enum TestElementType {
    kBool,
    kI32,
    kU32,
    kF32,
    kF16,
};
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, TestElementType type) {
    switch (type) {
        case kBool:
            out << "bool";
            break;
        case kI32:
            out << "i32";
            break;
        case kU32:
            out << "u32";
            break;
        case kF32:
            out << "f32";
            break;
        case kF16:
            out << "f16";
            break;
    }
    return out;
}

/// Base helper class for testing the SPIR-V writer implementation.
template <typename BASE>
class SpirvWriterTestHelperBase : public BASE {
  public:
    /// The test module.
    core::ir::Module mod;
    /// The test builder.
    core::ir::Builder b{mod};
    /// The type manager.
    core::type::Manager& ty{mod.Types()};

  protected:
    /// Errors produced during codegen or SPIR-V validation.
    std::string err_;

    /// SPIR-V output.
    std::string output_;

    /// Workgroup info
    Output::WorkgroupInfo workgroup_info;

    /// @returns the error string from the validation
    std::string Error() const { return err_; }

    /// Run the printer on the IR module and validate the result.
    /// @param options the optional writer options to use when raising the IR
    /// storage class with OpConstantNull
    /// @returns true if generation and validation succeeded
    bool Generate(Options options = {}) {
        auto result = writer::Generate(mod, options);
        if (result != Success) {
            err_ = result.Failure().reason.Str();
            return false;
        }

        output_ = Disassemble(result->spirv, SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
                                                 SPV_BINARY_TO_TEXT_OPTION_INDENT |
                                                 SPV_BINARY_TO_TEXT_OPTION_COMMENT);

        if (!Validate(result->spirv)) {
            return false;
        }
        workgroup_info = result->workgroup_info;

        return true;
    }

    /// Validate the generated SPIR-V using the SPIR-V Tools Validator.
    /// @param binary the SPIR-V binary module to validate
    /// @returns true if validation succeeded, false otherwise
    bool Validate(const std::vector<uint32_t>& binary) {
        std::string spv_errors;
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

        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_2);
        tools.SetMessageConsumer(msg_consumer);

        auto result = tools.Validate(binary);
        err_ = std::move(spv_errors);
        return result;
    }

    /// Helper to make a scalar type corresponding to the element type `type`.
    /// @param type the element type
    /// @returns the scalar type
    const core::type::Type* MakeScalarType(TestElementType type) {
        switch (type) {
            case kBool:
                return ty.bool_();
            case kI32:
                return ty.i32();
            case kU32:
                return ty.u32();
            case kF32:
                return ty.f32();
            case kF16:
                return ty.f16();
        }
        return nullptr;
    }

    /// Helper to make a vector type corresponding to the element type `type`.
    /// @param type the element type
    /// @returns the vector type
    const core::type::Type* MakeVectorType(TestElementType type) {
        return ty.vec2(MakeScalarType(type));
    }

    /// Helper to make a scalar value with the scalar type `type`.
    /// @param type the element type
    /// @param value the optional value to use
    /// @returns the scalar value
    core::ir::Constant* MakeScalarValue(TestElementType type, uint32_t value = 1) {
        switch (type) {
            case kBool:
                return b.Constant(true);
            case kI32:
                return b.Constant(core::i32(value));
            case kU32:
                return b.Constant(core::u32(value));
            case kF32:
                return b.Constant(core::f32(value));
            case kF16:
                return b.Constant(core::f16(value));
        }
        return nullptr;
    }

    /// Helper to make a vector value with an element type of `type`.
    /// @param type the element type
    /// @returns the vector value
    core::ir::Constant* MakeVectorValue(TestElementType type) {
        switch (type) {
            case kBool:
                return b.Composite(MakeVectorType(type), true, false);
            case kI32:
                return b.Composite(MakeVectorType(type), 42_i, -10_i);
            case kU32:
                return b.Composite(MakeVectorType(type), 42_u, 10_u);
            case kF32:
                return b.Composite(MakeVectorType(type), 42_f, -0.5_f);
            case kF16:
                return b.Composite(MakeVectorType(type), 42_h, -0.5_h);
        }
        return nullptr;
    }

    /// Helper to dump the disassembly of the Tint IR module.
    /// @returns the disassembly (with a leading newline)
    std::string IR() { return "\n" + core::ir::Disassembler(mod).Plain(); }
};

using SpirvWriterTest = SpirvWriterTestHelperBase<testing::Test>;

template <typename T>
using SpirvWriterTestWithParam = SpirvWriterTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_HELPER_TEST_H_
