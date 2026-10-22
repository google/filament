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
#include "draco/io/stl_encoder.h"

#include <sstream>
#include <string>
#include <utility>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"
#include "draco/io/file_reader_factory.h"
#include "draco/io/file_reader_interface.h"
#include "draco/io/stl_decoder.h"

namespace draco {

class StlEncoderTest : public ::testing::Test {
 protected:
  void CompareMeshes(const Mesh *mesh0, const Mesh *mesh1) {
    ASSERT_EQ(mesh0->num_faces(), mesh1->num_faces());
    ASSERT_EQ(mesh0->num_attributes(), mesh1->num_attributes());
    for (size_t att_id = 0; att_id < mesh0->num_attributes(); ++att_id) {
      ASSERT_EQ(mesh0->attribute(att_id)->attribute_type(),
                mesh1->attribute(att_id)->attribute_type());
      // Normals are recomputed during STL encoding and they may not
      // correspond to the source ones.
      if (mesh0->attribute(att_id)->attribute_type() !=
          GeometryAttribute::NORMAL) {
        ASSERT_EQ(mesh0->attribute(att_id)->size(),
                  mesh1->attribute(att_id)->size());
      }
    }
  }

  // Encode a mesh using the StlEncoder and then decode to verify the encoding.
  std::unique_ptr<Mesh> EncodeAndDecodeMesh(const Mesh *mesh) {
    EncoderBuffer encoder_buffer;
    StlEncoder encoder;
    Status status = encoder.EncodeToBuffer(*mesh, &encoder_buffer);
    if (!status.ok()) {
      return nullptr;
    }

    DecoderBuffer decoder_buffer;
    decoder_buffer.Init(encoder_buffer.data(), encoder_buffer.size());
    StlDecoder decoder;
    StatusOr<std::unique_ptr<Mesh>> status_or_mesh =
        decoder.DecodeFromBuffer(&decoder_buffer);
    if (!status_or_mesh.ok()) {
      return nullptr;
    }
    std::unique_ptr<Mesh> decoded_mesh = std::move(status_or_mesh).value();
    return decoded_mesh;
  }

  void test_encoding(const std::string &file_name) {
    const std::unique_ptr<Mesh> mesh(ReadMeshFromTestFile(file_name, true));

    ASSERT_NE(mesh, nullptr) << "Failed to load test model " << file_name;
    ASSERT_GT(mesh->num_faces(), 0);

    const std::unique_ptr<Mesh> decoded_mesh = EncodeAndDecodeMesh(mesh.get());
    CompareMeshes(mesh.get(), decoded_mesh.get());
  }
};

TEST_F(StlEncoderTest, TestStlEncoding) {
  // Test decoded mesh from encoded stl file stays the same.
  test_encoding("STL/bunny.stl");
  test_encoding("STL/test_sphere.stl");
}

}  // namespace draco
