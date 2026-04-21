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
#ifndef DRACO_IO_OBJ_DECODER_H_
#define DRACO_IO_OBJ_DECODER_H_

#include <string>
#include <unordered_map>

#include "draco/core/decoder_buffer.h"
#include "draco/core/status.h"
#include "draco/draco_features.h"
#include "draco/mesh/mesh.h"

namespace draco {

// Decodes a Wavefront OBJ file into draco::Mesh (or draco::PointCloud if the
// connectivity data is not needed).. This decoder can handle decoding of
// positions, texture coordinates, normals and triangular faces.
// All other geometry properties are ignored.
class ObjDecoder {
 public:
  ObjDecoder();

  // Decodes an obj file stored in the input file.
  // Optional argument |mesh_files| will be populated with all paths to files
  // relevant to the loaded mesh.
  Status DecodeFromFile(const std::string &file_name, Mesh *out_mesh);
  Status DecodeFromFile(const std::string &file_name, Mesh *out_mesh,
                        std::vector<std::string> *mesh_files);

  Status DecodeFromFile(const std::string &file_name,
                        PointCloud *out_point_cloud);

  Status DecodeFromBuffer(DecoderBuffer *buffer, Mesh *out_mesh);
  Status DecodeFromBuffer(DecoderBuffer *buffer, PointCloud *out_point_cloud);

  // Flag that can be used to turn on/off deduplication of input values.
  // This should be disabled only when we are sure that the input data does not
  // contain any duplicate entries.
  // Default: true
  void set_deduplicate_input_values(bool v) { deduplicate_input_values_ = v; }
  // Flag for whether using metadata to record other information in the obj
  // file, e.g. material names, object names.
  void set_use_metadata(bool flag) { use_metadata_ = flag; }
  // Enables preservation of polygons.
  void set_preserve_polygons(bool flag) { preserve_polygons_ = flag; }

 protected:
  Status DecodeInternal();
  DecoderBuffer *buffer() { return &buffer_; }

 private:
  // Resets internal counters for attributes and faces.
  void ResetCounters();

  // Parses the next mesh property definition (position, tex coord, normal, or
  // face). If the parsed data is unrecognized, it will be skipped.
  // Returns false when the end of file was reached.
  bool ParseDefinition(Status *status);

  // Attempts to parse definition of position, normal, tex coord, or face
  // respectively.
  // Returns false when the parsed data didn't contain the given definition.
  bool ParseVertexPosition(Status *status);
  bool ParseNormal(Status *status);
  bool ParseTexCoord(Status *status);
  bool ParseFace(Status *status);
  bool ParseMaterialLib(Status *status);
  bool ParseMaterial(Status *status);
  bool ParseObject(Status *status);

  // Parses triplet of position, tex coords and normal indices.
  // Returns false on error.
  bool ParseVertexIndices(std::array<int32_t, 3> *out_indices);

  // Maps specified point index to the parsed vertex indices (triplet of
  // position, texture coordinate, and normal indices) .
  void MapPointToVertexIndices(PointIndex vert_id,
                               const std::array<int32_t, 3> &indices);

  // Parses material file definitions from a separate file.
  bool ParseMaterialFile(const std::string &file_name, Status *status);
  bool ParseMaterialFileDefinition(Status *status);

  // Methods related to polygon triangulation and preservation.
  static int Triangulate(int tri_index, int tri_corner);
  static bool IsNewEdge(int tri_count, int tri_index, int tri_corner);

 private:
  // If set to true, the parser will count the number of various definitions
  // but it will not parse the actual data or add any new entries to the mesh.
  bool counting_mode_;
  int num_obj_faces_;
  int num_positions_;
  int num_tex_coords_;
  int num_normals_;
  int num_materials_;
  int last_sub_obj_id_;

  int pos_att_id_;
  int tex_att_id_;
  int norm_att_id_;
  int material_att_id_;
  int sub_obj_att_id_;     // Attribute id for storing sub-objects.
  int added_edge_att_id_;  // Attribute id for polygon reconstruction.

  bool deduplicate_input_values_;

  int last_material_id_;
  std::string material_file_name_;

  std::string input_file_name_;

  std::unordered_map<std::string, int> material_name_to_id_;
  std::unordered_map<std::string, int> obj_name_to_id_;

  bool use_metadata_;

  // Polygon preservation flags.
  bool preserve_polygons_;
  bool has_polygons_;

  std::vector<std::string> *mesh_files_;

  DecoderBuffer buffer_;

  // Data structure that stores the decoded data. |out_point_cloud_| must be
  // always set but |out_mesh_| is optional.
  Mesh *out_mesh_;
  PointCloud *out_point_cloud_;
};

}  // namespace draco

#endif  // DRACO_IO_OBJ_DECODER_H_
