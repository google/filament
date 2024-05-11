// Copyright 2016 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "draco/core/draco_test_utils.h"

#include <fstream>

#include "draco/core/macros.h"
#include "draco/io/file_utils.h"
#include "draco_test_base.h"

namespace draco {

namespace {
static constexpr char kTestDataDir[] = DRACO_TEST_DATA_DIR;
static constexpr char kTestTempDir[] = DRACO_TEST_TEMP_DIR;
}  // namespace

std::string GetTestFileFullPath(const std::string &file_name) {
  return std::string(kTestDataDir) + std::string("/") + file_name;
}

std::string GetTestTempFileFullPath(const std::string &file_name) {
  return std::string(kTestTempDir) + std::string("/") + file_name;
}

bool GenerateGoldenFile(const std::string &golden_file_name, const void *data,
                        int data_size) {
  const std::string path = GetTestFileFullPath(golden_file_name);
  return WriteBufferToFile(data, data_size, path);
}

bool CompareGoldenFile(const std::string &golden_file_name, const void *data,
                       int data_size) {
  const std::string golden_path = GetTestFileFullPath(golden_file_name);
  std::ifstream in_file(golden_path, std::ios::binary);
  if (!in_file || data_size < 0) {
    return false;
  }
  const char *const data_c8 = static_cast<const char *>(data);
  constexpr int buffer_size = 1024;
  char buffer[buffer_size];
  size_t extracted_size = 0;
  size_t remaining_data_size = data_size;
  int offset = 0;
  while ((extracted_size = in_file.read(buffer, buffer_size).gcount()) > 0) {
    if (remaining_data_size <= 0)
      break;  // Input and golden sizes are different.
    size_t size_to_check = extracted_size;
    if (remaining_data_size < size_to_check)
      size_to_check = remaining_data_size;
    for (uint32_t i = 0; i < size_to_check; ++i) {
      if (buffer[i] != data_c8[offset++]) {
        LOG(INFO) << "Test output differed from golden file at byte "
                  << offset - 1;
        return false;
      }
    }
    remaining_data_size -= extracted_size;
  }
  if (remaining_data_size != extracted_size) {
    // Both of these values should be 0 at the end.
    LOG(INFO) << "Test output size differed from golden file size";
    return false;
  }
  return true;
}

}  // namespace draco
