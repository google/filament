// Copyright (c) 2019 Google LLC
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

#include "test/fuzz/fuzz_test_util.h"

#include <iostream>

#include "tools/io.h"

namespace spvtools {
namespace fuzz {

bool IsEqual(const spv_target_env env,
             const std::vector<uint32_t>& expected_binary,
             const std::vector<uint32_t>& actual_binary) {
  if (expected_binary == actual_binary) {
    return true;
  }
  SpirvTools t(env);
  std::string expected_disassembled;
  std::string actual_disassembled;
  if (!t.Disassemble(expected_binary, &expected_disassembled,
                     kFuzzDisassembleOption)) {
    return false;
  }
  if (!t.Disassemble(actual_binary, &actual_disassembled,
                     kFuzzDisassembleOption)) {
    return false;
  }
  // Using expect gives us a string diff if the strings are not the same.
  EXPECT_EQ(expected_disassembled, actual_disassembled);
  // We then return the result of the equality comparison, to be used by an
  // assertion in the test root function.
  return expected_disassembled == actual_disassembled;
}

bool IsEqual(const spv_target_env env, const std::string& expected_text,
             const std::vector<uint32_t>& actual_binary) {
  std::vector<uint32_t> expected_binary;
  SpirvTools t(env);
  if (!t.Assemble(expected_text, &expected_binary, kFuzzAssembleOption)) {
    return false;
  }
  return IsEqual(env, expected_binary, actual_binary);
}

bool IsEqual(const spv_target_env env, const std::string& expected_text,
             const opt::IRContext* actual_ir) {
  std::vector<uint32_t> actual_binary;
  actual_ir->module()->ToBinary(&actual_binary, false);
  return IsEqual(env, expected_text, actual_binary);
}

bool IsEqual(const spv_target_env env, const opt::IRContext* ir_1,
             const opt::IRContext* ir_2) {
  std::vector<uint32_t> binary_1;
  ir_1->module()->ToBinary(&binary_1, false);
  std::vector<uint32_t> binary_2;
  ir_2->module()->ToBinary(&binary_2, false);
  return IsEqual(env, binary_1, binary_2);
}

bool IsValid(spv_target_env env, const opt::IRContext* ir) {
  std::vector<uint32_t> binary;
  ir->module()->ToBinary(&binary, false);
  SpirvTools t(env);
  return t.Validate(binary);
}

std::string ToString(spv_target_env env, const opt::IRContext* ir) {
  std::vector<uint32_t> binary;
  ir->module()->ToBinary(&binary, false);
  return ToString(env, binary);
}

std::string ToString(spv_target_env env, const std::vector<uint32_t>& binary) {
  SpirvTools t(env);
  std::string result;
  t.Disassemble(binary, &result, kFuzzDisassembleOption);
  return result;
}

void DumpShader(opt::IRContext* context, const char* filename) {
  std::vector<uint32_t> binary;
  context->module()->ToBinary(&binary, false);
  DumpShader(binary, filename);
}

void DumpShader(const std::vector<uint32_t>& binary, const char* filename) {
  auto write_file_succeeded =
      WriteFile(filename, "wb", &binary[0], binary.size());
  if (!write_file_succeeded) {
    std::cerr << "Failed to dump shader" << std::endl;
  }
}

}  // namespace fuzz
}  // namespace spvtools
