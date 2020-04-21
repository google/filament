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
#include "draco/compression/mesh/mesh_encoder.h"

#include "draco/compression/expert_encode.h"
#include "draco/core/decoder_buffer.h"
#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/io/obj_decoder.h"

namespace draco {

class MeshEncoderTest : public ::testing::TestWithParam<const char *> {
 protected:
  MeshEncoderTest() {}

  // Fills out_method with id of the encoding method used for the test.
  // Returns false if the encoding method is not set properly.
  bool GetMethod(MeshEncoderMethod *out_method) const {
    if (strcmp(GetParam(), "sequential") == 0) {
      *out_method = MESH_SEQUENTIAL_ENCODING;
      return true;
    }
    if (strcmp(GetParam(), "edgebreaker") == 0) {
      *out_method = MESH_EDGEBREAKER_ENCODING;
      return true;
    }
    return false;
  }
};

TEST_P(MeshEncoderTest, EncodeGoldenMesh) {
  // This test verifies that a given set of meshes are encoded to an expected
  // output. This is useful for catching bugs in code changes that are not
  // supposed to change the encoding.
  // The test is expected to fail when the encoding is modified. In such case,
  // the golden files need to be updated to reflect the changes.
  MeshEncoderMethod method;
  ASSERT_TRUE(GetMethod(&method))
      << "Test is run for an unknown encoding method";

  const std::string file_name = "test_nm.obj";
  std::string golden_file_name = file_name;
  golden_file_name += '.';
  golden_file_name += GetParam();
  golden_file_name += ".1.2.0.drc";
  const std::unique_ptr<Mesh> mesh(ReadMeshFromTestFile(file_name));
  ASSERT_NE(mesh, nullptr) << "Failed to load test model " << file_name;

  ExpertEncoder encoder(*mesh.get());
  encoder.SetEncodingMethod(method);
  encoder.SetAttributeQuantization(0, 20);
  EncoderBuffer buffer;
  ASSERT_TRUE(encoder.EncodeToBuffer(&buffer).ok())
      << "Failed encoding test mesh " << file_name << " with method "
      << GetParam();
  // Check that the encoded mesh was really encoded with the selected method.
  DecoderBuffer decoder_buffer;
  decoder_buffer.Init(buffer.data(), buffer.size());
  decoder_buffer.Advance(8);  // Skip the header to the encoding method id.
  uint8_t encoded_method;
  ASSERT_TRUE(decoder_buffer.Decode(&encoded_method));
  ASSERT_EQ(encoded_method, method);
  if (!FLAGS_update_golden_files) {
    EXPECT_TRUE(
        CompareGoldenFile(golden_file_name, buffer.data(), buffer.size()))
        << "Encoded data is different from the golden file. Please verify that "
           "the encoding works as expected and update the golden file if "
           "necessary (run the test with --update_golden_files flag).";
  } else {
    // Save the files into the local folder.
    EXPECT_TRUE(
        GenerateGoldenFile(golden_file_name, buffer.data(), buffer.size()))
        << "Failed to generate new golden file for " << file_name;
  }
}

INSTANTIATE_TEST_SUITE_P(MeshEncoderTests, MeshEncoderTest,
                         ::testing::Values("sequential", "edgebreaker"));

}  // namespace draco
