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

#ifndef SRC_TINT_LANG_WGSL_INSPECTOR_INSPECTOR_RUNNER_TEST_H_
#define SRC_TINT_LANG_WGSL_INSPECTOR_INSPECTOR_RUNNER_TEST_H_

#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "src/tint/lang/wgsl/inspector/inspector.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/utils/diagnostic/source.h"

namespace tint::inspector {

/// Utility class for running shaders in inspector tests
class InspectorRunner {
  public:
    InspectorRunner();
    virtual ~InspectorRunner();

    /// Create a Program with Inspector from the provided WGSL shader.
    /// Should only be called once per test.
    /// @param shader a WGSL shader
    /// @returns a reference to the Inspector for the built Program.
    Inspector& Initialize(std::string shader);

  protected:
    /// File created from input shader and used to create Program.
    std::unique_ptr<Source::File> file_;
    /// Program created by this runner.
    std::unique_ptr<Program> program_;
    /// Inspector for |program_|
    std::unique_ptr<Inspector> inspector_;
};

}  // namespace tint::inspector

#endif  // SRC_TINT_LANG_WGSL_INSPECTOR_INSPECTOR_RUNNER_TEST_H_
