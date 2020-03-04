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
#include "draco/mesh/triangle_soup_mesh_builder.h"

namespace draco {

void TriangleSoupMeshBuilder::Start(int num_faces) {
  mesh_ = std::unique_ptr<Mesh>(new Mesh());
  mesh_->SetNumFaces(num_faces);
  mesh_->set_num_points(num_faces * 3);
  attribute_element_types_.clear();
}

int TriangleSoupMeshBuilder::AddAttribute(
    GeometryAttribute::Type attribute_type, int8_t num_components,
    DataType data_type) {
  GeometryAttribute va;
  va.Init(attribute_type, nullptr, num_components, data_type, false,
          DataTypeLength(data_type) * num_components, 0);
  attribute_element_types_.push_back(-1);
  return mesh_->AddAttribute(va, true, mesh_->num_points());
}

void TriangleSoupMeshBuilder::SetAttributeValuesForFace(
    int att_id, FaceIndex face_id, const void *corner_value_0,
    const void *corner_value_1, const void *corner_value_2) {
  const int start_index = 3 * face_id.value();
  PointAttribute *const att = mesh_->attribute(att_id);
  att->SetAttributeValue(AttributeValueIndex(start_index), corner_value_0);
  att->SetAttributeValue(AttributeValueIndex(start_index + 1), corner_value_1);
  att->SetAttributeValue(AttributeValueIndex(start_index + 2), corner_value_2);
  // TODO(ostava): The below code should be called only for one attribute.
  // It will work OK even for multiple attributes, but it's redundant.
  mesh_->SetFace(face_id,
                 {{PointIndex(start_index), PointIndex(start_index + 1),
                   PointIndex(start_index + 2)}});
  attribute_element_types_[att_id] = MESH_CORNER_ATTRIBUTE;
}

void TriangleSoupMeshBuilder::SetPerFaceAttributeValueForFace(
    int att_id, FaceIndex face_id, const void *value) {
  const int start_index = 3 * face_id.value();
  PointAttribute *const att = mesh_->attribute(att_id);
  att->SetAttributeValue(AttributeValueIndex(start_index), value);
  att->SetAttributeValue(AttributeValueIndex(start_index + 1), value);
  att->SetAttributeValue(AttributeValueIndex(start_index + 2), value);
  mesh_->SetFace(face_id,
                 {{PointIndex(start_index), PointIndex(start_index + 1),
                   PointIndex(start_index + 2)}});
  int8_t &element_type = attribute_element_types_[att_id];
  if (element_type < 0) {
    element_type = MESH_FACE_ATTRIBUTE;
  }
}

std::unique_ptr<Mesh> TriangleSoupMeshBuilder::Finalize() {
#ifdef DRACO_ATTRIBUTE_VALUES_DEDUPLICATION_SUPPORTED
  // First deduplicate attribute values.
  if (!mesh_->DeduplicateAttributeValues()) {
    return nullptr;
  }
#endif
#ifdef DRACO_ATTRIBUTE_INDICES_DEDUPLICATION_SUPPORTED
  // Also deduplicate vertex indices.
  mesh_->DeduplicatePointIds();
#endif
  for (size_t i = 0; i < attribute_element_types_.size(); ++i) {
    if (attribute_element_types_[i] >= 0) {
      mesh_->SetAttributeElementType(
          static_cast<int>(i),
          static_cast<MeshAttributeElementType>(attribute_element_types_[i]));
    }
  }
  return std::move(mesh_);
}

}  // namespace draco
