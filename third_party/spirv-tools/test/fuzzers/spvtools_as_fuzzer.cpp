// Copyright (c) 2019 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdint>
#include <cstring>  // memcpy
#include <vector>

#include "source/spirv_target_env.h"
#include "spirv-tools/libspirv.hpp"
#include "test/fuzzers/random_generator.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  spv_target_env target_env = SPV_ENV_UNIVERSAL_1_0;
  if (size > 0) {
    spvtools::fuzzers::RandomGenerator random_gen(data, size);
    target_env = random_gen.GetTargetEnv();
  }

  const spv_context context = spvContextCreate(target_env);
  if (context == nullptr) {
    return 0;
  }

  std::vector<char> contents;
  contents.resize(size);
  memcpy(contents.data(), data, size);

  spv_binary binary = nullptr;
  spv_diagnostic diagnostic = nullptr;
  spvTextToBinaryWithOptions(context, contents.data(), contents.size(),
                             SPV_TEXT_TO_BINARY_OPTION_NONE, &binary,
                             &diagnostic);
  if (diagnostic) {
    spvDiagnosticPrint(diagnostic);
    spvDiagnosticDestroy(diagnostic);
    diagnostic = nullptr;
  }

  if (binary) {
    spvBinaryDestroy(binary);
    binary = nullptr;
  }

  spvTextToBinaryWithOptions(context, contents.data(), contents.size(),
                             SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS,
                             &binary, &diagnostic);
  if (diagnostic) {
    spvDiagnosticPrint(diagnostic);
    spvDiagnosticDestroy(diagnostic);
    diagnostic = nullptr;
  }

  if (binary) {
    spvBinaryDestroy(binary);
    binary = nullptr;
  }

  spvContextDestroy(context);

  return 0;
}
