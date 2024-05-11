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
#ifndef DRACO_IO_OBJ_ENCODER_H_
#define DRACO_IO_OBJ_ENCODER_H_

#include <unordered_map>

#include "draco/core/encoder_buffer.h"
#include "draco/mesh/mesh.h"

namespace draco {

// Class for encoding input draco::Mesh or draco::PointCloud into the Wavefront
// OBJ format.
class ObjEncoder {
 public:
  ObjEncoder();

  // Encodes the mesh or a point cloud  and saves it into a file.
  // Returns false when either the encoding failed or when the file couldn't be
  // opened.
  bool EncodeToFile(const PointCloud &pc, const std::string &file_name);
  bool EncodeToFile(const Mesh &mesh, const std::string &file_name);

  // Encodes the mesh or the point cloud into a buffer.
  bool EncodeToBuffer(const PointCloud &pc, EncoderBuffer *out_buffer);
  bool EncodeToBuffer(const Mesh &mesh, EncoderBuffer *out_buffer);

 protected:
  bool EncodeInternal();
  EncoderBuffer *buffer() const { return out_buffer_; }
  bool ExitAndCleanup(bool return_value);

 private:
  bool GetSubObjects();
  bool EncodeMaterialFileName();
  bool EncodePositions();
  bool EncodeTextureCoordinates();
  bool EncodeNormals();
  bool EncodeFaces();
  bool EncodeSubObject(FaceIndex face_id);
  bool EncodeMaterial(FaceIndex face_id);
  bool EncodeFaceCorner(FaceIndex face_id, int local_corner_id);

  void EncodeFloat(float val);
  void EncodeFloatList(float *vals, int num_vals);
  void EncodeInt(int32_t val);

  // Various attributes used by the encoder. If an attribute is not used, it is
  // set to nullptr.
  const PointAttribute *pos_att_;
  const PointAttribute *tex_coord_att_;
  const PointAttribute *normal_att_;
  const PointAttribute *material_att_;
  const PointAttribute *sub_obj_att_;

  // Buffer used for encoding float/int numbers.
  char num_buffer_[20];

  EncoderBuffer *out_buffer_;

  const PointCloud *in_point_cloud_;
  const Mesh *in_mesh_;

  // Store sub object name for each value.
  std::unordered_map<int, std::string> sub_obj_id_to_name_;
  // Current sub object id of faces.
  int current_sub_obj_id_;

  // Store material name for each value in material attribute.
  std::unordered_map<int, std::string> material_id_to_name_;
  // Current material id of faces.
  int current_material_id_;

  std::string file_name_;
};

}  // namespace draco

#endif  // DRACO_IO_OBJ_ENCODER_H_
