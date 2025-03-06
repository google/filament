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

#ifndef SRC_TINT_LANG_HLSL_WRITER_AST_PRINTER_HELPER_TEST_H_
#define SRC_TINT_LANG_HLSL_WRITER_AST_PRINTER_HELPER_TEST_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/hlsl/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/hlsl/writer/common/options.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/renamer.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::hlsl::writer {

/// Helper class for testing
template <typename BODY>
class TestHelperBase : public BODY, public ProgramBuilder {
  public:
    TestHelperBase() = default;
    ~TestHelperBase() override = default;

    /// @returns the default generator options for SanitizeAndBuild(), if no explicit options are
    /// provided.
    static Options DefaultOptions() {
        Options opts;
        opts.disable_robustness = true;
        return opts;
    }

    /// Builds the program and returns a ASTPrinter from the program.
    /// @note The generator is only built once. Multiple calls to Build() will
    /// return the same ASTPrinter without rebuilding.
    /// @return the built generator
    ASTPrinter& Build() {
        if (gen_) {
            return *gen_;
        }
        if (!IsValid()) {
            ADD_FAILURE() << "ProgramBuilder is not valid: " << Diagnostics();
        }
        program = std::make_unique<Program>(resolver::Resolve(*this));
        if (!program->IsValid()) {
            ADD_FAILURE() << program->Diagnostics();
        }
        gen_ = std::make_unique<ASTPrinter>(*program);
        return *gen_;
    }

    /// Builds the program, runs the program through the HLSL sanitizer
    /// and returns a ASTPrinter from the sanitized program.
    /// @param options The HLSL generator options.
    /// @note The generator is only built once. Multiple calls to Build() will
    /// return the same ASTPrinter without rebuilding.
    /// @return the built generator
    ASTPrinter& SanitizeAndBuild(const Options& options = DefaultOptions()) {
        if (gen_) {
            return *gen_;
        }
        if (!IsValid()) {
            ADD_FAILURE() << "ProgramBuilder is not valid: " << Diagnostics();
        }
        program = std::make_unique<Program>(resolver::Resolve(*this));
        if (!program->IsValid()) {
            ADD_FAILURE() << program->Diagnostics();
        }

        auto sanitized_result = Sanitize(*program, options);
        if (!sanitized_result.program.IsValid()) {
            ADD_FAILURE() << sanitized_result.program.Diagnostics();
        }

        ast::transform::Manager transform_manager;
        ast::transform::DataMap transform_data;
        ast::transform::DataMap outputs;
        transform_data.Add<ast::transform::Renamer::Config>(
            ast::transform::Renamer::Target::kHlslKeywords);
        transform_manager.Add<tint::ast::transform::Renamer>();
        auto result = transform_manager.Run(sanitized_result.program, transform_data, outputs);
        if (!result.IsValid()) {
            ADD_FAILURE() << result.Diagnostics();
        }
        *program = std::move(result);
        gen_ = std::make_unique<ASTPrinter>(*program);
        return *gen_;
    }

    /// The program built with a call to Build()
    std::unique_ptr<Program> program;

  private:
    std::unique_ptr<ASTPrinter> gen_;
};

/// TestHelper the the base class for HLSL writer unit tests.
/// Use this form when you don't need to template any further.
using TestHelper = TestHelperBase<testing::Test>;

/// TestParamHelper the the base class for HLSL unit tests that take a templated
/// parameter.
template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::hlsl::writer

#endif  // SRC_TINT_LANG_HLSL_WRITER_AST_PRINTER_HELPER_TEST_H_
