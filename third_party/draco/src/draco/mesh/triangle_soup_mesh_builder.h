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
#ifndef DRACO_MESH_TRIANGLE_SOUP_MESH_BUILDER_H_
#define DRACO_MESH_TRIANGLE_SOUP_MESH_BUILDER_H_

#include "draco/draco_features.h"
#include "draco/mesh/mesh.h"

namespace draco {

// Class for building meshes directly from attribute values that can be
// specified for each face corner. All attributes are automatically
// deduplicated.
class TriangleSoupMeshBuilder {
 public:
  // Starts mesh building for a given number of faces.
  // TODO(ostava): Currently it's necessary to select the correct number of
  // faces upfront. This should be generalized, but it will require us to
  // rewrite our attribute resizing functions.
  void Start(int num_faces);

  // Adds an empty attribute to the mesh. Returns the new attribute's id.
  int AddAttribute(GeometryAttribute::Type attribute_type,
                   int8_t num_components, DataType data_type);

  // Sets values for a given attribute on all corners of a given face.
  void SetAttributeValuesForFace(int att_id, FaceIndex face_id,
                                 const void *corner_value_0,
                                 const void *corner_value_1,
                                 const void *corner_value_2);

  // Sets value for a per-face attribute. If all faces of a given attribute are
  // set with this method, the attribute will be marked as per-face, otherwise
  // it will be marked as per-corner attribute.
  void SetPerFaceAttributeValueForFace(int att_id, FaceIndex face_id,
                                       const void *value);

  // Finalizes the mesh or returns nullptr on error.
  // Once this function is called, the builder becomes invalid and cannot be
  // used until the method Start() is called again.
  std::unique_ptr<Mesh> Finalize();

 private:
  std::vector<int8_t> attribute_element_types_;

  std::unique_ptr<Mesh> mesh_;
};

}  // namespace draco

#endif  // DRACO_MESH_TRIANGLE_SOUP_MESH_BUILDER_H_
