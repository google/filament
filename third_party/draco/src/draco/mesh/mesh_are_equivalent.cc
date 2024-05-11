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
#include "draco/mesh/mesh_are_equivalent.h"

#include <algorithm>

namespace draco {

void MeshAreEquivalent::PrintPosition(const Mesh &mesh, FaceIndex f,
                                      int32_t c) {
  fprintf(stderr, "Printing position for (%i,%i)\n", f.value(), c);
  const auto pos_att = mesh.GetNamedAttribute(GeometryAttribute::POSITION);
  const PointIndex ver_index = mesh.face(f)[c];
  const AttributeValueIndex pos_index = pos_att->mapped_index(ver_index);
  const auto pos = pos_att->GetValue<float, 3>(pos_index);
  fprintf(stderr, "Position (%f,%f,%f)\n", pos[0], pos[1], pos[2]);
}

Vector3f MeshAreEquivalent::GetPosition(const Mesh &mesh, FaceIndex f,
                                        int32_t c) {
  const auto pos_att = mesh.GetNamedAttribute(GeometryAttribute::POSITION);
  const PointIndex ver_index = mesh.face(f)[c];
  const AttributeValueIndex pos_index = pos_att->mapped_index(ver_index);
  const auto pos = pos_att->GetValue<float, 3>(pos_index);
  return Vector3f(pos[0], pos[1], pos[2]);
}

void MeshAreEquivalent::InitCornerIndexOfSmallestPointXYZ() {
  DRACO_DCHECK_EQ(mesh_infos_[0].corner_index_of_smallest_vertex.size(), 0);
  DRACO_DCHECK_EQ(mesh_infos_[1].corner_index_of_smallest_vertex.size(), 0);
  for (int i = 0; i < 2; ++i) {
    mesh_infos_[i].corner_index_of_smallest_vertex.reserve(num_faces_);
    for (FaceIndex f(0); f < num_faces_; ++f) {
      mesh_infos_[i].corner_index_of_smallest_vertex.push_back(
          ComputeCornerIndexOfSmallestPointXYZ(mesh_infos_[i].mesh, f));
    }
  }
  DRACO_DCHECK_EQ(mesh_infos_[0].corner_index_of_smallest_vertex.size(),
                  num_faces_);
  DRACO_DCHECK_EQ(mesh_infos_[1].corner_index_of_smallest_vertex.size(),
                  num_faces_);
}

void MeshAreEquivalent::InitOrderedFaceIndex() {
  DRACO_DCHECK_EQ(mesh_infos_[0].ordered_index_of_face.size(), 0);
  DRACO_DCHECK_EQ(mesh_infos_[1].ordered_index_of_face.size(), 0);
  for (int32_t i = 0; i < 2; ++i) {
    mesh_infos_[i].ordered_index_of_face.reserve(num_faces_);
    for (FaceIndex j(0); j < num_faces_; ++j) {
      mesh_infos_[i].ordered_index_of_face.push_back(j);
    }
    const FaceIndexLess less(mesh_infos_[i]);
    std::sort(mesh_infos_[i].ordered_index_of_face.begin(),
              mesh_infos_[i].ordered_index_of_face.end(), less);

    DRACO_DCHECK_EQ(mesh_infos_[i].ordered_index_of_face.size(), num_faces_);
    DRACO_DCHECK(std::is_sorted(mesh_infos_[i].ordered_index_of_face.begin(),
                                mesh_infos_[i].ordered_index_of_face.end(),
                                less));
  }
}

int32_t MeshAreEquivalent::ComputeCornerIndexOfSmallestPointXYZ(
    const Mesh &mesh, FaceIndex f) {
  Vector3f pos[3];  // For the three corners.
  for (int32_t i = 0; i < 3; ++i) {
    pos[i] = GetPosition(mesh, f, i);
  }
  const auto min_it = std::min_element(pos, pos + 3);
  return static_cast<int32_t>(min_it - pos);
}

void MeshAreEquivalent::Init(const Mesh &mesh0, const Mesh &mesh1) {
  mesh_infos_.clear();
  DRACO_DCHECK_EQ(mesh_infos_.size(), 0);

  num_faces_ = mesh1.num_faces();
  mesh_infos_.push_back(MeshInfo(mesh0));
  mesh_infos_.push_back(MeshInfo(mesh1));

  DRACO_DCHECK_EQ(mesh_infos_.size(), 2);
  DRACO_DCHECK_EQ(mesh_infos_[0].corner_index_of_smallest_vertex.size(), 0);
  DRACO_DCHECK_EQ(mesh_infos_[1].corner_index_of_smallest_vertex.size(), 0);
  DRACO_DCHECK_EQ(mesh_infos_[0].ordered_index_of_face.size(), 0);
  DRACO_DCHECK_EQ(mesh_infos_[1].ordered_index_of_face.size(), 0);

  InitCornerIndexOfSmallestPointXYZ();
  InitOrderedFaceIndex();
}

bool MeshAreEquivalent::operator()(const Mesh &mesh0, const Mesh &mesh1) {
  if (mesh0.num_faces() != mesh1.num_faces()) {
    return false;
  }
  if (mesh0.num_attributes() != mesh1.num_attributes()) {
    return false;
  }

  // The following function inits mesh info, i.e., computes the order of
  // faces with respect to the lex order. This way one can then compare the
  // the two meshes face by face. It also determines the first corner of each
  // face with respect to lex order.
  Init(mesh0, mesh1);

  // Check for every attribute that is valid that every corner is identical.
  typedef GeometryAttribute::Type AttributeType;
  const int att_max = AttributeType::NAMED_ATTRIBUTES_COUNT;
  for (int att_id = 0; att_id < att_max; ++att_id) {
    // First check for existence of the attribute in both meshes.
    const PointAttribute *const att0 =
        mesh0.GetNamedAttribute(AttributeType(att_id));
    const PointAttribute *const att1 =
        mesh1.GetNamedAttribute(AttributeType(att_id));
    if (att0 == nullptr && att1 == nullptr) {
      continue;
    }
    if (att0 == nullptr) {
      return false;
    }
    if (att1 == nullptr) {
      return false;
    }
    if (att0->data_type() != att1->data_type()) {
      return false;
    }
    if (att0->num_components() != att1->num_components()) {
      return false;
    }
    if (att0->normalized() != att1->normalized()) {
      return false;
    }
    if (att0->byte_stride() != att1->byte_stride()) {
      return false;
    }

    DRACO_DCHECK(att0->IsValid());
    DRACO_DCHECK(att1->IsValid());

    // Prepare blocks of memory to hold data of corners for this attribute.
    std::unique_ptr<uint8_t[]> data0(new uint8_t[att0->byte_stride()]);
    std::unique_ptr<uint8_t[]> data1(new uint8_t[att0->byte_stride()]);

    // Check every corner of every face.
    for (int i = 0; i < num_faces_; ++i) {
      const FaceIndex f0 = mesh_infos_[0].ordered_index_of_face[i];
      const FaceIndex f1 = mesh_infos_[1].ordered_index_of_face[i];
      const int c0_off = mesh_infos_[0].corner_index_of_smallest_vertex[f0];
      const int c1_off = mesh_infos_[1].corner_index_of_smallest_vertex[f1];

      for (int c = 0; c < 3; ++c) {
        // Get the index of each corner.
        const PointIndex corner0 = mesh0.face(f0)[(c0_off + c) % 3];
        const PointIndex corner1 = mesh1.face(f1)[(c1_off + c) % 3];
        // Map it to the right index for that attribute.
        const AttributeValueIndex index0 = att0->mapped_index(corner0);
        const AttributeValueIndex index1 = att1->mapped_index(corner1);

        // Obtaining the data.
        att0->GetValue(index0, data0.get());
        att1->GetValue(index1, data1.get());
        // Compare the data as is in memory.
        if (memcmp(data0.get(), data1.get(), att0->byte_stride()) != 0) {
          return false;
        }
      }
    }
  }
  return true;
}

bool MeshAreEquivalent::FaceIndexLess::operator()(FaceIndex f0,
                                                  FaceIndex f1) const {
  if (f0 == f1) {
    return false;
  }
  const int c0 = mesh_info.corner_index_of_smallest_vertex[f0];
  const int c1 = mesh_info.corner_index_of_smallest_vertex[f1];

  for (int i = 0; i < 3; ++i) {
    const Vector3f vf0 = GetPosition(mesh_info.mesh, f0, (c0 + i) % 3);
    const Vector3f vf1 = GetPosition(mesh_info.mesh, f1, (c1 + i) % 3);
    if (vf0 < vf1) {
      return true;
    }
    if (vf1 < vf0) {
      return false;
    }
  }
  // In case the two faces are equivalent.
  return false;
}

}  // namespace draco
