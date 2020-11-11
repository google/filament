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

#include "gtest/gtest.h"

#include <fstream>
#include <iostream>

#include "source/opt/def_use_manager.h"
#include "tools/io.h"

namespace spvtools {
namespace fuzz {

const spvtools::MessageConsumer kConsoleMessageConsumer =
    [](spv_message_level_t level, const char*, const spv_position_t& position,
       const char* message) -> void {
  switch (level) {
    case SPV_MSG_FATAL:
    case SPV_MSG_INTERNAL_ERROR:
    case SPV_MSG_ERROR:
      std::cerr << "error: line " << position.index << ": " << message
                << std::endl;
      break;
    case SPV_MSG_WARNING:
      std::cout << "warning: line " << position.index << ": " << message
                << std::endl;
      break;
    case SPV_MSG_INFO:
      std::cout << "info: line " << position.index << ": " << message
                << std::endl;
      break;
    default:
      break;
  }
};

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

bool IsEqual(const spv_target_env env, const std::vector<uint32_t>& binary_1,
             const opt::IRContext* ir_2) {
  std::vector<uint32_t> binary_2;
  ir_2->module()->ToBinary(&binary_2, false);
  return IsEqual(env, binary_1, binary_2);
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

void DumpTransformationsBinary(
    const protobufs::TransformationSequence& transformations,
    const char* filename) {
  std::ofstream transformations_file;
  transformations_file.open(filename, std::ios::out | std::ios::binary);
  transformations.SerializeToOstream(&transformations_file);
  transformations_file.close();
}

void DumpTransformationsJson(
    const protobufs::TransformationSequence& transformations,
    const char* filename) {
  std::string json_string;
  auto json_options = google::protobuf::util::JsonOptions();
  json_options.add_whitespace = true;
  auto json_generation_status = google::protobuf::util::MessageToJsonString(
      transformations, &json_string, json_options);
  if (json_generation_status == google::protobuf::util::Status::OK) {
    std::ofstream transformations_json_file(filename);
    transformations_json_file << json_string;
    transformations_json_file.close();
  }
}

void ApplyAndCheckFreshIds(
    const Transformation& transformation, opt::IRContext* ir_context,
    TransformationContext* transformation_context,
    const std::unordered_set<uint32_t>& issued_overflow_ids) {
  opt::analysis::DefUseManager::IdToDefMap before_transformation =
      ir_context->get_def_use_mgr()->id_to_defs();
  transformation.Apply(ir_context, transformation_context);
  opt::analysis::DefUseManager::IdToDefMap after_transformation =
      ir_context->get_def_use_mgr()->id_to_defs();
  std::unordered_set<uint32_t> fresh_ids_for_transformation =
      transformation.GetFreshIds();
  for (auto& entry : after_transformation) {
    uint32_t id = entry.first;
    bool introduced_by_transformation_message =
        fresh_ids_for_transformation.count(id);
    bool introduced_by_overflow_ids = issued_overflow_ids.count(id);
    ASSERT_FALSE(introduced_by_transformation_message &&
                 introduced_by_overflow_ids);
    if (before_transformation.count(entry.first)) {
      ASSERT_FALSE(introduced_by_transformation_message ||
                   introduced_by_overflow_ids);
    } else {
      ASSERT_TRUE(introduced_by_transformation_message ||
                  introduced_by_overflow_ids);
    }
  }
}

}  // namespace fuzz
}  // namespace spvtools
