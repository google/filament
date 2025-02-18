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

#ifndef SRC_TINT_LANG_HLSL_WRITER_HELPER_TEST_H_
#define SRC_TINT_LANG_HLSL_WRITER_HELPER_TEST_H_

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/hlsl/validate/validate.h"
#include "src/tint/lang/hlsl/writer/writer.h"
#include "src/tint/utils/command/command.h"
#include "src/tint/utils/text/string.h"

namespace tint::hlsl::writer {

/// Base helper class for testing the HLSL writer implementation.
template <typename BASE>
class HlslWriterTestHelperBase : public BASE {
  public:
    /// The test module.
    core::ir::Module mod;
    /// The test builder.
    core::ir::Builder b{mod};
    /// The type manager.
    core::type::Manager& ty{mod.Types()};

  protected:
    /// Validation errors
    std::string err_;

    /// Generated HLSL
    Output output_;

    /// Run the writer on the IR module and validate the result.
    /// @returns true if generation and validation succeeded
    bool Generate(Options options = {}) {
        auto result = writer::Generate(mod, options);
        if (result != Success) {
            err_ = result.Failure().reason.Str();
            return false;
        }
        output_ = result.Get();

        const char* dxc_path = validate::kDxcDLLName;
        auto dxc = tint::Command::LookPath(dxc_path);
        if (dxc.Found()) {
            uint32_t hlsl_shader_model = 66;
            bool require_16bit_types = true;

            auto validate_res =
                validate::ValidateUsingDXC(dxc.Path(), output_.hlsl, output_.entry_points,
                                           require_16bit_types, hlsl_shader_model);
            if (validate_res.failed) {
                size_t line_num = 1;

                std::stringstream err;
                err << "DXC was expected to succeed, but failed:\n\n";
                for (auto line : Split(output_.hlsl, "\n")) {
                    err << line_num++ << ": " << line << "\n";
                }
                err << "\n\n" << validate_res.output;

                err_ = err.str();
                return false;
            }
        }

        return true;
    }
};

/// Printer tests
using HlslWriterTest = HlslWriterTestHelperBase<testing::Test>;

/// Printer param tests
template <typename T>
using HlslWriterTestWithParam = HlslWriterTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::hlsl::writer

#endif  // SRC_TINT_LANG_HLSL_WRITER_HELPER_TEST_H_
