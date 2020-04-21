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
#include "draco/io/obj_decoder.h"

#include <sstream>

#include "draco/core/draco_test_base.h"
#include "draco/core/draco_test_utils.h"

namespace draco {

class ObjDecoderTest : public ::testing::Test {
 protected:
  template <class Geometry>
  std::unique_ptr<Geometry> DecodeObj(const std::string &file_name) const {
    return DecodeObj<Geometry>(file_name, false);
  }

  template <class Geometry>
  std::unique_ptr<Geometry> DecodeObj(const std::string &file_name,
                                      bool deduplicate_input_values) const {
    const std::string path = GetTestFileFullPath(file_name);
    ObjDecoder decoder;
    decoder.set_deduplicate_input_values(deduplicate_input_values);
    std::unique_ptr<Geometry> geometry(new Geometry());
    if (!decoder.DecodeFromFile(path, geometry.get()).ok()) {
      return nullptr;
    }
    return geometry;
  }

  template <class Geometry>
  std::unique_ptr<Geometry> DecodeObjWithMetadata(
      const std::string &file_name) const {
    const std::string path = GetTestFileFullPath(file_name);
    ObjDecoder decoder;
    decoder.set_use_metadata(true);
    std::unique_ptr<Geometry> geometry(new Geometry());
    if (!decoder.DecodeFromFile(path, geometry.get()).ok()) {
      return nullptr;
    }
    return geometry;
  }

