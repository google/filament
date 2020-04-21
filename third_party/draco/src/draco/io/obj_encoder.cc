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
#include "draco/io/obj_encoder.h"

#include <memory>

#include "draco/io/file_writer_factory.h"
#include "draco/io/file_writer_interface.h"
#include "draco/metadata/geometry_metadata.h"

namespace draco {

ObjEncoder::ObjEncoder()
    : pos_att_(nullptr),
      tex_coord_att_(nullptr),
      normal_att_(nullptr),
      material_att_(nullptr),
      sub_obj_att_(nullptr),
      out_buffer_(nullptr),
      in_point_cloud_(nullptr),
      in_mesh_(nullptr),
      current_sub_obj_id_(-1),
      current_material_id_(-1) {}

bool ObjEncoder::EncodeToFile(const PointCloud &pc,
                              const std::string &file_name) {
  std::unique_ptr<FileWriterInterface> file =
      FileWriterFactory::OpenWriter(file_name);
  if (!file) {
    return false;  // File could not be opened.
  }
  file_name_ = file_name;
  // Encode the mesh into a buffer.
  EncoderBuffer buffer;
  if (!EncodeToBuffer(pc, &buffer)) {
    return false;
  }
  // Write the buffer into the file.
  file->Write(buffer.data(), buffer.size());
  return true;
}

bool ObjEncoder::EncodeToFile(const Mesh &mesh, const std::string &file_name) {
  in_mesh_ = &mesh;
  return EncodeToFile(static_cast<const PointCloud &>(mesh), file_name);
}

bool ObjEncoder::EncodeToBuffer(const PointCloud &pc,
                                EncoderBuffer *out_buffer) {
  in_point_cloud_ = &pc;
  out_buffer_ = out_buffer;
  if (!EncodeInternal()) {
    return ExitAndCleanup(false);
  }
  return ExitAndCleanup(true);
}

bool ObjEncoder::EncodeToBuffer(const Mesh &mesh, EncoderBuffer *out_buffer) {
  in_mesh_ = &mesh;
  return EncodeToBuffer(static_cast<const PointCloud &>(mesh), out_buffer);
}

bool ObjEncoder::EncodeInternal() {
  pos_att_ = nullptr;
  tex_coord_att_ = nullptr;
  normal_att_ = nullptr;
  material_att_ = nullptr;
  sub_obj_att_ = nullptr;
  current_sub_obj_id_ = -1;
  current_material_id_ = -1;
  if (!GetSubObjects()) {
    return false;
  }
  if (!EncodeMaterialFileName()) {
    return false;
  }
  if (!EncodePositions()) {
    return false;
  }
  if (!EncodeTextureCoordinates()) {
    return false;
  }
  if (!EncodeNormals()) {
    return false;
  }
  if (in_mesh_ && !EncodeFaces()) {
    return false;
  }
  return true;
}

bool ObjEncoder::ExitAndCleanup(bool return_value) {
  in_mesh_ = nullptr;
  in_point_cloud_ = nullptr;
  out_buffer_ = nullptr;
  pos_att_ = nullptr;
  tex_coord_att_ = nullptr;
  normal_att_ = nullptr;
  material_att_ = nullptr;
  sub_obj_att_ = nullptr;
  current_sub_obj_id_ = -1;
  current_material_id_ = -1;
  file_name_.clear();
  return return_value;
}

bool ObjEncoder::GetSubObjects() {
  const GeometryMetadata *pc_metadata = in_point_cloud_->GetMetadata();
  if (!pc_metadata) {
    return true;
  }
  const AttributeMetadata *sub_obj_metadata =
      pc_metadata->GetAttributeMetadataByStringEntry("name", "sub_obj");
  if (!sub_obj_metadata) {
    return true;
  }
  sub_obj_id_to_name_.clear();
  for (const auto &entry : sub_obj_metadata->entries()) {
    // Sub-object id must be int.
    int value = 0;
    if (!entry.second.GetValue(&value)) {
      continue;
    }
    sub_obj_id_to_name_[value] = entry.first;
  }
  sub_obj_att_ = in_point_cloud_->GetAttributeByUniqueId(
      sub_obj_metadata->att_unique_id());
  if (sub_obj_att_ == nullptr || sub_obj_att_->size() == 0) {
    return false;
  }
  return true;
}

bool ObjEncoder::EncodeMaterialFileName() {
  const GeometryMetadata *pc_metadata = in_point_cloud_->GetMetadata();
  const AttributeMetadata *material_metadata = nullptr;
  if (pc_metadata) {
    material_metadata =
        pc_metadata->GetAttributeMetadataByStringEntry("name", "material");
  }
  std::string material_file_name;
  std::string material_full_path;
  if (!material_metadata) {
    return true;
  }
  if (!material_metadata->GetEntryString("file_name", &material_file_name))
    return false;
  buffer()->Encode("mtllib ", 7);
  buffer()->Encode(material_file_name.c_str(), material_file_name.size());
  buffer()->Encode("\n", 1);
  material_id_to_name_.clear();
  for (const auto &entry : material_metadata->entries()) {
    // Material id must be int.
    int value = 0;
    // Found entry that are not material id, e.g. file name as a string.
    if (!entry.second.GetValue(&value)) {
      continue;
    }
    material_id_to_name_[value] = entry.first;
  }
  material_att_ = in_point_cloud_->GetAttributeByUniqueId(
      material_metadata->att_unique_id());
  if (material_att_ == nullptr || material_att_->size() == 0) {
    return false;
  }
  return true;
}

bool ObjEncoder::EncodePositions() {
  const PointAttribute *const att =
      in_point_cloud_->GetNamedAttribute(GeometryAttribute::POSITION);
  if (att == nullptr || att->size() == 0) {
    return false;  // Position attribute must be valid.
  }
  std::array<float, 3> value;
  for (AttributeValueIndex i(0); i < static_cast<uint32_t>(att->size()); ++i) {
    if (!att->ConvertValue<float, 3>(i, &value[0])) {
      return false;
    }
    buffer()->Encode("v ", 2);
    EncodeFloatList(&value[0], 3);
    buffer()->Encode("\n", 1);
  }
  pos_att_ = att;
  return true;
}

bool ObjEncoder::EncodeTextureCoordinates() {
  const PointAttribute *const att =
      in_point_cloud_->GetNamedAttribute(GeometryAttribute::TEX_COORD);
  if (att == nullptr || att->size() == 0) {
    return true;  // It's OK if we don't have texture coordinates.
  }
  std::array<float, 2> value;
  for (AttributeValueIndex i(0); i < static_cast<uint32_t>(att->size()); ++i) {
    if (!att->ConvertValue<float, 2>(i, &value[0])) {
      return false;
    }
    buffer()->Encode("vt ", 3);
    EncodeFloatList(&value[0], 2);
    buffer()->Encode("\n", 1);
  }
  tex_coord_att_ = att;
  return true;
}

bool ObjEncoder::EncodeNormals() {
  const PointAttribute *const att =
      in_point_cloud_->GetNamedAttribute(GeometryAttribute::NORMAL);
  if (att == nullptr || att->size() == 0) {
    return true;  // It's OK if we don't have normals.
  }
  std::array<float, 3> value;
  for (AttributeValueIndex i(0); i < static_cast<uint32_t>(att->size()); ++i) {
    if (!att->ConvertValue<float, 3>(i, &value[0])) {
      return false;
    }
    buffer()->Encode("vn ", 3);
    EncodeFloatList(&value[0], 3);
    buffer()->Encode("\n", 1);
  }
  normal_att_ = att;
  return true;
}

bool ObjEncoder::EncodeFaces() {
  for (FaceIndex i(0); i < in_mesh_->num_faces(); ++i) {
    if (sub_obj_att_) {
      if (!EncodeSubObject(i)) {
        return false;
      }
    }
    if (material_att_) {
      if (!EncodeMaterial(i)) {
        return false;
      }
    }
    buffer()->Encode('f');
    for (int j = 0; j < 3; ++j) {
      if (!EncodeFaceCorner(i, j)) {
        return false;
      }
    }
    buffer()->Encode("\n", 1);
  }
  return true;
}

bool ObjEncoder::EncodeMaterial(FaceIndex face_id) {
  int material_id = 0;
  // Pick the first corner, all corners of a face should have same id.
  const PointIndex vert_index = in_mesh_->face(face_id)[0];
  const AttributeValueIndex index_id(material_att_->mapped_index(vert_index));
  if (!material_att_->ConvertValue<int>(index_id, &material_id)) {
    return false;
  }

  if (material_id != current_material_id_) {
    // Update material information.
    buffer()->Encode("usemtl ", 7);
    const auto mat_ptr = material_id_to_name_.find(material_id);
    // If the material id is not found.
    if (mat_ptr == material_id_to_name_.end()) {
      return false;
    }
    buffer()->Encode(mat_ptr->second.c_str(), mat_ptr->second.size());
    buffer()->Encode("\n", 1);
    current_material_id_ = material_id;
  }
  return true;
}

bool ObjEncoder::EncodeSubObject(FaceIndex face_id) {
  int sub_obj_id = 0;
  // Pick the first corner, all corners of a face should have same id.
  const PointIndex vert_index = in_mesh_->face(face_id)[0];
  const AttributeValueIndex index_id(sub_obj_att_->mapped_index(vert_index));
  if (!sub_obj_att_->ConvertValue<int>(index_id, &sub_obj_id)) {
    return false;
  }
  if (sub_obj_id != current_sub_obj_id_) {
    buffer()->Encode("o ", 2);
    const auto sub_obj_ptr = sub_obj_id_to_name_.find(sub_obj_id);
    if (sub_obj_ptr == sub_obj_id_to_name_.end()) {
      return false;
    }
    buffer()->Encode(sub_obj_ptr->second.c_str(), sub_obj_ptr->second.size());
    buffer()->Encode("\n", 1);
    current_sub_obj_id_ = sub_obj_id;
  }
  return true;
}

bool ObjEncoder::EncodeFaceCorner(FaceIndex face_id, int local_corner_id) {
  buffer()->Encode(' ');
  const PointIndex vert_index = in_mesh_->face(face_id)[local_corner_id];
  // Note that in the OBJ format, all indices are encoded starting from index 1.
  // Encode position index.
  EncodeInt(pos_att_->mapped_index(vert_index).value() + 1);
  if (tex_coord_att_ || normal_att_) {
    // Encoding format is pos_index/tex_coord_index/normal_index.
    // If tex_coords are not present, we must encode pos_index//normal_index.
    buffer()->Encode('/');
    if (tex_coord_att_) {
      EncodeInt(tex_coord_att_->mapped_index(vert_index).value() + 1);
    }
    if (normal_att_) {
      buffer()->Encode('/');
      EncodeInt(normal_att_->mapped_index(vert_index).value() + 1);
    }
  }
  return true;
}

void ObjEncoder::EncodeFloat(float val) {
  snprintf(num_buffer_, sizeof(num_buffer_), "%f", val);
  buffer()->Encode(num_buffer_, strlen(num_buffer_));
}

void ObjEncoder::EncodeFloatList(float *vals, int num_vals) {
  for (int i = 0; i < num_vals; ++i) {
    if (i > 0) {
      buffer()->Encode(' ');
    }
    EncodeFloat(vals[i]);
  }
}

void ObjEncoder::EncodeInt(int32_t val) {
  snprintf(num_buffer_, sizeof(num_buffer_), "%d", val);
  buffer()->Encode(num_buffer_, strlen(num_buffer_));
}

}  // namespace draco
