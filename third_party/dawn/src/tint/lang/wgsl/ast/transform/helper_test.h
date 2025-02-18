// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_HELPER_TEST_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_HELPER_TEST_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"

namespace tint::ast::transform {

/// @param program the program to get an output WGSL string from
/// @returns the output program as a WGSL string, or an error string if the
/// program is not valid.
inline std::string str(const Program& program) {
    if (!program.IsValid()) {
        return program.Diagnostics().Str();
    }

    wgsl::writer::Options options;
    auto result = wgsl::writer::Generate(program, options);
    if (result != Success) {
        return result.Failure().reason.Str();
    }

    auto res = result->wgsl;
    if (res.empty()) {
        return res;
    }
    // The WGSL sometimes has two trailing newlines. Strip them
    while (res.back() == '\n') {
        res.pop_back();
    }
    if (res.empty()) {
        return res;
    }
    return "\n" + res + "\n";
}

/// Helper class for testing transforms
template <typename BASE>
class TransformTestBase : public BASE {
  public:
    /// Transforms and returns the WGSL source `in`, transformed using
    /// `transform`.
    /// @param transform the transform to apply
    /// @param in the input WGSL source
    /// @param data the optional DataMap to pass to Transform::Run()
    /// @return the transformed output
    Output Run(std::string in,
               std::unique_ptr<transform::Transform> transform,
               const DataMap& data = {}) {
        std::vector<std::unique_ptr<transform::Transform>> transforms;
        transforms.emplace_back(std::move(transform));
        return Run(std::move(in), std::move(transforms), data);
    }

    /// Transforms and returns the WGSL source `in`, transformed using
    /// a transform of type `TRANSFORM`.
    /// @param in the input WGSL source
    /// @param data the optional DataMap to pass to Transform::Run()
    /// @return the transformed output
    template <typename... TRANSFORMS>
    Output Run(std::string in, const DataMap& data = {}) {
        wgsl::reader::Options options;
        options.allowed_features = wgsl::AllowedFeatures::Everything();
        auto file = std::make_unique<Source::File>("test", in);
        auto program = wgsl::reader::Parse(file.get(), options);

        // Keep this pointer alive after Transform() returns
        files_.emplace_back(std::move(file));

        return Run<TRANSFORMS...>(std::move(program), data);
    }

    /// Transforms and returns program `program`, transformed using a transform of
    /// type `TRANSFORM`.
    /// @param program the input Program
    /// @param data the optional DataMap to pass to Transform::Run()
    /// @return the transformed output
    template <typename... TRANSFORMS>
    Output Run(Program&& program, const DataMap& data = {}) {
        if (!program.IsValid()) {
            return Output(std::move(program));
        }

        Manager manager;
        DataMap outputs;
        for (auto* transform_ptr : std::initializer_list<Transform*>{new TRANSFORMS()...}) {
            manager.append(std::unique_ptr<Transform>(transform_ptr));
        }
        auto result = manager.Run(program, data, outputs);
        return {std::move(result), std::move(outputs)};
    }

    /// @param program the input program
    /// @param data the optional DataMap to pass to Transform::Run()
    /// @return true if the transform should be run for the given input.
    template <typename TRANSFORM>
    bool ShouldRun(Program&& program, const DataMap& data = {}) {
        if (!program.IsValid()) {
            ADD_FAILURE() << "ShouldRun() called with invalid program: " << program.Diagnostics();
            return false;
        }

        const Transform& t = TRANSFORM();

        DataMap outputs;
        auto result = t.Apply(program, data, outputs);
        if (!result) {
            return false;
        }
        if (!result->IsValid()) {
            ADD_FAILURE() << "Apply() called by ShouldRun() returned errors: "
                          << result->Diagnostics();
            return true;
        }
        return result.has_value();
    }

    /// @param in the input WGSL source
    /// @param data the optional DataMap to pass to Transform::Run()
    /// @return true if the transform should be run for the given input.
    template <typename TRANSFORM>
    bool ShouldRun(std::string in, const DataMap& data = {}) {
        wgsl::reader::Options options;
        options.allowed_features = wgsl::AllowedFeatures::Everything();
        auto file = std::make_unique<Source::File>("test", in);
        auto program = wgsl::reader::Parse(file.get(), options);
        return ShouldRun<TRANSFORM>(std::move(program), data);
    }

    /// @param output the output of the transform
    /// @returns the output program as a WGSL string, or an error string if the
    /// program is not valid.
    std::string str(const Output& output) { return transform::str(output.program); }

  private:
    std::vector<std::unique_ptr<Source::File>> files_;
};

using TransformTest = TransformTestBase<testing::Test>;

template <typename T>
using TransformTestWithParam = TransformTestBase<testing::TestWithParam<T>>;

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_HELPER_TEST_H_