  void test_decoding(const std::string &file_name) {
    const std::unique_ptr<Mesh> mesh(DecodeObj<Mesh>(file_name));
    ASSERT_NE(mesh, nullptr) << "Failed to load test model " << file_name;
    ASSERT_GT(mesh->num_faces(), 0);

    const std::unique_ptr<PointCloud> pc(DecodeObj<PointCloud>(file_name));
    ASSERT_NE(pc, nullptr) << "Failed to load test model " << file_name;
    ASSERT_GT(pc->num_points(), 0);
  }
};

TEST_F(ObjDecoderTest, ExtraVertexOBJ) {
  const std::string file_name = "extra_vertex.obj";
  test_decoding(file_name);
}

TEST_F(ObjDecoderTest, PartialAttributesOBJ) {
  const std::string file_name = "cube_att_partial.obj";
  test_decoding(file_name);
}

TEST_F(ObjDecoderTest, SubObjects) {
  // Tests loading an Obj with sub objects.
  const std::string file_name = "cube_att_sub_o.obj";
  const std::unique_ptr<Mesh> mesh(DecodeObj<Mesh>(file_name));
  ASSERT_NE(mesh, nullptr) << "Failed to load test model " << file_name;
  ASSERT_GT(mesh->num_faces(), 0);

  // A sub object attribute should be the fourth attribute of the mesh (in this
  // case).
  ASSERT_EQ(mesh->num_attributes(), 4);
  ASSERT_EQ(mesh->attribute(3)->attribute_type(), GeometryAttribute::GENERIC);
  // There should be 3 different sub objects used in the model.
  ASSERT_EQ(mesh->attribute(3)->size(), 3);
  // Verify that the sub object attribute has unique id == 3.
  ASSERT_EQ(mesh->attribute(3)->unique_id(), 3);
}

TEST_F(ObjDecoderTest, SubObjectsWithMetadata) {
  // Tests loading an Obj with sub objects.
  const std::string file_name = "cube_att_sub_o.obj";
  const std::unique_ptr<Mesh> mesh(DecodeObjWithMetadata<Mesh>(file_name));
  ASSERT_NE(mesh, nullptr) << "Failed to load test model " << file_name;
  ASSERT_GT(mesh->num_faces(), 0);

  ASSERT_EQ(mesh->num_attributes(), 4);
  ASSERT_EQ(mesh->attribute(3)->attribute_type(), GeometryAttribute::GENERIC);
  // There should be 3 different sub objects used in the model.
  ASSERT_EQ(mesh->attribute(3)->size(), 3);

  // Test material names stored in metadata.
  ASSERT_NE(mesh->GetMetadata(), nullptr);
  ASSERT_NE(mesh->GetAttributeMetadataByAttributeId(3), nullptr);
  int32_t sub_obj_id = 0;
  ASSERT_TRUE(mesh->GetAttributeMetadataByAttributeId(3)->GetEntryInt(
      "obj2", &sub_obj_id));
  ASSERT_EQ(sub_obj_id, 2);
}

TEST_F(ObjDecoderTest, QuadOBJ) {
  // Tests loading an Obj with quad faces.
  const std::string file_name = "cube_quads.obj";
  const std::unique_ptr<Mesh> mesh(DecodeObj<Mesh>(file_name));
  ASSERT_NE(mesh, nullptr) << "Failed to load test model " << file_name;
  ASSERT_EQ(mesh->num_faces(), 12);

  ASSERT_EQ(mesh->num_attributes(), 3);
  ASSERT_EQ(mesh->num_points(), 4 * 6);  // Four points per quad face.
}

TEST_F(ObjDecoderTest, ComplexPolyOBJ) {
  // Tests that we fail to load an obj with complex polygon (expected failure).
  const std::string file_name = "invalid/complex_poly.obj";
  const std::unique_ptr<Mesh> mesh(DecodeObj<Mesh>(file_name));
  ASSERT_EQ(mesh, nullptr);
}

TEST_F(ObjDecoderTest, EmptyNameOBJ) {
  // Tests that we load an obj file that has an sub-object defined with an empty
  // name.
  const std::string file_name = "empty_name.obj";
  const std::unique_ptr<Mesh> mesh(DecodeObj<Mesh>(file_name));
  ASSERT_NE(mesh, nullptr);
  ASSERT_EQ(mesh->num_attributes(), 1);
  // Three valid entries in the attribute are expected.
  ASSERT_EQ(mesh->attribute(0)->size(), 3);
}

TEST_F(ObjDecoderTest, PointCloudOBJ) {
  // Tests that we load an obj file that does not contain any faces.
  const std::string file_name = "test_lines.obj";
  const std::unique_ptr<Mesh> mesh(DecodeObj<Mesh>(file_name, false));
  ASSERT_NE(mesh, nullptr);
  ASSERT_EQ(mesh->num_faces(), 0);
  ASSERT_EQ(mesh->num_attributes(), 1);
  ASSERT_EQ(mesh->attribute(0)->size(), 484);
}

TEST_F(ObjDecoderTest, WrongAttributeMapping) {
  // Tests that we load an obj file that contains invalid mapping between
  // attribute indices and values. In such case the invalid indices should be
  // ignored.
  const std::string file_name = "test_wrong_attribute_mapping.obj";
  const std::unique_ptr<Mesh> mesh(DecodeObj<Mesh>(file_name, false));
  ASSERT_NE(mesh, nullptr);
  ASSERT_EQ(mesh->num_faces(), 1);
  ASSERT_EQ(mesh->num_attributes(), 1);
  ASSERT_EQ(mesh->attribute(0)->size(), 3);
}

TEST_F(ObjDecoderTest, TestObjDecodingAll) {
  // test if we can read all obj that are currently in test folder.
  test_decoding("bunny_norm.obj");
  // test_decoding("complex_poly.obj"); // not supported see test above
  test_decoding("cube_att.obj");
  test_decoding("cube_att_partial.obj");
  test_decoding("cube_att_sub_o.obj");
  test_decoding("cube_quads.obj");
  test_decoding("cube_subd.obj");
  test_decoding("eof_test.obj");
  test_decoding("extra_vertex.obj");
  test_decoding("mat_test.obj");
  test_decoding("one_face_123.obj");
  test_decoding("one_face_312.obj");
  test_decoding("one_face_321.obj");
  test_decoding("sphere.obj");
  test_decoding("test_nm.obj");
  test_decoding("test_nm_trans.obj");
  test_decoding("test_sphere.obj");
  test_decoding("three_faces_123.obj");
  test_decoding("three_faces_312.obj");
  test_decoding("two_faces_123.obj");
  test_decoding("two_faces_312.obj");
  test_decoding("inf_nan.obj");
}

}  // namespace draco
