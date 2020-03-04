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

#include <cctype>
#include <cmath>

#include "draco/io/file_utils.h"
#include "draco/io/parser_utils.h"
#include "draco/metadata/geometry_metadata.h"

namespace draco {

ObjDecoder::ObjDecoder()
    : counting_mode_(true),
      num_obj_faces_(0),
      num_positions_(0),
      num_tex_coords_(0),
      num_normals_(0),
      num_materials_(0),
      last_sub_obj_id_(0),
      pos_att_id_(-1),
      tex_att_id_(-1),
      norm_att_id_(-1),
      material_att_id_(-1),
      sub_obj_att_id_(-1),
      deduplicate_input_values_(true),
      last_material_id_(0),
      use_metadata_(false),
      out_mesh_(nullptr),
      out_point_cloud_(nullptr) {}

Status ObjDecoder::DecodeFromFile(const std::string &file_name,
                                  Mesh *out_mesh) {
  out_mesh_ = out_mesh;
  return DecodeFromFile(file_name, static_cast<PointCloud *>(out_mesh));
}

Status ObjDecoder::DecodeFromFile(const std::string &file_name,
                                  PointCloud *out_point_cloud) {
  std::vector<char> buffer;
  if (!ReadFileToBuffer(file_name, &buffer)) {
    return Status(Status::DRACO_ERROR, "Unable to read input file.");
  }
  buffer_.Init(buffer.data(), buffer.size());

  out_point_cloud_ = out_point_cloud;
  input_file_name_ = file_name;
  return DecodeInternal();
}

Status ObjDecoder::DecodeFromBuffer(DecoderBuffer *buffer, Mesh *out_mesh) {
  out_mesh_ = out_mesh;
  return DecodeFromBuffer(buffer, static_cast<PointCloud *>(out_mesh));
}

Status ObjDecoder::DecodeFromBuffer(DecoderBuffer *buffer,
                                    PointCloud *out_point_cloud) {
  out_point_cloud_ = out_point_cloud;
  buffer_.Init(buffer->data_head(), buffer->remaining_size());
  return DecodeInternal();
}

Status ObjDecoder::DecodeInternal() {
  // In the first pass, count the number of different elements in the geometry.
  // In case the desired output is just a point cloud (i.e., when
  // out_mesh_ == nullptr) the decoder will ignore all information about the
  // connectivity that may be included in the source data.
  counting_mode_ = true;
  ResetCounters();
  material_name_to_id_.clear();
  last_sub_obj_id_ = 0;
  // Parse all lines.
  Status status(Status::OK);
  while (ParseDefinition(&status) && status.ok()) {
  }
  if (!status.ok()) {
    return status;
  }

  bool use_identity_mapping = false;
  if (num_obj_faces_ == 0) {
    // Mesh has no faces. In this case we try to read the geometry as a point
    // cloud where every attribute entry is a point.

    // Ensure the number of all entries is same for all attributes.
    if (num_positions_ == 0) {
      return Status(Status::DRACO_ERROR, "No position attribute");
    }
    if (num_tex_coords_ > 0 && num_tex_coords_ != num_positions_) {
      return Status(Status::DRACO_ERROR,
                    "Invalid number of texture coordinates for a point cloud");
    }
    if (num_normals_ > 0 && num_normals_ != num_positions_) {
      return Status(Status::DRACO_ERROR,
                    "Invalid number of normals for a point cloud");
    }

    out_mesh_ = nullptr;  // Treat the output geometry as a point cloud.
    use_identity_mapping = true;
  }

  // Initialize point cloud and mesh properties.
  if (out_mesh_) {
    // Start decoding a mesh with the given number of faces. For point clouds we
    // silently ignore all data about the mesh connectivity.
    out_mesh_->SetNumFaces(num_obj_faces_);
  }
  if (num_obj_faces_ > 0) {
    out_point_cloud_->set_num_points(3 * num_obj_faces_);
  } else {
    out_point_cloud_->set_num_points(num_positions_);
  }

  // Add attributes if they are present in the input data.
  if (num_positions_ > 0) {
    GeometryAttribute va;
    va.Init(GeometryAttribute::POSITION, nullptr, 3, DT_FLOAT32, false,
            sizeof(float) * 3, 0);
    pos_att_id_ = out_point_cloud_->AddAttribute(va, use_identity_mapping,
                                                 num_positions_);
  }
  if (num_tex_coords_ > 0) {
    GeometryAttribute va;
    va.Init(GeometryAttribute::TEX_COORD, nullptr, 2, DT_FLOAT32, false,
            sizeof(float) * 2, 0);
    tex_att_id_ = out_point_cloud_->AddAttribute(va, use_identity_mapping,
                                                 num_tex_coords_);
  }
  if (num_normals_ > 0) {
    GeometryAttribute va;
    va.Init(GeometryAttribute::NORMAL, nullptr, 3, DT_FLOAT32, false,
            sizeof(float) * 3, 0);
    norm_att_id_ =
        out_point_cloud_->AddAttribute(va, use_identity_mapping, num_normals_);
  }
  if (num_materials_ > 0 && num_obj_faces_ > 0) {
    GeometryAttribute va;
    const auto geometry_attribute_type = GeometryAttribute::GENERIC;
    if (num_materials_ < 256) {
      va.Init(geometry_attribute_type, nullptr, 1, DT_UINT8, false, 1, 0);
    } else if (num_materials_ < (1 << 16)) {
      va.Init(geometry_attribute_type, nullptr, 1, DT_UINT16, false, 2, 0);
    } else {
      va.Init(geometry_attribute_type, nullptr, 1, DT_UINT32, false, 4, 0);
    }
    material_att_id_ =
        out_point_cloud_->AddAttribute(va, false, num_materials_);

    // Fill the material entries.
    for (int i = 0; i < num_materials_; ++i) {
      const AttributeValueIndex avi(i);
      out_point_cloud_->attribute(material_att_id_)->SetAttributeValue(avi, &i);
    }

    if (use_metadata_) {
      // Use metadata to store the name of materials.
      std::unique_ptr<AttributeMetadata> material_metadata =
          std::unique_ptr<AttributeMetadata>(new AttributeMetadata());
      material_metadata->AddEntryString("name", "material");
      // Add all material names.
      for (const auto &itr : material_name_to_id_) {
        material_metadata->AddEntryInt(itr.first, itr.second);
      }
      if (!material_file_name_.empty()) {
        material_metadata->AddEntryString("file_name", material_file_name_);
      }

      out_point_cloud_->AddAttributeMetadata(material_att_id_,
                                             std::move(material_metadata));
    }
  }
  if (!obj_name_to_id_.empty() && num_obj_faces_ > 0) {
    GeometryAttribute va;
    if (obj_name_to_id_.size() < 256) {
      va.Init(GeometryAttribute::GENERIC, nullptr, 1, DT_UINT8, false, 1, 0);
    } else if (obj_name_to_id_.size() < (1 << 16)) {
      va.Init(GeometryAttribute::GENERIC, nullptr, 1, DT_UINT16, false, 2, 0);
    } else {
      va.Init(GeometryAttribute::GENERIC, nullptr, 1, DT_UINT32, false, 4, 0);
    }
    sub_obj_att_id_ = out_point_cloud_->AddAttribute(
        va, false, static_cast<uint32_t>(obj_name_to_id_.size()));
    // Fill the sub object id entries.
    for (const auto &itr : obj_name_to_id_) {
      const AttributeValueIndex i(itr.second);
      out_point_cloud_->attribute(sub_obj_att_id_)->SetAttributeValue(i, &i);
    }
    if (use_metadata_) {
      // Use metadata to store the name of materials.
      std::unique_ptr<AttributeMetadata> sub_obj_metadata =
          std::unique_ptr<AttributeMetadata>(new AttributeMetadata());
      sub_obj_metadata->AddEntryString("name", "sub_obj");
      // Add all sub object names.
      for (const auto &itr : obj_name_to_id_) {
        const AttributeValueIndex i(itr.second);
        sub_obj_metadata->AddEntryInt(itr.first, itr.second);
      }
      out_point_cloud_->AddAttributeMetadata(sub_obj_att_id_,
                                             std::move(sub_obj_metadata));
    }
  }

  // Perform a second iteration of parsing and fill all the data.
  counting_mode_ = false;
  ResetCounters();
  // Start parsing from the beginning of the buffer again.
  buffer()->StartDecodingFrom(0);
  while (ParseDefinition(&status) && status.ok()) {
  }
  if (!status.ok()) {
    return status;
  }
  if (out_mesh_) {
    // Add faces with identity mapping between vertex and corner indices.
    // Duplicate vertices will get removed later.
    Mesh::Face face;
    for (FaceIndex i(0); i < num_obj_faces_; ++i) {
      for (int c = 0; c < 3; ++c) {
        face[c] = 3 * i.value() + c;
      }
      out_mesh_->SetFace(i, face);
    }
  }

#ifdef DRACO_ATTRIBUTE_VALUES_DEDUPLICATION_SUPPORTED
  if (deduplicate_input_values_) {
    out_point_cloud_->DeduplicateAttributeValues();
  }
#endif
#ifdef DRACO_ATTRIBUTE_INDICES_DEDUPLICATION_SUPPORTED
  out_point_cloud_->DeduplicatePointIds();
#endif
  return status;
}

void ObjDecoder::ResetCounters() {
  num_obj_faces_ = 0;
  num_positions_ = 0;
  num_tex_coords_ = 0;
  num_normals_ = 0;
  last_material_id_ = 0;
  last_sub_obj_id_ = 0;
}

bool ObjDecoder::ParseDefinition(Status *status) {
  char c;
  parser::SkipWhitespace(buffer());
  if (!buffer()->Peek(&c)) {
    // End of file reached?.
    return false;
  }
  if (c == '#') {
    // Comment, ignore the line.
    parser::SkipLine(buffer());
    return true;
  }
  if (ParseVertexPosition(status)) {
    return true;
  }
  if (ParseNormal(status)) {
    return true;
  }
  if (ParseTexCoord(status)) {
    return true;
  }
  if (ParseFace(status)) {
    return true;
  }
  if (ParseMaterial(status)) {
    return true;
  }
  if (ParseMaterialLib(status)) {
    return true;
  }
  if (ParseObject(status)) {
    return true;
  }
  // No known definition was found. Ignore the line.
  parser::SkipLine(buffer());
  return true;
}

bool ObjDecoder::ParseVertexPosition(Status *status) {
  std::array<char, 2> c;
  if (!buffer()->Peek(&c)) {
    return false;
  }
  if (c[0] != 'v' || c[1] != ' ') {
    return false;
  }
  // Vertex definition found!
  buffer()->Advance(2);
  if (!counting_mode_) {
    // Parse three float numbers for vertex position coordinates.
    float val[3];
    for (int i = 0; i < 3; ++i) {
      parser::SkipWhitespace(buffer());
      if (!parser::ParseFloat(buffer(), val + i)) {
        *status = Status(Status::DRACO_ERROR, "Failed to parse a float number");
        // The definition is processed so return true.
        return true;
      }
    }
    out_point_cloud_->attribute(pos_att_id_)
        ->SetAttributeValue(AttributeValueIndex(num_positions_), val);
  }
  ++num_positions_;
  parser::SkipLine(buffer());
  return true;
}

bool ObjDecoder::ParseNormal(Status *status) {
  std::array<char, 2> c;
  if (!buffer()->Peek(&c)) {
    return false;
  }
  if (c[0] != 'v' || c[1] != 'n') {
    return false;
  }
  // Normal definition found!
  buffer()->Advance(2);
  if (!counting_mode_) {
    // Parse three float numbers for the normal vector.
    float val[3];
    for (int i = 0; i < 3; ++i) {
      parser::SkipWhitespace(buffer());
      if (!parser::ParseFloat(buffer(), val + i)) {
        *status = Status(Status::DRACO_ERROR, "Failed to parse a float number");
        // The definition is processed so return true.
        return true;
      }
    }
    out_point_cloud_->attribute(norm_att_id_)
        ->SetAttributeValue(AttributeValueIndex(num_normals_), val);
  }
  ++num_normals_;
  parser::SkipLine(buffer());
  return true;
}

bool ObjDecoder::ParseTexCoord(Status *status) {
  std::array<char, 2> c;
  if (!buffer()->Peek(&c)) {
    return false;
  }
  if (c[0] != 'v' || c[1] != 't') {
    return false;
  }
  // Texture coord definition found!
  buffer()->Advance(2);
  if (!counting_mode_) {
    // Parse two float numbers for the texture coordinate.
    float val[2];
    for (int i = 0; i < 2; ++i) {
      parser::SkipWhitespace(buffer());
      if (!parser::ParseFloat(buffer(), val + i)) {
        *status = Status(Status::DRACO_ERROR, "Failed to parse a float number");
        // The definition is processed so return true.
        return true;
      }
    }
    out_point_cloud_->attribute(tex_att_id_)
        ->SetAttributeValue(AttributeValueIndex(num_tex_coords_), val);
  }
  ++num_tex_coords_;
  parser::SkipLine(buffer());
  return true;
}

bool ObjDecoder::ParseFace(Status *status) {
  char c;
  if (!buffer()->Peek(&c)) {
    return false;
  }
  if (c != 'f') {
    return false;
  }
  // Face definition found!
  buffer()->Advance(1);
  if (!counting_mode_) {
    std::array<int32_t, 3> indices[4];
    // Parse face indices (we try to look for up to four to support quads).
    int num_valid_indices = 0;
    for (int i = 0; i < 4; ++i) {
      if (!ParseVertexIndices(&indices[i])) {
        if (i == 3) {
          break;  // It's OK if there is no fourth vertex index.
        }
        *status = Status(Status::DRACO_ERROR, "Failed to parse vertex indices");
        return true;
      }
      ++num_valid_indices;
    }
    // Process the first face.
    for (int i = 0; i < 3; ++i) {
      const PointIndex vert_id(3 * num_obj_faces_ + i);
      MapPointToVertexIndices(vert_id, indices[i]);
    }
    ++num_obj_faces_;
    if (num_valid_indices == 4) {
      // Add an additional triangle for the quad.
      //
      //   3----2
      //   |  / |
      //   | /  |
      //   0----1
      //
      const PointIndex vert_id(3 * num_obj_faces_);
      MapPointToVertexIndices(vert_id, indices[0]);
      MapPointToVertexIndices(vert_id + 1, indices[2]);
      MapPointToVertexIndices(vert_id + 2, indices[3]);
      ++num_obj_faces_;
    }
  } else {
    // We are in the counting mode.
    // We need to determine how many triangles are in the obj face.
    // Go over the line and check how many gaps there are between non-empty
    // sub-strings.
    parser::SkipWhitespace(buffer());
    int num_indices = 0;
    bool is_end = false;
    while (buffer()->Peek(&c) && c != '\n') {
      if (parser::PeekWhitespace(buffer(), &is_end)) {
        buffer()->Advance(1);
      } else {
        // Non-whitespace reached.. assume it's index declaration, skip it.
        num_indices++;
        while (!parser::PeekWhitespace(buffer(), &is_end) && !is_end) {
          buffer()->Advance(1);
        }
      }
    }
    if (num_indices < 3 || num_indices > 4) {
      *status =
          Status(Status::DRACO_ERROR, "Invalid number of indices on a face");
      return false;
    }
    // Either one or two new triangles.
    num_obj_faces_ += num_indices - 2;
  }
  parser::SkipLine(buffer());
  return true;
}

bool ObjDecoder::ParseMaterialLib(Status *status) {
  // Allow only one material library per file for now.
  if (!material_name_to_id_.empty()) {
    return false;
  }
  std::array<char, 6> c;
  if (!buffer()->Peek(&c)) {
    return false;
  }
  if (std::memcmp(&c[0], "mtllib", 6) != 0) {
    return false;
  }
  buffer()->Advance(6);
  DecoderBuffer line_buffer = parser::ParseLineIntoDecoderBuffer(buffer());
  parser::SkipWhitespace(&line_buffer);
  material_file_name_.clear();
  if (!parser::ParseString(&line_buffer, &material_file_name_)) {
    *status = Status(Status::DRACO_ERROR, "Failed to parse material file name");
    return true;
  }
  parser::SkipLine(&line_buffer);

  if (!material_file_name_.empty()) {
    if (!ParseMaterialFile(material_file_name_, status)) {
      // Silently ignore problems with material files for now.
      return true;
    }
  }
  return true;
}

bool ObjDecoder::ParseMaterial(Status * /* status */) {
  // In second pass, skip when we don't use materials.
  if (!counting_mode_ && material_att_id_ < 0) {
    return false;
  }
  std::array<char, 6> c;
  if (!buffer()->Peek(&c)) {
    return false;
  }
  if (std::memcmp(&c[0], "usemtl", 6) != 0) {
    return false;
  }
  buffer()->Advance(6);
  DecoderBuffer line_buffer = parser::ParseLineIntoDecoderBuffer(buffer());
  parser::SkipWhitespace(&line_buffer);
  std::string mat_name;
  parser::ParseLine(&line_buffer, &mat_name);
  if (mat_name.length() == 0) {
    return false;
  }
  auto it = material_name_to_id_.find(mat_name);
  if (it == material_name_to_id_.end()) {
    // In first pass, materials found in obj that's not in the .mtl file
    // will be added to the list.
    last_material_id_ = num_materials_;
    material_name_to_id_[mat_name] = num_materials_++;

    return true;
  }
  last_material_id_ = it->second;
  return true;
}

bool ObjDecoder::ParseObject(Status *status) {
  std::array<char, 2> c;
  if (!buffer()->Peek(&c)) {
    return false;
  }
  if (std::memcmp(&c[0], "o ", 2) != 0) {
    return false;
  }
  buffer()->Advance(1);
  DecoderBuffer line_buffer = parser::ParseLineIntoDecoderBuffer(buffer());
  parser::SkipWhitespace(&line_buffer);
  std::string obj_name;
  if (!parser::ParseString(&line_buffer, &obj_name)) {
    return false;
  }
  if (obj_name.length() == 0) {
    return true;  // Ignore empty name entries.
  }
  auto it = obj_name_to_id_.find(obj_name);
  if (it == obj_name_to_id_.end()) {
    const int num_obj = static_cast<int>(obj_name_to_id_.size());
    obj_name_to_id_[obj_name] = num_obj;
    last_sub_obj_id_ = num_obj;
  } else {
    last_sub_obj_id_ = it->second;
  }
  return true;
}

bool ObjDecoder::ParseVertexIndices(std::array<int32_t, 3> *out_indices) {
  // Parsed attribute indices can be in format:
  // 1. POS_INDEX
  // 2. POS_INDEX/TEX_COORD_INDEX
  // 3. POS_INDEX/TEX_COORD_INDEX/NORMAL_INDEX
  // 4. POS_INDEX//NORMAL_INDEX
  parser::SkipCharacters(buffer(), " \t");
  if (!parser::ParseSignedInt(buffer(), &(*out_indices)[0]) ||
      (*out_indices)[0] == 0) {
    return false;  // Position index must be present and valid.
  }
  (*out_indices)[1] = (*out_indices)[2] = 0;
  char ch;
  if (!buffer()->Peek(&ch)) {
    return true;  // It may be OK if we cannot read any more characters.
  }
  if (ch != '/') {
    return true;
  }
  buffer()->Advance(1);
  // Check if we should skip texture index or not.
  if (!buffer()->Peek(&ch)) {
    return false;  // Here, we should be always able to read the next char.
  }
  if (ch != '/') {
    // Must be texture coord index.
    if (!parser::ParseSignedInt(buffer(), &(*out_indices)[1]) ||
        (*out_indices)[1] == 0) {
      return false;  // Texture index must be present and valid.
    }
  }
  if (!buffer()->Peek(&ch)) {
    return true;
  }
  if (ch == '/') {
    buffer()->Advance(1);
    // Read normal index.
    if (!parser::ParseSignedInt(buffer(), &(*out_indices)[2]) ||
        (*out_indices)[2] == 0) {
      return false;  // Normal index must be present and valid.
    }
  }
  return true;
}

void ObjDecoder::MapPointToVertexIndices(
    PointIndex vert_id, const std::array<int32_t, 3> &indices) {
  // Use face entries to store mapping between vertex and attribute indices
  // (positions, texture coordinates and normal indices).
  // Any given index is used when indices[x] != 0. For positive values, the
  // point is mapped directly to the specified attribute index. Negative input
  // indices indicate addressing from the last element (e.g. -1 is the last
  // attribute value of a given type, -2 the second last, etc.).
  if (indices[0] > 0) {
    out_point_cloud_->attribute(pos_att_id_)
        ->SetPointMapEntry(vert_id, AttributeValueIndex(indices[0] - 1));
  } else if (indices[0] < 0) {
    out_point_cloud_->attribute(pos_att_id_)
        ->SetPointMapEntry(vert_id,
                           AttributeValueIndex(num_positions_ + indices[0]));
  }

  if (tex_att_id_ >= 0) {
    if (indices[1] > 0) {
      out_point_cloud_->attribute(tex_att_id_)
          ->SetPointMapEntry(vert_id, AttributeValueIndex(indices[1] - 1));
    } else if (indices[1] < 0) {
      out_point_cloud_->attribute(tex_att_id_)
          ->SetPointMapEntry(vert_id,
                             AttributeValueIndex(num_tex_coords_ + indices[1]));
    } else {
      // Texture index not provided but expected. Insert 0 entry as the
      // default value.
      out_point_cloud_->attribute(tex_att_id_)
          ->SetPointMapEntry(vert_id, AttributeValueIndex(0));
    }
  }

  if (norm_att_id_ >= 0) {
    if (indices[2] > 0) {
      out_point_cloud_->attribute(norm_att_id_)
          ->SetPointMapEntry(vert_id, AttributeValueIndex(indices[2] - 1));
    } else if (indices[2] < 0) {
      out_point_cloud_->attribute(norm_att_id_)
          ->SetPointMapEntry(vert_id,
                             AttributeValueIndex(num_normals_ + indices[2]));
    } else {
      // Normal index not provided but expected. Insert 0 entry as the default
      // value.
      out_point_cloud_->attribute(norm_att_id_)
          ->SetPointMapEntry(vert_id, AttributeValueIndex(0));
    }
  }

  // Assign material index to the point if it is available.
  if (material_att_id_ >= 0) {
    out_point_cloud_->attribute(material_att_id_)
        ->SetPointMapEntry(vert_id, AttributeValueIndex(last_material_id_));
  }

  // Assign sub-object index to the point if it is available.
  if (sub_obj_att_id_ >= 0) {
    out_point_cloud_->attribute(sub_obj_att_id_)
        ->SetPointMapEntry(vert_id, AttributeValueIndex(last_sub_obj_id_));
  }
}

bool ObjDecoder::ParseMaterialFile(const std::string &file_name,
                                   Status *status) {
  const std::string full_path = GetFullPath(file_name, input_file_name_);
  std::vector<char> buffer;
  if (!ReadFileToBuffer(full_path, &buffer)) {
    return false;
  }

  // Backup the original decoder buffer.
  DecoderBuffer old_buffer = buffer_;

  buffer_.Init(buffer.data(), buffer.size());

  num_materials_ = 0;
  while (ParseMaterialFileDefinition(status)) {
  }

  // Restore the original buffer.
  buffer_ = old_buffer;
  return true;
}

bool ObjDecoder::ParseMaterialFileDefinition(Status * /* status */) {
  char c;
  parser::SkipWhitespace(buffer());
  if (!buffer()->Peek(&c)) {
    // End of file reached?.
    return false;
  }
  if (c == '#') {
    // Comment, ignore the line.
    parser::SkipLine(buffer());
    return true;
  }
  std::string str;
  if (!parser::ParseString(buffer(), &str)) {
    return false;
  }
  if (str == "newmtl") {
    parser::SkipWhitespace(buffer());
    parser::ParseLine(buffer(), &str);
    if (str.empty()) {
      return false;
    }
    // Add new material to our map.
    material_name_to_id_[str] = num_materials_++;
  }
  return true;
}

}  // namespace draco
