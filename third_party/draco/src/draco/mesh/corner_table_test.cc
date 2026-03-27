#include "draco/mesh/corner_table.h"

#include <memory>

#include "draco/core/draco_test_utils.h"
#include "draco/io/obj_decoder.h"
#include "draco/mesh/mesh_connected_components.h"
#include "draco/mesh/mesh_misc_functions.h"

namespace draco {

class CornerTableTest : public ::testing::Test {
 protected:
  std::unique_ptr<Mesh> DecodeObj(const std::string &file_name) const {
    const std::string path = GetTestFileFullPath(file_name);
    ObjDecoder decoder;
    std::unique_ptr<Mesh> mesh(new Mesh());
    if (!decoder.DecodeFromFile(path, mesh.get()).ok()) {
      return nullptr;
    }
    return mesh;
  }

  void TestEncodingCube() {
    // Build a CornerTable looking at the mesh and verify that the caching of
    // valences are reasonably correct and within range of expectations. This
    // test is built specifically for working with 'cubes' and has expectations
    // about the degree of each corner.
    const std::string file_name = "cube_att.obj";
    std::unique_ptr<draco::Mesh> in_mesh = DecodeObj(file_name);
    ASSERT_NE(in_mesh, nullptr) << "Failed to load test model " << file_name;
    draco::Mesh *mesh = nullptr;
    mesh = in_mesh.get();

    std::unique_ptr<CornerTable> utable =
        draco::CreateCornerTableFromPositionAttribute(mesh);
    draco::CornerTable *table = utable.get();

    table->GetValenceCache().CacheValences();
    table->GetValenceCache().CacheValencesInaccurate();

    for (VertexIndex index = static_cast<VertexIndex>(0);
         index < static_cast<VertexIndex>(table->num_vertices()); index++) {
      const auto valence = table->Valence(index);
      const auto valence2 = table->GetValenceCache().ValenceFromCache(index);
      const auto valence3 =
          table->GetValenceCache().ValenceFromCacheInaccurate(index);
      ASSERT_EQ(valence, valence2);
      ASSERT_GE(valence, valence3);  // may be clipped.

      // No more than 6 triangles can touch a cube corner.
      ASSERT_LE(valence, 6);
      ASSERT_LE(valence2, 6);
      ASSERT_LE(valence3, 6);

      // No less than 3 triangles can touch a cube corner.
      ASSERT_GE(valence, 3);
      ASSERT_GE(valence2, 3);
      ASSERT_GE(valence3, 3);
    }

    for (CornerIndex index = static_cast<CornerIndex>(0);
         index < static_cast<CornerIndex>(table->num_corners()); index++) {
      const auto valence = table->Valence(index);
      const auto valence2 = table->GetValenceCache().ValenceFromCache(index);
      const auto valence3 =
          table->GetValenceCache().ValenceFromCacheInaccurate(index);
      ASSERT_EQ(valence, valence2);
      ASSERT_GE(valence, valence3);  // may be clipped.

      // No more than 6 triangles can touch a cube corner, 6 edges result.
      ASSERT_LE(valence, 6);
      ASSERT_LE(valence2, 6);
      ASSERT_LE(valence3, 6);

      // No less than 3 triangles can touch a cube corner, 3 edges result.
      ASSERT_GE(valence, 3);
      ASSERT_GE(valence2, 3);
      ASSERT_GE(valence3, 3);
    }

    table->GetValenceCache().ClearValenceCache();
    table->GetValenceCache().ClearValenceCacheInaccurate();
  }
};

TEST_F(CornerTableTest, NormalWithSeams) { TestEncodingCube(); }

TEST_F(CornerTableTest, TestNonManifoldEdges) {
  std::unique_ptr<Mesh> mesh = DecodeObj("non_manifold_wrap.obj");
  ASSERT_NE(mesh, nullptr);
  std::unique_ptr<CornerTable> ct =
      draco::CreateCornerTableFromPositionAttribute(mesh.get());
  ASSERT_NE(ct, nullptr);

  MeshConnectedComponents connected_components;
  connected_components.FindConnectedComponents(ct.get());
  ASSERT_EQ(connected_components.NumConnectedComponents(), 2);
}

TEST_F(CornerTableTest, TestNewFace) {
  // Tests that we can add a new face to the corner table.
  const std::string file_name = "cube_att.obj";
  std::unique_ptr<draco::Mesh> mesh = DecodeObj(file_name);
  ASSERT_NE(mesh, nullptr);

  std::unique_ptr<CornerTable> ct =
      draco::CreateCornerTableFromPositionAttribute(mesh.get());
  ASSERT_NE(ct, nullptr);
  ASSERT_EQ(ct->num_faces(), 12);
  ASSERT_EQ(ct->num_corners(), 3 * 12);
  ASSERT_EQ(ct->num_vertices(), 8);

  const VertexIndex new_vi = ct->AddNewVertex();
  ASSERT_EQ(ct->num_vertices(), 9);

  ASSERT_EQ(ct->AddNewFace({VertexIndex(6), VertexIndex(7), new_vi}), 12);
  ASSERT_EQ(ct->num_faces(), 13);
  ASSERT_EQ(ct->num_corners(), 3 * 13);

  ASSERT_EQ(ct->Vertex(CornerIndex(3 * 12 + 0)), 6);
  ASSERT_EQ(ct->Vertex(CornerIndex(3 * 12) + 1), 7);
  ASSERT_EQ(ct->Vertex(CornerIndex(3 * 12) + 2), new_vi);
}

}  // namespace draco
