// Copyright 2022 The Draco Authors.
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
#include "draco/io/stl_decoder.h"

#include <string>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace draco {

class StlDecoderTest : public ::testing::Test {
 protected:
  void test_decoding(const std::string &file_name) {
    const std::string path = GetTestFileFullPath(file_name);
    StlDecoder decoder;
    DRACO_ASSIGN_OR_ASSERT(std::unique_ptr<Mesh> mesh,
                           decoder.DecodeFromFile(path));
    ASSERT_GT(mesh->num_faces(), 0);
    ASSERT_GT(mesh->num_points(), 0);
  }

  void test_decoding_should_fail(const std::string &file_name) {
    StlDecoder decoder;
    StatusOr<std::unique_ptr<Mesh>> statusOrMesh =
        decoder.DecodeFromFile(GetTestFileFullPath(file_name));
    ASSERT_FALSE(statusOrMesh.ok());
  }
};

TEST_F(StlDecoderTest, TestStlDecoding) {
  test_decoding("STL/bunny.stl");
  test_decoding("STL/test_sphere.stl");
  test_decoding_should_fail("STL/test_sphere_ascii.stl");
}

}  // namespace draco
